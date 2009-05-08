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
 * @file ProgramList.cpp Program List functions
 */

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
#include "VBI_VPSdecode.h"
#include "MixerDev.h"
#include "OSD.h"
#include "Providers.h"
#include "SettingsMaster.h"
#include "MultiFrames.h"
#include "EPG.h"

using namespace std;

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

enum eScanMode
{
    SCAN_MODE_PRESETS = 0,
    SCAN_MODE_CUSTOM_ORDER,
    SCAN_MODE_FULL_FREQUENCY,
    SCAN_MODE_LASTONE
};

eScanMode MyScanMode = SCAN_MODE_CUSTOM_ORDER;



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

static int TunerSwitchScreenUpdateDelayTimer = 0;

static int InitialNbMenuItems = -1;


//TODO->Remove this (find a way to store user channels in DScaler APP)
//The implementation is now in Channels
BOOL Load_Program_List_ASCII()
{
    return MyChannels.ReadFile(SZ_DEFAULT_PROGRAMS_FILENAME);
}

void Channel_SetCurrent()
{
    Channel_Change(CurrentProgram);
}

tstring Channel_GetName()
{
    if(CurrentProgram < MyChannels.GetSize())
    {
        return MyChannels.GetChannel(CurrentProgram)->GetName();
    }
    else
    {
        return _T("Unknown");
    }
}


tstring Channel_GetEPGName()
{
    if(CurrentProgram < MyChannels.GetSize())
    {
        return MyChannels.GetChannel(CurrentProgram)->GetEPGName();
    }
    else
    {
        return _T("Unknown");
    }
}


tstring Channel_GetVBIName(BOOL bOnlyWithCodes)
{
    tstring szName(VT_GetStationFromPDC());

    if (szName.empty())
    {
        szName = VPS_GetChannelNameFromCNI();
    }

    if (szName.empty())
    {
        szName = VT_GetStationFromIDP8301();
    }

    if (bOnlyWithCodes == FALSE)
    {
        if (szName.empty())
        {
             szName = VT_GetStation();
        }
    }

    return szName;
}


eVideoFormat SelectedVideoFormat(HWND hDlg)
{
    int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
    if(Format == -1)
    {
        Format = VIDEOFORMAT_LAST_TV;
    }

    return (eVideoFormat)Format;
}

DWORD SelectedScanSteps(HWND hDlg)
{
    static TCHAR sbuf[256];
    sbuf[255] = '\0';
    Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_STEPS), sbuf, 254);
    return (DWORD)_tcstol(sbuf, '\0', 10);
}

void SelectChannel(HWND hDlg, long ChannelToSelect)
{
    const CCountryChannels* channels = MyCountries.GetChannels(CountryCode);

    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), 0);

    for(int i(0); i < ComboBox_GetCount(GetDlgItem(hDlg, IDC_CHANNEL)); ++i)
    {
        CChannel* Channel = (CChannel*)ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), i);
        if(Channel != NULL && Channel->GetChannelNumber() == ChannelToSelect)
        {
            ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), i);
        }
    }
}

void SetFrequencyEditBox(HWND hDlg, long Frequency)
{
    TCHAR sbuf[256];

    _stprintf(sbuf, _T("%10.4lf"), (double)Frequency / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf);
}


void UpdateDetails(HWND hDlg, SmartPtr<CChannel> pChannel)
{
    static TCHAR sbuf[256];
    MyInUpdate = TRUE;
    if (!pChannel)
    {
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), _T(""));
        Edit_SetText(GetDlgItem(hDlg, IDC_EPGNAME), _T(""));
        Edit_SetText(GetDlgItem(hDlg, IDC_FREQUENCY), _T(""));
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), 0);
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_CHANNEL), 0);
        Button_SetCheck(GetDlgItem(hDlg, IDC_ACTIVE), BST_CHECKED);
    }
    else
    {
        // set the name
        Edit_SetText(GetDlgItem(hDlg, IDC_NAME), pChannel->GetName());

        // set the EPG name
        Edit_SetText(GetDlgItem(hDlg, IDC_EPGNAME), pChannel->GetEPGName());

        // set the frequency
        SetFrequencyEditBox(hDlg, pChannel->GetFrequency());

        // set the channel
        SelectChannel(hDlg, pChannel->GetChannelNumber());

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
    if (   (iCurrentProgramIndex >= 0)
        && (iCurrentProgramIndex < MyChannels.GetSize()) )
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
    static TCHAR sbuf[256];

    _stprintf(sbuf, _T("%10.4f"), MyCountries.GetLowerFrequency(CountryCode) / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), sbuf);

    _stprintf(sbuf, _T("%10.4f"), MyCountries.GetHigherFrequency(CountryCode) / 1000000.0);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), sbuf);

    //Keep user steps settings (if any)
    DWORD steps = SelectedScanSteps(hDlg);
    if (steps <= 0)
    {
        steps = SCAN_DEFAULT_STEPS;
    }

    _stprintf(sbuf, _T("%d"), steps);
    Edit_SetText(GetDlgItem(hDlg, IDC_SCAN_STEPS), sbuf);
}


