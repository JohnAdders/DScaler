//
// $Id: ToolbarWindow.cpp,v 1.3 2002-10-07 20:33:50 kooiman Exp $
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

#include "Bitmap.h"
#include "BitmapAsButton.h"
#include "ToolbarWindow.h"
#include "Toolbars.h"
#include "DebugLog.h"

#include "DScaler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////
// Toolbar window /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CToolbarWindow::CToolbarWindow(HWND hWndParent, HINSTANCE hInst, int Child)
: CWindowBorder(hWndParent, hInst),
IsToolbarVisible(0),
MainToolbarPosition(1),
bChildOrderChanged(TRUE)
{
    this->hWnd = NULL;
    this->hWndParent = hWndParent;
    
    WNDCLASS wc;
    if (!::GetClassInfo(hInst, "DSCALERTOOLBARWINDOW", &wc) )
    {
        //wc.cbSize = sizeof(WNDCLASS);
        wc.style = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ToolbarProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst; 
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = HBRUSH(NULL);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = "DSCALERTOOLBARWINDOW";        
        
        if (!RegisterClass(&wc))
        {
            return;
        }        
    }

    MDICREATESTRUCT mdic;
	::memset(&mdic, 0, sizeof(mdic));
	mdic.lParam = (LPARAM)this;

	hWndToolbar = CreateWindow(
      "DSCALERTOOLBARWINDOW",      
      "DSCALERTOOLBARWINDOW",
      (Child?WS_CHILD:WS_POPUP) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
      0,         // starting x position 
      50,         // starting y position 
      500,       // width 
      100,        // height 
      Child?hWndParent:NULL,       // parent window 
	  NULL,       // No menu 
      hInst,
      &mdic);      // pointer not needed 

    this->hWnd = hWndToolbar;

    TopMargin = 5;
    BottomMargin = 5;
    LeftMargin = 5;
    RightMargin = 5;

    ChildLeftRightMargin = 10;
    ChildTopBottomMargin = 5;

	m_lastWindowPos_x = -1;
    m_lastWindowPos_y = -1;
    m_lastWindowPos_w = -1;
    m_lastWindowPos_h = -1;
    
    SolidBorder(LeftMargin,TopMargin,RightMargin,BottomMargin,0xFFFFFF);
    CWindowBorder::Show();

    UpdateWindowPosition(hWndParent);
}


CToolbarWindow::~CToolbarWindow()
{
    for (int i = 0; i < vChildList.size(); i++)
    {
        if (vChildList[i].pChild != NULL)
        {
            ::SetWindowLong(vChildList[i].pChild->GethWnd(), GWL_USERDATA, NULL);


			//Doesn't work.
            //Is 'pure virtual function call', somehow fix this..
            //delete vChildList[i].pChild;
        }
    }
    vChildList.clear();
    if (hWndToolbar != NULL)
    {
        ::SetWindowLong(hWndToolbar, GWL_USERDATA, NULL);
		DestroyWindow(hWndToolbar);
        hWndToolbar = NULL;
    }
}



int CToolbarWindow::Width()
{
    RECT rc;
    GetWindowRect(hWnd, &rc);
    return rc.right-rc.left;
}

int CToolbarWindow::Height()
{
    RECT rc;
    GetWindowRect(hWnd, &rc);
    return rc.bottom-rc.top;
}

BOOL CToolbarWindow::Add(CToolbarChild *pChild, eToolbarRowAlign Align, int Order, int Row)
{
    if (pChild == NULL) return FALSE;
    if (pChild->GethWnd() == NULL) return FALSE;
    
    Remove(pChild); // No duplicates

    TChildInfo ci;
    ci.pChild = pChild;
    ci.bShow = TRUE;
    ci.Order = Order;
    ci.Row = Row;
    ci.Align = Align; //TOOLBARCHILD_ALIGN_LEFTCENTER;
	ci.pBarLeft = NULL;
	ci.pBarRight = NULL;
    vChildList.push_back(ci);    

    bChildOrderChanged = TRUE;
    return TRUE;
}

void CToolbarWindow::Remove(CToolbarChild *pChild)
{
    vector<TChildInfo> vNewList;

    for (int i = 0 ; i < vChildList.size(); i++)
    {
        if ((vChildList[i].pChild == pChild) || (vChildList[i].pChild == NULL))
        {
            bChildOrderChanged = TRUE;
        }
        else
        {
            vNewList.push_back(vChildList[i]);
        }
    }
    vChildList = vNewList;    
}

BOOL CToolbarWindow::AttachBar(CToolbarChild *pChild, int Left, CToolbarChild *pBar)
{
	for (int i = 0 ; i < vChildList.size(); i++)
    {
        if (vChildList[i].pChild == pChild)
        {
            if (Left)
			{
				vChildList[i].pBarLeft = pBar;
			}
			else
			{
				vChildList[i].pBarRight = pBar;
			}
			bChildOrderChanged = TRUE;
			return TRUE;
        }        
    }
	return FALSE;
}

