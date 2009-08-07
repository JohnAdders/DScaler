/////////////////////////////////////////////////////////////////////////////
// $Id$
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
#include "SettingsMaster.h"
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
#include "ComInitialise.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dshowsource\DSSource.h"
#endif

// Thread related variables
BOOL                bStopThread = FALSE;
BOOL                bIsPaused = FALSE;
BOOL                RequestToggleFlip = FALSE;
BOOL                bCheckSignalPresent = FALSE;
BOOL                bCheckSignalMissing = FALSE;
BOOL                bDoVerticalFlipSetting = FALSE;
HANDLE              g_hOutThread;
DWORD OutThreadID=0;
TGUIRequest            Request = { REQ_NONE, 0, 0 };

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
#ifndef _M_AMD64
            //the reason for using int 3 insted of _ASSERTE() is that the _ASSERTE
            //macro shows a dialog which might create problems.
            _asm int 3;
#endif
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
            LOG(3,_T("Timeout waiting for YUVOutThread to exit, terminating it via TerminateThread()"));
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
                _T("The video thread failed to exit in a timely manner and was forcefully ")
                _T("terminated.  You may experience further problems."),
                _T("Unexpected Error"), MB_OK);
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
            if ((req->param1 > 0) && Providers_GetSnapshotsSource())
            {
                Request.type = req->type;
                Request.param1 = req->param1;
                if (req->param1 > 1)
                {
                    Request.param2 = 1;
                }
                else
                {
                    Request.param2 = (Setting_GetValue(WM_STILL_GETVALUE, STILLSINMEMORY)) == TRUE ? 1 : 0;
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
    TCHAR name[13];
    int n = 0;
    int i = 0;
    int j;
    struct _stat st;

    while (n < 100)
    {
        _sntprintf(name, 13,  _T("sn%06d.dtv"),++n) ;
        if (_tstat(name, &st))
        {
            break;
        }
    }

    if(n == 100)
    {
        ErrorBox(_T("Could not create a file.  You may have too many snapshots already."));
        return;
    }

    FILE* file = _tfopen(name, _T("wb"));
    if (!file)
    {
        ErrorBox(_T("Could not open file in SaveStreamSnapshot"));
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
        LOG(1 , "Before Overlay Clean");
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
            LOG(1 , "Before Aspect Ratio");
            Providers_GetCurrentSource()->SetAspectRatioData();
            WorkoutOverlaySize(TRUE);
        }

        if (Providers_GetCurrentSource())
        {
            LOG(1 , "Before Current Source Start");
            Providers_GetCurrentSource()->Start();
            PrepareDeinterlaceMode();
            LOG(1 , "Before Start_Thread");
            Start_Thread();
        }

        LOG(1 , "Before Unmute");
        Audio_Unmute();
    }
    LOG(1 , "End of Start Thread");
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

void SetOutputThreadProcessor()
{
    DWORD_PTR rc;
    int ProcessorMask;

    if(!g_hOutThread)
    {
        return;
    }
    ProcessorMask = 1 << (DecodeProcessor);
    rc = SetThreadAffinityMask(g_hOutThread, ProcessorMask);
}

void SetOutputThreadPriority()
{
    if(!g_hOutThread)
    {
        return;
    }

    if (ThreadClassId == 0)
        SetThreadPriority(g_hOutThread, THREAD_PRIORITY_BELOW_NORMAL);
    else if (ThreadClassId == 1)
        SetThreadPriority(g_hOutThread, THREAD_PRIORITY_NORMAL);
    else if (ThreadClassId == 2)
        SetThreadPriority(g_hOutThread, THREAD_PRIORITY_ABOVE_NORMAL);
    else if (ThreadClassId == 3)
        SetThreadPriority(g_hOutThread, THREAD_PRIORITY_HIGHEST);
    else if (ThreadClassId == 4)
        SetThreadPriority(g_hOutThread, THREAD_PRIORITY_TIME_CRITICAL);
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

    DScalerThread thisThread(_T("YUV Out Thread"));

#ifdef WANT_DSHOW_SUPPORT
    //com init for this thread
    ComInitialise ThisThread(COINIT_MULTITHREADED);
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
    try
    {
        bUseExtraBuffer = Filter_WillWeDoOutput()
                       || (pMultiFrames && pMultiFrames->IsActive())
                       || (TimeShiftIsRunning() && !TimeShiftWorkOnInputFrames());


        // Anti-plop and update screen delay timers may have been cancelled.
        // Reset to default values
        bNoScreenUpdateDuringTuning = FALSE;

        // Sets processor Affinity and Thread priority according to menu selection
        SetOutputThreadProcessor();
        SetOutputThreadPriority();

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
            Request.type = REQ_NONE;
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
                LOG(2, _T("    Info.bMissedFrame %d - Info.bRunningLate %d"), Info.bMissedFrame, Info.bRunningLate);
#ifdef USE_PERFORMANCE_STATS
                for (int i = 0 ; i < PERF_TYPE_LASTONE ; ++i)
                {
                    if (pPerf->IsValid((ePerfType)i) && (pPerf->GetDurationLastCycle((ePerfType)i) != -1))
                    {
                        LOG(2, _T("    %s : %d (avg %d)"),
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
                LOG(2, _T("MemUsed %d Mo"), MemUsed / 1048576);

                // Check that the max is not reached
                if ((MemUsed / 1048576) >= Setting_GetValue(WM_STILL_GETVALUE, MAXMEMFORSTILLS))
                {
                    TCHAR text[128];
                    pAllocBuf = NULL;
                    _sntprintf(text, 128, _T("Max memory (%d Mo) reached\nChange the maximum value or\nclose some open stills"), Setting_GetValue(WM_STILL_GETVALUE, MAXMEMFORSTILLS));
                    OSD_ShowText(text, 0);
                }
                else
                {
                    pAllocBuf = (BYTE*)malloc(Info.OverlayPitch * Info.FrameHeight + 16);
                    Info.Overlay = START_ALIGNED16(pAllocBuf);
                }
                LOG(2, _T("Alloc for still - start buf %d, start frame %d"), pAllocBuf, Info.Overlay);
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
                // Note that we don't _T("own") these pictures but we have them for the time being
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

                if (!bMinimized && !bNoScreenUpdateDuringTuning &&
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
                        if(bAutoDetectMode == TRUE && !Info.bMissedFrame)
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

                        if(bAutoDetectMode == TRUE && !Info.bMissedFrame)
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

                try
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
                                LOG(1, _T("Falling out after Overlay_Lock_Back_Buffer"));
                                PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_STOP, 0);
                                PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_START, 0);
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
                        CHECK_FPU_STACK;

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
                                LOG(1, _T("Falling out after Overlay_Unlock_Back_Buffer"));
                                PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_STOP, 0);
                                PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_START, 0);
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
                                LOG(1, _T("Falling out after Overlay_Flip"));
                                PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_STOP, 0);
                                PostMessageToMainWindow(WM_COMMAND, IDM_OVERLAY_START, 0);
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
                catch(...)
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
                    ErrorBox(_T("Crash in output code. Restart DScaler."));
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
                        DO_EMMS;
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
                    if (Request.param2 && Setting_GetValue(WM_STILL_GETVALUE, OSDFORSTILLS))
                    {
                        OSD_ShowText(_T("Still(s) in memory"), 0);
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
    catch(...)
    {
        Providers_GetCurrentSource()->Stop();
        ErrorBox(_T("Crash in OutThreads main loop. Restart DScaler."));
        return 1;
    }

    // try and stop the capture
    try
    {
        Providers_GetCurrentSource()->Stop();
    }
    // if there is any exception thrown then exit the thread
    catch(...)
    {
        ErrorBox(_T("Crash in in OutThreads Providers_GetCurrentSource()->Stop(). Restart DScaler."));
        return 1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

extern int TunerSwitchScreenUpdateDelay; //Used in programlist.cpp, but affects g_hOutThread

SETTING OutThreadsSettings[OUTTHREADS_SETTING_LASTONE] =
{
    {
        "Hurry When Late", ONOFF, 0, (LONG_PTR*)&bHurryWhenLate,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "bHurryWhenLate", NULL,
    },
    {
        "Wait For Flip", ONOFF, 0, (LONG_PTR*)&WaitForFlip,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Threads", "WaitForFlip", NULL,
    },
    {
        "Vertical Mirror", ONOFF, 0, (LONG_PTR*)&bDoVerticalFlipSetting,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "DoVerticalFlip", NULL,
    },
    {
        "Tuner Switch Update Delay", SLIDER, 0, (LONG_PTR*)&TunerSwitchScreenUpdateDelay,
        120, 0, 1000, 1, 1,
        NULL,
        "Threads", "TunerSwitchScreenUpdateDelay", NULL,
    },
    {
        "JudderTerminator", ONOFF, 0, (LONG_PTR*)&DoAccurateFlips,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Threads", "DoAccurateFlips", NULL,
    },
    {
        "Autodetect Pulldown", ONOFF, 0, (LONG_PTR*)&bAutoDetectMode,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Pulldown", "bAutoDetectMode", NULL,
    },
    {
        "Do JudderTerminator On Video Modes", ONOFF, 0, (LONG_PTR*)&bJudderTerminatorOnVideo,
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

void OutThreads_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_CAPTURE_PAUSE, bIsPaused);
    CheckMenuItemBool(hMenu, IDM_AUTODETECT, bAutoDetectMode);
    CheckMenuItemBool(hMenu, IDM_JUDDERTERMINATOR, DoAccurateFlips);
}
