//
// $Id: Toolbars.cpp,v 1.9 2002-10-22 00:12:32 flibuste2 Exp $
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
// Revision 1.8  2002/10/17 06:48:22  flibuste2
// Modified the channel classes :
// -- changed ASCII save file
// -- added CChannelList class
//
// Revision 1.7  2002/10/17 00:28:41  flibuste2
// Channels.h / Channels.cpp define the current CHANNELLIST and COUNTRYLIST
// This first check-in is prior to other enhancements and mostly reproduces
// the existing data structures for channel settings and user program list
//
// Revision 1.6  2002/10/07 20:33:50  kooiman
// Fixed lots of toolbar bugs & added grip/separator bars.
//
// Revision 1.5  2002/10/02 10:52:35  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.4  2002/10/01 15:53:16  adcockj
// Fixed crash on startup
//
// Revision 1.3  2002/09/28 13:34:36  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
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
#include "TVFormats.h"
#include "Channels.h"
#include "ToolbarWindow.h"
#include "Toolbars.h"

#include "Status.h"
#include "DScaler.h"
#include "ProgramList.h"
#include "Providers.h"
#include "VBI_VideoText.h"
#include "MixerDev.h"
#include "Audio.h"
#include "AspectRatio.h"
#include "DebugLog.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//typedef vector<CChannel*> CHANNELLIST;

extern CUserChannels MyChannels;
extern long CurrentProgram;


extern BOOL bUseMixer;
extern void Mixer_SetVolume(int Volume);

extern void ShowText(HWND hWnd, LPCTSTR szText);

///////////////////////////////////////////////////////////////////////////////
// Toolbar channels ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CToolbarChannels::CToolbarChannels(CToolbarWindow *pToolbar) : CToolbarChild(pToolbar),
    LastChannel(-1),
    m_oldComboProc(NULL),
	m_hIconChannelUp(NULL),
	m_hIconChannelDown(NULL),
	m_hIconChannelPrevious(NULL)
{
    eEventType EventList[] = {EVENT_CHANNEL_CHANGE, EVENT_ENDOFLIST};
	EventCollector->Register(this, EventList);   

	long OldValue;
	long NewValue;
	if (EventCollector->LastEventValues(Providers_GetCurrentSource(), EVENT_CHANNEL_CHANGE, &OldValue, &NewValue)>0)
	{
		LastChannel = NewValue;
	}	
}

CToolbarChannels::~CToolbarChannels()
{
    if (m_hIconChannelUp != NULL)
	{
		::DestroyIcon(m_hIconChannelUp);
		m_hIconChannelUp = NULL;
	}
	if (m_hIconChannelDown != NULL)
	{
		::DestroyIcon(m_hIconChannelDown);
		m_hIconChannelDown = NULL;
	}
	if (m_hIconChannelPrevious != NULL)
	{
		::DestroyIcon(m_hIconChannelPrevious);
		m_hIconChannelPrevious = NULL;
	}	
}


HWND CToolbarChannels::CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst)
{
	HWND hWnd = CToolbarChild::CreateFromDialog(lpTemplate, hResourceInst);

	if (hWnd != NULL)
	{
		//Steal messsage proc from combobox:
		HWND hWndCombo = GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST);

		if (hWndCombo != NULL)
		{
			::SetWindowLong(hWndCombo, GWL_USERDATA, (LONG)this);
		
			m_oldComboProc = (WNDPROC)SetWindowLong(hWndCombo, GWL_WNDPROC, (LONG)MyComboProcWrap);
		}
	}
	
	return hWnd;
}	


LRESULT CToolbarChannels::MyComboProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	
	
	switch (message)
	{
	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KEYUP:
		SetFocus(m_pToolbar->GethWndParent());
		return FALSE;	
	
	}
	return CallWindowProc(m_oldComboProc, hDlg, message, wParam, lParam);	
}

