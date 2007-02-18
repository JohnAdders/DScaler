/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source_Audio.cpp,v 1.21 2007-02-18 21:50:04 robmuller Exp $
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
// This software was based on BT848Source_Audio.cpp.  Those portions are
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
// Revision 1.20  2005/03/16 15:03:56  atnak
// Fixed problem with Audio Channel reverting to Stereo automatically on
// load with external lines.
//
// Revision 1.19  2004/08/06 16:23:02  atnak
// Added typecast to make some warnings go away.
//
// Revision 1.18  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.17  2003/06/01 19:42:32  atnak
// Adds "external" sound type for either stereo or mono
//
// Revision 1.16  2003/02/06 19:44:46  ittarnavsky
// changes due to the new SoundChannel.h
//
// Revision 1.15  2003/01/10 17:38:18  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.14  2002/12/09 00:32:13  atnak
// Added new muting stuff
//
// Revision 1.13  2002/10/31 05:39:02  atnak
// Added SoundChannel change event for toolbar
//
// Revision 1.12  2002/10/31 05:02:54  atnak
// Settings cleanup and audio tweaks
//
// Revision 1.11  2002/10/28 11:10:11  atnak
// Various changes and revamp to settings
//
// Revision 1.10  2002/10/26 05:24:23  atnak
// Minor cleanups
//
// Revision 1.9  2002/10/20 07:41:04  atnak
// custom audio standard setup + etc
//
// Revision 1.8  2002/10/16 11:37:58  atnak
// added saa7130 support
//
// Revision 1.7  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.6  2002/10/03 23:36:22  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.5  2002/09/28 13:33:04  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.4  2002/09/26 16:34:19  kooiman
// Lots of toolbar fixes &added EVENT_VOLUME support.
//
// Revision 1.3  2002/09/14 19:40:48  atnak
// various changes
//
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file SAA7134Source.cpp CSAA7134Source Implementation (Audio)
 */

#include "stdafx.h"

#ifdef WANT_SAA713X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Source.h"
#include "SAA7134_Defines.h"
#include "Status.h"
#include "SoundChannel.h"


void CSAA7134Source::Mute()
{
    m_pSAA7134Card->SetAudioMute();
}


void CSAA7134Source::UnMute()
{
    m_pSAA7134Card->SetAudioUnMute();
}


void CSAA7134Source::SetupAudioSource()
{
    m_pSAA7134Card->SetAudioSource((eAudioInputSource)m_AudioSource->GetValue());
    m_pSAA7134Card->SetAudioSampleRate((eAudioSampleRate)m_AudioSampleRate->GetValue());

    if (m_AutoStereoSelect->GetValue() && m_AudioSource->GetValue() == AUDIOINPUTSOURCE_DAC)
    {
        m_pSAA7134Card->SetAudioChannel(AUDIOCHANNEL_STEREO);
    }
    else
    {
        m_pSAA7134Card->SetAudioChannel((eAudioChannel)m_AudioChannel->GetValue());
    }

    // Experimental
    // m_pSAA7134Card->SetAudioVolume(m_Volume->GetValue());
    // m_pSAA7134Card->SetAudioBass(m_Bass->GetValue());
    // m_pSAA7134Card->SetAudioTreble(m_Treble->GetValue());
    // m_pSAA7134Card->SetAudioBalance(m_Balance->GetValue());
}


void CSAA7134Source::SetupAudioStandard()
{
    if (m_CustomAudioStandard->GetValue())
    {
        m_pSAA7134Card->SetAudioCarrier1Freq(m_AudioMajorCarrier->GetValue());
        m_pSAA7134Card->SetAudioCarrier2Freq(m_AudioMinorCarrier->GetValue());
        m_pSAA7134Card->SetAudioCarrier1Mode((eAudioCarrierMode)m_AudioMajorCarrierMode->GetValue());
        m_pSAA7134Card->SetAudioCarrier2Mode((eAudioCarrierMode)m_AudioMinorCarrierMode->GetValue());
        m_pSAA7134Card->SetCh1FMDeemphasis((eAudioFMDeemphasis)m_AudioCh1FMDeemph->GetValue());
        m_pSAA7134Card->SetCh2FMDeemphasis((eAudioFMDeemphasis)m_AudioCh2FMDeemph->GetValue());
    }
    else
    {
        m_pSAA7134Card->SetAudioStandard((eAudioStandard)m_AudioStandard->GetValue());
    }
}


void CSAA7134Source::UpdateAudioStatus()
{
    eAudioChannel AudioChannel = m_pSAA7134Card->GetAudioChannel();

    if (AudioChannel != m_DetectedAudioChannel)
    {
        char szAudioChannel[256] = "";

        eSoundChannel BtSoundChannel;

        switch(AudioChannel)
        {
        case AUDIOCHANNEL_MONO:
            {
                BtSoundChannel = SOUNDCHANNEL_MONO;
                sprintf(szAudioChannel,"Mono");
                break;
            }
        case AUDIOCHANNEL_STEREO:
            {
                BtSoundChannel = SOUNDCHANNEL_STEREO;
                sprintf(szAudioChannel,"Stereo");
                break;
            }
        case AUDIOCHANNEL_LANGUAGE1:
            {
                BtSoundChannel = SOUNDCHANNEL_LANGUAGE1;
                sprintf(szAudioChannel,"Language 1");
                break;
            }
        case AUDIOCHANNEL_LANGUAGE2:
            {
                BtSoundChannel = SOUNDCHANNEL_LANGUAGE2;
                sprintf(szAudioChannel,"Language 2");
                break;
            }
        case AUDIOCHANNEL_EXTERNAL:
            {
				// External means it could either be stereo
				// or mono but we don't know because it comes
				// from an external source.
                BtSoundChannel = SOUNDCHANNEL_STEREO;
                sprintf(szAudioChannel,"External");
                break;
            }
        }

        StatusBar_ShowText(STATUS_AUDIO, szAudioChannel);
        m_DetectedAudioChannel = AudioChannel;

        // This is needed for the stereo indicator in the toolbar
        EventCollector->RaiseEvent(this, EVENT_SOUNDCHANNEL, -1, BtSoundChannel);
    }
}


