////////////////////////////////////////////////////////////////////////////
// $Id: TimeShift.cpp,v 1.34 2004-08-12 16:27:47 adcockj Exp $
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
#include "MixerDev.h"     // Mute and UnMute
#include "Settings.h"     // Setting_Set/GetValue
#include "Cpu.h"          // CpuFeatureFlags
#include "Providers.h"    // Providers_GetCurrentSource
#include "DebugLog.h"     // LOG

#include "SchedMessageBox.h" // For schedule timer notification

bool nofreespace = false;   // flag - no free disk space 
long EndRecordTime = 0;     // stores length of time to record (user input)
long sleeptime = 0;         // time to sleep before a scheduled recording (seconds)

bool RecordTimerCheckedF = false;  // Flag for a timed recording check
bool RecordTimerF = false;         // Flag for a timed recording
bool ScheduleF = false;            // Flag for a scheduled recording
bool TimedRecodingDone = false;    // Flag for a timed recording done
bool CancelSched = false;          // Flag to cancel scheduled recording

int m_Sync = 0; // holds custom AV sync setting from INI file
int m_Warn = 0; // holds flag for timeshift warnings (0 = enable / 1 = disable)

int m_Start = 0; // holds schedule start time from INI file
int m_Time = 0;  // holds length of time to record (minutes)

static BOOL WarningShown = FALSE;
static DWORD AVIFileSizeLimit = 0;
static DWORD AVIFileSizeLimitTemp = 0;

static char ExePath[MAX_PATH] = {0};
static char* SavingPath = NULL;

//free disk space routine - code based on MSDN example (but better)
ULONGLONG GetFreeDiskSpace() 
{

/////////////////////////////////////////////////////////
// NOTE:  This function returns an unsigned __int64    //
//        (i.e. an unsigned 64 bit integer) as the     //
//        amount of free space available to the caller //
//        for the volume specified in lpszDrivePath in // 
//        bytes.                                       //
/////////////////////////////////////////////////////////

    ULONGLONG totalbytes = 0;   // free disk space in bytes

    typedef BOOL (CALLBACK * DSKFREE)(LPCTSTR,
                                      PULARGE_INTEGER,
                                      PULARGE_INTEGER,
                                      PULARGE_INTEGER);
    OSVERSIONINFO VersionInfo;
    VersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    CString strProcName, strErrMsg;
    char lpszDrivePath[4] = "c:\\";

    lpszDrivePath[0] = SavingPath[0];

    // Get Operating System Version information
    if(!GetVersionEx(&VersionInfo))
        return totalbytes;

    switch ( VersionInfo.dwPlatformId )
    {
    case VER_PLATFORM_WIN32_NT  :
        if(VersionInfo.dwMajorVersion < 4)  // NT 3.51 or less
            break;
    case VER_PLATFORM_WIN32_WINDOWS :       // Windows 95 or higher
        if ( LOWORD(VersionInfo.dwBuildNumber) <= 1000 )
            break;                          // Non-Windows 95 OSR2

        // This code if for Windows 95 OSR2 or higher and
        // Windows NT 4.0 or higher and Windows XP
        HINSTANCE hinstLib;
        DSKFREE ProcAdd;
        ULARGE_INTEGER FreeBytesAvailableToCaller,
                       TotalNumberOfBytes,
                       TotalNumberOfFreeBytes;
        // Load Kernel32.dll
        hinstLib = LoadLibrary(_T("Kernel32.dll"));
        if ( hinstLib == NULL )
        {
            ::MessageBox( NULL,
                          _T("Could not load Kernel32.dll"),
                          NULL,
                          MB_OK | MB_ICONERROR | MB_TASKMODAL
                         );
            return totalbytes;
        }

        #ifdef _UNICODE   // Unicode version of GetDiskFreeSpaceEx
            strProcName = _T("GetDiskFreeSpaceExW");

        #else             // Standard version of GetDiskFreeSpaceEx
            strProcName = _T("GetDiskFreeSpaceExA");
        #endif

        // Get ProcAddress for GetDiskFreeSpaceEx function
        ProcAdd = (DSKFREE) GetProcAddress(hinstLib, strProcName);

        if ( ProcAdd == NULL )
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
        ProcAdd(lpszDrivePath,
                &FreeBytesAvailableToCaller,
                &TotalNumberOfBytes,
                &TotalNumberOfFreeBytes);

        totalbytes = FreeBytesAvailableToCaller.QuadPart;
        FreeLibrary(hinstLib);

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
    totalbytes =
        lpNumberOfFreeClusters*lpBytesPerSector*lpSectorsPerCluster;

    return totalbytes;
}


bool CTimeShift::OnDestroy(void)
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

        // If less than 300 MB free disk space don't even start recoding.
        ULONGLONG TotalFreeBytes = GetFreeDiskSpace();
        if (TotalFreeBytes < 314572800)
        {
            LeaveCriticalSection(&m_pTimeShift->m_lock); //unlock the stream
            MessageBox(hWnd,
                        "Sorry! You do not have enough\n"
                        "disk space to record a video.",
                        "Information",
                        MB_ICONEXCLAMATION | MB_OK);        
            return result; //exit TimeShift
        }

        // make sure the last file written in the series of file splits 
        // leaves at least 250MB free for the system (the means changing
        // AVIFileSizeLimitTemp to a temporary value for last file.
        //
        // Need to use a temp for AVIFileSizeLimit to stop INI overwrite
        AVIFileSizeLimitTemp = AVIFileSizeLimit; // Initialize the temp
        if (TotalFreeBytes < (262144000 + (AVIFileSizeLimit*1024*1024)))
            AVIFileSizeLimitTemp = ((TotalFreeBytes - 262144000)/(1024*1024));
        
        // Only start recording if we're stopped.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ?
            m_pTimeShift->Record(false) : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool CTimeShift::OnPause(void)
{
    AssureCreated();

    bool result = false;

    // if we've not been created there is nothing to pause
    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Save this off before we start "time shifing".
        if (m_pTimeShift->m_mode == MODE_STOPPED)
        {
            CSource* pSource = Providers_GetCurrentSource();
            if (pSource != NULL)
            {
                m_pTimeShift->m_origPixelWidth = pSource->GetWidth();
            }
        }

        // Only start "time shifing" if we're stopped.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ||
            m_pTimeShift->m_mode == MODE_RECORDING ||
            m_pTimeShift->m_mode == MODE_SHIFTING ?
            m_pTimeShift->Record(true) : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool CTimeShift::OnPlay(void)
{
    AssureCreated();

    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        // Save this off before we start playing.
        if (m_pTimeShift->m_mode == MODE_STOPPED)
        {
            CSource* pSource = Providers_GetCurrentSource();
            if (pSource != NULL)
            {
                m_pTimeShift->m_origPixelWidth = pSource->GetWidth();
            }
        }

        // Only start playing if we're stopped.
        result =
            m_pTimeShift->m_mode == MODE_STOPPED ||
            m_pTimeShift->m_mode == MODE_PAUSED ?
            m_pTimeShift->Play() : false;

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

bool CTimeShift::OnStop(void)
{
    // Initialise the flags for scheduled / timed recording.
    //
    // Need to so this in case a timed or scheduled recording
    // was stopped manually by the user.
    ScheduleF = false;
    TimedRecodingDone = false;
    RecordTimerCheckedF = false;
    RecordTimerF = false;

    bool result = false;

    // if we've not been created there is nothing to stop
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
        CSource* pSource = Providers_GetCurrentSource();
        if (result && pSource != NULL && pSource->GetWidth() != m_pTimeShift->m_origPixelWidth)
        {
            SetPixelWidth(m_pTimeShift->m_origPixelWidth);
        }

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }
    return result;
}

bool CTimeShift::OnGoNext(void)
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

bool CTimeShift::OnGoPrev(void)
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

bool CTimeShift::OnOptions(void)
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

bool CTimeShift::WorkOnInputFrames()
{
    bool result = true;

    if (m_pTimeShift)
    {
        if (m_pTimeShift->m_recHeight == TS_FULLHEIGHT)
        {
            result = false;
        }
    }

    return result;
}

bool CTimeShift::IsRunning()
{
    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        if (m_pTimeShift->m_mode != MODE_STOPPED)
        {
            result = true;
        }

        LeaveCriticalSection(&m_pTimeShift->m_lock);
    }

    return result;
}

// Called from the capture thread.
bool CTimeShift::OnNewInputFrame(TDeinterlaceInfo *pInfo)
{
    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        if(m_pTimeShift->m_mode == MODE_RECORDING ||
            m_pTimeShift->m_mode == MODE_PAUSED ||
            m_pTimeShift->m_mode == MODE_SHIFTING)
        {
            result = m_pTimeShift->WriteVideo(pInfo);
        }
        else if(m_pTimeShift->m_mode == MODE_PLAYING)
        {
            result = m_pTimeShift->ReadVideo(pInfo);
        }

        LeaveCriticalSection(&m_pTimeShift->m_lock);

        // It would be a good idea to fix the file size limitation. Until then we split the AVI.
        if(m_pTimeShift->m_BytesWritten > AVIFileSizeLimit*1024*1024
            || m_pTimeShift->m_BytesWritten > AVIFileSizeLimitTemp*1024*1024)
        {
            m_pTimeShift->OnStop();
            

            // Check free disk space while file splitting.
            // If less than 300 MB free disk space don't start a new file. 
            if (GetFreeDiskSpace() < 314572800)
                nofreespace = true; //this flag stops a new file being created
            m_pTimeShift->OnRecord();
    }

        // Timed recording check for completed
        if (RecordTimerF) // is it a timed recording?
        {
            time_t seconds;
            seconds = time (NULL);
            if (seconds >= EndRecordTime)
            {   m_pTimeShift->OnStop();   // Close the timed video recoding file    
                TimedRecodingDone = true; // Set the flag for timed recording done              
                m_pTimeShift->OnRecord(); // Message and close down
            }
        }
    }
    return result;
}

