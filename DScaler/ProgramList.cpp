/////////////////////////////////////////////////////////////////////////////
// $Id: ProgramList.cpp,v 1.85 2002-10-28 08:09:33 adcockj Exp $
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
// 26 Dec 2000   Eric Schmidt          Made it possible to have whitespace in
//                                     your channel names in program.txt.
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 11 Mar 2001   Laurent Garnier       Previous Channel feature added
//
// 06 Apr 2001   Laurent Garnier       New menu to select channel
//
// 26 May 2001   Eric Schmidt          Added Custom Channel Order.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.84  2002/10/26 07:37:54  atnak
// Fixed "pre switch mute delay".  (Reverted to the way it was in Rev 1.77)
//
// Revision 1.83  2002/10/25 15:04:39  adcockj
// Made sure current channels are loaded in the list
//
// Revision 1.82  2002/10/25 12:53:44  adcockj
// First cut at making new ProgramList dialog behave properly
//
// Revision 1.81  2002/10/22 05:29:43  flibuste2
// -- Activated AFC option for auto-scanning
//
// Revision 1.80  2002/10/22 00:13:50  flibuste2
// Reverted to 4.01 behaviour
// --Changed : clicking on "keys" no longer clear channe list (clear is done on "scan")
// --Added : auto scan
// --Added mute option
// Fixed a few bugs
// Tidied up code
//
// Revision 1.79  2002/10/17 06:49:27  flibuste2
// -- Adapted to changes from channels
// -- Fixed a numerous number of bug
// -- Removed the drag/drop from channel list to channel settings panel feature
//
// Revision 1.78  2002/10/17 00:28:41  flibuste2
// Channels.h / Channels.cpp define the current CHANNELLIST and COUNTRYLIST
// This first check-in is prior to other enhancements and mostly reproduces
// the existing data structures for channel settings and user program list
//
// Revision 1.77  2002/10/08 20:48:29  kooiman
// Changed to Hz instead of multiple of 62500 Hz.
//
// Revision 1.76  2002/09/30 16:25:18  adcockj
// Corrected problem with API and channel change
//
// Revision 1.75  2002/09/28 13:31:41  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.74  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.73  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.72  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.71  2002/08/18 14:35:29  robmuller
// Changed default.
//
// Revision 1.70  2002/08/16 18:45:56  kooiman
// Added optional screen update delay during tuner frequency switch.
//
// Revision 1.69  2002/08/11 19:53:32  robmuller
// Increased default value of PostSwitchMuteDelay from 20 to 100.
//
// Revision 1.68  2002/08/06 18:35:43  kooiman
// Expandable and more independent channel change notification.
//
// Revision 1.67  2002/08/05 12:04:26  kooiman
// Added functions for channel change notification
//
// Revision 1.66  2002/08/04 12:28:32  kooiman
// Fixed previous channel feature.
//
// Revision 1.65  2002/08/02 21:59:03  laurentg
// Hide the menu "Channels" from the menu bar when the source has no tuner or when the tuner is not the selected input
//
// Revision 1.64  2002/08/02 20:33:52  laurentg
// Menu for channels without inactive channels and cut on several columns
//
// Revision 1.63  2002/08/02 19:33:24  robmuller
// Hide disabled channels from the menu.
//
// Revision 1.62  2002/08/02 18:37:35  robmuller
// Patch #588554 by Markus Debus. Change channel on remove added.
//
// Revision 1.61  2002/07/27 15:20:34  laurentg
// Channels menu updated
//
// Revision 1.60  2002/07/09 17:37:10  robmuller
// Retry on tuner write error.
//
// Revision 1.59  2002/06/18 19:46:06  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.58  2002/06/13 14:00:41  adcockj
// Removed old Settings dialog header
//
// Revision 1.57  2002/06/13 12:10:22  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.56  2002/06/13 10:40:37  robmuller
// Made anti plop mute delay configurable.
//
// Revision 1.55  2002/05/28 11:51:12  robmuller
// Prevent fine tuning to a negative frequency.
//
// Revision 1.54  2002/04/13 18:56:23  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.53  2002/03/13 15:32:45  robmuller
// Fixed problem when selecting None from the channel combo box.
//
// Revision 1.52  2002/03/11 21:38:24  robmuller
// Enabled auto scroll to the program list box.
// Insert icon from the program list is now visible before the channel has moved.
// Moving channel up/down does not scroll the list box anymore.
//
// Revision 1.51  2002/03/10 23:14:45  robmuller
// Added Clear List button.
// Scan no longer clears the program list.
// Position in program list is maintained when removing an item.
// Added support for the delete key in the program list.
// Fixed typos.
//
// Revision 1.50  2002/02/26 19:21:32  adcockj
// Add Format to Channel.txt file changes by Mike Temperton with some extra stuff by me
//
// Revision 1.49  2002/02/24 20:20:12  temperton
// Now we use currently selected video format instead of tuner default
//
// Revision 1.48  2002/02/11 21:23:54  laurentg
// Grayed certain items in the Channels menu when the current input is not the tuner
//
// Revision 1.47  2002/02/09 02:51:38  laurentg
// Grayed the channels when the source has no tuner
//
// Revision 1.46  2002/02/08 08:14:42  adcockj
// Select saved channel on startup if in tuner mode
//
// Revision 1.45  2002/01/26 17:55:13  robmuller
// Added ability to enter frequency directly.
// Fixed: When using the channel combo box the tuner was not set to the new frequency.
//
// Revision 1.44  2002/01/19 17:23:43  robmuller
// Added patch #504738 submitted by Keng Hoo Chuah (hoo)
// (fixed crash if channel.txt does not start with [country])
//
// Revision 1.43  2002/01/17 22:25:23  robmuller
// Channel searching is no longer dependant on the duration of Sleep(3).
// MT2032 channel searching speedup.
//
// Revision 1.42  2001/12/18 14:45:05  adcockj
// Moved to Common Controls status bar
//
// Revision 1.41  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.40  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.39  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.38  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.37  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.36  2001/11/01 12:05:21  laurentg
// Coorection of bug item #477091
//
// Revision 1.35  2001/10/17 11:46:11  adcockj
// Bug fixes
//
// Revision 1.34  2001/09/12 15:59:18  adcockj
// Added mute during scan code
//
// Revision 1.33  2001/08/23 18:54:21  adcockj
// Menu and Settings fixes
//
// Revision 1.32  2001/08/23 16:03:26  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.31.2.7  2001/08/24 12:35:09  adcockj
// Menu handling changes
//
// Revision 1.31.2.6  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.31.2.5  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.31.2.4  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.31.2.3  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.31.2.2  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.31.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.31  2001/08/08 08:47:26  adcockj
// Stopped resetting program list when not in US mode
//
// Revision 1.30  2001/08/06 03:00:17  ericschmidt
// solidified auto-pixel-width detection
// preliminary pausing-of-live-tv work
//
// Revision 1.29  2001/08/05 16:31:34  adcockj
// Fixed crashing with PgUp
//
// Revision 1.28  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.27  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.26  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.25  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "debuglog.h"
#include "settings.h"
#include "TVFormats.h"
#include "Channels.h"
#include "ProgramList.h"
#include "DScaler.h"
#include "VBI.h"
#include "Status.h"
#include "Audio.h"
#include "VBI_VideoText.h"
#include "MixerDev.h"
#include "OSD.h"
#include "Providers.h"

