/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsDlg.h,v 1.12 2001-12-03 17:57:36 tobbej Exp $
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
// 22 Jun 2001   Torbjörn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file SettingsDlg.h Header file for CSettingsDlg class
 */

#if !defined(AFX_SETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_)
#define AFX_SETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DS_ApiCommon.h"
#include "HSListBox.h"

/**
 * An MFC generic settings dialog
 * @see CHSListBox
 */
class CSettingsDlg : public CDialog
{
// Construction
public:

    /**
     * Shows the settings dialog.
     * Static memberfunction that takes care of creating a generic settings dialog and show it
     *
     * @param caption text at top of the window
     * @param settings pointer to array of SETTING
     * @param Count size of settings array
     */
    static void ShowSettingsDlg(CString caption,SETTING* settings,long Count,CWnd* pParent=NULL);

    /// standard constructor
    CSettingsDlg(CWnd* pParent = NULL);

// Dialog Data
    //{{AFX_DATA(CSettingsDlg)
    enum { IDD = IDD_SETTINGS };
    CComboBox   m_Combo;
    CButton m_CheckBox;
    CSpinButtonCtrl m_Spin;
    CSliderCtrl m_Slider;
    CEdit   m_Edit;
    CButton m_DefaultButton;
    CHSListBox  m_ListBox;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSettingsDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CSettingsDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeList();
    afx_msg void OnChangeEdit();
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnSettingsDefault();
    afx_msg void OnCheckClick();
    afx_msg void OnSelchangeChoosefromlist();
    afx_msg void OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnClose();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
private:
    void UpdateControls();
    long m_SettingsCount;
    SETTING* m_Settings;
    long m_CurrentSetting;
    CString m_Caption;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_)