// Called from the capture thread.
bool CTimeShift::OnNewOutputFrame(TDeinterlaceInfo *pInfo)
{
    bool result = false;

    if (m_pTimeShift)
    {
        EnterCriticalSection(&m_pTimeShift->m_lock);

        if(m_pTimeShift->m_mode == MODE_RECORDING ||
            m_pTimeShift->m_mode == MODE_PAUSED ||
            m_pTimeShift->m_mode == MODE_SHIFTING)
        {
            result = m_pTimeShift->WriteVideo2(pInfo);
        }
        else if(m_pTimeShift->m_mode == MODE_PLAYING)
        {
            result = m_pTimeShift->ReadVideo2(pInfo);
        }

        LeaveCriticalSection(&m_pTimeShift->m_lock);

        // It would be a good idea to fix the file size limitation. Until then we split the AVI.
        if(m_pTimeShift->m_BytesWritten > AVIFileSizeLimit*1024*1024
            || m_pTimeShift->m_BytesWritten > AVIFileSizeLimitTemp*1024*1024)
        {
            m_pTimeShift->OnStop();
        
            // Check free disk space while file splitting.
            // ===========================================
            // Get the root directory of drive for recorded video files
            // If less than 300 MB free disk space don't start a new file. 
            if (GetFreeDiskSpace() < 314572800)
                nofreespace = true; //this flag stops a new file being created
            m_pTimeShift->OnRecord();
    }

        // Timed recording check for completed
        if (RecordTimerF) // is it a timed recording?
        {
            time_t seconds;
            seconds = time (NULL);
            if (seconds >= EndRecordTime)
            {   m_pTimeShift->OnStop();   // Close the timed video recoding file    
                TimedRecodingDone = true; // Set the flag for timed recording done              
                m_pTimeShift->OnRecord(); // Message and close down
            }
        }
    }
    return result;
}

bool CTimeShift::OnWaveInData(void)
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

bool CTimeShift::OnWaveOutDone(void)
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

bool CTimeShift::OnSetMenu(HMENU hMenu)
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

CTimeShift* CTimeShift::m_pTimeShift = NULL;
LPBYTE (*CTimeShift::m_YUVtoRGB)(LPBYTE,short *,DWORD) = NULL;
LPBYTE (*CTimeShift::m_AvgYUVtoRGB)(LPBYTE,short *,short *,DWORD) = NULL;
LPBYTE (*CTimeShift::m_RGBtoYUV)(short *,LPBYTE,DWORD) = NULL;

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

// YUV to RGB if processor is P3 or better - assembly code
// This routine does the conversion for video file data
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
        emms
    }

    return dest;
}

// The c-equivalent if P3-or-better is not present in cpu flags.
// YUV to RGB if processor is NOT P3 or better - C++ code
// This routine does the conversion for video file data
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
// Emu: changed 'mov esp, src2' to ecx 'mov ecx, src2' and it seems to work OK.
// Apparently 'esp' is a stack pointer and that was causing the crash.
//
// YUV to RGB if processor is P3 or better - assembly code
// This routine does the conversion for video file data.
// Each line is built from an average of ODD and EVEN pixels.
LPBYTE P3_AvgYUVtoRGB(LPBYTE dest, short *src1, short *src2, DWORD w)
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

// The c-equivalent if P3-or-better is not present in cpu flags.
//
// YUV to RGB if processor is NOT P3 or better - C++ code.
// This routine does the conversion for video file data.
// Each line is built from an average of ODD and EVEN pixels.
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
        emms
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

CTimeShift::CTimeShift()
    :
    m_mode(MODE_STOPPED),
    m_fps(30),
    m_curFile(0),
    m_lpbi(NULL),
    m_recordBits(NULL),
    m_playBits(NULL),
    m_gotPauseBits(FALSE),
    m_BytesWritten(0),
    m_pfile(NULL),
    m_psVideo(NULL),
    m_psAudio(NULL),
    m_psCompressedVideo(NULL),
    m_psCompressedAudio(NULL),
    m_psCompressedVideoP(NULL),
    m_psCompressedAudioP(NULL),
    m_pGetFrame(NULL),
    m_setOptsVideo(false),
    m_setOptsAudio(false),
    m_startTimeRecord(0),
    m_startTimePlay(0),
    m_thisTimeRecord(0),
    m_thisTimePlay(0),
    m_lastFrameRecord(0),
    m_lastFramePlay(0),
    m_startFramePlay(0),
    m_nextSampleRecord(0),
    m_nextSamplePlay(0),
    m_hWaveIn(NULL),
    m_nextWaveInHdr(0),
    m_hWaveOut(NULL),
    m_nextWaveOutHdr(0),
    m_recHeight(TS_HALFHEIGHTEVEN),
    m_origPixelWidth(720),
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
    // best for me is 176389 (defalt is 176400)
    // an INI file custom setting will change this value
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
    m_infoAudio.dwQuality = 0; // Laurent's comment : why 0 and not -1
    m_infoAudio.dwSampleSize = m_waveFormat.nBlockAlign;
    SetRect(&m_infoAudio.rcFrame, 0, 0, 0, 0);
    m_infoAudio.dwEditCount = 0;
    m_infoAudio.dwFormatChangeCount = 0;
    strcpy(m_infoAudio.szName, "Audio");

    memset(&m_optsVideo, 0, sizeof(m_optsVideo));
    memset(&m_optsAudio, 0, sizeof(m_optsAudio));
    m_optsAudio.cbFormat = sizeof(m_waveFormat);
    m_optsAudio.lpFormat = &m_waveFormat;

    // Overwrite any of the above defaults with whatever's in the ini file.
    ReadFromIni();

    m_pTimeShift = NULL;
}

