/////////////////////////////////////////////////////////////////////////////
// $Id: HierarchicalConfigParser.cpp,v 1.1 2004-11-19 23:51:04 atnak Exp $
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
//
//////////////////////////////////////////////////////////////////////////////

/** 
* @file HierarchicalConfigParser.h HierarchicalConfigParser Implementation
*/

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <crtdbg.h>
#include "HierarchicalConfigParser.h"

using namespace std;
using namespace HCParser;


CHCParser::CHCParser(const ParseTag* tagList)
{
	m_debugOutLevel	= DEBUG_OUT_NONE;
	m_reportContext	= NULL;
	m_tagListEntry	= tagList;
}

CHCParser::~CHCParser()
{
}

bool CHCParser::ParseLocalFile(const char* filename, void* reportContext)
{
	return ParseFile(filename, reportContext, true);
}

bool CHCParser::ParseLocalFile(const wchar_t* filename, void* reportContext)
{
	return ParseFile(filename, reportContext, true);
}

bool CHCParser::ParseFile(const char* filename, void* reportContext, bool localFile)
{
	FILE* fp = localFile ? OpenLocalFile(filename) : fopen(filename, "r");
	if (fp == NULL)
	{
		m_parseError.clear();
		SetParseError(FileError() << "Unable to open file '" << filename << "' for reading");
		return false;
	}

	bool success = ParseFile(fp, reportContext);
	fclose(fp);

	return success;
}

bool CHCParser::ParseFile(const wchar_t* filename, void* reportContext, bool localFile)
{
	FILE* fp = localFile ? OpenLocalFile(filename) : _wfopen(filename, L"r");
	if (fp == NULL)
	{
		m_parseError.clear();
		SetParseError(FileError() << "Unable to open file '" << filename << "' for reading");
		return false;
	}

	bool success = ParseFile(fp, reportContext);
	fclose(fp);

	return success;
}

bool CHCParser::ParseFile(FILE* fp, void* reportContext)
{
	m_parseError.clear();
	m_reportContext	= reportContext;
	m_lineNumber	= 0;
	m_linePoint		= NULL;

	bool success = ProcessStream(ifstream(fp));
	if (!success)
	{
#ifdef _DEBUG
		DebugOut(DEBUG_OUT_ERROR, "\n\n");
		DebugOut(DEBUG_OUT_ERROR, m_parseError);
#endif
	}

#ifdef _DEBUG
	DebugOut(DEBUG_OUT_ERROR, "\n");
#endif
	return success;
}

bool CHCParser::IsUnicodeOS()
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 3;

	// Windows NT (major version 3) and newer supports unicode.
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	return VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlConditionMask) != 0;
}

FILE* CHCParser::OpenLocalFile(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
	{
		if (IsUnicodeOS())
		{
			// If the OS uses unicode filenames, GetModuleFileNameW() should be
			// used so that path information is not lost.  Otherwise files with
			// unicode characters in the path will fail to open.  On the other hand,
			// GetModuleFileNameW() and _wfopen() will probably not work on OSes
			// that do not support unicode (95/98/Me).
			wchar_t wfilename[MAX_PATH];

			if (MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, MAX_PATH))
			{
				wchar_t* buffer = new wchar_t[MAX_PATH+wcslen(wfilename)];

				GetModuleFileNameW(NULL, buffer, MAX_PATH);
				wcscpy(wcsrchr(buffer, L'\\')+1, wfilename);

				fp = _wfopen(buffer, L"r");
				delete [] buffer;
			}
		}
		else
		{
			char* buffer = new char[MAX_PATH+strlen(filename)];

			GetModuleFileNameA(NULL, buffer, MAX_PATH);
			strcpy(strrchr(buffer, '\\')+1, filename);

			fp = fopen(buffer, "r");
			delete [] buffer;
		}
	}
	return fp;
}

FILE* CHCParser::OpenLocalFile(const wchar_t* filename)
{
	FILE* fp = _wfopen(filename, L"r");
	if (fp == NULL)
	{
		wchar_t* buffer = new wchar_t[MAX_PATH+wcslen(filename)];

		GetModuleFileNameW(NULL, buffer, MAX_PATH);
		wcscpy(wcsrchr(buffer, L'\\')+1, filename);

		fp = _wfopen(filename, L"r");
		delete [] buffer;
	}
	return fp;
}

