/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source_Audio.cpp,v 1.10 2002-10-26 05:24:23 atnak Exp $
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

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Source.h"
#include "SAA7134_Defines.h"
#include "Status.h"

ISetting* CSAA7134Source::GetVolume()
{
    return NULL;
}

ISetting* CSAA7134Source::GetBalance()
{
    return NULL;
}

void CSAA7134Source::Mute()
{
    m_pSAA7134Card->SetAudioMute();
}

void CSAA7134Source::UnMute()
{
    m_pSAA7134Card->SetAudioUnMute(m_Volume->GetValue());
}

void CSAA7134Source::VolumeOnChange(long NewValue, long OldValue)
{
    EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
}

void CSAA7134Source::BalanceOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioBalance(NewValue);
}

void CSAA7134Source::BassOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioBass(NewValue);
}

void CSAA7134Source::TrebleOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioTreble(NewValue);
}

void CSAA7134Source::AudioStandardOnChange(long NewValue, long OldValue)
{
    m_CustomAudioStandard->SetValue(FALSE, ONCHANGE_NONE);
    m_pSAA7134Card->SetAudioStandard((eAudioStandard)NewValue);
}

void CSAA7134Source::CustomAudioStandardOnChange(long NewValue, long OldValue)
{
    if (NewValue)
    {
        m_pSAA7134Card->SetAudioCarrier1Freq(m_AudioStandardCarrier1->GetValue());
        m_pSAA7134Card->SetAudioCarrier2Freq(m_AudioStandardCarrier2->GetValue());
        m_pSAA7134Card->SetAudioCarrier1Mode((eAudioCarrierMode)m_AudioStandardCarrier1Mode->GetValue());
        m_pSAA7134Card->SetAudioCarrier2Mode((eAudioCarrierMode)m_AudioStandardCarrier2Mode->GetValue());
        m_pSAA7134Card->SetCh1FMDeemphasis((eAudioFMDeemphasis)m_AudioStandardCh1FMDeemph->GetValue());
        m_pSAA7134Card->SetCh2FMDeemphasis((eAudioFMDeemphasis)m_AudioStandardCh2FMDeemph->GetValue());
    }
}

void CSAA7134Source::AudioSampleRateOnChange(long NewValue, long OldValue)
{
    if (GetCurrentAudioSetting()->GetValue() != AUDIOINPUTSOURCE_DAC)
    {
        m_pSAA7134Card->SetAudioSampleRate((eAudioSampleRate)NewValue);
    }
}

void CSAA7134Source::AudioSource1OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 0)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputSource)NewValue);
    }
}

void CSAA7134Source::AudioSource2OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 1)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputSource)NewValue);
    }
}

void CSAA7134Source::AudioSource3OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 2)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputSource)NewValue);
    }
}

void CSAA7134Source::AudioSource4OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 3)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputSource)NewValue);
    }
}

void CSAA7134Source::AudioSource5OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 4)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputSource)NewValue);
    }
}

void CSAA7134Source::AudioSource6OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 5)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputSource)NewValue);
    }
}


void CSAA7134Source::AudioChannelOnChange(long AudioChannel, long OldValue)
{
    if (AudioChannel == AUDIOCHANNEL_MONO &&
        GetCurrentAudioSetting()->GetValue() == AUDIOINPUTSOURCE_DAC)
    {
        if (m_AutoStereoSelect->GetValue())
        {
            m_AutoStereoSelect->SetValue(FALSE, ONCHANGE_NONE);
        }
    }
    m_pSAA7134Card->SetAudioChannel((eAudioChannel)AudioChannel);
}


void CSAA7134Source::AutoStereoSelectOnChange(long NewValue, long OldValue)
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


void CSAA7134Source::UpdateAudioStatus()
{
    char Text[256];
    char szAudioStandard[128];

    szAudioStandard[0] = '\0';
/*
    if (m_AudioStandardInStatusBar->GetValue())
    {
        if (m_DetectingAudioStandard)
        {
            strcpy(szAudioStandard," [Detecting...]");
        }
        else
        {
            char *s = (char*)m_pBT848Card->GetAudioStandardName(m_AudioStandardManual->GetValue());
            if (s != NULL)
            {
                sprintf(szAudioStandard," [%s]",s);
            }
        }
    }
*/
    switch(m_pSAA7134Card->GetAudioChannel())
    {
    case AUDIOCHANNEL_MONO:
        {
            sprintf(Text,"Mono%s",szAudioStandard);
            break;
        }
    case AUDIOCHANNEL_STEREO:
        {
            sprintf(Text,"Stereo%s",szAudioStandard);
            break;
        }
    case AUDIOCHANNEL_LANGUAGE1:
        {
            sprintf(Text,"Language 1%s",szAudioStandard);
            break;
        }
    case AUDIOCHANNEL_LANGUAGE2:
        {
            sprintf(Text,"Language 2%s",szAudioStandard);
            break;
        }
    }

    StatusBar_ShowText(STATUS_AUDIO, Text);
}


ISetting* CSAA7134Source::GetCurrentAudioSetting()
{
    return GetAudioSetting(m_VideoSource->GetValue());
}

ISetting* CSAA7134Source::GetAudioSetting(int nVideoSource)
{
    switch(nVideoSource)
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