CTimeShift::~CTimeShift()
{
    // NOTE: This will deadlock if we crash in OnNewFrame.  So, the solution
    // would be to fix any crash bugs and we can leave this lock as is.
    EnterCriticalSection(&m_lock);

    Stop();

    if (m_setOptsVideo && m_optsVideo.lpParms && m_optsVideo.cbParms)
        delete m_optsVideo.lpParms;
    if (m_setOptsAudio && m_optsAudio.lpFormat && m_optsAudio.cbFormat)
        delete m_optsAudio.lpFormat;

    AVIFileExit();

    LeaveCriticalSection(&m_lock);

    DeleteCriticalSection(&m_lock);
}

bool CTimeShift::AssureCreated(void)
{
    if (!m_pTimeShift)
    {
        // a bit messy - but its only a temp solution to stop warnings
        if(WarningShown == FALSE)
        {
            int Result  = MessageBox(
                                        hWnd, 
                                        "TimeShift (only manual recording is implemented at the moment)\n"
                                        "is an experimental/test feature. We have had reports of this\n" 
                                        "feature crashing and causing serious problems including disk\n"
                                        "corruption.\n"
                                        "\n"
                                        "Are you sure you want to continue?\n"
                                        "\n"
                                        "The recommended codec (for speed and quality) is XviD: release\n"
                                        "XviD-04102002-1. You can edit / stitch vids (re 2GB file splits)\n"
                                        "with MS Windows Movie Maker 2 (WMV output) - save  at high\n" 
                                        "bit rate for good results from the MP4 produced by DScaler.",
                                        "TimeShift Warning", 
                                        MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2
                                     );
            if(Result != IDYES)
            {
                return FALSE;
            }
            WarningShown = TRUE;
        }
        m_pTimeShift = new CTimeShift();
    }

    return m_pTimeShift != NULL;
}

bool CTimeShift::OnSetDimensions(void)
{
    return m_pTimeShift ? m_pTimeShift->SetDimensions() : false;
}

bool CTimeShift::OnGetDimenstions(int *w, int *h)
{
    if (m_pTimeShift && w && h)
    {
        *w = m_pTimeShift->m_bih.biWidth;
        *h = m_pTimeShift->m_bih.biHeight;
        return true;
    }

    return false;
}

bool CTimeShift::OnSetWaveInDevice(char *pszDevice)
{
    return m_pTimeShift ? m_pTimeShift->SetWaveInDevice(pszDevice) : false;
}

bool CTimeShift::OnGetWaveInDevice(char **ppszDevice)
{
    if (m_pTimeShift && ppszDevice)
    {
        *ppszDevice = (char*)&m_pTimeShift->m_waveInDevice;
        return true;
    }

    return false;
}

bool CTimeShift::OnSetWaveOutDevice(char *pszDevice)
{
    return m_pTimeShift ? m_pTimeShift->SetWaveOutDevice(pszDevice) : false;
}

bool CTimeShift::OnGetWaveOutDevice(char **ppszDevice)
{
    if (m_pTimeShift && ppszDevice)
    {
        *ppszDevice = (char*)&m_pTimeShift->m_waveOutDevice;
        return true;
    }

    return false;
}

bool CTimeShift::OnSetRecHeight(int index)
{
    return m_pTimeShift ? m_pTimeShift->SetRecHeight(index) : false;
}

bool CTimeShift::OnGetRecHeight(int *index)
{
    if (m_pTimeShift && index)
    {
        *index = m_pTimeShift->m_recHeight;
        return true;
    }

    return false;
}

bool CTimeShift::OnCompressionOptions(void)
{
    return m_pTimeShift ? m_pTimeShift->CompressionOptions() : false;
}

bool CTimeShift::SetPixelWidth(int pixelWidth)
{
    if (m_pTimeShift)
    {
        // Must be called from within one of the critical sections.
        // We have to let the capture thread do its thing to change the pixel
        // width.  Leave the critical section for this call.
        LeaveCriticalSection(&m_pTimeShift->m_lock);
        CSource* pSource = Providers_GetCurrentSource();
        if (pSource != NULL)
        {
            pSource->SetWidth(pixelWidth);
        }
        EnterCriticalSection(&m_pTimeShift->m_lock);
        return true;
    }
    else
    {
        return false;
    }
}

bool CTimeShift::DoMute(bool mute)
{
    if (m_pTimeShift)
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
        CTimeShift::OnWaveInData();
}

static void CALLBACK WaveOutProc(
    HWAVEOUT hwo,      
    UINT uMsg,         
    DWORD dwInstance,  
    DWORD dwParam1,    
    DWORD dwParam2)
{
    if (uMsg == WOM_DONE)
        CTimeShift::OnWaveOutDone();
}

bool CTimeShift::Record(bool pause)
{
    if (pause &&
        (m_mode == MODE_RECORDING ||
         m_mode == MODE_SHIFTING))
    {
        m_mode = MODE_PAUSED;
        m_startFramePlay = m_lastFramePlay;
        m_startTimePlay = 0;
        if (!m_playBits)
            m_playBits = new BYTE[m_bih.biSizeImage];
        DoMute(true);
        return true;
    }

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
        sprintf(fname, "%s\\ds%.3u.avi", SavingPath, curFile++);
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

    eVideoFormat VideoFormat = Providers_GetCurrentSource() ? Providers_GetCurrentSource()->GetFormat() : VIDEOFORMAT_NTSC_M;
    m_fps = (GetTVFormat(VideoFormat)->Is25fps) ? 25 : 30;
    m_infoVideo.dwRate = m_fps;
    UpdateAudioInfo();

    // Create the video stream.
    AVIFileCreateStream(m_pfile, &m_psVideo, &m_infoVideo);
    if (m_setOptsVideo)
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
        AVIStreamSetFormat(m_psCompressedAudio, 0, m_optsAudio.lpFormat,
                           m_optsAudio.cbFormat) != 0)
    {
        Stop();
        return false;
    }
