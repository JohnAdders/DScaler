/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsOleProperties.h,v 1.5 2002-07-11 17:56:38 tobbej Exp $
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
// Revision 1.4  2002/07/06 16:46:13  tobbej
// only deactivate page if it previously was activated
//
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
 * @file TreeSettingsOleProperties.h interface for the CTreeSettingsOleProperties class.
 */

#if !defined(AFX_TREESETTINGSOLEPROPERTIES_H__D135AD9A_6F42_4439_90A3_025DA1E71858__INCLUDED_)
#define AFX_TREESETTINGSOLEPROPERTIES_H__D135AD9A_6F42_4439_90A3_025DA1E71858__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TreeSettingsPage.h"
#include <vector>
#include "..\DScalerRes\resource.h"

/**
 * Property page for COM propery pages.
 * This class can be used to show property pages from COM objects, for example DirectShow filters.
 *
 * @todo IPropertyPage::TranslateAccelerator is never called
 * @todo handle help messages and show help for active page
 */
class CTreeSettingsOleProperties : public CTreeSettingsPage  
{
// Construction
public:
	CTreeSettingsOleProperties(CString name,ULONG cObjects,LPUNKNOWN FAR* lplpUnk,ULONG cPages,LPCLSID lpPageClsID,LCID lcid);
	virtual ~CTreeSettingsOleProperties();

// Dialog Data
	//{{AFX_DATA(CTreeSettingsOleProperties)
	enum { IDD = IDD_TREESETTINGS_OLEPAGE };
	CTabCtrl	m_tabCtrl;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeSettingsOleProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTreeSettingsOleProperties)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTreesettingsTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangingTreesettingsTab(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnOK();
	bool OnSetActive();

private:
	/**
	 * Internal IPropertyPageSite COM object.
	 * Used for comunicating with the the COM property page (IPropertyPage).
	 */
	class CPageSite : public IPropertyPageSite
	{
	public:
		CPageSite(LCID lcid):m_dwRef(0),m_lcid(lcid){};
		virtual ~CPageSite(){};
		//IUnknown
		ULONG STDMETHODCALLTYPE AddRef();
		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void ** ppvObject);
		ULONG STDMETHODCALLTYPE Release();
		
		//IPropertyPageSite
		HRESULT STDMETHODCALLTYPE OnStatusChange(DWORD dwFlags);
		HRESULT STDMETHODCALLTYPE GetLocaleID(LCID *pLocaleID);
		HRESULT STDMETHODCALLTYPE GetPageContainer(IUnknown **ppUnk);
		HRESULT STDMETHODCALLTYPE TranslateAccelerator(MSG *pMsg);
		
		DWORD GetStatus() {return m_dwStatus;};

	private:
		///IUnknown reference counter
		LONG m_dwRef;
		///page status
		DWORD m_dwStatus;
		///locale id
		LCID m_lcid;
	};
	
	class CPageInfo
	{
	public:
		CPageSite *m_pPageSite;
		CComPtr<IPropertyPage> m_pPropertyPage;
		bool m_bActivated;
	};
	///vector of all pages
	std::vector<CPageInfo*> m_pages;
};

#endif // !defined(AFX_TREESETTINGSOLEPROPERTIES_H__D135AD9A_6F42_4439_90A3_025DA1E71858__INCLUDED_)
