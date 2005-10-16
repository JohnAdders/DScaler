/////////////////////////////////////////////////////////////////////////////
// $Id: TDA10046.h,v 1.2 2005-10-16 17:02:58 kelddamsbo Exp $
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Keld Damsbo.  All rights reserved.
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
// Revision 1.1  2005/10/15 20:01:54  kelddamsbo
// Initial Upload
//
// Revision 1.0  2005/10/09 17:59:00  kdamsbo
// Initial upload.
//
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA10046.h CTDA10046 Header file
*/

#ifndef __TDA10046_H___
#define __TDA10046_H___

#include "ITuner.h"

// The I2C addresses for the TDA10046 chip.
#define I2C_ADDR_TDA10046_0		0x08
#define I2C_ADDR_TDA10046_1		0x09
#define I2C_ADDR_TDA10046_2		0x0A
#define I2C_ADDR_TDA10046_3		0x0B

#define TDA10046_IDENTITY		0x00
#define TDA10046_AUTO 			0x01
#define TDA10046_IN_CONF1 		0x02
#define TDA10046_IN_CONF2 		0x03
#define TDA10046_OUT_CONF1 		0x04
#define TDA10046_OUT_CONF2 		0x05
#define TDA10046_SYNC_STATUS 		0x06
#define TDA10046_CONF_C4 		0x07
#define TDA10046_CELL_ID_MSB 		0x08
#define TDA10046_CELL_ID_LSB 		0x09
#define TDA10046_TIME_ERROR 		0x0A
#define TDA10046_GPIO_SP_DS0 		0x0B
#define TDA10046_GPIO_SP_DS1 		0x0C

#define TDA10046_SCAN_CPT 		0x10
#define TDA10046_DSP_AD_MSB 		0x11
#define TDA10046_DSP_AD_LSB 		0x12
#define TDA10046_DSP_REG_MSB 		0x13
#define TDA10046_DSP_REG_LSB 		0x14

#define TDA10046_CONF_C1 		0x16

#define TDA10046_FREQ_ERROR_MSB		0x18
#define TDA10046_FREQ_ERROR_LSB		0x19
#define TDA10046_AGC_TUN_LEVEL 		0x1A
#define TDA10046_CONF_C2C3 		0x1B
#define TDA10046_NP_OUT			0x1C

#define TDA10046_CONF_TS1 		0x1E
#define TDA10046_CONF_TS2 		0x1F

#define TDA10046_CBER_MSB 		0x21
#define TDA10046_CBER_LSB 		0x22
#define TDA10046_CVBER_LUT 		0x23
#define TDA10046_VBER_MSB 		0x24
#define TDA10046_VBER_MID 		0x25
#define TDA10046_VBER_LSB 		0x26
#define TDA10046_UNCOR_CPT		0x27

#define TDA10046_IT_SEL			0x29
#define TDA10046_IT_STAT		0x2A

#define TDA10046_CONF_PLL1 		0x2D
#define TDA10046_CONF_PLL2 		0x2F
#define TDA10046_CONF_PLL3 		0x30
#define TDA10046_TIME_WREF_1 		0x31
#define TDA10046_TIME_WREF_2 		0x32
#define TDA10046_TIME_WREF_3 		0x33
#define TDA10046_TIME_WREF_4 		0x34
#define TDA10046_TIME_WREF_5 		0x35
#define TDA10046_TPS_LENGTH 		0x36
#define TDA10046_CONF_ADC 		0x37

#define TDA10046_IC_MODE 		0x3A
#define TDA10046_CONF_TRISTATE1 	0x3B
#define TDA10046_CONF_TRISTATE2 	0x3C
#define TDA10046_CONF_POLARITY		0x3D
#define TDA10046_FREQ_OFFSET 		0x3E
#define TDA10046_GPIO_SP_DS2 		0x3F
#define TDA10046_GPIO_SP_DS3 		0x40
#define TDA10046_GPIO_OUT_SEL 		0x41
#define TDA10046_GPIO_SELECT 		0x42
#define TDA10046_AGC_CONF 		0x43
#define TDA10046_AGC_THRESHOLD		0x44
#define TDA10046_AGC_RENORM		0x45
#define TDA10046_AGC_GAINS 		0x46
#define TDA10046_AGC_TUN_MIN 		0x47
#define TDA10046_AGC_TUN_MAX		0x48
#define TDA10046_AGC_IF_MIN		0x49
#define TDA10046_AGC_IF_MAX		0x4A

#define TDA10046_CONF_MISC		0x4C
#define TDA10046_FREQ_PHY2_MSB		0x4D
#define TDA10046_FREQ_PHY2_LSB		0x4E
#define TDA10046_CVBER_CTRL		0x4F
#define TDA10046_CHANNEL_INFO1		0x50
#define TDA10046_CHANNEL_INFO2		0x51
#define TDA10046_AGC_IF_LEVEL		0x52
#define TDA10046_EXTTPS_ODD		0x53
#define TDA10046_EXTTPS_EVEN		0x54
#define TDA10046_DIG_AGC_LEVEL		0x55
#define TDA10046_TEST			0x56
#define TDA10046_DSP_CODE_CPT		0x57
#define TDA10046_DSP_CODE_IN		0x58


class CTDA10046 : public CI2CDevice


protected:
	// from CI2CDevice

	// Gets the default I2C bus address for this tuner.
	virtual BYTE			GetDefaultAddress() const;


protected:
	// Writes the registers necessary for initialization.
	virtual void			WriteTDA10046Initialization();

	virtual void			EnableTuner();
	virtual void			DisableTuner();
