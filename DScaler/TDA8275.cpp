/////////////////////////////////////////////////////////////////////////////
// $Id: TDA8275.cpp,v 1.4 2005-03-09 07:04:39 atnak Exp $
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
// Revision 1.3  2005/03/09 06:33:41  atnak
// More errors and omissions fixes.
//
// Revision 1.2  2005/03/09 06:11:46  atnak
// Fixed errors and omissions.
//
// Revision 1.1  2005/03/08 18:14:03  atnak
// Initial upload.
//
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA8290.cpp CTDA8290 Implementation
*/

#include "stdafx.h"
#include "TDA8275.h"
#include "TDA8290.h"


// TDA8275 frequency dependent parameters (prgTab)
const CTDA8275::tProgramingParam CTDA8275::k_programmingTable[] =
{
	// LOmin,	LOmax,	spd,	BS,		BP,		CP,		GC3,	div1p5
	{ 55,		62,		3,		2,		0,		0,		3,		1	},
	{ 62,		66,		3,		3,		0,		0,		3,		1	},
	{ 66,		76,		3,		1,		0,		0,		3,		0	},
	{ 76,		84,		3,		2,		0,		0,		3,		0	},
	{ 84,		93,		3,		2,		0,		0,		1,		0	},
	{ 93,		98,		3,		3,		0,		0,		1,		0	},
	{ 98,		109,	3,		3,		1,		0,		1,		0	},
	{ 109,		123,	2,		2,		1,		0,		1,		1	},
	{ 123,		133,	2,		3,		1,		0,		1,		1	},
	{ 133,		151,	2,		1,		1,		0,		1,		0	},
	{ 151,		154,	2,		2,		1,		0,		1,		0	},
	{ 154,		181,	2,		2,		1,		0,		0,		0	},
	{ 181,		185,	2,		2,		2,		0,		1,		0	},
	{ 185,		217,	2,		3,		2,		0,		1,		0	},
	{ 217,		244,	1,		2,		2,		0,		1,		1	},
	{ 244,		265,	1,		3,		2,		0,		1,		1	},
	{ 265,		302,	1,		1,		2,		0,		1,		0	},
	{ 302,		324,	1,		2,		2,		0,		1,		0	},
	{ 324,		370,	1,		2,		3,		0,		1,		0	},
	{ 370,		454,	1,		3,		3,		0,		1,		0	},
	{ 454,		493,	0,		2,		3,		0,		1,		1	},
	{ 493,		530,	0,		3,		3,		0,		1,		1	},
	{ 530,		554,	0,		1,		3,		0,		1,		0	},
	{ 554,		604,	0,		1,		4,		0,		0,		0	},
	{ 604,		696,	0,		2,		4,		0,		0,		0	},
	{ 696,		740,	0,		2,		4,		1,		0,		0	},
	{ 740,		820,	0,		3,		4,		0,		0,		0	},
	{ 820,		865,	0,		3,		4,		1,		0,		0	}
};

const CTDA8275::tStandardParam CTDA8275::k_standardParamTable[TDA8290_STANDARD_LASTONE] =
{
	// sgIFkHz, sgIFLPFilter
	{ 5750,	1 },
	{ 6750,	0 },
	{ 7750,	0 },
	{ 7750,	0 },
	{ 7750,	0 },
	{ 7750,	0 },
	{ 1250,	0 },
	{ 4750,	0 },  // FM Radio (this value is a guess)
};


CTDA8275::CTDA8275()
{
}

CTDA8275::~CTDA8275()
{
}


void CTDA8275::Attach(CI2CBus* i2cBus, BYTE address)
{
	CI2CDevice::Attach(i2cBus, address);

	// Initialize the tuner.
	InitializeTuner();
}

void CTDA8275::InitializeTuner()
{
	WriteTDA8275Initialization();
}

eVideoFormat CTDA8275::GetDefaultVideoFormat()
{
	return VIDEOFORMAT_NTSC_M;
}

bool CTDA8275::HasRadio() const
{
	return true;
}

bool CTDA8275::SetTVFrequency(long frequencyHz, eVideoFormat videoFormat)
{
	if (m_ExternalIFDemodulator != NULL)
	{
		m_ExternalIFDemodulator->TunerSet(TRUE, videoFormat);
	}

	bool success = SetFrequency(frequencyHz, CTDA8290::GetTDA8290Standard(videoFormat));
	m_Frequency = frequencyHz;

	if (m_ExternalIFDemodulator != NULL)
	{
		m_ExternalIFDemodulator->TunerSet(FALSE, videoFormat);
	}

	return success;
}

bool CTDA8275::SetRadioFrequency(long frequencyHz)
{
	// This tuning of radio has not been tested AT ALL;
	return SetTVFrequency(frequencyHz, (eVideoFormat)(VIDEOFORMAT_LASTONE+1));
}

long CTDA8275::GetFrequency()
{
	return m_Frequency;
}

