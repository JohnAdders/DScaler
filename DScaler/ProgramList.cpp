/////////////////////////////////////////////////////////////////////////////
// $Id: ProgramList.cpp,v 1.78 2002-10-17 00:28:41 flibuste2 Exp $
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
const UINT WM_SCAN_PRESET_FREQ      = WM_SCAN_MESSAGE + 1;
const UINT WM_SCAN_AUTO             = WM_SCAN_MESSAGE + 2;
const UINT WM_SCAN_CUSTOM_ORDER     = WM_SCAN_MESSAGE + 3;
const UINT WM_SCAN_ABORT            = WM_SCAN_MESSAGE + 4;

const DWORD SCAN_DEFAULT_STEPS      = 62500;

enum SCAN_MODE 
{    
    SCAN_MODE_PRESETS = 1,
    SCAN_MODE_CUSTOM_ORDER,
    SCAN_MODE_AUTOSCAN
};

SCAN_MODE MyScanMode = SCAN_MODE_CUSTOM_ORDER;



// From outthreads.cpp
extern BOOL bNoScreenUpdateDuringTuning;


CHANNELLIST MyChannels;
COUNTRYLIST MyCountries;

int CountryCode = 1;

long CurrentProgram = 0;
long PreviousProgram = 0;


BOOL MyInScan = FALSE;
BOOL MyInUpdate = FALSE;
BOOL MyIsAudioMuted = FALSE;

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
    return Load_Program_List_ASCII(SZ_DEFAULT_PROGRAMS_FILENAME, &MyChannels);
}

//This is a helper method (copied from some code)
//that is best for muting/restoring audio.
void MuteAudio(BOOL muted)
{        
    if (MyIsAudioMuted == muted)
    {
        return;
    }
    if (TRUE == muted) 
    {
        Audio_Mute();
    }
    else 
    {
        Audio_Unmute();
    }
    
    Sleep(PreSwitchMuteDelay); // This helps reduce the static click noise. 
    MyIsAudioMuted = muted;
}
                     

void Channel_SetCurrent()
{
    Channel_Change(CurrentProgram);
}

const char* Channel_GetName()
{
    if(CurrentProgram < MyChannels.size())
    {
        return MyChannels[CurrentProgram]->GetName();
    }
    else
    {
        return "Unknown";
    }
}

void SelectChannel(HWND hDlg, long ChannelToSelect)
{
    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), 0);
    // then loop through looking for the correct channel
    for(int i(0); i < MyCountries[CountryCode]->GetSize() + 1; ++i)
    {
        int Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), i);
        if(Channel == ChannelToSelect)
        {
            ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), i);
        }
    }

}

void UpdateDetails(HWND hDlg)
{
    MyInUpdate = TRUE;
    if(CurrentProgram < MyChannels.size())
    {
        char sbuf[256];

        // set the name     
        LPCSTR Name = MyChannels[CurrentProgram]->GetName();
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), Name);

        // set the frequency
        sprintf(sbuf, "%10.4f", (double)(MyChannels[CurrentProgram]->GetFrequency()) / 1000000.0);
        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);

        // set the channel
        // select none to start off with
        SelectChannel(hDlg, (MyChannels[CurrentProgram]->GetChannelNumber()));
        
        // set format
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), (MyChannels[CurrentProgram]->GetFormat() + 1));

        // set active
        if(MyChannels[CurrentProgram]->IsActive())
        {
            Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
        }
        else
        {
            Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_UNCHECKED);
        }
    }
    else
    {
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), "");
        //ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), 0);
        Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
    }
    MyInUpdate = FALSE;
}


void UpdateAutoScanDetails(HWND hDlg)
{
    static char sbuf[256];
    CCountry * currentCountry = MyCountries[CountryCode];    
    
    sprintf(sbuf, "%10.4f", currentCountry->GetMinChannelFrequency() / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), sbuf);
    
    sprintf(sbuf, "%10.4f", currentCountry->GetMaxChannelFrequency() / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), sbuf);
    
    //Keep user steps settings (if any)
    Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_STEPS), sbuf, 254);
    sbuf[255] = '\0';
    DWORD steps = strtol(sbuf, '\0', 10);
    if (steps <= 0) 
    {
        steps = SCAN_DEFAULT_STEPS;
    }

    sprintf(sbuf, "%d", steps);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_STEPS), sbuf);
}

