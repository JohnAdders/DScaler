//
// $Id: MSP34x0AudioControls.cpp,v 1.1 2002-09-12 21:44:27 ittarnavsky Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2002 itt@myself.com
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
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MSP34x0.h"
#include "MSP34x0AudioControls.h"
#include "DebugLog.h"
#include "DScaler.h"

CMSP34x0AudioControls::CMSP34x0AudioControls() : CMSP34x0()
{
    m_bMuted = false;
    m_nVolume = 900;
    m_nBalance = 0;
    m_nBass = 0;
    m_nTreble = 0;
    m_bBassBoost = false;
}

bool CMSP34x0AudioControls::HasMute()
{
    return true;
}

void CMSP34x0AudioControls::SetMute(bool mute)
{
    m_bMuted = mute;
    if (m_bMuted)
    {
        SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0xFF00);
        SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0xFF00);
    }
    else
    {
        SetVolume(m_nVolume);
    }
}

bool CMSP34x0AudioControls::IsMuted()
{
    return m_bMuted;
}

void CMSP34x0AudioControls::SetVolume(WORD nVolume)
{
    if (nVolume < 0 || nVolume > 1000)
    {
        return;
    }
    m_nVolume = nVolume;
    nVolume = MulDiv(nVolume, 0x7f0, 1000);
    // Use Mute if less than 0x10
    if(nVolume < 0x10)
    {
        nVolume = 0;
    }
    SetDSPRegister(DSP_WR_LDSPK_VOLUME, nVolume << 4);
    SetDSPRegister(DSP_WR_HEADPH_VOLUME, nVolume << 4);
}

WORD CMSP34x0AudioControls::GetVolume()
{
    return m_nVolume;
}

void CMSP34x0AudioControls::SetBalance(WORD nBalance)
{
    m_nBalance = nBalance;
    SetDSPRegister(DSP_WR_LDSPK_BALANCE, (nBalance & 0xFF) << 8);
    SetDSPRegister(DSP_WR_HEADPH_BALANCE, (nBalance & 0xFF) << 8);
}

WORD CMSP34x0AudioControls::GetBalance()
{
    return m_nBalance;
}

void CMSP34x0AudioControls::SetBass(WORD nBass)
{
    if (nBass < -96)
    {
        return;
    }
    m_nBass = nBass;
    SetDSPRegister(DSP_WR_LDSPK_BASS, (nBass & 0xFF) << 8);
    SetDSPRegister(DSP_WR_HEADPH_BASS, (nBass & 0xFF) << 8);
}

WORD CMSP34x0AudioControls::GetBass()
{
    return m_nBass;
}

void CMSP34x0AudioControls::SetTreble(WORD nTreble)
{
    if (nTreble < -96)
    {
        return;
    }
    m_nTreble = nTreble;
    SetDSPRegister(DSP_WR_LDSPK_TREBLE, (nTreble & 0xFF) << 8);
    SetDSPRegister(DSP_WR_HEADPH_TREBLE, (nTreble & 0xFF) << 8);
}

WORD CMSP34x0AudioControls::GetTreble()
{
    return m_nTreble;
}


bool CMSP34x0AudioControls::HasEqualizers()
{
    return true; // \todo: fix this
}

WORD CMSP34x0AudioControls::GetEqualizerCount()
{
    return HasEqualizers() ? 5 : 0;
}

WORD CMSP34x0AudioControls::GetEqualizerLevel(WORD nIndex)
{
    return 0; // \todo: fix this
}

void CMSP34x0AudioControls::SetEqualizerLevel(WORD nIndex, WORD nLevel)
{
    if (nIndex < 0)
	{
		//Enable/disable equalizer	
		//Disable means: bass & treble control is active
		SetDSPRegister(DSP_WR_MODE_TONE_CTL, nLevel ? 0xFF00 : 0x0000);
		return;
	}
	
	if (!HasEqualizers() || nIndex > 4)
    {
        return;
    }
    if (nLevel < -96 || nLevel > 96)
    {
        return;
    }
	
    SetDSPRegister((eDSPWriteRegister)(DSP_WR_LDSPK_EQ1 + nIndex), (nLevel & 0xFF) << 8);
}