BOOL CToolbarWindow::Show() 
{ 
   UpdateWindowPosition(hWndParent);
   for (int i = 0; i < vChildList.size(); i++)
   {
       if (vChildList[i].pChild != NULL)
       {
           ShowWindow(vChildList[i].pChild->GethWnd(), (vChildList[i].bShow) ? SW_SHOW : SW_HIDE);
			if (vChildList[i].bShow)
			{
				vChildList[i].pChild->Show();
				if (vChildList[i].pBarLeft != NULL)
				{
					vChildList[i].pBarLeft->Show();
				}
				if (vChildList[i].pBarRight != NULL)
				{
					vChildList[i].pBarRight->Show();
				}
			}
			else
			{
				vChildList[i].pChild->Hide();
				if (vChildList[i].pBarLeft != NULL)
				{
					vChildList[i].pBarLeft->Hide();
				}
				if (vChildList[i].pBarRight != NULL)
				{
					vChildList[i].pBarRight->Hide();
				}
			}		   
       }
   }
   InvalidateRect(hWndToolbar, NULL, FALSE);   
   IsToolbarVisible = 1; 
   return ShowWindow(hWndToolbar, SW_SHOW); 
}

BOOL CToolbarWindow::Hide() 
{ 
    for (int i = 0; i < vChildList.size(); i++)
    {
       //ShowWindow(vChildList[i].GetWnd(), SW_HIDE);
    }

    IsToolbarVisible = 0; 
    return ShowWindow(hWndToolbar, SW_HIDE); 
}


BOOL CToolbarWindow::SetPos(int x, int y, int w, int h)
{    
    ///\todo implement rows

    if (bChildOrderChanged)
    {        
        bChildOrderChanged = FALSE;
		vChildOrder.clear();
        
        vector<int> TempList; 
        int i;
        int j;
        int k;
        ChildOrderRightPos = 0;
        for (i = 0; i < vChildList.size(); i++)
        {
            if ((vChildList[i].bShow && (vChildList[i].pChild != NULL)))
            {
                if ((vChildList[i].Align>=TOOLBARCHILD_ALIGN_LEFTTOP) && (vChildList[i].Align<=TOOLBARCHILD_ALIGN_LEFTBOTTOM))
                {
                    for (j = 0; j < ChildOrderRightPos; j++)
                    {
                        if (vChildList[i].Order<vChildList[vChildOrder[j]].Order)
                        {                                                
                            break;
                        }
                    }
                    ChildOrderRightPos++;
                }
                else
                {                
                    for (j = ChildOrderRightPos; j < vChildOrder.size(); j++)
                    {
                        if (vChildList[i].Order<vChildList[vChildOrder[j]].Order)
                        {                        
                            break;
                        }
                    }                
                }

                k=0; 
                TempList.clear(); 
                while (k<j) { TempList.push_back(vChildOrder[k]); k++; }
                TempList.push_back(i);
                while (k<vChildOrder.size()) { TempList.push_back(vChildOrder[k]); k++; }
                vChildOrder = TempList;            
            }
        }        

        //Find optimal height

        int MinHeight = TopMargin+BottomMargin;

        int n;
        for (n = 0; n < vChildOrder.size(); n++)
        {
            i = vChildOrder[n];
            if (vChildList[i].pChild->Height() > MinHeight)
            {
                MinHeight = vChildList[i].pChild->Height();
            }
        }
        FitHeight = MinHeight + TopMargin + BottomMargin;    
    }

    if (h<0)
    {
        if (h < -1)
		{
			h = FitHeight;
			y -= h;
		}
		else
		{
			h = FitHeight;
		}
    }

    if ((m_lastWindowPos_x != x) || (m_lastWindowPos_y != y) || (m_lastWindowPos_w != w) || (m_lastWindowPos_h != h))
    {
        SetWindowPos(hWndToolbar,NULL,x,y,w,h,SWP_NOACTIVATE|SWP_NOZORDER);				
        m_lastWindowPos_x = x;
        m_lastWindowPos_y = y;
        m_lastWindowPos_w = w;
        m_lastWindowPos_h = h;
    }

    
    //Set all child windows
    
    int Width;
    int XLPos = LeftMargin;
    int XRPos = w - RightMargin;
	int BarLeftWidth;
	int BarRightWidth;	
    int dy = 0;
    int n;
    int i;
    for (n = 0; n < vChildOrder.size(); n++)
    {              
       i = vChildOrder[n];
       
       Width = 0;
       if (vChildList[i].bShow && (vChildList[i].pChild != NULL))
       {
          Width = vChildList[i].pChild->Width();
       }
	   BarLeftWidth = 0;
	   BarRightWidth = 0;
	   if (vChildList[i].pBarLeft != NULL)
	   {
			BarLeftWidth = vChildList[i].pBarLeft->Width();
	   }
	   if (vChildList[i].pBarRight != NULL)
	   {
			BarRightWidth = vChildList[i].pBarRight->Width();
	   }
         
       if (Width > 0)
       {
            dy = 0;        
            if ((vChildList[i].Align==TOOLBARCHILD_ALIGN_LEFTCENTER) || (vChildList[i].Align==TOOLBARCHILD_ALIGN_RIGHTCENTER))
            {
                dy = (h - TopMargin - BottomMargin - vChildList[i].pChild->Height()) / 2;
            } else if ((vChildList[i].Align==TOOLBARCHILD_ALIGN_LEFTBOTTOM) || (vChildList[i].Align==TOOLBARCHILD_ALIGN_RIGHTBOTTOM))
            {
                dy = (h - BottomMargin - vChildList[i].pChild->Height());
            }
            if (n<ChildOrderRightPos)
            {                
				if (vChildList[i].pBarLeft != NULL)
				{
					vChildList[i].pBarLeft->SetPos(XLPos,TopMargin+dy, BarLeftWidth, vChildList[i].pChild->Height(), TRUE);
				}
				vChildList[i].pChild->SetPos(XLPos+BarLeftWidth,TopMargin+dy, Width, vChildList[i].pChild->Height(), TRUE);
				if (vChildList[i].pBarRight != NULL)
				{
					vChildList[i].pBarRight->SetPos(XLPos+BarLeftWidth+Width,TopMargin+dy, BarRightWidth, vChildList[i].pChild->Height(), TRUE);
				}
                XLPos += BarLeftWidth + Width + BarRightWidth + ChildLeftRightMargin;
            } 
            else 
            {   
                XRPos -= BarLeftWidth + Width + BarRightWidth + ChildLeftRightMargin;
                if (XRPos<XLPos)
                {                    
                    //Overlap
                    XRPos=XLPos + ChildLeftRightMargin;					
                }    
				if (vChildList[i].pBarLeft != NULL)
				{
					vChildList[i].pBarLeft->SetPos(XRPos,TopMargin+dy, BarLeftWidth, vChildList[i].pChild->Height(), TRUE);
				}
				vChildList[i].pChild->SetPos(XRPos+BarLeftWidth,TopMargin+dy, Width, vChildList[i].pChild->Height(), TRUE);                
				if (vChildList[i].pBarRight != NULL)
				{
					vChildList[i].pBarRight->SetPos(XRPos+BarLeftWidth+Width,TopMargin+dy, BarRightWidth, vChildList[i].pChild->Height(), TRUE);
				}                                
            }
       }
       
    }
    //Invalidate(NULL,FALSE);
    return TRUE;    
}

