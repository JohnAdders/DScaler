/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource_Audio.cpp,v 1.8 2004-06-02 18:44:07 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2004/04/19 17:33:30  to_see
// Added BTSCSAP and FM Audio
//
// Revision 1.6  2004/03/07 12:20:12  to_see
// added 2 Cards
// working Nicam-Sound
// Submenus in CX-Card for Soundsettings
// Click in "Autodetect" in "Setup card CX2388x" is now working
// added "Automute if no Tunersignal" in CX2388x Advanced
//
// Revision 1.5  2004/02/29 19:41:45  to_see
// new Submenu's in CX Card for Audio Channel and Audio Standard
// new AutoMute entry
//
// Revision 1.4  2004/02/27 20:51:00  to_see
// -more logging in CCX2388xCard::StartStopConexxantDriver
// -handling for IDC_AUTODETECT in CX2388xSource_UI.cpp
// -renamed eAudioStandard to eCX2388xAudioStandard,
//  eStereoType to eCX2388xStereoType and moved from
//  cx2388xcard.h to cx2388x_defines.h
// -moved Audiodetecting from CX2388xCard_Audio.cpp
//  to CX2388xSource_Audio.cpp
// -CCX2388xCard::AutoDetectTuner read
//  at first from Registers
//
// Revision 1.3  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.2  2002/12/10 14:53:16  adcockj
// Sound fixes for cx2388x
//
// Revision 1.1  2002/10/31 15:55:50  adcockj
// Moved audio code from Connexant dTV version
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xSource.cpp CCX2388xSource Implementation (Audio)
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"
#include "status.h"
#include "Audio.h"

void CCX2388xSource::Mute()
{
    m_pCard->SetAudioMute();
}

void CCX2388xSource::UnMute()
{
    m_pCard->SetAudioUnMute(m_Volume->GetValue());
}

void CCX2388xSource::VolumeOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioVolume(NewValue);    
	EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
}

void CCX2388xSource::BalanceOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioBalance(NewValue);
}

void CCX2388xSource::AudioStandardOnChange(long NewValue, long OldValue)
{
	StopUpdateAudioStatus();
    m_pCard->AudioInit(
                        m_VideoSource->GetValue(), 
                        (eVideoFormat)m_VideoFormat->GetValue(), 
                        (eCX2388xAudioStandard)NewValue,
                        (eCX2388xStereoType)m_StereoType->GetValue()
                      );
	
	StartUpdateAudioStatus();
}

void CCX2388xSource::StereoTypeOnChange(long NewValue, long OldValue)
{
	StopUpdateAudioStatus();
	m_pCard->AudioInit(
                        m_VideoSource->GetValue(), 
                        (eVideoFormat)m_VideoFormat->GetValue(), 
                        (eCX2388xAudioStandard)m_AudioStandard->GetValue(),
                        (eCX2388xStereoType)NewValue
                      );
    
	StartUpdateAudioStatus();
}

void CCX2388xSource::StartUpdateAudioStatus()
{
	m_AutoDetectA2StereoCounter		= 0;
	m_AutoDetectA2BilingualCounter	= 0;
	SetTimer(hWnd, TIMER_CX2388X, TIMER_CX2388X_MS, NULL);
}

void CCX2388xSource::StopUpdateAudioStatus()
{
	KillTimer(hWnd, TIMER_CX2388X);
}

