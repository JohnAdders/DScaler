/////////////////////////////////////////////////////////////////////////////
// $Id: Deinterlace.cpp,v 1.42 2002-10-29 11:05:28 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// 24 Jul 2000   John Adcock           Put all my deinterlacing code into this
//                                     file
//
// 09 Nov 2000   Tom Barry             Added Blended Clipping Deinterlace method
//
// 30 Dec 2000   Mark Rejhon           Split out deinterlace routines
//                                     into separate modules
//
// 05 Jan 2001   John Adcock           Added flip frequencies to DeintMethods
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out memcpySSE as separate function
//                                     Changed DeintMethods to reflect the two
//                                     film Mode functions replacing the one before
//                                     Moved CombFactor and CompareFields to new file
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.41  2002/09/11 18:19:40  adcockj
// Prelimainary support for CX2388x based cards
//
// Revision 1.40  2002/08/09 09:40:25  laurentg
// Cut the deinterlace menu only if there are a lot of deinterlace modes
//
// Revision 1.39  2002/07/29 21:33:06  laurentg
// "Show Video Method UI" feature restored
//
// Revision 1.38  2002/07/27 16:27:35  laurentg
// Deinterlace and Settings menus updated
//
// Revision 1.37  2002/06/18 19:46:06  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.36  2002/06/13 12:10:21  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.35  2002/06/07 16:38:34  robmuller
// Added missing FindClose() statement.
//
// Revision 1.34  2002/02/18 20:51:52  laurentg
// Statistics regarding deinterlace modes now takes into account the progressive mode
// Reset of the deinterlace statistics at each start of the decoding thread
// Reset action now resets the deinterlace statistics too
//
// Revision 1.33  2002/02/17 21:41:03  laurentg
// Action "Find and Lock Film mode" added
//
// Revision 1.32  2002/02/11 23:18:33  laurentg
// Creation of a DEINTERLACE_METHOD data structure for the progressive mode
//
// Revision 1.31  2002/02/10 21:42:29  laurentg
// New menu items "Progressive Scan" and "Film Mode"
//
// Revision 1.30  2001/12/18 14:45:05  adcockj
// Moved to Common Controls status bar
//
// Revision 1.29  2001/12/16 13:13:34  laurentg
// New statistics
//
// Revision 1.28  2001/12/15 17:41:55  adcockj
// Fix for crash in greedy when in pulldown comb mode
//
// Revision 1.27  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.26  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.25  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.24  2001/11/02 16:33:07  adcockj
// Removed conflict tags
//
// Revision 1.23  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.22  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.21  2001/08/18 17:23:51  adcockj
// Fix for manual method selection
//
// Revision 1.20  2001/08/14 11:36:03  adcockj
// Mixer change to allow restore of initial mixer settings
//
// Revision 1.19.2.4  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.19.2.3  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.19.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.19.2.1  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.19  2001/08/14 07:06:27  adcockj
// Fixed problem with scroling through methods
//
// Revision 1.18  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.17  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.16  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.15  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Deinterlace.h"
#include "CPU.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Prog.h"
#include "AspectRatio.h"
#include "Status.h"
#include "OSD.h"
#include "DebugLog.h"
#include "Providers.h"

// Statistics
long    nInitialTicks = -1;
long    nLastTicks = 0;
long    nTotalDeintModeChanges = 0;

DEINTERLACE_METHOD ProgressiveMethods[PROGPULLDOWNMODES_LAST_ONE] =
{
    // PROG_22_PULLDOWN_ODD
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "2:2 Pulldown Flip on Odd", "2:2 Odd", FALSE, TRUE, ProgMode, 25, 30, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 1, 0, 0, -1, NULL, 0, FALSE, FALSE, 
    },
    // PROG_22_PULLDOWN_EVEN
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "2:2 Pulldown Flip on Even", "2:2 Even", FALSE, TRUE, ProgMode, 25, 30, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 1, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_0
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 1st Full Frame", "3:2 1st", FALSE, TRUE, ProgMode, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_1
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 2nd Full Frame", "3:2 2nd", FALSE, TRUE, ProgMode, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_2
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 3rd Full Frame", "3:2 3rd", FALSE, TRUE, ProgMode, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_3
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 4th Full Frame", "3:2 4th", FALSE, TRUE, ProgMode, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_4
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 5th Full Frame", "3:2 5th", FALSE, TRUE, ProgMode, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // NORMAL_PROGRESSIVE
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "Progressive Scan", NULL, FALSE, FALSE, ProgMode, 50, 60, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 1, 0, 0, -1, NULL, 0, FALSE, FALSE, 
    },
};

