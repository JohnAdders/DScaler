/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file Providers.cpp Providers functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "Events.h"
#include "resource.h"
#include "Providers.h"
#include "StillProvider.h"
#include "BT848Provider.h"
#include "CX2388xProvider.h"
#include "SAA7134Provider.h"
#include "HardwareDriver.h"
#include "OutThreads.h"
#include "DScaler.h"
#include "Audio.h"
#include "VBI_WSSdecode.h"
#include "SettingsMaster.h"
#include "OpenDlg.h"
#include "MixerDev.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dshowsource\DSProvider.h"
static SmartPtr<CDSProvider> DSProvider;
#endif

using namespace std;

typedef struct
{
    tstring Name;
    SmartPtr<CSource> Object;
    BOOL DisplayInMenu;
} TSource;

typedef vector< SmartPtr<TSource> > SOURCELIST;

extern HMENU hMenu;

void Providers_NotifySourceChange(int OldSource);
void Providers_NotifySourcePreChange();

static SOURCELIST Sources;
static SmartPtr<CHardwareDriver> HardwareDriver;

#ifdef WANT_BT8X8_SUPPORT
static SmartPtr<CBT848Provider> BT848Provider;
#endif

#ifdef WANT_CX2388X_SUPPORT
static SmartPtr<CCX2388xProvider> CX2388xProvider;
#endif

#ifdef WANT_SAA713X_SUPPORT
static SmartPtr<CSAA7134Provider> SAA7134Provider;
#endif

static SmartPtr<CStillProvider> StillProvider;
static long CurrentSource = 0;
static long DefSourceIdx = -1;
long InitSourceIdx = -1;

extern TCHAR szIniFile[MAX_PATH];

void Providers_MixerSetup()
{
    MessageBox(GetMainWnd(),
        _T("The following dialog will allow you to configure how ")
        _T("DScaler uses the system mixer.  Configuring DScaler to use the ")
        _T("system mixer allows DScaler to change the volume level and the ")
        _T("mute state of your sound card by using the Windows mixer.  This is a ")
        _T("must for cards that do not support these functions in hardware. ")
        _T("You should specify the line into which your TV card's loopback ")
        _T("sound cable is attached.\n")
        _T("\n")
        _T("If you do not wish to let DScaler use the system mixer, leave ")
        _T("the \"Use the system mixer\" option unchecked.  If unsure, it ")
        _T("is recommended you use the system mixer."), _T("Next..."), MB_OK);

    Mixer_SetupDlg(GetMainWnd());
}


