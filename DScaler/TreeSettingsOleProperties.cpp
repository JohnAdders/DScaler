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
 * @file TreeSettingsOleProperties.cpp implementation of the CTreeSettingsOleProperties class.
 */

#include "stdafx.h"
#include "TreeSettingsOleProperties.h"

using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreeSettingsOleProperties::CTreeSettingsOleProperties(const string& name,ULONG cObjects,LPUNKNOWN FAR* lplpUnk,ULONG cPages,LPCLSID lpPageClsID,LCID lcid) :
    CTreeSettingsPage(name,IDD_TREESETTINGS_OLEPAGE),
    m_tabCtrl(NULL)
{
    for(int i=0;i<cPages;i++)
    {
        CComPtr<IPropertyPage> pPage;
        HRESULT hr=pPage.CoCreateInstance(lpPageClsID[i]);
        if(FAILED(hr))
        {
            continue;
            //handle error, exception?
        }
        CPageInfo *pInfo=new CPageInfo;
        pInfo->m_pPropertyPage=pPage;
        pInfo->m_pPageSite=new CPageSite(lcid);
        pInfo->m_pPageSite->AddRef();

        hr=pInfo->m_pPropertyPage->SetPageSite(pInfo->m_pPageSite);
        _ASSERTE(SUCCEEDED(hr));
        hr=pInfo->m_pPropertyPage->SetObjects(cObjects,lplpUnk);
        _ASSERTE(SUCCEEDED(hr));
        m_pages.push_back(pInfo);
    }
}

CTreeSettingsOleProperties::~CTreeSettingsOleProperties()
{
    for(int i=0;i<m_pages.size();i++)
    {
        CPageInfo *pPage=m_pages[i];
        HRESULT hr;

        //only deactivate propetypages if it previously has been activated
        if(pPage->m_bActivated)
        {
            hr=pPage->m_pPropertyPage->Deactivate();
        }
        hr=pPage->m_pPropertyPage->SetPageSite(NULL);

        //it looks like this makes the crossbar property page create an
        //access voilation, propably cause by trying to delete a pointer
        //that poits to 0xBADF00D
        hr=pPage->m_pPropertyPage->SetObjects(0,NULL);
        //_ASSERTE(SUCCEEDED(hr));
        pPage->m_pPropertyPage.Release();
        pPage->m_pPageSite->Release();
        pPage->m_pPageSite=NULL;
        delete pPage;
    }
    m_pages.erase(m_pages.begin(),m_pages.end());
}

BOOL CTreeSettingsOleProperties::ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_SIZE, OnSize);
    HANDLE_MSG(hDlg, WM_NOTIFY, OnNotify);
    default:
        return FALSE;
    }
}


LRESULT CTreeSettingsOleProperties::OnNotify(HWND hwnd, int id, LPNMHDR nmhdr)
{
    switch(id)
    {
    case IDD_TREESETTINGS_TAB:
        if(nmhdr->code == TCN_SELCHANGE)
        {
            return OnSelchangeTreesettingsTab(hwnd, nmhdr);
        }
        if(nmhdr->code == TCN_SELCHANGING)
        {
            return OnSelchangingTreesettingsTab(hwnd, nmhdr);
        }
        break;
    default:
        break;
    }
    return FALSE;
}

BOOL CTreeSettingsOleProperties::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    USES_CONVERSION;
    
    m_tabCtrl = GetDlgItem(hDlg, IDD_TREESETTINGS_TAB);

    //find maximum width and height of the pages
    int i;
    for(i=0;i<m_pages.size();i++)
    {
        PROPPAGEINFO pageInfo;
        HRESULT hr=m_pages[i]->m_pPropertyPage->GetPageInfo(&pageInfo);
        if(SUCCEEDED(hr))
        {
            if(pageInfo.size.cx>m_minWidth)
            {
                m_minWidth=pageInfo.size.cx+8;
            }
            if(pageInfo.size.cy>m_minHeight)
            {
                m_minHeight=pageInfo.size.cy+20;
            }
        }
    }
    m_bInitMinSize=FALSE;

    RECT rect = {0,0,0,0};
    //add the border size of the tab controll
    TabCtrl_AdjustRect(m_tabCtrl, TRUE, &rect);
    m_minWidth+=rect.right - rect.left;
    m_minHeight+=rect.bottom - rect.top;

    GetClientRect(m_tabCtrl, &rect);

    ClientToScreen(m_tabCtrl, rect);
    ScreenToClient(hDlg, rect);

    TabCtrl_AdjustRect(m_tabCtrl, FALSE, &rect);
    for(i=0;i<m_pages.size();i++)
    {
        PROPPAGEINFO pageInfo;
        HRESULT hr=m_pages[i]->m_pPropertyPage->GetPageInfo(&pageInfo);
        if(SUCCEEDED(hr))
        {
            TabCtrl_InsertItem(m_tabCtrl, i,OLE2T(pageInfo.pszTitle));
            hr=m_pages[i]->m_pPropertyPage->Activate(m_tabCtrl, &rect, FALSE);
            if(SUCCEEDED(hr))
            {
                m_pages[i]->m_bActivated=TRUE;
            }
            else
            {
                m_pages[i]->m_bActivated=FALSE;
            }
        }
    }

    return TRUE;
}

