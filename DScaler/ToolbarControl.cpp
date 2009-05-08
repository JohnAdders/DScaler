//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Jeroen Kooiman.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file ToolbarControl.cpp CToolbarControl Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "ToolbarWindow.h"
#include "Toolbars.h"
#include "ToolbarControl.h"

#include "Status.h"
#include "DScaler.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "Providers.h"

using namespace std;

/**
    Toolbar control creates & destroys the toolbar with the subbars inside.



*/

CToolbarControl::CToolbarControl(long SetMessage) : CSettingsHolder(SetMessage),
Toolbar1(NULL),
Toolbar1Channels(NULL),
Toolbar1Volume(NULL),
Toolbar1MediaPlayer(NULL),
Toolbar1Logo(NULL)
{
    CreateSettings(_T("Toolbars"));

    eEventType EventList[] = {EVENT_SOURCE_CHANGE, EVENT_VIDEOINPUT_CHANGE, EVENT_CHANNELLIST_CHANGE, EVENT_ENDOFLIST};
    EventCollector->Register(this, EventList);


    BOOL bShowChannels = FALSE;
    BOOL bShowVolume = FALSE;
    BOOL bShowMediaPlayer = FALSE;

    //Is there a tuner?
    if (Providers_GetCurrentSource() != NULL)
    {
        int VideoInput = Providers_GetCurrentSource()->GetInput(VIDEOINPUT);
        bShowChannels = Providers_GetCurrentSource()->InputHasTuner(VIDEOINPUT,VideoInput);
        bShowVolume = !Providers_IsStillSource(Providers_GetCurrentSource());
        bShowMediaPlayer = Providers_GetCurrentSource()->HasMediaControl();
    }

    //Set proper bit
    int Visible = HIWORD(m_Toolbar1Channels->GetValue());
    Visible = (Visible&1) | (bShowChannels?2:0);
    m_Toolbar1Channels->SetValue( MAKELONG(LOWORD(m_Toolbar1Channels->GetValue()), Visible) );
    Visible = HIWORD(m_Toolbar1Volume->GetValue());
    Visible = (Visible&1) | (bShowVolume?2:0);
    m_Toolbar1Volume->SetValue( MAKELONG(LOWORD(m_Toolbar1Volume->GetValue()), Visible) );
    Visible = HIWORD(m_Toolbar1MediaPlayer->GetValue());
    Visible = (Visible&1) | (bShowMediaPlayer?2:0);
    m_Toolbar1MediaPlayer->SetValue( MAKELONG(LOWORD(m_Toolbar1MediaPlayer->GetValue()), Visible) );

    MarginsDefault.l = 5;
    MarginsDefault.t = 5;
    MarginsDefault.r = 5;
    MarginsDefault.b = 5;
    MarginsDefault.child_lr = 0;
    MarginsDefault.child_tb = 5;
}


CToolbarControl::~CToolbarControl()
{
    Free();
}


void CToolbarControl::CreateSettings(LPCTSTR IniSection)
{
    m_ShowToolbar1 = new CShowToolbar1Setting(this, _T("ShowToolbar1"), FALSE, IniSection);
    m_Settings.push_back(m_ShowToolbar1);

    m_Toolbar1Position = new CToolbar1PositionSetting(this, _T("Toolbar1Position"), 1, 0, 1, IniSection);
    m_Settings.push_back(m_Toolbar1Position);

    m_Toolbar1Volume = new CToolbar1VolumeSetting(this, _T("VolumeBar"), MAKELONG(2,1), 0x80000000L, 0x7F000000L, IniSection);
    m_Settings.push_back(m_Toolbar1Volume);

    m_Toolbar1Channels = new CToolbar1ChannelsSetting(this, _T("ChannelBar"), MAKELONG(1,1), 0x80000000L, 0x7F000000L, IniSection);
    m_Settings.push_back(m_Toolbar1Channels);

    m_Toolbar1MediaPlayer = new CToolbar1MediaPlayerSetting(this, _T("MediaPlayerBar"), MAKELONG(3,1), 0x80000000L, 0x7F000000L, IniSection);
    m_Settings.push_back(m_Toolbar1MediaPlayer);

    ReadFromIni();
}

void CToolbarControl::ShowToolbar1OnChange(BOOL OldValue, BOOL NewValue)
{
}

void CToolbarControl::Toolbar1PositionOnChange(long OldValue, long NewValue)
{
}