LRESULT CALLBACK CToolbarChannels::MyComboProcWrap(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	CToolbarChannels *pThis = (CToolbarChannels*)::GetWindowLong(hDlg, GWL_USERDATA);
	if (pThis != NULL)
	{
		return pThis->MyComboProc(hDlg,message,wParam,lParam);
	}
	return FALSE;
}


void CToolbarChannels::OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
    if (pEventObject != (CEventObject*)Providers_GetCurrentSource())
	{
		return;
	}
	if (Event == EVENT_CHANNEL_CHANGE)
    {
        LastChannel = NewValue;		
    }
	if ((hWnd != NULL) && Visible())
	{
		UpdateControls(NULL, FALSE);
	}
}


void CToolbarChannels::UpdateControls(HWND hWnd, bool bInitDialog)
{
   if (hWnd == NULL)
   {
        hWnd = this->hWnd;
   }
   if (hWnd == NULL) return;
   
   if (bInitDialog)
   {
      int ChannelListSize = MyChannels.GetSize();
      int Channel;
      int nIndex;
      int CurrentIndex = 0;

	  SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), CB_RESETCONTENT, 0, 0);

      for(Channel = 0; Channel < ChannelListSize; Channel++)
      {
          nIndex = SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), CB_ADDSTRING, 0, (long)MyChannels.GetChannelName(Channel));
          SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), CB_SETITEMDATA, nIndex, Channel);

          if (CurrentProgram == Channel)
          {
              CurrentIndex = nIndex;              
          }
      }
      ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), CurrentIndex);                   
    }
	else
	{
		int nIndex;
        for(nIndex = 0; nIndex < MyChannels.GetSize(); nIndex++)
        {
            if (ComboBox_GetItemData(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), nIndex) == LastChannel)
            {                       
                ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), nIndex);             
                return;             
            }
        }
	}
	SetFocus(m_pToolbar->GethWndParent());
}