//    AVIFileCreateStream(m_pfile, &m_psAudio, &m_infoAudio);
//    if (m_setOptsAudio)
//    {
//        if (AVIMakeCompressedStream(&m_psCompressedAudio,
//                                    m_psAudio,
//                                    &m_optsAudio,
//                                    NULL) != AVIERR_OK)
//        {
//          LOG(1, "Error AVIMakeCompressedStream");
//            Stop();
//            return false;
//        }
//    }
//    else
//    {
//        m_psCompressedAudio = m_psAudio;
//        m_psAudio = NULL;
//    }
//    if (AVIStreamSetFormat(m_psCompressedAudio, 0, m_optsAudio.lpFormat, m_optsAudio.cbFormat) != 0)
//    {
//      LOG(1, "Error AVIStreamSetFormat");
//        Stop();
//        return false;
//    }

    // Mute the current live feed if we're pausing, waveIn can still hear it.
    if (pause)
    {
        DoMute(true);
    }
    

    int DeviceId;
    if(!GetWaveInDeviceIndex(&DeviceId))
    {
        ;
    }
    else
    {
        MMRESULT rslt;

        // If this fails, we just won't have any audio recorded, still continue.
        // Laurent's comment : it seems that m_waveFormat must be a WAVE PCM format; if not, we get a bad format error
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
            LOG(1, "%s", szErrorMsg);
            /// \todo tell the user, that something wrong
        }
    }


    m_mode = pause ? MODE_PAUSED : MODE_RECORDING;

    if (m_hWaveIn)
        waveInStart(m_hWaveIn);

    return true;
}

bool CTimeShift::Play(void)
{
    if (m_mode != MODE_PAUSED)
    {
        // Clear all variables.
        Stop();
    }

    // Find the first AVI available.
    char fname[MAX_PATH];
    int curFile = m_curFile;
    do 
    {
        sprintf(fname, "%s\\ds%.3u.avi", SavingPath, curFile);
        if (GetFileAttributes(fname) != 0xffffffff)
        {
            break;
        }
        if (++curFile == 1000)
        {
            curFile = 0;
        }
    }
    while (curFile != m_curFile); // Bail if looped all the way around.
    m_curFile = curFile;

    // Open the movie file for writing.
    if (!m_pfile && AVIFileOpen(&m_pfile, fname,
                                OF_READWRITE | OF_SHARE_DENY_NONE,
                                NULL) != 0)
    {
        if (m_mode != MODE_PAUSED)
        {
            Stop();
        }
        return false;
    }

    if (!m_psCompressedVideoP)
    {
        if (AVIFileGetStream(m_pfile, &m_psCompressedVideoP, streamtypeVIDEO, 0) != 0)
        {
            if (m_mode != MODE_PAUSED)
            {
                Stop();
            }
            return false;
        }
    }
    if (!m_psCompressedAudioP)
    {
        if (AVIFileGetStream(m_pfile, &m_psCompressedAudioP, streamtypeAUDIO, 0) != 0)
        {
            if (m_mode != MODE_PAUSED)
            {
                Stop();
            }
            return false;
        }
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
            {
                Stop();
            }
            return false;
        }
        
        m_fps = m_infoVideo.dwRate;

        // Make sure the pixel width is ready for the incoming AVI's width.
        CSource* pSource = Providers_GetCurrentSource();
        if (pSource != NULL && pSource->GetWidth() != m_infoVideo.rcFrame.right)
        {

            SetPixelWidth(m_infoVideo.rcFrame.right);
        }

        // Update the size for current pixel width.
        SetDimensions();

        // Mute the current live feed, we're going to playback our own audio.
        DoMute(true);
    }

    if (m_pGetFrame == NULL)
    {
        m_pGetFrame = AVIStreamGetFrameOpen(m_psCompressedVideoP, NULL);
        if (m_pGetFrame == NULL)
        {
            if (m_mode != MODE_PAUSED)
            {
                Stop();
            }
            return false;
        }
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
        /// \todo FIXME: Need to make sure our m_waveFormat jives with the new m_infoAudio.
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
            LOG(1, "%s", szErrorMsg);
            /// \todo tell the user, that something wrong
        }
    }

    m_mode = m_mode == MODE_PAUSED ? MODE_SHIFTING : MODE_PLAYING;

    return true;
}

bool CTimeShift::Stop(void)
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
        {
            waveInUnprepareHeader(m_hWaveIn,
                                  m_waveInHdrs + i,
                                  sizeof(WAVEHDR));
        }
    }

    if (m_hWaveOut)
    {
        LeaveCriticalSection(&m_lock);
        waveOutReset(m_hWaveOut);
        EnterCriticalSection(&m_lock);

        waveOutClose(m_hWaveOut);

        // The audio device is finished with the header, unprepare it.
        for (int i = 0; i < sizeof(m_waveOutHdrs)/sizeof(*m_waveOutHdrs); ++i)
        {
            // Unpreparing a header that wasn't prepared does nothing.
            waveOutUnprepareHeader(m_hWaveOut,
                                   m_waveOutHdrs + i,
                                   sizeof(WAVEHDR));
        }
    }

    if (m_pGetFrame)
    {
        LOG(1, "AVIStreamGetFrameClose m_pGetFrame %d",
        AVIStreamGetFrameClose(m_pGetFrame)
        );
    }

    if (m_psCompressedVideo)
    {
        LOG(1, "AVIStreamRelease m_psCompressedVideo %d",
        AVIStreamRelease(m_psCompressedVideo)
        );
    }

    if (m_psCompressedVideoP)
    {
        LOG(1, "AVIStreamRelease m_psCompressedVideoP %d",
        AVIStreamRelease(m_psCompressedVideoP)
        );
    }

    if (m_psVideo)
    {
        LOG(1, "AVIStreamRelease m_psVideo %d",
        AVIStreamRelease(m_psVideo)
        );
    }

    if (m_psCompressedAudio)
    {
        LOG(1, "AVIStreamRelease m_psCompressedAudio %d",
        AVIStreamRelease(m_psCompressedAudio)
        );
    }

    if (m_psCompressedAudioP)
    {
        LOG(1, "AVIStreamRelease m_psCompressedAudioP %d",
        AVIStreamRelease(m_psCompressedAudioP)
        );
    }

    if (m_psAudio)
    {
        LOG(1, "AVIStreamRelease m_psAudio %d",
        AVIStreamRelease(m_psAudio)
        );
    }

    if (m_pfile)
    {
        LOG(1, "AVIFileRelease m_pfile %d",
        AVIFileRelease(m_pfile)
        );
    }

    if (m_recordBits)
    {
        delete m_recordBits;
    }

    if (m_playBits)
    {
        delete m_playBits;
    }

    m_lpbi = NULL;
    m_recordBits = NULL;
    m_playBits = NULL;
    m_gotPauseBits = FALSE;
    m_BytesWritten = 0;

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
    m_lastFrameRecord = 0;
    m_lastFramePlay = 0;
    m_startFramePlay = 0;
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

bool CTimeShift::GoNext(void)
{
    int curFile = m_curFile;

    do 
    {
        if (++curFile == 1000)
        {
            curFile = 0;
        }
        char fname[MAX_PATH];
        sprintf(fname, "%s\\ds%.3u.avi", SavingPath, curFile);
        if (GetFileAttributes(fname) != 0xffffffff)
        {
            break;
        }
    }
    while (curFile != m_curFile); // Bail if we've looped all the way around.

    m_curFile = curFile;

    return Play();
}

bool CTimeShift::GoPrev(void)
{
    int curFile = m_curFile;

    do 
    {
        if (--curFile < 0)
        {
            curFile = 999;
        }
        char fname[MAX_PATH];
        sprintf(fname, "%s\\ds%.3u.avi", SavingPath, curFile);
        if (GetFileAttributes(fname) != 0xffffffff)
        {
            break;
        }
    } 
    while (curFile != m_curFile); // Bail if we've looped all the way around.

    m_curFile = curFile;

    return Play();
}

