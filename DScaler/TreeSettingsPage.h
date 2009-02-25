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
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2002/05/19 12:01:43  tobbej
// fixed sizing of propertypage
//
// Revision 1.2  2002/05/09 17:20:15  tobbej
// fixed resize problem in CTreeSettingsOleProperties
// (everytime a new page was activated the dialog size incresed)
//
// Revision 1.1  2002/04/24 19:04:01  tobbej
// new treebased settings dialog
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsPage.h header file for CTreeSettingsPage
 */

#if !defined(AFX_TREESETTINGSPAGE_H__FD745055_FF62_4B27_9F15_6D6F31EDFA3A__INCLUDED_)
#define AFX_TREESETTINGSPAGE_H__FD745055_FF62_4B27_9F15_6D6F31EDFA3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/**
 * Class for one page in the settings dialog.
 * The dialog box template used must have WS_CHILD and DS_CONTROL atributes
 * set and no borders, the DS_CONTROL atribut makes tab work properly.
 * This class can be used directly to create for example an empty page or a
 * page with only text (maybe some help text or a description of the subpages)
 * or it can be inherited from.
 *
 * @see CTreeSettingsDlg
 */
class CTreeSettingsPage : public CDialog
{
// Construction
public:
	/**
	 * Constructor.
	 * @param name name used in the tree
	 * @param nIDTemplate dialog template id
	 */
	CTreeSettingsPage(CString name,UINT nIDTemplate);   // standard constructor
	CTreeSettingsPage(CString TreeName,CString HeaderName,UINT nIDTemplate);

// Dialog Data
	//{{AFX_DATA(CTreeSettingsPage)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeSettingsPage)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTreeSettingsPage)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class CTreeSettingsDlg;
	virtual void OnOK();
	virtual void OnCancel();

	/**
	 * Called when the page is about to be activated.
	 *
	 * @return true to allow the page to be activated
	 */
	virtual bool OnSetActive();

	/**
	 * Called when the page is about to be deactivated.
	 *
	 * @return true to allow the page to be deactivated
	 */
	virtual bool OnKillActive();

	/**
	 * Called when cancel is pressed in CTreeSettingsDlg but before the dialog is closed.
	 * This function allows the page to abort the cancel.
	 *
	 * @return true to allow the cancel to continue or false to disalow cancel
	 */
	virtual bool OnQueryCancel();

	/**
	 * This function is used by CTreeSettingsDlg to get the minimum allowed
	 * size of the page.
	 * The default implementation of this function gets the windows size at
	 * first call and then uses that for width and height.
	 * @see CTreeSettingsDlg
	 * @param width minimum allowed width of the page
	 * @param height minimum allowed heith of the page
	 */
	virtual void GetMinSize(int &width,int &height);

	/// @return name of this page
	virtual CString GetName() {return m_name;};
	virtual CString GetHeaderName() {return m_HeaderName;};

	/// @return dialog template id for the dialog
	UINT GetDlgID() {return m_dlgID;};

protected:
	CString m_name;
	CString m_HeaderName;
	UINT m_dlgID;

	int m_minWidth;
	int m_minHeight;
	bool m_bInitMinSize;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREESETTINGSPAGE_H__FD745055_FF62_4B27_9F15_6D6F31EDFA3A__INCLUDED_)
