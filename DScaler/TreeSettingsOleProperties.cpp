/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsOleProperties.cpp,v 1.5 2002-07-06 16:46:13 tobbej Exp $
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
// Revision 1.4  2002/05/19 12:01:42  tobbej
// fixed sizing of propertypage
//
// Revision 1.3  2002/05/09 17:20:15  tobbej
// fixed resize problem in CTreeSettingsOleProperties
// (everytime a new page was activated the dialog size incresed)
//
// Revision 1.2  2002/05/02 20:05:51  tobbej
// removed an assert
//
// Revision 1.1  2002/04/24 19:04:01  tobbej
// new treebased settings dialog
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsOleProperties.cpp implementation of the CTreeSettingsOleProperties class.
 */

#include "stdafx.h"
#include "TreeSettingsOleProperties.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreeSettingsOleProperties::CTreeSettingsOleProperties(CString name,ULONG cObjects,LPUNKNOWN FAR* lplpUnk,ULONG cPages,LPCLSID lpPageClsID,LCID lcid)
:CTreeSettingsPage(name,IDD_TREESETTINGS_OLEPAGE),m_bIsPagesIsActivated(false)
{
	//{{AFX_DATA_INIT(CTreeSettingsOleProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

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
		ASSERT(SUCCEEDED(hr));
		hr=pInfo->m_pPropertyPage->SetObjects(cObjects,lplpUnk);
		ASSERT(SUCCEEDED(hr));
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
		if(m_bIsPagesIsActivated)
		{
			hr=pPage->m_pPropertyPage->Deactivate();
		}
		hr=pPage->m_pPropertyPage->SetPageSite(NULL);

		//it looks like this makes the crossbar property page create an
		//access voilation, propably cause by trying to delete a pointer
		//that poits to 0xBADF00D
		hr=pPage->m_pPropertyPage->SetObjects(0,NULL);
		//ASSERT(SUCCEEDED(hr));
		pPage->m_pPropertyPage.Release();
		pPage->m_pPageSite->Release();
		pPage->m_pPageSite=NULL;
		delete pPage;
	}
	m_pages.erase(m_pages.begin(),m_pages.end());
}

void CTreeSettingsOleProperties::DoDataExchange(CDataExchange* pDX)
{
	CTreeSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTreeSettingsOleProperties)
	DDX_Control(pDX, IDD_TREESETTINGS_TAB, m_tabCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTreeSettingsOleProperties, CTreeSettingsPage)
	//{{AFX_MSG_MAP(CTreeSettingsOleProperties)
	ON_NOTIFY(TCN_SELCHANGE, IDD_TREESETTINGS_TAB, OnSelchangeTreesettingsTab)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CTreeSettingsOleProperties::OnInitDialog()
{
	USES_CONVERSION;
	CTreeSettingsPage::OnInitDialog();
	
	//find maximum width and height of the pages
	for(int i=0;i<m_pages.size();i++)
	{
		PROPPAGEINFO pageInfo;
		HRESULT hr=m_pages[i]->m_pPropertyPage->GetPageInfo(&pageInfo);
		if(SUCCEEDED(hr))
		{
			if(pageInfo.size.cx>m_minWidth)
			{
				m_minWidth=pageInfo.size.cx;
			}
			if(pageInfo.size.cy>m_minHeight)
			{
				m_minHeight=pageInfo.size.cy;
			}
		}
	}
	m_bInitMinSize=false;

	CRect rect(0,0,0,0);
	//add the border size of the tab controll
	m_tabCtrl.AdjustRect(TRUE,&rect);
	m_minWidth+=rect.Width();
	m_minHeight+=rect.Height();
	
	m_tabCtrl.GetClientRect(&rect);

	m_tabCtrl.ClientToScreen(&rect);
	ScreenToClient(&rect);

	m_tabCtrl.AdjustRect(FALSE,&rect);
	for(i=0;i<m_pages.size();i++)
	{
		PROPPAGEINFO pageInfo;
		HRESULT hr=m_pages[i]->m_pPropertyPage->GetPageInfo(&pageInfo);
		if(SUCCEEDED(hr))
		{
			m_tabCtrl.InsertItem(i,OLE2T(pageInfo.pszTitle));	
			hr=m_pages[i]->m_pPropertyPage->Activate(m_hWnd,rect,FALSE);
		}
	}
	
	m_bIsPagesIsActivated=true;
	//simulate a click on one of the tabs
	LRESULT tmp;
	OnSelchangeTreesettingsTab(NULL,&tmp);
	return TRUE;
}

void CTreeSettingsOleProperties::OnSelchangeTreesettingsTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//activate and deactivate the proper pages
	int cursel=m_tabCtrl.GetCurSel();
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
	CRect rect;
	GetClientRect(&rect);
	PostMessage(WM_SIZE,SIZE_RESTORED,MAKELPARAM(rect.Width(),rect.Height()));

	*pResult = 0;
}

void CTreeSettingsOleProperties::OnSize(UINT nType, int cx, int cy) 
{
	CTreeSettingsPage::OnSize(nType, cx, cy);
	
	if(m_tabCtrl.m_hWnd==NULL)
		return;
	
	CRect rect;
	GetClientRect(&rect);
	m_tabCtrl.MoveWindow(&rect);

	m_tabCtrl.AdjustRect(FALSE,&rect);
	int cursel=m_tabCtrl.GetCurSel();
	HRESULT hr=m_pages[cursel]->m_pPropertyPage->Move(&rect);
}

void CTreeSettingsOleProperties::OnOK()
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
