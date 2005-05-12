//
// $Id: BitmapAsButton.cpp,v 1.5 2005-05-12 08:33:18 adcockj Exp $
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
// Revision 1.4  2003/10/27 10:39:50  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.3  2002/10/07 20:32:43  kooiman
// Fixed small bugs. Added 3/4state button.
//
// Revision 1.2  2002/09/26 16:34:19  kooiman
// Lots of toolbar fixes &added EVENT_VOLUME support.
//
// Revision 1.1  2002/09/25 22:32:09  kooiman
// Bitmap as a button.
//
// Revision 1.0  2001/11/25 02:03:21  kooiman
// initial version
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file BitmapAsButton.cpp CBitmapAsButton Implementation
 */

#include "stdafx.h"

#include "Bitmap.h"
#include "BitmapAsButton.h"
#include "DebugLog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


////////////////////////////////////////////////////////////////////////////////
// Bitmap as button ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CBitmapAsButton::CBitmapAsButton(eBitmapAsButtonType ButtonType) :
m_mouseldown(FALSE),
m_mouserdown(FALSE),
m_mouseover(FALSE),
m_trackmouse(FALSE),
pfnButtonProc(NULL),
pfnButtonProc_pThis(NULL),
pOriginalProc(NULL),
hCursorHand(NULL)
{
    this->ButtonType = ButtonType;

    bFailed = FALSE;

    hWndButton = NULL;
    hWndParent = NULL;
     
    ButtonWidth = 10;
    ButtonHeight = 10;

    SliderPos = 0;
    SliderRangeMin = 0;
    SliderRangeMax = 1;
    ButtonState = 0;
}

CBitmapAsButton::~CBitmapAsButton()
{
    pfnButtonProc = NULL;
    if (hWndButton != NULL)
    {        
        DestroyWindow(hWndButton);        
        hWndButton = NULL;
    }
}    

// Create window
BOOL CBitmapAsButton::Create(string sID, HWND hWndParent, int x, int y, HINSTANCE hInst)
{
    bFailed = TRUE; //assume the worst

    if (hCursorHand == NULL)
    {
        hCursorHand = ::LoadCursor(NULL, IDC_HAND);
    }

    WNDCLASS wc;
    if (! ::GetClassInfo(hInst, sID.c_str(), &wc) )
    {
        //wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = 0; //CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
        wc.lpfnWndProc = StaticButtonProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst; 
        wc.hIcon = NULL;
        wc.hCursor = hCursorHand;
        wc.hbrBackground = HBRUSH(NULL); //CreateSolidBrush(0);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = sID.c_str();
        //wc.hIconSm = NULL;
        
        if (!RegisterClass(&wc))
        {
            return FALSE;
        }        
    }

    MDICREATESTRUCT mdic;
	::memset(&mdic, 0, sizeof(mdic));
	mdic.lParam = (LPARAM)this;

    this->sID = sID;
    this->hWndParent = hWndParent;

    hWndButton = CreateWindow(
      sID.c_str(),   // class name
      sID.c_str(),   // title
      WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
      x,         // starting x position 
      y,         // starting y position 
      ButtonWidth,        // button width 
      ButtonHeight,        // button height 
      hWndParent,       // parent window 
      NULL,       // No menu 
      hInst,
      &mdic);      // pointer not needed 
    
    if (hWndButton != NULL)
    {
        bFailed = FALSE;
        
        if (m_bhBmp.GetBitmap(0)!=NULL)
        {
            ButtonWidth=m_bhBmp.Width(0);
            ButtonHeight=m_bhBmp.Height(0);
            SetWindowPos(hWndButton, NULL, 0,0, ButtonWidth, ButtonHeight, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);            
            SetRegion(0);
        }        
    }
    
    return !bFailed;
}