//Updates the Channel ListBox with the current Channels settings
//and selects the given program index
void RefreshProgramList(HWND hDlg, int ProgToSelect)
{
    MyInUpdate = TRUE;
    CHANNELLIST::iterator it;

    ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));

    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), (*it)->GetName());
    }
    
    CurrentProgram = ProgToSelect;
    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), ProgToSelect);    
    UpdateDetails(hDlg);    
    MyInUpdate = FALSE;
}


//Deletes existing channels (including customized settings)
//and restores the current country settings for the given country code
void ResetProgramList(HWND hDlg, int iCountryCode)
{
    MyInUpdate = TRUE;
    CHANNELLIST::iterator it;       
    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        delete *it;
    }
    MyChannels.clear();

    if(SCAN_MODE_CUSTOM_ORDER == MyScanMode)
    {
        for(int i(0); i < MyCountries[iCountryCode]->GetSize(); ++i)
        {
            if(MyCountries[iCountryCode]->GetFrequency(i) != 0)
            {
                char sbuf[256];
                int currentChannel = MyCountries[iCountryCode]->GetMinChannel() + i;
                sprintf(sbuf, "%d", currentChannel);
                MyChannels.push_back(new CChannel(sbuf, 
                                        MyCountries[iCountryCode]->GetFrequency(i),
                                        currentChannel,
                                        MyCountries[iCountryCode]->GetFormat(i),
                                        TRUE));        
            }
        }
        
    }
    
    MyInUpdate = FALSE;
    RefreshProgramList(hDlg, 0);    
}


void RefreshChannelList(HWND hDlg)
{
    static char sbuf[256];
    MyInUpdate = TRUE;
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CHANNEL));
    int Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), "None");
    SendMessage(GetDlgItem(hDlg, IDC_CHANNEL), CB_SETITEMDATA, Index, 0);
    for(int i(0); i < MyCountries[CountryCode]->GetSize(); ++i)
    {
        if(MyCountries[CountryCode]->GetFrequency(i) != 0)
        {    
            sprintf(sbuf, "%d", MyCountries[CountryCode]->GetMinChannel() + i);
            Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), sbuf);
            ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Index, MyCountries[CountryCode]->GetMinChannel() + i);
        }
    }    
    MyInUpdate = FALSE;
}


//returns TRUE if a video signal is found
BOOL FindFrequency(DWORD Freq, int Format, BOOL bUseAfcIfAvailable)
{
    char sbuf[256];

    if(Format == -1)
    {
        Format = VIDEOFORMAT_LASTONE;
    }

    if (!Providers_GetCurrentSource()->SetTunerFrequency(Freq, (eVideoFormat)Format))
    {
        sprintf(sbuf, "SetFrequency %10.2lf Failed.", (double) Freq / 1000000.0);
        ErrorBox(sbuf);
        return false;
    }

    int       MaxTuneDelay = 0;

    switch(Providers_GetCurrentSource()->GetTunerId())
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
    while (Providers_GetCurrentSource()->IsVideoPresent() == FALSE)
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
    return Providers_GetCurrentSource()->IsVideoPresent();
}

void ScanCustomChannel(HWND hDlg, int ChannelNum)
{
    BOOL result;
    MyInUpdate = TRUE;

    if(ChannelNum < 0 || ChannelNum >= MyChannels.size())
    {
        return;
    }

    MyChannels[ChannelNum]->SetActive(FALSE);

    CurrentProgram = ChannelNum;
    UpdateDetails(hDlg);
    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), ChannelNum);

    result = FindFrequency(MyChannels[ChannelNum]->GetFrequency(), MyChannels[ChannelNum]->GetFormat(), FALSE);

    MyChannels[ChannelNum]->SetActive(result);
    MyInUpdate = FALSE;
}

