//
// $Id: ToolbarControl.cpp,v 1.1 2002-09-25 22:32:50 kooiman Exp $
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
//
// $Log: not supported by cvs2svn $
// Revision 1.0  2002/08/03 17:57:52  kooiman
// initial version
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ToolbarWindow.h"
#include "Toolbars.h"
#include "ToolbarControl.h"

#include "Status.h"
#include "DScaler.h"
#include "AspectRatio.h"
#include "DebugLog.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



CToolbarControl::CToolbarControl(long SetMessage) : CSettingsHolder(SetMessage),
Toolbar1(NULL),
Toolbar1Channels(NULL),
Toolbar1Volume(NULL),
Toolbar1Logo(NULL)
{
    CreateSettings("Toolbars");
        
}


CToolbarControl::~CToolbarControl()
{
    Free();
}


void CToolbarControl::CreateSettings(LPCSTR IniSection)
{
    m_ShowToolbar1 = new CShowToolbar1Setting(this, "ShowToolbar1", FALSE, IniSection);
    m_Settings.push_back(m_ShowToolbar1);

    m_Toolbar1Position = new CToolbar1PositionSetting(this, "Toolbar1Position", 1, 0, 1, IniSection);
    m_Settings.push_back(m_Toolbar1Position);

    m_Toolbar1Volume = new CToolbar1VolumeSetting(this, "VolumeBar", MAKELONG(2,1), 0x80000000L, 0x7F000000L, IniSection);
    m_Settings.push_back(m_Toolbar1Volume);

    m_Toolbar1Channels = new CToolbar1ChannelsSetting(this, "ChannelBar", MAKELONG(1,1), 0x80000000L, 0x7F000000L, IniSection);
    m_Settings.push_back(m_Toolbar1Channels);

    ReadFromIni();
}

void CToolbarControl::ShowToolbar1OnChange(long OldValue, long NewValue)
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