#define MAX_CHANNELS 255

const UINT WM_SCAN_MESSAGE          = WM_USER + 0x700; //completely arbitrary

//The Preset scan scans the selected list/country and adds the listed
//channels to the current user list if they are found
const UINT WM_SCAN_PRESET_FREQ      = WM_SCAN_MESSAGE + 1;

//The auto scan scans a range of frequencies and add found frequencies to current list
const UINT WM_SCAN_AUTO             = WM_SCAN_MESSAGE + 2;

//Custom Order (keyed channels are nb)
const UINT WM_SCAN_CUSTOM_ORDER     = WM_SCAN_MESSAGE + 3;
const UINT WM_SCAN_ABORT            = WM_SCAN_MESSAGE + 4;

const DWORD SCAN_DEFAULT_STEPS      = 62500;

enum SCAN_MODE 
{    
    SCAN_MODE_PRESETS = 0,
    SCAN_MODE_CUSTOM_ORDER,
    SCAN_MODE_AUTOSCAN,
    SCAN_MODE_LASTONE
};

SCAN_MODE MyScanMode = SCAN_MODE_CUSTOM_ORDER;



// From outthreads.cpp
extern BOOL bNoScreenUpdateDuringTuning;


CUserChannels MyChannels;
CCountryList MyCountries;

int CountryCode = 1;

long CurrentProgram = 0;
long PreviousProgram = 0;


BOOL MyInScan = FALSE;
BOOL MyInUpdate = FALSE;
BOOL MyIsUsingAFC = TRUE;
BOOL MyIsAFCSupported = FALSE;

int WM_DRAGLISTMESSAGE = 0;
long DragItemIndex = 0;
     
int PreSwitchMuteDelay = 0;
int PostSwitchMuteDelay = 0;

int TunerSwitchScreenUpdateDelay = 0;

static int PostSwitchMuteTimer = 0;
static int TunerSwitchScreenUpdateDelayTimer = 0;

static int InitialNbMenuItems = -1;


//TODO->Remove this (find a way to store user channels in DScaler APP)
//The implementation is now in Channels
BOOL Load_Program_List_ASCII() {
    return MyChannels.ReadASCII(SZ_DEFAULT_PROGRAMS_FILENAME);
}
                   

void Channel_SetCurrent()
{
    Channel_Change(CurrentProgram);
}

const char* Channel_GetName()
{
    if(CurrentProgram < MyChannels.GetSize())
    {
        return MyChannels.GetChannel(CurrentProgram)->GetName();
    }
    else
    {
        return "Unknown";
    }
}


eVideoFormat SelectedVideoFormat(HWND hDlg)
{
    int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
    if(Format == -1)
    {
        Format = VIDEOFORMAT_LASTONE;
    }

    return (eVideoFormat)Format;
}

DWORD SelectedScanSteps(HWND hDlg)
{
    static char sbuf[256];
    sbuf[255] = '\0';
    Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_STEPS), sbuf, 254);    
    return (DWORD)strtol(sbuf, '\0', 10);
}

void UpdateDetails(HWND hDlg, const CChannel* const pChannel)
{
    static char sbuf[256];
    MyInUpdate = TRUE;
    if (NULL == pChannel)
    {
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), "");
        //ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), 0);
        Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
    }
    else 
    {
        // set the name     
        LPCSTR Name = pChannel->GetName();
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), pChannel->GetName());

        // set the frequency
        sprintf(sbuf, "%10.4f", (double)pChannel->GetFrequency() / 1000000.0);
        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);

        // set the channel
        // select none to start off with
        //SelectChannel(hDlg, (MyChannels.GetChannelNumber(iCurrentProgramIndex)));
        
        // set format
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), pChannel->GetFormat() + 1);

        // set active
        if(pChannel->IsActive())
        {
            Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
        }
        else
        {
            Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_UNCHECKED);
        }       
    }   
    MyInUpdate = FALSE;
}

void UpdateDetails(HWND hDlg, int iCurrentProgramIndex)
{    
    if (iCurrentProgramIndex < MyChannels.GetSize())
    {
        UpdateDetails(hDlg, MyChannels.GetChannel(iCurrentProgramIndex));
    }
    else 
    {
        //dont remove the cast or you'll end up with a stack overflow
        UpdateDetails(hDlg, (CChannel*)NULL);        
    }     

}


void UpdateAutoScanDetails(HWND hDlg)
{
    static char sbuf[256];
        
    sprintf(sbuf, "%10.4f", MyCountries.GetLowerFrequency(CountryCode) / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), sbuf);
    
    sprintf(sbuf, "%10.4f", MyCountries.GetHigherFrequency(CountryCode) / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), sbuf);
    
    //Keep user steps settings (if any)
    DWORD steps = SelectedScanSteps(hDlg);
    if (steps <= 0) 
    {
        steps = SCAN_DEFAULT_STEPS;
    }
   
    sprintf(sbuf, "%d", steps);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_STEPS), sbuf);
}


void RefreshProgramList(HWND hDlg, int ProgToSelect)
{   
    static char sbuf[256];
    MyInUpdate = TRUE;    
    ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
    
    for(int i = 0; i < MyChannels.GetSize(); i++)
    {
        CChannel* channel = MyChannels.GetChannel(i); 
        sprintf(sbuf, "%s", channel->GetName());
        ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
    }

    int index =  ProgToSelect;
    if (index >= MyChannels.GetSize())
    {
        index = 0;
    }

    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), index);           
    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), index);  
    UpdateDetails(hDlg, index); 
    CurrentProgram = index;        
    MyInUpdate = FALSE;
}


void RefreshChannelList(HWND hDlg, int iCountryCode)
{
    static char sbuf[256];
    MyInUpdate = TRUE;    
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CHANNEL));
    
    const CCountryChannels* channels = MyCountries.GetChannels(iCountryCode);
    for(int i = 0; i < channels->GetSize(); i++)
    {
        CChannel* channel = channels->GetChannel(i);
        sprintf(sbuf, "%d - %s", channel->GetChannelNumber(), channel->GetName());
        int insertAt = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), sbuf);
        ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CHANNEL), insertAt, channel->GetChannelNumber());
    }           
    MyInUpdate = FALSE;
}

void ClearProgramList(HWND hDlg)
{
    MyChannels.Clear();
    ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CHANNEL));
    Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
}