DEINTERLACE_METHOD FilmDeintMethods[FILMPULLDOWNMODES_LAST_ONE] =
{
    // FILM_22_PULLDOWN_ODD
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "2:2 Pulldown Flip on Odd", "2:2 Odd", FALSE, TRUE, FilmModePALOdd, 25, 30, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE, 
    },
    // FILM_22_PULLDOWN_EVEN
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "2:2 Pulldown Flip on Even", "2:2 Even", FALSE, TRUE, FilmModePALEven, 25, 30, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_0
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 1st Full Frame", "3:2 1st", FALSE, TRUE, FilmModeNTSC1st, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_1
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 2nd Full Frame", "3:2 2nd", FALSE, TRUE, FilmModeNTSC2nd, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_2
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 3rd Full Frame", "3:2 3rd", FALSE, TRUE, FilmModeNTSC3rd, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_3
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 4th Full Frame", "3:2 4th", FALSE, TRUE, FilmModeNTSC4th, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_4
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Skip 5th Full Frame", "3:2 5th", FALSE, TRUE, FilmModeNTSC5th, 1000, 24, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 2, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_32_PULLDOWN_COMB
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "3:2 Pulldown Use Comb Info", "3:2 Comb", FALSE, TRUE, FilmModeNTSCComb, 1000, 60, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 3, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
    // FILM_22_PULLDOWN_COMB
    {
        sizeof(DEINTERLACE_METHOD), DEINTERLACE_CURRENT_VERSION,
        "2:2 Pulldown Use Comb Info", "2:2 Comb", FALSE, TRUE, FilmModePALComb, 1000, 60, 
        0, NULL, 0, NULL, NULL, NULL, NULL, 3, 0, 0, -1, NULL, 0, FALSE, FALSE,
    },
};

const char* DeinterlaceNames[100];

long NumVideoModes = 0;
DEINTERLACE_METHOD* VideoDeintMethods[100] = {NULL,};
BOOL bIsFilmMode = FALSE;

long gVideoPulldownMode = 0;
eFilmPulldownMode gFilmPulldownMode = FILMPULLDOWNMODES_LAST_ONE;
eProgressivePulldownMode gProgMode = NORMAL_PROGRESSIVE;

BOOL bIsProgressiveMode = FALSE;

void Deinterlace_SetStatus(LPCSTR StatusText)
{
    StatusBar_ShowText(STATUS_MODE, StatusText);
}

void ResetDeinterlaceStats()
{
    DWORD CurrentTickCount = GetTickCount();
    int i;

    for(i = 0; i < PROGPULLDOWNMODES_LAST_ONE; i++)
    {
        ProgressiveMethods[i].ModeChanges = 0;
        ProgressiveMethods[i].ModeTicks = 0;
    }
    for(i = 0; i < FILMPULLDOWNMODES_LAST_ONE; i++)
    {
        FilmDeintMethods[i].ModeChanges = 0;
        FilmDeintMethods[i].ModeTicks = 0;
    }
    for(i = 0; i < NumVideoModes; i++)
    {
        VideoDeintMethods[i]->ModeChanges = 0;
        VideoDeintMethods[i]->ModeTicks = 0;
    }

    nInitialTicks = CurrentTickCount;
    nLastTicks = CurrentTickCount;

    GetCurrentDeintMethod()->ModeChanges++;
    nTotalDeintModeChanges = 1;
}