void ScanFrequency(HWND hDlg, int FreqNum)
{
    MyInUpdate = TRUE;

    if(FreqNum < 0 || FreqNum >= MyCountries[CountryCode]->GetSize())
    {
        return;
    }

    char sbuf[256];

    DWORD Freq = MyCountries[CountryCode]->GetFrequency(FreqNum);
    
    if(Freq == 0)
    {
        return;
    }

    for(int i = 0; i < MyChannels.size(); i++)
    {
        if(MyChannels[i]->GetFrequency() == Freq)
        {
            return;
        }
    }    

    int Format = MyCountries[CountryCode]->GetFormat(FreqNum);

    SelectChannel(hDlg, FreqNum + MyCountries[CountryCode]->GetMinChannel());
    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), Format + 1);

    sprintf(sbuf, "%10.4lf", (double)Freq / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);

    if(FindFrequency(Freq, Format, FALSE))
    {
        char sbuf[256];
        ++CurrentProgram;
        sprintf(sbuf, "Channel %d", CurrentProgram);
        MyChannels.push_back(new CChannel(
                                            sbuf, 
                                            Freq, 
                                            MyCountries[CountryCode]->GetMinChannel()+ FreqNum, 
                                            Format, 
                                            TRUE
                                         ));
        ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram - 1);
    }
    MyInUpdate = FALSE;
}


DWORD AutoScanFrequency(HWND hDlg, DWORD dFrequency) 
{    
    static char sbuf[255];
    sprintf(sbuf, "%10.4lf", (double)dFrequency / 1000000.0);
    
    MyInUpdate = TRUE;    
    eVideoFormat videoFormat = (eVideoFormat)(ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1);
    Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
    
    BOOL isAFCInUse =(Button_GetCheck(GetDlgItem(hDlg, IDC_SCAN_AFC)) == BST_CHECKED);
    if (FindFrequency(dFrequency, videoFormat, isAFCInUse)) {        
        
        CChannel * newChannel = new CChannel(
                sbuf, 
                dFrequency, 
                MyChannels.size(), 
                videoFormat, 
                TRUE
            );
        MyChannels.push_back(newChannel);
        ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);        
    }

    MyInUpdate = FALSE;
    return dFrequency;
}


//Old RefreshControls (more generic, handles all situations according to current state)
void UpdateEnabledState(HWND hDlg, BOOL bEnabled) 
{
    MyInUpdate = TRUE;

    //That's a shortcut
    BOOL enabledButMyInScan = (bEnabled && !MyInScan);
    

    //List box is always enabled
    ListBox_Enable(GetDlgItem(hDlg, IDC_PROGRAMLIST), TRUE);

    //Up and Down btn available when not scanning
    //Previous behaviour was to be disabled them when clicking on custom order checkbox    
    Button_Enable(GetDlgItem(hDlg, IDC_UP), enabledButMyInScan);
    Button_Enable(GetDlgItem(hDlg, IDC_DOWN), enabledButMyInScan);
    
    Button_Enable(GetDlgItem(hDlg, IDC_SCAN_RADIO1), enabledButMyInScan);
    Button_Enable(GetDlgItem(hDlg, IDC_SCAN_RADIO2), enabledButMyInScan);
    
    Button_Enable(GetDlgItem(hDlg, IDC_SETFREQ), enabledButMyInScan);    
    Button_Enable(GetDlgItem(hDlg, IDC_CHANNEL_MUTE), bEnabled);

    switch (MyScanMode) {
    
        case SCAN_MODE_PRESETS :            
            Button_Enable(GetDlgItem(hDlg, IDC_CLEAR), enabledButMyInScan);
            ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), bEnabled);
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN), bEnabled);
            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), bEnabled);
            Edit_Enable(GetDlgItem(hDlg, IDC_COUNTRY), bEnabled);

            //Disable the whole AutoScan Panel
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_STEPS), FALSE);             
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), FALSE);

            Button_Enable(GetDlgItem(hDlg, IDC_ADD), enabledButMyInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), enabledButMyInScan);
            break;

        case SCAN_MODE_CUSTOM_ORDER :
            //Disallow clearing the custom order settings
            Button_Enable(GetDlgItem(hDlg, IDC_CLEAR), FALSE);            
            ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), FALSE);

            //I dont really see the point of allowing scanning
            //if we disallow modifying the list
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN), bEnabled);

            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), bEnabled);
            Edit_Enable(GetDlgItem(hDlg, IDC_COUNTRY), bEnabled);
            
            //Disable the whole AutoScan Panel
            //(I know it's a copy/paste, but it's easier than a
            //smart algorithm)
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_STEPS), FALSE);             
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), FALSE);

            Button_Enable(GetDlgItem(hDlg, IDC_ADD), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), FALSE);            
            break;
    
        case  SCAN_MODE_AUTOSCAN :
            Button_Enable(GetDlgItem(hDlg, IDC_CLEAR), enabledButMyInScan);
            ComboBox_Enable(GetDlgItem(hDlg, IDC_CHANNEL), bEnabled);
            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_COUNTRY), FALSE);  

            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), enabledButMyInScan);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), enabledButMyInScan);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_STEPS), enabledButMyInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), enabledButMyInScan);

            Button_Enable(GetDlgItem(hDlg, IDC_ADD), enabledButMyInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), enabledButMyInScan);            
            break;
    }