// called every 250ms
void CCX2388xSource::UpdateAudioStatus()
{
	eSoundChannel SoundChannel = SOUNDCHANNEL_MONO;

	if(IsInTunerMode())
	{
		if(IsVideoPresent())
		{
			if(TRUE == (BOOL)m_AutoMute->GetValue())
			{
				if(Audio_IsMute() && !Audio_GetUserMute())
				{
					Audio_Unmute();
				}
			}

			switch(m_pCard->GetCurrentAudioStandard())
			{
			case AUDIO_STANDARD_A2:
				switch(m_pCard->GetCurrentStereoType())
				{
				case STEREOTYPE_AUTO:
					SoundChannel = AutoDetectA2Sound();
					break;
				
				case STEREOTYPE_MONO:
					SoundChannel = SOUNDCHANNEL_MONO;
					break;

				case STEREOTYPE_STEREO:
					SoundChannel = SOUNDCHANNEL_STEREO;
					break;

				case STEREOTYPE_ALT1:
					SoundChannel = SOUNDCHANNEL_LANGUAGE1;
					break;

				case STEREOTYPE_ALT2:
					SoundChannel = SOUNDCHANNEL_LANGUAGE2;
					break;
				}
				
				break;

			case AUDIO_STANDARD_NICAM:
				switch(m_pCard->GetCurrentStereoType())
				{
				case STEREOTYPE_AUTO:
					SoundChannel = AutoDetectNicamSound();
					break;
				case STEREOTYPE_MONO:
					SoundChannel = SOUNDCHANNEL_MONO;
					break;

				case STEREOTYPE_STEREO:
					SoundChannel = SOUNDCHANNEL_STEREO;
					break;

				case STEREOTYPE_ALT1:
					SoundChannel = SOUNDCHANNEL_LANGUAGE1;
					break;

				case STEREOTYPE_ALT2:
					SoundChannel = SOUNDCHANNEL_LANGUAGE2;
					break;
				}

				break;

			case AUDIO_STANDARD_BTSC:
				switch(m_pCard->GetCurrentStereoType())
				{
				case STEREOTYPE_AUTO:
				case STEREOTYPE_STEREO:
					SoundChannel = SOUNDCHANNEL_STEREO;
					break;
				
				case STEREOTYPE_MONO:
				case STEREOTYPE_ALT1:
				case STEREOTYPE_ALT2:
					SoundChannel = SOUNDCHANNEL_MONO;
					break;
				}

				break;

			case AUDIO_STANDARD_BTSC_SAP:
				SoundChannel = SOUNDCHANNEL_STEREO;
				break;

			case AUDIO_STANDARD_AUTO:
			case AUDIO_STANDARD_EIAJ:
			case AUDIO_STANDARD_FM:
				break;	// \todo: add more support
			}
		}

		// no Video present
		else
		{
			if(TRUE == (BOOL)m_AutoMute->GetValue())
			{
				if(!Audio_IsMute())
				{
					Audio_Mute();
				}
			}
		
			m_AutoDetectA2StereoCounter		= 0;
			m_AutoDetectA2BilingualCounter	= 0;
		}
	}

	// if not in tuner mode, show always Stereo
	else
	{
		SoundChannel = SOUNDCHANNEL_STEREO;
	}
	
	char szSoundChannel[256] = "";

    switch(SoundChannel)
    {
    case SOUNDCHANNEL_MONO:
		strcpy(szSoundChannel,"Mono");
		break;

    case SOUNDCHANNEL_STEREO:
		strcpy(szSoundChannel,"Stereo");
		break;

    case SOUNDCHANNEL_LANGUAGE1:
		strcpy(szSoundChannel,"Language 1");
		break;

    case SOUNDCHANNEL_LANGUAGE2:
		strcpy(szSoundChannel,"Language 2");
		break;
    }

    StatusBar_ShowText(STATUS_AUDIO, szSoundChannel);
    EventCollector->RaiseEvent(this, EVENT_SOUNDCHANNEL, -1, SoundChannel);
}

eSoundChannel CCX2388xSource::AutoDetectA2Sound()
{
	if(m_pCard->GetCurrentStereoType() == SOUNDCHANNEL_MONO)
	{
		return SOUNDCHANNEL_MONO;
	}
	
	eSoundChannel SoundChannelA2 = SOUNDCHANNEL_MONO;
	DWORD dwVal = m_pCard->GetAudioStatusRegister();

	switch(dwVal & 0x03)
	{
	case 0: // Stereo or Bilingual detected
		if((dwVal & 0x08) == 0x08)
		{
			// Test if C2 detected for bilingual
			if(m_AutoDetectA2BilingualCounter < 10) m_AutoDetectA2BilingualCounter++;
		}

		else
		{
			if(m_AutoDetectA2StereoCounter    < 10) m_AutoDetectA2StereoCounter++;
			if(m_AutoDetectA2BilingualCounter >  0) m_AutoDetectA2BilingualCounter--;
		}

		break;

	case 2:  // Mono detected
		if(m_AutoDetectA2StereoCounter    > 0) m_AutoDetectA2StereoCounter--;
		if(m_AutoDetectA2BilingualCounter > 0) m_AutoDetectA2BilingualCounter--;
		break;

	case 1:	// \todo: detect Dual Mono
	case 3:	// \todo: detect SAP
		break;
	}

	if(m_AutoDetectA2BilingualCounter > 5)
	{
		m_pCard->SetAutoA2StereoToStereo();
		SoundChannelA2 = SOUNDCHANNEL_LANGUAGE1;
	}

	else
	{
		if(m_AutoDetectA2StereoCounter > 5)
		{
			m_pCard->SetAutoA2StereoToStereo();
			SoundChannelA2 = SOUNDCHANNEL_STEREO;
		}

		else
		{
			m_pCard->SetAutoA2StereoToMono();
			SoundChannelA2 = SOUNDCHANNEL_MONO;
		}
	}

	return SoundChannelA2;
}

eSoundChannel CCX2388xSource::AutoDetectNicamSound()
{
	eSoundChannel SoundChannelNicam = SOUNDCHANNEL_MONO;

	// \todo
	// when Nicam detetected,
	// AUD_NICAM_STATUS1 = 0xfff0 or 0xfff1
	// AUD_NICAM_STATUS2 = 0x02
	// at the moment not shure how it works
	
	return SoundChannelNicam;
}