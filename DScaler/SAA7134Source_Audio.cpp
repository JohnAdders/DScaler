/////////////////////////////////////////////////////////////////////////////
// $id$
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
// $log$
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
    m_pSAA7134Card->SetAudioUnMute(m_Volume->GetValue(), (eAudioInputLine)GetCurrentAudioSetting()->GetValue());
}

void CSAA7134Source::VolumeOnChange(long NewValue, long OldValue)
{
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
    m_pSAA7134Card->SetAudioStandard((eAudioStandard)NewValue);
}

void CSAA7134Source::AudioSource1OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 0)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputLine)NewValue);
    }
}

void CSAA7134Source::AudioSource2OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 1)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputLine)NewValue);
    }
}

void CSAA7134Source::AudioSource3OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 2)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputLine)NewValue);
    }
}

void CSAA7134Source::AudioSource4OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 3)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputLine)NewValue);
    }
}

void CSAA7134Source::AudioSource5OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 4)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputLine)NewValue);
    }
}

void CSAA7134Source::AudioSource6OnChange(long NewValue, long OldValue)
{
    if(m_VideoSource->GetValue() == 5)
    {
        m_pSAA7134Card->SetAudioSource((eAudioInputLine)NewValue);
    }
}


void CSAA7134Source::AudioChannelOnChange(long NewValue, long OldValue)
{
    m_pSAA7134Card->SetAudioChannel((eSoundChannel)NewValue);    
}

void CSAA7134Source::AutoStereoSelectOnChange(long NewValue, long OldValue)
{
    /// \todo FIXME - check usability
}


void CSAA7134Source::HandleTimerMessages(int TimerId)
{
    /// \todo FIXME autodetec stero here - check usability
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