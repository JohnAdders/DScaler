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
 * @file BT848Source.cpp CBT848Source Implementation (Audio)
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Source.h"
#include "BT848_Defines.h"
#include "Status.h"
#include "Providers.h"
#include "DScaler.h"

using namespace std;

ISetting* CBT848Source::GetVolume()
{
    return m_Volume;
}

ISetting* CBT848Source::GetBalance()
{
    return m_Balance;
}

void CBT848Source::Mute()
{
    m_pBT848Card->SetAudioMute();
}

void CBT848Source::UnMute()
{
    m_pBT848Card->SetAudioUnMute((WORD)m_Volume->GetValue(), (eAudioInput)GetCurrentAudioSetting()->GetValue());
}

void CBT848Source::VolumeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioVolume((WORD)NewValue);    
    EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
}

void CBT848Source::BalanceOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioBalance((WORD)NewValue);
}

void CBT848Source::BassOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioBass((WORD)NewValue);
}

void CBT848Source::TrebleOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioTreble((WORD)NewValue);
}

void CBT848Source::AudioSource1OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 0)
    {
        m_pBT848Card->SetAudioSource((eAudioInput)NewValue);
    }
}

void CBT848Source::AudioSource2OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 1)
    {
        m_pBT848Card->SetAudioSource((eAudioInput)NewValue);
    }
}

void CBT848Source::AudioSource3OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 2)
    {
        m_pBT848Card->SetAudioSource((eAudioInput)NewValue);
    }
}

void CBT848Source::AudioSource4OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 3)
    {
        m_pBT848Card->SetAudioSource((eAudioInput)NewValue);
    }
}

void CBT848Source::AudioSource5OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 4)
    {
        m_pBT848Card->SetAudioSource((eAudioInput)NewValue);
    }
}

void CBT848Source::AudioSource6OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 5)
    {
        m_pBT848Card->SetAudioSource((eAudioInput)NewValue);
    }
}


void CBT848Source::AudioChannelOnChange(long NewValue, long OldValue)
{
   m_pBT848Card->SetAudioChannel((eSoundChannel)NewValue);   
    EventCollector->RaiseEvent(this, EVENT_SOUNDCHANNEL, OldValue, NewValue);
}

void CBT848Source::AutoStereoSelectOnChange(long NewValue, long OldValue)
{
    if (NewValue)
    {
        //Detect stereo
        m_KeepDetectingStereo = 0;
        m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, SOUNDCHANNEL_STEREO);
    }
}

void CBT848Source::AutoStereoDetectIntervalOnChange(long NewValue, long OldValue)
{
}

void CBT848Source::UseInputPin1OnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetUseInputPin1(m_UseInputPin1->GetValue() != 0);
}

void CBT848Source::UseEqualizerOnChange(long NewValue, long OldValue)
{
    if(m_pBT848Card->HasAudioEqualizers())
    {
        if (NewValue)
        {            
            m_pBT848Card->SetAudioBass(0);
            m_pBT848Card->SetAudioTreble(0);

            m_pBT848Card->SetAudioEqualizerLevel(-1, TRUE); //Enable equalizer

            m_pBT848Card->SetAudioEqualizerLevel(0, (WORD)m_EqualizerBand1->GetValue());
            m_pBT848Card->SetAudioEqualizerLevel(1, (WORD)m_EqualizerBand2->GetValue());
            m_pBT848Card->SetAudioEqualizerLevel(2, (WORD)m_EqualizerBand3->GetValue());
            m_pBT848Card->SetAudioEqualizerLevel(3, (WORD)m_EqualizerBand4->GetValue());
            m_pBT848Card->SetAudioEqualizerLevel(4, (WORD)m_EqualizerBand5->GetValue());
        }
        else
        {
            // Set equalizer bands to 0.
            m_pBT848Card->SetAudioEqualizerLevel(0, 0);
            m_pBT848Card->SetAudioEqualizerLevel(1, 0);
            m_pBT848Card->SetAudioEqualizerLevel(2, 0);
            m_pBT848Card->SetAudioEqualizerLevel(3, 0);
            m_pBT848Card->SetAudioEqualizerLevel(4, 0);

            m_pBT848Card->SetAudioEqualizerLevel(-1, FALSE); //Disable equalizer

            // Set bass & treble back
            m_pBT848Card->SetAudioBass((WORD)m_Bass->GetValue());
            m_pBT848Card->SetAudioTreble((WORD)m_Treble->GetValue());
        }
    }
}

