/////////////////////////////////////////////////////////////////////////////
// $Id: DSVideoFormatPage.h,v 1.5 2003-01-06 21:31:09 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2002/09/17 17:34:21  tobbej
// added a few todo items
//
// Revision 1.3  2002/09/11 16:41:03  tobbej
// implemented clear button
//
// Revision 1.2  2002/09/07 13:33:35  tobbej
// implemented delete and allow listbox to be reordered via drag and drop
//
// Revision 1.1  2002/09/04 17:08:31  tobbej
// new video format configuration dialog (resolution)
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSVideoFormatPage.h header file
 */

#if !defined(AFX_DSVIDEOFORMATPAGE_H__24867333_526D_4B36_8A75_E64A92C140A3__INCLUDED_)
#define AFX_DSVIDEOFORMATPAGE_H__24867333_526D_4B36_8A75_E64A92C140A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TreeSettingsPage.h"
#include "..\DScalerRes\resource.h"
#include "DSGraph.h"
#include <vector>
#include "HSListBox.h"

/**
 * CDSVideoFormatPage dialog.
 * Makes it posibel to customize the resolution submenu.
 * @todo validate settings in OnOK
 * @todo implement "Reset to default" button
 * @todo implement croping, might need changes to dsrend filter
 */
class CDSVideoFormatPage : public CTreeSettingsPage
{
// Construction
public:
	CDSVideoFormatPage(CString name,vector<CDShowGraph::CVideoFormat> &fmts,CSimpleSetting *pResolution);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDSVideoFormatPage)
	enum { IDD = IDD_DSHOW_VIDEOFMTS };
	CDragListBox m_ListBox;
	CSpinButtonCtrl	m_SpinWidth;
	CSpinButtonCtrl	m_SpinHeight;
	//CButton m_YUY2Check;
	//CComboBox m_SampleFormat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDSVideoFormatPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDSVideoFormatPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnVideoFmtSelChange();
	afx_msg void OnDeltaPosWidth(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDeltaPosHeight(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeWidth();
	afx_msg void OnChangeHeight();
	afx_msg void OnChangeName();
	//afx_msg void OnClickedYUY2();
	//afx_msg void OnSelEndOkFieldFmt();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClickedDelete();
	afx_msg void OnClickedNew();
	afx_msg void OnClickedClear();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	void OnOK();

private:
	void UpdateControlls();
	void GenerateName(int pos);
	vector<CDShowGraph::CVideoFormat> m_VideoFmt;
	
	/**
	 * Reference to the real video format vector that is used by dscaler,
	 * this is the one we save the changes to when closing dialog by clicking on ok
	 */
	vector<CDShowGraph::CVideoFormat> &m_RealVideoFmt;

	///flag to prevent controlls from messing with the settings when updating controlls
	bool m_bInUpdateControlls;
	CSimpleSetting *m_pResolutionSetting;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSVIDEOFORMATPAGE_H__24867333_526D_4B36_8A75_E64A92C140A3__INCLUDED_)