void RefreshProgramList(HWND hDlg, int ProgToSelect)
{
    static TCHAR sbuf[256];
    MyInUpdate = TRUE;
    ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));

    for(int i = 0; i < MyChannels.GetSize(); i++)
    {
        CChannel* Channel = MyChannels.GetChannel(i);
        _stprintf(sbuf, _T("%s"), Channel->GetName());
        ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
    }

    int index =  ProgToSelect;
    if (index >= MyChannels.GetSize())
    {
        index = 0;
    }

    if (index != -1)
    {
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), index);
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), index);
    }
    UpdateDetails(hDlg, index);
    CurrentProgram = index;
    MyInUpdate = FALSE;
}


void RefreshChannelList(HWND hDlg, int iCountryCode)
{
    static TCHAR sbuf[256];
    MyInUpdate = TRUE;

    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CHANNEL));

    int Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), _T("None"));
    ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CHANNEL), Index, 0);

    const CCountryChannels* channels = MyCountries.GetChannels(iCountryCode);
    for(int i = 0; i < channels->GetSize(); i++)
    {
        CChannel* channel = channels->GetChannel(i);
        // Channel names not yet available so adding name
        // to combo a bit pointless, the class does support this for the
        // future though
        //_stprintf(sbuf, _T("%d - %s"), channel->GetChannelNumber(), channel->GetName());
        _stprintf(sbuf, _T("%d"), channel->GetChannelNumber());
        int insertAt = ComboBox_AddString(GetDlgItem(hDlg, IDC_CHANNEL), sbuf);
        ComboBox_SetItemData(GetDlgItem(hDlg, IDC_CHANNEL), insertAt, (LPARAM)channel);
    }
    MyInUpdate = FALSE;
}

void ClearProgramList(HWND hDlg)
{
    MyChannels.Clear();
    ListBox_ResetContent(GetDlgItem(hDlg, IDC_PROGRAMLIST));
    Edit_SetText(GetDlgItem(hDlg, IDC_NAME), _T(""));
    Edit_SetText(GetDlgItem(hDlg, IDC_EPGNAME), _T(""));
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
            Button_Enable(GetDlgItem(hDlg, IDC_ADD), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), enabledButInScan);

            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), bEnabled);
            ComboBox_Enable(GetDlgItem(hDlg, IDC_COUNTRY), bEnabled);

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
            Button_Enable(GetDlgItem(hDlg, IDC_ADD), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), FALSE);

            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), bEnabled);
            ComboBox_Enable(GetDlgItem(hDlg, IDC_COUNTRY), bEnabled);

            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), FALSE);
            Edit_Enable(GetDlgItem(hDlg, IDC_SCAN_STEPS), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_SCAN_AFC), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_UP), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_DOWN), FALSE);
            break;

        case  SCAN_MODE_FULL_FREQUENCY :
            Button_Enable(GetDlgItem(hDlg, IDC_ADD), enabledButInScan);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), enabledButInScan);

            Button_Enable(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), FALSE);
            ComboBox_Enable(GetDlgItem(hDlg, IDC_COUNTRY), bEnabled);

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
    static TCHAR sbuf[256];
    if (Freq <= 0)
    {
        return 0;
    }
    if(Format == -1)
    {
        Format = VIDEOFORMAT_LAST_TV;
    }

    CSource* currentSource = Providers_GetCurrentSource();
    _ASSERTE(NULL != currentSource);

    if (!currentSource->SetTunerFrequency(Freq, (eVideoFormat)Format))
    {
        ErrorBox(MakeString() << _T("SetFrequency ") << (double)Freq / 1000000.0 << _T(" Failed."));
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
    case TUNER_MT2050:
    case TUNER_MT2050_PAL:
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

        if(ElapsedTicks > MaxTuneDelay)
        {
            break;
        }

        ElapsedTicks = GetTickCount() - StartTick;
        Sleep(3);
    }

    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
    {
        SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
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
                            _stprintf(sbuf, _T("Find_Frequency AFC Hit->freq=%d\n"), Freq);
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
    _ASSERTE(NULL != pNewChannel);
    MyChannels.AddChannel(pNewChannel);

    // We are going to add current channel at the end
    // so set up CurrentProgram to be the last one
    // as we will have just tunes to it and it will be
    // what is showing
    CurrentProgram = MyChannels.GetSize() - 1;
    ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), pNewChannel->GetName());
    ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
    UpdateDetails(hDlg, pNewChannel);
}

