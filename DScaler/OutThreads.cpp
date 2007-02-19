/////////////////////////////////////////////////////////////////////////////
// $Id: OutThreads.cpp,v 1.144 2007-02-19 23:20:36 robmuller Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////
//
// Refminements made by Mark Rejhon and Steve Grimm
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 09 Aug 2000   John Adcock           Changed WaitForNextFrame to use current RISC
//                                     pointer rather than the status flag
//                                     Also changed VBI processing 
//
// 17 Sep 2000   Mark Rejhon           Implemented Steve Grimm's changes
//                                     Some cleanup done.
//                                     Made refinements to Steve Grimm's changes
//
// 02 Jan 2001   John Adcock           Fixed bug at end of GetCombFactor assember
//                                     Made PAL pulldown detect remember last video
//                                     Mode
//                                     Removed bTV plug-in
//                                     Added Scaled BOB method
//
// 05 Jan 2001   John Adcock           First attempt at judder fix
//                                     Added loop to make sure that we are never
//                                     too early for a flip
//                                     Changed default for gPulldownMode to 2 frame
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing method
//                                     Split code that did adaptive method
//                                     out of UpdateNTSCPulldownMode
//                                     Added gNTSCFilmFallbackMode setting
//                                     Fixed PAL detection bug
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use TDeinterlaceInfo
//
// 24 Jul 2001   Eric Schmidt          Added TimeShift stuff.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.143  2007/02/19 14:48:50  adcockj
// Fixed various issues with d3d9 code and settings
//
// Revision 1.142  2007/02/19 10:13:45  adcockj
// Fixes for Critical thread and RECT issuesin D3D9 and overlay code
//
// Revision 1.141  2006/12/20 07:45:07  adcockj
// added DirectX code from Daniel Sabel
//
// Revision 1.140  2006/09/24 02:44:46  robmuller
// Added missing emms instructions. Should fix problems on non-sse machines.
//
// Revision 1.139  2006/09/24 01:08:16  robmuller
// Removed unused variable bWaitForVsync.
//
// Revision 1.138  2005/07/17 20:40:58  dosx86
// Uses the new time shift functions. Removed the "on new output frame" function.
//
// Revision 1.137  2005/03/23 14:20:58  adcockj
// Test fix for threading issues
//
// Revision 1.136  2004/12/16 21:58:17  robmuller
// Fix errors when compiling without DShow.
//
// Revision 1.135  2004/12/14 23:23:44  laurentg
// Action behind menu "DShow => Stop" now run by the output thread
//
// Revision 1.134  2004/12/14 21:30:14  laurentg
// DShow ChangeRes run by the output thread
//
// Revision 1.133  2004/12/13 23:24:44  laurentg
// Request in GUI thread regarding output thread - extended code
//
// Revision 1.132  2004/05/06 13:49:13  atnak
// updated a comment
//
// Revision 1.131  2003/11/21 14:56:36  robmuller
// Use the correct height with snapshots when using a half-height deinterlace method.
//
// Revision 1.130  2003/11/11 22:16:30  robmuller
// Add ability to include the performance statistics in a release build.
//
// Revision 1.129  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.128  2003/10/04 12:00:20  laurentg
// Always use system memory when taking a still to improve performance
//
// Revision 1.127  2003/08/24 07:13:54  atnak
// Added a state index to Start and Stop Capture
//
// Revision 1.126  2003/08/15 10:27:12  laurentg
// Wrong comment changed
//
// Revision 1.125  2003/08/15 09:22:28  atnak
// Updated a comment
//
// Revision 1.124  2003/08/04 23:48:24  laurentg
// Use extra buffer when recording DScaler output frames
//
// Revision 1.123  2003/07/31 07:00:38  atnak
// Cleaned up Stop_Thread() code
//
// Revision 1.122  2003/07/22 22:33:12  laurentg
// Correct handling of pause (P key) for video file playing
//
// Revision 1.121  2003/07/08 21:04:59  laurentg
// New timeshift mode (full height) - experimental
//
// Revision 1.120  2003/07/05 21:50:25  atnak
// Changed the main YUV thread loop so it runs at least once.
//
// Revision 1.119  2003/06/14 19:38:10  laurentg
// Preview mode improved
//
// Revision 1.118  2003/04/07 09:17:13  adcockj
// Fixes for correct operation of IsFirstInSeries
//
// Revision 1.117  2003/03/25 13:41:33  laurentg
// Replace a LOG(1) by a LOG(2)
//
// Revision 1.116  2003/03/25 13:10:19  laurentg
// New settings for stills : one to disable OSD when taking stills, one to limit the memory usage when storing stills in memory and two to define the number of frames in preview mode
//
// Revision 1.115  2003/03/23 09:24:27  laurentg
// Automatic leave preview mode when necessary
//
// Revision 1.114  2003/03/16 18:29:20  laurentg
// New multiple frames feature
//
// Revision 1.113  2003/03/05 22:08:44  laurentg
// Updated management of 16 bytes aligned buffer for stills
//
// Revision 1.112  2003/02/22 13:37:49  laurentg
// New statistics to check fields runnign late and no flip at time
//
// Revision 1.111  2003/02/05 19:57:58  laurentg
// New option to minimize DScaler when there is no signal and to restore it when a signal is detected
//
// Revision 1.110  2003/01/24 21:12:53  laurentg
// Call to WorkoutOverlaySize necessary just after adjusting the overscan for the source
//
// Revision 1.109  2003/01/24 01:55:17  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.108  2003/01/19 20:24:10  tobbej
// moved Start() call from output thread to Start_Capture()
//
// Revision 1.107  2003/01/18 10:52:11  laurentg
// SetOverscan renamed SetAspectRatioData
// Unnecessary call to SetOverscan deleted
// Overscan setting specific to calibration deleted
//
// Revision 1.106  2003/01/02 19:03:09  adcockj
// Removed extra Surface and replaced with memory buffer due to lack of blt support
//  and alignment problems
//
// Revision 1.105  2003/01/02 17:27:05  adcockj
// Improvements to extra surface code
//
// Revision 1.104  2003/01/02 16:22:09  adcockj
// Preliminary code to support output plugins properly
//
// Revision 1.103  2003/01/01 20:56:45  atnak
// Updates for various VideoText changes
//
// Revision 1.102  2002/12/13 20:31:16  tobbej
// added new assert macro for detecting if running on output thread
//
// Revision 1.101  2002/12/09 00:32:14  atnak
// Added new muting stuff
//
// Revision 1.100  2002/12/07 15:59:06  adcockj
// Modified mute behaviour
//
// Revision 1.99  2002/11/10 16:53:33  tobbej
// added some missing calls to DScalerDeinitializeThread() and CoUninitialize
//
// Revision 1.98  2002/11/01 11:09:49  laurentg
// Possibility to take still when pause is on restored
// Problem of memory leak when taking consecutive stills probably solved
// Display in OSD of the file name used for the still restored
//
// Revision 1.97  2002/10/29 20:51:55  laurentg
// Overlay must be locked when taking a still from the overlay
//
// Revision 1.96  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// Revision 1.95  2002/10/28 22:50:42  laurentg
// Little updates regarding stills
//
// Revision 1.94  2002/10/27 20:39:08  laurentg
// Performance statistics only computed in DEBUG buildd
// Developer OSD screen only present in DEBUG build
//
// Revision 1.93  2002/10/27 11:39:21  laurentg
// Memory buffer for still must be allocated even when saving in a file
//
// Revision 1.92  2002/10/27 11:29:29  laurentg
// New way to take stills - filling a memory buffer rather than the overlay
//
// Revision 1.91  2002/10/26 21:42:05  laurentg
// Take consecutive stills
//
// Revision 1.90  2002/10/26 15:22:04  adcockj
// Fixed issue with vertical flipping
//
// Revision 1.89  2002/10/08 12:12:35  adcockj
// Changed minimize behaviour back to how it was
//
// Revision 1.88  2002/10/07 18:36:20  adcockj
// Corrected judderterminator logic
//
// Revision 1.87  2002/09/29 13:53:40  adcockj
// Ensure Correct History stored
//
// Revision 1.86  2002/09/29 10:14:14  adcockj
// Fixed problem with history in OutThreads
//
// Revision 1.85  2002/09/28 14:49:39  tobbej
// fixed thread init/deinit for crashloging
//
// Revision 1.84  2002/09/17 17:28:25  tobbej
// updated crashloging to same version as in latest virtualdub
//
// Revision 1.83  2002/09/11 18:19:43  adcockj
// Prelimainary support for CX2388x based cards
//
// Revision 1.82  2002/09/09 03:00:15  lindsey
// Allowed filters to ask for more than 4 fields of history
//
// Revision 1.81  2002/09/07 20:59:45  kooiman
// Small fixes.
//
// Revision 1.80  2002/08/18 14:35:45  robmuller
// Changed default.
//
// Revision 1.79  2002/08/16 18:45:55  kooiman
// Added optional screen update delay during tuner frequency switch.
//
// Revision 1.78  2002/08/07 21:53:04  adcockj
// Removed todo item
//
// Revision 1.77  2002/08/06 21:35:08  robmuller
// Don't pause the image when VideoText contains transparency.
//
// Revision 1.76  2002/08/06 21:03:53  robmuller
// Duplicate #include statement removed.
//
// Revision 1.75  2002/08/05 21:01:56  laurentg
// Square pixels mode updated
//
// Revision 1.74  2002/07/20 13:07:36  laurentg
// New setting for vertical mirror
//
// Revision 1.73  2002/06/23 20:48:16  laurentg
// bHurryWhenLate set to FALSE by default
//
// Revision 1.72  2002/06/23 18:06:00  laurentg
// Vertical mirror
//
// Revision 1.71  2002/06/23 11:03:33  robmuller
// If vertical flipping make the odd fields even and the even fields odd.
//
// Revision 1.70  2002/06/22 20:33:31  robmuller
// Offset odd fields with a line when in vertical flip mode to remove jitter.
//
// Revision 1.69  2002/06/22 14:57:45  laurentg
// New vertical flip mode
//
// Revision 1.68  2002/06/06 21:40:00  robmuller
// Fixed: timeshifting and VBI data decoding was not done when minimized.
//
// Revision 1.67  2002/06/05 20:53:49  adcockj
// Default changes and settings fixes
//
// Revision 1.66  2002/05/26 06:32:25  robmuller
// Stop pull down, deinterlacing and output when in full VideoText mode.
//
// Revision 1.65  2002/05/01 20:35:26  tobbej
// fixed log message
//
// Revision 1.64  2002/04/28 16:47:44  laurentg
// Don't call AR autodetection for progressive source as the code works only for interlaced source
//
// Revision 1.63  2002/04/13 18:56:23  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.62  2002/02/23 00:37:15  laurentg
// AR statistics included in user's action to reset statistics
// AR statistics reseted at the startup of the decoding thread
//
// Revision 1.61  2002/02/20 18:38:27  tobbej
// removed unneeded WorkoutOverlaySize
//
// Revision 1.60  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.59  2002/02/19 10:24:08  tobbej
// increased timeout a bit, since dshow takes longer to terminate
//
// Revision 1.58  2002/02/18 20:51:51  laurentg
// Statistics regarding deinterlace modes now takes into account the progressive mode
// Reset of the deinterlace statistics at each start of the decoding thread
// Reset action now resets the deinterlace statistics too
//
// Revision 1.57  2002/02/14 23:16:59  laurentg
// Stop / start capture never needed when switching between files of the playlist
// CurrentX / CurrentY not updated in StillSource but in the main decoding loop
//
// Revision 1.56  2002/02/11 23:18:33  laurentg
// Creation of a DEINTERLACE_METHOD data structure for the progressive mode
//
// Revision 1.55  2002/02/10 21:42:29  laurentg
// New menu items "Progressive Scan" and "Film Mode"
//
// Revision 1.54  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.53  2002/01/26 18:04:28  laurentg
// Locking and unlocking the overlay and not the overlay back buffer when taking stills
//
// Revision 1.52  2001/12/17 19:31:17  tobbej
// added COM init to output thread
//
// Revision 1.51  2001/12/16 18:40:28  laurentg
// Reset statistics
//
// Revision 1.50  2001/12/16 13:13:34  laurentg
// New statistics
//
// Revision 1.49  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.48  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.47  2001/11/26 15:27:18  adcockj
// Changed filter structure
//
// Revision 1.46  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.45  2001/11/24 18:05:23  laurentg
// Managing of progressive source
//
// Revision 1.44  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.43  2001/11/22 17:20:23  adcockj
// Added version to info structure
//
// Revision 1.42  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.41  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.40  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.39  2001/11/17 18:15:57  adcockj
// Bugfixes (including very silly performance bug)
//
// Revision 1.38  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.37  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.36  2001/11/01 11:35:23  adcockj
// Pre release changes to version, help, comment and headers
//
// Revision 1.35  2001/09/24 23:14:05  laurentg
// Draw rectangles around analysis zones
// Correction regarding overscan
//
// Revision 1.34  2001/09/22 18:07:53  laurentg
// Automatic calibration now done for odd and even fields
//
// Revision 1.33  2001/09/05 15:08:22  adcockj
// Wrapped overlay calls with critical section
// Updated Loging
//
// Revision 1.32  2001/08/26 18:33:42  laurentg
// Automatic calibration improved
//
// Revision 1.31.2.10  2001/08/22 11:12:48  adcockj
// Added VBI support
//
// Revision 1.31.2.9  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.31.2.8  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.31.2.7  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.31.2.6  2001/08/19 14:43:47  adcockj
// Fixed memory leaks
//
// Revision 1.31.2.5  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.31.2.4  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.31.2.3  2001/08/16 06:43:34  adcockj
// moved more stuff into the new file (deonsn't compile)
//
// Revision 1.31.2.2  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
// Revision 1.31.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.31  2001/08/03 14:24:32  adcockj
// added extra info to splash screen and log
//
// Revision 1.30  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.29  2001/07/30 19:51:30  laurentg
// no message
//
// Revision 1.28  2001/07/29 22:51:09  laurentg
// OSD screen for card calibration improved
// Test patterns description added or corrected
//
// Revision 1.27  2001/07/27 16:11:32  adcockj
// Added support for new Crash dialog
//
// Revision 1.26  2001/07/26 22:38:04  laurentg
// Call to card calibration function added
//
// Revision 1.25  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
// Revision 1.24  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.23  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.22  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.21  2001/07/13 07:04:43  adcockj
// Attemp 1 at fixing MSP muting
//
// Revision 1.20  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file OutThreads.cpp Main video processing thread code
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "OutThreads.h"
#include "IOutput.h"
#include "VBI_VideoText.h"
#include "VBI.h"
#include "Deinterlace.h"
#include "AspectRatio.h"
#include "DScaler.h"
#include "DebugLog.h"
#include "Settings.h"
#include "Filter.h"
#include "Status.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "FD_Prog.h"
#include "FD_Common.h"
#include "FD_CommonFunctions.h"
#include "CPU.h"
#include "FieldTiming.h"
#include "MixerDev.h"
#include "Audio.h"
#include "TimeShift.h"
#include "Calibration.h"
#include "Crash.h"
#include "Providers.h"
#include "StillProvider.h"
#include "Perf.h"
#include "OSD.h"
#include "MultiFrames.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dshowsource\DSSource.h"
#endif

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
BOOL                RequestToggleFlip = FALSE;
BOOL				bCheckSignalPresent = FALSE;
BOOL				bCheckSignalMissing = FALSE;
BOOL                bDoVerticalFlipSetting = FALSE;
HANDLE              g_hOutThread;
DWORD OutThreadID=0;
TGUIRequest			Request = { REQ_NONE, 0, 0 };

