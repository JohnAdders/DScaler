//
// $Id: ToolbarWindow.cpp,v 1.1 2002-09-25 22:32:50 kooiman Exp $
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
    if (!::GetClassInfo(hInst, "TOOLBARWINDOW", &wc) )
    {
        //wc.cbSize = sizeof(WNDCLASS);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = ToolbarProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst; 
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = HBRUSH(NULL);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = "TOOLBARWINDOW";        
        
        if (!RegisterClass(&wc))
        {
            return;
        }        
    }

    MDICREATESTRUCT mdic;
	::memset(&mdic, 0, sizeof(mdic));
	mdic.lParam = (LPARAM)this;

    hWndToolbar = CreateWindow(
      "TOOLBARWINDOW",      
      "TOOLBARWINDOW",
      (Child?WS_CHILD:WS_POPUP) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
      0,         // starting x position 
      0,         // starting y position 
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

    ChildLeftRightMargin = 5;
    ChildTopBottomMargin = 5;
    
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
            //Doesn't work.
            //Is 'pure virtual function call', somehow fix this..
            //delete vChildList[i].pChild;
        }
    }
    vChildList.clear();
    if (hWndToolbar != NULL)
    {
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

BOOL CToolbarWindow::Show() 
{ 
   UpdateWindowPosition(hWndParent);
   for (int i = 0; i < vChildList.size(); i++)
   {
       if (vChildList[i].pChild != NULL)
       {
           ShowWindow(vChildList[i].pChild->GethWnd(), (vChildList[i].bShow) ? SW_SHOW : SW_HIDE);
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
    static int lastWindowPos_x = -1;
    static int lastWindowPos_y = -1;
    static int lastWindowPos_w = -1;
    static int lastWindowPos_h = -1;

    ///\todo implement rows

    if (bChildOrderChanged)
    {        
        bChildOrderChanged = FALSE;
        
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
        h = FitHeight;
    }

    if ((lastWindowPos_x != x) || (lastWindowPos_y != y) || (lastWindowPos_w != w) || (lastWindowPos_h != h))
    {
        SetWindowPos(hWndToolbar,NULL,x,y,w,h,SWP_NOACTIVATE|SWP_NOZORDER);
        lastWindowPos_x = x;
        lastWindowPos_y = y;
        lastWindowPos_w = w;
        lastWindowPos_h = h;
    }

    
    //Set all child windows
    
    int Width;
    int XLPos = LeftMargin;
    int XRPos = w - RightMargin;
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
            if (i<ChildOrderRightPos)
            {
                vChildList[i].pChild->SetPos(XLPos,TopMargin+dy, Width, vChildList[i].pChild->Height(), TRUE);
                XLPos += Width + ChildLeftRightMargin;
            } 
            else 
            {   
                XRPos -= Width + ChildLeftRightMargin;
                if (XRPos>=XLPos)
                {                    
                    vChildList[i].pChild->SetPos(XRPos,TopMargin+dy, Width, vChildList[i].pChild->Height(), TRUE);                
                }
                else
                {
                    //Overlap
                    XRPos=XLPos + ChildLeftRightMargin;
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
          return;
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
          int YPos = rc.bottom - Height;
          
          if ((rcBar.left != rc.left) || (rcBar.top != YPos) || (rcBar.right!=Width) || (rcBar.bottom != (YPos+Height)))
          {
              Height = -1;
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
    ::SetRect(&rcMiddle,LeftSize,TopSize,rcToolbar.right-RightSize,rcToolbar.bottom-BottomSize);
    
    if (hWnd == hWndParent)
    {        
        if (lpRect!=NULL)
        {
            RECT rcParent;            
            RECT rcInt;
            RECT rc2;

            GetClientRect(hWnd, &rcParent);            
            rc2.left= -rcParent.left + rcToolbar.left;
            rc2.top=  -rcParent.top  + rcToolbar.top;
            rc2.right=-rcParent.left + rcToolbar.right;
            rc2.bottom=-rcParent.top + rcToolbar.bottom;
            
            if (IntersectRect(&rcInt, &rc2, lpRect))
            {
                InvalidateRect(hWndToolbar, &rcInt, FALSE);
            }
            return;
        }        
        InvalidateRect(hWndToolbar, NULL, FALSE);
        return;
    }
    if (hWnd != hWndToolbar)
    {
        return;
    }
    
    // Border
    Paint(hWnd,hDC,lpRect, pPShift);
    
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
        if ((vChildList[i].pChild != NULL) && (vChildList[i].pChild->GethWnd() == hWndChild))
        {
            RECT rcInParent;
            RECT rcTotalInParent;
            RECT rcIntersection;
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
            return ::DefWindowProc(pThis->GethWndParent(), WM_NCLBUTTONDOWN, HTCAPTION, lParam);
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
m_PosH(10)
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
        return MoveWindow(hWnd,x,y,w,h,bUpdate);
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



HWND CToolbarChild::Create(LPCTSTR szClassName, HINSTANCE hResourceInst)
{
    WNDCLASS wc;
    if (!::GetClassInfo(hResourceInst, szClassName, &wc) )
    {
        //wc.cbSize = sizeof(WNDCLASS);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StaticToolbarChildProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hResourceInst; 
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = HBRUSH(NULL);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = "TOOLBARWINDOW";        
        
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
      WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
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

BOOL CToolbarChild::SkinDlgItem(UINT uItemID, string sIniEntry, eBitmapAsButtonType ButtonType, string sSection, string sIniFile)
{
    vector<string>States;
    if (ButtonType == BITMAPASBUTTON_PUSH)
    {
        States.push_back(sIniEntry);
        States.push_back(sIniEntry + "MouseOver");
        States.push_back(sIniEntry + "Click");  
    } else if (ButtonType == BITMAPASBUTTON_CHECKBOX)
    {
        States.push_back(sIniEntry);
        States.push_back(sIniEntry + "MouseOver");
        States.push_back(sIniEntry + "Click");  
        States.push_back(sIniEntry + "Checked");
        States.push_back(sIniEntry + "CheckedMouseOver");
        States.push_back(sIniEntry + "CheckedClick");  
    } else if (ButtonType == BITMAPASBUTTON_SLIDER)
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
    BitmapsFromIniSection.Read(sIniFile, sSection, "Bitmap", "Mask");
    
    HWND hWndItem = GetDlgItem(hWnd, uItemID);

    if ((hWndItem != NULL) && (BitmapsFromIniSection.Get(sIniEntry) != NULL))
    {
        CBitmapAsButton *Button = new CBitmapAsButton(ButtonType);
        Button->SetProcessMessage(this, StaticToolbarChildButtonProc);

        for (int i = 0 ; i < States.size(); i++)
        {
            Button->AddBitmap(i, BitmapsFromIniSection.Get(States[i]), BitmapsFromIniSection.GetMask(States[i]));
        }
        
        char szID[20];
        sprintf(szID,"#%u", uItemID);
        if (Button->TakeOver(hWndItem, szID, hWnd))
        {
            Buttons.push_back(Button);
            return TRUE;
        }
        else
        {
            delete Button;
            return FALSE;
        }                   
    }    
    return FALSE;   
}

BOOL CToolbarChild::RemoveSkinDlgItem(UINT uItemID)
{
    vector<CBitmapAsButton*> NewList;

    char szID[20];
    sprintf(szID,"#%u", uItemID);

    for (int i = 0; i < Buttons.size(); i++)
    {
        if (Buttons[i] != NULL)
        {
            if (Buttons[i]->GetID()  == szID)
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



LRESULT CALLBACK CToolbarChild::StaticToolbarChildProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CToolbarChild *pThis = NULL;

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
        return ToolbarChild->ToolbarChildProc(hWnd, message, wParam, lParam);
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


