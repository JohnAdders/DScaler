/////////////////////////////////////////////////////////////////////////////
// $Id: ParsingCommon.cpp,v 1.11 2004-12-29 20:17:19 to_see Exp $
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
// Revision 1.10  2004/12/17 00:22:51  atnak
// Added a keyword for setting tda9887 modes for all formats.
//
// Revision 1.9  2004/12/16 04:53:50  atnak
// Added "auto detect" and "user setup" to tuner parsing.
//
// Revision 1.8  2004/12/12 11:46:23  atnak
// Fixes bug with incorrect OutputPort active parsing.
//
// Revision 1.7  2004/12/08 21:25:21  atnak
// Minor changes.
//
// Revision 1.6  2004/12/01 22:01:17  atnak
// Fix the VC++ 6 incompatibility introduced by last change.
//
// Revision 1.5  2004/12/01 17:57:08  atnak
// Updates to HierarchicalConfigParser.
//
// Revision 1.4  2004/11/27 19:26:33  atnak
// Minor changes.
//
// Revision 1.3  2004/11/27 19:23:39  atnak
// Changed more constant names.
//
// Revision 1.2  2004/11/27 19:07:43  atnak
// Changed constant to more correct name.
//
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
const CParseConstant k_parseTunerConstants[] =
{
	PC( "AUTO",							TUNER_AUTODETECT				),
	PC( "SETUP",						TUNER_USER_SETUP				),
	PC( "ABSENT",						TUNER_ABSENT					),
	PC( "PHILIPS_PAL_I",				TUNER_PHILIPS_PAL_I				),
	PC( "PHILIPS_NTSC",					TUNER_PHILIPS_NTSC				),
	PC( "PHILIPS_SECAM",				TUNER_PHILIPS_SECAM				),
	PC( "PHILIPS_PAL",					TUNER_PHILIPS_PAL				),
	PC( "TEMIC_4002FH5_PAL",			TUNER_TEMIC_4002FH5_PAL			),
	PC( "TEMIC_4032FY5_NTSC",			TUNER_TEMIC_4032FY5_NTSC		),
	PC( "TEMIC_4062FY5_PAL_I",			TUNER_TEMIC_4062FY5_PAL_I		),
	PC( "TEMIC_4036FY5_NTSC",			TUNER_TEMIC_4036FY5_NTSC		),
	PC( "ALPS_TSBH1_NTSC",				TUNER_ALPS_TSBH1_NTSC			),
	PC( "ALPS_TSBE1_PAL",				TUNER_ALPS_TSBE1_PAL			),
	PC( "ALPS_TSBB5_PAL_I",				TUNER_ALPS_TSBB5_PAL_I			),
	PC( "ALPS_TSBE5_PAL",				TUNER_ALPS_TSBE5_PAL			),
	PC( "ALPS_TSBC5_PAL",				TUNER_ALPS_TSBC5_PAL			),
	PC( "TEMIC_4006FH5_PAL",			TUNER_TEMIC_4006FH5_PAL			),
	PC( "PHILIPS_1236D_NTSC_INPUT1",	TUNER_PHILIPS_1236D_NTSC_INPUT1	),
	PC( "PHILIPS_1236D_NTSC_INPUT2",	TUNER_PHILIPS_1236D_NTSC_INPUT2	),
	PC( "ALPS_TSCH6_NTSC",				TUNER_ALPS_TSCH6_NTSC			),
	PC( "TEMIC_4016FY5_PAL",			TUNER_TEMIC_4016FY5_PAL			),
	PC( "PHILIPS_MK2_NTSC",				TUNER_PHILIPS_MK2_NTSC			),
	PC( "TEMIC_4066FY5_PAL_I",			TUNER_TEMIC_4066FY5_PAL_I		),
	PC( "TEMIC_4006FN5_PAL",			TUNER_TEMIC_4006FN5_PAL			),
	PC( "TEMIC_4009FR5_PAL",			TUNER_TEMIC_4009FR5_PAL			),
	PC( "TEMIC_4039FR5_NTSC",			TUNER_TEMIC_4039FR5_NTSC		),
	PC( "TEMIC_4046FM5_MULTI",			TUNER_TEMIC_4046FM5_MULTI		),
	PC( "PHILIPS_PAL_DK",				TUNER_PHILIPS_PAL_DK			),
	PC( "PHILIPS_MULTI",				TUNER_PHILIPS_MULTI				),
	PC( "LG_I001D_PAL_I",				TUNER_LG_I001D_PAL_I			),
	PC( "LG_I701D_PAL_I",				TUNER_LG_I701D_PAL_I			),
	PC( "LG_R01F_NTSC",					TUNER_LG_R01F_NTSC				),
	PC( "LG_B01D_PAL",					TUNER_LG_B01D_PAL				),
	PC( "LG_B11D_PAL",					TUNER_LG_B11D_PAL				),
	PC( "TEMIC_4009FN5_PAL",			TUNER_TEMIC_4009FN5_PAL			),
	PC( "MT2032",						TUNER_MT2032					),
	PC( "SHARP_2U5JF5540_NTSC",			TUNER_SHARP_2U5JF5540_NTSC		),
	PC( "LG_TAPCH701P_NTSC",			TUNER_LG_TAPCH701P_NTSC			),
	PC( "SAMSUNG_PAL_TCPM9091PD27",		TUNER_SAMSUNG_PAL_TCPM9091PD27	),
	PC( "TEMIC_4106FH5",				TUNER_TEMIC_4106FH5				),
	PC( "TEMIC_4012FY5",				TUNER_TEMIC_4012FY5				),
	PC( "TEMIC_4136FY5",				TUNER_TEMIC_4136FY5				),
	PC( "LG_TAPCNEW_PAL",				TUNER_LG_TAPCNEW_PAL			),
	PC( "PHILIPS_FM1216ME_MK3",			TUNER_PHILIPS_FM1216ME_MK3		),
	PC( "LG_TAPCNEW_NTSC",				TUNER_LG_TAPCNEW_NTSC			),
	PC( "MT2032_PAL",					TUNER_MT2032_PAL				),
	PC( "PHILIPS_FI1286_NTSC_M_J",		TUNER_PHILIPS_FI1286_NTSC_M_J	),
	PC( "MT2050",						TUNER_MT2050					),
	PC( "MT2050_PAL",					TUNER_MT2050_PAL				),
	PC( "PHILIPS_4IN1",					TUNER_PHILIPS_4IN1				),
	PC( "TCL_2002N",					TUNER_TCL_2002N  				),
	PC( NULL )
};

