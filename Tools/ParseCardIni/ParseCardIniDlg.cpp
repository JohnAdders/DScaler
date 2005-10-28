/////////////////////////////////////////////////////////////////////////////
// $Id: ParseCardIniDlg.cpp,v 1.1 2005-10-28 16:43:13 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Torsten Seeboth. All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ParseCardIni.h"
#include "ParseCardIniDlg.h"

#include "DoParseCX2388x.h"
#include "DoParseSAA713x.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParseCardIniDlg Dialogfeld

CParseCardIniDlg::CParseCardIniDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CParseCardIniDlg::IDD, pParent),
	m_MinMaxInfo(NULL),
	m_GripperRect(NULL)
{
	//{{AFX_DATA_INIT(CParseCardIniDlg)
	m_bSortByName = FALSE;
	//}}AFX_DATA_INIT
    
    m_LastIniFile.Empty();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CParseCardIniDlg::~CParseCardIniDlg()
{
}

void CParseCardIniDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CParseCardIniDlg)
	DDX_Control(pDX, IDC_CHK_SORT, m_ctrlSortByName);
	DDX_Control(pDX, IDC_LIST_ERROR, m_ctrlListError);
	DDX_Control(pDX, IDC_TREE, m_ctrlTreeCard);
	DDX_Check(pDX, IDC_CHK_SORT, m_bSortByName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CParseCardIniDlg, CDialog)
	//{{AFX_MSG_MAP(CParseCardIniDlg)
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_BN_CLICKED(ID_OPEN_INI, OnBtnOpenIni)
	ON_BN_CLICKED(IDC_CHK_SORT, OnChkSort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParseCardIniDlg Nachrichten-Handler

BOOL CParseCardIniDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	ModifyStyle(0, WS_CLIPCHILDREN);
	
	CRect rec;
	GetWindowRect(&rec);

	m_MinMaxInfo.x = rec.Width();
	m_MinMaxInfo.y = rec.Height();

	m_ImageList.Create(IDB_TREE_IMAGE_LIST, 16, 1, RGB(255, 255, 255));	
	m_ctrlTreeCard.SetImageList(&m_ImageList, LVSIL_NORMAL);

	return TRUE;
}