void CToolbarControl::Toolbar1ChannelsOnChange(long OldValue, long NewValue)
{
}

void CToolbarControl::Toolbar1VolumeOnChange(long OldValue, long NewValue)
{
}

void CToolbarControl::Toolbar1MediaPlayerOnChange(long OldValue, long NewValue)
{
}


void CToolbarControl::Set(HWND hWnd, const tstring& SkinName, int ForceHide, int ForceResize)
{
    if (Toolbar1==NULL) //Initialize
    {
        if (!m_ShowToolbar1->GetValue())
        {
            //Don't make the toolbar if it's not necessary yet
            return;
        }

        //Create main toolbar window
        Toolbar1 = new CToolbarWindow(hWnd, hDScalerInst, 1);
        Toolbar1->DefaultColor(GetSysColor(COLOR_BTNFACE));
        Toolbar1->Margins(MarginsDefault.l,MarginsDefault.t,MarginsDefault.r,MarginsDefault.b,MarginsDefault.child_lr,MarginsDefault.child_tb);

        //Set default margin
        MarginsTop = MarginsDefault;
        MarginsBottom = MarginsDefault;

        //Create separator bar (maybe drag bar in future)
        int n = CreateToolbar1Bar();

        //Create channel subbar from a dialog
        Toolbar1Channels = new CToolbarChannels(Toolbar1);
        if (Toolbar1Channels->CreateFromDialog(MAKEINTRESOURCE(IDD_TOOLBAR_CHANNELS), hResourceInst)!=NULL)
        {
            //Add subbar to the main toolbar
            Toolbar1->Add(Toolbar1Channels,TOOLBARCHILD_ALIGN_LEFTCENTER,LOWORD(m_Toolbar1Channels->GetValue()),0);
            if (n >= 0)
            {
                Toolbar1->AttachBar(Toolbar1Channels, 0, Toolbar1Bars[n]);
            }
        }
        else
        {
            delete Toolbar1Channels;
            Toolbar1Channels = NULL;
        }

        if (Toolbar1Channels != NULL)
        {
            //Another bar
            n = CreateToolbar1Bar();
        }

        //Create volume subbar from a dialog
        Toolbar1Volume = new CToolbarVolume(Toolbar1);
        if (Toolbar1Volume->CreateFromDialog(MAKEINTRESOURCE(IDD_TOOLBAR_VOLUME), hResourceInst)!=NULL)
        {
            Toolbar1->Add(Toolbar1Volume,TOOLBARCHILD_ALIGN_LEFTCENTER,LOWORD(m_Toolbar1Volume->GetValue()),0);
            if (n >= 0)
            {
                Toolbar1->AttachBar(Toolbar1Volume, 0, Toolbar1Bars[n]);
            }
        }
        else
        {
            delete Toolbar1Volume;
            Toolbar1Volume = NULL;
        }

        if (Toolbar1Channels != NULL)
        {
            //Another bar
            n = CreateToolbar1Bar();
        }

        //Create media player subbar from a dialog
        Toolbar1MediaPlayer = new CToolbarMediaPlayer(Toolbar1);
        if (Toolbar1MediaPlayer->CreateFromDialog(MAKEINTRESOURCE(IDD_TOOLBAR_MEDIAPLAYER), hResourceInst)!=NULL)
        {
            Toolbar1->Add(Toolbar1MediaPlayer,TOOLBARCHILD_ALIGN_LEFTCENTER,LOWORD(m_Toolbar1MediaPlayer->GetValue()),0);
            if (n >= 0)
            {
                Toolbar1->AttachBar(Toolbar1MediaPlayer, 0, Toolbar1Bars[n]);
            }
        }
        else
        {
            delete Toolbar1MediaPlayer;
            Toolbar1MediaPlayer = NULL;
        }

        //Create logo subbar from a dialog
        Toolbar1Logo = new CToolbarLogo(Toolbar1);
        if (Toolbar1Logo->CreateFromDialog(MAKEINTRESOURCE(IDD_TOOLBAR_LOGO), hResourceInst)!=NULL)
        {
            Toolbar1->Add(Toolbar1Logo,TOOLBARCHILD_ALIGN_RIGHTCENTER, 99 ,0);
        }
        else
        {
            delete Toolbar1Logo;
            Toolbar1Logo = NULL;
        }
    }

    if (SkinName.empty()) //Remove skin
    {
        //Remove skin from main toolbar window
        Toolbar1->ClearSkin();
        //Restore default margins
        Toolbar1->Margins(MarginsDefault.l,MarginsDefault.t,MarginsDefault.r,MarginsDefault.b,MarginsDefault.child_lr,MarginsDefault.child_tb);

        //Remove skin from separator bars
        for (int i = 0; i < Toolbar1Bars.size(); i++)
        {
            if (Toolbar1Bars[i] != NULL)
            {
                TCHAR szID[40];
                _stprintf(szID,_T("Toolbar1Bar%d"),i);
                Toolbar1Bars[i]->RemoveSkin(szID);
                InvalidateRect(Toolbar1Bars[i]->GethWnd(), NULL, FALSE);
            }
        }

        //Remove skins from the controls in the subbar
        if (Toolbar1Channels!=NULL)
        {
            Toolbar1Channels->RemoveSkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINUP);
            Toolbar1Channels->RemoveSkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINDOWN);
            Toolbar1Channels->RemoveSkinDlgItem(IDC_TOOLBAR_CHANNELS_PREVIOUS);
            Toolbar1Channels->Reset();
            InvalidateRect(Toolbar1Channels->GethWnd(), NULL, FALSE);
        }

        if (Toolbar1Volume != NULL)
        {
            Toolbar1Volume->RemoveSkinDlgItem(IDC_TOOLBAR_VOLUME_MUTE);
            Toolbar1Volume->RemoveSkinDlgItem(IDC_TOOLBAR_VOLUME_SLIDER);
            Toolbar1Volume->RemoveSkinDlgItem(IDC_TOOLBAR_VOLUME_CHANNEL);
            Toolbar1Volume->Reset();
            InvalidateRect(Toolbar1Volume->GethWnd(), NULL, FALSE);
        }

        if (Toolbar1MediaPlayer != NULL)
        {
            Toolbar1MediaPlayer->RemoveSkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_PLAY);
            Toolbar1MediaPlayer->RemoveSkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_PAUSE);
            Toolbar1MediaPlayer->RemoveSkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_STOP);
            Toolbar1MediaPlayer->RemoveSkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_TIMESLIDER);
            Toolbar1MediaPlayer->Reset();
            InvalidateRect(Toolbar1MediaPlayer->GethWnd(), NULL, FALSE);
        }

        if (Toolbar1Logo != NULL)
        {
            Toolbar1Logo->RemoveSkinDlgItem(IDC_TOOLBAR_LOGO_LOGO);
            Toolbar1Logo->Reset();
            InvalidateRect(Toolbar1Logo->GethWnd(), NULL, FALSE);
        }
        InvalidateRect(Toolbar1->GethWnd(), NULL, FALSE);
    }

    if (!SkinName.empty()) //Load skin
    {
        TCHAR szSkinIniFile[MAX_PATH*2];
        _tcscpy(szSkinIniFile,GetSkinDirectory());
        _tcscat(szSkinIniFile,SkinName.c_str());
        _tcscat(szSkinIniFile,_T("\\skin.ini"));
        ///\todo check if the ini file exists

        //Set default margin
        MarginsTop = MarginsDefault;
        MarginsBottom = MarginsDefault;

        //Read margins from ini file
        TCHAR szBuffer[256];
        TCHAR szDefault[] = {0};
        for (int Pos = 0; Pos <= 1; Pos++)
        {
            GetPrivateProfileString(_T("Toolbar1"),(Pos==0)?_T("MarginsTop"):_T("MarginsBottom"), szDefault, szBuffer, 255, szSkinIniFile);
            if (szBuffer[0] != 0)
            {
                TToolbar1Margins Margins;
                Margins = MarginsDefault;

                if (_stscanf(szBuffer,_T("%d,%d,%d,%d,%d,%d"),&Margins.l,&Margins.t,&Margins.r,&Margins.b,&Margins.child_lr,&Margins.child_tb)>=4)
                {
                    if (Pos ==0)
                    {
                        MarginsTop = Margins;
                    }
                    else
                    {
                        MarginsBottom = Margins;
                    }
                }
            }
        }

        if (m_Toolbar1Position->GetValue()==0)
        {
            Toolbar1->Margins(MarginsTop.l,MarginsTop.t,MarginsTop.r,MarginsTop.b,MarginsTop.child_lr,MarginsTop.child_tb);
        }
        else
        {
            Toolbar1->Margins(MarginsBottom.l,MarginsBottom.t,MarginsBottom.r,MarginsBottom.b,MarginsBottom.child_lr,MarginsBottom.child_tb);
        }

        //Load border skin for main toolbar
        vector<int>Results;
        Toolbar1->LoadSkin(szSkinIniFile,_T("Toolbar1"),&Results);
        ///\todo Process errors

        //Load skin for toolbar separator window
        for (int i = 0; i < Toolbar1Bars.size(); i++)
        {
            if (Toolbar1Bars[i] != NULL)
            {
                TCHAR szID[40];
                _stprintf(szID,_T("Toolbar1Bar%d"),i);
                Toolbar1Bars[i]->SkinWindow(Toolbar1Bars[i]->GethWndPicture(), szID, _T("Bar"), BITMAPASBUTTON_PUSH, _T("Toolbar1"), szSkinIniFile);
                InvalidateRect(Toolbar1Bars[i]->GethWnd(), NULL, FALSE);
            }
        }

        //Load skins for controls in subbar
        if (Toolbar1Channels != NULL)
        {
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINUP, _T("SpinUp"), BITMAPASBUTTON_PUSH, _T("ChannelBar"), szSkinIniFile);
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINDOWN, _T("SpinDown"), BITMAPASBUTTON_PUSH, _T("ChannelBar"), szSkinIniFile);
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_PREVIOUS, _T("Previous"), BITMAPASBUTTON_PUSH, _T("ChannelBar"), szSkinIniFile);
            Toolbar1Channels->Reset();
            InvalidateRect(Toolbar1Channels->GethWnd(), NULL, FALSE);
        }

        if (Toolbar1Volume != NULL)
        {
            Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_MUTE, _T("Mute"), BITMAPASBUTTON_CHECKBOX, _T("VolumeBar"), szSkinIniFile);
            Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_SLIDER, _T("Volume"), BITMAPASBUTTON_SLIDER, _T("VolumeBar"), szSkinIniFile);
            Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_CHANNEL, _T("SoundChannel"), BITMAPASBUTTON_4STATE, _T("VolumeBar"), szSkinIniFile);
            Toolbar1Volume->Reset();
            InvalidateRect(Toolbar1Volume->GethWnd(), NULL, FALSE);
        }

        if (Toolbar1MediaPlayer != NULL)
        {
            Toolbar1MediaPlayer->SkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_PLAY, _T("Play"), BITMAPASBUTTON_PUSH, _T("MediaPlayerBar"), szSkinIniFile);
            Toolbar1MediaPlayer->SkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_PAUSE, _T("Pause"), BITMAPASBUTTON_PUSH, _T("MediaPlayerBar"), szSkinIniFile);
            Toolbar1MediaPlayer->SkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_STOP, _T("Stop"), BITMAPASBUTTON_PUSH, _T("MediaPlayerBar"), szSkinIniFile);
            Toolbar1MediaPlayer->SkinDlgItem(IDC_TOOLBAR_MEDIAPLAYER_TIMESLIDER, _T("ElapsedTime"), BITMAPASBUTTON_SLIDER, _T("MediaPlayerBar"), szSkinIniFile);
            Toolbar1MediaPlayer->Reset();
            InvalidateRect(Toolbar1MediaPlayer->GethWnd(), NULL, FALSE);
        }

        if (Toolbar1Logo != NULL)
        {
            Toolbar1Logo->SkinDlgItem(IDC_TOOLBAR_LOGO_LOGO, _T("Logo"), BITMAPASBUTTON_PUSH, _T("LogoBar"), szSkinIniFile);
            Toolbar1Logo->Reset();
            InvalidateRect(Toolbar1Logo->GethWnd(), NULL, FALSE);
        }
        InvalidateRect(Toolbar1->GethWnd(), NULL, FALSE);
    }

    if (Toolbar1!=NULL) //Update visibility
    {
        //Set position & visibility of subbars
        if (Toolbar1Channels!=NULL)
        {
            Toolbar1->SetChildPosition(Toolbar1Channels, LOWORD(m_Toolbar1Channels->GetValue()), 0);
            if ((HIWORD(m_Toolbar1Channels->GetValue())&3)==3)
            {
                Toolbar1->ShowChild(Toolbar1Channels);
            }
            else
            {
                Toolbar1->HideChild(Toolbar1Channels);
            }
        }
        if (Toolbar1Volume!=NULL)
        {
            Toolbar1->SetChildPosition(Toolbar1Volume, LOWORD(m_Toolbar1Volume->GetValue()), 0);
            if ((HIWORD(m_Toolbar1Volume->GetValue())&3)==3)
            {
                Toolbar1->ShowChild(Toolbar1Volume);
            }
            else
            {
                Toolbar1->HideChild(Toolbar1Volume);
            }
        }
        if (Toolbar1MediaPlayer!=NULL)
        {
            Toolbar1->SetChildPosition(Toolbar1MediaPlayer, LOWORD(m_Toolbar1MediaPlayer->GetValue()), 0);
            // Temporary : hide the toolbar when a skin is enabled
            // TODO : integrate this new toolbar in the skins
            extern SettingStringValue szSkinName;
            if ( ((HIWORD(m_Toolbar1MediaPlayer->GetValue())&3)==3)
              && ( !szSkinName ) )
            {
                Toolbar1->ShowChild(Toolbar1MediaPlayer);
            }
            else
            {
                Toolbar1->HideChild(Toolbar1MediaPlayer);
            }
        }
        if (Toolbar1Logo!=NULL)
        {
            if (1)
            {
                Toolbar1->ShowChild(Toolbar1Logo);
            }
            else
            {
                Toolbar1->HideChild(Toolbar1Logo);
            }
        }
        if (ForceResize)
        {
            Toolbar1->ForceUpdateWindowPosition(NULL);
        }
        //Set position & visibility of main toolbar
        if ((m_ShowToolbar1->GetValue() && (ForceHide==0)) || (ForceHide==2))
        {
            if ((m_Toolbar1Position->GetValue()==0) && (Toolbar1->GetPosition() != 0))
            {
                Toolbar1->SetPosition(0);
            }
            if ((m_Toolbar1Position->GetValue()==1) && (Toolbar1->GetPosition() != 1))
            {
                Toolbar1->SetPosition(1);
            }
            Toolbar1->Show();
        }
        else
        {
            Toolbar1->Hide();
            if (!m_ShowToolbar1->GetValue())
            {
                Free();
            }
        }
    }

    //if (ToolbarSideBar==NULL)
    //{
        //ToolbarSideBar = new CToolbarWindow(hWnd, hDScalerInst, 0);
        //ToolbarSideBar->LoadBorders(szIniFile,_T("SideBarChannels"),NULL);
        //ToolbarSideBar->SetPosition(-2);
    //}

    //Restore focus to main DScaler window
    if (Toolbar1!=NULL)
    {
        SetFocus(Toolbar1->GethWndParent());
    }

}

