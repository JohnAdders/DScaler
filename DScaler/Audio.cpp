/////////////////////////////////////////////////////////////////////////////
// $Id: Audio.cpp,v 1.18 2001-11-02 16:30:06 adcockj Exp $
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
// 11 Aug 2000   John Adcock           Moved Audio Functions in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 08 Jan 2001   John Adcock           Maybe fixed crashing bug
//
// 26 Feb 2001   Hermes Conrad         Sound Fixes
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.17  2001/08/30 10:08:24  adcockj
// Removed variable
//
// Revision 1.16.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.16.2.3  2001/08/17 16:35:13  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.16.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.16.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
>>>>>>> 1.16.2.4
// Revision 1.16  2001/08/05 16:31:55  adcockj
// Removed Fake MSP code
//
// Revision 1.15  2001/08/02 18:08:17  adcockj
// Made all logging code use new levels
//
// Revision 1.14  2001/08/02 07:45:10  adcockj
// Fixed problem with stereo settings
//
// Revision 1.13  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.12  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.11  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.10  2001/07/13 07:04:43  adcockj
// Attemp 1 at fixing MSP muting
//
// Revision 1.9  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "i2c.h"
#include "audio.h"
#include "Status.h"
#include "MixerDev.h"
#include "Providers.h"
#include "resource.h"


BOOL bSystemInMute = FALSE;

void Audio_Mute()
{
	if (bUseMixer == FALSE)
	{
        Providers_GetCurrentSource()->Mute();
	}

	if(bUseMixer == TRUE)
	{
		Mixer_Mute();
	}
}

void Audio_Unmute()
{
	if(!bSystemInMute)
	{
		if (bUseMixer == FALSE)
		{
            Providers_GetCurrentSource()->UnMute();
		}
    
		if(bUseMixer == TRUE)
		{
			Mixer_UnMute();
		}
	}
}

BOOL SystemInMute_OnChange(long NewValue)
{
	if(NewValue == TRUE)
	{
		bSystemInMute = TRUE;
		Audio_Mute();
	}
	else
	{
		bSystemInMute = FALSE;
		Audio_Unmute();
	}
	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING AudioSettings[AUDIO_SETTING_LASTONE] =
{
    {
        "System in Mute", ONOFF, 0, (long*)&bSystemInMute,
        FALSE, 0, 1, 1, 1, 
        NULL,
        NULL, NULL, SystemInMute_OnChange,
    },
};

SETTING* Audio_GetSetting(AUDIO_SETTING Setting)
{
    if(Setting > -1 && Setting < AUDIO_SETTING_LASTONE)
    {
        return &(AudioSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Audio_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < AUDIO_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(AudioSettings[i]));
    }
}

void Audio_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < AUDIO_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(AudioSettings[i]), bOptimizeFileAccess);
    }
}

void Audio_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_MUTE, bSystemInMute);
}

