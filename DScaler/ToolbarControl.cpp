//
// $Id: ToolbarControl.cpp,v 1.6 2003-08-09 14:44:57 laurentg Exp $
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
// Revision 1.5  2003/04/28 17:32:00  laurentg
// Fix to avoid crash when exiting
//
// Revision 1.4  2002/10/08 08:23:59  kooiman
// Fixed window auto-resize problem.
//
// Revision 1.3  2002/10/07 20:33:51  kooiman
// Fixed lots of toolbar bugs & added grip/separator bars.
//
// Revision 1.2  2002/09/26 16:34:19  kooiman
// Lots of toolbar fixes &added EVENT_VOLUME support.
//
// Revision 1.1  2002/09/25 22:32:50  kooiman
// Toolbar support.
//
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
#include "Providers.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/**
	Toolbar control creates & destroys the toolbar with the subbars inside.



*/

CToolbarControl::CToolbarControl(long SetMessage) : CSettingsHolder(SetMessage),
Toolbar1(NULL),
Toolbar1Channels(NULL),
Toolbar1Volume(NULL),
Toolbar1Logo(NULL)
{
    CreateSettings("Toolbars");
	
	eEventType EventList[] = {EVENT_SOURCE_CHANGE, EVENT_VIDEOINPUT_CHANGE, EVENT_ENDOFLIST};
	EventCollector->Register(this, EventList);

	
	BOOL bShowChannels = FALSE;

	//Is there a tuner?
	if (Providers_GetCurrentSource() != NULL)
	{
		int VideoInput = Providers_GetCurrentSource()->GetInput(VIDEOINPUT);
		bShowChannels = Providers_GetCurrentSource()->InputHasTuner(VIDEOINPUT,VideoInput);
	}
	
	//Set proper bit
	int Visible = HIWORD(m_Toolbar1Channels->GetValue());
	Visible = (Visible&1) | (bShowChannels?2:0);
	m_Toolbar1Channels->SetValue( MAKELONG(LOWORD(m_Toolbar1Channels->GetValue()), Visible) );				

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


void CToolbarControl::Set(HWND hWnd, LPCSTR szSkinName, int ForceHide)
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
    
    if ((szSkinName != NULL) && (szSkinName[0] == 0)) //Remove skin
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
				char szID[40];
				sprintf(szID,"Toolbar1Bar%d",i);
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
        if (Toolbar1Logo != NULL)
        {            
            Toolbar1Logo->RemoveSkinDlgItem(IDC_TOOLBAR_LOGO_LOGO);
            Toolbar1Logo->Reset();
			InvalidateRect(Toolbar1Logo->GethWnd(), NULL, FALSE);
        }
		InvalidateRect(Toolbar1->GethWnd(), NULL, FALSE);
    }
    
    if ((szSkinName != NULL) && (szSkinName[0] != 0)) //Load skin
    {
        char szSkinIniFile[MAX_PATH*2];
        strcpy(szSkinIniFile,GetSkinDirectory());
        strcat(szSkinIniFile,szSkinName);
        strcat(szSkinIniFile,"\\skin.ini");
		///\todo check if the ini file exists

		CBitmapCache BitmapCache;

		//Set default margin
		MarginsTop = MarginsDefault;
		MarginsBottom = MarginsDefault;

		//Read margins from ini file
		char szBuffer[256];
		char szDefault[] = {0};
		for (int Pos = 0; Pos <= 1; Pos++)
		{
			GetPrivateProfileString("Toolbar1",(Pos==0)?"MarginsTop":"MarginsBottom", szDefault, szBuffer, 255, szSkinIniFile);
			if (szBuffer[0] != 0)
			{
				TToolbar1Margins Margins;
				Margins = MarginsDefault;				

				if (sscanf(szBuffer,"%d,%d,%d,%d,%d,%d",&Margins.l,&Margins.t,&Margins.r,&Margins.b,&Margins.child_lr,&Margins.child_tb)>=4)
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
        Toolbar1->LoadSkin(szSkinIniFile,"Toolbar1",&Results, &BitmapCache);
        ///\todo Process errors

		//Load skin for toolbar separator window
		for (int i = 0; i < Toolbar1Bars.size(); i++)
		{
			if (Toolbar1Bars[i] != NULL)
			{
				char szID[40];
				sprintf(szID,"Toolbar1Bar%d",i);
				Toolbar1Bars[i]->SkinWindow(Toolbar1Bars[i]->GethWndPicture(), szID, "Bar", BITMAPASBUTTON_PUSH, "Toolbar1", szSkinIniFile, &BitmapCache);				
				InvalidateRect(Toolbar1Bars[i]->GethWnd(), NULL, FALSE);
			}
		}

        //Load skins for controls in subbar
		if (Toolbar1Channels != NULL)
        {
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINUP, "SpinUp", BITMAPASBUTTON_PUSH, "ChannelBar", szSkinIniFile, &BitmapCache);
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_SPINDOWN, "SpinDown", BITMAPASBUTTON_PUSH, "ChannelBar", szSkinIniFile, &BitmapCache);
            Toolbar1Channels->SkinDlgItem(IDC_TOOLBAR_CHANNELS_PREVIOUS, "Previous", BITMAPASBUTTON_PUSH, "ChannelBar", szSkinIniFile, &BitmapCache);
            Toolbar1Channels->Reset();
			InvalidateRect(Toolbar1Channels->GethWnd(), NULL, FALSE);
        }

        if (Toolbar1Volume != NULL)
        {
            Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_MUTE, "Mute", BITMAPASBUTTON_CHECKBOX, "VolumeBar", szSkinIniFile, &BitmapCache);
            Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_SLIDER, "Volume", BITMAPASBUTTON_SLIDER, "VolumeBar", szSkinIniFile, &BitmapCache);
			Toolbar1Volume->SkinDlgItem(IDC_TOOLBAR_VOLUME_CHANNEL, "SoundChannel", BITMAPASBUTTON_4STATE, "VolumeBar", szSkinIniFile, &BitmapCache);
            Toolbar1Volume->Reset();
			InvalidateRect(Toolbar1Volume->GethWnd(), NULL, FALSE);
        }        
        if (Toolbar1Logo != NULL)
        {
            Toolbar1Logo->SkinDlgItem(IDC_TOOLBAR_LOGO_LOGO, "Logo", BITMAPASBUTTON_PUSH, "LogoBar", szSkinIniFile, &BitmapCache);            
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
        //ToolbarSideBar->LoadBorders(szIniFile,"SideBarChannels",NULL);
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
		
	if (Toolbar1Bar->Create("Toolbar1Bar", hResourceInst) != NULL)			
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
			
			//Set proper bit
			int Visible = HIWORD(m_Toolbar1Channels->GetValue());
			Visible = (Visible&1) | (bShowChannels?2:0);
			m_Toolbar1Channels->SetValue( MAKELONG(LOWORD(m_Toolbar1Channels->GetValue()), Visible) );				

			if ((Toolbar1 != NULL) && Toolbar1->Visible())
			{
				CToolbarControl::Set(hWnd, NULL);
			}
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

//Delete all toolbars & childs
void CToolbarControl::Free()
{
    if (Toolbar1 != NULL)
    {                       
        Toolbar1->Remove(Toolbar1Channels);
        Toolbar1->Remove(Toolbar1Volume);
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
    CheckMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_VOLUME, (HIWORD(m_Toolbar1Volume->GetValue())!=0));
    
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_TOP, (m_ShowToolbar1->GetValue()!=0));
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_CHANNELS, (m_ShowToolbar1->GetValue()!=0));
    EnableMenuItemBool(hMenu, IDM_VIEW_MAINTOOLBAR_VOLUME, (m_ShowToolbar1->GetValue()!=0));
}