DEINTERLACE_METHOD* GetCurrentDeintMethod()
{
    if(bIsProgressiveMode)
    {
        return ProgressiveMethods + gProgMode;
    }
    else if(bIsFilmMode)
    {
        return FilmDeintMethods + gFilmPulldownMode;
    }
    else
    {
        return VideoDeintMethods[gVideoPulldownMode];
    }
}

DEINTERLACE_METHOD* GetProgressiveMethod(int Mode)
{
    return &(ProgressiveMethods[Mode]);
}

DEINTERLACE_METHOD* GetVideoDeintMethod(int Mode)
{
    if(Mode < NumVideoModes)
    {
        return VideoDeintMethods[Mode];
    }
    else
    {
        return NULL;
    }
}

DEINTERLACE_METHOD* GetVideoDeintIndex(int Index)
{
    int i;
    for(i = 0; i < NumVideoModes; i++)
    {
        if(VideoDeintMethods[i]->nMethodIndex == Index)
        {
            return VideoDeintMethods[i];
        }
    }
    //must get something
    return VideoDeintMethods[0];
}

DEINTERLACE_METHOD* GetFilmDeintMethod(eFilmPulldownMode Mode)
{
    if(Mode < FILMPULLDOWNMODES_LAST_ONE)
    {
        return FilmDeintMethods + Mode;
    }
    else
    {
        return NULL;
    }
}

BOOL IsProgressiveMode()
{
    return bIsProgressiveMode;
}

BOOL IsFilmMode()
{
    return bIsFilmMode;
}

BOOL InHalfHeightMode()
{
    if(bIsFilmMode || bIsProgressiveMode)
    {
        return FALSE;
    }
    else
    {
        return VideoDeintMethods[gVideoPulldownMode]->bIsHalfHeight;
    }
}

eFilmPulldownMode GetFilmMode()
{
    if(bIsFilmMode)
    {
        return gFilmPulldownMode;
    }
    else
    {
        return FILMPULLDOWNMODES_LAST_ONE;
    }
}

eProgressivePulldownMode GetProgMode()
{
    return gProgMode;
}


BOOL SetProgressiveMode()
{
    if (!bIsProgressiveMode)
    {
        DWORD CurrentTickCount = GetTickCount();
        BOOL WereInHalfHeight = InHalfHeightMode();

        if (nInitialTicks == -1)
        {
            nInitialTicks = CurrentTickCount;
            nLastTicks = CurrentTickCount;
        }
        else
        {
            if(bIsFilmMode == TRUE)
            {
                FilmDeintMethods[gFilmPulldownMode].ModeTicks += CurrentTickCount - nLastTicks;
            }
            else
            {
                VideoDeintMethods[gVideoPulldownMode]->ModeTicks += CurrentTickCount - nLastTicks;
            }
        }

        bIsProgressiveMode = TRUE;
        nLastTicks = CurrentTickCount;
        Deinterlace_SetStatus(GetDeinterlaceModeName());
        nTotalDeintModeChanges++;
        gProgMode = NORMAL_PROGRESSIVE;
        if(WereInHalfHeight != InHalfHeightMode())
        {
            WorkoutOverlaySize(TRUE);
        }

        return TRUE;
    }
    return FALSE;
}

BOOL UnsetProgressiveMode()
{
    if (bIsProgressiveMode)
    {
        DWORD CurrentTickCount = GetTickCount();

        if (nInitialTicks == -1)
        {
            nInitialTicks = CurrentTickCount;
            nLastTicks = CurrentTickCount;
        }
        else
        {
            ProgressiveMethods[gProgMode].ModeTicks += CurrentTickCount - nLastTicks;
        }

        bIsProgressiveMode = FALSE;
        nLastTicks = CurrentTickCount;
        Deinterlace_SetStatus(GetDeinterlaceModeName());
        nTotalDeintModeChanges++;
        if(bIsFilmMode == TRUE)
        {
            FilmDeintMethods[gFilmPulldownMode].ModeChanges++;
        }
        else
        {
            VideoDeintMethods[gVideoPulldownMode]->ModeChanges++;
        }
        if(InHalfHeightMode())
        {
            WorkoutOverlaySize(TRUE);
        }
        
        return TRUE;
    }
    return FALSE;
}

