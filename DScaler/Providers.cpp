/////////////////////////////////////////////////////////////////////////////
// $Id: Providers.cpp,v 1.37 2002-05-01 13:00:18 laurentg Exp $
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
// Revision 1.36  2002/04/28 16:46:49  laurentg
// Reinit WSS data when switching source
//
// Revision 1.35  2002/04/27 16:02:59  laurentg
// Initial source
//
// Revision 1.34  2002/04/27 11:21:04  tobbej
// fixed crashing, dont use c style malloc to allocate memory for structs with c++ members
//
// Revision 1.33  2002/04/27 00:38:33  laurentg
// New default source (still) used at DScaler startup or when there is no more source accessible
//
// Revision 1.32  2002/04/13 21:52:40  laurentg
// Management of no current source
//
// Revision 1.31  2002/04/13 18:56:23  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.30  2002/04/07 14:55:04  tobbej
// added asf and wmv filetypes to file-open dialog
//
// Revision 1.29  2002/04/06 11:46:45  laurentg
// Check that the current source is not NULL to avoid DScaler exits
//
// Revision 1.28  2002/03/02 18:33:56  laurentg
// At startup, mute the audio of all unused cards
//
// Revision 1.27  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.26  2002/02/18 23:25:01  laurentg
// At startup, go to the first source having a content
// Order of still sources changed (Patterns before Snapshots)
//
// Revision 1.25  2002/02/17 00:35:35  laurentg
// Problem to modify source submenu when menubar is hidden is solved
//
// Revision 1.24  2002/02/12 16:33:40  tobbej
// updated file-open menu with filetype for media files (avi for example)
//
// Revision 1.23  2002/02/11 21:33:13  laurentg
// Patterns as a new source from the Still provider
//
// Revision 1.22  2002/02/09 21:12:28  laurentg
// Old test patterns restored
// Loading of d3u files improved (absolute or relative path)
//
// Revision 1.21  2002/02/09 14:46:04  laurentg
// OSD main screen updated to display the correct input name (or channel)
// OSD main screen updated to display only activated filters
// Menu label for the BT848 providers now displays the name of the card
//
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
#include "Audio.h"
#include "VBI_WSSdecode.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dshowsource\DSProvider.h"
static CDSProvider* DSProvider = NULL;
#endif

typedef struct {
    std::string Name;
    CSource*    Object;
    BOOL        DisplayInMenu;
} TSource;

typedef vector<TSource*> SOURCELIST;

extern HMENU hMenu;

static SOURCELIST Sources;
static CHardwareDriver* HardwareDriver = NULL;
static CBT848Provider* BT848Provider = NULL;
static CStillProvider* StillProvider = NULL;
static long CurrentSource = 0;
static long DefSourceIdx = -1;
long InitSourceIdx = -1;

