/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source.cpp,v 1.38 2002-10-31 03:10:55 atnak Exp $
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
#include "..\help\helpids.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

void SAA7134_OnSetup(void *pThis, int Start)
{
   if (pThis != NULL)
   {
      ((CSAA7134Source*)pThis)->SavePerChannelSetup(Start);
   }
}

CSAA7134Source::CSAA7134Source(CSAA7134Card* pSAA7134Card, CContigMemory* PageTableDMAMem[4], CUserMemory* DisplayDMAMem[2], CUserMemory* VBIDMAMem[2], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex) :
    CSource(WM_SAA7134_GETVALUE, IDC_SAA7134),
    m_pSAA7134Card(pSAA7134Card),
    m_CurrentX(768),
    m_CurrentY(576),
    m_CurrentVBILines(19),
    m_Section(IniSection),
    m_IsFieldOdd(FALSE),
    m_InSaturationUpdate(FALSE),
    m_CurrentChannel(-1),
    m_SettingsByChannelStarted(FALSE),
    m_ChipName(ChipName),
    m_DeviceIndex(DeviceIndex),
    m_LastFieldIndex(0),
    m_hSAA7134ResourceInst(NULL),
    m_SettingsSetup(NULL)
{
    m_IDString = IniSection;
    CreateSettings(IniSection);

    m_InitialACPIStatus = m_pSAA7134Card->GetACPIStatus();

    // if the card is powered down we need to power it up
    if (m_InitialACPIStatus != 0)
    {
        m_pSAA7134Card->SetACPIStatus(0);
    }

    // Take over the card (not literally)
    m_pSAA7134Card->PrepareCard();

    SettingsPerChannel_RegisterOnSetup(this, SAA7134_OnSetup);

    ReadFromIni();
    LoadSettings(SETUP_CHANGE_ANY);

    InitializeUI();

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

    SetupCard();
    Reset();

    NotifyInputChange(0, VIDEOINPUT, -1, m_VideoSource->GetValue());
    NotifyInputChange(0, VIDEOFORMAT, -1, m_VideoFormat->GetValue());
}

CSAA7134Source::~CSAA7134Source()
{
    SAA7134_OnSetup(this, 0);
    // if the card was not in D0 state we restore the original ACPI power state
    if (m_InitialACPIStatus != 0)
    {
        m_pSAA7134Card->SetACPIStatus(m_InitialACPIStatus);
    }

    if (m_SettingsSetup != NULL)
    {
        delete [] m_SettingsSetup;
    }

    CleanupUI();

    KillTimer(hWnd, TIMER_MSP);
    delete m_pSAA7134Card;
}