BOOL CToolbarWindow::GetPos(LPRECT rc)
{
    return GetWindowRect(hWndToolbar,rc);
}

void CToolbarWindow::AdjustArea(RECT *ar, int Crop)
{
    if (!IsToolbarVisible) return;
        
    RECT rcBar;
    GetPos(&rcBar);
    
    int BarHeight = rcBar.bottom-rcBar.top;
    int BarWidth  = rcBar.right-rcBar.left;

    if (Crop)
    {
        if (MainToolbarPosition==0) //top
        {
            ar->top+=BarHeight;
        } 
        else if (MainToolbarPosition==1) //bottom
        {
            ar->bottom-=BarHeight;
        }
    }
    else
    {
        if (MainToolbarPosition==0) //top
        {
            ar->top-=BarHeight;
        } 
        else if (MainToolbarPosition==1) //bottom
        {
            ar->bottom+=BarHeight;
        }
    }
}    

void CToolbarWindow::UpdateWindowPosition(HWND hParentWnd)
{
      if (IsToolbarVisible<=0)
      {
         // return;
      }

      if (hParentWnd == NULL)
      {
          hParentWnd = hWnd;
      }

      RECT rc;
      RECT rcBar;      
      
      BOOL result = GetClientRect(hParentWnd, &rc);
      
      int Visible = IsToolbarVisible; 
      IsToolbarVisible = 0;
      GetDisplayAreaRect(hParentWnd, &rc); 
      IsToolbarVisible = Visible;
      GetPos(&rcBar); 

      int Width = rc.right-rc.left;
      int Height = rcBar.bottom-rcBar.top;      
      
      if (MainToolbarPosition==0) //top
      {          
          if ((rcBar.left != rc.left) || (rcBar.top != rc.top) || (rcBar.right!=Width) || (rcBar.bottom != Height))
          {
              Height = -1;
              SetPos(rc.left,rc.top,Width, Height);
          }
      }
      else if (MainToolbarPosition==1) //bottom
      {                    
          int YPos = rc.bottom; // - Height;
          
          //if ((rcBar.left != rc.left) || (rcBar.top != YPos) || (rcBar.right!=Width) || (rcBar.bottom != (YPos+Height)))
		  if ((rcBar.left != rc.left) || (rcBar.right!=Width) || (rcBar.bottom != YPos))
          {
              Height = -2;
              SetPos(rc.left,YPos,Width, Height);              
          }
      }
}