int Providers_Load(HMENU hMenu)
{
    int i(0);
    TSource* Source;
    CSource* DefaultSource = NULL;

//    ModifyMenu(hMenu, 5, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)CreatePopupMenu(), "&Sources");
    HMENU hSubMenu = GetSubMenu(hMenu, 5);

    Providers_Unload();
    HardwareDriver = new CHardwareDriver();
    if(HardwareDriver->LoadDriver() == TRUE)
    {
        BT848Provider = new CBT848Provider(HardwareDriver);
        for(i = 0; i < BT848Provider->GetNumberOfSources(); ++i)
        {
            Source = new TSource;
            if (BT848Provider->GetSource(i)->GetMenuLabel() == NULL)
            {
                Source->Name = "BT848 Card";
            }
            else
            {
                Source->Name = BT848Provider->GetSource(i)->GetMenuLabel();
            }
            Source->Object = BT848Provider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);
            // Mute the audio of this source
            CurrentSource = i;
            Audio_Mute();
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
        Source = new TSource;
        if (StillProvider->GetSource(i)->GetMenuLabel() == NULL)
        {
            Source->Name = "Still";
        }
        else
        {
            Source->Name = StillProvider->GetSource(i)->GetMenuLabel();
        }
        Source->Object = StillProvider->GetSource(i);
        Source->DisplayInMenu = TRUE;
        Sources.push_back(Source);
    }

#ifdef WANT_DSHOW_SUPPORT
    DSProvider = new CDSProvider();
    for(i = 0; i < DSProvider->GetNumberOfSources(); ++i)
    {
        Source = new TSource;
        Source->Name = DSProvider->getSourceName(i);
        Source->Object = DSProvider->GetSource(i);
        Source->DisplayInMenu = TRUE;
        Sources.push_back(Source);
    }
#endif

    DefaultSource = Providers_GetIntroSource();
    if (!DefaultSource || !DefaultSource->IsAccessAllowed())
    {
        ErrorBox("Can't load file DScaler.d3u");

        // We destroy the source if it exists
        DefSourceIdx = Providers_GetSourceIndex(DefaultSource);
        if (DefSourceIdx >= 0 && DefSourceIdx < Sources.size())
        {
            Source = *(Sources.begin() + DefSourceIdx);
            Sources.erase(Sources.begin() + DefSourceIdx);
            delete Source;
        }

        DefaultSource = NULL;
    }

    DefSourceIdx = Providers_GetSourceIndex(DefaultSource);

    // The default source is not listed in the menu
    if (DefSourceIdx >= 0 && DefSourceIdx < Sources.size())
    {
        Source = *(Sources.begin() + DefSourceIdx);
        Source->DisplayInMenu = FALSE;
    }

    if (InitSourceIdx >= 0 && InitSourceIdx < Sources.size() && Sources[InitSourceIdx]->Object->IsAccessAllowed())
    {
        CurrentSource = InitSourceIdx;
    }
    else if (DefSourceIdx >= 0 && DefSourceIdx < Sources.size())
    {
        CurrentSource = DefSourceIdx;
    }
    else
    {
        CurrentSource = Providers_FindSource();
    }

    for(i = 0; i < Sources.size() && i < 100 ; ++i)
    {
        if (Sources[i]->DisplayInMenu)
        {
            AppendMenu(hSubMenu, MF_STRING | MF_ENABLED, IDM_SOURCE_FIRST + i, Sources[i]->Name.c_str());
        }
    }

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
    for(vector<TSource*>::iterator it = Sources.begin(); 
        it != Sources.end(); 
        ++it)
    {
        delete *it;
    }
    Sources.clear();
}

CSource* Providers_GetCurrentSource()
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        return Sources[CurrentSource]->Object;
    }
    else
    {
        return NULL;
    }
}

long Providers_GetSourceIndex(CSource* Src)
{
    for (int i(0) ; i < Sources.size() ; i++)
    {
        if (Sources[i]->Object == Src)
        {
            return i;
        }
    }
    return -1;
}

CSource* Providers_GetStillsSource()
{
    return StillProvider->GetSource(0);
}

CSource* Providers_GetSnapshotsSource()
{
    return StillProvider->GetSource(2);
}

CSource* Providers_GetPatternsSource()
{
    return StillProvider->GetSource(1);
}

CSource* Providers_GetIntroSource()
{
    return StillProvider->GetSource(3);
}

int Providers_FindSource()
{
    int SourceIdx = 0;
    while ((SourceIdx < Sources.size()) && !Sources[SourceIdx]->Object->IsAccessAllowed())
    {
        SourceIdx++;
    }
    if(SourceIdx >= 0 && SourceIdx < Sources.size())
    {
        return SourceIdx;
    }
    else
    {
        return -1;
    }
}

void Providers_SetMenu(HMENU hMenu)
{
    for(int i(0); i < Sources.size(); ++i)
    {
        if (Sources[i]->DisplayInMenu)
        {
            CheckMenuItemBool(hMenu, IDM_SOURCE_FIRST + i, (CurrentSource == i));
            EnableMenuItem(hMenu, IDM_SOURCE_FIRST + i, Sources[i]->Object->IsAccessAllowed() ? MF_ENABLED : MF_GRAYED);
        }
    }

    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->Object->SetMenu(hMenu);
    }
}