//Old RefreshControls (more generic, handles all situations according to current state)
void UpdateEnabledState(HWND hDlg, BOOL bEnabled) 
{
    MyInUpdate = TRUE;

    //That's a shortcut
    BOOL enabledButInScan = (bEnabled && !MyInScan);
    

    //List box is always enabled
    ListBox_Enable(GetDlgItem(hDlg, IDC_PROGRAMLIST), TRUE);
    
    Button_Enable(GetDlgItem(hDlg, IDOK), enabledButInScan);
    Button_Enable(GetDlgItem(hDlg, IDCANCEL), enabledButInScan);
    Button_Enable(GetDlgItem(hDlg, IDC_CLEAR), enabledButInScan);        
        
    Button_Enable(GetDlgItem(hDlg, IDC_SCAN_RADIO1), enabledButInScan);
    Button_Enable(GetDlgItem(hDlg, IDC_SCAN_RADIO2), enabledButInScan);
    
    Button_Enable(GetDlgItem(hDlg, IDC_SETFREQ), enabledButInScan);    
    Button_Enable(GetDlgItem(hDlg, IDC_CHANNEL_MUTE), bEnabled);


    switch (MyScanMode) {    
        case SCAN_MODE_PRESETS :     
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN), bEnabled);    
            Button_Enable(GetDlgItem(hDlg, IDC_SEEK), FALSE);    

            ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_ADD), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), enabledButInScan);            
            
            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), bEnabled);
            Edit_Enable(GetDlgItem(hDlg, IDC_COUNTRY), bEnabled);
            
            //Disable the whole AutoScan Panel
            //(I know it's a copy/paste, but it's easier than a
            //smart algorithm)
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_STEPS), FALSE);             
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_UP), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_DOWN), enabledButInScan);                        
            break;

        case SCAN_MODE_CUSTOM_ORDER :
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN), bEnabled);    
            Button_Enable(GetDlgItem(hDlg, IDC_SEEK), FALSE);    

            ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_ADD), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), FALSE);            
            
            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), bEnabled);
            Edit_Enable(GetDlgItem(hDlg, IDC_COUNTRY), bEnabled);
             
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_STEPS), FALSE);             
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_UP), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_DOWN), FALSE);
            break;
    
        case  SCAN_MODE_AUTOSCAN :  
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN), FALSE);    
            Button_Enable(GetDlgItem(hDlg, IDC_SEEK), bEnabled);    

            ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_ADD), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), enabledButInScan); 

            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_COUNTRY), FALSE);  

            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), enabledButInScan);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), enabledButInScan);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_STEPS), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_UP), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_DOWN), enabledButInScan);
            
            //XXX->This is brutal and dirty and should be changed
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), enabledButInScan && MyIsAFCSupported);
            break;
    }
   
    MyInUpdate = FALSE;
}


//returns TRUE if a video signal is found
DWORD FindFrequency(DWORD Freq, int Format, DWORD dwAFCFrequencyDeviationThreshold)
{
    static char sbuf[256];
    if (Freq <= 0)
    {
        return 0;
    }
    if(Format == -1)
    {
        Format = VIDEOFORMAT_LASTONE;
    }
    
    CSource* currentSource = Providers_GetCurrentSource();
    ASSERT(NULL != currentSource);
    
    if (!currentSource->SetTunerFrequency(Freq, (eVideoFormat)Format))
    {
        sprintf(sbuf, "SetFrequency %10.2lf Failed.", (double) Freq / 1000000.0);
        ErrorBox(sbuf);
        return 0;
    }
 
    int       MaxTuneDelay = 0;

    switch(currentSource->GetTunerId())
    {
        // The MT2032 is a silicon tuner and tunes real fast, no delay needed at this point.
        // Even channels with interference and snow are tuned and detected in about max 80ms,
        // so 120ms seems to be a safe value.
    case TUNER_MT2032:
    case TUNER_MT2032_PAL:
        MaxTuneDelay = 120;
        break;
    default:
        MaxTuneDelay = 225;
        Sleep(100);
        break;
    }

    int       StartTick = 0;
    int       ElapsedTicks = 0;

    StartTick = GetTickCount();
    while (currentSource->IsVideoPresent() == FALSE)
    {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
        {
            SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
        }

        if(ElapsedTicks > MaxTuneDelay)
        {
            break;
        }
 
        ElapsedTicks = GetTickCount() - StartTick;
        Sleep(3);
    }
    
    DWORD returnedFrequency = 0;
    if (currentSource->IsVideoPresent())
    {
        if (dwAFCFrequencyDeviationThreshold > 0)
        {
            long afcDeviation = 0;
            eTunerAFCStatus afcStatus = currentSource->GetTuner()->GetAFCStatus(afcDeviation);           
            switch (afcStatus)
            {
                case TUNER_AFC_NOTSUPPORTED :
                    {
                        //we should have tested this and disabled the checkbox
                        //before...
                        //Use normal behaviour
                        returnedFrequency = Freq;
                    }
                    break;

                case TUNER_AFC_NOCARRIER :
                    {
                        //nothing here
                        returnedFrequency = 0;
                    }
                    break;

                case TUNER_AFC_CARRIER :
                    {
                        //Deviation
                        //Generic tuner : -125000, -62500, 0, +62500, + 125000
                        //(0 is a mostly a hit)
                        //CMT2032 : actual deviation
                        
                        //For the time being, we do not handle a user setting
                        //for this, so we'll use the freq steps set in the dialog box
                        if (afcDeviation < dwAFCFrequencyDeviationThreshold)
                        {
                            sprintf(sbuf, "Find_Frequency AFC Hit->freq=%d\n", Freq);
                            LOGD(sbuf);
                            returnedFrequency = Freq;
                        }
                    }
                    break;

                default :
                    returnedFrequency = 0;
                    break;
            }
        }//if use afc
        else
        {
            returnedFrequency = Freq;
        }
        
    }//if video present
    else 
    {
        returnedFrequency = 0;
    }
    return returnedFrequency;
}

 
void AddScannedChannel(HWND hDlg, CChannel* pNewChannel)
{ 
    ASSERT(NULL != pNewChannel);
    static char sbuf[256];
    MyChannels.AddChannel(pNewChannel);
    
    //Increment current program ?!
    CurrentProgram = MyChannels.GetSize() - 1;         
    sprintf(sbuf, "%s", pNewChannel->GetName());        
    ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
    
    sprintf(sbuf, "%d - %s", pNewChannel->GetChannelNumber(), pNewChannel->GetName());
    int insertAt = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), sbuf);
    ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CHANNEL), insertAt, pNewChannel->GetChannelNumber());    
    
    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
    UpdateDetails(hDlg, pNewChannel);          
}

//Scans the given country preset and adds it to the list
//if found
void ScanChannel(HWND hDlg, int iCurrentChannelIndex, int iCountryCode, BOOL AlwaysAdd)
{            
    ASSERT(iCountryCode >= 0);
    ASSERT(iCountryCode < MyCountries.GetSize());



    MyInUpdate = TRUE; 

    CChannel* channel = MyCountries.GetChannels(iCountryCode)->GetChannel(iCurrentChannelIndex);
    UpdateDetails(hDlg, channel);
    DWORD ReturnedFreq = FindFrequency(channel->GetFrequency(), channel->GetFormat(), 0);

    // add channel if frequency found
    // or if we want all the channels
    CChannel* NewChannel = NULL;
    if (ReturnedFreq == 0)
    {        
        if (TRUE == AlwaysAdd) 
        {
            NewChannel = new CChannel(
                                        channel->GetName(),
                                        channel->GetFrequency(),
                                        channel->GetChannelNumber(),
                                        channel->GetFormat(),
                                        FALSE
                                     );
        }        
    }
    else
    {               
        NewChannel = new CChannel(
                                    channel->GetName(),
                                    ReturnedFreq,
                                    channel->GetChannelNumber(),
                                    channel->GetFormat(),
                                    TRUE
                                 );
    }   

    if (NULL != NewChannel) 
    {
        AddScannedChannel(hDlg, NewChannel);
    }

    MyInUpdate = FALSE;
}

