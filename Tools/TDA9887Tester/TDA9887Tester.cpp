/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth. All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TDA9887Tester.h"
#include "TDA9887TesterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTDA9887TesterApp

BEGIN_MESSAGE_MAP(CTDA9887TesterApp, CWinApp)
	//{{AFX_MSG_MAP(CTDA9887TesterApp)
		// HINWEIS - Hier werden Mapping-Makros vom Klassen-Assistenten eingefügt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VERÄNDERN!
	//}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTDA9887TesterApp Konstruktion

CTDA9887TesterApp::CTDA9887TesterApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// Das einzige CTDA9887TesterApp-Objekt

CTDA9887TesterApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTDA9887TesterApp Initialisierung

BOOL CTDA9887TesterApp::InitInstance()
{
    CHardwareDriver* pDriver = new CHardwareDriver();

	if(pDriver->LoadDriver() == FALSE)
    {
        delete pDriver;
        AfxMessageBox("Can't open driver", MB_OK);
        return FALSE;
    }
      
	CTDA9887TesterDlg dlg(pDriver);
	m_pMainWnd = &dlg;
	dlg.DoModal();

	delete pDriver;
	return FALSE;
}

