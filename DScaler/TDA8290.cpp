/////////////////////////////////////////////////////////////////////////////
// $Id: TDA8290.cpp,v 1.1 2005-03-07 09:21:13 atnak Exp $
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
// Sections in this code were ported from video4linux project's "tda8290c"
// file.  Copyright is unspecified.  List of contributers, as reported by
// CVS accounts follows: kraxel (revision 1.6)
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA8290.cpp CTDA8290 Implementation
*/

#include "stdafx.h"
#include "TDA8290.h"


CTDA8290::CTDA8290()
{
}

CTDA8290::~CTDA8290()
{
}

void CTDA8290::Init(bool bPreInit, eVideoFormat videoFormat)
{
	if (bPreInit)
	{
		// 4l2: "enable bridge" [docs: GPIO_1,2 deliver I2C clock and data]
		WriteToSubAddress(TDA8290_GPIO2, 0xC0);
	}
	else
	{
		// 4l2: "disable bridge" [docs: GPIO_1,2 are in tristate]
		WriteToSubAddress(TDA8290_GPIO2, 0x80);
		// V-Sync: position = 3 lines, polarity = negative first edge, width = 16 lines
		// (Setting comes from v4l2.)
		WriteToSubAddress(TDA8290_V_SYNC_DEL, 0x6F);
	}
}

void CTDA8290::TunerSet(bool bPreSet, eVideoFormat videoFormat)
{
	if (bPreSet)
	{
		SetAudioStandard(videoFormat);
		// Turn off 6 db gain.
		WriteToSubAddress(TDA8290_ADC, 0x14);
		// Soft reset.
		WriteToSubAddress(TDA8290_CLEAR, 0x00);
		// 4l2: "enable bridge"
		WriteToSubAddress(TDA8290_GPIO2, 0xC0);
	}
	else
	{
		// 4l2: "disable bridge"
		WriteToSubAddress(TDA8290_GPIO2, 0x80);
		// Turn on 6 db gain.
		WriteToSubAddress(TDA8290_ADC, 0x54);
	}
}

bool CTDA8290::Detect()
{
	// I don't know how to do this.  Maybe test for successful reads on
	// a random read registers and test for failed reads on a random
	// write registers.  The TDA8290 chip would probably always be coupled
	// with the TDA8275 chip so this might not be necessary.
	return true;
}

eTunerAFCStatus CTDA8290::GetAFCStatus(long &nFreqDeviation)
{
	// Bits: 0..6 := AFC readout, 7 := 1=PLL locked to picture carrier
	BYTE afc_reg = 0x00;

	if (ReadFromSubAddress(TDA8290_AFC_REG, &afc_reg, 1))
	{
		// I couldn't find it specified how to interpret the AFC readout.  It says in
		// the data-sheet:
		//
		// "- Wide TDA8290 AFC window (-840...830 kHz) and very accurate AFC information
		// (13.2 kHz step) provided via I2C for channel fine tuning."
		//
		// And:
		// "The second one is the high precise AFC readout (7 bit) which delivers the IF
		// frequency deviation from nominal.  The AFC information is valid at [-840...830 kHz]
		// around the nominal frequency, ..."
		//
		// I guess the 13.2 kHz step is a rounded approximation because (13.2 x 127 - 840kHz)
		// doesn't work out exactly to 830 kHz.  Using this reasoning, the step value is
		// (830 + 840) / 127 = 13.1496.......  I'm also guessing it's not in 2's complement
		// because there's no way to get step x readout to fall on zero.

		// Deviation in Hz: AFC readout x ((830kHz + 840kHz) / 127) - 840kHz.
		// (0.5 is added for rounding.)
		nFreqDeviation = (afc_reg & 0x7F) * ((double)1670000 / 127) - 840000 + 0.5;

		return (afc_reg & 0x80) != 0 ? TUNER_AFC_CARRIER : TUNER_AFC_NOCARRIER;
	}
	return TUNER_AFC_NOTSUPPORTED;
}

void CTDA8290::SetAudioStandard(eVideoFormat videoFormat)
{
	// Bits: 0..6 := standard, 7 := expert mode
	BYTE standard_reg = 0x01;

	switch (videoFormat)
	{
	case VIDEOFORMAT_PAL_B:
	case VIDEOFORMAT_SECAM_B:
		standard_reg = 0x02;	// B
		break;
	case VIDEOFORMAT_PAL_G:
	case VIDEOFORMAT_PAL_H:        
	case VIDEOFORMAT_PAL_N:
	case VIDEOFORMAT_SECAM_G:
	case VIDEOFORMAT_SECAM_H:
		standard_reg = 0x04;	// G/H
		break;
	case VIDEOFORMAT_PAL_I:
		standard_reg = 0x08;	// I
		break;
	case VIDEOFORMAT_PAL_D:
	case VIDEOFORMAT_SECAM_D:	
	case VIDEOFORMAT_SECAM_K:
	case VIDEOFORMAT_SECAM_K1:
		standard_reg = 0x10;	// D/K
		break;
	case VIDEOFORMAT_SECAM_L:
	case VIDEOFORMAT_SECAM_L1:
		standard_reg = 0x20;	// L
		break;
	case VIDEOFORMAT_PAL_60:    
		// Unsupported
		break;
	case VIDEOFORMAT_PAL_M:
	case VIDEOFORMAT_PAL_N_COMBO:
	case VIDEOFORMAT_NTSC_M:
		standard_reg = 0x01;	// M/N
		break;
	case VIDEOFORMAT_NTSC_50:
	case VIDEOFORMAT_NTSC_M_Japan:
		standard_reg = 0x01;	// M/N
		break;
	case (VIDEOFORMAT_LASTONE+1):
		// Radio: Unsupported
		break;
	}

	WriteToSubAddress(TDA8290_STANDARD_REG, standard_reg);
}

