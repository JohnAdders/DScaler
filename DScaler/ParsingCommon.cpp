/////////////////////////////////////////////////////////////////////////////
// $Id: ParsingCommon.cpp,v 1.2 2004-11-27 19:07:43 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2004/11/27 01:00:54  atnak
// New file for storing common parsing procedures for ini file card lists.
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ParsingCommon.h"
#include "HierarchicalConfigParser.h"

using namespace HCParser;

static void SetTDA9887ModeMaskAndBits(OUT TTDA9887Modes&, IN BYTE, IN bool);


//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////

// It's not imperative that this list be updated when a new tuner
// is added but not updating it will mean the new tuner cannot be
// referred to by its name in the card list ini files.
const ParseConstant k_parseTunerConstants[] =
{
	{ "ABSENT",						PC_VALUE(0) },
	{ "PHILIPS_PAL_I",				PC_VALUE(1) },
	{ "PHILIPS_NTSC",				PC_VALUE(2) },
	{ "PHILIPS_SECAM",				PC_VALUE(3) },
	{ "PHILIPS_PAL",				PC_VALUE(4) },
	{ "TEMIC_4002FH5_PAL",			PC_VALUE(5) },
	{ "TEMIC_4032FY5_NTSC",			PC_VALUE(6) },
	{ "TEMIC_4062FY5_PAL_I",		PC_VALUE(7) },
	{ "TEMIC_4036FY5_NTSC",			PC_VALUE(8) },
	{ "ALPS_TSBH1_NTSC",			PC_VALUE(9) },
	{ "ALPS_TSBE1_PAL",				PC_VALUE(10) },
	{ "ALPS_TSBB5_PAL_I",			PC_VALUE(11) },
	{ "ALPS_TSBE5_PAL",				PC_VALUE(12) },
	{ "ALPS_TSBC5_PAL",				PC_VALUE(13) },
	{ "TEMIC_4006FH5_PAL",			PC_VALUE(14) },
	{ "PHILIPS_1236D_NTSC_INPUT1",	PC_VALUE(15) },
	{ "PHILIPS_1236D_NTSC_INPUT2",	PC_VALUE(16) },
	{ "ALPS_TSCH6_NTSC",			PC_VALUE(17) },
	{ "TEMIC_4016FY5_PAL",			PC_VALUE(18) },
	{ "PHILIPS_MK2_NTSC",			PC_VALUE(19) },
	{ "TEMIC_4066FY5_PAL_I",		PC_VALUE(20) },
	{ "TEMIC_4006FN5_PAL",			PC_VALUE(21) },
	{ "TEMIC_4009FR5_PAL",			PC_VALUE(22) },
	{ "TEMIC_4039FR5_NTSC",			PC_VALUE(23) },
	{ "TEMIC_4046FM5_MULTI",		PC_VALUE(24) },
	{ "PHILIPS_PAL_DK",				PC_VALUE(25) },
	{ "PHILIPS_MULTI",				PC_VALUE(26) },
	{ "LG_I001D_PAL_I",				PC_VALUE(27) },
	{ "LG_I701D_PAL_I",				PC_VALUE(28) },
	{ "LG_R01F_NTSC",				PC_VALUE(29) },
	{ "LG_B01D_PAL",				PC_VALUE(30) },
	{ "LG_B11D_PAL",				PC_VALUE(31) },
	{ "TEMIC_4009FN5_PAL",			PC_VALUE(32) },
	{ "MT2032",						PC_VALUE(33) },
	{ "SHARP_2U5JF5540_NTSC",		PC_VALUE(34) },
	{ "LG_TAPCH701P_NTSC",			PC_VALUE(35) },
	{ "SAMSUNG_PAL_TCPM9091PD27",	PC_VALUE(36) },
	{ "TEMIC_4106FH5",				PC_VALUE(37) },
	{ "TEMIC_4012FY5",				PC_VALUE(38) },
	{ "TEMIC_4136FY5",				PC_VALUE(39) },
	{ "LG_TAPCNEW_PAL",				PC_VALUE(40) },
	{ "PHILIPS_FM1216ME_MK3",		PC_VALUE(41) },
	{ "LG_TAPCNEW_NTSC",			PC_VALUE(42) },
	{ "MT2032_PAL",					PC_VALUE(43) },
	{ "PHILIPS_FI1286_NTSC_M_J",	PC_VALUE(44) },
	{ "MT2050",						PC_VALUE(45) },
	{ "MT2050_PAL",					PC_VALUE(46) },
	{ "PHILIPS_4IN1",				PC_VALUE(47) },
	{ NULL }
};

