/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source_Audio.cpp,v 1.22 2002-09-16 14:37:36 kooiman Exp $
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
// Revision 1.21  2002/09/15 19:52:22  kooiman
// Adressed some NICAM AM issues.
//
// Revision 1.20  2002/09/15 15:57:27  kooiman
// Added Audio standard support.
//
// Revision 1.19  2002/09/12 21:52:32  ittarnavsky
// Removed references to HasMSP.  Few cosmetic name changes
//
// Revision 1.18  2002/09/07 20:54:50  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.17  2002/08/07 09:57:24  kooiman
// Modified for configurable per channel settings.
//
// Revision 1.16  2002/08/05 13:26:34  kooiman
// Added support for per channel settings including BT volume.
//
// Revision 1.15  2002/07/02 20:00:07  adcockj
// New setting for MSP input pin selection
//
// Revision 1.14  2002/04/07 10:37:53  adcockj
// Made audio source work per input
//
// Revision 1.13  2002/03/04 20:44:49  adcockj
// Reversed incorrect changed
//
// Revision 1.11  2002/02/03 18:11:03  adcockj
// Fixed volume key
//
// Revision 1.10  2002/02/01 04:43:55  ittarnavsky
// some more audio related fixes
// removed the handletimermessages and getaudioname methods
// which break the separation of concerns oo principle
//
// Revision 1.9  2002/01/23 22:57:29  robmuller
// Revision D/G improvements. The code is following the documentation much closer now.
//
// Revision 1.8  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.7  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.6  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.5  2001/11/29 14:04:06  adcockj
// Added Javadoc comments
//
// Revision 1.4  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.3  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Source.h"
#include "BT848_Defines.h"
#include "Status.h"

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
    m_pBT848Card->SetAudioUnMute(m_Volume->GetValue(), (eAudioInput)GetCurrentAudioSetting()->GetValue());
}

void CBT848Source::VolumeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioVolume(NewValue);    
}

void CBT848Source::BalanceOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioBalance(NewValue);
}

void CBT848Source::BassOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioBass(NewValue);
}

void CBT848Source::TrebleOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioTreble(NewValue);
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
    m_pBT848Card->SetAudioChannel((eSoundChannel)NewValue); // FIXME, (m_UseInputPin1->GetValue() != 0));    
}

void CBT848Source::AutoStereoSelectOnChange(long NewValue, long OldValue)
{
    if (NewValue)
    {
        //Detect stereo
        m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, this, StaticAudioStandardDetected);
    }
}

void CBT848Source::UseInputPin1OnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioChannel((eSoundChannel)m_AudioChannel->GetValue()); // FIXME, (NewValue != 0));    
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

			m_pBT848Card->SetAudioEqualizerLevel(0, m_EqualizerBand1->GetValue());
			m_pBT848Card->SetAudioEqualizerLevel(1, m_EqualizerBand2->GetValue());
			m_pBT848Card->SetAudioEqualizerLevel(2, m_EqualizerBand3->GetValue());
			m_pBT848Card->SetAudioEqualizerLevel(3, m_EqualizerBand4->GetValue());
			m_pBT848Card->SetAudioEqualizerLevel(4, m_EqualizerBand5->GetValue());
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
			m_pBT848Card->SetAudioBass(m_Bass->GetValue());
			m_pBT848Card->SetAudioTreble(m_Treble->GetValue());
		}
	}
}

void CBT848Source::EqualizerBand1OnChange(long NewValue, long OldValue)
{
	if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
	{
		m_pBT848Card->SetAudioEqualizerLevel(0, NewValue);
	}
}

void CBT848Source::EqualizerBand2OnChange(long NewValue, long OldValue)
{
	if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
	{
		m_pBT848Card->SetAudioEqualizerLevel(1, NewValue);
	}
}
void CBT848Source::EqualizerBand3OnChange(long NewValue, long OldValue)
{
	if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
	{
		m_pBT848Card->SetAudioEqualizerLevel(2, NewValue);
	}
}
void CBT848Source::EqualizerBand4OnChange(long NewValue, long OldValue)
{
	if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
	{
		m_pBT848Card->SetAudioEqualizerLevel(3, NewValue);
	}
}
void CBT848Source::EqualizerBand5OnChange(long NewValue, long OldValue)
{
	if(m_UseEqualizer->GetValue() && m_pBT848Card->HasAudioEqualizers())
	{
		m_pBT848Card->SetAudioEqualizerLevel(4, NewValue);
	}
}

void CBT848Source::AudioLoudnessOnChange(long NewValue, long OldValue)
{
	m_pBT848Card->SetAudioLoudness(m_AudioLoudness->GetValue());
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
    if (m_DetectingAudioStandard) 
    {
        return;
    }   
    switch (NewValue)
    {
    case 0: //Find standard based on video format
    case 1: //Standard from video format with fallback on auto detect
        {
            long Standard = m_pBT848Card->GetAudioStandardFromVideoFormat((eVideoFormat)m_VideoFormat->GetValue());
            if ((NewValue == 0) || (Standard != 0))
            {
                m_pBT848Card->SetAudioStandard(Standard, (eVideoFormat)m_VideoFormat->GetValue());
                m_pBT848Card->SetAudioVolume(m_Volume->GetValue());

                m_AudioStandardManual->SetValue(Standard, TRUE);
                m_AudioStandardMajorCarrier->SetValue(m_pBT848Card->GetAudioStandardMajorCarrier(-1), TRUE);
                m_AudioStandardMinorCarrier->SetValue(m_pBT848Card->GetAudioStandardMinorCarrier(-1), TRUE);

                if (m_AutoStereoSelect->GetValue())
                {
                    // Find stereo modes
                    m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, this, StaticAudioStandardDetected);         
                }
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
            m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 1, this, StaticAudioStandardDetected); 
        }
        break;
    case 4: //Manual
        {        
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
            m_pBT848Card->SetAudioVolume(m_Volume->GetValue());
            // Find stereo modes
            if (m_AutoStereoSelect->GetValue())
            {
                m_pBT848Card->DetectAudioStandard(m_AudioStandardDetectInterval->GetValue(), 2, this, StaticAudioStandardDetected); 
            }
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
    }    
}

void CBT848Source::StaticAudioStandardDetected(void *pThis, int What, long Value)
{
    if (pThis != NULL)
    {        
        if (What == 1)
        {            
            ((CBT848Source*)pThis)->AudioStandardDetected(Value);
        } 
        else if (What==2)
        {
            ((CBT848Source*)pThis)->SupportedSoundChannelsDetected((eSupportedSoundChannels)Value);
        }
    }
}

void CBT848Source::AudioStandardDetected(long Standard)
{    
    m_AudioStandardManual->SetValue(Standard, TRUE);
    m_AudioStandardMajorCarrier->SetValue(m_pBT848Card->GetAudioStandardMajorCarrier(-1), TRUE);
    m_AudioStandardMinorCarrier->SetValue(m_pBT848Card->GetAudioStandardMinorCarrier(-1), TRUE);
    
    m_pBT848Card->SetAudioVolume(m_Volume->GetValue());
    m_DetectingAudioStandard = 0;
}

void CBT848Source::AudioStandardManualOnChange(long NewValue, long OldValue)
{
    if (m_AudioStandardDetect->GetValue() == 4)
    {
        m_pBT848Card->SetAudioStandard(NewValue, (eVideoFormat)m_VideoFormat->GetValue());
        m_pBT848Card->SetAudioVolume(m_Volume->GetValue());
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



void CBT848Source::HandleTimerMessages(int TimerId)
{
    /// \todo FIXME autodetec stero here
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

