/////////////////////////////////////////////////////////////////////////////
// $Id: FieldTiming.cpp,v 1.35 2003-03-09 19:46:25 laurentg Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use TDeinterlaceInfo
//
// 17 Jun 2001   John Adcock           Added autoformat detection based on input
//                                     frequency
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.34  2003/03/08 20:01:26  laurentg
// New setting "always sleep"
//
// Revision 1.33  2003/02/26 20:53:31  laurentg
// New timing setting MaxFieldShift
//
// Revision 1.32  2003/02/22 13:36:36  laurentg
// New statistics to check fields runnign late and no flip at time
//
// Revision 1.31  2003/01/24 08:57:55  adcockj
// Fixed autodetect bug (fix by Laurent)
//
// Revision 1.30  2002/09/19 17:33:44  adcockj
// Made looging for format detect less noisy
//
// Revision 1.29  2002/09/16 20:08:21  adcockj
// fixed format detect for cx2388x
//
// Revision 1.28  2002/09/15 14:20:38  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.27  2002/08/26 18:25:10  adcockj
// Fixed problem with PAL/NTSC detection
//
// Revision 1.26  2002/06/13 12:10:22  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.25  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.24  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.23  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.22  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.21  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.20  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.16.2.2  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.16.2.1  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.16  2001/08/11 12:02:13  adcockj
// Updated SleepInterval default
//
// Revision 1.15  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.14  2001/07/28 13:24:40  adcockj
// Added UI for Overlay Controls and fixed issues with SettingsDlg
//
// Revision 1.13  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.12  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.11  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "Deinterlace.h"
#include "DScaler.h"
#include "Providers.h"

LARGE_INTEGER TimerFrequency;
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
long MaxFieldShift = 1;		// Maximum shift beween the last received field and the field to process
BOOL bAlwaysSleep = FALSE;	// Define if sleep must be called in all circonstances

void Timing_Setup()
{
    bIsPAL = GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps;

    // get the Frequency of the high resolution timer
    QueryPerformanceFrequency(&TimerFrequency);

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
                LOG(2, " Last %f", RecentTicks);
                LOG(2, " Running Average %f", RunningAverageCounterTicks);
            }
            else
            {
                LOG(2, " Last %f (IGNORED)", RecentTicks);
                LOG(2, " Old Running Average %f", RunningAverageCounterTicks);
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

    if(bDoAutoFormatDetect == TRUE)
    {
        if(pInfo->CurrentFrame == 0 && (((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0) || !(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK)))
        {
            LARGE_INTEGER CurrentTime;
            QueryPerformanceCounter(&CurrentTime);
            if(LastTenFieldTime.QuadPart != 0)
            {
                long TenFieldTime;
                TenFieldTime = MulDiv((int)(CurrentTime.QuadPart - LastTenFieldTime.QuadPart), 10000, (int)(TimerFrequency.QuadPart) * NumFields);

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
                            LOG(1, "Went to 50Hz Mode - Last Ten Count %d", TenFieldTime);
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
                            LOG(1, "Went to 60Hz Mode - Last Ten Count %d", TenFieldTime);
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
                            LOG(1, "Went to 60Hz Mode - Last Ten Count %d", TenFieldTime);
                        }
                    }
                }
                else
                {
                    LOG(2, "Got unexpected Last Ten Count %d", TenFieldTime);
                    RepeatCount = 0;
                }
            }
            LastTenFieldTime.QuadPart = CurrentTime.QuadPart;
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
        "Auto Format Detect", ONOFF, 0, (long*)&bDoAutoFormatDetect,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Timing", "DoAutoFormatDetect", NULL,
    },
    {
        "50Hz Format", ITEMFROMLIST, 0, (long*)&FiftyHzFormat,
        VIDEOFORMAT_PAL_B, 0, VIDEOFORMAT_LASTONE - 1, 1, 1,
        VideoFormatNames,
        "Timing", "50Hz", NULL,
    },
    {
        "60Hz Format", ITEMFROMLIST, 0, (long*)&SixtyHzFormat,
        VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, 1, 1,
        VideoFormatNames,
        "Timing", "60Hz", NULL,
    },
    {
        "Format Change Threshold", SLIDER, 0, (long*)&FormatChangeThreshold,
        2, 0, 50, 1, 1,
        NULL,
        "Timing", "FormatChangeThreshold", NULL,
    },
    {
        "Sleep Interval", SLIDER, 0, (long*)&SleepInterval,
        1, 0, 100, 1, 1,
        NULL,
        "Threads", "SleepInterval", NULL,
    },
    {
        "Sleep Skip Fields", SLIDER, 0, (long*)&SleepSkipFields,
        0, 0, 60, 1, 1,
        NULL,
        "Threads", "SleepSkipFields", NULL,
    },
    {
        "Sleep Skip Fields Late", SLIDER, 0, (long*)&SleepSkipFieldsLate,
        0, 0, 60, 1, 1,
        NULL,
        "Threads", "SleepSkipFieldsLate", NULL,
    },
    {
        "Maximum shift between received and processed fields", SLIDER, 0, (long*)&MaxFieldShift,
        1, 0, 2, 1, 1,
        NULL,
        "Threads", "MaxFieldShift", NULL,
    },
    {
        "Always sleep even when late", ONOFF, 0, (long*)&bAlwaysSleep,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Timing", "AlwaysSleep", NULL,
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

void Timing_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < TIMING_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(TimingSettings[i]));
    }
}

void Timing_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < TIMING_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(TimingSettings[i]), bOptimizeFileAccess);
    }
}

void Timing_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_AUTO_FORMAT, bDoAutoFormatDetect);
}

CTreeSettingsGeneric* Timing_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Field Timing Settings",TimingSettings, TIMING_SETTING_LASTONE);
}

