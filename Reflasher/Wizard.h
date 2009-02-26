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

#if !defined(AFX_WIZARD_H__D5250C8E_BC21_49FC_A006_BB0DDD671D72__INCLUDED_)
#define AFX_WIZARD_H__D5250C8E_BC21_49FC_A006_BB0DDD671D72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Wizard.h : Header-Datei
//

#include "Page1.h"
#include "Page2.h"
#include "Page3.h"
#include "Page4.h"

#include "My2388xCard.h"

/////////////////////////////////////////////////////////////////////////////
// CWizard

class CWizard : public CPropertySheet
{
    DECLARE_DYNAMIC(CWizard)

// Konstruktion
public:
    CWizard(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CWizard(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attribute
public:
    CMy2388xCard* m_pCard;

private:
    void    InitWizard();
    CPage1    m_Page1;
    CPage2    m_Page2;
    CPage3    m_Page3;
    CPage4    m_Page4;

// Operationen
public:

// Überschreibungen
    // Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
    //{{AFX_VIRTUAL(CWizard)
    //}}AFX_VIRTUAL

// Implementierung
public:
    DWORD m_dwNewValue;
    virtual ~CWizard();

    // Generierte Nachrichtenzuordnungsfunktionen
protected:
    //{{AFX_MSG(CWizard)
        // HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_WIZARD_H__D5250C8E_BC21_49FC_A006_BB0DDD671D72__INCLUDED_
