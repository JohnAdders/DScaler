//
// $Id: AudioControls.cpp,v 1.4 2003-10-27 10:39:50 adcockj Exp $
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
// Revision 1.3  2002/10/15 19:16:29  kooiman
// Fixed Spatial effect for Audio decoder & MSP
//
// Revision 1.2  2002/09/15 15:57:27  kooiman
// Added Audio standard support.
//
// Revision 1.1  2002/09/12 21:50:59  ittarnavsky
// Added for the change from IAudioControls to CAudioControls
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file AudioControl.cpp Audio Control Classes
 */


#include "stdafx.h"
#include "AudioControls.h"


CAudioControls::CAudioControls()
{
}

CAudioControls::~CAudioControls()
{
}


bool CAudioControls::HasMute()
{
    return FALSE;
}

void CAudioControls::SetMute(bool bMute)
{
}

bool CAudioControls::IsMuted()
{
    return FALSE;
}

bool CAudioControls::HasVolume()
{
    return FALSE;
}

void CAudioControls::SetVolume(WORD nVolume)
{
}

WORD CAudioControls::GetVolume()
{
    return 0;
}

bool CAudioControls::HasBalance()
{
    return FALSE;
}

void CAudioControls::SetBalance(WORD nBalance)
{
}

WORD CAudioControls::GetBalance()
{
    return 0;
}

bool CAudioControls::HasBass()
{
    return FALSE;
}

void CAudioControls::SetBass(WORD nLevel)
{
}

WORD CAudioControls::GetBass()
{
    return 0;
}

bool CAudioControls::HasTreble()
{
    return FALSE;
}

void CAudioControls::SetTreble(WORD nLevel)
{
}

WORD CAudioControls::GetTreble()
{
    return 0;
}

bool CAudioControls::HasEqualizers()
{
    return FALSE;
}

WORD CAudioControls::GetEqualizerCount()
{
    return 0;
}

void CAudioControls::SetEqualizerLevel(WORD nIndex, WORD nLevel)
{
}

WORD CAudioControls::GetEqualizerLevel(WORD nIndex)
{
    return 0;
}

bool CAudioControls::HasBassBoost()
{
    return FALSE;
}

void CAudioControls::SetBassBoost(bool bBoost)
{
}

bool CAudioControls::IsBassBoosted()
{
    return FALSE;
}

bool CAudioControls::HasLoudness()
{
    return FALSE;
}

void CAudioControls::SetLoudness(WORD nLevel)
{
}

WORD CAudioControls::GetLoudness()
{
    return 0;
}

bool CAudioControls::HasSpatialEffect()
{
    return FALSE;
}

void CAudioControls::SetSpatialEffect(int nLevel)
{
}

int CAudioControls::GetSpatialEffect()
{
    return 0;
}

bool CAudioControls::HasDolby()
{
    return FALSE;
}

void CAudioControls::SetDolby(WORD nMode, WORD nNoise, WORD nSpatial, WORD nPan, WORD nPanorama)
{
}

bool CAudioControls::HasAutoVolumeCorrection()
{
    return FALSE;
}

void CAudioControls::SetAutoVolumeCorrection(long nDecayTimeIndex)
{

}

long CAudioControls::GetAutoVolumeCorrection()
{
    return 0;
}