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
#include "DynamicFunction.h"


using namespace std;

long NumFilters = 0;

FILTER_METHOD* Filters[100] = {NULL,};

// Internal functions for per channel settings
void RegisterSettings(SmartPtr<CSettingsHolder> Holder, FILTER_METHOD* Filter);

vector< SmartPtr<CSettingsHolder> > FilterHolders;

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
                        CHECK_FPU_STACK;
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
                    CHECK_FPU_STACK;
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


void LoadFilterPlugin(LPCTSTR szFileName)
{
    DynamicFunctionC1<FILTER_METHOD*, long> GetFilterPluginInfo(szFileName, "GetFilterPluginInfo");
    FILTER_METHOD* pMethod;

    if(!GetFilterPluginInfo)
    {
        return;
    }

    pMethod = GetFilterPluginInfo(CpuFeatureFlags);
    if(pMethod != NULL)
    {
        if(pMethod->SizeOfStructure == sizeof(FILTER_METHOD) &&
            pMethod->FilterStructureVersion >= FILTER_VERSION_3 &&
            pMethod->InfoStructureVersion == DEINTERLACE_INFO_CURRENT_VERSION)
        {
            Filters[NumFilters] = pMethod;
            SmartPtr<CSettingsHolder> Holder(new CSettingsHolder(WM_APP + pMethod->nSettingsOffset, pMethod->HelpID));

            RegisterSettings(Holder, pMethod);
            Holder->ReadFromIni();
            SettingsMaster->Register(Holder);

            if(pMethod->pfnPluginStart != NULL)
            {
                pMethod->pfnPluginStart();
            }
            NumFilters++;
            FilterHolders.push_back(Holder);
        }
        else
        {
            LOG(1, _T("Plugin %s obsolete"), szFileName);
        }
    }
    else
    {
        LOG(1, _T("Plugin %s not compatible with your CPU"), szFileName);
    }
}

void UnloadFilterPlugins()
{
    // get rid of the settings before we unload the filters
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        if(Filters[i]->pfnPluginExit!=NULL)
        {
            Filters[i]->pfnPluginExit();
        }
        SettingsMaster->Unregister(FilterHolders[i]);
        Filters[i] = NULL;
    }
    FilterHolders.clear();
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
        AppendMenuA(hFilterMenu, MF_STRING | MF_ENABLED, FilterMethod->MenuId, FilterMethod->szMenuName);
    }
    else
    {
        AppendMenuA(hFilterMenu, MF_STRING | MF_ENABLED, FilterMethod->MenuId, FilterMethod->szName);
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
                std::swap(Filters[i], Filters[i+1]);
                std::swap(FilterHolders[i], FilterHolders[i+1]);
            }
        }
    }

    LOG(2, _T("Filter order:"));
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

    hFindFile = FindFirstFile(_T("FLT_*.dll"), &FindFileData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        BOOL RetVal = TRUE;
        while(RetVal != 0)
        {
            // check to see if the extension is .dll
            // FindFirst... searches long and short file names so a filter that is named
            // FLT_bogus.dllasdf will be found when searching for FLT_*.dll since it's short
            // file name is FLT_bo~1.dll

            if(_tcsicmp(_T(".dll"), &FindFileData.cFileName[_tcslen(FindFileData.cFileName)-4]) == 0)
            {
                try
                {
                    LOG(1, _T("Loading %s ..."), FindFileData.cFileName);
                    LoadFilterPlugin(FindFileData.cFileName);
                }
                catch(...)
                {
                    LOG(1, _T("Crash Loading %s"), FindFileData.cFileName);
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

        AppendMenu(hFilterMenu, MF_STRING | MF_GRAYED, 0, _T("             Input Filters"));
        for(i = 0; i < NumFilters; i++)
        {
            if(Filters[i]->bOnInput)
            {
                AddUIForFilterPlugin(hFilterMenu, Filters[i], 7000 + i);
            }
        }
        AppendMenu(hFilterMenu, MF_SEPARATOR, 0, _T(""));
        AppendMenu(hFilterMenu, MF_STRING | MF_GRAYED, 0, _T("             Output Filters"));
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
    for(i = 0; i < NumFilters; i++)
    {
        if(wMenuID == Filters[i]->MenuId)
        {
            tstring Text(MBCSToTString(Filters[i]->szName));
            Filters[i]->bActive = !Filters[i]->bActive;
            if(Filters[i]->bActive)
            {
                Text += _T(" ON");
            }
            else
            {
                Text += _T(" OFF");
            }
            OSD_ShowText(Text, 0);
            return TRUE;
        }
    }
    return FALSE;
}

void GetFilterSettings(vector< SmartPtr<CSettingsHolder> >& Holders, vector< tstring >& Names)
{
    Holders = FilterHolders;
    Names.clear();
    for(int i(0); i < NumFilters; i++)
    {
        Names.push_back(MBCSToTString(Filters[i]->szName));
    }
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

void Filter_SetMenu(HMENU hMenu)
{
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        CheckMenuItemBool(hMenu, Filters[i]->MenuId, Filters[i]->bActive);
    }
}

void RegisterSettings(SmartPtr<CSettingsHolder> Holder, FILTER_METHOD* Filter)
{
    tstring szDescription(_T("Flt On Off - ") + MBCSToTString(Filter->szName));

    int iOnOffSetting = -1;

    CSettingGroup* pOnOffGroup = SettingsMaster->GetGroup(szDescription.c_str(), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    szDescription = _T("Flt Settings - ") + MBCSToTString(Filter->szName);

    CSettingGroup* pSettingsGroup = SettingsMaster->GetGroup(szDescription.c_str(), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    int i;

    for (i = 0; i < Filter->nSettings; i++ )
    {
        if (Filter->pSettings[i].pValue == (LONG_PTR*)&Filter->bActive)
        {
            Holder->AddSetting(Filter->pSettings + i, pOnOffGroup);
            iOnOffSetting = i;
            break;
        }
    }

    for (i = 0; i < Filter->nSettings; i++ )
    {
        if (i != iOnOffSetting)
        {
            Holder->AddSetting(Filter->pSettings + i);
        }
    }
}
