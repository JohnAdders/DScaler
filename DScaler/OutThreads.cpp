/////////////////////////////////////////////////////////////////////////////
// $Id: OutThreads.cpp,v 1.67 2002-06-05 20:53:49 adcockj Exp $
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

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "OutThreads.h"
#include "Other.h"
#include "VBI_VideoText.h"
#include "VBI.h"
#include "Deinterlace.h"
#include "AspectRatio.h"
#include "DScaler.h"
#include "DebugLog.h"
#include "VBI.h"
#include "Settings.h"
#include "Filter.h"
#include "Status.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
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

typedef enum
{
    STILL_NONE,
    STILL_TIFF,
    STILL_SNAPSHOT,
} eStreamStillType;

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
eStreamStillType    RequestStillType = STILL_NONE;
HANDLE              OutThread;

// Dynamically updated variables
BOOL                bAutoDetectMode = TRUE;


// TRB 10/28/00 changes, parms, and new fields for sync problem fixes
BOOL                WaitForFlip = TRUE;       // User parm, default=TRUE
BOOL                DoAccurateFlips = TRUE;     // User parm, default=TRUE
BOOL                bHurryWhenLate = FALSE;    // " , default=FALSE, skip processing if behind
BOOL                bIsOddField = FALSE;
BOOL                bWaitForVsync = FALSE;
BOOL                bReversePolarity = FALSE;
BOOL                bJudderTerminatorOnVideo = TRUE;

/// \todo should be able to get of this variable
long                OverlayPitch = 0;

// cope with older DX header files
#if !defined(DDFLIP_DONOTWAIT)
    #define DDFLIP_DONOTWAIT 0
#endif

