//
// $Id: Toolbars.cpp,v 1.2 2002-09-26 16:34:19 kooiman Exp $
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


typedef vector<CChannel*> CHANNELLIST;

extern CHANNELLIST MyChannels;
extern long CurrentProgram;


extern BOOL bUseMixer;
extern void Mixer_SetVolume(int Volume);

extern void ShowText(HWND hWnd, LPCTSTR szText);

///////////////////////////////////////////////////////////////////////////////
// Toolbar channels ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CToolbarChannels::CToolbarChannels(CToolbarWindow *pToolbar) : CToolbarChild(pToolbar),
LastChannel(-1),
m_oldComboProc(NULL)
{
    eEventType EventList[] = {EVENT_CHANNEL_CHANGE, EVENT_ENDOFLIST};
	EventCollector->Register(this, EventList);   

	long OldValue;
	long NewValue;
	if (EventCollector->LastEventValues(EVENT_CHANNEL_CHANGE, &OldValue, &NewValue)>0)
	{
		LastChannel = NewValue;
	}	
}

CToolbarChannels::~CToolbarChannels()
{
    EventCollector->Unregister(this);

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
		
			m_oldComboProc = (void*)SetWindowLong(hWndCombo, GWL_WNDPROC, (LONG)MyComboProcWrap);
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
		return FALSE;	
	
	}
	return CallWindowProc((WNDPROC)m_oldComboProc, hDlg, message, wParam, lParam);	
}

LRESULT CToolbarChannels::MyComboProcWrap(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	CToolbarChannels *pThis = (CToolbarChannels*)::GetWindowLong(hDlg, GWL_USERDATA);
	if (pThis != NULL)
	{
		return pThis->MyComboProc(hDlg,message,wParam,lParam);
	}
	return FALSE;
}


void CToolbarChannels::OnEvent(eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
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
      int ChannelListSize = MyChannels.size();
      int Channel;
      int nIndex;
      int CurrentIndex = 0;

	  SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), CB_RESETCONTENT, 0, 0);

      for(Channel = 0; Channel < ChannelListSize; Channel++)
      {
          nIndex = SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), CB_ADDSTRING, 0, (long)MyChannels[Channel]->GetName());
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
        for(nIndex = 0; nIndex < MyChannels.size(); nIndex++)
        {
            if (ComboBox_GetItemData(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), nIndex) == LastChannel)
            {                       
                ComboBox_SetCurSel(GetDlgItem(hWnd, IDC_TOOLBAR_CHANNELS_LIST), nIndex);             
                return;             
            }
        }
	}
}


