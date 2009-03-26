/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock. All rights reserved.
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
 * @file FD_Prog.cpp Progressive film mode functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "OutThreads.h"
#include "FD_Prog.h"
#include "Deinterlace.h"
#include "DebugLog.h"
#include "SettingsMaster.h"

long ThresholdStill = 50;
long ThresholdMotion = 10;
long RepeatCount = 4;


BOOL ProgExpectChange(TDeinterlaceInfo* pInfo, eProgressivePulldownMode FilmMode);


void UpdateProgPulldownModePAL(TDeinterlaceInfo* pInfo)
{
    static int ConsecutiveStills = 0;
    static int PatternSpots = 0;
    static eProgressivePulldownMode FilmModeBeingDetected = NORMAL_PROGRESSIVE;

    if(pInfo == NULL)
    {
        ConsecutiveStills = 0;
        PatternSpots = 0;
        FilmModeBeingDetected = NORMAL_PROGRESSIVE;
        return;
    }

    // if the diff is above the high threshold then
    // we are seeing a diff, so see if we were expecting on or not
    if(pInfo->FieldDiff > ThresholdMotion)
    {
        if(IsFilmMode())
        {
            if(!ProgExpectChange(pInfo, GetProgMode()))
            {
                // Go back to video
                SetVideoDeinterlaceMode(0);
                FilmModeBeingDetected = NORMAL_PROGRESSIVE;
                LOG(2, "Gone back to video");
            }
        }
        else
        {
            eProgressivePulldownMode GuessMode = NORMAL_PROGRESSIVE;
            if(ConsecutiveStills == 1)
            {
                switch(pInfo->CurrentFrame)
                {
                case 0:
                    GuessMode = PROG_22_PULLDOWN_EVEN;
                    break;
                case 1:
                    GuessMode = PROG_22_PULLDOWN_ODD;
                    break;
                case 2:
                    GuessMode = PROG_22_PULLDOWN_EVEN;
                    break;
                case 3:
                    GuessMode = PROG_22_PULLDOWN_ODD;
                    break;
                }
            }
            if(GuessMode != NORMAL_PROGRESSIVE)
            {
                if(GuessMode != FilmModeBeingDetected)
                {
                    PatternSpots = 0;
                    FilmModeBeingDetected = GuessMode;
                    LOG(2, "StartDetecting %d", GuessMode);
                }
                else
                {
                    ++PatternSpots;
                    if(PatternSpots >= RepeatCount)
                    {
                        SetFilmDeinterlaceMode((eFilmPulldownMode)FilmModeBeingDetected);
                        LOG(2, "Gone to film %d", FilmModeBeingDetected);
                    }
                    else
                    {
                        LOG(2, "Found again %d", GuessMode);
                    }
                }
            }
            else
            {
                PatternSpots = 0;
                FilmModeBeingDetected = NORMAL_PROGRESSIVE;
                LOG(2, "Reset bad pattern", GuessMode);
            }
        }
        ConsecutiveStills = 0;
    }
    // if the diff is below the low threshold then
    // we are seeing a static image
    // see if we were expecting the same image to lock onto a film mode
    else if(pInfo->FieldDiff < ThresholdStill)
    {
        if(!IsFilmMode())
        {
            ++ConsecutiveStills;
            if(ProgExpectChange(pInfo, FilmModeBeingDetected))
            {
                PatternSpots = 0;
                FilmModeBeingDetected = NORMAL_PROGRESSIVE;
                LOG(2, "Reset found still when expecting change");
            }
        }
    }
    // if we're in a film mode carry on but if we're trying to lock reset
    // as the evidence in inconclusive
    else
    {
        if(!IsFilmMode())
        {
            ConsecutiveStills = 0;
            PatternSpots = 0;
            FilmModeBeingDetected = NORMAL_PROGRESSIVE;
            LOG(2, "Reset neither change nor still");
        }
    }
}