const ParseConstant k_parseTDAFormatConstants[] =
{
	{ "PAL-BG",						PC_VALUE(TDA9887_FORMAT_PAL_BG)		},
	{ "PAL-I",						PC_VALUE(TDA9887_FORMAT_PAL_I)		},
	{ "PAL-DK",						PC_VALUE(TDA9887_FORMAT_PAL_DK)		},
	{ "PAL-MN",						PC_VALUE(TDA9887_FORMAT_PAL_MN)		},
	{ "SECAM-L",					PC_VALUE(TDA9887_FORMAT_SECAM_L)	},
	{ "SECAM-DK",					PC_VALUE(TDA9887_FORMAT_SECAM_DK)	},
	{ "NTSC-M",						PC_VALUE(TDA9887_FORMAT_NTSC_M)		},
	{ "NTSC-JP",					PC_VALUE(TDA9887_FORMAT_NTSC_JP)	},
	{ "Radio",						PC_VALUE(TDA9887_FORMAT_RADIO)		},
	{ NULL }
};

const ParseConstant k_parseDemodulationConstants[] =
{
	{ "intercarrier",				PC_VALUE(0) },
	{ "qss",						PC_VALUE(1) },
	{ NULL }
};

const ParseConstant k_parseYesNoConstants[] =
{
	{ "yes",						PC_VALUE(1) },
	{ "no",							PC_VALUE(0) },
	{ "true",						PC_VALUE(1) },
	{ "false",						PC_VALUE(0) },
	{ "active",						PC_VALUE(1) },
	{ "inactive",					PC_VALUE(0) },
	{ "1",							PC_VALUE(1) },
	{ "0",							PC_VALUE(0) },
	{ NULL }
};

const ParseConstant k_parseTakeOverPointConstants[] =
{
	{ "min",						PC_VALUE(0x00) },
	{ "max",						PC_VALUE(0x1f) },
	{ "default",					PC_VALUE(0x10) },
	{ NULL }
};

const ParseTag k_parseUseTDA9887SetOverride[] =
{
	{ "Format",			PARSE_CONSTANT,					1, 8, NULL, k_parseTDAFormatConstants, PASS_TO_PARENT },
	{ "Intercarrier",	0,								0, 0, NULL, NULL, PASS_TO_PARENT },
	{ "QSS",			0,								0, 0, NULL, NULL, PASS_TO_PARENT },
	{ "Demodulation",	PARSE_CONSTANT,					0, 16, NULL, k_parseDemodulationConstants, PASS_TO_PARENT },
	{ "OutputPort1",	PARSE_CONSTANT,					0, 8, NULL, k_parseYesNoConstants, PASS_TO_PARENT },
	{ "OutputPort2",	PARSE_CONSTANT,					0, 8, NULL, k_parseYesNoConstants, PASS_TO_PARENT },
	{ "TakeOverPoint",	PARSE_NUMERIC|PARSE_CONSTANT,	0, 8, NULL, k_parseTakeOverPointConstants, PASS_TO_PARENT },
	{ NULL }
};

const ParseTag k_parseUseTDA9887[] =
{
	{ "Use",			PARSE_CONSTANT,					0, 8, NULL, k_parseYesNoConstants, PASS_TO_PARENT },
	{ "SetModes",		PARSE_CHILDREN,					0, 9, k_parseUseTDA9887SetOverride, NULL, PASS_TO_PARENT },
	{ NULL }
};


//////////////////////////////////////////////////////////////////////////
// Interpreters
//////////////////////////////////////////////////////////////////////////

BOOL ReadTunerProc(IN int report, IN const ParseTag* tag, IN unsigned char type,
				   IN const char* value, IN OUT TParseTunerInfo* tunerInfo)
{
	if (report == REPORT_OPEN)
	{
		// Set the default tuner id for if there is no value.
		tunerInfo->tunerId = TUNER_ABSENT;
	}
	else if (report == REPORT_VALUE)
	{
		if (type & PARSE_CONSTANT)
		{
			tunerInfo->tunerId = (eTunerId)reinterpret_cast<int>(value);
		}
		else
		{
			long l = CHCParser::Str2Long(value);

			if (l < 0 || l >= TUNER_LASTONE)
			{
				throw string("Invalid tuner Id");
			}
			tunerInfo->tunerId = (eTunerId)l;
		}
	}
	else if (report == REPORT_CLOSE)
	{
		// The value could be considered read at the end of REPORT_VALUE
		// but putting the 'return TRUE' here is also good.
		return TRUE;
	}
	return FALSE;
}


