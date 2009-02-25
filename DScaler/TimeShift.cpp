////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
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
// Revision 1.43  2007/02/18 22:42:37  robmuller
// Removed include to unused header file.
//
// Revision 1.42  2006/12/13 01:10:01  robmuller
// Fix compile warnings with Visual Studio 2005 Express.
//
// Revision 1.41  2005/10/15 19:45:54  dosx86
// Added support for file size limits
//
// Revision 1.40  2005/10/13 04:57:46  dosx86
// Changed the path verification so that the default path is also verified before it's used
//
// Revision 1.39  2005/09/24 18:42:37  dosx86
// Some of the settings functions were changed so they don't depend on the current values in the TIME_SHIFT structure. The settings only get saved when OK is pressed in the configuration dialog.
//
// Revision 1.38  2005/07/24 08:47:35  adcockj
// removed unused setting
//
// Revision 1.37  2005/07/17 20:43:23  dosx86
// Removed the C++ class and reorganized the functions. Uses the new AVI file
// functions.
//
// Revision 1.34  2004/08/12 16:27:47  adcockj
// added timeshift changes from emu
//
// Revision 1.33  2003/12/29 01:58:46  robmuller
// Fix problem with latest addition.
//
// Revision 1.32  2003/12/29 01:27:53  robmuller
// Added AVI file splitting.
//
// Revision 1.31  2003/12/28 22:59:00  robmuller
// IsRunning() returns TRUE now if timeshifting is being done.
//
// Revision 1.30  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.29  2003/10/11 17:25:53  laurentg
// Comments updated
//
// Revision 1.28  2003/10/11 15:45:50  laurentg
// Saving of compression options fixed
//
// Revision 1.27  2003/09/13 13:59:09  laurentg
// half height mode removed - Some menu actions like play or pause disabled
//
// Revision 1.26  2003/08/04 23:48:24  laurentg
// Use extra buffer when recording DScaler output frames
//
// Revision 1.25  2003/07/29 13:33:07  atnak
// Overhauled mixer code
//
// Revision 1.24  2003/07/08 21:04:59  laurentg
// New timeshift mode (full height) - experimental
//
// Revision 1.23  2003/07/05 12:59:51  laurentg
// Timeshift enabled + some improvments
//
// Revision 1.22  2003/07/02 21:44:19  laurentg
// TimeShift settings
//
// Revision 1.21  2003/04/23 08:20:19  adcockj
// Save width on record
//
// Revision 1.20  2003/01/21 09:09:55  adcockj
// Prevent warning from appearing on exit if timshift hasn't been used
//
// Revision 1.19  2003/01/20 15:18:33  adcockj
// Added warning
//
// Revision 1.18  2002/12/06 08:20:21  atnak
// Fixes audio settings corrupting if user Cancels audio options.
//
// Revision 1.17  2002/06/05 22:03:40  adcockj
// Hopefully fixed some timeshift issues and some bracketing changes
//
// Revision 1.16  2002/05/24 22:51:52  robmuller
// Patch #560339 from PietOO.
// Added missing emms statements.
//
// Revision 1.15  2002/02/19 16:04:01  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.14  2002/02/09 11:09:50  temperton
// Frame rate of created AVI now depends on TV format.
//
// Revision 1.13  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.12  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.11  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.10  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.9  2001/11/20 11:43:00  temperton
// Store wave-device names instead of indexes in ini
//
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

/**
 * @file TimeShift.cpp Time Shift Functions
 */

////////////////////////////////////////////////////////////////////////////////////////
//Emu Changes 
//
//28 March 04  Edited the main warning message
//             put in another warning message 
//             played m_waveFormat.nAvgBytesPerSec to sync 
//             the sound and vid.
//             (Might have done some other stuff that I cant remember).
//
//30 March 04  Commented out: 
//             if (m_setOptsVideo && m_optsVideo.lpParms && m_optsVideo.cbParms) 
//             delete m_optsVideo.lpParms; 
//             in 'bool CTimeShift::SetVideoOptions(AVICOMPRESSOPTIONS *opts)' 
//             Codec setting / config saving / retreval now seems fine .....
//             I tesed for an hour and could not make it fall over - whatever I tried  
// 
//02 April 04  Added disk space management:
//             1. If less than 250MB - can't start recording.
//             2. If 'set video file size' + 250MB (for system) is less than available 
//                space change 'set video file size' to available space - 250MB.
//             3. If less than 300MB on a file split - dont start new file.
//
//30 June 04   1. Made disk space management general A to Z.
//             2. Added record timer fo finite recordings.
//             3. Added simple single event record scheduler
//             4. Added AV sync adjuster - stored in INI.
//             5. #define BT848_ADC_CRUSH (0<<1) // set CRUSH default to off for BT848
//
//09 July 04   1. Fixed the TS_HALFHEIGHTEVEN option. This is the routine that creates   
//                a single line for the video recording from the live ODD and EVEN lines. 
//                The recorded line is the pixel average of the ODD and EVEN lines. This 
//                pixel averaging makes the recorded image less.
/////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"       // manditory precompiled header first
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "TimeShift.h"    // this module
#include "DScaler.h"      // hWnd global
#include "TSOptionsDlg.h" // CTSOptionsDlg
#include "TSCompressionDlg.h"
#include "MixerDev.h"     // Mute and UnMute
#include "Settings.h"     // Setting_Set/GetValue
#include "Cpu.h"          // CpuFeatureFlags
#include "Providers.h"    // Providers_GetCurrentSource
#include "DebugLog.h"     // LOG

#define P3_OR_BETTER (FEATURE_SSE | FEATURE_MMXEXT)
#define BUG()\
{\
    char bugText[256];\
    _snprintf(bugText, sizeof(bugText), "Bug found in %s around line %d",\
                                        __FILE__, __LINE__);\
    MessageBox(NULL, bugText, "Error", MB_OK);\
}

/* This is just to ensure that the TimeShift data is handled properly */
TIME_SHIFT *timeShift = NULL;

/* Internal function prototypes */
bool TimeShiftGetDimensions(BITMAPINFOHEADER *bih, int recHeight,
                            tsFormat_t format);
bool TimeShiftGetWaveInDeviceIndex(int *index);
bool TimeShiftGetWaveOutDeviceIndex(int *index);
bool TimeShiftReadFromINI(void);
bool TimeShiftWriteToINI(void);

/** Checks the recording format and changes its value if it's invalid
 * \param format The format to check
 * \return A valid format
 */

__inline tsFormat_t makeFormatValid(tsFormat_t format)
{
    if (format != FORMAT_YUY2 && format != FORMAT_RGB)
       format = FORMAT_YUY2;

    return format;
}

/**********************************************************************
 *                   Image processing functions                       *
 **********************************************************************/

/** \todo Use this formula, it's our (BT and dScaler) "standard".

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

/* YUV <--> RGB conversion

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

/* Used for the YUV -> RGB conversions.
   Note names are most-signif first, but the contents are least-signif first. */
static unsigned short const8000[4] = { 0x8000, 0x8000, 0x8000, 0x8000 };
static unsigned short const00ff[4] = { 0x00ff, 0x00ff, 0x00ff, 0x00ff };
static short          RvRuGvGu[4]  = { -5629, -11689, -15, 22966 };
static short          GvGuBvBu[4]  = { 29025, -45, -5629, -11689 };
static short          RvRuBvBu[4]  = { 29025, -45, -15,    22966 };

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

