/////////////////////////////////////////////////////////////////////////////
// $Id: OSD.h,v 1.9 2001-11-02 16:30:08 adcockj Exp $
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 8 Nov 2000    Michael Eskin         Initial onscreen display
//
// 28 Nov 2000   Mark Rejhon           Reorganization and visual improvements
//
// 24 Feb 2001   Michael Samblanet     Added invalidation rectangle
//
// 25 Feb 2001   Laurent Garnier       Added management of multiple OSD texts
//
// 03 Mar 2001   Laurent Garnier       Added functions OSD_ShowInfosScreen
//                                     and OSD_GetLineYpos
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __OSD_H___
#define __OSD_H___

#include "settings.h"

// Get Hold of the OSD.c file settings
SETTING* OSD_GetSetting(OSD_SETTING Setting);
void OSD_ReadSettingsFromIni();
void OSD_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void OSD_SetMenu(HMENU hMenu);
BOOL ProcessOSDSelection(HWND hWnd, WORD wMenuID);


// Make sure that the timer ID does not conflict with those in DTV.H
#define OSD_TIMER_ID            42
#define OSD_TIMER_DELAY         4000
#define OSD_TIMER_REFRESH_ID    41
#define OSD_TIMER_REFRESH_DELAY 750

#define OSD_MAX_TEXT            64

// Values for "ShowType" parameter in functions OSD_Show and OSD_RefreshInfosScreen
#define OSD_PERSISTENT      1
#define OSD_AUTOHIDE        2
#define OSD_REFRESH_DATA    3

enum eOSDTextXPos
{
    OSD_XPOS_LEFT = 0,
    OSD_XPOS_RIGHT,
    OSD_XPOS_CENTER,
};

typedef struct
{
    char            szText[512];        // Text of OSD
    double          Size;               // Size of OSD as percentage of screen height
    long            TextColor;          // Text color (RGB)
    long            BackgroundColor;    // Background color (RGB)
    eOSDBackground  BackgroundMode;     // Background mode
    eOSDTextXPos    TextXPos;           // Text position / Xpos
    double          XPos;               // X position (0 = left, 1 = right)
    double          YPos;               // Y position (0 = top, 1 = bottom)
    RECT            CurrentRect;        // MRS 2-24-01 Saves the current drawn rectangle (used to limit invalidation area)
} TOsdInfo;

void OSD_ClearAllTexts();
void OSD_AddText(LPCTSTR szText, double Size, long NewTextColor, long BackgroundColor, eOSDBackground BackgroundMode, eOSDTextXPos TextXPos, double XPos, double YPos);
void OSD_Show(HWND hWnd, int ShowType, int refresh_delay);
void OSD_ShowText(HWND hWnd, LPCTSTR szText, double Size);
void OSD_ShowTextPersistent(HWND hWnd, LPCTSTR szText, double Size);
void OSD_ShowTextOverride(HWND hWnd, LPCTSTR szText, double Size);
void OSD_Redraw(HWND hWnd, HDC hDC);
void OSD_Clear(HWND hWnd);
void OSD_RefreshInfosScreen(HWND hWnd, double Size, int ShowType);
void OSD_ShowNextInfosScreen(HWND hWnd, double Size);
void OSD_ShowInfosScreen(HWND hWnd, int IdxScreen, double Size);
void OSD_ActivateInfosScreen(HWND hWnd, int IdxScreen, double Size);
void OSD_UpdateMenu(HMENU hMenu);
void OSD_SetMenu(HMENU hMenu);

#endif