// Capture state variables
LONG                g_nCaptureStatus = 0;

// Dynamically updated variables
BOOL                bAutoDetectMode = TRUE;


// TRB 10/28/00 changes, parms, and new fields for sync problem fixes
BOOL                WaitForFlip = TRUE;       // User parm, default=TRUE
BOOL                DoAccurateFlips = TRUE;     // User parm, default=TRUE
BOOL                bHurryWhenLate = FALSE;    // " , default=FALSE, skip processing if behind
BOOL                bIsOddField = FALSE;
BOOL                bReversePolarity = FALSE;
BOOL                bJudderTerminatorOnVideo = TRUE;

BOOL                bNoScreenUpdateDuringTuning = FALSE; //Don't update if set. Code to enable/disable this in programlist.cpp

// cope with older DX header files
#if !defined(DDFLIP_DONOTWAIT)
    #define DDFLIP_DONOTWAIT 0
#endif


// Zero out all the picture hsitory
// need to do this if we drop a frame
void ClearPictureHistory(TDeinterlaceInfo* pInfo)
{
    memset(pInfo->PictureHistory, 0, MAX_PICTURE_HISTORY * sizeof(TPicture*));
}

// Shift the picture history up one
// do this before adding a new picture at the index zero
// the NumFieldsValid should be set to the number of different fields that
// you hold in the source.  This must not be greater than MAX_PICTURE_HISTORY
void ShiftPictureHistory(TDeinterlaceInfo* pInfo, int NumFieldsValid)
{
    memmove(&pInfo->PictureHistory[1], &pInfo->PictureHistory[0], sizeof(pInfo->PictureHistory[0]) * (NumFieldsValid - 1));
    pInfo->PictureHistory[0] = NULL;
}