string CHCParser::GetError()
{
	return m_parseError.str();
}

wstring CHCParser::GetErrorUnicode()
{
	return m_parseError.wstr();
}

long CHCParser::Str2Long(const char* text)
{
	const char* c = text;
	long l = 0;
	bool negative = false;

	if (*c == '-')
	{
		negative = true;
		c++;
	}

	if (*c == '0' && *++c == 'x')
	{
		while (*++c != '\0')
		{
			l *= 0x10;

			if (*c >= '0' && *c <= '9')
			{
				l += *c - '0';
			}
			else if (*c >= 'a' && *c <= 'f')
			{
				l += 0x10 + *c - 'a';
			}
			else if (*c >= 'A' && *c <= 'F')
			{
				l += 0x10 + *c - 'A';
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
		for ( ; *c != '\0'; c++)
		{
			if (*c >= '0' && *c <= '9')
			{
				l *= 10;
				l += *c - '0';
			}
			else
			{
				return 0;
			}
		}
	}

	return negative ? -l : l;
}

void CHCParser::TrimLeft()
{
	_ASSERT(m_linePoint != NULL);
	for ( ; IsSpace(*m_linePoint); m_linePoint++) ;
}

bool CHCParser::IsSpace(int c)
{
	return c == ' ' || c == '\t';
}

bool CHCParser::IsAlpha(int c)
{
	return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

bool CHCParser::IsAlnum(int c)
{
	return IsAlpha(c) || IsDigit(c) || c == '_' || c == '-';
}

bool CHCParser::IsDigit(int c)
{
	return c >= '0' && c <= '9';
}

bool CHCParser::IsHex(int c)
{
	return IsDigit(c) || c >= 'A' && c <= 'F' || c >= 'a' && c <= 'f';
}

bool CHCParser::IsDelim(int c)
{
	return c == ',' || c == ')' || c == '(' || c == '{' || c == '}' || c == '=';
}

void CHCParser::SetParseError(ParseError& error)
{
	if (!m_parseError.empty())
	{
		return;
	}

	if (error.type() == PARSE_ERROR_LINE || error.type() == PARSE_ERROR_POINT)
	{
		m_parseError << "Error on line " <<	m_lineNumber;
		if (error.type() == PARSE_ERROR_POINT && m_linePoint != NULL)
		{
			m_parseError << " character " << (long)(m_linePoint - m_lineBuffer)+1;
		}
	}
	else
	{
		m_parseError << "Error";
	}

	m_parseError << ": " << error.wstr();
}

bool CHCParser::ProcessStream(ifstream& ifs)
{
	InitializeParseState();

	while (1)
	{
		// Read one line into a buffer
		ifs.getline(m_lineBuffer, MAX_LINE_LENGTH);
		if (ifs.fail())
		{
			m_linePoint = NULL;
			if (ifs.gcount() == MAX_LINE_LENGTH-1)
			{
				SetParseError(LineError() << "Line is too long");
				return false;
			}
			break;
		}

		m_linePoint = m_lineBuffer;
		m_lineNumber++;
		TrimLeft();

		if (*m_linePoint == ';' || *m_linePoint == '#')
		{
			continue;
		}
		if (m_parseStates.front().expect & EXPECT_SECTION && *m_linePoint == '[')
		{
			if (!ProcessSection())
			{
				return false;
			}
		}
		if (!ProcessLine())
		{
			return false;
		}
	}

	if (m_parseStates.front().expect & (EXPECT_CLOSE_V|EXPECT_CLOSE_L))
	{
		if (m_parseStates.front().expect & EXPECT_CLOSE_V)
		{
			SetParseError(LineError() << "EOF while expecting ')'");
		}
		else
		{
			SetParseError(LineError() << "EOF while expecting '}'");
		}
		return false;
	}

	while (m_parseStates.size() > 1)
	{
		if (!CloseValue())
		{
			return false;
		}
	}

	// The root state ensures there's at least one open tag at the end.
	if (!CloseTag())
	{
		return false;
	}
	return true;
}

void CHCParser::DebugOut(int level, ParseError& error, bool appendExpect)
{
#ifdef _DEBUG
	if (level <= m_debugOutLevel)
	{
		ParseError pe;

		pe << error.wstr();

		if (appendExpect && m_debugOutLevel >= DEBUG_OUT_EXPECT)
		{
			pe << "[";
			AddExpectLine(pe, true);
			pe << "]";
		}
		cout << pe.str();
	}
#endif
}

void CHCParser::DebugOut(int level, const char* message, bool appendExpect)
{
#ifdef _DEBUG
	DebugOut(level, ParseError() << message, appendExpect);
#endif
}

bool CHCParser::ProcessLine()
{
	while (1)
	{
		TrimLeft();

		if (*m_linePoint == '\0')
		{
			break;
		}

		if ((m_parseStates.front().expect & EXPECT_TAG) && IsAlpha(*m_linePoint))
		{
			if (ProcessTag())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_OPEN_L) && *m_linePoint == '{')
		{
			m_linePoint++;
			if (ProcessOpenTagListBracket())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_CLOSE_L) && *m_linePoint == '}')
		{
			m_linePoint++;
			if (ProcessCloseTagListBracket())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_OPEN_V) && *m_linePoint == '(')
		{
			m_linePoint++;
			if (ProcessOpenValueBracket())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_CLOSE_V) && *m_linePoint == ')')
		{
			m_linePoint++;
			if (ProcessCloseValueBracket())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_EQUAL) && *m_linePoint == '=')
		{
			m_linePoint++;
			if (ProcessEqual())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_NEXT_COMMA) && *m_linePoint == ',')
		{
			m_linePoint++;
			if (ProcessComma())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_CLOSE_COMMA) && *m_linePoint == ',')
		{
			m_linePoint++;
			if (ProcessComma())
			{
				continue;
			}
		}
		if ((m_parseStates.front().expect & EXPECT_VALUE) && (IsAlnum(*m_linePoint) || *m_linePoint == '"'))
		{
			if (ProcessValue())
			{
				continue;
			}

			if (m_parseError.empty())
			{
				PointError pe;
				pe << "Given value not a valid";
				if (m_parseStates.front().parseTags->parseTypes & PARSE_STRING)
				{
					pe << " string";
				}
				if (m_parseStates.front().parseTags->parseTypes & PARSE_NUMERIC)
				{
					pe << " numeric";
				}
				if (m_parseStates.front().parseTags->parseTypes & PARSE_CONSTANT)
				{
					pe << " constant";
				}
				SetParseError(pe);
				return false;
			}
		}

		if (m_parseError.empty())
		{
			PointError pe;
			if (*m_linePoint < 0x20)
			{
				pe << "Unexpected character \\" << (int)*m_linePoint << " expecting ";
			}
			else
			{
				pe << "Unexpected character '" << *m_linePoint << "' expecting ";
			}

			AddExpectLine(pe);
			SetParseError(pe);
		}
		return false;
	}

