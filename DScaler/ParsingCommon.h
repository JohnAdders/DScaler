/////////////////////////////////////////////////////////////////////////////
// $Id: ParsingCommon.h,v 1.3 2004-12-01 22:01:18 atnak Exp $
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
// Revision 1.2  2004/12/01 17:57:08  atnak
// Updates to HierarchicalConfigParser.
//
// Revision 1.1  2004/11/27 01:00:54  atnak
// New file for storing common parsing procedures for ini file card lists.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __PARSINGCOMMON_H___
#define __PARSINGCOMMON_H___

#include "HierarchicalConfigParser.h"
#include "TunerID.h"
#include "TDA9887.h"

//
// TunerID parsing
//
// ParseTag:
//   { "Tuner", PARSE_NUMERIC|PARSE_CONSTANT, ..., 32, ..., k_parseTunerConstants, ... }
//
// Interpreter:
//   ReadTunerProc(..., ParseTunerInfo* tunerInfo)
//
//
// UseTDA9887 parsing
//
// ParseTag:
//   { "UseTDA9887", PARSE_CHILDREN, ..., 1, k_parseUseTDA9887, ..., ... }
//
// Interpreter:
//   ReadUseTDA9887Proc(..., ParseUseTDA9887Info* useTDA9887Info)
//

//////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////

extern const HCParser::CParseConstant k_parseTunerConstants[];
extern const HCParser::CParseTag k_parseUseTDA9887[];


//////////////////////////////////////////////////////////////////////////
// Structures
//////////////////////////////////////////////////////////////////////////

typedef struct _ParseTunerInfo TParseTunerInfo;
typedef struct _ParseUseTDA9887Info TParseUseTDA9887Info;

struct _ParseTunerInfo
{
	eTunerId	tunerId;
};

struct _ParseUseTDA9887Info
{
	BOOL					useTDA9887;
	TTDA9887Modes			tdaModes[TDA9887_FORMAT_LASTONE];
	eTDA9887Format			_readingFormat;
	TTDA9887Modes			_readingModes;
};


//////////////////////////////////////////////////////////////////////////
// Interpreters
//////////////////////////////////////////////////////////////////////////

BOOL ReadTunerProc(IN int report, IN const HCParser::CParseTag* tag, IN unsigned char type,
				   IN const HCParser::CParseValue* value, IN OUT TParseTunerInfo* tunerInfo);

BOOL ReadUseTDA9887Proc(IN int report, IN const HCParser::CParseTag* tag, IN unsigned char type,
						IN const HCParser::CParseValue* value, IN OUT TParseUseTDA9887Info* useTDA9887Info);


#endif

