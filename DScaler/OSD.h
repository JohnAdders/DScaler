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
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file osd.h osd Header file
 */
 
#ifndef __OSD_H___
#define __OSD_H___

#include "settings.h"

// Make sure that the timer ID does not conflict with those in DTV.H
#define OSD_TIMER_ID                42
#define OSD_TIMER_DELAY             4000
#define OSD_TIMER_REFRESH_ID        41
#define OSD_TIMER_REFRESH_DELAY     750

#define OSD_MAX_TEXT                64

void OSD_Init();
void OSD_Exit();

// General use functions
void OSD_ShowText(const std::string& szText, double dSize, BOOL bPersistent = FALSE, BOOL bOverride = FALSE);
void OSD_ShowTextPersistent(const std::string& szText, double dSize);
void OSD_ShowTextOverride(const std::string& szText, double dSize);
void OSD_ShowSourceComments();
void OSD_Clear();

void OSD_ActivateInfosScreen(INT IdxScreen, DOUBLE Size);
void OSD_ShowInfosScreen(INT IdxScreen, DOUBLE dSize);
void OSD_ShowNextInfosScreen(DOUBLE dSize);

// Reserved functions
void OSD_ShowText(HDC, LPRECT, LPCTSTR szText, double dSize, BOOL bPersistent = FALSE);
void OSD_ShowInfosScreen(HDC, LPRECT, INT IdxScreen, DOUBLE Size);
void OSD_ShowNextInfosScreen(HDC, LPRECT, DOUBLE Size);
void OSD_Clear(HDC, LPRECT);

void OSD_InvalidateTextsArea();

void OSD_ProcessDisplayUpdate(HDC hDC, LPRECT lpRect);
void OSD_RefreshInfosScreen(HDC hDC, LPRECT lpRect, double Size);
void OSD_Redraw(HDC hDC, LPRECT lpRect);

LONG OSD_GetPaintedRects(RECT* pRectBuffer, LONG nBufferSize);
void OSD_ResetPaintedRects();


// Get Hold of the OSD.c file settings
SETTING* OSD_GetSetting(OSD_SETTING Setting);
void OSD_ReadSettingsFromIni();
void OSD_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* OSD_GetTreeSettingsPage();
void OSD_FreeSettings();

void OSD_UpdateMenu(HMENU hMenu);
void OSD_SetMenu(HMENU hMenu);
BOOL ProcessOSDSelection(HWND hWnd, WORD wMenuID);


#endif

