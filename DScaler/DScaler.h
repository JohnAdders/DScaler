/////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.h,v 1.55 2005-03-23 14:20:56 adcockj Exp $
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

/** 
 * @file dscaler.h dscaler Header file
 */
 
#ifndef __DSCALER_H___
#define __DSCALER_H___

#include "settings.h"

// Get Hold of the DScaler.c file settings
SETTING* DScaler_GetSetting(DSCALER_SETTING Setting);
void DScaler_ReadSettingsFromIni();
void DScaler_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void DScaler_FreeSettings();
CTreeSettingsGeneric* DScaler_GetTreeSettingsPage();
CTreeSettingsGeneric* DScaler_GetTreeSettingsPage2();
CTreeSettingsGeneric* DScaler_GetTreeSettingsPage3();
CTreeSettingsGeneric* DScaler_GetTreeSettingsPage4();

void SetThreadProcessorAndPriority();

LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam);
LONG APIENTRY MainWndProcSafe(HWND hWnd, UINT message, UINT wParam, LONG lParam);

void MainWndOnCreate(HWND hWnd);
void MainWndOnInitBT(HWND hWnd);
void SaveWindowPos(HWND hWnd);
void SaveActualPStripTiming(HWND hPSWnd);
void UpdateWindowState();
void SetMenuAnalog();
void CleanUpMemory();

void Overlay_Stop(HWND hWnd);
void Overlay_Start(HWND hWnd);
void ShowText(HWND hWnd, LPCTSTR szText);

BOOL GetDisplayAreaRect(HWND hWnd, LPRECT lpRect, BOOL WithToolbar = FALSE);
void InvalidateDisplayAreaRect(HWND hWnd, LPRECT lpRect, BOOL bErase);
void AddDisplayAreaRect(HWND hWnd, LPRECT lpRect);

LPCSTR GetSkinDirectory();

HMENU GetSubMenuWithName(HMENU hMenu, int nPos, LPCSTR szMenuText);

HMENU GetFiltersSubmenu();
HMENU GetVideoDeinterlaceSubmenu();
HMENU GetChannelsSubmenu();
HMENU GetOSDSubmenu();
HMENU GetPatternsSubmenu();
HMENU GetVTCodepageSubmenu();
HMENU GetOutResoSubmenu();

void SetMixedModeMenu(HMENU hMenu, BOOL bShow);

void RedrawMenuBar(HMENU hChangedMenu);

void SetTrayTip(const char* ChannelName);

#define TIMER_STATUS        1
#define TIMER_STATUS_MS     2000

#define TIMER_MSP           8
#define TIMER_MSP_MS        1000

#define TIMER_MSP_INTERNAL  9

#define TIMER_CX2388X       10
#define TIMER_CX2388X_MS    250

#define TIMER_KEYNUMBER     99
#define TIMER_KEYNUMBER_MS  1000

#define TIMER_AUTOSAVE      55
#define TIMER_AUTOSAVE_MS   1500

#define TIMER_BOUNCE        43
#define TIMER_ORBIT         44

#define TIMER_HIDECURSOR    45
#define TIMER_HIDECURSOR_MS 5000

#define TIMER_VTFLASHER     67
#define TIMER_VTFLASHER_MS  500

#define TIMER_VTINPUT       68

#define TIMER_FINDPULL      46
#define TIMER_FINDPULL_MS   4000

#define TIMER_SLEEPMODE     73
#define TIMER_SLEEPMODE_MS  2000

#define TIMER_TAKESTILL     51

#define TIMER_TOOLBAR       52
#define TIMER_TOOLBAR_MS    500

enum eSMState
{
    SM_WaitMode,
    SM_ChangeMode,
    SM_UpdateMode
};

typedef struct
{
    eSMState State;
    int iPeriod;
    int Period;
    time_t SleepAt;
} TSMState;

#define UWM_VIDEOTEXT           WM_APP + 0x1000
#define UWM_INPUTSIZE_CHANGE    WM_APP + 0x1001
#define UWM_SQUAREPIXELS_CHECK  WM_APP + 0x1002
#define UWM_EVENTADDEDTOQUEUE   WM_APP + 0x1003
#define UWM_OSD                 WM_APP + 0x1004
#define UWM_DEINTERLACE_SETSTATUS WM_APP + 0x1005
#define UWM_SWITCH_WINDOW		WM_APP + 0x1006
#define UWM_OVERLAY_UPDATE		WM_APP + 0x1007

#define VTM_VTHEADERUPDATE      0x0001
#define VTM_VTPAGEUPDATE        0x0002
#define VTM_VTPAGEREFRESH       0x0003
#define VTM_VTCOMMENTUPDATE     0x0004

#define OSDM_DISPLAYUPDATE      0x0001

extern HINSTANCE hDScalerInst;
extern HINSTANCE hResourceInst;
HWND GetMainWnd();
void PostMessageToMainWindow(UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL IsMainWindowThread();
void WaitForMainWindowEvent();
void ResetMainWindowEvent();

extern HWND hPSWnd;
extern HCURSOR hCursorHand;

extern BOOL bShowCrashDialog;

extern BOOL bInMenuOrDialogBox;

extern BOOL bVTSingleKeyToggle;
extern BOOL bVTAutoCodePage;

extern BOOL bMinimized;

#endif
