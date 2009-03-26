/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
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
 * @file FD_50Hz.cpp PAL film mode functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "DebugLog.h"
#include "SettingsMaster.h"

long PALFilmFallbackIndex = INDEX_VIDEO_2FRAME;
long PALBadCadenceIndex = INDEX_VIDEO_GREEDY;
// Default values which can be overwritten by the INI file
long PulldownThresholdLow = 30;
long PulldownThresholdHigh = 10;
long PALPulldownRepeatCount = 3;
long PALPulldownRepeatCount2 = 1;
long MaxCallsToPALComb = 20;

long MovementThreshold = 100;
long CombThreshold = 150;
extern BOOL bFallbackToVideo;
extern long ThresholdPulldownMismatch;
extern long ThresholdPulldownComb;

///////////////////////////////////////////////////////////////////////////////
// UpdatePALPulldownMode
//
// This is the 2:2 pulldown detection for PAL.
///////////////////////////////////////////////////////////////////////////////
void UpdatePALPulldownMode(TDeinterlaceInfo* pInfo)
{
    static long LastCombFactor = 0;
    static long RepeatCount;
    static long LastPolarity = -1;
    static double LastDiff = 0;
    static DWORD StartFilmTicks = 0;
    double PercentDecrease = 0;
    double PercentIncrease = 0;
    static long FieldsSinceLastChange = 0;
    static long PrivateRepeatCount = PALPulldownRepeatCount;
    static long NotSureCount = 0;
    static BOOL NeedToCheckComb = FALSE;

    // call with pInfo as NULL to reset static variables when we start the thread
    // each time
    if(pInfo == NULL)
    {
        LastCombFactor = 0;
        RepeatCount = 0;
        LastPolarity = -1;
        LastDiff = 0;
        ResetModeSwitches();
        StartFilmTicks = 0;
        FieldsSinceLastChange = 0;
        PrivateRepeatCount = PALPulldownRepeatCount;
        NotSureCount = 0;
        NeedToCheckComb = FALSE;
        return;
    }

    if(pInfo->PictureHistory[0] == NULL)
    {
        return;
    }

    BOOL IsOdd = ((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);

    if(pInfo->CombFactor < 0 || pInfo->FieldDiff < 0)
    {
        return;
    }

    if(FieldsSinceLastChange < 0x7FFF)
    {
        ++FieldsSinceLastChange;
    }

    if(NeedToCheckComb == TRUE)
    {
        if(!IsFilmMode())
        {
            RepeatCount = 0;
            NeedToCheckComb = FALSE;
        }
        else if(GetFilmMode() != FILM_22_PULLDOWN_COMB)
        {
            NeedToCheckComb = FALSE;
        }
    }

    PercentDecrease = ((double)pInfo->CombFactor * 100.0) / ((double)LastCombFactor + 100.0);
    PercentIncrease = ((double)(pInfo->CombFactor - LastCombFactor) * 100.0) / ((double)LastCombFactor + 100.0);

    if(!IsFilmMode() || GetFilmMode() == FILM_22_PULLDOWN_COMB)
    {
        if(PercentDecrease < PulldownThresholdLow &&
            LastDiff > PulldownThresholdLow &&
            pInfo->FieldDiff > MovementThreshold &&
            LastCombFactor > CombThreshold)
        {
            if(LastPolarity == IsOdd)
            {
                if(RepeatCount < PrivateRepeatCount)
                {
                    if(RepeatCount == 0)
                    {
                        RepeatCount = 1;
                    }
                    else
                    {
                        if((GetTickCount() - StartFilmTicks) < 100)
                        {
                            LOG(2, "Upped RepeatCount %d Gap %d", RepeatCount, (GetTickCount() - StartFilmTicks));
                            RepeatCount++;
                        }
                        else
                        {
                            LOG(2, "Reset RepeatCount - Gap %d Too long", (GetTickCount() - StartFilmTicks));
                            RepeatCount = 1;
                        }
                    }
                }
                else
                {
                    if(FieldsSinceLastChange > 100 || (GetFilmMode() == FILM_22_PULLDOWN_COMB))
                    {
                        if(IsOdd == TRUE)
                        {
                            SetFilmDeinterlaceMode(FILM_22_PULLDOWN_ODD);
                            LOG(2, "Gone to Odd");
                        }
                        if(IsOdd == FALSE)
                        {
                            SetFilmDeinterlaceMode(FILM_22_PULLDOWN_EVEN);
                            LOG(2, "Gone to Even");
                        }
                        PrivateRepeatCount = PALPulldownRepeatCount;
                        NotSureCount = 0;
                        FieldsSinceLastChange = 0;
                    }
                }
            }
            else
            {
                LastPolarity = IsOdd;
                RepeatCount = 1;
                LOG(2, "Reset RepeatCount %d", RepeatCount);
            }
            StartFilmTicks = GetTickCount();
        }
    }
    else
    {
        // we are in a real 2:2 film Mode but we need to check that nothing
        // bad has happened

        // if we are about to flip
        // check that if there is movement that there is not too much combing
        // going on
        if(pInfo->FieldDiff >= ThresholdPulldownMismatch && // only force video if this field is very different,
            LastPolarity == IsOdd &&
            pInfo->CombFactor > (LastCombFactor + ThresholdPulldownComb) &&   // and it'd produce artifacts
            pInfo->CombFactor > ThresholdPulldownComb)
        {
            if(bFallbackToVideo)
            {
                SetVideoDeinterlaceIndex(PALFilmFallbackIndex);
                LOG(2, "Gone back to video");
                RepeatCount = 0;
            }
            else
            {
                if(FieldsSinceLastChange < 100)
                {
                    SetVideoDeinterlaceIndex(PALFilmFallbackIndex);
                    FieldsSinceLastChange = 0;
                    PrivateRepeatCount = PALPulldownRepeatCount * 4;
                    LOG(2, "Changes too fast go back to video and make it harder");
                    RepeatCount = 0;
                }
                else
                {
                    // Reset the paramters of the Comb method
                    FilmModePALComb(NULL);
                    SetFilmDeinterlaceMode(FILM_22_PULLDOWN_COMB);
                    LOG(2, "Gone to Comb Mode");
                    RepeatCount = PrivateRepeatCount;
                    NeedToCheckComb = TRUE;
                }
            }
        }
        if(LastPolarity == IsOdd)
        {
            if(pInfo->FieldDiff >= ThresholdPulldownMismatch)
            {
                if(pInfo->CombFactor < LastCombFactor)
                {
                    if(pInfo->CombFactor + ThresholdPulldownComb < LastCombFactor)
                    {
                        if(NotSureCount > 0)
                        {
                            LOG(2, "Reset not sure Count");
                            NotSureCount = 0;
                        }
                    }
                    else
                    {
                        if(NotSureCount > 0)
                        {
                            --NotSureCount;
                            LOG(2, "Decreased not sure Count %d", NotSureCount);
                        }
                    }
                }
                else
                {
                    ++NotSureCount;
                    if(NotSureCount > 3)
                    {
                        if(bFallbackToVideo)
                        {
                            SetVideoDeinterlaceIndex(PALFilmFallbackIndex);
                            LOG(2, "Gone back to because we're not sure");
                            RepeatCount = 0;
                        }
                        else
                        {
                            if(FieldsSinceLastChange < 100)
                            {
                                SetVideoDeinterlaceIndex(PALFilmFallbackIndex);
                                FieldsSinceLastChange = 0;
                                PrivateRepeatCount = PALPulldownRepeatCount * 4;
                                LOG(2, "Changes too fast go back to video and make it harder because we're not sure");
                                RepeatCount = 0;
                            }
                            else
                            {
                                // Reset the paramters of the Comb method
                                FilmModePALComb(NULL);
                                SetFilmDeinterlaceMode(FILM_22_PULLDOWN_COMB);
                                LOG(2, "Gone to Comb Mode because we're not sure");
                                RepeatCount = PrivateRepeatCount;
                                NeedToCheckComb = TRUE;
                            }
                        }
                    }
                    else
                    {
                        LOG(2, "Increased not sure Count %d", NotSureCount);
                    }
                }
            }
        }
    }

    LastDiff = PercentDecrease;
    LastCombFactor = pInfo->CombFactor;
}

BOOL FlipPALOdd(int CurrentFrame, BOOL bIsOdd)
{
    return bIsOdd;
}

BOOL FlipPALEven(int CurrentFrame, BOOL bIsOdd)
{
    return !bIsOdd;
}

BOOL FilmModePALOdd(TDeinterlaceInfo* pInfo)
{
    return SimpleFilmMode(pInfo, FlipPALOdd);
}

BOOL FilmModePALEven(TDeinterlaceInfo* pInfo)
{
    return SimpleFilmMode(pInfo, FlipPALEven);
}

BOOL FilmModePALComb(TDeinterlaceInfo* pInfo)
{
    static long NumCalls = 0;
    DEINTERLACE_METHOD* DeintMethod;

    if(pInfo == NULL)
    {
        NumCalls = 0;
        return FALSE;
    }

    ++NumCalls;
    if(NumCalls > MaxCallsToPALComb)
    {
        SetVideoDeinterlaceIndex(PALFilmFallbackIndex);
        LOG(2, "Gone back to video from Comb Mode");
    }
    DeintMethod = GetVideoDeintIndex(PALBadCadenceIndex);
    if(DeintMethod != NULL && DeintMethod->nMethodIndex == PALBadCadenceIndex)
    {
        return DeintMethod->pfnAlgorithm(pInfo);
    }
    else
    {
        return Bob(pInfo);
    }
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FD50Settings[FD50_SETTING_LASTONE] =
{
    {
        "Pulldown Threshold Low", SLIDER, 0, &PulldownThresholdLow,
        40, 0, 100, 2, 1,
        NULL,
        "Pulldown", "PulldownThresholdLow", NULL,
    },
    {
        "Pulldown Threshold High", SLIDER, 0, &PulldownThresholdHigh,
        10, 0, 200, 2, 1,
        NULL,
        "Pulldown", "PulldownThresholdHigh", NULL,
    },
    {
        "PAL Video Mode", ITEMFROMLIST, 0, &PALFilmFallbackIndex,
        INDEX_VIDEO_2FRAME, 0, 99, 1, 1,
        DeinterlaceNames,
        "Pulldown", "PALFilmFallbackMode", NULL,
    },
    {
        "PAL Pulldown Repeat Count In", SLIDER, 0, &PALPulldownRepeatCount,
        3, 1, 10, 1, 1,
        NULL,
        "Pulldown", "PALPulldownRepeatCount", NULL,
    },
    {
        "PAL Pulldown Repeat Count Out", SLIDER, 0, &PALPulldownRepeatCount2,
        1, 1, 10, 1, 1,
        NULL,
        "Pulldown", "PALPulldownRepeatCount2", NULL,
    },
    {
        "Max Calls to PAL Comb Method", SLIDER, 0, (long*)&MaxCallsToPALComb,
        20, 0, 1000, 10, 1,
        NULL,
        "Pulldown", "MaxCallsToPALComb", NULL,

    },
    {
        "PAL Bad Cadence Mode", ITEMFROMLIST, 0, (long*)&PALBadCadenceIndex,
        INDEX_VIDEO_GREEDY, 0, 99, 1, 1,
        DeinterlaceNames,
        "Pulldown", "PALBadCadenceMode", NULL,
    },
};

SETTING* FD50_GetSetting(FD50_SETTING Setting)
{
    if(Setting > -1 && Setting < FD50_SETTING_LASTONE)
    {
        return &(FD50Settings[Setting]);
    }
    else
    {
        return NULL;
    }
}

SmartPtr<CTreeSettingsGeneric> FD50_GetTreeSettingsPage()
{
    SmartPtr<CSettingsHolder> Holder(SettingsMaster->FindMsgHolder(WM_FD50_GETVALUE));
    return new CTreeSettingsGeneric("2:2 Pulldown Settings", Holder);
}
