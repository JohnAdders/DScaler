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
 * @file Filter.cpp Filter functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Filter.h"
#include "Settings.h"
#include "CPU.h"
#include "DScaler.h"
#include "OSD.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"
#include "crash.h"

long NumFilters = 0;

FILTER_METHOD* Filters[100] = {NULL,};

// Internal functions for per channel settings
void RegisterSettings(FILTER_METHOD* Filter);

CSettingsHolderStandAlone FilterSettingsHolder;

long Filter_DoInput(TDeinterlaceInfo* pInfo, int History, BOOL HurryUp)
{
    long SourceAspectAdjust = 1000;
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        if(Filters[i]->bActive && Filters[i]->bOnInput)
        {
            if(!HurryUp || Filters[i]->bAlwaysRun)
            {
                if(History >= Filters[i]->HistoryRequired)
                {
                    if(!(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK) || 
                        Filters[i]->CanDoInterlaced)
                    {
                        SourceAspectAdjust *= Filters[i]->pfnAlgorithm(pInfo);
                        CHECK_FPU_STACK
                        SourceAspectAdjust /= 1000;
                    }
                }
            }
        }
    }
    return SourceAspectAdjust;
}

void Filter_DoOutput(TDeinterlaceInfo* pInfo, int History, BOOL HurryUp)
{
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        if(Filters[i]->bActive && !Filters[i]->bOnInput)
        {
            if(!HurryUp || Filters[i]->bAlwaysRun)
            {
                if(History >= Filters[i]->HistoryRequired)
                {
                    Filters[i]->pfnAlgorithm(pInfo);
                    CHECK_FPU_STACK
                }
            }
        }
    }
}

BOOL Filter_WillWeDoOutput()
{
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        if(Filters[i]->bActive && !Filters[i]->bOnInput)
        {
            return TRUE;
        }
    }
    return FALSE;
}


void LoadFilterPlugin(LPCSTR szFileName)
{
    GETFILTERPLUGININFO* pfnGetFilterPluginInfo;
    FILTER_METHOD* pMethod;
    HMODULE hPlugInMod;

    hPlugInMod = LoadLibrary(szFileName);
    if(hPlugInMod == NULL)
    {
        LOG(1, " Loading %s failed.", szFileName);
        return;
    }
    
    pfnGetFilterPluginInfo = (GETFILTERPLUGININFO*)GetProcAddress(hPlugInMod, "GetFilterPluginInfo");
    if(pfnGetFilterPluginInfo == NULL)
    {
        return;
    }

    pMethod = pfnGetFilterPluginInfo(CpuFeatureFlags);
    if(pMethod != NULL)
    {
        if(pMethod->SizeOfStructure == sizeof(FILTER_METHOD) &&
            pMethod->FilterStructureVersion >= FILTER_VERSION_3 &&
            pMethod->InfoStructureVersion == DEINTERLACE_INFO_CURRENT_VERSION)
        {
            Filters[NumFilters] = pMethod;
            pMethod->hModule = hPlugInMod;

            // read in settings
            for(int i = 0; i < pMethod->nSettings; i++)
            {
                Setting_ReadFromIni(&(pMethod->pSettings[i]));
            }

            if(pMethod->pfnPluginStart != NULL)
            {
                pMethod->pfnPluginStart();
            }
            NumFilters++;
        }
        else
        {
            LOG(1, "Plugin %s obsolete", szFileName);
        }
    }
    else
    {
        LOG(1, "Plugin %s not compatible with your CPU", szFileName);
    }
}

void UnloadFilterPlugins()
{
    // get rid of the settings before we unload the filters
    FilterSettingsHolder.ClearSettingList(TRUE, TRUE);

    int i;
    for(i = 0; i < NumFilters; i++)
    {
        if(Filters[i]->pfnPluginExit!=NULL)
        {
            Filters[i]->pfnPluginExit();
        }
        FreeLibrary(Filters[i]->hModule);
        Filters[i] = NULL;
    }
    NumFilters = 0;
}

void AddUIForFilterPlugin(HMENU hFilterMenu, FILTER_METHOD* FilterMethod, int MenuId)
{
    if(FilterMethod->MenuId == 0)
    {
        FilterMethod->MenuId = MenuId;
    }
    if(FilterMethod->szMenuName != NULL)
    {
        AppendMenu(hFilterMenu, MF_STRING | MF_ENABLED, FilterMethod->MenuId, FilterMethod->szMenuName);
    }
    else
    {
        AppendMenu(hFilterMenu, MF_STRING | MF_ENABLED, FilterMethod->MenuId, FilterMethod->szName);
    }
}

// Filters are ordered based on the HistoryRequired field.
// This forces the filters that do not access any previous frames to be executed first.
// Some filters receive a forced order rating.

int GetFilterOrder(FILTER_METHOD* Filter_Method)
{
    if(strcmp(Filter_Method->szName, "Temporal Noise Filter") == 0)
    {
        return 35;
    }
    else if(strcmp(Filter_Method->szName, "Gradual Noise Filter") == 0)
    {
        return 35;
    }
    else if(strcmp(Filter_Method->szName, "Temporal Comb Filter") == 0)
    {
        return 45;
    }
    else if(strcmp(Filter_Method->szName, "Adaptive Noise Filter") == 0)
    {
        return 55;
    }
    else return Filter_Method->HistoryRequired*10;
}