//Create a separator bar window
int CToolbarControl::CreateToolbar1Bar()
{
    CToolbar1Bar *Toolbar1Bar = new CToolbar1Bar(Toolbar1);

    if (Toolbar1Bar->Create(_T("Toolbar1Bar"), hResourceInst) != NULL)
    {
        // Ok
        Toolbar1Bar->Margins(10,10);
    }
    else
    {
        delete Toolbar1Bar;
        Toolbar1Bar = NULL;
    }

    if (Toolbar1Bar != NULL)
    {
        int n = Toolbar1Bars.size();
        Toolbar1Bars.push_back(Toolbar1Bar);
        return n;
    }
    return -1;
}

//Update toolbar position
void CToolbarControl::Adjust(HWND hWnd, BOOL bRedraw, BOOL ForceUpdate)
{
    if ((Toolbar1!=NULL) && Toolbar1->Visible())
    {
        if (ForceUpdate)
        {
            Toolbar1->ForceUpdateWindowPosition(hWnd);
        }
        else
        {
            Toolbar1->UpdateWindowPosition(hWnd);
        }
        if (bRedraw)
        {
            InvalidateRect(Toolbar1->GethWnd(), NULL, FALSE);
        }
    }
}

//Listen to source/input changes to show/hide the channel bar
void CToolbarControl::OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
    if ((Event == EVENT_SOURCE_CHANGE) ||
        (Event == EVENT_VIDEOINPUT_CHANGE) && (pEventObject == (CEventObject*)Providers_GetCurrentSource()))
    {
        if (Providers_GetCurrentSource() != NULL)
        {
            int VideoInput = Providers_GetCurrentSource()->GetInput(VIDEOINPUT);
            BOOL bShowChannels = Providers_GetCurrentSource()->InputHasTuner(VIDEOINPUT,VideoInput);
            BOOL bShowVolume = !Providers_IsStillSource(Providers_GetCurrentSource());
            BOOL bShowMediaPlayer = Providers_GetCurrentSource()->HasMediaControl();

            //Set proper bit
            int Visible = HIWORD(m_Toolbar1Channels->GetValue());
            Visible = (Visible&1) | (bShowChannels?2:0);
            m_Toolbar1Channels->SetValue( MAKELONG(LOWORD(m_Toolbar1Channels->GetValue()), Visible) );
            Visible = HIWORD(m_Toolbar1Volume->GetValue());
            Visible = (Visible&1) | (bShowVolume?2:0);
            m_Toolbar1Volume->SetValue( MAKELONG(LOWORD(m_Toolbar1Volume->GetValue()), Visible) );
            Visible = HIWORD(m_Toolbar1MediaPlayer->GetValue());
            Visible = (Visible&1) | (bShowMediaPlayer?2:0);
            m_Toolbar1MediaPlayer->SetValue( MAKELONG(LOWORD(m_Toolbar1MediaPlayer->GetValue()), Visible) );

            if ((Toolbar1 != NULL) && Toolbar1->Visible())
            {
                CToolbarControl::Set(GetMainWnd(), NULL);
            }
        }
    }
    else if (Event == EVENT_CHANNELLIST_CHANGE)
    {
        if (Toolbar1Channels != NULL)
        {
            Toolbar1Channels->Reset();
        }
    }
}