/* These are used for the RGB -> YUV conversion functions.
   Note names are most-signif first, but the contents are least-signif first. */
static short const128_0_128_0[4] = { 0, 128, 0, 128 };
static short UgUbYgYb[4]         = { 3736, 19235,  16384, -10879 };
static short Ur0Yr0[4]           = {    0,  9798,     0,   -5505 };
static short VgVbYgYb[4]         = { 3736, 19235, -2654,  -13730 };
static short Vr0Yr0[4]           = {    0,  9798,     0,   16384 };

/** YUV to RGB conversion with SSE and MMX instructions */
LPBYTE P3_YUVtoRGB(LPBYTE dest, LPBYTE src, DWORD w)
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
        emms
    }

    return dest;
}

/** YUV to RGB conversion without inline assembly */
LPBYTE C_YUVtoRGB(LPBYTE dest, LPBYTE src, DWORD w)
{
    DWORD w2 = w >> 1;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 : (x))

    while (w2--)
    {
        // Y0 U Y1 V for YUY2...
        int Y0 = (int)DWORD(*src++);
        int U =  (int)DWORD(*src++) - 128;
        int Y1 = (int)DWORD(*src++);
        int V =  (int)DWORD(*src++) - 128;

        int preR = (U * -30 + V * 45931) >> 15;
        int preG = (U * -11258 + V * -23378) >> 15;
        int preB = (U * 58050 + V * -90) >> 15;

        *dest++ = clip(Y0 + preB);
        *dest++ = clip(Y0 + preG);
        *dest++ = clip(Y0 + preR);

        *dest++ = clip(Y1 + preB);
        *dest++ = clip(Y1 + preG);
        *dest++ = clip(Y1 + preR);
    }

    return dest;
}

/** Copies YUV pixels
 * \param dest Where the YUV pixels should be copied to
 * \param src  Where the YUV pixels should be copied from
 * \param w    The number of pixels to copy
 * \return \a dest offset to the pixel after the last pixel that was copied
 */

LPBYTE C_CopyYUV(LPBYTE dest, LPBYTE src, DWORD w)
{
    w <<= 1;

    memcpy(dest, src, w);

    return dest + w;
}

/** Averages YUV scanlines and does a conversion to RGB
 * \note Uses SSE and MMX instructions
 */

LPBYTE P3_AvgYUVtoRGB(LPBYTE dest, LPBYTE src1, LPBYTE src2, DWORD w)
{
    _asm
    {
        mov esi, src1
        mov ecx, src2

        mov edi, dest
        mov eax, w
        shr eax, 2

    next4:
        // Process 4 pixels.  First YUYV is a, second is b.
        // Source must be 8-byte aligned.
        movq mm1, [esi]     ; mm1 = Vb+128 Y1b Ub+128 Y0b  Va+128 Y1a Ua+128 Y0a
        movq mm2, [ecx]     ; mm2 = Vb+128 Y1b Ub+128 Y0b  Va+128 Y1a Ua+128 Y0a
        add esi, 8
        add ecx, 8
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
        emms
    }

    return dest;
}

/** Averages YUV scanlines and does a conversion to RGB
 * \note This is the version that does not use inline assembly
 */

LPBYTE C_AvgYUVtoRGB(LPBYTE dest, LPBYTE src1, LPBYTE src2, DWORD w)
{
    DWORD w2 = w >> 1;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 : (x))

    while (w2--)
    {
        // Y0 U Y1 V for YUY2...
        int Y0a = (int)DWORD(*src1++);
        int Ua =  (int)DWORD(*src1++) - 128;
        int Y1a = (int)DWORD(*src1++);
        int Va =  (int)DWORD(*src1++) - 128;

        int Y0b = (int)DWORD(*src2++);
        int Ub =  (int)DWORD(*src2++) - 128;
        int Y1b = (int)DWORD(*src2++);
        int Vb =  (int)DWORD(*src2++) - 128;

        int Y0 = (Y0a + Y0b) >> 1;
        int U =  (Ua + Ub) >> 1;
        int Y1 = (Y1a + Y1b) >> 1;
        int V =  (Va + Vb) >> 1;

        int preR = (U * -30 + V * 45931) >> 15;
        int preG = (U * -11258 + V * -23378) >> 15;
        int preB = (U * 58050 + V * -90) >> 15;

        *dest++ = clip(Y0 + preB);
        *dest++ = clip(Y0 + preG);
        *dest++ = clip(Y0 + preR);

        *dest++ = clip(Y1 + preB);
        *dest++ = clip(Y1 + preG);
        *dest++ = clip(Y1 + preR);
    }

    return dest;
}

/** Averages two YUV scanlines together
 * \param dest Where the averaged scanline should be stored
 * \param s1   The first input scanline
 * \param s2   The second input scanline
 * \param w    The number of pixels in \a s1 and \a s2
 * \return \a dest offset to the pixel after the last pixel in the scanline
 */

LPBYTE C_AvgYUV(LPBYTE dest, LPBYTE s1, LPBYTE s2, DWORD w)
{
    w >>= 1;
    while (w > 0)
    {
        /* Y0 */
        *dest++ = (BYTE)(((int)*s1++ + (int)*s2++) >> 1);

        /* U */
        *dest++ = (BYTE)(((int)*s1++ + (int)*s2++) >> 1);

        /* Y1 */
        *dest++ = (BYTE)(((int)*s1++ + (int)*s2++) >> 1);

        /* V */
        *dest++ = (BYTE)(((int)*s1++ + (int)*s2++) >> 1);

        w--;
    }

    return dest;
}

LPBYTE P3_AvgYUV(LPBYTE dest, LPBYTE s1, LPBYTE s2, DWORD w)
{
    __asm
    {
        mov edi, dest
        mov esi, s1
        mov ebx, s2

        mov ecx, w
        shr ecx, 2
        jz end              /* Don't copy anything if the counter is equal to zero */

        next4:
          movq mm0, [esi]   /* Copy 4 pixels from s1 */
          movq mm1, [ebx]   /* Copy 4 pixels from s2 */

          pavgb mm0, mm1    /* Average the two scanlines */

          movq [edi], mm0   /* Save the averaged scanline back into dest */

          add edi, 8
          add esi, 8
          add ebx, 8

          dec ecx
        jnz next4

        end:
        mov dest, edi
        emms
    }

    return dest;
}

/** Converts a RGB scanline to a YUV scanline
 * \note Uses SSE and MMX instructions
 */

LPBYTE P3_RGBtoYUV(LPBYTE dest, LPBYTE src, DWORD w)
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
        emms
    }

    return src;
}

/** Converts a RGB scanline to a YUV scanline */
LPBYTE C_RGBtoYUV(LPBYTE dest, LPBYTE src, DWORD w)
{
    DWORD w2 = w >> 1;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 : (x))

    while (w2--)
    {
        int B = *src++;
        int G = *src++;
        int R = *src++;

        int Y = (R * 9798 + G * 19235 + B * 3736) >> 15;
        int U = ((R * -5505 + G * -10879 + B * 16384) >> 15) + 128;

        *dest++ = clip(Y);
        *dest++ = clip(U);

        B = *src++;
        G = *src++;
        R = *src++;

        Y = (R * 9798 + G * 19235 + B * 3736) >> 15;
        int V = ((R * 16384 + G * -13730 + B * -2654) >> 15) + 128;

        *dest++ = clip(Y);
        *dest++ = clip(V);
    }

    return src;
}

