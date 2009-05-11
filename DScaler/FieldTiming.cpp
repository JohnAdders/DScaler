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

/**
 * @file FieldTiming.cpp Field Timing functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "Deinterlace.h"
#include "DScaler.h"
#include "Providers.h"
#include "SettingsMaster.h"

LARGE_INTEGER TimerFrequency;
HANDLE hTimerEvent;
UINT   wTimerRes;
double RunningAverageCounterTicks;
double StartAverageCounterTicks;
LARGE_INTEGER LastFieldTime;
LARGE_INTEGER CurrentFieldTime;
LARGE_INTEGER LastFlipTime;
LARGE_INTEGER CurrentFlipTime;
BOOL bIsPAL;
BOOL FlipAdjust;
int nDroppedFields = 0;
int nNotWaitedFields = 0;
int nLateFields = 0;
int nNoFlipAtTime = 0;
int nUsedFields = 0;
double Weight = 0.005;
BOOL bDoAutoFormatDetect = TRUE;
eVideoFormat FiftyHzFormat = VIDEOFORMAT_PAL_B;
eVideoFormat SixtyHzFormat = VIDEOFORMAT_NTSC_M;
long FormatChangeThreshold = 2;
long SleepInterval = 1;         // " , default=0, how long to wait for BT chip
long SleepSkipFields = 0;       // Number of fields to skip before doing sleep interval
long SleepSkipFieldsLate = 0;   // Number of fields to skip before doing sleep interval, when we're running late
long MaxFieldShift = 1;        // Maximum shift beween the last received field and the field to process
BOOL bAlwaysSleep = FALSE;    // Define if sleep must be called in all circonstances
BOOL bGiveUpProcDuringJT = FALSE;

void Timing_Setup()
{
    #define TARGET_RESOLUTION 3

    TIMECAPS tc;

    if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    {
        ErrorBox(_T("Failed to get Timer Resolution"));
    }

    wTimerRes = min(max(tc.wPeriodMin, TARGET_RESOLUTION), tc.wPeriodMax);
    timeBeginPeriod(wTimerRes);

    bIsPAL = GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps;

    // get the Frequency of the high resolution timer
    QueryPerformanceFrequency(&TimerFrequency);

    hTimerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(hTimerEvent == NULL)
    {
        ErrorBox(_T("Failed to create timing event"));
    }
    if(bIsPAL)
    {
        RunningAverageCounterTicks = (double)TimerFrequency.QuadPart / 25.0;
        StartAverageCounterTicks = RunningAverageCounterTicks;
    }
    else
    {
        RunningAverageCounterTicks = (double)TimerFrequency.QuadPart / 29.97;
        StartAverageCounterTicks = RunningAverageCounterTicks;
    }
    FlipAdjust = FALSE;
    Timing_Reset();
}

void Timing_CleanUp()
{
    if(hTimerEvent != NULL)
    {
        CloseHandle(hTimerEvent);
    }

    // reset the timer resolution
    timeEndPeriod(wTimerRes);
}


void Timing_UpdateRunningAverage(TDeinterlaceInfo* pInfo, int NumFields)
    {
    if(!(pInfo->bRunningLate))
    {
        LARGE_INTEGER CurrentFieldTime;
        QueryPerformanceCounter(&CurrentFieldTime);

        // if we're not running late then
        // and we got a good clean run last time
        if(LastFieldTime.QuadPart != 0)
        {
            // gets the last ticks odd - odd
            double RecentTicks = (double)((CurrentFieldTime.QuadPart - LastFieldTime.QuadPart) * 2.0 / (double)NumFields);
            // only allow values within 5% if current Value
            // should prevent spurious values getting through
            if(RecentTicks > RunningAverageCounterTicks * 0.95 &&
                RecentTicks < RunningAverageCounterTicks * 1.05)
            {
                // update the average
                // we're doing this weighted average because
                // it has lots of nice properties
                // especially that we don't need to keep a
                // data history
                RunningAverageCounterTicks = Weight * RecentTicks + (1.0 - Weight) * RunningAverageCounterTicks;
                LOG(2, _T(" Last %f"), RecentTicks);
                LOG(2, _T(" Running Average %f"), RunningAverageCounterTicks);
            }
            else
            {
                LOG(2, _T(" Last %f (IGNORED)"), RecentTicks);
                LOG(2, _T(" Old Running Average %f"), RunningAverageCounterTicks);
            }
        }
        // save current Value for next time
        LastFieldTime.QuadPart = CurrentFieldTime.QuadPart;
    }
    else
    {
        // if we're running late then
        // time will be rubbish
        // so make sure it won't be used
        LastFieldTime.QuadPart = 0;
    }
}

void Timing_SmartSleep(TDeinterlaceInfo* pInfo, BOOL bRunningLate, BOOL& bSleptAlready)
{
    if(bSleptAlready == FALSE || SleepSkipFields == 0)
    {
        static int nSleepSkipFields = 0;
        // Sleep less often if we're running late.

        // Increment sleep skipping counter, so we can sleep only every X fields specified by SleepSkipField
        if (bRunningLate)
        {
            // Sleep skipping whenever we're running late
            ++nSleepSkipFields;
            Sleep((nSleepSkipFields % (SleepSkipFieldsLate + 1)) ? 0 : SleepInterval);
        }
        else
        {
            // Sleep skipping whenever we're running on time
            ++nSleepSkipFields;
            Sleep((nSleepSkipFields % (SleepSkipFields + 1)) ? 0 : SleepInterval);
        }
    }
    bSleptAlready = TRUE;
}

void Timimg_AutoFormatDetect(TDeinterlaceInfo* pInfo, int NumFields)
{
    static long RepeatCount = 0;
    static LARGE_INTEGER LastTenFieldTime = {LONGLONG(0)};
    static long Counted = 0;
    static long LastFrame = 0;

    if(bDoAutoFormatDetect == TRUE)
    {
        if(pInfo->bRunningLate)
        {
            return;
        }

        if(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
        {
            Counted += ((pInfo->CurrentFrame * 2 - LastFrame * 2 + NumFields) % NumFields);
        }
        else if(!(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK))
        {
            Counted += (pInfo->CurrentFrame - LastFrame + NumFields) % NumFields;
        }
        else
        {
            return;
        }

        LastFrame = pInfo->CurrentFrame;

        if(Counted >= NumFields)
        {
            LARGE_INTEGER CurrentTime;
            QueryPerformanceCounter(&CurrentTime);
            if(LastTenFieldTime.QuadPart != 0)
            {
                long TenFieldTime;
                TenFieldTime = MulDiv((int)(CurrentTime.QuadPart - LastTenFieldTime.QuadPart), 10000, (int)(TimerFrequency.QuadPart) * Counted);

                // If we are not on a 50Hz Mode and we get 50hz timings then flip
                // to 50hz Mode
                if(TenFieldTime >= 195 && TenFieldTime <= 205)
                {
                    if(!(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps))
                    {
                        ++RepeatCount;
                        if(RepeatCount > FormatChangeThreshold)
                        {
                            Providers_GetCurrentSource()->SetFormat(FiftyHzFormat);
                            LOG(1, _T("Went to 50Hz Mode - Last Ten Count %d"), TenFieldTime);
                        }
                    }
                }
                // If we are not on a 60Hz Mode and we get 60hz timings then flip
                // to 60hz Mode, however this is not what we seem to get when playing
                // back 60Hz stuff when in PAL
                else if(TenFieldTime >= 160 && TenFieldTime <= 172)
                {
                    if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
                    {
                        ++RepeatCount;
                        if(RepeatCount > FormatChangeThreshold)
                        {
                            Providers_GetCurrentSource()->SetFormat(SixtyHzFormat);
                            LOG(1, _T("Went to 60Hz Mode - Last Ten Count %d"), TenFieldTime);
                        }
                    }
                }
                // If we are not on a 60Hz Mode and we get 60hz timings then flip
                // to 60hz Mode, in my tests I get 334 come back as the timings
                // when playing NTSC in PAL Mode so this check is also needed
                else if(TenFieldTime >= 330 && TenFieldTime <= 340)
                {
                   if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
                    {
                        ++RepeatCount;
                        if(RepeatCount > FormatChangeThreshold)
                        {
                            Providers_GetCurrentSource()->SetFormat(SixtyHzFormat);
                            LOG(1, _T("Went to 60Hz Mode - Last Ten Count %d"), TenFieldTime);
                        }
                    }
                }
                else
                {
                    LOG(2, _T("Got unexpected Last Ten Count %d"), TenFieldTime);
                    RepeatCount = 0;
                }
            }
            LastTenFieldTime.QuadPart = CurrentTime.QuadPart;
            Counted = 0;
        }
    }
}

void Timing_Reset()
{
    LastFieldTime.QuadPart = 0;
    CurrentFieldTime.QuadPart = 0;
    LastFlipTime.QuadPart = 0;
    CurrentFlipTime.QuadPart = 0;
}

void Timing_WaitForTimeToFlip(TDeinterlaceInfo* pInfo, DEINTERLACE_METHOD* CurrentMethod, BOOL* bStopThread)
{
    BOOL bWait = FALSE;

    if(pInfo->bMissedFrame == FALSE && FlipAdjust == FALSE)
    {
        if(LastFlipTime.QuadPart == 0)
        {
            QueryPerformanceCounter(&LastFlipTime);
        }
        else
        {
            LONGLONG TicksToWait;

            // work out the required ticks between flips
            if(bIsPAL)
            {
                TicksToWait = (LONGLONG)(RunningAverageCounterTicks * 25.0 / (double)CurrentMethod->FrameRate50Hz);
            }
            else
            {
                TicksToWait = (LONGLONG)(RunningAverageCounterTicks * 30.0 / (double)CurrentMethod->FrameRate60Hz);
            }
            QueryPerformanceCounter(&CurrentFlipTime);

            if(bGiveUpProcDuringJT == TRUE)
            {
                // see if we can afford to allow another process to cut in at this point
                // if we don't give up some time here we will hog the processor
                // this method is in test and should be used with an elevated priority on the
                // decoding thread so that when the event is fired out thread gets priority
                long MilliSecondsToWait = (long)((CurrentFlipTime.QuadPart - LastFlipTime.QuadPart) * 1000 / TimerFrequency.QuadPart);
                if(MilliSecondsToWait > wTimerRes * 2)
                {
                    ResetEvent(hTimerEvent);

                    MMRESULT wTimerID = timeSetEvent(
                                                    MilliSecondsToWait - wTimerRes,
                                                    wTimerRes,
                                                    (LPTIMECALLBACK)hTimerEvent,
                                                    (DWORD)NULL,
                                                    TIME_ONESHOT | TIME_CALLBACK_EVENT_SET
                                                 );
                    if(wTimerID != NULL)
                    {
                        // if this succeeded then wait for our event to be fired
                        // but allow for the timer to not work and fall out after
                        // 1/5 of a second.
                        // note we don't really care if this works or not
                        // as the downside is using all the CPU
                        WaitForSingleObject(hTimerEvent, 200);
                    }
                }
                QueryPerformanceCounter(&CurrentFlipTime);
                if((CurrentFlipTime.QuadPart - LastFlipTime.QuadPart) > TicksToWait)
                {
                    LOG(1, _T(" Too late back from mm timer - %d"), (long)(CurrentFlipTime.QuadPart - LastFlipTime.QuadPart));
                }
            }

            while(!(*bStopThread) && (CurrentFlipTime.QuadPart - LastFlipTime.QuadPart) < TicksToWait)
            {
                QueryPerformanceCounter(&CurrentFlipTime);
                bWait = TRUE;
            }
            LastFlipTime.QuadPart = CurrentFlipTime.QuadPart;
        }
    }
    else
    {
        QueryPerformanceCounter(&LastFlipTime);
    }
    if (!bWait)
    {
        Timing_IncrementNoFlipAtTime();
    }
    FlipAdjust = FALSE;
}

int Timing_GetDroppedFields()
{
    return nDroppedFields;
}

void Timing_ResetDroppedFields()
{
    nDroppedFields = 0;
}

int Timing_GetNotWaitedFields()
{
    return nNotWaitedFields;
}

void Timing_ResetNotWaitedFields()
{
    nNotWaitedFields = 0;
}

int Timing_GetLateFields()
{
    return nLateFields;
}

void Timing_ResetLateFields()
{
    nLateFields = 0;
}

int Timing_GetUsedFields()
{
    return nUsedFields;
}

void Timing_ResetUsedFields()
{
    nUsedFields = 0;
}

int Timing_GetNoFlipAtTime()
{
    return nNoFlipAtTime;
}

void Timing_ResetNoFlipAtTime()
{
    nNoFlipAtTime = 0;
}

void Timing_IncrementUsedFields()
{
    ++nUsedFields;
}

void Timing_AddDroppedFields(int nDropped)
{
    nDroppedFields += nDropped;
}

void Timing_IncrementNotWaitedFields()
{
    ++nNotWaitedFields;
}

void Timing_AddLateFields(int nLate)
{
    nLateFields += nLate;
}

void Timing_IncrementNoFlipAtTime()
{
    ++nNoFlipAtTime;
}

void Timing_SetFlipAdjustFlag(BOOL NewValue)
{
    FlipAdjust = NewValue;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING TimingSettings[TIMING_SETTING_LASTONE] =
{
    {
        "Auto Format Detect", ONOFF, 0, (LONG_PTR*)&bDoAutoFormatDetect,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Timing", "DoAutoFormatDetect", NULL,
    },
    {
        "50Hz Format", ITEMFROMLIST, 0, (LONG_PTR*)&FiftyHzFormat,
        VIDEOFORMAT_PAL_B, 0, VIDEOFORMAT_LASTONE - 1, 1, 1,
        VideoFormatNames,
        "Timing", "50Hz", NULL,
    },
    {
        "60Hz Format", ITEMFROMLIST, 0, (LONG_PTR*)&SixtyHzFormat,
        VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, 1, 1,
        VideoFormatNames,
        "Timing", "60Hz", NULL,
    },
    {
        "Format Change Threshold", SLIDER, 0, (LONG_PTR*)&FormatChangeThreshold,
        2, 0, 50, 1, 1,
        NULL,
        "Timing", "FormatChangeThreshold", NULL,
    },
    {
        "Sleep Interval", SLIDER, 0, (LONG_PTR*)&SleepInterval,
        1, 0, 100, 1, 1,
        NULL,
        "Threads", "SleepInterval", NULL,
    },
    {
        "Sleep Skip Fields", SLIDER, 0, (LONG_PTR*)&SleepSkipFields,
        0, 0, 60, 1, 1,
        NULL,
        "Threads", "SleepSkipFields", NULL,
    },
    {
        "Sleep Skip Fields Late", SLIDER, 0, (LONG_PTR*)&SleepSkipFieldsLate,
        0, 0, 60, 1, 1,
        NULL,
        "Threads", "SleepSkipFieldsLate", NULL,
    },
    {
        "Maximum shift between received and processed fields", SLIDER, 0, (LONG_PTR*)&MaxFieldShift,
        1, 0, 2, 1, 1,
        NULL,
        "Threads", "MaxFieldShift", NULL,
    },
    {
        "Always sleep even when late", ONOFF, 0, (LONG_PTR*)&bAlwaysSleep,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Timing", "AlwaysSleep", NULL,
    },
    {
        "Give Up processor during JudderTerminator", ONOFF, 0, (LONG_PTR*)&bGiveUpProcDuringJT,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Timing", "GiveUpProcDuringJT", NULL,
    },
};

SETTING* Timing_GetSetting(TIMING_SETTING Setting)
{
    if(Setting > -1 && Setting < TIMING_SETTING_LASTONE)
    {
        return &(TimingSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Timing_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_AUTO_FORMAT, bDoAutoFormatDetect);
}

