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

#if !defined(AFX_PAGE4_H__7E564D4C_E19B_434E_9638_552808A264FD__INCLUDED_)
#define AFX_PAGE4_H__7E564D4C_E19B_434E_9638_552808A264FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Page4.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CPage4 

class CPage4 : public CPropertyPage
{
    DECLARE_DYNCREATE(CPage4)

// Konstruktion
public:
    CPage4();
    ~CPage4();

// Dialogfelddaten
    //{{AFX_DATA(CPage4)
    enum { IDD = IDD_PAGE4 };
        // HINWEIS - Der Klassen-Assistent fügt hier Datenelemente ein.
        //    Innerhalb dieser generierten Quellcodeabschnitte NICHTS BEARBEITEN!
    //}}AFX_DATA


// Überschreibungen
    // Der Klassen-Assistent generiert virtuelle Funktionsüberschreibungen
    //{{AFX_VIRTUAL(CPage4)
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
    //}}AFX_VIRTUAL

// Implementierung
protected:
    // Generierte Nachrichtenzuordnungsfunktionen
    //{{AFX_MSG(CPage4)
        // HINWEIS: Der Klassen-Assistent fügt hier Member-Funktionen ein
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_PAGE4_H__7E564D4C_E19B_434E_9638_552808A264FD__INCLUDED_