#ifdef _DEBUG
	DebugOut(DEBUG_OUT_EXPECT, " EOL", true);
#endif
	if (m_parseStates.front().expect & EXPECT_CLOSE_EOL)
	{
		while (m_parseStates.front().passEOL)
		{
			if (!CloseValue())
			{
				return false;
			}
		}
		if (!CloseTag())
		{
			return false;
		}
	}
	return true;
}

bool CHCParser::TagTakesValues(const ParseTag* parseTag)
{
	if (parseTag->tagName == NULL)
	{
		return false;
	}

	if (parseTag->parseTypes & PARSE_CHILDREN)
	{
		for (long i = 0; parseTag->subTags[i].tagName != NULL; i++)
		{
			if (TagTakesValues(&parseTag->subTags[i]))
			{
				return true;
			}
		}
		return false;
	}
	return true;
}

long CHCParser::GetNextIterateValuesIndex()
{
	long paramIndex = m_parseStates.front().paramIndex;
	const ParseTag* parseTags = m_parseStates.front().parseTags;

	while (parseTags[++paramIndex].tagName != NULL)
	{
		if (TagTakesValues(&parseTags[paramIndex]))
		{
			break;
		}
	}

	if (parseTags[paramIndex].tagName == NULL)
	{
		return -1;
	}

	return paramIndex;
}

