////////////////////////////////////////////////////////////////////////////
// $Id: TimeShift.cpp,v 1.2 2001-07-24 12:24:25 adcockj Exp $
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
// Revision 1.1  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"       // manditory precompiled header first
#include "TimeShift.h"    // this module
#include "DScaler.h"      // hWnd global
#include "TSOptionsDlg.h" // CTSOptionsDlg
#include "Audio.h"        // Mute and Unmute
#include "BT848.h"        // CurrentX/Y

/*
  This is my personal TODO list, FWIW.  -Eric

  NOW:
  - work on playback and recording simultaneously, read/write avi.
    this is the "guts" of time-shifting.

  LATER:
  - Do X and Y scaling for when the FrameWidth/Height don't match the AVI.
    Y is easy, X is hard.  Perhaps auto-setting CurrentX is way to go for X.
  - enumerate saved avi file dynamic radio checked menu items.
  - in options dialog, add waveform buffer count and size (advanced section).
  - make sure that if m_mode is STOPPED, memory & resource usage is minimized.
  - Remove 4GB filesize limitation.  Maybe try stop then immeditate rerecord.
  - BUG: During playback in windowed mode, writing too wide of lines.
  - TWEAK: You can see the live feed peek in between clips during play mode.
  - For 1/2 height AVIs, average lines?
  - Make full-height AVIs an option for faster systems.
  - Put OnNewFrame() earlier in the OutThreads loop?

  OPTIMIZE:
  - Is there a codec out there that takes YUY2, compress it, but don't convert
    to RGB in the process?  If so, we can get rid of our rgb<-->yuv routines.

  DOC:
  - For speed, video clips are saved at 1/2 height.  Perhaps future versions can
    be optimized to handle full-height recording.
  - pixel width affects DF/S.  Choose a low enough pixel width for 0 DF/S.
  - Currently, things are touchy.  i.e. if you record with pixel width 640, or
    audio at 44.1kHz, etc, make sure you have these same settings on playback.
*/


/////////////////////////////// Static Interface ///////////////////////////////

bool TimeShift::OnDestroy(void)
{
    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        delete m_pTimeShift;
        m_pTimeShift = NULL;

        result = true;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
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
        result = m_pTimeShift->m_mode == MODE_STOPPED ?
            m_pTimeShift->StartRecord() : false;

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

        // Only start playing if we're stopped.
        result = m_pTimeShift->m_mode == MODE_STOPPED ?
            m_pTimeShift->StartPlay() : false;

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
            m_pTimeShift->m_mode == MODE_PLAYING ?
            m_pTimeShift->Stop() : false;

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
        // access member variables anyway so recording can continue.

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
            m_pTimeShift->m_mode == MODE_RECORDING ?
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
        result = m_pTimeShift->m_mode == MODE_RECORDING ?
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
        result = m_pTimeShift->m_mode == MODE_PLAYING ?
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

TimeShift *TimeShift::m_pTimeShift = NULL;

TimeShift::TimeShift()
    :
    m_mode(MODE_STOPPED),
    m_fps(30), // Our avis will always be 30 fps.
    m_curFile(0),
    m_w(0),
    m_h(0),
    m_bpp(24), // We'll always convert to 24bits for compression codecs.
    m_pitch(0),
    m_bits(NULL),
	m_pfile(NULL),
    m_psVideo(NULL),
    m_psAudio(NULL),
    m_psCompressedVideo(NULL),
    m_psCompressedAudio(NULL),
    m_pGetFrame(NULL),
    m_bSetOpts(false),
    m_startTime(0),
    m_thisTime(0),
    m_lastTime(0),
    m_nextSample(0),
    m_hWaveIn(NULL),
    m_waveInDevice(0),
    m_nextWaveInHdr(0),
    m_hWaveOut(NULL),
    m_waveOutDevice(0),
    m_nextWaveOutHdr(0)
{
    InitializeCriticalSection(&m_lock);

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
	m_infoVideo.dwSuggestedBufferSize = m_pitch * m_h;
    m_infoVideo.dwQuality = (DWORD)-1;
    m_infoVideo.dwSampleSize = 0;
	SetRect(&m_infoVideo.rcFrame, 0, 0, m_w, m_h);
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
    EnterCriticalSection(&m_lock);

    Stop();

    if (m_bSetOpts && m_optsVideo.lpParms && m_optsVideo.cbParms)
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
        *w = m_pTimeShift->m_w;
        *h = m_pTimeShift->m_h;
        return true;
    }

    return false;
}

bool TimeShift::OnSetWaveInDevice(int index)
{
    return m_pTimeShift ? m_pTimeShift->SetWaveInDevice(index) : false;
}

