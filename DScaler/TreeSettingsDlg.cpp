/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsDlg.cpp,v 1.13 2002-07-27 13:48:53 laurentg Exp $
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
// Revision 1.12  2002/07/20 13:07:36  laurentg
// New setting for vertical mirror
//
// Revision 1.11  2002/07/20 12:09:39  laurentg
// Card calibration settings added in the tree settings
//
// Revision 1.10  2002/07/19 15:31:39  laurentg
// New settings (other settings) added in the tree settings + related menu items deleted
//
// Revision 1.9  2002/07/19 11:59:12  laurentg
// OSD settings added in the tree settings
//
// Revision 1.8  2002/07/11 17:41:37  tobbej
// dont allow focus to change if Show() failed
//
// Revision 1.7  2002/07/03 00:45:41  laurentg
// Add a new section in the Change Settings dialog box to set the thread priorities
//
// Revision 1.6  2002/06/30 18:51:34  laurentg
// IDH_STILL added
//
// Revision 1.5  2002/06/23 21:49:39  laurentg
// Change the title of the dialog box
//
// Revision 1.4  2002/06/23 21:41:22  laurentg
// New entry in the tree settings dialog box for still settings
//
// Revision 1.3  2002/06/13 12:10:23  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.2  2002/05/09 17:20:15  tobbej
// fixed resize problem in CTreeSettingsOleProperties
// (everytime a new page was activated the dialog size incresed)
//
// Revision 1.1  2002/04/24 19:04:00  tobbej
// new treebased settings dialog
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsDlg.cpp implementation file for tree settings dialog
 */

#include "stdafx.h"
#include "TreeSettingsDlg.h"
#include "TreeSettingsGeneric.h"
#include "Filter.h"
#include "Deinterlace.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "AspectRatio.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "Other.h"
#include "ProgramList.h"
#include "StillSource.h"
#include "DScaler.h"
#include "OSD.h"
#include "Calibration.h"
#include "OutThreads.h"
#include "..\help\helpids.h"

#include <afxpriv.h>	//WM_COMMANDHELP

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CTreeSettingsDlg::CTreeSettingsDlg(CString caption,CWnd* pParent /*=NULL*/)
	: CDialog(CTreeSettingsDlg::IDD, pParent),
	m_settingsDlgCaption(caption),
	m_iCurrentPage(-1),
	m_iStartPage(-1)
{
	//{{AFX_DATA_INIT(CTreeSettingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTreeSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTreeSettingsDlg)
	DDX_Control(pDX, IDC_TREESETTINGS_TREE, m_tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTreeSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CTreeSettingsDlg)
	ON_NOTIFY(TVN_SELCHANGING, IDC_TREESETTINGS_TREE, OnSelchangingTree)
	ON_BN_CLICKED(IDC_HELPBTN, OnHelpBtn)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsDlg message handlers

void CTreeSettingsDlg::OnOK() 
{
	for(int i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		//check if the page has been created
		if(::IsWindow(pPage->m_hWnd))
		{
			if(!pPage->UpdateData())
			{
				//show the page that failed
				if(m_iCurrentPage!=i)
				{
					ShowPage(i);
				}
				return;
			}
		}
	}
	for(i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		//check if the page has been created
		if(::IsWindow(pPage->m_hWnd))
		{
			pPage->OnOK();
		}
	}
	
	CDialog::OnOK();
}

void CTreeSettingsDlg::OnCancel() 
{
	for(int i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		if(!pPage->OnQueryCancel())
		{
			//maybe also call ShowPage(i) so the page that aborted the cancel is shown
			return;
		}
	}

	for(i=0;i<m_pages.size();i++)
	{
		CTreeSettingsPage *pPage=m_pages[i].m_pPage;
		
		//call OnCancel only on pages that has been created (selected atleast once)
		if(::IsWindow(pPage->m_hWnd))
		{
			pPage->OnCancel();
		}
	}
	
	CDialog::OnCancel();
}

void CTreeSettingsDlg::OnSelchangingTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;
	int iPage=pNMTreeView->itemNew.lParam;
	if(m_iCurrentPage!=iPage)
	{
		if(!ShowPage(iPage))
		{
			*pResult=TRUE;
		}
	}
}

