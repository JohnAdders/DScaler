/////////////////////////////////////////////////////////////////////////////
// $Id: VideoSettings.h,v 1.3 2001-07-12 16:16:40 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 29 Jan 2001   John Adcock           Original Release
//
// 31 Mar 2001   Laurent Garnier       Last used format saved per video input
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VIDEOSETTINGS_H___
#define __VIDEOSETTINGS_H___

#include "Settings.h"

SETTING* VideoSettings_GetSetting(VIDEOSETTINGS_SETTING Setting);
void VideoSettings_ReadSettingsFromIni();
void VideoSettings_WriteSettingsToIni();

BOOL APIENTRY VideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void VideoSettings_Load();
void VideoSettings_Save();
void VideoSettings_LoadTVFormat();
void VideoSettings_SaveTVFormat();
void VideoSettings_SaveCurrentToAllInputs();
void VideoSettings_SaveCurrentToAllFormats();
void VideoSettings_SetupDefaults();

#endif