BOOL ReadUseTDA9887Proc(IN int report, IN const ParseTag* tag, IN unsigned char type,
						IN const char* value, IN OUT TParseUseTDA9887Info* useTDA9887Info)
{
	// Use
	if (tag == k_parseUseTDA9887 + 0)
	{
		if (report == REPORT_VALUE)
		{
			useTDA9887Info->useTDA9887 = reinterpret_cast<int>(value) != 0;
		}
	}
	// SetOverride
	else if (tag == k_parseUseTDA9887 + 1)
	{
		if (report == REPORT_OPEN)
		{
			useTDA9887Info->_readingFormat = TDA9887_FORMAT_NONE;
			useTDA9887Info->_readingModes.mask = 0;
			useTDA9887Info->_readingModes.bits = 0;
		}
		else if (report == REPORT_CLOSE)
		{
			// This should not fail if HCParser enforces the minimum limit.
			ASSERT(useTDA9887Info->_readingFormat != TDA9887_FORMAT_NONE);
			// This should not fail because constants only provide 0 .. TDA9887_LASTFORMAT.
			ASSERT(useTDA9887Info->_readingFormat >= 0 &&
				useTDA9887Info->_readingFormat < TDA9887_FORMAT_LASTONE);

			// It is pointless copying if mask is zero.
			if (useTDA9887Info->_readingModes.mask != 0)
			{
				memcpy(&useTDA9887Info->tdaModes[useTDA9887Info->_readingFormat],
					&useTDA9887Info->_readingModes, sizeof(TTDA9887Modes));
			}
		}
	}
	// Format
	else if (tag == k_parseUseTDA9887SetOverride + 0)
	{
		if (report == REPORT_VALUE)
		{
			useTDA9887Info->_readingFormat = (eTDA9887Format)reinterpret_cast<int>(value);
		}
	}
	// Intercarrier
	else if (tag == k_parseUseTDA9887SetOverride + 1)
	{
		if (report == REPORT_TAG)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_CARRIER_QSS, FALSE);
		}
	}
	// QSS
	else if (tag == k_parseUseTDA9887SetOverride + 2)
	{
		if (report == REPORT_TAG)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_CARRIER_QSS, TRUE);
		}
	}
	// Demodulation
	else if (tag == k_parseUseTDA9887SetOverride + 3)
	{
		if (report == REPORT_VALUE)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_CARRIER_QSS, reinterpret_cast<int>(value) != 0);
		}
	}
	// OutputPort1
	else if (tag == k_parseUseTDA9887SetOverride + 4)
	{
		if (report == REPORT_VALUE)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_OUTPUTPORT1_INACTIVE, reinterpret_cast<int>(value) != 0);
		}
	}
	// OutputPort2
	else if (tag == k_parseUseTDA9887SetOverride + 5)
	{
		if (report == REPORT_VALUE)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_OUTPUTPORT2_INACTIVE, reinterpret_cast<int>(value) != 0);
		}
	}
	// TakeOverPoint
	else if (tag == k_parseUseTDA9887SetOverride + 6)
	{
		if (report == REPORT_VALUE)
		{
			BYTE point = (BYTE)((type & PARSE_CONSTANT) ?
				reinterpret_cast<BYTE>(value) : CHCParser::Str2Long(value));

			if (point & ~TDA9887_SM_TAKEOVERPOINT_MASK)
			{
				throw string("Invalid value of TakeOverPoint");
			}

			useTDA9887Info->_readingModes.mask |= TDA9887_SM_TAKEOVERPOINT_MASK;
			useTDA9887Info->_readingModes.bits &= ~TDA9887_SM_TAKEOVERPOINT_MASK;
			useTDA9887Info->_readingModes.bits |= (point << TDA9887_SM_TAKEOVERPOINT_OFFSET);
		}
	}
	// This unknown tag is hopefully the parent tag.  There's no other way of
	// checking.  An incorrect tag here can only result from incorrect setup
	// outside ParsingCommon's control.
	else
	{
		if (report == REPORT_OPEN)
		{
			useTDA9887Info->useTDA9887 = TRUE;
			ZeroMemory(useTDA9887Info->tdaModes, sizeof(useTDA9887Info->tdaModes));
		}
		else if (report == REPORT_CLOSE)
		{
			// Everything about the UseTDA9887 tag is considered read and
			// ParseUseTDA9887Info considered fill at this point.
			return TRUE;
		}
	}
	return FALSE;
}

static void SetTDA9887ModeMaskAndBits(OUT TTDA9887Modes& specifics, IN BYTE bit, IN bool set)
{
	specifics.mask |= bit;
	if (set)
	{
		specifics.bits |= bit;
	}
	else
	{
		specifics.bits &= ~bit;
	}
}


