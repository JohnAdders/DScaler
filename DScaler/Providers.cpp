/////////////////////////////////////////////////////////////////////////////
// $Id: Providers.cpp,v 1.21 2002-02-09 14:46:04 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// Revision 1.20  2002/02/09 02:51:38  laurentg
// Grayed the channels when the source has no tuner
//
// Revision 1.19  2002/02/08 00:36:06  laurentg
// Support of a new type of file : DScaler patterns
//
// Revision 1.18  2002/02/02 12:41:44  laurentg
// CurrentX and CurrentY set when changing source and when switching between still files
//
// Revision 1.17  2002/02/02 01:31:18  laurentg
// Access to the files of the playlist added in the menus
// Save Playlist added
// "Video Adjustments ..." restored in the popup menu
//
// Revision 1.16  2002/02/01 00:41:58  laurentg
// Playlist code updated
//
// Revision 1.15  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.14  2001/12/09 22:00:42  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
// Revision 1.13  2001/12/08 20:00:24  laurentg
// Access control on sources
//
// Revision 1.12  2001/12/08 13:48:40  laurentg
// New StillSource for snapshots done during the DScaler session
//
// Revision 1.11  2001/12/08 12:01:26  laurentg
// Providers_AddSource and Providers_RemoveSource deleted
//
// Revision 1.10  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.9  2001/11/25 21:29:50  laurentg
// Take still, Open file, Close file callbacks updated
//
// Revision 1.8  2001/11/24 22:54:25  laurentg
// Close file added for still source
//
// Revision 1.7  2001/11/24 17:58:06  laurentg
// Still source
//
// Revision 1.6  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.5  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.4  2001/11/14 11:28:03  adcockj
// Bug fixes
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.6  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.5  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.3  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Providers.h"
#include "BT848Provider.h"
#include "StillProvider.h"
#include "HardwareDriver.h"
#include "OutThreads.h"
#include "DScaler.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dshowsource\DSProvider.h"
static CDSProvider* DSProvider = NULL;
#endif

typedef vector<CSource*> SOURCELIST;

static SOURCELIST Sources;
static CHardwareDriver* HardwareDriver = NULL;
static CBT848Provider* BT848Provider = NULL;
static CStillProvider* StillProvider = NULL;
static long CurrentSource = 0;

int Providers_Load(HMENU hMenu)
{
    int i(0);
    char Text[265];

    ModifyMenu(hMenu, 5, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)CreatePopupMenu(), "Sources");
    HMENU hSubMenu = GetSubMenu(hMenu, 5);

    Providers_Unload();

    HardwareDriver = new CHardwareDriver();
    if(HardwareDriver->LoadDriver() == TRUE)
    {
        BT848Provider = new CBT848Provider(HardwareDriver);
        for(i = 0; i < BT848Provider->GetNumberOfSources(); ++i)
        {
            if(Sources.size() < 100)
            {
                if (BT848Provider->GetSource(i)->GetMenuLabel() == NULL)
                {
                    sprintf(Text, "BT848 Card %d", i + 1);
                }
                else
                {
                    strcpy(Text, BT848Provider->GetSource(i)->GetMenuLabel());
                }
                AppendMenu(hSubMenu, MF_STRING | MF_ENABLED, IDM_SOURCE_FIRST + Sources.size(), Text);
            }
            Sources.push_back(BT848Provider->GetSource(i));
        }
    }
    else
    {
        ErrorBox("Can't load Hardware Driver, possibly caused by corrupt installation.  Reboot and try again.");
        delete HardwareDriver;
        HardwareDriver = NULL;
    }
    StillProvider = new CStillProvider();
    for(i = 0; i < StillProvider->GetNumberOfSources(); ++i)
    {
        if(Sources.size() < 100)
        {
            if (StillProvider->GetSource(i)->GetMenuLabel() == NULL)
            {
                sprintf(Text, "Still %d", i + 1);
            }
            else
            {
                strcpy(Text, StillProvider->GetSource(i)->GetMenuLabel());
            }
            AppendMenu(hSubMenu, MF_STRING | MF_ENABLED, IDM_SOURCE_FIRST + Sources.size(), Text);
        }
        Sources.push_back(StillProvider->GetSource(i));
    }

#ifdef WANT_DSHOW_SUPPORT
    DSProvider = new CDSProvider();
    for(i = 0; i < DSProvider->GetNumberOfSources(); ++i)
    {
        if(Sources.size() < 100)
        {
            AppendMenu(hSubMenu, MF_STRING | MF_ENABLED, IDM_SOURCE_FIRST + Sources.size(),DSProvider->getSourceName(i).c_str());
        }
        Sources.push_back(DSProvider->GetSource(i));
    }
#endif
    Providers_UpdateMenu(hMenu);

    return Sources.size();
}

