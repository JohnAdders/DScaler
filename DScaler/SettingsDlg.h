/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsDlg.h,v 1.3 2001-06-24 14:07:22 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 22 Jun 2001   Torbjörn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_)
#define AFX_SETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DS_ApiCommon.h"
#include "resource.h"
#include "HSListBox.h"

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

class CSettingsDlg : public CDialog
{
// Construction
public:
	static void ShowSettingsDlg(CString caption,SETTING *settings,long count,CWnd *pParent=NULL);
	friend CSettingsDlg::ShowSettingsDlg(SETTING *,long );

	CSettingsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_SETTINGS };
	CComboBox	m_combo;
	CButton	m_chk;
	CSpinButtonCtrl	m_spin;
	CSliderCtrl	m_slider;
	CEdit	m_edit;
	CButton	m_btnDefault;
	CHSListBox	m_lstbox;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
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
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateControls();
	long m_settingsCount;
	SETTING* m_settings;
	long m_currentSetting;
	CString m_caption;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__AF9102F2_06B5_43B0_8DD7_C3CFFA8DCCDB__INCLUDED_)
