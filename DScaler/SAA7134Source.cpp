/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source.cpp,v 1.79 2003-05-29 15:55:25 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on BT848Source.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.78  2003/05/26 20:49:04  laurentg
// Corrections for datacasting
// Enable the Cancel button in the setup card dialog box
//
// Revision 1.77  2003/04/17 09:48:36  atnak
// Changed the default for White Peak to be off.
//
// Revision 1.76  2003/04/07 09:17:14  adcockj
// Fixes for correct operation of IsFirstInSeries
//
// Revision 1.75  2003/03/09 19:48:28  laurentg
// Updated field statistics
//
// Revision 1.74  2003/03/08 20:01:26  laurentg
// New setting "always sleep"
//
// Revision 1.73  2003/02/26 20:53:33  laurentg
// New timing setting MaxFieldShift
//
// Revision 1.72  2003/02/25 21:47:05  laurentg
// Slightly update relative to preceeding updates
//
// Revision 1.71  2003/02/22 13:42:42  laurentg
// New counter to count fields runnign late
// Update input frequency on cleanish field changes only which means when the field is no running late
//
// Revision 1.70  2003/02/06 19:45:47  ittarnavsky
// removed dependency on BT848_Defines.h
//
// Revision 1.69  2003/01/27 22:04:12  laurentg
// First step to merge setup hardware and hardware info dialog boxes
// CPU flag information moved in the general hardware dialog box
// Hardware info dialog box available for CX2388x
//
// Revision 1.68  2003/01/23 02:18:45  atnak
// Minor changes
//
// Revision 1.67  2003/01/23 01:52:22  atnak
// Fixed settings
//
// Revision 1.66  2003/01/20 11:35:37  adcockj
// Made Reset call Consistent with other sources
//
// Revision 1.65  2003/01/18 13:55:43  laurentg
// New methods GetHDelay and GetVDelay
//
// Revision 1.64  2003/01/18 10:52:11  laurentg
// SetOverscan renamed SetAspectRatioData
// Unnecessary call to SetOverscan deleted
// Overscan setting specific to calibration deleted
//
// Revision 1.63  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.62  2003/01/13 13:56:28  adcockj
// First attemp at SAA7134 setting groups
//
// Revision 1.61  2003/01/12 16:19:34  adcockj
// Added SettingsGroup activity setting
// Corrected event sequence and channel change behaviour
//
// Revision 1.60  2003/01/11 15:22:27  adcockj
// Interim Checkin of setting code rewrite
//  - Remove CSettingsGroupList class
//  - Fixed bugs in format switching
//  - Some new CSettingGroup code
//
// Revision 1.59  2003/01/11 12:53:58  adcockj
// Interim Check in of settings changes
//  - bug fixes for overlay settings changes
//  - Bug fixes for new settings changes
//  - disables settings per channel completely
//
// Revision 1.58  2003/01/10 17:38:15  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.57  2003/01/08 19:59:38  laurentg
// Analogue Blanking setting by source
//
// Revision 1.56  2003/01/08 00:22:41  atnak
// Put back VBI upscale divisor
//
// Revision 1.55  2003/01/07 23:27:03  laurentg
// New overscan settings
//
// Revision 1.54  2003/01/07 22:59:59  atnak
// Removed variable upscale devisor and locked in at 0x200 scaling
// for 27Mhz VBI stepping
//
// Revision 1.53  2003/01/05 18:35:45  laurentg
// Init function for VBI added
//
// Revision 1.52  2003/01/05 16:54:54  laurentg
// Updated parameters for VBI_DecodeLine
//
// Revision 1.51  2003/01/01 20:56:45  atnak
// Updates for various VideoText changes
//
// Revision 1.50  2002/12/10 12:58:07  adcockj
// Removed NotifyInputChange and NotifyVideoFormatChange functions and replaced with
//  calls to EventCollector->RaiseEvent
//
// Revision 1.49  2002/12/10 12:17:31  atnak
// NotifyInputChange() + VIDEOFORMAT to NotifyVideoFormatChange()
//
// Revision 1.48  2002/12/10 11:05:45  atnak
// Fixed FlyVideo 3000 audio for external inputs
//
// Revision 1.47  2002/12/09 00:32:13  atnak
// Added new muting stuff
//
// Revision 1.46  2002/12/07 15:59:06  adcockj
// Modified mute behaviour
//
// Revision 1.45  2002/11/10 09:30:57  atnak
// Added Chroma only comb filter mode for SECAM
//
// Revision 1.44  2002/11/10 05:11:23  atnak
// Added adjustable audio input level
//
// Revision 1.43  2002/11/08 12:16:12  atnak
// Fixed settings not being set at startup
//
// Revision 1.42  2002/11/07 20:33:17  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.41  2002/11/07 18:54:21  atnak
// Redid getting next field -- fixes some issues
//
// Revision 1.40  2002/10/31 05:39:02  atnak
// Added SoundChannel change event for toolbar
//
// Revision 1.39  2002/10/31 05:02:55  atnak
// Settings cleanup and audio tweaks
//
// Revision 1.38  2002/10/31 03:10:55  atnak
// Changed CSource::GetTreeSettingsPage to return CTreeSettingsPage*
//
// Revision 1.37  2002/10/30 04:35:47  atnak
// Added attempt to reduce driver conflict instability
//
// Revision 1.36  2002/10/30 03:28:21  atnak
// Added helpid to SAA713x Advanced tree settings page
//
// Revision 1.35  2002/10/29 03:07:18  atnak
// Added SAA713x TreeSettings Page
//
// Revision 1.34  2002/10/28 11:10:12  atnak
// Various changes and revamp to settings
//
// Revision 1.33  2002/10/26 17:51:53  adcockj
// Simplified hide cusror code and removed PreShowDialogOrMenu & PostShowDialogOrMenu
//
// Revision 1.32  2002/10/26 16:18:34  atnak
// Added SAA7134_SETTING summing check
//
// Revision 1.31  2002/10/26 05:24:23  atnak
// Minor cleanups
//
// Revision 1.30  2002/10/26 04:42:50  atnak
// Added AGC config and automatic volume leveling control
//
// Revision 1.29  2002/10/23 17:05:19  atnak
// Added variable VBI sample rate scaling
//
// Revision 1.28  2002/10/22 04:08:50  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.27  2002/10/20 09:27:55  atnak
// Fixes negative dropped frames for accurate
//
// Revision 1.26  2002/10/20 07:41:04  atnak
// custom audio standard setup + etc
//
// Revision 1.25  2002/10/15 04:34:26  atnak
// increased the amount of VBI samples to get better decoding
//
// Revision 1.24  2002/10/12 20:03:12  atnak
// added half second wait for DecodeVBI() after channel change
//
// Revision 1.23  2002/10/12 01:37:28  atnak
// fixes negative dropped frames bug
//
// Revision 1.22  2002/10/10 12:13:19  atnak
// fixed reverse polarity for odd before even
//
// Revision 1.21  2002/10/09 13:20:15  atnak
// fixed up field start lines
//
// Revision 1.20  2002/10/08 20:42:05  atnak
// forgot to comment out debug line
//
// Revision 1.19  2002/10/08 20:35:39  atnak
// whitepeak, colorpeak, comb filter UI options
//
// Revision 1.18  2002/10/08 12:30:38  atnak
// tweaks and fixes
//
// Revision 1.17  2002/10/06 09:49:19  atnak
// Smarter GetNextField sleeping
//
// Revision 1.16  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.15  2002/10/03 23:36:22  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.14  2002/09/29 13:53:40  adcockj
// Ensure Correct History stored
//
// Revision 1.13  2002/09/29 10:14:15  adcockj
// Fixed problem with history in OutThreads
//
// Revision 1.12  2002/09/28 13:33:04  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.11  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.10  2002/09/25 15:11:12  adcockj
// Preliminary code for format specific support for settings per channel
//
// Revision 1.9  2002/09/16 20:08:21  adcockj
// fixed format detect for cx2388x
//
// Revision 1.8  2002/09/16 19:34:19  adcockj
// Fix for auto format change
//
// Revision 1.7  2002/09/16 17:52:34  atnak
// Support for SAA7134Res.dll dialogs
//
// Revision 1.6  2002/09/15 14:28:07  atnak
// Tweaked VBI and VDelay settings
//
// Revision 1.5  2002/09/15 14:20:38  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:24:03  atnak
// changed some UI stuff
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Source.h"
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
#include "SAA7134_Defines.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSAA7134Source::CSAA7134Source(CSAA7134Card* pSAA7134Card, CContigMemory* PageTableDMAMem[4], CUserMemory* DisplayDMAMem[2], CUserMemory* VBIDMAMem[2], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex) :
    CSource(WM_SAA7134_GETVALUE, IDC_SAA7134),
    m_pSAA7134Card(pSAA7134Card),
    m_CurrentX(768),
    m_CurrentY(576),
    m_CurrentVBILines(19),
    m_Section(IniSection),
    m_IDString(IniSection),
    m_InSaturationUpdate(FALSE),
    m_ChipName(ChipName),
    m_DeviceIndex(DeviceIndex),
    m_CurrentFieldID(0),
    m_ProcessingFieldID(-1),
    m_hSAA7134ResourceInst(NULL),
    m_DetectedAudioChannel((eAudioChannel)-1),
    m_bSelectCardCancelButton(TRUE)
{
    CreateSettings(IniSection);

    DisableOnChange();

    // Take over the card (not literally)
    m_pSAA7134Card->PrepareCard();

// Used for register guessing!
//  m_pSAA7134Card->DumpRegisters();
//  exit(1);

    for (int i(0); i < 2; ++i)
    {
        m_DisplayPageTableLinear[i] = (DWORD*)PageTableDMAMem[i]->GetUserPointer();
        m_DisplayPageTablePhysical[i] = PageTableDMAMem[i]->TranslateToPhysical(m_DisplayPageTableLinear[i], 4096, NULL);
        m_VBIPageTableLinear[i] = (DWORD*)PageTableDMAMem[i+2]->GetUserPointer();
        m_VBIPageTablePhysical[i] = PageTableDMAMem[i+2]->TranslateToPhysical(m_VBIPageTableLinear[i], 4096, NULL);
    }

    for (int j(0); j < 2; ++j)
    {
        m_pDisplay[j] = (BYTE*)DisplayDMAMem[j]->GetUserPointer();
        m_DisplayDMAMem[j] = DisplayDMAMem[j];
        m_pVBILines[j] = (BYTE*)VBIDMAMem[j]->GetUserPointer();
        m_VBIDMAMem[j] = VBIDMAMem[j];
    }

    // Set up 2 sets of pointers to the start of odd and even lines
    for (int k(0); k < 2; k++)
    {
        m_OddFields[k].pData = m_pDisplay[k] + 2048;
        m_OddFields[k].Flags = PICTURE_INTERLACED_ODD;
        m_OddFields[k].IsFirstInSeries = FALSE;
        m_EvenFields[k].pData = m_pDisplay[k];
        m_EvenFields[k].Flags = PICTURE_INTERLACED_EVEN;
        m_EvenFields[k].IsFirstInSeries = FALSE;
    }

    ReadFromIni();

    SetupCard();

    InitializeUI();
}


