/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file BT848Source.cpp CBT848Source Implementation
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

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

using namespace std;

extern long EnableCancelButton;

CBT848Source::CBT848Source(SmartPtr<CBT848Card> pBT848Card, SmartPtr<CContigMemory> RiscDMAMem, SmartPtr<CUserMemory> DisplayDMAMem[5], SmartPtr<CUserMemory> VBIDMAMem[5], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex) :
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

    // can't test for IsSPISource until settings are initialized
    if (m_pBT848Card->IsSPISource(m_VideoSource->GetValue()))
    {
        m_IsVideoProgressive->SetValue(TRUE);
        for (int k(0); k < 5; k++)
        {
            m_OddFields[k].pData = m_pDisplay[k];
            m_OddFields[k].Flags = PICTURE_PROGRESSIVE;
            m_OddFields[k].IsFirstInSeries = FALSE;
        }
    }

    InitializeUI();
}

CBT848Source::~CBT848Source()
{
    EventCollector->Unregister(this);

    KillTimer(GetMainWnd(), TIMER_MSP);
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
        pAudioDecoder = dynamic_cast<CAudioDecoder*>(pEventObject);
        
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
    CSettingGroup *pAtlasGroup = GetSettingsGroup("BT848 - Atlas", SETTING_BY_FORMAT | SETTING_BY_INPUT, TRUE);

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

    m_PixelWidth = new CPixelWidthSetting(this, "Pixel Width", 720, 120, DSCALER_MAX_WIDTH, IniSection, pVideoGroup);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth", pAdvancedGroup);
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 12, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LAST_TV - 1, IniSection, pVideoFormatGroup);
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

    m_HorizOffset = new CHorizOffsetSetting(this, "Horizontal Offset", 0, 0, 255, IniSection, pAtlasGroup);
    m_Settings.push_back(m_HorizOffset);

    m_VertOffset = new CVertOffsetSetting(this, "Vertical Offset", 0, 0, 63, IniSection, pAtlasGroup);
    m_Settings.push_back(m_VertOffset);

    m_IsVideoProgressive = new CIsVideoProgressiveSetting(this, "Is Video Progressive", FALSE, IniSection, pAtlasGroup);
    m_Settings.push_back(m_IsVideoProgressive);

    m_AD9882PLL = new CAD9882PLLSetting(this, "AD9882 PLL (Width)", 800, 0, 4095, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882PLL);

    m_AD9882VCO = new CAD9882VCOSetting(this, "AD9882 VCO", 1, 0, 3, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882VCO);

    m_AD9882Pump = new CAD9882PumpSetting(this, "AD9882 Pump", 1, 0, 7, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882Pump);

    m_AD9882Phase = new CAD9882PhaseSetting(this, "AD9882 Phase", 0, 0, 31, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882Phase);

    m_AD9882PreCoast = new CAD9882PreCoastSetting(this, "AD9882 Pre-Coast", 0, 0, 255, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882PreCoast);

    m_AD9882PostCoast = new CAD9882PostCoastSetting(this, "AD9882 Post-Coast", 0, 0, 255, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882PostCoast);

    m_AD9882HSync = new CAD9882HSyncSetting(this, "AD9882 Hsync PW (Pos)", 32, 0, 255, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882HSync);

    m_AD9882SyncSep = new CAD9882SyncSepSetting(this, "AD9882 Sync Sep Thresh", 32, 0, 255, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882SyncSep);

    m_AD9882SOGThresh = new CAD9882SOGThreshSetting(this, "AD9882 SOG Thresh", 15, 0, 31, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882SOGThresh);

    m_AD9882SOG = new CAD9882SOGSetting(this, "AD9882 Sync-On-Green", FALSE, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882SOG);

    m_AD9882CoastSel = new CAD9882CoastSelSetting(this, "AD9882 Coast Enabled", TRUE, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882CoastSel);

    m_AD9882CoastOvr = new CAD9882CoastOvrSetting(this, "AD9882 Coast Override", FALSE, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882CoastOvr);

    m_AD9882CoastPol = new CAD9882CoastPolSetting(this, "AD9882 Active High Coast Polarity", TRUE, IniSection, pAtlasGroup);
    m_Settings.push_back(m_AD9882CoastPol);

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
    if (m_IsVideoProgressive->GetValue())
    {
        CreateSPIRiscCode();
    }
    else
    {
        CreateRiscCode(bCaptureVBI);
    }
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

        SetTimer(GetMainWnd(), TIMER_MSP, TIMER_MSP_MS, NULL);
    }
    
}