void CBT848Source::EqualizerBand1OnChange(long NewValue, long OldValue)
{
    if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
    {
        m_pBT848Card->SetAudioEqualizerLevel(0, (WORD)NewValue);
    }
}

void CBT848Source::EqualizerBand2OnChange(long NewValue, long OldValue)
{
    if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
    {
        m_pBT848Card->SetAudioEqualizerLevel(1, (WORD)NewValue);
    }
}
void CBT848Source::EqualizerBand3OnChange(long NewValue, long OldValue)
{
    if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
    {
        m_pBT848Card->SetAudioEqualizerLevel(2, (WORD)NewValue);
    }
}
void CBT848Source::EqualizerBand4OnChange(long NewValue, long OldValue)
{
    if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
    {
        m_pBT848Card->SetAudioEqualizerLevel(3, (WORD)NewValue);
    }
}
void CBT848Source::EqualizerBand5OnChange(long NewValue, long OldValue)
{
    if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
    {
        m_pBT848Card->SetAudioEqualizerLevel(4, (WORD)NewValue);
    }
}

void CBT848Source::AudioLoudnessOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioLoudness((WORD)m_AudioLoudness->GetValue());
}

void CBT848Source::AudioSuperbassOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioBassBoost(m_AudioSuperbass->GetValue() ? true : false );
}

void CBT848Source::AudioSpatialEffectOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioSpatialEffect(NewValue);
}

void CBT848Source::AudioAutoVolumeCorrectionOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioAutoVolumeCorrection(NewValue);
}

void CBT848Source::AudioStandardDetectOnChange(long NewValue, long OldValue)
{        
    switch (NewValue)
    {
    case 0: //Find standard based on video format
    case 1: //Standard from video format with fallback on auto detect
        {
            if (m_DetectingAudioStandard) 
            {
                //Abort detection
                m_KeepDetectingStereo = 0;
                m_pBT848Card->DetectAudioStandard(-1,0,(eSoundChannel)0);
                m_DetectingAudioStandard = 0;
            }
            long Standard = m_pBT848Card->GetAudioStandardFromVideoFormat((eVideoFormat)m_VideoFormat->GetValue());
            if ((NewValue == 0) || (Standard != 0))
            {
                m_pBT848Card->SetAudioStandard(Standard, (eVideoFormat)m_VideoFormat->GetValue());                

                m_AudioStandardManual->SetValue(Standard, TRUE);
                m_AudioStandardMajorCarrier->SetValue(m_pBT848Card->GetAudioStandardMajorCarrier(-1), TRUE);
                m_AudioStandardMinorCarrier->SetValue(m_pBT848Card->GetAudioStandardMinorCarrier(-1), TRUE);
              
                if (m_AutoStereoSelect->GetValue())
                {
                    // Find stereo modes
                    m_KeepDetectingStereo = 0;
                    m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, SOUNDCHANNEL_STEREO);
                }
                else
                {
                    m_pBT848Card->SetAudioChannel((eSoundChannel)m_AudioChannel->GetValue());                               
                }
                //Set controls                
                if (m_InitAudioControls)
                {
                    m_InitAudioControls = FALSE;
                    InitAudioControls();
                }
                m_pBT848Card->SetAudioVolume((WORD)m_Volume->GetValue());
                break;
            } 
            else
            {
                // Auto detect (no break)
            }
        }
    case 2: // Detect
    case 3:
        {
            m_DetectingAudioStandard = 1;
            m_KeepDetectingStereo = 0;
            m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 1,
                (m_AutoStereoSelect->GetValue())?SOUNDCHANNEL_STEREO : (eSoundChannel)(m_AudioChannel->GetValue()));
        }
        break;
    case 4: //Manual
        {        
            if (m_DetectingAudioStandard) 
            {
                //Abort detection
                m_KeepDetectingStereo = 0;
                m_pBT848Card->DetectAudioStandard(-1,0,(eSoundChannel)0);
                m_DetectingAudioStandard = 0;
            }
            m_pBT848Card->SetAudioStandard(m_AudioStandardManual->GetValue(), (eVideoFormat)m_VideoFormat->GetValue());            
            if ((m_AudioStandardMajorCarrier->GetValue() != 0) && (m_AudioStandardMinorCarrier->GetValue() != 0))
            {
                m_pBT848Card->SetAudioStandardCarriers(m_AudioStandardMajorCarrier->GetValue(),m_AudioStandardMinorCarrier->GetValue());    
            }
            else
            {
                m_AudioStandardMajorCarrier->SetValue(m_pBT848Card->GetAudioStandardMajorCarrier(-1), TRUE);
                m_AudioStandardMinorCarrier->SetValue(m_pBT848Card->GetAudioStandardMinorCarrier(-1), TRUE);
            }            
            // Find stereo modes
            if (m_AutoStereoSelect->GetValue())
            {
                m_KeepDetectingStereo = 0;
                m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, SOUNDCHANNEL_STEREO);
            }
            else
            {
                m_pBT848Card->SetAudioChannel((eSoundChannel)m_AudioChannel->GetValue());                
            }
            //Set controls            
            if (m_InitAudioControls)
            {
                m_InitAudioControls = FALSE;
                InitAudioControls();
            }
            m_pBT848Card->SetAudioVolume((WORD)m_Volume->GetValue());
        }
        break;
    }        
}

