/////////////////////////////////////////////////////////////////////////////
// $Id: DI_TomsMoComp.c,v 1.8 2008-02-08 13:43:20 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Tom Barry.  All rights reserved.
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
//  Also, this program is "Philanthropy-Ware".  That is, if you like it and 
//  feel the need to reward or inspire the author then please feel free (but
//  not obligated) to consider joining or donating to the Electronic Frontier
//  Foundation. This will help keep cyber space free of barbed wire and bullsh*t.  
//  See www.eff.org for details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 04 May 2002   Tom Barry             Added TomsMoComp Deinterlace method
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2003/06/17 12:46:29  adcockj
// Added Help for new deinterlace methods
//
// Revision 1.6  2003/03/25 10:13:10  laurentg
// Allow running of TomsMoComp SSE and 3DNOW methods with Toms'agreement
//
// Revision 1.5  2002/12/10 16:32:19  adcockj
// Fix StrangeBob for MMX
//
// Revision 1.4  2002/11/26 21:32:14  adcockj
// Made new strange bob method optional
//
// Revision 1.3  2002/07/08 18:16:43  adcockj
// final fixes fro alpha 3
//
// Revision 1.2  2002/07/08 17:44:58  adcockj
// Corrected Settings messages
//
// Revision 1.1  2002/07/07 20:07:24  trbarry
// First cut at TomsMoComp, motion compensated deinterlace
//
// Revision 1.0  2002/05/04 16:13:33  trbarry
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include <DS_Deinterlace.h>
#define USE_FOR_DSCALER
#include "TomsMoComp.h"
#include "..\help\helpids.h"

long SearchEffort = 03;
BOOL UseStrangeBob = FALSE;

/*
#define IS_SSE 1
#include "DI_TomsMoComp.asm"
#undef IS_SSE

#define IS_3DNOW 1
#include "DI_TomsMoComp.asm"
#undef IS_3DNOW

#define IS_MMX 1
#include "DI_TomsMoComp.asm"
#undef IS_MMX
*/

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_TOMSMOCOMPSETTINGS[DI_TOMSMOCOMP_SETTING_LASTONE] =
{
    {
        "Search Effort", SLIDER, 0, &SearchEffort,
        5, 0, 255, 1, 1,
        NULL,
        "Deinterlace", "SearchEffort", NULL,
    },
    {
        "Use Strange Bob", YESNO, 0, &UseStrangeBob,
        0, 0, 1, 1, 1,
        NULL,
        "Deinterlace", "UseStrangeBob", NULL,
    },
};

DEINTERLACE_METHOD TomsMoCompMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Video (TomsMoComp)", 
    "TomsMoComp",
    FALSE, 
    FALSE, 
    DeinterlaceTomsMoComp_SSE, 
    50, 
    60,
    DI_TOMSMOCOMP_SETTING_LASTONE,
    DI_TOMSMOCOMPSETTINGS,
    INDEX_VIDEO_TOMSMOCOMP,
    NULL,
    NULL,
    NULL,
    NULL,
    4,									// number fields needed
    0,
    0,
    WM_DI_TOMSMOCOMP_GETVALUE - WM_APP,
    NULL,
    0,
    FALSE,
    FALSE,
    IDH_TOMSMOCOMP,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    if (CpuFeatureFlags & FEATURE_SSE2)
    {
		// there doesn't really seem to be much advantage in performance
		// of the SSE2 version of this. Let's not maintain it for now
	    TomsMoCompMethod.pfnAlgorithm = DeinterlaceTomsMoComp_SSE; 
//	    TomsMoCompMethod.pfnAlgorithm = DeinterlaceTomsMoComp_SSE2; 
    }
    else if (CpuFeatureFlags & FEATURE_SSE)
    {
	    TomsMoCompMethod.pfnAlgorithm = DeinterlaceTomsMoComp_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
		TomsMoCompMethod.pfnAlgorithm = DeinterlaceTomsMoComp_3DNOW;
    }
    else
    {
        TomsMoCompMethod.pfnAlgorithm = DeinterlaceTomsMoComp_MMX;
    }

    return &TomsMoCompMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
