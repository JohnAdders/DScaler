/////////////////////////////////////////////////////////////////////////////
// $Id: Splash.cpp,v 1.5 2001-07-12 16:16:40 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Jan 2001   John Adcock           Split into separate file
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "Splash.h"
#include "DScaler.h"
#include "DebugLog.h"

HWND SplashWnd = NULL;

BOOL APIENTRY SplashProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static HBITMAP hSplashBm;
    static HBITMAP hOldBm;
    static BITMAP bm;
    PAINTSTRUCT wps;
    HDC hdc;
    HDC hMemDC;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            int Width = GetSystemMetrics(SM_CXSCREEN);
            int Height = GetSystemMetrics(SM_CYSCREEN);
            hSplashBm = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_STARTUP), IMAGE_BITMAP, 0, 0, LR_VGACOLOR);
            GetObject(hSplashBm, sizeof(BITMAP), (LPSTR) &bm);

            SetWindowPos(hDlg, HWND_TOPMOST, 
                (Width - bm.bmWidth) / 2, 
                (Height - bm.bmHeight) / 2, 
                bm.bmWidth, 
                bm.bmHeight, 
                SWP_SHOWWINDOW);
            SetTimer(hDlg, 2, 5000, NULL);
            InvalidateRect(hDlg, NULL, TRUE);
            return TRUE;
        }
        break;

    case WM_ERASEBKGND:
        hdc = BeginPaint(hDlg, &wps);
        hMemDC = CreateCompatibleDC(hdc);
        hOldBm = (HBITMAP)SelectObject(hMemDC, hSplashBm);
        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
        SelectObject(hMemDC, hOldBm);
        DeleteDC(hMemDC);
        DeleteDC(hdc);
        EndPaint(hDlg, &wps);
        return TRUE;
        break;
    
    case WM_TIMER:
        if (wParam == 2)
        {
            SplashWnd  = NULL;
            DeleteObject(hSplashBm);
            EndDialog(hDlg, 0);
        }
        return FALSE;
    }
    return FALSE;
    UNREFERENCED_PARAMETER(lParam);
}

void ShowSpashScreen()
{
    SplashWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_SPLASHBOX), NULL, SplashProc);
}

void HideSplashScreen()
{
    if (SplashWnd != NULL)
    {
        EndDialog(SplashWnd, 0);
        SplashWnd  = NULL;
    }
}


void AddSplashTextLine(const char* szText)
{
    if(SplashWnd)
    {
        int nItem;
        nItem = ListBox_AddString(GetDlgItem(SplashWnd, IDC_LIST1), szText);
        ListBox_SetCurSel(GetDlgItem(SplashWnd, IDC_LIST1), nItem);
        InvalidateRect(GetDlgItem(SplashWnd, IDC_LIST1), NULL, TRUE);
        Sleep(20);
    }
    LOG(szText);
}