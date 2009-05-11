/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file OpenDlg.cpp implementation file for COpenDlg
 */

#include "stdafx.h"
#include "dscaler.h"
#include "OpenDlg.h"
#include "..\DScalerRes\resource.h"
#include "crash.h"
#include "DynamicFunction.h"

/////////////////////////////////////////////////////////////////////////////
// COpenDlg dialog

COpenDlg::COpenDlg() :
    CDSDialog(MAKEINTRESOURCE(IDD_OPEN))
{
}


INT_PTR COpenDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    default:
        return FALSE;
    }
}


/////////////////////////////////////////////////////////////////////////////
// COpenDlg message handlers

BOOL COpenDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    // setup auto complete
    DynamicFunctionS2<HRESULT, HWND, DWORD> pSHAC(_T("SHLWAPI.DLL"), "SHAutoComplete");
    HRESULT hr;
    if(pSHAC)
    {
        //this only work in single threaded apartments
        //(same problem as with the file open dialogs)
        hr = pSHAC(GetDlgItem(hwnd, IDC_OPEN_FILE), SHACF_URLALL|SHACF_FILESYS_ONLY);
        _ASSERTE(SUCCEEDED(hr));
    }

    // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
    return TRUE;
}

void COpenDlg::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDOK:
        OnOK(hDlg);
        break;
    case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        break;
    case IDC_OPEN_BROWSE:
        OnBrowse(hDlg);
        break;
    }
}


void COpenDlg::OnOK(HWND hDlg)
{
    m_FileName = GetDlgItemString(hDlg, IDC_OPEN_FILE);
    EndDialog(hDlg, IDOK);
}

void COpenDlg::OnBrowse(HWND hDlg)
{
    OPENFILENAME OpenFileInfo;
    TCHAR FilePath[MAX_PATH];
    TCHAR* FileFilters;
    FileFilters =
                            _T("All Files\0*.*;\0")
    #ifndef WANT_DSHOW_SUPPORT
                            _T("All Supported Files\0*.d3u;*.pat;*.tif;*.tiff;*.jpg;*.jpeg;\0")
    #else
                            _T("All Supported Files\0*.d3u;*.pat;*.tif;*.tiff;*.jpg;*.jpeg;*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv;*.grf\0")
    #endif
                            _T("TIFF Files\0*.tif;*.tiff\0")
                            _T("JPEG Files\0*.jpg;*.jpeg\0")
                            _T("DScaler Playlists\0*.d3u\0")
                            _T("DScaler Patterns\0*.pat\0")
    #ifdef WANT_DSHOW_SUPPORT
                            _T("Media Files (*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv)\0*.avi;*.mpg;*.mpeg;*.mpe;*.asf;*.wmv\0")
                            _T("GraphEdit Filter Graphs (*.grf)\0*.grf\0")
    #endif
                            ;

    FilePath[0] = 0;
    ZeroMemory(&OpenFileInfo,sizeof(OpenFileInfo));
    OpenFileInfo.lStructSize = sizeof(OpenFileInfo);
    OpenFileInfo.hwndOwner = hDlg;
    OpenFileInfo.lpstrFilter = FileFilters;
    OpenFileInfo.nFilterIndex = 2;
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
        SetWindowText(GetDlgItem(hDlg, IDC_OPEN_FILE), FilePath);
    }
}