void SetFilmDeinterlaceMode(eFilmPulldownMode Mode)
{
    if(bIsProgressiveMode)
    {
        if(Mode <= FILM_32_PULLDOWN_4)
        {
            DWORD CurrentTickCount = GetTickCount();

            if (nInitialTicks == -1)
            {
                nInitialTicks = CurrentTickCount;
                nLastTicks = CurrentTickCount;
            }
            else
            {
                ProgressiveMethods[gProgMode].ModeTicks += CurrentTickCount - nLastTicks;
            }
            gProgMode = (eProgressivePulldownMode)Mode;
            ProgressiveMethods[gProgMode].ModeChanges++;
            nLastTicks = CurrentTickCount;
            bIsFilmMode = TRUE;
            Deinterlace_SetStatus(GetDeinterlaceModeName());
        }
    }
    else
    {
        if (gFilmPulldownMode != Mode || bIsFilmMode == FALSE)
        {
            DWORD CurrentTickCount = GetTickCount();
            BOOL WereInHalfHeight = InHalfHeightMode();

            if (nInitialTicks == -1)
            {
                nInitialTicks = CurrentTickCount;
                nLastTicks = CurrentTickCount;
            }
            else
            {
                if(bIsFilmMode == TRUE)
                {
                    FilmDeintMethods[gFilmPulldownMode].ModeTicks += CurrentTickCount - nLastTicks;
                }
                else
                {
                    VideoDeintMethods[gVideoPulldownMode]->ModeTicks += CurrentTickCount - nLastTicks;
                }
            }
            gFilmPulldownMode = Mode;
            bIsFilmMode = TRUE;
            nLastTicks = CurrentTickCount;
            Deinterlace_SetStatus(GetDeinterlaceModeName());
            nTotalDeintModeChanges++;
            gProgMode = NORMAL_PROGRESSIVE;
            FilmDeintMethods[gProgMode].ModeChanges++;
            if(WereInHalfHeight != InHalfHeightMode())
            {
                WorkoutOverlaySize(TRUE);
            }
        }
    }
}

void SetVideoDeinterlaceMode(int Mode)
{
    if(bIsProgressiveMode)
    {
        if(bIsFilmMode == TRUE)
        {
            DWORD CurrentTickCount = GetTickCount();

            if (nInitialTicks == -1)
            {
                nInitialTicks = CurrentTickCount;
                nLastTicks = CurrentTickCount;
            }
            else
            {
                ProgressiveMethods[gProgMode].ModeTicks += CurrentTickCount - nLastTicks;
            }
            gProgMode = NORMAL_PROGRESSIVE;
            ProgressiveMethods[gProgMode].ModeChanges++;
            nLastTicks = CurrentTickCount;
            bIsFilmMode = FALSE;
            Deinterlace_SetStatus(GetDeinterlaceModeName());
        }
    }
    else
    {
        if (gVideoPulldownMode != Mode || bIsFilmMode == TRUE)
        {
            DWORD CurrentTickCount = GetTickCount();
            BOOL WereInHalfHeight = InHalfHeightMode();

            if (nInitialTicks == -1)
            {
                nInitialTicks = CurrentTickCount;
                nLastTicks = CurrentTickCount;
            }
            else
            {
                if(bIsFilmMode == TRUE)
                {
                    FilmDeintMethods[gFilmPulldownMode].ModeTicks += CurrentTickCount - nLastTicks;
                }
                else
                {
                    VideoDeintMethods[gVideoPulldownMode]->ModeTicks += CurrentTickCount - nLastTicks;
                }
            }
            gVideoPulldownMode = Mode;
            bIsFilmMode = FALSE;
            nLastTicks = CurrentTickCount;
            Deinterlace_SetStatus(GetDeinterlaceModeName());
            nTotalDeintModeChanges++;
            VideoDeintMethods[gVideoPulldownMode]->ModeChanges++;
            if(WereInHalfHeight != InHalfHeightMode())
            {
                WorkoutOverlaySize(TRUE);
            }
        }
    }
}

