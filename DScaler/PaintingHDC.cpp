/////////////////////////////////////////////////////////////////////////////
// $Id: PaintingHDC.cpp,v 1.4 2003-02-27 01:28:03 atnak Exp $
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
// Revision 1.3  2003/01/24 01:55:17  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.2  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.1  2001/11/22 13:19:37  temperton
// Added CPaintingHDC class for double buffering painting
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PaintingHDC.h"


CPaintingHDC::CPaintingHDC()
{
    m_hBufferDC = NULL;
    m_hOriginalDC = NULL;
    m_hSaveBmp = NULL;
    m_hBmp = NULL;
}


CPaintingHDC::~CPaintingHDC()
{
    if (m_hBufferDC != NULL)
    {
        DeleteObject(SelectObject(m_hBufferDC, m_hSaveBmp));
        DeleteDC(m_hBufferDC);
    }
}


HDC CPaintingHDC::BeginPaint(HDC hDC, LPRECT pRect)
{
    UpdateGeometry(hDC, pRect);
    CopyRect(&m_PaintRect, pRect);
    return m_hBufferDC;
}


void CPaintingHDC::EndPaint()
{
    BitBltRects(&m_PaintRect, 1);
    m_hOriginalDC = NULL;
}


HDC CPaintingHDC::GetBufferDC()
{
    return m_hBufferDC;
}


BOOL CPaintingHDC::UpdateGeometry(HDC hDC, LPRECT pRect, BOOL bBufferTrim)
{
    BOOL bRecreated = FALSE;

    if (m_hBufferDC != NULL)
    {
        if ((bBufferTrim != FALSE && (m_Rect.right != pRect->right ||
            m_Rect.bottom != pRect->bottom)) || (bBufferTrim == FALSE &&
            (m_Rect.right < pRect->right || m_Rect.bottom < pRect->bottom)))
        {
            DeleteObject(SelectObject(m_hBufferDC, m_hSaveBmp));
            DeleteDC(m_hBufferDC);
            m_hBufferDC = NULL;
        }
    }

    if (m_hBufferDC == NULL)
    {
        m_hBufferDC = CreateCompatibleDC(hDC);

        if (m_hBufferDC != NULL)
        {
            m_hBmp = CreateCompatibleBitmap(hDC, pRect->right, pRect->bottom);

            if (m_hBmp != NULL)
            {
                m_hSaveBmp = SelectObject(m_hBufferDC, m_hBmp);

                if (m_hSaveBmp == NULL)
                {
                    DeleteObject(m_hBmp);
                    m_hBmp = NULL;
                }
            }

            if (m_hBmp == NULL)
            {
                DeleteDC(m_hBufferDC);
                m_hBufferDC = NULL;
            }
            else
            {
                m_Rect.right = pRect->right;
                m_Rect.bottom = pRect->bottom;
                bRecreated = TRUE;
            }
        }
    }

    if (m_hBufferDC != NULL)
    {
        m_hOriginalDC = hDC;

        m_Rect.left = pRect->left;
        m_Rect.top = pRect->top;
    }

    return bRecreated;
}


void CPaintingHDC::BitBltRects(LPRECT pRectList, LONG nRectCount, HDC hDstDC)
{
    if (m_hBufferDC == NULL)
    {
        return;
    }

    if (hDstDC == NULL)
    {
        hDstDC = m_hOriginalDC;
    }

    if (hDstDC == NULL)
    {
        return;
    }

    for (int i = 0; i < nRectCount; i++)
    {
        BitBlt(hDstDC, pRectList[i].left, pRectList[i].top,
            pRectList[i].right - pRectList[i].left,
            pRectList[i].bottom - pRectList[i].top,
            m_hBufferDC, pRectList[i].left, pRectList[i].top, SRCCOPY);
    }
}