//Is Point in toolbar?
// (Used for cursor hide in main DScaler program)
BOOL CToolbarControl::PtInToolbar(POINT Pt)
{
    if ((Toolbar1!=NULL) && Toolbar1->Visible())
    {
        RECT Rc;
        GetWindowRect(Toolbar1->GethWnd(), &Rc);
        if (PtInRect(&Rc, Pt))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CToolbarControl::AutomaticDisplay(POINT Pt)
{
    if ((Toolbar1 != NULL) && m_ShowToolbar1->GetValue())
    {
        RECT Rc;
        GetWindowRect(Toolbar1->GethWnd(), &Rc);
        BOOL IsInBar = PtInRect(&Rc, Pt);
        if (IsInBar && !Toolbar1->Visible())
        {
            // Show the toolbar
            Set(GetMainWnd(), NULL, 0);
            return TRUE;
        }
        else if (!IsInBar && Toolbar1->Visible())
        {
            // Hide the toolbar
            Set(GetMainWnd(), NULL, 1);
            return TRUE;
        }
    }
    return FALSE;
}

//Delete all toolbars & childs
void CToolbarControl::Free()
{
    if (Toolbar1 != NULL)
    {
        Toolbar1->Remove(Toolbar1Channels);
        Toolbar1->Remove(Toolbar1Volume);
        Toolbar1->Remove(Toolbar1MediaPlayer);
        Toolbar1->Remove(Toolbar1Logo);
    }
    /*if (ToolbarSideBar != NULL)
    {
        delete ToolbarSideBar;
        ToolbarSideBar = NULL;
    }   */
    if (Toolbar1Channels != NULL)
    {
        delete Toolbar1Channels;
        Toolbar1Channels = NULL;
    }
    if (Toolbar1Volume != NULL)
    {
        delete Toolbar1Volume;
        Toolbar1Volume = NULL;
    }
    if (Toolbar1MediaPlayer != NULL)
    {
        delete Toolbar1MediaPlayer;
        Toolbar1MediaPlayer = NULL;
    }
    if (Toolbar1Logo != NULL)
    {
        delete Toolbar1Logo;
        Toolbar1Logo = NULL;
    }
    for (int i = 0; i < Toolbar1Bars.size(); i++)
    {
        if (Toolbar1Bars[i] != NULL)
        {
            delete Toolbar1Bars[i];
        }
    }
    Toolbar1Bars.clear();

    if (Toolbar1 != NULL)
    {
        delete Toolbar1;
        Toolbar1 = NULL;
    }
}

//Subtract toolbar from area
void CToolbarControl::AdjustArea(LPRECT lpRect, int Crop)
{
    if ((Toolbar1!=NULL) && Toolbar1->Visible())
    {
        Toolbar1->AdjustArea(lpRect,Crop);
    }
}

void CToolbarControl::UpdateMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_VIEW_TOOLBARS_MAINTOOLBAR, (m_ShowToolbar1->GetValue()!=0));
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_TOP, (m_Toolbar1Position->GetValue()==0));
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_CHANNELS, ((HIWORD(m_Toolbar1Channels->GetValue())&1)!=0));
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_VOLUME, ((HIWORD(m_Toolbar1Volume->GetValue())&1)!=0));
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_MEDIAPLAYER, ((HIWORD(m_Toolbar1MediaPlayer->GetValue())&1)!=0));

    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_TOP, (m_ShowToolbar1->GetValue()!=0));
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_CHANNELS, (m_ShowToolbar1->GetValue()!=0));
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_VOLUME, (m_ShowToolbar1->GetValue()!=0));
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_MEDIAPLAYER, (m_ShowToolbar1->GetValue()!=0));
}