void SetVideoDeinterlaceIndex(int index)
{
    int i;
    for(i = 0; i < NumVideoModes; i++)
    {
        if(VideoDeintMethods[i]->nMethodIndex == index)
        {
            SetVideoDeinterlaceMode(i);
            return;
        }
    }
    //must set it to something
    SetVideoDeinterlaceMode(0);
}

char* GetDeinterlaceModeName()
{
    if(bIsProgressiveMode)
    {
        return ProgressiveMethods[gProgMode].szName;
    }
    else if(bIsFilmMode)
    {
        return FilmDeintMethods[gFilmPulldownMode].szName;
    }
    else
    {
        return VideoDeintMethods[gVideoPulldownMode]->szName;
    }
}

void PrepareDeinterlaceMode()
{
    bIsProgressiveMode = FALSE;
    bIsFilmMode = FALSE;
    eVideoFormat VideoFormat(Providers_GetCurrentSource()->GetFormat());

    if(GetTVFormat(VideoFormat)->Is25fps)
    {
        SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
    }
    else
    {
        SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
    }
    // If that didn't work then go into whatever they loaded up first
    if(gVideoPulldownMode == -1)
    {
        SetVideoDeinterlaceMode(0);
    }
    // Update the status bar
    Deinterlace_SetStatus(GetDeinterlaceModeName());
}

void IncrementDeinterlaceMode()
{
    long Mode;
    if(bIsProgressiveMode)
    {
        return;
    }
    else if(bIsFilmMode)
    {
        Mode = gFilmPulldownMode;
        Mode++;
        if(Mode == FILM_32_PULLDOWN_4 + 1)
        {
            SetVideoDeinterlaceMode(0);
        }
        else
        {
            SetFilmDeinterlaceMode((eFilmPulldownMode)Mode);
        }
    }
    else
    {
        Mode = gVideoPulldownMode;
        Mode++;
        if(Mode == NumVideoModes)
        {
            SetFilmDeinterlaceMode((eFilmPulldownMode)(0));
        }
        else
        {
            SetVideoDeinterlaceMode(Mode);
        }
    }
}

void DecrementDeinterlaceMode()
{
    long Mode;
    if(bIsProgressiveMode)
    {
        return;
    }
    else if(bIsFilmMode)
    {
        Mode = gFilmPulldownMode;
        Mode--;
        if(Mode < 0)
        {
            SetVideoDeinterlaceMode(NumVideoModes - 1);
        }
        else
        {
            SetFilmDeinterlaceMode((eFilmPulldownMode)Mode);
        }
    }
    else
    {
        Mode = gVideoPulldownMode;
        Mode--;
        if(Mode < 0)
        {
            SetFilmDeinterlaceMode((eFilmPulldownMode)(FILM_32_PULLDOWN_4));
        }
        else
        {
            SetVideoDeinterlaceMode(Mode);
        }
    }
}

void ShowVideoModeUI()
{
    if(VideoDeintMethods[gVideoPulldownMode]->pfnPluginShowUI != NULL)
    {
        VideoDeintMethods[gVideoPulldownMode]->pfnPluginShowUI(hWnd);
    }
}

