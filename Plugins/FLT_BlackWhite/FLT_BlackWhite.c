/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier.  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "DS_Filter.h"
#include "..\help\helpids.h"

FILTER_METHOD BlackWhiteMethod;

long BlackWhite(TDeinterlaceInfo* pInfo)
{
    char* Pixels;
    int y;
    int Cycles;
    const __int64 qwLumiMask       = 0x00FF00FF00FF00FF;
    const __int64 qwChromaHalfMask = 0x8000800080008000;

    // Need to have the current field to do the filtering.
    if (pInfo->PictureHistory[0] == NULL)
    {
        return 1000;
    }

    Cycles = pInfo->LineLength / 8;
    Pixels = pInfo->PictureHistory[0]->pData;

    for (y = 0; y < pInfo->FieldHeight; ++y)
    {
        _asm
        {
            mov     eax, Pixels
            mov     ecx, Cycles
            movq    mm1, qwLumiMask
            movq    mm2, qwChromaHalfMask

LOOP_LABEL:
            movq    mm0, [eax]

            pand    mm0, mm1
            por     mm0, mm2

            movq    [eax], mm0

            add     eax, 8
            dec     ecx
            jne     LOOP_LABEL
        }
        Pixels += pInfo->InputPitch;
    }
    DO_EMMS;

    return 1000;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_BlackWhiteSettings[FLT_BLACKWHITE_SETTING_LASTONE] =
{
    {
        "Black and White Filter", ONOFF, 0, (LONG_PTR*)&(BlackWhiteMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "BlackWhiteFilter", "UseBlackWhiteFilter", NULL,
    },
};

FILTER_METHOD BlackWhiteMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Black and White Filter",
    "&Black and White",
    FALSE,
    TRUE,
    BlackWhite,
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_BLACKWHITE_SETTING_LASTONE,
    FLT_BlackWhiteSettings,
    WM_FLT_BLACKWHITE_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_BLACK_WHITE,
};

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &BlackWhiteMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
