//
// $Id$
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


BOOL CAudioControls::HasMute()
{
    return FALSE;
}

void CAudioControls::SetMute(BOOL bMute)
{
}

BOOL CAudioControls::IsMuted()
{
    return FALSE;
}

BOOL CAudioControls::HasVolume()
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

BOOL CAudioControls::HasBalance()
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

BOOL CAudioControls::HasBass()
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

BOOL CAudioControls::HasTreble()
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

BOOL CAudioControls::HasEqualizers()
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

BOOL CAudioControls::HasBassBoost()
{
    return FALSE;
}

void CAudioControls::SetBassBoost(BOOL bBoost)
{
}

BOOL CAudioControls::IsBassBoosted()
{
    return FALSE;
}

BOOL CAudioControls::HasLoudness()
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

BOOL CAudioControls::HasSpatialEffect()
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

BOOL CAudioControls::HasDolby()
{
    return FALSE;
}

void CAudioControls::SetDolby(WORD nMode, WORD nNoise, WORD nSpatial, WORD nPan, WORD nPanorama)
{
}

BOOL CAudioControls::HasAutoVolumeCorrection()
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