void CHCParser::AddExpectLine(ParseError& pe, bool debugging)
{
	const char* namesReadable[EXPECT_MAX] = { "", "tag-name", "",
		"value", "'='", "','", "'('", "')'", "'{'", "'}'", "EOL", "','" };
	const char* namesDebug[EXPECT_MAX] = { "se", "ta", "", "va", "eq",
		"co", "ov", "cv", "ol", "cl", "ec", "ce" };

	const char** names = debugging ? namesDebug : namesReadable;
	const char* comma = debugging ? "," : ", ";
	const char* lastComma = debugging ? "," : " or ";

	unsigned short expect = m_parseStates.front().expect;
	int last = -1;
	int count = 0;

	for (int i = 0; i < EXPECT_MAX; i++)
	{
		if (*(names[i]) != '\0' && expect & (1 << i))
		{
			if (last != -1)
			{
				if (count++)
				{
					pe << comma;
				}
				pe << names[last];
			}
			last = i;
		}
	}

	if (last != -1)
	{
		if (count++)
		{
			pe << lastComma;
		}
		pe << names[last];
	}
	else if (!debugging)
	{
		pe << " end-of-file";
	}
}

bool CHCParser::ProcessSection()
{
	while (m_parseStates.size() > 2)
	{
		if (!CloseValue())
		{
			return false;
		}
	}
	if (m_parseStates.front().paramIndex != -1)
	{
		if (!CloseTag())
		{
			return false;
		}
	}
	if (!OpenTag(0) || !OpenValue(false))
	{
		return false;
	}

	m_parseStates.front().expect = EXPECT_TAG|EXPECT_SECTION;

	char* parseStart = ++m_linePoint;
	for ( ; *m_linePoint != '\0' && *m_linePoint != ']'; m_linePoint++) ;

	if (*m_linePoint == '\0')
	{
		SetParseError(PointError() << "End of line before ']'");
		return false;
	}

	while (*(m_linePoint+1) != '\0')
	{
		char* lastPoint = m_linePoint;
		for (m_linePoint++; IsSpace(*m_linePoint); m_linePoint++) ;

		if (*m_linePoint == '\0')
		{
			m_linePoint = lastPoint;
			break;
		}

		for ( ; *m_linePoint != '\0' && *m_linePoint != ']'; m_linePoint++) ;

		if (*m_linePoint == '\0')
		{
			SetParseError(PointError() << "Trailing garbage after ']'");
			return false;
		}
	}

	// Don't progress past this point so that this the '\0' we add here
	// will be read again and this whole line will be taken as finished.
	*m_linePoint = '\0';

	if (!AcceptValue(m_parseStates.front().parseTags,
		m_parseStates.front().parseTags->parseTypes,
		parseStart, (unsigned long)(m_linePoint - parseStart)))
	{
		return false;
	}

	while (m_parseStates.size() > 2)
	{
		if (!CloseValue())
		{
			return false;
		}
	}

	return true;
}

bool CHCParser::ProcessTag()
{
	char* parseStart = m_linePoint;
	for (m_linePoint++; IsAlnum(*m_linePoint); m_linePoint++) ;

	if (!IsSpace(*m_linePoint) && !IsDelim(*m_linePoint) && *m_linePoint != '\0')
	{
		return false;
	}

	char delim = *m_linePoint;
	*m_linePoint = '\0';

	if (m_parseStates.front().paramIndex != -1)
	{
		if (!OpenTagList(false))
		{
			return false;
		}
	}

	const ParseTag* parseTag;

	while (1)
	{
		parseTag = m_parseStates.front().parseTags;
		for ( ; parseTag->tagName != NULL; parseTag++)
		{
			if (stricmp(parseTag->tagName, parseStart) == 0)
			{
				break;
			}
		}

		if (parseTag->tagName != NULL || m_parseStates.front().bracketOpened ||
			m_parseStates.size() == 2)
		{
			break;
		}

		if (!CloseValue() || !CloseTag())
		{
			return false;
		}
	}

	if (parseTag->tagName == NULL)
	{
		// unrecognized tag
		for (char* p = parseStart; *p != '\0'; p++)
		{
			if (*p < 0x20)
			{
				*p = '_';
			}
		}

		SetParseError(PointError() << "Unrecognized tag '" << parseStart << "'");
		return false;
	}
	*m_linePoint = delim;

#ifdef _DEBUG
	DebugOut(DEBUG_OUT_REPORT, "\n");
#endif
	return OpenTag((long)(parseTag - m_parseStates.front().parseTags));
}

