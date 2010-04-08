/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file Splash.cpp Splash window
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Splash.h"
#include "DScaler.h"
#include "DebugLog.h"
#include "IOutput.h"

HWND SplashWnd = NULL;

INT_PTR CALLBACK SplashProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
            RECT ScreenRect;
            GetActiveOutput()->GetMonitorRect(GetMainWnd(), &ScreenRect);
            int Width = ScreenRect.right  - ScreenRect.left;
            int Height = ScreenRect.bottom - ScreenRect.top;
            hSplashBm = (HBITMAP)LoadImage(hDScalerInst, MAKEINTRESOURCE(IDB_STARTUP), IMAGE_BITMAP, 0, 0, LR_VGACOLOR);
            GetObject(hSplashBm, sizeof(BITMAP), (LPTSTR) &bm);

            SetWindowPos(hDlg, HWND_TOPMOST,
                ScreenRect.left + (Width - bm.bmWidth) / 2,
                ScreenRect.top + (Height - bm.bmHeight) / 2,
                bm.bmWidth,
                bm.bmHeight,
                SWP_SHOWWINDOW);
            SetTimer(hDlg, 2, 5000, NULL);
            InvalidateRect(hDlg, NULL, TRUE);
            return FALSE;
        }
        break;

    case WM_ERASEBKGND:
        hdc = BeginPaint(hDlg, &wps);
        hMemDC = CreateCompatibleDC(hdc);
        hOldBm = (HBITMAP)SelectObject(hMemDC, hSplashBm);
        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
        SelectObject(hMemDC, hOldBm);
        DeleteDC(hMemDC);
        InvalidateRect(GetDlgItem(hDlg, IDC_LIST1), NULL, FALSE);
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
    SplashWnd = CreateDialog(hResourceInst, MAKEINTRESOURCE(IDD_SPLASHBOX), NULL, SplashProc);
    ShowWindow(SplashWnd, SW_SHOW);
    UpdateWindow(SplashWnd);
}

void HideSplashScreen()
{
    if (SplashWnd != NULL)
    {
        EndDialog(SplashWnd, 0);
        SplashWnd  = NULL;
    }
}


void AddSplashTextLine(const TCHAR* szText)
{
    if(SplashWnd)
    {
        int nItem;
        HWND listWnd(GetDlgItem(SplashWnd, IDC_LIST1));
        nItem = ListBox_AddString(listWnd, szText);
        ListBox_SetCurSel(listWnd, nItem);
        // update the list box
        UpdateWindow(listWnd);
        // make sure we're on top
        // otherwise we don't get painted
        SetWindowPos(SplashWnd,HWND_TOPMOST,
                    0,0,0,0,
                    SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOACTIVATE);

   }
    LOG(1, szText);
}