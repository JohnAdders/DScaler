/////////////////////////////////////////////////////////////////////////////
// DI_Adaptive.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Mark Rejhon and Steve Grimm.  All rights reserved.
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
// 07 Jan 2001   John Adcock           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"

long		StaticImageFieldCount = 100;
long		LowMotionFieldCount = 4;
long		StaticImageMode = INDEX_WEAVE;
long		LowMotionMode = INDEX_VIDEO_2FRAME;
long		HighMotionMode = INDEX_VIDEO_2FRAME;
long		AdaptiveThres32Pulldown = 15;
long		AdaptiveThresPulldownMismatch = 900;


long NumVideoModes = 0;
long CurrentIndex = -1;
DEINTERLACE_METHOD** DeintMethods = NULL;
DEINTERLACE_METHOD* CurrentMethod = NULL;
HWND ghwndStatus = NULL;

///////////////////////////////////////////////////////////////////////////////
// UpdateAdaptiveMode
//
// Switches to a new adaptive mode.  Updates the status bar if needed.
///////////////////////////////////////////////////////////////////////////////
void UpdateAdaptiveMode(long Index)
{
	int i;

	if (CurrentIndex == Index)
		return;

	for(i = 0; i < NumVideoModes; i++)
	{
		if(DeintMethods[i]->nMethodIndex == Index)
		{
			CurrentMethod = DeintMethods[i];
			if(ghwndStatus != NULL)
			{
				char AdaptiveName[200];
				char* ModeName;

				ModeName = DeintMethods[i]->szShortName;
				if (ModeName == NULL)
				{
					ModeName = DeintMethods[i]->szName;
				}
				wsprintf(AdaptiveName, "Adaptive - %s", ModeName);
				SendMessage(ghwndStatus, WM_SETTEXT, 0, (LPARAM) AdaptiveName);
			}
			CurrentIndex = Index;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// DeinterlaceAdaptive
//
// This mode supports three styles of deinterlacing and switch among
// them depending on the amount of motion in the scene.  If there's a lot of
// motion, we use HighMotionMode.  If there's little or no motion for at least
// LowMotionFieldCount fields, we switch to LowMotionMode.  And if, after that,
// there's no motion for StaticImageFieldCount fields, we switch to
// StaticImageMode.
//
// Exactly which modes are used in these three cases is configurable in the INI
// file. It is entirely legal for some or all of the three video modes to be the
// same; by default, VIDEO_MODE_2FRAME is used for both the high and low motion
// modes.  On slower machines VIDEO_MODE_BOB (high) and VIDEO_MODE_WEAVE (low)
// can be used instead since they're less CPU-intensive.
///////////////////////////////////////////////////////////////////////////////
BOOL DeinterlaceAdaptive(DEINTERLACE_INFO *info)
{
	static long MATCH_COUNT = 0;

	// If this is our first time, update the current adaptive mode to whatever
	// the ini file said our high-motion mode should be.
	if (CurrentIndex == -1)
		UpdateAdaptiveMode(HighMotionMode);

	// reset MATCH_COUNT when we are called and the info
	// struct doesn't contain at least an odd and an even frame
	if(info->EvenLines[0] == NULL || info->OddLines[0] == NULL)
	{
		return FALSE;
	}

	// If the field difference is bigger than the threshold, then
	// the current field is very different from the field two fields ago.
	// so reset the match count and switch back
	// to either low or high motion
	//CompareFields(info);
    if(info->FieldDiff > AdaptiveThres32Pulldown)
	{
		MATCH_COUNT = 0;

		// If we're not in low-motion mode, it might be okay to drop to
		// low-motion mode.
		if (CurrentIndex != LowMotionMode &&
			info->FieldDiff < AdaptiveThresPulldownMismatch)
		{
			UpdateAdaptiveMode(LowMotionMode);
		}
		// If we're not in high-motion mode, wwe might have
        // to go to high-motion mode
		if(CurrentIndex != HighMotionMode &&
            info->FieldDiff >= AdaptiveThresPulldownMismatch)
		{
			UpdateAdaptiveMode(HighMotionMode);
		}
	}
    else
	{
		MATCH_COUNT++;

		if (MATCH_COUNT >= LowMotionFieldCount &&
			CurrentIndex == HighMotionMode)
		{
			UpdateAdaptiveMode(LowMotionMode);
		}
		if (MATCH_COUNT >= StaticImageFieldCount &&
			CurrentIndex == LowMotionMode)
		{
			UpdateAdaptiveMode(StaticImageMode);
		}
	}
	if(CurrentMethod != NULL)
	{
		return CurrentMethod->pfnAlgorithm(info);
	}
	else
	{
		return FALSE;
	}
}

void __cdecl AdaptiveShowUI(HWND hwndMain)
{
}

void __cdecl AdaptiveStart(long NumPlugIns, DEINTERLACE_METHOD** OtherPlugins, HWND hwndStatus)
{
	DeintMethods = OtherPlugins;
	NumVideoModes = NumPlugIns;
	ghwndStatus = hwndStatus;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_AdaptiveSettings[DI_ADAPTIVE_SETTING_LASTONE] =
{
	{
		"Low Motion Field Count", SLIDER, 0, &LowMotionFieldCount,
		4, 1, 1000, 1, 1,
		NULL,
		"Pulldown", "LowMotionFieldCount", NULL,
	},
	{
		"Static Image Field Count", SLIDER, 0, &StaticImageFieldCount,
		100, 1, 1000, 1, 1,
		NULL,
		"Pulldown", "StaticImageFieldCount", NULL,
	},
	{
		"Static Image Mode", ITEMFROMLIST, 0, &StaticImageMode,
		INDEX_WEAVE, 0, 99, 1, 1,
		NULL,
		"Pulldown", "StaticImageMode", NULL,
	},
	{
		"Low Motion Mode", ITEMFROMLIST, 0, &LowMotionMode,
		INDEX_VIDEO_2FRAME, 0, 99, 1, 1,
		NULL,
		"Pulldown", "LowMotionMode", NULL,
	},
	{
		"High Motion Mode", ITEMFROMLIST, 0, &HighMotionMode,
		INDEX_VIDEO_2FRAME, 0, 99, 1, 1,
		NULL,
		"Pulldown", "HighMotionMode", NULL,
	},
	{
		"Adaptive Threshold 3:2 Pulldown", SLIDER, 0, &AdaptiveThres32Pulldown,
		15, 1, 5000, 5, 1,
		NULL,
		"Pulldown", "AdaptiveThres32Pulldown", NULL,
	},
	{
		"Adaptive Threshold 3:2 Pulldown Mismatch", SLIDER, 0, &AdaptiveThresPulldownMismatch,
		900, 1, 10000, 10, 1,
		NULL,
		"Pulldown", "AdaptiveThresPulldownMismatch", NULL,
	},
};

DEINTERLACE_METHOD AdaptiveMethod =
{
	sizeof(DEINTERLACE_METHOD),
	DEINTERLACE_CURRENT_VERSION,
	"Adaptive", 
	NULL, 
	FALSE, 
	FALSE, 
	DeinterlaceAdaptive, 
	50, 
	60,
	DI_ADAPTIVE_SETTING_LASTONE,
	DI_AdaptiveSettings,
	INDEX_ADAPTIVE,
	NULL,
	AdaptiveStart,
	AdaptiveShowUI,
	NULL,
	4,
	0,
	0,
	WM_DI_ADAPTIVE_GETVALUE - WM_USER,
	NULL,
	0,
	TRUE,
	FALSE,
};

__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
	return &AdaptiveMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
