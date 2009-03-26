/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 1998-2001 Mark Findlay.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file Credits.h  Credits Header file
 */

#if !defined(__CREDITS_H_)
#define __CREDITS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\DScalerRes\resource.h"

/** Credits Dialog
    Creates a scolling credit screen
*/
class CCredits : public CDialog
{
// Construction
public:
    CCredits(CWnd* pParent = NULL);   // standard constructor

    #define     DISPLAY_TIMER_ID        150        // timer id

     RECT        m_ScrollRect,r;           // rect of Static Text frame
    int         nArrIndex,nCounter;           // work ints
    CString     m_szWork;               // holds display line
    BOOL        m_bFirstTime;
    BOOL        m_bDrawText;
    int         nClip;
    int         nCurrentFontHeight;

    CWnd*       m_pDisplayFrame;

    CBitmap     m_bmpWork;                  // bitmap holder
    CBitmap*     pBmpOld;                    // other bitmap work members
    CBitmap*    m_bmpCurrent;
    HBITMAP     m_hBmpOld;

    CSize         m_size;                     // drawing helpers
    CPoint         m_pt;
    BITMAP         m_bmpInfo;
    CDC         m_dcMem;
    BOOL         m_bProcessingBitmap;

// Dialog Data
    //{{AFX_DATA(CCredits)
    enum { IDD = IDD_CREDITS };
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CCredits)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CCredits)
    virtual void OnOK();
    afx_msg void OnPaint();
    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnDestroy();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

#endif