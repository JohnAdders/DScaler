///////////////////////////////////////////////////////////////////////////////
// DI_Greedy2Frame.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock, Tom Barry, Steve Grimm  All rights reserved.
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
///////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 08 Feb 2000   John Adcock           New Method
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"

long GreedyTwoFrameThreshold = 8;

#define IS_SSE 1
#include "DI_Greedy2Frame.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "DI_Greedy2Frame.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "DI_Greedy2Frame.asm"
#undef IS_MMX

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_Greedy2FrameSettings[DI_GREEDY2FRAME_SETTING_LASTONE] =
{
	{
		"Greedy 2 Frame Threshold", SLIDER, 0, &GreedyTwoFrameThreshold,
		4, 0, 128, 1, 1,
		NULL,
		"Deinterlace", "GreedyTwoFrameThreshold", NULL,
	},
};


DEINTERLACE_METHOD Greedy2FrameMethod =
{
	sizeof(DEINTERLACE_METHOD),
	DEINTERLACE_CURRENT_VERSION,
	"Greedy 2 Frame", 
	NULL,
	FALSE, 
	FALSE, 
	DeinterlaceGreedy2Frame_MMX, 
	50, 
	60,
	DI_GREEDY2FRAME_SETTING_LASTONE,
	DI_Greedy2FrameSettings,
	INDEX_VIDEO_GREEDY2FRAME,
	NULL,
	NULL,
	NULL,
	NULL,
	4,
	0,
	0,
	WM_DI_GREEDY2FRAME_GETVALUE - WM_USER,
	NULL,
	0,
	FALSE,
	FALSE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE)
    {
        Greedy2FrameMethod.pfnAlgorithm = DeinterlaceGreedy2Frame_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        Greedy2FrameMethod.pfnAlgorithm = DeinterlaceGreedy2Frame_3DNOW;
    }
    else
    {
        Greedy2FrameMethod.pfnAlgorithm = DeinterlaceGreedy2Frame_MMX;
    }
	return &Greedy2FrameMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
