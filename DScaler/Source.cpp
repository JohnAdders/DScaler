/////////////////////////////////////////////////////////////////////////////
// $Id: Source.cpp,v 1.20 2005-03-23 14:21:00 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.19  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.18  2003/08/15 16:51:11  laurentg
// New event type EVENT_NO_VOLUME
// Update the volume toolbar when exiting from the audio mixer setup dialog box
//
// Revision 1.17  2003/08/15 14:26:30  laurentg
// Management of volume
//
// Revision 1.16  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.15  2003/01/12 16:19:35  adcockj
// Added SettingsGroup activity setting
// Corrected event sequence and channel change behaviour
//
// Revision 1.14  2003/01/10 17:38:33  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.13  2002/12/10 12:58:07  adcockj
// Removed NotifyInputChange and NotifyVideoFormatChange functions and replaced with
//  calls to EventCollector->RaiseEvent
//
// Revision 1.12  2002/10/22 04:08:50  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.11  2002/10/17 13:31:37  adcockj
// Give Holo3d different menu and updated settings
//
// Revision 1.10  2002/09/28 13:31:41  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.9  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.8  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.7  2002/05/06 15:38:50  laurentg
// Comments for a source
//
// Revision 1.6  2002/04/15 22:50:09  laurentg
// Change again the available formats for still saving
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.5  2002/02/22 23:22:23  laurentg
// Only notify size change if it concerns current source
//
// Revision 1.4  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.3  2001/11/25 21:19:40  laurentg
// New method GetMenuLabel and method GetMenu renamed GetSourceMenu
//
// Revision 1.2  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.1  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.4  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.3  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.2  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file Source.cpp CSource Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Events.h"
#include "Source.h"
#include "DScaler.h"
#include "Providers.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"

CSource::CSource(long SetMessage, long MenuId) :
    CSettingsHolder(SetMessage),
    m_FieldFrequency(0),
    m_Comments("")
{
    m_hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(MenuId));
}

CSource::~CSource()
{
    DestroyMenu(m_hMenu);
}

double CSource::GetFieldFrequency()
{
    return m_FieldFrequency;
}

HMENU CSource::GetSourceMenu()
{
    return m_hMenu;
}

char* CSource::GetComments()
{
    return (char*)m_Comments.c_str();
}

void CSource::NotifySizeChange()
{
    if (Providers_GetCurrentSource() == this)
    {
    	//tell dscaler that size has changed, the real work will be done in the main message loop
	    PostMessageToMainWindow(UWM_INPUTSIZE_CHANGE,0,0);
    }
}

void CSource::NotifySquarePixelsCheck()
{
    if (Providers_GetCurrentSource() == this)
    {
    	// Tell dscaler to check whether "square pixels" AR mode must be on or off
        // The real work will be done in the main message loop
	    PostMessageToMainWindow(UWM_SQUAREPIXELS_CHECK,0,0);
    }
}

BOOL CSource::HasTuner()
{
    ITuner* tuner = GetTuner();
    if (NULL == tuner)
    {
        return FALSE;
    }
    //this may differ from base behaviour..
    if (GetTunerId() == TUNER_ABSENT)
    {
        return FALSE;
    }
    return (GetTuner()->GetTunerId() != TUNER_ABSENT);
}


void CSource::ChangeDefaultsForSetup(WORD Setup, BOOL bDontSetValue)
{
    if (Setup & SETUP_CHANGE_VIDEOINPUT)
    {
        ChangeDefaultsForVideoInput(bDontSetValue);
    }
    if (Setup & SETUP_CHANGE_VIDEOFORMAT)
    {
        ChangeDefaultsForVideoFormat(bDontSetValue);
    }
    if (Setup & SETUP_CHANGE_AUDIOINPUT)
    {
        ChangeDefaultsForAudioInput(bDontSetValue);
    }
}

void CSource::SetSourceAsCurrent()
{
    // may need to register settings in here
    // not sure yet

	if (GetVolume() == NULL)
	{
	    EventCollector->RaiseEvent(this, EVENT_NO_VOLUME, 0, 1);
	}
}

void CSource::UnsetSourceAsCurrent()
{
    SettingsMaster->SaveSettings();
}
