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
 * @file TreeSettingsOleProperties.h interface for the CTreeSettingsOleProperties class.
 */

#if !defined(_TREESETTINGSOLEPROPERTIES_H__)
#define _TREESETTINGSOLEPROPERTIES_H__

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
    CTreeSettingsOleProperties(const tstring& name,ULONG cObjects,LPUNKNOWN FAR* lplpUnk,ULONG cPages,LPCLSID lpPageClsID,LCID lcid);
    virtual ~CTreeSettingsOleProperties();

// Implementation
private:
    // Generated message map functions
    INT_PTR ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    LRESULT OnSelchangeTreesettingsTab(HWND hDlg, NMHDR* pNMHDR);
    LRESULT OnSelchangingTreesettingsTab(HWND hDlg, NMHDR* pNMHDR);
    LRESULT OnNotify(HWND hwnd, int id, LPNMHDR nmhdr);
    void OnSize(HWND hDlg, UINT nType, int cx, int cy);
    void OnOK(HWND hDlg);
    BOOL OnSetActive(HWND hDlg);

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
        BOOL m_bActivated;
    };
    ///vector of all pages
    std::vector<CPageInfo*> m_pages;
    HWND m_tabCtrl;

};

#endif