#pragma message ("DScaler/ProgramList.cpp : Remove the following line when AFC is implemented in autoscan")    
    Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), FALSE);
    
    MyInUpdate = FALSE;
}

//Called when user activates Freq Scanning
void BeginScan(HWND hDlg) 
{    
    static char sbuf[256];

    ASSERT(FALSE == MyInScan);
    MyInScan = TRUE;
    UpdateEnabledState(hDlg, TRUE);

    //Audio_Mute();
    Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Abort");
    CurrentProgram = MyChannels.size();
    
    switch (MyScanMode) 
    {
        case SCAN_MODE_CUSTOM_ORDER :
            PostMessage(hDlg, WM_SCAN_CUSTOM_ORDER, 0, 0);
            break;

        case SCAN_MODE_PRESETS :
            PostMessage(hDlg, WM_SCAN_PRESET_FREQ, 0, 0);
            break;

        case SCAN_MODE_AUTOSCAN :                    
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

    Button_SetText(GetDlgItem(hDlg, IDC_SCAN), "Scan");
    if(MyChannels.size() > 0)
    {
        CurrentProgram = CurrentProgram = MyChannels.size()-1;
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
        Channel_Change(CurrentProgram);
    }    
    if (SCAN_MODE_AUTOSCAN != MyScanMode)
    {
        UpdateDetails(hDlg);
    }
    //Audio_Unmute();
    UpdateEnabledState(hDlg, TRUE);
}


void ChangeChannelInfo(HWND hDlg)
{
    MyInUpdate = TRUE;
    char sbuf[265];

    if(CurrentProgram < MyChannels.size())
    {
        char* cLast;
        Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
        double dFreq = strtod(sbuf, &cLast);
        long Freq = (long)(dFreq * 1000000.0);
        int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
        Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
        int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
        delete MyChannels[CurrentProgram];
        Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
        BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
        MyChannels[CurrentProgram] = new CChannel(sbuf, Freq, Channel, Format, Active);
        ListBox_DeleteString(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
        ListBox_InsertString(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram, sbuf);
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
    }
    MyInUpdate = FALSE;
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


void CloseDialog(HWND hDlg, BOOL bCancelled)
{
    if (MyInScan)
    {
        EndScan(hDlg);
    }

    if (TRUE == bCancelled)
    {       
        Load_Program_List_ASCII(SZ_DEFAULT_PROGRAMS_FILENAME, &MyChannels);
    }
    else 
    {
        BOOL channelsSaved = FALSE;

        while(!channelsSaved) 
        {
            channelsSaved = Write_Program_List_ASCII(SZ_DEFAULT_PROGRAMS_FILENAME, &MyChannels);
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

    Unload_Country_Settings(&MyCountries);
    EndDialog(hDlg, TRUE);
}



BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    char sbuf[256];
    static SCAN_MODE OldScanMode;
    static int OldCountryCode;
    static BOOL OldIsAudioMuted;

    switch (message)
    {
    case WM_INITDIALOG:  
        MyInScan = FALSE;
        MyInUpdate = FALSE;
                
        OldScanMode = MyScanMode;
        OldCountryCode = CountryCode;
        OldIsAudioMuted = MyIsAudioMuted;

        SetCapture(hDlg);
        ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
       
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
        if (Load_Country_Settings(SZ_DEFAULT_CHANNELS_FILENAME, &MyCountries) && (MyCountries.size() > 0)) 
        {
            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COUNTRY));
            i = 0;
            for(COUNTRYLIST::iterator it = MyCountries.begin(); 
                it != MyCountries.end(); 
                ++it)
            {
                ComboBox_AddString(GetDlgItem(hDlg, IDC_COUNTRY), ((*it)->GetName()));
                i++;
            }
            
            switch (MyScanMode) 
            {
                case SCAN_MODE_CUSTOM_ORDER :
                    Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_RADIO1), BST_CHECKED);                    
                    Button_SetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), BST_CHECKED);
                    //Refresh the listbox before displaying
                    
                    break;

                case SCAN_MODE_PRESETS :
                    Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_RADIO1), BST_CHECKED);                                                    
                    break;

                case SCAN_MODE_AUTOSCAN :                
                    Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_RADIO2), BST_CHECKED);                    
                    break;
            }   
            Button_SetCheck(GetDlgItem(hDlg, IDC_CHANNEL_MUTE), MyIsAudioMuted);
            ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COUNTRY), CountryCode);            
            RefreshChannelList(hDlg);

            RefreshProgramList(hDlg, CountryCode);            
            // if we have any channels then also fill the details box with the current program
            UpdateDetails(hDlg);
            UpdateAutoScanDetails(hDlg);

            //This will trigger an event on IDC_COUNTRY            
            SetFocus(GetDlgItem(hDlg, IDC_COUNTRY));
            UpdateEnabledState(hDlg, TRUE);
        }
        else
        {            
            CString errorMessage("No MyCountries Loaded, \"");
            errorMessage = errorMessage + SZ_DEFAULT_CHANNELS_FILENAME;
            errorMessage = errorMessage + "\" must be missing or corrupted";
            
            ErrorBox((LPCSTR)errorMessage);
            
            errorMessage.Empty();
            EndDialog(hDlg, 0);            
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
                ChangeChannelInfo(hDlg);
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
                ChangeChannelInfo(hDlg);
            }
        }
        break;

    case WM_SCAN_CUSTOM_ORDER :
        if (MyInScan == TRUE) {
            ScanCustomChannel(hDlg, wParam);
            if(wParam < MyChannels.size())
            {
                PostMessage(hDlg, WM_SCAN_CUSTOM_ORDER, wParam + 1, 0);
            }
            else
            {          
                EndScan(hDlg);                
            }
        }
        break;
    
    case WM_SCAN_PRESET_FREQ :
        if (MyInScan == TRUE) {
            ScanFrequency(hDlg, wParam);
            if(wParam < MyCountries[CountryCode]->GetSize())
            {
                PostMessage(hDlg, WM_SCAN_PRESET_FREQ, wParam + 1, 0);
            }
            else
            {
                EndScan(hDlg);
            }
        }
        break;
    
    case WM_SCAN_AUTO :
        if (MyInScan == TRUE) {
            
            Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_STEPS), sbuf, 254);
            sbuf[255] = '\0';
            DWORD step = (DWORD)strtol(sbuf, '\0', 10);
            
            DWORD newFrequency = AutoScanFrequency(hDlg, wParam);
            
            if ((newFrequency > 0) && (newFrequency < lParam))
            {            
                if (MyChannels.size() >= MAX_CHANNELS) 
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
                    PostMessage(hDlg, WM_SCAN_AUTO, newFrequency + step, lParam);                
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
            if (MyInUpdate == FALSE && HIWORD(wParam) == LBN_SELCHANGE)
            {
                i = ListBox_GetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST));

                if ((i >= 0) && (i < MyChannels.size()))
                {
                    CurrentProgram = i;
                    Channel_Change(CurrentProgram);
                }
                else
                {
                    CurrentProgram = 0;
                }                
                UpdateDetails(hDlg);
                UpdateAutoScanDetails(hDlg);
            }
            break;

        case IDC_COUNTRY:          
            //prevent this event from being processed too often
            //(You get a combo box change event on select, expand and display...)
            if (CountryCode != ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COUNTRY))) {
                CountryCode = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COUNTRY));
                if (MyScanMode == SCAN_MODE_CUSTOM_ORDER) 
                {
                    ResetProgramList(hDlg, CountryCode);
                }
                else             
                {
                    RefreshChannelList(hDlg);
                }
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
            ResetProgramList(hDlg, CountryCode);
            UpdateEnabledState(hDlg, TRUE);            
            break;

        case IDC_CHANNEL:
            if(MyInUpdate == FALSE && HIWORD(wParam) == CBN_SELCHANGE)
            {
                char sbuf[256];
                // set the frequency
                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                long Freq = 0;
                int Format = -1;
                Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
                if(Channel != 0)
                {
                    Freq = MyCountries[CountryCode]->GetFrequency(Channel - MyCountries[CountryCode]->GetMinChannel());
                    Format = MyCountries[CountryCode]->GetFormat(Channel - MyCountries[CountryCode]->GetMinChannel());
                }
                sprintf(sbuf, "%10.4lf", Freq / 1000000.0);
                Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY),sbuf);
                ScrollBar_SetPos(GetDlgItem(hDlg, IDC_FINETUNE), 50, FALSE);
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), Format + 1);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg);
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
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_NAME:
            if(MyInUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_ACTIVE:
            if(MyInUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg);
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
                ChangeChannelInfo(hDlg);
            }
            break;

        case IDC_ADD:
            {
                MyInUpdate = TRUE;
                char* cLast;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = strtod(sbuf, &cLast);
                long Freq = (long)(dFreq * 1000000.0);

                int Channel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                Channel = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Channel);
                int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
                Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf , 255);
                BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
                MyChannels.push_back(new CChannel(sbuf, Freq, Channel, Format, Active));
                CurrentProgram = ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
                ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
                MyInUpdate = FALSE;
            }
            break;
        case IDC_REMOVE:
            if(CurrentProgram >= 0 && CurrentProgram < MyChannels.size())
            {
                int TopIndex = 0;
                delete MyChannels[CurrentProgram];
                MyChannels.erase(&MyChannels[CurrentProgram]);
                if(CurrentProgram >= MyChannels.size())
                {
                    CurrentProgram = MyChannels.size() - 1;
                }
                Channel_Change(CurrentProgram);
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
                UpdateDetails(hDlg);
            }
            break;
        case IDC_UP:
            if(CurrentProgram > 0 && CurrentProgram < MyChannels.size())
            {
                int TopIndex = 0;
                CChannel* Temp = MyChannels[CurrentProgram];
                MyChannels[CurrentProgram] = MyChannels[CurrentProgram - 1];
                MyChannels[CurrentProgram - 1] = Temp;
                --CurrentProgram; 
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;
        case IDC_DOWN:
            if(CurrentProgram >= 0 && CurrentProgram < MyChannels.size() - 1)
            {
                int TopIndex = 0;
                CChannel* Temp = MyChannels[CurrentProgram];
                MyChannels[CurrentProgram] = MyChannels[CurrentProgram + 1];
                MyChannels[CurrentProgram + 1] = Temp;
                ++CurrentProgram; 
                TopIndex = ListBox_GetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST));
                RefreshProgramList(hDlg, CurrentProgram);
                ListBox_SetTopIndex(GetDlgItem(hDlg, IDC_PROGRAMLIST), TopIndex);
            }
            break;

        case IDC_CLEAR:
            //Be polite and ask user
            if (MessageBox(
                        hDlg, 
                        "Are you sure ?", 
                        "Clear All", 
                        MB_YESNO | MB_ICONQUESTION | MB_APPLMODAL)
                    == IDYES)
            {
                //The enabled state of this button tells us if we can clear or not
                //no need to check any state
                ResetProgramList(hDlg, CountryCode);
                Edit_SetText(GetDlgItem(hDlg, IDC_NAME), "");
                //This selects "Same as tuner" on "Clear"
                //I'm not sure this is really wanted (you may want to keep current settings)
                //ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
            }
            break;

        case IDC_SCAN:
            if(MyInScan == TRUE)
            {
                EndScan(hDlg);
            }
            else
            {
                BeginScan(hDlg);
            }
            break;

        case IDC_CHANNEL_MUTE :    
            {
                BOOL muteAudio = (Button_GetCheck(GetDlgItem(hDlg, IDC_CHANNEL_MUTE)) == BST_CHECKED);
                MuteAudio(muteAudio);            
            }
            break;

        case IDOK:   
            CloseDialog(hDlg, FALSE);            
            break;

        case IDCANCEL:
             //bCustomChannelOrder = OldCustom;
            MyScanMode = OldScanMode;
            CountryCode = OldCountryCode;
            MyIsAudioMuted = OldIsAudioMuted;
            CloseDialog(hDlg, TRUE);
            break;

        case IDC_SCAN_RADIO1 :
            if (Button_GetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER)) == BST_CHECKED)
            {
                MyScanMode = SCAN_MODE_CUSTOM_ORDER;
            }
            else {
                MyScanMode = SCAN_MODE_PRESETS;
            }
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
                CChannel* Temp = MyChannels[DragItemIndex];
                CurrentProgram = DragItemIndex;
                if(Item < DragItemIndex)
                {
                    while(CurrentProgram > Item)
                    {
                        MyChannels[CurrentProgram] = MyChannels[CurrentProgram - 1];
                        --CurrentProgram;
                    }
                }
                else
                {
                    while(CurrentProgram < Item)
                    {
                        MyChannels[CurrentProgram] = MyChannels[CurrentProgram + 1];
                        ++CurrentProgram;
                    }
                }
                MyChannels[Item] = Temp;
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
    CHANNELLIST::iterator it;

    // Zero out the program list
    for(it = MyChannels.begin(); it != MyChannels.end(); ++it)
    {
        delete (*it);
    }
    MyChannels.clear();
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
    BOOL audioWasMuted = MyIsAudioMuted;

    if (Providers_GetCurrentSource()->HasTuner() == TRUE)
    {
        if(NewChannel >= 0 && NewChannel < MyChannels.size())
        {
            if (MyChannels[NewChannel]->GetFrequency() != 0)
            {
				int OldChannel = CurrentProgram;                
                MuteAudio(TRUE);             
                if (EventCollector != NULL)
                {
                    EventCollector->RaiseEvent(Providers_GetCurrentSource(), EVENT_CHANNEL_PRECHANGE, OldChannel, NewChannel);
                }
                if (!DontStorePrevious)
                {
                    PreviousProgram = CurrentProgram;
                }
                CurrentProgram = NewChannel;
                if(MyChannels[CurrentProgram]->GetFormat() != -1)
                {
                    VideoFormat = (eVideoFormat)MyChannels[CurrentProgram]->GetFormat();
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
                                                     MyChannels[CurrentProgram]->GetFrequency(), 
                                                     VideoFormat))
                    {
                        break;
                    }
                }
                
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
                    MuteAudio(audioWasMuted);//not FALSE because it'll overwrite user settings
                }

                if (EventCollector != NULL)
                {
                    EventCollector->RaiseEvent(Providers_GetCurrentSource(), EVENT_CHANNEL_CHANGE, OldChannel, NewChannel);
                }
                //Sleep(PostSwitchMuteDelay); //now timer controlled
                VT_ChannelChange();                                

                StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
                OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
				
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

    if(MyChannels.size() > 0)
    {
        CurrentProg = CurrentProgram;
        PreviousProgram = CurrentProg;
        // look for next active channel
        ++CurrentProg;
        while(CurrentProg < MyChannels.size() && 
            !MyChannels[CurrentProg]->IsActive())
        {
            ++CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == MyChannels.size())
        {
            CurrentProg = 0;
            while(CurrentProg < MyChannels.size() && 
                !MyChannels[CurrentProg]->IsActive())
            {
                ++CurrentProg;
            }

            // see if we looped around again
            if(CurrentProg == MyChannels.size())
            {
                CurrentProg = 0;
            }
        }
    
        Channel_Change(CurrentProg);

        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
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

    if(MyChannels.size() > 0)
    {
        CurrentProg = CurrentProgram;
        PreviousProgram = CurrentProg;
        // look for next active channel
        --CurrentProg;
        while(CurrentProg > -1 && 
            !MyChannels[CurrentProg]->IsActive())
        {
            --CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == -1)
        {
            CurrentProg = MyChannels.size() - 1;
            while(CurrentProg > -1  && 
                !MyChannels[CurrentProg]->IsActive())
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

        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, "No Channels");
        OSD_ShowText(hWnd, "No Channels", 0);
    }
}

