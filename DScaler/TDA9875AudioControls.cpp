//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 Robert Milharcic.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9875AudioControls.cpp TDA9875AudioControls Implementation
 */

#include "stdafx.h"
#include "TDA9875.h"
#include "TDA9875AudioControls.h"
#include "DebugLog.h"
#include "DScaler.h"
#include "DebugLog.h"

#include <math.h>

CTDA9875AudioControls::CTDA9875AudioControls() : CTDA9875()
{
    m_bMuted = true;
    m_bKeepItMuted = false;
    m_nVolume = 900;
    m_nBalance = 0;
    m_nBass = 0;
    m_nTreble = 0;
    m_nLoudness = 0;
    m_nSpatialEffect = 0;
    m_bSpatialEffectReg = 0;
    m_bAVLReg = 0;
    m_nAVLDecay = 0;
    m_bBassBoost = false;
    m_SoundChannel = SOUNDCHANNEL_MONO;

    eEventType EventList[] = {EVENT_SOUNDCHANNEL, EVENT_ENDOFLIST};
    EventCollector->Register(this, EventList);
}

void CTDA9875AudioControls::OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
    if (Event == EVENT_SOUNDCHANNEL)
    {
        m_SoundChannel = (eSoundChannel)NewValue;
        SetSpatialEffect(m_nSpatialEffect);
    }
}

bool CTDA9875AudioControls::HasMute()
{
    return true;
}

void CTDA9875AudioControls::SetMute(bool mute)
{
    if (m_bKeepItMuted)
    {
        m_bMuted = false;
        return;
    }
    m_bMuted = mute;
    if (m_bMuted)
        WriteToSubAddress(TDA9875_MUT, TDA9875_MUTE_ON);
    else
        WriteToSubAddress(TDA9875_MUT, TDA9875_MUTE_OFF);
}

bool CTDA9875AudioControls::IsMuted()
{
    return m_bMuted;
}

bool CTDA9875AudioControls::HasVolume()
{
    return true;
}

void CTDA9875AudioControls::SetVolume(WORD nVolume)
{

    m_nVolume = (short)nVolume;

    // sanity check
    if (m_nVolume < 0)
        m_nVolume = 0;
    if (m_nVolume > 1000)
        m_nVolume = 1000;

    SetMute(false);

    int nBalLeft = (m_nBalance <= 0) ? 127 : (127 - m_nBalance);
    int nBalRight = (m_nBalance >= 0) ? 127 : (127 + m_nBalance);
    
    int nVolLeft = MulDiv(m_nVolume, nBalLeft, 127);
    int nVolRight = MulDiv(m_nVolume, nBalRight, 127);

    BYTE bVolumeTDA9875Left;
    BYTE bVolumeTDA9875Right;

    bVolumeTDA9875Left  = 
        (nVolLeft > 0) ? 
            (BYTE)floor(33.21928*log10(1.66416*nVolLeft - 0.66416) - 82.5) : -84;
    
    bVolumeTDA9875Right = 
        (nVolRight > 0) ? 
            (BYTE)floor(33.21928*log10(1.66416*nVolRight - 0.66416) - 82.5) : -84;

    // main
    WriteToSubAddress(TDA9875_MVL, bVolumeTDA9875Left);
    WriteToSubAddress(TDA9875_MVR, bVolumeTDA9875Right);

    // aux
    WriteToSubAddress(TDA9875_AVL, bVolumeTDA9875Left);
    WriteToSubAddress(TDA9875_AVR, bVolumeTDA9875Right);
}

WORD CTDA9875AudioControls::GetVolume()
{
    return (WORD)m_nVolume;
}

bool CTDA9875AudioControls::HasBalance()
{
    return true;
}

void CTDA9875AudioControls::SetBalance(WORD nBalance)
{
    m_nBalance = (short)nBalance;
    
    // sanity check
    if (m_nBalance < -127)
        m_nBalance = -127;
    if (m_nBalance > 127)
        m_nBalance = 127;

    SetVolume((WORD)m_nVolume);
}

WORD CTDA9875AudioControls::GetBalance()
{
    return (WORD)m_nBalance;
}

bool CTDA9875AudioControls::HasBass()
{
    return true;
}