LRESULT CToolbarChannels::ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{      
    if ((hWnd == NULL) && (message == WM_INITDIALOG))
    {
       m_hIconChannelUp = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_CHANNELS_UP),IMAGE_ICON,0,0,0);
        SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_SPINUP),BM_SETIMAGE,IMAGE_ICON,LPARAM(m_hIconChannelUp));

        m_hIconChannelDown = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_CHANNELS_DOWN),IMAGE_ICON,0,0,0);
        SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_SPINDOWN),BM_SETIMAGE,IMAGE_ICON,LPARAM(m_hIconChannelDown));
        		
		m_hIconChannelPrevious = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_CHANNELS_PREVIOUS),IMAGE_ICON,0,0,0);
        SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_PREVIOUS),BM_SETIMAGE,IMAGE_ICON,LPARAM(m_hIconChannelPrevious));

        UpdateControls(hDlg, TRUE);        
        return TRUE;
    }
    if (hWnd != hDlg) { return FALSE; }

    switch (message)
    {
    case  WM_ERASEBKGND:   
        return TRUE;
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;            
        ::BeginPaint(hDlg,&ps);                        
        if (ps.fErase)
        {                    
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,NULL);
        }
        else
        {
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,&ps.rcPaint);
        }
        ::EndPaint(hDlg, &ps);
        return TRUE;
        break;    


    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDC_TOOLBAR_CHANNELS_LIST:
                {
                    int Channel;
                    int Index =  SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_LIST), CB_GETCURSEL, 0, 0);
                    Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_LIST), Index);

                    if ((Channel >= 0) && (Channel != LastChannel))
                    {
                        LastChannel = Channel;
                        SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDC_TOOLBAR_CHANNELS_LIST,Channel);
						SetFocus(m_pToolbar->GethWndParent());
                    }
                    return TRUE;
                }
                break;
            case IDC_TOOLBAR_CHANNELS_SPINUP:
                {
                    SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDM_CHANNELPLUS,0);                                       
					SetFocus(m_pToolbar->GethWndParent());
                    return TRUE;
                }
            case IDC_TOOLBAR_CHANNELS_SPINDOWN:
                {
                    SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDM_CHANNELMINUS,0);
					SetFocus(m_pToolbar->GethWndParent());
                    return TRUE;
                }
            case IDC_TOOLBAR_CHANNELS_PREVIOUS:
                {
                    SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDM_CHANNEL_PREVIOUS,0);
					SetFocus(m_pToolbar->GethWndParent());
                    return TRUE;
                }
                break;            
            default:
                break;
        }
        break;
    case WM_NCHITTEST:
        {
            return HTCLIENT;
        }
        break;
	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *pDrawItem;
			char szText[100];
			HICON hIcon = NULL;
			int Align = TOOLBARBUTTON_ICON_HALIGN_CENTER|TOOLBARBUTTON_ICON_VALIGN_CENTER|
						TOOLBARBUTTON_TEXT_HALIGN_CENTER|TOOLBARBUTTON_TEXT_VALIGN_CENTER;
			
			szText[0] = 0;
			pDrawItem = (DRAWITEMSTRUCT*)lParam;
			
			if ((pDrawItem != NULL) && (pDrawItem->CtlID==IDC_TOOLBAR_CHANNELS_SPINUP))
			{
				hIcon = m_hIconChannelUp;
			}
			else if ((pDrawItem != NULL) && (pDrawItem->CtlID==IDC_TOOLBAR_CHANNELS_SPINDOWN))
			{
				hIcon = m_hIconChannelDown;
			}
			else if ((pDrawItem != NULL) && (pDrawItem->CtlID==IDC_TOOLBAR_CHANNELS_PREVIOUS))
			{
				hIcon = m_hIconChannelPrevious;
			}

			if ((pDrawItem != NULL) && ((hIcon != NULL) || (szText[0]!=0)))
			{
				RECT rc;
				GetClientRect(GetDlgItem(hDlg, pDrawItem->CtlID), &rc);

				DrawItem(pDrawItem, hIcon, szText, rc.right-rc.left, rc.bottom-rc.top, Align);				
			}
		}
		break;
            
    case WM_CLOSE:
    case WM_DESTROY:					
		if ((hDlg != NULL) && (m_oldComboProc!=NULL))
		{
		SetWindowLong(hDlg, GWL_WNDPROC, (LONG)m_oldComboProc);
		}
        break;
    }
    return FALSE;    
}

void CToolbarChannels::Reset()
{
    UpdateControls(hWnd, TRUE);        
}


///////////////////////////////////////////////////////////////////////////////
// Toolbar Volume ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CToolbarVolume::CToolbarVolume(CToolbarWindow *pToolbar) : CToolbarChild(pToolbar),
m_Mute(0),
m_Volume(0),
m_hIconMute(NULL),
m_hIconUnMute(NULL),
m_hIconMono(NULL),
m_hIconStereo(NULL),
m_hIconLang1(NULL),
m_hIconLang2(NULL)
{
	eEventType EventList[] = {EVENT_MUTE, EVENT_VOLUME, EVENT_MIXERVOLUME, EVENT_SOUNDCHANNEL, EVENT_SOURCE_CHANGE, EVENT_ENDOFLIST};
	EventCollector->Register(this, EventList);
	
	long OldValue;
	long NewValue;
	if (EventCollector->LastEventValues(EVENT_MUTE, NULL, &OldValue, &NewValue)>0)
	{
		m_Mute = NewValue;
	}

	m_VolumeMin = 0;
	m_VolumeMax = 100;
	m_UseMixer = TRUE;

	if (bUseMixer)
	{
		if (EventCollector->LastEventValues(EVENT_MIXERVOLUME, NULL, &OldValue, &NewValue)>0)
		{
			m_Volume = NewValue;
		}		
	} 
	else 
	{
		m_UseMixer = FALSE;
		if (EventCollector->LastEventValues(Providers_GetCurrentSource(), EVENT_VOLUME, &OldValue, &NewValue)>0)
		{
			m_Volume = NewValue;
		}
		
		if (Providers_GetCurrentSource() != NULL)
		{
			ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
			if (pSetting != NULL)
			{
				m_VolumeMin = pSetting->GetMin();
				m_VolumeMax = pSetting->GetMax();
			}
		}
	}

	m_SoundChannel = SOUNDCHANNEL_MONO;

	if (EventCollector->LastEventValues(Providers_GetCurrentSource(), EVENT_SOUNDCHANNEL, &OldValue, &NewValue)>0)
	{
		m_SoundChannel = (eSoundChannel)NewValue;
	}
}