//Scans the given country preset and adds it to the list
//if found
void ScanChannelPreset(HWND hDlg, int iCurrentChannelIndex, int iCountryCode)
{
    _ASSERTE(iCountryCode >= 0);
    _ASSERTE(iCountryCode < MyCountries.GetSize());

    MyInUpdate = TRUE;

    CChannel* channel = MyCountries.GetChannels(iCountryCode)->GetChannel(iCurrentChannelIndex);
    UpdateDetails(hDlg, channel);

    // don't attempt to scan duplicate frequencies
    if(!MyChannels.GetChannelByFrequency(channel->GetFrequency()))
    {
        DWORD ReturnedFreq = FindFrequency(channel->GetFrequency(), channel->GetFormat(), 0);

        // add channel if frequency found
        if (ReturnedFreq != 0)
        {
            tstring sbuf;

            // if teletext is active then get channel names
            if (   bCaptureVBI
                && (   Setting_GetValue(WM_VBI_GETVALUE, DOTELETEXT)
                    || Setting_GetValue(WM_VBI_GETVALUE, DOVPS)) )
            {
                // Torsten's comment:
                // Because VBI decoding is not stopped before tuned
                // into the new channel we got in some cases old vbi data
                // from the previous tuned channel. So let's at first wait
                // some time to get stable signals from tuner. *After* waiting
                // is done then clear all buffers in vbi decoding and try again.
                Sleep(50);
                VBI_ChannelChange();

                int i = 0;
                while(i < 12 && (sbuf[0] == '\0' || sbuf[0] == ' '))
                {
                    MSG msg;
                    while (PeekMessage(&msg, NULL, 0, 0xffffffff, PM_REMOVE) == TRUE)
                    {
                        SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
                    }

                    // Laurent's comment: Increase the search duration because
                    // 10 * 200 seems to be sometimes not enough for some networks.
                    // The fact that the code ID in P8/30/1 needs to be received
                    // twice with the same value before being used is the main reason.
                    Sleep(200);
                    sbuf = Channel_GetVBIName(TRUE);
                    ++i;
                }
                if(sbuf.empty())
                {
                    sbuf = Channel_GetVBIName(FALSE);
                }
                if(sbuf.empty())
                {
                    tostringstream oss;
                    oss << MyChannels.GetSize() + 1;
                    sbuf = _T("Channel ") + oss.str();
                }
            }
            else
            {
                tostringstream oss;
                oss << MyChannels.GetSize() + 1;
                sbuf = _T("Channel ") + oss.str();
            }
            CChannel* NewChannel = new CChannel(
                                        sbuf.c_str(),
                                        ReturnedFreq,
                                        channel->GetChannelNumber(),
                                        channel->GetFormat(),
                                        TRUE
                                     );
            AddScannedChannel(hDlg, NewChannel);
        }
    }

    MyInUpdate = FALSE;
}

//Scans the given country preset and adds it to the list
// with active set appropriately
void ScanChannelCustom(HWND hDlg, int iCurrentChannelIndex, int iCountryCode)
{
    _ASSERTE(iCountryCode >= 0);
    _ASSERTE(iCountryCode < MyCountries.GetSize());

    MyInUpdate = TRUE;

    CChannel* channel = MyCountries.GetChannels(iCountryCode)->GetChannel(iCurrentChannelIndex);
    UpdateDetails(hDlg, channel);
    DWORD ReturnedFreq = FindFrequency(channel->GetFrequency(), channel->GetFormat(), 0);

    // add channel if frequency found
    // or if we want all the channels
    CChannel* NewChannel = NULL;
    if (ReturnedFreq == 0)
    {
        NewChannel = new CChannel(
                                    channel->GetName(),
                                    channel->GetFrequency(),
                                    channel->GetChannelNumber(),
                                    channel->GetFormat(),
                                    FALSE
                                 );
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
    static TCHAR sbuf[256];

    DWORD afcThreshold = 0;
    if (MyIsUsingAFC)
    {
        //Use the steps - 1 for AFC threshold
        afcThreshold = SelectedScanSteps(hDlg);
    }

    eVideoFormat videoFormat = SelectedVideoFormat(hDlg);

    SetFrequencyEditBox(hDlg, dwFrequency);
    DWORD returned = FindFrequency(dwFrequency, videoFormat, afcThreshold);

    if (returned == 0)
    {

    }
    else
    {
        //returned = returned + afcThreshold;
        _stprintf(sbuf, _T("%10.4lf MHz"), (double)dwFrequency / 1000000.0);

        AddScannedChannel(hDlg,
            new CChannel(
                    sbuf,
                    returned,
                    0,
                    videoFormat,
                    TRUE));

    }

    return returned;
}

//Called when user activates Freq Scanning
void BeginScan(HWND hDlg)
{
    static TCHAR sbuf[256];

    _ASSERTE(FALSE == MyInScan);
    MyInScan = TRUE;

    UpdateEnabledState(hDlg, TRUE);

    CurrentProgram = 0;
    Button_SetText(GetDlgItem(hDlg, IDC_SCAN), _T("Abort"));
    switch (MyScanMode)
    {
        case SCAN_MODE_CUSTOM_ORDER :
            PostMessage(hDlg, WM_SCAN_CUSTOM_ORDER, 0, CountryCode);
            break;

        case SCAN_MODE_PRESETS :
            PostMessage(hDlg, WM_SCAN_PRESET_FREQ, 0, CountryCode);
            break;

        case SCAN_MODE_FULL_FREQUENCY :
            sbuf[255] = '\0';
            Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_MIN_FREQ), sbuf, 254);
            DWORD minFrequency = (DWORD)(_tcstod(sbuf, '\0') * 1000000.0);

            Edit_GetText(GetDlgItem(hDlg, IDC_SCAN_MAX_FREQ), sbuf, 254);
            DWORD maxFrequency = (DWORD)(_tcstod(sbuf, '\0') * 1000000.0);

            PostMessage(hDlg, WM_SCAN_AUTO, minFrequency, maxFrequency);
            break;
    }
    UpdateEnabledState(hDlg, TRUE);
}

