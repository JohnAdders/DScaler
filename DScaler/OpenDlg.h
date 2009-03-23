/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Torbjörn Jansson.  All rights reserved.
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
 * @file OpenDlg.h header file for COpenDlg 
 */

#if !defined(AFX_OPENDLG_H__FBCD6DFF_AB97_4632_A1FD_E93DE3A1FAE5__INCLUDED_)
#define AFX_OPENDLG_H__FBCD6DFF_AB97_4632_A1FD_E93DE3A1FAE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/**
 * This class is a simple open dialog similar to how windows run command and
 * mediaplayer open dialog works.
 */
class COpenDlg : public CDialog
{
// Construction
public:
    COpenDlg(CWnd* pParent = NULL);   // standard constructor
    
    /**
     * Creates a open dialog and shows it.
     * @param hParent parent window
     * @param FileName filename of selected file if returned TRUE
     * @return TRUE if a file was selected and closed with ok button
     */
    static BOOL ShowOpenDialog(HWND hParent,CString &FileName);

// Dialog Data
    //{{AFX_DATA(COpenDlg)
    enum { IDD = IDD_OPEN };
    CEdit    m_File;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COpenDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL
    
// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(COpenDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnBrowse();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void OnOK();
    CString m_FileName;
private:
    /**
     * Makes the edit box auto complete.
     * This will only work with internet explorer 5 or later installed,
     * if it is not installed this function will do nothing.
     */
    void SetupAutoComplete();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPENDLG_H__FBCD6DFF_AB97_4632_A1FD_E93DE3A1FAE5__INCLUDED_)
