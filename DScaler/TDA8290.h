/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Atsushi Nakagawa.  All rights reserved.
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
// Revision 1.3  2005/03/09 07:04:39  atnak
// Added code for radio.
//
// Revision 1.2  2005/03/08 18:12:17  atnak
// Updates.
//
// Revision 1.1  2005/03/07 09:21:13  atnak
// Initial checkin of TDA8290 files.  This class is coupled with TDA8275.
//
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA8290.h CTDA8290 Header file
*/

#ifndef __TDA9890_H___
#define __TDA9890_H___

#include "ITuner.h"


// The I2C address of the TDA8290 chip.
#define I2C_ADDR_TDA8290		0x4B


// Subaddresses used by TDA8290.  Addresses marked R
// are read mode.  The rest are write mode.
#define TDA8290_CLEAR						0x00
#define TDA8290_STANDARD_REG				0x01
#define TDA8290_DIV_FUNC					0x02
#define TDA8290_FILTERS_1					0x03
#define TDA8290_FILTERS_2					0x04
#define TDA8290_ADC_HEADR					0x05
#define TDA8290_GRP_DELAY					0x06
#define TDA8290_DTO_PC						0x07
#define TDA8290_PC_PLL_FUNC					0x08
#define TDA8290_AGC_FUNC					0x09
#define TDA8290_IF_AGC_SET					0x0F
#define TDA8290_T_AGC_FUNC					0x10
#define TDA8290_TOP_ADJUST_REG				0x11
#define TDA8290_SOUNDSET_1					0x13
#define TDA8290_SOUNDSET_2					0x14
// ... address defines skipped ...
#define TDA8290_ADC_SAT						0x1A	// R
#define TDA8290_AFC_REG						0x1B	// R
#define TDA8290_AGC_HVPLL					0x1C	// R
#define TDA8290_IF_AGC_STAT					0x1D	// R
#define TDA8290_T_AGC_STAT					0x1E	// R
#define TDA8290_IDENTITY					0x1F	// R
#define TDA8290_GPIO1						0x20
#define TDA8290_GPIO2						0x21
#define TDA8290_GPIO3						0x22
#define TDA8290_TEST						0x23
#define TDA8290_PLL1						0x24
#define TDA8290_PLL1R						0x24	// R
#define TDA8290_PLL2						0x25
#define TDA8290_PLL3						0x26
#define TDA8290_PLL4						0x27
#define TDA8290_ADC							0x28
// ... address defines skipped ...
#define TDA8290_PLL5						0x2C
#define TDA8290_PLL6						0x2D
#define TDA8290_PLL7						0x2E
#define TDA8290_PLL8						0x2F	//R
#define TDA8290_V_SYNC_DEL					0x30


enum eTDA8290Standard
{
	TDA8290_STANDARD_MN = 0,
	TDA8290_STANDARD_B,
	TDA8290_STANDARD_GH,
	TDA8290_STANDARD_I,
	TDA8290_STANDARD_DK,
	TDA8290_STANDARD_L,
	TDA8290_STANDARD_L2,
	TDA8290_STANDARD_RADIO,
	TDA8290_STANDARD_LASTONE
};


// This class is closely coupled with the TDA8275 class.
class CTDA8290 : public IExternalIFDemodulator
{
public:
	CTDA8290();
	virtual ~CTDA8290();

	// Return a CTDA8290 object or NULL if none is detected.
	static CTDA8290* CreateDetectedTDA8290(IN CI2CBus* i2cBus);

	// from IExternalIFDemodulator

	// This needs to be called once with bPreInit=TRUE before TDA8275
	// init and again with bPreInit=FALSE after TDA8275 init.
	virtual void Init(bool bPreInit, eVideoFormat videoFormat);
	// This needs to be called once with bPreSet=TRUE before TDA8275
	// tuner set and again with bPreSet=FALSE after TDA8275 tuner set.
	virtual void TunerSet(bool bPreSet, eVideoFormat videoFormat);

	// Returns true if TDA8290 is detected at the attached address.
	virtual bool Detect();
	// Returns whether or not the picture carrier is locked.
	virtual eTunerAFCStatus GetAFCStatus(OUT long &nFreqDeviation);

public:
	// Convert eVideoFormat to eTDA8290Standard.
	static eTDA8290Standard GetTDA8290Standard(eVideoFormat videoFormat);

protected:
	// from CI2CDevice
	virtual BYTE GetDefaultAddress() const { return I2C_ADDR_TDA8290; }

	// Programs the TDA8290 for an audio standard to match the video format.
	virtual void SetVideoSystemStandard(eTDA8290Standard standard);
};


#endif