void CToolbarWindow::SetPosition(int Pos)
{
    if (MainToolbarPosition != Pos)
    {
        MainToolbarPosition = Pos;
        UpdateWindowPosition(NULL);
    }
}

int CToolbarWindow::GetPosition()
{
    return MainToolbarPosition;
}


void CToolbarWindow::ShowChild(CToolbarChild *pChild)
{
    for (int i = 0; i < vChildList.size(); i++)
    {
        if (vChildList[i].pChild == pChild)
        {
            if (!vChildList[i].bShow)
            {
                vChildList[i].bShow = TRUE;
                bChildOrderChanged = TRUE;
            }
            return;
        }
    }
}

void CToolbarWindow::HideChild(CToolbarChild *pChild)
{
    for (int i = 0; i < vChildList.size(); i++)
    {
        if (vChildList[i].pChild == pChild)
        {
            if (vChildList[i].bShow)
            {
                vChildList[i].bShow = FALSE;
                bChildOrderChanged = TRUE;
            }
            return;
        }
    }
}
    
void CToolbarWindow::SetChildPosition(CToolbarChild *pChild, int Order, int Row)
{
    for (int i = 0; i < vChildList.size(); i++)
    {
        if (vChildList[i].pChild == pChild)
        {
            if ((vChildList[i].Order != Order) || (vChildList[i].Row != Row))
            {
                bChildOrderChanged = TRUE;
                vChildList[i].Order = Order;
                vChildList[i].Row = Row;
            }
            return;
        }
    }
}

void CToolbarWindow::PaintToolbar(HWND hWnd, HDC hDC, LPRECT lpRect, POINT *pPShift)
{    
    RECT rcToolbar;
    RECT rcMiddle ;
    RECT rc;

    FindBorderSizes();
    FindLocations();
    
    GetPos(&rcToolbar);
    rcToolbar.right = rcToolbar.right - rcToolbar.left;
    rcToolbar.bottom = rcToolbar.bottom - rcToolbar.top;
    rcToolbar.left = 0;
    rcToolbar.top = 0;
    
	if (IsSkinned())
	{			        
		// Border
		Paint(hWnd,hDC,lpRect, pPShift);

		::SetRect(&rcMiddle,LeftSize,TopSize,rcToolbar.right-RightSize,rcToolbar.bottom-BottomSize);
	}
	else
	{
		rcMiddle = rcToolbar;
	}
    
    if (lpRect == NULL)
    {
        lpRect = &rcToolbar;
    }
    
    // Center    
    if (IntersectRect(&rc, &rcMiddle, lpRect))
    {        
        if (pPShift!=NULL)
        {
            rc.left+=pPShift->x;
            rc.top+=pPShift->y;
            rc.right+=pPShift->x;
            rc.bottom+=pPShift->y;
        }   
        if (m_hBrush == NULL)
        {
            m_hBrush = CreateSolidBrush(DefaultColorRef);
        }
        FillRect(hDC,&rc,m_hBrush);
    }

}

void CToolbarWindow::PaintChildBG(HWND hWndChild, HDC hDC, LPRECT lpRect)
{
    int i;
    for (i = 0; i < vChildList.size(); i++)
    {        
        if ((vChildList[i].pChild != NULL) && 
			 (	 (vChildList[i].pChild->GethWnd() == hWndChild)
			  || ((vChildList[i].pBarLeft != NULL) && (vChildList[i].pBarLeft->GethWnd() == hWndChild)) 
			  || ((vChildList[i].pBarRight != NULL) && (vChildList[i].pBarRight->GethWnd() == hWndChild))
			 )
			)
        {
            RECT rcInParent;
            RECT rcTotalInParent;
            POINT PShift;

            vChildList[i].pChild->GetPos(&rcTotalInParent);
            PShift.x = rcTotalInParent.left;
            PShift.y = rcTotalInParent.top;

            if (lpRect == NULL)
            {
                lpRect = &rcTotalInParent;
            }
            else
            {
                rcInParent.left = lpRect->left + PShift.x;
                rcInParent.top = lpRect->top + PShift.y;
                rcInParent.right = lpRect->right + PShift.x;
                rcInParent.bottom = lpRect->bottom + PShift.y;
                lpRect = &rcInParent;
            }
            
            //Shift back
            PShift.x=-PShift.x;
            PShift.y=-PShift.y;
            PaintToolbar(hWndToolbar, hDC, lpRect, &PShift);            
            return;
        }
    }
}

