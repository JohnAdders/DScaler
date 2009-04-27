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
#include "FD_Prog.h"
#include "AspectRatio.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "OverlayOutput.h"
#include "D3D9Output.h"
#include "ProgramList.h"
#include "StillSource.h"
#include "DScaler.h"
#include "OSD.h"
#include "Calibration.h"
#include "OutThreads.h"
#include "SettingsPerChannel.h"
#include "Providers.h"
#include "BT848Source.h"
#include "VBI_VideoText.h"
#include "..\help\helpids.h"
#include "SettingsMaster.h"
#include "TimeShift.h"
#include "EPG.h"

using namespace std;

CTreeSettingsDlg::CPageInfo::CPageInfo() :
        m_parent(-1),
        m_hTreeItem(NULL),
        m_hWnd(NULL)
{
}

CTreeSettingsDlg::CPageInfo::~CPageInfo()
{
    if(m_hWnd && m_pPage)
    {
        m_pPage->Destroy(m_hWnd);
        m_hWnd= NULL;
    }
}

CTreeSettingsDlg::CTreeSettingsDlg(const string& Caption) : 
    CDSDialog(MAKEINTRESOURCE(IDD_TREESETTING)),
    m_settingsDlgCaption(Caption),
    m_iCurrentPage(-1),
    m_iStartPage(-1)
{
}

CTreeSettingsDlg::~CTreeSettingsDlg()
{
}

int CTreeSettingsDlg::AddPage(SmartPtr<CTreeSettingsPage> pPage,int parent)
{
    CPageInfo newPage;
    newPage.m_pPage=pPage;
    newPage.m_parent=parent;
    newPage.m_hTreeItem=NULL;
    m_pages.push_back(newPage);
    return m_pages.size()-1;
}