void CTDA9875AudioControls::SetBass(WORD nBass)
{

    m_nBass = (short)nBass;

    // sanity check
    if (m_nBass < -96)
        m_nBass = -96;
    if (m_nBass > 127)
        m_nBass = 127;

    BYTE bBassTDA9875 = (BYTE)floor(0.12108*m_nBass + 0.12332);

    // main
    WriteToSubAddress(TDA9875_MBA, bBassTDA9875);
    // aux
    WriteToSubAddress(TDA9875_ABA, bBassTDA9875);
}

WORD CTDA9875AudioControls::GetBass()
{
    return (WORD)m_nBass;
}

bool CTDA9875AudioControls::HasTreble()
{
    return true;
}

void CTDA9875AudioControls::SetTreble(WORD nTreble)
{
    m_nTreble = (short)nTreble;
    
    // sanity check
    if (m_nTreble < -96)
        m_nTreble = -96;
    if (m_nTreble > 127)
        m_nTreble = 127;

    BYTE bTrebleTDA9875 = (BYTE)floor(0.10762*m_nTreble - 1.16816);

    // main
    WriteToSubAddress(TDA9875_MTR, bTrebleTDA9875);
    
    // aux
    WriteToSubAddress(TDA9875_ATR, bTrebleTDA9875);
}

WORD CTDA9875AudioControls::GetTreble()
{
    return (WORD)m_nTreble;
}

bool CTDA9875AudioControls::HasEqualizers()
{
    return false;
}

bool CTDA9875AudioControls::HasBassBoost()
{
    return true;
}

void CTDA9875AudioControls::SetBassBoost(bool bBoost)
{    
    m_bBassBoost = bBoost;
    if (bBoost)
        WriteToSubAddress(TDA9875_BBO, 85);
    else
        WriteToSubAddress(TDA9875_BBO, 0);
}

bool CTDA9875AudioControls::IsBassBoosted()
{
    return m_bBassBoost;
}


bool CTDA9875AudioControls::HasLoudness()
{
    return true;
}

void CTDA9875AudioControls::SetLoudness(WORD nLevel)
{
    m_nLoudness = (short)nLevel;
    
    // sanity check
    if (m_nTreble < 0)
        m_nTreble = 0;
    if (m_nTreble > 255)
        m_nTreble = 255;

    BYTE bLoudnessTDA9875 = (BYTE)floor(0.07059*m_nLoudness + 0.5);

    WriteToSubAddress(TDA9875_LOU, bLoudnessTDA9875);

}
 
WORD CTDA9875AudioControls::GetLoudness()
{
    return (WORD)m_nLoudness;
}

bool CTDA9875AudioControls::HasDolby()
{
    return false;
}

bool CTDA9875AudioControls::HasAutoVolumeCorrection()
{
    return true;
}
void CTDA9875AudioControls::SetAutoVolumeCorrection(long milliSeconds)
{
    if (milliSeconds < 1000)
    {
        m_bAVLReg = 0; // off
        m_nAVLDecay = 0;
    }
    else if (milliSeconds < 3000)
    {
        m_bAVLReg = 0x01;  //2 seconds
        m_nAVLDecay = 2000;
    }
    else if (milliSeconds < 6000)
    {
        m_bAVLReg = 0x02;  //4 seconds
        m_nAVLDecay = 4000;
    }
    else
    {
        m_bAVLReg = 0x03; //8 seconds
        m_nAVLDecay = 8000;
    }

    WriteToSubAddress(TDA9875_AER, m_bSpatialEffectReg | m_bAVLReg);
}


long CTDA9875AudioControls::GetAutoVolumeCorrection()
{
    return m_nAVLDecay;
}

bool CTDA9875AudioControls::HasSpatialEffect()
{
    return true;
}

void CTDA9875AudioControls::SetSpatialEffect(int nLevel)
{
    // sanity check
    if (nLevel > 127)
        nLevel = 127;
    if (nLevel < -128)
        nLevel = -128;

    m_nSpatialEffect = nLevel;
    m_bSpatialEffectReg = (BYTE)((abs(nLevel)+17)*5/182);
    if (m_SoundChannel == SOUNDCHANNEL_STEREO)
        m_bSpatialEffectReg = (m_bSpatialEffectReg << 4);
    else
        m_bSpatialEffectReg = (m_bSpatialEffectReg << 2);

    WriteToSubAddress(TDA9875_AER, m_bSpatialEffectReg | m_bAVLReg);
}

int CTDA9875AudioControls::GetSpatialEffect()
{
    return m_nSpatialEffect;
}
