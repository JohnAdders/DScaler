/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsPerChannel.h,v 1.9 2003-10-27 10:39:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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

/** 
 * @file settingsperchannel.h settingsperchannel Header file
 */
 
#ifndef __SETTINGSPERCHANNEL_H___
#define __SETTINGSPERCHANNEL_H___

#include "setting.h"

void SettingsPerChannel_ReadSettingsFromIni();
void SettingsPerChannel_WriteSettingsToIni(BOOL bOptimizeFileAccess);
SETTING* SettingsPerChannel_GetSetting(int iSetting);

BOOL SettingsPerChannel_IsPerChannel();
BOOL SettingsPerChannel_IsPerFormat();
BOOL SettingsPerChannel_IsPerInput();


#endif