void CToolbarControl::Set(HWND hWnd, LPCSTR szSkinName)
{    
    //Initialize
    if (Toolbar1==NULL) 
    {
        Toolbar1 = new CToolbarWindow(hWnd, hDScalerInst, 1);
        Toolbar1->DefaultColor(GetSysColor(COLOR_BTNFACE));
                
        Toolbar1Channels = new CToolbarChannels(Toolbar1);        
        if (Toolbar1Channels->CreateFromDialog(MAKEINTRESOURCE(IDD_TOOLBAR_CHANNELS), hResourceInst)!=NULL)                
        {
            Toolbar1->Add(Toolbar1Channels,TOOLBARCHILD_ALIGN_LEFTCENTER,LOWORD(m_Toolbar1Channels->GetValue()),0);
        }                
        else
        {
            delete Toolbar1Channels;
            Toolbar1Channels = NULL;
        }

        Toolbar1Volume = new CToolbarVolume(Toolbar1);
        if (Toolbar1Volume->CreateFromDialog(MAKEINTRESOURCE(IDD_TOOLBAR_VOLUME), hResourceInst)!=NULL)
        {            
            Toolbar1->Add(Toolbar1Volume,TOOLBARCHILD_ALIGN_LEFTCENTER,LOWORD(m_Toolbar1Volume->GetValue()),0);
        }                
        else
        {
            delete Toolbar1Volume;
            Toolbar1Volume = NULL;
        }

        
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

    //Remove skin
    if ((szSkinName != NULL) && (szSkinName[0] == 0))
    {      
        Toolbar1->ClearSkin();
        
        if (Toolbar1Channels!=NULL)
        {
            Toolbar1Channels->RemoveSkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINUP);
            Toolbar1Channels->RemoveSkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINDOWN);
            Toolbar1Channels->RemoveSkinDlgItem(IDC_TOOLBAR_CHANNELS_PREVIOUS);
            Toolbar1Channels->Reset();
        }

        if (Toolbar1Volume != NULL)
        {
            Toolbar1Volume->RemoveSkinDlgItem(IDC_TOOLBAR_VOLUME_MUTE);
            Toolbar1Volume->RemoveSkinDlgItem(IDC_TOOLBAR_VOLUME_SLIDER);
            Toolbar1Volume->Reset();
        }
        if (Toolbar1Logo != NULL)
        {            
            Toolbar1Logo->RemoveSkinDlgItem(IDC_TOOLBAR_LOGO_LOGO);
            Toolbar1Logo->Reset();
        }
    }

    //Load skin
    if ((szSkinName != NULL) && (szSkinName[0] != 0))
    {
        char szSkinIniFile[MAX_PATH*2];
        strcpy(szSkinIniFile,GetSkinDirectory());
        strcat(szSkinIniFile,szSkinName);
        strcat(szSkinIniFile,"\\skin.ini");

        ///\todo check if the ini file exists
        
        vector<int>Results;
        Toolbar1->LoadSkin(szSkinIniFile,"Toolbar1",&Results);

        ///\todo Process errors

        if (Toolbar1Channels != NULL)
        {
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINUP, "SpinUp", BITMAPASBUTTON_PUSH, "ChannelBar", szSkinIniFile);
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINDOWN, "SpinDown", BITMAPASBUTTON_PUSH, "ChannelBar", szSkinIniFile);
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_PREVIOUS, "Previous", BITMAPASBUTTON_PUSH, "ChannelBar", szSkinIniFile);
            Toolbar1Channels->Reset();
        }

        if (Toolbar1Volume != NULL)
        {
            Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_MUTE, "Mute", BITMAPASBUTTON_CHECKBOX, "VolumeBar", szSkinIniFile);
            Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_SLIDER, "Volume", BITMAPASBUTTON_SLIDER, "VolumeBar", szSkinIniFile);            
            Toolbar1Volume->Reset();
        }        
        if (Toolbar1Logo != NULL)
        {
            Toolbar1Logo->SkinDlgItem(IDC_TOOLBAR_LOGO_LOGO, "Logo", BITMAPASBUTTON_PUSH, "LogoBar", szSkinIniFile);            
            Toolbar1Logo->Reset();
        }
        InvalidateRect(Toolbar1->GethWnd(), NULL, FALSE);
    }

    //Update visibility
    if (Toolbar1!=NULL)
    {                
        if (Toolbar1Channels!=NULL)
        {
            if (HIWORD(m_Toolbar1Channels->GetValue()))
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
            if (HIWORD(m_Toolbar1Volume->GetValue()))
            {
                Toolbar1->ShowChild(Toolbar1Volume);
            }
            else
            {
                Toolbar1->HideChild(Toolbar1Volume);
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
        if (m_ShowToolbar1->GetValue())
        {
            if ((m_Toolbar1Position->GetValue()==1) && (Toolbar1->GetPosition() != 1))
            {
                Toolbar1->SetPosition(1);
            }
            if ((m_Toolbar1Position->GetValue()==2) && (Toolbar1->GetPosition() != 0))
            {
                Toolbar1->SetPosition(0);
            }
            Toolbar1->Show();   
        }
        else
        {            
            Toolbar1->Hide();
        }
    }

    //if (ToolbarSideBar==NULL) 
    //{
        //ToolbarSideBar = new CToolbarWindow(hWnd, hDScalerInst, 0);        
        //ToolbarSideBar->LoadBorders(szIniFile,"SideBarChannels",NULL);
        //ToolbarSideBar->SetPosition(-2);
    //}
    
}

void CToolbarControl::Adjust(HWND hWnd, BOOL bRedraw)
{
    if ((Toolbar1!=NULL) && Toolbar1->Visible())
    {
        Toolbar1->UpdateWindowPosition(hWnd);
        if (bRedraw)
        {
            InvalidateRect(Toolbar1->GethWnd(), NULL, FALSE);
        }
    }
}

void CToolbarControl::Free()
{
    if (Toolbar1 != NULL)
    {                       
        Toolbar1->Remove(Toolbar1Channels);
        Toolbar1->Remove(Toolbar1Volume);
        Toolbar1->Remove(Toolbar1Logo);
        delete Toolbar1;
        Toolbar1 = NULL;
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
    if (Toolbar1Logo != NULL)
    {
        delete Toolbar1Logo;
        Toolbar1Logo = NULL;
    }
} 

void CToolbarControl::AdjustArea(LPRECT lpRect, int Crop)
{
        if ((Toolbar1!=NULL) && Toolbar1->Visible())
        {
            Toolbar1->AdjustArea(lpRect,1);
        }            
}

void CToolbarControl::UpdateMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_VIEW_TOOLBARS_MAINTOOLBAR, (m_ShowToolbar1->GetValue()!=0));
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_TOP, (m_Toolbar1Position->GetValue()==2));
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_CHANNELS, (HIWORD(m_Toolbar1Channels->GetValue())!=0));
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_VOLUME, (HIWORD(m_Toolbar1Volume->GetValue())!=0));
    
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_TOP, (m_ShowToolbar1->GetValue()!=0));
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_CHANNELS, (m_ShowToolbar1->GetValue()!=0));
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_VOLUME, (m_ShowToolbar1->GetValue()!=0));
}


BOOL CToolbarControl::ProcessToolbar1Selection(HWND hWnd, UINT uItem)
{
    switch (uItem)
    {
    case IDM_VIEW_TOOLBARS_MAINTOOLBAR: 
        m_ShowToolbar1->SetValue(!m_ShowToolbar1->GetValue());        
        CToolbarControl::Set(hWnd, NULL);  
        WorkoutOverlaySize(TRUE);
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
        CToolbarControl::Set(hWnd, NULL);
        WorkoutOverlaySize(TRUE);
        break;
    case IDM_VIEW_MAINTOOLBAR_CHANNELS:
        m_Toolbar1Channels->SetValue( MAKELONG(LOWORD(m_Toolbar1Channels->GetValue()), HIWORD(m_Toolbar1Channels->GetValue()) ? 0:1) );
        CToolbarControl::Set(hWnd, NULL);        
        break;
    case IDM_VIEW_MAINTOOLBAR_VOLUME:
        m_Toolbar1Volume->SetValue( MAKELONG(LOWORD(m_Toolbar1Volume->GetValue()), HIWORD(m_Toolbar1Volume->GetValue())) ? 0:1 );
        CToolbarControl::Set(hWnd, NULL);        
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
