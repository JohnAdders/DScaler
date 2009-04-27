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
 * @file TreeSettingsPage.cpp implementation file for CTreeSettingsPage
 */

#include "stdafx.h"
#include "TreeSettingsPage.h"
#include "DScaler.h"

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsPage dialog
using namespace std;

CTreeSettingsPage::CTreeSettingsPage(const string& TreeName, UINT nIDTemplate): 
    CDSDialog(MAKEINTRESOURCE(nIDTemplate)),
    m_name(TreeName),
    m_HeaderName(TreeName),
    m_minHeight(0),
    m_minWidth(0),
    m_bInitMinSize(TRUE)
{
}

CTreeSettingsPage::CTreeSettingsPage(const string& TreeName,const string& HeaderName,UINT nIDTemplate) :
            CDSDialog(MAKEINTRESOURCE(nIDTemplate)),
            m_name(TreeName),
            m_HeaderName(HeaderName),
            m_minHeight(0),
            m_minWidth(0),
            m_bInitMinSize(TRUE)
{
}

CTreeSettingsPage::~CTreeSettingsPage()
{
}

BOOL CTreeSettingsPage::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    //dont allow escape key to be processed by the page
    //the escape key will normaly close the dialog
    if((message==WM_KEYDOWN) && (wParam==VK_ESCAPE))
    {
        return TRUE;
    }

    return ChildDialogProc(hDlg, message, wParam, lParam);
}


void CTreeSettingsPage::GetMinSize(HWND hWnd, int& width,int& height)
{
    //check if minimum size is initialized
    if(m_bInitMinSize && hWnd != NULL)
    {
        //use current window width and height as minimum
        RECT rect;
        GetClientRect(hWnd, &rect);
        m_minHeight=rect.bottom - rect.top;
        m_minWidth=rect.right - rect.left;
        m_bInitMinSize=FALSE;
    }
    width=m_minWidth;
    height=m_minHeight;
}

void CTreeSettingsPage::OnOK(HWND hDlg)
{
    EndDialog(hDlg, IDOK);
}

void CTreeSettingsPage::OnCancel(HWND hDlg)
{
    EndDialog(hDlg, IDCANCEL);
}

BOOL CTreeSettingsPage::OnKillActive(HWND hDlg)
{
    return TRUE;
}

BOOL CTreeSettingsPage::OnSetActive(HWND hDlg)
{
    return TRUE;
}

BOOL CTreeSettingsPage::ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}
