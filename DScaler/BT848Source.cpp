/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source.cpp,v 1.132 2004-05-12 16:52:42 adcockj Exp $
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
// Revision 1.131  2003/11/14 13:24:55  adcockj
// PMS card fixes
//
// Revision 1.130  2003/11/06 19:36:56  adcockj
// Increase size of vertical delay
//
// Revision 1.129  2003/10/27 16:22:56  adcockj
// Added preliminary support for PMS PDI Deluxe card
//
// Revision 1.128  2003/10/27 10:39:50  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.127  2003/08/15 18:20:26  laurentg
// Save in the source if it is the first setup
//
// Revision 1.126  2003/07/05 10:55:57  laurentg
// New method SetWidth
//
// Revision 1.125  2003/06/14 14:29:21  laurentg
// Video format saved per video input for BT8x8 and CX2388x
//
// Revision 1.124  2003/06/02 15:30:11  adcockj
// Fix for bdelay problems
//
// Revision 1.123  2003/05/30 12:21:19  laurentg
// Don't forget to notify video format change if necessary when switching source and video input of destination source is tuner
//
// Revision 1.122  2003/05/29 17:07:27  laurentg
// no message
//
// Revision 1.121  2003/03/24 23:24:48  laurentg
// Temporary patch to bypass a probably bug in the code managing the source settings
//
// Revision 1.120  2003/03/09 19:48:28  laurentg
// Updated field statistics
//
// Revision 1.119  2003/03/09 11:35:24  laurentg
// Judder Terminator - input timing slightly updated
//
// Revision 1.118  2003/03/08 20:01:24  laurentg
// New setting "always sleep"
//
// Revision 1.117  2003/02/26 20:53:25  laurentg
// New timing setting MaxFieldShift
//
// Revision 1.116  2003/02/22 13:42:42  laurentg
// New counter to count fields runnign late
// Update input frequency on cleanish field changes only which means when the field is no running late
//
// Revision 1.115  2003/02/16 10:31:38  laurentg
// GetNextFieldAccurate : call to Timing_UpdateRunningAverage as it is done for CX2388x
//
// Revision 1.114  2003/02/03 19:09:16  adcockj
// Added VBI skips so that hopefully PAL60 CC will work correctly
//
// Revision 1.113  2003/01/25 23:46:25  laurentg
// Reset after the loading of the new settings in VideoFormatOnChange
//
// Revision 1.112  2003/01/18 13:55:43  laurentg
// New methods GetHDelay and GetVDelay
//
// Revision 1.111  2003/01/18 10:52:11  laurentg
// SetOverscan renamed SetAspectRatioData
// Unnecessary call to SetOverscan deleted
// Overscan setting specific to calibration deleted
//
// Revision 1.110  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.109  2003/01/13 21:13:43  adcockj
// Allow h&V delays to be done in sort of real time
//
// Revision 1.108  2003/01/13 17:46:44  adcockj
// HDelay and VDelay turned from absolute to adjustments
//
// Revision 1.107  2003/01/12 16:19:32  adcockj
// Added SettingsGroup activity setting
// Corrected event sequence and channel change behaviour
//
// Revision 1.106  2003/01/11 15:22:24  adcockj
// Interim Checkin of setting code rewrite
//  - Remove CSettingsGroupList class
//  - Fixed bugs in format switching
//  - Some new CSettingGroup code
//
// Revision 1.105  2003/01/11 12:53:57  adcockj
// Interim Check in of settings changes
//  - bug fixes for overlay settings changes
//  - Bug fixes for new settings changes
//  - disables settings per channel completely
//
// Revision 1.104  2003/01/10 17:51:45  adcockj
// Removed SettingFlags
//
// Revision 1.103  2003/01/10 17:37:43  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.102  2003/01/08 19:59:34  laurentg
// Analogue Blanking setting by source
//
// Revision 1.101  2003/01/07 23:27:01  laurentg
// New overscan settings
//
// Revision 1.100  2003/01/07 16:49:05  adcockj
// Changes to allow variable sampling rates for VBI
//
// Revision 1.99  2003/01/05 19:01:12  adcockj
// Made some changes to Laurent's last set of VBI fixes
//
// Revision 1.98  2003/01/05 18:35:45  laurentg
// Init function for VBI added
//
// Revision 1.97  2003/01/05 16:54:53  laurentg
// Updated parameters for VBI_DecodeLine
//
// Revision 1.96  2003/01/01 20:56:46  atnak
// Updates for various VideoText changes
//
// Revision 1.95  2002/12/10 12:58:07  adcockj
// Removed NotifyInputChange and NotifyVideoFormatChange functions and replaced with
//  calls to EventCollector->RaiseEvent
//
// Revision 1.94  2002/12/09 00:32:14  atnak
// Added new muting stuff
//
// Revision 1.93  2002/12/07 15:59:06  adcockj
// Modified mute behaviour
//
// Revision 1.92  2002/12/03 13:28:23  adcockj
// Corrected per channel settings code
//
// Revision 1.91  2002/11/07 20:33:16  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.90  2002/10/31 03:10:55  atnak
// Changed CSource::GetTreeSettingsPage to return CTreeSettingsPage*
//
// Revision 1.89  2002/10/29 03:05:48  atnak
// Added a virtual GetTreeSettingsPage() to all CSources
//
// Revision 1.88  2002/10/26 17:51:52  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.87  2002/10/22 04:08:50  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.86  2002/10/15 18:31:45  kooiman
// Added stereo detect interval for continuous scanning for stereo mode.
//
// Revision 1.85  2002/10/15 15:25:19  kooiman
// Setting groups changes.
//
// Revision 1.84  2002/10/11 21:45:31  ittarnavsky
// commented out the call to GetNumAudioInputs()
//
// Revision 1.83  2002/10/08 21:16:09  kooiman
// Fixed accidental remove of line of code.
//
// Revision 1.82  2002/10/08 20:43:16  kooiman
// Added Automatic Frequency Control for tuners. Changed to Hz instead of multiple of 62500 Hz.
//
// Revision 1.81  2002/10/07 22:31:27  kooiman
// Fixed audio initialization.
//
// Revision 1.80  2002/10/07 20:31:02  kooiman
// Fixed autodetect bugs.
//
// Revision 1.79  2002/09/29 13:53:40  adcockj
// Ensure Correct History stored
//
// Revision 1.78  2002/09/29 10:14:14  adcockj
// Fixed problem with history in OutThreads
//
// Revision 1.77  2002/09/28 14:31:32  kooiman
// Base class this pointer apparently not equal to this of main class. fixed comparison.
//
// Revision 1.76  2002/09/28 13:33:04  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.75  2002/09/27 14:13:29  kooiman
// Improved stereo detection & manual audio standard dialog box.
//
// Revision 1.74  2002/09/26 16:37:20  kooiman
// Volume event support.
//
// Revision 1.73  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.72  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.71  2002/09/22 17:47:04  adcockj
// Fixes for holo3d
//
// Revision 1.70  2002/09/21 08:28:04  kooiman
// Preparations for fm radio accidentally slipped in. Disabled it till it works.
//
// Revision 1.69  2002/09/20 19:19:07  kooiman
// Force call to audiostandard detect onchange.
//
// Revision 1.68  2002/09/16 20:08:21  adcockj
// fixed format detect for cx2388x
//
// Revision 1.67  2002/09/16 19:34:18  adcockj
// Fix for auto format change
//
// Revision 1.66  2002/09/16 14:37:36  kooiman
// Added stereo autodetection.
//
// Revision 1.65  2002/09/15 15:57:27  kooiman
// Added Audio standard support.
//
// Revision 1.64  2002/09/15 14:20:37  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.63  2002/09/12 21:55:23  ittarnavsky
// Removed references to HasMSP and UseInputPin1
//
// Revision 1.62  2002/09/07 20:54:50  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.61  2002/09/02 19:07:21  kooiman
// Added BT848 advanced settings to advanced settings dialog
//
// Revision 1.60  2002/08/27 22:02:32  kooiman
// Added Get/Set input for video and audio for all sources. Added source input change notification.
//
// Revision 1.59  2002/08/26 18:25:09  adcockj
// Fixed problem with PAL/NTSC detection
//
// Revision 1.58  2002/08/19 18:58:24  adcockj
// Changed video defaults
//
// Revision 1.57  2002/08/15 14:16:18  kooiman
// Cleaner settings per channel implementation
//
// Revision 1.56  2002/08/13 21:21:24  kooiman
// Improved settings per channel to account for source and input changes.
//
// Revision 1.55  2002/08/13 21:04:42  kooiman
// Add IDString() to Sources for identification purposes.
//
// Revision 1.54  2002/08/12 22:42:28  kooiman
// Fixed small spelling error.
//
// Revision 1.53  2002/08/12 19:54:27  laurentg
// Selection of video card to adjust DScaler settings
//
// Revision 1.52  2002/08/11 16:56:35  laurentg
// More information displayed in the title of the BT card setup dialog box
//
// Revision 1.51  2002/08/11 14:16:54  laurentg
// Disable Cancel button when the select card is displayed at startup
//
// Revision 1.50  2002/08/11 12:08:24  laurentg
// Cut BT Card setup and general hardware setup in two different windows
//
// Revision 1.49  2002/08/09 13:33:24  laurentg
// Processor speed and trade off settings moved from BT source settings to DScaler settings
//
// Revision 1.48  2002/08/08 21:15:07  kooiman
// Fix settings per channel timing issue.
//
// Revision 1.47  2002/08/08 12:35:39  kooiman
// Better channel settings support for BT848 settings.
//
// Revision 1.46  2002/08/07 21:53:04  adcockj
// Removed todo item
//
// Revision 1.45  2002/08/05 13:25:17  kooiman
// Added BT volume to save by channel settings.
//
// Revision 1.44  2002/08/05 12:05:28  kooiman
// Added support for per channel settings.
//
// Revision 1.43  2002/07/02 20:00:07  adcockj
// New setting for MSP input pin selection
//
// Revision 1.42  2002/06/22 15:00:22  laurentg
// New vertical flip mode
//
// Revision 1.41  2002/06/16 18:54:59  robmuller
// ACPI powersafe support.
//
// Revision 1.40  2002/06/05 20:53:49  adcockj
// Default changes and settings fixes
//
// Revision 1.39  2002/04/15 22:50:08  laurentg
// Change again the available formats for still saving
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.38  2002/04/10 07:14:50  adcockj
// Fixed crash on saving settings
//
// Revision 1.37  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.36  2002/03/12 21:10:04  robmuller
// Corrected error in TradeOff setting.
//
// Revision 1.35  2002/03/04 20:44:49  adcockj
// Reversed incorrect changed
//
// Revision 1.33  2002/02/23 16:41:09  laurentg
// Set timer TIMER_MSP only if current card has a MSP
//
// Revision 1.32  2002/02/23 00:30:47  laurentg
// NotifySizeChange
//
// Revision 1.31  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.30  2002/02/17 20:32:34  laurentg
// Audio input display suppressed from the OSD main screen
// GetStatus modified to display the video input name in OSD main screen even when there is no signal
//
// Revision 1.29  2002/02/17 18:45:08  laurentg
// At the first hardware setup, select the correct audio input
//
// Revision 1.28  2002/02/17 17:46:59  laurentg
// Mute the audio when switching to another card
//
// Revision 1.27  2002/02/10 21:34:31  laurentg
// Default value for "Save Settings By Format" is now ON
//
// Revision 1.26  2002/02/09 14:46:05  laurentg
// OSD main screen updated to display the correct input name (or channel)
// OSD main screen updated to display only activated filters
// Menu label for the BT848 providers now displays the name of the card
//
// Revision 1.25  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.24  2002/02/08 19:27:18  adcockj
// Fixed problems with video settings dialog
//
// Revision 1.23  2002/01/26 17:54:48  laurentg
// Bug correction regarding pixel width updates
//
// Revision 1.22  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.21  2002/01/21 14:33:17  robmuller
// Fixed: setting wrong audio input in tuner mode in VideoSourceOnChange().
//
// Revision 1.20  2002/01/17 22:22:06  robmuller
// Added member function GetTunerId().
//
// Revision 1.19  2002/01/13 12:47:58  adcockj
// Fix for pixel width and format change
//
// Revision 1.18  2001/12/22 13:18:04  adcockj
// Tuner bugfixes
//
// Revision 1.17  2001/12/19 19:24:45  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.16  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.15  2001/12/16 10:14:16  laurentg
// Calculation of used fields restored
//
// Revision 1.14  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.13  2001/12/03 19:33:59  adcockj
// Bug fixes for settings and memory
//
// Revision 1.12  2001/12/03 17:27:56  adcockj
// SECAM NICAM patch from Quenotte
//
// Revision 1.11  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.10  2001/11/29 14:04:06  adcockj
// Added Javadoc comments
//
// Revision 1.9  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.8  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.7  2001/11/22 22:27:00  adcockj
// Bug Fixes
//
// Revision 1.6  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.5  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.4  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.11  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.10  2001/08/22 18:38:31  adcockj
// Fixed Recursive bug
//
// Revision 1.1.2.9  2001/08/22 11:12:48  adcockj
// Added VBI support
//
// Revision 1.1.2.8  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.1.2.7  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.6  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.5  2001/08/19 14:43:47  adcockj
// Fixed memory leaks
//
// Revision 1.1.2.4  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.3  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.2  2001/08/16 06:43:34  adcockj
// moved more stuff into the new file (deonsn't compile)
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file BT848Source.cpp CBT848Source Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Source.h"
#include "DScaler.h"
#include "VBI.h"
#include "VBI_VideoText.h"
#include "VBI_VPSdecode.h"
#include "Audio.h"
#include "VideoSettings.h"
#include "OutThreads.h"
#include "OSD.h"
#include "Status.h"
#include "FieldTiming.h"
#include "ProgramList.h"
#include "BT848_Defines.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"
#include "Providers.h"
#include "Setting.h"
#include "SettingsMaster.h"