CToolbarVolume::~CToolbarVolume()
{
	if (m_hIconMute != NULL)
	{
		::DestroyIcon(m_hIconMute);
		m_hIconMute = NULL;
	}
	if (m_hIconUnMute != NULL)
	{
		::DestroyIcon(m_hIconUnMute);
		m_hIconUnMute = NULL;
	}
}


void CToolbarVolume::OnEvent(CEventObject *pObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
	bool bVolumeLimitsChanged = FALSE;
	if ((Event == EVENT_SOURCE_CHANGE) || (bUseMixer != m_UseMixer))
	{
		m_VolumeMin = 0;
		m_VolumeMax = 100;
		m_UseMixer = bUseMixer;
	
		if (!bUseMixer && (Providers_GetCurrentSource() != NULL))
		{
			ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
			if (pSetting != NULL)
			{
				m_VolumeMin = pSetting->GetMin();
				m_VolumeMax = pSetting->GetMax();
			}
		}
		bVolumeLimitsChanged = TRUE;
	}
	else if (Event == EVENT_MUTE)
    {
        m_Mute = (NewValue)? TRUE : FALSE;
    } 
    else if ((Event == EVENT_VOLUME) && (!bUseMixer) && (pObject == (CEventObject*)Providers_GetCurrentSource()))		
    {
        m_Volume = NewValue;
    }
	else if ((Event == EVENT_MIXERVOLUME) && bUseMixer)
	{
		m_Volume = NewValue;
		LOG(2,"Toolbar Volume: Event: volume = %d",m_Volume);
	}    
	else if ((Event == EVENT_SOUNDCHANNEL) && (pObject == (CEventObject*)Providers_GetCurrentSource()))
	{
		m_SoundChannel = (eSoundChannel)NewValue;
	}
	if ((hWnd != NULL) && Visible())
	{		
		UpdateControls(NULL, bVolumeLimitsChanged);
	}
}

void CToolbarVolume::UpdateControls(HWND hWnd, bool bInitDialog)
{
   if (hWnd == NULL)
   {
        hWnd = this->hWnd;
   }
   if (hWnd == NULL) return;

   if (bInitDialog)
   {
        SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_VOLUME_SLIDER), TBM_SETRANGE, TRUE,(LPARAM)MAKELONG(0,m_VolumeMax-m_VolumeMin+1));
   }
   
    LOG(2,"Toolbar Volume: Update controls: volume = %d",m_Volume);
    SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_VOLUME_SLIDER), TBM_SETPOS, TRUE, m_VolumeMin+m_Volume);
  
    // Mute
    CheckDlgButton(hWnd, IDC_TOOLBAR_VOLUME_MUTE, m_Mute?BST_CHECKED:BST_UNCHECKED);  	
	//Repaint control with correct m_Mute
	InvalidateRect(GetDlgItem(hWnd, IDC_TOOLBAR_VOLUME_MUTE), NULL, FALSE);

	
	//Sound channel
	CheckDlgButton(hWnd, IDC_TOOLBAR_VOLUME_CHANNEL, (UINT)(m_SoundChannel-SOUNDCHANNEL_MONO));
	//Repaint control with correct m_SoundChannel
	InvalidateRect(GetDlgItem(hWnd, IDC_TOOLBAR_VOLUME_CHANNEL), NULL, FALSE);

	SetFocus(m_pToolbar->GethWndParent());
	
}