void CBT848Source::Reset()
{
    m_pBT848Card->ResetHardware(m_RiscBasePhysical);
    m_pBT848Card->SetVideoSource(m_VideoSource->GetValue());
    if (m_BDelay->GetValue() != 0)
    {
        // BDELAY override from .ini file
        m_pBT848Card->SetBDelay((BYTE)m_BDelay->GetValue());
    }
    else
    {
        m_pBT848Card->SetBDelay(GetTVFormat(GetFormat())->bDelayB);  
    }


    m_pBT848Card->SetContrastBrightness((WORD)m_Contrast->GetValue(), (WORD)m_Brightness->GetValue());
    m_pBT848Card->SetHue((BYTE)m_Hue->GetValue());
    m_pBT848Card->SetSaturationU((WORD)m_SaturationU->GetValue());
    m_pBT848Card->SetSaturationV((WORD)m_SaturationV->GetValue());
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
        m_pBT848Card->SetWhiteCrushUp((BYTE)m_BtWhiteCrushUp->GetValue());
        m_pBT848Card->SetWhiteCrushDown((BYTE)m_BtWhiteCrushDown->GetValue());
    }
    else
    {
        // set up the PMS gains for use in component modes
        m_pBT848Card->SetPMSChannelGain(1, (WORD)m_PMSGain1->GetValue());
        m_pBT848Card->SetPMSChannelGain(2, (WORD)m_PMSGain2->GetValue());
        m_pBT848Card->SetPMSChannelGain(3, (WORD)m_PMSGain3->GetValue());
        m_pBT848Card->SetPMSChannelGain(4, (WORD)m_PMSGain4->GetValue());
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

    // may have changed from interlaced to progressive or vice-versa
    if (m_pBT848Card->IsSPISource(m_VideoSource->GetValue()))
    {
        m_IsVideoProgressive->SetValue(TRUE);
    }
    else
    {
        m_IsVideoProgressive->SetValue(FALSE);
    }

    if((m_CardType->GetValue() == TVCARD_CWCEC_ATLAS) &&
        m_pBT848Card->IsSPISource(m_VideoSource->GetValue()))
    {
        m_pBT848Card->GetAD9882()->Wakeup();
        m_pBT848Card->GetAD9882()->SetPLL((WORD)m_AD9882PLL->GetValue());
        m_pBT848Card->GetAD9882()->SetVCO((BYTE)m_AD9882VCO->GetValue());
        m_pBT848Card->GetAD9882()->SetPump((BYTE)m_AD9882Pump->GetValue());
        m_pBT848Card->GetAD9882()->SetPhase((BYTE)m_AD9882Phase->GetValue());
        m_pBT848Card->GetAD9882()->SetPreCoast((BYTE)m_AD9882PreCoast->GetValue());
        m_pBT848Card->GetAD9882()->SetPostCoast((BYTE)m_AD9882PostCoast->GetValue());
        m_pBT848Card->GetAD9882()->SetHSync((BYTE)m_AD9882HSync->GetValue());
        m_pBT848Card->GetAD9882()->SetSyncSep((BYTE)m_AD9882SyncSep->GetValue());
        m_pBT848Card->GetAD9882()->SetSOGThresh((BYTE)m_AD9882SOGThresh->GetValue());
        m_pBT848Card->GetAD9882()->SetSOG((BOOLEAN)m_AD9882SOG->GetValue());
        m_pBT848Card->GetAD9882()->SetCoastSel((BOOLEAN)m_AD9882CoastSel->GetValue());
        m_pBT848Card->GetAD9882()->SetCoastOvr((BOOLEAN)m_AD9882CoastOvr->GetValue());
        m_pBT848Card->GetAD9882()->SetCoastPol((BOOLEAN)m_AD9882CoastPol->GetValue());
    }

    for (int i(0); i < 5; i++)
    {
        if (m_IsVideoProgressive->GetValue())
        {
            m_OddFields[i].pData = m_pDisplay[i];
            m_OddFields[i].Flags = PICTURE_PROGRESSIVE;
        }
        else
        {
            m_OddFields[i].pData = m_pDisplay[i] + 2048;
            m_OddFields[i].Flags = PICTURE_INTERLACED_ODD;
        }
        m_OddFields[i].IsFirstInSeries = FALSE;
    }

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

    m_NumFields = 10;
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


void CBT848Source::CreateSPIRiscCode()
{
    DWORD* pRiscCode;
    WORD nField;
    WORD nLine;
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytesPerLine;
    DWORD BytesPerLine = 0;
    DWORD HorizOffs = m_HorizOffset->GetValue();
    DWORD VertOffs = m_VertOffset->GetValue();

    m_NumFields = 5;  // really, number of frames
    pRiscCode = (DWORD*)m_RiscBaseLinear;
    // we create the RISC code for 5 non-interlaced odd fields
    // Note: no VBI support
    for(nField = 0; nField < m_NumFields; nField++)
    {
        // sync with packed FIFO data
        *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_FM1);
        *(pRiscCode++) = 0;

        BytesPerLine = m_CurrentX * 2;
        // skip unused lines (required for AD9882 capture)
        for(nLine = 0; nLine < VertOffs; nLine++)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SKIP | BT848_RISC_SOL | BT848_RISC_EOL | (BytesPerLine + HorizOffs));
        }

        // work out the position of the first line
        pUser = m_pDisplay[nField];
        for(nLine = 0; nLine < m_CurrentY; nLine++)
        {

            pPhysical = m_DisplayDMAMem[nField]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
            {
                return;
            }
            if(HorizOffs)
            {
                // skip unused pixels (required for AD9882 capture)
                *(pRiscCode++) = BT848_RISC_SKIP | BT848_RISC_SOL | HorizOffs;
                *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_EOL | BytesPerLine;
            }
            else
            {
                *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
            }
            *(pRiscCode++) = pPhysical;
            // non-interlaced, so don't skip lines
            pUser += 2048;
        }
        // sync to even field
        *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
        *(pRiscCode++) = 0;
    }

    m_BytesPerRISCField = ((long)pRiscCode - (long)m_RiscBaseLinear) / 5;
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
        KillTimer(GetMainWnd(), TIMER_MSP);
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

    ShiftPictureHistory(pInfo, m_NumFields);

    if(m_IsVideoProgressive->GetValue())
    {
        pInfo->PictureHistory[0] = &m_OddFields[pInfo->CurrentFrame];
        pInfo->LineLength = m_CurrentX * 2;
        pInfo->FrameWidth = m_CurrentX;
        pInfo->FrameHeight = m_CurrentY;
        pInfo->FieldHeight = m_CurrentY;
        pInfo->InputPitch = 2048;
    }
    else
    {
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
    }

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