CSAA7134Source::~CSAA7134Source()
{

    CleanupUI();
    KillTimer(hWnd, TIMER_MSP);

    // SAA7134 reserves input -1 as the clean up indicator
    m_pSAA7134Card->SetVideoSource(-1);
    delete m_pSAA7134Card;
}

void CSAA7134Source::SetSourceAsCurrent()
{
//LOG(1, "CSAA7134Source::SetSourceAsCurrent m_VideoSource %d", m_VideoSource->GetValue());
//LOG(1, "CSAA7134Source::SetSourceAsCurrent m_VideoFormat %d", m_VideoFormat->GetValue());
    // need to call up to parent to run register settings functions
    CSource::SetSourceAsCurrent();

    // tell the rest of DScaler what the setup is
    // A side effect of the Raise events is to tell the settings master
    // what our current setup is
    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, -1, m_VideoSource->GetValue());
    EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, -1, m_VideoFormat->GetValue());
    EventCollector->RaiseEvent(this, EVENT_AUDIOINPUT_CHANGE, -1, m_AudioSource->GetValue());

    // reset the tuner
    // this will kick of the change channel event
    // which must happen after the video input event
    if(IsInTunerMode())
    {
        Channel_Reset();
    }

    // make sure the defaults are correct
    // but don't change the values
    ChangeDefaultsForSetup(SETUP_CHANGE_ANY, TRUE);

    SettingsMaster->LoadSettings();

    Reset();
}