int Providers_Load(HMENU hMenu)
{
    int i(0);
    SmartPtr<TSource> Source;
    SmartPtr<CSource> DefaultSource;
    MENUITEMINFO    MenuItemInfo;
        BOOL AllowCx2388xDShow = FALSE;

    HMENU hSubMenu = GetSubMenu(hMenu, 0);

    Providers_Unload();
    HardwareDriver = new CHardwareDriver();
    if(HardwareDriver->LoadDriver() == TRUE)
    {
#ifdef WANT_BT8X8_SUPPORT
        BT848Provider = new CBT848Provider(HardwareDriver);
        for(i = 0; i < BT848Provider->GetNumberOfSources(); ++i)
        {
            Source = new TSource;
            if (BT848Provider->GetSource(i)->GetMenuLabel().empty())
            {
                Source->Name = _T("BT848 Card");
            }
            else
            {
                Source->Name = BT848Provider->GetSource(i)->GetMenuLabel();
            }
            Source->Object = BT848Provider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

            // Set CurrentSource for Providers_GetCurrentSource function
            // used in mixer code
            CurrentSource = Sources.size()-1;
            // The first time, setup the audio mixer for the card
            if (((CBT848Source*)(BT848Provider->GetSource(i).GetRawPointer()))->IsInitialSetup())
            {
                Providers_MixerSetup();
            }

            // Mute the audio of each source
            BT848Provider->GetSource(i)->Mute();
        }
#endif // WANT_BT8X8_SUPPORT

#ifdef WANT_CX2388X_SUPPORT
        CX2388xProvider = new CCX2388xProvider(HardwareDriver.GetRawPointer());

        // if we have any of these cards warn the user about the driver issues
        // hopefully this will be enought to get a new release out
        if(CX2388xProvider->GetNumberOfSources() > 0)
        {
            int RegFlag = GetPrivateProfileInt(_T("CX2388x"), _T("UseDShow"), -1, szIniFile);
            if(RegFlag == -1)
            {
                int Resp = MessageBox(GetMainWnd(), _T(" You have a CX2388x card.  There have been several reported instability ")
                    _T("problems with these cards if we allow the drivers to run.  Beacuse of this you can either run with ")
                    _T("DScaler's own drivers or with the DShow drivers but not both.  Do you want to use DScaler's own driver?"),
                    _T("CX2388x Question"), MB_YESNO | MB_ICONQUESTION);
                if(Resp == IDYES)
                {
                    AllowCx2388xDShow = FALSE;
                    WritePrivateProfileInt(_T("CX2388x"), _T("UseDShow"), 0, szIniFile);
                }
                else
                {
                    AllowCx2388xDShow = TRUE;
                    WritePrivateProfileInt(_T("CX2388x"), _T("UseDShow"), 1, szIniFile);
                }
            }
            else
            {
                AllowCx2388xDShow = (RegFlag == 1);
            }
        }

        for(i = 0; i < CX2388xProvider->GetNumberOfSources(); ++i)
        {
            Source = new TSource;
            if (CX2388xProvider->GetSource(i)->GetMenuLabel().empty())
            {
                Source->Name = _T("CX Card");
            }
            else
            {
                Source->Name = CX2388xProvider->GetSource(i)->GetMenuLabel();
            }
            Source->Object = CX2388xProvider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

            // Set CurrentSource for Providers_GetCurrentSource function
            // used in mixer code
            CurrentSource = Sources.size()-1;
            // The first time, setup the audio mixer for the card
            if (((CCX2388xSource*)(CX2388xProvider->GetSource(i).GetRawPointer()))->IsInitialSetup())
            {
                Providers_MixerSetup();
            }

            // Mute the audio of each source
            CX2388xProvider->GetSource(i)->Mute();
        }
#endif // WANT_CX2388X_SUPPORT

#ifdef WANT_SAA713X_SUPPORT
        SAA7134Provider = new CSAA7134Provider(HardwareDriver);
        for(i = 0; i < SAA7134Provider->GetNumberOfSources(); ++i)
        {
            Source = new TSource;
            if (SAA7134Provider->GetSource(i)->GetMenuLabel().empty())
            {
                Source->Name = _T("SAA7134 Card");
            }
            else
            {
                Source->Name = SAA7134Provider->GetSource(i)->GetMenuLabel();
            }
            Source->Object = SAA7134Provider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

            // Set CurrentSource for Providers_GetCurrentSource function
            // used in mixer code
            CurrentSource = Sources.size()-1;
            // The first time, setup the audio mixer for the card
            if (((CSAA7134Source*)(SAA7134Provider->GetSource(i).GetRawPointer()))->IsInitialSetup())
            {
                Providers_MixerSetup();
            }

            // Mute the audio of each source
            SAA7134Provider->GetSource(i)->Mute();
        }
#endif//xxx

        // Use by default the first BT8x8/CX2388x/SAA7134 source as initial source
        // Do that before loading other sources
        if (InitSourceIdx == -1)
        {
            InitSourceIdx = Providers_FindSource();
        }
    }
    else
    {
        ErrorBox(_T("Can't load Hardware Driver, possibly caused by corrupt installation.  Reboot and try again."));
        HardwareDriver = 0L;
    }

    StillProvider = new CStillProvider();
    for(i = 0; i < StillProvider->GetNumberOfSources(); ++i)
    {
        Source = new TSource;
        if (StillProvider->GetSource(i)->GetMenuLabel().empty())
        {
            Source->Name = _T("Still");
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
        if(AllowCx2388xDShow || DSProvider->GetSource(i)->IDString().find(_T("VEN_14F1&DEV_88")) == tstring::npos)
        {
            Source = new TSource;
            Source->Name = DSProvider->GetSourceName(i);
            Source->Name += _T(" (DShow)");
            Source->Object = DSProvider->GetSource(i);
            Source->DisplayInMenu = TRUE;
            Sources.push_back(Source);

            // Set CurrentSource for Providers_GetCurrentSource function
            // used in mixer code
            CurrentSource = Sources.size()-1;
            // The first time, setup the audio mixer for the card
            if (((CDSSourceBase*)(DSProvider->GetSource(i).GetRawPointer()))->IsInitialSetup())
            {
                Providers_MixerSetup();
            }

            DSProvider->GetSource(i)->Mute();
        }
    }
#endif

    DefaultSource = Providers_GetIntroSource();
    if (!DefaultSource || !DefaultSource->IsAccessAllowed())
    {
        ErrorBox(_T("Can't load file DScaler.d3u or file defined in it"));

        // We destroy the source if it exists
        DefSourceIdx = Providers_GetSourceIndex(DefaultSource);
        if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
        {
            Sources.erase(Sources.begin() + DefSourceIdx);
        }

        DefaultSource = 0L;
    }

    DefSourceIdx = Providers_GetSourceIndex(DefaultSource);

    // The default source is not listed in the menu
    if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
    {
        Source = Sources[DefSourceIdx];
        Source->DisplayInMenu = FALSE;
    }

    if (InitSourceIdx >= 0 && static_cast<size_t>(InitSourceIdx) < Sources.size() &&
        Sources[InitSourceIdx]->Object->IsAccessAllowed())
    {
        CurrentSource = InitSourceIdx;
    }
    else if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
    {
        CurrentSource = DefSourceIdx;
    }
    else
    {
        CurrentSource = Providers_FindSource();
    }

    Providers_NotifySourceChange(-1);

    for(size_t n = 0; n < Sources.size() && n < 100 ; ++n)
    {
        if (Sources[n]->DisplayInMenu)
        {
//            AppendMenu(hSubMenu, MF_STRING | MF_ENABLED, IDM_SOURCE_FIRST + n, Sources[n]->Name.c_str());
            MenuItemInfo.cbSize = sizeof (MenuItemInfo);
            MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
            MenuItemInfo.fType = MFT_STRING;
            MenuItemInfo.dwTypeData = (LPTSTR) Sources[n]->Name.c_str();
            MenuItemInfo.cch = _tcslen (Sources[n]->Name.c_str());
            MenuItemInfo.wID = IDM_SOURCE_FIRST + n;
            InsertMenuItem(hSubMenu, GetMenuItemCount(hSubMenu) - 4, TRUE, &MenuItemInfo);
        }
    }

    Providers_UpdateMenu(hMenu);

    return Sources.size();
}

void Providers_Unload()
{
    Sources.clear();

#ifdef WANT_DSHOW_SUPPORT
    DSProvider = 0L;
#endif
    StillProvider = 0L;
#ifdef WANT_BT8X8_SUPPORT
    BT848Provider = 0L;
#endif
#ifdef WANT_CX2388X_SUPPORT
    CX2388xProvider = 0L;
#endif
#ifdef WANT_SAA713X_SUPPORT
    SAA7134Provider = 0L;
#endif

    HardwareDriver = 0L;
}

CSource* Providers_GetCurrentSource()
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        return Sources[CurrentSource]->Object.GetRawPointer();
    }
    else
    {
        return NULL;
    }
}

