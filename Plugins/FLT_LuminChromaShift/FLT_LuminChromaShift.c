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

// Taking a multiple of 2 for ONE_PIXEL_SHIFT is probably a good way to optimize the filter
#define ONE_PIXEL_SHIFT        16

FILTER_METHOD LuminChromaShiftMethod;

static BYTE WorkLine[768*2];    // Buffer to store one line of data
static int LuminShiftVal = 0;
static int delta_x1 = 0;
static int delta_x2 = 0;
static int coef_x1 = ONE_PIXEL_SHIFT;
static int coef_x2 = 0;

void __cdecl LuminChromaShiftStart( void )
{
    int Shift = (LuminShiftVal >= 0) ? LuminShiftVal : -LuminShiftVal;
    delta_x1 = Shift / ONE_PIXEL_SHIFT;
    if ((Shift % ONE_PIXEL_SHIFT) == 0)
    {
        delta_x2 = delta_x1;
    }
    else
    {
        delta_x2 = delta_x1 + 1;
    }
    coef_x2 = Shift % ONE_PIXEL_SHIFT;
    coef_x1 = ONE_PIXEL_SHIFT - coef_x2;
    if (LuminShiftVal < 0)
    {
        delta_x1 *= -1;
        delta_x2 *= -1;
    }
    return;
}

long __cdecl LuminChromaShift(TDeinterlaceInfo* pInfo)
{
    BYTE* Pixels;
    int x, y;

    // Need to have the current field to do the filtering.
    if (pInfo->PictureHistory[0] == NULL)
    {
        return 1000;
    }

    // Leave if there is no shift to apply
    if (LuminShiftVal == 0)
    {
        return 1000;
    }

    Pixels = pInfo->PictureHistory[0]->pData;
    for (y = 0; y < pInfo->FieldHeight; ++y)
    {
        // Save the original line content in a buffer
        // This buffer is then used to update the current line
        memcpy(WorkLine, Pixels, pInfo->LineLength);

        for (x = 0; x < pInfo->FrameWidth; ++x)
        {
            int x1 = x-delta_x1;
            int x2 = x-delta_x2;
            if (x1 < 0)
            {
                x1 = 0;
            }
            else if (x1 >= pInfo->FrameWidth)
            {
                x1 = pInfo->FrameWidth - 1;
            }
            if (x2 < 0)
            {
                x2 = 0;
            }
            else if (x2 >= pInfo->FrameWidth)
            {
                x2 = pInfo->FrameWidth - 1;
            }
            Pixels[x*2] = (WorkLine[x1*2] * coef_x1 + WorkLine[x2*2] * coef_x2) / ONE_PIXEL_SHIFT;
        }
        Pixels += pInfo->InputPitch;
    }

    return 1000;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

BOOL LuminShiftVal_OnChange(long NewValue)
{
    LuminShiftVal = (int)NewValue;
    LuminChromaShiftStart();
    return FALSE;
}

SETTING FLT_LuminChromaShiftSettings[FLT_LUMINCHROMASHIFT_SETTING_LASTONE] =
{
    {
        "Y/C Shift Filter", ONOFF, 0, &(LuminChromaShiftMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "LuminChromaShiftFilter", "UseLuminChromaShiftFilter", NULL,
    },
    {
        "Luminance Horizontal Shift", SLIDER, 0, &LuminShiftVal,
        0, -ONE_PIXEL_SHIFT*5, ONE_PIXEL_SHIFT*5, 1, 1,
        NULL,
        "LuminChromaShiftFilter", "LuminHorizShift", LuminShiftVal_OnChange,
    },
};

FILTER_METHOD LuminChromaShiftMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Y/C Shift Filter",
    "Y/C Shift",
    FALSE,
    TRUE,
    LuminChromaShift, 
    0,
    TRUE,
    LuminChromaShiftStart,
    NULL,
    NULL,
    FLT_LUMINCHROMASHIFT_SETTING_LASTONE,
    FLT_LuminChromaShiftSettings,
    WM_FLT_LUMINCHROMASHIFT_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_LUMINCHROMASHIFT,
};

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &LuminChromaShiftMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