BOOL CToolbarWindow::LoadSkin(const char *szSkinIniFile,  const char *szSection, vector<int> *Results, CBitmapCache *pBitmapCache)
{
	bChildOrderChanged = TRUE;
	return CWindowBorder::LoadSkin(szSkinIniFile, szSection, Results, pBitmapCache);
}

void CToolbarWindow::ClearSkin()
{
	bChildOrderChanged = TRUE;
	CWindowBorder::ClearSkin();
}

void CToolbarWindow::Margins(int l,int t,int r,int b, int child_lr, int child_tb)
{
	TopMargin = t;
    BottomMargin = b;
    LeftMargin = l;
    RightMargin = t;
    ChildLeftRightMargin = child_lr;
    ChildTopBottomMargin = child_tb;
}

LRESULT CALLBACK CToolbarWindow::ToolbarProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CToolbarWindow *pThis = NULL;

    if ( message == WM_NCCREATE )
	{
		if (lParam == 0)
        {
            return FALSE;
        }
        MDICREATESTRUCT * pMDIC = (MDICREATESTRUCT *)((LPCREATESTRUCT) lParam)->lpCreateParams;
		CToolbarWindow *pThis = (CToolbarWindow*) (pMDIC->lParam);
		::SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);        
        return TRUE;
	}
    
	pThis = (CToolbarWindow*)::GetWindowLong(hWnd, GWL_USERDATA);    
    
    switch (message)
    {
    case WM_NCCREATE:
        {            
            return TRUE;
        }
    case WM_ERASEBKGND:
         //LOG(2,"Toolbar main: 0x%08x: erase bg",pThis->GethWnd());
	     return TRUE;		 
    case WM_PAINT:
        {
            if ((pThis!=NULL) && (pThis->GethWnd() != NULL))
            {                                
                PAINTSTRUCT ps;            
                ::BeginPaint(pThis->GethWnd(),&ps);
                //LOG(2,"Toolbar main: 0x%08x: wm_paint: %d %d,%d,%d,%d",pThis->GethWnd(),ps.fErase,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right,ps.rcPaint.bottom);
                if (ps.fErase)
                {                    
                    pThis->PaintToolbar(pThis->GethWnd(),ps.hdc,NULL);                    
                }
                else
                {
                   pThis->PaintToolbar(pThis->GethWnd(),ps.hdc,&ps.rcPaint);
                }
                ::EndPaint(pThis->GethWnd(), &ps);
                return TRUE;
            }            
        }
        break;
    case WM_NCHITTEST:
        {
            //return ::DefWindowProc(pThis->GethWndParent(), WM_NCLBUTTONDOWN, HTCAPTION, lParam);
        }
        break;
    }
    return FALSE;
}



///////////////////////////////////////////////////////////////////////////////
// Toolbar child //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CToolbarChild::CToolbarChild(CToolbarWindow *pToolbar) :
hWnd(NULL),
m_PosX(0),
m_PosY(0),
m_PosW(10),
m_PosH(10),
m_Visible(0),
m_BgBrush(NULL),
m_hPen3DShadow(NULL),
m_hPen3DLight(NULL)
{
   m_pToolbar = pToolbar;
}


CToolbarChild::~CToolbarChild()
{
    int i;    

    for (i = 0; i < Buttons.size(); i++)
    {
        if (Buttons[i] != NULL)
        {
            delete Buttons[i];
        }
    }
    Buttons.clear();
    if (hWnd != NULL)
    {
        ::SetWindowLong(hWnd, GWL_USERDATA, NULL);
		DestroyWindow(hWnd);
        hWnd = NULL;
    }
}


BOOL CToolbarChild::SetPos(int x, int y, int w, int h, BOOL bUpdate)
{
    if ((m_PosX != x) || (m_PosY != y) || (m_PosW != w) || (m_PosH != h))
    {
        m_PosX = x;
        m_PosY = y;
        m_PosW = w;
        m_PosH = h;
        //return MoveWindow(hWnd,x,y,w,h,bUpdate);
		return SetWindowPos(hWnd, NULL, x,y,w,h, SWP_NOZORDER|SWP_NOACTIVATE);		
    }
    return TRUE;
}

BOOL CToolbarChild::GetPos(LPRECT lpRect)
{
    ::SetRect(lpRect, m_PosX, m_PosY, m_PosX+m_PosW, m_PosY+m_PosH);
    return TRUE;
}

int CToolbarChild::Width()
{
    RECT rc;
    GetWindowRect(hWnd, &rc);
    return rc.right-rc.left;
}

int CToolbarChild::Height()
{
    RECT rc;
    GetWindowRect(hWnd, &rc);
    return rc.bottom-rc.top;
}


