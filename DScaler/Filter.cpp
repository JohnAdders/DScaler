/////////////////////////////////////////////////////////////////////////////
// $Id: Filter.cpp,v 1.9 2001-08-02 16:43:05 adcockj Exp $
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
// 03 Feb 2001   John Adcock           Initial Version
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.7  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.6  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Filter.h"
#include "Settings.h"
#include "CPU.h"
#include "DScaler.h"
#include "OSD.h"
#include "DebugLog.h"
#include "SettingsDlg.h"

long NumFilters = 0;

FILTER_METHOD* Filters[100] = {NULL,};

long Filter_DoInput(DEINTERLACE_INFO* pInfo, BOOL HurryUp)
{
    long SourceAspectAdjust = 1000;
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        if(Filters[i]->bActive && Filters[i]->bOnInput)
        {
            if(!HurryUp || Filters[i]->bAlwaysRun)
            {
                SourceAspectAdjust *= Filters[i]->pfnAlgorithm(pInfo);
                SourceAspectAdjust /= 1000;
            }
        }
    }
    return SourceAspectAdjust;
}

void Filter_DoOutput(DEINTERLACE_INFO* pInfo, BOOL HurryUp)
{
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        if(Filters[i]->bActive && !Filters[i]->bOnInput)
        {
            if(!HurryUp || Filters[i]->bAlwaysRun)
            {
                Filters[i]->pfnAlgorithm(pInfo);
            }
        }
    }
}

void LoadFilterPlugin(LPCSTR szFileName)
{
    GETFILTERPLUGININFO* pfnGetFilterPluginInfo;
    FILTER_METHOD* pMethod;
    HMODULE hPlugInMod;

    hPlugInMod = LoadLibrary(szFileName);
    if(hPlugInMod == NULL)
    {
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
            pMethod->FilterStructureVersion >= FILTER_VERSION_1)
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
    }
}

void UnloadFilterPlugins()
{
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

void AddUIForFilterPlugin(HMENU hMenu, FILTER_METHOD* FilterMethod)
{
    static MenuId = 7000;
    if(FilterMethod->MenuId == 0)
    {
        FilterMethod->MenuId = MenuId++;
    }
    if(FilterMethod->szMenuName != NULL)
    {
        AppendMenu(hMenu, MF_STRING | MF_ENABLED, FilterMethod->MenuId, FilterMethod->szMenuName);
    }
    else
    {
        AppendMenu(hMenu, MF_STRING | MF_ENABLED, FilterMethod->MenuId, FilterMethod->szName);
    }
}

BOOL LoadFilterPlugins()
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    int i;
    HMENU hMenu;

    hFindFile = FindFirstFile("FLT_*.dll", &FindFileData);

    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        BOOL RetVal = TRUE;
        while(RetVal != 0)
        {
            __try
            {
                LoadFilterPlugin(FindFileData.cFileName);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) 
            { 
                LOG(1, " Crash Loading %s", FindFileData.cFileName);
            }
            RetVal = FindNextFile(hFindFile, &FindFileData);
        }
    }

    if(NumFilters > 0)
    {
        hMenu = GetFiltersSubmenu();
        if(hMenu == NULL) return FALSE;

        for(i = 0; i < NumFilters; i++)
        {
            AddUIForFilterPlugin(hMenu, Filters[i]);
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
            OSD_ShowText(hWnd, szText, 0);
            return TRUE;
        }
    }
    return FALSE;
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
        if(message == (UINT)(WM_USER + Filters[i]->nSettingsOffset))
        {
            *bDone = TRUE;
            pSetting = Filter_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                RetVal =  Setting_GetValue(pSetting);
            }
            break;
        }
        else if(message == (UINT)(WM_USER + Filters[i]->nSettingsOffset + 100))
        {
            *bDone = TRUE;
            pSetting = Filter_GetSetting(i, wParam);
            if(pSetting != NULL)
            {
                Setting_SetValue(pSetting, lParam);
            }
            break;
        }
        else if(message == (UINT)(WM_USER + Filters[i]->nSettingsOffset + 200))
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
    int i,j;
    for(i = 0; i < NumFilters; i++)
    {
        for(j = 0; j < Filters[i]->nSettings; j++)
        {
            Setting_ReadFromIni(&(Filters[i]->pSettings[j]));
        }
    }
}


void Filter_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i,j;
    for(i = 0; i < NumFilters; i++)
    {
        for(j = 0; j < Filters[i]->nSettings; j++)
        {
            Setting_WriteToIni(&(Filters[i]->pSettings[j]), bOptimizeFileAccess);
        }
    }
}

void Filter_SetMenu(HMENU hMenu)
{
    int i;
    for(i = 0; i < NumFilters; i++)
    {
        CheckMenuItemBool(hMenu, Filters[i]->MenuId, Filters[i]->bActive);
    }
}

void Filter_ShowUI(LPCSTR SearchText)
{
    for(int i(0); i < NumFilters; i++)
    {
        if(strstr(Filters[i]->szName, SearchText) != NULL)
        {
            CSettingsDlg::ShowSettingsDlg(Filters[i]->szName, Filters[i]->pSettings, Filters[i]->nSettings);
        }
    }
}