void CToolbarVolume::Reset()
{
    UpdateControls(hWnd, TRUE);        
}

LRESULT CToolbarVolume::ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ((hWnd == NULL) && (message == WM_INITDIALOG))
    {
    	m_hIconUnMute = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_VOLUME_UNMUTE),IMAGE_ICON,0,0,0);
		m_hIconMute = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_VOLUME_MUTE),IMAGE_ICON,0,0,0);

		m_hIconMono = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_VOLUME_MONO),IMAGE_ICON,0,0,0);
		m_hIconStereo = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_VOLUME_STEREO),IMAGE_ICON,0,0,0);
		m_hIconLang1  = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_VOLUME_STEREO),IMAGE_ICON,0,0,0);
		m_hIconLang2  = (HICON)LoadImage(hResourceInst, MAKEINTRESOURCE(IDI_TOOLBAR_VOLUME_STEREO),IMAGE_ICON,0,0,0);
        
        
		UpdateControls(hDlg, TRUE);        
        return TRUE;
    }
    if (hWnd != hDlg) { return FALSE; }

    switch (message)
    {
    case  WM_ERASEBKGND:
        return TRUE;
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;            
        ::BeginPaint(hDlg,&ps);                        
        if (ps.fErase)
        {                    
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,NULL);
        }
        else
        {
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,&ps.rcPaint);
        }
        ::EndPaint(hDlg, &ps);
        return TRUE;
        break;    
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_TOOLBAR_VOLUME_SLIDER))
        {                                        
            int Volume = SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_VOLUME_SLIDER), TBM_GETPOS, 0,0);
			Volume -= m_VolumeMin;

			if (Volume != m_Volume)
			{
				SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDC_TOOLBAR_VOLUME_SLIDER,Volume);
			}
            SetFocus(m_pToolbar->GethWndParent());
            return TRUE;
        }
        break;         
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDC_TOOLBAR_VOLUME_MUTE:
                {
                    //BOOL bMute = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_TOOLBAR_VOLUME_MUTE));
                   
					m_Mute=!m_Mute;
					CheckDlgButton(hDlg, IDC_TOOLBAR_VOLUME_MUTE, m_Mute);					
                    SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDC_TOOLBAR_VOLUME_MUTE, m_Mute);
					SetFocus(m_pToolbar->GethWndParent());					
                    return TRUE;
                }                   
                break;
			case IDC_TOOLBAR_VOLUME_CHANNEL:
				{
                    int num = m_SoundChannel;
					num++;
					if (num > SOUNDCHANNEL_LANGUAGE2)
					{
						num = SOUNDCHANNEL_MONO;
					}					
					SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND, IDC_TOOLBAR_VOLUME_CHANNEL, num);
					SetFocus(m_pToolbar->GethWndParent());					
                    return TRUE;
                }                   
                break;
            default:
            break;
        }    
        break;
	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *pDrawItem;
			HICON hIcon = NULL;
			int Align = TOOLBARBUTTON_ICON_HALIGN_CENTER|TOOLBARBUTTON_ICON_VALIGN_CENTER|
						TOOLBARBUTTON_TEXT_HALIGN_CENTER|TOOLBARBUTTON_TEXT_VALIGN_CENTER;
			char szText[100];
			
			szText[0] = 0;
			pDrawItem = (DRAWITEMSTRUCT*)lParam;
			
			if ((pDrawItem != NULL) && (pDrawItem->CtlID==IDC_TOOLBAR_VOLUME_CHANNEL))
			{
				switch (m_SoundChannel)
				{
				case SOUNDCHANNEL_STEREO: 
					hIcon = m_hIconStereo;
					strcpy(szText,"Stereo");
					break;
				case SOUNDCHANNEL_LANGUAGE1:
					hIcon = m_hIconLang1;
					strcpy(szText,"Lang 1");
					break;
				case SOUNDCHANNEL_LANGUAGE2:
					hIcon = m_hIconLang2;		
					strcpy(szText,"Lang 2");
					break;
				default:
					hIcon = m_hIconMono;						
					strcpy(szText,"Mono");
				}
				Align = TOOLBARBUTTON_ICON_HALIGN_CENTER|TOOLBARBUTTON_ICON_VALIGN_TOP|
						TOOLBARBUTTON_TEXT_HALIGN_CENTER|TOOLBARBUTTON_TEXT_VALIGN_BOTTOM;
			}
			
			if ((pDrawItem != NULL) && (pDrawItem->CtlID==IDC_TOOLBAR_VOLUME_MUTE))
			{
				if (m_Mute)
				{
					hIcon = m_hIconMute;
				}
				else
				{
					hIcon = m_hIconUnMute;
				}
			}

			if ((pDrawItem != NULL) && ((hIcon != NULL) || (szText[0]!=0)))
			{
				RECT rc;
				GetClientRect(GetDlgItem(hDlg, pDrawItem->CtlID), &rc);

				DrawItem(pDrawItem, hIcon, szText, rc.right-rc.left, rc.bottom-rc.top, Align);				
			}
		}
		break;
    case WM_NCHITTEST:
        {
            return HTCLIENT;
        }
        break;
 
    case WM_CLOSE:
    case WM_DESTROY:
        break;
    }
    return FALSE;
    
}



