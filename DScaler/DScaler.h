/////////////////////////////////////////////////////////////////////////////
// dTV.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Lo
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
// 20 Feb 2001   Michael Samblanet     Added TIMER_BOUNCE, TIMER_BOUNCE_MS
// 21 Feb 2001   Michael Samblanet     Removed bounce timer - added as setting
// 23 Feb 2001   Michael Samblanet     Added TIMER_ORBIT
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DSCALER_H___
#define __DSCALER_H___

#include "settings.h"

// Get Hold of the DScaler.c file settings
SETTING* DScaler_GetSetting(DSCALER_SETTING Setting);
void DScaler_ReadSettingsFromIni();
void DScaler_WriteSettingsToIni();

void SetThreadProcessorAndPriority();

LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam);
LONG APIENTRY MainWndProcSafe(HWND hWnd, UINT message, UINT wParam, LONG lParam);

void MainWndOnCreate(HWND hWnd);
void MainWndOnInitBT(HWND hWnd);
void SaveWindowPos(HWND hWnd);
void UpdateWindowState();
void SetMenuAnalog();
void CleanUpMemory();

void Overlay_Stop(HWND hWnd);
void Overlay_Start(HWND hWnd);
void ShowText(HWND hWnd, LPCTSTR szText);
BOOL IsStatusBarVisible();

extern HMENU GetFiltersSubmenu();
extern HMENU GetVideoDeinterlaceSubmenu();
extern HMENU GetChannelsSubmenu();
extern HMENU GetOSDSubmenu1();
extern HMENU GetOSDSubmenu2();

#define TIMER_STATUS        1
#define TIMER_STATUS_MS     2000

#define TIMER_KEYNUMBER     99
#define TIMER_KEYNUMBER_MS  1000

#define TIMER_MSP           8
#define TIMER_MSP_MS        10000

#define TIMER_AUTOSAVE      55
#define TIMER_AUTOSAVE_MS   1500

#define TIMER_BOUNCE        43
#define TIMER_ORBIT         44

#define TIMER_HIDECURSOR	45
#define TIMER_HIDECURSOR_MS	5000

extern HINSTANCE hInst;
extern HWND hWnd;

extern int pgsizex;
extern int pgsizey;
extern int pgstartx;
extern int pgstarty;

extern BOOL bIgnoreMouse;

#endif