long Providers_GetSourceIndex(SmartPtr<CSource>& Src)
{
    for (size_t i(0) ; i < Sources.size() ; i++)
    {
        if (Sources[i]->Object == Src)
        {
            return static_cast<long>(i);
        }
    }
    return -1;
}

SmartPtr<CSource> Providers_GetStillsSource()
{
    return StillProvider->GetSource(0);
}

CSource* Providers_GetSnapshotsSource()
{
    return StillProvider->GetSource(2).GetRawPointer();
}

SmartPtr<CSource> Providers_GetPatternsSource()
{
    return StillProvider->GetSource(1);
}

SmartPtr<CSource> Providers_GetIntroSource()
{
    return StillProvider->GetSource(3);
}

BOOL Providers_IsStillSource(CSource* source)
{
    if (StillProvider.IsValid())
    {
        for (int i=0 ; i<StillProvider->GetNumberOfSources() ; i++)
        {
            if (StillProvider->GetSource(i) == source)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

int Providers_FindSource()
{
    size_t SourceIdx = 0;
    while ((SourceIdx < Sources.size()) && !Sources[SourceIdx]->Object->IsAccessAllowed())
    {
        SourceIdx++;
    }
    if(SourceIdx >= 0 && SourceIdx < Sources.size())
    {
        return static_cast<int>(SourceIdx);
    }
    else
    {
        return -1;
    }
}

void Providers_SetMenu(HMENU hMenu)
{
    for(size_t i(0); i < Sources.size(); ++i)
    {
        if (Sources[i]->DisplayInMenu)
        {
            CheckMenuItemBool(hMenu, IDM_SOURCE_FIRST + i, (CurrentSource == i));
            EnableMenuItem(hMenu, IDM_SOURCE_FIRST + i, Sources[i]->Object->IsAccessAllowed() ? MF_ENABLED : MF_GRAYED);
        }
    }

    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->SetMenu(hMenu);
    }
}

void Providers_UpdateMenu(HMENU hMenu)
{
    // first we need to detach the old menu if there is one
    // if we don't do this our menu gets deleted
    RemoveMenu(hMenu, 1, MF_BYPOSITION);

    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        // get The name of our menu
        TCHAR Text[256];
        HMENU hSubMenu = Sources[CurrentSource]->Object->GetSourceMenu();
        GetMenuString(hSubMenu, 0, Text, 256, MF_BYPOSITION);
        // Add the new menu
        InsertMenu(hMenu, 1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)GetSubMenu(hSubMenu, 0), Text);

        // Update our menu
        Sources[CurrentSource]->Object->UpdateMenu();
    }
    else
    {
        // Add an empty new menu
        InsertMenu(hMenu, 1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)CreatePopupMenu(), _T("No source"));
    }

    RedrawMenuBar(hMenu);
}

