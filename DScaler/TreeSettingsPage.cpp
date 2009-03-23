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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsPage dialog


CTreeSettingsPage::CTreeSettingsPage(CString TreeName,UINT nIDTemplate)
:CDialog(nIDTemplate, NULL),
m_dlgID(nIDTemplate),
m_name(TreeName),
m_HeaderName(TreeName),
m_minHeight(0),
m_minWidth(0),
m_bInitMinSize(TRUE)
{
    //{{AFX_DATA_INIT(CTreeSettingsPage)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

CTreeSettingsPage::CTreeSettingsPage(CString TreeName,CString HeaderName,UINT nIDTemplate)
:CDialog(nIDTemplate, NULL),
m_dlgID(nIDTemplate),
m_name(TreeName),
m_HeaderName(HeaderName),
m_minHeight(0),
m_minWidth(0),
m_bInitMinSize(TRUE)
{

}

void CTreeSettingsPage::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTreeSettingsPage)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTreeSettingsPage, CDialog)
    //{{AFX_MSG_MAP(CTreeSettingsPage)
        // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsPage message handlers

BOOL CTreeSettingsPage::PreTranslateMessage(MSG* pMsg)
{
    //dont allow escape key to be processed by the page
    //the escape key will normaly close the dialog
    if((pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_ESCAPE))
    {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CTreeSettingsPage::GetMinSize(int &width,int &height)
{
    //check if minimum size is initialized
    if(m_bInitMinSize && m_hWnd!=NULL)
    {
        //use current window width and height as minimum
        CRect rect;
        GetClientRect(&rect);
        m_minHeight=rect.Height();
        m_minWidth=rect.Width();
        m_bInitMinSize=FALSE;
    }
    width=m_minWidth;
    height=m_minHeight;
}

void CTreeSettingsPage::OnOK()
{
    EndDialog(IDOK);
}

void CTreeSettingsPage::OnCancel()
{
    EndDialog(IDCANCEL);
}

BOOL CTreeSettingsPage::OnKillActive()
{
    return TRUE;
}

BOOL CTreeSettingsPage::OnSetActive()
{
    return TRUE;
}

BOOL CTreeSettingsPage::OnQueryCancel()
{
    return TRUE;
}