LRESULT CToolbarChannels::ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{      
    if (message == WM_GETDLGCODE)
	{
        return DLGC_WANTCHARS;
	}
	if ((hWnd == NULL) && (message == WM_INITDIALOG))
    {
        HBITMAP hBmp;
        
        hBmp = LoadBitmap(hResourceInst, MAKEINTRESOURCE(IDB_TOOLBAR_CHANNELS_UP));
        SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_SPINUP),BM_SETIMAGE,IMAGE_BITMAP,LPARAM(hBmp));

        hBmp = LoadBitmap(hResourceInst, MAKEINTRESOURCE(IDB_TOOLBAR_CHANNELS_DOWN));
        SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_SPINDOWN),BM_SETIMAGE,IMAGE_BITMAP,LPARAM(hBmp));

        hBmp = LoadBitmap(hResourceInst, MAKEINTRESOURCE(IDB_TOOLBAR_CHANNELS_PREVIOUS));
        SendMessage(GetDlgItem(hDlg, IDC_TOOLBAR_CHANNELS_PREVIOUS),BM_SETIMAGE,IMAGE_BITMAP,LPARAM(hBmp));        

        UpdateControls(hDlg, TRUE);        
        return TRUE;
    }
    if (hWnd != hDlg) { return FALSE; }

    switch (message)
    {
    case  WM_ERASEBKGND:
        //LOG(2,"Toolbar Channels: 0x%08x: bg erase",hWnd);
        return TRUE;
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;            
        ::BeginPaint(hDlg,&ps);                        
        //LOG(2,"Toolbar Channels: 0x%08x: wm_paint: %d %d,%d,%d,%d",hWnd,ps.fErase,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom);
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
                    }
                    return TRUE;
                }
                break;
            case IDC_TOOLBAR_CHANNELS_SPINUP:
                {
                    SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDM_CHANNELPLUS,0);                                       
                    return TRUE;
                }
            case IDC_TOOLBAR_CHANNELS_SPINDOWN:
                {
                    SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDM_CHANNELMINUS,0);
                    return TRUE;
                }
            case IDC_TOOLBAR_CHANNELS_PREVIOUS:
                {
                    SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDM_CHANNEL_PREVIOUS,0);
                    return TRUE;
                }
                break;            
            default:
                break;
        }
        break;
    case WM_NCHITTEST:
        {
            //return ::DefWindowProc(pThis->GethWndParent(), WM_NCLBUTTONDOWN, HTCAPTION, lParam);
            return HTCLIENT;
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
m_Volume(0)
{
	eEventType EventList[] = {EVENT_MUTE, EVENT_VOLUME, EVENT_MIXERVOLUME, EVENT_ENDOFLIST};
	EventCollector->Register(this, EventList);
	
	long OldValue;
	long NewValue;
	if (EventCollector->LastEventValues(EVENT_MUTE, &OldValue, &NewValue)>0)
	{
		m_Mute = NewValue;
	}
	if (EventCollector->LastEventValues(EVENT_VOLUME, &OldValue, &NewValue)>0)
	{
		m_Volume = NewValue;
	} 
	else if (EventCollector->LastEventValues(EVENT_MIXERVOLUME, &OldValue, &NewValue)>0)
	{
		m_Volume = NewValue*10;
	}
}

CToolbarVolume::~CToolbarVolume()
{
	EventCollector->Unregister(this);
}


void CToolbarVolume::OnEvent(eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
	if (Event == EVENT_MUTE)
    {
        m_Mute = (NewValue)? TRUE : FALSE;
    } 
    else if (Event == EVENT_VOLUME)
    {
        m_Volume = NewValue;
    }
	else  if (Event == EVENT_MIXERVOLUME)
	{
		m_Volume = NewValue*10;
	}    
	if ((hWnd != NULL) && Visible())
	{
		UpdateControls(NULL, FALSE);
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
        SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_VOLUME_SLIDER), TBM_SETRANGE, TRUE,(LPARAM)MAKELONG((int)0,(int)1000));
   }
   
   SendMessage(GetDlgItem(hWnd, IDC_TOOLBAR_VOLUME_SLIDER), TBM_SETPOS, TRUE, m_Volume);

   // Mute
   CheckDlgButton(hWnd, IDC_TOOLBAR_VOLUME_MUTE, m_Mute);  
}

void CToolbarVolume::Reset()
{
    UpdateControls(hWnd, TRUE);        
}

LRESULT CToolbarVolume::ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ((hWnd == NULL) && (message == WM_INITDIALOG))
    {
        UpdateControls(hDlg, TRUE);        
        return TRUE;
    }
    if (hWnd != hDlg) { return FALSE; }

    switch (message)
    {
    case  WM_ERASEBKGND:
        //LOG(2,"Toolbar Volume: 0x%08x: bg erase",hWnd);
        return TRUE;
        break;
    case WM_PAINT:
        PAINTSTRUCT ps;            
        ::BeginPaint(hDlg,&ps);                        
        //LOG(2,"Toolbar Volume: 0x%08x: wm_paint: %d %d,%d,%d,%d",hWnd,ps.fErase,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom);
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

            SendMessage(m_pToolbar->GethWndParent(),WM_COMMAND,IDC_TOOLBAR_VOLUME_SLIDER,Volume);
            
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
                    return TRUE;
                }                   
                break;
            default:
            break;
        }    
        break;
    case WM_NCHITTEST:
        {
            //return ::DefWindowProc(pThis->GethWndParent(), WM_NCLBUTTONDOWN, HTCAPTION, lParam);
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

CToolbarLogo::CToolbarLogo(CToolbarWindow *pToolbar) : CToolbarChild(pToolbar)
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
	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KEYUP:
		return FALSE;	
    case WM_CLOSE:
    case WM_DESTROY:
        return FALSE;
    }
    return FALSE;
    
}