void CSAA7134Source::CreateSettings(LPCSTR IniSection)
{
    // JA 13/Jan/2003
    // Added new groups
    // Functionality to store settings by Audio Input has not been used
    // as I don't think it would work as expected so the audio settings can only be
    // set by channel
    // Also there is no format stored by input

    CSettingGroup* pVideoFormatGroup = GetSettingsGroup("SAA7134 - Video Format", SETTING_BY_INPUT, TRUE);
    CSettingGroup* pVideoGroup = GetSettingsGroup("SAA7134 - Video Basic", SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, TRUE);
    CSettingGroup* pGainControlGroup = GetSettingsGroup("SAA7134 - Video Gain Control", SETTING_BY_INPUT, TRUE);
    CSettingGroup* pVideoMiscGroup = GetSettingsGroup("SAA7134 - Video Miscellaneous", SETTING_BY_INPUT | SETTING_BY_CHANNEL, TRUE);

    CSettingGroup* pAudioSourceGroup  = GetSettingsGroup("SAA7134 - Audio Source", SETTING_BY_INPUT, TRUE);
    CSettingGroup* pAudioStandardGroup = GetSettingsGroup("SAA7134 - Audio Standard", SETTING_BY_FORMAT | SETTING_BY_INPUT | SETTING_BY_CHANNEL, TRUE);
    CSettingGroup* pAudioCustomGroup = GetSettingsGroup("SAA7134 - Audio Standard Custom", SETTING_BY_CHANNEL, TRUE);
    CSettingGroup* pAudioGroup = GetSettingsGroup("SAA7134 - Audio Basic", SETTING_BY_AUDIOINPUT, TRUE);
    CSettingGroup* pAudioMiscGroup = GetSettingsGroup("SAA7134 - Audio Miscellaneous", SETTING_BY_AUDIOINPUT, TRUE);

    m_Brightness = new CBrightnessSetting(this, "Brightness", SAA7134_DEFAULT_BRIGHTNESS, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", SAA7134_DEFAULT_CONTRAST, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Contrast);

    m_Saturation = new CSaturationSetting(this, "Saturation", SAA7134_DEFAULT_SATURATION, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Saturation);

    m_Hue = new CHueSetting(this, "Hue", SAA7134_DEFAULT_HUE, 0, 255, IniSection, pVideoGroup);
    m_Settings.push_back(m_Hue);

    m_TopOverscan = new CTopOverscanSetting(this, "Overscan at Top", SAA7134_DEFAULT_NTSC_OVERSCAN, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_TopOverscan);

    m_PixelWidth = new CPixelWidthSetting(this, "Sharpness", 720, 120, DSCALER_MAX_WIDTH, IniSection, pVideoGroup);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth", pVideoGroup);
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 6, IniSection);
    m_Settings.push_back(m_VideoSource);

//    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection, pVideoFormatGroup);
    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection);
    m_Settings.push_back(m_VideoFormat);

    m_ReversePolarity = new CYesNoSetting("Reverse Polarity", FALSE, IniSection, "ReversePolarity");
    m_Settings.push_back(m_ReversePolarity);

    m_CardType = new CSliderSetting("Card Type", SAA7134CARDID_UNKNOWN, SAA7134CARDID_UNKNOWN, m_pSAA7134Card->GetMaxCards() - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_HPLLMode = new CHPLLModeSetting(this, "HPLL Locking Mode", HPLLMODE_FAST_TRACKING, HPLLMODE_TV, HPLLMODE_LASTONE - 1, IniSection, pVideoMiscGroup);
    m_Settings.push_back(m_HPLLMode);

    m_WhitePeak = new CWhitePeakSetting(this, "White Peak", FALSE, IniSection, pVideoMiscGroup);
    m_Settings.push_back(m_WhitePeak);

    m_ColorPeak = new CColorPeakSetting(this, "Color Peak", TRUE, IniSection, pVideoMiscGroup);
    m_Settings.push_back(m_ColorPeak);

    m_AdaptiveCombFilter = new CAdaptiveCombFilterSetting(this, "Adaptive Comb Filter", COMBFILTER_FULL, COMBFILTER_FULL, IniSection, m_CombFilterSzList, pVideoGroup);
    m_Settings.push_back(m_AdaptiveCombFilter);

    m_HDelay = new CHDelaySetting(this, "Horizontal Delay", 0, 0, 20, IniSection, pVideoMiscGroup);
    m_HDelay->SetStepValue(2);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay", 0, -60, 260, IniSection, pVideoMiscGroup);
    m_VDelay->SetStepValue(2);
    m_Settings.push_back(m_VDelay);

    m_AudioStandard = new CAudioStandardSetting(this, "Audio Standard", AUDIOSTANDARD_BG_DUAL_FM, AUDIOSTANDARD_BG_DUAL_FM, AUDIOSTANDARD_LASTONE-1, IniSection, pAudioStandardGroup);
    m_Settings.push_back(m_AudioStandard);

    m_AudioChannel = new CAudioChannelSetting(this, "Audio Channel", AUDIOCHANNEL_STEREO, AUDIOCHANNEL_MONO, AUDIOCHANNEL_LANGUAGE2, IniSection, pAudioGroup);
    m_Settings.push_back(m_AudioChannel);

    m_AudioSampleRate = new CAudioSampleRateSetting(this, "Audio Sample Rate", AUDIOSAMPLERATE_32000Hz, AUDIOSAMPLERATE_32000Hz, AUDIOSAMPLERATE_48000Hz, IniSection, pAudioGroup);
    m_Settings.push_back(m_AudioSampleRate);

    m_AutoStereoSelect = new CAutoStereoSelectSetting(this, "Auto Stereo Select", TRUE, IniSection, pAudioGroup);
    m_Settings.push_back(m_AutoStereoSelect);

    m_Volume = new CVolumeSetting(this, "Volume", 0, 0, 1000, IniSection, pAudioMiscGroup);
    m_Volume->SetStepValue(20);
    m_Settings.push_back(m_Volume);

    m_Bass = new CBassSetting(this, "Bass", 0, -96, 127, IniSection, pAudioMiscGroup);
    m_Settings.push_back(m_Bass);

    m_Treble = new CTrebleSetting(this, "Treble", 0, -96, 127, IniSection, pAudioMiscGroup);
    m_Settings.push_back(m_Treble);

    m_Balance = new CBalanceSetting(this, "Balance", 0, -127, 127, IniSection, pAudioMiscGroup);
    m_Settings.push_back(m_Balance);

    m_AudioSource = new CAudioSourceSetting(this, "Audio Source", AUDIOINPUTSOURCE_LINE1, AUDIOINPUTSOURCE_DAC, AUDIOINPUTSOURCE_LINE2, IniSection, pAudioSourceGroup);
    m_Settings.push_back(m_AudioSource);

    m_CustomAudioStandard = new CCustomAudioStandardSetting(this, "Use Custom Audio Standard", FALSE, IniSection, pAudioCustomGroup);
    m_Settings.push_back(m_CustomAudioStandard);

    m_AudioMajorCarrier = new CAudioMajorCarrierSetting(this, "Audio Major Carrier", AUDIO_CARRIER_5_5, 0, AUDIO_CARRIER_10_7, IniSection, pAudioCustomGroup);
    m_Settings.push_back(m_AudioMajorCarrier);

    m_AudioMinorCarrier = new CAudioMinorCarrierSetting(this, "Audio Minor Carrier", AUDIO_CARRIER_5_5, 0, AUDIO_CARRIER_10_7, IniSection, pAudioCustomGroup);
    m_Settings.push_back(m_AudioMinorCarrier);

    m_AudioMajorCarrierMode = new CAudioMajorCarrierModeSetting(this, "Audio Major Carrier Mode", AUDIOCHANNELMODE_FM, AUDIOCHANNELMODE_NONE, AUDIOCHANNELMODE_EIAJ, IniSection, pAudioCustomGroup);
    m_Settings.push_back(m_AudioMajorCarrierMode);

    m_AudioMinorCarrierMode = new CAudioMinorCarrierModeSetting(this, "Audio Minor Carrier Mode", AUDIOCHANNELMODE_FM, AUDIOCHANNELMODE_NONE, AUDIOCHANNELMODE_EIAJ, IniSection, pAudioCustomGroup);
    m_Settings.push_back(m_AudioMinorCarrierMode);

    m_AudioCh1FMDeemph = new CAudioCh1FMDeemphSetting(this, "Audio Channel 1 FM De-emphasis", AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_ADAPTIVE, IniSection, pAudioCustomGroup);
    m_Settings.push_back(m_AudioCh1FMDeemph);

    m_AudioCh2FMDeemph = new CAudioCh2FMDeemphSetting(this, "Audio Channel 2 FM De-emphasis", AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_ADAPTIVE, IniSection, pAudioCustomGroup);
    m_Settings.push_back(m_AudioCh2FMDeemph);

    // HELPTEXT: Automatic Volume Leveling control to avoid
    // digital clipping at analog audio output
    m_AutomaticVolumeLevel = new CAutomaticVolumeLevelSetting(this, "Automatic Volume Leveling", AUTOMATICVOLUME_MEDIUMDECAY, AUTOMATICVOLUME_LONGDECAY, IniSection, m_AutomaticVolumeSzList);
    m_Settings.push_back(m_AutomaticVolumeLevel);

    // HELPTEXT: Lower means better VBI reception/decoding but
    // how far it can be lowered depends on individual cards.
    m_VBIUpscaleDivisor = new CVBIUpscaleDivisorSetting(this, "VBI Upscale Divisor", 0x200, 0x186, 0x200, IniSection);
    m_Settings.push_back(m_VBIUpscaleDivisor);

    m_VBIDebugOverlay = new CYesNoSetting("VBI Debug Overlay", FALSE, IniSection, "VBIDebugOverlay");
    m_Settings.push_back(m_VBIDebugOverlay);

    m_AutomaticGainControl = new CAutomaticGainControlSetting(this, "Automatic Gain Control", TRUE, IniSection, pGainControlGroup);
    m_Settings.push_back(m_AutomaticGainControl);

    m_GainControlLevel = new CGainControlLevelSetting(this, "Gain Control Level", 0x0100, 0x0000, 0x01FF, IniSection, pGainControlGroup);
    m_Settings.push_back(m_GainControlLevel);

    m_VideoMirror = new CVideoMirrorSetting(this, "Mirroring", FALSE, IniSection);
    m_Settings.push_back(m_VideoMirror);

    m_AudioLine1Voltage = new CAudioLine1VoltageSetting(this, "Audio Line 1 Input Signal", AUDIOLINEVOLTAGE_2VRMS, AUDIOLINEVOLTAGE_2VRMS, IniSection, m_LineVoltageSzList);
    m_Settings.push_back(m_AudioLine1Voltage);

    m_AudioLine2Voltage = new CAudioLine2VoltageSetting(this, "Audio Line 2 Input Signal", AUDIOLINEVOLTAGE_2VRMS, AUDIOLINEVOLTAGE_2VRMS, IniSection, m_LineVoltageSzList);
    m_Settings.push_back(m_AudioLine2Voltage);

    m_BottomOverscan = new CBottomOverscanSetting(this, "Overscan at Bottom", SAA7134_DEFAULT_NTSC_OVERSCAN, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_BottomOverscan);

    m_LeftOverscan = new CLeftOverscanSetting(this, "Overscan at Left", SAA7134_DEFAULT_NTSC_OVERSCAN, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_LeftOverscan);

    m_RightOverscan = new CRightOverscanSetting(this, "Overscan at Right", SAA7134_DEFAULT_NTSC_OVERSCAN, 0, 150, IniSection, pVideoGroup);
    m_Settings.push_back(m_RightOverscan);

#ifdef _DEBUG    
    if (SAA7134_SETTING_LASTONE != m_Settings.size())
    {
        LOGD("Number of settings in SAA7134 source is not equal to the number of settings in DS_Control.h");
        LOGD("DS_Control.h and SAA7134Source.cpp are probably not in sync with eachother.");
    }
#endif

}

