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
#include "DS_Filter.h"
#include "..\help\helpids.h"

long        DispatchSharpness( TDeinterlaceInfo *pInfo );


long        gUsePrefetching = TRUE;

// Stored information about the machine, used when choosing which code version to run

static long gCpuFeatureFlags = 0;

long        Sharpness = 128;

// The main code is included from a separate file to allow different versions
// for different processors.

#define USE_PREFETCH

#define IS_SSE
#include "FLT_Sharpness.asm"
#undef  IS_SSE

#define IS_3DNOW
#include "FLT_Sharpness.asm"
#undef  IS_3DNOW

#undef USE_PREFETCH

#define IS_SSE
#include "FLT_Sharpness.asm"
#undef  IS_SSE

#define IS_3DNOW
#include "FLT_Sharpness.asm"
#undef  IS_3DNOW

#define IS_MMXEXT
#include "FLT_Sharpness.asm"
#undef  IS_MMXEXT

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
FILTER_METHOD SharpnessMethod;

SETTING FLT_SharpnessSettings[FLT_SHARPNESS_SETTING_LASTONE] =
{
    {
        "Sharpness", SLIDER, 0, (LONG_PTR*)&Sharpness,
        128, 0, 255, 1, 1,
        NULL,
        "SharpnessFilter", "Sharpness", NULL,
    },
    {
        "Fast Memory Access", ONOFF, 0, (LONG_PTR*)&gUsePrefetching,
        TRUE, 0, 1, 1, 1,
        NULL,
        "SharpnessFilter", "UsePrefetching", NULL,
    },
    {
        "Sharpness Filter", ONOFF, 0, (LONG_PTR*)&(SharpnessMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "SharpnessFilter", "UseSharpnessFilter", NULL,
    },
};

FILTER_METHOD SharpnessMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Sharpness Filter",
    "&Sharpness",
    FALSE,
    TRUE,
    DispatchSharpness,
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_SHARPNESS_SETTING_LASTONE,
    FLT_SharpnessSettings,
    WM_FLT_SHARPNESS_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_SHARPNESS,
};



long DispatchSharpness( TDeinterlaceInfo *pInfo )
{

    if( gUsePrefetching == TRUE )
    {
        if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
        {
            FilterSharpness_SSE_PREFETCH( pInfo );
        }
        else if( gCpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterSharpness_3DNOW_PREFETCH( pInfo );
        }
        else
        {
            FilterSharpness_MMX( pInfo );
        }
    }
    else
    {
        if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
        {
            FilterSharpness_SSE( pInfo );
        }
        else if( gCpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterSharpness_3DNOW( pInfo );
        }
        else
        {
            FilterSharpness_MMX( pInfo );
        }
    }
    return 1000;
}


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    gCpuFeatureFlags = CpuFeatureFlags;

    return &SharpnessMethod;
}


BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}