int CToolbarChild::Visible()
{
	return m_Visible;
}

BOOL CToolbarChild::Show()
{
	if (hWnd != NULL)
	{
		UpdateWindow();
		if (!m_Visible)
		{			
			ShowWindow(hWnd, SW_SHOW);
			m_Visible = 1;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CToolbarChild::Hide()
{
	if (hWnd != NULL)
	{
		if (m_Visible)
		{						
			ShowWindow(hWnd, SW_HIDE);
			m_Visible = 0;
		}
		return TRUE;
	}
	return FALSE;
	
}


HWND CToolbarChild::Create(LPCTSTR szClassName, HINSTANCE hResourceInst)
{
    WNDCLASS wc;
    if (!::GetClassInfo(hResourceInst, szClassName, &wc) )
    {
        //wc.cbSize = sizeof(WNDCLASS);
        wc.style = CS_SAVEBITS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StaticToolbarChildProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hResourceInst; 
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = HBRUSH(NULL);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = szClassName;
        
        if (!RegisterClass(&wc))
        {
            return NULL;
        }        
    }

    MDICREATESTRUCT mdic;
	::memset(&mdic, 0, sizeof(mdic));
	mdic.lParam = (LPARAM)this;

    this->hResourceInst = hResourceInst;

    hWnd = CreateWindow(
      szClassName,      
      szClassName,
      WS_CHILD | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
      0,         // starting x position 
      0,         // starting y position 
      10,        // width 
      10,        // height 
      m_pToolbar->GethWnd(),  // parent window 
      NULL,        // No menu 
      hResourceInst,
      &mdic);      // pointer to structure with this   
    
   return hWnd;
}

HWND CToolbarChild::CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst)
{
    this->hResourceInst = hResourceInst;

    hWnd = ::CreateDialogParam(hResourceInst, 
                       lpTemplate,
                       m_pToolbar->GethWnd(), (DLGPROC)StaticToolbarChildDialogProc, LPARAM(this));        
    return hWnd;
}



BOOL CToolbarChild::SkinWindow(HWND hWndItem, string sID, string sIniEntry, eBitmapAsButtonType ButtonType, string sSection, string sIniFile, CBitmapCache *pBitmapCache)
{
    vector<string>States;
    if (ButtonType == BITMAPASBUTTON_PUSH)
    {
        States.push_back(sIniEntry);
        States.push_back(sIniEntry + "MouseOver");
        States.push_back(sIniEntry + "Click");  
    } 
	else if (ButtonType == BITMAPASBUTTON_CHECKBOX)
    {
        States.push_back(sIniEntry);
        States.push_back(sIniEntry + "MouseOver");
        States.push_back(sIniEntry + "Click");  
        States.push_back(sIniEntry + "Checked");
        States.push_back(sIniEntry + "CheckedMouseOver");
        States.push_back(sIniEntry + "CheckedClick");  
    } 
	else if (ButtonType == BITMAPASBUTTON_3STATE)
    {
        States.push_back(sIniEntry);
        States.push_back(sIniEntry + "MouseOver");
        States.push_back(sIniEntry + "Click");  
        States.push_back(sIniEntry + "1");
        States.push_back(sIniEntry + "1MouseOver");
        States.push_back(sIniEntry + "1Click");  
		States.push_back(sIniEntry + "2");
        States.push_back(sIniEntry + "2MouseOver");
        States.push_back(sIniEntry + "2Click");  
	}
	else if (ButtonType == BITMAPASBUTTON_4STATE)
    {
        States.push_back(sIniEntry);
        States.push_back(sIniEntry + "MouseOver");
        States.push_back(sIniEntry + "Click");  
        States.push_back(sIniEntry + "1");
        States.push_back(sIniEntry + "1MouseOver");
        States.push_back(sIniEntry + "1Click");  
		States.push_back(sIniEntry + "2");
        States.push_back(sIniEntry + "2MouseOver");
        States.push_back(sIniEntry + "2Click");  
		States.push_back(sIniEntry + "3");
        States.push_back(sIniEntry + "3MouseOver");
        States.push_back(sIniEntry + "3Click");  
    } 
	else if (ButtonType == BITMAPASBUTTON_SLIDER)
    {
        States.push_back(sIniEntry);
        States.push_back(sIniEntry + "MouseOver");
        States.push_back(sIniEntry + "Click");  
        States.push_back(sIniEntry + "Selected");
        States.push_back(sIniEntry + "SelectedMouseOver");
        States.push_back(sIniEntry + "SelectedClick");  
        States.push_back(sIniEntry + "Slider");
        States.push_back(sIniEntry + "SliderMouseOver");
        States.push_back(sIniEntry + "SliderClick");  
    }
    
    for (int i = 0 ; i < States.size(); i++)
    {
        BitmapsFromIniSection.Register(States[i]);   
    }
    BitmapsFromIniSection.Read(sIniFile, sSection, "Bitmap", "Mask", pBitmapCache);
    
    if ((hWndItem != NULL) && (BitmapsFromIniSection.Get(sIniEntry) != NULL))
    {
        CBitmapAsButton *Button = NULL;
		int i;
		int n;

		for (i = 0; i < Buttons.size(); i++)
		{
			if ((Buttons[i] != NULL) && (Buttons[i]->hWnd() == hWndItem))
			{
				Button = Buttons[i];
				break;
			}
		}
		n = i;
		if (Button == NULL)
		{
			Button = new CBitmapAsButton(ButtonType);
			Buttons.push_back(Button);
		}
        Button->SetProcessMessage(this, StaticToolbarChildButtonProc);

        for (i = 0 ; i < States.size(); i++)
        {
            Button->AddBitmap(i, BitmapsFromIniSection.Get(States[i]), BitmapsFromIniSection.GetMask(States[i]));
        }
        
        if (Button->TakeOver(hWndItem, sID.c_str(), hWnd))
        {            
            return TRUE;
        }
        else
        {
            delete Button;
			Buttons[n] = NULL;
            return FALSE;
        }                   
    }    
    return FALSE;   
}

BOOL CToolbarChild::RemoveSkin(string sID)
{
    vector<CBitmapAsButton*> NewList;
    
    for (int i = 0; i < Buttons.size(); i++)
    {
        if (Buttons[i] != NULL)
        {
            if (Buttons[i]->GetID()  == sID)
            {
                Buttons[i]->RestoreBack();
                delete Buttons[i];
                Buttons[i] = NULL;
            }
            else
            {
                NewList.push_back(Buttons[i]);
            }            
        }
    }
    Buttons = NewList;
    return TRUE;
}

BOOL CToolbarChild::SkinDlgItem(UINT uItemID, string sIniEntry, eBitmapAsButtonType ButtonType, string sSection, string sIniFile, CBitmapCache *pBitmapCache)
{
	HWND hWndItem = GetDlgItem(hWnd, uItemID); 
	char szID[20];
	sprintf(szID,"#%u", uItemID);
    
    return SkinWindow(hWndItem, szID, sIniEntry, ButtonType, sSection, sIniFile, pBitmapCache);
}

BOOL CToolbarChild::RemoveSkinDlgItem(UINT uItemID)
{
	char szID[20];
    sprintf(szID,"#%u", uItemID);

	return RemoveSkin(szID);
}

LRESULT CToolbarChild::ButtonChildProc(string sID, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
    char *szID = (char*)sID.c_str();    
    if (szID[0]=='#')
    {
        UINT winID = atoi(szID+1);
        if (winID!=0)
        {
            if (message == WM_LBUTTONUP)
            {
                SendMessage(hWndParent, WM_COMMAND, winID, 0);
            }
        }
    }
    return FALSE;
}

void CToolbarChild::DrawItem(DRAWITEMSTRUCT* pDrawItem, HICON hIcon, LPCSTR szText, int Width, int Height, int Align)
{
	if (pDrawItem == NULL)
	{
		return;
	}
	if (m_BgBrush == NULL)
	{
		m_BgBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	}
	if (m_hPen3DShadow == NULL)
	{
		m_hPen3DShadow = ::CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DSHADOW));
	}
	if (m_hPen3DLight == NULL)
	{
		m_hPen3DLight = ::CreatePen(PS_SOLID,1,GetSysColor(COLOR_3DLIGHT));
	}

	FillRect(pDrawItem->hDC, &pDrawItem->rcItem, m_BgBrush);
	
	if ((pDrawItem->CtlType == ODT_BUTTON) && ((pDrawItem->itemState&17)!=0))
	{
		//Draw sunken border

		LPRECT lpRect = &pDrawItem->rcItem;
		if ((m_hPen3DShadow != NULL) && (m_hPen3DLight!=NULL))
		{
			HPEN hPenOld = (HPEN)::SelectObject(pDrawItem->hDC, m_hPen3DShadow);					
				
			MoveToEx(pDrawItem->hDC, lpRect->left, lpRect->bottom-1, NULL);
			LineTo(pDrawItem->hDC, lpRect->left, lpRect->top);
			LineTo(pDrawItem->hDC, lpRect->right-1, lpRect->top);

			::SelectObject(pDrawItem->hDC, m_hPen3DLight);
			LineTo(pDrawItem->hDC, lpRect->right-1, lpRect->bottom-1);
			LineTo(pDrawItem->hDC, lpRect->left, lpRect->bottom-1);

			::SelectObject(pDrawItem->hDC, hPenOld);
		}
	}
	if (hIcon != NULL)
	{	
		int X = pDrawItem->rcItem.left+1;
		int Y = pDrawItem->rcItem.top+1;
		
		ICONINFO IconInfo;

		if (GetIconInfo(hIcon, &IconInfo)) //try to align icon
		{
			BITMAP bm;
			if (::GetObject (IconInfo.hbmColor, sizeof (bm), & bm))
			{
				int BmWidth =  bm.bmWidth;
				int BmHeight =  bm.bmHeight;
				if (Align & TOOLBARBUTTON_ICON_HALIGN_RIGHT)
				{	
					X = Width-BmWidth;
				}
				else if (Align & TOOLBARBUTTON_ICON_HALIGN_CENTER)
				{
					X = (Width-BmWidth)/2;
				}

				if (Align & TOOLBARBUTTON_ICON_VALIGN_BOTTOM)
				{	
					Y = Height-BmHeight;
				}
				else if (Align & TOOLBARBUTTON_ICON_VALIGN_CENTER)
				{
					Y = (Height-BmHeight)/2;
				}
				X += pDrawItem->rcItem.left;
				Y += pDrawItem->rcItem.top;
			}
		}

		if (pDrawItem->itemState & ODS_SELECTED) 
		{
			X++;
			Y++;
		} 
		
		::DrawIconEx(pDrawItem->hDC, X,Y,
					hIcon, 0,0, 0, NULL, DI_NORMAL);
	}
	if ((szText!=NULL) && (szText[0]!=0))
	{		
        int X = pDrawItem->rcItem.left+1;
		int Y = pDrawItem->rcItem.top+1;
		
		SIZE Size;
		if (GetTextExtentPoint(pDrawItem->hDC, szText, strlen(szText), &Size))
		{
			if (Align & TOOLBARBUTTON_TEXT_HALIGN_RIGHT)
			{
				X = Width-Size.cx;
			}
			else if (Align & TOOLBARBUTTON_TEXT_HALIGN_CENTER)
			{
				X = (Width-Size.cx)/2; 
			}
			if (Align & TOOLBARBUTTON_TEXT_VALIGN_BOTTOM)
			{
				Y = (Height-Size.cy);
			}
			else if (Align & TOOLBARBUTTON_TEXT_VALIGN_CENTER)
			{
				Y = (Height-Size.cy)/2;
			}
			X += pDrawItem->rcItem.left;
			Y += pDrawItem->rcItem.top;
		}
		
		if (pDrawItem->itemState & ODS_SELECTED) 
		{
			X++;
			Y++;
		}            
		
        int nMode = SetBkMode(pDrawItem->hDC, TRANSPARENT);        		
        if (pDrawItem->itemState & ODS_DISABLED)
		{
            DrawState(pDrawItem->hDC, (HBRUSH)NULL, NULL, (long)szText, 0, X,Y,Size.cx,Size.cy,DST_TEXT|DSS_DISABLED);
		}
        else
		{
            TextOut(pDrawItem->hDC, X, Y, szText, strlen(szText));
        }
        SetBkMode(pDrawItem->hDC, nMode);
	}
}