BOOL CTreeSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_settingsDlgCaption);

	for(int i=0;i<m_pages.size();i++)
	{
		HTREEITEM hParent=TVI_ROOT;
		if(m_pages[i].m_parent>=0)
		{

			if(m_pages[i].m_parent<m_pages.size())
			{
				hParent=m_pages[m_pages[i].m_parent].m_hTreeItem;
			}
		}
		HTREEITEM hNode=m_tree.InsertItem(m_pages[i].m_pPage->GetName(),0,0,hParent);
		m_tree.SetItemState(hNode,TVIS_EXPANDED,TVIS_EXPANDED);
		m_tree.SetItemData(hNode,i);
		m_pages[i].m_hTreeItem=hNode;
	}

	//show the first page
	if(m_iStartPage>=0 && m_iStartPage<m_pages.size())
	{
		ShowPage(m_iStartPage);
	}
	else
	{
		ShowPage(0);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CTreeSettingsDlg::AddPage(CTreeSettingsPage *pPage,int parent)
{
	CPageInfo newPage;
	newPage.m_pPage=pPage;
	newPage.m_parent=parent;
	newPage.m_hTreeItem=NULL;
	m_pages.push_back(newPage);
	return m_pages.size()-1;
}

bool CTreeSettingsDlg::ShowPage(int iPage)
{
	if(iPage<0 || iPage>=m_pages.size())
	{
		return false;
	}
	CRect pageSize;
	CWnd *pPageFrame=GetDlgItem(IDC_TREESETTINGS_PAGEFRAME);
	ASSERT(pPageFrame!=NULL);
	pPageFrame->GetWindowRect(&pageSize);
	ScreenToClient(&pageSize);

	CTreeSettingsPage *pNewPage=m_pages[iPage].m_pPage;
	//create the new page if nessesary
	if(!::IsWindow(pNewPage->m_hWnd))
	{
		if(!pNewPage->Create(pNewPage->GetDlgID(),this))
		{
			//create failed
			return false;
		}
	}
	//can the new page be activated?
	if(!pNewPage->OnSetActive())
	{
		return false;
	}
	
	//deactivate current page
	CTreeSettingsPage *pCurrentPage=NULL;
	if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
	{
		pCurrentPage=m_pages[m_iCurrentPage].m_pPage;
		if(pCurrentPage->OnKillActive())
		{
			pCurrentPage->ShowWindow(SW_HIDE);
		}
		else
		{
			return false;
		}
	}
	
	//show the new page
	pNewPage->ShowWindow(SW_SHOWNOACTIVATE);
	m_iCurrentPage=iPage;
	
	//make sure the new page is properly positioned
	CRect rect;
	GetClientRect(&rect);
	PostMessage(WM_SIZE,SIZE_RESTORED,MAKELPARAM(rect.Width(),rect.Height()));
	return true;
}

void CTreeSettingsDlg::OnHelpBtn() 
{
	//send WM_COMMANDHELP (mfc internal message) to the currently active page
	if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
	{
		m_pages[m_iCurrentPage].m_pPage->SendMessage(WM_COMMANDHELP);
	}
	
}

void CTreeSettingsDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
	if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
	{
		CRect rect;
		int height;
		int width;

		//active page
		CWnd *pPageFrame=GetDlgItem(IDC_TREESETTINGS_PAGEFRAME);
		pPageFrame->GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.right=cx-10;
		rect.bottom=cy-50;
		pPageFrame->MoveWindow(rect,FALSE);
		
		//check if the current size is enough for the active page
		int minWidth=0;
		int minHeight=0;
		m_pages[m_iCurrentPage].m_pPage->GetMinSize(minWidth,minHeight);
		if(rect.Width()<minWidth || rect.Height()<minHeight)
		{
			CRect dlgRect;
			GetWindowRect(&dlgRect);
			
			//calculate by how much the size must be increased
			int newWidth=(minWidth-rect.Width())>0 ? minWidth-rect.Width() : 0;
			int newHeight=(minHeight-rect.Height())>0 ?minHeight-rect.Height() : 0;
			
			dlgRect.right+=newWidth;
			dlgRect.bottom+=newHeight;
			MoveWindow(&dlgRect);
			return;
		}
		m_pages[m_iCurrentPage].m_pPage->MoveWindow(rect,FALSE);

		//line
		CRect line;
		CWnd *pLine=GetDlgItem(IDC_TREESETTINGS_LINE);
		pLine->GetWindowRect(&line);
		ScreenToClient(&line);
		height=line.Height();
		rect.top=cy-40-height;
		rect.bottom=rect.top+height;
		pLine->MoveWindow(&rect,FALSE);

		
		//tree
		m_tree.GetWindowRect(&rect);
		ScreenToClient(&rect);
		rect.bottom=cy-40;
		m_tree.MoveWindow(&rect,FALSE);
		
		//buttons
		CWnd *pOkBtn=GetDlgItem(IDOK);
		CWnd *pCancelBtn=GetDlgItem(IDCANCEL);
		CWnd *pHelpBtn=GetDlgItem(IDC_HELPBTN);

		pHelpBtn->GetWindowRect(&rect);
		ScreenToClient(&rect);
		width=rect.Width();
		height=rect.Height();

		rect.right=cx-10;
		rect.left=rect.right-width;
		rect.top=cy-30;
		rect.bottom=rect.top+height;
		pHelpBtn->MoveWindow(&rect,FALSE);

		rect.right=rect.left-5;
		rect.left=rect.right-width;
		pCancelBtn->MoveWindow(&rect,FALSE);

		rect.right=rect.left-5;
		rect.left=rect.right-width;
		pOkBtn->MoveWindow(&rect,FALSE);
		InvalidateRect(NULL);
	}
	
}

