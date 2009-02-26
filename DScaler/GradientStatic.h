/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
 * @file GradientStatic.h header file for CGradientStatic
 */

#if !defined(AFX_GRADIENTSTATIC_H__0709E3A1_C8B6_11D6_B74E_004033A0FB96__INCLUDED_)
#define AFX_GRADIENTSTATIC_H__0709E3A1_C8B6_11D6_B74E_004033A0FB96__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef UINT (CALLBACK* LPFNGRADIENTFILL)(HDC,CONST PTRIVERTEX,DWORD,CONST PVOID,DWORD,DWORD);

/**
 * CGradientStatic is a CStatic that has a customizable background gradient.
 * Use SetWindowText to change text and SetFont to change font
 * (both functions comes from CWnd).
 */
class CGradientStatic : public CStatic
{
public:
    CGradientStatic();
    virtual ~CGradientStatic();
    void SetGradientColor(COLORREF clLeft,COLORREF clRight);
    void SetTextColor(COLORREF cl);
    void SetSpacing(int iNoOfPixels);
    
// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGradientStatic)
    //}}AFX_VIRTUAL

// Implementation
public:
    

    // Generated message map functions
protected:
    //{{AFX_MSG(CGradientStatic)
    afx_msg void OnPaint();
    afx_msg void OnSysColorChange();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

private:
    void PaintRect(CDC& dc, int x, int y, int w, int h, COLORREF color);
    int m_iSpacing;
    
    long m_clLeft;
    long m_clRight;
    long m_clText;
    bool m_bUsingDefaultColors;

    HINSTANCE m_hMsimg32;
    LPFNGRADIENTFILL m_pfnGradientFill;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRADIENTSTATIC_H__0709E3A1_C8B6_11D6_B74E_004033A0FB96__INCLUDED_)
