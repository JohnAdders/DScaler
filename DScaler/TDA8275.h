/////////////////////////////////////////////////////////////////////////////
// $Id: TDA8275.h,v 1.3 2005-03-09 09:35:16 atnak Exp $
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
// Revision 1.2  2005/03/09 07:53:41  atnak
// Added the TDA8275 tuner.
//
// Revision 1.1  2005/03/08 18:14:03  atnak
// Initial upload.
//
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA8290.h CTDA8290 Header file
*/

#ifndef __TDA9875_H___
#define __TDA9875_H___

#include "ITuner.h"
#include "TDA8290.h"


// The I2C addresses for the TDA8275 chip.
// (These are standard tuner addresses.  a.k.a C0, C2, C4, C6)
#define I2C_ADDR_TDA8275_0		0x60
#define I2C_ADDR_TDA8275_1		0x61
#define I2C_ADDR_TDA8275_2		0x62
#define I2C_ADDR_TDA8275_3		0x63


// Subaddresses used by TDA8275.  Addresses marked R
// are read mode.  The rest are write mode.
#define TDA8275_STATUS						0x00	// R
#define TDA8275_DB1							0x00
#define TDA8275_DB2							0x10
#define TDA8275_DB3							0x20
#define TDA8275_CB1							0x30
#define TDA8275_BB							0x40
#define TDA8275_AB1							0x50
#define TDA8275_AB2							0x60
#define TDA8275_AB3							0x70
#define TDA8275_AB4							0x80
#define TDA8275_GB							0x90
#define TDA8275_TB							0xA0
#define TDA8275_SDB3						0xB0
#define TDA8275_SDB4						0xC0


class CTDA8275 : II2CTuner
{
public:
	CTDA8275();
	virtual ~CTDA8275();

	// from ITuner

	// Perform initializing calls on the tuner.
	virtual bool			InitializeTuner();

	// Gets this tuner's tuner ID.
	virtual eTunerId		GetTunerId() { return TUNER_TDA8275; }
	// Gets the video format that was specified as default on construction.
	virtual eVideoFormat	GetDefaultVideoFormat();
	// Gets whether or not this tuner supports radio tuning.
	virtual bool			HasRadio() const;
	// The tuner tunes into the specified frequency in TV mode with videoFormat.
	virtual bool			SetTVFrequency(long frequencyHz, eVideoFormat videoFormat);
	// The tuner tunes into the specified frequency for radio mode.
	virtual bool			SetRadioFrequency(long frequencyHz);
	// Gets the frequency to which the tuner is tuned into.
	virtual long			GetFrequency();

	// Returns whether or not PLL is locked onto a picture carrier.
	virtual eTunerLocked	IsLocked();
	// Gets the AFC status and frequency deviation.
	virtual eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);

protected:
	// from CI2CDevice

	// Gets the default I2C bus address for this tuner.
	virtual BYTE			GetDefaultAddress() const;

protected:
	// Writes the registers necessary for initialization.
	virtual void			WriteTDA8275Initialization();
	// Sets the tuner to tune into a specific frequency.
	virtual bool			SetFrequency(long frequencyHz, eTDA8290Standard standard);

	typedef struct
	{
		WORD	loMin;
		WORD	loMax;
		BYTE	spd;
		BYTE	BS;
		BYTE	BP;
		BYTE	CP;
		BYTE	GC3;
		BYTE	div1p5;
	} tProgramingParam;

	typedef struct
	{
		WORD	sgIFkHz;
		BYTE	sgIFLPFilter;
	} tStandardParam;

	static const tProgramingParam	k_programmingTable[];
	static const tStandardParam		k_standardParamTable[TDA8290_STANDARD_LASTONE];

private:
	long			m_Frequency;
};


#endif