//Called when user aborts scan or
//when a WM_SCAN_ABORT is received
void EndScan(HWND hDlg)
{
    _ASSERTE(TRUE == MyInScan);
    MyInScan = FALSE;

    if(MyChannels.GetSize() > 0)
    {
        CurrentProgram = MyChannels.GetSize()-1;
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
        Channel_Change(CurrentProgram);
    }

    Button_SetText(GetDlgItem(hDlg, IDC_SCAN), _T("Scan"));
    UpdateDetails(hDlg, CurrentProgram);
    UpdateEnabledState(hDlg, TRUE);
}

int GetCurrentChannelNumber(HWND hDlg)
{
    int Channel(0);
    int ChannelIndex = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
    if(ChannelIndex != -1)
    {
        CChannel* pChannel = (CChannel*)ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), ChannelIndex);
        if(pChannel != NULL)
        {
            Channel = pChannel->GetChannelNumber();
        }
    }
    return Channel;
}

void ChangeChannelInfo(HWND hDlg, int iCurrentProgramIndex)
{
    MyInUpdate = TRUE;
    TCHAR sbuf[256];
    TCHAR sbuf2[256];

    if(iCurrentProgramIndex < MyChannels.GetSize())
    {
        TCHAR* cLast;
        Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
        double dFreq = _tcstod(sbuf, &cLast);
        long Freq = (long)(dFreq * 1000000.0);
        int Channel = GetCurrentChannelNumber(hDlg);
        int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
        Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf, 255);
        Edit_GetText(GetDlgItem(hDlg, IDC_EPGNAME), sbuf2, 255);
        BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
        MyChannels.SetChannel(iCurrentProgramIndex, new CChannel(sbuf, sbuf2, Freq, Channel, (eVideoFormat)Format, Active));
        ListBox_DeleteString(GetDlgItem(hDlg, IDC_PROGRAMLIST), iCurrentProgramIndex);
        ListBox_InsertString(GetDlgItem(hDlg, IDC_PROGRAMLIST), iCurrentProgramIndex, sbuf);
        ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), iCurrentProgramIndex);
    }
    MyInUpdate = FALSE;
}


void TidyUp(HWND hDlg)
{
    if (MyInScan)
    {
        EndScan(hDlg);
    }
    MyCountries.Clear();
}