void CBT848Source::AudioStandardDetectIntervalOnChange(long NewValue, long OldValue)
{

}

void CBT848Source::SupportedSoundChannelsDetected(eSupportedSoundChannels supported)
{    
    if (m_AutoStereoSelect->GetValue())
    {
        if (m_pBT848Card->IsAudioChannelDetected((eSoundChannel)2) == 2)
        {
            m_AudioChannel->SetValue(2); //Set to stereo
        }        
        else
        {
            //Set value to compatible channel (probably mono)
            m_AudioChannel->SetValue(m_pBT848Card->IsAudioChannelDetected((eSoundChannel)m_AudioChannel->GetValue()), TRUE);
        }
        m_KeepDetectingStereo = 1;
    }    
}


void CBT848Source::AudioStandardDetected(long Standard)
{    
    m_AudioStandardManual->SetValue(Standard, TRUE);
    m_AudioStandardMajorCarrier->SetValue(m_pBT848Card->GetAudioStandardMajorCarrier(-1), TRUE);
    m_AudioStandardMinorCarrier->SetValue(m_pBT848Card->GetAudioStandardMinorCarrier(-1), TRUE);
        
    if (m_InitAudioControls)
    {
        m_InitAudioControls = FALSE;
        InitAudioControls();
    }
    m_pBT848Card->SetAudioVolume((WORD)m_Volume->GetValue());
    m_DetectingAudioStandard = 0;
}

void CBT848Source::AudioStandardManualOnChange(long NewValue, long OldValue)
{
    if (m_AudioStandardDetect->GetValue() == 4)
    {
        m_pBT848Card->SetAudioStandard(NewValue, (eVideoFormat)m_VideoFormat->GetValue());
        m_pBT848Card->SetAudioVolume((WORD)m_Volume->GetValue());
        m_AudioStandardMajorCarrier->SetValue(m_pBT848Card->GetAudioStandardMajorCarrier(NewValue), TRUE);
        m_AudioStandardMinorCarrier->SetValue(m_pBT848Card->GetAudioStandardMinorCarrier(NewValue), TRUE);
    }
}

void CBT848Source::AudioStandardMajorCarrierOnChange(long NewValue, long OldValue)
{
    if (m_AudioStandardDetect->GetValue() == 4)
    {
        m_pBT848Card->SetAudioStandardCarriers(m_AudioStandardMajorCarrier->GetValue(),m_AudioStandardMinorCarrier->GetValue());
    }
}

void CBT848Source::AudioStandardMinorCarrierOnChange(long NewValue, long OldValue)
{
    if (m_AudioStandardDetect->GetValue() == 4)
    {
        m_pBT848Card->SetAudioStandardCarriers(m_AudioStandardMajorCarrier->GetValue(),m_AudioStandardMinorCarrier->GetValue());
    }
}

void CBT848Source::AudioStandardInStatusBarOnChange(long NewValue, long OldValue)
{

}

