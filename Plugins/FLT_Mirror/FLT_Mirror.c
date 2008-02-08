/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_Mirror.c,v 1.5 2008-02-08 13:43:21 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Rob Muller.  All rights reserved.
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
// Revision 1.4  2002/08/07 21:52:05  adcockj
// Added some missing help files
//
// Revision 1.3  2002/06/18 19:46:09  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.2  2002/06/13 12:10:26  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.1  2002/06/06 12:46:56  robmuller
// Initial checkin of the Mirror filter.
//
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

#define IS_SSE
#include "FLT_Mirror.asm"
#undef  IS_SSE

#define IS_3DNOW
#include "FLT_Mirror.asm"
#undef  IS_3DNOW

#define IS_MMX
#include "FLT_Mirror.asm"
#undef  IS_MMX


FILTER_METHOD MirrorMethod;

SETTING FLT_MirrorSettings[FLT_MIRROR_SETTING_LASTONE] =
{
    {
        "Mirror Filter", ONOFF, 0, &(MirrorMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "MirrorFilter", "UseMirrorFilter", NULL,
    },
};

FILTER_METHOD MirrorMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Mirror Filter",
    "&Mirror",
    FALSE,
    TRUE,
    FilterMirror_MMX, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_MIRROR_SETTING_LASTONE,
    FLT_MirrorSettings,
    WM_FLT_MIRROR_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_MIRROR,
};

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    if(CpuFeatureFlags & FEATURE_MMXEXT || CpuFeatureFlags & FEATURE_SSE)
    {
        MirrorMethod.pfnAlgorithm = FilterMirror_SSE;
    }
    else if(CpuFeatureFlags & FEATURE_3DNOW)
    {
        MirrorMethod.pfnAlgorithm = FilterMirror_3DNOW;
    }
    else
    {
        MirrorMethod.pfnAlgorithm = FilterMirror_MMX;
    }

    return &MirrorMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}