void CTreeSettingsDlg::ShowTreeSettingsDlg(int iSettingsMask)
{
    int mask = iSettingsMask;

    if ( !(mask & (FILTER_SETTINGS_MASK | DEINTERLACE_SETTINGS_MASK | ADVANCED_SETTINGS_MASK)) )
    {
        mask = FILTER_SETTINGS_MASK | DEINTERLACE_SETTINGS_MASK | ADVANCED_SETTINGS_MASK;
    }

    CTreeSettingsDlg dlg("DScaler settings");

    long i;

    SmartPtr<CTreeSettingsPage> RootPage(new CTreeSettingsPage("Filter settings", IDD_TREESETTINGS_EMPTY));

    //the default help id is HID_BASE_RESOURCE+dialog template id
    //but we cant use that for empty pages and the generic property page
    //so set a new help id to use insted.
    //since the IDH_FILTERS already contains HID_BASE_RESOURCE, subtract that
    RootPage->SetHelpID(IDH_FILTERS);

    if (mask & FILTER_SETTINGS_MASK)
    {
        int Root = dlg.AddPage(RootPage);
        vector< SmartPtr<CSettingsHolder> > Holders;
        vector< string > Names;
        GetFilterSettings(Holders, Names);
        for(i = 0; i < Holders.size(); i++)
        {
            SmartPtr<CTreeSettingsGeneric> pPage = new CTreeSettingsGeneric(Names[i], Holders[i]);
            pPage->SetHelpID(Holders[i]->GetHelpID());
            dlg.AddPage(pPage, Root);
        }
    }

    SmartPtr<CTreeSettingsPage> DeintRootPage(new CTreeSettingsPage("Video Methods", IDD_TREESETTINGS_EMPTY));
    DeintRootPage->SetHelpID(IDH_DEINTERLACE);

    if (mask & DEINTERLACE_SETTINGS_MASK)
    {
        int Root = dlg.AddPage(DeintRootPage);
        vector< SmartPtr<CSettingsHolder> > Holders;
        vector< string > Names;
        GetDeinterlaceSettings(Holders, Names);
        for(i = 0; i < Holders.size(); i++)
        {
            SmartPtr<CTreeSettingsGeneric> pPage = new CTreeSettingsGeneric(Names[i], Holders[i]);
            pPage->SetHelpID(Holders[i]->GetHelpID());
            dlg.AddPage(pPage, Root);
        }
    }

    SmartPtr<CTreeSettingsPage> AdvRootPage(new CTreeSettingsPage("Advanced Settings", IDD_TREESETTINGS_EMPTY));
    AdvRootPage->SetHelpID(IDH_ADVANCED);

    if (mask & ADVANCED_SETTINGS_MASK)
    {
        int Root = dlg.AddPage(AdvRootPage);
        SmartPtr<CTreeSettingsPage> pPage;
        SmartPtr<CSettingsHolder> Holder;

        if (Providers_GetCurrentSource())
        {
            Holder = Providers_GetCurrentSource()->GetSettingsPage();
            if (Holder)
            {
                string SettingsName(MakeString() << Providers_GetCurrentSource()->IDString() << " Advanced");
                pPage = new CTreeSettingsGeneric(SettingsName, Holder);
                dlg.AddPage(pPage, Root);
            }
        }

        Holder = SettingsMaster->FindMsgHolder(WM_FD50_GETVALUE);
        pPage = new CTreeSettingsGeneric("2:2 Pulldown Settings", Holder);
        pPage->SetHelpID(IDH_22_PULLDOWN);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_FD60_GETVALUE);
        pPage = new CTreeSettingsGeneric("3:2 Pulldown Settings", Holder);
        pPage->SetHelpID(IDH_32_PULLDOWN);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_FD_COMMON_GETVALUE);
        pPage = new CTreeSettingsGeneric("Pulldown Shared Settings", Holder);
        pPage->SetHelpID(IDH_PULLDOWN_SHARED);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_FDPROG_GETVALUE);
        pPage = new CTreeSettingsGeneric("Progressive Pulldown Settings", Holder);
        pPage->SetHelpID(IDH_PULLDOWN_SHARED);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_ASPECT_GETVALUE);
        pPage = new CTreeSettingsGeneric("Aspect Ratio Settings", Holder);
        pPage->SetHelpID(IDH_ASPECT);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_TIMING_GETVALUE);
        pPage = new CTreeSettingsGeneric("Field Timing Settings", Holder);
        pPage->SetHelpID(IDH_TIMING);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_OUTTHREADS_GETVALUE);
        pPage = new CTreeSettingsGeneric("Decoding / Output Settings", Holder);
        pPage->SetHelpID(IDH_ADVANCED);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_ANTIPLOP_GETVALUE);
        pPage = new CTreeSettingsGeneric("Anti Plop Settings", Holder);
        // \todo AntiPlop Help
        pPage->SetHelpID(IDH_ADVANCED);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_OSD_GETVALUE);
        pPage = new CTreeSettingsGeneric("OSD Settings", Holder);
        pPage->SetHelpID(IDH_ADVANCED);
        dlg.AddPage(pPage, Root);

        pPage = new CTreeSettingsGeneric("Teletext Settings", VideoText_GetSettingsPage());
        pPage->SetHelpID(IDH_TELETEXT);
        dlg.AddPage(pPage, Root);

        pPage = new CTreeSettingsGeneric("Threads Priority Settings", DScaler_GetSettingsPage1());
        pPage->SetHelpID(IDH_ADVANCED);
        dlg.AddPage(pPage, Root);

        pPage =  new CTreeSettingsGeneric("Activate Setting's Saving", SettingsMaster->GetSettingsPage());
        pPage->SetHelpID(IDH_SETTINGSBYCHANNEL);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_STILL_GETVALUE);
        pPage = new CTreeSettingsGeneric("Still Settings", Holder);
        pPage->SetHelpID(IDH_STILL);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_CALIBR_GETVALUE);
        pPage = new CTreeSettingsGeneric("Card Calibration Settings", Holder);
        pPage->SetHelpID(IDH_CALIBRATION);
        dlg.AddPage(pPage, Root);

        pPage = new CTreeSettingsGeneric("Overlay Settings", Overlay_GetSettingsPage());
        pPage->SetHelpID(IDH_OVERLAY);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_DEBUG_GETVALUE);
        pPage = new CTreeSettingsGeneric("Logging Settings", Holder);
        pPage->SetHelpID(IDH_LOGGING);
        dlg.AddPage(pPage, Root);

        pPage = new CTreeSettingsGeneric("Channel Settings", DScaler_GetSettingsPage3());
        pPage->SetHelpID(IDH_ADVANCED);
        dlg.AddPage(pPage, Root);

        pPage = new CTreeSettingsGeneric("PowerStrip Settings", DScaler_GetSettingsPage4());
        pPage->SetHelpID(IDH_ADVANCED);
        dlg.AddPage(pPage, Root);

        pPage = new CTreeSettingsGeneric("Other Settings", DScaler_GetSettingsPage2());
        pPage->SetHelpID(IDH_ADVANCED);
        dlg.AddPage(pPage, Root);

        Holder = SettingsMaster->FindMsgHolder(WM_EPG_GETVALUE);
        pPage = new CTreeSettingsGeneric("EPG Settings", Holder);
        pPage->SetHelpID(IDH_EPG);
        dlg.AddPage(pPage, Root);

        Holder = new CSettingsHolder;
        pPage = new CTreeSettingsGeneric("Direct3D Settings", Holder);
        pPage->SetHelpID(IDH_D3D9);
        dlg.AddPage(pPage, Root);
    }

    dlg.DoModal(GetMainWnd());
}

