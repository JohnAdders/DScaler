/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth.  All rights reserved.
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

#include "stdafx.h"
#include "reflasher.h"
#include "Page3.h"
#include "Wizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CPage3 

IMPLEMENT_DYNCREATE(CPage3, CPropertyPage)

CPage3::CPage3() : CPropertyPage(CPage3::IDD)
{
    //{{AFX_DATA_INIT(CPage3)
        // HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
    //}}AFX_DATA_INIT
    m_psp.dwFlags &= ~PSP_HASHELP; 
}

CPage3::~CPage3()
{
}

void CPage3::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPage3)
        // HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPage3, CPropertyPage)
    //{{AFX_MSG_MAP(CPage3)
    ON_EN_CHANGE(IDC_EDIT_YES, OnChangeEditYes)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CPage3 

BOOL CPage3::OnSetActive() 
{
    GetDlgItem(IDC_EDIT_YES)->SetWindowText("");    
    
    CPropertySheet* parent = (CPropertySheet*)GetParent();
    parent->SetWizardButtons(PSWIZB_BACK);
    
    return CPropertyPage::OnSetActive();
}

void CPage3::OnChangeEditYes() 
{
    CString strTemp;
    GetDlgItem(IDC_EDIT_YES)->GetWindowText(strTemp);
    
    CPropertySheet* parent = (CPropertySheet*)GetParent();

    strTemp.Compare("YES") == 0 ?
        parent->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT):
        parent->SetWizardButtons(PSWIZB_BACK);
}

LRESULT CPage3::OnWizardNext() 
{
    CWizard* parent = (CWizard*)GetParent();

    parent->m_pCard->WriteDword(0x00365000 + 8, parent->m_dwNewValue);
    Sleep(50);// give I2C bus some time to reflash the eeprom...

    parent->m_pCard->WriteDword(0x00365000 + 12, 0xFFFFFFFF);
    Sleep(50);// give I2C bus some time to reflash the eeprom...

    return CPropertyPage::OnWizardNext();
}
