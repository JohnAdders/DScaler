//
// $Id: MSP34x0AudioControls.h,v 1.3 2002-10-15 19:16:29 kooiman Exp $
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
// Revision 1.2  2002/09/15 15:58:33  kooiman
// Added Audio standard detection & some MSP fixes.
//
// Revision 1.1  2002/09/12 21:44:27  ittarnavsky
// split the MSP34x0 in two files one for the AudioControls the other foe AudioDecoder
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__MSP34X0AUDIOCONTROLS_H__)
#define __MSP34X0AUDIOCONTROLS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BT848_Defines.h"
#include "Setting.h"

#include "AudioControls.h"

class CMSP34x0AudioControls : public CMSP34x0, public CAudioControls
{
public:
    CMSP34x0AudioControls();
	virtual ~CMSP34x0AudioControls() {};    
	void SetDolby(long Mode, long nNoise, long nSpatial, long nPan, long Panorama);	

    // from IAudioControls
    bool HasMute();
    void SetMute(bool mute=true);
    bool IsMuted();
    void SetVolume(WORD nVolume);
    WORD GetVolume();
    void SetBalance(WORD nBalance);
    WORD GetBalance();
    void SetBass(WORD nLevel);
    WORD GetBass();
    void SetTreble(WORD nLevel);
    WORD GetTreble();
    bool HasEqualizers();
    WORD GetEqualizerCount();
    void SetEqualizerLevel(WORD nIndex, WORD nLevel);
    WORD GetEqualizerLevel(WORD nIndex);
    bool HasBassBoost();
    void SetBassBoost(bool bBoost);
    bool IsBassBoosted();
    bool HasLoudness();
    void SetLoudness(WORD nLevel);
    WORD GetLoudness();
    bool HasAutoVolumeCorrection();
    void SetAutoVolumeCorrection(long milliSeconds);
    long GetAutoVolumeCorrection();
    bool HasSpatialEffect();
    void SetSpatialEffect(int nLevel);
    int  GetSpatialEffect();

private:
    bool m_bMuted;
    int m_nVolume;
    int m_nBalance;
    int m_nBass;
    int m_nTreble;
    bool m_bBassBoost;
};

#endif // !defined(__MSP34X0AUDIOCONTROLS_H__)