bool TimeShift::OnGetWaveInDevice(int *index)
{
    if (m_pTimeShift && index)
    {
        *index = m_pTimeShift->m_waveInDevice;
        return true;
    }

    return false;
}

bool TimeShift::OnSetWaveOutDevice(int index)
{
    return m_pTimeShift ? m_pTimeShift->SetWaveOutDevice(index) : false;
}

bool TimeShift::OnGetWaveOutDevice(int *index)
{
    if (m_pTimeShift && index)
    {
        *index = m_pTimeShift->m_waveOutDevice;
        return true;
    }

    return false;
}

bool TimeShift::OnCompressionOptions(void)
{
    return m_pTimeShift ? m_pTimeShift->CompressionOptions() : false;
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

bool TimeShift::StartRecord(void)
{
    // Clear all variables.
    Stop();

    // Update the size in case pixel width has changed.
    SetDimensions();

    m_bits = new BYTE[m_pitch * m_h];

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

    // Open the movie file for writing.
    if (AVIFileOpen(&m_pfile, fname, OF_WRITE | OF_CREATE, NULL) != 0)
    {
        Stop();
        return false;
    }

    // Create the video stream.
    AVIFileCreateStream(m_pfile, &m_psVideo, &m_infoVideo);
    if (m_bSetOpts)
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
    // NOTE: Add space for the color table if we want to save 8-bit avis.
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

    // If this fails, we just won't have any audio recorded, still continue.
    if (waveInOpen(&m_hWaveIn,
                   m_waveInDevice,
                   &m_waveFormat,
                   (DWORD)WaveInProc,
                   0,
                   CALLBACK_FUNCTION) == MMSYSERR_NOERROR)
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

    m_mode = MODE_RECORDING;

    if (m_hWaveIn)
        waveInStart(m_hWaveIn);

    return true;
}

bool TimeShift::StartPlay(void)
{
    // Clear all variables.
    Stop();

    // Mute the current live feed, we're going to playback our own audio.
    Audio_Mute();

    // Find the first avi available.
    char fname[MAX_PATH];
    int curFile = m_curFile;
    do {
        sprintf(fname, "ds%.3u.avi", curFile);
        if (GetFileAttributes(fname) != 0xffffffff)
            break;
        if (++curFile == 1000)
            curFile = 0;
    } while (curFile != m_curFile); // Bail if we've looped all the way around.
    m_curFile = curFile;

    // Open the movie file for writing.
    if (AVIFileOpen(&m_pfile, fname, OF_READ, NULL) != 0)
    {
        Stop();
        return false;
    }

    if (AVIFileGetStream(m_pfile, &m_psCompressedVideo, 0, 0) != 0 ||
        AVIFileGetStream(m_pfile, &m_psCompressedAudio, 0, 1) != 0)
    {
        Stop();
        return false;
    }

    if (AVIStreamInfo(m_psCompressedVideo, &m_infoVideo, sizeof(m_infoVideo))
        != 0 ||
        AVIStreamInfo(m_psCompressedAudio, &m_infoAudio, sizeof(m_infoAudio))
        != 0)
    {
        Stop();
        return false;
    }

    if ((m_pGetFrame = AVIStreamGetFrameOpen(m_psCompressedVideo, NULL)) ==NULL)
    {
        Stop();
        return false;
    }

    // If this fails, we just won't have audio playback.
    // FIXME: Need to make sure our m_waveFormat jives with the new m_infoAudio.
    if (waveOutOpen(&m_hWaveOut,
                    m_waveOutDevice,
                    &m_waveFormat,
                    (DWORD)WaveOutProc,
                    0,
                    CALLBACK_FUNCTION | WAVE_ALLOWSYNC) == MMSYSERR_NOERROR)
    {
        // Start sending audio data to the waveOut device.
        // Call twice to keep two buffers going at all times.
        ReadAudio();
        ReadAudio();
    }

    m_mode = MODE_PLAYING;

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
        AVIStreamClose(m_psCompressedVideo);

    if (m_psVideo)
        AVIStreamClose(m_psVideo);

    if (m_psCompressedAudio)
        AVIStreamClose(m_psCompressedAudio);

    if (m_psAudio)
        AVIStreamClose(m_psAudio);

    if (m_pfile)
        AVIFileClose(m_pfile);

    if (m_pGetFrame)
        AVIStreamGetFrameClose(m_pGetFrame);

    if (m_bits)
        delete m_bits;

    m_bits = NULL;

    m_pfile = NULL;
    m_psVideo = NULL;
    m_psCompressedVideo = NULL;
    m_psAudio = NULL;
    m_psCompressedAudio = NULL;
    m_pGetFrame = NULL;

    m_startTime = 0;
    m_thisTime = 0;
    m_lastTime = 0;
    m_nextSample = 0;

    m_hWaveIn = 0;
    m_nextWaveInHdr = 0;
    m_hWaveOut = 0;
    m_nextWaveOutHdr = 0;

    // Stop mode, allow live feed in again.
    Audio_Unmute();

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

    return StartPlay();
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

    return StartPlay();
}

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

inline LPBYTE mmxYUVtoRGB(LPBYTE dest, short *src, DWORD w)
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
        movq mm1, [esi] ; mm1 = Vb+128 Y1b Ub+128 Y0b  Va+128 Y1a Ua+128 Y0a
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

inline LPBYTE mmxRGBtoYUV(short *dest, LPBYTE src, DWORD w)
{
#if 1
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
#else
    LPBYTE dst = (LPBYTE)dest;
    DWORD w2 = w >> 1;
    while (w2--)
    {
        int B = *src++;
        int G = *src++;
        int R = *src++;

        int Y = (R * 9798 + G * 19235 + B * 3736) >> 15;
        int U = ((R * -5505 + G * -10879 + B * 16384) >> 15) + 128;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 :(x))

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
#endif
    return src;
}

bool TimeShift::WriteVideo(DEINTERLACE_INFO *info)
{
    if (!m_psCompressedVideo)
        return false;

    // Nothing to do if we don't have the last 2 source fields.
    if (info->EvenLines[0] == NULL)
        return false;

    m_thisTime = GetTickCount();

    // Make sure everything starts out at zero.
    if (!m_startTime)
        m_lastTime = m_startTime = m_thisTime;

    DWORD thisFrame =
        DWORD((double)m_fps * double(m_thisTime - m_startTime) / 1000.0);

    LPBYTE dest = m_bits;
    DWORD w = min(m_w, info->FrameWidth);
    DWORD h = min(m_h, info->FieldHeight);
    DWORD left = 0;//info->SourceRect.left;
    DWORD top = 0;//info->SourceRect.top;

    for (int y = h - 1; y >= 0; --y)
        dest = mmxYUVtoRGB(dest, info->EvenLines[0][y + top] + left, w);

    if (!info->IsOdd)
        AVIStreamWrite(m_psCompressedVideo,
                       thisFrame,
                       1,
                       m_bits,
                       m_pitch * m_h,
                       AVIIF_KEYFRAME,
                       NULL,
                       NULL);

    m_lastTime = m_thisTime;

    return true;
}

bool TimeShift::ReadVideo(DEINTERLACE_INFO *info)
{
    if (!m_psCompressedVideo)
        return false;

    // Nothing to do if we don't have the last 2 source fields.
    if ((!info->IsOdd && info->EvenLines[0] == NULL) ||
        (info->IsOdd && info->OddLines[0] == NULL))
        return false;

    m_thisTime = GetTickCount();

    // Make sure everything starts out at zero.
    if (!m_startTime)
        m_lastTime = m_startTime = m_thisTime;

    DWORD thisFrame =
        DWORD((double)m_fps * double(m_thisTime - m_startTime) / 1000.0);

    // If we're at the end of this clip, start playing the next one.
    if (thisFrame >= m_infoVideo.dwLength)
    {
        // Just change the mode so no further frames will be read.
        // Can't call Stop from this thread.  Too dangerous.
        m_mode = MODE_STOPPED;

        // We basically want to trigger an OnGoNext() but I don't want to call
        // that from within this thread.
        PostMessage(hWnd, WM_COMMAND, IDM_TSNEXT, 0);

        return false;
    }

    static LPBITMAPINFOHEADER lpbi = NULL;
    if (!info->IsOdd)
        lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(m_pGetFrame, thisFrame);

    if (lpbi)
    {
        LPBYTE src = LPBYTE(lpbi) + sizeof(*lpbi);
        DWORD w = min(lpbi->biWidth, info->FrameWidth);
        DWORD h = min(lpbi->biHeight, info->FieldHeight);
        DWORD left = 0;
        DWORD top = 0;

        if (info->IsOdd)
            for (int y = h - 1; y >= 0; --y)
                src = mmxRGBtoYUV(info->OddLines[0][y + top] + left, src, w);
        else
            for (int y = h - 1; y >= 0; --y)
                src = mmxRGBtoYUV(info->EvenLines[0][y + top] + left, src, w);
    }

    m_lastTime = m_thisTime;

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
                   m_nextSample,
                   numSamples,
                   m_waveInBufs[m_nextWaveInHdr],
                   count,
                   AVIIF_KEYFRAME,
                   &bytesWritten,
                   &samplesWritten);

    // Re-add this buffer to the waveIn queue.
    waveInAddBuffer(m_hWaveIn, m_waveInHdrs + m_nextWaveInHdr, sizeof(WAVEHDR));

    ++m_nextWaveInHdr %= sizeof(m_waveInHdrs)/sizeof(*m_waveInHdrs);
    m_nextSample += numSamples;

    return true;
}

