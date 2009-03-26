/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 DScaler team.  All rights reserved.
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
 * @file SettingsPerChannel.cpp Settings Per Channel Functions
 */

#include "stdafx.h"
#include <stack>
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "setting.h"
#include "settings.h"
#include "providers.h"
#include "DebugLog.h"
#include "ProgramList.h"
#include "Source.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BOOL bSpcChannelSpecific = FALSE;
BOOL bSpcVideoInputSpecific = TRUE;
BOOL bSpcVideoFormatSpecific = TRUE;

BOOL SettingsPerChannel_ChannelSpecific_Change(long NewValue)
{
    bSpcChannelSpecific = NewValue;
    return FALSE;
}

BOOL SettingsPerChannel_VideoInputSpecific_Change(long NewValue)
{
    bSpcVideoInputSpecific = NewValue;
    return FALSE;
}

BOOL SettingsPerChannel_VideoFormatSpecific_Change(long NewValue)
{
    bSpcVideoFormatSpecific = NewValue;
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
// On/off settings //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

SETTING SettingsPerChannel_CommonSettings[SETTINGSPERCHANNEL_SETTING_LASTONE] =
{
    {
        "Settings per channel", ONOFF, 0, (long*)&bSpcChannelSpecific, 
        0, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerChannelEnabled", SettingsPerChannel_ChannelSpecific_Change,
    },
    {
        "Save per input", ONOFF, 0, (long*)&bSpcVideoInputSpecific,
        1, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerVideoInput", SettingsPerChannel_VideoInputSpecific_Change,
    },
    {
        "Save per Format", ONOFF, 0, (long*)&bSpcVideoFormatSpecific,
        1, 0, 1, 1, 1, NULL,
        "SettingsPerChannel", "SettingsPerVideoFormat", SettingsPerChannel_VideoFormatSpecific_Change,
    },
};

SETTING* SettingsPerChannel_GetSetting(int Setting)
{
    if(Setting > -1 && Setting < SETTINGSPERCHANNEL_SETTING_LASTONE)
    {
        return &(SettingsPerChannel_CommonSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

BOOL SettingsPerChannel_IsPerChannel()
{
    return bSpcChannelSpecific;
}

BOOL SettingsPerChannel_IsPerFormat()
{
    return bSpcVideoFormatSpecific;
}

BOOL SettingsPerChannel_IsPerInput()
{
    return bSpcVideoInputSpecific;
}
