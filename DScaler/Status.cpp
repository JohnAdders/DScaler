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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.15  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.14  2003/01/26 10:34:57  tobbej
// changed statusbar updates from output thread to be thread safe (PostMessage instead of SendMessage)
//
// Revision 1.13  2002/03/17 10:00:25  robmuller
// Status bar text is updated when the status bar is hidden.
//
// Revision 1.12  2001/12/18 14:45:05  adcockj
// Moved to Common Controls status bar
//
// Revision 1.11  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.10  2001/11/09 14:19:34  adcockj
// Bug fixes
//
// Revision 1.9  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.8  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.7  2001/08/15 09:41:16  adcockj
// Fix for statusbar font
//
// Revision 1.6.2.2  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.6.2.1  2001/08/15 09:41:44  adcockj
// Fix for status bar font
//
// Revision 1.6  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.5  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

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