LRESULT CALLBACK CToolbarChild::StaticToolbarChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CToolbarChild *pThis = NULL;

	if (message == WM_GETDLGCODE)
	{
        return DLGC_WANTCHARS;
	}

    if ( message == WM_NCCREATE )
	{
		if (lParam == 0)
        {
            return FALSE;
        }
        MDICREATESTRUCT * pMDIC = (MDICREATESTRUCT *)((LPCREATESTRUCT) lParam)->lpCreateParams;
		CToolbarChild *pThis = (CToolbarChild*) (pMDIC->lParam);
		::SetWindowLong(hWnd, GWL_USERDATA, (LONG)pThis);        
        return TRUE;
	}
    
	pThis = (CToolbarChild*)::GetWindowLong(hWnd, GWL_USERDATA);    
    
    if (pThis != NULL)
    {
        return pThis->ToolbarChildProc(hWnd, message, wParam, lParam);
    }
    return FALSE;
}

LRESULT CALLBACK CToolbarChild::StaticToolbarChildDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{    
    if (message == WM_INITDIALOG)
    {
        ::SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);        
    }
            
    CToolbarChild* ToolbarChild = (CToolbarChild*)::GetWindowLong(hWnd, GWL_USERDATA);
    if (ToolbarChild != NULL)
    {
        LRESULT Result = ToolbarChild->ToolbarChildProc(hWnd, message, wParam, lParam);		
    }
    return FALSE;
}


LRESULT CToolbarChild::StaticToolbarChildButtonProc(string sID, void *pThis, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CToolbarChild* ToolbarChild = (CToolbarChild*)pThis;

    if (ToolbarChild!=NULL)
    {
        return ToolbarChild->ButtonChildProc(sID, hWndParent, MouseFlags, hDlg, message, wParam, lParam);
    }
    return FALSE;
}