void CSAA7134Source::OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
}


void CSAA7134Source::HandleTimerMessages(int TimerId)
{
    UpdateAudioStatus();
}


void CSAA7134Source::Reset()
{
//LOG(1, "CSAA7134Source::Reset 1 m_VideoSource %d m_VideoFormat %d", m_VideoSource->GetValue(), m_VideoFormat->GetValue());
    m_pSAA7134Card->ResetHardware();

    SetupDMAMemory();

    SetupVideoSource();

    m_pSAA7134Card->SetVideoMirror(m_VideoMirror->GetValue());
    m_pSAA7134Card->SetVBIGeometry(m_VBIUpscaleDivisor->GetValue());
    m_pSAA7134Card->SetAutomaticVolume((eAutomaticVolume)m_AutomaticVolumeLevel->GetValue());
    m_pSAA7134Card->SetAudioLine1Voltage((eAudioLineVoltage)m_AudioLine1Voltage->GetValue());
    m_pSAA7134Card->SetAudioLine2Voltage((eAudioLineVoltage)m_AudioLine2Voltage->GetValue());
//LOG(1, "CSAA7134Source::Reset 2 m_VideoSource %d m_VideoFormat %d", m_VideoSource->GetValue(), m_VideoFormat->GetValue());
}


void CSAA7134Source::SetupVideoSource()
{
    m_pSAA7134Card->SetVideoSource(m_VideoSource->GetValue());

    m_pSAA7134Card->SetAutomaticGainControl(m_AutomaticGainControl->GetValue());
    m_pSAA7134Card->SetGainControl(m_GainControlLevel->GetValue());

    m_pSAA7134Card->SetHPLLMode((eHPLLMode)m_HPLLMode->GetValue());

    m_pSAA7134Card->SetWhitePeak(m_WhitePeak->GetValue());
    m_pSAA7134Card->SetColorPeak(m_ColorPeak->GetValue());

    SetupAudioSource();
    SetupVideoStandard();
}


void CSAA7134Source::SetupVideoStandard()
{
    eVideoFormat VideoFormat = (eVideoFormat) m_VideoFormat->GetValue();
    eVideoStandard VideoStandard = TVFormat2VideoStandard(VideoFormat);

    m_CurrentX = m_PixelWidth->GetValue();

    m_CurrentVBILines = kMAX_VBILINES;
    m_pSAA7134Card->SetVideoStandard(VideoStandard,
                                    m_CurrentVBILines,
                                    m_CurrentX,
                                    m_CurrentY,
                                    m_HDelay->GetValue(),
                                    m_VDelay->GetValue(),
                                    m_VBIUpscaleDivisor->GetValue());

    m_pSAA7134Card->SetBrightness(m_Brightness->GetValue());
    m_pSAA7134Card->SetContrast(m_Contrast->GetValue());
    m_pSAA7134Card->SetSaturation(m_Saturation->GetValue());
    m_pSAA7134Card->SetHue(m_Hue->GetValue());
    m_pSAA7134Card->SetCombFilter((eCombFilter)m_AdaptiveCombFilter->GetValue());

    NotifySizeChange();

    SetupAudioStandard();
}