extern long EnableCancelButton;

CBT848Source::CBT848Source(CBT848Card* pBT848Card, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex) :
    CSource(WM_BT848_GETVALUE, IDC_BT848),
    m_pBT848Card(pBT848Card),
    m_CurrentX(720),
    m_CurrentY(480),
    m_CurrentVBILines(19),
    m_Section(IniSection),
    m_IsFieldOdd(FALSE),
    m_IDString(IniSection),
    m_InSaturationUpdate(FALSE),
    m_ChipName(ChipName),
    m_DeviceIndex(DeviceIndex),
    m_DetectingAudioStandard(0),
	m_InitialSetup(FALSE)
{
    CreateSettings(IniSection);

    // stop on change messages from coming through
    DisableOnChange();

    eEventType EventList[] = {EVENT_CHANNEL_PRECHANGE,EVENT_CHANNEL_CHANGE,EVENT_AUDIOSTANDARD_DETECTED,EVENT_AUDIOCHANNELSUPPORT_DETECTED,EVENT_ENDOFLIST};
    EventCollector->Register(this, EventList);

    m_RiscBaseLinear = (DWORD*)RiscDMAMem->GetUserPointer();
    m_RiscBasePhysical = RiscDMAMem->TranslateToPhysical(m_RiscBaseLinear, 83968, NULL);
    for(int i(0); i < 5; ++i)
    {
        m_pDisplay[i] = (BYTE*)DisplayDMAMem[i]->GetUserPointer();
        m_DisplayDMAMem[i] = DisplayDMAMem[i];
        m_pVBILines[i] = (BYTE*)VBIDMAMem[i]->GetUserPointer();
        m_VBIDMAMem[i] = VBIDMAMem[i];
    }

    // Set up 5 sets of pointers to the start of odd and even lines
    for (int j(0); j < 5; j++)
    {
        m_OddFields[j].pData = m_pDisplay[j] + 2048;
        m_OddFields[j].Flags = PICTURE_INTERLACED_ODD;
        m_OddFields[j].IsFirstInSeries = FALSE;
        m_EvenFields[j].pData = m_pDisplay[j];
        m_EvenFields[j].Flags = PICTURE_INTERLACED_EVEN;
        m_EvenFields[j].IsFirstInSeries = FALSE;
    }
    // loads up core settings like card and tuner type
    ReadFromIni();
    
    SetupCard();

    InitializeUI();
}

CBT848Source::~CBT848Source()
{
    EventCollector->Unregister(this);

    KillTimer(hWnd, TIMER_MSP);
    delete m_pBT848Card;
}

