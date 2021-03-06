/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
 * @file TDA9875AudioControls.h CTDA9875AudioControls Header
 */
#if !defined(__TDA9875AUDIOCONTROLS_H__)
#define __TDA9875AUDIOCONTROLS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BT848_Defines.h"
#include "Setting.h"
#include "events.h"
#include "AudioControls.h"
#include "soundchannel.h"

class CTDA9875AudioControls : public CTDA9875, public CAudioControls, public CEventObject
{
public:

    BOOL m_bKeepItMuted;

    CTDA9875AudioControls();
    virtual ~CTDA9875AudioControls() {};

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    // from IAudioControls
    BOOL HasMute();
    void SetMute(BOOL mute = TRUE);
    BOOL IsMuted();

    BOOL HasVolume();
    void SetVolume(WORD nVolume);
    WORD GetVolume();

    BOOL HasBalance();
    void SetBalance(WORD nBalance);
    WORD GetBalance();

    BOOL HasBass();
    void SetBass(WORD nLevel);
    WORD GetBass();

    BOOL HasTreble();
    void SetTreble(WORD nLevel);
    WORD GetTreble();

    BOOL HasEqualizers();

    BOOL HasBassBoost();
    void SetBassBoost(BOOL bBoost);
    BOOL IsBassBoosted();

    BOOL HasLoudness();
    void SetLoudness(WORD nLevel);
    WORD GetLoudness();

    BOOL HasDolby();

    BOOL HasAutoVolumeCorrection();
    void SetAutoVolumeCorrection(long milliSeconds);
    long GetAutoVolumeCorrection();

    BOOL HasSpatialEffect();
    void SetSpatialEffect(int nLevel);
    int  GetSpatialEffect();

private:

    BOOL m_bMuted;
    short m_nVolume;
    short m_nBalance;
    short m_nBass;
    short m_nTreble;
    short m_nLoudness;

    BYTE m_bSpatialEffectReg;
    int m_nSpatialEffect;

    BYTE m_bAVLReg;
    int m_nAVLDecay;

    BOOL m_bBassBoost;

    eSoundChannel m_SoundChannel;
};

#endif // !defined(__TDA9875AUDIOCONTROLS_H__)