BOOL APIENTRY ProgramListProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    TCHAR sbuf[256];
    TCHAR sbuf2[256];
    static eScanMode OldScanMode;
    static int OldCountryCode;
    static BOOL OldIsUsingAFC;
    static int OldCurrentProgram;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            MyInScan = FALSE;
            MyInUpdate = TRUE;

            OldScanMode = MyScanMode;
            OldCountryCode = CountryCode;
            OldCurrentProgram = CurrentProgram;

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
            ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), _T("Same as Tuner"));
            for(i = 0; i < VIDEOFORMAT_LAST_TV; ++i)
            {
                ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), VideoFormatNames[i]);
            }
            ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), _T("Tuner default"));
            ComboBox_AddString(GetDlgItem(hDlg, IDC_FORMAT), _T("FM Radio"));

            // load up the country settings and update the dialog controls
            // with country setting info if relevant
            if (!MyCountries.ReadASCII(SZ_DEFAULT_CHANNELS_FILENAME))
            {
                tstring errorMessage(MakeString() << _T("Channel presets cannot be loaded, \"") <<
                                                    SZ_DEFAULT_CHANNELS_FILENAME << 
                                                    _T("\" is corrupted or missing"));
                ErrorBox(errorMessage);
                EndDialog(hDlg, FALSE);
                return TRUE;
            }

            ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COUNTRY));
            for(i = 0; i < MyCountries.GetSize(); i++)
            {
                ComboBox_AddString(GetDlgItem(hDlg, IDC_COUNTRY), MyCountries.GetCountryName(i));
            }
            ComboBox_AddString(GetDlgItem(hDlg, IDC_COUNTRY), _T("Full Frequency Scan") );

            switch (MyScanMode)
            {
                case SCAN_MODE_CUSTOM_ORDER :
                    Button_SetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), BST_CHECKED);
                    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COUNTRY), CountryCode);
                    RefreshChannelList(hDlg, CountryCode);
                    break;

                case SCAN_MODE_PRESETS :
                    Button_SetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER), BST_UNCHECKED);
                    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COUNTRY), CountryCode);
                    RefreshChannelList(hDlg, CountryCode);
                    break;

                case SCAN_MODE_FULL_FREQUENCY :
                    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COUNTRY), MyCountries.GetSize());
                    break;
            }

            if (!Providers_GetCurrentSource()->IsVideoPresent())
            {
                //make sure it is muted when video is absent
                Audio_SetUserMute(TRUE);
            }
            Button_SetCheck(GetDlgItem(hDlg, IDC_SCAN_AFC), (MyIsUsingAFC) ? BST_CHECKED : BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hDlg, IDC_CHANNEL_MUTE), ((TRUE == Audio_GetUserMute()) ? BST_CHECKED : BST_UNCHECKED));

            RefreshProgramList(hDlg, CurrentProgram);

            MyInUpdate = FALSE;

            // if we have any channels then also fill the details box with the current program
            UpdateDetails(hDlg, CurrentProgram);
            UpdateAutoScanDetails(hDlg);

            //This will trigger an event on IDC_COUNTRY
            SetFocus(GetDlgItem(hDlg, IDC_COUNTRY));
            UpdateEnabledState(hDlg, TRUE);
        }
        // we set focus
        return FALSE;
        break;

    case WM_HSCROLL:
        if(MyInUpdate == FALSE)
        {
            TCHAR* cLast;
            if(LOWORD(wParam) == SB_LEFT ||
                LOWORD(wParam) == SB_PAGELEFT ||
                LOWORD(wParam) == SB_LINELEFT)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = _tcstod(sbuf, &cLast);
                int Freq = (int)(dFreq * 1000000.0);
                Freq -= 62500;
                if(Freq < 0)
                {
                    Freq = 0;
                }
                SetFrequencyEditBox(hDlg, Freq);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            else if(LOWORD(wParam) == SB_RIGHT ||
                LOWORD(wParam) == SB_PAGERIGHT ||
                LOWORD(wParam) == SB_LINERIGHT)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = _tcstod(sbuf, &cLast);
                long Freq = (long)(dFreq * 1000000.0);
                //++Freq;
                Freq += 62500;
                SetFrequencyEditBox(hDlg, Freq);
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
                ScanChannelCustom(hDlg, wParam, lParam);
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
                ScanChannelPreset(hDlg, wParam, lParam);
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
        if (MyInScan == TRUE)
        {
            DWORD newFrequency = ScanFrequency(hDlg, wParam);

            if (newFrequency < lParam)
            {
                if (MyChannels.GetSize() >= MAX_CHANNELS)
                {
                    MessageBox(
                                hDlg,
                                _T("Number of channels limit reached.\nRemove some channels before proceeding further"),
                                _T("Channel Limit Reached"),
                                MB_OK | MB_ICONINFORMATION | MB_APPLMODAL
                              );
                    PostMessage(hDlg, WM_SCAN_ABORT, 0, 0);
                }
                else
                {
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
            if ((HIWORD(wParam) == LBN_SELCHANGE) && (MyInUpdate == FALSE) && (FALSE == MyInScan))
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
            if(HIWORD(wParam) == LBN_SELCHANGE)
            {
                int NewCountryCode = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COUNTRY));
                if(NewCountryCode <  MyCountries.GetSize())
                {
                    CountryCode = NewCountryCode;
                    RefreshChannelList(hDlg, CountryCode);
                    if (Button_GetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER)) == BST_CHECKED)
                    {
                        MyScanMode = SCAN_MODE_CUSTOM_ORDER;
                    }
                    else
                    {
                        MyScanMode = SCAN_MODE_PRESETS;
                    }
                }
                else
                {
                    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_CHANNEL));
                    MyScanMode = SCAN_MODE_FULL_FREQUENCY;
                }
                UpdateAutoScanDetails(hDlg);
                UpdateEnabledState(hDlg, TRUE);
            }
            break;

        case IDC_CUSTOMCHANNELORDER:
            if(HIWORD(wParam) == BN_CLICKED)
            {
                if (Button_GetCheck(GetDlgItem(hDlg, IDC_CUSTOMCHANNELORDER)) == BST_CHECKED)
                {
                    MyScanMode = SCAN_MODE_CUSTOM_ORDER;
                }
                else
                {
                    MyScanMode = SCAN_MODE_PRESETS;
                }

                UpdateEnabledState(hDlg, TRUE);
            }
            break;

        case IDC_CHANNEL:
            if(HIWORD(wParam) == CBN_SELCHANGE && MyInUpdate == FALSE)
            {
                int ChannelNum = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_CHANNEL));
                CChannel* Channel = (CChannel*)ComboBox_GetItemData(GetDlgItem(hDlg, IDC_CHANNEL), ChannelNum);
                if(Channel != NULL && Channel != (CChannel*)(-1))
                {
                    long Freq = Channel->GetFrequency();
                    SetFrequencyEditBox(hDlg, Freq);
                    ScrollBar_SetPos(GetDlgItem(hDlg, IDC_FINETUNE), 50, FALSE);
                    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_FORMAT), Channel->GetFormat() + 1);
                    Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                    ChangeChannelInfo(hDlg, CurrentProgram);
                }
            }
            break;

        case IDC_SETFREQ:
            if(HIWORD(wParam) == BN_CLICKED && MyInUpdate == FALSE)
            {
                TCHAR* cLast;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                double dFreq = _tcstod(sbuf, &cLast);
                long Freq = (long)(dFreq * 1000000.0);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_NAME:
            //(DB) Although it's nice to have
            //the listbox updated immediately,
            //reacting to each textbox change is not good
            //according to the way ChangeChannelInfo is currently implemented
            if(HIWORD(wParam) == EN_CHANGE && MyInUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_EPGNAME:
            //(DB) Same comment as for IDC_NAME
            if(HIWORD(wParam) == EN_CHANGE && MyInUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_ACTIVE:
            //(DB) Same comment as for IDC_NAME
            if(HIWORD(wParam) == BN_CLICKED && MyInUpdate == FALSE)
            {
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_FORMAT:
            if(HIWORD(wParam) == CBN_SELCHANGE && MyInUpdate == FALSE)
            {
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 255);
                TCHAR* cLast;
                double dFreq = _tcstod(sbuf, &cLast);
                long Freq = (long)(dFreq * 1000000.0);
                Providers_GetCurrentSource()->SetTunerFrequency(Freq, SelectedVideoFormat(hDlg));
                ChangeChannelInfo(hDlg, CurrentProgram);
            }
            break;

        case IDC_ADD:
            if(HIWORD(wParam) == BN_CLICKED)
            {
                MyInUpdate = TRUE;
                Edit_GetText(GetDlgItem(hDlg, IDC_FREQUENCY), sbuf, 254);
                sbuf[255] = '\0';
                double dFreq = _tcstod(sbuf, '\0');
                long Freq = (long)(dFreq * 1000000.0);

                int Channel = GetCurrentChannelNumber(hDlg);
                int Format = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FORMAT)) - 1;
                Edit_GetText(GetDlgItem(hDlg, IDC_NAME), sbuf, 255);
                Edit_GetText(GetDlgItem(hDlg, IDC_EPGNAME), sbuf2, 255);
                BOOL Active = (Button_GetCheck(GetDlgItem(hDlg, IDC_ACTIVE)) == BST_CHECKED);
                SmartPtr<CChannel> NewChannel(new CChannel(sbuf, sbuf2, Freq, Channel, (eVideoFormat)Format, Active));
                MyChannels.AddChannel(NewChannel);
                CurrentProgram = ListBox_AddString(GetDlgItem(hDlg, IDC_PROGRAMLIST), sbuf);
                ListBox_SetCurSel(GetDlgItem(hDlg, IDC_PROGRAMLIST), CurrentProgram);
                MyInUpdate = FALSE;
            }
            break;
        case IDC_REMOVE:
            if(HIWORD(wParam) == BN_CLICKED && CurrentProgram >= 0 && CurrentProgram < MyChannels.GetSize())
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
            if(HIWORD(wParam) == BN_CLICKED && CurrentProgram > 0 && CurrentProgram < MyChannels.GetSize())
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
            if(HIWORD(wParam) == BN_CLICKED && CurrentProgram >= 0 && CurrentProgram < MyChannels.GetSize() - 1)
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
            if(HIWORD(wParam) == BN_CLICKED)
            {
                //The enabled state of this button tells us if we can clear or not
                //no need to check any state
                ClearProgramList(hDlg);
            }
            break;

        case IDC_SCAN :
            if(HIWORD(wParam) == BN_CLICKED)
            {
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
                                    _T("You have requested a channel position scan and you already have channels.\n")
                                    _T("Proceeding will delete all your existing setup.\n")
                                    _T("Do you want to proceed and delete the existing list?"),
                                    _T("Clear All"),
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
            }
            break;

        case IDC_SCAN_AFC :
            if(HIWORD(wParam) == BN_CLICKED)
            {
                MyIsUsingAFC = (Button_GetCheck(GetDlgItem(hDlg, IDC_SCAN_AFC)) == BST_CHECKED);
            }
            break;

        case IDC_CHANNEL_MUTE :
            if(HIWORD(wParam) == BN_CLICKED)
            {
                BOOL muteAudio = (Button_GetCheck(GetDlgItem(hDlg, IDC_CHANNEL_MUTE)) == BST_CHECKED);
                Audio_SetUserMute(muteAudio);
            }
            break;

        case IDOK:
            if(HIWORD(wParam) == BN_CLICKED)
            {
                TidyUp(hDlg);
                // try to write out programs
                if (!MyChannels.WriteFile(SZ_DEFAULT_PROGRAMS_FILENAME))
                {
                    tstring dummy(MakeString() << _T("Unable to write to file \n\"") <<
                                                SZ_DEFAULT_PROGRAMS_FILENAME <<
                                                _T("\""));
                    ErrorBox(dummy);
                }
                SettingsMaster->SaveAllSettings(TRUE);
                MyEPG.ReloadEPGData();    // Reload EPG data
                EndDialog(hDlg, TRUE);
            }
            break;

        case IDCANCEL:
            if(HIWORD(wParam) == BN_CLICKED)
            {
                TidyUp(hDlg);
                // revert to previously saved channel list
                MyChannels.Clear();
                MyChannels.ReadFile(SZ_DEFAULT_PROGRAMS_FILENAME);
                // revert to previous settings
                MyScanMode = OldScanMode;
                CountryCode = OldCountryCode;
                CurrentProgram = OldCurrentProgram;
                MyIsUsingAFC = OldIsUsingAFC;
                // Tune in to whatever we were showing before
                // we went into dialog
                Channel_Change(CurrentProgram);
                EndDialog(hDlg, FALSE);
            }
            break;

        case IDC_HELPBTN:
            if(HIWORD(wParam) == BN_CLICKED)
            {
                HtmlHelp(GetMainWnd(), _T("DScaler.chm::/ProgramList.htm"), HH_DISPLAY_TOPIC, 0);
            }
            break;

        default:
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


//---------------------------------------------------------------------------
void Channel_Change(int NewChannel, int DontStorePrevious)
{
    eVideoFormat VideoFormat;

    if (Providers_GetCurrentSource()->HasTuner() == TRUE)
    {
        if(NewChannel >= 0 && NewChannel < MyChannels.GetSize())
        {
            if (MyChannels.GetChannel(NewChannel)->GetFrequency() != 0)
            {
                int OldChannel = CurrentProgram;

                Audio_Mute(PreSwitchMuteDelay);

                // save any channel specific settings
                // do this only when there is a real change
                // this avoids saving before the channel has been set properly on
                // the settings master when the input is changed to tuner
                if(OldChannel != NewChannel)
                {
                    SettingsMaster->SaveGroupedSettings();
                }

                if (EventCollector)
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
                    VideoFormat = MyChannels.GetChannel(CurrentProgram)->GetFormat();
                }
                else
                {
                    VideoFormat = VIDEOFORMAT_LAST_TV;
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
                                                     MyChannels.GetChannel(CurrentProgram)->GetFrequency(),
                                                     VideoFormat))
                    {
                        break;
                    }
                }

                Audio_Unmute(PostSwitchMuteDelay);

                if (EventCollector)
                {
                    EventCollector->RaiseEvent(Providers_GetCurrentSource(), EVENT_CHANNEL_CHANGE, OldChannel, NewChannel);
                }
                // load any channel specific settings
                // do this only when there is a real change
                // must be done after the event is fired as this sets the
                // channel name on the settings master
                if(OldChannel != NewChannel)
                {
                    SettingsMaster->LoadGroupedSettings();
                }

                VBI_ChannelChange();

                StatusBar_ShowText(STATUS_TEXT, MyChannels.GetChannel(CurrentProgram)->GetName());
                OSD_ShowText(MyChannels.GetChannel(CurrentProgram)->GetName(), 0);
                MyEPG.ShowOSD();
                SetTrayTip(MyChannels.GetChannel(CurrentProgram)->GetName());
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
            !MyChannels.GetChannel(CurrentProg)->IsActive())
        {
            ++CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == MyChannels.GetSize())
        {
            CurrentProg = 0;
            while(CurrentProg < MyChannels.GetSize() &&
                !MyChannels.GetChannel(CurrentProg)->IsActive())
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
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, _T("No Channels"));
        OSD_ShowText(_T("No Channels"), 0);
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
            !MyChannels.GetChannel(CurrentProg)->IsActive())
        {
            --CurrentProg;
        }

        // see if we looped around
        if(CurrentProg == -1)
        {
            CurrentProg = MyChannels.GetSize() - 1;
            while(CurrentProg > -1  &&
                !MyChannels.GetChannel(CurrentProg)->IsActive())
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
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, _T("No Channels"));
        OSD_ShowText(_T("No Channels"), 0);
    }
}

