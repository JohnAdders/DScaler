/////////////////////////////////////////////////////////////////////////////
// $Id: TSOptionsDlg.h,v 1.12 2005-09-24 18:48:21 dosx86 Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2001   Eric Schmidt          Original Release.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// EMU: 23rd June 2004 added a simple scheduler / timer and
// custom AV sync setting (it is stored in the INI file).
// Put radio button to enable / disable 'TimeShift' warnings.
//
// $Log: not supported by cvs2svn $
// Revision 1.11  2005/07/17 20:46:25  dosx86
// Updated to use the new time shift options dialog. Redid some of the help text.
//
// Revision 1.10  2004/08/12 16:27:47  adcockj
// added timeshift changes from emu
//
// Revision 1.9  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.8  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.7  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.6  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.5  2001/08/06 03:00:17  ericschmidt
// solidified auto-pixel-width detection
// preliminary pausing-of-live-tv work
//
// Revision 1.4  2001/07/26 15:28:14  ericschmidt
// Added AVI height control, i.e. even/odd/averaged lines.
// Used existing cpu/mmx detection in TimeShift code.
//
// Revision 1.3  2001/07/24 12:25:49  adcockj
// Added copyright notice as per standards
//
// Revision 1.2  2001/07/24 12:24:25  adcockj
// Added Id to comment block
//
// Revision 1.1  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
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
	CEdit	m_PathDisplay;
	CStatic	m_SizeGiB;
	CButton	m_SizeCheckBox;
	CEdit	m_SizeEdit;
	CComboBox	m_WaveOutComboBox;
	CComboBox	m_WaveInComboBox;
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
