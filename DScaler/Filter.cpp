/////////////////////////////////////////////////////////////////////////////
// $Id: Filter.cpp,v 1.23 2002-08-08 12:39:13 kooiman Exp $
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
// Revision 1.22  2002/06/18 19:46:06  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.21  2002/06/13 12:10:22  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.20  2002/06/07 16:36:27  robmuller
// Added filter sorting. Added missing FindClose() statement.
//
// Revision 1.19  2002/04/24 19:10:38  tobbej
// test of new tree based setting dialog
//
// Revision 1.18  2001/11/26 15:27:18  adcockj
// Changed filter structure
//
// Revision 1.17  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.16  2001/11/22 22:32:09  adcockj
// Removed extra filer settings menu item
//
// Revision 1.15  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.14  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.13  2001/09/09 18:30:19  adcockj
// Filter menu fix
//
// Revision 1.12  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.11  2001/08/25 10:48:02  laurentg
// Log messages added
//
// Revision 1.10  2001/08/23 16:03:26  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.9  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
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
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Filter.h"
#include "Settings.h"
#include "CPU.h"
#include "DScaler.h"
#include "OSD.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"

long NumFilters = 0;

FILTER_METHOD* Filters[100] = {NULL,};

// Internal functions for per channel settings
void Filter_ChannelSubSection(FILTER_METHOD *Filter, char *szSubSection);
void Filter_RegisterChannelSettings(FILTER_METHOD *Filter, const char *szSubSection);
void Filter_SaveByChannelSetup(void *pThis, int Start);


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
                    if(!((pInfo->PictureHistory[0]->Flags | PICTURE_INTERLACED_MASK) > 0) || 
                        Filters[i]->CanDoInterlaced)
                    {
                        SourceAspectAdjust *= Filters[i]->pfnAlgorithm(pInfo);
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
                }
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

// This is a first attempt to order the filters. Sorting is done on the HistoryRequired field.
// Since filters with HistoryRequired == 1 can't be disturbed by the other filters we 
// put them in front of the list.
// This is by no means ideal but it does solve many of the current filter order problems.

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
            if(Filters[i]->HistoryRequired > Filters[i+1]->HistoryRequired)
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
            __try
            {
                LOG(1, "Loading %s ...", FindFileData.cFileName);
                LoadFilterPlugin(FindFileData.cFileName);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) 
            { 
                LOG(1, "Crash Loading %s", FindFileData.cFileName);
            }
            RetVal = FindNextFile(hFindFile, &FindFileData);
        }
        FindClose(hFindFile);

        SortFilterPlugins();
    }

    SettingsPerChannel_RegisterOnSetup(NULL,Filter_SaveByChannelSetup);

    if(NumFilters > 0)
    {
        HMENU hFilterMenu = GetFiltersSubmenu();
        if(hFilterMenu == NULL)
        {
            return FALSE;
        }

        for(i = 0; i < NumFilters; i++)
        {
            AddUIForFilterPlugin(hFilterMenu, Filters[i], 7000 + i);
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


void Filter_SaveByChannelSetup(void *pThis, int Start)

{
    if (Start)
    {
        int i;
        char szSubSection[100];                
        for(i = 0; i < NumFilters; i++)
        {            
            Filter_ChannelSubSection(Filters[i], szSubSection);
            SettingsPerChannel_UnregisterSection(szSubSection);
            Filter_RegisterChannelSettings(Filters[i], szSubSection);            
        }     
    }   
}

void Filter_RegisterChannelSettings(FILTER_METHOD *Filter, const char *szSubSection)
{
    // register settings for this channel    

    if (Filter == NULL)
    {
        return;
    }

    char szDescription[100];
        
    sprintf(szDescription,"Flt - %s",Filter->szName);
    SettingsPerChannel_RegisterSetSection(szSubSection);

    int iOnOffSetting = -1;

    int i;
    for (i = 0; i < Filter->nSettings; i++ )
    {        
        if (Filter->pSettings[i].pValue == (long*)&Filter->bActive)
        {            
            SettingsPerChannel_RegisterSetting(szSubSection,szDescription,FALSE, &Filter->pSettings[i]); 
            iOnOffSetting = i;
            break;
        }        
    }
    if (iOnOffSetting < 0) 
    {
        SettingsPerChannel_RegisterSetting(szSubSection,szDescription,FALSE, (long**)NULL);         
    }
    for (i = 0; i < Filter->nSettings; i++ )
    {        
        if (i != iOnOffSetting)
        {
            SettingsPerChannel_RegisterSetting(szSubSection,szDescription,FALSE, &Filter->pSettings[i]); 
        }
    }
}


void Filter_ChannelSubSection(FILTER_METHOD *Filter, char *szSubSection)
{
    if ( (Filter->nSettings>0) 
          && (Filter->pSettings[0].szIniSection!=NULL) 
          && (Filter->pSettings[0].szIniSection[0]!=0) )
    {
        strcpy(szSubSection,Filter->pSettings[0].szIniSection);
    }
    else
    {
        strcpy(szSubSection,Filter->szName);
    }
}