eTunerLocked CTDA8275::IsLocked()
{
	BYTE status[3];

	if (ReadFromSubAddress(TDA8275_STATUS, status, 3))
	{
		// LOCK = bit 22
		return (status[2] & 0x40) != 0 ? TUNER_LOCK_ON : TUNER_LOCK_OFF;
	}

	return TUNER_LOCK_NOTSUPPORTED;
}

eTunerAFCStatus CTDA8275::GetAFCStatus(long &nFreqDeviation)
{
	if (m_ExternalIFDemodulator != NULL)
	{
		return m_ExternalIFDemodulator->GetAFCStatus(nFreqDeviation);
	}
	return TUNER_AFC_NOTSUPPORTED;
}

BYTE CTDA8275::GetDefaultAddress() const
{
	return I2C_ADDR_TDA8275_1;
}

void CTDA8275::WriteTDA8275Initialization()
{
	// 2 TDA827x Initialization
	WriteToSubAddress(TDA8275_DB1, 0x00);
	WriteToSubAddress(TDA8275_DB2, 0x00);
	WriteToSubAddress(TDA8275_DB3, 0x40);
	WriteToSubAddress(TDA8275_AB3, 0x2A);
	WriteToSubAddress(TDA8275_GB, 0xFF);
	WriteToSubAddress(TDA8275_TB, 0x00);
	WriteToSubAddress(TDA8275_SDB3, 0x00);
	WriteToSubAddress(TDA8275_SDB4, 0x40);

	/*
	// These values come from the "2/ TDA827x Initialization"
	// code in the data-sheet.  Those that weren't specified
	// there were substituted with default values from the
	// default column of the data-sheet.

	BYTE initializationBytes[13] = {
		// DB1, DB2, DB3, CB1
		0x00, 0x00, 0x40, 0x70,
		// BB, AB1, AB2, AB3
		0x00, 0x83, 0x3F, 0x2A,
		// AB4, GB, TB, SDB3, SDB4
		0x04, 0xFF, 0x00, 0x00, 0x40 };

	WriteToSubAddress(TDA8275_DB1, &initializationBytes, 13);
	*/
}

bool CTDA8275::SetFrequency(long frequencyHz, eTDA8290Standard standard)
{
	BYTE sgIFLPFilter = k_standardParamTable[(int)standard].sgIFLPFilter;
	LONG sgIFHz = k_standardParamTable[(int)standard].sgIFkHz * 1000;

	// sgRFHz + sgIFHz
	LONG freqRFIFHz = frequencyHz + sgIFHz;

	const tProgramingParam* row = k_programmingTable;
	const tProgramingParam* last = (const tProgramingParam*)((size_t)row + sizeof(k_programmingTable)) - 1;

	// Find the matching row of the programming table for this frequency.
	for ( ; row != last && freqRFIFHz > row->loMax * 1000000; row++) ;

	// The data-sheet says:
	// N11toN0=round((2^spd)*Flo*1000000*(16MHz/(2^6))
	//
	// Then uses this code to get the n11ton0 value:
	// lgN11toN0 = Round((2 ^ prgTab(c, 3)) * (sgRFMHz + sgIFMHz) * 1000000 / (16000000 / 2 ^ 6))
	//
	// Notice the discrepancy with division of (16MHz/2^6).  'prgTab(c, 3)' is row->spd,
	// (sgRFMHz + sgIFMHz) is (freqRFIFHz / 1000000).

	// 0.5 is added for rounding.
	WORD n11ton0 = (WORD)((double)(1 << row->spd) * ((double)freqRFIFHz / 250000) + 0.5);

	BYTE channelBytes[8];
	channelBytes[0] = (n11ton0 >> 6) & 0x3F;
	channelBytes[1] = (n11ton0 << 2) & 0xFC;
	channelBytes[3] = 0x40;
	channelBytes[4] = sgIFLPFilter ? 0x72 : 0x52; // 7MHz (US) / 9Mhz (Europe)
	channelBytes[5] = (row->spd << 6)|(row->div1p5 << 5)|(row->BS << 3)|row->BP;
	channelBytes[6] = 0x8F | (row->GC3 << 4);
	channelBytes[7] = 0x8F;

	if (!WriteToSubAddress(TDA8275_DB1, channelBytes, 8) ||
		!WriteToSubAddress(TDA8275_AB4, 0x00))
	{
		return false;
	}

	bool success = false;

	// 2.2 Re-initialize PLL and gain path
	success &= WriteToSubAddress(TDA8275_AB2, 0xBF);
	success &= WriteToSubAddress(TDA8275_CB1, 0xD2);
	Sleep(1);
	success &= WriteToSubAddress(TDA8275_CB1, 0x56);
	Sleep(1); // Only 550us required.
	success &= WriteToSubAddress(TDA8275_CB1, 0x52);
	Sleep(550); // 550ms delay required.
	success &= WriteToSubAddress(TDA8275_CB1, 0x50|row->CP);

	// 3 Enabling VSYNC mode for AGC2
	WriteToSubAddress(TDA8275_AB2, 0x7F);
	WriteToSubAddress(TDA8275_AB4, 0x08);
	return success;
}