void Channel_Previous()
{
    if(MyChannels.size() > 0)
    {
        if (MyChannels[PreviousProgram]->GetFrequency() != 0)
            Channel_Change(PreviousProgram);

        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd,MyChannels[CurrentProgram]->GetName(), 0);
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
        for (int j = 0; j < MyChannels.size(); ++j)
        {
            if (MyChannels[j]->GetFrequency() != 0 && int(MyChannels[j]->GetChannelNumber()) == ChannelNumber)
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
        StatusBar_ShowText(STATUS_TEXT, MyChannels[CurrentProgram]->GetName());
        OSD_ShowText(hWnd, MyChannels[CurrentProgram]->GetName(), 0);
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
    CHANNELLIST::iterator it;
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
    for (it = MyChannels.begin(); it != MyChannels.end() && (j < MAX_CHANNELS); ++it)
    {
        if (((*it)->GetFrequency() != 0) && (*it)->IsActive() )
        {
            // Cut every 28 channels which is ok even when in 640x480
            // For the first column, take into account the first items (InitialNbMenuItems)
            // but reduce by 1 because of the two line separators
            if ((j+InitialNbMenuItems-1) % 28)
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED, IDM_CHANNEL_SELECT + j, (*it)->GetName());
            }
            else
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED | MF_MENUBARBREAK, IDM_CHANNEL_SELECT + j, (*it)->GetName());
            }
            j++;
        }
    }
}

