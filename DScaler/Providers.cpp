/////////////////////////////////////////////////////////////////////////////
// $Id: Providers.cpp,v 1.3 2001-11-09 12:42:07 adcockj Exp $
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
#include "Providers.h"
#include "BT848Provider.h"
#include "StillProvider.h"
#include "HardwareDriver.h"
#include "OutThreads.h"

typedef vector<CInterlacedSource*> SOURCELIST;

static SOURCELIST Sources;
static CHardwareDriver* HardwareDriver = NULL;
static CBT848Provider* BT848Provider = NULL;
static CStillProvider* StillProvider = NULL;
static CurrentSource = 0;

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
                sprintf(Text, "BT848 Card %d", i + 1);
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
            sprintf(Text, "Still %d", i + 1);
            AppendMenu(hSubMenu, MF_STRING | MF_ENABLED, IDM_SOURCE_FIRST + Sources.size(), Text);
        }
        Sources.push_back(StillProvider->GetSource(i));
    }

    Providers_UpdateMenu(hMenu);

    return Sources.size();
}

void Providers_Unload()
{
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

CInterlacedSource* Providers_GetCurrentSource()
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
    for(int i(0); i < Sources.size(); ++i)
    {
        CheckMenuItemBool(hMenu, IDM_SOURCE_FIRST + i, (CurrentSource == i));
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
        HMENU hSubMenu = Sources[CurrentSource]->GetMenu();
        GetMenuString(hSubMenu, 0, Text, 256, MF_BYPOSITION);
        // Add the new menu
        InsertMenu(hMenu, 6, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)GetSubMenu(hSubMenu, 0), Text);
    }
    else
    {
        return;
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
            CurrentSource = NewSource;
            Providers_UpdateMenu(GetMenu(hWnd));
            Start_Capture();
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

CInterlacedSource*  Providers_GetByIndex(long Index)
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

void Providers_WriteToIni()
{
    if(CurrentSource >= 0 && CurrentSource < Sources.size())
    {
        Sources[CurrentSource]->WriteToIni();
    }
}