//Does pretty much as ScanChannel but using only dlg settings and given frequency
DWORD ScanFrequency(HWND hDlg, DWORD dwFrequency)
{
    static char sbuf[256];
        
    DWORD afcThreshold = 0;
    if (MyIsUsingAFC)
    {
        //Use the steps - 1 for AFC threshold
        afcThreshold = SelectedScanSteps(hDlg);        
    }

    eVideoFormat videoFormat = SelectedVideoFormat(hDlg);
    
    sprintf(sbuf, "%10.4lf MHz", dwFrequency / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);

    DWORD returned = FindFrequency(dwFrequency, videoFormat, afcThreshold);
      
    if (returned == 0)
    {
    
    }
    else
    {              
        //returned = returned + afcThreshold;                
        AddScannedChannel(hDlg,
            new CChannel(
                    sbuf,
                    returned,
                    MyChannels.GetSize() + 1,
                    videoFormat,
                    TRUE)); 
        
    }
    
    return returned;
}

//Called when user activates Freq Scanning
void BeginScan(HWND hDlg) 
{    
    static char sbuf[256];
    
    ASSERT(FALSE == MyInScan);
    MyInScan = TRUE;
         
    UpdateEnabledState(hDlg, TRUE);
            
    CurrentProgram = 0;        
    switch (MyScanMode) 
    {
        case SCAN_MODE_CUSTOM_ORDER :            
            Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Abort");            
            PostMessage(hDlg, WM_SCAN_CUSTOM_ORDER, 0, CountryCode);            
            break;

        case SCAN_MODE_PRESETS :               
            Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Abort");
            PostMessage(hDlg, WM_SCAN_PRESET_FREQ, 0, CountryCode);
            break;

        case SCAN_MODE_AUTOSCAN :                    
            Button_SetText(GetDlgItem(hDlg, IDC_SEEK), "Abort");
            sbuf[255] = '\0';
            Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), sbuf, 254);                    
            DWORD minFrequency = (DWORD)(strtod(sbuf, '\0') * 1000000.0);                    

            Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), sbuf, 254);                    
            DWORD maxFrequency = (DWORD)(strtod(sbuf, '\0') * 1000000.0);                    
            PostMessage(hDlg, WM_SCAN_AUTO, minFrequency, maxFrequency);
            break;
    }    
    UpdateEnabledState(hDlg, TRUE);
}

//Called when user aborts scan or 
//when a WM_SCAN_ABORT is received
void EndScan(HWND hDlg) 
{
    ASSERT(TRUE == MyInScan);
    MyInScan = FALSE;
    
    if(MyChannels.GetSize() > 0)
    {
        CurrentProgram = MyChannels.GetSize()-1;
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
        Channel_Change(CurrentProgram);
    }    

    Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Scan");      
    Button_SetText(GetDlgItem(hDlg, IDC_SEEK), "Seek");
    UpdateDetails(hDlg, CurrentProgram);
    UpdateEnabledState(hDlg, TRUE);
}


void ChangeChannelInfo(HWND hDlg, int iCurrentProgramIndex)
{
    MyInUpdate = TRUE;
    char sbuf[265];

    if(iCurrentProgramIndex < MyChannels.GetSize())
    {
        char* cLast;
        Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
        double dFreq = strtod(sbuf, &cLast);
        long Freq = (long)(dFreq * 1000000.0);
        int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
        Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
        int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;        
        Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
        BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
        MyChannels.SetChannel(iCurrentProgramIndex, new CChannel(sbuf, Freq, Channel, (eVideoFormat)Format, Active));
        ListBox_DeleteString(GetDlgItem(hDlg, IDC_PROGRAMLIST), iCurrentProgramIndex);
        ListBox_InsertString(GetDlgItem(hDlg, IDC_PROGRAMLIST), iCurrentProgramIndex, sbuf);
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), iCurrentProgramIndex);
    }
    MyInUpdate = FALSE;
}



void CloseDialog(HWND hDlg, BOOL bCancelled)
{    
    if (MyInScan)
    {
        EndScan(hDlg);
    }

    if (TRUE == bCancelled)
    {   
        MyChannels.Clear();
        MyChannels.ReadASCII(SZ_DEFAULT_PROGRAMS_FILENAME);
    }
    else 
    {
        BOOL channelsSaved = FALSE;

        while(!channelsSaved) 
        {
            channelsSaved = MyChannels.WriteASCII(SZ_DEFAULT_PROGRAMS_FILENAME);
            channelsSaved = MyChannels.WriteXML("programs.xml");
            if (!channelsSaved)
            {
                CString dummy("Unable to write to file \n\""); 
                dummy += SZ_DEFAULT_PROGRAMS_FILENAME;
                dummy += "\"";
                channelsSaved = (MessageBox(
                               hDlg, 
                               (LPCSTR)dummy, 
                               "Cannot Save Channels!", 
                               MB_RETRYCANCEL | MB_APPLMODAL | MB_ICONEXCLAMATION) 
                             == IDCANCEL);
                dummy.Empty();
            }
        }//while 
        WriteSettingsToIni(TRUE);
    }

    MyCountries.Clear();
    EndDialog(hDlg, TRUE);
}



BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    char sbuf[256];
    static SCAN_MODE OldScanMode;
    static int OldCountryCode;
    static BOOL OldIsUsingAFC;

    switch (message)
    {
    case WM_INITDIALOG:  
        {
            MyInScan = FALSE;
            MyInUpdate = TRUE;
                
            OldScanMode = MyScanMode;            
            OldCountryCode = CountryCode;
        
            OldIsUsingAFC = MyIsUsingAFC;
            //Test AFC caps of current source
            long afcDeviation = 0;
            MyIsAFCSupported = (Providers_GetCurrentSource()->GetTuner()->GetAFCStatus(afcDeviation) != TUNER_AFC_NOTSUPPORTED);           
            MyIsUsingAFC = MyIsUsingAFC && MyIsAFCSupported;

            SetCapture(hDlg);            
      
            WM_DRAGLISTMESSAGE = RegisterWindowMessage(DRAGLISTMSGSTRING);
            MakeDragList(GetDlgItem(hDlg, IDC_PROGRAMLIST));
               
            ScrollBar_SetRange(GetDlgItem(hDlg, IDC_FINETUNE), 0, 100, FALSE);
            ScrollBar_SetPos(GetDlgItem(hDlg, IDC_FINETUNE), 50, FALSE);
        
            // fill the formats box
            ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "Same as Tuner");
            for(i = 0; i < VIDEOFORMAT_LASTONE; ++i)
            {
                ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), VideoFormatNames[i]);
            }
		    ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "Tuner default");
		    ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), "FM Radio");

            // load up the country settings and update the dialog controls 
            // with country setting info if relevant
            if (!MyCountries.ReadASCII(SZ_DEFAULT_CHANNELS_FILENAME))
            {            
                CString errorMessage("Channel presets cannot be loaded, \"");
                errorMessage = errorMessage + SZ_DEFAULT_CHANNELS_FILENAME;
                errorMessage = errorMessage + "\" is corrupted or missing";            
                ErrorBox((LPCSTR)errorMessage);            
                errorMessage.Empty();                         
            }

            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COUNTRY));            
            for(i = 0; i < MyCountries.GetSize(); i++)
            {
                ComboBox_AddString(GetDlgItem(hDlg, IDC_COUNTRY), MyCountries.GetCountryName(i));                
            }
            ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COUNTRY), 0);
        
            switch (MyScanMode) 
            {
                case SCAN_MODE_CUSTOM_ORDER :
                    Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_RADIO1), BST_CHECKED);                    
                    Button_SetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), BST_CHECKED);
                    break;

                case SCAN_MODE_PRESETS :
                    Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_RADIO1), BST_CHECKED);                                                    
                    break;

                case SCAN_MODE_AUTOSCAN :                
                    Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_RADIO2), BST_CHECKED);                    
                    break;
            }   
            
            if (!Providers_GetCurrentSource()->IsVideoPresent())
            {
                //make sure it is muted when video is absent
                Audio_Mute();
            }
            Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_AFC), (MyIsUsingAFC) ? BST_CHECKED : BST_UNCHECKED);            
            Button_SetCheck(GetDlgItem(hDlg, IDC_CHANNEL_MUTE), ((TRUE == Audio_IsMuted()) ? BST_CHECKED : BST_UNCHECKED));            

            RefreshProgramList(hDlg, 0);            

            MyInUpdate = FALSE;

            RefreshChannelList(hDlg, 0);            
            
            // if we have any channels then also fill the details box with the current program
            UpdateDetails(hDlg, CurrentProgram);
            UpdateAutoScanDetails(hDlg);

            //This will trigger an event on IDC_COUNTRY            
            SetFocus(GetDlgItem(hDlg, IDC_COUNTRY));
            UpdateEnabledState(hDlg, TRUE);            
        }
        break;

    case WM_HSCROLL:
        if(MyInUpdate == FALSE)
        {
            char* cLast;
            if(LOWORD(wParam) == SB_LEFT ||
                LOWORD(wParam) == SB_PAGELEFT ||
                LOWORD(wParam) == SB_LINELEFT)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                int Freq = (int)(dFreq * 1000000.0);
                Freq -= 62500;
                if(Freq < 0)
                {
                    Freq = 0;
                }
                sprintf(sbuf, "%10.4lf", (double)Freq / 1000000.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            else if(LOWORD(wParam) == SB_RIGHT ||
                LOWORD(wParam) == SB_PAGERIGHT ||
                LOWORD(wParam) == SB_LINERIGHT)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 1000000.0);
                //++Freq;
                Freq += 62500;
                sprintf(sbuf, "%10.4f", (double)Freq / 1000000.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
        }
        break;

    //Program listing is empty - we go through the current selected presets,
    //and we add then to the list with the correct active flag
    //wParam = current channel index - lParam = current country code
     case WM_SCAN_CUSTOM_ORDER :
        if (MyInScan == TRUE) 
        {           
            const CChannelList* sourceChannels = MyCountries.GetChannels(lParam);
            if (wParam < sourceChannels->GetSize()) 
            {                
                ScanChannel(hDlg, wParam, lParam, TRUE);
                PostMessage(hDlg, WM_SCAN_CUSTOM_ORDER, wParam + 1, lParam);
            }
            else 
            {
                PostMessage(hDlg, WM_SCAN_ABORT, 0, 0);
            }
        }
        break;
    
    //Program listing is empty - we go through the current selected presets,
    //adding and activating only channels that are found
    //wParam = current channel index - lParam = current country code
    case WM_SCAN_PRESET_FREQ :
        if (MyInScan == TRUE) 
        {     
            const CChannelList* sourceChannels = MyCountries.GetChannels(lParam);
            if (wParam < sourceChannels->GetSize()) 
            {                
                ScanChannel(hDlg, wParam, lParam, FALSE);
                PostMessage(hDlg, WM_SCAN_PRESET_FREQ, wParam + 1, lParam);
            }
            else 
            {
                PostMessage(hDlg, WM_SCAN_ABORT, 0, 0);
            }
        }
        break;
        
        //Scans using settings and adds to current list
        //wParam = current channel frequency - lParam = max frequency
    case WM_SCAN_AUTO :
        if (MyInScan == TRUE) {
                        
            DWORD newFrequency = ScanFrequency(hDlg, wParam);
            
            if (newFrequency < lParam)
            {            
                if (MyChannels.GetSize() >= MAX_CHANNELS) 
                {
                    MessageBox(
                            hDlg, 
                            "Number of channels limit reached.\nRemove some channels before proceeding further", 
                            "Channel Limit Reached", 
                            MB_OK | MB_ICONINFORMATION | MB_APPLMODAL
                            );                    
                    PostMessage(hDlg, WM_SCAN_ABORT, 0, 0);                
                }
                else {
                    PostMessage(hDlg, WM_SCAN_AUTO, wParam + SelectedScanSteps(hDlg), lParam);                
                }
            }
            else 
            {                
                PostMessage(hDlg, WM_SCAN_ABORT, 0, 0);
            }
        }
        break;
    
    case WM_SCAN_ABORT :
        EndScan(hDlg);
        break;
   
    case WM_VKEYTOITEM:
        if(LOWORD(wParam) == VK_DELETE && (HWND)lParam == GetDlgItem(hDlg, IDC_PROGRAMLIST))
        {
            SendMessage(hDlg, WM_COMMAND, IDC_REMOVE, 0);
        }
        // let the list box handle any key presses.
        return -1;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDC_PROGRAMLIST:
            if (MyInUpdate == FALSE && HIWORD(wParam) == LBN_SELCHANGE && (FALSE == MyInScan))
            {
                i = ListBox_GetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST));

                if ((i >= 0) && (i < MyChannels.GetSize()))
                {
                    CurrentProgram = i;
                    Channel_Change(CurrentProgram);
                }
                else
                {
                    CurrentProgram = 0;
                }                
                UpdateDetails(hDlg, CurrentProgram);
                UpdateAutoScanDetails(hDlg);
            }
            break;

        case IDC_COUNTRY:          
            //prevent this event from being processed too often
            //(You get a combo box change event on select, expand and display...)
            if (CountryCode != ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COUNTRY))) 
            {
                CountryCode = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COUNTRY));
                RefreshChannelList(hDlg, CountryCode);  
                UpdateAutoScanDetails(hDlg);
                UpdateEnabledState(hDlg, TRUE);             
            }
            break;

        case IDC_CUSTOMCHANNELORDER:
            if (Button_GetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER)) == BST_CHECKED)
            {                
                MyScanMode = SCAN_MODE_CUSTOM_ORDER;
            }
            else 
            {
                MyScanMode = SCAN_MODE_PRESETS;            
            }            

            UpdateEnabledState(hDlg, TRUE);            
            break;

        case IDC_CHANNEL:
            if(MyInUpdate == FALSE && HIWORD(wParam) == CBN_SELCHANGE)
            {
                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));                
                Channel_Change(Channel, TRUE);                
                ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), Channel);
                CurrentProgram = Channel;     
                UpdateDetails(hDlg, Channel);                   
            }
            break;

        case IDC_SETFREQ:
            if(MyInUpdate == FALSE)
            {
                char* cLast;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 1000000.0);
                sprintf(sbuf, "%10.4lf", (double)Freq / 1000000.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_NAME:
            //(DB) Although it's nice to have 
            //the listbox updated immediately, 
            //reacting to each textbox change is not good
            //according to the way ChangeChannelInfo is currently implemented
            
            if(MyInUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_ACTIVE:
            //(DB) Same comment as for IDC_NAME
            if(MyInUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_FORMAT:
            if(MyInUpdate == FALSE && HIWORD(wParam) == CBN_SELCHANGE)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                char* cLast;
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 1000000.0);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_ADD:
            {
                MyInUpdate = TRUE;                
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 254);
                sbuf[255] = '\0';
                double dFreq = strtod(sbuf, '\0');
                long Freq = (long)(dFreq * 1000000.0);

                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
                int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
                Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
                BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
                MyChannels.AddChannel(sbuf, Freq, Channel, (eVideoFormat)Format, Active);
                CurrentProgram = ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
                ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
                MyInUpdate = FALSE;
            }
            break;
        case IDC_REMOVE:
            if(CurrentProgram >= 0 && CurrentProgram < MyChannels.GetSize())
            {
                int TopIndex = 0;
                MyChannels.RemoveChannel(CurrentProgram);               
                if(CurrentProgram >= MyChannels.GetSize())
                {
                    CurrentProgram = MyChannels.GetSize() - 1;
                }
                Channel_Change(CurrentProgram);
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));                
                RefreshProgramList(hDlg, CurrentProgram);                
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
                UpdateDetails(hDlg, CurrentProgram);
            }
            break;
        case IDC_UP:
            if(CurrentProgram > 0 && CurrentProgram < MyChannels.GetSize())
            {
                int TopIndex = 0;
                MyChannels.SwapChannels(CurrentProgram, CurrentProgram - 1);                               
                --CurrentProgram; 
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;
        case IDC_DOWN:
            if(CurrentProgram >= 0 && CurrentProgram < MyChannels.GetSize() - 1)
            {
                int TopIndex = 0;
                MyChannels.SwapChannels(CurrentProgram, CurrentProgram + 1);
                ++CurrentProgram; 
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;

        case IDC_CLEAR:
            //Be polite and ask user
            /*if (MessageBox(
                        hDlg, 
                        "Are you sure ?", 
                        "Clear All", 
                        MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL)
                    == IDYES)
            {*/
                //The enabled state of this button tells us if we can clear or not
                //no need to check any state
                ClearProgramList(hDlg);
                //This selects "Same as tuner" on "Clear"
                //I'm not sure this is really wanted (you may want to keep current settings)
                //ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
         //   }
            break;

        case IDC_SCAN :
        case IDC_SEEK :
            if(MyInScan == TRUE)
            {
                EndScan(hDlg);
            }
            else
            {
                // It doesn't really make sense to do a US style scan
                // unless the list is empty so tell the user
                if(MyChannels.GetSize() > 0 && MyScanMode == SCAN_MODE_CUSTOM_ORDER)
                {
                    if (MessageBox(
                                hDlg, 
                                "You have requested a channel position scan and you already have channels.\n"
                                "Proceeding will delete all your existing setup.\n"
                                "Do you want to proceed and delete the existing list?", 
                                "Clear All", 
                                MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL)
                            == IDYES)
                    {
                        ClearProgramList(hDlg);
                    }
                    else
                    {
                        return TRUE;
                    }
                }
                BeginScan(hDlg);
            }
            break;

        case IDC_SCAN_AFC :
            {
                MyIsUsingAFC = (Button_GetCheck(GetDlgItem(hDlg, IDC_SCAN_AFC)) == BST_CHECKED);
            }
            break;

        case IDC_CHANNEL_MUTE :    
            {
                BOOL muteAudio = (Button_GetCheck(GetDlgItem(hDlg, IDC_CHANNEL_MUTE)) == BST_CHECKED);
                if (FALSE == muteAudio)
                {
                    Audio_Unmute();
                }
                else 
                {
                    Audio_Mute();
                }
            }
            break;

        case IDOK:   
            CloseDialog(hDlg, FALSE);            
            break;

        case IDCANCEL:
             //bCustomChannelOrder = OldCustom;
            MyScanMode = OldScanMode;
            CountryCode = OldCountryCode;    
            MyIsUsingAFC = OldIsUsingAFC;
            CloseDialog(hDlg, TRUE);
            break;

        case IDC_SCAN_RADIO1 :
            if (Button_GetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER)) == BST_CHECKED)
            {
                MyScanMode = SCAN_MODE_CUSTOM_ORDER;
            }
            else 
            {
                MyScanMode = SCAN_MODE_PRESETS;
            }
            RefreshChannelList(hDlg, CountryCode);
            UpdateEnabledState(hDlg, TRUE);
            break;

        case IDC_SCAN_RADIO2 :
            MyScanMode = SCAN_MODE_AUTOSCAN;
            UpdateEnabledState(hDlg, TRUE);           
            break;
        }
        
        break;
    }   

    if(message == WM_DRAGLISTMESSAGE)
    {
        int Item = 0;
        LPDRAGLISTINFO pDragInfo = (LPDRAGLISTINFO) lParam; 
        switch(pDragInfo->uNotification)
        {
        case DL_BEGINDRAG:
            DragItemIndex = ListBox_GetCurSel(pDragInfo->hWnd);
            SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
            Item = LBItemFromPt(pDragInfo->hWnd, pDragInfo->ptCursor, FALSE);
            DrawInsert(hDlg, pDragInfo->hWnd, Item);
            break;
        case DL_DROPPED:
            DrawInsert(hDlg, pDragInfo->hWnd, -1);               
            Item = LBItemFromPt(pDragInfo->hWnd, pDragInfo->ptCursor, FALSE);
            if((Item >= 0) && (Item != DragItemIndex)) 
            {
                CurrentProgram = DragItemIndex;
                if(Item < DragItemIndex)
                {
                    while(CurrentProgram > Item)
                    {
                        MyChannels.SwapChannels(CurrentProgram, CurrentProgram - 1);
                        --CurrentProgram;
                    }
                }
                else
                {
                    while(CurrentProgram < Item)
                    {
                        MyChannels.SwapChannels(CurrentProgram, CurrentProgram + 1);
                        ++CurrentProgram;
                    }
                }
                CurrentProgram = Item; 
                int TopIndex = 0;
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);                
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;
        case DL_CANCELDRAG:
            DrawInsert(hDlg, pDragInfo->hWnd, -1);
            break;
        case DL_DRAGGING:
            Item = LBItemFromPt(pDragInfo->hWnd, pDragInfo->ptCursor, TRUE);
            DrawInsert(hDlg, pDragInfo->hWnd, Item);
            SetWindowLong(hDlg, DWL_MSGRESULT, DL_MOVECURSOR);
            break;
        }
        return (TRUE);
    }

    return (FALSE);
}