BOOL Providers_HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    if(LOWORD(wParam) >= IDM_SOURCE_FIRST && LOWORD(wParam) <= IDM_SOURCE_LAST)
    {
        int NewSource(LOWORD(wParam) - IDM_SOURCE_FIRST);
        if(NewSource >= 0 && static_cast<size_t>(NewSource) < Sources.size())
        {
            Providers_NotifySourcePreChange();
            Stop_Capture();
            WSS_init();
            int OldSource = CurrentSource;
            CurrentSource = NewSource;
            Providers_NotifySourceChange(OldSource);
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
        COpenDlg OpenDlg;
        if(OpenDlg.DoModal(hWnd) == IDOK)
        {
            Stop_Capture();
            for(size_t i = 0; i < Sources.size(); ++i)
            {
                if(Sources[i]->Object->OpenMediaFile(OpenDlg.GetFileName().c_str(), FALSE))
                {
                    Providers_NotifySourcePreChange();

                    int OldSource = CurrentSource;
                    CurrentSource = static_cast<long>(i);
                    WSS_init();
                    Providers_UpdateMenu(hMenu);
                    Start_Capture();
                    Providers_NotifySourceChange(OldSource);
                    return TRUE;
                }
            }
            Start_Capture();
            MessageBox(hWnd, _T("Unsupported File Type"), _T("DScaler Warning"), MB_OK);
            return TRUE;
        }
    }
    else if (LOWORD(wParam) == IDM_SWITCH_SOURCE)
    {
        Providers_NotifySourcePreChange();

        Stop_Capture();
        WSS_init();
        int OldSource = CurrentSource;
        if (DefSourceIdx >= 0 && static_cast<size_t>(DefSourceIdx) < Sources.size())
        {
            CurrentSource = DefSourceIdx;
        }
        else
        {
            CurrentSource = Providers_FindSource();
        }
        Providers_UpdateMenu(hMenu);
        Start_Capture();

        Providers_NotifySourceChange(OldSource);
        return TRUE;
    }
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
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
    return Sources[Index]->Object.GetRawPointer();
}


void Provider_HandleTimerMessages(int TimerId)
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->HandleTimerMessages(TimerId);
    }
}

void Providers_ReadFromIni()
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->ReadFromIni();
    }
}

void Providers_WriteToIni(BOOL bOptimizeFileAccess)
{
    if(CurrentSource >= 0 && static_cast<size_t>(CurrentSource) < Sources.size())
    {
        Sources[CurrentSource]->Object->WriteToIni(bOptimizeFileAccess);
    }
}

void Providers_ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
    for (size_t i(0) ; i < Sources.size() ; i++)
    {
        Sources[i]->Object->ChangeSettingsBasedOnHW(ProcessorSpeed, TradeOff);
    }
}


void Providers_NotifySourcePreChange()
{
    if (EventCollector.IsValid())
    {
        EventCollector->RaiseEvent(NULL, EVENT_SOURCE_PRECHANGE, (long)Providers_GetCurrentSource(), (long)Providers_GetCurrentSource(), NULL);
    }
    if(Providers_GetCurrentSource() != NULL)
    {
        Providers_GetCurrentSource()->UnsetSourceAsCurrent();
    }
    // Laurent 30/05/2003 Not necessary because it is done two lines below
    // when calling UnsetSourceAsCurrent
    // good time to save the current settings
    // SettingsMaster->SaveGroupedSettings();
}

void Providers_NotifySourceChange(int OldSource)
{
    CSource* pOldSource = NULL;

    if(OldSource >= 0 && static_cast<size_t>(OldSource) < Sources.size())
    {
        pOldSource = Sources[OldSource]->Object.GetRawPointer();
    }
    if (EventCollector.IsValid())
    {
        EventCollector->RaiseEvent(NULL, EVENT_SOURCE_CHANGE, (long)pOldSource, (long)Providers_GetCurrentSource(), NULL);
    }

    if(Providers_GetCurrentSource() != NULL)
    {
        Providers_GetCurrentSource()->SetSourceAsCurrent();
    }
}
