/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source_Audio.cpp,v 1.9 2002-01-23 22:57:29 robmuller Exp $
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
    return NULL;
}

ISetting* CBT848Source::GetBalance()
{
    return NULL;
}

void CBT848Source::Mute()
{
    m_pBT848Card->SetAudioMute();
}

void CBT848Source::UnMute()
{
    m_pBT848Card->SetAudioUnMute(m_Volume->GetValue());
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

void CBT848Source::AudioSourceOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioSource((eAudioInput)NewValue);
}

void CBT848Source::AudioChannelOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAudioChannel((eSoundChannel)NewValue);
}

void CBT848Source::AutoStereoSelectOnChange(long NewValue, long OldValue)
{
    /// \todo FIXME
}

void CBT848Source::HandleTimerMessages(int TimerId)
{
    m_pBT848Card->HandleTimerMessages(TimerId);

    if(TimerId == TIMER_MSP)
    {
        if (StatusBar_IsVisible() == TRUE)
        {
            char szText[128];
            m_pBT848Card->GetMSPPrintMode(szText);
            StatusBar_ShowText(STATUS_AUDIO, szText);
        }
    }

    /*if(TimerId == TIMER_MSP && m_AutoStereoSelect->GetValue() == TRUE && m_pBT848Card->HasMSP())
    {
        eSoundChannel newChannel = m_pBT848Card->IsAudioChannelDetected((eSoundChannel)m_AudioChannel->GetValue());
        if (newChannel != m_AudioChannel->GetValue())
        {
            m_AudioChannel->SetValue(newChannel);
            if (StatusBar_IsVisible() == TRUE)
            {
                char szText[128];
                m_pBT848Card->GetMSPPrintMode(szText);
                StatusBar_ShowText(STATUS_AUDIO, szText);
            }
        }
    }*/
}
