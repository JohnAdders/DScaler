/////////////////////////////////////////////////////////////////////////////
// $Id: DI_Adaptive.c,v 1.18 2008-02-08 13:43:19 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Mark Rejhon and Steve Grimm.  All rights reserved.
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
// 07 Jan 2001   John Adcock           Split into separate module
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.17  2003/02/14 21:53:49  laurentg
// TomsMoComp used as default mode instead of GreedyHM for low and high motion modes
//
// Revision 1.16  2002/06/18 19:46:06  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.15  2002/06/13 12:10:23  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.14  2002/06/13 08:22:07  adcockj
// Changed default motion methods to greedyHM
//
// Revision 1.13  2001/11/21 15:21:40  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.12  2001/07/13 16:13:32  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"
#include "..\help\helpids.h"

long        StaticImageFieldCount = 100;
long        LowMotionFieldCount = 4;
long        StaticImageMode = INDEX_WEAVE;
long        LowMotionMode = INDEX_VIDEO_2FRAME;
long        HighMotionMode = INDEX_VIDEO_2FRAME;
long        AdaptiveThres32Pulldown = 15;
long        AdaptiveThresPulldownMismatch = 900;

long NumVideoModes = 0;
long CurrentIndex = -1;
DEINTERLACE_METHOD** DeintMethods = NULL;
DEINTERLACE_METHOD* CurrentMethod = NULL;
DEINTERLACEPLUGINSETSTATUS* pfnSetStatus;
LPCSTR ModeList[100];


