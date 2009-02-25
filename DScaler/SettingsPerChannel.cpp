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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.25  2003/01/10 17:38:32  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.24  2002/12/04 15:15:25  adcockj
// Removed  RegLog as it is now redundant
//
// Revision 1.23  2002/10/07 20:33:05  kooiman
// Fixed source==NULL bug.
//
// Revision 1.22  2002/10/02 10:52:35  kooiman
// Fixed C++ type casting for events.
//
// Revision 1.21  2002/09/29 13:56:30  adcockj
// Fixed some cursor hide problems
//
// Revision 1.20  2002/09/28 18:08:20  adcockj
// Fixed crashing due to bad cast
//
// Revision 1.19  2002/09/28 13:34:08  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.18  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.16  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.15  2002/09/15 15:00:05  kooiman
// Added check for Providers_GetCurrentSource() == NULL.
//
// Revision 1.14  2002/09/06 15:12:31  kooiman
// Get the correct video input in time.
//
// Revision 1.13  2002/09/02 19:07:21  kooiman
// Added BT848 advanced settings to advanced settings dialog
//
// Revision 1.12  2002/09/01 15:15:56  kooiman
// Fixed bug introduced by bug fix.
//
// Revision 1.11  2002/08/31 16:30:17  kooiman
// Fix duplicate names when changing sources.
//
// Revision 1.10  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.9  2002/08/21 20:27:13  kooiman
// Improvements and cleanup of some settings per channel code.
//
// Revision 1.8  2002/08/15 14:16:45  kooiman
// More small improvements & bug fixes
//
// Revision 1.7  2002/08/14 22:04:25  kooiman
// Bug fix
//
// Revision 1.6  2002/08/13 21:21:24  kooiman
// Improved settings per channel to account for source and input changes.
//
// Revision 1.5  2002/08/08 21:17:05  kooiman
// Fixed some bugs.
//
// Revision 1.4  2002/08/08 20:00:00 kooiman
// Revised version of settings per channel
//
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


void SettingsPerChannel_ReadSettingsFromIni()
{
    int i;
    
    // Common settings
    for (i = 0; i < SETTINGSPERCHANNEL_SETTING_LASTONE; i++)
    {       
       Setting_ReadFromIni(&SettingsPerChannel_CommonSettings[i]);
    }    
}

void SettingsPerChannel_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for (i = 0; i < SETTINGSPERCHANNEL_SETTING_LASTONE; i++)
    {       
       Setting_WriteToIni(&SettingsPerChannel_CommonSettings[i], bOptimizeFileAccess);
    }
}


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
