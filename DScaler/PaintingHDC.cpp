/////////////////////////////////////////////////////////////////////////////
// $Id: PaintingHDC.cpp,v 1.2 2001-11-23 10:49:17 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Mike Temperton.  All rights reserved.
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
//  GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2001/11/22 13:19:37  temperton
// Added CPaintingHDC class for double buffering painting
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "PaintingHDC.h"

CPaintingHDC::CPaintingHDC()
    :
    m_hBufferDC(0),
    m_hOriginalDC(0),
    m_hSave(0),
    m_hBmp(0),
    m_bEnabled(true)
{
    ;
}

CPaintingHDC::~CPaintingHDC()
{
    if(m_hBufferDC)
    {
        DeleteObject(SelectObject(m_hBufferDC, m_hSave));
        DeleteDC(m_hBufferDC);
    }
}

HDC CPaintingHDC::BeginPaint(HDC hDC, LPRECT pRect)
{
    if(!m_bEnabled)
    {
        return hDC;
    }

    if(memcmp(pRect, &m_Rect, sizeof(RECT)) || !m_hBufferDC)
    {
        if(m_hBufferDC)
        {
            DeleteObject(SelectObject(m_hBufferDC, m_hSave));
            DeleteDC(m_hBufferDC);
        }

        m_hBufferDC = CreateCompatibleDC(hDC);
        m_hBmp = CreateCompatibleBitmap(hDC, pRect->right, pRect->bottom);
        m_hSave = SelectObject(m_hBufferDC, m_hBmp);
        
        m_Rect = *pRect;
    }

    m_hOriginalDC = hDC;
        
    return m_hBufferDC;
}

CPaintingHDC::EndPaint(void)
{
    if(m_hBufferDC && m_hOriginalDC)
    {        
        BitBlt(m_hOriginalDC, 
            m_Rect.left, 
            m_Rect.top,
            m_Rect.right - m_Rect.left,
            m_Rect.bottom - m_Rect.top,
            m_hBufferDC,
            m_Rect.left,
            m_Rect.top,
            SRCCOPY);
        
        m_hOriginalDC = NULL;
    }
}

void CPaintingHDC::SetEnabled(BOOL bEnabled)
{
    m_bEnabled = bEnabled;

    if(!m_bEnabled && m_hBufferDC)
    {
        DeleteObject(SelectObject(m_hBufferDC, m_hSave));
        DeleteDC(m_hBufferDC);

        m_hBufferDC = NULL;
    }
}