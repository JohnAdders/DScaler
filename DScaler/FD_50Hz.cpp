/////////////////////////////////////////////////////////////////////////////
// FD_50Hz.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 02 Jan 2001   John Adcock           Made PAL pulldown detect remember last video
//                                     mode
//
// 07 Jan 2001   John Adcock           Fixed PAL detection bug
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use DEINTERLACE_INFO
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_Common.h"
#include "DebugLog.h"

long gPALFilmFallbackIndex = INDEX_VIDEO_2FRAME;
// Default values which can be overwritten by the INI file
long PulldownThresholdLow = 30;
long PulldownThresholdHigh = 10;
long PALPulldownRepeatCount = 3;
long PALPulldownRepeatCount2 = 1;


///////////////////////////////////////////////////////////////////////////////
// UpdatePALPulldownMode
//
// This is the 2:2 pulldown detection for PAL.
///////////////////////////////////////////////////////////////////////////////
void UpdatePALPulldownMode(DEINTERLACE_INFO *pInfo)
{
	static long LastCombFactor = 0;
	static long RepeatCount;
	static long LastPolarity = -1;
	static double LastDiff = 0;
	static DWORD StartFilmTicks = 0;
	double PercentDecrease = 0;
	double PercentIncrease = 0;

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
		return;
	}
    
    if(pInfo->CombFactor < 0)
    {
        return;
    }
	
    PercentDecrease = ((double)pInfo->CombFactor * 100.0) / ((double)LastCombFactor + 100.0);
	PercentIncrease = ((double)(pInfo->CombFactor - LastCombFactor) * 100.0) / ((double)LastCombFactor + 100.0);

	if(!IsFilmMode())
	{
		if(PercentDecrease < PulldownThresholdLow && LastDiff > PulldownThresholdLow)
		{
			if(LastPolarity == pInfo->IsOdd)
			{
				if(RepeatCount < PALPulldownRepeatCount)
				{
					if(RepeatCount == 0)
					{
						RepeatCount = 1;
					}
					else
					{
						if((GetTickCount() - StartFilmTicks) < 100)
						{
							LOG(" Upped RepeatCount %d Gap %d", RepeatCount, (GetTickCount() - StartFilmTicks));
							RepeatCount++;
						}
						else
						{
							LOG(" Reset RepeatCount - Gap %d Too long", (GetTickCount() - StartFilmTicks));
							RepeatCount = 1;					
						}
					}
				}
				else
				{
					if(pInfo->IsOdd == TRUE)
					{
						SetFilmDeinterlaceMode(FILM_22_PULLDOWN_ODD);
						LOG(" Gone to Odd");
					}
					if(pInfo->IsOdd == FALSE)
					{
						SetFilmDeinterlaceMode(FILM_22_PULLDOWN_EVEN);
						LOG(" Gone to Even");
					}
				}
			}
			else
			{
				LastPolarity = pInfo->IsOdd;
				RepeatCount = 1;
				LOG(" Reset RepeatCount %d", RepeatCount);
			}
			StartFilmTicks = GetTickCount();
		}
	}
	else
	{
		if(PercentDecrease < PulldownThresholdLow)
		{
			if(LastPolarity != pInfo->IsOdd)
			{
				if(LastDiff > PulldownThresholdLow)
				{
					RepeatCount--;
					LOG(" Downed RepeatCount 1 %d", RepeatCount);
				}
			}
			else
			{
				if(RepeatCount < PALPulldownRepeatCount)
				{
					RepeatCount++;
					LOG(" Upped RepeatCount 1 %d", RepeatCount);
				}
			}
		}
		
		if(PercentIncrease > PulldownThresholdHigh && LastDiff > PulldownThresholdLow)
		{
			if(GetFilmMode() == FILM_22_PULLDOWN_ODD && pInfo->IsOdd == TRUE)
			{
				RepeatCount--;
				LOG(" Downed RepeatCount 2 %d", RepeatCount);
			}
			if(GetFilmMode() == FILM_22_PULLDOWN_EVEN && pInfo->IsOdd == FALSE)
			{
				RepeatCount--;
				LOG(" Downed RepeatCount 2 %d", RepeatCount);
			}
		}
		// FIXME: Should have a different parameter here
		if(RepeatCount <= (PALPulldownRepeatCount - PALPulldownRepeatCount2))
		{
			SetVideoDeinterlaceIndex(gPALFilmFallbackIndex);
			LOG(" Back To Video Mode");
			RepeatCount = PALPulldownRepeatCount - 1;
			LastPolarity = !pInfo->IsOdd;
			StartFilmTicks = GetTickCount();
		}
	}

	LastDiff = PercentDecrease;
	LastCombFactor = pInfo->CombFactor;
}

BOOL FilmModePALEven(DEINTERLACE_INFO *pInfo)
{
	if (!pInfo->IsOdd)
	{
		Weave(pInfo);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL FilmModePALOdd(DEINTERLACE_INFO *pInfo)
{
	if (pInfo->IsOdd)
	{
		Weave(pInfo);
		return TRUE;
	}
	else
	{
		return FALSE;
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
		"PAL Film Fallback Mode", ITEMFROMLIST, 0, &gPALFilmFallbackIndex,
		INDEX_VIDEO_2FRAME, 0, 99, 1, 1,
		NULL,
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

void FD50_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < FD50_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(FD50Settings[i]));
	}
}

void FD50_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < FD50_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(FD50Settings[i]));
	}
}