bool CTimeShift::WriteVideo(TDeinterlaceInfo* pInfo)
{
    // Basic tests to see if we can proceed
    if ((m_recHeight == TS_FULLHEIGHT) || (!m_psCompressedVideo) || (pInfo->PictureHistory[0] == NULL))
    {
        return false;
    }

    // Sets recHeight to TS_HALFHEIGHTEVEN whatever the user chosen m_recHeight is, if
    // mode is paused or mode is shifting - need to so this to stop overloading system.
    int recHeight = (m_mode == MODE_PAUSED || m_mode == MODE_SHIFTING) ? TS_HALFHEIGHTEVEN : m_recHeight;

    BOOL DataOkForPause = false;

    // Recording height tests
    if( ((recHeight == TS_HALFHEIGHTEVEN) && (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN))
     || ((recHeight == TS_HALFHEIGHTODD)  && (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD ))
     || ((recHeight == TS_HALFHEIGHTAVG)  && (pInfo->PictureHistory[1] != NULL) && (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN)))
    {
        m_thisTimeRecord = GetTickCount();

        // Make sure everything starts out at zero.
        if (!m_startTimeRecord)
        {
            m_startTimeRecord = m_thisTimeRecord;
        }

        DWORD thisFrame =
            DWORD((double)m_fps * double(m_thisTimeRecord - m_startTimeRecord)
                  / 1000.0 + 0.5);

        if (m_lastFrameRecord == 0 || m_lastFrameRecord != thisFrame)
        {
            LPBYTE dest = m_recordBits;
            DWORD frameWidth = (pInfo->FrameWidth >> 2) << 2;
            DWORD w = min(m_bih.biWidth, frameWidth);
            DWORD h = min(m_bih.biHeight, pInfo->FieldHeight);
            DWORD more = (m_bih.biBitCount >> 3) * (m_bih.biWidth - w);
            int y = h - 1;

            // IMPORTANT: In some of the following cases, we're using the previous
            // even field data, and presumably, since we only handle these cases
            // when we receive odd field data, then the even data must've been
            // received 1/60 of a second ago, and therefore is valid for us to use.
            // Otherwise, crash!

            switch (recHeight)
            {
            case TS_HALFHEIGHTAVG:
                for (; y >= 0; --y)
                {
                    BYTE* EvenLine = pInfo->PictureHistory[0]->pData + y * pInfo->InputPitch;
                    BYTE* OddLine = pInfo->PictureHistory[1]->pData + y * pInfo->InputPitch;
                    // Note: m_AvgYUVtoRGB creates just one line from an average of ODD and EVEN line pixels 
                    dest = m_AvgYUVtoRGB(dest,
                                         (SHORT*)EvenLine,
                                         (SHORT*)OddLine,
                                      w) + more;
                }
                DataOkForPause = true;
                break;

            case TS_HALFHEIGHTEVEN:
                for (; y >= 0; --y)
                {
                    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData + y * pInfo->InputPitch;
                    dest = m_YUVtoRGB(dest,
                                      (SHORT*)CurrentLine,
                                      w) + more;
                }
                DataOkForPause = true;
                break;

            case TS_HALFHEIGHTODD:
                for (; y >= 0; --y)
                {
                    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData + y * pInfo->InputPitch;
                    dest = m_YUVtoRGB(dest,
                                      (SHORT*)CurrentLine,
                                         w) + more;
                }
                DataOkForPause = true;
                break;

            default:
                break;
            }

            MyAVIStreamWrite(m_psCompressedVideo,
                           thisFrame,
                           1,
                           m_recordBits,
                           m_bih.biSizeImage,
                           AVIIF_KEYFRAME);

            // Even if we failed to write to the stream, we'll leave space.
            m_infoVideo.dwLength = thisFrame + 1;
            m_lastFrameRecord = thisFrame;
        }
    }

    switch (m_mode)
    {
    case MODE_PAUSED:
        //** \todo Make pause bits YUY2 so we don't have to convert every time.
        //         Use pInfo->pMemcpy() here, but it crashes. ???
        if (!m_gotPauseBits && DataOkForPause)
        {
            memcpy(m_playBits, m_recordBits, m_bih.biSizeImage);
            m_gotPauseBits = true;
        }

        if (m_gotPauseBits)
        {
            LPBYTE src = m_playBits;
            DWORD frameWidth = (pInfo->FrameWidth >> 2) << 2;
            DWORD w = min(m_bih.biWidth, frameWidth);
            DWORD h = min(m_bih.biHeight, pInfo->FieldHeight);
            DWORD more = (m_bih.biBitCount >> 3) * (m_bih.biWidth - w);

            for (int y = h - 1; y >= 0; --y)
            {
                BYTE* CurrentLine = pInfo->PictureHistory[0]->pData + y * pInfo->InputPitch;
                src = m_RGBtoYUV((SHORT*)CurrentLine, src, w) + more;
            }
        }
        break;

    case MODE_SHIFTING:
        ReadVideo(pInfo);
        break;
    }

    return true;
}

bool CTimeShift::ReadVideo(TDeinterlaceInfo *pInfo)
{
    if (m_recHeight == TS_FULLHEIGHT)
    {
        return false;
    }

    if (!m_psCompressedVideoP)
    {
        return false;
    }

    if (pInfo->PictureHistory[0] == NULL)
    {
        return false;
    }

    m_thisTimePlay = GetTickCount();

    // Make sure everything starts out at zero.
    if (!m_startTimePlay)
    {
        m_startTimePlay = m_thisTimePlay;
    }

    DWORD thisFrame = m_startFramePlay + 
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

    if (m_pGetFrame)
    {
        // Check that the current frame is not empty
        // If empty, find the nearest one that is not empty
        if (m_lastFramePlay == 0 || m_lastFramePlay != thisFrame)
        {
            DWORD nonEmptyFrame = AVIStreamNearestSample(m_psCompressedVideoP, thisFrame);
            if (nonEmptyFrame != thisFrame)
            {
                LOG(2, "thisFrame %d, nonEmptyFrame %d", thisFrame, nonEmptyFrame);
            }
            if (nonEmptyFrame != -1)
            {
                m_lastFramePlay = thisFrame;
                m_lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(m_pGetFrame, nonEmptyFrame);
            }
        }
    }
    else
    {
        m_lpbi = NULL;
    }

    if (m_lpbi)
    {
        LPBYTE src = LPBYTE(m_lpbi) + m_lpbi->biSize;
        DWORD frameWidth = (pInfo->FrameWidth >> 2) << 2;
        DWORD w = min(m_lpbi->biWidth, frameWidth);
        DWORD h = min(m_lpbi->biHeight, pInfo->FieldHeight);
        DWORD more = (m_lpbi->biBitCount >> 3) * (m_lpbi->biWidth - w);

        for (int y = h - 1; y >= 0; --y)
        {
            BYTE* CurrentLine = pInfo->PictureHistory[0]->pData + y * pInfo->InputPitch;
            src = m_RGBtoYUV((SHORT*)CurrentLine, src, w) + more;
        }
    }

    return true;
}