bool CHCParser::ProcessOpenTagListBracket()
{
#ifdef _DEBUG
	DebugOut(DEBUG_OUT_EXPECT, " {");
#endif
	return OpenTagList(true);
}

bool CHCParser::ProcessCloseTagListBracket()
{
#ifdef _DEBUG
	DebugOut(DEBUG_OUT_EXPECT, " }");
#endif
	while (!m_parseStates.front().bracketOpened)
	{
		if (!CloseValue())
		{
			return false;
		}
	}
	return CloseValue() && CloseTag();
}

bool CHCParser::ProcessOpenValueBracket()
{
	DebugOut(DEBUG_OUT_EXPECT, " (");
	return OpenValue(true);
}

bool CHCParser::ProcessCloseValueBracket()
{
	DebugOut(DEBUG_OUT_EXPECT, " )");
	while (!m_parseStates.front().bracketOpened)
	{
		if (!CloseValue())
		{
			return false;
		}
	}

	return CloseValue();
}

bool CHCParser::ProcessComma()
{
	DebugOut(DEBUG_OUT_EXPECT, ",");
	while (1)
	{
		if (m_parseStates.front().paramIndex != -1)
		{
			if (m_parseStates.front().iterateValues)
			{
				long nextIndex = GetNextIterateValuesIndex();
				if (nextIndex != -1)
				{
					return OpenTag(nextIndex);
				}
				if (!CloseTag())
				{
					return false;
				}
			}
			else
			{
				return CloseTag();
			}
		}

		if (!CloseValue())
		{
			return false;
		}
	}
	return true;
}

bool CHCParser::ProcessEqual()
{
	DebugOut(DEBUG_OUT_EXPECT, " =");

	if (m_parseStates.front().expect & EXPECT_OPEN_V)
	{
		m_parseStates.front().expect &= ~(EXPECT_TAG|EXPECT_OPEN_L);
		m_parseStates.front().expect |= EXPECT_VALUE;
	}
	else
	{
		m_parseStates.front().expect |= EXPECT_TAG;
	}

	m_parseStates.front().expect &= ~EXPECT_EQUAL;
	m_parseStates.front().expect |= EXPECT_CLOSE_EOL|EXPECT_CLOSE_COMMA;
	DebugOut(DEBUG_OUT_EXPECT, "", true);
	return true;
}

bool CHCParser::ProcessValue()
{
	DebugOut(DEBUG_OUT_EXPECT, " Value");

	if (m_parseStates.front().paramIndex != -1)
	{
		if (!OpenValue(false))
		{
			return false;
		}
	}

	if (!(m_parseStates.front().expect & EXPECT_VALUE))
	{
		return false;
	}
	m_parseStates.front().expect &= ~EXPECT_VALUE;

	if (*m_linePoint == '"')
	{
		if (m_parseStates.front().parseTags->parseTypes & PARSE_STRING)
		{
			char* parseStart = ++m_linePoint;
			for ( ; *m_linePoint != '\0' && *m_linePoint != '"'; m_linePoint++) ;

			if (*m_linePoint == '\0')
			{
				SetParseError(PointError() << "End of line before closing '\"'");
				return false;
			}

			// Overwrite the closing double-quotes.
			*m_linePoint++ = '\0';

			return AcceptValue(m_parseStates.front().parseTags, PARSE_STRING,
				parseStart, (unsigned long)(m_linePoint - parseStart));
		}
	}
	else if (IsAlnum(*m_linePoint))
	{
		char* parseStart = m_linePoint;
		for ( ; IsAlnum(*m_linePoint); m_linePoint++) ;

		char delim = *m_linePoint;
		*m_linePoint = '\0';

		if (!AcceptValue(m_parseStates.front().parseTags,
			m_parseStates.front().parseTags->parseTypes & (PARSE_CONSTANT|PARSE_NUMERIC),
			parseStart, (unsigned long)(m_linePoint - parseStart)))
		{
			return false;
		}

		*m_linePoint = delim;
		return true;
	}

	return false;
}

