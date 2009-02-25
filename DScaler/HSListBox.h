/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jun 2001   Torbjörn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file HSListBox.h Header file for CHSListBox class.
 * @see CHSListBox
 */

#if !defined(AFX_HSLISTBOX_H__3CCC6FDD_B7F9_486D_A66D_98E0D8B92C0C__INCLUDED_)
#define AFX_HSLISTBOX_H__3CCC6FDD_B7F9_486D_A66D_98E0D8B92C0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/** CListBox derived class with a horizontal scrollbar.

    If you call any memberfunction that modifies the strings that is not implemented in this class
    the horizontal scrollbar might become out of sync with the text in it.

    It shoudn't be any problem implementing the other functions if we need it.
*/
class CHSListBox : public CListBox
{
// Construction
public:
    CHSListBox();

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CHSListBox)
    //}}AFX_VIRTUAL

// Implementation
public:
    void ResetContent( );
    virtual ~CHSListBox();
    int AddString(LPCTSTR lpszItem);
    int InsertString(int nIndex, LPCTSTR lpszItem);

// Generated message map functions
protected:
    int m_nMaxWidth;
    //{{AFX_MSG(CHSListBox)
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
private:

    /**
     * Calculates the size of a given string
     * @param lpszItem the string
     * @return size of the string
     */
    CSize GetTextSize(LPCTSTR lpszItem);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HSLISTBOX_H__3CCC6FDD_B7F9_486D_A66D_98E0D8B92C0C__INCLUDED_)
