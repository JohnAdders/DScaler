//
// $Id: MSP34x0.h,v 1.5 2001-12-18 23:36:01 adcockj Exp $
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
// Revision 1.4  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.3  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
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
#include "IAudioControls.h"
#include "AudioDecoder.h"

#define MSP_MODE_AM_DETECT   0
#define MSP_MODE_FM_RADIO    2
#define MSP_MODE_FM_TERRA    3
#define MSP_MODE_FM_SAT      4
#define MSP_MODE_FM_NICAM1   5
#define MSP_MODE_FM_NICAM2   6

/** Class that allows control of feature on a MSP chip.
    Currently this class only support earlier versiosn of this chip.
*/
class CMSP34x0 : public CI2CDevice
{
public:
    CMSP34x0();
	virtual ~CMSP34x0() {};
    WORD GetVersion();
    WORD GetProductCode();

protected:
    virtual BYTE GetDefaultAddress() const;
    WORD GetRegister(BYTE subAddress, WORD reg);
    void SetRegister(BYTE subAddress, WORD reg, WORD value);

protected:
    bool m_bNicam;

};

class CMSP34x0Controls : public CMSP34x0, public IAudioControls
{
public:
    CMSP34x0Controls();
	virtual ~CMSP34x0Controls() {};
    void SetLoudnessAndSuperBass(long nLoudness, bool bSuperBass);
    void SetSpatialEffects(long nSpatial);
    void SetEqualizer(long EqIndex, long nLevel);
    void Reset();

    // from IAudioControls
    void SetMute(bool mute=true);
    bool IsMuted();
    void SetVolume(WORD volume);
    WORD GetVolume();
    void SetBalance(WORD balance);
    WORD GetBalance();
    void SetBass(WORD level);
    WORD GetBass();
    void SetTreble(WORD level);
    WORD GetTreble();

private:
    bool m_Muted;
    int m_Volume;
    int m_Balance;
    int m_Bass;
    int m_Treble;

};

class CMSP34x0Decoder : public CMSP34x0, public CAudioDecoder
{
public:

    enum eMajorMode
    {
        MSP34x0_MAJORMODE_NTSC = 0,
        MSP34x0_MAJORMODE_PAL_BG,
        MSP34x0_MAJORMODE_PAL_I,
        MSP34x0_MAJORMODE_PAL_DK_SECAM_SAT,
    };

    enum eMinorMode
    {
        MSP34x0_MINORMODE_PAL_BG_FM = 0,
        MSP34x0_MINORMODE_PAL_BG_NICAM,
        MSP34x0_MINORMODE_PAL_DK_NICAM,
        MSP34x0_MINORMODE_PAL_DK1_FM,
        MSP34x0_MINORMODE_PAL_DK2_FM,
        MSP34x0_MINORMODE_PAL_SAT_FM_SB,
        MSP34x0_MINORMODE_PAL_SAT_FM_S,
        MSP34x0_MINORMODE_PAL_SAT_FM_B,
    };

public:
    CMSP34x0Decoder();
	virtual ~CMSP34x0Decoder() {};
    // from CAudioDecoder the default Getters are used
    void SetVideoFormat(eVideoFormat videoFormat);
    void SetSoundChannel(eSoundChannel soundChannel);
    void SetAudioInput(eAudioInput audioInput);
    eSoundChannel IsAudioChannelDetected(eSoundChannel desiredAudioChannel);

    void SetMode(long MSPMode);
    void SetMajorMinorMode(eMajorMode MajorMode, eMinorMode MinorMode);

private:
    void SetCarrier(int cdo1, int cdo2);

private:
    int m_nMode;
    eMinorMode m_nMinorMode;
    eMajorMode m_nMajorMode;
};


#endif // !defined(__MSP34X0_H__)