///////////////////////////////////////////////////////////////////////////////
// Toolbar Logo ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CToolbarLogo::CToolbarLogo(CToolbarWindow* pToolbar) : CToolbarChild(pToolbar)
{    
	OriginalLogoWidth = 0;
	OriginalLogoHeight =0;
}

HWND CToolbarLogo::CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst)
{
	HWND hWnd = CToolbarChild::CreateFromDialog(lpTemplate, hResourceInst);
	
	if (hWnd != NULL)
	{
		RECT rc;
		if (GetClientRect(GetDlgItem(hWnd, IDC_TOOLBAR_LOGO_LOGO), &rc))
		{
			OriginalLogoWidth = rc.right-rc.left;
			OriginalLogoHeight = rc.bottom-rc.top;
		}
	}
	return hWnd;

}

void CToolbarLogo::Reset()
{
    if ((Buttons.size()>0) && (Buttons[0] != NULL))
    {
        int Width = Buttons[0]->Width();
        int Height = Buttons[0]->Height();
        SetWindowPos(hWnd, NULL, 0,0, Width, Height, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
    }
	else
	{
		if (OriginalLogoWidth>0)
		{
			SetWindowPos(hWnd, NULL, 0,0, OriginalLogoWidth, OriginalLogoHeight, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		}
	}
    RECT rc;
    if (GetClientRect(GetDlgItem(hWnd, IDC_TOOLBAR_LOGO_LOGO), &rc))
    {
        //Fit to window
        SetWindowPos(GetDlgItem(hWnd, IDC_TOOLBAR_LOGO_LOGO), NULL, 0,0, rc.right-rc.left, rc.bottom-rc.top, SWP_NOZORDER|SWP_NOACTIVATE);
    }
}

LRESULT CToolbarLogo::ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ((hWnd == NULL) && (message == WM_INITDIALOG))
    {                
		return TRUE;
    }
    if (hWnd != hDlg) { return FALSE; }

    switch (message)
    {
    case  WM_ERASEBKGND:
        return TRUE;
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;            
        ::BeginPaint(hDlg,&ps);                        
        if (ps.fErase)
        {                    
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,NULL);
        }
        else
        {
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,&ps.rcPaint);
        }
        ::EndPaint(hDlg, &ps);
        return TRUE;
        break;    
    case WM_NCHITTEST:
        {
            return HTCLIENT;
        }
        break; 
	case WM_COMMAND:
		SetFocus(m_pToolbar->GethWndParent());
		return FALSE;
    case WM_CLOSE:
    case WM_DESTROY:
        return FALSE;
    }
    return FALSE;    
}


