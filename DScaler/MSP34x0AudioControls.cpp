//
// $Id: MSP34x0AudioControls.cpp,v 1.7 2007-02-18 21:17:43 robmuller Exp $
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
// Revision 1.6  2007/02/18 21:15:31  robmuller
// Added option to not compile BT8x8 code.
//
// Revision 1.5  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.4  2002/10/15 19:16:29  kooiman
// Fixed Spatial effect for Audio decoder & MSP
//
// Revision 1.3  2002/09/27 14:14:22  kooiman
// MSP34xx fixes.
//
// Revision 1.2  2002/09/15 15:58:33  kooiman
// Added Audio standard detection & some MSP fixes.
//
// Revision 1.1  2002/09/12 21:44:27  ittarnavsky
// split the MSP34x0 in two files one for the AudioControls the other foe AudioDecoder
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file MSP34x0AudioControls.cpp CMSP34x0AudioControls Implementation
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

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

    SetDSPRegister(DSP_WR_SCART1_VOLUME, (nVolume!=0) ? 0x4000 : 0);    
    SetDSPRegister(DSP_WR_SCART2_VOLUME, (nVolume!=0) ? 0x4000 : 0);        
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
    if ((nIndex < 0) || (nIndex>32767))
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

void CMSP34x0AudioControls::SetAutoVolumeCorrection(long milliSeconds)
{
    if (milliSeconds == 0)
	{
		SetDSPRegister(DSP_WR_AVC, 0x0000);
	}
	else
	{      	
        WORD Val = 0x08; //8 seconds
        if (milliSeconds <= 4000)
        {
            Val = 0x04;  //4 seconds
        }
        if (milliSeconds <= 2000)
        {
            Val = 0x02;  //2 seconds
        }
        if (milliSeconds < 1000)
        {
            Val = 0x01;  //20 ms
        }
        
		SetDSPRegister(DSP_WR_AVC, 0x8000 | ((Val&0x0F) << 8));
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

void CMSP34x0AudioControls::SetSpatialEffect(int nLevel)
{
    if (nLevel > 127)
    {
        nLevel = 127;
    }
    if (nLevel < -128)
    {
        nLevel = -128;
    }
    if (nLevel < 0)
	{
		nLevel += 256;
	}
	// Mode A, Automatic high pass gain
    SetDSPRegister(DSP_WR_LDSPK_SPATIALEFF, ((nLevel & 0xFF) << 8) | 0x8);
}

bool CMSP34x0AudioControls::HasSpatialEffect()
{
    return true;
}

int CMSP34x0AudioControls::GetSpatialEffect()
{
    WORD Result = GetDSPRegister(DSP_RD_LDSPK_SPATIALEFF);    
    int nLevel = ((Result >> 8) & 0xFF);
    if (nLevel>=128)
    {
        nLevel-=256;
    }
    return nLevel;
}


bool CMSP34x0AudioControls::HasAutoVolumeCorrection()
{
    ///\todo Which MSP chips support this?
    return TRUE;
}

long CMSP34x0AudioControls::GetAutoVolumeCorrection()
{
    WORD Result = GetDSPRegister(DSP_RD_AVC);
    if (Result&0x8000)
    {
        if (Result&0x0010)
        {
            return 20;
        }
        else if (Result&0x0020)
        {
            return 2000;
        } 
        else if (Result&0x0040)
        {
            return 4000;
        }
        else if (Result&0x0080)
        {
            return 8000;
        }
        return 0;
    }
    return 0;
}

#endif // WANT_BT8X8_SUPPORT