bool CTimeShift::WriteVideo2(TDeinterlaceInfo* pInfo)
{
    if (m_recHeight != TS_FULLHEIGHT)
    {
        return false;
    }

    if (!m_psCompressedVideo)
    {
        return false;
    }

    BOOL DataOkForPause = false;

    m_thisTimeRecord = GetTickCount();

    // Make sure everything starts out at zero.
    if (!m_startTimeRecord)
    {
        m_startTimeRecord = m_thisTimeRecord;
    }

    DWORD thisFrame =
        DWORD((double)m_fps * double(m_thisTimeRecord - m_startTimeRecord)
              / 1000.0 + 0.5);

    if (m_lastFrameRecord == 0 || m_lastFrameRecord != thisFrame)
    {
        LPBYTE dest = m_recordBits;
        DWORD frameWidth = (pInfo->FrameWidth >> 2) << 2;
        DWORD w = min(m_bih.biWidth, frameWidth);
        DWORD h = min(m_bih.biHeight, pInfo->FrameHeight);
        DWORD more = (m_bih.biBitCount >> 3) * (m_bih.biWidth - w);
        int y = h - 1;

        for (; y >= 0; --y)
        {
            BYTE* CurrentLine = pInfo->Overlay + y * pInfo->OverlayPitch;
            dest = m_YUVtoRGB(dest,
                              (SHORT*)CurrentLine,
                              w) + more;
        }
        DataOkForPause = true;

        MyAVIStreamWrite(m_psCompressedVideo,
                       thisFrame,
                       1,
                       m_recordBits,
                       m_bih.biSizeImage,
                       AVIIF_KEYFRAME);

        // Even if we failed to write to the stream, we'll leave space.
        m_infoVideo.dwLength = thisFrame + 1;
        m_lastFrameRecord = thisFrame;
    }

    switch (m_mode)
    {
    case MODE_PAUSED:
        /** \todo Make pause bits YUY2 so we don't have to convert every time.
                  Use pInfo->pMemcpy() here, but it crashes. ???
        */
        if (!m_gotPauseBits && DataOkForPause)
        {
            memcpy(m_playBits, m_recordBits, m_bih.biSizeImage);
            m_gotPauseBits = true;
        }

        if (m_gotPauseBits)
        {
            LPBYTE src = m_playBits;
            DWORD frameWidth = (pInfo->FrameWidth >> 2) << 2;
            DWORD w = min(m_bih.biWidth, frameWidth);
            DWORD h = min(m_bih.biHeight, pInfo->FrameHeight);
            DWORD more = (m_bih.biBitCount >> 3) * (m_bih.biWidth - w);

            for (int y = h - 1; y >= 0; --y)
            {
                BYTE* CurrentLine = pInfo->Overlay + y * pInfo->OverlayPitch;
                src = m_RGBtoYUV((SHORT*)CurrentLine, src, w) + more;
            }
        }
        break;

    case MODE_SHIFTING:
        ReadVideo2(pInfo);
        break;
    }

    return true;
}

bool CTimeShift::ReadVideo2(TDeinterlaceInfo *pInfo)
{
    if (m_recHeight != TS_FULLHEIGHT)
    {
        return false;
    }

    if (!m_psCompressedVideoP)
    {
        return false;
    }

    m_thisTimePlay = GetTickCount();

    // Make sure everything starts out at zero.
    if (!m_startTimePlay)
    {
        m_startTimePlay = m_thisTimePlay;
    }

    DWORD thisFrame = m_startFramePlay + 
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

    if (m_pGetFrame)
    {
        // Check that the current frame is not empty
        // If empty, find the nearest one that is not empty
        if (m_lastFramePlay == 0 || m_lastFramePlay != thisFrame)
        {
            DWORD nonEmptyFrame = AVIStreamNearestSample(m_psCompressedVideoP, thisFrame);
            if (nonEmptyFrame != thisFrame)
            {
                LOG(2, "thisFrame %d, nonEmptyFrame %d", thisFrame, nonEmptyFrame);
            }
            if (nonEmptyFrame != -1)
            {
                m_lastFramePlay = thisFrame;
                m_lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(m_pGetFrame, nonEmptyFrame);
            }
        }
    }
    else
    {
        m_lpbi = NULL;
    }

    if (m_lpbi)
    {
        LPBYTE src = LPBYTE(m_lpbi) + m_lpbi->biSize;
        DWORD frameWidth = (pInfo->FrameWidth >> 2) << 2;
        DWORD w = min(m_lpbi->biWidth, frameWidth);
        DWORD h = min(m_lpbi->biHeight, pInfo->FrameHeight);
        DWORD more = (m_lpbi->biBitCount >> 3) * (m_lpbi->biWidth - w);

        for (int y = h - 1; y >= 0; --y)
        {
            BYTE* CurrentLine = pInfo->Overlay + y * pInfo->OverlayPitch;
            src = m_RGBtoYUV((SHORT*)CurrentLine, src, w) + more;
        }
    }

    return true;
}

bool CTimeShift::WriteAudio(void)
{
    if (!m_psCompressedAudio)
    {
        return false;
    }

    DWORD count = m_waveInHdrs[m_nextWaveInHdr].dwBytesRecorded;

    DWORD numSamples = count / m_infoAudio.dwSampleSize;

    MyAVIStreamWrite(m_psCompressedAudio,
                   m_nextSampleRecord,
                   numSamples,
                   m_waveInBufs[m_nextWaveInHdr],
                   count,
                   AVIIF_KEYFRAME);

    // Re-add this buffer to the waveIn queue.
    waveInAddBuffer(m_hWaveIn, m_waveInHdrs + m_nextWaveInHdr, sizeof(WAVEHDR));

    ++m_nextWaveInHdr %= sizeof(m_waveInHdrs)/sizeof(*m_waveInHdrs);
    m_nextSampleRecord += numSamples;

    return true;
}

DWORD CTimeShift::MyAVIStreamWrite(PAVISTREAM pavi, LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, DWORD dwFlags)
{
    DWORD result = 0;
    long bytesWritten = 0;
    long samplesWritten = 0;

    // If this fails, there's nothing we can do but continue, no problem.
    result = AVIStreamWrite(pavi,
                            lStart,
                            lSamples,
                            lpBuffer,
                            cbBuffer,
                            dwFlags,
                            &samplesWritten,
                            &bytesWritten);
    m_BytesWritten += bytesWritten;

    return result;
}

