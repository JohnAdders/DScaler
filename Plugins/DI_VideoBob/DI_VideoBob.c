/////////////////////////////////////////////////////////////////////////////
// DI_VideoBob.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
// Based on code from Virtual Dub Plug-in by Gunnar Thalin
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
// 30 Dec 2000   Mark Rejhon           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"

long EdgeDetect = 625;
long JaggieThreshold = 73;

#define IS_SSE 1
#include "DI_VideoBob.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "DI_VideoBob.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "DI_VideoBob.asm"
#undef IS_MMX

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_VideoBobSettings[DI_VIDEOBOB_SETTING_LASTONE] =
{
	{
		"Weave Edge Detect", SLIDER, 0, &EdgeDetect,
		625, 0, 10000, 5, 1,
		NULL,
		"Deinterlace", "EdgeDetect", NULL,
	},
	{
		"Weave Jaggie Threshold", SLIDER, 0, &JaggieThreshold,
		73, 0, 5000, 5, 1,
		NULL,
		"Deinterlace", "JaggieThreshold", NULL,
	},
};

DEINTERLACE_METHOD VideoBobMethod =
{
	sizeof(DEINTERLACE_METHOD),
	DEINTERLACE_CURRENT_VERSION,
	"Video Deinterlace (Bob)", 
	"Bob",
	FALSE, 
	FALSE, 
	DeinterlaceFieldBob_MMX, 
	50, 
	60,
	DI_VIDEOBOB_SETTING_LASTONE,
	DI_VideoBobSettings,
	INDEX_VIDEO_BOB,
	NULL,
	NULL,
	NULL,
	1,
	0,
	0,
	WM_DI_VIDEOBOB_GETVALUE - WM_USER,
	NULL,
	0,
	FALSE,
	FALSE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE)
    {
        VideoBobMethod.pfnAlgorithm = DeinterlaceFieldBob_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        VideoBobMethod.pfnAlgorithm = DeinterlaceFieldBob_3DNOW;
    }
    else
    {
        VideoBobMethod.pfnAlgorithm = DeinterlaceFieldBob_MMX;
    }
	return &VideoBobMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
