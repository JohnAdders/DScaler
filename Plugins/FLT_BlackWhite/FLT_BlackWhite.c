/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_BlackWhite.c,v 1.2 2003-03-22 13:16:26 laurentg Exp $
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
// Change Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2003/03/13 00:43:59  laurentg
// New filter black & white
//
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

FILTER_METHOD BlackWhiteMethod;

long BlackWhite(TDeinterlaceInfo* pInfo)
{
    char* Pixels;
    int x;
    int y;
    int Cycles;

    // Need to have the current field to do the filtering.
    if (pInfo->PictureHistory[0] == NULL)
    {
        return 1000;
    }

    Cycles = pInfo->LineLength / 2;

    for (y = 0; y < pInfo->FieldHeight; ++y)
    {
        Pixels = (char*)pInfo->PictureHistory[0]->pData + y * pInfo->InputPitch + 1;
		for (x = 0; x < Cycles; x++)
		{
			*Pixels = (char)128;
			Pixels+=2;
		}
    }
    

    return 1000;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_BlackWhiteSettings[FLT_BLACKWHITE_SETTING_LASTONE] =
{
    {
        "Enable", ONOFF, 0, &(BlackWhiteMethod.bActive),
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