bool CTimeShift::ReadAudio(void)
{
    if (!m_psCompressedAudioP)
    {
        return false;
    }

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

bool CTimeShift::SetDimensions(void)
{
    CSource *pSource=Providers_GetCurrentSource();
    ASSERT(pSource!=NULL);

    if (m_mode == MODE_STOPPED)
    {
        // Use current pixel width and field heights to determine our AVI size.
        int w = pSource->GetWidth();
        int h = pSource->GetHeight();

        // Reset all the bitmappInfo stuff.
        memset(&m_bih, 0, sizeof(m_bih));
        m_bih.biSize = sizeof(m_bih);
        m_bih.biWidth = (w >> 2) << 2; // 4-pixel (12-byte) align.
        if (m_recHeight == TS_FULLHEIGHT)
        {
            m_bih.biHeight = (h >> 1) << 1;
        }
        else
        {
            m_bih.biHeight = h >> 1; // For speed (for now), we use 1/2 height AVIs.
        }
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
bool CTimeShift::SetWaveInDevice(char* pszDevice)
{
    if (m_mode == MODE_STOPPED && pszDevice)
    {
        lstrcpy((char*) m_waveInDevice, pszDevice);

        return true;
    }

    return false;
}

bool CTimeShift::SetWaveOutDevice(char* pszDevice)
{
    if (m_mode == MODE_STOPPED && pszDevice)
    {
        lstrcpy((char*) m_waveOutDevice, pszDevice);

        return true;
    }

    return false;
}

bool CTimeShift::GetWaveInDeviceIndex(int *index)
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

bool CTimeShift::GetWaveOutDeviceIndex(int *index)
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

bool CTimeShift::SetRecHeight(int index)
{
    if (m_mode == MODE_STOPPED)
    {
        m_recHeight = index;

        return true;
    }

    return false;
}

bool CTimeShift::CompressionOptions(void)
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
    // Also, for the options dialog, set the pInfo header's compression so it'll
    // show up under "Current Format:", then set it back to BI_RGB for the
    // actual recording process.
    BITMAPINFOHEADER bih = m_bih;
    bih.biCompression = m_infoVideo.fccHandler ? m_infoVideo.fccHandler :BI_RGB;
    AVIStreamSetFormat(psVideo, 0, &bih, sizeof(bih));

    // Create the audio stream and set its format.
    PAVISTREAM psAudio = NULL;
    AVIFileCreateStream(pfile, &psAudio, &m_infoAudio);
    AVIStreamSetFormat(psAudio, 0, m_optsAudio.lpFormat, m_optsAudio.cbFormat);

    // Prompt for compression options.
    AVICOMPRESSOPTIONS optsVideo = m_optsVideo;
    if (optsVideo.cbParms > 0)
    {
        optsVideo.lpParms = new BYTE[optsVideo.cbParms];
        memcpy(optsVideo.lpParms, m_optsVideo.lpParms, optsVideo.cbParms);
    }
    AVICOMPRESSOPTIONS optsAudio = m_optsAudio;
    if (optsAudio.cbFormat > 0)
    {
        optsAudio.lpFormat = new BYTE[optsAudio.cbFormat];
        memcpy(optsAudio.lpFormat, m_optsAudio.lpFormat, optsAudio.cbFormat);
    }
    const int numStreams = 2;
    PAVISTREAM streams[numStreams] = {psVideo, psAudio};
    LPAVICOMPRESSOPTIONS opts[numStreams] = {&optsVideo, &optsAudio};

    // hWnd is the main global hwnd.
    if (AVISaveOptions(hWnd, 0, numStreams, streams, opts))
    {
        // For audio, we need to reset the wave format.
        // Check the user clicked OK on this stream setup by looking for
        // AVICOMPRESSF_VALID (This is undocumented but seems to work --AtNak)

        //Emu - Notes: Blanked this code out for now.

        // Just leave it at the default - CD Quality audio PCM.
        // Would need to have an AV custom setting for all possible
        // codec/ bitrate / channels etc. and i just can't be
        // bothered doing it at the moment .... so stick to PCM

        /*
        if (optsAudio.dwFlags & AVICOMPRESSF_VALID)
        {
            SetAudioOptions(&optsAudio);
            UpdateAudioInfo();
        }
        */

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

bool CTimeShift::SetVideoOptions(AVICOMPRESSOPTIONS *opts)
{
    // Set the newly selected compression codec, if it was indeed set.
    if (opts->fccHandler)
        m_infoVideo.fccHandler = opts->fccHandler;

    // Tweak the options a bit before saving them off.
    opts->fccHandler = m_infoVideo.fccHandler; // In case it was zero.
    opts->dwFlags &= ~AVICOMPRESSF_INTERLEAVE; // No interleaving.

    // ****** 'delete m_optsVideo.lpParms;' is the cause of the crash ******
    // I think its to do with the buffer structure set up under
    // AVICOMPRESSOPTIONS - my hard disk goes berzerk when 'delete m_op...' is exicuted
    // ..... video compression options are set and saved and retreived fine with
    // it commented out - so WGAF ... - AVICOMPRESSOPTIONS is considered to be a very poor API
    //    if (m_setOptsVideo && m_optsVideo.lpParms && m_optsVideo.cbParms) //naughty code
    //        delete m_optsVideo.lpParms;                                   //naughty code

    m_optsVideo = *opts;
    if (opts->lpParms && opts->cbParms)
    {
        m_optsVideo.lpParms = new BYTE[opts->cbParms];
        memcpy(m_optsVideo.lpParms, opts->lpParms, opts->cbParms);
    }

    m_setOptsVideo = true;

    return true;
}


bool CTimeShift::SetAudioOptions(AVICOMPRESSOPTIONS *opts)
{
    opts->fccHandler = 0;

    if (m_setOptsAudio && m_optsAudio.lpFormat && m_optsAudio.cbFormat)
        delete m_optsAudio.lpFormat;
    m_optsAudio = *opts;
    if (opts->lpFormat && opts->cbFormat)
    {
        m_optsAudio.lpFormat = new BYTE[opts->cbFormat];
        memcpy(m_optsAudio.lpFormat, opts->lpFormat, opts->cbFormat);

        // If the format given isn't even as big as a WAVEFORMATEX, we'll
        // take what we can get and leave any old parameters at the end of
        // our structure untouched.  Otherwise, we only care about the first
        // sizeof(WAVEFORMATEX) bytes.
        memcpy(&m_waveFormat,
               m_optsAudio.lpFormat,
               min(m_optsAudio.cbFormat, sizeof(m_waveFormat)));
//      m_waveFormat.nSamplesPerSec = ((WAVEFORMATEX*)(m_optsAudio.lpFormat))->nSamplesPerSec;
//      m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
//      m_waveFormat.wBitsPerSample = 16;
        // Laurent's comment : we keep a WAVE PCM format to avoid an error when running waveInOpen
        m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
//      LOG(1, "Audio format %d %d %d %d", m_waveFormat.wFormatTag, m_waveFormat.nChannels, m_waveFormat.nSamplesPerSec, m_waveFormat.wBitsPerSample);
    }

    m_setOptsAudio = true;

    return true;
}

/*

// Emu Notes: A varient of 'CTimeShift::SetAudioOptions' when testing audio codec selection.
bool CTimeShift::SetAudioOptions(AVICOMPRESSOPTIONS *opts)
{

// Just leave it at the default - CD Quality audio PCM.
// Would need to have an AV custom setting for all possible
// codec/ bitrate / channels etc. and i just can't be
// bothered doing it at the moment .... so stick to PCM



    //opts->fccHandler = 0;
    //m_infoAudio.fccHandler
    // Set the newly selected compression codec, if it was indeed set.
    //if (opts->fccHandler)
    //    m_infoAudio.fccHandler = opts->fccHandler;
    
//  opts->fccHandler = m_infoAudio.fccHandler; // In case it was zero.

    //if (m_setOptsAudio && m_optsAudio.lpFormat && m_optsAudio.cbFormat)
    //    delete m_optsAudio.lpFormat;
    m_optsAudio = *opts;
    if (opts->lpFormat && opts->cbFormat)
    {
        m_optsAudio.lpFormat = new BYTE[opts->cbFormat];
        memcpy(m_optsAudio.lpFormat, opts->lpFormat, opts->cbFormat);

        // If the format given isn't even as big as a WAVEFORMATEX, we'll
        // take what we can get and leave any old parameters at the end of
        // our structure untouched.  Otherwise, we only care about the first
        // sizeof(WAVEFORMATEX) bytes.
        memcpy(&m_waveFormat,
               m_optsAudio.lpFormat,
               min(m_optsAudio.cbFormat, sizeof(m_waveFormat)));

        m_waveFormat.nSamplesPerSec = ((WAVEFORMATEX*)(m_optsAudio.lpFormat))->nSamplesPerSec;
        m_waveFormat.wFormatTag = ((WAVEFORMATEX*)(m_optsAudio.lpFormat))->wFormatTag;
        m_waveFormat.nAvgBytesPerSec = (((WAVEFORMATEX*)(m_optsAudio.lpFormat))->nSamplesPerSec) *
            ((WAVEFORMATEX*)(m_optsAudio.lpFormat))->nBlockAlign;
        m_waveFormat.wBitsPerSample = ((WAVEFORMATEX*)(m_optsAudio.lpFormat))->wBitsPerSample;

        m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
        m_waveFormat.wBitsPerSample = 16;
        // Laurent's comment : we keep a WAVE PCM format to avoid an error when running waveInOpen

        /*
        WORD  wFormatTag;      
    WORD  nChannels;       
    DWORD nSamplesPerSec;  
    DWORD nAvgBytesPerSec; 
    WORD  nBlockAlign;     
    WORD  wBitsPerSample;  
    WORD  cbSize;  
    */
        
        //  m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
//      LOG(1, "Audio format %d %d %d %d", m_waveFormat.wFormatTag, m_waveFormat.nChannels, m_waveFormat.nSamplesPerSec, m_waveFormat.wBitsPerSample);
//    }
//
//    m_setOptsAudio = true;
//
//    return true;
//}

bool CTimeShift::UpdateAudioInfo(void)
{
    // Leave all other stream header fields as they were above.
    m_infoAudio.fccHandler = 0; // optsAudio.fccHandler // Should be zero.

    
    m_infoAudio.dwScale = m_waveFormat.nBlockAlign;
    m_infoAudio.dwRate = m_waveFormat.nAvgBytesPerSec;
    m_infoAudio.dwSuggestedBufferSize = m_infoAudio.dwRate / m_fps;
    m_infoAudio.dwSampleSize = m_waveFormat.nBlockAlign;

    return true;
}

bool CTimeShift::ReadFromIni(void)
{
    extern char szIniFile[MAX_PATH];

    AVICOMPRESSOPTIONS opts;
    if (GetPrivateProfileStruct(
        "TimeShift", "Video", &opts, sizeof(opts), szIniFile))
    {
        opts.lpParms = new BYTE[opts.cbParms];

        // If this one fails, that's ok, there may not be compression params.
        GetPrivateProfileStruct(
            "TimeShift", "CodecVideo", opts.lpParms, opts.cbParms, szIniFile);

        SetVideoOptions(&opts);

        delete opts.lpParms;
    }

    if (GetPrivateProfileStruct(
        "TimeShift", "Audio", &opts, sizeof(opts), szIniFile))
    {
        opts.lpFormat = new BYTE[opts.cbFormat];

        GetPrivateProfileStruct(
            "TimeShift", "FormatAudio", opts.lpFormat, opts.cbFormat, szIniFile);

        SetAudioOptions(&opts);

        delete opts.lpFormat;
    }

    UpdateAudioInfo();


    GetPrivateProfileString(
        "TimeShift", "WaveInDevice", "", m_waveInDevice, MAXPNAMELEN, szIniFile);

    GetPrivateProfileString(
        "TimeShift", "WaveOutDevice", "", m_waveOutDevice, MAXPNAMELEN, szIniFile);

    m_recHeight = GetPrivateProfileInt(
        "TimeShift", "RecHeight", m_recHeight, szIniFile);

    // Get the custom AV sync setting from the INI file. If
    // it does not exist create an entry with defalt value.
    if (GetPrivateProfileInt(
            "TimeShift", "Sync", m_Sync, szIniFile) != NULL)
    {
        m_Sync = GetPrivateProfileInt(
            "TimeShift", "Sync", m_Sync, szIniFile);
        m_waveFormat.nAvgBytesPerSec = m_Sync; //set the custom Syc value
    }
    else 
    {
        m_Sync = m_waveFormat.nAvgBytesPerSec; //defalut is 176400
        WritePrivateProfileInt(
            "TimeShift", "Sync", m_Sync, szIniFile);
    }

    return true;
}

bool CTimeShift::WriteToIni(void)
{
    extern char szIniFile[MAX_PATH];
    char temp[1000];

    if (m_setOptsVideo)
    {
        WritePrivateProfileStruct(
            "TimeShift", "Video", &m_optsVideo, sizeof(m_optsVideo),
            szIniFile);

        if (m_optsVideo.lpParms && m_optsVideo.cbParms)
            WritePrivateProfileStruct(
                "TimeShift", "CodecVideo",
                m_optsVideo.lpParms, m_optsVideo.cbParms,
                szIniFile);
    }

    /*
    // emu - Just leave it at the default - CD Quality audio PCM.
    // Would need to have an AV custom setting for all possible
    // codec/ bitrate / channels etc. and i just can't be
    // bothered doing it at the moment .... so stick to PCM

    if (m_setOptsAudio)
    {
        WritePrivateProfileStruct(
            "TimeShift", "Audio", &m_optsAudio, sizeof(m_optsAudio),
            szIniFile);

        if (m_optsAudio.lpFormat && m_optsAudio.cbFormat)
            WritePrivateProfileStruct(
                "TimeShift", "FormatAudio",
                m_optsAudio.lpFormat, m_optsAudio.cbFormat,
                szIniFile);
    }
    */

    WritePrivateProfileString("TimeShift", "WaveInDevice", m_waveInDevice, szIniFile);

    WritePrivateProfileString("TimeShift", "WaveOutDevice", m_waveOutDevice, szIniFile);

    sprintf(temp, "%u", m_recHeight);
    WritePrivateProfileString("TimeShift", "RecHeight", temp, szIniFile);

    return true;
}

SETTING TimeShiftSettings[TIMESHIFT_SETTING_LASTONE] =
{
    {
        "Saving path for timeshift files", CHARSTRING, 0, (long*)&SavingPath,
         (long)ExePath, 0, 0, 0, 0,
         NULL,
        "TimeShift", "SavingPath", NULL,
    },
    {
        "Shown warning message", ONOFF, 0, (long*)&WarningShown,
        TRUE, 0, 1, 1, 1,
        NULL,
        "TimeShift", "ShownWarning", NULL,
    },
    {
        "AVI file size limit in MB", SLIDER, 0, (long*)&AVIFileSizeLimit,
        2000, 0, 1000000, 1, 1,
        NULL,
        "TimeShift", "AVIFileSizeLimit", NULL,
    },
};

SETTING* TimeShift_GetSetting(TIMESHIFT_SETTING Setting)
{
    if(Setting > -1 && Setting < TIMESHIFT_SETTING_LASTONE)
    {
        return &(TimeShiftSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void TimeShift_ReadSettingsFromIni()
{
    int i;
    struct stat st;

    GetModuleFileName (NULL, ExePath, sizeof(ExePath));
    *(strrchr(ExePath, '\\')) = '\0';

    for(i = 0; i < TIMESHIFT_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(TimeShiftSettings[i]));
    }

    if ((SavingPath == NULL) || stat(SavingPath, &st))
    {
        LOG(1, "Incorrect path for timeshift files; using %s", ExePath);
        Setting_SetValue(TimeShift_GetSetting(TIMESHIFTSAVINGPATH), (long)ExePath);
    }
}

void TimeShift_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < TIMESHIFT_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(TimeShiftSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* TimeShift_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("TimeShift Settings", TimeShiftSettings, TIMESHIFT_SETTING_LASTONE);
}

void TimeShift_FreeSettings()
{
    int i;
    for(i = 0; i < TIMESHIFT_SETTING_LASTONE; i++)
    {
        Setting_Free(&TimeShiftSettings[i]);
    }
}

bool CTimeShift::CancelSchedule(void)
{
    CancelSched = true; // Set flag to show scheduled recording cancelled
    return 0;
}