// Take over the processing/drawing of dialog control
BOOL CBitmapAsButton::TakeOver(HWND hTakeOver, string sID, HWND hWndParent )
{
    //Restore first if it was already taken over
	RestoreBack(hTakeOver);

	bFailed = TRUE; //assume the worst

    if (hCursorHand == NULL)
    {
        hCursorHand = ::LoadCursor(NULL, IDC_HAND);
        // hand isn't available on all OSes so fall back to arrow
        if(hCursorHand == NULL)
        {
            hCursorHand = ::LoadCursor(NULL, IDC_ARROW);
        }
    }
     
    SetWindowLong(hTakeOver, GWL_USERDATA, (LONG)this);

    this->sID = sID;
    this->hWndParent = hWndParent;
    this->hWndButton = hTakeOver;

    void *pOrg = (void*)SetWindowLong(hTakeOver, GWL_WNDPROC, (LONG)StaticButtonProc);
    if (pOrg == NULL)
    {        
        //Failed
        hWndButton = NULL;
        return FALSE;
    }    
    
    if (m_bhBmp.GetBitmap(0)!=NULL)
    {
        ButtonWidth=m_bhBmp.Width(0);
        ButtonHeight=m_bhBmp.Height(0);

        if (this->pOriginalProc == NULL)
        {
            RECT rc;
            if (GetWindowRect(hWndButton, &rc))
            {
                this->OriginalWidth = rc.right-rc.left;
                this->OriginalHeight = rc.bottom-rc.top;
            }
        }
        SetWindowPos(hWndButton, NULL, 0,0, ButtonWidth, ButtonHeight, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);            
        SetRegion(0);    
    }        
    if (this->pOriginalProc == NULL)
    {
        this->pOriginalProc = pOrg;
    }    

    bFailed = FALSE;

    return !bFailed;
}

BOOL CBitmapAsButton::RestoreBack(HWND hWnd)
{
    if (hWnd == NULL)
	{
		hWnd = hWndButton;
	}
	if ((pOriginalProc != NULL) && (hWnd != NULL) && (hWnd==hWndButton))
    {
        SetWindowRgn(hWnd, NULL, FALSE);
        SetWindowLong(hWnd, GWL_WNDPROC, (LONG)pOriginalProc);    
        pOriginalProc = NULL;
        if ((OriginalWidth>0) && (OriginalHeight>0))
        {
            SetWindowPos(hWnd, NULL, 0,0, OriginalWidth, OriginalHeight, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE); 
        }
        OriginalWidth = -1;
        OriginalHeight = -1;
        InvalidateRect(hWnd, NULL, FALSE);
        hWndButton = NULL;        
        return TRUE;
    }
    return FALSE;
}

HRGN CBitmapAsButton::SetRegion(int State)
{
    if (hWndButton != NULL)
    {
        HRGN hRgn = m_bhBmp.GetWindowRegion(State);        
        SetWindowRgn(hWndButton, hRgn, TRUE);
        return hRgn;
    }
    return NULL;
}

// Add bitmaps
void CBitmapAsButton::AddBitmap(int State, HBITMAP hBmp, HBITMAP hBmpMask, BOOL bDeleteBitmapOnExit)
{
    m_bhBmp.Add(hBmp, hBmpMask, State, bDeleteBitmapOnExit);    
    if (State == 0)
    {
        int NewWidth = m_bhBmp.Width(State);
        int NewHeight = m_bhBmp.Height(State);
        if ((NewWidth != ButtonWidth) || (ButtonHeight!=NewHeight))
        {
            ButtonWidth=NewWidth;
            ButtonHeight=NewHeight;
            if (pOriginalProc != NULL)
            {
                RECT rc;
                if (GetWindowRect(hWndButton, &rc))
                {
                    this->OriginalWidth = rc.right-rc.left;
                    this->OriginalHeight = rc.bottom-rc.top;
                }
            }
            SetWindowPos(hWndButton, NULL, 0,0, ButtonWidth, ButtonHeight, SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);            
        }
        SetRegion(0);
    }
}