void CParseCardIniDlg::OnPaint() 
{
	CDialog::OnPaint();

	CRect rec;
	GetClientRect(rec);
	rec.left = rec.right  - GetSystemMetrics(SM_CXHSCROLL);
	rec.top  = rec.bottom - GetSystemMetrics(SM_CYVSCROLL);
	m_GripperRect = rec;

	CClientDC dc(this);
	dc.DrawFrameControl(rec, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
}

UINT CParseCardIniDlg::OnNcHitTest(CPoint point) 
{
	UINT ht = CDialog::OnNcHitTest(point);
	if(ht == HTCLIENT)
	{
		CRect rec;
		GetWindowRect(rec);
		rec.left = rec.right  - GetSystemMetrics(SM_CXHSCROLL);
		rec.top  = rec.bottom - GetSystemMetrics(SM_CYVSCROLL);
		
		if(rec.PtInRect(point))
		{
			ht = HTBOTTOMRIGHT;
		}
	}

	return ht;
}

void CParseCardIniDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	lpMMI->ptMinTrackSize = m_MinMaxInfo;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// the following #define's are used for resizing controls in dialog window
#define BTN_OK_OFFSET_TO_RIGHT			22
#define BTN_OK_OFFSET_TO_BOTTOM			12

#define BTN_OPENINI_OFFSET_TO_RIGHT		126
#define BTN_OPENINI_OFFSET_TO_BOTTOM	12

#define LIST_ERROR_OFFSET_TO_LEFT		9 
#define LIST_ERROR_OFFSET_TO_RIGHT		9
#define LIST_ERROR_OFFSET_TO_BOTTOM		44

#define STATIC_ERROR_OFFSET_TO_BOTTOM	109

#define TREE_OFFSET_TO_LEFT				9 
#define TREE_OFFSET_TO_RIGHT			9
#define TREE_OFFSET_TO_TOP				26
#define TREE_OFFSET_TO_BOTTOM			134

#define CHECK_OFFSET_TO_BOTTOM	        12



void CParseCardIniDlg::OnSize(UINT nType, int cx, int cy) 
{
	CRect rectDialog;
	GetClientRect(&rectDialog);

	// move the "OK" Button Ctrl
    if(GetDlgItem(IDOK)->GetSafeHwnd())
	{
		CRect rectCtrl;
		GetDlgItem(IDOK)->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);

		int iOldWidth  = rectCtrl.Width();
		int iOldHeight = rectCtrl.Height();

		rectCtrl.right  = cx - BTN_OK_OFFSET_TO_RIGHT;
		rectCtrl.left   = cx - BTN_OK_OFFSET_TO_RIGHT  - iOldWidth;

		rectCtrl.bottom = cy - BTN_OK_OFFSET_TO_BOTTOM;
		rectCtrl.top    = cy - BTN_OK_OFFSET_TO_BOTTOM - iOldHeight;

		GetDlgItem(IDOK)->MoveWindow(&rectCtrl);
	}

	// move the "Open ini file..." Button Ctrl
    if(GetDlgItem(ID_OPEN_INI)->GetSafeHwnd())
	{
		CRect rectCtrl;
		GetDlgItem(ID_OPEN_INI)->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);

		int iOldWidth  = rectCtrl.Width();
		int iOldHeight = rectCtrl.Height();

		rectCtrl.right  = cx - BTN_OPENINI_OFFSET_TO_RIGHT;
		rectCtrl.left   = cx - BTN_OPENINI_OFFSET_TO_RIGHT  - iOldWidth;

		rectCtrl.bottom = cy - BTN_OPENINI_OFFSET_TO_BOTTOM;
		rectCtrl.top    = cy - BTN_OPENINI_OFFSET_TO_BOTTOM - iOldHeight;

		GetDlgItem(ID_OPEN_INI)->MoveWindow(&rectCtrl);
		GetDlgItem(ID_OPEN_INI)->Invalidate();
	}
	
	// move the Error List Ctrl
	if(GetDlgItem(IDC_LIST_ERROR)->GetSafeHwnd())
	{
		CRect rectCtrl;
		GetDlgItem(IDC_LIST_ERROR)->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);

		int iOldHeight = rectCtrl.Height();

		rectCtrl.right  = cx - LIST_ERROR_OFFSET_TO_RIGHT;
		rectCtrl.left   = LIST_ERROR_OFFSET_TO_LEFT;

		rectCtrl.bottom = cy - LIST_ERROR_OFFSET_TO_BOTTOM;
		rectCtrl.top    = cy - LIST_ERROR_OFFSET_TO_BOTTOM - iOldHeight;

		GetDlgItem(IDC_LIST_ERROR)->MoveWindow(&rectCtrl);
		GetDlgItem(IDC_LIST_ERROR)->Invalidate();
	}

	// move the Error List Static Text Ctrl
	if(GetDlgItem(IDC_STATIC_ERROR)->GetSafeHwnd())
	{
		CRect rectCtrl;
		GetDlgItem(IDC_STATIC_ERROR)->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);

		int iOldHeight = rectCtrl.Height();

		rectCtrl.bottom = cy - STATIC_ERROR_OFFSET_TO_BOTTOM;
		rectCtrl.top    = cy - STATIC_ERROR_OFFSET_TO_BOTTOM - iOldHeight;

		GetDlgItem(IDC_STATIC_ERROR)->MoveWindow(&rectCtrl);
		GetDlgItem(IDC_STATIC_ERROR)->Invalidate();
	}
	
	// move the Card Tree Ctrl
	if(GetDlgItem(IDC_TREE)->GetSafeHwnd())
	{
		CRect rectCtrl;
		rectCtrl.right  = cx - TREE_OFFSET_TO_RIGHT;
		rectCtrl.left   = TREE_OFFSET_TO_LEFT;
		rectCtrl.top    = TREE_OFFSET_TO_TOP;
		rectCtrl.bottom = cy - TREE_OFFSET_TO_BOTTOM;

		GetDlgItem(IDC_TREE)->MoveWindow(&rectCtrl);
	}

	// move the "Sort by Name" Check Ctrl
	if(GetDlgItem(IDC_TREE)->GetSafeHwnd())
	{
		CRect rectCtrl;
		GetDlgItem(IDC_CHK_SORT)->GetWindowRect(&rectCtrl);
		ScreenToClient(&rectCtrl);

		int iOldHeight = rectCtrl.Height();

		rectCtrl.bottom = cy - CHECK_OFFSET_TO_BOTTOM;
		rectCtrl.top    = cy - CHECK_OFFSET_TO_BOTTOM - iOldHeight;

		GetDlgItem(IDC_CHK_SORT)->MoveWindow(&rectCtrl);
	}

    // Redraw the Gripper
    InvalidateRect(m_GripperRect);
	
    CDialog::OnSize(nType, cx, cy);
}

void CParseCardIniDlg::OnBtnOpenIni() 
{
	UpdateData(TRUE);

    CFileDialog FileOpenDlg( TRUE,	NULL, NULL,
 							 OFN_EXPLORER | OFN_PATHMUSTEXIST,
 							 "DScaler Card ini Files|SAA713xCards.ini;CX2388xCards.ini|"
 							 "All ini files (*.ini)|*.ini|"
 							 "All files (*.*)|*.*||", this );
 	
	if(FileOpenDlg.DoModal() == IDOK)
	{
        m_LastIniFile = FileOpenDlg.GetPathName();
        StartParseCardIni(m_bSortByName);
	}
}

void CParseCardIniDlg::OnChkSort() 
{
    if(!m_LastIniFile.IsEmpty())
    {
        StartParseCardIni(m_ctrlSortByName.GetCheck());
    }
}

void CParseCardIniDlg::StartParseCardIni(BOOL bSort)
{
    const char* pszCXCard  = "CX2388xCards.ini";
    const char* pszSAACard = "SAA713xCards.ini";
    
    char szFileName[_MAX_FNAME];
    char szFileExt[_MAX_EXT];
    
    // get filename and extension from path
    _splitpath(m_LastIniFile, NULL, NULL, szFileName, szFileExt);

    // add extension to filename
    strcat(szFileName, szFileExt);

    // get the chip type from filename
    if(strcmp(szFileName, pszCXCard) == 0)
	{
        CWaitCursor wait;
        CDoParseCX2388x(m_LastIniFile, &m_ctrlListError, &m_ctrlTreeCard, bSort);
	}

	else if(strcmp(szFileName, pszSAACard) == 0)
	{
        CWaitCursor wait;
		CDoParseSAA713x(m_LastIniFile, &m_ctrlListError, &m_ctrlTreeCard, bSort);
	}

    else
    {
        AfxMessageBox("Unknown card ini file.");
        m_LastIniFile.Empty();
    }
}
