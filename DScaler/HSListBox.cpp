/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
 * @file HSListBox.cpp implementation of the CHSListBox class.
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "HSListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHSListBox

CHSListBox::CHSListBox()
:m_nMaxWidth(0)
{
}

CHSListBox::~CHSListBox()
{
}

int CHSListBox::AddString(LPCTSTR lpszItem)
{
    int nRet = CListBox::AddString(lpszItem);

    //
    SCROLLINFO scrollInfo;
    memset(&scrollInfo, 0, sizeof(SCROLLINFO));
    scrollInfo.cbSize = sizeof(SCROLLINFO);
    scrollInfo.fMask  = SIF_ALL;
    GetScrollInfo(SB_VERT, &scrollInfo, SIF_ALL);

    //is the vertical scrollbar visibel?
    int nScrollWidth = 0;
    if(GetCount() > 1 && ((int)scrollInfo.nMax >= (int)scrollInfo.nPage))
    {
        nScrollWidth = GetSystemMetrics(SM_CXVSCROLL);
    }
    if(nRet!=LB_ERR && nRet!=LB_ERRSPACE)
    {
        m_nMaxWidth = max(m_nMaxWidth, GetTextSize(lpszItem).cx);
        SetHorizontalExtent(m_nMaxWidth + 4);
    }

    return nRet;
}
int CHSListBox::InsertString(int nIndex, LPCTSTR lpszItem)
{
    int nRet=CListBox::InsertString(nIndex,lpszItem);
    
    if(nRet!=LB_ERR && nRet!=LB_ERRSPACE)
    {
        m_nMaxWidth = max(m_nMaxWidth, GetTextSize(lpszItem).cx);
        SetHorizontalExtent(m_nMaxWidth + 4);
    }

    return nRet;
}
void CHSListBox::ResetContent()
{
    CListBox::ResetContent();
    m_nMaxWidth=0;
    SetHorizontalExtent(0);
}


BEGIN_MESSAGE_MAP(CHSListBox, CListBox)
    //{{AFX_MSG_MAP(CHSListBox)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHSListBox message handlers


CSize CHSListBox::GetTextSize(LPCTSTR lpszItem)
{
    CSize sSize;
    CClientDC myDC(this);

    CFont* pListBoxFont = GetFont();
    if(pListBoxFont != NULL)
    {
        CFont* pOldFont = myDC.SelectObject(pListBoxFont);
        GetTextExtentPoint32(myDC.m_hDC, lpszItem, strlen(lpszItem), &sSize);
        myDC.SelectObject(pOldFont);
    }
    return sSize;
}