void Channel_Previous()
{
    if(MyChannels.GetSize() > 0)
    {
        if (MyChannels.GetChannel(PreviousProgram)->GetFrequency() != 0)
        {
            Channel_Change(PreviousProgram);
        }
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, _T("No Channels"));
        OSD_ShowText(_T("No Channels"), 0);
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
            if (MyChannels.GetChannel(j)->GetFrequency() != 0 && int(MyChannels.GetChannel(j)->GetChannelNumber()) == ChannelNumber)
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
    }
    else
    {
        StatusBar_ShowText(STATUS_TEXT, _T("Not Found"));
        OSD_ShowText(_T("Not Found"), 0);
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
        if ((MyChannels.GetChannel(i)->GetFrequency() != 0) && MyChannels.GetChannel(i)->IsActive())
        {
            // Cut every 28 channels which is ok even when in 640x480
            // For the first column, take into account the first items (InitialNbMenuItems)
            // but reduce by 1 because of the two line separators
            if ((j+InitialNbMenuItems-1) % 28)
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED, IDM_CHANNEL_SELECT + j, MyChannels.GetChannel(i)->GetName());
            }
            else
            {
                AppendMenu(hMenuChannels, MF_STRING | MF_ENABLED | MF_MENUBARBREAK, IDM_CHANNEL_SELECT + j, MyChannels.GetChannel(i)->GetName());
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

    CheckMenuItemBool(hMenuChannels, IDM_CHANNEL_PREVIEW, pMultiFrames && (pMultiFrames->GetMode() == PREVIEW_CHANNELS) && pMultiFrames->IsActive());

    EnableMenuItem(hMenuChannels, IDM_CHANNELPLUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNELMINUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNEL_PREVIOUS, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNEL_LIST, bHasTuner?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenuChannels, IDM_CHANNEL_PREVIEW, bHasTuner && bInTunerMode?MF_ENABLED:MF_GRAYED);

    for (int channelIndex = 0; channelIndex < MyChannels.GetSize() && (j < MAX_CHANNELS); channelIndex++)
    {
        if ((MyChannels.GetChannel(channelIndex)->GetFrequency() != 0) && MyChannels.GetChannel(channelIndex)->IsActive())
        {
            EnableMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, bHasTuner ? MF_ENABLED : MF_GRAYED);
            CheckMenuItem(hMenuChannels, IDM_CHANNEL_SELECT + j, (CurrentProgram == channelIndex) ? MF_CHECKED : MF_UNCHECKED);
            j++;
        }
    }

    // Hide the menu _T("Channels") from the menu bar
    // when the source has no tuner or when the tuner
    // is not the selected input
    HMENU hSubMenu = GetSubMenu(hMenu, 2);
    if (hSubMenu != hMenuChannels)
    {
        if (!bHasTuner || !bInTunerMode)
        {
            RemoveMenu(hMenu, 2, MF_BYPOSITION);
        }
        else
        {
            InsertMenu(hMenu, 2, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)hMenuChannels, _T("&Channels"));
        }
        RedrawMenuBar(hMenu);
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
                if ((MyChannels.GetChannel(channelIndex)->GetFrequency() != 0) && MyChannels.GetChannel(channelIndex)->IsActive())
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
    {
        "Scan Mode", SLIDER, 0, (long*)&MyScanMode,
        0, 0, SCAN_MODE_LASTONE, 1, 1,
        NULL,
        "Show", "ScanMode", NULL,
    },
    {
        "Use AFC While Scanning", ONOFF, 0, (long*)&MyIsUsingAFC,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "ScanUsingAFC", NULL,
    },
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
        600, 0, 1000, 1, 1,
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
