/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source_Audio.cpp,v 1.4 2001-11-25 01:58:34 ittarnavsky Exp $
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

CSetting* CBT848Source::GetVolume()
{
    return NULL;
}

CSetting* CBT848Source::GetBalance()
{
    return NULL;
}

void CBT848Source::Mute()
{
    m_pBT848Card->Mute();
}

void CBT848Source::UnMute()
{
    m_pBT848Card->UnMute(m_Volume->GetValue());
}

void CBT848Source::MSPModeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPMode(NewValue);
}

void CBT848Source::VolumeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPVolume(NewValue);
}

void CBT848Source::SpatialOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPSpatial(NewValue);
}

void CBT848Source::BalanceOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPBalance(NewValue);
}

void CBT848Source::BassOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPBass(NewValue);
}

void CBT848Source::TrebleOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPTreble(NewValue);
}

void CBT848Source::LoudnessOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPSuperBassLoudness(NewValue, m_SuperBass->GetValue());
}

void CBT848Source::SuperBassOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPSuperBassLoudness(m_Loudness->GetValue(), NewValue);
}

void CBT848Source::Equalizer1OnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPEqualizer(0, NewValue);
}

void CBT848Source::Equalizer2OnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPEqualizer(1, NewValue);
}

void CBT848Source::Equalizer3OnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPEqualizer(2, NewValue);
}

void CBT848Source::Equalizer4OnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPEqualizer(3, NewValue);
}

void CBT848Source::Equalizer5OnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPEqualizer(4, NewValue);
}

void CBT848Source::AudioSourceOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioSource((CBT848Card::eCardType)m_CardType->GetValue(), (CBT848Card::eAudioMuxType)NewValue);
}

void CBT848Source::MSPStereoOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPStereo((eSoundChannel)NewValue);
}

void CBT848Source::MSPMajorModeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPMajorMinorMode(NewValue,
                                        m_MSPMinorMode->GetValue()
                                      );
}

void CBT848Source::MSPMinorModeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetMSPMajorMinorMode(
                                        m_MSPMajorMode->GetValue(),
                                        NewValue
                                      );
}

void CBT848Source::AutoStereoSelectOnChange(long NewValue, long OldValue)
{
}

void CBT848Source::HandleTimerMessages(int TimerId)
{
    if(TimerId == TIMER_MSP && m_AutoStereoSelect->GetValue() == TRUE && m_pBT848Card->HasMSP())
    {
        eSoundChannel newChannel = m_pBT848Card->GetMSPWatchMode((eSoundChannel)m_MSPStereo->GetValue());
        if (newChannel != m_MSPStereo->GetValue())
        {
            m_MSPStereo->SetValue(newChannel);
            if (StatusBar_IsVisible() == TRUE)
            {
                char szText[128];
                m_pBT848Card->GetMSPPrintMode(szText);
                StatusBar_ShowText(STATUS_AUDIO, szText);
            }
        }
    }
}