const CParseConstant k_parseTDAFormatConstants[] =
{
	PC( "GLOBAL",						TDA9887_FORMAT_NONE			),
	PC( "PAL-BG",						TDA9887_FORMAT_PAL_BG		),
	PC( "PAL-I",						TDA9887_FORMAT_PAL_I		),
	PC( "PAL-DK",						TDA9887_FORMAT_PAL_DK		),
	PC( "PAL-MN",						TDA9887_FORMAT_PAL_MN		),
	PC( "SECAM-L",						TDA9887_FORMAT_SECAM_L		),
	PC( "SECAM-DK",						TDA9887_FORMAT_SECAM_DK		),
	PC( "NTSC-M",						TDA9887_FORMAT_NTSC_M		),
	PC( "NTSC-JP",						TDA9887_FORMAT_NTSC_JP		),
	PC( "Radio",						TDA9887_FORMAT_RADIO		),
	PC( NULL )
};

const CParseConstant k_parseCarrierConstants[] =
{
	PC( "intercarrier",				0 ),
	PC( "qss",						1 ),
	PC( NULL )
};

const CParseConstant k_parseYesNoConstants[] =
{
	PC( "yes",						1 ),
	PC( "no",						0 ),
	PC( "true",						1 ),
	PC( "false",					0 ),
	PC( "active",					1 ),
	PC( "inactive",					0 ),
	PC( "1",						1 ),
	PC( "0",						0 ),
	PC( NULL )
};

const CParseConstant k_parseTakeoverPointConstants[] =
{
	PC( "min",						TDA9887_SM_TAKEOVERPOINT_MIN		),
	PC( "max",						TDA9887_SM_TAKEOVERPOINT_MAX		),
	PC( "default",					TDA9887_SM_TAKEOVERPOINT_DEFAULT	),
	PC( NULL )
};

const CParseTag k_parseUseTDA9887SetOverride[] =
{
	PT( "Format",			PARSE_CONSTANT,		0, 8,	k_parseTDAFormatConstants,		PASS_TO_PARENT	),
	PT( "Intercarrier",		0,					0, 0,	NULL,							PASS_TO_PARENT	),
	PT( "QSS",				0,					0, 0,	NULL,							PASS_TO_PARENT	),
	PT( "Carrier",			PARSE_CONSTANT,		0, 16,	k_parseCarrierConstants,		PASS_TO_PARENT	),
	PT( "OutputPort1",		PARSE_CONSTANT,		0, 8,	k_parseYesNoConstants,			PASS_TO_PARENT	),
	PT( "OutputPort2",		PARSE_CONSTANT,		0, 8,	k_parseYesNoConstants,			PASS_TO_PARENT	),
	PT( "TakeOverPoint",	PARSE_NUM_OR_CONST,	0, 8,	k_parseTakeoverPointConstants,	PASS_TO_PARENT	),
	PT( NULL )
};