/**********************************************************************
 *                TimeShift internal utility functions                *
 **********************************************************************/

/** Free disk space routine - code based on MSDN example (but better)
 * \return An unsigned __int64 (i.e. an unsigned 64 bit integer) as the amount
 *         of free space available to the caller for the volume specified in
 *         lpszDrivePath in bytes.
 */

ULONGLONG GetFreeDiskSpace(void) 
{
    ULONGLONG totalbytes = 0; /**< Free disk space in bytes */

    typedef BOOL (CALLBACK *DSKFREE)(LPCTSTR,
                                     PULARGE_INTEGER,
                                     PULARGE_INTEGER,
                                     PULARGE_INTEGER);
    OSVERSIONINFO VersionInfo;
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    CString strProcName, strErrMsg;
    char lpszDrivePath[4] = "c:\\";

    if (!timeShift)
       return totalbytes;

    lpszDrivePath[0] = timeShift->savingPath[0];

    // Get Operating System Version information
    if(!GetVersionEx(&VersionInfo))
        return totalbytes;

    switch ( VersionInfo.dwPlatformId )
    {
        case VER_PLATFORM_WIN32_NT:
            if (VersionInfo.dwMajorVersion < 4)  // NT 3.51 or less
               break;

        case VER_PLATFORM_WIN32_WINDOWS:       // Windows 95 or higher
            if (LOWORD(VersionInfo.dwBuildNumber) <= 1000)
               break;                          // Non-Windows 95 OSR2

            // This code if for Windows 95 OSR2 or higher and
            // Windows NT 4.0 or higher and Windows XP
            ULARGE_INTEGER FreeBytesAvailableToCaller;
            ULARGE_INTEGER TotalNumberOfBytes;
            ULARGE_INTEGER TotalNumberOfFreeBytes;
            DSKFREE        ProcAdd;

            #ifdef _UNICODE   // Unicode version of GetDiskFreeSpaceEx
                strProcName = _T("GetDiskFreeSpaceExW");
            #else             // Standard version of GetDiskFreeSpaceEx
                strProcName = _T("GetDiskFreeSpaceExA");
            #endif

            // Get ProcAddress for GetDiskFreeSpaceEx function
            ProcAdd = (DSKFREE)GetProcAddress(GetModuleHandle("kernel32.dll"),
                                              strProcName);

            if (!ProcAdd)
            {
                strErrMsg = _T("Could not get the address for ");
                strErrMsg += strProcName;
                ::MessageBox( NULL,
                              strErrMsg,
                              NULL,
                              MB_OK | MB_ICONERROR | MB_TASKMODAL
                             );
                return totalbytes;
            }

            // Execute the GetDiskFreeSpaceEx function
            if (ProcAdd(lpszDrivePath,
                        &FreeBytesAvailableToCaller,
                        &TotalNumberOfBytes,
                        &TotalNumberOfFreeBytes))
               totalbytes = FreeBytesAvailableToCaller.QuadPart;

        return totalbytes;

        default :
            ::MessageBox( NULL,
                          _T("Platform not supported"),
                          NULL,
                          MB_OK | MB_ICONERROR | MB_TASKMODAL
                         );
        return totalbytes;
    }

    // This code if for Non-Windows 95 OSR2 (early Windows 95) and
    // Windows NT 3.51 or lower (well you never know! - would DScaler run?)
    DWORD lpSectorsPerCluster = 0;
    DWORD lpBytesPerSector = 0;
    DWORD lpNumberOfFreeClusters = 0;
    DWORD lpTotalNumberOfClusters = 0;

    GetDiskFreeSpace(lpszDrivePath,
                     &lpSectorsPerCluster,
                     &lpBytesPerSector,
                     &lpNumberOfFreeClusters,
                     &lpTotalNumberOfClusters);
    totalbytes = (ULONGLONG)lpNumberOfFreeClusters *
                 (ULONGLONG)lpBytesPerSector * (ULONGLONG)lpSectorsPerCluster;

    return totalbytes;
}

/** Gets a maximum file size for a volume
 * \return 0 if there's no restrictions for the volume or the maximum file size
 *         in mebibytes for the volume in savingPath.
 */

DWORD GetMaximumVolumeFileSize(const char *path)
{
    char  volume[] = "C:\\";
    char  fileSystem[32];
    DWORD result   = 0;

    if (!timeShift || !path)
       return result;

    volume[0] = path[0];

    if (!GetVolumeInformation(volume, NULL, 0, NULL, NULL, NULL, fileSystem,
                              sizeof(fileSystem)))
       return result;

    _strlwr(fileSystem);
    if (strlen(fileSystem) >= 3)
    {
        /* Check for a FAT file system. Look for the string fat([0-9][0-9])? */
        if (strncmp(fileSystem, "fat", 3)==0)
        {
            if (strlen(fileSystem)==3 ||
                (strlen(fileSystem)==5 &&
                 fileSystem[3] >= '0' && fileSystem[3] <= '9' &&
                 fileSystem[4] >= '0' && fileSystem[4] <= '9'))
               result = AVI_4GiB_LIMIT; /* Use a limit that's a bit less than 4GiB */
        }
    }

    return result;
}

/** Finds the name of a file that data can be written to
 * \param fileName A pointer to where the file name will be stored
 * \param length   The length of \a fileName in bytes
 * \return true if a file name was found or false if no file name is available
 */

bool FindNextFileName(char *fileName, DWORD length)
{
    int curFile = 0;

    if (!timeShift)
       return false;

    do
    {
        if (curFile==1000)
        {
            ErrorBox("Could not create a file.  "
                     "Delete some of your video files and try again.");

            return false;
        }

        if (_snprintf(fileName, length, "%sds%.3u.avi",
                      timeShift->savingPath, curFile++) >= length)
        {
            BUG();
            return false;
        }
    } while (GetFileAttributes(fileName) != 0xffffffff);

    return true;
}

/**********************************************************************
 *                    TimeShift interface functions                   *
 **********************************************************************/

/** TimeShift init
 * \return true if the initialization succeeded or false if it failed
 */

bool TimeShiftInit(HWND hWnd)
{
    WAVEFORMATEX *wfx;

    if (!timeShift)
    {
        timeShift = (TIME_SHIFT *)malloc(sizeof(TIME_SHIFT));
        if (!timeShift)
           return false;

        memset(timeShift, 0, sizeof(TIME_SHIFT));

        InitializeCriticalSection(&timeShift->lock);

        timeShift->mode       = MODE_STOPPED;
        timeShift->format     = FORMAT_YUY2;
        timeShift->recHeight  = TS_HALFHEIGHTEVEN;
        timeShift->fccHandler = 0;
        timeShift->hWnd       = hWnd;

        /* Default audio settings */
        wfx = &timeShift->waveFormat;

        wfx->wFormatTag      = WAVE_FORMAT_PCM;
        wfx->nChannels       = 2;
        wfx->nSamplesPerSec  = 48000;
        wfx->wBitsPerSample  = 16;
        wfx->nBlockAlign     = (wfx->nChannels * wfx->wBitsPerSample) / 8;
        wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;

        /* Set the default saving path */
        TimeShiftSetSavingPath(NULL);

        /* Overwrite any of the above defaults with whatever's in the INI
           file */
        TimeShiftReadFromINI();
    } else
      BUG();

    return true;
}

