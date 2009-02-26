/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSOPTIONSDLG_H__C76FD300_76CF_11D5_A1E2_AD9180F1D520__INCLUDED_)
#define AFX_TSOPTIONSDLG_H__C76FD300_76CF_11D5_A1E2_AD9180F1D520__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TimeShift.h"

/** Holds all of the configuration variables for the TimeShift options */
typedef struct
{
    int        recHeight;  /**< Equal to a TS_* height constant */
    tsFormat_t format;     /**< A FORMAT_* constant */
    DWORD      sizeLimit;  /**< The maximum size of each recorded file (0 = no limit) */
    char       path[MAX_PATH + 1];
    FOURCC     fcc;        /**< The FOURCC of the video codec to use */
} TS_OPTIONS;

/** TimeShift options dialog */
class CTSOptionsDlg : public CDialog
{

public:
// Construction
    CTSOptionsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
    //{{AFX_DATA(CTSOptionsDlg)
    enum { IDD = IDD_TSOPTIONS };
    CEdit    m_PathDisplay;
    CStatic    m_SizeGiB;
    CButton    m_SizeCheckBox;
    CEdit    m_SizeEdit;
    CComboBox    m_WaveOutComboBox;
    CComboBox    m_WaveInComboBox;
    //}}AFX_DATA

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTSOptionsDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CTSOptionsDlg)
    afx_msg void OnButtonCompression();
    virtual BOOL OnInitDialog();
    afx_msg void OnCompressionhelp();
    afx_msg void OnWavehelp();
    afx_msg void OnHeighthelp();
    afx_msg void OnButtonMixer();
    afx_msg void OnTSPathSelect();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnUpdateTSSize();
    afx_msg void OnKillfocusTSSize();
    afx_msg void OnTSSizeNoLimit();
    virtual void OnOK();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    BOOL IsChecked(int id);
    void SetChecked(int id, BOOL checked);

private:
    TS_OPTIONS options;

    /** Keeps track of the last usable size (> 0) so it can be restored if the
     * "no limit" check box is unchecked
     */
    DWORD m_lastSize;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSOPTIONSDLG_H__C76FD300_76CF_11D5_A1E2_AD9180F1D520__INCLUDED_)
