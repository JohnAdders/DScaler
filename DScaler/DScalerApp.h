/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
 * @file DScalerApp.h DScalerMFC Header
 */

#if !defined(AFX_DSCALER_H__8ACFC662_CD94_4C47_A4C9_E1C7B7A52446__INCLUDED_)
#define AFX_DSCALER_H__8ACFC662_CD94_4C47_A4C9_E1C7B7A52446__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

/*! \mainpage DScaler Developer Help

    \section intro Introduction

    This file has been produced from the comments in the source code.
    It is hoped that this will aid in the understanding of how DScaler has
    been built and aid new developers to get up to speed quickly.

    If you find any problems in this file please post bug reports to
    johnadcock@hotmail.com

    \section arch DScaler Archetecture

    DScaler is curretly in a transition phase moving away from it's c background
    to a more object based approach.  For this reason you will see that this documentation
    tends to cover the newer functions.

    New users should firstly try and understand what is going on at the heart of DScaler by
    examining the OutThreads.cpp code in particular the YUVOutThread function.

*/

/** MFC class for DScaler.  This is defines so that we can use MFC dialogs
*/
class CDScalerApp : public CWinApp
{
public:
    CDScalerApp();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDScalerApp)
    public:
    virtual BOOL InitInstance();
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
    //}}AFX_VIRTUAL

// Implementation

    //{{AFX_MSG(CDScalerApp)
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSCALER_H__8ACFC662_CD94_4C47_A4C9_E1C7B7A52446__INCLUDED_)
