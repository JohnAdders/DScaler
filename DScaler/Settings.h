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
 * @file settings.h settings Header file
 */

#ifndef __SETTINGS_H___
#define __SETTINGS_H___

#include "DS_Control.h"
#include "DS_ApiCommon.h"
#include "TreeSettingsGeneric.h"
/////////////////////////////////////////////////////////////////////////////
// Functions to manipulate settings structure
/////////////////////////////////////////////////////////////////////////////

long Setting_GetValue(long GetValueMsg, long SettingIndex);
void Setting_SetValue(long GetValueMsg, long SettingIndex, long Value);
void Setting_SetDefault(long GetValueMsg, long SettingIndex);
void Setting_ChangeDefault(long GetValueMsg, long SettingIndex, long Default);
void Setting_SetupSlider(long GetValueMsg, long SettingIndex, HWND hSlider);
void Setting_SetControlValue(long GetValueMsg, long SettingIndex, HWND hControl);
void Setting_SetFromControl(long GetValueMsg, long SettingIndex, HWND hControl);
void Setting_OSDShow(long GetValueMsg, long SettingIndex, HWND hWnd);
void Setting_Up(long GetValueMsg, long SettingIndex);
void Setting_Down(long GetValueMsg, long SettingIndex);
// End of new UI code header

void SetIniFileForSettings(LPSTR Name);
LPCSTR GetIniFileForSettings();
void WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, int nValue, LPCTSTR lpFileName);
int GetCurrentAdjustmentStepCount(void* pSetting);

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


#define CheckMenuItemBool(hMenu, MenuId, Condition) CheckMenuItem(hMenu, MenuId, (Condition)?MF_CHECKED:MF_UNCHECKED)
#define EnableMenuItemBool(hMenu, MenuId, Condition) EnableMenuItem(hMenu, MenuId, (Condition)?MF_ENABLED:MF_GRAYED)

void BeautifyIniFile(LPCTSTR lpIniFileName);

#endif
