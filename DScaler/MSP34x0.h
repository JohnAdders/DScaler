//
// $Id: MSP34x0.h,v 1.2 2001-11-26 13:02:27 adcockj Exp $
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
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__MSP34X0_H__)
#define __MSP34X0_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BT848_Defines.h"
#include "Setting.h"

#include "I2CDevice.h"

#define MSP_MODE_AM_DETECT   0
#define MSP_MODE_FM_RADIO    2
#define MSP_MODE_FM_TERRA    3
#define MSP_MODE_FM_SAT      4
#define MSP_MODE_FM_NICAM1   5
#define MSP_MODE_FM_NICAM2   6

class CMSP34x0 : public CI2CDevice  
{
public:
    CMSP34x0();
    void Reset();
    WORD GetVersion();
    WORD GetProductCode();
    void SetMute();
    void SetVolume(long nVolume);
    void SetBalance(long nBalance);
    void SetBass(long nBass);
    void SetTreble(long nTreble);
    void SetLoudnessAndSuperBass(long nLoudness, bool bSuperBass);
    void SetSpatialEffects(long nSpatial);
    void SetEqualizer(long EqIndex, long nLevel);
    void SetMode(long MSPMode);
    void SetStereo(eSoundChannel soundChannel);
    void SetMajorMinorMode(int MajorMode, int MinorMode);
    void GetPrintMode(LPSTR Text);
    eSoundChannel GetWatchMode(eSoundChannel desiredSoundChannel);

protected:
    virtual BYTE GetDefaultAddress() const;

private:
    void SetCarrier(int cdo1, int cdo2);

    WORD GetRegister(BYTE subAddress, WORD reg);
    void SetRegister(BYTE subAddress, WORD reg, WORD value);

private:
    bool m_bNicam;
    int m_nMode;
    int m_nMajorMode;
    int m_nMinorMode;
    eSoundChannel m_eSoundChannel;
};

#endif // !defined(__MSP34X0_H__)
