/////////////////////////////////////////////////////////////////////////////
// $Id: DI_OldGame.c,v 1.1 2001-07-30 08:25:22 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Lindsey Dubb.  All rights reserved.
// based on OddOnly and Temporal Noise DScaler Plugins
// (c) John Adcock & Steve Grimm
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
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"

//uncomment to turn on logging
//#define LD_DEBUG

static long                         gMaxComb = 300;
static DEINTERLACEPLUGINSETSTATUS*  gPfnSetStatus = NULL;

#define IS_SSE 1
#include "DI_OldGame.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "DI_OldGame.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "DI_OldGame.asm"
#undef IS_MMX

SETTING DI_OldGameSettings[DI_OLDGAME_SETTING_LASTONE] =
{
    {
        "Maximum still comb", SLIDER, 0, &gMaxComb,
        300, 1, 10000, 1, 1,
        NULL,
        "DI_OldGame", "maxComb", NULL,
    },
};


#ifdef LD_DEBUG
void __cdecl OldGameDebugStart(long NumPlugIns, DEINTERLACE_METHOD** OtherPlugins, DEINTERLACEPLUGINSETSTATUS* SetStatus)
{
    gPfnSetStatus = SetStatus;
}
#endif



DEINTERLACE_METHOD OldGameMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Old Game", 
    NULL,     // And why would anyone use this as an adaptive method?
    TRUE, 
    FALSE, 
    NULL, 
    50, 
    60,
    DI_OLDGAME_SETTING_LASTONE,
    DI_OldGameSettings,
    INDEX_OLD_GAME,
    NULL,
#ifdef LD_DEBUG
    OldGameDebugStart,
#else
    NULL,
#endif
    NULL,
    NULL,
    2,
    0,
    0,
    -1,
    NULL,
    0,
    FALSE,
    TRUE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE)
    {
        OldGameMethod.pfnAlgorithm = OldGameFilter_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        OldGameMethod.pfnAlgorithm = OldGameFilter_3DNOW;
    }
    else
    {
        OldGameMethod.pfnAlgorithm = OldGameFilter_MMX;
    }
    return &OldGameMethod;
}


BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