///////////////////////////////////////////////////////////////////////////////
// UpdateAdaptiveMode
//
// Switches to a new adaptive mode.  Updates the status bar if needed.
///////////////////////////////////////////////////////////////////////////////
void UpdateAdaptiveMode(long Index)
{
    int i;
    BOOL bFound = FALSE;

    if (CurrentIndex == Index && CurrentMethod != NULL)
        return;

    if (Index == INDEX_ADAPTIVE)
    {
        CurrentMethod = DeintMethods[0];
        if(pfnSetStatus != NULL)
        {
            pfnSetStatus("Adaptive - Recursion Error");
        }
        return;
    }

    for(i = 0; i < NumVideoModes && bFound == FALSE; i++)
    {
        if(DeintMethods[i]->nMethodIndex == Index)
        {
            CurrentMethod = DeintMethods[i];
            bFound = TRUE;
            if(pfnSetStatus != NULL)
            {
                static char AdaptiveName[200];
                char* ModeName;

                ModeName = DeintMethods[i]->szShortName;
                if (ModeName == NULL)
                {
                    ModeName = DeintMethods[i]->szName;
                }
                wsprintf(AdaptiveName, "Adaptive - %s", ModeName);
                pfnSetStatus(AdaptiveName);
            }
            CurrentIndex = Index;
        }
    }

    if(bFound == FALSE)
    {
        CurrentMethod = DeintMethods[0];
        if(pfnSetStatus != NULL)
        {
            pfnSetStatus("Adaptive - Error Finding Index");
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// DeinterlaceAdaptive
//
// This mode supports three styles of deinterlacing and switch among
// them depending on the amount of motion in the scene.  If there's a lot of
// motion, we use HighMotionMode.  If there's little or no motion for at least
// LowMotionFieldCount fields, we switch to LowMotionMode.  And if, after that,
// there's no motion for StaticImageFieldCount fields, we switch to
// StaticImageMode.
//
// Exactly which modes are used in these three cases is configurable in the INI
// file. It is entirely legal for some or all of the three video modes to be the
// same; by default, VIDEO_MODE_2FRAME is used for both the high and low motion
// modes.  On slower machines VIDEO_MODE_BOB (high) and VIDEO_MODE_WEAVE (low)
// can be used instead since they're less CPU-intensive.
///////////////////////////////////////////////////////////////////////////////
BOOL DeinterlaceAdaptive(TDeinterlaceInfo* pInfo)
{
    static long StaticMatchCount = 0;
    static long LowMatchCount = 0;

    // If this is our first time, update the current adaptive mode to whatever
    // the ini file said our high-motion mode should be.
    if (CurrentIndex == -1 || CurrentMethod == NULL)
    {
        UpdateAdaptiveMode(HighMotionMode);
        StaticMatchCount = 0;
        LowMatchCount = 0;
    }

    // reset MATCH_COUNT when we are called and the pInfo
    // struct doesn't contain at least an odd and an even frame
    if(pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[1] == NULL)
    {
        return FALSE;
    }

    // if we have very liitle motion update
    // to static after StaticImageFieldCount consecutive 
    // static fields
    // also check if its OK to go into low motion mode
    if(pInfo->FieldDiff < AdaptiveThres32Pulldown)
    {
        StaticMatchCount++;
        LowMatchCount++;
        if(CurrentIndex != StaticImageMode &&
            StaticMatchCount >= StaticImageFieldCount)
        {
            UpdateAdaptiveMode(StaticImageMode);
        }
        else if(CurrentIndex == HighMotionMode &&
            LowMatchCount >= LowMotionFieldCount)
        {
            UpdateAdaptiveMode(LowMotionMode);
        }
    }
    // if there is some motion then
    // switch back to low from high only after LowMotionFieldCount
    // consecutive low or static fields
    // if we are in static then switch straight away
    else if(pInfo->FieldDiff < AdaptiveThresPulldownMismatch)
    {
        LowMatchCount++;
        StaticMatchCount = 0;

        if ((CurrentIndex == HighMotionMode &&
            LowMatchCount >= LowMotionFieldCount) ||
            CurrentIndex == StaticImageMode)
        {
            UpdateAdaptiveMode(LowMotionMode);
        }
    }
    // high levels of motion just switch to high mode
    else
    {
        LowMatchCount = 0;
        StaticMatchCount = 0;

        if(CurrentIndex != HighMotionMode)
        {
            UpdateAdaptiveMode(HighMotionMode);
        }
    }

    if(CurrentMethod != NULL)
    {
        return CurrentMethod->pfnAlgorithm(pInfo);
    }
    else
    {
        return FALSE;
    }
}

void __cdecl AdaptiveStart(long NumPlugIns, DEINTERLACE_METHOD** OtherPlugins, DEINTERLACEPLUGINSETSTATUS* SetStatus)
{
    int i;
    int j;
    
    DeintMethods = OtherPlugins;
    NumVideoModes = NumPlugIns;
    pfnSetStatus = SetStatus;

    for(i = 0; i < 100; i++)
    {
        ModeList[i] = "";
        for(j = 0; j < NumPlugIns; j++)
        {
            if(DeintMethods[j]->nMethodIndex == i)
            {
                ModeList[i] = DeintMethods[j]->szName;
                break;
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_AdaptiveSettings[DI_ADAPTIVE_SETTING_LASTONE] =
{
    {
        "Low Motion Field Count", SLIDER, 0, &LowMotionFieldCount,
        4, 1, 1000, 1, 1,
        NULL,
        "Pulldown", "LowMotionFieldCount", NULL,
    },
    {
        "Static Image Field Count", SLIDER, 0, &StaticImageFieldCount,
        100, 1, 1000, 1, 1,
        NULL,
        "Pulldown", "StaticImageFieldCount", NULL,
    },
    {
        "Static Image Mode", ITEMFROMLIST, 0, &StaticImageMode,
        INDEX_WEAVE, 0, 99, 1, 1,
        ModeList,
        "Pulldown", "StaticImageMode", NULL,
    },
    {
        "Low Motion Mode", ITEMFROMLIST, 0, &LowMotionMode,
        INDEX_VIDEO_TOMSMOCOMP, 0, 99, 1, 1,
        ModeList,
        "Pulldown", "LowMotionMode", NULL,
    },
    {
        "High Motion Mode", ITEMFROMLIST, 0, &HighMotionMode,
        INDEX_VIDEO_TOMSMOCOMP, 0, 99, 1, 1,
        ModeList,
        "Pulldown", "HighMotionMode", NULL,
    },
    {
        "Adaptive Threshold 3:2 Pulldown", SLIDER, 0, &AdaptiveThres32Pulldown,
        15, 1, 5000, 5, 1,
        NULL,
        "Pulldown", "AdaptiveThres32Pulldown", NULL,
    },
    {
        "Adaptive Threshold 3:2 Pulldown Mismatch", SLIDER, 0, &AdaptiveThresPulldownMismatch,
        900, 1, 10000, 10, 1,
        NULL,
        "Pulldown", "AdaptiveThresPulldownMismatch", NULL,
    },
};

DEINTERLACE_METHOD AdaptiveMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Adaptive", 
    NULL, 
    FALSE, 
    FALSE, 
    DeinterlaceAdaptive, 
    50, 
    60,
    DI_ADAPTIVE_SETTING_LASTONE,
    DI_AdaptiveSettings,
    INDEX_ADAPTIVE,
    NULL,
    AdaptiveStart,
    NULL,
    NULL,
    4,
    0,
    0,
    WM_DI_ADAPTIVE_GETVALUE - WM_APP,
    NULL,
    0,
    TRUE,
    FALSE,
    IDH_ADAPTIVE,
};

__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    return &AdaptiveMethod;
}

BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