void CBT848Source::MSP34xxFlagsOnChange(long NewValue, long OldValue)
{   
    // \todo: change this
    m_pBT848Card->SetAudioDecoderValue(0, NewValue); 
}

void CBT848Source::HandleTimerMessages(int TimerId)
{
    if (TimerId == TIMER_MSP)
    {        
        string Text;
        string szAudioStandard(".[");

        if (m_AutoStereoSelect->GetValue() && (m_AutoStereoDetectInterval->GetValue() > 0) && (m_KeepDetectingStereo > 0))
        {
            m_KeepDetectingStereo++;
            if ( ((m_KeepDetectingStereo-1) * TIMER_MSP_MS ) >= m_AutoStereoDetectInterval->GetValue())
            {
               //Waited at least 'AutoStereoDetectInterval' milliseconds, startup a new stereo detection
               // Accuracy is 1 second (TIMER_MSP_MS)
               m_KeepDetectingStereo = 0;
               m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, SOUNDCHANNEL_STEREO);m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, SOUNDCHANNEL_STEREO);
            }
        }         
        
        if (m_AudioStandardInStatusBar->GetValue())
        {
            if (m_DetectingAudioStandard)
            {
                szAudioStandard += "Detecting...]";
            }
            else 
            {
                char *s = (char*)m_pBT848Card->GetAudioStandardName(m_AudioStandardManual->GetValue());
                if (s != NULL)
                {           
                    szAudioStandard += s;
                    szAudioStandard += "]";;
                }
            }
        }
        
        switch(m_AudioChannel->GetValue())
        {
        case SOUNDCHANNEL_MONO:
            {
                Text = "Mono" + szAudioStandard;
                break;
            }
        case SOUNDCHANNEL_STEREO:
            {
                Text = "Stereo" + szAudioStandard;
                break;
            }
        case SOUNDCHANNEL_LANGUAGE1:
            {
                Text = "Language 1" + szAudioStandard;
                break;
            }
        case SOUNDCHANNEL_LANGUAGE2:
            {
                Text = "Language 2" + szAudioStandard;
                break;
            }
        }
        
        StatusBar_ShowText(STATUS_AUDIO, Text);
    }
}

ISetting* CBT848Source::GetCurrentAudioSetting()
{
    switch(m_VideoSource->GetValue())
    {
    case 0:
        return m_AudioSource1;
    case 1:
        return m_AudioSource2;
    case 2:
        return m_AudioSource3;
    case 3:
        return m_AudioSource4;
    case 4:
        return m_AudioSource5;
    default:
    case 5:
        return m_AudioSource6;
    }
}


void CBT848Source::InitAudio()
{
    m_pBT848Card->InitAudio((m_UseInputPin1->GetValue() != 0));

    m_InitAudioControls = TRUE; //done in audiostandarddetectonchange
    
    m_pBT848Card->SetAudioSource((eAudioInput)GetCurrentAudioSetting()->GetValue());
    m_AudioStandardDetect->SetValue(m_AudioStandardDetect->GetValue());
}

void CBT848Source::InitAudioControls()
{    
    m_Balance->SetValue(m_Balance->GetValue());

    m_UseEqualizer->SetValue(m_UseEqualizer->GetValue());
    if (!m_UseEqualizer->GetValue())
    {
        m_Bass->SetValue(m_Bass->GetValue());
        m_Treble->SetValue(m_Treble->GetValue());
    }
    else
    {
        m_EqualizerBand1->SetValue(m_EqualizerBand1->GetValue());
        m_EqualizerBand2->SetValue(m_EqualizerBand2->GetValue());
        m_EqualizerBand3->SetValue(m_EqualizerBand3->GetValue());
        m_EqualizerBand4->SetValue(m_EqualizerBand4->GetValue());
        m_EqualizerBand5->SetValue(m_EqualizerBand5->GetValue());
    }

    m_AudioLoudness->SetValue(m_AudioLoudness->GetValue());
    m_AudioSuperbass->SetValue(m_AudioSuperbass->GetValue());
    m_AudioSpatialEffect->SetValue(m_AudioSpatialEffect->GetValue());
    m_AudioAutoVolumeCorrection->SetValue(m_AudioAutoVolumeCorrection->GetValue()); 
}

#endif // WANT_BT8X8_SUPPORT