void Channels_SetMenu(HMENU hMenu)
{
    int NDisabledChannels = 0;
    CHANNELLIST::iterator it;
    HMENU hMenuChannels(GetChannelsSubmenu());
    int i, j;
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

    i = j = 0;
    for (it = MyChannels.begin(); it != MyChannels.end() && (j < MAX_CHANNELS); ++it)
    {
        if (((*it)->GetFrequency() != 0) && (*it)->IsActive() )
        {
            EnableMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, bHasTuner ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, (CurrentProgram == i) ? MF_CHECKED : MF_UNCHECKED);
            j++;
        }
        i++;
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
    int i, j;
    CHANNELLIST::iterator it;

    if ( (wMenuID >= IDM_CHANNEL_SELECT) && (wMenuID < (IDM_CHANNEL_SELECT+MAX_CHANNELS)) )
    {
        if (Providers_GetCurrentSource()->IsInTunerMode())
        {
            i = j = 0;
            for (it = MyChannels.begin(); it != MyChannels.end() && (j < MAX_CHANNELS); ++it)
            {
                if (((*it)->GetFrequency() != 0) && (*it)->IsActive() )
                {
                    if ((wMenuID - IDM_CHANNEL_SELECT) == j)
                    {
                        Channel_Change(i);
                        break;
                    }
                    j++;
                }
                i++;
            }
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
#pragma message ("DScaler/ProgramList.cpp : (DB) I dont know what to do with this")
   /* {
        "Custom Channel Order", ONOFF, 0, (long*)&bCustomChannelOrder,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Show", "CustomChannelOrder", NULL,
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