void CBT848Source::SetSourceAsCurrent()
{
    // need to call up to parent to run register settings functions
    CSource::SetSourceAsCurrent();

    // tell the rest of DScaler what the setup is
    // A side effect of the Raise events is to tell the settings master
    // what our current setup is
    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, -1, m_VideoSource->GetValue());
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, -1, m_VideoFormat->GetValue());
    EventCollector->RaiseEvent(this, EVENT_VOLUME, 0, m_Volume->GetValue());

    int OldFormat = m_VideoFormat->GetValue();

    // reset the tuner
    // this will kick of the change channel event
    // which must happen after the video input event
    if(IsInTunerMode())
    {
        Channel_Reset();
    }
	else
	{
		// We read what is the video format saved for this video input
	    SettingsMaster->LoadOneSetting(m_VideoFormat);
	}

    // tell the world if the format has changed
    if(OldFormat != m_VideoFormat->GetValue())
    {
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldFormat, m_VideoFormat->GetValue());
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldFormat, m_VideoFormat->GetValue());

		// We save the video format attached to this video input
	    SettingsMaster->WriteOneSetting(m_VideoFormat);
    }

    // make sure the defaults are correct
    // but don't change the values
    ChangeDefaultsForSetup(SETUP_CHANGE_ANY, TRUE);

    // load up any channel/input/format specifc settings
    SettingsMaster->LoadSettings();

    Reset();
}

void CBT848Source::OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
    if (pEventObject == (CEventObject*)this)
    {
        if (Event == EVENT_CHANNEL_PRECHANGE)
        {
            ChannelChange(1, OldValue, NewValue);
        }
        else if (Event == EVENT_CHANNEL_CHANGE)
        {
            ChannelChange(0, OldValue, NewValue);
        }
    }
	
    if ((Event == EVENT_AUDIOSTANDARD_DETECTED) || (Event == EVENT_AUDIOCHANNELSUPPORT_DETECTED))
	{
        CAudioDecoder* pAudioDecoder;
        try 
        {
            pAudioDecoder = dynamic_cast<CAudioDecoder*>(pEventObject);
        }
        catch (...)
        {
            pAudioDecoder = NULL;
        }
        
        if ((pAudioDecoder!=NULL) && m_pBT848Card->IsMyAudioDecoder(pAudioDecoder))
        {
            if (Event == EVENT_AUDIOSTANDARD_DETECTED)
            {
                AudioStandardDetected(NewValue);        
	        }
	        else if (Event == EVENT_AUDIOCHANNELSUPPORT_DETECTED)
	        {
		        SupportedSoundChannelsDetected((eSupportedSoundChannels)NewValue);
	        }
        }
    }
}

