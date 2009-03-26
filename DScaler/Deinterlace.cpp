/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file Deinterlace.cpp Deinterlacing functions
 */

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
#include "DynamicFunction.h"
#include "SettingsMaster.h"

using namespace std;

// Statistics
long    nInitialTicks = -1;
long    nLastTicks = 0;
long    nTotalDeintModeChanges = 0;

vector< SmartPtr<CSettingsHolder> > DeinterlaceHolders;

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

///This function is called from the output thread.
void Deinterlace_SetStatus(LPCSTR StatusText)
{
    size_t len=strlen(StatusText);
    if(len>0)
    {
        LPSTR tmpstr=(LPSTR)malloc(len+1);
        strncpy(tmpstr,StatusText,len+1);
        PostMessageToMainWindow(UWM_DEINTERLACE_SETSTATUS, (WPARAM)tmpstr,0);
    }
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
        if(VideoDeintMethods[gVideoPulldownMode] != NULL)
        {
            return VideoDeintMethods[gVideoPulldownMode]->bIsHalfHeight;
        }
        else
        {
            return FALSE;
        }
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
            FilmDeintMethods[gFilmPulldownMode].ModeChanges++;
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
        SetVideoDeinterlaceIndex(Setting_GetValue(WM_FD50_GETVALUE, PALFILMFALLBACKMODE));
    }
    else
    {
        SetVideoDeinterlaceIndex(Setting_GetValue(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE));
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
        VideoDeintMethods[gVideoPulldownMode]->pfnPluginShowUI(GetMainWnd());
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
        OSD_ShowText(GetDeinterlaceModeName(), 0);
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
                if(!bIsFilmMode || (Setting_GetValue(WM_OUTTHREADS_GETVALUE, AUTODETECT) == FALSE))
                {
                    SetVideoDeinterlaceMode(i);
                    OSD_ShowText(GetDeinterlaceModeName(), 0);
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
            Setting_SetValue(WM_FD50_GETVALUE, PALFILMFALLBACKMODE, nDeinterlaceIndex);
        }
        else
        {
            Setting_SetValue(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE, nDeinterlaceIndex);
        }
    }
    return bFound;
}

void LoadDeintPlugin(LPCSTR szFileName)
{
    DynamicFunctionC1<DEINTERLACE_METHOD*, long> GetDeinterlacePluginInfo(szFileName, "GetDeinterlacePluginInfo");
    DEINTERLACE_METHOD* pMethod;

    if(!GetDeinterlacePluginInfo)
    {
        return;
    }

    pMethod = GetDeinterlacePluginInfo(CpuFeatureFlags);
    if(pMethod != NULL)
    {
        if(pMethod->SizeOfStructure == sizeof(DEINTERLACE_METHOD) &&
            pMethod->DeinterlaceStructureVersion >= DEINTERLACE_VERSION_3)
        {
            VideoDeintMethods[NumVideoModes] = pMethod;
            
            SmartPtr<CSettingsHolder> Holder(new CSettingsHolder(WM_APP + pMethod->nSettingsOffset, pMethod->HelpID));
            Holder->AddSettings(pMethod->pSettings, pMethod->nSettings);
            Holder->ReadFromIni();
            SettingsMaster->Register(Holder);

            if(pMethod->pfnPluginInit != NULL)
            {
                pMethod->pfnPluginInit();
            }
            NumVideoModes++;
            DeinterlaceHolders.push_back(Holder);
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
        VideoDeintMethods[i] = NULL;
        SettingsMaster->Unregister(DeinterlaceHolders[i]);
    }
    DeinterlaceHolders.clear();
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
    static DWORD MenuId = 6000;
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
            // check to see if the extension is .dll
            // FindFirst... searches long and short file names so a filter that is named
            // FLT_bogus.dllasdf will be found when searching for FLT_*.dll since it's short
            // file name is FLT_bo~1.dll

            if(_stricmp(".dll", &FindFileData.cFileName[strlen(FindFileData.cFileName)-4]) == 0)
            {
                try
                {
                    LoadDeintPlugin(FindFileData.cFileName);
                }
                catch(...)
                { 
                    LOG(1, "Crash Loading %s", FindFileData.cFileName);
                }
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

void GetDeinterlaceSettings(vector< SmartPtr<CSettingsHolder> >& Holders, vector< string >& Names)
{
    Holders = DeinterlaceHolders;
    Names.clear();
    for(int i(0); i < NumVideoModes; i++)
    {
        Names.push_back(VideoDeintMethods[i]->szName);
    }
}


////////////////////////////////////////////////////////////////////////////

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