void CSAA7134Source::CreateSettings(LPCSTR IniSection)
{
    m_Brightness = new CBrightnessSetting(this, "Brightness", SAA7134_DEFAULT_BRIGHTNESS, 0, 255, IniSection);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", SAA7134_DEFAULT_CONTRAST, 0, 255, IniSection);
    m_Settings.push_back(m_Contrast);

    m_Saturation = new CSaturationSetting(this, "Saturation", SAA7134_DEFAULT_SATURATION, 0, 255, IniSection);
    m_Settings.push_back(m_Saturation);

    m_Hue = new CHueSetting(this, "Hue", SAA7134_DEFAULT_HUE, 0, 255, IniSection);
    m_Settings.push_back(m_Hue);

    m_Overscan = new COverscanSetting(this, "Overscan", SAA7134_DEFAULT_NTSC_OVERSCAN, 0, 150, IniSection);
    m_Settings.push_back(m_Overscan);

    m_PixelWidth = new CPixelWidthSetting(this, "Sharpness", 720, 120, DSCALER_MAX_WIDTH, IniSection);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth");
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 6, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection);
    m_Settings.push_back(m_VideoFormat);

    m_ReversePolarity = new CYesNoSetting("Reverse Polarity", FALSE, IniSection, "ReversePolarity");
    m_Settings.push_back(m_ReversePolarity);

    m_CardType = new CSliderSetting("Card Type", TVCARD_UNKNOWN, TVCARD_UNKNOWN, m_pSAA7134Card->GetMaxCards() - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_HPLLMode = new CHPLLModeSetting(this, "HPLL Locking Mode", HPLLMODE_FAST_TRACKING, HPLLMODE_TV, HPLLMODE_LASTONE - 1, IniSection);
    m_Settings.push_back(m_HPLLMode);

    m_WhitePeak = new CWhitePeakSetting(this, "White Peak", TRUE, IniSection);
    m_Settings.push_back(m_WhitePeak);

    m_ColorPeak = new CColorPeakSetting(this, "Color Peak", TRUE, IniSection);
    m_Settings.push_back(m_ColorPeak);

    m_AdaptiveCombFilter = new CAdaptiveCombFilterSetting(this, "Adaptive Comb Filter", TRUE, IniSection);
    m_Settings.push_back(m_AdaptiveCombFilter);

    m_HDelay = new CHDelaySetting(this, "Horizontal Delay", 0, 0, 20, IniSection);
    m_HDelay->SetStepValue(2);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay", 0, -60, 260, IniSection);
    m_VDelay->SetStepValue(2);
    m_Settings.push_back(m_VDelay);

    m_AudioStandard = new CAudioStandardSetting(this, "Audio Standard", AUDIOSTANDARD_BG_DUAL_FM, AUDIOSTANDARD_BG_DUAL_FM, AUDIOSTANDARD_LASTONE-1, IniSection);
    m_Settings.push_back(m_AudioStandard);

    m_AudioChannel = new CAudioChannelSetting(this, "Audio Channel", AUDIOCHANNEL_STEREO, AUDIOCHANNEL_MONO, AUDIOCHANNEL_LANGUAGE2, IniSection);
    m_Settings.push_back(m_AudioChannel);

    m_AudioSampleRate = new CAudioSampleRateSetting(this, "Audio Sample Rate", AUDIOSAMPLERATE_32000Hz, AUDIOSAMPLERATE_32000Hz, AUDIOSAMPLERATE_48000Hz, IniSection);
    m_Settings.push_back(m_AudioSampleRate);

    m_AutoStereoSelect = new CAutoStereoSelectSetting(this, "Auto Stereo Select", TRUE, IniSection);
    m_Settings.push_back(m_AutoStereoSelect);

    m_Volume = new CVolumeSetting(this, "Volume", 0, 0, 1000, IniSection);
    m_Volume->SetStepValue(20);
    m_Settings.push_back(m_Volume);

    m_Bass = new CBassSetting(this, "Bass", 0, -96, 127, IniSection);
    m_Settings.push_back(m_Bass);

    m_Treble = new CTrebleSetting(this, "Treble", 0, -96, 127, IniSection);
    m_Settings.push_back(m_Treble);

    m_Balance = new CBalanceSetting(this, "Balance", 0, -127, 127, IniSection);
    m_Settings.push_back(m_Balance);

    m_bSavePerInput = new CYesNoSetting("Save Per Input", TRUE, IniSection, "SavePerInput");
    m_Settings.push_back(m_bSavePerInput);

    m_bSavePerFormat = new CYesNoSetting("Save Per Format", TRUE, IniSection, "SavePerFormat");
    m_Settings.push_back(m_bSavePerFormat);

    m_AudioSource = new CAudioSourceSetting(this, "Audio Source", AUDIOINPUTSOURCE_LINE1, AUDIOINPUTSOURCE_DAC, AUDIOINPUTSOURCE_LINE2, IniSection);
    m_Settings.push_back(m_AudioSource);

    m_CustomAudioStandard = new CCustomAudioStandardSetting(this, "Use Custom Audio Standard", FALSE, IniSection);
    m_Settings.push_back(m_CustomAudioStandard);

    m_AudioMajorCarrier = new CAudioMajorCarrierSetting(this, "Audio Major Carrier", AUDIO_CARRIER_5_5, 0, AUDIO_CARRIER_10_7, IniSection);
    m_Settings.push_back(m_AudioMajorCarrier);

    m_AudioMinorCarrier = new CAudioMinorCarrierSetting(this, "Audio Minor Carrier", AUDIO_CARRIER_5_5, 0, AUDIO_CARRIER_10_7, IniSection);
    m_Settings.push_back(m_AudioMinorCarrier);

    m_AudioMajorCarrierMode = new CAudioMajorCarrierModeSetting(this, "Audio Major Carrier Mode", AUDIOCHANNELMODE_FM, AUDIOCHANNELMODE_NONE, AUDIOCHANNELMODE_EIAJ, IniSection);
    m_Settings.push_back(m_AudioMajorCarrierMode);

    m_AudioMinorCarrierMode = new CAudioMinorCarrierModeSetting(this, "Audio Minor Carrier Mode", AUDIOCHANNELMODE_FM, AUDIOCHANNELMODE_NONE, AUDIOCHANNELMODE_EIAJ, IniSection);
    m_Settings.push_back(m_AudioMinorCarrierMode);

    m_AudioCh1FMDeemph = new CAudioCh1FMDeemphSetting(this, "Audio Channel 1 FM De-emphasis", AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_ADAPTIVE, IniSection);
    m_Settings.push_back(m_AudioCh1FMDeemph);

    m_AudioCh2FMDeemph = new CAudioCh2FMDeemphSetting(this, "Audio Channel 2 FM De-emphasis", AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_OFF, AUDIOFMDEEMPHASIS_ADAPTIVE, IniSection);
    m_Settings.push_back(m_AudioCh2FMDeemph);

    // HELPTEXT: Automatic Volume Leveling control to avoid
    // digital clipping at analog audio output
    m_AutomaticVolumeLevel = new CAutomaticVolumeLevelSetting(this, "Automatic Volume Leveling", AUTOMATICVOLUME_MEDIUMDECAY, AUTOMATICVOLUME_LONGDECAY, IniSection, m_AutomaticVolumeSzList);
    m_Settings.push_back(m_AutomaticVolumeLevel);

    // HELPTEXT: Lower means better VBI reception/decoding but
    // how far it can be lowered depends on individual cards.
    m_VBIUpscaleDivisor = new CVBIUpscaleDivisorSetting(this, "VBI Upscale Divisor", 0x1A8, 0x186, 0x400, IniSection);
    m_Settings.push_back(m_VBIUpscaleDivisor);

    m_VBIDebugOverlay = new CYesNoSetting("VBI Debug Overlay", FALSE, IniSection, "VBIDebugOverlay");
    m_Settings.push_back(m_VBIDebugOverlay);

    m_AutomaticGainControl = new CAutomaticGainControlSetting(this, "Automatic Gain Control", TRUE, IniSection);
    m_Settings.push_back(m_AutomaticGainControl);

    m_GainControlLevel = new CGainControlLevelSetting(this, "Gain Control Level", 0x0100, 0x0000, 0x01FF, IniSection);
    m_Settings.push_back(m_GainControlLevel);

    m_VideoMirror = new CVideoMirrorSetting(this, "Mirroring", FALSE, IniSection);
    m_Settings.push_back(m_VideoMirror);

#ifdef _DEBUG    
    if (SAA7134_SETTING_LASTONE != m_Settings.size())
    {
        LOGD("Number of settings in SAA7134 source is not equal to the number of settings in DS_Control.h");
        LOGD("DS_Control.h or SAA7134Source.cpp are probably not in sync with eachother.");
    }
#endif

    SetupSettings();
}