int CTreeSettingsDlg::AddPages(CComPtr<ISpecifyPropertyPages> SpecifyPages, int Parent)
{
    return 0;
}

BOOL CTreeSettingsDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
        HANDLE_MSG(hDlg, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hDlg, WM_SIZE, OnSize);
        HANDLE_MSG(hDlg, WM_PAINT, OnPaint);
        HANDLE_MSG(hDlg, WM_GETMINMAXINFO, OnGetMinMaxInfo);
    default:
        return FALSE;
    }
}

void CTreeSettingsDlg::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDOK:
        OnOK(hDlg);
        break;
    case IDCANCEL:
        OnCancel(hDlg);
        break;
    }
}

LRESULT CTreeSettingsDlg::OnNotify(HWND hwnd, int id, LPNMHDR nmhdr)
{
    switch(id)
    {
    case IDC_TREESETTINGS_TREE:
        if(nmhdr->code == TVN_SELCHANGING)
        {
            return OnSelchangingTree(hwnd, nmhdr);
        }
        break;
    default:
        break;
    }
    return FALSE;
}


#ifdef __TODO__
void CTreeSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDSDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTreeSettingsDlg)
    DDX_Control(pDX, IDC_TREESETTINGS_TREE, m_tree);
    DDX_Control(pDX, IDC_TREESETTINGS_STATIC, m_PageHeader);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTreeSettingsDlg, CDSDialog)
    //{{AFX_MSG_MAP(CTreeSettingsDlg)
    ON_NOTIFY(TVN_SELCHANGING, IDC_TREESETTINGS_TREE, OnSelchangingTree)
    ON_BN_CLICKED(IDC_HELPBTN, OnHelpBtn)
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_WM_GETMINMAXINFO()
//    ON_WM_NCHITTEST()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsDlg message handlers

void CTreeSettingsDlg::OnOK(HWND hDlg)
{
    int i;
    for(i=0;i<m_pages.size();i++)
    {
        //check if the page has been created
        if(::IsWindow(m_pages[i].m_hWnd))
        {
            m_pages[i].m_pPage->OnOK(m_pages[i].m_hWnd);
        }
    }

    EndDialog(hDlg, IDOK);
}

void CTreeSettingsDlg::OnCancel(HWND hDlg)
{
    int i;
    for(i=0;i<m_pages.size();i++)
    {
        //call OnCancel only on pages that has been created (selected atleast once)
        if(IsWindow(m_pages[i].m_hWnd))
        {
            m_pages[i].m_pPage->OnCancel(m_pages[i].m_hWnd);
        }
    }

    EndDialog(hDlg, IDCANCEL);
}