/** TimeShift shutdown */
void TimeShiftShutdown(void)
{
    if (timeShift)
    {
        /* Stop recording */
        TimeShiftStop();

        DeleteCriticalSection(&timeShift->lock);

        free(timeShift);
        timeShift = NULL;
    } else
      BUG();
}

bool TimeShiftPause(void)
{
    bool result = false;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        /* Only start "time shifing" if paused */
        if (timeShift->mode==MODE_PAUSED)
           result = TimeShiftRecord();
           else if (timeShift->mode==MODE_RECORDING ||
                    timeShift->mode==MODE_SHIFTING)
           {
                /* Pause recording/shifting */
                /*timeShift->mode = MODE_PAUSED;
                result = true;*/
           }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

bool TimeShiftRecord(void)
{
    int       deviceId;
    DWORD     rate, scale;
    char      fileName[MAX_PATH + 1];
    bool      result = false;
    ULONGLONG freeSpace;
    DWORD     limit;

    if (!timeShift)
       return false;

    /* If less than 300 MB free disk space don't even start recoding */
    freeSpace = GetFreeDiskSpace();
    if (freeSpace < 314572800)
    {
        MessageBox(timeShift->hWnd,
                   "Sorry! You do not have enough\n"
                   "disk space to record a video.",
                   "Information",
                   MB_ICONEXCLAMATION | MB_OK);

        return false;
    }

    EnterCriticalSection(&timeShift->lock);

    /* Start recording if stopped  */
    if (timeShift->mode==MODE_STOPPED)
    {
        /* Clear all variables. */
        TimeShiftStop();

        /* Update the recording format */
        TimeShiftGetDimensions(&timeShift->bih, timeShift->recHeight,
                               timeShift->format);

        /* Set up the scanline function pointers */
        if (CpuFeatureFlags & P3_OR_BETTER)
        {
            if (timeShift->format==FORMAT_YUY2)
            {
                timeShift->lpbCopyScanline = C_CopyYUV;
                timeShift->lpbAvgScanline  = P3_AvgYUV;
            } else
            {
                timeShift->lpbCopyScanline = P3_YUVtoRGB;
                timeShift->lpbAvgScanline  = P3_AvgYUVtoRGB;
            }
        } else
        {
            if (timeShift->format==FORMAT_YUY2)
            {
                timeShift->lpbCopyScanline = C_CopyYUV;
                timeShift->lpbAvgScanline  = C_AvgYUV;
            } else
            {
                timeShift->lpbCopyScanline = C_YUVtoRGB;
                timeShift->lpbAvgScanline  = C_AvgYUVtoRGB;
            }
        }

        result = true;

        /* This buffer shouldn't be allocated at this point */
        if (timeShift->buffer.record)
        {
            BUG();
            free(timeShift->buffer.record);
        }

        timeShift->buffer.record = (BYTE *)malloc(timeShift->bih.biSizeImage);
        if (!timeShift->buffer.record)
        {
            TimeShiftStop();

            MessageBox(timeShift->hWnd, "Could not allocate a recording "
                                        "buffer. Your system is too low on "
                                        "memory.",
                                        "Error",
                                        MB_OK | MB_ICONERROR);

            result = false;
        }

        /* Get the file name to use */
        if (result && FindNextFileName(fileName, sizeof(fileName)))
        {
            timeShift->file = aviFileCreate();
            if (!timeShift->file)
            {
                TimeShiftStop();

                MessageBox(timeShift->hWnd, "Could not allocate memory for "
                                            "the AVI recording data. Your "
                                            "system is too low on memory",
                                            "Error",
                                            MB_OK | MB_ICONERROR);

                result = false;
            }

            if (result)
            {
                eVideoFormat VideoFormat;

                if (Providers_GetCurrentSource())
                   VideoFormat = Providers_GetCurrentSource()->GetFormat();
                   else
                   VideoFormat = VIDEOFORMAT_NTSC_M;

                if (GetTVFormat(VideoFormat)->Is25fps)
                {
                    /* PAL */
                    rate  = PAL_RATE;
                    scale = PAL_SCALE;
                } else
                {
                    /* NTSC */
                    rate  = NTSC_RATE;
                    scale = NTSC_SCALE;
                }

                /* Set the format of the video stream */
                aviVideoDefineStream(timeShift->file, rate, scale,
                                     timeShift->bih.biWidth,
                                     timeShift->bih.biHeight);
                aviVideoSetHandler(timeShift->file,
                                   timeShift->fccHandler,
                                   timeShift->bih.biCompression,
                                   timeShift->bih.biBitCount);

                if (TimeShiftGetWaveInDeviceIndex(&deviceId))
                {
                    /* Set the format of the audio stream */
                    aviAudioDefineStream(timeShift->file, deviceId,
                                         &timeShift->waveFormat,
                                         &timeShift->waveFormat);
                }
            }

            if (result)
            {
                limit = GetMaximumVolumeFileSize(fileName);
                if (limit)
                {
                    /* There's a limit to the size of the recorded files. If
                       the set TimeShift limit is 0 (unlimited) or greater than
                       the value that GetMaximumVolumeFileSize returned then
                       use the returned value. Otherwise, use the non-zero
                       limit that the user set. */
                    if (timeShift->sizeLimit && timeShift->sizeLimit < limit)
                       limit = timeShift->sizeLimit;
                } else
                {
                    /* There's no specific limit to the size of the recorded
                       files so use the limit that was set by the user */
                    limit = timeShift->sizeLimit;
                }

                aviSetLimit(timeShift->file, limit);
                if (aviBeginWriting(timeShift->file, fileName))
                   timeShift->mode = MODE_RECORDING;
                   else
                   {
                       aviDisplayError(timeShift->file, timeShift->hWnd, NULL);
                       TimeShiftStop();
                       result = false;
                   }
            }
        } else
          result = false;
    }

    LeaveCriticalSection(&timeShift->lock);

    return result;
}

bool TimeShiftStop(void)
{
    bool result = false;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        timeShift->mode = MODE_STOPPED;

        if (timeShift->file)
        {
            aviFileDestroy(timeShift->file);
            timeShift->file = NULL;
        }

        if (timeShift->buffer.record)
        {
            free(timeShift->buffer.record);
            timeShift->buffer.record = NULL;
        }

        LeaveCriticalSection(&timeShift->lock);

        result = true;
    }

    return result;
}

/**
 * \note Internal function
 */