SmartPtr<CBT848Card> CBT848Source::GetBT848Card()
{
    return m_pBT848Card;
}

string CBT848Source::GetStatus()
{
    string pRetVal;

    if (IsInTunerMode())
    {
        pRetVal = Channel_GetVBIName();

        if (pRetVal.empty())
        {
            pRetVal = Channel_GetName();
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
    int CurrentPos = m_NumFields;
    while(CurrentPos > (m_NumFields - 1))
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
    PostMessageToMainWindow(WM_BT848_SETVALUE, TVFORMAT, NewFormat);
}


CSliderSetting* CBT848Source::GetBrightness()
{
    return m_Brightness;
}

CSliderSetting* CBT848Source::GetContrast()
{
    return m_Contrast;
}

CSliderSetting* CBT848Source::GetHue()
{
    return m_Hue;
}

CSliderSetting* CBT848Source::GetSaturation()
{
    return m_Saturation;
}

CSliderSetting* CBT848Source::GetSaturationU()
{
    return m_SaturationU;
}

CSliderSetting* CBT848Source::GetSaturationV()
{
    return m_SaturationV;
}

CSliderSetting* CBT848Source::GetTopOverscan()
{
    return m_TopOverscan;
}

CSliderSetting* CBT848Source::GetBottomOverscan()
{
    return m_BottomOverscan;
}

CSliderSetting* CBT848Source::GetLeftOverscan()
{
    return m_LeftOverscan;
}

CSliderSetting* CBT848Source::GetRightOverscan()
{
    return m_RightOverscan;
}

CSliderSetting* CBT848Source::GetHDelay()
{
    return m_HDelay;
}

CSliderSetting* CBT848Source::GetVDelay()
{
    return m_VDelay;
}

void CBT848Source::BtAgcDisableOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetAgcDisable(NewValue);
}

void CBT848Source::BtCrushOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetCrush(NewValue);
}

void CBT848Source::BtEvenChromaAGCOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetEvenChromaAGC(NewValue);
}

void CBT848Source::BtOddChromaAGCOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetOddChromaAGC(NewValue);
}

void CBT848Source::BtEvenLumaPeakOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetEvenLumaPeak(NewValue);
}

void CBT848Source::BtOddLumaPeakOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetOddLumaPeak(NewValue);
}

void CBT848Source::BtFullLumaRangeOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetFullLumaRange(NewValue);
}

void CBT848Source::BtEvenLumaDecOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetEvenLumaDec(NewValue);
}

void CBT848Source::BtOddLumaDecOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetOddLumaDec(NewValue);
}

