/////////////////////////////////////////////////////////////////////////////
// $Id: OpenDlg.cpp,v 1.1 2003-02-22 16:45:02 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Torbjörn Jansson.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

/**
 * @file OpenDlg.cpp implementation file for COpenDlg
 */

#include "stdafx.h"
#include "dscaler.h"
#include "OpenDlg.h"
#include "..\DScalerRes\resource.h"
#include "crash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenDlg dialog

COpenDlg::COpenDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpenDlg::IDD, pParent),m_hSHLWAPIDLL(NULL)
{
	//{{AFX_DATA_INIT(COpenDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COpenDlg::~COpenDlg()
{
	if(m_hSHLWAPIDLL!=NULL)
	{
		FreeLibrary(m_hSHLWAPIDLL);
		m_hSHLWAPIDLL=NULL;
	}
}

void COpenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenDlg)
	DDX_Control(pDX, IDC_OPEN_FILE, m_File);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenDlg, CDialog)
	//{{AFX_MSG_MAP(COpenDlg)
	ON_BN_CLICKED(IDC_OPEN_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenDlg message handlers

BOOL COpenDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetupAutoComplete();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpenDlg::SetupAutoComplete()
{
	typedef HRESULT (WINAPI *SHAUTOCOMPLETEFN) (HWND hTarget, DWORD dwFlags);
	if(m_File.m_hWnd==NULL)
	{
		return;
	}
	if(m_hSHLWAPIDLL==NULL)
	{
		m_hSHLWAPIDLL=LoadLibrary("SHLWAPI.DLL");
		if(m_hSHLWAPIDLL==NULL)
		{
			return;
		}
	}
	HRESULT hr;
	SHAUTOCOMPLETEFN pSHAC=(SHAUTOCOMPLETEFN)GetProcAddress(m_hSHLWAPIDLL,"SHAutoComplete");
	if(pSHAC!=NULL)
	{
		//this only work in singel threaded apartments
		//(same problem as with the file open dialogs)
		hr=pSHAC(m_File.m_hWnd, SHACF_URLALL|SHACF_FILESYS_ONLY);
		ASSERT(SUCCEEDED(hr));
	}
}

void COpenDlg::OnOK()
{
	m_File.GetWindowText(m_FileName);
	CDialog::OnOK();
}

bool COpenDlg::ShowOpenDialog(HWND hParent,CString &FileName)
{
	COpenDlg dlg(CWnd::FromHandle(hParent));
	
	if(dlg.DoModal()==IDOK)
	{
		FileName=dlg.m_FileName;
		return true;
	}
	else
	{
		return false;
	}
}

void COpenDlg::OnBrowse() 
{
	OPENFILENAME OpenFileInfo;
	char FilePath[MAX_PATH];
	char* FileFilters;
	FileFilters =
							"All Files\0*.*;\0"
	#ifndef WANT_DSHOW_SUPPORT
							"All Supported Files\0*.d3u;*.pat;*.tif;*.tiff;*.jpg;*.jpeg;\0"
	#else
							"All Supported Files\0*.d3u;*.pat;*.tif;*.tiff;*.jpg;*.jpeg;*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv;*.grf\0"
	#endif
							"TIFF Files\0*.tif;*.tiff\0"
							"JPEG Files\0*.jpg;*.jpeg\0"
							"DScaler Playlists\0*.d3u\0"
							"DScaler Patterns\0*.pat\0"
	#ifdef WANT_DSHOW_SUPPORT
							"Media Files (*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv)\0*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv\0"
							"GraphEdit Filter Graphs (*.grf)\0*.grf\0"
	#endif
							;

	FilePath[0] = 0;
	ZeroMemory(&OpenFileInfo,sizeof(OpenFileInfo));
	OpenFileInfo.lStructSize = sizeof(OpenFileInfo);
	OpenFileInfo.hwndOwner = m_hWnd;
	OpenFileInfo.lpstrFilter = FileFilters;
	OpenFileInfo.nFilterIndex = 1;
	OpenFileInfo.lpstrCustomFilter = NULL;
	OpenFileInfo.lpstrFile = FilePath;
	OpenFileInfo.nMaxFile = sizeof(FilePath);
	OpenFileInfo.lpstrFileTitle = NULL;
	OpenFileInfo.lpstrInitialDir = NULL;
	OpenFileInfo.lpstrTitle = NULL;
	OpenFileInfo.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	OpenFileInfo.lpstrDefExt = NULL;
	if(GetOpenFileName(&OpenFileInfo))
	{
		m_File.SetWindowText(FilePath);
	}
}