void CTreeSettingsDlg::ShowTreeSettingsDlg(int iSettingsMask)
{
	//the following is just a quick test of the new treebased settings dialog
	//it shoud probably be cleand up and moved somwere else

    int mask = iSettingsMask;

    if ( !(mask & (FILTER_SETTINGS_MASK | DEINTERLACE_SETTINGS_MASK | ADVANCED_SETTINGS_MASK)) )
    {
        mask = FILTER_SETTINGS_MASK | DEINTERLACE_SETTINGS_MASK | ADVANCED_SETTINGS_MASK;
    }

	vector<CTreeSettingsGeneric*> pages;
	CTreeSettingsDlg dlg(CString("DScaler settings"));

    int Root;
    long Num;
    long i;

	CTreeSettingsPage RootPage(CString("Filter settings"),IDD_TREESETTINGS_EMPTY);
	
	//the default help id is HID_BASE_RESOURCE+dialog template id
	//but we cant use that for empty pages and the generic property page
	//so set a new help id to use insted.
	//since the IDH_FILTERS already contains HID_BASE_RESOURCE, subtract that
	RootPage.SetHelpID(IDH_FILTERS);
	Root = dlg.AddPage(&RootPage);

    if (mask & FILTER_SETTINGS_MASK)
    {    
	    FILTER_METHOD** FilterMethods;
	    GetFilterSettings(FilterMethods, Num);
	    for(i = 0; i < Num; i++)
	    {
		    CTreeSettingsGeneric *pPage = new CTreeSettingsGeneric(
                                                                    FilterMethods[i]->szName,
                                                                    FilterMethods[i]->pSettings,
                                                                    FilterMethods[i]->nSettings
                                                                  );
		    pPage->SetHelpID(FilterMethods[i]->HelpID);
		    
		    pages.push_back(pPage);
		    dlg.AddPage(pPage, Root);
	    }
    }

    CTreeSettingsPage DeintRootPage(CString("Video Methods"), IDD_TREESETTINGS_EMPTY);
	
	DeintRootPage.SetHelpID(IDH_DEINTERLACE);
	Root = dlg.AddPage(&DeintRootPage);

    if (mask & DEINTERLACE_SETTINGS_MASK)
    {
        DEINTERLACE_METHOD** DeintMethods;
	    GetDeinterlaceSettings(DeintMethods, Num);
	    for(i = 0; i < Num; i++)
	    {
		    CTreeSettingsGeneric* pPage = new CTreeSettingsGeneric(
                                                                    DeintMethods[i]->szName,
                                                                    DeintMethods[i]->pSettings,
                                                                    DeintMethods[i]->nSettings
                                                                  );
		    pPage->SetHelpID(DeintMethods[i]->HelpID);
		    
		    pages.push_back(pPage);
		    dlg.AddPage(pPage, Root);
	    }
    }

    CTreeSettingsPage AdvRootPage(CString("Advanced Settings"), IDD_TREESETTINGS_EMPTY);
	AdvRootPage.SetHelpID(IDH_ADVANCED);
	Root = dlg.AddPage(&AdvRootPage);

    if (mask & ADVANCED_SETTINGS_MASK)
    {
        CTreeSettingsGeneric* pPage = FD50_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_22_PULLDOWN);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = FD60_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_32_PULLDOWN);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = FD_Common_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_PULLDOWN_SHARED);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = Aspect_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_ASPECT);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = Timing_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_TIMING);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = OutThreads_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_ADVANCED);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = AntiPlop_GetTreeSettingsPage();
        // \todo AntiPlop Help
	    pPage->SetHelpID(IDH_ADVANCED);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = OSD_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_ADVANCED);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = DScaler_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_ADVANCED);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = Still_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_STILL);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = Calibr_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_CALIBRATION);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = Other_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_OVERLAY);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = Debug_GetTreeSettingsPage();
	    pPage->SetHelpID(IDH_LOGGING);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);

        pPage = DScaler_GetTreeSettingsPage2();
	    pPage->SetHelpID(IDH_ADVANCED);
	    pages.push_back(pPage);
	    dlg.AddPage(pPage, Root);
    }

    dlg.DoModal();

	for(vector<CTreeSettingsGeneric*>::iterator it=pages.begin();it!=pages.end();it++)
	{
		delete *it;
	}
	pages.erase(pages.begin(),pages.end());
}