void CSAA7134Source::SetupSettings()
{
    #define PER_VIDEOINPUT      SETUP_PER_VIDEOINPUT
    #define PER_VIDEOFORMAT     SETUP_PER_VIDEOFORMAT
    #define PER_AUDIOINPUT      SETUP_PER_AUDIOINPUT
    #define PER_CHANNEL         SETUP_PER_CHANNEL
    
    TSettingsSetup SettingsSetup[] =
    {
        /*
        { m_CardType,               SETUP_NONE },
        { m_TunerType,              SETUP_NONE },
        { m_bSavePerInput,          SETUP_NONE },
        { m_bSavePerFormat,         SETUP_NONE },

        { m_CustomPixelWidth,       SETUP_SINGLE },
        { m_ReversePolarity,        SETUP_SINGLE },
        { m_AutomaticVolumeLevel,   SETUP_SINGLE },
        { m_VBIUpscaleDivisor,      SETUP_SINGLE },
        { m_VideoMirror,            SETUP_SINGLE },
        */

        { m_VideoSource,            SETUP_CHANGE_VIDEOINPUT },
        { m_AutomaticGainControl,   PER_VIDEOINPUT },
        { m_GainControlLevel,       PER_VIDEOINPUT },
        { m_HPLLMode,               PER_VIDEOINPUT | PER_CHANNEL },
        { m_WhitePeak,              PER_VIDEOINPUT | PER_CHANNEL },
        { m_ColorPeak,              PER_VIDEOINPUT | PER_CHANNEL },
        { m_AdaptiveCombFilter,     PER_VIDEOINPUT | PER_CHANNEL },
        { m_HDelay,                 PER_VIDEOINPUT | PER_CHANNEL },
        { m_VDelay,                 PER_VIDEOINPUT | PER_CHANNEL },

        { m_VideoFormat,            SETUP_CHANGE_VIDEOFORMAT | PER_VIDEOINPUT | PER_CHANNEL },
        { m_Brightness,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Contrast,               PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Saturation,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Hue,                    PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_Overscan,               PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_PixelWidth,             PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },
        { m_AudioStandard,          PER_VIDEOINPUT | PER_VIDEOFORMAT | PER_CHANNEL },

        { m_AudioSource,            SETUP_CHANGE_AUDIOINPUT | PER_VIDEOINPUT },
        { m_AudioSampleRate,        PER_VIDEOINPUT | PER_AUDIOINPUT },
        { m_AudioChannel,           PER_VIDEOINPUT | PER_AUDIOINPUT },
        { m_AutoStereoSelect,       PER_VIDEOINPUT | PER_AUDIOINPUT },
        { m_Volume,                 PER_VIDEOINPUT | PER_AUDIOINPUT | PER_CHANNEL },
        { m_Bass,                   PER_VIDEOINPUT | PER_AUDIOINPUT | PER_CHANNEL },
        { m_Treble,                 PER_VIDEOINPUT | PER_AUDIOINPUT | PER_CHANNEL },
        { m_Balance,                PER_VIDEOINPUT | PER_AUDIOINPUT | PER_CHANNEL },

        { m_CustomAudioStandard,    PER_CHANNEL },
        { m_AudioMajorCarrier,      PER_CHANNEL },
        { m_AudioMinorCarrier,      PER_CHANNEL },
        { m_AudioMajorCarrierMode,  PER_CHANNEL },
        { m_AudioMinorCarrierMode,  PER_CHANNEL },
        { m_AudioCh1FMDeemph,       PER_CHANNEL },
        { m_AudioCh2FMDeemph,       PER_CHANNEL },
        { NULL,                     0 }
    };

    #undef PER_VIDEOINPUT
    #undef PER_VIDEOFORMAT
    #undef PER_AUDIOINPUT
    #undef PER_CHANNEL

    WORD ListCount = sizeof(SettingsSetup)/sizeof(TSettingsSetup);

    m_SettingsSetup = new TSettingsSetup[ListCount];
    memcpy(m_SettingsSetup, SettingsSetup, sizeof(SettingsSetup));
}


void CSAA7134Source::SaveSettings(WORD ChangedSetup)
{
    WORD PerSetupMask;

    PerSetupMask = (ChangedSetup & SETUP_CHANGE_ANY) >> 4;

    for (int i(0); m_SettingsSetup[i].Setting != NULL; i++)
    {
        if (m_SettingsSetup[i].Setup & PerSetupMask)
        {
            // Save the setting
            m_SettingsSetup[i].Setting->WriteToIni(TRUE);

            // If this change affects another setup
            if (m_SettingsSetup[i].Setup & SETUP_CHANGE_ANY)
            {
                PerSetupMask |= (m_SettingsSetup[i].Setup & SETUP_CHANGE_ANY) >> 4;
            }
        }
    }
}


void CSAA7134Source::LoadSettings(WORD ChangedSetup)
{
    WORD PerSetupMask;
    WORD EnabledSectionMask = SETUP_PER_AUDIOINPUT;
    WORD IniSectionMask = 0xFF;
    char szSection[128];

    // Find out which sections are used
    if (m_bSavePerInput->GetValue())
    {
        EnabledSectionMask |= SETUP_PER_VIDEOINPUT;
    }
    if (m_bSavePerFormat->GetValue())
    {
        EnabledSectionMask |= SETUP_PER_VIDEOFORMAT;
    }

    // Adjust to the new defaults
    ChangeDefaultsForSetup(ChangedSetup);
    PerSetupMask = (ChangedSetup & SETUP_CHANGE_ANY) >> 4;

    for (int i(0); m_SettingsSetup[i].Setting != NULL; i++)
    {
        if (m_SettingsSetup[i].Setup & PerSetupMask)
        {
            // Generate the section name if we don't already have it
            if ((m_SettingsSetup[i].Setup & EnabledSectionMask) != IniSectionMask)
            {
                IniSectionMask = (m_SettingsSetup[i].Setup & EnabledSectionMask);
                GetIniSectionName(szSection, IniSectionMask);
            }

            // Change the section and read the value
            m_SettingsSetup[i].Setting->SetSection(szSection);
            m_SettingsSetup[i].Setting->ReadFromIni();

            // If this change affects another setup
            if (m_SettingsSetup[i].Setup & SETUP_CHANGE_ANY)
            {
                ChangeDefaultsForSetup(m_SettingsSetup[i].Setup);
                PerSetupMask |= (m_SettingsSetup[i].Setup & SETUP_CHANGE_ANY) >> 4;
            }
        }
    }

    ChangeChannelSectionNames();
}


void CSAA7134Source::GetIniSectionName(char* pBuffer, WORD IniSectionMask)
{
    sprintf(pBuffer, "%s", m_Section.c_str());
    pBuffer += strlen(pBuffer);

    if (IniSectionMask & SETUP_PER_VIDEOINPUT)
    {
        sprintf(pBuffer, "_VI%d", m_VideoSource->GetValue());
        pBuffer += strlen(pBuffer);
    }
    if (IniSectionMask & SETUP_PER_VIDEOFORMAT)
    {
        sprintf(pBuffer, "_VF%d", m_VideoFormat->GetValue());
        pBuffer += strlen(pBuffer);
    }
    if (IniSectionMask & SETUP_PER_AUDIOINPUT)
    {
        sprintf(pBuffer, "_AI%d", m_AudioSource->GetValue());
        pBuffer += strlen(pBuffer);
    }
    if (IniSectionMask & SETUP_PER_CHANNEL)
    {
        /*
        sprintf(pBuffer, "_C%d", m_Channel->GetValue());
        pBuffer += strlen(pBuffer);
        */
    }
}


void CSAA7134Source::ChangeDefaultsForSetup(WORD Setup)
{
    if (Setup & SETUP_CHANGE_VIDEOINPUT)
    {
        ChangeDefaultsForVideoInput();
    }
    if (Setup & SETUP_CHANGE_VIDEOFORMAT)
    {
        ChangeDefaultsForVideoFormat();
    }
    if (Setup & SETUP_CHANGE_AUDIOINPUT)
    {
        ChangeDefaultsForAudioInput();
    }
}


void CSAA7134Source::ChangeDefaultsForVideoInput()
{
    int nInput = m_VideoSource->GetValue();

    m_AudioSource->ChangeDefault(m_pSAA7134Card->GetInputAudioLine(nInput));
}