void CSAA7134Source::Start()
{
    m_pSAA7134Card->StartCapture(bCaptureVBI);
    Timing_Reset();

    // This timer is used to update STATUS_AUDIO
    SetTimer(hWnd, TIMER_MSP, TIMER_MSP_MS, NULL);

    NotifySquarePixelsCheck();
    m_ProcessingFieldID = -1;

    VBI_Init_data(27.0 * 0x200 / m_VBIUpscaleDivisor->GetValue());

    // Just before we start allow change messages again
    EnableOnChange();
}


void CSAA7134Source::Stop()
{
    // disable OnChange messages while video is stopped
    DisableOnChange();
    // stop capture
    m_pSAA7134Card->StopCapture();    
    KillTimer(hWnd, TIMER_MSP);
}


void CSAA7134Source::SetupDMAMemory()
{
    WORD nFrame;
    DWORD nPages;
    eRegionID RegionID;

    // Turn off all DMA
    m_pSAA7134Card->SetDMA(REGIONID_VIDEO_A, FALSE);
    m_pSAA7134Card->SetDMA(REGIONID_VIDEO_B, FALSE);
    m_pSAA7134Card->SetDMA(REGIONID_VBI_A, FALSE);
    m_pSAA7134Card->SetDMA(REGIONID_VBI_B, FALSE);

    for (nFrame = 0; nFrame < 2; nFrame++)
    {
        nPages = CreatePageTable(m_DisplayDMAMem[nFrame],
                                    kMAX_VIDLINES,
                                    m_DisplayPageTableLinear[nFrame]
                                );

        RegionID = (nFrame == 0) ? REGIONID_VIDEO_A : REGIONID_VIDEO_B;

        m_pSAA7134Card->SetPageTable(RegionID, m_DisplayPageTablePhysical[nFrame], nPages);
        m_pSAA7134Card->SetBaseOffsets(RegionID, 0, 2048, 4096);
        m_pSAA7134Card->SetBSwapAndWSwap(RegionID, TRUE, FALSE);

        // It should now be safe to enable the channel
        m_pSAA7134Card->SetDMA(RegionID, TRUE);

        nPages = CreatePageTable(m_VBIDMAMem[nFrame],
                                    kMAX_VBILINES,
                                    m_VBIPageTableLinear[nFrame]
                                );

        RegionID = (nFrame == 0) ? REGIONID_VBI_A : REGIONID_VBI_B;

        // Offset the base offset by 40 because VBI_DecodeLine wants
        // data to start 40 bytes into the buffer.
        m_pSAA7134Card->SetPageTable(RegionID, m_VBIPageTablePhysical[nFrame], nPages);
        m_pSAA7134Card->SetBaseOffsets(RegionID, 40, 40 + kMAX_VBILINES * 2048, 2048);
        m_pSAA7134Card->SetBSwapAndWSwap(RegionID, FALSE, FALSE);

        // It should now be safe to enable the channel
        m_pSAA7134Card->SetDMA(RegionID, TRUE);
    }
}


DWORD CSAA7134Source::CreatePageTable(CUserMemory* pDMAMemory, DWORD nPagesWanted, LPDWORD pPageTable)
{
    const WORD PAGE_SIZE = 4096;
    const WORD PAGE_MASK = (~(PAGE_SIZE-1));
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytes;
    DWORD nPages;

    if (pDMAMemory == NULL || nPagesWanted == 0 || pPageTable == NULL)
    {
        return 0;
    }

    pUser = (LPBYTE) pDMAMemory->GetUserPointer();

    // This routine should get nPagesWanted pages in their page boundries
    for (nPages = 0; nPages < nPagesWanted; nPages++)
    {
        pPhysical = pDMAMemory->TranslateToPhysical(pUser, PAGE_SIZE, &GotBytes);
        if (pPhysical == 0 || pPhysical & ~PAGE_MASK > 0 || GotBytes < PAGE_SIZE)
        {
            break;
        }

        *(pPageTable++) = pPhysical;
        pUser += PAGE_SIZE;
    }

    return nPages;
}


CSAA7134Card* CSAA7134Source::GetCard()
{
    return m_pSAA7134Card;
}


void CSAA7134Source::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    TPicture* FieldBuffer;

    if (AccurateTiming)
    {
        GetNextFieldAccurate(pInfo);
    }
    else
    {
        GetNextFieldNormal(pInfo);
    }

    FieldBuffer = GetFieldBuffer(m_CurrentFieldID);

    if (m_VBIDebugOverlay->GetValue())
    {
        BYTE nFrameIndex;
        BOOL bIsFieldOdd;

        GetFrameIndex(m_CurrentFieldID, &nFrameIndex, &bIsFieldOdd);

        BYTE* pVBI = m_pVBILines[nFrameIndex];
        pVBI += bIsFieldOdd ? (m_CurrentVBILines * 2048) : 0;

        for (int nLine(0); nLine < m_CurrentVBILines; nLine++)
        {
            for (int i(0); i < 2048; i++)
            {
                FieldBuffer->pData[nLine * 4096 + i] = pVBI[nLine * 2048 + i];
            }
        }
    }

    GiveNextField(pInfo, FieldBuffer);

    pInfo->LineLength = m_CurrentX * 2;
    pInfo->FrameWidth = m_CurrentX;
    pInfo->FrameHeight = m_CurrentY;
    pInfo->FieldHeight = m_CurrentY / 2;
    pInfo->InputPitch = 4096;

    Timing_IncrementUsedFields();

    // auto input detect
    Timimg_AutoFormatDetect(pInfo, 10);
}


TPicture* CSAA7134Source::GetFieldBuffer(TFieldID FieldID)
{
    BYTE nFrameIndex;
    BOOL bIsFieldOdd;

    GetFrameIndex(FieldID, &nFrameIndex, &bIsFieldOdd);

    if (bIsFieldOdd)
    {
        if (m_ReversePolarity->GetValue() == FALSE)
        {
            return &m_OddFields[nFrameIndex];
        }
        else
        {
            nFrameIndex = (kMAX_FRAMEBUFFERS + nFrameIndex - 1) % kMAX_FRAMEBUFFERS;
            return &m_EvenFields[nFrameIndex];
        }
    }
    else
    {
        if (m_ReversePolarity->GetValue() == FALSE)
        {
            return &m_EvenFields[nFrameIndex];
        }
        else
        {
            return &m_OddFields[nFrameIndex];
        }
    }
    return NULL;
}


void CSAA7134Source::GiveNextField(TDeinterlaceInfo* pInfo, TPicture* pPicture)
{
    if (pInfo->bMissedFrame)
    {
        ClearPictureHistory(pInfo);
    }

    // Re-enumerate our 4 field cycle to a 10 field cycle
    if ((pPicture->Flags & PICTURE_INTERLACED_EVEN) > 0 ||
        pInfo->bMissedFrame)
    {
        pInfo->CurrentFrame = (pInfo->CurrentFrame + 1) % 5;
    }

    // we only have 4 unique fields
    ShiftPictureHistory(pInfo, 4);
    pInfo->PictureHistory[0] = pPicture;
}


