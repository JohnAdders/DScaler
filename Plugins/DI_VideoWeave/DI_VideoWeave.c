/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock, Tom Barry, Steve Grimm  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 30 Dec 2000   Mark Rejhon           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2002/06/18 19:46:08  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.5  2002/06/13 12:10:25  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.4  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"
#include "..\help\helpids.h"

long TemporalTolerance = 300;
long SpatialTolerance = 600;
long SimilarityThreshold = 25;

#define IS_SSE 1
#include "DI_VideoWeave.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "DI_VideoWeave.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "DI_VideoWeave.asm"
#undef IS_MMX

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_VideoWeaveSettings[DI_VIDEOWEAVE_SETTING_LASTONE] =
{
    {
        "Temporal Tolerance", SLIDER, 0, &TemporalTolerance,
        300, 0, 5000, 10, 1,
        NULL,
        "Deinterlace", "TemporalTolerance", NULL,
    },
    {
        "Spatial Tolerance", SLIDER, 0, &SpatialTolerance,
        600, 0, 5000, 10, 1,
        NULL,
        "Deinterlace", "SpatialTolerance", NULL,
    },
    {
        "Similarity Threshold", SLIDER, 0, &SimilarityThreshold,
        25, 0, 255, 1, 1,
        NULL,
        "Deinterlace", "SimilarityThreshold", NULL,
    },
};

DEINTERLACE_METHOD VideoWeaveMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Video Deinterlace (Weave)", 
    "Weave", 
    FALSE, 
    FALSE, 
    DeinterlaceFieldWeave_MMX, 
    50, 
    60,
    DI_VIDEOWEAVE_SETTING_LASTONE,
    DI_VideoWeaveSettings,
    INDEX_VIDEO_WEAVE,
    NULL,
    NULL,
    NULL,
    NULL,
    3,
    0,
    0,
    WM_DI_VIDEOWEAVE_GETVALUE - WM_APP,
    NULL,
    0,
    FALSE,
    FALSE,
    IDH_VIDEOWEAVE,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE)
    {
        VideoWeaveMethod.pfnAlgorithm = DeinterlaceFieldWeave_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        VideoWeaveMethod.pfnAlgorithm = DeinterlaceFieldWeave_3DNOW;
    }
    else
    {
        VideoWeaveMethod.pfnAlgorithm = DeinterlaceFieldWeave_MMX;
    }
    return &VideoWeaveMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