void CBT848Source::CreateSettings(LPCSTR IniSection)
{
    CSettingGroup *pVideoFormatGroup = GetSettingsGroup("BT848 - Video Format", SETTING_BY_INPUT, TRUE);
    CSettingGroup *pVideoGroup = GetSettingsGroup("BT848 - Video", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, TRUE);
    CSettingGroup *pAudioGroup = GetSettingsGroup("BT848 - Audio", SETTING_BY_CHANNEL);
    CSettingGroup *pAudioStandard = GetSettingsGroup("BT848 - Audio Standard", SETTING_BY_CHANNEL);
    CSettingGroup *pPMSGroup = GetSettingsGroup("BT848 - PMS", SETTING_BY_FORMAT | SETTING_BY_INPUT, TRUE);

	//
	// WARNING : temporary change to have correct tuner audio input after DScaler first start
	//
	//CSettingGroup *pAudioSource = GetSettingsGroup("BT848 - Audio Source", SETTING_BY_INPUT, TRUE);
    CSettingGroup *pAudioSource = GetSettingsGroup("BT848 - Audio Source", 0);

	CSettingGroup *pAudioChannel = GetSettingsGroup("BT848 - Audio Channel", SETTING_BY_CHANNEL);
    CSettingGroup *pAudioControl = GetSettingsGroup("BT848 - Audio Control", SETTING_BY_CHANNEL);
    CSettingGroup *pAudioOther  = GetSettingsGroup("BT848 - Audio Other", SETTING_BY_CHANNEL);
    CSettingGroup *pAudioEqualizerGroup = GetSettingsGroup("BT848 - Audio Equalizer", SETTING_BY_CHANNEL);
    
    CSettingGroup *pAdvancedGroup = GetSettingsGroup("BT848 - Advanced Flags", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT);
    CSettingGroup *pAdvancedTimingGroup = GetSettingsGroup("BT848 - Advanced Timing", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT);
    
    m_Brightness = new CBrightnessSetting(this, "Brightness", DEFAULT_BRIGHTNESS_NTSC, -128, 127, IniSection, pVideoGroup);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", DEFAULT_CONTRAST_NTSC, 0, 511, IniSection, pVideoGroup);
    m_Settings.push_back(m_Contrast);

    m_Hue = new CHueSetting(this, "Hue", DEFAULT_HUE_NTSC, -128, 127, IniSection, pVideoGroup);
    m_Settings.push_back(m_Hue);

    m_Saturation = new CSaturationSetting(this, "Saturation", (DEFAULT_SAT_V_NTSC + DEFAULT_SAT_U_NTSC) / 2, 0, 511, IniSection, pVideoGroup);
    m_Settings.push_back(m_Saturation);

    m_SaturationU = new CSaturationUSetting(this, "Blue Saturation", DEFAULT_SAT_U_NTSC, 0, 511, IniSection, pVideoGroup);
    m_Settings.push_back(m_SaturationU);

    m_SaturationV = new CSaturationVSetting(this, "Red Saturation", DEFAULT_SAT_V_NTSC, 0, 511, IniSection, pVideoGroup);
    m_Settings.push_back(m_SaturationV);

    m_TopOverscan = new CTopOverscanSetting(this, "Overscan at Top", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_TopOverscan);

    m_BDelay = new CBDelaySetting(this, "Macrovision Timing", 0, 0, 255, IniSection, pAdvancedTimingGroup);
    m_Settings.push_back(m_BDelay);

    m_BtAgcDisable = new CBtAgcDisableSetting(this, "AGC Disable", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtAgcDisable);

    m_BtCrush = new CBtCrushSetting(this, "Crush", TRUE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtCrush);

    m_BtEvenChromaAGC = new CBtEvenChromaAGCSetting(this, "Even Chroma AGC", TRUE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtEvenChromaAGC);

    m_BtOddChromaAGC = new CBtOddChromaAGCSetting(this, "Odd Chroma AGC", TRUE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtOddChromaAGC);

    m_BtEvenLumaPeak = new CBtEvenLumaPeakSetting(this, "Even Luma Peak", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtEvenLumaPeak);

    m_BtOddLumaPeak = new CBtOddLumaPeakSetting(this, "Odd Luma Peak", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtOddLumaPeak);

    m_BtFullLumaRange = new CBtFullLumaRangeSetting(this, "Full Luma Range", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtFullLumaRange);

    m_BtEvenLumaDec = new CBtEvenLumaDecSetting(this, "Even Luma Dec", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtEvenLumaDec);

    m_BtOddLumaDec = new CBtOddLumaDecSetting(this, "Odd Luma Dec", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtOddLumaDec);

    m_BtEvenComb = new CBtEvenCombSetting(this, "Even Comb", TRUE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtEvenComb);

    m_BtOddComb = new CBtOddCombSetting(this, "Odd Comb", TRUE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtOddComb);

    m_BtColorBars = new CBtColorBarsSetting(this, "Color Bars", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtColorBars);

    m_BtGammaCorrection = new CBtGammaCorrectionSetting(this, "Gamma Correction", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtGammaCorrection);

    m_BtCoring = new CBtCoringSetting(this, "Coring", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtCoring);

    m_BtHorFilter = new CBtHorFilterSetting(this, "Horizontal Filter", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtHorFilter);

    m_BtVertFilter = new CBtVertFilterSetting(this, "Vertical Filter", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtVertFilter);

    m_BtColorKill = new CBtColorKillSetting(this, "Color Kill", FALSE, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtColorKill);

    m_BtWhiteCrushUp = new CBtWhiteCrushUpSetting(this, "White Crush Upper", 0xCF, 0, 255, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtWhiteCrushUp);

    m_BtWhiteCrushDown = new CBtWhiteCrushDownSetting(this, "White Crush Lower", 0x7F, 0, 255, IniSection, pAdvancedGroup);
    m_Settings.push_back(m_BtWhiteCrushDown);

    m_PixelWidth = new CPixelWidthSetting(this, "Sharpness", 720, 120, DSCALER_MAX_WIDTH, IniSection, pAdvancedGroup);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth", pAdvancedGroup);
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 12, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection, pVideoFormatGroup);
    m_Settings.push_back(m_VideoFormat);

    m_HDelay = new CHDelaySetting(this, "Horizontal Delay Adjust", 0, -16, 16, IniSection, pAdvancedTimingGroup);
    m_HDelay->SetStepValue(2);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay Adjust", 0, -40, 40, IniSection, pAdvancedTimingGroup);
    m_VDelay->SetStepValue(2);
    m_Settings.push_back(m_VDelay);

    m_ReversePolarity = new CYesNoSetting("Reverse Polarity", FALSE, IniSection, "ReversePolarity", pAdvancedGroup);
    m_Settings.push_back(m_ReversePolarity);

    m_CardType = new CSliderSetting("Card Type", TVCARD_UNKNOWN, TVCARD_UNKNOWN, TVCARD_LASTONE - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_AudioSource1 = new CAudioSource1Setting(this, "Audio Source 1", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection, pAudioSource);
    m_Settings.push_back(m_AudioSource1);

    m_AudioChannel = new CAudioChannelSetting(this, "Audio Channel", SOUNDCHANNEL_STEREO, SOUNDCHANNEL_MONO, SOUNDCHANNEL_LANGUAGE2, IniSection, pAudioChannel);
    m_Settings.push_back(m_AudioChannel);

    m_AutoStereoSelect = new CAutoStereoSelectSetting(this, "Auto Stereo Select", FALSE, IniSection, pAudioChannel);
    m_Settings.push_back(m_AutoStereoSelect);

    m_Volume = new CVolumeSetting(this, "Volume", 900, 0, 1000, IniSection, pAudioControl);
    m_Volume->SetStepValue(20);
    //m_Volume->SetOSDDivider(10);
    m_Settings.push_back(m_Volume);

    m_Bass = new CBassSetting(this, "Bass", 0, -96, 127, IniSection, pAudioControl);
    m_Settings.push_back(m_Bass);

    m_Treble = new CTrebleSetting(this, "Treble", 0, -96, 127, IniSection, pAudioControl);
    m_Settings.push_back(m_Treble);

    m_Balance = new CBalanceSetting(this, "Balance", 0, -127, 127, IniSection, pAudioControl);
    m_Settings.push_back(m_Balance);

    // save per input removed
    m_Settings.push_back(NULL);
    
    // save per format removed
    m_Settings.push_back(NULL);
    
    m_AudioSource2 = new CAudioSource2Setting(this, "Audio Source 2", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection, pAudioSource);
    m_Settings.push_back(m_AudioSource2);

    m_AudioSource3 = new CAudioSource3Setting(this, "Audio Source 3", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection, pAudioSource);
    m_Settings.push_back(m_AudioSource3);

    m_AudioSource4 = new CAudioSource4Setting(this, "Audio Source 4", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection, pAudioSource);
    m_Settings.push_back(m_AudioSource4);

    m_AudioSource5 = new CAudioSource5Setting(this, "Audio Source 5", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection, pAudioSource);
    m_Settings.push_back(m_AudioSource5);

    m_AudioSource6 = new CAudioSource6Setting(this, "Audio Source 6", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection, pAudioSource);
    m_Settings.push_back(m_AudioSource6);

    m_UseInputPin1 = new CUseInputPin1Setting(this, "Use MSP Input Pin 1", FALSE, IniSection, pAudioChannel);
    m_Settings.push_back(m_UseInputPin1);

    m_UseEqualizer = new CUseEqualizerSetting(this, "Use equalizer", FALSE, IniSection, pAudioEqualizerGroup);
    m_Settings.push_back(m_UseEqualizer);

    m_EqualizerBand1 = new CEqualizerBand1Setting(this, "Equalizer band 1", 0, -96, 96, IniSection, pAudioEqualizerGroup);
    m_Settings.push_back(m_EqualizerBand1);

    m_EqualizerBand2 = new CEqualizerBand2Setting(this, "Equalizer band 2", 0, -96, 96, IniSection, pAudioEqualizerGroup);
    m_Settings.push_back(m_EqualizerBand2);

    m_EqualizerBand3 = new CEqualizerBand3Setting(this, "Equalizer band 3", 0, -96, 96, IniSection, pAudioEqualizerGroup);
    m_Settings.push_back(m_EqualizerBand3);

    m_EqualizerBand4 = new CEqualizerBand4Setting(this, "Equalizer band 4", 0, -96, 96, IniSection, pAudioEqualizerGroup);
    m_Settings.push_back(m_EqualizerBand4);

    m_EqualizerBand5 = new CEqualizerBand5Setting(this, "Equalizer band 5", 0, -96, 96, IniSection, pAudioEqualizerGroup);
    m_Settings.push_back(m_EqualizerBand5);

    m_AudioLoudness = new CAudioLoudnessSetting(this, "Loudness", 0, 0, 255, IniSection, pAudioControl);
    m_Settings.push_back(m_AudioLoudness);

    m_AudioSuperbass = new CAudioSuperbassSetting(this, "Super Bass", FALSE, IniSection, pAudioControl);
    m_Settings.push_back(m_AudioSuperbass);

    m_AudioSpatialEffect = new CAudioSpatialEffectSetting(this, "Spatial Effect", 0, -128, 127, IniSection, pAudioControl);
    m_Settings.push_back(m_AudioSpatialEffect);

    m_AudioAutoVolumeCorrection = new CAudioAutoVolumeCorrectionSetting(this, "Automatic Volume Correction", 0, 0, 60*1000, IniSection, pAudioControl);
    m_Settings.push_back(m_AudioAutoVolumeCorrection);

    m_AudioStandardDetect = new CAudioStandardDetectSetting(this, "Audio Standard Detect", 0, 0, 4, IniSection, pAudioStandard);
    m_Settings.push_back(m_AudioStandardDetect);
    
    m_AudioStandardDetectInterval = new CAudioStandardDetectIntervalSetting(this, "Audio Standard Detect Interval (ms)", 200, 0, 10000, IniSection, pAudioStandard);
    m_Settings.push_back(m_AudioStandardDetectInterval);

    m_AudioStandardManual = new CAudioStandardManualSetting(this, "Audio Standard Manual", 0, 0, 0x7ff-1, IniSection, pAudioStandard);
    m_Settings.push_back(m_AudioStandardManual);

    m_AudioStandardMajorCarrier = new CAudioStandardMajorCarrierSetting(this, "Audio Standard Major carrier", 0, 0, 0x7ffffffL, IniSection, pAudioStandard);
    m_Settings.push_back(m_AudioStandardMajorCarrier);

    m_AudioStandardMinorCarrier = new CAudioStandardMinorCarrierSetting(this, "Audio Standard Minor carrier", 0, 0, 0x7ffffffL, IniSection, pAudioStandard);
    m_Settings.push_back(m_AudioStandardMinorCarrier);

    m_AudioStandardInStatusBar = new CAudioStandardInStatusBarSetting(this, "Audio Standard in Statusbar", FALSE, IniSection, pAudioStandard);
    m_Settings.push_back(m_AudioStandardInStatusBar);

    m_MSP34xxFlags = new CMSP34xxFlagsSetting(this, "MSP34xx Flags", 0, 0, 0x7ffffffL, IniSection, pAudioOther);
    m_Settings.push_back(m_MSP34xxFlags);    

    m_AutoStereoDetectInterval = new CAutoStereoDetectIntervalSetting(this, "Auto Stereo Detect Interval", 0, 0, 24*60*1000, IniSection, pAudioChannel);
    m_Settings.push_back(m_AutoStereoDetectInterval);

    m_BottomOverscan = new CBottomOverscanSetting(this, "Overscan at Bottom", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_BottomOverscan);

    m_LeftOverscan = new CLeftOverscanSetting(this, "Overscan at Left", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_LeftOverscan);

    m_RightOverscan = new CRightOverscanSetting(this, "Overscan at Right", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_RightOverscan);

    m_PMSGain1 = new CPMSGain1Setting(this, "Gain Channel 1", 0x90, 0, 511, IniSection, pPMSGroup);
    m_Settings.push_back(m_PMSGain1);

    m_PMSGain2 = new CPMSGain2Setting(this, "Gain Channel 2", 302, 0, 511, IniSection, pPMSGroup);
    m_Settings.push_back(m_PMSGain2);

    m_PMSGain3 = new CPMSGain3Setting(this, "Gain Channel 3", 302, 0, 511, IniSection, pPMSGroup);
    m_Settings.push_back(m_PMSGain3);

    m_PMSGain4 = new CPMSGain4Setting(this, "Gain Channel 4", 302, 0, 511, IniSection, pPMSGroup);
    m_Settings.push_back(m_PMSGain4);


#ifdef _DEBUG    
    if (BT848_SETTING_LASTONE != m_Settings.size())
    {
        LOGD("Number of settings in BT848 source is not equal to the number of settings in DS_Control.h");
        LOGD("DS_Control.h or BT848Source.cpp are probably not in sync with eachother.");
    }
#endif
}