BOOL CTreeSettingsOleProperties::OnSetActive(HWND hDlg)
{
    //try to activate a tab, if no tab can be activated, return FALSE
    //so access to this whole page will be denied
    BOOL bActivated=FALSE;
    for(int i=0;i<m_pages.size();i++)
    {
        TabCtrl_SetCurSel(m_tabCtrl, i);
        //simulate a click on one of the tabs
        LRESULT result = OnSelchangingTreesettingsTab(hDlg, NULL);
        if(result==FALSE)
        {
            result = OnSelchangeTreesettingsTab(hDlg, NULL);
            bActivated=TRUE;
            break;
        }
    }
    return bActivated;
}

LRESULT CTreeSettingsOleProperties::OnSelchangingTreesettingsTab(HWND hDlg, NMHDR* pNMHDR)
{
    int cursel=TabCtrl_GetCurSel(m_tabCtrl);
    if(!m_pages[cursel]->m_bActivated)
    {
        ErrorBox("This page can not be activated");
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

LRESULT CTreeSettingsOleProperties::OnSelchangeTreesettingsTab(HWND hDlg, NMHDR* pNMHDR)
{
    //activate and deactivate the proper pages
    int cursel=TabCtrl_GetCurSel(m_tabCtrl);
    for(int i=0;i<m_pages.size();i++)
    {
        if(i==cursel)
        {
            m_pages[i]->m_pPropertyPage->Show(SW_SHOWNORMAL);
        }
        else
        {
            m_pages[i]->m_pPropertyPage->Show(SW_HIDE);
        }
    }

    //make sure the page is properly positioned
    RECT rect;
    GetClientRect(hDlg, &rect);
    PostMessage(hDlg, WM_SIZE,SIZE_RESTORED,MAKELPARAM(rect.right - rect.left,rect.bottom - rect.top));

    return 0;
}

void CTreeSettingsOleProperties::OnSize(HWND hDlg, UINT nType, int cx, int cy)
{
    if(m_tabCtrl==NULL)
        return;

    RECT rect;
    GetClientRect(hDlg, &rect);
    rect.right = cx;
    rect.bottom = cy;
    MoveWindow(m_tabCtrl, rect);

    TabCtrl_AdjustRect(m_tabCtrl, FALSE, &rect);
    int cursel=TabCtrl_GetCurSel(m_tabCtrl);
    HRESULT hr=m_pages[cursel]->m_pPropertyPage->Move(&rect);
}

void CTreeSettingsOleProperties::OnOK(HWND hDlg)
{
    //check if any settings in the pages need to be aplied
    for(int i=0;i<m_pages.size();i++)
    {
        if(m_pages[i]->m_pPropertyPage->IsPageDirty()==S_OK)
        {
            HRESULT hr=m_pages[i]->m_pPropertyPage->Apply();
            //FIXME: log error if any
        }
   }
   EndDialog(hDlg, IDOK);
}

ULONG CTreeSettingsOleProperties::CPageSite::AddRef()
{
    return InterlockedIncrement(&m_dwRef);
}

HRESULT CTreeSettingsOleProperties::CPageSite::QueryInterface(REFIID iid,void ** ppvObject)
{
    if(iid==IID_IUnknown)
    {
        *ppvObject=this;
        AddRef();
        return S_OK;
    }
    else if(iid==IID_IPropertyPageSite)
    {
        *ppvObject=this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

ULONG CTreeSettingsOleProperties::CPageSite::Release()
{
    long l=InterlockedDecrement(&m_dwRef);
    if(l==0)
        delete this;
    return l;
}

HRESULT CTreeSettingsOleProperties::CPageSite::OnStatusChange(DWORD dwFlags)
{
    m_dwStatus=dwFlags;
    return S_OK;
}

HRESULT CTreeSettingsOleProperties::CPageSite::GetLocaleID(LCID *pLocaleID)
{
    if(pLocaleID==NULL)
        return E_POINTER;
    *pLocaleID=m_lcid;
    return E_FAIL;
}

HRESULT CTreeSettingsOleProperties::CPageSite::GetPageContainer(IUnknown **ppUnk)
{
    //OleCreatePropertyFrame also returns E_NOTIMPL
    return E_NOTIMPL;
}

HRESULT CTreeSettingsOleProperties::CPageSite::TranslateAccelerator(MSG *pMsg)
{
    //OleCreatePropertyFrame also returns E_NOTIMPL
    return E_NOTIMPL;
}
