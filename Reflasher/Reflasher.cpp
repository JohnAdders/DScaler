/////////////////////////////////////////////////////////////////////////////
// $Id: Reflasher.cpp,v 1.1 2004-04-14 08:15:44 adcockj Exp $
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

#include "HardwareDriver.h"
#include "My2388xCard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReflasherApp

BEGIN_MESSAGE_MAP(CReflasherApp, CWinApp)
	//{{AFX_MSG_MAP(CReflasherApp)
		// HINWEIS - Hier werden Mapping-Makros vom Klassen-Assistenten eingefügt und entfernt.
		//    Innerhalb dieser generierten Quelltextabschnitte NICHTS VERÄNDERN!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReflasherApp Konstruktion

CReflasherApp::CReflasherApp()
{
	// ZU ERLEDIGEN: Hier Code zur Konstruktion einfügen
	// Alle wichtigen Initialisierungen in InitInstance platzieren
}

/////////////////////////////////////////////////////////////////////////////
// Das einzige CReflasherApp-Objekt

CReflasherApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CReflasherApp Initialisierung

BOOL CReflasherApp::InitInstance()
{
	// Standardinitialisierung
	// Wenn Sie diese Funktionen nicht nutzen und die Größe Ihrer fertigen 
	//  ausführbaren Datei reduzieren wollen, sollten Sie die nachfolgenden
	//  spezifischen Initialisierungsroutinen, die Sie nicht benötigen, entfernen.

#ifdef _AFXDLL
	Enable3dControls();			// Diese Funktion bei Verwendung von MFC in gemeinsam genutzten DLLs aufrufen
#else
	Enable3dControlsStatic();	// Diese Funktion bei statischen MFC-Anbindungen aufrufen
#endif
	
    CHardwareDriver* m_pDriver = new CHardwareDriver();
	if(m_pDriver->LoadDriver() == FALSE)
    {
        delete m_pDriver;
		m_pDriver = NULL;
        AfxMessageBox("Can't open driver", MB_OK);
        return -1;
    }
      
    DWORD	dwSubSystemId	= NULL;
    int		iDeviceIndex	= NULL;
	if(m_pDriver->DoesThisPCICardExist(0x14F1, 0x8800, iDeviceIndex, dwSubSystemId) == FALSE)
    {
        delete m_pDriver;
		m_pDriver = NULL;

        AfxMessageBox("Card not Found", MB_OK);
        return -1;
    }

	CMy2388xCard* m_pCard = new CMy2388xCard(m_pDriver);
	if (m_pCard->OpenPCICard(0x14F1, 0x8800, iDeviceIndex) == FALSE)
	{
        delete m_pCard;
		m_pCard = NULL;

        delete m_pDriver;
		m_pDriver = NULL;

        AfxMessageBox("Error at Open PCI-Card", MB_OK);
        return -1;
	}

	CWizard wizard("Reflasher");
	wizard.m_psh.hInstance = ::GetModuleHandle(NULL);
	wizard.m_pCard = m_pCard;
	
	m_pMainWnd = &wizard;
	wizard.DoModal();

	delete m_pCard;
    delete m_pDriver;

	return FALSE;
}