void CBT848Source::Start()
{
    m_pBT848Card->StopCapture();
    CreateRiscCode(bCaptureVBI);
    m_pBT848Card->StartCapture(bCaptureVBI);
    m_pBT848Card->SetDMA(TRUE);
    Timing_Reset();
    NotifySizeChange();
    NotifySquarePixelsCheck();

    VBI_Init_data(GetTVFormat((eVideoFormat)m_VideoFormat->GetValue())->Bt848VBISamplingFrequency);

    // Just before we start allow change messages again
    EnableOnChange();
    
    // seems to be required
    // otherwise I get no sound on startup
    if(IsInTunerMode())
    {
        m_AudioStandardDetect->SetValue(m_AudioStandardDetect->GetValue());    
        m_pBT848Card->SetAudioChannel((eSoundChannel)m_AudioChannel->GetValue());

        SetTimer(hWnd, TIMER_MSP, TIMER_MSP_MS, NULL);
    }
    
}

void CBT848Source::Reset()
{
    m_pBT848Card->ResetHardware(m_RiscBasePhysical);
    m_pBT848Card->SetVideoSource(m_VideoSource->GetValue());
    if (m_BDelay->GetValue() != 0)
    {
        // BDELAY override from .ini file
        m_pBT848Card->SetBDelay(m_BDelay->GetValue());
    }
    else
    {
        m_pBT848Card->SetBDelay(GetTVFormat(GetFormat())->bDelayB);  
    }


    m_pBT848Card->SetContrastBrightness(m_Contrast->GetValue(), m_Brightness->GetValue());
    m_pBT848Card->SetHue(m_Hue->GetValue());
    m_pBT848Card->SetSaturationU(m_SaturationU->GetValue());
    m_pBT848Card->SetSaturationV(m_SaturationV->GetValue());
    m_pBT848Card->SetFullLumaRange(m_BtFullLumaRange->GetValue());

    if(m_CardType->GetValue() != TVCARD_PMSDELUXE && m_CardType->GetValue() != TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetEvenLumaDec(m_BtEvenLumaDec->GetValue());
        m_pBT848Card->SetOddLumaDec(m_BtOddLumaDec->GetValue());
        m_pBT848Card->SetEvenChromaAGC(m_BtEvenChromaAGC->GetValue());
        m_pBT848Card->SetOddChromaAGC(m_BtOddChromaAGC->GetValue());
        m_pBT848Card->SetEvenLumaPeak(m_BtEvenLumaPeak->GetValue());
        m_pBT848Card->SetOddLumaPeak(m_BtOddLumaPeak->GetValue());
        m_pBT848Card->SetColorKill(m_BtColorKill->GetValue());
        m_pBT848Card->SetHorFilter(m_BtHorFilter->GetValue());
        m_pBT848Card->SetVertFilter(m_BtVertFilter->GetValue());
        m_pBT848Card->SetCoring(m_BtCoring->GetValue());
        m_pBT848Card->SetEvenComb(m_BtEvenComb->GetValue());
        m_pBT848Card->SetOddComb(m_BtOddComb->GetValue());
        m_pBT848Card->SetAgcDisable(m_BtAgcDisable->GetValue());
        m_pBT848Card->SetCrush(m_BtCrush->GetValue());
        m_pBT848Card->SetColorBars(m_BtColorBars->GetValue());
        m_pBT848Card->SetGammaCorrection(m_BtGammaCorrection->GetValue());
        m_pBT848Card->SetWhiteCrushUp(m_BtWhiteCrushUp->GetValue());
        m_pBT848Card->SetWhiteCrushDown(m_BtWhiteCrushDown->GetValue());
    }
    else
    {
        // set up the PMS gains for use in component modes
        m_pBT848Card->SetPMSChannelGain(1, m_PMSGain1->GetValue());
        m_pBT848Card->SetPMSChannelGain(2, m_PMSGain2->GetValue());
        m_pBT848Card->SetPMSChannelGain(3, m_PMSGain3->GetValue());
        m_pBT848Card->SetPMSChannelGain(4, m_PMSGain4->GetValue());
    }

    m_CurrentX = m_PixelWidth->GetValue();
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    
    NotifySizeChange();

    m_MSP34xxFlags->SetValue(m_MSP34xxFlags->GetValue());
        
    InitAudio();    
}


void CBT848Source::CreateRiscCode(BOOL bCaptureVBI)
{
    DWORD* pRiscCode;
    WORD nField;
    WORD nLine;
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytesPerLine;
    DWORD BytesPerLine = 0;

    pRiscCode = (DWORD*)m_RiscBaseLinear;
    // we create the RISC code for 10 fields
    // the first one (0) is even
    // last one (9) is odd
    for(nField = 0; nField < 10; nField++)
    {
        // First we sync onto either the odd or even field
        if(nField & 1)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
        }
        else
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
        }
        *(pRiscCode++) = 0;

        // Create VBI code of required
        if (bCaptureVBI)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
            *(pRiscCode++) = 0;

            // if we are doing PAL60 then
            // we need to skip VBI 3 lines
            // so that the lines are in the right place
            // with CC on line 11 (corresponding to line 21 of video)
            if(m_VideoFormat->GetValue() == VIDEOFORMAT_PAL_60)
            {
                *(pRiscCode++) = (DWORD) (BT848_RISC_SKIP | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL);
                *(pRiscCode++) = (DWORD) (BT848_RISC_SKIP | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL);
                *(pRiscCode++) = (DWORD) (BT848_RISC_SKIP | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL);
            }

            pUser = m_pVBILines[nField / 2];
            if((nField & 1) == 1)
            {
                pUser += m_CurrentVBILines * 2048;
            }
            for (nLine = 0; nLine < m_CurrentVBILines; nLine++)
            {
                pPhysical = m_VBIDMAMem[nField / 2]->TranslateToPhysical(pUser, VBI_SPL, &GotBytesPerLine);
                if(pPhysical == 0 || VBI_SPL > GotBytesPerLine)
                {
                    return;
                }
                *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL;
                *(pRiscCode++) = pPhysical;
                pUser += 2048;
            }
        }

        *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
        *(pRiscCode++) = 0;


        // work out the position of the first line
        // first line is line zero an even line
        pUser = m_pDisplay[nField / 2];
        if(nField & 1)
        {
            pUser += 2048;
        }
        BytesPerLine = m_CurrentX * 2;
        for (nLine = 0; nLine < m_CurrentY / 2; nLine++)
        {

            pPhysical = m_DisplayDMAMem[nField / 2]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
            {
                return;
            }
            *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
            *(pRiscCode++) = pPhysical;
            // since we are doing all the lines of the same
            // polarity at the same time we skip two lines
            pUser += 4096;
        }
    }

    m_BytesPerRISCField = ((long)pRiscCode - (long)m_RiscBaseLinear) / 10;
    *(pRiscCode++) = BT848_RISC_JUMP;
    *(pRiscCode++) = m_RiscBasePhysical;

    m_pBT848Card->SetRISCStartAddress(m_RiscBasePhysical);
}


void CBT848Source::Stop()
{
    // disable OnChange messages while video is stopped
    DisableOnChange();
    // stop capture
    m_pBT848Card->StopCapture();
    if(IsInTunerMode())
    {
        KillTimer(hWnd, TIMER_MSP);
    }
}

