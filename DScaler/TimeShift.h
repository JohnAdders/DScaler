/////////////////////////////////////////////////////////////////////////////
// $Id: TimeShift.h,v 1.4 2001-07-26 15:28:14 ericschmidt Exp $
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

#include "DS_ApiCommon.h" // DEINTERLACE_INFO struct.

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

class TimeShift
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
    static bool OnPlay(void);
    static bool OnPause(void) { return false; } // Not yet implemented.
    static bool OnStop(void);
    static bool OnFastForward(void) { return false; } // Not yet implemented.
    static bool OnFastBackward(void) { return false; } // Not yet implemented.
    static bool OnGoNext(void);
    static bool OnGoPrev(void);

    // Pops up options dialog.  Call when stopped.
    static bool OnOptions(void);

    // Call these when you have new frames or audio data to read/write.
    static bool OnNewFrame(DEINTERLACE_INFO *info);
    static bool OnWaveInData(void);
    static bool OnWaveOutDone(void);

    // Call this to update the radio check in the timeshift submenu.
    static bool OnSetMenu(HMENU hMenu);

// Implementation
private:
    TimeShift();
    ~TimeShift();

    // Create the timeshift object if it's not already created.
    static bool AssureCreated(void);

    // Can only call this when stopped.  Call before control methods.
    static bool OnSetDimensions(void);
    static bool OnGetDimenstions(int *w, int *h);
    static bool OnSetWaveInDevice(int index);
    static bool OnGetWaveInDevice(int *index);
    static bool OnSetWaveOutDevice(int index);
    static bool OnGetWaveOutDevice(int *index);
    static bool OnSetRecHeight(int index);
    static bool OnGetRecHeight(int *index);

    // Called from within the options dialog to popup compression dialog.
    static bool OnCompressionOptions(void);

    bool StartRecord(void);
    bool StartPlay(void);
    bool Stop(void);
    bool GoNext(void);
    bool GoPrev(void);

    // The actaual videoframe/audiosample avi writing methods.
    bool WriteVideo(DEINTERLACE_INFO *info);
    bool ReadVideo(DEINTERLACE_INFO *info);
    bool WriteAudio(void);
    bool ReadAudio(void);

    // These simply implement their public static counterparts.
    bool SetDimensions(); // Uses current DScaler settings to set.
    bool SetWaveInDevice(int index);
    bool SetWaveOutDevice(int index);
    bool SetRecHeight(int index);
    bool CompressionOptions(void);

    bool SetVideoOptions(AVICOMPRESSOPTIONS *opts);
    bool UpdateAudioInfo(void);
    bool ReadFromIni(void);
    bool WriteToIni(void);

    static TimeShift *m_pTimeShift;

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
    int m_w;
    int m_h;
    const int m_bpp;
    int m_pitch;
    LPBYTE m_bits;

    WAVEFORMATEX m_waveFormat;

	PAVIFILE m_pfile;
	PAVISTREAM m_psVideo;
	PAVISTREAM m_psAudio;
	PAVISTREAM m_psCompressedVideo;
	PAVISTREAM m_psCompressedAudio;
    AVISTREAMINFO m_infoVideo;
    AVISTREAMINFO m_infoAudio;
    PGETFRAME m_pGetFrame;
	AVICOMPRESSOPTIONS m_optsVideo;
    bool m_bSetOpts;

    DWORD m_startTime;
    DWORD m_thisTime;
    DWORD m_lastTime;
    DWORD m_nextSample;

    // FIXME: Make the 4 and 1<<17 semi-configurable in an advanced dialog?
    // The lower the numbers, the more "skipping" in the avi audio.
    // Also, isn't 2 all we need?  It is for waveOut for sure, but waveIn?

    HWAVEIN m_hWaveIn;
    int m_waveInDevice;
    WAVEHDR m_waveInHdrs[4];
    BYTE m_waveInBufs[4][1<<17]; // 512KB total buffer space.
    int m_nextWaveInHdr;

    HWAVEOUT m_hWaveOut;
    int m_waveOutDevice;
    WAVEHDR m_waveOutHdrs[4];
    BYTE m_waveOutBufs[4][1<<15]; // 128KB total buffer space.
    int m_nextWaveOutHdr;

    int m_recHeight; // One of TS_*HEIGHT* #defines above.
};

#endif // __TIMESHIFT_H___