bool TimeShiftWriteVideo(TDeinterlaceInfo *pInfo)
{
    aviTime_t frameTime;
    LPBYTE    dest;
    DWORD     w, h;
    DWORD     more;
    DWORD     y;
    long      pitch;
    int       offset;
    BYTE      *scanLine[2];
    bool      result = true;

    if (!timeShift || !timeShift->file || !pInfo->PictureHistory[0])
       return false;

    if ((timeShift->recHeight==TS_HALFHEIGHTEVEN &&
         (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN)) ||
        (timeShift->recHeight==TS_HALFHEIGHTODD &&
         (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)) ||
        (timeShift->recHeight==TS_HALFHEIGHTAVG &&
         pInfo->PictureHistory[1] &&
         (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN)) ||
        (timeShift->recHeight==TS_FULLHEIGHT &&
         pInfo->PictureHistory[1] &&
         (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)))
    {
        if (aviFrameReady(timeShift->file))
        {
            frameTime = aviGetTimer();
            dest      = timeShift->buffer.record;
            w         = (pInfo->FrameWidth >> 2) << 2;

            if (timeShift->bih.biWidth < w)
               w = timeShift->bih.biWidth;

            if (timeShift->recHeight != TS_FULLHEIGHT)
               h = min(timeShift->bih.biHeight, pInfo->FieldHeight);
               else
               h = timeShift->bih.biHeight; /* Use the actual height of the frame */

            more = (timeShift->bih.biBitCount >> 3) *
                   (timeShift->bih.biWidth - w);

            /* YUY2 scans from top to bottom. RGB is the opposite. */
            if (timeShift->format==FORMAT_YUY2)
            {
                y     = 0;
                pitch = pInfo->InputPitch;
            } else
            {
                if (timeShift->recHeight != TS_FULLHEIGHT)
                   y = h - 1;
                   else
                   y = (h >> 1) - 1;

                pitch = -pInfo->InputPitch;
            }

            switch (timeShift->recHeight)
            {
                case TS_HALFHEIGHTAVG:
                    scanLine[0] = pInfo->PictureHistory[0]->pData + y *
                                  pInfo->InputPitch;
                    scanLine[1] = pInfo->PictureHistory[1]->pData + y *
                                  pInfo->InputPitch;

                    for (y = 0; y < h; y++)
                    {
                        dest = timeShift->lpbAvgScanline(dest,
                                                         scanLine[0],
                                                         scanLine[1],
                                                         w) + more;

                        scanLine[0] += pitch;
                        scanLine[1] += pitch;
                    }
                break;

                case TS_HALFHEIGHTEVEN:
                case TS_HALFHEIGHTODD:
                    scanLine[0] = pInfo->PictureHistory[0]->pData + y *
                                  pInfo->InputPitch;

                    for (y = 0; y < h; y++)
                    {
                        dest = timeShift->lpbCopyScanline(dest,
                                                          scanLine[0],
                                                          w) + more;

                        scanLine[0] += pitch;
                    }
                break;

                case TS_FULLHEIGHT:
                    /* If the recording format is RGB, then the first scanline
                       in the recording buffer that will be written to is at
                       y = h - 1. If h - 1 is an odd value, then the offset
                       needs to be set so that the first scanline that's saved
                       is the odd one instead of the even one so that the order
                       of the scanlines in the frame is correct. */
                    if (timeShift->format==FORMAT_RGB && ((h - 1) % 2))
                       offset = 1;
                       else
                       offset = 0;

                    /* Even */
                    scanLine[0] = pInfo->PictureHistory[1]->pData + y *
                                  pInfo->InputPitch;

                    /* Odd */
                    scanLine[1] = pInfo->PictureHistory[0]->pData + y *
                                  pInfo->InputPitch;

                    for (y = 0; y < h; y++)
                    {
                        dest = timeShift->lpbCopyScanline(dest,
                                                          scanLine[(y + offset) % 2],
                                                          w) + more;

                        scanLine[(y + offset) % 2] += pitch;
                    }
                break;
            }

            if (!aviSaveFrame(timeShift->file, timeShift->buffer.record,
                              frameTime))
               result = false;
        }
    }

    return result;
}

/* Called from the capture thread */
bool TimeShiftOnNewInputFrame(TDeinterlaceInfo *pInfo)
{
    bool result = false;
    bool error  = false;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        if (timeShift->mode==MODE_RECORDING ||
            timeShift->mode==MODE_PAUSED ||
            timeShift->mode==MODE_SHIFTING)
        {
            if (!TimeShiftWriteVideo(pInfo))
            {
                result = false;
                error  = true;
            } else
            {
                /* If the file size limit has been reached then start recording
                   to a different file */
                if (aviLimitReached(timeShift->file))
                {
                    /* This is pretty cheap but it works */
                    TimeShiftStop();
                    TimeShiftRecord();
                }
            }
        }

        LeaveCriticalSection(&timeShift->lock);

        /* Stop recording if something bad happened */
        if (error)
        {
            aviDisplayError(timeShift->file, timeShift->hWnd,
                            "Recording stopped. You can try and recover "
                            "the data with VirtualDub.");
            TimeShiftStop();
        } else
        {
        }
    }

    return result;
}

bool TimeShiftIsRunning(void)
{
    bool result = false;

    if (timeShift)
    {
        if (timeShift->mode != MODE_STOPPED)
           result = true;
    }

    return result;
}

bool TimeShiftWorkOnInputFrames(void)
{
    return true;
}

bool TimeShiftCancelSchedule(void)
{
    return true;
}

/**********************************************************************
 *                    TimeShift get/set functions                     *
 **********************************************************************/

bool TimeShiftGetDimensions(BITMAPINFOHEADER *bih, int recHeight,
                            tsFormat_t format)
{
    CSource *pSource = Providers_GetCurrentSource();
    int     w, h;
    bool    result = false;

    if (!pSource || !bih)
    {
        BUG();
        return false;
    }

    /* Use the default width and field heights to determine the size */
    w = pSource->GetWidth();
    h = pSource->GetHeight();

    /* Create the bitmap info header */
    memset(bih, 0, sizeof(BITMAPINFOHEADER));
    bih->biSize  = sizeof(BITMAPINFOHEADER);
    bih->biWidth = (w >> 2) << 2; /* 4-pixel (12-byte) align */

    bih->biHeight = h >> 1;
    if (recHeight==TS_FULLHEIGHT)
       bih->biHeight <<= 1; /* Make biHeight a multiple of 2 */

    bih->biPlanes      = 1;
    bih->biBitCount    = format==FORMAT_YUY2 ? 16 : 24;
    bih->biCompression = format==FORMAT_YUY2 ? YUY2_FOURCC : RGB_FOURCC;
    bih->biSizeImage   = (bih->biBitCount >> 3) * bih->biWidth * bih->biHeight;

    return true;
}