void CBT848Source::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    if(AccurateTiming)
    {
        GetNextFieldAccurate(pInfo);
    }
    else
    {
        GetNextFieldNormal(pInfo);
    }

    ShiftPictureHistory(pInfo, 10);
    if(m_IsFieldOdd)
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            pInfo->PictureHistory[0] = &m_OddFields[pInfo->CurrentFrame];
        }
        else
        {
            pInfo->PictureHistory[0] = &m_EvenFields[pInfo->CurrentFrame];
        }
    }
    else
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            pInfo->PictureHistory[0] = &m_EvenFields[pInfo->CurrentFrame];
        }
        else
        {
            pInfo->PictureHistory[0] = &m_OddFields[(pInfo->CurrentFrame + 4) % 5];
        }
    }

    pInfo->LineLength = m_CurrentX * 2;
    pInfo->FrameWidth = m_CurrentX;
    pInfo->FrameHeight = m_CurrentY;
    pInfo->FieldHeight = m_CurrentY / 2;
    pInfo->InputPitch = 4096;

    Timing_IncrementUsedFields();

    // auto input detect
    Timimg_AutoFormatDetect(pInfo, 10);
}

int CBT848Source::GetWidth()
{
    return m_CurrentX;
}

int CBT848Source::GetHeight()
{
    return m_CurrentY;
}

void CBT848Source::SetWidth(int w)
{
	m_PixelWidth->SetValue(w);
}

CBT848Card* CBT848Source::GetBT848Card()
{
    return m_pBT848Card;
}

LPCSTR CBT848Source::GetStatus()
{
    static char szStatus[24];
    LPCSTR pRetVal;

    if (IsInTunerMode())
    {
        VT_GetStation(szStatus, sizeof(szStatus));

        if (*szStatus == '\0')
        {
            VPS_GetChannelName(szStatus, sizeof(szStatus));
        }
        if (*szStatus == '\0')
        {
            pRetVal = Channel_GetName();
        }
        else
        {
            pRetVal = szStatus;
        }
    }
    else
    {
        pRetVal = m_pBT848Card->GetInputName(m_VideoSource->GetValue());
    }
    return pRetVal;
}

int CBT848Source::GetRISCPosAsInt()
{
    int CurrentPos = 10;
    while(CurrentPos > 9)
    {
        DWORD CurrentRiscPos = m_pBT848Card->GetRISCPos();
        CurrentPos = (CurrentRiscPos - m_RiscBasePhysical) / m_BytesPerRISCField;
    }

    return CurrentPos;
}

eVideoFormat CBT848Source::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}

void CBT848Source::SetFormat(eVideoFormat NewFormat)
{
    PostMessage(hWnd, WM_BT848_SETVALUE, TVFORMAT, NewFormat);
}


ISetting* CBT848Source::GetBrightness()
{
    return m_Brightness;
}

ISetting* CBT848Source::GetContrast()
{
    return m_Contrast;
}

ISetting* CBT848Source::GetHue()
{
    return m_Hue;
}

ISetting* CBT848Source::GetSaturation()
{
    return m_Saturation;
}

ISetting* CBT848Source::GetSaturationU()
{
    return m_SaturationU;
}

ISetting* CBT848Source::GetSaturationV()
{
    return m_SaturationV;
}

ISetting* CBT848Source::GetTopOverscan()
{
    return m_TopOverscan;
}

ISetting* CBT848Source::GetBottomOverscan()
{
    return m_BottomOverscan;
}

ISetting* CBT848Source::GetLeftOverscan()
{
    return m_LeftOverscan;
}

ISetting* CBT848Source::GetRightOverscan()
{
    return m_RightOverscan;
}

ISetting* CBT848Source::GetHDelay()
{
    return m_HDelay;
}

ISetting* CBT848Source::GetVDelay()
{
    return m_VDelay;
}

void CBT848Source::BtAgcDisableOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAgcDisable(NewValue);
}

void CBT848Source::BtCrushOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetCrush(NewValue);
}

void CBT848Source::BtEvenChromaAGCOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenChromaAGC(NewValue);
}

void CBT848Source::BtOddChromaAGCOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddChromaAGC(NewValue);
}

void CBT848Source::BtEvenLumaPeakOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenLumaPeak(NewValue);
}

void CBT848Source::BtOddLumaPeakOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddLumaPeak(NewValue);
}

void CBT848Source::BtFullLumaRangeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetFullLumaRange(NewValue);
}

void CBT848Source::BtEvenLumaDecOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenLumaDec(NewValue);
}

void CBT848Source::BtOddLumaDecOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddLumaDec(NewValue);
}

void CBT848Source::BtEvenCombOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenComb(NewValue);
}

void CBT848Source::BtOddCombOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddComb(NewValue);
}

void CBT848Source::BtColorBarsOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetColorBars(NewValue);
}

void CBT848Source::BtGammaCorrectionOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetGammaCorrection(NewValue);
}

void CBT848Source::BtVertFilterOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetVertFilter(NewValue);
}

void CBT848Source::BtHorFilterOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetHorFilter(NewValue);
}

////////////////////////////////////////////////////////////////////////////////////
// The following function will continually check the position in the RISC code
// until it is  is different from what we already have.
// We know were we are so we set the current field to be the last one
// that has definitely finished.
//
// Added code here to use a user specified parameter for how long to sleep.  Note that
// windows timer tick resolution is really MUCH worse than 1 millesecond.  Who knows 
// what logic W98 really uses?
//
// Note also that sleep(0) just tells the operating system to dispatch some other
// task now if one is ready, not to sleep for zero seconds.  Since I've taken most
// of the unneeded waits out of other processing here Windows will eventually take 
// control away from us anyway, We might as well choose the best time to do it, without
// waiting more than needed. 
//
// Also added code to HurryWhenLate.  This checks if the new field is already here by
// the time we arrive.  If so, assume we are not keeping up with the BT chip and skip
// some later processing.  Skip displaying this field and use the CPU time gained to 
// get back here faster for the next one.  This should help us degrade gracefully on
// slower or heavily loaded systems but use all available time for processing a good
// picture when nothing else is running.  TRB 10/28/00
//
void CBT848Source::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
	BOOL bLate = TRUE;
    int NewPos;
    int FieldDistance;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    int Counter(0);
    LARGE_INTEGER StartOfWait;

    QueryPerformanceCounter(&StartOfWait);
    
    while(OldPos == (NewPos = GetRISCPosAsInt()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        bLate = FALSE;							// if we waited then we are not late

        if(++Counter == 1000)
        {
            //check we're not in a tight loop here for too long
            //sometimes boards with external chips seem to hang and need
            //resetting, for other boards this won't do any harm (hopefully)
            LARGE_INTEGER EndOfWait;
            QueryPerformanceCounter(&EndOfWait);
            if(EndOfWait.QuadPart -  StartOfWait.QuadPart > 150000)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_RESET, 0);
                //  after tell the card to reset just exit out and we will probably show garbage here
                break;      
            }
            Counter = 0;
        }
    }
	if (bLate)
	{
		if (bAlwaysSleep)
		{
			Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
		}
		Timing_IncrementNotWaitedFields();
	}

    FieldDistance = (10 + NewPos - OldPos) % 10;
    if(FieldDistance == 1)
    {
        pInfo->bMissedFrame = FALSE;
		if (bLate)
		{
            LOG(2, " Running late but right field");
			if (pInfo->bRunningLate)
			{
				Timing_AddDroppedFields(1);
			}
		}
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        NewPos = (OldPos + 1) % 10;
        pInfo->bMissedFrame = FALSE;
        Timing_AddLateFields(FieldDistance - 1);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Field(s)", FieldDistance - 1);
    }

    switch(NewPos)
    {
    case 0: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 4; break;
    case 1: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 0; break;
    case 2: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 0; break;
    case 3: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 1; break;
    case 4: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 1; break;
    case 5: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 2; break;
    case 6: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 2; break;
    case 7: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 3; break;
    case 8: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 3; break;
    case 9: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 4; break;
    }
}

