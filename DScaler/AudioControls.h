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
    virtual bool HasMute();
    virtual void SetMute(bool bMute = true);
    virtual bool IsMuted();
    virtual bool HasVolume();
    virtual void SetVolume(WORD nVolume);
    virtual WORD GetVolume();
    virtual bool HasBalance();
    virtual void SetBalance(WORD nBalance);
    virtual WORD GetBalance();
    virtual bool HasBass();
    virtual void SetBass(WORD nLevel);
    virtual WORD GetBass();
    virtual bool HasTreble();
    virtual void SetTreble(WORD nLevel);
    virtual WORD GetTreble();
    virtual bool HasEqualizers();
    virtual WORD GetEqualizerCount();
    virtual void SetEqualizerLevel(WORD nIndex, WORD nLevel);
    virtual WORD GetEqualizerLevel(WORD nIndex);
    virtual bool HasBassBoost();
    virtual void SetBassBoost(bool bBoost);
    virtual bool IsBassBoosted();
    virtual bool HasLoudness();
    virtual void SetLoudness(WORD nLevel);
    virtual WORD GetLoudness();
    virtual bool HasSpatialEffect();
    virtual void SetSpatialEffect(int nLevel);
    virtual int  GetSpatialEffect();
    virtual bool HasDolby();
    virtual void SetDolby(WORD nMode, WORD nNoise, WORD nSpatial, WORD nPan, WORD nPanorama);    
    // \todo: define a getter for Dolby
    
    virtual bool HasAutoVolumeCorrection();
    virtual void SetAutoVolumeCorrection(long milliSeconds);
    virtual long GetAutoVolumeCorrection();
};

#endif // !defined(__AUDIOCONTROLS_H__)
