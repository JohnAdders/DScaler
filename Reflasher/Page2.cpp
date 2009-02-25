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
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "reflasher.h"
#include "Page2.h"
#include "Wizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CPage2 

IMPLEMENT_DYNCREATE(CPage2, CPropertyPage)

CPage2::CPage2() : CPropertyPage(CPage2::IDD)
{
	//{{AFX_DATA_INIT(CPage2)
	m_iPalNtsc = -1;
	//}}AFX_DATA_INIT
	m_psp.dwFlags &= ~PSP_HASHELP; 
}

CPage2::~CPage2()
{
}

void CPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPage2)
	DDX_Radio(pDX, IDC_RADIO_PAL, m_iPalNtsc);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CPage2)
	ON_BN_CLICKED(IDC_RADIO_NTSC, OnRadioNtsc)
	ON_BN_CLICKED(IDC_RADIO_PAL, OnRadioPal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CPage2 

#define F0_REVID_CLASS_MM 0x002f0008

BOOL CPage2::OnSetActive() 
{
	// try to autodetect the chip type
	if(m_iPalNtsc == -1)
	{
		CWizard* parent = (CWizard*)GetParent();
		DWORD dwTemp = parent->m_pCard->ReadDword(F0_REVID_CLASS_MM);

		switch (dwTemp)
		{
		case 0x04000005:	// CX23883-19
			m_iPalNtsc = 1;
			break;

		
		case 0x04000003:	// CX23881-27
			m_iPalNtsc = 0;
			break;
		}

		UpdateData(FALSE);
	}
	
	EnableButtons();	
	return CPropertyPage::OnSetActive();
}

void CPage2::OnRadioNtsc() 
{
	UpdateData(TRUE);
	EnableButtons();	
}

void CPage2::OnRadioPal() 
{
	UpdateData(TRUE);
	EnableButtons();	
}

void CPage2::EnableButtons()
{
	CPropertySheet* parent = (CPropertySheet*)GetParent();
	
	m_iPalNtsc == -1 ?
		parent->SetWizardButtons(PSWIZB_BACK) :
		parent->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);
}

LRESULT CPage2::OnWizardNext() 
{
	CWizard* parent = (CWizard*)GetParent();

	if(m_iPalNtsc == 0)
	{
		// for PAL
		parent->m_dwNewValue = 0x014d5349;
	}

	else if(m_iPalNtsc == 1)
	{
		// for NTSC
		parent->m_dwNewValue = 0x004d5349;
	}
	
	return CPropertyPage::OnWizardNext();
}
