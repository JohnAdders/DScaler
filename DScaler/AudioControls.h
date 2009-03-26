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
 * @file AudioControls.h AudioControls Header file
 */

#if !defined(__AUDIOCONTROLS_H__)
#define __AUDIOCONTROLS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// \todo: create a generic audio controls enumeration

class CAudioControls
{
public:
    CAudioControls();
    virtual ~CAudioControls();
    virtual BOOL HasMute();
    virtual void SetMute(BOOL bMute = TRUE);
    virtual BOOL IsMuted();
    virtual BOOL HasVolume();
    virtual void SetVolume(WORD nVolume);
    virtual WORD GetVolume();
    virtual BOOL HasBalance();
    virtual void SetBalance(WORD nBalance);
    virtual WORD GetBalance();
    virtual BOOL HasBass();
    virtual void SetBass(WORD nLevel);
    virtual WORD GetBass();
    virtual BOOL HasTreble();
    virtual void SetTreble(WORD nLevel);
    virtual WORD GetTreble();
    virtual BOOL HasEqualizers();
    virtual WORD GetEqualizerCount();
    virtual void SetEqualizerLevel(WORD nIndex, WORD nLevel);
    virtual WORD GetEqualizerLevel(WORD nIndex);
    virtual BOOL HasBassBoost();
    virtual void SetBassBoost(BOOL bBoost);
    virtual BOOL IsBassBoosted();
    virtual BOOL HasLoudness();
    virtual void SetLoudness(WORD nLevel);
    virtual WORD GetLoudness();
    virtual BOOL HasSpatialEffect();
    virtual void SetSpatialEffect(int nLevel);
    virtual int  GetSpatialEffect();
    virtual BOOL HasDolby();
    virtual void SetDolby(WORD nMode, WORD nNoise, WORD nSpatial, WORD nPan, WORD nPanorama);
    // \todo: define a getter for Dolby

    virtual BOOL HasAutoVolumeCorrection();
    virtual void SetAutoVolumeCorrection(long milliSeconds);
    virtual long GetAutoVolumeCorrection();
};

#endif // !defined(__AUDIOCONTROLS_H__)
