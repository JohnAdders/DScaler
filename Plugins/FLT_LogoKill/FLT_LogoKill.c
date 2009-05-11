/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

#pragma warning(disable : 4799)

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

long Top_UI = 5;
long Left_UI = 5;
long Width_UI = 30;
long Height_UI = 30;
long Max = 128;
long gUseSmoothing = TRUE;
long gSmoothingValue = 10;
int    LSR0;
int LSR1;
int    USR0;
int USR1;
int    VSR0;
int VSR1;
int LSC0[768];
int LSC1[768];
int USC0[768];
int USC1[768];
int VSC0[768];
int VSC1[768];

static long gCpuFeatureFlags = 0;
static const __int64 ShiftMask = 0xfefffefffefffeff;    // to avoid shifting chroma to luma

typedef enum
{
    MODE_GREY,
    MODE_MAX,
    MODE_DYNAMIC_MAX,
    MODE_WEIGHTED,
    MODE_BLACK,
    MODE_SMOOTHING_ONLY,
    MODE_LASTONE,
} eMODE;

eMODE Mode = MODE_GREY;

LPCSTR ModeList[] =
{
    "Grey",
    "Limit To Max Value",
    "Dynamic Max",
    "Weighted Average",
    "Black",
    "Smoothing only",
};

#pragma pack(push, 1)   // save state and set packing alignment to 1 byte
typedef struct
{
    BYTE Lumi1;
    BYTE Chroma1;
    BYTE Lumi2;
    BYTE Chroma2;
} TwoPixel;
typedef struct
{
    BYTE Lumi;
    BYTE Chroma;
} TwoByte;
#pragma pack(pop)

long DispatchLogoKiller( TDeinterlaceInfo *pInfo );

#define IS_SSE
#include "FLT_LogoKill.asm"
#undef  IS_SSE

#define IS_3DNOW
#include "FLT_LogoKill.asm"
#undef  IS_3DNOW

#define IS_MMXEXT
#include "FLT_LogoKill.asm"
#undef  IS_MMXEXT

BOOL Top_OnChange(long NewValue)
{
    Top_UI = NewValue;
    return TRUE;
}

BOOL Left_OnChange(long NewValue)
{
    Left_UI = NewValue;
    return TRUE;
}

BOOL Width_OnChange(long NewValue)
{
    Width_UI = NewValue;
    return TRUE;
}

BOOL Height_OnChange(long NewValue)
{
    Height_UI = NewValue;
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

FILTER_METHOD LogoKillMethod;

SETTING FLT_LogoKillSettings[FLT_LOGOKILL_SETTING_LASTONE] =
{
    {
        "Top", SLIDER, 0, (LONG_PTR*)&Top_UI,
        35, 0, 998, 1, 1,
        NULL,
        "LogoKillFilter", "Top", Top_OnChange,
    },
    {
        "Left", SLIDER, 0, (LONG_PTR*)&Left_UI,
        50, 0, 996, 1, 1,
        NULL,
        "LogoKillFilter", "Left", Left_OnChange,
    },
    {
        "Width", SLIDER, 0, (LONG_PTR*)&Width_UI,
        50, 4, 1000, 1, 1,
        NULL,
        "LogoKillFilter", "Width", Width_OnChange,
    },
    {
        "Height", SLIDER, 0, (LONG_PTR*)&Height_UI,
        85, 2, 1000, 1, 1,
        NULL,
        "LogoKillFilter", "Height", Height_OnChange,
    },
    {
        "Mode", ITEMFROMLIST, 0, (LONG_PTR*)&Mode,
        MODE_GREY, MODE_GREY, MODE_LASTONE -1, 1, 1,
        ModeList,
        "LogoKillFilter", "Mode", NULL,
    },
    {
        "Max", SLIDER, 0, (LONG_PTR*)&Max,
        128, 0, 255, 1, 1,
        NULL,
        "LogoKillFilter", "Max", NULL,
    },
    {
        "Smoothing", ONOFF, 0, (LONG_PTR*)&gUseSmoothing,
        TRUE, 0, 1, 1, 1,
        NULL,
        "LogoKillFilter", "UseSmoothing", NULL,
    },
    {
        "Amount of smoothing", SLIDER, 0, (LONG_PTR*)&gSmoothingValue,
        10, 0, 50, 1, 1,
        NULL,
        "LogoKillFilter", "Amout of smoothing", NULL,
    },
    {
        "Logo Kill Filter", ONOFF, 0, (LONG_PTR*)&(LogoKillMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "LogoKillFilter", "UseLogoKillFilter", NULL,
    },
};

FILTER_METHOD LogoKillMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Logo Killer Filter",
    "&Logo Killer",
    FALSE,
    TRUE,
    DispatchLogoKiller,
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_LOGOKILL_SETTING_LASTONE,
    FLT_LogoKillSettings,
    WM_FLT_LOGOKILL_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_LOGO_KILLER,
};

long DispatchLogoKiller( TDeinterlaceInfo *pInfo )
{

    if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
    {
        FilterLogoKiller_SSE( pInfo );
    }
    else if( gCpuFeatureFlags & FEATURE_3DNOW )
    {
        FilterLogoKiller_3DNOW( pInfo );
    }
    else
    {
        FilterLogoKiller_MMX( pInfo );
    }
    return 1000;
}

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    gCpuFeatureFlags = CpuFeatureFlags;

    return &LogoKillMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