void UpdateProgPulldownModeNTSC(TDeinterlaceInfo* pInfo)
{
    static int ConsecutiveStills = 0;
    static int PatternSpots = 0;
    static eProgressivePulldownMode FilmModeBeingDetected = NORMAL_PROGRESSIVE;

    if(pInfo == NULL)
    {
        ConsecutiveStills = 0;
        PatternSpots = 0;
        FilmModeBeingDetected = NORMAL_PROGRESSIVE;
        return;
    }

    // if the diff is above the high threshold then
    // we are seeing a diff, so see if we were expecting on or not
    if(pInfo->FieldDiff > ThresholdMotion)
    {
        if(IsFilmMode())
        {
            if(!ProgExpectChange(pInfo, GetProgMode()))
            {
                // Go back to video
                SetVideoDeinterlaceMode(0);
                FilmModeBeingDetected = NORMAL_PROGRESSIVE;
                LOG(2, "Gone back to video");
            }
        }
        else
        {
            eProgressivePulldownMode GuessMode = NORMAL_PROGRESSIVE;
            if(ConsecutiveStills == 1)
            {
                switch(pInfo->CurrentFrame)
                {
                case 0:
                    GuessMode = PROG_32_PULLDOWN_0;
                    break;
                case 1:
                    GuessMode = PROG_32_PULLDOWN_4;
                    break;
                case 2:
                    GuessMode = PROG_32_PULLDOWN_3;
                    break;
                case 3:
                    GuessMode = PROG_32_PULLDOWN_2;
                    break;
                case 4:
                    GuessMode = PROG_32_PULLDOWN_1;
                    break;
                }
            }
            else if(ConsecutiveStills == 2)
            {
                switch(pInfo->CurrentFrame)
                {
                case 0:
                    GuessMode = PROG_32_PULLDOWN_3;
                    break;
                case 1:
                    GuessMode = PROG_32_PULLDOWN_2;
                    break;
                case 2:
                    GuessMode = PROG_32_PULLDOWN_1;
                    break;
                case 3:
                    GuessMode = PROG_32_PULLDOWN_0;
                    break;
                case 4:
                    GuessMode = PROG_32_PULLDOWN_4;
                    break;
                }
            }
            if(GuessMode != NORMAL_PROGRESSIVE)
            {
                if(GuessMode != FilmModeBeingDetected)
                {
                    PatternSpots = 0;
                    FilmModeBeingDetected = GuessMode;
                    LOG(2, "StartDetecting %d", GuessMode);
                }
                else
                {
                    ++PatternSpots;
                    if(PatternSpots >= RepeatCount && (ConsecutiveStills == 2))
                    {
                        SetFilmDeinterlaceMode((eFilmPulldownMode)FilmModeBeingDetected);
                        LOG(2, "Gone to film %d", FilmModeBeingDetected);
                    }
                    else
                    {
                        LOG(2, "Found again %d", GuessMode);
                    }
                }
            }
            else
            {
                PatternSpots = 0;
                FilmModeBeingDetected = NORMAL_PROGRESSIVE;
                LOG(2, "Reset bad pattern", GuessMode);
            }
        }
        ConsecutiveStills = 0;
    }
    // if the diff is below the low threshold then
    // we are seeing a static image
    // see if we were expecting the same image to lock onto a film mode
    else if(pInfo->FieldDiff < ThresholdStill)
    {
        if(!IsFilmMode())
        {
            ++ConsecutiveStills;
            if(ProgExpectChange(pInfo, FilmModeBeingDetected))
            {
                PatternSpots = 0;
                FilmModeBeingDetected = NORMAL_PROGRESSIVE;
                LOG(2, "Reset found still when expecting change");
            }
        }
    }
    // if we're in a film mode carry on but if we're trying to lock reset
    // as the evidence in inconclusive
    else
    {
        if(!IsFilmMode())
        {
            ConsecutiveStills = 0;
            PatternSpots = 0;
            FilmModeBeingDetected = NORMAL_PROGRESSIVE;
            LOG(2, "Reset neither change nor still");
        }
    }
}


BOOL ProgMode(TDeinterlaceInfo* pInfo)
{
    if(ProgExpectChange(pInfo, GetProgMode()) == TRUE)
    {
        BYTE* CurrentLine = pInfo->PictureHistory[0]->pData;
        if (CurrentLine != NULL)
        {
            BYTE *lpOverlay = pInfo->Overlay;
            for (int i = 0; i < pInfo->FrameHeight; i++)
            {
                pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);
                lpOverlay += pInfo->OverlayPitch;
                CurrentLine += pInfo->InputPitch;
            }
            _asm
            {
                emms
            }
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

BOOL ProgExpectChange(TDeinterlaceInfo* pInfo, eProgressivePulldownMode FilmMode)
{
    switch(FilmMode)
    {
    case PROG_22_PULLDOWN_ODD:
        return ((pInfo->CurrentFrame & 1) != 0);
        break;
    case PROG_22_PULLDOWN_EVEN:
        return ((pInfo->CurrentFrame & 1) == 0);
        break;
    case PROG_32_PULLDOWN_0:
        return (pInfo->CurrentFrame == 0 || pInfo->CurrentFrame == 3);
        break;
    case PROG_32_PULLDOWN_1:
        return (pInfo->CurrentFrame == 4 || pInfo->CurrentFrame == 2);
        break;
    case PROG_32_PULLDOWN_2:
        return (pInfo->CurrentFrame == 3 || pInfo->CurrentFrame == 1);
        break;
    case PROG_32_PULLDOWN_3:
        return (pInfo->CurrentFrame == 2 || pInfo->CurrentFrame == 0);
        break;
    case PROG_32_PULLDOWN_4:
        return (pInfo->CurrentFrame == 1 || pInfo->CurrentFrame == 4);
        break;
    case NORMAL_PROGRESSIVE:
    default:
        return TRUE;
        break;
    }
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FDProgSettings[FDPROG_SETTING_LASTONE] =
{
    {
        "Threshold Still", SLIDER, 0, &ThresholdStill,
        30, 0, 10000, 10, 1,
        NULL,
        "ProgPulldown", "ThresholdStill", NULL,
    },
    {
        "Threshold Motion", SLIDER, 0, &ThresholdMotion,
        200, 0, 10000, 10, 1,
        NULL,
        "ProgPulldown", "ThresholdMotion", NULL,
    },
    {
        "Repeat Count", SLIDER, 0, &RepeatCount,
        4, 0, 20, 1, 1,
        NULL,
        "ProgPulldown", "RepeatCount", NULL,
    },
};

SETTING* FDProg_GetSetting(FDPROG_SETTING Setting)
{
    if(Setting > -1 && Setting < FDPROG_SETTING_LASTONE)
    {
        return &(FDProgSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void FDProg_SetMenu(HMENU hMenu)
{
}

SmartPtr<CTreeSettingsGeneric> FDProg_GetTreeSettingsPage()
{
    SmartPtr<CSettingsHolder> Holder(SettingsMaster->FindMsgHolder(WM_FDPROG_GETVALUE));
    return new CTreeSettingsGeneric("Progressive Pulldown Settings", Holder);
}
