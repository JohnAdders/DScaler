/////////////////////////////////////////////////////////////////////////////
// $Id: TimeShift.h,v 1.19 2003-10-27 10:39:54 adcockj Exp $
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
// Revision 1.18  2003/10/11 17:25:53  laurentg
// Comments updated
//
// Revision 1.17  2003/10/11 15:45:50  laurentg
// Saving of compression options fixed
//
// Revision 1.16  2003/09/13 13:59:09  laurentg
// half height mode removed - Some menu actions like play or pause disabled
//
// Revision 1.15  2003/08/04 23:48:24  laurentg
// Use extra buffer when recording DScaler output frames
//
// Revision 1.14  2003/07/08 21:04:59  laurentg
// New timeshift mode (full height) - experimental
//
// Revision 1.13  2003/07/05 12:59:51  laurentg
// Timeshift enabled + some improvments
//
// Revision 1.12  2003/07/02 21:44:19  laurentg
// TimeShift settings
//
// Revision 1.11  2002/02/09 11:09:50  temperton
// Frame rate of created AVI now depends on TV format.
//
// Revision 1.10  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.9  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.8  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.7  2001/11/20 11:43:00  temperton
// Store wave-device names instead of indexes in ini
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
 * @file timeshift.h timeshift Header file
 */
 
#ifndef __TIMESHIFT_H___
#define __TIMESHIFT_H___

#include "DS_ApiCommon.h" // TDeinterlaceInfo struct.
#include "DS_Control.h"
#include "TreeSettingsGeneric.h"

// We'll only have one instance of a TimeShift, so everything's static.
// Plus, we keep a semi-C-style function call convention which matches much
// of the rest of the DScaler code, instead of referencing a global object ptr.

// Declare this for friend declaration below.
class CTSOptionsDlg;

// These match the radio group in TSOptionsDlg.
#define TS_FULLHEIGHT     0
#define TS_HALFHEIGHTEVEN 1
#define TS_HALFHEIGHTODD  2
#define TS_HALFHEIGHTAVG  3

/**
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
  - Use the standard DScaler Settings for ini file stuff?
  - Use -height for right-side-up AVI?  Matches odd/even field orientation.

  OPTIMIZE:
  - Is there a codec out there that takes YUY2, compress it, but don't convert
    to RGB in the process?  If so, we can get rid of our rgb<-->yuv routines.

  DOC:
  - Must setup Audio Mixer through its dialog.  You can then uncheck the box
    if you don't normally use it, but all the dropdowns should point to the
    input on your audio card that your capture card's audio is attached to.
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
class CTimeShift
{
    friend CTSOptionsDlg;

// Interface
public:
    /** There is no public "create" method, it's created the first time you
        record, popup options, etc.  Call Destroy() as often as you want to
        assure that all timeshift resources are freed.
    */
    static bool OnDestroy(void);

    /// Standard control method.
    static bool OnRecord(void);
    /// Standard control method.
    static bool OnPause(void);
    /// Standard control method.
    static bool OnPlay(void);
    /// Standard control method.
    static bool OnStop(void);
    /// Not yet implemented.
    static bool OnFastForward(void) { return false; }
    /// Not yet implemented.
    static bool OnFastBackward(void) { return false; }
    /// Standard control method.
    static bool OnGoNext(void);
    /// Standard control method.
    static bool OnGoPrev(void);

    /// Pops up options dialog.  Call when stopped.
    static bool OnOptions(void);

	static bool WorkOnInputFrames();
	static bool IsRunning();

    /// Call these when you have new frames or audio data to read/write.
    static bool OnNewInputFrame(TDeinterlaceInfo *info);
    static bool OnNewOutputFrame(TDeinterlaceInfo *info);
    static bool OnWaveInData(void);
    static bool OnWaveOutDone(void);

    /// Call this to update the radio check in the timeshift submenu.
    static bool OnSetMenu(HMENU hMenu);

    static LPBYTE(*m_YUVtoRGB)(LPBYTE dest,short *src,DWORD w);
    static LPBYTE(*m_AvgYUVtoRGB)(LPBYTE dest,short *src1,short *src2,DWORD w);
    static LPBYTE(*m_RGBtoYUV)(short *dest,LPBYTE src,DWORD w);

