/////////////////////////////////////////////////////////////////////////////
// $Id: Page3.h,v 1.1 2004-04-14 08:15:44 adcockj Exp $
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

#if !defined(AFX_PAGE3_H__207FD57C_C768_41E0_957F_728B3FCF3C2C__INCLUDED_)
#define AFX_PAGE3_H__207FD57C_C768_41E0_957F_728B3FCF3C2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Page3.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CPage2 

class CPage3 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPage3)

// Konstruktion
public:
	CPage3();
	~CPage3();

// Dialogfelddaten
	//{{AFX_DATA(CPage3)
	enum { IDD = IDD_PAGE3 };
		// HINWEIS - Der Klassen-Assistent fügt hier Datenelemente ein.
		//    Innerhalb dieser generierten Quellcodeabschnitte NICHTS BEARBEITEN!
	//}}AFX_DATA


// Überschreibungen
	// Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CPage3)
	public:
	virtual BOOL OnSetActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CPage3)
	afx_msg void OnChangeEditYes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PAGE2_H__ADC6B1D0_AFFD_4F93_9BF0_AED9DB937987__INCLUDED_
