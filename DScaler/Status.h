/////////////////////////////////////////////////////////////////////////////
// $Id: Status.h,v 1.6 2001-11-02 16:30:08 adcockj Exp $
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
// 11 Aug 2000   John Adcock           Reorganised code
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __STATUS_H___
#define __STATUS_H___

enum eStatusBarBox
{
    STATUS_TEXT,
    STATUS_PAL,
    STATUS_FPS,
    STATUS_KEY,
    STATUS_AUDIO,
    STATUS_BAR
};

BOOL StatusBar_Init();
BOOL StatusBar_IsVisible();
BOOL StatusBar_Adjust(HWND hWnd);
HWND StatusBar_AddField(int iId, int iMin, int iMax, BOOL bNewGroup);
void StatusBar_Destroy();
void StatusBar_ShowText(eStatusBarBox Field, LPCTSTR  szText);
void StatusBar_ShowWindow(BOOL bShow);
void StatusBar_Repaint();
int StatusBar_Height();
HWND StatusBar_GetHWnd(eStatusBarBox Box);


#endif