BOOL ProcessDeinterlaceSelection(HWND hWnd, WORD wMenuID)
{
    int     nDeinterlaceIndex = 0;
    int     bFound = FALSE;
    int     i;

    if(bIsProgressiveMode)
    {
        return FALSE;
    }

    if(wMenuID >= IDM_FIRST_DEINTMETHOD && wMenuID <= IDM_LAST_DEINTMETHOD)
    {
        // Usually done through other means than DScaler menus
        bFound = TRUE;
        nDeinterlaceIndex = wMenuID - IDM_FIRST_DEINTMETHOD;
        SetVideoDeinterlaceIndex(wMenuID - IDM_FIRST_DEINTMETHOD);
        OSD_ShowText(hWnd, GetDeinterlaceModeName(), 0);
    }
    else
    {
        // Usually done through the DScaler menus
        for(i = 0; i < NumVideoModes; i++)
        {
            if(wMenuID == VideoDeintMethods[i]->MenuId)
            {
                bFound = TRUE;
                nDeinterlaceIndex = VideoDeintMethods[i]->nMethodIndex;
                if(!bIsFilmMode || (Setting_GetValue(OutThreads_GetSetting(AUTODETECT)) == FALSE))
                {
                    SetVideoDeinterlaceMode(i);
                    OSD_ShowText(hWnd, GetDeinterlaceModeName(), 0);
                }
                else
                {
                    gVideoPulldownMode = i;
                }
                break;
            }
        }
    }

    // Now save the current deinterlace setting
    if (bFound)
    {
        eVideoFormat VideoFormat(Providers_GetCurrentSource()->GetFormat());
        if(GetTVFormat(VideoFormat)->Is25fps)
        {
            Setting_SetValue(FD50_GetSetting(PALFILMFALLBACKMODE), nDeinterlaceIndex);
        }
        else
        {
            Setting_SetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE), nDeinterlaceIndex);
        }
    }
    return bFound;
}

void LoadDeintPlugin(LPCSTR szFileName)
{
    GETDEINTERLACEPLUGININFO* pfnGetDeinterlacePluginInfo;
    DEINTERLACE_METHOD* pMethod;
    HMODULE hPlugInMod;

    hPlugInMod = LoadLibrary(szFileName);
    if(hPlugInMod == NULL)
    {
        return;
    }
    
    pfnGetDeinterlacePluginInfo = (GETDEINTERLACEPLUGININFO*)GetProcAddress(hPlugInMod, "GetDeinterlacePluginInfo");
    if(pfnGetDeinterlacePluginInfo == NULL)
    {
        return;
    }

    pMethod = pfnGetDeinterlacePluginInfo(CpuFeatureFlags);
    if(pMethod != NULL)
    {
        if(pMethod->SizeOfStructure == sizeof(DEINTERLACE_METHOD) &&
            pMethod->DeinterlaceStructureVersion >= DEINTERLACE_VERSION_3)
        {
            VideoDeintMethods[NumVideoModes] = pMethod;
            pMethod->hModule = hPlugInMod;
            
            // read in settings
            for(int i = 0; i < pMethod->nSettings; i++)
            {
                Setting_ReadFromIni(&(pMethod->pSettings[i]));
            }
            if(pMethod->pfnPluginInit != NULL)
            {
                pMethod->pfnPluginInit();
            }
            NumVideoModes++;
        }
    }
}

void UnloadDeinterlacePlugins()
{
    int i;
    for(i = 0; i < NumVideoModes; i++)
    {
        if(VideoDeintMethods[i]->pfnPluginExit!=NULL)
        {
            VideoDeintMethods[i]->pfnPluginExit();
        }
        FreeLibrary(VideoDeintMethods[i]->hModule);
        VideoDeintMethods[i] = NULL;
    }
    NumVideoModes = 0;
}