bool CMSP34x0AudioControls::HasBassBoost()
{
    return true;
}

void CMSP34x0AudioControls::SetBassBoost(bool bBoost)
{
    WORD ldspkr, headph;

    ldspkr = GetDSPRegister(DSP_RD_LDSPK_LOUDNESS);
    headph = GetDSPRegister(DSP_RD_HEADPH_LOUDNESS);
    if (bBoost) 
    {
        ldspkr |= 4;
        headph |= 4;
    } 
    else 
    {
        ldspkr &= 0xFFFB;
        headph &= 0xFFFB;
    }
    SetDSPRegister(DSP_WR_LDSPK_LOUDNESS, ldspkr);
    SetDSPRegister(DSP_WR_HEADPH_LOUDNESS, headph);
}

bool CMSP34x0AudioControls::IsBassBoosted()
{
    return (GetDSPRegister(DSP_RD_LDSPK_LOUDNESS) & 4) == 4;
}


bool CMSP34x0AudioControls::HasLoudness()
{
    return true;
}

void CMSP34x0AudioControls::SetLoudness(WORD nLevel)
{
    WORD ldspkr, headph;

    if (nLevel > 68)
    {
        return;
    }

    ldspkr = (GetDSPRegister(DSP_RD_LDSPK_LOUDNESS) & 0xFF) | ((nLevel & 0xFF) << 8);
    headph = (GetDSPRegister(DSP_RD_HEADPH_LOUDNESS) & 0xFF) | ((nLevel & 0xFF) << 8);

    SetDSPRegister(DSP_WR_LDSPK_LOUDNESS, ldspkr);
    SetDSPRegister(DSP_WR_HEADPH_LOUDNESS, headph);
}
 

WORD CMSP34x0AudioControls::GetLoudness()
{
    return (GetDSPRegister(DSP_RD_LDSPK_LOUDNESS) >> 8) & 0xFF;
}

void CMSP34x0AudioControls::SetAutomaticVolumeCorrection(long nDecayTimeIndex)
{
	if (nDecayTimeIndex == 0)
	{
		SetDSPRegister(DSP_WR_AVC, 0x0000);
	}
	else
	{
		SetDSPRegister(DSP_WR_AVC, 0x8000 | ((nDecayTimeIndex&0x0F) << 8));
	}
}

void CMSP34x0AudioControls::SetDolby(long Mode, long nNoise, long nSpatial, long nPan, long Panorama)
{
    //if (!m_HasDolby)
    //{
    //    return;
    //}

   // Disable spatial effects
   SetDSPRegister(DSP_WR_LDSPK_SPATIALEFF, 0);

    switch (Mode)
    {
      case 1: //through
         SetDSPRegister(DSP_WR_SURROUND_PROCESSING, 0); 
         SetDSPRegister(DSP_WR_SURROUND_NOISE, 0);
         break;
      case 2: //prologic
         SetDSPRegister(DSP_WR_SURROUND_PROCESSING, 0x0100); 
         SetDSPRegister(DSP_WR_SURROUND_NOISE, 0);
         break;
      case 3: //noise mode
         SetDSPRegister(DSP_WR_SURROUND_PROCESSING, 0); 
         SetDSPRegister(DSP_WR_SURROUND_NOISE, WORD(0x8000 | (nNoise&0xF0)));
         break;
    }

    // Set Virtual surround Spatial effects		
    SetDSPRegister(DSP_WR_SURROUND_SPATIAL, WORD(int(nSpatial) << 8) );

	// Set Panorama effect...		
    SetDSPRegister(DSP_WR_SURROUND_PANORAMA, WORD(int(nPan)     << 8) );

	// Based on requested mode, set it.
	SetDSPRegister(DSP_WR_SURROUND_PANORAMA_MODE, ((Panorama==1) ? 0x50 :
			((Panorama==2) ? 0x60 : 0)));
		
}


void CMSP34x0AudioControls::SetSpatialEffects(long nSpatial)
{
    if (nSpatial < 0)
	{
		nSpatial+=256;
	}
	// Mode A, Automatic high pass gain
    SetDSPRegister(DSP_WR_LDSPK_SPATIALEFF, ((nSpatial & 0xFF) << 8) | 0x8);
}