void CBT848Source::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
	BOOL bLate = TRUE;
    int NewPos;
    int FieldDistance;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    static int FieldCount(-1);
    int Counter(0);
    LARGE_INTEGER StartOfWait;

    QueryPerformanceCounter(&StartOfWait);
    
    while(OldPos == (NewPos = GetRISCPosAsInt()))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        bLate = FALSE;							// if we waited then we are not late
        if(++Counter == 1000)
        {
            //check we're not in a tight loop here for too long
            //sometimes boards with external chips seem to hang and need
            //resetting, for other boards this won't do any harm (hopefully)
            LARGE_INTEGER EndOfWait;
            QueryPerformanceCounter(&EndOfWait);
            if(EndOfWait.QuadPart -  StartOfWait.QuadPart > 150000)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_RESET, 0);
                //  after tell the card to reset just exit out and we will probably show garbage here
                break;      
            }
            Counter = 0;
        }
    }
	if (bLate)
	{
		Timing_IncrementNotWaitedFields();
	}

    FieldDistance = (10 + NewPos - OldPos) % 10;
    if(FieldDistance == 1)
    {
        // No skipped fields, do nothing
		if (bLate)
		{
            LOG(2, " Running late but right field");
		}
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        NewPos = (OldPos + 1) % 10;
        Timing_SetFlipAdjustFlag(TRUE);
        Timing_AddLateFields(FieldDistance - 1);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Fields", FieldDistance - 1);
        Timing_Reset();
        FieldCount = -1;
    }

    switch(NewPos)
    {
    case 0: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 4; break;
    case 1: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 0; break;
    case 2: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 0; break;
    case 3: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 1; break;
    case 4: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 1; break;
    case 5: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 2; break;
    case 6: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 2; break;
    case 7: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 3; break;
    case 8: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 3; break;
    case 9: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 4; break;
    }
    
    // do input frequency on cleanish field changes only
	if (FieldCount != -1)
	{
	    FieldCount += FieldDistance;
	}
    if(FieldDistance == 1 && !bLate)
    {
	    if(FieldCount > 1)
		{
	        Timing_UpdateRunningAverage(pInfo, FieldCount);
	        FieldCount = 0;
		}
		else if (FieldCount == -1)
		{
	        FieldCount = 0;
		}
    }

	if (bAlwaysSleep || !bLate)
	{
	    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
	}
}

void CBT848Source::VideoSourceOnChange(long NewValue, long OldValue)
{
    Audio_Mute(PreSwitchMuteDelay);

    Stop_Capture();

    SettingsMaster->SaveSettings();

    // Capture is stopped so other onchange messages are
    // disabled so if anything that happens in those needs to be triggered
    // we have to manage that ourselves

    // here we have to watch for a format switch


    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_PRECHANGE, OldValue, NewValue);
    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, OldValue, NewValue);

    int OldFormat = m_VideoFormat->GetValue();
    
    // set up channel
    // this must happen after the VideoInput change is sent
    if(m_pBT848Card->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
    }
	else
	{
		// We read what is the video format saved for this video input
	    SettingsMaster->LoadOneSetting(m_VideoFormat);
	}

    // tell the world if the format has changed
    if(OldFormat != m_VideoFormat->GetValue())
    {
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldFormat, m_VideoFormat->GetValue());
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldFormat, m_VideoFormat->GetValue());

		// We save the video format attached to this video input
	    SettingsMaster->WriteOneSetting(m_VideoFormat);
    }

    // make sure the defaults are correct
    // but don't change the values
    ChangeDefaultsForSetup(SETUP_CHANGE_ANY, TRUE);

    SettingsMaster->LoadSettings();

    // reset here when we have all the settings
    Reset();
    
    Audio_Unmute(PostSwitchMuteDelay);
    Start_Capture();
}

void CBT848Source::VideoFormatOnChange(long NewValue, long OldValue)
{
    Stop_Capture();

    SettingsMaster->SaveSettings();

    // OK Capture is stopped so other onchange messages are
    // disabled so if anything that happens in those needs to be triggered
    // we have to manage that ourselves

    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldValue, NewValue);   
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldValue, NewValue);
    
    // make sure the defaults are correct
    // but don't change the values
    ChangeDefaultsForSetup(SETUP_CHANGE_ANY, TRUE);

    SettingsMaster->LoadSettings();

    // reset here when we have all the settings
    Reset();

    Start_Capture();
}

void CBT848Source::PixelWidthOnChange(long NewValue, long OldValue)
{
    if(NewValue != 768 &&
        NewValue != 754 &&
        NewValue != 720 &&
        NewValue != 640 &&
        NewValue != 384 &&
        NewValue != 320)
    {
        m_CustomPixelWidth->SetValue(NewValue);
    }
    Stop_Capture();
    m_CurrentX = NewValue;
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    
    NotifySizeChange();

    Start_Capture();
}

void CBT848Source::HDelayOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
}

void CBT848Source::VDelayOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
}

void CBT848Source::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pBT848Card->SetContrastBrightness(m_Contrast->GetValue(), Brightness);
}

void CBT848Source::BtWhiteCrushUpOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushUp(NewValue);
}

void CBT848Source::BtWhiteCrushDownOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushDown(NewValue);
}

void CBT848Source::BtCoringOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetCoring(NewValue);
}

void CBT848Source::BtColorKillOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetColorKill(NewValue);
}

void CBT848Source::BDelayOnChange(long NewValue, long OldValue)
{
    // zero means use format's default value
    if(NewValue != 0)
    {
        m_pBT848Card->SetBDelay(NewValue);  
    }
    else
    {
        m_pBT848Card->SetBDelay(GetTVFormat(GetFormat())->bDelayB);  
    }
}

void CBT848Source::HueOnChange(long Hue, long OldValue)
{
    m_pBT848Card->SetHue(Hue);
}

void CBT848Source::ContrastOnChange(long Contrast, long OldValue)
{
    m_pBT848Card->SetContrastBrightness(Contrast, m_Brightness->GetValue());
}

void CBT848Source::SaturationUOnChange(long SatU, long OldValue)
{
    m_pBT848Card->SetSaturationU(SatU);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatU + m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatU - m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(SatU - m_SaturationV->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CBT848Source::SaturationVOnChange(long SatV, long OldValue)
{
    m_pBT848Card->SetSaturationV(SatV);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatV + m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatV - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(SatV - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}


void CBT848Source::SaturationOnChange(long Sat, long OldValue)
{
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        long NewSaturationU = m_SaturationU->GetValue() + (Sat - OldValue);
        long NewSaturationV = m_SaturationV->GetValue() + (Sat - OldValue);
        m_SaturationU->SetValue(NewSaturationU);
        m_SaturationV->SetValue(NewSaturationV);
        m_Saturation->SetMin(abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CBT848Source::TopOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialTopOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CBT848Source::BottomOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialBottomOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CBT848Source::LeftOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialLeftOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CBT848Source::RightOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialRightOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CBT848Source::TunerTypeOnChange(long TunerId, long OldValue)
{
    m_pBT848Card->InitTuner((eTunerId)TunerId);
}

void CBT848Source::PMSGain1OnChange(long Gain, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_PMSDELUXE || m_CardType->GetValue() == TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetPMSChannelGain(1, Gain);
    }
}

void CBT848Source::PMSGain2OnChange(long Gain, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_PMSDELUXE || m_CardType->GetValue() == TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetPMSChannelGain(2, Gain);
    }
}

void CBT848Source::PMSGain3OnChange(long Gain, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_PMSDELUXE || m_CardType->GetValue() == TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetPMSChannelGain(3, Gain);
    }
}

void CBT848Source::PMSGain4OnChange(long Gain, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_PMSDELUXE || m_CardType->GetValue() == TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetPMSChannelGain(4, Gain);
    }
}

BOOL CBT848Source::IsInTunerMode()
{
    return m_pBT848Card->IsInputATuner(m_VideoSource->GetValue());
}


void CBT848Source::SetupCard()
{
    long OrigTuner = m_TunerType->GetValue();

    if(m_CardType->GetValue() == TVCARD_UNKNOWN)
    {
		m_InitialSetup = TRUE;

        // try to detect the card
        m_CardType->SetValue(m_pBT848Card->AutoDetectCardType());
        m_TunerType->SetValue(m_pBT848Card->AutoDetectTuner((eTVCardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        EnableCancelButton = 0;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
        EnableCancelButton = 1;

        if(m_TunerType->GetValue() != TUNER_ABSENT)
        {
            m_AudioSource1->SetValue(AUDIOINPUT_TUNER);
        }
        else
        {
            m_AudioSource1->SetValue(AUDIOINPUT_EXTERNAL);
        }
    }
    
    m_pBT848Card->SetCardType(m_CardType->GetValue());
    m_pBT848Card->InitTuner((eTunerId)m_TunerType->GetValue());

    // if the tuner has changed during this function
    // change the default format
    // but do so after the Tuner has been set on the card
    if(OrigTuner != m_TunerType->GetValue())
    {
        ChangeTVSettingsBasedOnTuner();

        // All the defaults should be set for NTSC
        // so in case we changed the format based on the tuner
        // reset here, actaully change the values too
        ChangeDefaultsForSetup(SETUP_CHANGE_ANY, FALSE);
    }
    
    InitAudio();

    // set up card specific menu
    DestroyMenu(m_hMenu);
    m_hMenu = m_pBT848Card->GetCardSpecificMenu();
    Providers_UpdateMenu(m_hMenu);

}

void CBT848Source::ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
    // now do defaults based on the processor speed selected
    if(ProcessorSpeed == 0)
    {
        // User has selected below 300 MHz
        m_PixelWidth->ChangeDefault(640);
    }
    else if(ProcessorSpeed == 1)
    {
        // User has selected 300 MHz - 500 MHz
        m_PixelWidth->ChangeDefault(720);
    }
    else if(ProcessorSpeed == 2)
    {
        // User has selected 500 MHz - 1 GHz
        m_PixelWidth->ChangeDefault(720);
    }
    else
    {
        // user has fast processor use best defaults
        m_PixelWidth->ChangeDefault(720);
    }

}

/** ChangeTVSettingsBasedOnTuner
    This function only gets called when the tuner is set
    when the card is first found and all it does is set the default
    video format
*/
void CBT848Source::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        // be a bit defensive here to avoid a possible
        // crash
        if(m_pBT848Card->GetTuner() != NULL)
        {
            eVideoFormat videoFormat = m_pBT848Card->GetTuner()->GetDefaultVideoFormat();
            m_VideoFormat->ChangeDefault(videoFormat);
        }
        else
        {
            LOG(1, " NULL Tuner in ChangeTVSettingsBasedOnTuner");
        }
    }
}


BOOL CBT848Source::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    //Doesn't work yet
    if(VideoFormat == (VIDEOFORMAT_LASTONE+1))
    {
    	return m_pBT848Card->GetTuner()->SetRadioFrequency(FrequencyId);
    }
    
    if(VideoFormat == VIDEOFORMAT_LASTONE)
    {
        VideoFormat = m_pBT848Card->GetTuner()->GetDefaultVideoFormat();
    }
    if(VideoFormat != m_VideoFormat->GetValue())
    {
        m_VideoFormat->SetValue(VideoFormat);
    }    
    return m_pBT848Card->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
}