// Draw
void CBitmapAsButton::Draw(HDC hDC, LPRECT lpRect)
{    
    int State = 0;
    if (m_mouseldown||m_mouserdown)
    {
        State = 2;
    } 
    else if (m_mouseover)
    {
        State = 1;
    }

    if (ButtonType == BITMAPASBUTTON_PUSH)
    {        
        if (m_bhBmp.GetBitmap(State) == NULL) { State = 0; }

        POINT P1;
        POINT *pPBmpStart = NULL;
        if (lpRect != NULL)
        {
            pPBmpStart = &P1;
            pPBmpStart->x = lpRect->left;
            pPBmpStart->y = lpRect->top;
        }
        
        m_bhBmp.Draw(hDC,pPBmpStart,lpRect,State);
    }
    else if ((ButtonType == BITMAPASBUTTON_CHECKBOX)
			|| (ButtonType == BITMAPASBUTTON_3STATE) || (ButtonType == BITMAPASBUTTON_4STATE))
    {
        State += 3*ButtonState;		

        if (m_bhBmp.GetBitmap(State) == NULL) 
        { 
            if (State>(3*ButtonState)) 
			{ 
				State=3*ButtonState; 
			}
        }
        if (m_bhBmp.GetBitmap(State) == NULL) 
		{ 
			State = 0; 
        }

        POINT P1;
        POINT *pPBmpStart = NULL;
        if (lpRect != NULL)
        {
            pPBmpStart = &P1;
            pPBmpStart->x = lpRect->left;
            pPBmpStart->y = lpRect->top;
        }
        
        m_bhBmp.Draw(hDC,pPBmpStart,lpRect,State);
    }
    else if (ButtonType == BITMAPASBUTTON_SLIDER)
    {
        int SliderX = (SliderPos-SliderRangeMin)*ButtonWidth/(SliderRangeMax-SliderRangeMin);
        RECT rcLeft;
        RECT rcRight;
        RECT rc;
        POINT P1;
        
        int State3 = State+3;

        ::SetRect(&rcLeft, 0, 0, SliderX, ButtonHeight);
        ::SetRect(&rcRight, SliderX, 0, ButtonWidth, ButtonHeight);

        if (m_bhBmp.GetBitmap(State) == NULL) 
        {
            State = 0;
        }
        if (m_bhBmp.GetBitmap(State3) == NULL) 
        {
            State3 = 3;
        }
        
        if (lpRect == NULL)
        {
            m_bhBmp.Draw(hDC,NULL,&rcLeft,State3);
            m_bhBmp.Draw(hDC,&P1,&rcRight,State);
        }
        else
        {
            if (IntersectRect(&rc, &rcLeft,lpRect))
            {
                P1.x=rc.left;
                P1.y=rc.top;
                m_bhBmp.Draw(hDC,&P1,&rc,State3);
            }
            if (IntersectRect(&rc, &rcRight,lpRect))
            {
                P1.x=rc.left;
                P1.y=rc.top;
                m_bhBmp.Draw(hDC,&P1,&rc,State);
            }
        }

    }
}


