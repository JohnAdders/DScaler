////////////////////////////////////////////////////////////////////////////
// $Id: TimeShift.cpp,v 1.9 2001-11-20 11:43:00 temperton Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2001   Eric Schmidt          Original Release
//                                     Got 'record mode' working.
//                                     Got 'play mode' working.
//                                     Got 'prev/next' working.
//                                     More to come.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2001/11/04 14:44:58  adcockj
// Bug fixes
//
// Revision 1.7  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.6.2.3  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.6.2.2  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.6.2.1  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.6  2001/08/06 03:00:17  ericschmidt
// solidified auto-pixel-width detection
// preliminary pausing-of-live-tv work
//
// Revision 1.5  2001/07/27 15:52:26  ericschmidt
// P3-or-better handling.
// Preliminary pixel-width auto-setting on playback.
//
// Revision 1.4  2001/07/26 15:28:14  ericschmidt
// Added AVI height control, i.e. even/odd/averaged lines.
// Used existing cpu/mmx detection in TimeShift code.
//
// Revision 1.3  2001/07/24 12:25:49  adcockj
// Added copyright notice as per standards
//
// Revision 1.2  2001/07/24 12:24:25  adcockj
// Added Id to comment block
//
// Revision 1.1  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"       // manditory precompiled header first
#include "TimeShift.h"    // this module
#include "DScaler.h"      // hWnd global
#include "TSOptionsDlg.h" // CTSOptionsDlg
#include "MixerDev.h"     // Mute and UnMute
#include "Settings.h"     // Setting_Set/GetValue
#include "Cpu.h"          // CpuFeatureFlags
// TODO: remove 
#include "OutThreads.h"

/*
  Here are some suggestions for TODO tasks...

  NEXT:
  - For pause-then-playbackshifted code...
    - can't write frames once pGetFrame is created.
    - write our own compressor/decompressor?
    - use no compression for now with AVIStreamRead() instead of getframe?
    - actually, it works right now, but the file it creates is a corrupt AVI.

  LATER:
  - Add a 'Preferred Pixelwidth While Recording' edit as an option.
  - in options dialog, add waveform buffer count and size (advanced section).
  - enumerate saved avi file dynamic radio checked menu items.
  - make sure that if m_mode is STOPPED, memory & resource usage is minimized.
  - Remove 4GB filesize limitation.  Maybe try stop then immeditate rerecord.
  - BUG: During playback in windowed mode, writing too wide of lines.
  - TWEAK: You can see the live feed peek in between clips during play mode.
  - Add slowforward/backward feature.
  - Play audio (option) during fastforward/backward.
  - Implement full height recording option.
  - Use the standard DScaler Settings for ini file stuff?
  - Use -height for right-side-up AVI?  Matches odd/even field orientation.

  OPTIMIZE:
  - Is there a codec out there that takes YUY2, compress it, but don't convert
    to RGB in the process?  If so, we can get rid of our rgb<-->yuv routines.

  DOC:
  - Must setup Audio Mixer through its dialog.  You can then uncheck the box
    if you don't normally use it, but all the dropdowns should point to the
    input on your audio card that your capture card's audio is attached to.
  - For speed, video clips are saved at 1/2 height.  Perhaps future versions can
    be optimized to handle full-height recording.
  - pixel width affects DF/S.  Choose a low enough pixel width for 0 DF/S.
  - Currently, If you record with audio at, say, 44.1kHz, etc, make sure you
    have these same settings on playback.

  LATEST CHANGES:
  - Solidified the auto-pixelwidth-setting feature for AVIs of varying width.
  - Added preliminary pause feature.  Left it disabled since it's unstable.
  - Added use of mixerdev to mute, but still record audio during pause/shifting.
  - Made public static function pointers to the mmx/c yuv<->rgb converters.
  - Disabled all height control features.  They made pause/shifting difficult.
*/


/////////////////////////////// Static Interface ///////////////////////////////

bool TimeShift::OnDestroy(void)
{
    bool result = false;

    if (m_pTimeShift)
    {
        delete m_pTimeShift;
        m_pTimeShift = NULL;

        result = true;
    }

    return result;
}

