/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_LogoKill.c,v 1.26 2008-02-08 13:43:21 adcockj Exp $
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
// This code is based on the 
// LogoAway VirtualDub filter by Krzysztof Wojdon (c) 2000
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.25  2006/10/06 13:35:32  adcockj
// Added projects for .NET 2005 and fixed most of the warnings and errors
//
// Revision 1.24  2005/03/11 13:31:55  adcockj
// Get rid of a load of compilation warnings in vs.net
//
// Revision 1.23  2002/11/18 20:08:55  robmuller
// Added Smoothing Only mode.
//
// Revision 1.22  2002/11/06 21:03:15  adcockj
// Fixed black feature in logo killer ;) (note to self must test before checking in....)
//
// Revision 1.21  2002/11/06 20:54:07  adcockj
// Added black as option in logo killer
//
// Revision 1.20  2002/11/05 13:07:08  adcockj
// Logo Killer Smoothing patch from Jochen Trenner
//
// Revision 1.19  2002/11/02 11:11:42  robmuller
// Removed 'Experimental' from the name of the filter.
//
// Revision 1.18  2002/10/29 15:24:21  robmuller
// Remove smoothing setting from logo killer since it is not used anymore.
//
// Revision 1.17  2002/10/16 12:21:50  adcockj
// Fixed a few bugs in new weighted average mode
// Removed old weighted average modes
//
// Revision 1.16  2002/10/14 20:43:42  robmuller
// Changed into input filter. New mode added. Miscellaneous improvements.
//
// Revision 1.15  2002/10/09 22:16:35  robmuller
// Implemented 3dnow and MMX versions.
//
// Revision 1.14  2002/09/08 10:56:02  robmuller
// Fixed error in smoothing.
//
// Revision 1.13  2002/08/17 11:42:06  kooiman
// Applied LogoKiller Filter Smoothing option from Jochen Trenner.
//
// Revision 1.12  2002/06/18 19:46:09  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.11  2002/06/13 12:10:26  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.10  2002/04/30 18:17:06  adcockj
// New weighted logo killer modes from Jochen Trenner
//
// Revision 1.9  2001/12/15 17:50:47  adcockj
// Fix for zero width bug
//
// Revision 1.8  2001/11/26 15:27:19  adcockj
// Changed filter structure
//
// Revision 1.7  2001/11/21 15:21:41  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.6  2001/08/09 21:34:59  adcockj
// Fixed bugs raise by Timo and Keld
//
// Revision 1.5  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE

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
int	LSR0;
int LSR1;
int	USR0;
int USR1;
int	VSR0;
int VSR1;
int LSC0[768];
int LSC1[768];
int USC0[768];
int USC1[768];
int VSC0[768];
int VSC1[768];

static long gCpuFeatureFlags = 0;
static const __int64 ShiftMask = 0xfefffefffefffeff;	// to avoid shifting chroma to luma

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
        "Top", SLIDER, 0, &Top_UI,
        35, 0, 998, 1, 1,
        NULL,
        "LogoKillFilter", "Top", Top_OnChange,
    },
    {
        "Left", SLIDER, 0, &Left_UI,
        50, 0, 996, 1, 1,
        NULL,
        "LogoKillFilter", "Left", Left_OnChange,
    },
    {
        "Width", SLIDER, 0, &Width_UI,
        50, 4, 1000, 1, 1,
        NULL,
        "LogoKillFilter", "Width", Width_OnChange,
    },
    {
        "Height", SLIDER, 0, &Height_UI,
        85, 2, 1000, 1, 1,
        NULL,
        "LogoKillFilter", "Height", Height_OnChange,
    },
    {
        "Mode", ITEMFROMLIST, 0, (long*)&Mode,
        MODE_GREY, MODE_GREY, MODE_LASTONE -1, 1, 1,
        ModeList,
        "LogoKillFilter", "Mode", NULL,
    },
    {
        "Max", SLIDER, 0, &Max,
        128, 0, 255, 1, 1,
        NULL,
        "LogoKillFilter", "Max", NULL,
    },
    {
        "Smoothing", ONOFF, 0, &gUseSmoothing,
        TRUE, 0, 1, 1, 1,
        NULL,
        "LogoKillFilter", "UseSmoothing", NULL,
    },
    {
        "Amount of smoothing", SLIDER, 0, &gSmoothingValue,
        10, 0, 50, 1, 1,
        NULL,
        "LogoKillFilter", "Amout of smoothing", NULL,
    },
    {
        "Logo Kill Filter", ONOFF, 0, &(LogoKillMethod.bActive),
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