ISetting* CSAA7134Source::GetVolume()
{
    return NULL;
}


ISetting* CSAA7134Source::GetBalance()
{
    return NULL;
}


///////////////////////////////////////////////////////////////////////////

void CSAA7134Source::VolumeOnChange(long NewValue, long OldValue)
{
    EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
}

void CSAA7134Source::BalanceOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioBalance((WORD)NewValue);
}

void CSAA7134Source::BassOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioBass((WORD)NewValue);
}

void CSAA7134Source::TrebleOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioTreble((WORD)NewValue);
}

void CSAA7134Source::AudioStandardOnChange(long NewValue, long OldValue)
{
    if (m_CustomAudioStandard->GetValue() == FALSE)
    {
        m_pSAA7134Card->SetAudioStandard((eAudioStandard)NewValue);
    }
}

void CSAA7134Source::AudioSampleRateOnChange(long NewValue, long OldValue)
{
    if (m_AudioSource->GetValue() != AUDIOINPUTSOURCE_DAC)
    {
        m_pSAA7134Card->SetAudioSampleRate((eAudioSampleRate)NewValue);
    }
}

void CSAA7134Source::AudioSourceOnChange(long NewValue, long OldValue)
{
    EventCollector->RaiseEvent(this, EVENT_AUDIOINPUT_PRECHANGE, OldValue, NewValue);
    EventCollector->RaiseEvent(this, EVENT_AUDIOINPUT_CHANGE, OldValue, NewValue);
    SetupAudioSource();
}

void CSAA7134Source::AudioChannelOnChange(long AudioChannel, long OldValue)
{
    if (AudioChannel == AUDIOCHANNEL_MONO &&
		m_AudioSource->GetValue() == AUDIOINPUTSOURCE_DAC)
    {
        if (m_AutoStereoSelect->GetValue())
        {
            m_AutoStereoSelect->SetValue(FALSE, TRUE);
        }
    }
    m_pSAA7134Card->SetAudioChannel((eAudioChannel)AudioChannel);
}

void CSAA7134Source::AutoStereoSelectOnChange(long NewValue, long OldValue)
{
	if (m_AudioSource->GetValue() == AUDIOINPUTSOURCE_DAC)
	{
		if (NewValue)
		{
			m_pSAA7134Card->SetAudioChannel(AUDIOCHANNEL_STEREO);
		}
		else
		{
			if (m_AudioChannel->GetValue() == AUDIOCHANNEL_MONO)
			{
				m_pSAA7134Card->SetAudioChannel(AUDIOCHANNEL_MONO);
			}
		}
	}
}

void CSAA7134Source::CustomAudioStandardOnChange(long NewValue, long OldValue)
{
    SetupAudioStandard();
}

void CSAA7134Source::AudioMajorCarrierOnChange(long NewValue, long OldValue)
{
    if (m_CustomAudioStandard->GetValue() == TRUE)
    {
        m_pSAA7134Card->SetAudioCarrier1Freq(NewValue);
    }
}

void CSAA7134Source::AudioMinorCarrierOnChange(long NewValue, long OldValue)
{
    if (m_CustomAudioStandard->GetValue() == TRUE)
    {
        m_pSAA7134Card->SetAudioCarrier2Freq(NewValue);
    }
}

void CSAA7134Source::AudioMajorCarrierModeOnChange(long NewValue, long OldValue)
{
    if (m_CustomAudioStandard->GetValue() == TRUE)
    {
        m_pSAA7134Card->SetAudioCarrier1Mode((eAudioCarrierMode)NewValue);
    }
}

void CSAA7134Source::AudioMinorCarrierModeOnChange(long NewValue, long OldValue)
{
    if (m_CustomAudioStandard->GetValue() == TRUE)
    {
        m_pSAA7134Card->SetAudioCarrier2Mode((eAudioCarrierMode)NewValue);
    }
}

void CSAA7134Source::AudioCh1FMDeemphOnChange(long NewValue, long OldValue)
{
    if (m_CustomAudioStandard->GetValue() == TRUE)
    {
        m_pSAA7134Card->SetCh1FMDeemphasis((eAudioFMDeemphasis)NewValue);
    }
}

void CSAA7134Source::AudioCh2FMDeemphOnChange(long NewValue, long OldValue)
{
    if (m_CustomAudioStandard->GetValue() == TRUE)
    {
        m_pSAA7134Card->SetCh2FMDeemphasis((eAudioFMDeemphasis)NewValue);
    }
}

void CSAA7134Source::AutomaticVolumeLevelOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAutomaticVolume((eAutomaticVolume)NewValue);
}

#endif//xxx