void CSAA7134Source::GetFrameIndex(TFieldID FieldID, BYTE* pFrameIndex, BOOL* pIsFieldOdd)
{
    *pFrameIndex = (FieldID >> FIELDID_FRAMESHIFT) % kMAX_FRAMEBUFFERS;
    // The first field is the lower field (odd)
    *pIsFieldOdd = ((FieldID & FIELDID_SECONDFIELD) == 0);
}


void CSAA7134Source::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    TFieldID    NextFieldID;
    int         FieldDistance;
    BOOL        bTryToCatchUp = TRUE;
    BOOL        bSlept = FALSE;
	BOOL        bWaited;

    // This function waits for the next field
	bWaited = PollForNextField(&NextFieldID, &FieldDistance, TRUE);
    if (bWaited)
    {
        // if we waited then we are not late
        pInfo->bRunningLate = FALSE;
    }
	else
	{
		if (bAlwaysSleep)
		{
			// A sleep must be done
		}
		Timing_IncrementNotWaitedFields();
	}

    // The distance from the new field the field card
    // is currently working on
    if (FieldDistance == 1)
    {
        // No skipped fields
        pInfo->bMissedFrame = FALSE;

		if (!bWaited)
		{
            LOG(2, " Running late but right field");
			if (pInfo->bRunningLate)
			{
				// Not sure why we need to do this
				Timing_AddDroppedFields(1);
			}
		}
    }
    else if (bTryToCatchUp && FieldDistance <= (MaxFieldShift+1))
    {
        // Try to catch up
        pInfo->bMissedFrame = FALSE;
        LOG(2, " Running late by %d fields", FieldDistance - 1);
        Timing_AddLateFields(FieldDistance - 1);
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Field(s)", FieldDistance - 1);

        // Use the most recent field
        NextFieldID = GetPrevFieldID(m_ProcessingFieldID);
    }

    // Update the current field
    m_CurrentFieldID = NextFieldID;
}


void CSAA7134Source::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    TFieldID    NextFieldID;
    int         FieldDistance;
    BOOL        bSlept = FALSE;
	BOOL        bWaited;

    // This function waits for the next field
	bWaited = PollForNextField(&NextFieldID, &FieldDistance, FALSE);
    if (bWaited)
    {
        // if we waited then we are not late
        pInfo->bRunningLate = FALSE;
    }
	else
	{
		Timing_IncrementNotWaitedFields();
	}

    // The distance from the new field the field card
    // is currently working on
    if (FieldDistance == 1)
    {
        // No skipped fields, do nothing
		if (!bWaited)
		{
            LOG(2, " Running late but right field");
		}
    }
    else if (FieldDistance <= (MaxFieldShift+1))
    {
        // Slightly late but try to recover
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Running late by %d fields", FieldDistance - 1);
        Timing_AddLateFields(FieldDistance - 1);
    }
    else
    {
        // There is too much delay to recover, throw
        // out the extra fields.
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(FieldDistance - 1);
        LOG(2, " Dropped %d Fields", FieldDistance - 1);
        Timing_Reset();

        // Use the most recent field
        NextFieldID = GetPrevFieldID(m_ProcessingFieldID);
    }

    // Update the current field
    m_CurrentFieldID = NextFieldID;

    // we've just got a new field
    // we are going to time the odd to odd
    // input frequency
    if (m_CurrentFieldID & FIELDID_SECONDFIELD)
    {
        Timing_UpdateRunningAverage(pInfo, 2);
    }

	if (bAlwaysSleep || bWaited)
	{
	    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
	}
}


BOOL CSAA7134Source::PollForNextField(TFieldID* pNextFieldID, int* pFieldDistance, BOOL bSmartSleep)
{
    TFieldID    NextFieldID;
    TFieldID    ProcessingFieldID;
    int         FieldDistance;
    BOOL        bWaited = FALSE;

    // Initialize if we need to
    if (m_ProcessingFieldID == -1)
    {
        while (!m_pSAA7134Card->GetProcessingFieldID(&m_ProcessingFieldID))
        {
            // do nothing
        }
        InitializeSmartSleep();
        NextFieldID = m_ProcessingFieldID;
    }
    else
    {
        NextFieldID = GetNextFieldID(m_CurrentFieldID);
    }

    if (bSmartSleep)
    {
        ULONGLONG   PerformanceTick;
        ULONGLONG   TimePassed;

        while (TRUE)
        {
            if (m_pSAA7134Card->GetProcessingFieldID(&ProcessingFieldID))
            {
                // Check if the field is finished
                if (ProcessingFieldID != NextFieldID ||
                    ProcessingFieldID != m_ProcessingFieldID)
                {
                    break;
                }
                bWaited = TRUE;
            }

            PerformSmartSleep(&PerformanceTick, &TimePassed);
        }

        // We can't recalculate unless we waited
        UpdateSmartSleep(bWaited, PerformanceTick, TimePassed);
    }
    else
    {
        while (TRUE)
        {
            if (m_pSAA7134Card->GetProcessingFieldID(&ProcessingFieldID))
            {
                // Check if the field is finished
                if (ProcessingFieldID != NextFieldID ||
                    ProcessingFieldID != m_ProcessingFieldID)
                {
                    break;
                }
                bWaited = TRUE;
            }
        }
    }

    // Get the distance from the new field to the processing
    // field (used to calculate the number of dropped fields)
    FieldDistance = GetFieldDistance(NextFieldID, ProcessingFieldID);

    // Determine if the card has done a complete circuit
    if (GetFieldDistance(NextFieldID, m_ProcessingFieldID) > FieldDistance)
    {
        FieldDistance += kMAX_FIELDBUFFERS;
    }
    
    // Updated the processing field ID
    m_ProcessingFieldID = ProcessingFieldID;

    *pNextFieldID = NextFieldID;
    *pFieldDistance = FieldDistance;

    return bWaited;
}


void CSAA7134Source::InitializeSmartSleep()
{
    if (QueryPerformanceFrequency((PLARGE_INTEGER)&m_PerformanceFrequency))
    {
        m_LastFieldPerformanceCount = 0;
        m_MinimumFieldDelay = 0;
    }
    else
    {
        // there is no high-resolution counter
        m_PerformanceFrequency = 0;
    }
}