void CSAA7134Source::ChangeDefaultsForVideoFormat()
{
    eVideoFormat VideoFormat = (eVideoFormat)m_VideoFormat->GetValue();

    if(IsPALVideoFormat(VideoFormat))
    {
        m_Saturation->ChangeDefault(SAA7134_DEFAULT_PAL_SATURATION);
        m_Overscan->ChangeDefault(SAA7134_DEFAULT_PAL_OVERSCAN);
    }
    else if(IsNTSCVideoFormat(VideoFormat))
    {
        m_Saturation->ChangeDefault(SAA7134_DEFAULT_NTSC_SATURATION);
        m_Overscan->ChangeDefault(SAA7134_DEFAULT_NTSC_OVERSCAN);
    }
    else
    {
        m_Saturation->ChangeDefault(SAA7134_DEFAULT_SATURATION);
        m_Overscan->ChangeDefault(SAA7134_DEFAULT_OVERSCAN);
    }

    eAudioStandard AudioStandard = TVFormat2AudioStandard(VideoFormat);
    m_AudioStandard->ChangeDefault(AudioStandard);
}


void CSAA7134Source::ChangeDefaultsForAudioInput()
{

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
    m_pSAA7134Card->ResetHardware();

    SetupDMAMemory();

    SetupVideoSource();
    SetupAudioSource();
    SetupVideoStandard();
    SetupAudioStandard();

    SetOverscan();

    m_pSAA7134Card->SetAutomaticGainControl(m_AutomaticGainControl->GetValue());
    m_pSAA7134Card->SetGainControl(m_GainControlLevel->GetValue());
    m_pSAA7134Card->SetVideoMirror(m_VideoMirror->GetValue());

    m_pSAA7134Card->SetHPLLMode((eHPLLMode)m_HPLLMode->GetValue());

    m_pSAA7134Card->SetWhitePeak(m_WhitePeak->GetValue());
    m_pSAA7134Card->SetColorPeak(m_ColorPeak->GetValue());
    m_pSAA7134Card->SetCombFilter(m_AdaptiveCombFilter->GetValue());

    m_pSAA7134Card->SetVBIGeometry(m_VBIUpscaleDivisor->GetValue());

    if (m_AutoStereoSelect->GetValue())
    {
        m_pSAA7134Card->SetAudioChannel(AUDIOCHANNEL_STEREO);
    }
    else
    {
        m_pSAA7134Card->SetAudioChannel((eAudioChannel)m_AudioChannel->GetValue());
    }

    m_AutomaticVolumeLevel->SetValue(m_AutomaticVolumeLevel->GetValue());
    m_AudioSampleRate->SetValue(m_AudioSampleRate->GetValue());

    m_Volume->SetValue(m_Volume->GetValue());
    m_Bass->SetValue(m_Bass->GetValue());
    m_Treble->SetValue(m_Treble->GetValue());
    m_Balance->SetValue(m_Balance->GetValue());
}


void CSAA7134Source::SetupVideoSource()
{
    m_pSAA7134Card->SetVideoSource(m_VideoSource->GetValue());
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
    NotifySizeChange();

    m_pSAA7134Card->SetBrightness(m_Brightness->GetValue());
    m_pSAA7134Card->SetContrast(m_Contrast->GetValue());
    m_pSAA7134Card->SetSaturation(m_Saturation->GetValue());
    m_pSAA7134Card->SetHue(m_Hue->GetValue());
}


void CSAA7134Source::ChangeCardSettings(WORD ChangedSetup)
{
    if (ChangedSetup & SETUP_CHANGE_VIDEOINPUT)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputSource)m_AudioSource->GetValue());
        m_AutomaticGainControl->SetValue(m_AutomaticGainControl->GetValue());
        m_GainControlLevel->SetValue(m_GainControlLevel->GetValue());
        m_HPLLMode->SetValue(m_HPLLMode->GetValue());
        m_WhitePeak->SetValue(m_WhitePeak->GetValue());
        m_ColorPeak->SetValue(m_ColorPeak->GetValue());
        m_AdaptiveCombFilter->SetValue(m_AdaptiveCombFilter->GetValue());
    }

    if (ChangedSetup & (SETUP_CHANGE_VIDEOINPUT | SETUP_CHANGE_VIDEOFORMAT))
    {
        SetupVideoStandard();
        SetupAudioStandard();

        SetOverscan();
    }

    if (ChangedSetup & (SETUP_CHANGE_VIDEOINPUT | SETUP_CHANGE_AUDIOINPUT))
    {
        m_AudioSampleRate->SetValue(m_AudioSampleRate->GetValue());
        m_AudioChannel->SetValue(m_AudioChannel->GetValue());
        m_AutoStereoSelect->SetValue(m_AutoStereoSelect->GetValue());
        m_Volume->SetValue(m_Volume->GetValue());
        m_Bass->SetValue(m_Bass->GetValue());
        m_Treble->SetValue(m_Treble->GetValue());
        m_Balance->SetValue(m_Balance->GetValue());
    }
}


void CSAA7134Source::Start()
{
    m_pSAA7134Card->StartCapture(bCaptureVBI);
    Audio_Unmute();
    Timing_Reset();
    {
        // This timer is used to update STATUS_AUDIO
        SetTimer(hWnd, TIMER_MSP, 1000, NULL);
    }
    NotifySquarePixelsCheck();
    m_ProcessingRegionID = REGIONID_INVALID;
}


