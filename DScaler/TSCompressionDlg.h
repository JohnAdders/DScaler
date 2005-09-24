/* $Id: TSCompressionDlg.h,v 1.2 2005-09-24 18:47:56 dosx86 Exp $ */

/** \file
 * Time Shift compression options dialog
 * \author Nick Kochakian
 */

#if !defined(AFX_TSCOMPRESSION_H__0383010E_0797_44D3_B4E6_2992E75326F3__INCLUDED_)
#define AFX_TSCOMPRESSION_H__0383010E_0797_44D3_B4E6_2992E75326F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TSCompression.h : header file
//

#include "TSOptionsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CTSCompressionDlg dialog

class CTSCompressionDlg : public CDialog
{
// Construction
public:
	CTSCompressionDlg(CWnd* pParent = NULL, TS_OPTIONS *options = NULL);

// Dialog Data
	//{{AFX_DATA(CTSCompressionDlg)
	enum { IDD = IDD_TSCOMPRESSION };
	CString	m_AudioFormat;
	CString	m_VideoFormat;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTSCompressionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTSCompressionDlg)
	afx_msg void OnConfigVideo();
	afx_msg void OnConfigAudio();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    TS_OPTIONS *options;
    FOURCC     m_fcc;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TSCOMPRESSION_H__0383010E_0797_44D3_B4E6_2992E75326F3__INCLUDED_)
