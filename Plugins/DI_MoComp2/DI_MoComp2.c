/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Tom Barry & John Adcock.  All rights reserved.
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
//
//  (From Tom Barry)
//  Also, this program is "Philanthropy-Ware".  That is, if you like it and
//  feel the need to reward or inspire the author then please feel free (but
//  not obligated) to consider joining or donating to the Electronic Frontier
//  Foundation. This will help keep cyber space free of barbed wire and bullsh*t.
//  See www.eff.org for details
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include <DS_Deinterlace.h>
#include "MoComp2.h"
#include "..\help\helpids.h"

long SearchEffort = 1;

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_MoComp2Settings[DI_MOCOMP2_SETTING_LASTONE] =
{
    {
        "Search Effort", SLIDER, 0, (LONG_PTR*)&SearchEffort,
        1, 0, 255, 1, 1,
        NULL,
        "Deinterlace", "MoComp2SE", NULL,
    },
};

DEINTERLACE_METHOD MoComp2Method =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Video (MoComp2)",
    "MoComp2",
    FALSE,
    FALSE,
    DeinterlaceMoComp2_SSE,
    50,
    60,
    DI_MOCOMP2_SETTING_LASTONE,
    DI_MoComp2Settings,
    INDEX_VIDEO_MOCOMP2,
    NULL,
    NULL,
    NULL,
    NULL,
    4,                                    // number fields needed
    0,
    0,
    WM_DI_MOCOMP2_GETVALUE - WM_APP,
    NULL,
    0,
    FALSE,
    FALSE,
    IDH_MOCOMP2,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE2)
    {
        MoComp2Method.pfnAlgorithm = DeinterlaceMoComp2_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_SSE)
    {
        MoComp2Method.pfnAlgorithm = DeinterlaceMoComp2_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        MoComp2Method.pfnAlgorithm = DeinterlaceMoComp2_3DNOW;
    }
    else
    {
        MoComp2Method.pfnAlgorithm = DeinterlaceMoComp2_MMX;
    }

    return &MoComp2Method;
}

BOOL WINAPI NoCRTDllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