// Implementation
private:
    CTimeShift();
    ~CTimeShift();

    /// Create the timeshift object if it's not already created.
    static bool AssureCreated(void);

    /// Can only call this when stopped.  Call before control methods.
    static bool OnSetDimensions(void);
    /// Can only call this when stopped.  Call before control methods.
    static bool OnGetDimenstions(int *w, int *h);
    /// Can only call this when stopped.  Call before control methods.
    static bool OnSetWaveInDevice(char *pszDevice);
    /// Can only call this when stopped.  Call before control methods.
    static bool OnGetWaveInDevice(char **ppszDevice);
    /// Can only call this when stopped.  Call before control methods.
    static bool OnSetWaveOutDevice(char *pszDevice);
    /// Can only call this when stopped.  Call before control methods.
    static bool OnGetWaveOutDevice(char **ppszDevice);
    /// Can only call this when stopped.  Call before control methods.
    static bool OnSetRecHeight(int index);
    /// Can only call this when stopped.  Call before control methods.
    static bool OnGetRecHeight(int *index);

    /// Called from within the options dialog to popup compression dialog.
    static bool OnCompressionOptions(void);

    /// A thread-safe wrapper for setting CurrentX.
    static bool SetPixelWidth(int pixelWidth);

    /// A wrapper for using the MixerDev for muting during pause/shifting.
    static bool DoMute(bool mute);

    /// Start recording, with the option of pausing live TV.
    bool Record(bool pause);
    bool Play(void);
    bool Stop(void);
    bool GoNext(void);
    bool GoPrev(void);

    /// The actaual videoframe/audiosample avi writing methods.
    bool WriteVideo(TDeinterlaceInfo *info);
    bool ReadVideo(TDeinterlaceInfo *info);
    bool WriteVideo2(TDeinterlaceInfo *info);
    bool ReadVideo2(TDeinterlaceInfo *info);
    bool WriteAudio(void);
    bool ReadAudio(void);

    /// These simply implement their public static counterparts.
    /// Uses current DScaler settings to set.
    bool SetDimensions(); 
    bool SetWaveInDevice(char* pszDevice);
    bool SetWaveOutDevice(char* pszDevice);
    bool GetWaveInDeviceIndex(int *index);
    bool GetWaveOutDeviceIndex(int *index);
    bool SetRecHeight(int index);
    bool CompressionOptions(void);

    bool SetVideoOptions(AVICOMPRESSOPTIONS *opts);
    bool SetAudioOptions(AVICOMPRESSOPTIONS *opts);
    bool UpdateAudioInfo(void);
    bool ReadFromIni(void);
    bool WriteToIni(void);

    static CTimeShift *m_pTimeShift;

    /// Audio and video come from different threads.
    CRITICAL_SECTION m_lock;

    enum MODE
    {
        /// Not playing nor recording nor paused, default.
        MODE_STOPPED,
        /// Standard playback of avi file.
        MODE_PLAYING,
        /// Standard recording of live capture.
        MODE_RECORDING,
        /// Recording, and playing at a previous offset.
        MODE_SHIFTING,
        /// Shifting, but playing is paused.
        MODE_PAUSED,
        /// Scanning forward while playing or shifting.
        MODE_FASTFWD,
        /// Scanning backward while playing or shifting.
        MODE_REWIND
    } m_mode;

    int m_fps;

    int m_curFile;

    BITMAPINFOHEADER m_bih;
    LPBITMAPINFOHEADER m_lpbi;
    LPBYTE m_recordBits;
    /// We'll decode the compressed bits into here.
    LPBYTE m_playBits; 
    bool m_gotPauseBits;

    WAVEFORMATEX m_waveFormat;

	PAVIFILE m_pfile;
	PAVISTREAM m_psVideo;
	PAVISTREAM m_psAudio;
	PAVISTREAM m_psCompressedVideo;
	PAVISTREAM m_psCompressedAudio;
	PAVISTREAM m_psCompressedVideoP;
	PAVISTREAM m_psCompressedAudioP;
    AVISTREAMINFO m_infoVideo;
    AVISTREAMINFO m_infoAudio;
    PGETFRAME m_pGetFrame;
	AVICOMPRESSOPTIONS m_optsVideo;
	AVICOMPRESSOPTIONS m_optsAudio;
    bool m_setOptsVideo;
    bool m_setOptsAudio;

    DWORD m_startTimeRecord;
    DWORD m_startTimePlay;
    DWORD m_thisTimeRecord;
    DWORD m_thisTimePlay;
    DWORD m_lastFrameRecord;
    DWORD m_lastFramePlay;
    DWORD m_startFramePlay;
    DWORD m_nextSampleRecord;
    DWORD m_nextSamplePlay;

    /** \todo FIXME: Make the 4 and 1<<17 semi-configurable in an advanced dialog?
              The lower the numbers, the more "skipping" in the avi audio.
              Also, isn't 2 all we need?  It is for waveOut for sure, but waveIn?
    */
    HWAVEIN m_hWaveIn;
    char m_waveInDevice[MAXPNAMELEN];
    WAVEHDR m_waveInHdrs[4];
    /// 512KB total buffer space.
    BYTE m_waveInBufs[4][1<<17];
    int m_nextWaveInHdr;

    HWAVEOUT m_hWaveOut;
    char m_waveOutDevice[MAXPNAMELEN];
    WAVEHDR m_waveOutHdrs[4];
    /// 128KB total buffer space.
    BYTE m_waveOutBufs[4][1<<15];
    int m_nextWaveOutHdr;

    /// One of TS_*HEIGHT* #defines above.
    int m_recHeight;
    /// What it was before we changed it.
    int m_origPixelWidth;
    /// What it was before we changed it.
    BOOL m_origUseMixer;
};

SETTING* TimeShift_GetSetting(TIMESHIFT_SETTING Setting);
void TimeShift_ReadSettingsFromIni();
void TimeShift_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* TimeShift_GetTreeSettingsPage();
void TimeShift_FreeSettings();

#endif // __TIMESHIFT_H___