/**
 * This function triggers an assert if called on output thread.
 * This can be used during debuging to make sure that a certain part of the
 * code is not called from the output thread.
 * To use this function use the ASSERTONOUTTHREAD macro, this will make sure
 * that the assert is only triggered in debug build.
 */
#ifdef _DEBUG
void AssertOnOutThread()
{
	if(OutThreadID!=0)
	{
		if(GetCurrentThreadId()==OutThreadID)
		{
			//the reason for using int 3 insted of ASSERT() is that the ASSERT
			//macro shows a dialog which might create problems.
			_asm int 3;
		}
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
void Start_Thread()
{
    // make sure we start with a clean sheet of paper
    GetActiveOutput()->Overlay_Clean();

    bStopThread = FALSE;

    g_hOutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,   // No security.
                                (DWORD) 0,                      // Same stack size.
                                YUVOutThread,                   // Thread procedure.
                                NULL,                           // Parameter.
                                (DWORD) 0,                      // Start immediatly.
                                (LPDWORD) &OutThreadID);        // Thread ID.
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Thread()
{
    if (g_hOutThread != NULL)
    {
        SetThreadPriority(g_hOutThread, THREAD_PRIORITY_NORMAL);

        // Signal the stop
        bStopThread = TRUE;

        // Wait one second for the thread to exit gracefully
        DWORD dwResult = WaitForSingleObject(g_hOutThread, 1000);

        if (dwResult != WAIT_OBJECT_0)
        {
            LOG(3,"Timeout waiting for YUVOutThread to exit, terminating it via TerminateThread()");
            TerminateThread(g_hOutThread, 0);
        }

        CloseHandle(g_hOutThread);
        g_hOutThread = NULL;

        if (dwResult != WAIT_OBJECT_0)
        {
            // THIS SHOULD NOT HAPPEN

            // ...but it is and it's something we need to debug.
            // There are deadlocks with GUI access from the YUVOutThread.
            // --AtNak 2003-07-31  [In progress comments, remove when fixed.]
            // http://sourceforge.net/mailarchive/message.php?msg_id=5703600

            MessageBox(GetMainWnd(),
                "The video thread failed to exit in a timely manner and was forcefully "
                "terminated.  You may experience further problems.",
                "Unexpected Error", MB_OK);
        }
    }
}

void Pause_Capture()
{
    bIsPaused = TRUE;
}

void UnPause_Capture()
{
    bIsPaused = FALSE;
}

void PutRequest(TGUIRequest *req)
{
	if (Request.type == REQ_NONE)
	{
		switch (req->type)
		{
		case REQ_STILL:
			if ((req->param1 > 0) && (Providers_GetSnapshotsSource() != NULL))
			{
				Request.type = req->type;
				Request.param1 = req->param1;
				if (req->param1 > 1)
				{
					Request.param2 = 1;
				}
				else
				{
					Request.param2 = (Setting_GetValue(Still_GetSetting(STILLSINMEMORY)) == TRUE) ? 1 : 0;
				}
			}
			break;
		case REQ_SNAPSHOT:
			Request.type = req->type;
			break;
#ifdef WANT_DSHOW_SUPPORT
		case REQ_DSHOW_STOP:
			Request.type = req->type;
			break;
		case REQ_DSHOW_CHANGERES:
			Request.type = req->type;
			Request.param1 = req->param1;
			break;
#endif
		default:
			break;
		}
	}
}

void Toggle_Vertical_Flip()
{
    RequestToggleFlip = TRUE;
}

// save the Info structure to a snapshot file
// these files will make it easier to test 
// deinterlacing techniques as we can start
// to exchange the actual data we are each looking
// at and have the ability to recreate results
void SaveStreamSnapshot(TDeinterlaceInfo* pInfo)
{
    FILE* file;
    char name[13];
    int n = 0;
    int i = 0;
    int j;
    struct stat st;

    while (n < 100)
    {
        sprintf(name,"sn%06d.dtv",++n) ;
        if (stat(name, &st))
        {
            break;
        }
    }

    if(n == 100)
    {
        ErrorBox("Could not create a file.  You may have too many snapshots already.");
        return;
    }

    file = fopen(name,"wb");
    if (!file)
    {
        ErrorBox("Could not open file in SaveStreamSnapshot");
        return;
    }

    // just save the Info struct
    // most of the data is pointers which will be useless
    // to anyone else
    // but NULLs will be useful in determining how many
    // fields we have.
    // The rest will contain all the data we need to use
    // the data in a test program
    fwrite(pInfo, sizeof(TDeinterlaceInfo), 1, file);

    // save all the fields
    i = 0;
    while(i < MAX_FIELD_HISTORY && pInfo->PictureHistory[i] != NULL)
    {
        //write out the header with flags
        fwrite(pInfo->PictureHistory[i], sizeof(TPicture), 1, file);

        // write out the data
        BYTE* CurLine = pInfo->PictureHistory[i]->pData;
        for(j = 0; j < pInfo->FieldHeight; ++j)
        {
            fwrite(CurLine, pInfo->LineLength, 1, file);
            CurLine += pInfo->InputPitch;
        }
        i++;      
    }
    fclose(file);
}

void Pause_Toggle_Capture()
{
    if(bIsPaused)
    {
        UnPause_Capture();
        if (Providers_GetCurrentSource())
        {
			Providers_GetCurrentSource()->UnPause();
        }
    }
    else
    {
        Pause_Capture();
        if (Providers_GetCurrentSource())
        {
			Providers_GetCurrentSource()->Pause();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Start_Capture()
{
    if (g_nCaptureStatus++ == 0)
    {
        // make sure half height Modes are set correctly
        GetActiveOutput()->Overlay_Clean();

        // moved this stuff out of the processing thared to avoid windo calls
        // in the processing thread
		if (pMultiFrames && pMultiFrames->IsActive())
		{
			// Check whether preview mode must be switched to OFF
			if ( (Providers_GetCurrentSource() != pMultiFrames->GetSource())
			  || ( (pMultiFrames->GetMode() == PREVIEW_CHANNELS)
			    && !Providers_GetCurrentSource()->IsInTunerMode() ) )
			{
				delete pMultiFrames;
				pMultiFrames = NULL;
			}
		}
		if (!pMultiFrames || !pMultiFrames->IsActive())
		{
			Providers_GetCurrentSource()->SetAspectRatioData();
			WorkoutOverlaySize(TRUE);
		}

        if (Providers_GetCurrentSource())
        {
			Providers_GetCurrentSource()->Start();
            PrepareDeinterlaceMode();
            Start_Thread();
        }

        Audio_Unmute();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Capture()
{
    if (g_nCaptureStatus-- == 1)
    {
        Audio_Mute();

        if (pCalibration->IsRunning())
        {
            pCalibration->Stop();
        }
        if (Providers_GetCurrentSource())
        {
            //  Stop The Output Thread
            Stop_Thread();

			//there is a Stop() call at the end of the output thread too
			//maybe that call shoud be removed and add a __try/__except here?
			//or just let the __try in MainWndProcSafe catch any problems?
            Providers_GetCurrentSource()->Stop();
        }
        UpdateSquarePixelsMode(FALSE);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Reset_Capture()
{
    if (!Providers_GetCurrentSource())
        return;
    Stop_Capture();
    GetActiveOutput()->Overlay_Clean();
    PrepareDeinterlaceMode();
    Providers_GetCurrentSource()->Reset();
    WorkoutOverlaySize(TRUE);
    Start_Capture();
}

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter)
{
    BOOL bFlipNow = TRUE;
    TDeinterlaceInfo Info;
    DEINTERLACE_METHOD* PrevDeintMethod = NULL;
    DEINTERLACE_METHOD* CurrentMethod = NULL;
    int nHistory = 0;
    long SourceAspectAdjust = 1000;
	BYTE* pAllocBuf = NULL;
	BOOL bTakeStill = FALSE;
	BOOL bUseOverlay = TRUE;
    BOOL bUseExtraBuffer;
	BOOL bIsPresent;
	int nMissing = 0;
	BYTE* lpMultiBuffer = NULL;
	int MultiPitch = 0;
    BOOL IsFirstInSeriesFlag = TRUE;

    DScalerInitializeThread("YUV Out Thread");

#ifdef WANT_DSHOW_SUPPORT
    //com init for this thread
    CoInitializeEx(NULL,COINIT_MULTITHREADED);
#endif

    CSource* pSource = Providers_GetCurrentSource();
    BOOL bIsPAL = GetTVFormat(pSource->GetFormat())->Is25fps;

    Timing_Setup();

    // set up Deinterlace Info struct
    memset(&Info, 0, sizeof(Info));
    Info.Version = DEINTERLACE_INFO_CURRENT_VERSION;
    Info.CpuFeatureFlags = CpuFeatureFlags;
    Info.OverlayPitch = 0;
    if(CpuFeatureFlags & FEATURE_SSE)
    {
        Info.pMemcpy = memcpySSE;
    }
    else
    {
        Info.pMemcpy = memcpyMMX;
    }

    ResetDeinterlaceStats();
    ResetARStats();

    // catch anything fatal in this loop so we don't crash the machine
    __try
    {
        VDCHECKPOINT;

        bUseExtraBuffer = Filter_WillWeDoOutput()
			           || (pMultiFrames && pMultiFrames->IsActive())
					   || (TimeShiftIsRunning() && !TimeShiftWorkOnInputFrames());


        // Anti-plop and update screen delay timers may have been cancelled.
        // Reset to default values
        bNoScreenUpdateDuringTuning = FALSE;   

        // Sets processor Affinity and Thread priority according to menu selection
        SetThreadProcessorAndPriority();

        PrevDeintMethod = GetCurrentDeintMethod();

        // reset the static variables in the detection code
        if (bIsPAL)
        {
            UpdatePALPulldownMode(NULL);
            UpdateProgPulldownModePAL(NULL);
        }
        else
        {
            UpdateNTSCPulldownMode(NULL);
            UpdateProgPulldownModeNTSC(NULL);
        }
        VDCHECKPOINT;

		do
        {
			if (bCheckSignalPresent)
			{
				// Check every second if there is signal present
				// If there is a signal, ask the main thread to restore the main window
				Sleep(1000);
				if (!Providers_GetCurrentSource()->IsVideoPresent())
				{
					continue;
				}
				PostMessageToMainWindow(UWM_SWITCH_WINDOW,0,0);
				bCheckSignalPresent = FALSE;
			}

			if (bCheckSignalMissing)
			{
				// Wait 30 consecutive fields before deciding that the signal is missing
				// Then ask the main thread to minimize the main window
				bIsPresent = Providers_GetCurrentSource()->IsVideoPresent();
				if (!bIsPresent)
				{
					nMissing++;
					if (nMissing > 30)
					{
						PostMessageToMainWindow(UWM_SWITCH_WINDOW,0,0);
						bCheckSignalMissing = FALSE;
						nMissing = 0;
					}
				}
				else
				{
					nMissing = 0;
				}
			}

#ifdef USE_PERFORMANCE_STATS
            pPerf->StartCount(PERF_WAIT_FIELD);
#endif

            // update with any changes
            CurrentMethod = GetCurrentDeintMethod();
            Info.bDoAccurateFlips = DoAccurateFlips;
            Info.bRunningLate = bHurryWhenLate;
            Info.bMissedFrame = FALSE;
            //Info.OverlayPitch = OverlayPitch;
            Info.CombFactor = -1;
            Info.FieldDiff = -1;
            bFlipNow = FALSE;
			bTakeStill = FALSE;
			pAllocBuf = NULL;
			bUseOverlay = TRUE;
            GetDestRect(&Info.DestRect);

			if (pMultiFrames && pMultiFrames->IsActive())
			{
				pMultiFrames->SelectFrame();
			}

			// Go and get the next input field
            // JudderTerminator
            // We pass down a flag telling the function if we are
            // requesting accurate timings of the incoming fields
            // This will use up CPU but will
            // allow us to flip at the right times
            // collect the timings even if we are in video
            // mode and bJudderTerminatorOnVideo if off
            // to build up more accurate timings ready for
            // when film is detected.
            // don't do accurate timings if we are minimized as we want to reduce CPU
            // consumption
            pSource->GetNextField(&Info, Info.bDoAccurateFlips && !bMinimized);

#ifdef USE_PERFORMANCE_STATS
            pPerf->StopCount(PERF_WAIT_FIELD);
#endif

            if (Info.bMissedFrame || Info.bRunningLate)
            {
                LOG(2, "    Info.bMissedFrame %d - Info.bRunningLate %d", Info.bMissedFrame, Info.bRunningLate);
#ifdef USE_PERFORMANCE_STATS
                for (int i = 0 ; i < PERF_TYPE_LASTONE ; ++i)
                {
                    if (pPerf->IsValid((ePerfType)i) && (pPerf->GetDurationLastCycle((ePerfType)i) != -1))
                    {
                        LOG(2, "    %s : %d (avg %d)",
                            pPerf->GetName((ePerfType)i), 
                            pPerf->GetDurationLastCycle((ePerfType)i) * 10, 
                            pPerf->GetAverageDuration((ePerfType)i));
                    }
                }
#endif
            }

            pPerf->InitCycle();

			if (Request.type == REQ_STILL)
			{
				bTakeStill = TRUE;

				// When taking a still, we always work with system memory for performance reasons

				// Memory already allocated to store snapshots
				int MemUsed = ((CStillSource*)Providers_GetSnapshotsSource())->CountMemoryUsage();

				// If we are taking a still in memory, we need to allocate
				// a memory buffer and use this buffer as output
				// That means too that the overlay will not be updated
				bUseOverlay = FALSE;
				Info.OverlayPitch = (Info.FrameWidth * 2 * sizeof(BYTE) + 15) & 0xfffffff0;

				// Add the memory needed for the new snapshot
				MemUsed += Info.OverlayPitch * Info.FrameHeight;
				LOG(2, "MemUsed %d Mo", MemUsed / 1048576);

				// Check that the max is not reached
				if ((MemUsed / 1048576) >= Setting_GetValue(Still_GetSetting(MAXMEMFORSTILLS)))
				{
					char text[128];
					pAllocBuf = NULL;
					sprintf(text, "Max memory (%d Mo) reached\nChange the maximum value or\nclose some open stills", Setting_GetValue(Still_GetSetting(MAXMEMFORSTILLS)));
					OSD_ShowText(text, 0);
				}
				else
				{
					pAllocBuf = (BYTE*)malloc(Info.OverlayPitch * Info.FrameHeight + 16);
					Info.Overlay = START_ALIGNED16(pAllocBuf);
				}
				LOG(2, "Alloc for still - start buf %d, start frame %d", pAllocBuf, Info.Overlay);
				if (pAllocBuf == NULL)
				{
					Request.type = REQ_NONE;
					bTakeStill = FALSE;
					bUseOverlay = TRUE;
				}

				// After that line, we must use variable bTakeStill instead of Request
				// because Request could be set by the GUI thread at the same time,
				// and we must be certain that memory allocation has been done
			}

            if(bIsPaused == FALSE)
            {
                if(Info.PictureHistory[0] == NULL)
                {
                    Sleep(50);
                    continue;
                }
                // calculate History: How many consecutive non-null PictureHistory[]s do we have?
                for( nHistory = 1; (nHistory < MAX_PICTURE_HISTORY) && (Info.PictureHistory[nHistory] != NULL); ++nHistory )
                {
                    ; // Do nothing
                }

                // Vertical flipping support
                // Here we change all the valid history so that the top line is the bottom one
                // and the pitch is negative
                // Note that we don't "own" these pictures but we have them for the time being
                // so make sure that we change them back after we use them
                // so just in case the setting gets changes while we are using it 
                // we save the current value
                BOOL bDoVerticalFlipCurrent = bDoVerticalFlipSetting;
                if (bDoVerticalFlipCurrent)
                {
                    for(int i(0); i < MAX_PICTURE_HISTORY; ++i)
                    {
                        if(Info.PictureHistory[i] != NULL)
                        {
                            Info.PictureHistory[i]->pData += (Info.FieldHeight-1) * Info.InputPitch;

                            // make the odd fields even and the even fields odd.
                            if(Info.PictureHistory[i]->Flags & PICTURE_INTERLACED_ODD)
                            {
                                Info.PictureHistory[i]->Flags &= ~PICTURE_INTERLACED_ODD;
                                Info.PictureHistory[i]->Flags |= PICTURE_INTERLACED_EVEN;
                            }
                            else if(Info.PictureHistory[i]->Flags & PICTURE_INTERLACED_EVEN)
                            {
                                Info.PictureHistory[i]->Flags &= ~PICTURE_INTERLACED_EVEN;
                                Info.PictureHistory[i]->Flags |= PICTURE_INTERLACED_ODD;
                            }
                        }
                    }
                    Info.InputPitch *= -1;
                }

                // will be set to TRUE on first pass though
                // FALSE at all other times eben with dropped frames
                Info.PictureHistory[0]->IsFirstInSeries = IsFirstInSeriesFlag;
                IsFirstInSeriesFlag = FALSE;

                // Card calibration
				if (pCalibration->IsRunning())
				{
#ifdef USE_PERFORMANCE_STATS
                    pPerf->StartCount(PERF_CALIBRATION);
#endif

                    pCalibration->Make(&Info, GetTickCount());

#ifdef USE_PERFORMANCE_STATS
                    pPerf->StopCount(PERF_CALIBRATION);
#endif
				}

                // update the source area
                GetSourceRect(&Info.SourceRect);
                
#ifdef USE_PERFORMANCE_STATS
                pPerf->StartCount(PERF_INPUT_FILTERS);
#endif

                // do any filters that operarate on the input
                // only
                SourceAspectAdjust = Filter_DoInput(&Info, nHistory, (Info.bRunningLate || Info.bMissedFrame));

#ifdef USE_PERFORMANCE_STATS
                pPerf->StopCount(PERF_INPUT_FILTERS);
#endif

				if (TimeShiftWorkOnInputFrames())
				{
#ifdef USE_PERFORMANCE_STATS
	                pPerf->StartCount(PERF_TIMESHIFT);
#endif

					// NOTE: I might go ahead and make the TimeShift module an input
					// filter at some point (i.e. FLT_TimeShift), but I'm not sure
					// if that's the right thing to do since it ties into other
					// parts of the app.
					TimeShiftOnNewInputFrame(&Info);

#ifdef USE_PERFORMANCE_STATS
	                pPerf->StopCount(PERF_TIMESHIFT);
#endif
				}

                if (!Info.bMissedFrame && !bMinimized && !bNoScreenUpdateDuringTuning &&
                    (VT_GetState() != VT_BLACK  || VT_IsTransparencyInPage()))
                {
#ifdef USE_PERFORMANCE_STATS
                    pPerf->StartCount(PERF_PULLDOWN_DETECT);
#endif

                    // do film detect
                    if(Info.PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK)
                    {
                        if (UnsetProgressiveMode())
                        {
                            CurrentMethod = GetCurrentDeintMethod();
                        }

                        // we have an interlaced source
                        if(bAutoDetectMode == TRUE)
                        {
                            if(bIsPAL)
                            {
                                // we will need always need both comb and diff
                                // for film detect to work properly
                                PerformFilmDetectCalculations(&Info, TRUE, TRUE);
                                UpdatePALPulldownMode(&Info);
                            }
                            else
                            {
                                // we will need always need diff
                                // comb is needed in film Mode or when we ask for it
                                PerformFilmDetectCalculations(&Info, 
                                                            IsFilmMode() ||
                                                                CurrentMethod->bNeedCombFactor,
                                                            TRUE);
                                UpdateNTSCPulldownMode(&Info);
                            }
                            // get the current method again
                            // after the film Modes have been selected
                            CurrentMethod = GetCurrentDeintMethod();
                        }
                        else
                        {
                            PerformFilmDetectCalculations(&Info, 
                                                            CurrentMethod->bNeedCombFactor, 
                                                            CurrentMethod->bNeedFieldDiff);
                        }
                    }
                    else
                    {
                        // we have an progressive source
                        // so do special progressive detect
                        // so that we can do judder termination on
                        // 24/25 fps sources even though we get the feed
                        // at a full frame rate
                        SetProgressiveMode();

                        if(bAutoDetectMode == TRUE)
                        {
                            PerformProgFilmDetectCalculations(&Info);
                            if(bIsPAL)
                            {
                                UpdateProgPulldownModePAL(&Info);
                            }
                            else
                            {
                                UpdateProgPulldownModeNTSC(&Info);
                            }
                        }
                        CurrentMethod = GetCurrentDeintMethod();
                    }

#ifdef USE_PERFORMANCE_STATS
                    pPerf->StopCount(PERF_PULLDOWN_DETECT);
#endif
                }

                if (bCaptureVBI == TRUE)
                {
#ifdef USE_PERFORMANCE_STATS
                    pPerf->StartCount(PERF_VBI);
#endif

                    pSource->DecodeVBI(&Info);

#ifdef USE_PERFORMANCE_STATS
                    pPerf->StopCount(PERF_VBI);
#endif
                }

                // do we need to unlock overlay if we crash
                BOOL bOverlayLocked = FALSE;

                __try
                {
                    if(!Info.bRunningLate && !bMinimized && !bNoScreenUpdateDuringTuning &&
                        (VT_GetState() != VT_BLACK  || VT_IsTransparencyInPage()))
                    {
                        BOOL bFlipNow = FALSE;

                        // do the aspect ratio only every other frame
                        // also do this outside of the internal lock
                        // to avoid conflicts
                        if (Info.PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
                        {
#ifdef USE_PERFORMANCE_STATS
                            pPerf->StartCount(PERF_RATIO);
#endif

                            AdjustAspectRatio(SourceAspectAdjust, &Info);

#ifdef USE_PERFORMANCE_STATS
                            pPerf->StopCount(PERF_RATIO);
#endif
                        }

						if(bUseOverlay)
						{
#ifdef USE_PERFORMANCE_STATS
	                        pPerf->StartCount(PERF_LOCK_OVERLAY);
#endif
							// Need to be careful with the locking
                            if(!GetActiveOutput()->Overlay_Lock_Back_Buffer(&Info, bUseExtraBuffer))
							{
								Providers_GetCurrentSource()->Stop();
								LOG(1, "Falling out after Overlay_Lock_Back_Buffer");
								PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_STOP, 0);
								PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_START, 0);
#ifdef WANT_DSHOW_SUPPORT
								CoUninitialize();
#endif
								DScalerDeinitializeThread();
								return 1;
							}
							bOverlayLocked = TRUE;
#ifdef USE_PERFORMANCE_STATS
	                        pPerf->StopCount(PERF_LOCK_OVERLAY);
#endif
						}

#ifdef USE_PERFORMANCE_STATS
                        pPerf->StartCount(PERF_DEINTERLACE);
#endif

                        if (IsProgressiveMode())
                        {
                            bFlipNow = CurrentMethod->pfnAlgorithm(&Info);
                        }
                        // if we have dropped a field then do BOB 
                        // or if we need to get more history
                        // if we are doing a half height Mode then just do that
                        // anyway as it will be just as fast
                        else if(CurrentMethod->bIsHalfHeight == FALSE && 
                                ((Info.bMissedFrame == TRUE) || (nHistory < CurrentMethod->nFieldsRequired)))
                        {
                            bFlipNow = Bob(&Info);
                        }
                        else
                        {
                            bFlipNow = CurrentMethod->pfnAlgorithm(&Info);
                        }
                        CHECK_FPU_STACK
                    
#ifdef USE_PERFORMANCE_STATS
                        pPerf->StopCount(PERF_DEINTERLACE);
#endif

                        if (bFlipNow)
                        {
#ifdef USE_PERFORMANCE_STATS
                            pPerf->StartCount(PERF_OUTPUT_FILTERS);
#endif

                            // Do any filters that run on the output
                            // need to do this while the surface is locked
                            Filter_DoOutput(&Info, nHistory, Info.bMissedFrame);

#ifdef USE_PERFORMANCE_STATS
                            pPerf->StopCount(PERF_OUTPUT_FILTERS);
#endif

							if (!TimeShiftWorkOnInputFrames())
							{
#ifdef USE_PERFORMANCE_STATS
				                pPerf->StartCount(PERF_TIMESHIFT);
#endif

								/* CTimeShift::OnNewOutputFrame(&Info); */

#ifdef USE_PERFORMANCE_STATS
				                pPerf->StopCount(PERF_TIMESHIFT);
#endif
							}

                        }

#ifdef USE_PERFORMANCE_STATS
                        pPerf->StartCount(PERF_UNLOCK_OVERLAY);
#endif

						if(bOverlayLocked)
						{
							// there should be no exit paths between this unlock and the lock
                            // so we should be OK
                            // somewhere above we will have locked the buffer, unlock before flip
							if(!GetActiveOutput()->Overlay_Unlock_Back_Buffer(bUseExtraBuffer))
							{
								Providers_GetCurrentSource()->Stop();
								LOG(1, "Falling out after Overlay_Unlock_Back_Buffer");
								PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_STOP, 0);
								PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_START, 0);
#ifdef WANT_DSHOW_SUPPORT
								CoUninitialize();
#endif
								DScalerDeinitializeThread();
								return 1;
							}
							bOverlayLocked = FALSE;
						}

#ifdef USE_PERFORMANCE_STATS
                        pPerf->StopCount(PERF_UNLOCK_OVERLAY);
#endif

						if(bTakeStill && !bFlipNow)
						{
							bTakeStill = FALSE;
							if (pAllocBuf != NULL)
							{
								free(pAllocBuf);
							}
						}

                        // flip overlay if required
                        if (bFlipNow && bUseOverlay)
                        {
#ifdef USE_PERFORMANCE_STATS
	                        pPerf->StartCount(PERF_FLIP_OVERLAY);
#endif

							// setup flip flag
							// the odd and even flags may help the scaled bob
							// on some cards
							DWORD FlipFlag = (WaitForFlip)?DDFLIP_WAIT:DDFLIP_DONOTWAIT;
							if(CurrentMethod->nMethodIndex == INDEX_SCALER_BOB)
							{
								if(Info.PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
								{
									FlipFlag |= DDFLIP_ODD;
								}
								else if(Info.PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN)
								{
									FlipFlag |= DDFLIP_EVEN;
								}
							}

							if (pMultiFrames && pMultiFrames->IsActive())
							{
								pMultiFrames->UpdateFrame(&Info, &bUseExtraBuffer, &lpMultiBuffer, &MultiPitch);
							}
							else
							{
								lpMultiBuffer = NULL;
								MultiPitch = 0;
							}

							// JudderTerminator
							// Here we space out the flips by waiting for a fixed time between
							// flip calls.
							// We need to go in if:
							// - JudderTerminator is On
							// - We are in film mode or we want JT on Video
							// - the deinterlace method is the same as last time
							if(Info.bDoAccurateFlips && (IsFilmMode() || bJudderTerminatorOnVideo) && PrevDeintMethod == CurrentMethod)
							{
								Timing_WaitForTimeToFlip(&Info, CurrentMethod, &bStopThread);
							}
							else if(Info.bDoAccurateFlips && (IsFilmMode() || bJudderTerminatorOnVideo))
							{
								Timing_IncrementNoFlipAtTime();
							}

							if(!GetActiveOutput()->Overlay_Flip(FlipFlag, bUseExtraBuffer, lpMultiBuffer, MultiPitch, &Info))
							{
								Providers_GetCurrentSource()->Stop();
								LOG(1, "Falling out after Overlay_Flip");
								PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_STOP, 0);
								PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_START, 0);
								DScalerDeinitializeThread();
								return 1;
							}
                                                       

                            // update which surface we write to only after a flip
							bUseExtraBuffer = Filter_WillWeDoOutput()
										   || (pMultiFrames && pMultiFrames->IsActive())
										   || (TimeShiftIsRunning() && !TimeShiftWorkOnInputFrames());

#ifdef USE_PERFORMANCE_STATS
	                        pPerf->StopCount(PERF_FLIP_OVERLAY);
#endif
						}
                    }
                }                   
                __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
                { 
                    if(bOverlayLocked == TRUE)
                    {
#ifdef USE_PERFORMANCE_STATS
                        pPerf->StartCount(PERF_UNLOCK_OVERLAY);
#endif

                        GetActiveOutput()->Overlay_Unlock_Back_Buffer(bUseExtraBuffer);

#ifdef USE_PERFORMANCE_STATS
                        pPerf->StopCount(PERF_UNLOCK_OVERLAY);
#endif
                    }
                    ErrorBox("Crash in output code. Restart DScaler.");
					DScalerDeinitializeThread();
					return 1;
                }
            
                // Vertical flipping support
                // change the pictures back to how they were before
                // not that we use the bDoVerticalFlipCurrent
                // at the top and here, so this variable cannot change 
                // during the processing
                if (bDoVerticalFlipCurrent)
                {
                    // Use of a negative pitch + a pointer to the last line of the field
                    Info.InputPitch *= -1;

                    for(int i(0); i < MAX_PICTURE_HISTORY; ++i)
                    {
                        if(Info.PictureHistory[i] != NULL)
                        {
                            Info.PictureHistory[i]->pData -= (Info.FieldHeight-1) * Info.InputPitch;

                            // make the odd fields even and the even fields odd.
                            if(Info.PictureHistory[i]->Flags & PICTURE_INTERLACED_ODD)
                            {
                                Info.PictureHistory[i]->Flags &= ~PICTURE_INTERLACED_ODD;
                                Info.PictureHistory[i]->Flags |= PICTURE_INTERLACED_EVEN;
                            }
                            else if(Info.PictureHistory[i]->Flags & PICTURE_INTERLACED_EVEN)
                            {
                                Info.PictureHistory[i]->Flags &= ~PICTURE_INTERLACED_EVEN;
                                Info.PictureHistory[i]->Flags |= PICTURE_INTERLACED_ODD;
                            }
                        }
                    }
                }
            }
			else
			{
				// When taking a still, we always work with system memory for performance reasons
				if (bTakeStill)
				{
					// We need to copy the content of the overlay in the memory buffer

					BYTE* StillBuffer = Info.Overlay;
					int LinePitch = Info.OverlayPitch;

                    // make sure the lock is checked
                    // and paired
					if(GetActiveOutput()->Overlay_Lock(&Info))
					{
						BYTE* CurrentLine = Info.Overlay;
						BYTE* DestLine = StillBuffer;
						for (int i = 0; i < Info.FrameHeight; i++)
						{
							Info.pMemcpy(DestLine, CurrentLine, Info.LineLength);
							DestLine += LinePitch;
							CurrentLine += Info.OverlayPitch;
						}
						_asm
						{
							emms
						}
                        GetActiveOutput()->Overlay_Unlock();
					}
					else
					{
						bTakeStill = FALSE;
						if (pAllocBuf != NULL)
						{
							free(pAllocBuf);
						}
					}

					Info.Overlay = StillBuffer;
					Info.OverlayPitch = LinePitch;
				}
			}

			if (bTakeStill)
			{
				if (Request.param2)
				{
					((CStillSource*) Providers_GetSnapshotsSource())->SaveSnapshotInMemory(CurrentMethod->bIsHalfHeight ? Info.FieldHeight : Info.FrameHeight, Info.FrameWidth, pAllocBuf, Info.OverlayPitch);
				}
				else
				{
					((CStillSource*) Providers_GetSnapshotsSource())->SaveSnapshotInFile(CurrentMethod->bIsHalfHeight ? Info.FieldHeight : Info.FrameHeight, Info.FrameWidth, Info.Overlay, Info.OverlayPitch);
					if (pAllocBuf != NULL)
					{
						free(pAllocBuf);
					}
				}
				Request.param1--;
				if (Request.param1 <= 0)
				{
					if (Request.param2 && Setting_GetValue(Still_GetSetting(OSDFORSTILLS)))
					{
						OSD_ShowText("Still(s) in memory", 0);
					}
					Request.type = REQ_NONE;
				}
			}

            // if asked save the current Info to a file
            if(Request.type == REQ_SNAPSHOT)
            {
				SaveStreamSnapshot(&Info);
                Request.type = REQ_NONE;
            }

#ifdef WANT_DSHOW_SUPPORT
            // if request for changing capture resolution
            if(Request.type == REQ_DSHOW_CHANGERES)
            {
				((CDSCaptureSource*)Providers_GetCurrentSource())->ChangeRes(Request.param1);
                Request.type = REQ_NONE;
            }

            // if request for stopping the graph
            if(Request.type == REQ_DSHOW_STOP)
            {
				((CDSSourceBase*)Providers_GetCurrentSource())->StopAndSeekToBeginning();
                Request.type = REQ_NONE;
            }
#endif

			if (pMultiFrames && pMultiFrames->IsSwitchRequested())
			{
				pMultiFrames->HandleSwitch();
				if (!pMultiFrames->IsActive())
				{
					delete pMultiFrames;
					pMultiFrames = NULL;
				}
			}

            // save the last pulldown Mode so that we know if its changed
            PrevDeintMethod = CurrentMethod;
        }
        while(!bStopThread);

    }
    // if there is any exception thrown then exit the thread
    __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
    { 
        Providers_GetCurrentSource()->Stop();
		ErrorBox("Crash in OutThreads main loop. Restart DScaler.");

#ifdef WANT_DSHOW_SUPPORT
		//i don't know if it's a good idea or not to call CoUninitialize when there has been a crash
		CoUninitialize();
#endif
		DScalerDeinitializeThread();
        return 1;
    }

    // try and stop the capture
    __try
    {
        Providers_GetCurrentSource()->Stop();
    }
    // if there is any exception thrown then exit the thread
    __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
    {
		ErrorBox("Crash in in OutThreads Providers_GetCurrentSource()->Stop(). Restart DScaler.");
#ifdef WANT_DSHOW_SUPPORT
		CoUninitialize();
#endif
        DScalerDeinitializeThread();
        return 1;
    }
#ifdef WANT_DSHOW_SUPPORT
    //com deinit
    CoUninitialize();
#endif
    DScalerDeinitializeThread();
    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

extern int TunerSwitchScreenUpdateDelay; //Used in programlist.cpp, but affects g_hOutThread

SETTING OutThreadsSettings[OUTTHREADS_SETTING_LASTONE] =
{
    {
        "Hurry When Late", ONOFF, 0, (long*)&bHurryWhenLate,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "bHurryWhenLate", NULL,
    },
    {
        "Wait For Flip", ONOFF, 0, (long*)&WaitForFlip,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Threads", "WaitForFlip", NULL,
    },
    {
        "Vertical Mirror", ONOFF, 0, (long*)&bDoVerticalFlipSetting,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "DoVerticalFlip", NULL,
    },
    {
        "Tuner Switch Update Delay", SLIDER, 0, (long*)&TunerSwitchScreenUpdateDelay,
        120, 0, 1000, 1, 1,
        NULL,
        "Threads", "TunerSwitchScreenUpdateDelay", NULL,
    },
    {
        "JudderTerminator", ONOFF, 0, (long*)&DoAccurateFlips,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "DoAccurateFlips", NULL,
    },
    {
        "Autodetect Pulldown", ONOFF, 0, (long*)&bAutoDetectMode,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Pulldown", "bAutoDetectMode", NULL,
    },
    {
        "Do JudderTerminator On Video Modes", ONOFF, 0, (long*)&bJudderTerminatorOnVideo,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Timing", "DoJudderTerminatorOnVideo", NULL,
    },
};

SETTING* OutThreads_GetSetting(OUTTHREADS_SETTING Setting)
{
    if(Setting > -1 && Setting < OUTTHREADS_SETTING_LASTONE)
    {
        return &(OutThreadsSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void OutThreads_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < OUTTHREADS_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(OutThreadsSettings[i]));
    }
}

void OutThreads_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < OUTTHREADS_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(OutThreadsSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* OutThreads_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Decoding / Output Settings", OutThreadsSettings, OUTTHREADS_SETTING_LASTONE);
}

void OutThreads_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_CAPTURE_PAUSE, bIsPaused);
    CheckMenuItemBool(hMenu, IDM_AUTODETECT, bAutoDetectMode);
    CheckMenuItemBool(hMenu, IDM_JUDDERTERMINATOR, DoAccurateFlips);
}
