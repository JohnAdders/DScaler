/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_Chroma.c,v 1.1 2003-01-02 13:15:01 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "math.h"
#include "..\help\helpids.h"

long VertShift = 0;
long HorzShift = 0;
BOOL UseLowPassFilter = FALSE;

FILTER_METHOD ChromaMethod;

long __cdecl FilterChroma(TDeinterlaceInfo* pInfo)
{
    if(VertShift == 0 && 
        HorzShift == 0 && 
        UseLowPassFilter == FALSE)
    {
        return 1000;
    }
    return 1000;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_ChromaSettings[FLT_CHROMA_SETTING_LASTONE] =
{
    {
        "Horizontal Shift", SLIDER, 0, &HorzShift,
        0, -32, 32, 1, 1,
        NULL,
        "ChromaFilter", "HorzShift", NULL,
    },
    {
        "Vertical Shift", SLIDER, 0, &VertShift,
        0, -32, 32, 1, 1,
        NULL,
        "ChromaFilter", "VertShift", NULL,
    },
    {
        "Vertical Low-Pass Filter", ONOFF, 0, &UseLowPassFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "ChromaFilter", "UseLowPassFilter", NULL,
    },
    {
        "Chroma Filter", ONOFF, 0, &(ChromaMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "ChromaFilter", "UseChromaFilter", NULL,
    },
};

FILTER_METHOD GammaMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Chroma Filter",
    "&Chroma",
    FALSE,
    TRUE,
    FilterChroma, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_CHROMA_SETTING_LASTONE,
    FLT_ChromaSettings,
    WM_FLT_CHROMA_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_FILTERS,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &GammaMethod;
}