BOOL CBT848Source::IsVideoPresent()
{
    return m_pBT848Card->IsVideoPresent();
}


void CBT848Source::DecodeVBI(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    BYTE* pVBI = (LPBYTE) m_pVBILines[(pInfo->CurrentFrame + 4) % 5];
    if (m_IsFieldOdd)
    {
        pVBI += m_CurrentVBILines * 2048;
    }
    for (nLineTarget = 0; nLineTarget < m_CurrentVBILines ; nLineTarget++)
    {
       VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget, m_IsFieldOdd);
    }
}

LPCSTR CBT848Source::GetMenuLabel()
{
    return m_pBT848Card->GetCardName(m_pBT848Card->GetCardType());
}

void CBT848Source::SetAspectRatioData()
{
    AspectSettings.InitialTopOverscan = m_TopOverscan->GetValue();
    AspectSettings.InitialBottomOverscan = m_BottomOverscan->GetValue();
    AspectSettings.InitialLeftOverscan = m_LeftOverscan->GetValue();
    AspectSettings.InitialRightOverscan = m_RightOverscan->GetValue();
    AspectSettings.bAnalogueBlanking = FALSE;
}

void CBT848Source::ChannelChange(int PreChange, int OldChannel, int NewChannel)
{    
    if ((PreChange) && m_AutoStereoSelect->GetValue())
    {
        m_AudioChannel->SetValue(SOUNDCHANNEL_MONO);
    }
    
    if (!PreChange && (m_AudioStandardDetect->GetValue()==3))
    {
        m_AudioStandardDetect->SetValue(m_AudioStandardDetect->GetValue());
    } 
    else if ((!PreChange) && m_AutoStereoSelect->GetValue())
    {      
        m_KeepDetectingStereo = 0;
        m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2,
            (m_AutoStereoSelect->GetValue())?SOUNDCHANNEL_STEREO : (eSoundChannel)(m_AudioChannel->GetValue()));
    }
    ///\todo schedule to occur after audio standard / stereo detect
    if (!PreChange && (m_AudioAutoVolumeCorrection->GetValue() > 0))
    {
       //Turn off & on after channel change
       long nDecayTimeIndex = m_AudioAutoVolumeCorrection->GetValue();
       m_AudioAutoVolumeCorrection->SetValue(0);
       m_AudioAutoVolumeCorrection->SetValue(nDecayTimeIndex);
    }
}

int CBT848Source::GetDeviceIndex()
{
    return m_DeviceIndex;
}

const char* CBT848Source::GetChipName()
{
    return m_ChipName.c_str();
}



int  CBT848Source::NumInputs(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_pBT848Card->GetNumInputs();      
  }
  /*
  else if (InputType == AUDIOINPUT)
  {
      return m_pBT848Card->GetNumAudioInputs();      
  }
  */
  return 0;
}

BOOL CBT848Source::SetInput(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      m_VideoSource->SetValue(Nr);
      return TRUE;
  }
  else if (InputType == AUDIOINPUT)
  {      
      m_pBT848Card->SetAudioSource((eAudioInput)Nr);          
      return TRUE;      
  }
  return FALSE;
}

int CBT848Source::GetInput(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_VideoSource->GetValue();
  }
  else if (InputType == AUDIOINPUT)
  {
      return m_pBT848Card->GetAudioInput();    
  }
  return -1;
}

const char* CBT848Source::GetInputName(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      if ((Nr>=0) && (Nr < m_pBT848Card->GetNumInputs()) )
      {
          return m_pBT848Card->GetInputName(Nr);
      }
  } 
  else if (InputType == AUDIOINPUT)
  {      
      return m_pBT848Card->GetAudioInputName((eAudioInput)Nr);
  }
  return NULL;
}

BOOL CBT848Source::InputHasTuner(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        return m_pBT848Card->IsInputATuner(Nr);
    }
    else
    {
        return FALSE;
    }
  }
  return FALSE;
}


ITuner* CBT848Source::GetTuner() 
{
    return m_pBT848Card->GetTuner();
}


CTreeSettingsPage* CBT848Source::GetTreeSettingsPage()
{
    vector <CSimpleSetting*>vSettingsList;

    if(m_CardType->GetValue() != TVCARD_PMSDELUXE && m_CardType->GetValue() != TVCARD_SWEETSPOT)
    {
        vSettingsList.push_back(m_BtAgcDisable);
        vSettingsList.push_back(m_BtCrush);
        vSettingsList.push_back(m_BtEvenChromaAGC);
        vSettingsList.push_back(m_BtOddChromaAGC);
        vSettingsList.push_back(m_BtEvenLumaPeak);
        vSettingsList.push_back(m_BtOddLumaPeak);
        vSettingsList.push_back(m_BtFullLumaRange);
        vSettingsList.push_back(m_BtEvenLumaDec);
        vSettingsList.push_back(m_BtOddLumaDec);
        vSettingsList.push_back(m_BtEvenComb);
        vSettingsList.push_back(m_BtOddComb);
        vSettingsList.push_back(m_BtGammaCorrection);
        vSettingsList.push_back(m_BtCoring);
        vSettingsList.push_back(m_BtHorFilter);
        vSettingsList.push_back(m_BtVertFilter);
        vSettingsList.push_back(m_BtColorKill);
        vSettingsList.push_back(m_BtWhiteCrushUp);
        vSettingsList.push_back(m_BtWhiteCrushDown);
    }
    else
    {
        vSettingsList.push_back(m_PMSGain1);
        vSettingsList.push_back(m_PMSGain2);
        vSettingsList.push_back(m_PMSGain3);
        vSettingsList.push_back(m_PMSGain4);
    }

    vSettingsList.push_back(m_BDelay);
    vSettingsList.push_back(m_BtFullLumaRange);
    vSettingsList.push_back(m_CustomPixelWidth);
    vSettingsList.push_back(m_HDelay);
    vSettingsList.push_back(m_VDelay);
    vSettingsList.push_back(m_ReversePolarity);

    return new CTreeSettingsGeneric("BT8x8 Advanced",vSettingsList);
}