void Providers_UpdateMenu(HMENU hMenu)
{
    // first we need to detach the old menu if there is one
    // if we don't do this our menu gets deleted
    RemoveMenu(hMenu, 6, MF_BYPOSITION);

    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        // get The name of our menu
        char Text[256];
        HMENU hSubMenu = Sources[CurrentSource]->Object->GetSourceMenu();
        GetMenuString(hSubMenu, 0, Text, 256, MF_BYPOSITION);
        // Add the new menu
        InsertMenu(hMenu, 6, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)GetSubMenu(hSubMenu, 0), Text);

        // Update our menu
        Sources[CurrentSource]->Object->UpdateMenu();
    }
    else
    {
        // Add an empty new menu
        InsertMenu(hMenu, 6, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)CreatePopupMenu(), "No source");
    }
}

BOOL Providers_HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    if(LOWORD(wParam) >= IDM_SOURCE_FIRST && LOWORD(wParam) <= IDM_SOURCE_LAST)
    {
        int NewSource(LOWORD(wParam) - IDM_SOURCE_FIRST);
        if(NewSource >= 0 && NewSource < Sources.size())
        {
            Stop_Capture();
            WSS_init();
            CurrentSource = NewSource;
            Providers_UpdateMenu(hMenu);
            Start_Capture();
            return TRUE;
        }
    }
    else if (LOWORD(wParam) == IDM_SOURCE_INITIAL)
    {
        InitSourceIdx = CurrentSource;
        return TRUE;
    }
    else if (LOWORD(wParam) == IDM_OPEN_FILE)
    {
        OPENFILENAME OpenFileInfo;
        char FilePath[MAX_PATH];
        char* FileFilters;
        FileFilters =
#ifndef WANT_DSHOW_SUPPORT
                        "All Supported Files\0*.d3u;*.pat;*.tif;*.tiff;*.jpg;*.jpeg\0"
#else
                        "All Supported Files\0*.d3u;*.pat;*.tif;*.tiff;*.jpg;*.jpeg;*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv\0"
#endif
                        "TIFF Files\0*.tif;*.tiff\0"
                        "JPEG Files\0*.jpg;*.jpeg\0"
                        "DScaler Playlists\0*.d3u\0"
                        "DScaler Patterns\0*.pat\0"
#ifdef WANT_DSHOW_SUPPORT
                        "Media Files (*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv)\0*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv\0"
#endif
                        ;

        ZeroMemory(&OpenFileInfo,sizeof(OpenFileInfo));
        OpenFileInfo.lStructSize = sizeof(OpenFileInfo);
        OpenFileInfo.hwndOwner = hWnd;
        OpenFileInfo.lpstrFilter = FileFilters;
        OpenFileInfo.nFilterIndex = 1;
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
                if(Sources[i]->Object->OpenMediaFile(FilePath, FALSE))
                {
                    CurrentSource = i;
                    WSS_init();
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
    else if (LOWORD(wParam) == IDM_SWITCH_SOURCE)
    {
        Stop_Capture();
        WSS_init();
        if (DefSourceIdx >= 0 && DefSourceIdx < Sources.size())
        {
            CurrentSource = DefSourceIdx;
        }
        else
        {
            CurrentSource = Providers_FindSource();
        }
        Providers_UpdateMenu(hMenu);
        Start_Capture();
        return TRUE;
    }
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        return Sources[CurrentSource]->Object->HandleWindowsCommands(hWnd, wParam, lParam);
    }
    return FALSE;
}

long Providers_GetNumber()
{
    return Sources.size();
}

CSource*  Providers_GetByIndex(long Index)
{
    return Sources[Index]->Object;
}


void Provider_HandleTimerMessages(int TimerId)
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->Object->HandleTimerMessages(TimerId);
    }
}

void Providers_ReadFromIni()
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->Object->ReadFromIni();
    }
}

void Providers_WriteToIni(BOOL bOptimizeFileAccess)
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->Object->WriteToIni(bOptimizeFileAccess);
    }
}
