/////////////////////////////////////////////////////////////////////////////
// settings.h
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
// 21 Dec 2000   John Adcock           Added function to setup ini file name
//
// 05 Jan 2001   John Adcock           Added GetRefreshRate
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __SETTINGS_H___
#define __SETTINGS_H___

#include "DS_Control.h"
#include "DS_ApiCommon.h"

void DisplayUISubMenuAsDialog(UI_SUBMENU* pSubMenu);

/////////////////////////////////////////////////////////////////////////////
// Functions to manipulate settings structure
/////////////////////////////////////////////////////////////////////////////
long Setting_GetValue(SETTING* pSetting);
BOOL Setting_SetValue(SETTING* pSetting, long Value);
void Setting_SetDefault(SETTING* pSetting);
void Setting_SetupSlider(SETTING* pSetting, HWND hSlider);
HWND Setting_CreateControl(SETTING* pSetting, HWND hDlg, int* VertPos);
void Setting_SetControlValue(SETTING* pSetting, HWND hControl);
BOOL Setting_SetFromControl(SETTING* pSetting, HWND hControl);
void Setting_ReadFromIni(SETTING* pSetting);
void Setting_WriteToIni(SETTING* pSetting);
void Setting_OSDShow(SETTING* pSetting, HWND hWnd);
void Setting_Up(SETTING* pSetting);
void Setting_Down(SETTING* pSetting);
void Setting_ChangeValue(SETTING* pSetting, eCHANGEVALUE NewValue);
void Setting_SetSection(SETTING* pSetting, LPSTR NewValue);
LONG Settings_HandleSettingMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone);

// End of new UI code header

void SetIniFileForSettings(LPSTR Name);
LPCSTR GetIniFileForSettings();
void LoadSettingsFromIni();
void WriteSettingsToIni();
void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName);

//---------------------------------------------------------------------------
// 2000-12-19 Added by Mark Rejhon
// These are constants for the GetCurrentAdjustmentStepCount()
// function.  This is a feature to allow accelerated slider adjustments
// For example, adjusting Contrast or Brightness faster the longer you
// hold down the adjustment key.
#define ADJ_MINIMUM_REPEAT_BEFORE_ACCEL     6      // Minimum number of taps before acceleration begins
#define ADJ_KEYB_TYPEMATIC_REPEAT_DELAY     200    // Milliseconds threshold for consecutive keypress repeat
#define ADJ_KEYB_TYPEMATIC_ACCEL_STEP       2000   // Milliseconds between each acceleration of adjustment
#define ADJ_KEYB_TYPEMATIC_MAX_STEP         10     // Maximum adjustment step at one time
#define ADJ_BUTTON_REPRESS_REPEAT_DELAY     400    // Milliseconds threshold for consecutive button repress
#define ADJ_BUTTON_REPRESS_ACCEL_STEP       400    // Milliseconds between each acceleration of adjustment
#define ADJ_BUTTON_REPRESS_MAX_STEP         30     // Maximum adjustment step at one time


#endif