LRESULT CTreeSettingsDlg::OnSelchangingTree(HWND hDlg, NMHDR* pNMHDR)
{
    NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
    int iPage=pNMTreeView->itemNew.lParam;
    if(m_iCurrentPage!=iPage)
    {
        if(!ShowPage(hDlg, iPage))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CTreeSettingsDlg::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    SetWindowText(hDlg, m_settingsDlgCaption.c_str());

    m_StaticFont = CreateFont(18,0,0,0,900,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,"Arial");
    SetWindowFont(GetDlgItem(hDlg, IDC_TREESETTINGS_STATIC), m_StaticFont, TRUE);

    //don't know if m_PageHeader:s default colors looks good on all versions of windows
    //the comented lines below changes the gradient to a black and white gradient with white text
    /*m_PageHeader.SetGradientColor(RGB(0,0,0),RGB(255,255,255));
    m_PageHeader.SetTextColor(RGB(255,255,255));*/

    HWND hTree = GetDlgItem(hDlg, IDC_TREESETTINGS_TREE);

    TreeView_SetImageList(hTree, NULL, TVSIL_NORMAL);

    int ExpandPage = m_iStartPage;
    if ((ExpandPage<0) || (ExpandPage>=m_pages.size()))
    {
        ExpandPage = 0;
    }

    for(int i=0;i<m_pages.size();i++)
    {
        TVINSERTSTRUCT TVItem;
        memset(&TVItem, 0, sizeof(TVINSERTSTRUCT));
        TVItem.hParent = TVI_ROOT;
        if(m_pages[i].m_parent>=0)
        {

            if(m_pages[i].m_parent<m_pages.size())
            {
                TVItem.hParent = m_pages[m_pages[i].m_parent].m_hTreeItem;
            }
        }
        TVItem.hInsertAfter = TVI_LAST;
        TVItem.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
        TVItem.item.pszText = (LPSTR)m_pages[i].m_pPage->GetName().c_str();
        TVItem.item.lParam = i;
        TVItem.item.state = (i==ExpandPage)?TVIS_EXPANDED:0;
        TVItem.item.stateMask = TVIS_EXPANDED;

        m_pages[i].m_hTreeItem = TreeView_InsertItem(hTree, &TVItem);
    }

    //show the first page
    if(m_iStartPage>=0 && m_iStartPage<m_pages.size())
    {
        ShowPage(hDlg, m_iStartPage);
    }
    else
    {
        ShowPage(hDlg, 0);
    }

    RECT rect;
    GetWindowRect(hDlg, &rect);
    m_MinMaxInfo.x = rect.right - rect.left;
    m_MinMaxInfo.y = rect.bottom - rect.top;

    // repaint the dialog to get gripper
    InvalidateRect(hDlg, NULL, TRUE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CTreeSettingsDlg::ShowPage(HWND hDlg, int iPage)
{
    if(iPage<0 || iPage>=m_pages.size())
    {
        return FALSE;
    }
    HWND hPageFrame = GetDlgItem(hDlg, IDC_TREESETTINGS_PAGEFRAME);
    _ASSERTE(hPageFrame!=NULL);

    CTreeSettingsPage *pNewPage=m_pages[iPage].m_pPage;
    //create the new page if nessesary
    if(m_pages[iPage].m_hWnd == NULL)
    {
        m_pages[iPage].m_hWnd = pNewPage->Create(hDlg);
        if(m_pages[iPage].m_hWnd == NULL)
        {
            //create failed
            return FALSE;
        }
    }
    //can the new page be activated?
    if(!pNewPage->OnSetActive(m_pages[iPage].m_hWnd))
    {
        return FALSE;
    }

    //deactivate current page
    CTreeSettingsPage *pCurrentPage=NULL;
    if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
    {
        pCurrentPage=m_pages[m_iCurrentPage].m_pPage;
        if(pCurrentPage->OnKillActive(m_pages[m_iCurrentPage].m_hWnd))
        {
            ShowWindow(m_pages[m_iCurrentPage].m_hWnd, SW_HIDE);
        }
        else
        {
            return FALSE;
        }
    }

    //show the new page
    ShowWindow(m_pages[iPage].m_hWnd, SW_SHOWNOACTIVATE);
    m_iCurrentPage=iPage;

    //make sure the new page is properly positioned
    RECT rect;
    GetClientRect(hDlg, &rect);
    PostMessage(hDlg, WM_SIZE,SIZE_RESTORED,MAKELPARAM(rect.right - rect.left,rect.bottom - rect.top));

    SetWindowText(GetDlgItem(hDlg, IDC_TREESETTINGS_STATIC), pNewPage->GetHeaderName().c_str());
    return TRUE;
}

void CTreeSettingsDlg::OnHelpBtn(HWND hDlg)
{
    /// \todo sort out help
    //send WM_COMMANDHELP (mfc internal message) to the currently active page
    //if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
    //{
    //    m_pages[m_iCurrentPage].m_pPage->SendMessage(WM_COMMANDHELP);
    //}

}

void CTreeSettingsDlg::OnSize(HWND hDlg, UINT nType, int cx, int cy)
{
    // redraw Gripper
    InvalidateRect(hDlg, &m_GripperRect, TRUE);

    if(m_iCurrentPage>=0 && m_iCurrentPage<m_pages.size())
    {
        RECT rect;
        int height;
        int width;

        int righthalf = cx/3;

        //active page
        HWND hPageFrame=GetDlgItem(hDlg, IDC_TREESETTINGS_PAGEFRAME);
        GetWindowRect(hPageFrame, &rect);
        ScreenToClient(hDlg, rect);
        rect.left = righthalf;
        rect.right = cx-10;
        rect.bottom = cy-50;
        MoveWindow(hPageFrame, rect);

        //check if the current size is enough for the active page
        int minWidth=0;
        int minHeight=0;
        m_pages[m_iCurrentPage].m_pPage->GetMinSize(m_pages[m_iCurrentPage].m_hWnd, minWidth, minHeight);
        int Width(rect.right - rect.left);
        int Height(rect.bottom - rect.top);
        if(Width < minWidth || Height < minHeight)
        {
            RECT dlgRect;
            GetWindowRect(hDlg, &dlgRect);

            //calculate by how much the size must be increased
            dlgRect.right += (minWidth-Width)>0 ? minWidth-Width : 0;
            dlgRect.bottom += (minHeight-Height)>0 ?minHeight-Height : 0;

            MoveWindow(hDlg, dlgRect);
            return;
        }

        MoveWindow(m_pages[m_iCurrentPage].m_hWnd, rect);
        InvalidateRect(m_pages[m_iCurrentPage].m_hWnd, NULL, TRUE);

        RECT gradient;
        HWND hPageHeader = GetDlgItem(hDlg, IDC_TREESETTINGS_STATIC);
        GetWindowRect(hPageHeader, &gradient);
        ScreenToClient(hDlg, gradient);
        gradient.left=rect.left;
        gradient.right=rect.right;
        MoveWindow(hPageHeader, gradient);
        InvalidateRect(hPageHeader, NULL, TRUE);

        //line
        RECT line;
        HWND hLine = GetDlgItem(hDlg, IDC_TREESETTINGS_LINE);
        GetWindowRect(hLine, &line);
        ScreenToClient(hDlg, line);
        height=line.bottom - line.top;
        rect.top=cy-40-height;
        rect.bottom=rect.top+height;
        MoveWindow(hLine, rect);

        //tree
        HWND hTree = GetDlgItem(hDlg, IDC_TREESETTINGS_TREE);
        GetWindowRect(hTree, &rect);
        ScreenToClient(hDlg, rect);
        rect.right = rect.left + righthalf - 15;
        rect.bottom=cy-40;
        MoveWindow(hTree, rect);

        //buttons
        HWND hOkBtn = GetDlgItem(hDlg, IDOK);
        HWND hCancelBtn = GetDlgItem(hDlg, IDCANCEL);
        HWND hHelpBtn = GetDlgItem(hDlg, IDC_HELPBTN);

        GetWindowRect(hHelpBtn, &rect);
        ScreenToClient(hDlg, rect);
        width= rect.right - rect.left;
        height= rect.bottom - rect.top;

        rect.right=cx-20;
        rect.left=rect.right-width;
        rect.top=cy-30;
        rect.bottom=rect.top+height;
        MoveWindow(hHelpBtn, rect);
        InvalidateRect(hHelpBtn, NULL, TRUE);

        rect.right=rect.left-5;
        rect.left=rect.right-width;
        MoveWindow(hCancelBtn, rect);
        InvalidateRect(hCancelBtn, NULL, TRUE);

        rect.right=rect.left-5;
        rect.left=rect.right-width;
        MoveWindow(hOkBtn, rect);
        InvalidateRect(hOkBtn, NULL, TRUE);
    }
}

// the next code based on free sources from
// www.codeguru.com
void CTreeSettingsDlg::OnPaint(HWND hDlg)
{
    RECT rc;
    GetClientRect(hDlg, &rc);

    rc.left = rc.right  - GetSystemMetrics(SM_CXHSCROLL);
    rc.top  = rc.bottom - GetSystemMetrics(SM_CYVSCROLL);

    m_GripperRect = rc;

    // paint the Gripper
    HDC dc = GetDC(hDlg);
    DrawFrameControl(dc, &rc, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
    DeleteDC(dc);
}

void CTreeSettingsDlg::OnGetMinMaxInfo(HWND hDlg, MINMAXINFO FAR* lpMMI)
{
    if( (m_MinMaxInfo.x > 0) && (m_MinMaxInfo.y > 0) )
    {
        lpMMI->ptMinTrackSize = m_MinMaxInfo;
    }
}