///////////////////////////////////////////////////////////////////////////////
void Start_Thread()
{
    DWORD LinkThreadID;

    // make sure we start with a clean sheet of paper
    Overlay_Clean();

    bStopThread = FALSE;

    OutThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,  // No security.
                             (DWORD) 0,                     // Same stack size.
                             YUVOutThread,                  // Thread procedure.
                             NULL,                          // Parameter.
                             (DWORD) 0,                     // Start immediatly.
                             (LPDWORD) & LinkThreadID);     // Thread ID.
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Thread()
{
    DWORD ExitCode;
    int i;
    BOOL Thread_Stopped = FALSE;

    if (OutThread != NULL)
    {
        i = 10;
        SetThreadPriority(OutThread, THREAD_PRIORITY_NORMAL);
        bStopThread = TRUE;
        while(i-- > 0 && !Thread_Stopped)
        {
            if (GetExitCodeThread(OutThread, &ExitCode) == TRUE)
            {
                if (ExitCode != STILL_ACTIVE)
                {
                    Thread_Stopped = TRUE;
                }
                else
                {
                    Sleep(100);
                }
            }
            else
            {
                Thread_Stopped = TRUE;
            }
        }

        if (Thread_Stopped == FALSE)
        {
            TerminateThread(OutThread, 0);
            Sleep(100);
        }
        CloseHandle(OutThread);
        OutThread = NULL;
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

void RequestStreamSnap()
{
   RequestStillType = STILL_SNAPSHOT;
}

void RequestStill()
{
   RequestStillType = STILL_TIFF;
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
    }
    else
    {
        Pause_Capture();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Start_Capture()
{
    // ame sure half height Modes are set correctly
    Overlay_Clean();
    if (Providers_GetCurrentSource())
    {
        PrepareDeinterlaceMode();
        Start_Thread();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Stop_Capture()
{
    if (pCalibration->IsRunning())
    {
        pCalibration->Stop();
    }
    if (Providers_GetCurrentSource())
    {
        //  Stop The Output Thread
        Stop_Thread();
        Providers_GetCurrentSource()->Stop();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Reset_Capture()
{
    if (!Providers_GetCurrentSource())
        return;
    Stop_Capture();
    Overlay_Clean();
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
        pSource->SetOverscan();
        pSource->Start();

        // Sets processor Affinity and Thread priority according to menu selection
        SetThreadProcessorAndPriority();

        PrevDeintMethod = GetCurrentDeintMethod();

        // reset the static variables in the detection code
        if (bIsPAL)
            UpdatePALPulldownMode(NULL);
        else
            UpdateNTSCPulldownMode(NULL);

        while(!bStopThread)
        {
            pPerf->StartCount(PERF_WAIT_FIELD);

            // update with any changes
            CurrentMethod = GetCurrentDeintMethod();
            Info.bDoAccurateFlips = DoAccurateFlips;
            Info.bRunningLate = bHurryWhenLate;
            Info.bMissedFrame = FALSE;
            Info.OverlayPitch = OverlayPitch;
            Info.CombFactor = -1;
            Info.FieldDiff = -1;
            bFlipNow = FALSE;
            GetDestRect(&Info.DestRect);
            
            pSource->GetNextField(&Info, Info.bDoAccurateFlips && (IsFilmMode() || bJudderTerminatorOnVideo));

            pPerf->StopCount(PERF_WAIT_FIELD);

            if (Info.bMissedFrame || Info.bRunningLate)
            {
                LOG(2, "    Info.bMissedFrame %d - Info.bRunningLate %d", Info.bMissedFrame, Info.bRunningLate);
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
            }

            pPerf->InitCycle();

            if(bIsPaused == FALSE)
            {
                if(Info.PictureHistory[0] == NULL)
                {
                    Sleep(50);
                    continue;
                }
                // calculate History
                if(Info.PictureHistory[1] == NULL)
                {
                    nHistory = 1;
                }
                else if(Info.PictureHistory[2] == NULL)
                {
                    nHistory = 2;
                }
                else if(Info.PictureHistory[3] == NULL)
                {
                    nHistory = 3;
                }
                else
                {
                    nHistory = 4;
                }
                
                // Card calibration
				if (pCalibration->IsRunning())
				{
                    pPerf->StartCount(PERF_CALIBRATION);

    				pCalibration->Make(&Info, GetTickCount());

                    pPerf->StopCount(PERF_CALIBRATION);
				}

                // update the source area
                GetSourceRect(&Info.SourceRect);
                
                pPerf->StartCount(PERF_INPUT_FILTERS);

                // do any filters that operarate on the input
                // only
                SourceAspectAdjust = Filter_DoInput(&Info, nHistory, (Info.bRunningLate || Info.bMissedFrame));

                pPerf->StopCount(PERF_INPUT_FILTERS);

                pPerf->StartCount(PERF_TIMESHIFT);

                // NOTE: I might go ahead and make the TimeShift module an input
                // filter at some point (i.e. FLT_TimeShift), but I'm not sure
                // if that's the right thing to do since it ties into other
                // parts of the app.
                CTimeShift::OnNewFrame(&Info);

                pPerf->StopCount(PERF_TIMESHIFT);

                if(!Info.bMissedFrame && VTState != VT_BLACK)
                {
                    pPerf->StartCount(PERF_PULLDOWN_DETECT);

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
                        if (SetProgressiveMode())
                        {
                            CurrentMethod = GetCurrentDeintMethod();
                        }
                    }

                    pPerf->StopCount(PERF_PULLDOWN_DETECT);
                }

                if (bCaptureVBI == TRUE)
                {
                    pPerf->StartCount(PERF_VBI);

                    pSource->DecodeVBI(&Info);

                    pPerf->StopCount(PERF_VBI);
                }

                // do we need to unlock overlay if we crash
                BOOL bOverlayLocked = FALSE;

                __try
                {
                    if (!Info.bRunningLate  && VTState != VT_BLACK)
                    {
                        BOOL bFlipNow = FALSE;

                        // do the aspect ratio only every other frame
                        // also do this outside of the internal lock
                        // to avoid conflicts
                        if (Info.PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
                        {
                            pPerf->StartCount(PERF_RATIO);

                            AdjustAspectRatio(SourceAspectAdjust, &Info);

                            pPerf->StopCount(PERF_RATIO);
                        }

                        pPerf->StartCount(PERF_LOCK_OVERLAY);

                        if(!Overlay_Lock_Back_Buffer(&Info))
                        {
                            Providers_GetCurrentSource()->Stop();
                            LOG(1, "Falling out after Overlay_Lock_Back_Buffer");
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
                            ExitThread(1);
                            return 1;
                        }
                        bOverlayLocked = TRUE;

                        pPerf->StopCount(PERF_LOCK_OVERLAY);

                        pPerf->StartCount(PERF_DEINTERLACE);

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
                    
                        pPerf->StopCount(PERF_DEINTERLACE);

                        if (bFlipNow)
                        {
                            pPerf->StartCount(PERF_OUTPUT_FILTERS);

                            // Do any filters that run on the output
                            // need to do this while the surface is locked
                            Filter_DoOutput(&Info, nHistory, Info.bMissedFrame);

                            pPerf->StopCount(PERF_OUTPUT_FILTERS);
                        }

                        pPerf->StartCount(PERF_UNLOCK_OVERLAY);

                        // somewhere above we will have locked the buffer, unlock before flip
                        if(!Overlay_Unlock_Back_Buffer())
                        {
                            Providers_GetCurrentSource()->Stop();
                            LOG(1, "Falling out after Overlay_Unlock_Back_Buffer");
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
                            PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
                            ExitThread(1);
                            return 0;
                        }
                        bOverlayLocked = FALSE;

                        pPerf->StopCount(PERF_UNLOCK_OVERLAY);

                        // flip if required
                        if (bFlipNow)
                        {
                            pPerf->StartCount(PERF_FLIP_OVERLAY);

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

                            // Need to wait for a good time to flip
                            // only if we have been in the same Mode for at least one flip
                            if(Info.bDoAccurateFlips && PrevDeintMethod == CurrentMethod)
                            {
                                Timing_WaitForTimeToFlip(&Info, CurrentMethod, &bStopThread);
                            }

                            if(!Overlay_Flip(FlipFlag))
                            {
                                Providers_GetCurrentSource()->Stop();
                                LOG(1, "Falling out after Overlay_Flip");
                                PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_STOP, 0);
                                PostMessage(hWnd, WM_COMMAND, IDM_OVERLAY_START, 0);
                                ExitThread(1);
                                return 0;
                            }

                            pPerf->StopCount(PERF_FLIP_OVERLAY);
                        }
                    }
                }                   
                // if there is any exception thrown in the above then just carry on
                __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
                { 
                    if(bOverlayLocked == TRUE)
                    {
                        pPerf->StartCount(PERF_UNLOCK_OVERLAY);

                        Overlay_Unlock_Back_Buffer();

                        pPerf->StopCount(PERF_UNLOCK_OVERLAY);
                    }
                    LOG(1, "Crash in output code");
                }
            }

            // if asked save the current Info to a file
            if(RequestStillType == STILL_NONE)
            {
                ; // carry on
            }
            else if(RequestStillType == STILL_SNAPSHOT)
            {
                SaveStreamSnapshot(&Info);
                RequestStillType = STILL_NONE;
            }
            else if(RequestStillType == STILL_TIFF)
            {
                StillProvider_SaveSnapshot(&Info);
                RequestStillType = STILL_NONE;
            }

            // save the last pulldown Mode so that we know if its changed
            PrevDeintMethod = CurrentMethod;
        }
    }
    // if there is any exception thrown then exit the thread
    __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
    { 
        Providers_GetCurrentSource()->Stop();
        LOG(1, "Crash in OutThreads main loop");
        ExitThread(1);
        return 0;
    }

    // try and stop the capture
    __try
    {
        Providers_GetCurrentSource()->Stop();
    }
    // if there is any exception thrown then exit the thread
    __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info())) 
    { 
        LOG(1, "Crash in in OutThreads Providers_GetCurrentSource()->Stop()");
        ExitThread(1);
        return 0;
    }

#ifdef WANT_DSHOW_SUPPORT
	//com deinit
	CoUninitialize();
#endif

    ExitThread(0);
    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING OutThreadsSettings[OUTTHREADS_SETTING_LASTONE] =
{
    {
        "Hurry When Late", ONOFF, 0, (long*)&bHurryWhenLate,
        TRUE, 0, 1, 1, 1,
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
        "Wait For VSync", ONOFF, 0, (long*)&bWaitForVsync,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "bWaitForVsync", NULL,
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

void OutThreads_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_CAPTURE_PAUSE, bIsPaused);
    CheckMenuItemBool(hMenu, IDM_AUTODETECT, bAutoDetectMode);
    CheckMenuItemBool(hMenu, IDM_JUDDERTERMINATOR, DoAccurateFlips);
}