void Channels_Exit()
{    
    MyChannels.Clear();
}


static VOID CALLBACK TunerSwitchScreenUpdateDelayTimerProc( 
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)
{
    TunerSwitchScreenUpdateDelayTimer = 0;
    KillTimer(hwnd, idTimer);    
    bNoScreenUpdateDuringTuning = FALSE;
}

static VOID CALLBACK PostSwitchMuteDelayTimerProc( 
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)
{
    PostSwitchMuteTimer = 0;
    KillTimer(hwnd, idTimer);    
    Audio_Unmute();
}


//---------------------------------------------------------------------------
void Channel_Change(int NewChannel, int DontStorePrevious)
{
    eVideoFormat VideoFormat;
    BOOL audioWasMuted = Audio_IsMuted();

    if (Providers_GetCurrentSource()->HasTuner() == TRUE)
    {
        if(NewChannel >= 0 && NewChannel < MyChannels.GetSize())
        {
            if (MyChannels.GetChannelFrequency(NewChannel) != 0)
            {
				int OldChannel = CurrentProgram;                
                Audio_Mute();             
                Sleep(PreSwitchMuteDelay); // This helps reduce the static click noise.                
                if (EventCollector != NULL)
                {
                    EventCollector->RaiseEvent(Providers_GetCurrentSource(), EVENT_CHANNEL_PRECHANGE, OldChannel, NewChannel);
                }
                if (!DontStorePrevious)
                {
                    PreviousProgram = CurrentProgram;
                }
                CurrentProgram = NewChannel;
                if(MyChannels.GetChannel(CurrentProgram)->GetFormat() != -1)
                {
                    VideoFormat = MyChannels.GetChannelFormat(CurrentProgram);
                }
                else
                {
                    VideoFormat = VIDEOFORMAT_LASTONE;
                }
                
                if (TunerSwitchScreenUpdateDelay > 0)
                {
                    if (TunerSwitchScreenUpdateDelayTimer > 0)
                    {
                        bNoScreenUpdateDuringTuning = FALSE;
                        KillTimer(NULL, TunerSwitchScreenUpdateDelayTimer);
                    }                    
                    TunerSwitchScreenUpdateDelayTimer = SetTimer(NULL, NULL, TunerSwitchScreenUpdateDelay, TunerSwitchScreenUpdateDelayTimerProc);
                    bNoScreenUpdateDuringTuning = TRUE;                
                }
                // try up to three times if something goes wrong.
                // \todo: fix tuner write errors
                for(int i = 0; i < 3; i++)
                {
                    if(Providers_GetCurrentSource()->SetTunerFrequency(
                                                     MyChannels.GetChannelFrequency(CurrentProgram), 
                                                     VideoFormat))
                    {
                        break;
                    }
                }
                if (FALSE == audioWasMuted)
                {
                    if (PostSwitchMuteDelay > 0)
                    {
                        if (PostSwitchMuteTimer > 0)
                        {
                            KillTimer(NULL, PostSwitchMuteTimer);
                        }
                        PostSwitchMuteTimer = SetTimer(NULL, NULL, PostSwitchMuteDelay, PostSwitchMuteDelayTimerProc);
                    }
                    else
                    {
                        Audio_Unmute();                    
                    }
                }
                if (EventCollector != NULL)
                {
                    EventCollector->RaiseEvent(Providers_GetCurrentSource(), EVENT_CHANNEL_CHANGE, OldChannel, NewChannel);
                }
                //Sleep(PostSwitchMuteDelay); //now timer controlled
                VT_ChannelChange();                                

                StatusBar_ShowText(STATUS_TEXT, MyChannels.GetChannel(CurrentProgram)->GetName());
                OSD_ShowText(hWnd,MyChannels.GetChannel(CurrentProgram)->GetName(), 0);
				
            }
        }
    }
}