bool CHCParser::AcceptValue(const ParseTag* parseTag, unsigned char types,
								   char* value, unsigned long length)
{
	_ASSERT(types & (PARSE_STRING|PARSE_CONSTANT|PARSE_NUMERIC));

	if (length > m_parseStates.front().parseTags->maxParseLength)
	{
		SetParseError(PointError() << "Value length of '" << parseTag->tagName <<
			"' " << length << " is longer than limit(" << parseTag->maxParseLength << ")");
		return false;
	}

	if (types & PARSE_CONSTANT && parseTag->constants != NULL)
	{
		const ParseConstant* pc = parseTag->constants;
		for ( ; pc->constant != NULL; pc++)
		{
			if (stricmp(pc->constant, value) == 0)
			{
				break;
			}
		}

		if (pc->constant != NULL)
		{
			return ReportValue(parseTag, PARSE_CONSTANT, pc->value);
		}
	}
	if (types & PARSE_NUMERIC)
	{
		char* c = value;
		if (*c == '-')
		{
			c++;
		}

		if (*c == '0' && *++c == 'x')
		{
			if (*++c != '\0')
			{
				while (IsHex(*++c)) ;
			}
			else
			{
				c--;
			}
		}
		else
		{
			for ( ; IsDigit(*c); c++) ;
		}

		if (*c == '\0')
		{
			return ReportValue(parseTag, PARSE_NUMERIC, value);
		}
	}
	if (types & PARSE_STRING)
	{
		return ReportValue(parseTag, PARSE_STRING, value);
	}

	return false;
}

void CHCParser::InitializeParseState()
{
	m_parseStates.clear();

	static ParseTag s_rootParent[2] =
	{
		{ "",	PARSE_CHILDREN,	0, 1, m_tagListEntry,	NULL, NULL },
		{ NULL,	0,				0, 0, NULL,				NULL, NULL },
	};

	ParseState parseState;
	parseState.paramIndex = -1;
	parseState.bracketOpened = false;
	parseState.iterateValues = false;
	parseState.parseTags = s_rootParent;
	parseState.expect = 0;
	parseState.paramCounts = NULL;
	parseState.valueOpened = false;
	m_parseStates.push_front(parseState);

	OpenTag(0);
	OpenTagList(false);
}

bool CHCParser::OpenTag(long tagIndex)
{
	if (m_parseStates.front().paramIndex != -1)
	{
		if (!CloseTag(true))
		{
			return false;
		}
	}

	m_parseStates.front().paramIndex = tagIndex;
	m_parseStates.front().expect &= ~EXPECT_TAG;
	bool iterateValues = m_parseStates.front().iterateValues;

	if (m_parseStates.front().parseTags[tagIndex].parseTypes & PARSE_CHILDREN)
	{
		m_parseStates.front().expect |= EXPECT_OPEN_L|(iterateValues ? 0 : EXPECT_TAG);

		// Count the number of sub tags
		const ParseTag* parseTag = m_parseStates.front().parseTags[tagIndex].subTags;
		_ASSERT(parseTag != NULL);
		unsigned long paramCount;
		for (paramCount = 0; parseTag->tagName != NULL; parseTag++, paramCount++) ;

		// Initialize the number of values parsed for every sub tag
		m_parseStates.front().openedTagParamCounts.resize(paramCount, 0);
	}

	m_parseStates.front().expect |= EXPECT_OPEN_V|(iterateValues ? EXPECT_VALUE : EXPECT_EQUAL);

	if (iterateValues)
	{
		if (GetNextIterateValuesIndex() != -1)
		{
			m_parseStates.front().expect |= EXPECT_NEXT_COMMA;
		}
	}

	DebugOut(DEBUG_OUT_EXPECT, ":OT", true);
	return true;
}