void CBT848Source::BtEvenCombOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetEvenComb(NewValue);
}

void CBT848Source::BtOddCombOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetOddComb(NewValue);
}

void CBT848Source::BtColorBarsOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetColorBars(NewValue);
}

void CBT848Source::BtGammaCorrectionOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetGammaCorrection(NewValue);
}

void CBT848Source::BtVertFilterOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetVertFilter(NewValue);
}

void CBT848Source::BtHorFilterOnChange(BOOL NewValue, BOOL OldValue)
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
    int OldPos;
    DWORD StartOfWait = GetTickCount();
    
    if(m_IsVideoProgressive->GetValue())
    {
        OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;
    }
    else
    {
        OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % m_NumFields;
    }

    while(OldPos == (NewPos = GetRISCPosAsInt()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;       // if we waited then we are not late
        bLate = FALSE;                     // if we waited then we are not late

        //check we're not in a tight loop here for too long
        //sometimes boards with external chips seem to hang and need
        //resetting, for other boards this won't do any harm (hopefully)
        if(GetTickCount() > StartOfWait + 200)
        {
            PostMessageToMainWindow(WM_COMMAND, IDM_RESET, 0);
            //  after tell the card to reset just exit out and we will probably show garbage here
            break;      
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

    FieldDistance = (m_NumFields + NewPos - OldPos) % m_NumFields;
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
        NewPos = (OldPos + 1) % m_NumFields;
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

    if(m_IsVideoProgressive->GetValue())
    {
        pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
    }
    else
    {
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
}

void CBT848Source::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    BOOL bLate = TRUE;
    int NewPos;
    int FieldDistance;
    int OldPos;
    static int FieldCount(-1);
    int Counter(0);
    DWORD StartOfWait = GetTickCount();
    
    if(m_IsVideoProgressive->GetValue())
    {
        OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;
    }
    else
    {
        OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % m_NumFields;
    }

    while(OldPos == (NewPos = GetRISCPosAsInt()))
    {
        pInfo->bRunningLate = FALSE;    // if we waited then we are not late
        bLate = FALSE;                  // if we waited then we are not late
        if(++Counter == 1000)
        {
            //check we're not in a tight loop here for too long
            //sometimes boards with external chips seem to hang and need
            //resetting, for other boards this won't do any harm (hopefully)
            if(GetTickCount() >  StartOfWait + 200)
            {
                PostMessageToMainWindow(WM_COMMAND, IDM_RESET, 0);
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

    FieldDistance = (m_NumFields + NewPos - OldPos) % m_NumFields;
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
        NewPos = (OldPos + 1) % m_NumFields;
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

    if(m_IsVideoProgressive->GetValue())
    {
        pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
    }
    else
    {
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
        NewValue != 320 &&
        !(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS &&
        (NewValue == 800 ||
        NewValue == 1024)))
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
    m_pBT848Card->SetContrastBrightness((WORD)m_Contrast->GetValue(), (WORD)Brightness);
}

void CBT848Source::BtWhiteCrushUpOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushUp((BYTE)NewValue);
}

void CBT848Source::BtWhiteCrushDownOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushDown((BYTE)NewValue);
}

void CBT848Source::BtCoringOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetCoring(NewValue);
}

void CBT848Source::BtColorKillOnChange(BOOL NewValue, BOOL OldValue)
{
    m_pBT848Card->SetColorKill(NewValue);
}

void CBT848Source::BDelayOnChange(long NewValue, long OldValue)
{
    // zero means use format's default value
    if(NewValue != 0)
    {
        m_pBT848Card->SetBDelay((BYTE)NewValue);  
    }
    else
    {
        m_pBT848Card->SetBDelay(GetTVFormat(GetFormat())->bDelayB);  
    }
}

void CBT848Source::HueOnChange(long Hue, long OldValue)
{
    m_pBT848Card->SetHue((BYTE)Hue);
}

void CBT848Source::ContrastOnChange(long Contrast, long OldValue)
{
    m_pBT848Card->SetContrastBrightness((WORD)Contrast, (WORD)m_Brightness->GetValue());
}

void CBT848Source::SaturationUOnChange(long SatU, long OldValue)
{
    m_pBT848Card->SetSaturationU((WORD)SatU);
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
    m_pBT848Card->SetSaturationV((WORD)SatV);
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
        m_pBT848Card->SetPMSChannelGain(1, (WORD)Gain);
    }
}

void CBT848Source::PMSGain2OnChange(long Gain, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_PMSDELUXE || m_CardType->GetValue() == TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetPMSChannelGain(2, (WORD)Gain);
    }
}