//Process menu commands
BOOL CToolbarControl::ProcessToolbar1Selection(HWND hWnd, UINT uItem)
{
    switch (uItem)
    {
    case IDM_VIEW_TOOLBARS_MAINTOOLBAR: 
        m_ShowToolbar1->SetValue(!m_ShowToolbar1->GetValue());		
		if ((Toolbar1 == NULL) && (m_ShowToolbar1->GetValue()))
		{
			extern char* szSkinName;
			CToolbarControl::Set(hWnd, szSkinName);	
		}
		else
		{
			CToolbarControl::Set(hWnd, NULL);
		}
		WorkoutOverlaySize(TRUE);
		CToolbarControl::Set(hWnd, NULL);
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
		CToolbarControl::Set(hWnd, NULL);
        break;
    case IDM_VIEW_MAINTOOLBAR_CHANNELS:
		{
			int Visible = HIWORD(m_Toolbar1Channels->GetValue());
			Visible = (Visible&2) | ((Visible&1)?0:1);
			m_Toolbar1Channels->SetValue( MAKELONG(LOWORD(m_Toolbar1Channels->GetValue()), Visible) );
			CToolbarControl::Set(hWnd, NULL);        
			WorkoutOverlaySize(TRUE);
			CToolbarControl::Set(hWnd, NULL);
		}
        break;
    case IDM_VIEW_MAINTOOLBAR_VOLUME:
        m_Toolbar1Volume->SetValue( MAKELONG(LOWORD(m_Toolbar1Volume->GetValue()), HIWORD(m_Toolbar1Volume->GetValue())?0:1) );
        CToolbarControl::Set(hWnd, NULL);
        WorkoutOverlaySize(TRUE);
		CToolbarControl::Set(hWnd, NULL);
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