void CSAA7134Source::PerformSmartSleep(ULONGLONG* pPerformanceTick, ULONGLONG* pTimePassed)
{
    ULONGLONG       PerformanceCount;
    ULONGLONG       WaitTimeSpent;
    ULONG           SleepTime;

    if (m_PerformanceFrequency)
    {
        // Get the current time
        QueryPerformanceCounter((PLARGE_INTEGER)&PerformanceCount);

        // Calculate how much time has passed since the last field
        WaitTimeSpent = PerformanceCount - m_LastFieldPerformanceCount;

        // If we've gone past the field delay time
        if (WaitTimeSpent > m_MinimumFieldDelay)
        {
            // Wait blindly with 1ms delays
            SleepTime = (m_PerformanceFrequency * 0.001);
        }
        else
        {
            // Sleep for three quarters of the remaining time
            SleepTime = (m_MinimumFieldDelay - WaitTimeSpent) * 3 / 4;
        }

        // NOTE: Sleep is not very accurate with only 10ms accuracy
        Sleep(SleepTime * 1000 / m_PerformanceFrequency);

        *pPerformanceTick = PerformanceCount;
        *pTimePassed = WaitTimeSpent;
    }
    else
    {
        // There is no high-resolution counter,
        // just sleep for one millisecond instead
        Sleep(1);
    }
}


void CSAA7134Source::UpdateSmartSleep(BOOL bRecalculate, ULONGLONG LastTick, ULONGLONG TimePassed)
{
    if (m_PerformanceFrequency)
    {
        // Update the field time with the current time
        QueryPerformanceCounter((PLARGE_INTEGER)&m_LastFieldPerformanceCount);

        // Recalculate the field to field delay
        if (bRecalculate)
        {
            // If we waited longer than 30ms, something probably went wrong
            if (TimePassed * 1000 / m_PerformanceFrequency > 30)
            {
                // reset to 10ms
                m_MinimumFieldDelay = (m_PerformanceFrequency * 0.010);
            }
            else
            {
                ULONGLONG LastSleptTime;

                LastSleptTime = m_LastFieldPerformanceCount - LastTick;

                // Add the time passed while waiting for the field
                // to half the duration of our last sleep
                m_MinimumFieldDelay = TimePassed + LastSleptTime / 2;
            }
        }
    }
}


void CSAA7134Source::DecodeVBI(TDeinterlaceInfo* pInfo)
{
    BYTE nFrameIndex;
    BOOL bIsFieldOdd;
    int nLineTarget;
    BYTE* pVBI;

    // Wait a few ticks after changing the channel
    if (m_ChannelChangeTick)
    {
        DWORD CurrentTick = GetTickCount();

        if (CurrentTick > m_ChannelChangeTick &&
            CurrentTick - m_ChannelChangeTick < 500)
        {
            return;
        }
        m_ChannelChangeTick = 0;
    }

    GetFrameIndex(m_CurrentFieldID, &nFrameIndex, &bIsFieldOdd);

    // VBI should have been DMA'd before the video
    pVBI = (LPBYTE) m_pVBILines[nFrameIndex];

    if (bIsFieldOdd)
    {
        pVBI += m_CurrentVBILines * 2048;
    }

    for (nLineTarget = 0; nLineTarget < m_CurrentVBILines; nLineTarget++)
    {
        VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget - 2, bIsFieldOdd);
    }
}


void CSAA7134Source::SetupCard()
{
    long OrigTuner = m_TunerType->GetValue();

    if (m_CardType->GetValue() == SAA7134CARDID_UNKNOWN)
    {
        // try to detect the card
        m_CardType->SetValue(m_pSAA7134Card->AutoDetectCardType());
        m_TunerType->SetValue(m_pSAA7134Card->AutoDetectTuner((eSAA7134CardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        m_bSelectCardCancelButton = FALSE;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
        m_bSelectCardCancelButton = TRUE;
    }

    m_pSAA7134Card->SetCardType(m_CardType->GetValue());
    m_pSAA7134Card->InitTuner((eTunerId)m_TunerType->GetValue());

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
}


BOOL CSAA7134Source::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    if (VideoFormat == VIDEOFORMAT_LASTONE)
    {
        VideoFormat = m_pSAA7134Card->GetTuner()->GetDefaultVideoFormat();
    }
    if (VideoFormat != m_VideoFormat->GetValue())
    {
        m_VideoFormat->SetValue(VideoFormat);
    }

    // switching to fast tracking speeds up channel changing a bit
    m_pSAA7134Card->SetVSyncRecovery(VSYNCRECOVERY_FAST_TRACKING);
    BOOL Success = m_pSAA7134Card->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
    m_pSAA7134Card->SetVSyncRecovery(VSYNCRECOVERY_NORMAL);

    if (Success)
    {
        StatusBar_ShowText(STATUS_AUDIO, "");
        m_DetectedAudioChannel = (eAudioChannel)-1;

        // This is used in DecodeVBI() so old VBI isn't
        // used for the new channel
        m_ChannelChangeTick = GetTickCount();
        return TRUE;
    }
    return FALSE;
}


BOOL CSAA7134Source::IsInTunerMode()
{
    return m_pSAA7134Card->IsInputATuner(m_VideoSource->GetValue());
}


BOOL CSAA7134Source::IsVideoPresent()
{
    return m_pSAA7134Card->IsVideoPresent();
}


LPCSTR CSAA7134Source::GetStatus()
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
        pRetVal = m_pSAA7134Card->GetInputName(m_VideoSource->GetValue());
    }

    return pRetVal;
}


void CSAA7134Source::SetFormat(eVideoFormat NewFormat)
{
    PostMessage(hWnd, WM_SAA7134_SETVALUE, SAA7134TVFORMAT, NewFormat);
}


eVideoFormat CSAA7134Source::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}


int CSAA7134Source::GetWidth()
{
    return m_CurrentX;
}


int CSAA7134Source::GetHeight()
{
    return m_CurrentY;
}


void CSAA7134Source::SetAspectRatioData()
{
    AspectSettings.InitialTopOverscan = m_TopOverscan->GetValue();
    AspectSettings.InitialBottomOverscan = m_BottomOverscan->GetValue();
    AspectSettings.InitialLeftOverscan = m_LeftOverscan->GetValue();
    AspectSettings.InitialRightOverscan = m_RightOverscan->GetValue();
    AspectSettings.bAnalogueBlanking = FALSE;
}


const char* CSAA7134Source::GetChipName()
{
    return m_ChipName.c_str();
}


int CSAA7134Source::GetDeviceIndex()
{
    return m_DeviceIndex;
}


int  CSAA7134Source::NumInputs(eSourceInputType InputType)
{
    if (InputType == VIDEOINPUT)
    {
        return m_pSAA7134Card->GetNumInputs();
    }
    /*  else if (InputType == AUDIOINPUT)
    {
        return m_pSAA7134Card->GetNumAudioInputs();
    }*/
    return 0;
}


BOOL CSAA7134Source::SetInput(eSourceInputType InputType, int Nr)
{
    if (InputType == VIDEOINPUT)
    {
        m_VideoSource->SetValue(Nr);
        return TRUE;
    }
    /* else if (InputType == AUDIOINPUT)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInput)Nr);
        return TRUE;
    }*/
    return FALSE;
}


int CSAA7134Source::GetInput(eSourceInputType InputType)
{
    if (InputType == VIDEOINPUT)
    {
        return m_VideoSource->GetValue();
    }
    /*  else if (InputType == AUDIOINPUT)
    {
        return m_pSAA7134Card->GetAudioInput();
    }*/
    return -1;
}


