/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_Mirror.c,v 1.1 2002-06-06 12:46:56 robmuller Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"

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
    WM_FLT_MIRROR_GETVALUE - WM_USER,
    TRUE,
    1,
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

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}