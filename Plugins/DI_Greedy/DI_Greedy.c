/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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

#include "windows.h"
#include "DS_Deinterlace.h"
#include "..\help\helpids.h"

long GreedyMaxComb = 15;

#define IS_SSE 1
#include "DI_Greedy.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "DI_Greedy.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "DI_Greedy.asm"
#undef IS_MMX

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_GreedySettings[DI_GREEDY_SETTING_LASTONE] =
{
    {
        "Greedy Max Comb", SLIDER, 0, (LONG_PTR*)&GreedyMaxComb,
        15, 0, 255, 1, 1,
        NULL,
        "Deinterlace", "GreedyMaxComb", NULL,
    },
};

DEINTERLACE_METHOD GreedyMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Video (Greedy, Low Motion)",
    "Greedy",
    FALSE,
    FALSE,
    DeinterlaceGreedy_MMX,
    50,
    60,
    DI_GREEDY_SETTING_LASTONE,
    DI_GreedySettings,
    INDEX_VIDEO_GREEDY,
    NULL,
    NULL,
    NULL,
    NULL,
    3,
    0,
    0,
    WM_DI_GREEDY_GETVALUE - WM_APP,
    NULL,
    0,
    FALSE,
    FALSE,
    IDH_GREEDY,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE)
    {
        GreedyMethod.pfnAlgorithm = DeinterlaceGreedy_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        GreedyMethod.pfnAlgorithm = DeinterlaceGreedy_3DNOW;
    }
    else
    {
        GreedyMethod.pfnAlgorithm = DeinterlaceGreedy_MMX;
    }
    return &GreedyMethod;
}

BOOL WINAPI NoCRTDllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
