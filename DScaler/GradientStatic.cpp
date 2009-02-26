/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
 * @file GradientStatic.cpp implementation of CGradientStatic
 */

#include "stdafx.h"
#include "GradientStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CGradientStatic::CGradientStatic()
:m_pfnGradientFill(NULL),
m_iSpacing(10),
m_hMsimg32(NULL),
m_bUsingDefaultColors(true)
{
    m_clLeft=GetSysColor(COLOR_ACTIVECAPTION);
    m_clRight=GetSysColor(COLOR_BTNFACE);
    m_clText=GetSysColor(COLOR_CAPTIONTEXT);

    //try to use msimg32.dll for drawing the gradient
    m_hMsimg32=LoadLibrary("msimg32.dll");
    if(m_hMsimg32)
    {
        m_pfnGradientFill=(LPFNGRADIENTFILL)GetProcAddress(m_hMsimg32, "GradientFill");
    }
}

CGradientStatic::~CGradientStatic()
{
    if(m_hMsimg32!=NULL)
    {
        FreeLibrary(m_hMsimg32);
        m_pfnGradientFill=NULL;
    }
}


BEGIN_MESSAGE_MAP(CGradientStatic, CStatic)
    //{{AFX_MSG_MAP(CGradientStatic)
    ON_WM_PAINT()
    ON_WM_SYSCOLORCHANGE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGradientStatic message handlers
void CGradientStatic::PaintRect(CDC& dc,int x,int y,int w,int h,COLORREF color)
{
    CBrush brush(color);
    CBrush* pOldBrush=dc.SelectObject(&brush);
    dc.PatBlt(x,y,w,h,PATCOPY);
    dc.SelectObject(pOldBrush);
}

void CGradientStatic::OnPaint() 
{
    CPaintDC dc(this); // device context for painting

    CRect rect;
    GetClientRect(&rect);

    if(m_pfnGradientFill!=NULL)
    {
        TRIVERTEX rcVertex[2];
        rect.right--; // exclude this point, like FillRect does 
        rect.bottom--;
        rcVertex[0].x=rect.left;
        rcVertex[0].y=rect.top;
        rcVertex[0].Red=GetRValue(m_clLeft)<<8;    // color values from 0x0000 to 0xff00 !!!!
        rcVertex[0].Green=GetGValue(m_clLeft)<<8;
        rcVertex[0].Blue=GetBValue(m_clLeft)<<8;
        rcVertex[0].Alpha=0x0000;
        rcVertex[1].x=rect.right; 
        rcVertex[1].y=rect.bottom;
        rcVertex[1].Red=GetRValue(m_clRight)<<8;
        rcVertex[1].Green=GetGValue(m_clRight)<<8;
        rcVertex[1].Blue=GetBValue(m_clRight)<<8;
        rcVertex[1].Alpha=0;
        GRADIENT_RECT rect;
        rect.UpperLeft=0;
        rect.LowerRight=1;
        
        // fill the area 
        m_pfnGradientFill(dc,rcVertex,2,&rect,1,GRADIENT_FILL_RECT_H);
    }
    else
    {
        // Get the intensity values for the ending color
        int r1=GetRValue(m_clRight); // red
        int g1=GetGValue(m_clRight); // green
        int b1=GetBValue(m_clRight); // blue
        
        // Get the intensity values for the begining color
        int r2=GetRValue(m_clLeft); // red
        int g2=GetGValue(m_clLeft); // green
        int b2=GetBValue(m_clLeft); // blue
        
        int x=rect.right;
        int w=x; // width of area to shade
        int xDelta= max(w/255,1); // width of one shade band
        
        // Paint far right 1/6 of caption the background color
        PaintRect(dc,x,0,rect.right-x,rect.bottom,m_clRight);
        
        int r;
        int g;
        int b;
        while(x>xDelta)
        {
            x-=xDelta;
            if(r1>r2)
            {
                r=r1-(r1-r2)*(w-x)/w;
            }
            else
            {
                r=r1+(r2-r1)*(w-x)/w;
            }
            
            if(g1>g2)
            {
                g=g1-(g1-g2)*(w-x)/w;
            }
            else
            {
                g=g1+(g2-g1)*(w-x)/w;
            }
            
            if(b1>b2)
            {
                b=b1-(b1-b2)*(w-x)/w;
            }
            else
            {
                b=b1+(b2-b1)*(w-x)/w;
            }
            
            // Paint bands right to left
            PaintRect(dc, x, 0, xDelta, rect.bottom, RGB(r, g, b));
        }
    }
    
    dc.SetTextColor(m_clText);
    dc.SetBkMode(TRANSPARENT);
    
    CFont *pFont=GetFont();
    CFont *pOldFont=dc.SelectObject(pFont);
    
    GetClientRect(&rect);
    
    CString wndText;
    GetWindowText(wndText);
    
    //convert CStatic flags to DrawText flags
    //if vertical centering is on, then we can only draw one line of text 
    //or it will not be centered at all.
    UINT DrawFlags=(GetWindowLong(m_hWnd,GWL_STYLE)&SS_CENTERIMAGE) ? DT_VCENTER|DT_SINGLELINE : 0;
    if(GetWindowLong(m_hWnd,GWL_STYLE)&SS_CENTER)
    {
        DrawFlags|=DT_CENTER;
    }
    else if(GetWindowLong(m_hWnd,GWL_STYLE)&SS_RIGHT)
    {
        rect.right-=m_iSpacing;
        DrawFlags|=DT_RIGHT;
    }
    else
    {
        rect.left+=m_iSpacing;
        DrawFlags|=DT_LEFT;
    }
    //three dots at the end of the text if it is too large
    DrawFlags|=DT_END_ELLIPSIS;

    dc.DrawText(wndText,&rect,DrawFlags);
    
    if(pOldFont!=NULL)
    {
        dc.SelectObject(pOldFont);
    }
}

void CGradientStatic::SetGradientColor(COLORREF clLeft,COLORREF clRight)
{
    m_clLeft=clLeft;
    m_clRight=clRight;
    m_bUsingDefaultColors=false;
}

void CGradientStatic::SetTextColor(COLORREF cl)
{
    m_clText=cl;
    m_bUsingDefaultColors=false;
}

void CGradientStatic::OnSysColorChange()
{
    CStatic::OnSysColorChange();
    
    //track changes in system colors so that the gradient folows windows colors
    if(m_bUsingDefaultColors)
    {
        m_clLeft=GetSysColor(COLOR_ACTIVECAPTION);
        m_clRight=GetSysColor(COLOR_BTNFACE);
        m_clText=GetSysColor(COLOR_CAPTIONTEXT);
    }
}

void CGradientStatic::SetSpacing(int iNoOfPixels)
{
    m_iSpacing=iNoOfPixels;
}