void SortFilterPlugins()
{
    FILTER_METHOD* temp;

    if(NumFilters < 2)
    {
        return;
    }

    for(int j = 1; j < NumFilters; j++)
    {
        for(int i = 0; i < NumFilters - j; i++)
        {
            if(GetFilterOrder(Filters[i]) > GetFilterOrder(Filters[i+1]))
            {
                temp = Filters[i];
                Filters[i] = Filters[i+1];
                Filters[i+1] = temp;
            }
        }
    }

    LOG(2, "Filter order:");
    for(int i = 0; i < NumFilters; i++)
    {
        LOG(2, "  - %s", Filters[i]->szName);
    }
}

BOOL LoadFilterPlugins()
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    int i;

    hFindFile = FindFirstFile("FLT_*.dll", &FindFileData);

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
                __try
                {
                    LOG(1, "Loading %s ...", FindFileData.cFileName);
                    LoadFilterPlugin(FindFileData.cFileName);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) 
                { 
                    LOG(1, "Crash Loading %s", FindFileData.cFileName);
                }
            }
            RetVal = FindNextFile(hFindFile, &FindFileData);
        }
        FindClose(hFindFile);

        SortFilterPlugins();
    }

    if(NumFilters > 0)
    {
        HMENU hFilterMenu = GetFiltersSubmenu();
        if(hFilterMenu == NULL)
        {
            return FALSE;
        }

        AppendMenu(hFilterMenu, MF_STRING | MF_GRAYED, 0, "             Input Filters");
        for(i = 0; i < NumFilters; i++)
        {
            RegisterSettings(Filters[i]);

            if(Filters[i]->bOnInput)
            {
                AddUIForFilterPlugin(hFilterMenu, Filters[i], 7000 + i);
            }
        }
        AppendMenu(hFilterMenu, MF_SEPARATOR, 0, "");
        AppendMenu(hFilterMenu, MF_STRING | MF_GRAYED, 0, "             Output Filters");
        for(i = 0; i < NumFilters; i++)
        {
            if(!Filters[i]->bOnInput)
            {
                AddUIForFilterPlugin(hFilterMenu, Filters[i], 7000 + i);
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL ProcessFilterSelection(HWND hWnd, WORD wMenuID)
{
    int i;
    char szText[256];
    for(i = 0; i < NumFilters; i++)
    {
        if(wMenuID == Filters[i]->MenuId)
        {
            Filters[i]->bActive = !Filters[i]->bActive;
            if(Filters[i]->bActive)
            {
                sprintf(szText, "%s ON", Filters[i]->szName);
            }
            else
            {
                sprintf(szText, "%s OFF", Filters[i]->szName);
            }
            OSD_ShowText(szText, 0);
            return TRUE;
        }
    }
    return FALSE;
}

void GetFilterSettings(FILTER_METHOD**& MethodsArray,long& Num)
{
    MethodsArray = Filters;
    Num= NumFilters;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING* Filter_GetSetting(long nIndex, long Setting)
{
    if(nIndex < 0 || nIndex >= NumFilters)
    {
        return NULL;
    }
    if(Setting > -1 && Setting < Filters[nIndex]->nSettings)
    {
        return &(Filters[nIndex]->pSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

LONG Filter_HandleSettingsMsg(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
    int i;
    LONG RetVal = 0;
    SETTING* pSetting;
    for(i = 0; i < NumFilters; i++)
    {
        if(message == (UINT)(WM_APP + Filters[i]->nSettingsOffset))
        {
            *bDone = TRUE;
            pSetting = Filter_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                RetVal =  Setting_GetValue(pSetting);
            }
            break;
        }
        else if(message == (UINT)(WM_APP + Filters[i]->nSettingsOffset + 100))
        {
            *bDone = TRUE;
            pSetting = Filter_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                Setting_SetValue(pSetting, lParam);
            }
            break;
        }
        else if(message == (UINT)(WM_APP + Filters[i]->nSettingsOffset + 200))
        {
            *bDone = TRUE;
            pSetting = Filter_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                Setting_ChangeValue(pSetting, (eCHANGEVALUE)lParam);
            }
            break;
        }
    }
    return RetVal;
}

void Filter_ReadSettingsFromIni()
{
    FilterSettingsHolder.ReadFromIni();
}

void Filter_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    FilterSettingsHolder.WriteToIni(bOptimizeFileAccess);
}

void Filter_SetMenu(HMENU hMenu)
{
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        CheckMenuItemBool(hMenu, Filters[i]->MenuId, Filters[i]->bActive);
    }
}

void RegisterSettings(FILTER_METHOD* Filter)
{
    char szDescription[100];
        
    sprintf(szDescription,"Flt On Off - %s",Filter->szName);

    int iOnOffSetting = -1;

    CSettingGroup* pOnOffGroup = FilterSettingsHolder.GetSettingsGroup(szDescription, SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    sprintf(szDescription,"Flt Settings - %s",Filter->szName);

    CSettingGroup* pSettingsGroup = FilterSettingsHolder.GetSettingsGroup(szDescription, SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    int i;

    for (i = 0; i < Filter->nSettings; i++ )
    {        
        if (Filter->pSettings[i].pValue == (long*)&Filter->bActive)
        {            
            FilterSettingsHolder.AddSetting(&Filter->pSettings[i], pOnOffGroup);
            iOnOffSetting = i;
            break;
        }        
    }

    for (i = 0; i < Filter->nSettings; i++ )
    {        
        if (i != iOnOffSetting)
        {
            FilterSettingsHolder.AddSetting(&Filter->pSettings[i], pSettingsGroup);
        }
    }
}