void Providers_Unload()
{
#ifdef WANT_DSHOW_SUPPORT
	if(DSProvider!=NULL)
	{
		delete DSProvider;
		DSProvider=NULL;
	}
#endif
    if(StillProvider != NULL)
    {
        delete StillProvider;
        StillProvider = NULL;
    }
    if(BT848Provider != NULL)
    {
        delete BT848Provider;
        BT848Provider = NULL;
    }
    if(HardwareDriver != NULL)
    {
        HardwareDriver->UnloadDriver();
        delete HardwareDriver;
        HardwareDriver = NULL;
    }
    Sources.clear();
}

CSource* Providers_GetCurrentSource()
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        return Sources[CurrentSource];
    }
    else
    {
        return NULL;
    }
}

void Providers_SetMenu(HMENU hMenu)
{
    if(Providers_GetCurrentSource()->HasTuner())
    {
        EnableMenuItem(hMenu, IDM_CHANNELPLUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_CHANNELMINUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_CHANNEL_PREVIOUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_CHANNEL_LIST, MF_ENABLED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_CHANNELPLUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_CHANNELMINUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_CHANNEL_PREVIOUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_CHANNEL_LIST, MF_GRAYED);
    }
    
    for(int i(0); i < Sources.size(); ++i)
    {
        CheckMenuItemBool(hMenu, IDM_SOURCE_FIRST + i, (CurrentSource == i));
        EnableMenuItem(hMenu, IDM_SOURCE_FIRST + i, Sources[i]->IsAccessAllowed() ? MF_ENABLED : MF_GRAYED);
    }

    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->SetMenu(hMenu);
    }
}

void Providers_UpdateMenu(HMENU hMenu)
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        // first we need to detach the old menu if there is one
        // if we don't do this our menu gets deleted
        RemoveMenu(hMenu, 6, MF_BYPOSITION);

        // get The name of our menu
        char Text[256];
        HMENU hSubMenu = Sources[CurrentSource]->GetSourceMenu();
        GetMenuString(hSubMenu, 0, Text, 256, MF_BYPOSITION);
        // Add the new menu
        InsertMenu(hMenu, 6, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)GetSubMenu(hSubMenu, 0), Text);

        // Update our menu
        Sources[CurrentSource]->UpdateMenu();
    }
    else
    {
        return;
    }
}

BOOL Providers_HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    CSource*    pCurrentSource = Providers_GetCurrentSource();
    HMENU       hMenu = GetMenu(hWnd);

    if(LOWORD(wParam) >= IDM_SOURCE_FIRST && LOWORD(wParam) <= IDM_SOURCE_LAST)
    {
        int NewSource(LOWORD(wParam) - IDM_SOURCE_FIRST);
        if(NewSource >= 0 && NewSource < Sources.size())
        {
            Stop_Capture();
            CurrentSource = NewSource;
            Providers_UpdateMenu(hMenu);
            CurrentX = Sources[CurrentSource]->GetWidth();
            CurrentY = Sources[CurrentSource]->GetHeight();
            Start_Capture();
            return TRUE;
        }
    }
    else if (LOWORD(wParam) == IDM_OPEN_FILE)
    {
        OPENFILENAME OpenFileInfo;
        char FilePath[MAX_PATH];
        
        char* FileFilters = "All Supported Files\0*.d3u;*.pat;*.tif;*.tiff\0"
                           "TIFF Files\0*.tif;*.tiff\0"
                           "DScaler Playlists\0*.d3u\0"
                           "DScaler Patterns\0*.pat\0";

        OpenFileInfo.lStructSize = sizeof(OpenFileInfo);
        OpenFileInfo.hwndOwner = hWnd;
        OpenFileInfo.lpstrFilter = FileFilters;
        OpenFileInfo.lpstrCustomFilter = NULL;
        FilePath[0] = 0;
        OpenFileInfo.lpstrFile = FilePath;
        OpenFileInfo.nMaxFile = sizeof(FilePath);
        OpenFileInfo.lpstrFileTitle = NULL;
        OpenFileInfo.lpstrInitialDir = NULL;
        OpenFileInfo.lpstrTitle = NULL;
        OpenFileInfo.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        OpenFileInfo.lpstrDefExt = NULL;
        if (GetOpenFileName(&OpenFileInfo))
        {
            Stop_Capture();
            for(int i = 0; i < Sources.size(); ++i)
            {
                if(Sources[i]->OpenMediaFile(FilePath, FALSE))
                {
                    CurrentSource = i;
                    Providers_UpdateMenu(hMenu);
                    Start_Capture();
                    return TRUE;
                }
            }
            Start_Capture();
            MessageBox(hWnd, "Unsupported File Type", "DScaler Warning", MB_OK);
            return TRUE;
        }
    }
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        return Sources[CurrentSource]->HandleWindowsCommands(hWnd, wParam, lParam);
    }
    return FALSE;
}

long Providers_GetNumber()
{
    return Sources.size();
}

CSource*  Providers_GetByIndex(long Index)
{
    return Sources[Index];
}


void Provider_HandleTimerMessages(int TimerId)
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->HandleTimerMessages(TimerId);
    }
}

void Providers_ReadFromIni()
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->ReadFromIni();
    }
}

void Providers_WriteToIni(BOOL bOptimizeFileAccess)
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->WriteToIni(bOptimizeFileAccess);
    }
}
