/////////////////////////////////////////////////////////////////////////////
// $Id: Wizard.cpp,v 1.1 2004-04-14 08:15:44 adcockj Exp $
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
#include "Reflasher.h"
#include "Wizard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWizard

IMPLEMENT_DYNAMIC(CWizard, CPropertySheet)

CWizard::CWizard(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	InitWizard();
}

CWizard::CWizard(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	InitWizard();
}

CWizard::~CWizard()
{
}

BEGIN_MESSAGE_MAP(CWizard, CPropertySheet)
	//{{AFX_MSG_MAP(CWizard)
		// HINWEIS - Der Klassen-Assistent fügt hier Zuordnungsmakros ein und entfernt diese.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CWizard 

void CWizard::InitWizard()
{
	m_pCard	= NULL;
	m_dwNewValue = 0;
	
	AddPage(&m_Page1);
	AddPage(&m_Page2);
	AddPage(&m_Page3);
	AddPage(&m_Page4);
	
	m_psh.dwFlags &= ~PSH_HASHELP;
	SetWizardMode();
}
