//
// $Id: MSP34x0.h,v 1.21 2002-10-11 21:50:02 ittarnavsky Exp $
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
// Revision 1.20  2002/09/27 14:14:22  kooiman
// MSP34xx fixes.
//
// Revision 1.19  2002/09/17 17:58:29  kooiman
// Small additions/fixes.
//
// Revision 1.18  2002/09/16 14:37:36  kooiman
// Added stereo autodetection.
//
// Revision 1.17  2002/09/15 15:58:33  kooiman
// Added Audio standard detection & some MSP fixes.
//
// Revision 1.16  2002/09/12 21:44:44  ittarnavsky
// split the MSP34x0 in two files one for the AudioControls the other foe AudioDecoder
//
// Revision 1.15  2002/09/07 20:54:49  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.14  2002/07/02 20:00:10  adcockj
// New setting for MSP input pin selection
//
// Revision 1.13  2002/03/04 20:03:50  adcockj
// About box changes
//
// Revision 1.12  2002/02/01 04:43:56  ittarnavsky
// some more audio related fixes
// removed the handletimermessages and getaudioname methods
// which break the separation of concerns oo principle
//
// Revision 1.11  2002/01/27 23:54:32  robmuller
// Removed the Auto Standard Detect of the rev G chips. + some reorganization of code.
//
// Revision 1.10  2002/01/23 22:57:28  robmuller
// Revision D/G improvements. The code is following the documentation much closer now.
//
// Revision 1.9  2001/12/21 11:07:31  adcockj
// Even more RevA fixes
//
// Revision 1.8  2001/12/20 23:46:21  ittarnavsky
// further RevA programming changes
//
// Revision 1.7  2001/12/20 12:55:54  adcockj
// First stab at supporting older MSP chips
//
// Revision 1.6  2001/12/19 19:26:17  ittarnavsky
// started rewrite of the sound standard selection
//
// Revision 1.5  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
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
#include "AudioControls.h"
#include "AudioDecoder.h"

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
    void Reset();