void Channel_Reset()
{
    Channel_Change(CurrentProgram);
}

void Channel_Increment()
{
    int CurrentProg;

    if(MyChannels.GetSize() > 0)
    {
        CurrentProg = CurrentProgram;
        PreviousProgram = CurrentProg;
        // look for next active channel
        ++CurrentProg;
        while(CurrentProg < MyChannels.GetSize() && 
            !MyChannels.GetChannelActive(CurrentProg))
        {
            ++CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == MyChannels.GetSize())
        {
            CurrentProg = 0;
            while(CurrentProg < MyChannels.GetSize() && 
                !MyChannels.GetChannelActive(CurrentProg))
            {
                ++CurrentProg;
            }

            // see if we looped around again
            if(CurrentProg == MyChannels.GetSize())
            {
                CurrentProg = 0;
            }
        }
    
        Channel_Change(CurrentProg);

        StatusBar_ShowText(STATUS_TEXT, MyChannels.GetChannelName(CurrentProgram));
        OSD_ShowText(hWnd,MyChannels.GetChannelName(CurrentProgram), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "No Channels");
        OSD_ShowText(hWnd, "No Channels", 0);
    }
}

void Channel_Decrement()
{
    int CurrentProg;

    if(MyChannels.GetSize() > 0)
    {
        CurrentProg = CurrentProgram;
        PreviousProgram = CurrentProg;
        // look for next active channel
        --CurrentProg;
        while(CurrentProg > -1 && 
            !MyChannels.GetChannelActive(CurrentProg))
        {
            --CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == -1)
        {
            CurrentProg = MyChannels.GetSize() - 1;
            while(CurrentProg > -1  && 
                !MyChannels.GetChannelActive(CurrentProg))
            {
                --CurrentProg;
            }

            // see if we looped around again
            if(CurrentProg == -1)
            {
                CurrentProg = 0;
            }
        }
    
        Channel_Change(CurrentProg);

        StatusBar_ShowText(STATUS_TEXT, MyChannels.GetChannel(CurrentProgram)->GetName());
        OSD_ShowText(hWnd,MyChannels.GetChannel(CurrentProgram)->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "No Channels");
        OSD_ShowText(hWnd, "No Channels", 0);
    }
}

void Channel_Previous()
{
    if(MyChannels.GetSize() > 0)
    {
        if (MyChannels.GetChannelFrequency(PreviousProgram) != 0)
            Channel_Change(PreviousProgram);

        StatusBar_ShowText(STATUS_TEXT, MyChannels.GetChannel(CurrentProgram)->GetName());
        OSD_ShowText(hWnd,MyChannels.GetChannel(CurrentProgram)->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "No Channels");
        OSD_ShowText(hWnd, "No Channels", 0);
    }

}

void Channel_ChangeToNumber(int ChannelNumber, int DontStorePrevious)
{
    BOOL found = FALSE;

    if (SCAN_MODE_CUSTOM_ORDER == MyScanMode)
    {
        // Find the channel the user typed.
        for (int j = 0; j < MyChannels.GetSize(); ++j)
        {
            if (MyChannels.GetChannelFrequency(j) != 0 && int(MyChannels.GetChannelNumber(j)) == ChannelNumber)
            {
                found = TRUE;
                ChannelNumber = j;
                break;
            }
        }
    }
    else
    {
        found = TRUE;
        ChannelNumber = ChannelNumber - 1;
    }

    if (found)
    {
        Channel_Change(ChannelNumber, DontStorePrevious);
        found = CurrentProgram == ChannelNumber;
    }

    if (found)
    {
        StatusBar_ShowText(STATUS_TEXT, MyChannels.GetChannel(CurrentProgram)->GetName());
        OSD_ShowText(hWnd, MyChannels.GetChannel(CurrentProgram)->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "Not Found");
        OSD_ShowText(hWnd, "Not Found", 0);
    }
}


