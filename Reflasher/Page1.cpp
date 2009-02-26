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
#include "Reflasher.h"
#include "Page1.h"
#include "Wizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CPage1 

IMPLEMENT_DYNCREATE(CPage1, CPropertyPage)

CPage1::CPage1() : CPropertyPage(CPage1::IDD)
{
    //{{AFX_DATA_INIT(CPage1)
        // HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
    //}}AFX_DATA_INIT

    m_psp.dwFlags &= ~PSP_HASHELP; 
}

CPage1::~CPage1()
{
}

void CPage1::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CPage1)
        // HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPage1, CPropertyPage)
    //{{AFX_MSG_MAP(CPage1)
        // HINWEIS: Der Klassen-Assistent fügt hier Zuordnungsmakros für Nachrichten ein
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CPage1 

BOOL CPage1::OnSetActive() 
{
    BOOL bEnableNextButton = FALSE;

    if(IsAnMsiCard())
    {
        GetDlgItem(IDC_STATIC_CARD)->SetWindowText("MSI Master Card found.");

        if(IsEepromCorrupted())
        {
            GetDlgItem(IDC_STATIC_EEPROM)->SetWindowText("EEPROM is corrupted.");
            bEnableNextButton = TRUE;
        }

        else 
        {
            GetDlgItem(IDC_STATIC_EEPROM)->SetWindowText("EEPROM is not corrupted.");
        }
    }

    else
    {
        GetDlgItem(IDC_STATIC_CARD)->SetWindowText("MSI Master Card not found.");
    }
    
    CPropertySheet* parent = (CPropertySheet*)GetParent();

    bEnableNextButton == TRUE ?
        parent->SetWizardButtons(PSWIZB_NEXT) :
        parent->SetWizardButtons(NULL);

    return CPropertyPage::OnSetActive();
}


BOOL CPage1::IsAnMsiCard()
{
    CWizard* parent = (CWizard*)GetParent();
    return parent->m_pCard->ReadDword(0x00365000 + 4) == 0x86061462;
    //return parent->m_pCard->ReadDword(0x00365000 + 4) == 0x34010070;
}

BOOL CPage1::IsEepromCorrupted()
{
    BOOL bReturn = TRUE;

    CWizard* parent = (CWizard*)GetParent();
    DWORD dwTemp1 = parent->m_pCard->ReadDword(0x00365000 + 8);
    DWORD dwTemp2 = parent->m_pCard->ReadDword(0x00365000 + 12);

    if(    (dwTemp1 == 0x014d5349) || (dwTemp1 == 0x004d5349))
    {
        if(dwTemp2 == 0xFFFFFFFF)
        {
            bReturn = FALSE;
        }
    }

    return bReturn;
}