bool CHCParser::CloseTag(bool openNext)
{
	if (m_parseStates.front().valueOpened)
	{
		m_parseStates.front().valueOpened = false;

		_ASSERT(m_parseStates.front().paramIndex != -1);
		long paramIndex = m_parseStates.front().paramIndex;
		const ParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];

		if (parseTag->parseTypes & PARSE_CHILDREN)
		{
			for (long i = 0; parseTag->subTags[i].tagName != NULL; i++)
			{
				if (m_parseStates.front().openedTagParamCounts[i] < parseTag->subTags[i].minimumNumber)
				{
					SetParseError(LineError() << "Number of '" << parseTag->subTags[i].tagName <<
						"' specified is less than limit(" << parseTag->subTags[i].minimumNumber << ")");
					return false;
				}
			}
		}

		if (!ReportClose(parseTag))
		{
			return false;
		}
	}

	m_parseStates.front().openedTagParamCounts.resize(0);
	m_parseStates.front().paramIndex = -1;
	m_parseStates.front().expect &= ~(EXPECT_NEXT_COMMA|EXPECT_VALUE|EXPECT_OPEN_V);

	if (!openNext)
	{
		m_parseStates.front().expect &= ~(EXPECT_CLOSE_COMMA|EXPECT_CLOSE_EOL|EXPECT_OPEN_L|EXPECT_EQUAL);
		m_parseStates.front().expect |= EXPECT_TAG;
		
		if (!(m_parseStates.front().expect & EXPECT_CLOSE_L))
		{
			m_parseStates.front().expect |= EXPECT_SECTION;
		}
	}

	DebugOut(DEBUG_OUT_EXPECT, ":CT", true);
	return true;
}

bool CHCParser::OpenValue(bool withBracket)
{
	_ASSERT(m_parseStates.front().paramIndex != -1);
	long paramIndex = m_parseStates.front().paramIndex;
	const ParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];

	// Make sure another value can be accepted
	unsigned long maxValuesCount = 1;
	if (parseTag->parseTypes & PARSE_CHILDREN)
	{
		maxValuesCount = parseTag->maxParseLength;
	}

	if (++(*m_parseStates.front().paramCounts)[paramIndex] > maxValuesCount)
	{
		SetParseError(PointError() << "Number of values for '" <<
			parseTag->tagName << "' has exceeded its limit(" << maxValuesCount << ")");
		return false;
	}

	if (!m_parseStates.front().valueOpened && !ReportOpen(parseTag))
	{
		return false;
	}
	m_parseStates.front().valueOpened = true;
	m_parseStates.front().expect &= ~(EXPECT_OPEN_V|EXPECT_VALUE|EXPECT_EQUAL);

	ParseState parseState;
	parseState.paramIndex = -1;
	parseState.paramCounts = &m_parseStates.front().openedTagParamCounts;
	parseState.passEOL = !withBracket && (m_parseStates.front().expect & EXPECT_CLOSE_EOL);
	parseState.valueOpened = false;

	if (withBracket)
	{
		parseState.bracketOpened = true;
		parseState.expect = EXPECT_CLOSE_V;
	}
	else
	{
		parseState.bracketOpened = false;
		parseState.expect = m_parseStates.front().expect & (EXPECT_CLOSE_V|
			EXPECT_CLOSE_L|EXPECT_CLOSE_EOL|EXPECT_NEXT_COMMA|EXPECT_CLOSE_COMMA);
	}

	if (parseTag->parseTypes & PARSE_CHILDREN)
	{
		parseState.iterateValues = true;
		parseState.parseTags = parseTag->subTags;
		m_parseStates.push_front(parseState);
		DebugOut(DEBUG_OUT_EXPECT, ":OV", true);

		long firstIndex = GetNextIterateValuesIndex();
		if (firstIndex != -1)
		{
			if (!OpenTag(firstIndex))
			{
				return false;
			}
			if (!withBracket)
			{
				if (!OpenValue(false))
				{
					return false;
				}
			}
		}
	}
	else
	{
		parseState.iterateValues = false;
		parseState.parseTags = parseTag;
		parseState.expect |= EXPECT_VALUE;
		m_parseStates.push_front(parseState);
		DebugOut(DEBUG_OUT_EXPECT, ":OV", true);
	}
	return true;
}