LRESULT CBitmapAsButton::ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
        return TRUE;

    if (hDlg != hWndButton)
    {
        return FALSE;
    }
    switch(message)
    {        
    case WM_NCHITTEST:
        {
            LONG Result;
            if (pfnButtonProc != NULL)
            {
                UINT MouseFlags = 0;
                if (m_mouseldown)
                {
                    MouseFlags |= MK_LBUTTON;
                }
                if (m_mouserdown)
                {
                    MouseFlags |= MK_RBUTTON;
                }

                Result = pfnButtonProc(sID,pfnButtonProc_pThis,hWndParent,MouseFlags,hDlg,message,wParam,lParam);
                if (Result)
                {
                    return Result;
                }
            }
        }
        return HTCLIENT;

    ///// Mouse ///////////////////////////////////////////////

    case WM_RBUTTONDOWN:			
        m_mouserdown = TRUE;
        ::InvalidateRect( hDlg, NULL, false );
        break;
    case WM_RBUTTONUP:
        m_mouserdown = FALSE;
        ::InvalidateRect( hDlg, NULL, false );            
        break;

    case WM_LBUTTONDOWN:			
        m_mouseldown = TRUE;        

        if (ButtonType == BITMAPASBUTTON_SLIDER)
        {
            int xPos = GET_X_LPARAM(lParam); 
            int yPos = GET_Y_LPARAM(lParam); 
            int NewPos = SliderRangeMin + xPos*(SliderRangeMax-SliderRangeMin)/(ButtonWidth-1);
            if (NewPos != SliderPos)
            {                
                BOOL bRight = (NewPos>SliderPos);                                
                SliderPos = NewPos;                
                SendMessage(hWndParent, WM_HSCROLL, bRight?SB_RIGHT:SB_LEFT, (LPARAM)hWndButton);                
            }
        }
        ::InvalidateRect( hDlg, NULL, false );
        break;
    case WM_LBUTTONUP:
        m_mouseldown = FALSE;
        if (ButtonType == BITMAPASBUTTON_CHECKBOX)
        {
            ButtonState = (ButtonState)?0:1;
        }
		else if (ButtonType == BITMAPASBUTTON_3STATE)
        {
            /*ButtonState++;
			if (ButtonState>=3)
			{
				ButtonState=0;
			}*/
        }
		else if (ButtonType == BITMAPASBUTTON_4STATE)
        {
            /*ButtonState++;
			if (ButtonState>=4)
			{
				ButtonState=0;
			}*/
        }
        ::InvalidateRect( hDlg, NULL, false );            
        break;
    
    case WM_MOUSEMOVE:
        {
            if(!m_trackmouse)
            {   
                TRACKMOUSEEVENT lpEventTrack;

                lpEventTrack.cbSize      = sizeof( TRACKMOUSEEVENT );
                lpEventTrack.dwFlags     = TME_LEAVE;
                lpEventTrack.hwndTrack   = hDlg;

                if(!::_TrackMouseEvent(&lpEventTrack))
                {
                    //Failed
                }
                m_trackmouse = TRUE;
            }

            if (wParam != MK_LBUTTON )
            {	
                m_mouseldown = false;                    
            }
            if (wParam != MK_RBUTTON )
            {
                m_mouserdown = false; 
            }

            if (m_mouseldown)
            {
                if (ButtonType == BITMAPASBUTTON_SLIDER)
                {
                    int xPos = GET_X_LPARAM(lParam); 
                    int yPos = GET_Y_LPARAM(lParam);                     
                    int NewPos = SliderRangeMin + xPos*(SliderRangeMax-SliderRangeMin)/(ButtonWidth-1);
                    if (NewPos != SliderPos)
                    {                
                        BOOL bRight = (NewPos>SliderPos);
                        SliderPos = NewPos;
                        PAINTSTRUCT ps;
                        ::BeginPaint(hDlg,&ps);
                        Draw(ps.hdc,NULL);
                        ::EndPaint(hDlg,&ps);
                        SendMessage(hWndParent, WM_HSCROLL, bRight?SB_RIGHT:SB_LEFT, (LPARAM)hWndButton);                
                    }
                }
            }

            if (!m_mouseldown && !m_mouserdown)
            {
                m_mouseover = true;
                ::InvalidateRect( hDlg, NULL, false );                  
            }                
        }
        break;
    case WM_MOUSELEAVE:		    
        m_mouseover = false;
        m_mouseldown = false;
        m_mouserdown = false;
        ::InvalidateRect( hDlg, NULL, false );
        m_trackmouse = FALSE;
        break;        
    
    case WM_SETCURSOR:
        if (GetCursor() != hCursorHand)
        {
            SetCursor(hCursorHand);
        }
        break;
    ///////////////////////////////////////////////////////////

    case WM_PAINT:    
        {                
            PAINTSTRUCT ps;
            ::BeginPaint(hDlg,&ps);
            //LOG(2,"BitmapButton: 0x%08x: [%s] wm_paint: %d %d,%d,%d,%d",hDlg,sID.c_str(),ps.fErase,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom);
            if (ps.fErase) 
            {
                Draw(ps.hdc,NULL);
            }
            else
            {
                Draw(ps.hdc,&ps.rcPaint);
            }
            ::EndPaint(hDlg,&ps);
        }
        return TRUE;
        break;
    case WM_ERASEBKGND:
        //LOG(2,"BitmapButton: 0x%08x: [%s] erase bg",hDlg,sID.c_str());
        return TRUE;
    /////////////////////// Checkbox //////////////////////////////
	/////////////////////// 3/4state //////////////////////////////
    case BM_GETCHECK:
        if (ButtonType == BITMAPASBUTTON_CHECKBOX)
        {
            return (ButtonState)?BST_CHECKED:BST_UNCHECKED;            
        }
		if ((ButtonType == BITMAPASBUTTON_3STATE)||(ButtonType == BITMAPASBUTTON_4STATE))
        {
            return ButtonState;
        }
        break;

    case BM_SETCHECK:
        if (ButtonType == BITMAPASBUTTON_CHECKBOX)
        {
            ButtonState = (wParam == BST_CHECKED) ? 1:0;
            InvalidateRect(hDlg, NULL, false);
        }
		if ((ButtonType == BITMAPASBUTTON_3STATE)||(ButtonType == BITMAPASBUTTON_4STATE))
        {
            ButtonState = wParam;
            InvalidateRect(hDlg, NULL, false);
        }
        break;
	
    ////////////////////////// Slider //////////////////////////////
    case TBM_SETRANGE:
        if (ButtonType == BITMAPASBUTTON_SLIDER)
        {
            SliderRangeMin = LOWORD(lParam);
            SliderRangeMax = HIWORD(lParam);            
            if (wParam)
            {
                ::InvalidateRect( hDlg, NULL, false );   
            }
        }
        break;
    case TBM_SETPOS:
        if (ButtonType == BITMAPASBUTTON_SLIDER)
        {
            if (lParam< SliderRangeMin)
            {
                SliderPos = SliderRangeMin;
            }
            else if (lParam > SliderRangeMax)
            {
                SliderPos = SliderRangeMax;
            }
            else
            {
                SliderPos = lParam;
            }
            
            if (wParam)
            {
                ::InvalidateRect( hDlg, NULL, false );   
            }
        }
        break;
    case TBM_GETPOS:
        if (ButtonType == BITMAPASBUTTON_SLIDER)
        {
            return SliderPos;
        }
        break;

    }

    // Call button proc

    if (pfnButtonProc != NULL)
    {
        LONG Result;
        UINT MouseFlags = 0;
        if (m_mouseldown)
        {
            MouseFlags |= MK_LBUTTON;
        }
        if (m_mouserdown)
        {
            MouseFlags |= MK_RBUTTON;
        }
        Result = pfnButtonProc(sID,pfnButtonProc_pThis,hWndParent, MouseFlags, hDlg,message,wParam,lParam);        
        switch (message)
        {
        case WM_CLOSE:
        case WM_DESTROY:
            pfnButtonProc = NULL;
            break;
        }
        return Result;
    }

    return FALSE;
}


LRESULT CALLBACK CBitmapAsButton::StaticButtonProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
	CBitmapAsButton *pThis = NULL;

    if ( message == WM_NCCREATE )
	{
		if (lParam == 0)
        {
            return FALSE;
        }
        MDICREATESTRUCT * pMDIC = (MDICREATESTRUCT *)((LPCREATESTRUCT) lParam)->lpCreateParams;
		CBitmapAsButton *pThis = (CBitmapAsButton*) (pMDIC->lParam);
		::SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);        
        return TRUE;
	}
    
	pThis = (CBitmapAsButton*)::GetWindowLong(hWnd, GWL_USERDATA);    
    
    if (pThis != NULL)
    {
        return pThis->ButtonProc(hWnd, message, wParam, lParam);
    }
    return FALSE;
}