///////////////////////////////////////////////////////////////////////////////
// Toolbar Bar ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CToolbar1Bar::CToolbar1Bar(CToolbarWindow* pToolbar) : CToolbarChild(pToolbar)
{    
	OriginalWidth = 0;
	OriginalHeight =0;
	LeftMargin = 0;
	RightMargin = 0;
	hWndPicture = NULL;
}

CToolbar1Bar::~CToolbar1Bar()
{
	if (hWndPicture != NULL)
	{
		DestroyWindow(hWndPicture);
		hWndPicture = NULL;
	}
	if (hBmp != NULL)
	{
		::DeleteObject(hBmp);
		hBmp = NULL;
	}
}

HWND CToolbar1Bar::Create(LPCSTR szClassName, HINSTANCE hResourceInst)
{
	HWND hWnd = CToolbarChild::Create(szClassName, hResourceInst);
	
	if (hWnd != NULL)
	{
		//Create picture window
		hWndPicture = CreateWindow(
						  "Static",      
						  "",
						  SS_BITMAP | WS_CHILD | WS_VISIBLE,
						  LeftMargin,         // starting x position 
						  0,         // starting y position 
						  10,        // width 
						  100,        // height 
						  hWnd,  // parent window 
						  NULL,        // No menu 
						  hResourceInst,
						  NULL); 
		    
		if (hWndPicture != NULL)
		{
			hBmp = LoadBitmap(hResourceInst, MAKEINTRESOURCE(IDB_TOOLBAR_BAR_BAR));
	        SendMessage(hWndPicture,STM_SETIMAGE,IMAGE_BITMAP,LPARAM(hBmp));
			
			RECT rc;
			GetWindowRect(hWndPicture, &rc);
			OriginalWidth = rc.right-rc.left;
			OriginalHeight = rc.bottom-rc.top;

			Reset();			
		}
	}
	return hWnd;

}

void CToolbar1Bar::Reset()
{
    if ((Buttons.size()>0) && (Buttons[0] != NULL))
    {
        int Width = Buttons[0]->Width();
        int Height = Buttons[0]->Height();
		CToolbarChild::SetPos(0,0,LeftMargin + Width + RightMargin,Height, TRUE);			
    }
	else
	{
		if (OriginalWidth>0)
		{
			CToolbarChild::SetPos(0,0,LeftMargin + OriginalWidth + RightMargin,OriginalHeight, TRUE);			
		}
	}	
}

void CToolbar1Bar::Margins(int l,int r) 
{ 
	LeftMargin=l; 
	RightMargin=r;
	Reset();
	if (hWndPicture != NULL)
	{
		SetWindowPos(hWndPicture, NULL, LeftMargin,0, 0,0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	}
}

LRESULT CToolbar1Bar::ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ((hWnd == NULL) && (message == WM_INITDIALOG))
    {                
		return TRUE;
    }
    if (hWnd != hDlg) { return FALSE; }

    switch (message)
    {
    case  WM_ERASEBKGND:
        return TRUE;
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;            
        ::BeginPaint(hDlg,&ps);                        
        if (ps.fErase)
        {                    
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,NULL);
        }
        else
        {
            m_pToolbar->PaintChildBG(hDlg,ps.hdc,&ps.rcPaint);
        }
        ::EndPaint(hDlg, &ps);
        return TRUE;
        break;    
    case WM_NCHITTEST:
        {
            return HTCLIENT;
        }
        break; 
	case WM_COMMAND:
		SetFocus(m_pToolbar->GethWndParent());
		return FALSE;
    case WM_CLOSE:
    case WM_DESTROY:
        return FALSE;
    }
    return FALSE;    
}