void Channels_UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuChannels;
    int             j;
    hMenuChannels = GetChannelsSubmenu();
    if(hMenuChannels == NULL) return;

    if (InitialNbMenuItems == -1)
    {
        InitialNbMenuItems = GetMenuItemCount(hMenuChannels);
    }

    j = GetMenuItemCount(hMenuChannels);
    while (j > InitialNbMenuItems)
    {
        --j;
        RemoveMenu(hMenuChannels, j, MF_BYPOSITION);
    }
    
    j = 0;
    for (int i = 0; (i < MyChannels.GetSize()) && (j < MAX_CHANNELS); i++)
    {
        if ((MyChannels.GetChannelFrequency(i) != 0) && MyChannels.GetChannelActive(i))
        {
            // Cut every 28 channels which is ok even when in 640x480
            // For the first column, take into account the first items (InitialNbMenuItems)
            // but reduce by 1 because of the two line separators
            if ((j+InitialNbMenuItems-1) % 28)
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED, IDM_CHANNEL_SELECT + j, MyChannels.GetChannelName(i));
            }
            else
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED | MF_MENUBARBREAK, IDM_CHANNEL_SELECT + j, MyChannels.GetChannelName(i));
            }
            j++;
        }
    }
}

void Channels_SetMenu(HMENU hMenu)
{
    int NDisabledChannels = 0;    
    HMENU hMenuChannels(GetChannelsSubmenu());
    int j = 0;
    if(hMenuChannels == NULL) return;

    if (InitialNbMenuItems == -1)
    {
        InitialNbMenuItems = GetMenuItemCount(hMenuChannels);
    }
    
    BOOL bHasTuner = Providers_GetCurrentSource() ? Providers_GetCurrentSource()->HasTuner() : FALSE;
    BOOL bInTunerMode = Providers_GetCurrentSource() ? Providers_GetCurrentSource()->IsInTunerMode() : FALSE;

    EnableMenuItem(hMenuChannels, IDM_CHANNELPLUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNELMINUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNEL_PREVIOUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNEL_LIST, bHasTuner?MF_ENABLED:MF_GRAYED);
   
    for (int channelIndex = 0; channelIndex < MyChannels.GetSize() && (j < MAX_CHANNELS); channelIndex++)
    {
        if ((MyChannels.GetChannelFrequency(channelIndex) != 0) && MyChannels.GetChannelActive(channelIndex))
        {
            EnableMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, bHasTuner ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, (CurrentProgram == channelIndex) ? MF_CHECKED : MF_UNCHECKED);
            j++;
        }
    }

    // Hide the menu "Channels" from the menu bar
    // when the source has no tuner or when the tuner
    // is not the selected input
    HMENU hSubMenu = GetSubMenu(hMenu, 2);
    if (!bHasTuner || !bInTunerMode)
    {
        if (hSubMenu == hMenuChannels)
        {
            RemoveMenu(hMenu, 2, MF_BYPOSITION);
        }
    }
    else
    {
        if (hSubMenu != hMenuChannels)
        {
            InsertMenu(hMenu, 2, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)hMenuChannels, "&Channels");
        }
    }
}

BOOL ProcessProgramSelection(HWND hWnd, WORD wMenuID)
{
    int j = 0;
    if ( (wMenuID >= IDM_CHANNEL_SELECT) && (wMenuID < (IDM_CHANNEL_SELECT+MAX_CHANNELS)) )
    {
        if (Providers_GetCurrentSource()->IsInTunerMode())
        {            
            for (int channelIndex = 0; channelIndex < MyChannels.GetSize() && (j < MAX_CHANNELS); channelIndex++)
            {
                if ((MyChannels.GetChannelFrequency(channelIndex) != 0) && MyChannels.GetChannelActive(channelIndex))
                {
                    if ((wMenuID - IDM_CHANNEL_SELECT) == j)
                    {
                        Channel_Change(channelIndex);
                        break;
                    }
                    j++;
                }                
            }//for each channel
        }
        else
        {
            SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
            SendMessage(hWnd, WM_COMMAND, wMenuID, 0);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL CurrentProgram_OnChange(long NewValue)
{
    CurrentProgram = NewValue;
    Channel_Change(CurrentProgram);
    return FALSE;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING ChannelsSettings[CHANNELS_SETTING_LASTONE] =
{
    {
        "CountryCode", SLIDER, 0, (long*)&CountryCode,
        1, 0, 100, 1, 1,
        NULL,
        "Show", "CountryCode", NULL,
    },
    {
        "Current Program", SLIDER, 0, (long*)&CurrentProgram,
        0, 0, MAX_CHANNELS, 1, 1,
        NULL,
        "Show", "LastProgram", CurrentProgram_OnChange,
    },
    
    //that is the old settings for scanning
    /*{
        "Custom Channel Order", ONOFF, 0, (long*)&bCustomChannelOrder,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Show", "CustomChannelOrder", NULL,
    }, */ 

    //Which may need to be replaced by this for 
    //backward compat (but according to John, it's not needed)
    /*{
        NULL, SLIDER, 0, NULL,
        0, 0, 0, 0, 0,
        NULL,
        NULL, NULL, NULL,
    },*/
    {
        "Scan Mode", SLIDER, 0, (long*)&MyScanMode,
        0, 0, SCAN_MODE_LASTONE, 1, 1,
        NULL,
        "Show", "ScanMode", NULL,
    }, 

    //Cant commit DS_Control.h for some reason...
    /*{
        "Use AFC While Scanning", ONOFF, 0, (long*)&MyIsUsingAFC,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "ScanUsingAFC", NULL,
    }, */
};

SETTING* Channels_GetSetting(CHANNELS_SETTING Setting)
{
    if(Setting > -1 && Setting < CHANNELS_SETTING_LASTONE)
    {
        return &(ChannelsSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Channels_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < CHANNELS_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(ChannelsSettings[i]));
    }
}

void Channels_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < CHANNELS_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(ChannelsSettings[i]), bOptimizeFileAccess);
    }
}

SETTING AntiPlopSettings[ANTIPLOP_SETTING_LASTONE] =
{
    {
        "Pre switch mute delay", SLIDER, 0, (long*)&PreSwitchMuteDelay,
        100, 0, 1000, 1, 1,
        NULL,
        "Audio", "PreSwitchMuteDelay", NULL,
    },
    {
        "Post switch mute delay", SLIDER, 0, (long*)&PostSwitchMuteDelay,
        150, 0, 1000, 1, 1,
        NULL,
        "Audio", "PostSwitchMuteDelay", NULL,
    },    
};

SETTING* AntiPlop_GetSetting(ANTIPLOP_SETTING Setting)
{
    if(Setting > -1 && Setting < ANTIPLOP_SETTING_LASTONE)
    {
        return &(AntiPlopSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void AntiPlop_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < ANTIPLOP_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(AntiPlopSettings[i]));
    }
}

void AntiPlop_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < ANTIPLOP_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(AntiPlopSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* AntiPlop_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Anti Plop Settings", AntiPlopSettings, ANTIPLOP_SETTING_LASTONE);
}