bool TimeShiftGetWaveInDeviceIndex(int *index)
{
    int        count;
    int        i;
    bool       result = false;
    WAVEINCAPS wic;

    if (timeShift && index)
    {
        EnterCriticalSection(&timeShift->lock);

        count  = waveInGetNumDevs();
        *index = 0;
        if (strlen(timeShift->waveInDevice) && count)
        {
            for (i = 0; i < count && !result; i++)
            {
                if (waveInGetDevCaps(i, &wic, sizeof(wic))==MMSYSERR_NOERROR)
                {
                    if (!lstrcmp(timeShift->waveInDevice, wic.szPname))
                    {
                        *index = i;
                        result = true;
                    }
                }
            }
        } else
          result = true;

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

bool TimeShiftSetWaveInDevice(char *pszDevice)
{
    bool result = false;

    if (timeShift && pszDevice)
    {
        EnterCriticalSection(&timeShift->lock);
        if (timeShift->mode==MODE_STOPPED)
        {
            strncpy(timeShift->waveInDevice, pszDevice,
                    sizeof(timeShift->waveInDevice));
            result = true;
        }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

bool TimeShiftGetWaveOutDeviceIndex(int *index)
{
    int         count;
    int         i;
    bool        result = false;
    WAVEOUTCAPS woc;

    if (timeShift && index)
    {
        EnterCriticalSection(&timeShift->lock);

        count  = waveOutGetNumDevs();
        *index = 0;
        if (strlen(timeShift->waveOutDevice) && count)
        {
            for(i = 0; i < count && !result; i++)
            {
                if (waveOutGetDevCaps(i, &woc, sizeof(woc))==MMSYSERR_NOERROR)
                {
                    if (!lstrcmp(timeShift->waveOutDevice, woc.szPname))
                    {
                        *index = i;
                        result = true;
                    }
                }
            }
        } else
          result = true;

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

bool TimeShiftSetWaveOutDevice(char *pszDevice)
{
    bool result = false;

    if (timeShift && pszDevice)
    {
        EnterCriticalSection(&timeShift->lock);
        if (timeShift->mode==MODE_STOPPED)
        {
            strncpy(timeShift->waveOutDevice, pszDevice,
                    sizeof(timeShift->waveOutDevice));
            result = true;
        }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

/** Determines if the saving path is valid
 * \return true if the saving path is valid or false otherwise
 * \pre The path to check should be set in savingPath in timeShift
 * \pre If the path contains slashes, they must be back slashes
 */

bool TimeShiftVerifySavingPath(void)
{
    bool  result;
    DWORD attr;
    char  *subString;

    result = true;

    /* Check the file attributes first. Make sure the path exists. */
    attr = GetFileAttributes(timeShift->savingPath);
    if (attr != INVALID_FILE_ATTRIBUTES)
    {
        if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
        {
            /* Might be a file. Try removing everything after the last back
               slash in the path. */
            subString = strrchr(timeShift->savingPath, '\\');
            if (subString)
            {
                /* This should be safe. Even if this points to the last
                   non-NULL character, there always needs to be one byte
                   reserved for the NULL charater at the end of the string. */
                subString[1] = '\0';

                attr = GetFileAttributes(timeShift->savingPath);
                if (attr==INVALID_FILE_ATTRIBUTES ||
                    !(attr & FILE_ATTRIBUTE_DIRECTORY))
                   result = false;
            } else
              result = false;
        }
    } else
      result = false;

    return result;
}

/** Sets a new saving path. DScaler's directory is used by default if the
 * given path was invalid.
 * \param path The new path to set. The default path is always set if this is
 *             NULL.
 * \return true if \a path was used or false if the default path was set
 *         instead
 */

bool TimeShiftSetSavingPath(char *path)
{
    DWORD size;
    BOOL  failed = TRUE;
    int   i;

    if (!timeShift)
       return false;

    EnterCriticalSection(&timeShift->lock);

    if (timeShift->mode==MODE_STOPPED)
    {
        failed = FALSE;

        if (!path)
           failed = TRUE;

        if (!failed)
        {
            /* Get a copy of the new path */
            if (strlen(path) < sizeof(timeShift->savingPath) - 1)
            {
                strcpy(timeShift->savingPath, path);

                /* Convert all forward slashes to back slashes */
                for (i = strlen(timeShift->savingPath) - 1; i >= 0; i--)
                {
                    if (timeShift->savingPath[i]=='/')
                       timeShift->savingPath[i] = '\\';
                }
            } else
            {
                /* Can't use the path because it's longer than our buffer */
                failed = TRUE;
            }
        }

        if (!failed)
        {
            /* Check to see if this path is valid */
            if (!TimeShiftVerifySavingPath())
               failed = TRUE;
        }

        if (failed)
        {
            if (path)
               LOG(1, "Bad path: %s. Using the default.", path);

            /* Use the default path */
            size = GetModuleFileName(NULL, timeShift->savingPath,
                                     sizeof(timeShift->savingPath));
            if (!size || size >= sizeof(timeShift->savingPath) ||
                !TimeShiftVerifySavingPath())
               strncpy(timeShift->savingPath, TS_DEFAULT_PATH,
                       sizeof(timeShift->savingPath));
        }

        /* Make sure the path ends with a back slash */
        i = strlen(timeShift->savingPath) - 1;
        if (timeShift->savingPath[i] != '\\')
        {
            if (i + 2 < sizeof(timeShift->savingPath))
            {
                timeShift->savingPath[i + 1] = '\\';
                timeShift->savingPath[i + 2] = '\0';
            } else
              strncpy(timeShift->savingPath, TS_DEFAULT_PATH,
                      sizeof(timeShift->savingPath));
        }
    }

    LeaveCriticalSection(&timeShift->lock);

    return failed ? false : true;
}

bool TimeShiftIsPathValid(const char *path)
{
    DWORD attr = GetFileAttributes(path);

    return (attr==INVALID_FILE_ATTRIBUTES ||
            !(attr & FILE_ATTRIBUTE_DIRECTORY)) ? false : true;
}

bool TimeShiftSetFileSizeLimit(DWORD sizeLimit)
{
    bool result = false;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        if (timeShift->mode==MODE_STOPPED)
        {
            if (sizeLimit > MAX_FILE_SIZE)
               sizeLimit = MAX_FILE_SIZE;

            timeShift->sizeLimit = sizeLimit;
            result               = true;
        }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

const char *TimeShiftGetSavingPath(void)
{
    return timeShift ? timeShift->savingPath : NULL;
}

DWORD TimeShiftGetFileSizeLimit(void)
{
    return timeShift ? timeShift->sizeLimit : 0;
}

bool TimeShiftGetDimensions(int *w, int *h)
{
    bool result = false;

    if (timeShift && w && h)
    {
        EnterCriticalSection(&timeShift->lock);
        *w = timeShift->bih.biWidth;
        *h = timeShift->bih.biHeight;
        LeaveCriticalSection(&timeShift->lock);

        result = true;
    }

    return result;
}

bool TimeShiftGetWaveInDevice(char **ppszDevice)
{
    bool result = false;

    if (timeShift && ppszDevice)
    {
        *ppszDevice = (char *)&timeShift->waveInDevice;
        result = true;
    }

    return result;
}

bool TimeShiftGetWaveOutDevice(char **ppszDevice)
{
    bool result = false;

    if (timeShift && ppszDevice)
    {
        *ppszDevice = (char*)&timeShift->waveOutDevice;
        result = true;
    }

    return result;
}

bool TimeShiftGetRecHeight(int *index)
{
    bool result = false;

    if (timeShift && index)
    {
        *index = timeShift->recHeight;
        result = true;
    }

    return result;
}

bool TimeShiftSetRecHeight(int index)
{
    bool result = false;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        if (timeShift->mode==MODE_STOPPED)
        {
            timeShift->recHeight = index;
            result               = true;
        }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

bool TimeShiftGetRecFormat(tsFormat_t *format)
{
    bool result = false;

    if (timeShift && format)
    {
        *format = timeShift->format;
        result  = true;
    }

    return result;
}

bool TimeShiftSetRecFormat(tsFormat_t format)
{
    bool result = false;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        if (timeShift->mode==MODE_STOPPED)
        {
            timeShift->format = format;
            result            = true;
        }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

bool TimeShiftGetFourCC(FOURCC *fcc)
{
    bool result = false;

    if (timeShift && fcc)
    {
        EnterCriticalSection(&timeShift->lock);

        if (timeShift->mode==MODE_STOPPED)
        {
            *fcc   = timeShift->fccHandler;
            result = true;
        }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

bool TimeShiftSetFourCC(FOURCC fcc)
{
    bool result = false;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        if (timeShift->mode==MODE_STOPPED)
        {
            timeShift->fccHandler = fcc;
            result                = true;
        }

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

/** Gets a text description describing the specified video settings
 * \param dest      Where the text should be stored
 * \param length    The total length of dest in bytes
 * \param recHeight A TS_*HEIGHT constant
 * \param format    A FORMAT_* constant
 * \return true if the description was saved in \a dest or false if there was
 *         an error
 */

bool TimeShiftGetVideoCompressionDesc(LPSTR dest, DWORD length, int recHeight,
                                      tsFormat_t format)
{
    bool             result = false;
    BITMAPINFOHEADER bih;

    if (dest)
    {
        if (TimeShiftGetDimensions(&bih, recHeight, format))
        {
            if (_snprintf(dest, length, "%dx%dx%d (%s)",
                                bih.biWidth,
                                bih.biHeight,
                                bih.biBitCount,
                                format==FORMAT_YUY2 ? "YUY2" : "RGB") > 0)
               result = true;
        }
    }

    return result;
}

/** Gets a text description describing the specified audio settings
 * \param dest   Where the text should be stored
 * \param length The total length of dest in bytes
 * \return true if the description was saved in \a dest or false if there was
 *         an error
 */

bool TimeShiftGetAudioCompressionDesc(LPSTR dest, DWORD length)
{
    bool result = false;

    if (timeShift && dest)
    {
        EnterCriticalSection(&timeShift->lock);

        if (_snprintf(dest, length, "%d Hz, %d bits, %s",
                      timeShift->waveFormat.nSamplesPerSec,
                      timeShift->waveFormat.wBitsPerSample,
                      timeShift->waveFormat.nChannels==2 ? "stereo" :
                                                           "mono") > 0)
           result = true;

        LeaveCriticalSection(&timeShift->lock);
    }

    return result;
}

/**********************************************************************
 *                    TimeShift dialog functions                      *
 **********************************************************************/

bool TimeShiftOnOptions(void)
{
    bool result = false;

    if (timeShift)
    {
        // No enter critical section here.  If we're stopped, there is no
        // need for one.  If we're recording, we'll show an error box and not
        // access critical member variables anyway so recording can continue.

        if (timeShift->mode==MODE_STOPPED)
        {
            CTSOptionsDlg dlg(CWnd::FromHandle(timeShift->hWnd));
            if (dlg.DoModal()==IDOK)
            {
                result = true;
                TimeShiftWriteToINI();
            }
        } else
          MessageBox(timeShift->hWnd,
                     "TimeShift options are only available during stop mode.",
                     "Information",
                     MB_OK);
    }

    return result;
}

/** Selects a codec to use for video compression
 * \param hWndParent The parent window of the dialog that will be displayed
 * \param recHeight  A TS_*HEIGHT constant
 * \param format     A FORMAT_* constant
 * \param fccHandler A pointer to the default FCC to use. The selected FCC will
 *                   also be stored here.
 * \retval true  A new FCC has been selected and \a fccHandler was updated
 * \retval false A new FCC was not selected and the value of \a fccHandler was
 *               not changed
 */

bool TimeShiftVideoCompressionOptions(HWND hWndParent, int recHeight,
                                      tsFormat_t format, FOURCC *fccHandler)
{
    COMPVARS         cv;
    bool             result = false;
    BITMAPINFOHEADER bih;

    if (fccHandler && TimeShiftGetDimensions(&bih, recHeight, format))
    {
        memset(&cv, 0, sizeof(COMPVARS));

        cv.cbSize     = sizeof(COMPVARS);
        cv.dwFlags    = ICMF_COMPVARS_VALID;
        cv.fccType    = ICTYPE_VIDEO;
        cv.fccHandler = *fccHandler;

        if (ICCompressorChoose(hWndParent, 0, &bih, NULL, &cv, NULL))
        {
            *fccHandler = cv.fccHandler;
            result      = true;

            ICCompressorFree(&cv);
        }
    } else
      BUG();

    return result;
}

bool TimeShiftOnSetMenu(HMENU hMenu)
{
    bool result = false;
    int  item   = IDM_TSSTOP;

    if (timeShift)
    {
        EnterCriticalSection(&timeShift->lock);

        switch (timeShift->mode)
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

    if (timeShift)
       LeaveCriticalSection(&timeShift->lock);

    return result;
}

/**********************************************************************
 *                        TimeShift settings                          *
 **********************************************************************/

bool TimeShiftReadFromINI(void)
{
    extern char szIniFile[MAX_PATH];
    char        path[MAX_PATH + 1];
    bool        result = false;

    if (timeShift)
    {
        GetPrivateProfileString("TimeShift", "WaveInDevice", "",
                                timeShift->waveInDevice, MAXPNAMELEN,
                                szIniFile);
        GetPrivateProfileString("TimeShift", "WaveOutDevice", "",
                                timeShift->waveOutDevice, MAXPNAMELEN,
                                szIniFile);

        timeShift->recHeight = GetPrivateProfileInt("TimeShift", "RecHeight",
                                                    timeShift->recHeight,
                                                    szIniFile);

        timeShift->format = (tsFormat_t)GetPrivateProfileInt("TimeShift",
                                                             "RecFormat",
                                                             timeShift->format,
                                                             szIniFile);

        /* Make sure the format is valid */
        timeShift->format = makeFormatValid(timeShift->format);

        GetPrivateProfileStruct("TimeShift", "VideoFCC",
                                &timeShift->fccHandler, sizeof(FOURCC),
                                szIniFile);

        GetPrivateProfileString("TimeShift", "SavingPath",
                                timeShift->savingPath, path, sizeof(path),
                                szIniFile);
        TimeShiftSetSavingPath(path);

        TimeShiftSetFileSizeLimit(GetPrivateProfileInt("TimeShift",
                                                       "AVIFileSizeLimit",
                                                       timeShift->sizeLimit,
                                                       szIniFile));

        result = true;
    }

    return result;
}

bool TimeShiftWriteToINI(void)
{
    extern char szIniFile[MAX_PATH];
    bool        result = false;
    TCHAR       temp[32];

    if (timeShift)
    {
        WritePrivateProfileString("TimeShift", "SavingPath",
                                  timeShift->savingPath, szIniFile);

        _snprintf(temp, sizeof(temp), "%u", timeShift->sizeLimit);
        WritePrivateProfileString("TimeShift", "AVIFileSizeLimit", temp,
                                  szIniFile);

        WritePrivateProfileString("TimeShift", "WaveInDevice",
                                  timeShift->waveInDevice, szIniFile);
        WritePrivateProfileString("TimeShift", "WaveOutDevice",
                                  timeShift->waveOutDevice, szIniFile);

        _snprintf(temp, sizeof(temp), "%u", timeShift->recHeight);
        WritePrivateProfileString("TimeShift", "RecHeight", temp, szIniFile);

        _snprintf(temp, sizeof(temp), "%u", timeShift->format);
        WritePrivateProfileString("TimeShift", "RecFormat", temp, szIniFile);

        WritePrivateProfileStruct("TimeShift", "VideoFCC",
                                  &timeShift->fccHandler, sizeof(FOURCC),
                                  szIniFile);

        result = true;
    }

    return result;
}












#ifdef ____TO_DO
bool CTimeShift::OnRecord(void)
{
    AssureCreated();

    bool result = false;
    extern char szIniFile[MAX_PATH];

    //////////////////////////////////////////
    // Simple single event scheduler.  It just
    // times down until time to record within 
    // the next 24 hour period.
    //////////////////////////////////////////

    // Get the scheduled record time from INI file
    m_Start = 0; // Initialize it first
    m_Start = (GetPrivateProfileInt(
        "Schedule", "Start", m_Start, szIniFile));

    if (m_Start != 0) //has a schedule time been set (0 = not set)?
    {
        ScheduleF = true; // set the flag to show it is a scheduled recording
        /* 
        Get local time and fill the structure.
        For reference: the structure of tm is:
        struct tm
        {
            int tm_sec;   // seconds after the minute (0-61)
            int tm_min;   // minutes after the hour (0-59)
            int tm_hour;  // hours since midnight (0-23)
            int tm_mday;  // day of the month (1-31)
            int tm_mon;   // months since January (0-11)
            int tm_year;  // elapsed years since 1900 
            int tm_wday;  // days since Sunday (0-6)
            int tm_yday;  // days since January 1st (0-365)
            int tm_isdst; // 1 if daylight savings is on, zero if not, -1 if unknown
        }
        */
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
    
        int m_hours = (m_Start / 100); // Get the scheduled hours value
        
        sleeptime = 0; // Initialise sleeptime   

        // Calculate number of seconds to wait from the hours part
        if (m_hours > timeinfo->tm_hour)
        {
            sleeptime = ((m_hours - timeinfo->tm_hour) * 3600);
        }   
        if (m_hours < timeinfo->tm_hour)
        {
            sleeptime = (((m_hours + 24) - timeinfo->tm_hour) * 3600);
        }
        
        // Calculate number of seconds to wait from the minutes part
        int m_minutes = (m_Start % 100); 

        if (m_minutes > timeinfo->tm_min)
        {
            sleeptime = (sleeptime + (m_minutes - timeinfo->tm_min) * 60);
        }
        
        if (m_minutes < timeinfo->tm_min)
        {
            if (m_hours == timeinfo->tm_hour)
            {
                sleeptime = (24 * 3600);
            }
            sleeptime = (sleeptime - (timeinfo->tm_min - m_minutes) * 60);
        }

        m_Start = 0; //reset the schedule start time 
        WritePrivateProfileInt(
        "Schedule", "Start", m_Start, szIniFile); // reset INI

        // Do a timer messagebox to the start of the scheduled recording
        SchedMessageBox mbox(CWnd::FromHandle(hWnd));   // handle
        int m_delay = sleeptime; // set the count-down period
        bool m_close = true; // irrelevant - hard coded the switch
        // irrelevant - hardcoded the icon
        SchedMessageBox::MBIcon mbicon = SchedMessageBox::MBIcon::MBICONINFORMATION;
        // call the timer messagebox
        mbox.MessageBox("Message is hard coded! NULL", m_delay,
            m_close,(SchedMessageBox::MBIcon)mbicon);

        if (true == CancelSched) // Was the scheduled recording cancelled
        {
            CancelSched = false; // Reset flag for next time thru
            return 0; // Quit
        }
    }
        
    ////////////////////////////////////////
    // End of simple single event scheduler.
    ////////////////////////////////////////

    ///////////////////////
    // Simple record timer.
    ///////////////////////
    // Need to flag this is first time thru
    // because can come thru here more than once
    // during a single recording (on a file split)
    if (false == RecordTimerCheckedF)
    {
        RecordTimerCheckedF = true; // set the checked flag

        // Get the Timed Recordng value from INI file   
        m_Time = 0; // Initialise
        m_Time = (GetPrivateProfileInt(
        "Schedule", "Time", m_Time, szIniFile));

        if (m_Time != 0) // Has the record timer been set
        {
            RecordTimerF = true; // set the flag to show it is a timed recording

            // do the calcs to find recording end time
            time_t seconds;
            seconds = time (NULL);
            EndRecordTime = (seconds + (m_Time * 60));
            // Now we know that when system clock == EndRecordTime
            // the timed recording is done.

            // Clear the INI file of the timed recording value
            m_Time = 0; // Reset the timer
            WritePrivateProfileInt(
            "Schedule", "Time", m_Time, szIniFile);
        }
        else
        {
            RecordTimerCheckedF = false; // need to reset check flag
            // in case it was just a manual recording - so we check 
            // next time there is a recording to see if timed
        }
    }
    //////////////////////////////
    // End of simple record timer.
    //////////////////////////////

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Save this off before we start recording.
        if (m_pTimeShift->m_mode == MODE_STOPPED)
        {
            CSource* pSource = Providers_GetCurrentSource();
            if (pSource != NULL)
            {
                m_pTimeShift->m_origPixelWidth = pSource->GetWidth();
            }
        }

        // if nofreespace is true it means a recording was in progress but running
        // low on space and the disk space check routine has decided another file
        // cannot be started (on a file split), and set the flag to get out of TimeShift
        if (nofreespace) 
        {
            // Message boxes for endless or timed recording
            LeaveCriticalSection(&m_pTimeShift->m_lock); //unlock the stream 
            if (RecordTimerF)
            {
                MessageBox(hWnd,
                    "Sorry! Recording stopped!\n"
                    "\n"
                    "You do not have enough\n"
                    "disk space to continue.\n"
                    "\n"
                    "Your recording was saved.",
                    "Information",
                    MB_ICONEXCLAMATION | MB_OK);
            }
            else
            {
                MessageBox(hWnd,
                    "Sorry! Timed recording stopped!\n"
                    "\n"
                    "You do not have enough\n"
                    "disk space to continue.\n"
                    "\n"
                    "Only a part of your timed\n"
                    "recording was saved.",
                    "Information",
                    MB_ICONEXCLAMATION | MB_OK);
            }
            nofreespace = false; // Reset the nofreespace flag
            
            // Reset the flags for scheduled / timed recording
            RecordTimerCheckedF = false; // We check again on new recording
            ScheduleF = false; // Rest flag so to check again on new recording
            TimedRecodingDone = false; // Rest flag so to check again on new recording          
            RecordTimerF = false; // Rest flag so to check again on new recording
            return result; // Exit TimeShift
        }
    
        // Message and close down if timed recording is done
        if (TimedRecodingDone) // Has the timed recording finished?
        {
            LeaveCriticalSection(&m_pTimeShift->m_lock); //unlock the stream
            
            
            if (ScheduleF) // Was it a scheduled recording?
            {
                MessageBox(hWnd,
                        "Your scheduled recording was saved.",
                        "Recording Information",
                        MB_ICONEXCLAMATION | MB_OK);
            }
            else
            {
                MessageBox(hWnd,
                        "Your timed recording was saved.",
                        "Recording Information",
                        MB_ICONEXCLAMATION | MB_OK);
            }
            // Initialise the flags for scheduled / timed recording
            ScheduleF = false;
            TimedRecodingDone = false;
            RecordTimerCheckedF = false;
            RecordTimerF = false;
            return result; //exit TimeShift         
        }

        // Only start recording if we're stopped.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ?
            m_pTimeShift->Record(false) : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool TimeShiftSetPixelWidth(int pixelWidth)
{
    bool    result = false;
    CSource *pSource;

    if (timeShift)
    {
        pSource = Providers_GetCurrentSource();
        if (pSource)
           pSource->SetWidth(pixelWidth);

        result = true;
    }

    return result;
}

bool TimeShiftDoMute(bool mute)
{
    bool result = false;

    if (timeShift)
    {
        Mixer_SetMute(mute);

        //  I don't know what this below is for but what it's doing
        //  can cause a lot of problems --AtNak 2003-07-29

/*        if (mute)
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
                /// \todo ???
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
        }*/

        result = true;
    }

    return result;
}
#endif