bool TimeShift::ReadAudio(void)
{
    if (!m_psCompressedAudio)
        return false;

    DWORD count = sizeof(*m_waveOutBufs);

    DWORD numSamples = count / m_infoAudio.dwSampleSize;
    long bytesRead = 0;
    long samplesRead = 0;

    // Even if this fails, we still need to send _something_ to waveOut device.
    // Otherwise, we'd never get notification of its completion and the the
    // audio rendering cycle would stop.
    AVIStreamRead(m_psCompressedAudio,
                  m_nextSample,
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
    m_nextSample += numSamples;

    return true;
}

bool TimeShift::SetDimensions(void)
{
    if (m_mode == MODE_STOPPED)
    {
        // Use current pixel width and field heights to determine our AVI size.
        int w = CurrentX;
        int h = CurrentY;

        // 4-pixel (12-byte) align the width, truncating, for mmx operations.
        m_w = (w >> 2) << 2;
        m_h = h >> 1; // For speed (for now), we use 1/2 height AVIs.

        // Guaranteed to be QWORD-aligned since the width is.
        m_pitch = m_w * (m_bpp >> 3);

        // Reset all the bitmapinfo stuff.
        memset(&m_bih, 0, sizeof(m_bih));
        m_bih.biSize = sizeof(m_bih);
        m_bih.biWidth = m_w;
        m_bih.biHeight = m_h;
        m_bih.biPlanes = 1;
        m_bih.biBitCount = m_bpp;
        m_bih.biCompression = BI_RGB; // The codec will compress our RGB data.
        m_bih.biSizeImage = m_pitch * m_h;

        m_infoVideo.dwSuggestedBufferSize = m_pitch * m_h;
        SetRect(&m_infoVideo.rcFrame, 0, 0, m_w, m_h);

        return true;
    }

    return false;
}

bool TimeShift::SetWaveInDevice(int index)
{
    if (m_mode == MODE_STOPPED)
    {
        m_waveInDevice = index;

        return true;
    }

    return false;
}

bool TimeShift::SetWaveOutDevice(int index)
{
    if (m_mode == MODE_STOPPED)
    {
        m_waveOutDevice = index;

        return true;
    }

    return false;
}

bool TimeShift::CompressionOptions(void)
{
    bool result = false;

    // Must be stopped before attempting to bring up compression options dialog.
    ASSERT(m_mode == MODE_STOPPED);

    // Update these settings for the options dialog to display.
    SetDimensions();

    char fname[20] = "dstemp.avi";
    DeleteFile(fname);

	PAVIFILE pfile;
    AVIFileOpen(&pfile, fname, OF_WRITE | OF_CREATE, NULL);

	// Create the video stream and set its format.
	PAVISTREAM psVideo = NULL;
    AVIFileCreateStream(pfile, &psVideo, &m_infoVideo);

    // NOTE: Add space for the color table if we want to save 8-bit avis.
    // Also, for the options dialog, set the info header's compression so it'll
    // show up under "Current Format:", then set it back to BI_RGB for the
    // actual recording process.
    m_bih.biCompression =
        m_infoVideo.fccHandler ? m_infoVideo.fccHandler : BI_RGB;
    AVIStreamSetFormat(psVideo, 0, &m_bih, sizeof(m_bih));
    m_bih.biCompression = BI_RGB;

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
        AVIStreamClose(psAudio);
    if (psVideo)
        AVIStreamClose(psVideo);
    if (pfile)
        AVIFileClose(pfile);

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

    if (m_bSetOpts && m_optsVideo.lpParms && m_optsVideo.cbParms)
        delete m_optsVideo.lpParms;
    m_optsVideo = *opts;
    if (opts->lpParms && opts->cbParms)
    {
        m_optsVideo.lpParms = new BYTE[opts->cbParms];
        memcpy(m_optsVideo.lpParms, opts->lpParms, opts->cbParms);
    }

    m_bSetOpts = true;

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

    m_waveInDevice = GetPrivateProfileInt(
        "TimeShift", "WaveIn", m_waveInDevice, szIniFile);

    m_waveOutDevice = GetPrivateProfileInt(
        "TimeShift", "WaveOut", m_waveOutDevice, szIniFile);

    return true;
}

bool TimeShift::WriteToIni(void)
{
    extern char szIniFile[MAX_PATH];
    char temp[1000];

    if (m_bSetOpts)
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

    sprintf(temp, "%u", m_waveInDevice);
    WritePrivateProfileString("TimeShift", "WaveIn", temp, szIniFile);

    sprintf(temp, "%u", m_waveOutDevice);
    WritePrivateProfileString("TimeShift", "WaveOut", temp, szIniFile);

    return true;
}
