/////////////////////////////////////////////////////////////////////////////
// $Id: TimeShift.h,v 1.9 2001-11-22 13:32:03 adcockj Exp $
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

#ifndef __TIMESHIFT_H___
#define __TIMESHIFT_H___

#include "DS_ApiCommon.h" // TDeinterlaceInfo struct.

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

class CTimeShift
{
    friend CTSOptionsDlg;

// Interface
public:
    // There is no public "create" method, it's created the first time you
    // record, popup options, etc.  Call Destroy() as often as you want to
    // assure that all timeshift resources are freed.
    static bool OnDestroy(void);

    // Standard control method.
    static bool OnRecord(void);
    static bool OnPause(void);
    static bool OnPlay(void);
    static bool OnStop(void);
    static bool OnFastForward(void) { return false; } // Not yet implemented.
    static bool OnFastBackward(void) { return false; } // Not yet implemented.
    static bool OnGoNext(void);
    static bool OnGoPrev(void);

    // Pops up options dialog.  Call when stopped.
    static bool OnOptions(void);

    // Call these when you have new frames or audio data to read/write.
    static bool OnNewFrame(TDeinterlaceInfo *info);
    static bool OnWaveInData(void);
    static bool OnWaveOutDone(void);

    // Call this to update the radio check in the timeshift submenu.
    static bool OnSetMenu(HMENU hMenu);

    static LPBYTE(*m_YUVtoRGB)(LPBYTE dest,short *src,DWORD w);
    static LPBYTE(*m_AvgYUVtoRGB)(LPBYTE dest,short *src1,short *src2,DWORD w);
    static LPBYTE(*m_RGBtoYUV)(short *dest,LPBYTE src,DWORD w);

// Implementation
private:
    CTimeShift();
    ~CTimeShift();

    // Create the timeshift object if it's not already created.
    static bool AssureCreated(void);

    // Can only call this when stopped.  Call before control methods.
    static bool OnSetDimensions(void);
    static bool OnGetDimenstions(int *w, int *h);
    static bool OnSetWaveInDevice(char *pszDevice);
    static bool OnGetWaveInDevice(char **ppszDevice);
    static bool OnSetWaveOutDevice(char *pszDevice);
    static bool OnGetWaveOutDevice(char **ppszDevice);
    static bool OnSetRecHeight(int index);
    static bool OnGetRecHeight(int *index);

    // Called from within the options dialog to popup compression dialog.
    static bool OnCompressionOptions(void);

    // A thread-safe wrapper for setting CurrentX.
    static bool SetBT848PixelWidth(int pixelWidth);

    // A wrapper for using the MixerDev for muting during pause/shifting.
    static bool DoMute(bool mute);

    // Start recording, with the option of pausing live TV.
    bool Record(bool pause);
    bool Play(void);
    bool Stop(void);
    bool GoNext(void);
    bool GoPrev(void);

    // The actaual videoframe/audiosample avi writing methods.
    bool WriteVideo(TDeinterlaceInfo *info);
    bool ReadVideo(TDeinterlaceInfo *info);
    bool WriteAudio(void);
    bool ReadAudio(void);

    // These simply implement their public static counterparts.
    bool SetDimensions(); // Uses current DScaler settings to set.
    bool SetWaveInDevice(char* pszDevice);
    bool SetWaveOutDevice(char* pszDevice);
    bool GetWaveInDeviceIndex(int *index);
    bool GetWaveOutDeviceIndex(int *index);
    bool SetRecHeight(int index);
    bool CompressionOptions(void);

    bool SetVideoOptions(AVICOMPRESSOPTIONS *opts);
    bool UpdateAudioInfo(void);
    bool ReadFromIni(void);
    bool WriteToIni(void);

    static CTimeShift *m_pTimeShift;

    CRITICAL_SECTION m_lock; // Audio and video come from different threads.

    enum MODE
    {
        MODE_STOPPED,   // Not playing nor recording nor paused, default.
        MODE_PLAYING,   // Standard playback of avi file.
        MODE_RECORDING, // Standard recording of live capture.
        MODE_SHIFTING,  // Recording, and playing at a previous offset.
        MODE_PAUSED,    // Shifting, but playing is paused.
        MODE_FASTFWD,   // Scanning forward while playing or shifting.
        MODE_REWIND     // Scanning backward while playing or shifting.
    } m_mode;

    const int m_fps;

    int m_curFile;

    BITMAPINFOHEADER m_bih;
    LPBITMAPINFOHEADER m_lpbi;
    LPBYTE m_recordBits;
    LPBYTE m_playBits; // We'll decode the compressed bits into here.
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
    bool m_setOpts;

    DWORD m_startTimeRecord;
    DWORD m_startTimePlay;
    DWORD m_thisTimeRecord;
    DWORD m_thisTimePlay;
    DWORD m_nextSampleRecord;
    DWORD m_nextSamplePlay;

    // FIXME: Make the 4 and 1<<17 semi-configurable in an advanced dialog?
    // The lower the numbers, the more "skipping" in the avi audio.
    // Also, isn't 2 all we need?  It is for waveOut for sure, but waveIn?

    HWAVEIN m_hWaveIn;
    char m_waveInDevice[MAXPNAMELEN];
    WAVEHDR m_waveInHdrs[4];
    BYTE m_waveInBufs[4][1<<17]; // 512KB total buffer space.
    int m_nextWaveInHdr;

    HWAVEOUT m_hWaveOut;
    char m_waveOutDevice[MAXPNAMELEN];
    WAVEHDR m_waveOutHdrs[4];
    BYTE m_waveOutBufs[4][1<<15]; // 128KB total buffer space.
    int m_nextWaveOutHdr;

    int m_recHeight; // One of TS_*HEIGHT* #defines above.
    int m_origPixelWidth; // What it was before we changed it.
    BOOL m_origUseMixer; // What it was before we changed it.
};

#endif // __TIMESHIFT_H___