bool CHCParser::OpenTagList(bool withBracket)
{
	_ASSERT(m_parseStates.front().paramIndex != -1);
	long paramIndex = m_parseStates.front().paramIndex;
	const ParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];
	_ASSERT(parseTag->parseTypes & PARSE_CHILDREN);

	if (m_parseStates.front().expect & EXPECT_OPEN_V)
	{
		if (m_parseStates.front().paramCounts != NULL)
		{
			if (++(*m_parseStates.front().paramCounts)[paramIndex] > parseTag->maxParseLength)
			{
				SetParseError(PointError() << "Number of values for " <<
					parseTag->tagName << " has exceeded its limit(" << parseTag->maxParseLength << ")");
				return false;
			}
		}
	}

	if (!m_parseStates.front().valueOpened && !ReportOpen(parseTag))
	{
		return false;
	}
	m_parseStates.front().valueOpened = true;
	m_parseStates.front().expect &= ~(EXPECT_OPEN_L|EXPECT_OPEN_V|EXPECT_VALUE|EXPECT_EQUAL);

	ParseState parseState;
	parseState.paramIndex = -1;
	parseState.paramCounts = &m_parseStates.front().openedTagParamCounts;
	parseState.passEOL = !withBracket && (m_parseStates.front().expect & EXPECT_CLOSE_EOL);
	parseState.valueOpened = false;

	if (withBracket)
	{
		parseState.bracketOpened = true;
		parseState.expect = EXPECT_CLOSE_L;
	}
	else
	{
		parseState.bracketOpened = false;
		parseState.expect = m_parseStates.front().expect & (EXPECT_CLOSE_L|EXPECT_CLOSE_EOL);
	}

	parseState.iterateValues = false;
	parseState.parseTags = parseTag->subTags;
	parseState.expect |= EXPECT_TAG|(withBracket ? 0 : EXPECT_SECTION);
	m_parseStates.push_front(parseState);

	DebugOut(DEBUG_OUT_EXPECT, ":OL", true);
	return true;
}

bool CHCParser::CloseValue()
{
	if (m_parseStates.front().paramIndex != -1)
	{
		if (!CloseTag())
		{
			return false;
		}
	}
	m_parseStates.pop_front();

	if (!m_parseStates.front().iterateValues)
	{
		long paramIndex = m_parseStates.front().paramIndex;
		const ParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];

		m_parseStates.front().expect |= EXPECT_CLOSE_COMMA;

		if (parseTag->parseTypes & PARSE_CHILDREN)
		{
			if (m_parseStates.front().expect & EXPECT_OPEN_L)
			{
				m_parseStates.front().expect |= EXPECT_EQUAL;
			}
		}
		else
		{
			m_parseStates.front().expect |= EXPECT_CLOSE_EOL;
		}
	}

	DebugOut(DEBUG_OUT_EXPECT, ":CV", true);
	return true;
}

bool CHCParser::ReportOpen(const ParseTag* parseTag)
{
	if (*parseTag->tagName != '\0')
	{
#ifdef _DEBUG
		DebugOut(DEBUG_OUT_REPORT, ParseError() << parseTag->tagName << "<");
#endif
	}
	return ReportValue(parseTag, 0, NULL, REPORT_OPEN);
}

bool CHCParser::ReportClose(const ParseTag* parseTag)
{
	if (*parseTag->tagName != '\0')
	{
		DebugOut(DEBUG_OUT_REPORT, ">");
	}
	return ReportValue(parseTag, 0, NULL, REPORT_CLOSE);
}

bool CHCParser::ReportValue(const ParseTag* parseTag, unsigned char type, const char* value, int reason)
{
#ifdef _DEBUG
	if (value != NULL)
	{
		switch (type)
		{
		case PARSE_CONSTANT: DebugOut(DEBUG_OUT_REPORT, "Constant"); break;
		case PARSE_NUMERIC: DebugOut(DEBUG_OUT_REPORT, "Numeric"); break;
		case PARSE_STRING: DebugOut(DEBUG_OUT_REPORT, "String"); break;
		}
		DebugOut(DEBUG_OUT_REPORT, ParseError() << "=" << value);
	}
#endif

	if (parseTag->readProc != NULL)
	{
		try {
			(parseTag->readProc)(reason, parseTag, type, value, m_reportContext);
		}
		catch (string error) {
			SetParseError(PointError() << error.c_str());
			return false;
		}
		catch (wstring error) {
			SetParseError(PointError() << error.c_str());
			return false;
		}
	}
	return true;
}