const char* CSAA7134Source::GetInputName(eSourceInputType InputType, int Nr)
{
    if (InputType == VIDEOINPUT)
    {
        if ((Nr>=0) && (Nr < m_pSAA7134Card->GetNumInputs()) )
        {
            return m_pSAA7134Card->GetInputName(Nr);
        }
    }
    /*  else if (InputType == AUDIOINPUT)
    {
        return m_pSAA7134Card->GetAudioInputName((eAudioInput)Nr);
    }*/
    return NULL;
}


BOOL CSAA7134Source::InputHasTuner(eSourceInputType InputType, int Nr)
{
    if (InputType == VIDEOINPUT)
    {
        if (m_TunerType->GetValue() != TUNER_ABSENT)
        {
            return m_pSAA7134Card->IsInputATuner(Nr);
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}


ITuner* CSAA7134Source::GetTuner()
{
    return m_pSAA7134Card->GetTuner();
}


////////////////////////////////////////////////////////////////////////

ISetting* CSAA7134Source::GetBrightness()
{
    return m_Brightness;
}

ISetting* CSAA7134Source::GetContrast()
{
    return m_Contrast;
}

ISetting* CSAA7134Source::GetHue()
{
    return m_Hue;
}

ISetting* CSAA7134Source::GetSaturation()
{
    return m_Saturation;
}

ISetting* CSAA7134Source::GetSaturationU()
{
    return NULL;
}

ISetting* CSAA7134Source::GetSaturationV()
{
    return NULL;
}

ISetting* CSAA7134Source::GetTopOverscan()
{
    return m_TopOverscan;
}

ISetting* CSAA7134Source::GetBottomOverscan()
{
    return m_BottomOverscan;
}

ISetting* CSAA7134Source::GetLeftOverscan()
{
    return m_LeftOverscan;
}

ISetting* CSAA7134Source::GetRightOverscan()
{
    return m_RightOverscan;
}

ISetting* CSAA7134Source::GetHDelay()
{
    return m_HDelay;
}

ISetting* CSAA7134Source::GetVDelay()
{
    return m_VDelay;
}


////////////////////////////////////////////////////////////////////////

void CSAA7134Source::VideoSourceOnChange(long NewValue, long OldValue)
{
//LOG(1, "CSAA7134Source::VideoSourceOnChange Old %d New %d", OldValue, NewValue);
    Audio_Mute(PreSwitchMuteDelay);

    Stop_Capture();

    SettingsMaster->SaveSettings();

    // OK Capture is stopped so other onchange messages are
    // disabled so if anything that happens in those needs to be triggered
    // we have to manage that ourselves

    // here we have to watch for a format switch


    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_PRECHANGE, OldValue, NewValue);
    EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, OldValue, NewValue);

    int OldFormat = m_VideoFormat->GetValue();
    
    // set up channel
    // this must happen after the VideoInput change is sent
    if(m_pSAA7134Card->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
    }

    // tell the world if the format has changed
    if(OldFormat != m_VideoFormat->GetValue())
    {
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_PRECHANGE, OldValue, m_VideoFormat->GetValue());
        EventCollector->RaiseEvent(this, EVENT_VIDEOFORMAT_CHANGE, OldValue, m_VideoFormat->GetValue());
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


void CSAA7134Source::VideoFormatOnChange(long NewValue, long OldValue)
{
//LOG(1, "CSAA7134Source::VideoFormatOnChange Old %d New %d", OldValue, NewValue);
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

    Reset();

    Start_Capture();
}


void CSAA7134Source::PixelWidthOnChange(long NewValue, long OldValue)
{
    if (NewValue != 768 &&
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

    m_pSAA7134Card->SetGeometry(m_CurrentX,
                                m_CurrentY,
                                m_HDelay->GetValue(),
                                m_VDelay->GetValue());
    NotifySizeChange();

    Start_Capture();
}


void CSAA7134Source::HDelayOnChange(long HDelay, long OldValue)
{
    Stop_Capture();
    m_pSAA7134Card->SetGeometry(m_CurrentX, m_CurrentY, HDelay, m_VDelay->GetValue());
    Start_Capture();
}


void CSAA7134Source::VDelayOnChange(long VDelay, long OldValue)
{
    long Minimum;

    // If VBI is enabled, video cannot overlap VBI
    if (bCaptureVBI)
    {
        Minimum = m_pSAA7134Card->GetMinimumVDelayWithVBI();
    }
    else
    {
        Minimum = m_pSAA7134Card->GetMinimumVDelay();
    }

    if (VDelay < Minimum)
    {
        m_VDelay->SetValue(Minimum, TRUE);

        if (Minimum == OldValue)
        {
            return;
        }
    }

    Stop_Capture();
    m_pSAA7134Card->SetGeometry(m_CurrentX, m_CurrentY, m_HDelay->GetValue(), VDelay);
    Start_Capture();
}


void CSAA7134Source::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pSAA7134Card->SetBrightness(Brightness);
}


void CSAA7134Source::HueOnChange(long Hue, long OldValue)
{
    m_pSAA7134Card->SetHue(Hue);
}


void CSAA7134Source::ContrastOnChange(long Contrast, long OldValue)
{
    m_pSAA7134Card->SetContrast(Contrast);
}


void CSAA7134Source::SaturationOnChange(long Sat, long OldValue)
{
    m_pSAA7134Card->SetSaturation(Sat);
}


void CSAA7134Source::TopOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialTopOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CSAA7134Source::BottomOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialBottomOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CSAA7134Source::LeftOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialLeftOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CSAA7134Source::RightOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialRightOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}


void CSAA7134Source::TunerTypeOnChange(long TunerId, long OldValue)
{
    m_pSAA7134Card->InitTuner((eTunerId)TunerId);
}


void CSAA7134Source::HPLLModeOnChange(long HPLLMode, long OldValue)
{
    m_pSAA7134Card->SetHPLLMode((eHPLLMode)HPLLMode);
}


void CSAA7134Source::WhitePeakOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetWhitePeak(NewValue);
}


void CSAA7134Source::ColorPeakOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetColorPeak(NewValue);
}


void CSAA7134Source::AdaptiveCombFilterOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetCombFilter((eCombFilter)NewValue);
}


void CSAA7134Source::VBIUpscaleDivisorOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pSAA7134Card->SetVBIGeometry(NewValue);
    Start_Capture();
}


void CSAA7134Source::AutomaticGainControlOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAutomaticGainControl(NewValue);
}


void CSAA7134Source::GainControlLevelOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetGainControl(NewValue);
}


void CSAA7134Source::VideoMirrorOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetVideoMirror(NewValue);
}


void CSAA7134Source::AudioLine1VoltageOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioLine1Voltage((eAudioLineVoltage)NewValue);
}


void CSAA7134Source::AudioLine2VoltageOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioLine2Voltage((eAudioLineVoltage)NewValue);
}