bool TimeShift::OnRecord(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only start recording if we're stopped.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ?
            m_pTimeShift->Record(false) : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnPause(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only start "time shifing" if we're stopped.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ?
            m_pTimeShift->Record(true) : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnPlay(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Save this off before we start playing.
        m_pTimeShift->m_origPixelWidth = CurrentX;

        // Only start playing if we're stopped.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ||
            m_pTimeShift->m_mode == MODE_PAUSED ?
            m_pTimeShift->Play() : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnStop(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only stop recording if we're recording.
        result =
            m_pTimeShift->m_mode == MODE_RECORDING ||
            m_pTimeShift->m_mode == MODE_PAUSED ||
            m_pTimeShift->m_mode == MODE_PLAYING ||
            m_pTimeShift->m_mode == MODE_SHIFTING ?
            m_pTimeShift->Stop() : false;

        // Reset the user's pixel width outside the Stop function since we
        // call it between clips and pixelwidth-setting is slow.
        if (result && CurrentX != m_pTimeShift->m_origPixelWidth)
            SetBT848PixelWidth(m_pTimeShift->m_origPixelWidth);

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnGoNext(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only track skip if playing.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ||
            m_pTimeShift->m_mode == MODE_PLAYING ?
            m_pTimeShift->GoNext() : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnGoPrev(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only track skip if playing.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ||
            m_pTimeShift->m_mode == MODE_PLAYING ?
            m_pTimeShift->GoPrev() : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnOptions(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        // No enter critical section here.  If we're stopped, there is no
        // need for one.  If we're recording, we'll show an error box and not
        // access critical member variables anyway so recording can continue.

        if (m_pTimeShift->m_mode == MODE_STOPPED)
        {
            CTSOptionsDlg dlg(CWnd::FromHandle(hWnd));
            result = dlg.DoModal() == IDOK;

            // Save off any changes we've made now, rather than in destructor.
            // Even if cancel was hit, there still may be new compressions
            // options to save.
            m_pTimeShift->WriteToIni();
        }
        else
        {
            MessageBox(hWnd,
                       "TimeShift options are only available during stop mode.",
                       "Information",
                       MB_OK);
        }
    }

    return result;
}

// Called from the capture thread.
bool TimeShift::OnNewFrame(DEINTERLACE_INFO *info)
{
    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only write the frame if we're recording.
        result =
            m_pTimeShift->m_mode == MODE_RECORDING ||
            m_pTimeShift->m_mode == MODE_PAUSED ||
            m_pTimeShift->m_mode == MODE_SHIFTING ?
            m_pTimeShift->WriteVideo(info) :
            m_pTimeShift->m_mode == MODE_PLAYING ?
            m_pTimeShift->ReadVideo(info) :
            false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnWaveInData(void)
{
    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only write the buffer if we're recording.
        result =
            m_pTimeShift->m_mode == MODE_RECORDING ||
            m_pTimeShift->m_mode == MODE_PAUSED ||
            m_pTimeShift->m_mode == MODE_SHIFTING ?
            m_pTimeShift->WriteAudio() : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnWaveOutDone(void)
{
    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Only read the buffer if we're playing.
        result =
            m_pTimeShift->m_mode == MODE_PLAYING ||
            m_pTimeShift->m_mode == MODE_SHIFTING ?
            m_pTimeShift->ReadAudio() : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::OnSetMenu(HMENU hMenu)
{
    bool result = false;

    int item = IDM_TSSTOP;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        switch (m_pTimeShift->m_mode)
        {
        default:
        case MODE_STOPPED:
            item = IDM_TSSTOP;
            break;

        case MODE_PLAYING:
            item = IDM_TSPLAY;
            break;

        case MODE_RECORDING:
        case MODE_SHIFTING:
            item = IDM_TSRECORD;
            break;

        case MODE_PAUSED:
            item = IDM_TSPAUSE;
            break;

        case MODE_FASTFWD:
            item = IDM_TSFFWD;
            break;

        case MODE_REWIND:
            item = IDM_TSRWND;
            break;
        }
    }

    CheckMenuRadioItem(hMenu, IDM_TSRECORD, IDM_TSNEXT, item, MF_BYCOMMAND);

    if (m_pTimeShift)
        LeaveCriticalSection(&m_pTimeShift->m_lock);

    return result;
}

////////////////////////////// Instance Interface //////////////////////////////

/*
  FIXME: Use this formula, it's our (BT and dScaler) "standard".

2.10.3 YCrCb to RGB Conversion

The 4:2:2 YCrCb data stream from the video decoder portion of the Fusion
878A
must be converted to 4:4:4 YCrCb before the RGB conversion occurs, using an
interpolation filter on the chroma data path. The even valid chroma data
passes through unmodified, while the odd data is generated by averaging
adjacent even data. The chroma component is up-sampled using the following
equations:

For n = 0, 2, 4, etc.
Cb n = Cb n
Cr n = Cr n
Cb n+1 = (Cb n + Cb n+2 )/2
Cr n+1 = (Cr n + Cr n+2 )/2

RGB Conversion:
R = 1.164(Y–16) + 1.596(Cr–128)
G = 1.164(Y–16) – 0.813(Cr–128) – 0.391(Cb–128)
B = 1.164(Y–16) + 2.018(Cb–128)
Y range = [16,235]
Cr/Cb range = [16,240]
RGB range = [0,255]

With Full Luma range on Luma is [0,255] and off it is [16,255]  I think this means that the correct functions are

When Full Luma range is on:

R = (Y) * 1.00 + (U - 128.0) *  0.00 + (V - 128.0) *  1.402;
G = (Y) * 1.00 + (U - 128.0) * -0.344136 + (V - 128.0) * -0.714136;
B = (Y) * 1.00 + (U - 128.0) *  1.772 + (V - 128.0) *  0.00;

When Full Luma range is off:

R = (Y - 16) * 1.0711 + (U - 128.0) *  0.00 + (V - 128.0) *  1.402;
G = (Y - 16) * 1.0711 + (U - 128.0) * -0.344136 + (V - 128.0) * -0.714136;
B = (Y - 16) * 1.0711 + (U - 128.0) *  1.772 + (V - 128.0) *  0.00;

*/

TimeShift *TimeShift::m_pTimeShift = NULL;
LPBYTE (*TimeShift::m_YUVtoRGB)(LPBYTE,short *,DWORD) = NULL;
LPBYTE (*TimeShift::m_AvgYUVtoRGB)(LPBYTE,short *,short *,DWORD) = NULL;
LPBYTE (*TimeShift::m_RGBtoYUV)(short *,LPBYTE,DWORD) = NULL;

// The mmx code requires at least a Pentium-III.
#define P3_OR_BETTER (FEATURE_SSE | FEATURE_MMXEXT)

/* yuv <--> rgb conversion

  Start with assumed "exact" values for rgb-to-yuv conversion.

  Y = R *  .299 + G *  .587 + B *  .114;
  U = R * -.169 + G * -.332 + B *  .500 + 128.;
  V = R *  .500 + G * -.419 + B * -.0813 + 128.;

  My adjustment...

  Y = R *  .299 + G *  .587 + B *  .114;
  U = R * -.168 + G * -.332 + B *  .500 + 128.;
  V = R *  .500 + G * -.419 + B * -.081 + 128.;

  In matrix form...

  | Y | = |  .299  .587  .114 |   | R |   |  0 |
  | U | = | -.168 -.332  .500 | x | G | + |128.|
  | V | = |  .500 -.419 -.081 |   | B |   |128.|

  | Y |   |  0 | = |  .299  .587  .114 |   | R |
  | U | - |128.| = | -.168 -.332  .500 | x | G |
  | V |   |128.| = |  .500 -.419 -.081 |   | B |

  |  .299  .587  .114 |-1   | Y        | = | R |
  | -.168 -.332  .500 |   x | U - 128. | = | G |
  |  .500 -.419 -.081 |     | V - 128. | = | B |

  | 1.0 -9.26427290271e-4 1.40168872043  |   | Y        | = | R |
  | 1.0 -0.34357761684   -0.713442079258 | x | U - 128. | = | G |
  | 1.0  1.77155318285   -0.002758130563 |   | V - 128. | = | B |
  
  (
   Or, if we use Intel's formulas... (see next func)

   | 1.0 -9.400500833-4  1.135920183  |   | Y        | = | R |
   | 1.0 -0.3959190767  -0.5782887929 | x | U - 128. | = | G |
   | 1.0  2.041101518   -0.001619414  |   | V - 128. | = | B |
  )

  So, we have...

  R = Y + (U - 128.0) * -9.26427290271e-4 + (V - 128.0) *  1.40168872043;
  G = Y + (U - 128.0) * -0.34357761684    + (V - 128.0) * -0.713442079258;
  B = Y + (U - 128.0) *  1.77155318285    + (V - 128.0) * -0.002758130563;

*/

// Note names are most-signif first, but the contents are least-signif first.
static unsigned short const8000[4] = { 0x8000, 0x8000, 0x8000, 0x8000 };
static unsigned short const00ff[4] = { 0x00ff, 0x00ff, 0x00ff, 0x00ff };
static short RvRuGvGu[4] = { -5629, -11689, -15, 22966 };
static short GvGuBvBu[4] = { 29025, -45, -5629, -11689 };
static short RvRuBvBu[4] = { 29025, -45, -15,    22966 };

// src will be calculated from the average of src1 and src2.
LPBYTE P3_YUVtoRGB(LPBYTE dest, short *src, DWORD w)
{
    _asm
    {
        mov esi, src
        mov edi, dest
        mov eax, w
        shr eax, 2

    next4:
        // Process 4 pixels.  First YUYV is a, second is b.
        // Source must be 8-byte aligned.
        movq mm1, [esi]     ; mm1 = Vb+128 Y1b Ub+128 Y0b  Va+128 Y1a Ua+128 Y0a
        add esi, 8
        pxor mm1, const8000    ; mm1 = Vb Y1b Ub Y0b  Va Y1a Ua Y0a

        movq mm0, mm1          ; mm0 = XX Y1b XX Y0b  XX Y1a XX Y0a
        psraw mm1, 8           ; mm1 = Vb Ub Va Ua

        pshufw mm2, mm1, 0x44  ; mm2 = Va Ua Va Ua
        pmaddwd mm2, RvRuGvGu  ; mm2 = preRa preGa
        psrad mm2, 14          ; mm2 >>= 14

        pshufw mm3, mm1, 0xee  ; mm3 = Vb Ub Vb Ub
        pmaddwd mm3, GvGuBvBu  ; mm3 = preGb preBb
        psrad mm3, 14          ; mm3 >>= 14

        pmaddwd mm1, RvRuBvBu  ; mm1 = preRb preBa
        psrad mm1, 14          ; mm1 >>= 14

        movq mm6, mm2          ; mm6 = preRa preGa
        packssdw mm2, mm1      ; mm2 = XXX preBa preRa preGa
        pshufw mm2, mm2, 0x92  ; mm2 = preBa preRa preGa preBa

        pand mm0, const00ff    ; mm0 = Y1b Y0b Y1a Y0a

        pshufw mm4, mm0, 0x40  ; mm4 = Y1a Y0a Y0a Y0a
        paddsw mm4, mm2        ; mm4 = B1a R0a G0a B0a

        packssdw mm6, mm3      ; mm6 = preGb preBb preRa preGa
        pshufw mm5, mm0, 0xa5  ; mm5 = Y0b Y0b Y1a Y1a
        paddsw mm5, mm6        ; mm5 = G0b B0b R1a G1a

        packssdw mm1, mm3      ; mm1 = preGb preBb preRb XXX
        pshufw mm1, mm1, 0x79  ; mm1 = preRb preGb preBb preRb

        pshufw mm3, mm0, 0xfe  ; mm3 = Y1b Y1b Y1b Y0b
        paddsw mm3, mm1        ; mm3 = R1b G1b B1b R0b

        // Could speed this up slightly by unrolling this whole loop,
        // so can guarantee alignment and can do a movq instead of two
        // of these movds.
        packuswb mm4, mm4      ; mm4 = X X X X B1a R0a G0a B0a
        packuswb mm5, mm5      ; mm5 = X X X X G0b B0b R1a G1a
        packuswb mm3, mm3      ; mm3 = X X X X R1b G1b B1b R0b

        movd [edi+0], mm4
        movd [edi+4], mm5
        movd [edi+8], mm3
        add edi, 12

        dec eax
        jnz next4

        mov dest, edi
    }

    return dest;
}

// The c-equivalent if P3-or-better is not present in cpu flags.
LPBYTE C_YUVtoRGB(LPBYTE dest, short *src, DWORD w)
{
    LPBYTE s = (LPBYTE)src;
    DWORD w2 = w >> 1;
    while (w2--)
    {
        // Y0 U Y1 V for YUY2...
        int Y0 = (int)DWORD(*s++);
        int U =  (int)DWORD(*s++) - 128;
        int Y1 = (int)DWORD(*s++);
        int V =  (int)DWORD(*s++) - 128;

        int preR = (U * -30 + V * 45931) >> 15;
        int preG = (U * -11258 + V * -23378) >> 15;
        int preB = (U * 58050 + V * -90) >> 15;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 : (x))

        *dest++ = clip(Y0 + preB);
        *dest++ = clip(Y0 + preG);
        *dest++ = clip(Y0 + preR);

        *dest++ = clip(Y1 + preB);
        *dest++ = clip(Y1 + preG);
        *dest++ = clip(Y1 + preR);
    }

    return dest;
}

// src will be calculated from the average of src1 and src2.
LPBYTE P3_AvgYUVtoRGB(LPBYTE dest, short *src1, short *src2, DWORD w)
{
    _asm
    {
        mov esi, src1
        mov esp, src2
        mov edi, dest
        mov eax, w
        shr eax, 2

    next4:
        // Process 4 pixels.  First YUYV is a, second is b.
        // Source must be 8-byte aligned.
        movq mm1, [esi]     ; mm1 = Vb+128 Y1b Ub+128 Y0b  Va+128 Y1a Ua+128 Y0a
        movq mm2, [esp]     ; mm2 = Vb+128 Y1b Ub+128 Y0b  Va+128 Y1a Ua+128 Y0a
        add esi, 8
        add esp, 8
        pavgb mm1, mm2         ; mm1 = byte-wise average of mm1 and mm2
        pxor mm1, const8000    ; mm1 = Vb Y1b Ub Y0b  Va Y1a Ua Y0a

        movq mm0, mm1          ; mm0 = XX Y1b XX Y0b  XX Y1a XX Y0a
        psraw mm1, 8           ; mm1 = Vb Ub Va Ua

        pshufw mm2, mm1, 0x44  ; mm2 = Va Ua Va Ua
        pmaddwd mm2, RvRuGvGu  ; mm2 = preRa preGa
        psrad mm2, 14          ; mm2 >>= 14

        pshufw mm3, mm1, 0xee  ; mm3 = Vb Ub Vb Ub
        pmaddwd mm3, GvGuBvBu  ; mm3 = preGb preBb
        psrad mm3, 14          ; mm3 >>= 14

        pmaddwd mm1, RvRuBvBu  ; mm1 = preRb preBa
        psrad mm1, 14          ; mm1 >>= 14

        movq mm6, mm2          ; mm6 = preRa preGa
        packssdw mm2, mm1      ; mm2 = XXX preBa preRa preGa
        pshufw mm2, mm2, 0x92  ; mm2 = preBa preRa preGa preBa

        pand mm0, const00ff    ; mm0 = Y1b Y0b Y1a Y0a

        pshufw mm4, mm0, 0x40  ; mm4 = Y1a Y0a Y0a Y0a
        paddsw mm4, mm2        ; mm4 = B1a R0a G0a B0a

        packssdw mm6, mm3      ; mm6 = preGb preBb preRa preGa
        pshufw mm5, mm0, 0xa5  ; mm5 = Y0b Y0b Y1a Y1a
        paddsw mm5, mm6        ; mm5 = G0b B0b R1a G1a

        packssdw mm1, mm3      ; mm1 = preGb preBb preRb XXX
        pshufw mm1, mm1, 0x79  ; mm1 = preRb preGb preBb preRb

        pshufw mm3, mm0, 0xfe  ; mm3 = Y1b Y1b Y1b Y0b
        paddsw mm3, mm1        ; mm3 = R1b G1b B1b R0b

        // Could speed this up slightly by unrolling this whole loop,
        // so can guarantee alignment and can do a movq instead of two
        // of these movds.
        packuswb mm4, mm4      ; mm4 = X X X X B1a R0a G0a B0a
        packuswb mm5, mm5      ; mm5 = X X X X G0b B0b R1a G1a
        packuswb mm3, mm3      ; mm3 = X X X X R1b G1b B1b R0b

        movd [edi+0], mm4
        movd [edi+4], mm5
        movd [edi+8], mm3
        add edi, 12

        dec eax
        jnz next4

        mov dest, edi
    }

    return dest;
}

// The c-equivalent if P3-or-better is not present in cpu flags.
LPBYTE C_AvgYUVtoRGB(LPBYTE dest, short *src1, short *src2, DWORD w)
{
    LPBYTE s1 = (LPBYTE)src1;
    LPBYTE s2 = (LPBYTE)src2;
    DWORD w2 = w >> 1;
    while (w2--)
    {
        // Y0 U Y1 V for YUY2...
        int Y0a = (int)DWORD(*s1++);
        int Ua =  (int)DWORD(*s1++) - 128;
        int Y1a = (int)DWORD(*s1++);
        int Va =  (int)DWORD(*s1++) - 128;

        int Y0b = (int)DWORD(*s2++);
        int Ub =  (int)DWORD(*s2++) - 128;
        int Y1b = (int)DWORD(*s2++);
        int Vb =  (int)DWORD(*s2++) - 128;

        int Y0 = (Y0a + Y0b) >> 1;
        int U =  (Ua + Ub) >> 1;
        int Y1 = (Y1a + Y1b) >> 1;
        int V =  (Va + Vb) >> 1;

        int preR = (U * -30 + V * 45931) >> 15;
        int preG = (U * -11258 + V * -23378) >> 15;
        int preB = (U * 58050 + V * -90) >> 15;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 : (x))

        *dest++ = clip(Y0 + preB);
        *dest++ = clip(Y0 + preG);
        *dest++ = clip(Y0 + preR);

        *dest++ = clip(Y1 + preB);
        *dest++ = clip(Y1 + preG);
        *dest++ = clip(Y1 + preR);
    }

    return dest;
}

/*

  Y = R *  .299 + G *  .587 + B *  .114;
  U = R * -.168 + G * -.332 + B *  .500 + 128.;
  V = R *  .500 + G * -.419 + B * -.081 + 128.;

  These are the ones used by an example on Intel's dev site...
  If we go with these, we should recalculate the inverse for the other mmx func.

  Y = 0.299 R + 0.587 G + 0.114 B
  U =-0.146 R - 0.288 G + 0.434 B			
  V = 0.617 R - 0.517 G - 0.100 G

  Y = [(9798 R + 19235G + 3736 B) / 32768]
  U = [(-4784 R - 9437 G + 4221 B) / 32768] + 128	
  V = [(20218R - 16941G - 3277 B) / 32768] + 128

*/

// Note names are most-signif first, but the contents are least-signif first.
static short const128_0_128_0[4] = { 0, 128, 0, 128 };
static short UgUbYgYb[4] = { 3736, 19235,  16384, -10879 };
static short Ur0Yr0[4] =   {    0,  9798,     0,   -5505 };
static short VgVbYgYb[4] = { 3736, 19235, -2654,  -13730 };
static short Vr0Yr0[4] =   {    0,  9798,     0,   16384 };

LPBYTE P3_RGBtoYUV(short *dest, LPBYTE src, DWORD w)
{
    _asm
    {
        mov esi, src
        mov edi, dest
        mov eax, w
        shr eax, 2

        pxor mm0, mm0              ; mm0 = 0, constant
        movq mm7, const128_0_128_0 ; constant

    next4:
        // Process 4 pixels.  First YUYV is a, second is b.
        // Source must be 12-byte aligned.
        movd mm1, [esi+0]      ; mm1 = X X X X B1a R0a G0a B0a
        movd mm2, [esi+4]      ; mm2 = X X X X G0b B0b R1a G1a
        movd mm3, [esi+8]      ; mm3 = X X X X R1b G1b B1b R0b
        add esi, 12

        punpcklbw mm1, mm0     ; mm1 = B1a R0a G0a B0a
        punpcklbw mm2, mm0     ; mm2 = G0b B0b R1a G1a
        punpcklbw mm3, mm0     ; mm3 = R1b G1b B1b R0b


        pshufw mm4, mm1, 0x44  ; mm4 = G0a B0a G0a B0a
        pmaddwd mm4, UgUbYgYb  ; mm4 =   Ugb     Ygb

        pshufw mm5, mm1, 0x88  ; mm5 = R0a XXX R0a XXX
        pmaddwd mm5, Ur0Yr0    ; mm5 =   Ur      Yr

        paddd mm4, mm5         ; mm4 =   Ua      Y0a
        psrad mm4, 15          ; mm4 >>= 15


        pshufw mm5, mm1, 0x0f  ; mm5 = XXX XXX B1a B1a
        pshufw mm6, mm2, 0x00  ; mm6 = XXX XXX G1a G1a

        punpcklwd mm5, mm6     ; mm5 = G1a B1a G1a B1a
        pmaddwd mm5, VgVbYgYb  ; mm5 =   Vgb     Ygb

        pshufw mm6, mm2, 0x44  ; mm6 = R1a XXX R1a XXX
        pmaddwd mm6, Vr0Yr0    ; mm6 =   Vr      Yr

        paddd mm5, mm6         ; mm5 =   Va      Y1a
        psrad mm5, 15          ; mm5 >>= 15


        pshufw mm2, mm2, 0xee  ; mm2 = G0b B0b G0b B0b
        pmaddwd mm2, UgUbYgYb  ; mm2 =   Ugb     Ygb

        pshufw mm1, mm3, 0x00  ; mm1 = R0b XXX R0b XXX
        pmaddwd mm1, Ur0Yr0    ; mm1 =   Ur      Yr

        paddd mm2, mm1         ; mm2 =   Ub      Y0b
        psrad mm2, 15          ; mm2 >>= 15


        pshufw mm6, mm3, 0x99  ; mm6 = G1b B1b G1b B1b
        pmaddwd mm6, VgVbYgYb  ; mm6 =   Vgb     Ygb

        pshufw mm3, mm3, 0xcc  ; mm3 = R1b XXX R1b XXX
        pmaddwd mm3, Vr0Yr0    ; mm3 =   Vr      Yr

        paddd mm6, mm3         ; mm6 =   Vb      Y1b
        psrad mm6, 15          ; mm6 >>= 15


        packssdw mm4, mm5      ; mm4 = Va Y1a Ua Y0a
        packssdw mm2, mm6      ; mm2 = Vb Y1b Ub Y0b

        paddsw mm4, mm7        ; mm4 = Va+128 Y1a Ua+128 Y0a
        paddsw mm2, mm7        ; mm2 = Vb+128 Y1b Ub+128 Y0b

        packuswb mm4, mm2   ; mm4 = Vb+128 Y1b Ub+128 Y0b Va+128 Y1a Ua+128 Y0a


        // Destination must be 8-byte aligned.
        movq [edi], mm4
        add edi, 8


        dec eax
        jnz next4

        mov src, esi
    }

    return src;
}

// The c-equivalent if P3-or-better is not present in cpu flags.
LPBYTE C_RGBtoYUV(short *dest, LPBYTE src, DWORD w)
{
    LPBYTE dst = (LPBYTE)dest;
    DWORD w2 = w >> 1;
    while (w2--)
    {
        int B = *src++;
        int G = *src++;
        int R = *src++;

        int Y = (R * 9798 + G * 19235 + B * 3736) >> 15;
        int U = ((R * -5505 + G * -10879 + B * 16384) >> 15) + 128;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 : (x))

        *dst++ = clip(Y);
        *dst++ = clip(U);

        B = *src++;
        G = *src++;
        R = *src++;

        Y = (R * 9798 + G * 19235 + B * 3736) >> 15;
        int V = ((R * 16384 + G * -13730 + B * -2654) >> 15) + 128;

        *dst++ = clip(Y);
        *dst++ = clip(V);
    }

    return src;
}

TimeShift::TimeShift()
    :
    m_mode(MODE_STOPPED),
    m_fps(30), // Our AVIs will always be 30 fps.
    m_curFile(0),
    m_lpbi(NULL),
    m_recordBits(NULL),
    m_playBits(NULL),
    m_gotPauseBits(FALSE),
	m_pfile(NULL),
    m_psVideo(NULL),
    m_psAudio(NULL),
    m_psCompressedVideo(NULL),
    m_psCompressedAudio(NULL),
    m_psCompressedVideoP(NULL),
    m_psCompressedAudioP(NULL),
    m_pGetFrame(NULL),
    m_setOpts(false),
    m_startTimeRecord(0),
    m_startTimePlay(0),
    m_thisTimeRecord(0),
    m_thisTimePlay(0),
    m_nextSampleRecord(0),
    m_nextSamplePlay(0),
    m_hWaveIn(NULL),
    m_nextWaveInHdr(0),
    m_hWaveOut(NULL),
    m_nextWaveOutHdr(0),
    m_recHeight(TS_HALFHEIGHTEVEN),
    m_origPixelWidth(CurrentX),
    m_origUseMixer(-1)
{
    m_waveInDevice[0] = 0;
    m_waveOutDevice[0] = 0;

    InitializeCriticalSection(&m_lock);

    if (CpuFeatureFlags & P3_OR_BETTER)
    {
        m_YUVtoRGB = P3_YUVtoRGB;
        m_AvgYUVtoRGB = P3_AvgYUVtoRGB;
        m_RGBtoYUV = P3_RGBtoYUV;
    }
    else
    {
        // NOTE: If you're here, you'll need to set your Pixel Width to 320 in
        // order to get a decent frame rate.
        m_YUVtoRGB = C_YUVtoRGB;
        m_AvgYUVtoRGB = C_AvgYUVtoRGB;
        m_RGBtoYUV = C_RGBtoYUV;
    }

    memset(&m_bih, 0, sizeof(m_bih));

    AVIFileInit();

    // Default to full frames video.
	memset(&m_infoVideo, 0, sizeof(m_infoVideo));
	m_infoVideo.fccType = streamtypeVIDEO;
	m_infoVideo.fccHandler = 0;//mmioFOURCC('D', 'I', 'B', ' ');
    m_infoVideo.dwFlags = 0;
    m_infoVideo.dwCaps = 0;
    m_infoVideo.wPriority = 0;
    m_infoVideo.wLanguage = 0;
	m_infoVideo.dwScale = 1;
	m_infoVideo.dwRate = m_fps;
    m_infoVideo.dwStart = 0;
    m_infoVideo.dwLength = 0;
    m_infoVideo.dwInitialFrames = 0;
	m_infoVideo.dwSuggestedBufferSize = m_bih.biSizeImage;
    m_infoVideo.dwQuality = (DWORD)-1;
    m_infoVideo.dwSampleSize = 0;
	SetRect(&m_infoVideo.rcFrame, 0, 0, m_bih.biWidth, m_bih.biHeight);
    m_infoVideo.dwEditCount = 0;
    m_infoVideo.dwFormatChangeCount = 0;
    strcpy(m_infoVideo.szName, "Video");

    // Default to CD Quality audio.
    m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    m_waveFormat.nChannels = 2;
    m_waveFormat.nSamplesPerSec = 44100;
    m_waveFormat.nAvgBytesPerSec = 176400; // nSamplesPer * nBlockAlign ?
    m_waveFormat.nBlockAlign = 4;
    m_waveFormat.wBitsPerSample = 16;
    m_waveFormat.cbSize = 0;

	memset(&m_infoAudio, 0, sizeof(m_infoAudio));
	m_infoAudio.fccType = streamtypeAUDIO;
	m_infoAudio.fccHandler = 0; // Zero for PCM audio.
    m_infoAudio.dwFlags = 0;
    m_infoAudio.dwCaps = 0;
    m_infoAudio.wPriority = 0;
    m_infoAudio.wLanguage = 0;
	m_infoAudio.dwScale = m_waveFormat.nBlockAlign;
	m_infoAudio.dwRate = m_waveFormat.nAvgBytesPerSec;
    m_infoAudio.dwStart = 0;
    m_infoAudio.dwLength = 0;
    m_infoAudio.dwInitialFrames = 0;
	m_infoAudio.dwSuggestedBufferSize = m_infoAudio.dwRate / m_fps;
    m_infoAudio.dwQuality = 0;
    m_infoAudio.dwSampleSize = m_waveFormat.nBlockAlign;
	SetRect(&m_infoAudio.rcFrame, 0, 0, 0, 0);
    m_infoAudio.dwEditCount = 0;
    m_infoAudio.dwFormatChangeCount = 0;
    strcpy(m_infoAudio.szName, "Audio");

	memset(&m_optsVideo, 0, sizeof(m_optsVideo));

    // Overwrite any of the above defaults with whatever's in the ini file.
    ReadFromIni();

    m_pTimeShift = NULL;
}

TimeShift::~TimeShift()
{
    // NOTE: This will deadlock if we crash in OnNewFrame.  So, the solution
    // would be to fix any crash bugs and we can leave this lock as is.
    EnterCriticalSection(&m_lock);

    Stop();

    if (m_setOpts && m_optsVideo.lpParms && m_optsVideo.cbParms)
        delete m_optsVideo.lpParms;

    AVIFileExit();

    LeaveCriticalSection(&m_lock);

    DeleteCriticalSection(&m_lock);
}

bool TimeShift::AssureCreated(void)
{
    if (!m_pTimeShift)
        m_pTimeShift = new TimeShift;

    return m_pTimeShift != NULL;
}

bool TimeShift::OnSetDimensions(void)
{
    return m_pTimeShift ? m_pTimeShift->SetDimensions() : false;
}

bool TimeShift::OnGetDimenstions(int *w, int *h)
{
    if (m_pTimeShift && w && h)
    {
        *w = m_pTimeShift->m_bih.biWidth;
        *h = m_pTimeShift->m_bih.biHeight;
        return true;
    }

    return false;
}

bool TimeShift::OnSetWaveInDevice(char *pszDevice)
{
    return m_pTimeShift ? m_pTimeShift->SetWaveInDevice(pszDevice) : false;
}

bool TimeShift::OnGetWaveInDevice(char **ppszDevice)
{
    if (m_pTimeShift && ppszDevice)
    {
        *ppszDevice = (char*)&m_pTimeShift->m_waveInDevice;
        return true;
    }

    return false;
}

bool TimeShift::OnSetWaveOutDevice(char *pszDevice)
{
    return m_pTimeShift ? m_pTimeShift->SetWaveOutDevice(pszDevice) : false;
}

bool TimeShift::OnGetWaveOutDevice(char **ppszDevice)
{
    if (m_pTimeShift && ppszDevice)
    {
        *ppszDevice = (char*)&m_pTimeShift->m_waveOutDevice;
        return true;
    }

    return false;
}

bool TimeShift::OnSetRecHeight(int index)
{
    return m_pTimeShift ? m_pTimeShift->SetRecHeight(index) : false;
}

bool TimeShift::OnGetRecHeight(int *index)
{
    if (m_pTimeShift && index)
    {
        *index = m_pTimeShift->m_recHeight;
        return true;
    }

    return false;
}

bool TimeShift::OnCompressionOptions(void)
{
    return m_pTimeShift ? m_pTimeShift->CompressionOptions() : false;
}

bool TimeShift::SetBT848PixelWidth(int pixelWidth)
{
    bool result = false;

    if (m_pTimeShift)
    {
        // Must be called from within one of the critical sections.
        // We have to let the capture thread do its thing to change the pixel
        // width.  Leave the critical section for this call.
        LeaveCriticalSection(&m_pTimeShift->m_lock);
        //TODO: Reinstate this line
        //result = Setting_SetValue(BT848_GetSetting(CURRENTX), pixelWidth) == 0;
        EnterCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShift::DoMute(bool mute)
{
    if (m_pTimeShift)
    {
        if (mute)
        {
            m_pTimeShift->m_origUseMixer = bUseMixer;
            bUseMixer = TRUE;

            // This is kind of ugly since there's no real interface to the mixer
            // module for what needs to be done here, but this does the job.
            // Adapted from MixerDev.cpp.
            extern CSoundSystem* pSoundSystem;
            extern long MixerIndex;
            pSoundSystem->SetMixer(MixerIndex);
            if (pSoundSystem->GetMixer())
            {
                // TODO: ???
                //Mixer_OnInputChange((eVideoSourceType)Setting_GetValue(
                //   BT848_GetSetting(VIDEOSOURCE)));

                Mixer_Mute();
            }
            else
            {
                // Mixer device not present, revert to original settings.
                bUseMixer = m_pTimeShift->m_origUseMixer;
                m_pTimeShift->m_origUseMixer = -1;
            }
        }
        else if (m_pTimeShift->m_origUseMixer != -1)
        {
            Mixer_UnMute();

            // Revert to original settings.
            bUseMixer = m_pTimeShift->m_origUseMixer;
            m_pTimeShift->m_origUseMixer = -1;
        }

        return true;
    }

    return false;
}

static void CALLBACK WaveInProc(
    HWAVEIN hwi,      
    UINT uMsg,         
    DWORD dwInstance,  
    DWORD dwParam1,    
    DWORD dwParam2)
{
    if (uMsg == WIM_DATA)
        TimeShift::OnWaveInData();
}

static void CALLBACK WaveOutProc(
    HWAVEOUT hwo,      
    UINT uMsg,         
    DWORD dwInstance,  
    DWORD dwParam1,    
    DWORD dwParam2)
{
    if (uMsg == WOM_DONE)
		TimeShift::OnWaveOutDone();
}

bool TimeShift::Record(bool pause)
{
    // Clear all variables.
    Stop();

    // Update the size in case pixel width has changed.
    SetDimensions();

    m_recordBits = new BYTE[m_bih.biSizeImage];
    if (pause && !m_playBits)
        m_playBits = new BYTE[m_bih.biSizeImage];

    // Find the next available file name.
    char fname[MAX_PATH];
    int curFile = 0;
    do {
        if (curFile == 1000)
        {
            ErrorBox("Could not create a file.  "
                     "Delete some of your video files and try again.");
            return false;
        }
        sprintf(fname, "ds%.3u.avi", curFile++);
    } while (GetFileAttributes(fname) != 0xffffffff);
    m_curFile = curFile - 1;

    // Open a new movie file for reading and writing.
    if (AVIFileOpen(&m_pfile, fname,
                    OF_READWRITE | OF_CREATE | OF_SHARE_DENY_NONE,
                    NULL) != 0)
    {
        Stop();
        return false;
    }

    // Create the video stream.
    AVIFileCreateStream(m_pfile, &m_psVideo, &m_infoVideo);
    if (m_setOpts)
    {
        if (AVIMakeCompressedStream(&m_psCompressedVideo,
                                    m_psVideo,
                                    &m_optsVideo,
                                    NULL) != AVIERR_OK)
        {
            Stop();
            return false;
        }
    }
    else
    {
        m_psCompressedVideo = m_psVideo;
        m_psVideo = NULL;
    }
    // NOTE: Add space for the color table if we want to save 8-bit AVIs.
    if (AVIStreamSetFormat(m_psCompressedVideo, 0, &m_bih, sizeof(m_bih)) != 0)
    {
        Stop();
        return false;
    }

    // Create the audio stream.
    if (AVIFileCreateStream(m_pfile, &m_psCompressedAudio, &m_infoAudio) != 0 ||
        AVIStreamSetFormat(m_psCompressedAudio, 0, &m_waveFormat,
                           sizeof(WAVEFORMATEX)) != 0)
    {
        Stop();
        return false;
    }

    // Mute the current live feed if we're pausing, waveIn can still hear it.
    if (pause)
        DoMute(true);
    

    int DeviceId;
    if(!GetWaveInDeviceIndex(&DeviceId))
    {
        ;
    }
    else
    {
        MMRESULT rslt;

        // If this fails, we just won't have any audio recorded, still continue.
        rslt = waveInOpen(&m_hWaveIn,
                        DeviceId,
                        &m_waveFormat,
                        (DWORD)WaveInProc,
                        0,
                        CALLBACK_FUNCTION);

        if(rslt==MMSYSERR_NOERROR)
        {
            for (int i = 0; i < sizeof(m_waveInHdrs)/sizeof(*m_waveInHdrs); ++i)
            {
                memset(m_waveInHdrs + i, 0, sizeof(WAVEHDR));
                m_waveInHdrs[i].lpData = (char *)m_waveInBufs[i];
                m_waveInHdrs[i].dwBufferLength = sizeof(m_waveInBufs[i]);
                waveInPrepareHeader(m_hWaveIn, m_waveInHdrs + i, sizeof(WAVEHDR));
                waveInAddBuffer(m_hWaveIn, m_waveInHdrs + i, sizeof(WAVEHDR));
            }
        }
        else
        {
            char szErrorMsg[200];
            sprintf(szErrorMsg, "Error %x in waveInOpen()", rslt);
            //TODO: tell the user, that something wrong
        }
    }


    m_mode = pause ? MODE_PAUSED : MODE_RECORDING;

    if (m_hWaveIn)
        waveInStart(m_hWaveIn);

    return true;
}

bool TimeShift::Play(void)
{
    if (m_mode != MODE_PAUSED)
        // Clear all variables.
        Stop();

    // Find the first AVI available.
    char fname[MAX_PATH];
    int curFile = m_curFile;
    do {
        sprintf(fname, "ds%.3u.avi", curFile);
        if (GetFileAttributes(fname) != 0xffffffff)
            break;
        if (++curFile == 1000)
            curFile = 0;
    } while (curFile != m_curFile); // Bail if looped all the way around.
    m_curFile = curFile;

    // Open the movie file for writing.
    if (!m_pfile && AVIFileOpen(&m_pfile, fname,
                                OF_READWRITE | OF_SHARE_DENY_NONE,
                                NULL) != 0)
    {
        if (m_mode != MODE_PAUSED)
            Stop();
        return false;
    }

    if (AVIFileGetStream(m_pfile, &m_psCompressedVideoP, 0, 0) != 0 ||
        AVIFileGetStream(m_pfile, &m_psCompressedAudioP, 0, 1) != 0)
    {
        if (m_mode != MODE_PAUSED)
            Stop();
        return false;
    }

    if (m_mode != MODE_PAUSED)
    {
        if (AVIStreamInfo(m_psCompressedVideoP,
                          &m_infoVideo,
                          sizeof(m_infoVideo)) != 0 ||
            AVIStreamInfo(m_psCompressedAudioP,
                          &m_infoAudio,
                          sizeof(m_infoAudio)) != 0)
        {
            if (m_mode != MODE_PAUSED)
                Stop();
            return false;
        }

        // Make sure the pixel width is ready for the incoming AVI's width.
        if (CurrentX != m_infoVideo.rcFrame.right)
            SetBT848PixelWidth(m_infoVideo.rcFrame.right);

        // Update the size for current pixel width.
        SetDimensions();

        // Mute the current live feed, we're going to playback our own audio.
        DoMute(true);
    }

    if ((m_pGetFrame = AVIStreamGetFrameOpen(m_psCompressedVideoP, NULL))
        == NULL)
    {
        if (m_mode != MODE_PAUSED)
            Stop();
        return false;
    }

    m_gotPauseBits = false;

    int DeviceId;
    if(!GetWaveOutDeviceIndex(&DeviceId))
    {
        ;
    }
    else
    {
        MMRESULT rslt;
  
        // If this fails, we just won't have audio playback.
        // FIXME: Need to make sure our m_waveFormat jives with the new m_infoAudio.
        rslt = waveOutOpen(&m_hWaveOut,
                           DeviceId,
                           &m_waveFormat,
                           (DWORD)WaveOutProc,
                           0,
                           CALLBACK_FUNCTION | WAVE_ALLOWSYNC);
    
        if(rslt==MMSYSERR_NOERROR)
        {
            // Start sending audio data to the waveOut device.
            // Call twice to keep two buffers going at all times.
            ReadAudio();
            ReadAudio();
        }
        else
        {
            char szErrorMsg[200];
            sprintf(szErrorMsg, "Error %x in waveOutOpen()", rslt);
            //TODO: tell the user, that something wrong
        }
    }

    m_mode = m_mode == MODE_PAUSED ? MODE_SHIFTING : MODE_PLAYING;

    return true;
}

bool TimeShift::Stop(void)
{
    // Set mode to stopped first in case we continue to receive callbacks.
    m_mode = MODE_STOPPED;

    // The critical section setup is to prevent us from paying attention
    // to any forthcoming audio device notifications.
    // We're in a critical section right now, but we have to free that up
    // for the audio callback mechanism that will be triggered by the reset,
    // then restore it's original lock.

    if (m_hWaveIn)
    {
        LeaveCriticalSection(&m_lock);
        waveInReset(m_hWaveIn);
        EnterCriticalSection(&m_lock);

        waveInClose(m_hWaveIn);

        // The audio device is finished with the header, unprepare it.
        for (int i = 0; i < sizeof(m_waveInHdrs)/sizeof(*m_waveInHdrs); ++i)
            waveInUnprepareHeader(m_hWaveIn,
                                  m_waveInHdrs + i,
                                  sizeof(WAVEHDR));
    }

    if (m_hWaveOut)
    {
        LeaveCriticalSection(&m_lock);
        waveOutReset(m_hWaveOut);
        EnterCriticalSection(&m_lock);

        waveOutClose(m_hWaveOut);

        // The audio device is finished with the header, unprepare it.
        for (int i = 0; i < sizeof(m_waveOutHdrs)/sizeof(*m_waveOutHdrs); ++i)
            // Unpreparing a header that wasn't prepared does nothing.
            waveOutUnprepareHeader(m_hWaveOut,
                                   m_waveOutHdrs + i,
                                   sizeof(WAVEHDR));
    }

    if (m_psCompressedVideo)
        AVIStreamRelease(m_psCompressedVideo);

    if (m_psCompressedVideoP)
        AVIStreamRelease(m_psCompressedVideoP);

    if (m_psVideo)
        AVIStreamRelease(m_psVideo);

    if (m_psCompressedAudio)
        AVIStreamRelease(m_psCompressedAudio);

    if (m_psCompressedAudioP)
        AVIStreamRelease(m_psCompressedAudioP);

    if (m_psAudio)
        AVIStreamRelease(m_psAudio);

    if (m_pfile)
        AVIFileRelease(m_pfile);

    if (m_pGetFrame)
        AVIStreamGetFrameClose(m_pGetFrame);

    if (m_recordBits)
        delete m_recordBits;

    if (m_playBits)
        delete m_playBits;

    m_lpbi = NULL;
    m_recordBits = NULL;
    m_playBits = NULL;
    m_gotPauseBits = FALSE;

    m_pfile = NULL;
    m_psVideo = NULL;
    m_psCompressedVideo = NULL;
    m_psCompressedVideoP = NULL;
    m_psAudio = NULL;
    m_psCompressedAudio = NULL;
    m_psCompressedAudioP = NULL;
    m_pGetFrame = NULL;

    m_startTimeRecord = 0;
    m_startTimePlay = 0;
    m_thisTimeRecord = 0;
    m_thisTimePlay = 0;
    m_nextSampleRecord = 0;
    m_nextSamplePlay = 0;

    m_hWaveIn = 0;
    m_nextWaveInHdr = 0;
    m_hWaveOut = 0;
    m_nextWaveOutHdr = 0;

    // Stop mode, allow live feed in again.
    DoMute(false);

    return true;
}

bool TimeShift::GoNext(void)
{
    int curFile = m_curFile;

    do {
        if (++curFile == 1000)
            curFile = 0;
        char fname[MAX_PATH];
        sprintf(fname, "ds%.3u.avi", curFile);
        if (GetFileAttributes(fname) != 0xffffffff)
            break;
    } while (curFile != m_curFile); // Bail if we've looped all the way around.

    m_curFile = curFile;

    return Play();
}

bool TimeShift::GoPrev(void)
{
    int curFile = m_curFile;

    do {
        if (--curFile < 0)
            curFile = 999;
        char fname[MAX_PATH];
        sprintf(fname, "ds%.3u.avi", curFile);
        if (GetFileAttributes(fname) != 0xffffffff)
            break;
    } while (curFile != m_curFile); // Bail if we've looped all the way around.

    m_curFile = curFile;

    return Play();
}

bool TimeShift::WriteVideo(DEINTERLACE_INFO *info)
{
    if (!m_psCompressedVideo)
        return false;

//      if ((!info->IsOdd && info->EvenLines[0] == NULL) ||
//          (info->IsOdd && info->OddLines[0] == NULL))
//          return false;
    // Use this more restrictive check.
    if (info->EvenLines[0] == NULL || info->OddLines[0] == NULL)
        return false;

    m_thisTimeRecord = GetTickCount();

    // Make sure everything starts out at zero.
    if (!m_startTimeRecord)
        m_startTimeRecord = m_thisTimeRecord;

    DWORD thisFrame =
        DWORD((double)m_fps * double(m_thisTimeRecord - m_startTimeRecord)
              / 1000.0 + 0.5);

    LPBYTE dest = m_recordBits;
    DWORD frameWidth = (info->FrameWidth >> 2) << 2;
    DWORD w = min(m_bih.biWidth, frameWidth);
    DWORD h = min(m_bih.biHeight, info->FieldHeight);
    DWORD more = (m_bih.biBitCount >> 3) * (m_bih.biWidth - w);

    // Our AVI is 30fps.  Write after even field is received.
    if (!info->IsOdd)
    {
        int y = h - 1;

        // IMPORTANT: In some of the following cases, we're using the previous
        // even field data, and presumably, since we only handle these cases
        // when we receive odd field data, then the even data must've been
        // received 1/60 of a second ago, and therefore is valid for us to use.
        // Otherwise, crash!

        switch (m_mode == MODE_PAUSED || m_mode == MODE_SHIFTING ?
                TS_HALFHEIGHTEVEN : m_recHeight)
        {
        case TS_FULLHEIGHT:
            // Not yet implemented.
            // I think you'd have to have a monster machine to get a good frame
            // rate out of this, so it's a low priority to implement.  I've got
            // a Pentium-III 733MHz and even with 1/2-height, I'm teetering on
            // 0 DF/S with pixel width 640.
            // No break here for now, just let if fall into the default case.

        default:
        case TS_HALFHEIGHTEVEN:
            for (; y >= 0; --y)
                dest = m_YUVtoRGB(dest,
                                  info->EvenLines[0][y],
                                  w) + more;
            break;
#if 0 // EAS20010805: There is a crash bug here somewhere...
        case TS_HALFHEIGHTODD:
            for (; y >= 0; --y)
                dest = m_YUVtoRGB(dest,
                                  info->OddLines[0][y],
                                  w) + more;
            break;

        case TS_HALFHEIGHTAVG:
            for (; y >= 0; --y)
                dest = m_AvgYUVtoRGB(dest,
                                     info->EvenLines[0][y],
                                     info->OddLines[0][y],
                                     w) + more;
            break;
#endif // 0
        }

        long bytesWritten = 0;
        long samplesWritten = 0;
        AVIStreamWrite(m_psCompressedVideo,
                       thisFrame,
                       1,
                       m_recordBits,
                       m_bih.biSizeImage,
                       AVIIF_KEYFRAME,
                       &samplesWritten,
                       &bytesWritten);

        // Even if we failed to write to the stream, we'll leave space.
        m_infoVideo.dwLength = thisFrame + 1;
    }

    switch (m_mode)
    {
    case MODE_PAUSED:
        // FIXME: Make pause bits YUY2 so we don't have to convert every time.
        // FIXME: Use info->pMemcpy() here, but it crashes. ???
        if (!m_gotPauseBits && !info->IsOdd)
        {
            memcpy(m_playBits, m_recordBits, m_bih.biSizeImage);
            m_gotPauseBits = true;
        }

        if (m_gotPauseBits)
        {
            LPBYTE src = m_playBits;
            DWORD frameWidth = (info->FrameWidth >> 2) << 2;
            DWORD w = min(m_bih.biWidth, frameWidth);
            DWORD h = min(m_bih.biHeight, info->FieldHeight);
            DWORD more = (m_bih.biBitCount >> 3) * (m_bih.biWidth - w);

            if (info->IsOdd)
                for (int y = h - 1; y >= 0; --y)
                    src = m_RGBtoYUV(info->OddLines[0][y], src, w) + more;
            else
                for (int y = h - 1; y >= 0; --y)
                    src = m_RGBtoYUV(info->EvenLines[0][y], src, w) + more;
        }
        break;

    case MODE_SHIFTING:
        ReadVideo(info);
        break;
    }

    return true;
}

bool TimeShift::ReadVideo(DEINTERLACE_INFO *info)
{
    if (!m_psCompressedVideoP)
        return false;

    if ((!info->IsOdd && info->EvenLines[0] == NULL) ||
        (info->IsOdd && info->OddLines[0] == NULL))
        return false;

    m_thisTimePlay = GetTickCount();

    // Make sure everything starts out at zero.
    if (!m_startTimePlay)
        m_startTimePlay = m_thisTimePlay;

    DWORD thisFrame =
        DWORD((double)m_fps * double(m_thisTimePlay - m_startTimePlay)
              / 1000.0 + 0.5);

    // If we're at the end of this clip, start playing the next one.
    if (m_mode == MODE_PLAYING && thisFrame >= m_infoVideo.dwLength)
    {
        // Just change the mode so no further frames will be read.
        // Can't call Stop from this thread.  Too dangerous.
        m_mode = MODE_STOPPED;

        // We basically want to trigger an OnGoNext() but I don't want to call
        // that from within this thread.
        PostMessage(hWnd, WM_COMMAND, IDM_TSNEXT, 0);

        return false;
    }

    // Our AVI is 30fps.  Read after odd field is received.
    if (info->IsOdd)
        m_lpbi = m_pGetFrame ?
            (LPBITMAPINFOHEADER)AVIStreamGetFrame(m_pGetFrame, thisFrame) :NULL;

    if (m_lpbi)
    {
        LPBYTE src = LPBYTE(m_lpbi) + m_lpbi->biSize;
        DWORD frameWidth = (info->FrameWidth >> 2) << 2;
        DWORD w = min(m_lpbi->biWidth, frameWidth);
        DWORD h = min(m_lpbi->biHeight, info->FieldHeight);
        DWORD more = (m_lpbi->biBitCount >> 3) * (m_lpbi->biWidth - w);

        if (info->IsOdd)
            for (int y = h - 1; y >= 0; --y)
                src = m_RGBtoYUV(info->OddLines[0][y], src, w) + more;
        else
            // FIXME? Maybe keep the previous field around and memcpy here
            // instead of reprocessing the pixels we already did last time?
            // Probably not, the mmx code is almost as fast and we'd have to
            // do two memcpys.  One to save it, one to copy to even lines.
            for (int y = h - 1; y >= 0; --y)
                src = m_RGBtoYUV(info->EvenLines[0][y], src, w) + more;
    }

    if (!info->IsOdd)
        m_lpbi = NULL;

    return true;
}

bool TimeShift::WriteAudio(void)
{
    if (!m_psCompressedAudio)
        return false;

    DWORD count = m_waveInHdrs[m_nextWaveInHdr].dwBytesRecorded;

    DWORD numSamples = count / m_infoAudio.dwSampleSize;
    long bytesWritten = 0;
    long samplesWritten = 0;

    // If this fails, there's nothing we can do but continue, no problem.
    AVIStreamWrite(m_psCompressedAudio,
                   m_nextSampleRecord,
                   numSamples,
                   m_waveInBufs[m_nextWaveInHdr],
                   count,
                   AVIIF_KEYFRAME,
                   &samplesWritten,
                   &bytesWritten);

    // Re-add this buffer to the waveIn queue.
    waveInAddBuffer(m_hWaveIn, m_waveInHdrs + m_nextWaveInHdr, sizeof(WAVEHDR));

    ++m_nextWaveInHdr %= sizeof(m_waveInHdrs)/sizeof(*m_waveInHdrs);
    m_nextSampleRecord += numSamples;

    return true;
}

bool TimeShift::ReadAudio(void)
{
    if (!m_psCompressedAudioP)
        return false;

    DWORD count = sizeof(*m_waveOutBufs);

    DWORD numSamples = count / m_infoAudio.dwSampleSize;
    long bytesRead = 0;
    long samplesRead = 0;

    // Even if this fails, we still need to send _something_ to waveOut device.
    // Otherwise, we'd never get notification of its completion and the the
    // audio rendering cycle would stop.
    AVIStreamRead(m_psCompressedAudioP,
                  m_nextSamplePlay,
                  numSamples,
                  m_waveOutBufs[m_nextWaveOutHdr],
                  count,
                  &bytesRead,
                  &samplesRead);

    memset(m_waveOutHdrs + m_nextWaveOutHdr, 0, sizeof(*m_waveOutHdrs));
    m_waveOutHdrs[m_nextWaveOutHdr].lpData =
        (LPSTR)m_waveOutBufs[m_nextWaveOutHdr];
    m_waveOutHdrs[m_nextWaveOutHdr].dwBufferLength = bytesRead;

    // Preparing a previously prepared header does nothing.
    waveOutPrepareHeader(m_hWaveOut,
                         m_waveOutHdrs + m_nextWaveOutHdr,
                         sizeof(*m_waveOutHdrs));

    waveOutWrite(m_hWaveOut,
                 m_waveOutHdrs + m_nextWaveOutHdr,
                 sizeof(*m_waveOutHdrs));

    ++m_nextWaveOutHdr %= sizeof(m_waveOutHdrs)/sizeof(*m_waveOutHdrs);
    m_nextSamplePlay += numSamples;

    return true;
}

bool TimeShift::SetDimensions(void)
{
    if (m_mode == MODE_STOPPED)
    {
        // Use current pixel width and field heights to determine our AVI size.
        int w = CurrentX;
        int h = CurrentY;

        // Reset all the bitmapinfo stuff.
        memset(&m_bih, 0, sizeof(m_bih));
        m_bih.biSize = sizeof(m_bih);
        m_bih.biWidth = (w >> 2) << 2; // 4-pixel (12-byte) align.
        m_bih.biHeight = h >> 1; // For speed (for now), we use 1/2 height AVIs.
        m_bih.biPlanes = 1;
        m_bih.biBitCount = 24; // Always convert to 24bits for compression.
        m_bih.biCompression = BI_RGB; // The codec will compress our RGB data.
        m_bih.biSizeImage =
            (m_bih.biBitCount >> 3) * m_bih.biWidth * m_bih.biHeight;

        m_infoVideo.dwSuggestedBufferSize = m_bih.biSizeImage;
        SetRect(&m_infoVideo.rcFrame, 0, 0, m_bih.biWidth, m_bih.biHeight);

        return true;
    }

    return false;
}

bool TimeShift::SetWaveInDevice(char* pszDevice)
{
    if (m_mode == MODE_STOPPED && pszDevice)
    {
        lstrcpy((char*) m_waveInDevice, pszDevice);

        return true;
    }

    return false;
}

bool TimeShift::SetWaveOutDevice(char* pszDevice)
{
    if (m_mode == MODE_STOPPED && pszDevice)
    {
        lstrcpy((char*) m_waveOutDevice, pszDevice);

        return true;
    }

    return false;
}

bool TimeShift::GetWaveInDeviceIndex(int *index)
{
    int count = waveInGetNumDevs();

    if(!m_waveInDevice[0] && count)
    {
        *index = 0;
        return true;
    }

    for(int i=0; i < count; ++i)
    {
        WAVEINCAPS wic;
        if(waveInGetDevCaps(i, &wic, sizeof(wic))==MMSYSERR_NOERROR)
        {
            if(!lstrcmp(m_waveInDevice, wic.szPname))
            {
                *index = i;

                return true;
            }
        }
    }

    return false;
}

bool TimeShift::GetWaveOutDeviceIndex(int *index)
{
    int count = waveOutGetNumDevs();

    if(!m_waveOutDevice[0] && count)
    {
        *index = 0;
        return true;
    }

    for(int i=0; i < count; ++i)
    {
        WAVEOUTCAPS wic;
        if(waveOutGetDevCaps(i, &wic, sizeof(wic))==MMSYSERR_NOERROR)
        {
            if(!lstrcmp(m_waveOutDevice, wic.szPname))
            {
                *index = i;

                return true;
            }
        }
    }

    return false;
}

bool TimeShift::SetRecHeight(int index)
{
    if (m_mode == MODE_STOPPED)
    {
        m_recHeight = index;

        return true;
    }

    return false;
}

bool TimeShift::CompressionOptions(void)
{
    // Must be stopped before attempting to bring up compression options dialog.
    if (m_mode != MODE_STOPPED)
        return false;

    bool result = false;

    // Update these settings for the options dialog to display.
    SetDimensions();

    char fname[20] = "dstemp.avi";
    DeleteFile(fname);

	PAVIFILE pfile;
    AVIFileOpen(&pfile, fname, OF_WRITE | OF_CREATE, NULL);

	// Create the video stream and set its format.
	PAVISTREAM psVideo = NULL;
    AVIFileCreateStream(pfile, &psVideo, &m_infoVideo);

    // NOTE: Add space for the color table if we want to save 8-bit AVIs.
    // Also, for the options dialog, set the info header's compression so it'll
    // show up under "Current Format:", then set it back to BI_RGB for the
    // actual recording process.
    BITMAPINFOHEADER bih = m_bih;
    bih.biCompression = m_infoVideo.fccHandler ? m_infoVideo.fccHandler :BI_RGB;
    AVIStreamSetFormat(psVideo, 0, &bih, sizeof(bih));

	// Create the audio stream and set its format.
	PAVISTREAM psAudio = NULL;
    AVIFileCreateStream(pfile, &psAudio, &m_infoAudio);
    AVIStreamSetFormat(psAudio, 0, &m_waveFormat, sizeof(WAVEFORMATEX));

    // Prompt for compression options.
	AVICOMPRESSOPTIONS optsVideo = m_optsVideo;
  	AVICOMPRESSOPTIONS optsAudio;
    memset(&optsAudio, 0, sizeof(optsAudio));
    const int numStreams = 2;
    PAVISTREAM streams[numStreams] = {psVideo, psAudio};
    LPAVICOMPRESSOPTIONS opts[numStreams] = {&optsVideo, &optsAudio};

    // hWnd is the main global hwnd.
  	if (AVISaveOptions(hWnd, 0, numStreams, streams, opts))
    {
        // For audio, we need to reset the wave format.
        if (optsAudio.lpFormat && optsAudio.cbFormat)
        {
            // If the format given isn't even as big as a WAVEFORMATEX, we'll
            // take what we can get and leave any old parameters at the end of
            // our structure untouched.  Otherwise, we only care about the first
            // sizeof(WAVEFORMATEX) bytes.
            memcpy(&m_waveFormat,
                   optsAudio.lpFormat,
                   min(optsAudio.cbFormat, sizeof(m_waveFormat)));

            UpdateAudioInfo();
        }

        SetVideoOptions(&optsVideo);

        AVISaveOptionsFree(numStreams, opts);

        result = true;
    }

    // Clean up everything we created here.
    if (psAudio)
        AVIStreamRelease(psAudio);
    if (psVideo)
        AVIStreamRelease(psVideo);
    if (pfile)
        AVIFileRelease(pfile);

    DeleteFile(fname);

    return result;
}

bool TimeShift::SetVideoOptions(AVICOMPRESSOPTIONS *opts)
{
    // Set the newly selcted compression codec, if it was indeed set.
    if (opts->fccHandler)
        m_infoVideo.fccHandler = opts->fccHandler;

    // Tweak the options a bit before saving them off.
    opts->fccHandler = m_infoVideo.fccHandler; // In case it was zero.
    opts->dwFlags &= ~AVICOMPRESSF_INTERLEAVE; // No interleaving.

    if (m_setOpts && m_optsVideo.lpParms && m_optsVideo.cbParms)
        delete m_optsVideo.lpParms;
    m_optsVideo = *opts;
    if (opts->lpParms && opts->cbParms)
    {
        m_optsVideo.lpParms = new BYTE[opts->cbParms];
        memcpy(m_optsVideo.lpParms, opts->lpParms, opts->cbParms);
    }

    m_setOpts = true;

    return true;
}

bool TimeShift::UpdateAudioInfo(void)
{
    // Leave all other stream header fields as they were above.
    m_infoAudio.fccHandler = 0; // optsAudio.fccHandler // Should be zero.
    m_infoAudio.dwScale = m_waveFormat.nBlockAlign;
    m_infoAudio.dwRate = m_waveFormat.nAvgBytesPerSec;
    m_infoAudio.dwSuggestedBufferSize = m_infoAudio.dwRate / m_fps;
    m_infoAudio.dwSampleSize = m_waveFormat.nBlockAlign;

    return true;
}

bool TimeShift::ReadFromIni(void)
{
    extern char szIniFile[MAX_PATH];
    char temp[1000] = "";

    AVICOMPRESSOPTIONS opts;
    if (GetPrivateProfileStruct(
        "TimeShift", "Video", &opts, sizeof(opts), szIniFile))
    {
        opts.lpParms = new BYTE[opts.cbParms];

        // If this one fails, that's ok, there may not be compression params.
        GetPrivateProfileStruct(
            "TimeShift", "Codec", opts.lpParms, opts.cbParms, szIniFile);

        SetVideoOptions(&opts);

        delete opts.lpParms;
    }

    GetPrivateProfileStruct(
        "TimeShift", "Audio", &m_waveFormat, sizeof(m_waveFormat), szIniFile);

    UpdateAudioInfo();

    GetPrivateProfileString(
        "TimeShift", "WaveInDevice", "", m_waveInDevice, MAXPNAMELEN, szIniFile);

    GetPrivateProfileString(
        "TimeShift", "WaveOutDevice", "", m_waveOutDevice, MAXPNAMELEN, szIniFile);

    m_recHeight = GetPrivateProfileInt(
        "TimeShift", "RecHeight", m_recHeight, szIniFile);

    return true;
}

bool TimeShift::WriteToIni(void)
{
    extern char szIniFile[MAX_PATH];
    char temp[1000];

    if (m_setOpts)
    {
        WritePrivateProfileStruct(
            "TimeShift", "Video", &m_optsVideo, sizeof(m_optsVideo),
            szIniFile);

        if (m_optsVideo.lpParms && m_optsVideo.cbParms)
            WritePrivateProfileStruct(
                "TimeShift", "Codec",
                m_optsVideo.lpParms, m_optsVideo.cbParms,
                szIniFile);
    }

    WritePrivateProfileStruct(
        "TimeShift", "Audio", &m_waveFormat, sizeof(m_waveFormat), szIniFile);

    WritePrivateProfileString("TimeShift", "WaveInDevice", m_waveInDevice, szIniFile);

    WritePrivateProfileString("TimeShift", "WaveOutDevice", m_waveOutDevice, szIniFile);

    sprintf(temp, "%u", m_recHeight);
    WritePrivateProfileString("TimeShift", "RecHeight", temp, szIniFile);

    return true;
}