const CParseTag k_parseUseTDA9887[] =
{
	PT( "Use",				PARSE_CONSTANT,		0, 8,	k_parseYesNoConstants,			PASS_TO_PARENT	),
	PT( "SetModes",			PARSE_CHILDREN,		0, 9,	k_parseUseTDA9887SetOverride,	PASS_TO_PARENT	),
	PT( NULL )
};


//////////////////////////////////////////////////////////////////////////
// Interpreters
//////////////////////////////////////////////////////////////////////////

BOOL ReadTunerProc(IN int report, IN const CParseTag* tag, IN unsigned char type,
				   IN const CParseValue* value, IN OUT TParseTunerInfo* tunerInfo)
{
	if (report == REPORT_OPEN)
	{
		// Set the default tuner id for if there is no value.
		tunerInfo->tunerId = TUNER_ABSENT;
	}
	else if (report == REPORT_VALUE)
	{
		int n = value->GetNumber();
		if (n < TUNER_AUTODETECT || n >= TUNER_LASTONE)
		{
			throw string("Invalid tuner Id");
		}
		tunerInfo->tunerId = static_cast<eTunerId>(n);
	}
	else if (report == REPORT_CLOSE)
	{
		// The value could be considered read at the end of REPORT_VALUE
		// but putting the 'return TRUE' here is also good.
		return TRUE;
	}
	return FALSE;
}


BOOL ReadUseTDA9887Proc(IN int report, IN const CParseTag* tag, IN unsigned char type,
						IN const CParseValue* value, IN OUT TParseUseTDA9887Info* useTDA9887Info)
{
	// Use
	if (tag == k_parseUseTDA9887 + 0)
	{
		if (report == REPORT_VALUE)
		{
			useTDA9887Info->useTDA9887 = value->GetNumber() != 0;
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
			// This should not fail because constants only provide 0 .. TDA9887_LASTFORMAT.
			ASSERT(useTDA9887Info->_readingFormat == TDA9887_FORMAT_NONE ||
				(useTDA9887Info->_readingFormat >= 0 &&
				useTDA9887Info->_readingFormat < TDA9887_FORMAT_LASTONE));

			// It is pointless copying if mask is zero.
			if (useTDA9887Info->_readingModes.mask != 0)
			{
				if (useTDA9887Info->_readingFormat == TDA9887_FORMAT_NONE)
				{
					// Copy for all formats.
					for (int i = 0; i < TDA9887_FORMAT_LASTONE; i++)
					{
						useTDA9887Info->tdaModes[i].bits &= ~useTDA9887Info->_readingModes.mask;
						useTDA9887Info->tdaModes[i].bits |= useTDA9887Info->_readingModes.bits;
						useTDA9887Info->tdaModes[i].mask |= useTDA9887Info->_readingModes.mask;
					}
				}
				else
				{
					int i = useTDA9887Info->_readingFormat;
					useTDA9887Info->tdaModes[i].bits &= ~useTDA9887Info->_readingModes.mask;
					useTDA9887Info->tdaModes[i].bits |= useTDA9887Info->_readingModes.bits;
					useTDA9887Info->tdaModes[i].mask |= useTDA9887Info->_readingModes.mask;
				}
			}
		}
	}
	// Format
	else if (tag == k_parseUseTDA9887SetOverride + 0)
	{
		if (report == REPORT_VALUE)
		{
			useTDA9887Info->_readingFormat = static_cast<eTDA9887Format>(value->GetNumber());
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
	// Carrier
	else if (tag == k_parseUseTDA9887SetOverride + 3)
	{
		if (report == REPORT_VALUE)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_CARRIER_QSS, value->GetNumber() != 0);
		}
	}
	// OutputPort1
	else if (tag == k_parseUseTDA9887SetOverride + 4)
	{
		if (report == REPORT_VALUE)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_OUTPUTPORT1_INACTIVE, value->GetNumber() == 0);
		}
	}
	// OutputPort2
	else if (tag == k_parseUseTDA9887SetOverride + 5)
	{
		if (report == REPORT_VALUE)
		{
			SetTDA9887ModeMaskAndBits(useTDA9887Info->_readingModes,
				TDA9887_SM_OUTPUTPORT2_INACTIVE, value->GetNumber() == 0);
		}
	}
	// TakeOverPoint
	else if (tag == k_parseUseTDA9887SetOverride + 6)
	{
		if (report == REPORT_VALUE)
		{
			BYTE point = static_cast<BYTE>(value->GetNumber());
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