void CBT848Source::PMSGain3OnChange(long Gain, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_PMSDELUXE || m_CardType->GetValue() == TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetPMSChannelGain(3, (WORD)Gain);
    }
}

void CBT848Source::PMSGain4OnChange(long Gain, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_PMSDELUXE || m_CardType->GetValue() == TVCARD_SWEETSPOT)
    {
        m_pBT848Card->SetPMSChannelGain(4, (WORD)Gain);
    }
}

void CBT848Source::HorizOffsetOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        // since this setting affects the DMA program, we must do a stop/start
        Stop_Capture();
        Start_Capture();
    }
}

void CBT848Source::VertOffsetOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        // since this setting affects the DMA program, we must do a stop/start
        Stop_Capture();
        Start_Capture();
    }
}

void CBT848Source::IsVideoProgressiveOnChange(BOOL NewValue, BOOL OldValue)
{
    // since this setting affects the DMA program, we must do a stop/start
    Stop_Capture();
    Start_Capture();
}

void CBT848Source::AD9882PLLOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetPLL((WORD)NewValue);
    }
}

void CBT848Source::AD9882VCOOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetVCO((BYTE)NewValue);
    }
}

void CBT848Source::AD9882PumpOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetPump((BYTE)NewValue);
    }
}

void CBT848Source::AD9882PhaseOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetPhase((BYTE)NewValue);
    }
}

void CBT848Source::AD9882PreCoastOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetPreCoast((BYTE)NewValue);
    }
}

void CBT848Source::AD9882PostCoastOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetPostCoast((BYTE)NewValue);
    }
}

void CBT848Source::AD9882HSyncOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetHSync((BYTE)NewValue);
    }
}

void CBT848Source::AD9882SyncSepOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetSyncSep((BYTE)NewValue);
    }
}

void CBT848Source::AD9882SOGThreshOnChange(long NewValue, long OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetSOGThresh((BYTE)NewValue);
    }
}

void CBT848Source::AD9882SOGOnChange(BOOL NewValue, BOOL OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetSOG((BOOLEAN)NewValue);
    }
}

void CBT848Source::AD9882CoastSelOnChange(BOOL NewValue, BOOL OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetCoastSel((BOOLEAN)NewValue);
    }
}

void CBT848Source::AD9882CoastOvrOnChange(BOOL NewValue, BOOL OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetCoastOvr((BOOLEAN)NewValue);
    }
}

void CBT848Source::AD9882CoastPolOnChange(BOOL NewValue, BOOL OldValue)
{
    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        m_pBT848Card->GetAD9882()->SetCoastPol((BOOLEAN)NewValue);
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
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), GetMainWnd(), (DLGPROC) SelectCardProc, (LPARAM)this);
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
        if(m_pBT848Card->GetTuner())
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
    /*
    //Doesn't work yet
    // T.S.: that's ok because DScaler is no radio app
    if(VideoFormat == (VIDEOFORMAT_LASTONE+1))
    {
        return m_pBT848Card->GetTuner()->SetRadioFrequency(FrequencyId);
    }
    */

    if(VideoFormat == VIDEOFORMAT_LAST_TV)
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

string CBT848Source::GetMenuLabel()
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

string CBT848Source::GetChipName()
{
    return m_ChipName;
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

string CBT848Source::GetInputName(eSourceInputType InputType, int Nr)
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
    return "";
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


SmartPtr<ITuner> CBT848Source::GetTuner() 
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

    if(m_CardType->GetValue() == TVCARD_CWCEC_ATLAS)
    {
        vSettingsList.push_back(m_HorizOffset);
        vSettingsList.push_back(m_VertOffset);
        vSettingsList.push_back(m_AD9882PLL);
        vSettingsList.push_back(m_AD9882VCO);
        vSettingsList.push_back(m_AD9882Pump);
        vSettingsList.push_back(m_AD9882Phase);
        vSettingsList.push_back(m_AD9882PreCoast);
        vSettingsList.push_back(m_AD9882PostCoast);
        vSettingsList.push_back(m_AD9882HSync);
        vSettingsList.push_back(m_AD9882SyncSep);
        vSettingsList.push_back(m_AD9882SOGThresh);
        vSettingsList.push_back(m_AD9882SOG);
        vSettingsList.push_back(m_AD9882CoastSel);
        vSettingsList.push_back(m_AD9882CoastOvr);
        vSettingsList.push_back(m_AD9882CoastPol);
    }

    return new CTreeSettingsGeneric("BT8x8 Advanced",vSettingsList);
}

#endif // WANT_BT8X8_SUPPORT