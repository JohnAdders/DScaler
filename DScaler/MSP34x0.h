//
// $Id$
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

/**
 * @file msp34x0.h msp34x0 Header
 */

#ifdef WANT_BT8X8_SUPPORT

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
        DEM_RD_MODUS = 0x0030,
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

#endif // WANT_BT8X8_SUPPORT
