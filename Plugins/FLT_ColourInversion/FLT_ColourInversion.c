/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_ColourInversion.c,v 1.2 2002-08-07 21:52:05 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 TM. All rights reserved.
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

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

FILTER_METHOD InversionMethod;

long FilterInversion(TDeinterlaceInfo* pInfo)
{
    short* Pixels;
    int x;
    int y;
    int Cycles;

    // Need to have the current and next-to-previous fields to do the filtering.
    if (pInfo->PictureHistory[0] == NULL )
    {
        return 1000;
    }

    Cycles = pInfo->LineLength / 2;

    for (y = 0; y < pInfo->FieldHeight; ++y)
    {
        Pixels = (short*)pInfo->PictureHistory[0]->pData + y * pInfo->InputPitch/2;
		for (x = 0; x < Cycles; x++)
		{
			*Pixels = 255 - *Pixels;
			Pixels++;
		}
    }
    

    return 1000;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_InversionSettings[FLT_INVERSION_SETTING_LASTONE] =
{
    {
        "Enable", ONOFF, 0, &(InversionMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "ColourInversion", "UseInversionFilter", NULL,
    },
};

FILTER_METHOD InversionMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Colour Inversion",
    "&Colour Inversion",
    FALSE,
    TRUE,
    FilterInversion, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_INVERSION_SETTING_LASTONE,
    FLT_InversionSettings,
    WM_FLT_INVERSION_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_INVERSION,
};

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &InversionMethod;
}

