//
// $Id: NoAudioControls.cpp,v 1.1 2001-12-05 21:45:11 ittarnavsky Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
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
#include "NoAudioControls.h"


CNoAudioControls::CNoAudioControls()
{
    m_Mute = false;
    m_Volume = 0;
    m_Balance = 0;
    m_Bass = 0;
    m_Treble = 0;
}

CNoAudioControls::~CNoAudioControls()
{
}

void CNoAudioControls::SetMute(bool mute)
{
    m_Mute = mute;
}

bool CNoAudioControls::IsMuted()
{
    return m_Mute;
}

void CNoAudioControls::SetVolume(WORD volume)
{
    m_Volume = volume;
}

WORD CNoAudioControls::GetVolume()
{
    return m_Volume;
}

void CNoAudioControls::SetBalance(WORD balance)
{
    m_Balance = balance;
}

WORD CNoAudioControls::GetBalance()
{
    return m_Balance;
}

void CNoAudioControls::SetBass(WORD level)
{
    m_Bass = level;
}

WORD CNoAudioControls::GetBass()
{
    return m_Bass;
}

void CNoAudioControls::SetTreble(WORD level)
{
    m_Treble = level;
}

WORD CNoAudioControls::GetTreble()
{
    return m_Treble;
}