//Process menu commands
BOOL CToolbarControl::ProcessToolbar1Selection(HWND hWnd, UINT uItem)
{
    extern BOOL bIsFullScreen;
    switch (uItem)
    {
    case IDM_VIEW_TOOLBARS_MAINTOOLBAR:
        m_ShowToolbar1->SetValue(!m_ShowToolbar1->GetValue());
        if ((Toolbar1 == NULL) && (m_ShowToolbar1->GetValue()))
        {
            extern SettingStringValue szSkinName;
            CToolbarControl::Set(hWnd, (LPCTSTR)szSkinName);
        }
        else
        {
            CToolbarControl::Set(hWnd, NULL);
        }
        WorkoutOverlaySize(TRUE);
        CToolbarControl::Set(hWnd, NULL, bIsFullScreen?1:0);
        break;
    case IDM_VIEW_MAINTOOLBAR_TOP:
        if (m_Toolbar1Position->GetValue() == 1)
        {
            m_Toolbar1Position->SetValue(0);
        }
        else
        {
            m_Toolbar1Position->SetValue(1);
        }
        if (m_Toolbar1Position->GetValue()==0)
        {
            Toolbar1->Margins(MarginsTop.l,MarginsTop.t,MarginsTop.r,MarginsTop.b,MarginsTop.child_lr,MarginsTop.child_tb);
        }
        else
        {
            Toolbar1->Margins(MarginsBottom.l,MarginsBottom.t,MarginsBottom.r,MarginsBottom.b,MarginsBottom.child_lr,MarginsBottom.child_tb);
        }
        CToolbarControl::Set(hWnd, NULL);
        WorkoutOverlaySize(TRUE);
        CToolbarControl::Set(hWnd, NULL, bIsFullScreen?1:0);
        break;
    case IDM_VIEW_MAINTOOLBAR_CHANNELS:
        {
            int Visible = HIWORD(m_Toolbar1Channels->GetValue());
            Visible = (Visible&2) | ((Visible&1)?0:1);
            m_Toolbar1Channels->SetValue( MAKELONG(LOWORD(m_Toolbar1Channels->GetValue()), Visible) );
            CToolbarControl::Set(hWnd, NULL);
            WorkoutOverlaySize(TRUE);
            CToolbarControl::Set(hWnd, NULL, bIsFullScreen?1:0);
        }
        break;
    case IDM_VIEW_MAINTOOLBAR_VOLUME:
        {
            int Visible = HIWORD(m_Toolbar1Volume->GetValue());
            Visible = (Visible&2) | ((Visible&1)?0:1);
            m_Toolbar1Volume->SetValue( MAKELONG(LOWORD(m_Toolbar1Volume->GetValue()), Visible) );
            CToolbarControl::Set(hWnd, NULL);
            WorkoutOverlaySize(TRUE);
            CToolbarControl::Set(hWnd, NULL, bIsFullScreen?1:0);
        }
        break;
    case IDM_VIEW_MAINTOOLBAR_MEDIAPLAYER:
        {
            int Visible = HIWORD(m_Toolbar1MediaPlayer->GetValue());
            Visible = (Visible&2) | ((Visible&1)?0:1);
            m_Toolbar1MediaPlayer->SetValue( MAKELONG(LOWORD(m_Toolbar1MediaPlayer->GetValue()), Visible) );
            CToolbarControl::Set(hWnd, NULL);
            WorkoutOverlaySize(TRUE);
            CToolbarControl::Set(hWnd, NULL, bIsFullScreen?1:0);
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

