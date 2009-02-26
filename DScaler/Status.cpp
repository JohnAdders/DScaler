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
 * @file Status.cpp Status bar functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Status.h"
#include "DScaler.h"
#include "OutThreads.h"

HWND hwndStatusBar = NULL;

BOOL StatusBar_Init()
{
    hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, "Starting ...", GetMainWnd(), IDC_STATUSBAR);

    if (!hwndStatusBar)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL StatusBar_IsVisible()
{
    return IsWindowVisible(hwndStatusBar);
}

void StatusBar_ShowText(eStatusBarBox Box, LPCSTR szText)
{
    //this function shoud not be called from the output thread, use Deinterlace_SetStatus instead
    ASSERTONOUTTHREAD;
    SendMessage(hwndStatusBar, SB_SETTEXT, Box, (LPARAM) szText);
}


BOOL StatusBar_Adjust(HWND hWnd)
{
    RECT RectWnd;
    int Height = StatusBar_Height();

    GetClientRect(hWnd, &RectWnd);
        
    int Widths[STATUS_LASTONE] = {200, 400, 450, -1,};
    if(RectWnd.right - RectWnd.left > 550)
    {
        Widths[2] = RectWnd.right - RectWnd.left - 100;
    }
    SendMessage(hwndStatusBar, SB_SETPARTS, STATUS_LASTONE, (LPARAM) (LPINT) &Widths);

    MoveWindow(hwndStatusBar, RectWnd.left, RectWnd.bottom - Height, RectWnd.right - RectWnd.left, Height, TRUE);
    InvalidateRect(hwndStatusBar, NULL, FALSE);
    return TRUE;
}

void StatusBar_ShowWindow(BOOL bShow)
{
    ShowWindow(hwndStatusBar, (bShow)?SW_SHOW:SW_HIDE);
}

void StatusBar_Destroy(void)
{
    DestroyWindow(hwndStatusBar);   
    hwndStatusBar = NULL;
}

void StatusBar_Repaint()
{
    if (IsWindowVisible(hwndStatusBar))
    {
        //InvalidateRect(hwndStatusBar, NULL, TRUE);
    }
}

int StatusBar_Height()
{
    RECT Rect;
    GetClientRect(hwndStatusBar, &Rect);
    return (Rect.bottom - Rect.top);
}