int DeintMethodCompare(const void* arg1, const void* arg2)
{
    DEINTERLACE_METHOD* pMethod1 = *(DEINTERLACE_METHOD**)arg1;
    DEINTERLACE_METHOD* pMethod2 = *(DEINTERLACE_METHOD**)arg2;

    if(pMethod1->nMethodIndex < pMethod2->nMethodIndex)
    {
        return -1;
    }
    else if(pMethod1->nMethodIndex > pMethod2->nMethodIndex)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void AddUIForDeintPlugin(HMENU hMenu, DEINTERLACE_METHOD* DeintMethod)
{
    static MenuId = 6000;
    if(DeintMethod->MenuId == 0)
    {
        DeintMethod->MenuId = MenuId++;
    }
    AppendMenu(hMenu, MF_STRING | MF_ENABLED, DeintMethod->MenuId, DeintMethod->szName);
}

BOOL LoadDeinterlacePlugins()
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    int i;
    HMENU hMenu;

    hFindFile = FindFirstFile("DI_*.dll", &FindFileData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        BOOL RetVal = TRUE;
        while(RetVal != 0)
        {
            __try
            {
                LoadDeintPlugin(FindFileData.cFileName);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) 
            { 
                LOG(1, "Crash Loading %s", FindFileData.cFileName);
            }
            RetVal = FindNextFile(hFindFile, &FindFileData);
        }
        FindClose(hFindFile);
    }

    // put the plug-ins into index order
    // this should prevent confusion in the UI
    if(NumVideoModes > 1)
    {
        qsort((void*) VideoDeintMethods, NumVideoModes, sizeof(DEINTERLACE_METHOD*), DeintMethodCompare);
    }
    if(NumVideoModes > 0)
    {
        hMenu = GetVideoDeinterlaceSubmenu();
        if(hMenu == NULL) return FALSE;

        // Blank out the names list
        for(i = 0; i < 100; ++i)
        {
            DeinterlaceNames[i] = "";
        }

        for(i = 0; i < NumVideoModes; i++)
        {
            if(VideoDeintMethods[i]->pfnPluginStart != NULL)
            {
                VideoDeintMethods[i]->pfnPluginStart(NumVideoModes, VideoDeintMethods, Deinterlace_SetStatus);
            }
            AddUIForDeintPlugin(hMenu, VideoDeintMethods[i]);
            
            // update the names list which will be used in the generic settings 
            // dialog box
            DeinterlaceNames[VideoDeintMethods[i]->nMethodIndex] = VideoDeintMethods[i]->szName;
        }
        // Cut the menu in two colums if there are a lot of deinterlace video modes
        if (GetMenuItemCount(hMenu) > 29)
        {
            char Text[32];
            GetMenuString(hMenu, IDM_PROGRESSIVE_SCAN, Text, sizeof(Text), MF_BYCOMMAND);
            ModifyMenu(hMenu, IDM_PROGRESSIVE_SCAN, MF_MENUBARBREAK | MF_STRING, IDM_PROGRESSIVE_SCAN, Text);
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void GetDeinterlaceSettings(DEINTERLACE_METHOD**& DeinterlaceMethods,long& NumMethods)
{
	//ASSERT(ppSettings!=NULL && *pNumFilters!=NULL);
	DeinterlaceMethods = VideoDeintMethods;
	NumMethods = NumVideoModes;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
// there are no settings at the moment but here is a good place to set
// up the DeintModeNames array used where Modes are to be selected
/////////////////////////////////////////////////////////////////////////////
SETTING* Deinterlace_GetSetting(long nIndex, long Setting)
{
    if(nIndex < 0 || nIndex >= NumVideoModes)
    {
        return NULL;
    }
    if(Setting > -1 && Setting < VideoDeintMethods[nIndex]->nSettings)
    {
        return &(VideoDeintMethods[nIndex]->pSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

LONG Deinterlace_HandleSettingsMsg(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
    int i;
    LONG RetVal = 0;
    SETTING* pSetting;
    for(i = 0; i < NumVideoModes; i++)
    {
        if(message == (UINT)(WM_APP + VideoDeintMethods[i]->nSettingsOffset))
        {
            *bDone = TRUE;
            pSetting = Deinterlace_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                RetVal =  Setting_GetValue(pSetting);
            }
            break;
        }
        else if(message == (UINT)(WM_APP + VideoDeintMethods[i]->nSettingsOffset + 100))
        {
            *bDone = TRUE;
            pSetting = Deinterlace_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                Setting_SetValue(pSetting, lParam);
            }
            break;
        }
        else if(message == (UINT)(WM_APP + VideoDeintMethods[i]->nSettingsOffset + 200))
        {
            *bDone = TRUE;
            pSetting = Deinterlace_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                Setting_ChangeValue(pSetting, (eCHANGEVALUE)lParam);
            }
            break;
        }
    }
    return RetVal;
}


void Deinterlace_ReadSettingsFromIni()
{
    int i,j;
    for(i = 0; i < NumVideoModes; i++)
    {
        for(j = 0; j < VideoDeintMethods[i]->nSettings; j++)
        {
            Setting_ReadFromIni(&(VideoDeintMethods[i]->pSettings[j]));
        }
    }
}

void Deinterlace_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i,j;
    for(i = 0; i < NumVideoModes; i++)
    {
        for(j = 0; j < VideoDeintMethods[i]->nSettings; j++)
        {
            Setting_WriteToIni(&(VideoDeintMethods[i]->pSettings[j]), bOptimizeFileAccess);
        }
    }
}

void Deinterlace_SetMenu(HMENU hMenu)
{
    int i;


    EnableMenuItem(hMenu, IDM_PROGRESSIVE_SCAN, bIsProgressiveMode ? MF_ENABLED : MF_GRAYED);
    CheckMenuItemBool(hMenu, IDM_PROGRESSIVE_SCAN, (bIsProgressiveMode && !bIsFilmMode));

    if(bIsFilmMode)
    {
        if(bIsProgressiveMode)
        {
            CheckMenuItemBool(hMenu, IDM_22PULLODD, (gProgMode == PROG_22_PULLDOWN_ODD) );
            CheckMenuItemBool(hMenu, IDM_22PULLEVEN, (gProgMode == PROG_22_PULLDOWN_EVEN) );
            CheckMenuItemBool(hMenu, IDM_32PULL1, (gProgMode == PROG_32_PULLDOWN_0) );
            CheckMenuItemBool(hMenu, IDM_32PULL2, (gProgMode == PROG_32_PULLDOWN_1) );
            CheckMenuItemBool(hMenu, IDM_32PULL3, (gProgMode == PROG_32_PULLDOWN_2) );
            CheckMenuItemBool(hMenu, IDM_32PULL4, (gProgMode == PROG_32_PULLDOWN_3) );
            CheckMenuItemBool(hMenu, IDM_32PULL5, (gProgMode == PROG_32_PULLDOWN_4) );
        }
        else
        {
            CheckMenuItemBool(hMenu, IDM_22PULLODD, (gFilmPulldownMode == FILM_22_PULLDOWN_ODD) );
            CheckMenuItemBool(hMenu, IDM_22PULLEVEN, (gFilmPulldownMode == FILM_22_PULLDOWN_EVEN) );
            CheckMenuItemBool(hMenu, IDM_32PULL1, (gFilmPulldownMode == FILM_32_PULLDOWN_0) );
            CheckMenuItemBool(hMenu, IDM_32PULL2, (gFilmPulldownMode == FILM_32_PULLDOWN_1) );
            CheckMenuItemBool(hMenu, IDM_32PULL3, (gFilmPulldownMode == FILM_32_PULLDOWN_2) );
            CheckMenuItemBool(hMenu, IDM_32PULL4, (gFilmPulldownMode == FILM_32_PULLDOWN_3) );
            CheckMenuItemBool(hMenu, IDM_32PULL5, (gFilmPulldownMode == FILM_32_PULLDOWN_4) );
        }
    }
    else
    {
        CheckMenuItem(hMenu, IDM_22PULLODD, MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_22PULLEVEN, MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_32PULL1, MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_32PULL2, MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_32PULL3, MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_32PULL4, MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_32PULL5, MF_UNCHECKED);
    }

    for(i = 0; i < NumVideoModes; i++)
    {
        EnableMenuItem(hMenu, VideoDeintMethods[i]->MenuId, bIsProgressiveMode ? MF_GRAYED : MF_ENABLED);
        if(!bIsProgressiveMode && !bIsFilmMode)
        {
            CheckMenuItemBool(hMenu, VideoDeintMethods[i]->MenuId, (gVideoPulldownMode == i));
        }
        else
        {
            CheckMenuItemBool(hMenu, VideoDeintMethods[i]->MenuId, FALSE);
        }
    }

    EnableMenuItem(hMenu, IDM_DEINTERLACE_SHOWVIDEOMETHODUI, bIsProgressiveMode || (VideoDeintMethods[gVideoPulldownMode]->pfnPluginShowUI == NULL) ? MF_GRAYED : MF_ENABLED);
}