protected:
    virtual BYTE GetDefaultAddress() const;

    enum eDEMReadRegister
    {
        DEM_RD_STANDARD_RESULT = 0x007E,
        DEM_RD_STATUS = 0x0200,
        DEM_RD_C_AD_BITS = 0x0023,
        DEM_RD_ADD_BITS = 0x0038,
        DEM_RD_CIB_BITS = 0x003E,
        DEM_RD_ERROR_RATE = 0x0057,
        DEM_RD_PLL_CAPS = 0x021F,
        DEM_RD_AGC_GAIN = 0x021E,
    };
    WORD GetDEMRegister(eDEMReadRegister reg);

    enum eDEMWriteRegister
    {
        DEM_WR_STANDARD_SELECT = 0x0020,
        DEM_WR_MODUS = 0x0030,
        DEM_WR_I2S_CONFIG = 0x0040,
        DEM_WR_AUTO_FMAM = 0x0021,
        DEM_WR_A2_THRE = 0x0022,
        DEM_WR_CM_THRE = 0x0024,
        DEM_WR_AD_CV = 0x00BB,
        DEM_WR_MODE_REG = 0x0083,
        DEM_WR_FIR1 = 0x0001,
        DEM_WR_FIR2 = 0x0005,
        DEM_WR_DCO1_LO = 0x0093,
        DEM_WR_DCO1_HI = 0x009B,
        DEM_WR_DCO2_LO = 0x00A3,
        DEM_WR_DCO2_HI = 0x00AB,
        DEM_WR_PLL_CAPS = 0x001F,
        DEM_WR_LOAD_REG_12 = 0x0056,
        DEM_WR_LOAD_REG_1 = 0x0060,
        DEM_WR_SEARCH_NICAM = 0x0078,
        DEM_WR_SELF_TEST = 0x0792,
        DEM_WR_FAWCT_SOLL = 0x0107,
        DEM_WR_FAW_ER_TOL = 0x010F,
        DEM_WR_AUDIO_PLL = 0x02D7,
    };
    void SetDEMRegister(eDEMWriteRegister reg, WORD value);

    enum eDSPReadRegister
    {
        DSP_RD_LDSPK_VOLUME = 0x0000,
        DSP_RD_LDSPK_BALANCE = 0x0001,
        DSP_RD_LDSPK_BASS = 0x0002,
        DSP_RD_LDSPK_TREBLE = 0x0003,
        DSP_RD_LDSPK_LOUDNESS = 0x0004,
        DSP_RD_LDSPK_SPATIALEFF = 0x0005,
        DSP_RD_HEADPH_VOLUME = 0x0006,
        DSP_RD_SCART1_VOLUME = 0x0007,
        DSP_RD_LDSPK_SOURCE = 0x0008,
        DSP_RD_HEADPH_SOURCE = 0x0009,
        DSP_RD_SCART1_SOURCE = 0x000A,
        DSP_RD_I2S_SOURCE = 0x000B,
        DSP_RD_QPEAK_SOURCE = 0x000C,
        DSP_RD_SCART_PRESCALE = 0x000D,
        DSP_RD_FMAM_PRESCALE = 0x000E,
        DSP_RD_NICAM_PRESCALE = 0x0010,
        DSP_RD_I2S2_PRESCALE = 0x0012,
        DSP_RD_ACB = 0x0013,
        DSP_RD_BEEPER = 0x0014,
        DSP_RD_I2S1_PRESCALE = 0x0016,
        DSP_RD_QPEAK_LEFT = 0x0019,
        DSP_RD_QPEAK_RIGHT = 0x001A,
        DSP_RD_VERSION_CODE = 0x001E,
        DSP_RD_PRODUCT_CODE = 0x001F,
        DSP_RD_MODE_TONE_CTL = 0x0020,
        DSP_RD_LDSPK_EQ1 = 0x0021,
        DSP_RD_LDSPK_EQ2 = 0x0022,
        DSP_RD_LDSPK_EQ3 = 0x0023,
        DSP_RD_LDSPK_EQ4 = 0x0024,
        DSP_RD_LDSPK_EQ5 = 0x0025,
        DSP_RD_SUBWFR_LEVEL = 0x002C,
        DSP_RD_SUBWFR_FREQ = 0x002D,
        DSP_RD_HEADPH_BALANCE = 0x0030,
        DSP_RD_HEADPH_BASS = 0x0031,
        DSP_RD_HEADPH_TREBLE = 0x0032,
        DSP_RD_HEADPH_LOUDNESS = 0x0033,
        DSP_RD_SCART2_VOLUME = 0x0040,
        DSP_RD_SCART2_SOURCE = 0x0041,
        DSP_RD_MDB_STRENGTH = 0x0068,
        DSP_RD_MDB_APLITUDE = 0x0069,
        DSP_RD_MDB_HARMONIC = 0x006A,
        DSP_RD_MDB_LOPASS = 0x006B,
        DSP_RD_MDB_HIPASS = 0x006C,
        DSP_RD_FM_DEEMPH = 0x000F,
        DSP_RD_IDENT_MODE = 0x0015,
        DSP_RD_FM_FC_NOTCH = 0x0017,
        DSP_RD_A2_STEREO_DETECT = 0x0018,
        DSP_RD_FM1_DCLVL = 0x001B,
        DSP_RD_FM2_DCLVL = 0x001C,
        DSP_RD_AVC = 0x0029,
    };
    WORD GetDSPRegister(eDSPReadRegister reg);

    enum eDSPWriteRegister
    {
        DSP_WR_LDSPK_VOLUME = 0x0000,
        DSP_WR_LDSPK_BALANCE = 0x0001,
        DSP_WR_LDSPK_BASS = 0x0002,
        DSP_WR_LDSPK_TREBLE = 0x0003,
        DSP_WR_LDSPK_LOUDNESS = 0x0004,
        DSP_WR_LDSPK_SPATIALEFF = 0x0005,
        DSP_WR_HEADPH_VOLUME = 0x0006,
        DSP_WR_SCART1_VOLUME = 0x0007,
        DSP_WR_LDSPK_SOURCE = 0x0008,
        DSP_WR_HEADPH_SOURCE = 0x0009,
        DSP_WR_SCART1_SOURCE = 0x000A,
        DSP_WR_I2S_SOURCE = 0x000B,
        DSP_WR_QPEAK_SOURCE = 0x000C,
        DSP_WR_SCART_PRESCALE = 0x000D,
        DSP_WR_FMAM_PRESCALE = 0x000E,
        DSP_WR_NICAM_PRESCALE = 0x0010,
        DSP_WR_I2S2_PRESCALE = 0x0012,
        DSP_WR_ACB = 0x0013,
        DSP_WR_BEEPER = 0x0014,
        DSP_WR_I2S1_PRESCALE = 0x0016,
        DSP_WR_MODE_TONE_CTL = 0x0020,
        DSP_WR_LDSPK_EQ1 = 0x0021,
        DSP_WR_LDSPK_EQ2 = 0x0022,
        DSP_WR_LDSPK_EQ3 = 0x0023,
        DSP_WR_LDSPK_EQ4 = 0x0024,
        DSP_WR_LDSPK_EQ5 = 0x0025,
        DSP_WR_SUBWFR_LEVEL = 0x002C,
        DSP_WR_SUBWFR_FREQ = 0x002D,
        DSP_WR_HEADPH_BALANCE = 0x0030,
        DSP_WR_HEADPH_BASS = 0x0031,
        DSP_WR_HEADPH_TREBLE = 0x0032,
        DSP_WR_HEADPH_LOUDNESS = 0x0033,
        DSP_WR_SCART2_VOLUME = 0x0040,
        DSP_WR_SCART2_SOURCE = 0x0041,
        DSP_WR_MDB_STRENGTH = 0x0068,
        DSP_WR_MDB_APLITUDE = 0x0069,
        DSP_WR_MDB_HARMONIC = 0x006A,
        DSP_WR_MDB_LOPASS = 0x006B,
        DSP_WR_MDB_HIPASS = 0x006C,
        DSP_WR_FM_DEEMPH = 0x000F,
        DSP_WR_IDENT_MODE = 0x0015,
        DSP_WR_FM_FC_NOTCH = 0x0017,
		DSP_WR_AVC = 0x0029,
		DSP_WR_SURROUND_PROCESSING = 0x0048,
		DSP_WR_SURROUND_NOISE = 0x004D,
		DSP_WR_SURROUND_SPATIAL = 0x0049,
		DSP_WR_SURROUND_PANORAMA = 0x004A,
		DSP_WR_SURROUND_PANORAMA_MODE = 0x004B
    };
    void SetDSPRegister(eDSPWriteRegister reg, WORD value);

private:
    WORD GetRegister(BYTE subAddress, WORD reg);
    void SetRegister(BYTE subAddress, WORD reg, WORD value);
};

#endif // !defined(__MSP34X0_H__)