void CSAA7134Source::Stop()
{
    // stop capture
    m_pSAA7134Card->StopCapture();
    Audio_Mute();
    {
        KillTimer(hWnd, TIMER_MSP);
    }
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

        m_pSAA7134Card->SetPageTable(RegionID, m_VBIPageTablePhysical[nFrame], nPages);
        m_pSAA7134Card->SetBaseOffsets(RegionID, 0, 0 + kMAX_VBILINES * 2048, 2048);
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


void CSAA7134Source::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    if (AccurateTiming)
    {
        GetNextFieldAccurate(pInfo);
    }
    else
    {
        GetNextFieldNormal(pInfo);
    }

    if (!pInfo->bRunningLate)
    {
    }

    if (m_IsFieldOdd)
    {
        if (m_ReversePolarity->GetValue() == FALSE)
        {
            if (m_VBIDebugOverlay->GetValue())
            {
                for (int nLine(0); nLine < m_CurrentVBILines; nLine++)
                {
                    for (int i(0); i < 2048; i++)
                    {
                        m_OddFields[m_CurrentFrame].pData[nLine * 4096 + i] =
                            m_pVBILines[m_CurrentFrame][(m_CurrentVBILines + nLine) * 2048 + i];
                    }
                }
            }

            GiveNextField(pInfo, &m_OddFields[m_CurrentFrame]);
        }
        else
        {
            GiveNextField(pInfo, &m_EvenFields[(pInfo->CurrentFrame + 1) % 2]);
        }
    }
    else
    {
        if (m_ReversePolarity->GetValue() == FALSE)
        {
            if (m_VBIDebugOverlay->GetValue())
            {
                for (int nLine(0); nLine < m_CurrentVBILines; nLine++)
                {
                    for (int i(0); i < 2048; i++)
                    {
                        m_EvenFields[m_CurrentFrame].pData[nLine * 4096 + i] =
                            m_pVBILines[m_CurrentFrame][nLine * 2048 + i];
                    }
                }
            }

            GiveNextField(pInfo, &m_EvenFields[m_CurrentFrame]);
        }
        else
        {
            GiveNextField(pInfo, &m_OddFields[m_CurrentFrame]);
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


void CSAA7134Source::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    eRegionID   RegionID;
    BOOL        bIsFieldOdd;
    int         FieldIndex;
    int         SkippedFields;
    BOOL        bTolerateDrops = TRUE;
    BOOL        bFirstTime;

    bFirstTime = (m_ProcessingRegionID == REGIONID_INVALID);

    // This function waits until field is ready
    WaitForFinishedField(RegionID, bIsFieldOdd, pInfo);

    if (bFirstTime)
    {
        FieldIndex = EnumulateField(RegionID, bIsFieldOdd);
        SkippedFields = 0;
    }
    else
    {
        FieldIndex = EnumulateField(RegionID, bIsFieldOdd);
        SkippedFields = (4 + FieldIndex - m_LastFieldIndex - 1) % 4;
    }

    if (SkippedFields != 0)
    {
        if (bTolerateDrops && SkippedFields < 2)
        {
            // Try to catch up but we have to drop if too late
            if (EnumulateField(m_ProcessingRegionID,
                m_IsProcessingFieldOdd) == (m_LastFieldIndex + 1) % 4)
            {
                m_LastFieldIndex = (m_LastFieldIndex + 1) % 4;
                Timing_AddDroppedFields(1);
                LOG(2, " Dropped Frame");
            }
            else
            {
                LOG(2, "Running Late");
            }

            m_ProcessingRegionID = RegionID;
            m_IsProcessingFieldOdd = bIsFieldOdd;

            FieldIndex = (m_LastFieldIndex + 1) % 4;
            DenumulateField(FieldIndex, &RegionID, &bIsFieldOdd);
        }
        else
        {
            // delete all history
            ClearPictureHistory(pInfo);
            pInfo->bMissedFrame = TRUE;
            Timing_AddDroppedFields(SkippedFields);
            LOG(2, " Dropped Frame");
        }
    }
    else
    {
        pInfo->bMissedFrame = FALSE;
        if (pInfo->bRunningLate)
        {
            Timing_AddDroppedFields(1);
            LOG(2, "Running Late");
        }
    }

    m_LastFieldIndex = FieldIndex;

    if (RegionID2TaskID(RegionID) == TASKID_A)
    {
        m_CurrentFrame = 0;
    }
    else
    {
        m_CurrentFrame = 1;
    }

    m_IsFieldOdd = bIsFieldOdd;
}


void CSAA7134Source::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    eRegionID RegionID;
    BOOL bIsFieldOdd;
    BOOL bSlept = FALSE;
    int FieldIndex;
    int SkippedFields;
    BOOL bFirstTime = FALSE;

    if (m_ProcessingRegionID == REGIONID_INVALID)
    {
        while (!m_pSAA7134Card->GetProcessingRegion(m_ProcessingRegionID,
                        m_IsProcessingFieldOdd))
        {
            // do nothing
        }
        bFirstTime = TRUE;
    }

    while (!GetFinishedField(RegionID, bIsFieldOdd))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    if (bFirstTime)
    {
        FieldIndex = EnumulateField(RegionID, bIsFieldOdd);
        SkippedFields = 0;
    }
    else
    {
        FieldIndex = EnumulateField(RegionID, bIsFieldOdd);
        SkippedFields = (4 + FieldIndex - m_LastFieldIndex - 1) % 4;
    }

    if (SkippedFields == 0)
    {
    }
    else if (SkippedFields == 1)
    {
        m_ProcessingRegionID = RegionID;
        m_IsProcessingFieldOdd = bIsFieldOdd;
        FieldIndex = (m_LastFieldIndex + 1) % 4;
        DenumulateField(FieldIndex, &RegionID, &bIsFieldOdd);
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Slightly late");
    }
    // This might not be possible with only a 4 field cycle
    else if (SkippedFields == 2)
    {
        m_ProcessingRegionID = RegionID;
        m_IsProcessingFieldOdd = bIsFieldOdd;
        FieldIndex = (m_LastFieldIndex + 1) % 4;
        DenumulateField(FieldIndex, &RegionID, &bIsFieldOdd);
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Very late");
    }
    else
    {
        // delete all history
        ClearPictureHistory(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(SkippedFields);
        LOG(2, " Dropped Frame");
        Timing_Reset();
    }

    m_LastFieldIndex = FieldIndex;

    if (RegionID2TaskID(RegionID) == TASKID_A)
    {
        m_CurrentFrame = 0;
    }
    else
    {
        m_CurrentFrame = 1;
    }

    m_IsFieldOdd = bIsFieldOdd;

    // we've just got a new field
    // we are going to time the odd to odd
    // input frequency
    if (m_IsFieldOdd)
    {
        Timing_UpdateRunningAverage(pInfo, 2);
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}


void CSAA7134Source::GiveNextField(TDeinterlaceInfo* pInfo, TPicture* picture)
{
    if (pInfo->bMissedFrame)
    {
        ClearPictureHistory(pInfo);
        picture->IsFirstInSeries = TRUE;
    }
    else
    {
        picture->IsFirstInSeries = FALSE;
    }

    // Re-enumerate our 4 field cycle to a 10 field cycle
    if ((picture->Flags & PICTURE_INTERLACED_EVEN) > 0 ||
        pInfo->bMissedFrame)
    {
        pInfo->CurrentFrame = (pInfo->CurrentFrame + 1) % 5;
    }

    // we only have 4 unique fields
    ShiftPictureHistory(pInfo, 4);
    pInfo->PictureHistory[0] = picture;
}


BOOL CSAA7134Source::WaitForFinishedField(eRegionID& RegionID, BOOL& bIsFieldOdd,
                                          TDeinterlaceInfo* pInfo)
{
    BOOL        bUsingInterrupts = FALSE;
    ULONGLONG   Frequency;

    if (bUsingInterrupts)
    {
        // Need to implement interrupts

        return FALSE;
    }

    if (QueryPerformanceFrequency((PLARGE_INTEGER)&Frequency))
    {
        ULONGLONG       PerformanceCount;
        ULONGLONG       WaitTimeSpent;
        ULONG           SleepTime;
        BOOL            bWaited = FALSE;

        if (m_ProcessingRegionID == REGIONID_INVALID)
        {
            while (!m_pSAA7134Card->GetProcessingRegion(
                                m_ProcessingRegionID,
                                m_IsProcessingFieldOdd
                            ))
            {
                // do nothing
            }

            QueryPerformanceCounter((PLARGE_INTEGER)&m_LastPerformanceCount);
            m_MinimumFieldDelay = 0;
        }

        while (!GetFinishedField(RegionID, bIsFieldOdd))
        {
            bWaited = TRUE;

            QueryPerformanceCounter((PLARGE_INTEGER)&PerformanceCount);
            WaitTimeSpent = PerformanceCount - m_LastPerformanceCount;

            if (WaitTimeSpent > m_MinimumFieldDelay)
            {
                // Wait blindly with 1ms delays
                SleepTime = (Frequency * 0.001);
            }
            else
            {
                SleepTime = (m_MinimumFieldDelay - WaitTimeSpent) * 3 / 4;
            }

            // Sleep is only accurate to 10ms.  We should have 1ms accuracy.
            Sleep(SleepTime * 1000 / Frequency);
        }

        QueryPerformanceCounter((PLARGE_INTEGER)&m_LastPerformanceCount);

        if (bWaited)
        {
            // If we waited longer than 30ms, something probably went wrong
            if (WaitTimeSpent * 1000 / Frequency > 30)
            {
                // reset to 10ms
                m_MinimumFieldDelay = (Frequency * 0.010);
            }
            else
            {
                ULONGLONG Delta = m_LastPerformanceCount - PerformanceCount;
                m_MinimumFieldDelay = WaitTimeSpent + Delta / 2;
            }
            pInfo->bRunningLate = FALSE;
        }
    }
    else
    {
        BOOL bSlept = FALSE;

        while (m_ProcessingRegionID == REGIONID_INVALID)
        {
            m_pSAA7134Card->GetProcessingRegion(
                            m_ProcessingRegionID,
                            m_IsProcessingFieldOdd
                            );
        }

        while (!GetFinishedField(RegionID, bIsFieldOdd))
        {
            // need to sleep more often
            // so that we don't take total control of machine
            // in normal operation
            Timing_SmartSleep(pInfo, FALSE, bSlept);
            pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        }
    }

    return TRUE;
}


BOOL CSAA7134Source::GetFinishedField(eRegionID& DoneRegionID, BOOL& bDoneIsFieldOdd)
{
    eRegionID RegionID;
    BOOL bIsFieldOdd;

/*    // DEBUGGIN
    while (1)
    {
        m_pSAA7134Card->CheckRegisters((DWORD*) &m_pDisplay[0][2048 + 4190],
            (DWORD*) &m_pDisplay[0][4190], (DWORD*) &m_pDisplay[1][2048 + 4190],
            (DWORD*) &m_pDisplay[1][4190]);
    }
    //*/

    if (!m_pSAA7134Card->GetProcessingRegion(RegionID, bIsFieldOdd))
    {
        return FALSE;
    }

    if (RegionID == m_ProcessingRegionID && bIsFieldOdd == m_IsProcessingFieldOdd)
    {
        return FALSE;
    }

    DoneRegionID = m_ProcessingRegionID;
    bDoneIsFieldOdd = m_IsProcessingFieldOdd;

    m_ProcessingRegionID = RegionID;
    m_IsProcessingFieldOdd = bIsFieldOdd;

    return TRUE;
}


int CSAA7134Source::EnumulateField(eRegionID RegionID, BOOL bIsFieldOdd)
{
    if (RegionID == REGIONID_VIDEO_A)
    {
        return bIsFieldOdd ? 0 : 1;
    }
    else if (RegionID == REGIONID_VIDEO_B)
    {
        return bIsFieldOdd ? 2 : 3;
    }

    return -1;
}


void CSAA7134Source::DenumulateField(int Index, eRegionID* RegionID, BOOL* bIsFieldOdd)
{
    switch (Index)
    {
    case 0: *RegionID = REGIONID_VIDEO_A; *bIsFieldOdd = TRUE; break;
    case 1: *RegionID = REGIONID_VIDEO_A; *bIsFieldOdd = FALSE; break;
    case 2: *RegionID = REGIONID_VIDEO_B; *bIsFieldOdd = TRUE; break;
    case 3: *RegionID = REGIONID_VIDEO_B; *bIsFieldOdd = FALSE; break;
    }
}


// \todo this might be bad
CSAA7134Card* CSAA7134Source::GetSAA7134Card()
{
    return m_pSAA7134Card;
}


LPCSTR CSAA7134Source::GetStatus()
{
    static LPCSTR pRetVal = "";
    if (IsInTunerMode())
    {
        if (*VT_GetStation() != 0x00)
        {
            pRetVal = VT_GetStation();
        }
        else if (VPSLastName[0] != 0x00)
        {
            pRetVal = VPSLastName;
        }
        else
        {
            pRetVal = Channel_GetName();
        }
    }
    else
    {
        pRetVal = m_pSAA7134Card->GetInputName(m_VideoSource->GetValue());
    }
    return pRetVal;
}


/*
 *  These are used by the "Video Adjustments ..." dialog.
 */

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

ISetting* CSAA7134Source::GetOverscan()
{
    return m_Overscan;
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


////////////////////////////////////////////////////////////////////////
void CSAA7134Source::VideoSourceOnChange(long NewValue, long OldValue)
{
    NotifyInputChange(1, VIDEOINPUT, OldValue, NewValue);

    Stop_Capture();
    Audio_Mute();

    SetupVideoSource();

    SaveSettings(SETUP_CHANGE_VIDEOINPUT);
    LoadSettings(SETUP_CHANGE_VIDEOINPUT);
    ChangeCardSettings(SETUP_CHANGE_VIDEOINPUT);

    NotifyInputChange(0, VIDEOINPUT, OldValue, NewValue);

    // set up sound
    if (m_pSAA7134Card->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
    }
    Audio_Unmute();
    Start_Capture();
}

void CSAA7134Source::VideoFormatOnChange(long NewValue, long OldValue)
{
    NotifyInputChange(1, VIDEOFORMAT, OldValue, NewValue);
    Stop_Capture();

    SaveSettings(SETUP_CHANGE_VIDEOFORMAT);
    LoadSettings(SETUP_CHANGE_VIDEOFORMAT);
    ChangeCardSettings(SETUP_CHANGE_VIDEOFORMAT);

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

    m_pSAA7134Card->SetGeometry(m_CurrentX, m_CurrentY,
                                m_HDelay->GetValue(), m_VDelay->GetValue());
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
        m_VDelay->SetValue(Minimum, ONCHANGE_NONE);

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

void CSAA7134Source::OverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialOverscan = Overscan;
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
    m_pSAA7134Card->SetCombFilter(NewValue);
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


/////////////////////////////////////////////////////////////////////////

BOOL CSAA7134Source::IsInTunerMode()
{
    return m_pSAA7134Card->IsInputATuner(m_VideoSource->GetValue());
}


void CSAA7134Source::SetupCard()
{
    BOOL bCardChanged = FALSE;

    if (m_CardType->GetValue() == TVCARD_UNKNOWN)
    {
        // try to detect the card
        m_CardType->SetValue(m_pSAA7134Card->AutoDetectCardType());
        m_TunerType->SetValue(m_pSAA7134Card->AutoDetectTuner((eSAA7134CardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        m_bSelectCardCancelButton = FALSE;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
        m_bSelectCardCancelButton = TRUE;

        bCardChanged = TRUE;
    }
    m_pSAA7134Card->SetCardType(m_CardType->GetValue());
    m_pSAA7134Card->InitTuner((eTunerId)m_TunerType->GetValue());
}

void CSAA7134Source::ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
    // now do defaults based on the processor speed selected
    if (ProcessorSpeed == 0)
    {
        // User has selected below 300 MHz
        m_PixelWidth->ChangeDefault(640);
    }
    else if (ProcessorSpeed == 1)
    {
        // User has selected 300 MHz - 500 MHz
        m_PixelWidth->ChangeDefault(720);
    }
    else if (ProcessorSpeed == 2)
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

void CSAA7134Source::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if (m_TunerType->GetValue() != TUNER_ABSENT)
    {
        eVideoFormat videoFormat = m_pSAA7134Card->GetTuner()->GetDefaultVideoFormat();
        m_VideoFormat->ChangeDefault(videoFormat);

        SaveSettings(SETUP_CHANGE_VIDEOFORMAT);
        LoadSettings(SETUP_CHANGE_VIDEOFORMAT);
        ChangeCardSettings(SETUP_CHANGE_VIDEOFORMAT);
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
        // This is used in DecodeVBI() so old VBI isn't
        // used for the new channel
        m_ChannelChangeTick = GetTickCount();
        return TRUE;
    }
    return FALSE;
}


BOOL CSAA7134Source::IsVideoPresent()
{
    return m_pSAA7134Card->IsVideoPresent();
}


void CSAA7134Source::DecodeVBI(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    // VBI should have been DMA'd before the video
    BYTE* pVBI = (LPBYTE) m_pVBILines[m_CurrentFrame];

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

    BYTE ConvertBuffer[2048];

    if (m_IsFieldOdd)
    {
        pVBI += m_CurrentVBILines * 2048;
    }

    // Convert SAA7134's VBI buffer to the way DScaler wants it
    // 1. Shift the data 40 bytes to to the left
    // 2. Horizontal scale 262.54% (0x400/0x186)  Some of this is already
    //    done.  We get the card to do 0x400/m_VBIUpscaleDivisor scaling
    //    for us in SAA7134Card, so we need to trim this up a bit.
    //      - ala. SAA7134Card::SetTaskVBIGeometry()
    //
    for (int i(0); i < 40; i++)
    {
        ConvertBuffer[i] = 0x00;
    }

    double ScaleRatio = (double) 0x186 / m_VBIUpscaleDivisor->GetValue();

    for (nLineTarget = 0; nLineTarget < m_CurrentVBILines; nLineTarget++)
    {
        for (int i(40), j(0); i < 2048; i++, j++)
        {
            ConvertBuffer[i] = pVBI[nLineTarget * 2048 + (int)(j * ScaleRatio)];
        }
        VBI_DecodeLine(ConvertBuffer, nLineTarget, m_IsFieldOdd);
    }
}


/*
// This tries to decode VideoText by first calculating the clock
// sync.. but doesn't work very well.
void CSAA7134Source::DecodeVBILine(BYTE* VBILine, int Line)
{
    static double StepLength = 0;
    USHORT  FallingEdge[8], Trench[8];
    USHORT  Peak, Threshold;
    BOOL    bRaising;
    USHORT  EdgeStop0, EdgeStop7;
    double  m1, m2;
    double  BitIndex;
    BYTE    RawData[45];
    int     n, i, j;

    // Find the 8 falling edges in the Clock Run-In (10101010
    // 10101010) and work out high/low the threshold.
    bRaising = TRUE;
    Peak = 0;
    Threshold = 0;

    for (i = 1, n = 0; i < 120; i++)
    {
        if (bRaising)
        {
            if (VBILine[i] > VBILine[Peak])
            {
                Peak = i;
            }
            else if (VBILine[i] < VBILine[Peak] - 32)
            {
                Threshold += VBILine[Peak];

                bRaising = FALSE;
                FallingEdge[n] = i;
                Trench[n] = i;
            }
        }
        else
        {
            if (VBILine[i] < VBILine[Trench[n]])
            {
                Trench[n] = i;
            }
            else if (VBILine[i] > VBILine[Trench[n]] + 32)
            {
                Threshold += VBILine[Trench[n]];

                if (++n == 8)
                {
                    break;
                }
                bRaising = TRUE;
                Peak = i;
            }
        }
    }

    // Make sure we have 8 trenches
    if (n != 8)
    {
        return;
    }

    // Find the end of first falling edge
    for (i = Trench[0] - 1; (VBILine[i] - VBILine[Trench[0]]) <= 32; i++) {}
    EdgeStop0 = i;

    // Find the end of the last falling edge
    for (i = Trench[7] - 1; (VBILine[i] - VBILine[Trench[7]]) <= 32; i++) {}
    EdgeStop7 = i;

    // Work out the slopes of the two falling edges
    m1 = (double) (VBILine[EdgeStop0] - VBILine[FallingEdge[0]]) / (EdgeStop0 - FallingEdge[0]);
    m2 = (double) (VBILine[EdgeStop7] - VBILine[FallingEdge[7]]) / (EdgeStop7 - FallingEdge[7]);

    //LOG(0, "%f, %f", m1, m2);

    // We can only use the edges if the slopes are similar
    if ((m1 < 0 == m2 < 0) && ((double) fabs(m1 - m2) < 2.0))
    {
        // Space between the mean of the two falling edges
        // divided by the number of bits in between (14)
        StepLength = (double) ((FallingEdge[7] + EdgeStop7) -
                               (FallingEdge[0] + EdgeStop0)) / 2 / 14;
    }

    // We can't proceed until we have a valid StepLength
    if ((USHORT) StepLength == 0)
    {
        return;
    }

    // Calculate the high/low threshold
    Threshold = Threshold / 16;


    // (VBI_decode_vt wants bits in reverse)
    RawData[0] = 0x55;
    RawData[1] = 0x55;
    RawData[2] = 0x00;

    // this should bring us to the start of 11100100
    // Add 0.5 to for rounding
    BitIndex = (double) Trench[7] + StepLength + 0.5;

    for (j = 0; j < 8; j++, BitIndex += StepLength)
    {
        if (VBILine[(USHORT)BitIndex] > Threshold)
        {
            RawData[2] |= 1 << j;
        }
    }

    // make sure there is a Framing Code
    if (RawData[2] != 0x27)
    {
        return;
    }

    // get the rest of the data
    for (i = 3; i < 45; i++)
    {
        RawData[i] = 0x00;
        for (j = 0; j < 8; j++, BitIndex += StepLength)
        {
            if (VBILine[(USHORT)BitIndex] > Threshold)
            {
                RawData[i] |= 1 << j;
            }
        }
    }

    VBI_decode_vt(RawData);
}
*/


LPCSTR CSAA7134Source::GetMenuLabel()
{
    return m_pSAA7134Card->GetCardName(m_pSAA7134Card->GetCardType());
}

void CSAA7134Source::SetOverscan()
{
    AspectSettings.InitialOverscan = m_Overscan->GetValue();
}

void CSAA7134Source::SavePerChannelSetup(int Start)
{
    if (Start&1)
    {
        m_SettingsByChannelStarted = TRUE;
        ChangeChannelSectionNames();
    }
    else
    {
        m_SettingsByChannelStarted = FALSE;
        if (m_ChannelSubSection.size() > 0)
        {
            SettingsPerChannel_UnregisterSection(m_ChannelSubSection.c_str());
        }
    }
}


void CSAA7134Source::ChangeChannelSectionNames()
{
    if (!m_SettingsByChannelStarted)
    {
        return;
    }

    std::string sOldSection = m_ChannelSubSection;

    WORD IniSectionMask = 0;

    if (m_bSavePerInput->GetValue())
    {
        IniSectionMask |= SETUP_PER_VIDEOINPUT;
    }
    if (m_bSavePerFormat->GetValue())
    {
        IniSectionMask |= SETUP_PER_VIDEOFORMAT;
    }

    if(IniSectionMask)
    {
        char szSection[128];

        GetIniSectionName(szSection, IniSectionMask);
        m_ChannelSubSection = szSection;
    }
    else
    {
        m_ChannelSubSection = m_Section;
    }

    if (sOldSection != m_ChannelSubSection)
    {
        if (sOldSection.size() > 0)
        {
            if (m_CurrentChannel >= 0)
            {
                SettingsPerChannel_SaveChannelSettings(sOldSection.c_str(), m_VideoSource->GetValue(), m_CurrentChannel, GetFormat());
            }
            SettingsPerChannel_UnregisterSection(sOldSection.c_str());
        }

        SettingsPerChannel_RegisterSetSection(m_ChannelSubSection.c_str());
        SettingsPerChannel_RegisterSetting("Brightness", "SAA713x - Brightness",TRUE, m_Brightness);
        SettingsPerChannel_RegisterSetting("Hue", "SAA713x - Hue", TRUE, m_Hue);
        SettingsPerChannel_RegisterSetting("Contrast", "SAA713x - Contrast", TRUE, m_Contrast);
        SettingsPerChannel_RegisterSetting("Saturation","SAA713x - Saturation",TRUE, m_Saturation);

        SettingsPerChannel_RegisterSetting("Overscan", "SAA713x - Overscan", FALSE, m_Overscan);

        SettingsPerChannel_RegisterSetting("Volume", "SAA713x - Volume", TRUE, m_Volume);            
        SettingsPerChannel_RegisterSetting("Balance", "SAA713x - Balance", TRUE, m_Balance);
        SettingsPerChannel_RegisterSetting("BassTreble", "SAA713x - Bass & Treble", FALSE);            
        SettingsPerChannel_RegisterSetting("BassTreble", "SAA713x - Bass & Treble", FALSE, m_Bass);            
        SettingsPerChannel_RegisterSetting("BassTreble", "SAA713x - Bass & Treble", FALSE, m_Treble);        

        SettingsPerChannel_RegisterSetting("HPLLMode", "SAA713x - HPLLMode", TRUE, m_HPLLMode);
        SettingsPerChannel_RegisterSetting("AudioChannel", "SAA713x - Audio Channel", FALSE, m_AudioChannel);

        SettingsPerChannel_RegisterSetting("Delays", "SAA713x - H/V Delay", FALSE);
        SettingsPerChannel_RegisterSetting("Delays", "SAA713x - H/V Delay", FALSE, m_HDelay);
        SettingsPerChannel_RegisterSetting("Delays", "SAA713x - H/V Delay", FALSE, m_VDelay);

        SettingsPerChannel_RegisterSetting("Miscellaneous", "SAA713x - Miscellaneous", FALSE);
        SettingsPerChannel_RegisterSetting("Miscellaneous", "SAA713x - Miscellaneous", FALSE, m_WhitePeak);
        SettingsPerChannel_RegisterSetting("Miscellaneous", "SAA713x - Miscellaneous", FALSE, m_ColorPeak);
        SettingsPerChannel_RegisterSetting("Miscellaneous", "SAA713x - Miscellaneous", FALSE, m_AdaptiveCombFilter);

        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_AudioStandard);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_CustomAudioStandard);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_AudioMajorCarrier);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_AudioMinorCarrier);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_AudioMajorCarrierMode);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_AudioMinorCarrierMode);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_AudioCh1FMDeemph);
        SettingsPerChannel_RegisterSetting("AudioStandard", "SAA713x - Audio Standard", TRUE, m_AudioCh2FMDeemph);
    }
}


int CSAA7134Source::GetDeviceIndex()
{
    return m_DeviceIndex;
}

const char* CSAA7134Source::GetChipName()
{
    return m_ChipName.c_str();
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


CTreeSettingsPage* CSAA7134Source::GetTreeSettingsPage()
{
    CTreeSettingsGeneric* pPage;

    vector <CSimpleSetting*>vSettingsList;

    vSettingsList.push_back(m_AutomaticVolumeLevel);
    vSettingsList.push_back(m_VBIUpscaleDivisor);
    vSettingsList.push_back(m_VBIDebugOverlay);
    vSettingsList.push_back(m_AutomaticGainControl);
    vSettingsList.push_back(m_GainControlLevel);
    vSettingsList.push_back(m_VideoMirror);
    vSettingsList.push_back(m_CustomPixelWidth);
    vSettingsList.push_back(m_HDelay);
    vSettingsList.push_back(m_VDelay);
    vSettingsList.push_back(m_ReversePolarity);

    pPage = new CTreeSettingsGeneric("SAA713x Advanced", vSettingsList);
    
    pPage->SetHelpID(IDH_SAA713X_ADV);

    return pPage;
}
