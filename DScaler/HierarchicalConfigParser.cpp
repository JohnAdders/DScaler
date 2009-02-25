/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Revision 1.15  2005/05/12 08:33:18  adcockj
// hopefully fixed win98 comapatability
//
// Revision 1.14  2005/03/20 14:05:48  adcockj
// Fixed win98 runtime error
//
// Revision 1.13  2004/12/12 11:26:08  atnak
// Fixes problem found by Torsten with hex numbers not properly being parsed.
//
// Revision 1.12  2004/12/06 00:07:31  atnak
// Fix to VC6 working differenting for wostringstream::operator <<(const char*)
//
// Revision 1.11  2004/12/02 18:16:00  atnak
// Minor fix.
//
// Revision 1.10  2004/12/01 22:01:17  atnak
// Fix the VC++ 6 incompatibility introduced by last change.
//
// Revision 1.9  2004/12/01 17:57:07  atnak
// Updates to HierarchicalConfigParser.
//
// Revision 1.8  2004/11/27 00:31:55  atnak
// More bug fixes.
//
// Revision 1.7  2004/11/26 23:12:20  atnak
// Fixed problem with line number being wrong introduced few changes back.
//
// Revision 1.6  2004/11/22 20:38:17  atnak
// Bug fix and new features.
//
// Revision 1.5  2004/11/21 23:18:34  atnak
// Added another reporting.
//
// Revision 1.4  2004/11/21 16:21:16  atnak
// Bug fix and slight change.
//
// Revision 1.3  2004/11/21 14:17:38  atnak
// Made line comments able to start anywhere in the line.
//
// Revision 1.2  2004/11/20 16:36:40  atnak
// Removed the use of ifstream and added custom buffered line reader.
//
// Revision 1.1  2004/11/19 23:51:04  atnak
// Release of the configuration parser that is used for card list parsing.
//
//
//////////////////////////////////////////////////////////////////////////////

/** 
* @file HierarchicalConfigParser.h HierarchicalConfigParser Implementation
*/

#include "stdafx.h"
#include <iostream>
#include <crtdbg.h>
#include "HierarchicalConfigParser.h"

using namespace std;
using namespace HCParser;


void HCParser::PASS_TO_PARENT(int, const CParseTag*, unsigned char, const CParseValue*, void*)
{
};

CHCParser::CHCParser(const CParseTag* tagList)
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
	m_newlineChar	= 0;

	m_readBuffer = new char[MAX_READ_BUFFER];
	m_bufferPosition = m_bufferLength = MAX_READ_BUFFER;

	bool success = ProcessStream(fp);
	delete [] m_readBuffer;
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
	// however windows 98 doesn't support the VerfifyVersionInfo function
	// so we call it via the indirect method
	HINSTANCE h = LoadLibrary("kernel32.dll");

	BOOL (WINAPI* lpVerifyVersionInfoA)(LPOSVERSIONINFOEXA lpVersionInformation, DWORD dwTypeMask, DWORDLONG dwlConditionMask) = NULL;

    lpVerifyVersionInfoA = (BOOL (WINAPI*)(LPOSVERSIONINFOEXA lpVersionInformation, DWORD dwTypeMask, DWORDLONG dwlConditionMask)) GetProcAddress(h,"VerifyVersionInfoA");
	bool result = false;
	if(lpVerifyVersionInfoA)
	{
		result = lpVerifyVersionInfoA(&osvi, VER_MAJORVERSION, dwlConditionMask) != 0;
	}
	FreeLibrary(h);
	return result;
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

int CHCParser::Str2Int(const char* text)
{
	const char* c = text;
	int n = 0;
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
			n *= 0x10;

			if (*c >= '0' && *c <= '9')
			{
				n += *c - '0';
			}
			else if (*c >= 'a' && *c <= 'f')
			{
				n += 0xA + *c - 'a';
			}
			else if (*c >= 'A' && *c <= 'F')
			{
				n += 0xA + *c - 'A';
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
				n *= 10;
				n += *c - '0';
			}
			else
			{
				return 0;
			}
		}
	}

	return negative ? -n : n;
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

long CHCParser::ReadLineIntoBuffer(FILE* fp)
{
	*m_lineBuffer = '\0';
	m_linePoint = m_lineBuffer;

	while (m_bufferLength > 0)
	{
		for ( ; m_bufferPosition < m_bufferLength; m_bufferPosition++)
		{
			if (m_readBuffer[m_bufferPosition] == '\n' || m_readBuffer[m_bufferPosition] == '\r')
			{
				if (m_newlineChar == 0)
				{
					m_newlineChar = m_readBuffer[m_bufferPosition];
				}
				if (m_readBuffer[m_bufferPosition] == m_newlineChar)
				{
					m_lineNumber++;
				}

				// Stop if there is something in the buffer.
				if (*m_lineBuffer != '\0')
				{
					m_bufferPosition++;
					*m_linePoint = '\0';
					return (long)(m_linePoint - m_lineBuffer);
				}
				continue;
			}
			if (m_linePoint >= m_lineBuffer + MAX_LINE_LENGTH-1)
			{
				SetParseError(LineError() << "Line is too long");
				return -1;
			}

			*m_linePoint++ = m_readBuffer[m_bufferPosition];
		}

		if (m_bufferLength != MAX_READ_BUFFER)
		{
			break;
		}

		m_bufferLength = fread(m_readBuffer, 1, MAX_READ_BUFFER, fp);
		if (m_bufferLength != MAX_READ_BUFFER && !feof(fp))
		{
			SetParseError(FileError() << "File I/O error while reading");
			return -1;
		}

		m_bufferPosition = 0;
	}

	if (*m_lineBuffer != '\0')
	{
		m_bufferLength = 0;
		*m_linePoint = '\0';
		return (long)(m_linePoint - m_lineBuffer);
	}
	return 0;
}

bool CHCParser::ProcessStream(FILE* fp)
{
	InitializeParseState();

	while (1)
	{
		// Read one line into a buffer
		long length = ReadLineIntoBuffer(fp);
		if (length == -1)
		{
			return false;
		}
		if (length == 0)
		{
			break;
		}

		m_linePoint = m_lineBuffer;
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

		if (*m_linePoint == '\0' || *m_linePoint == ';' || *m_linePoint == '#')
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

bool CHCParser::TagTakesValues(const CParseTag* parseTag)
{
	if (parseTag->tagName == NULL)
	{
		return false;
	}

	if (parseTag->parseTypes & PARSE_CHILDREN)
	{
		for (long i = 0; parseTag->attributes.subTags[i].tagName != NULL; i++)
		{
			if (TagTakesValues(&parseTag->attributes.subTags[i]))
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
	const CParseTag* parseTags = m_parseStates.front().parseTags;

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
	const char* namesReadable[EXPECT_MAX] = { "", "tag-name",
		"value", "'='", "','", "'('", "')'", "'{'", "'}'", "EOL", "','" };
	const char* namesDebug[EXPECT_MAX] = { "se", "ta", "va", "eq",
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

	m_parseStates.front().expect = EXPECT_TAG|EXPECT_SECTION;
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

	const CParseTag* parseTag;

	while (1)
	{
		parseTag = m_parseStates.front().parseTags;
		for ( ; parseTag->tagName != NULL; parseTag++)
		{
			if (_stricmp(parseTag->tagName, parseStart) == 0)
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

	if (!ReportTag(parseTag))
	{
		return false;
	}
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
		m_parseStates.front().expect &= ~EXPECT_TAG;
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
		if (m_parseStates.front().parseTags->parseTypes & (PARSE_CONSTANT|PARSE_NUMERIC))
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
	}

	return false;
}

bool CHCParser::AcceptValue(const CParseTag* parseTag, unsigned char types,
								   char* value, unsigned long length)
{
	_ASSERT(types & (PARSE_STRING|PARSE_CONSTANT|PARSE_NUMERIC));

	if (length > m_parseStates.front().parseTags->maxParseLength)
	{
		SetParseError(PointError() << "Value length of '" << parseTag->tagName <<
			"' " << length << " is longer than limit(" << parseTag->maxParseLength << ")");
		return false;
	}

	if (types & PARSE_CONSTANT && parseTag->attributes.constants != NULL)
	{
		const CParseConstant* pc = parseTag->attributes.constants;
		for ( ; pc->constant != NULL; pc++)
		{
			if (_stricmp(pc->constant, value) == 0)
			{
				break;
			}
		}

		if (pc->constant != NULL)
		{
			return ReportValue(parseTag, PARSE_CONSTANT, &pc->value);
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
			return ReportValue(parseTag, PARSE_NUMERIC, &CParseValue(value));
		}
	}
	if (types & PARSE_STRING)
	{
		return ReportValue(parseTag, PARSE_STRING, &CParseValue(value));
	}

	return false;
}

void CHCParser::InitializeParseState()
{
	m_parseStates.clear();

	memset(m_rootParent, 0, sizeof(m_rootParent));
	m_rootParent[0].tagName				= "";
	m_rootParent[0].parseTypes			= PARSE_CHILDREN;
	m_rootParent[0].maxParseLength		= 1;
	m_rootParent[0].attributes.subTags	= m_tagListEntry;


	ParseState parseState;
	parseState.paramIndex = -1;
	parseState.bracketOpened = false;
	parseState.iterateValues = false;
	parseState.parseTags = m_rootParent;
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
		const CParseTag* parseTag = m_parseStates.front().parseTags[tagIndex].attributes.subTags;
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
	else
	{
		m_parseStates.front().expect |= EXPECT_NEXT_COMMA;
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
		const CParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];

		if (parseTag->parseTypes & PARSE_CHILDREN)
		{
			for (long i = 0; parseTag->attributes.subTags[i].tagName != NULL; i++)
			{
				if (m_parseStates.front().openedTagParamCounts[i] < parseTag->attributes.subTags[i].minimumNumber)
				{
					SetParseError(LineError() << "Number of '" << parseTag->attributes.subTags[i].tagName <<
						"' specified is less than limit(" << parseTag->attributes.subTags[i].minimumNumber << ")");
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
	const CParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];

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
		parseState.parseTags = parseTag->attributes.subTags;
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
	const CParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];
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
	parseState.parseTags = parseTag->attributes.subTags;
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
		const CParseTag* parseTag = &m_parseStates.front().parseTags[paramIndex];

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

bool CHCParser::ReportTag(const CParseTag* parseTag)
{
	return ReportValue(parseTag, 0, NULL, REPORT_TAG);
}

bool CHCParser::ReportOpen(const CParseTag* parseTag)
{
	if (*parseTag->tagName != '\0')
	{
#ifdef _DEBUG
		DebugOut(DEBUG_OUT_REPORT, ParseError() << parseTag->tagName << "<");
#endif
	}
	return ReportValue(parseTag, 0, NULL, REPORT_OPEN);
}

bool CHCParser::ReportClose(const CParseTag* parseTag)
{
	if (*parseTag->tagName != '\0')
	{
		DebugOut(DEBUG_OUT_REPORT, ">");
	}
	return ReportValue(parseTag, 0, NULL, REPORT_CLOSE);
}

bool CHCParser::ReportValue(const CParseTag* parseTag, unsigned char type,
							const CParseValue* value, int report)
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

		if (value->GetString() != NULL)
		{
			DebugOut(DEBUG_OUT_REPORT, ParseError() << "=" << value->GetString());
		}
		else
		{
			DebugOut(DEBUG_OUT_REPORT, ParseError() << "=" << value->GetNumber());
		}
	}
#endif

	ParseReadProc* readProc = parseTag->readProc;

	if (readProc == PASS_TO_PARENT)
	{
		list<ParseState>::iterator it = m_parseStates.begin();

		// The top level state's paramIndex can be -1 but there'll always
		// be a lower level state where paramIndex is not -1.  (If need be
		// the sentinel value.)
		if (it->paramIndex == -1)
		{
			it++;
		}

		// Look at older states until a NULL or a good callback is found.
		// There is a sentinel value at the root (m_rootParent) so there's
		// no need to check 'it' against m_parseStates.end().
		for ( ; it->parseTags[it->paramIndex].readProc == PASS_TO_PARENT; it++) ;
		readProc = it->parseTags[it->paramIndex].readProc;
	}

	if (readProc != NULL)
	{
		try {
			(readProc)(report, parseTag, type, value, m_reportContext);
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


//////////////////////////////////////////////////////////////////////////
// CParseValue
//////////////////////////////////////////////////////////////////////////

CParseValue::CParseValue() :
	m_string(NULL),
	m_number(0)
{
}

CParseValue::CParseValue(const char* str) :
	m_string(str),
	m_number(0)
{
}

CParseValue::CParseValue(int number) :
	m_string(NULL),
	m_number(number)
{

}

CParseValue::~CParseValue()
{
}

const char* CParseValue::GetString() const
{
	return m_string;
}

int CParseValue::GetNumber() const
{
	if (m_string != NULL)
	{
		return CHCParser::Str2Int(m_string);
	}
	return m_number;
}


//////////////////////////////////////////////////////////////////////////
// CParseConstant
//////////////////////////////////////////////////////////////////////////

CParseConstant::CParseConstant(const char* constant, CParseValue value)
{
	this->constant	= constant;
	this->value		= value;
}


//////////////////////////////////////////////////////////////////////////
// CParseTag
//////////////////////////////////////////////////////////////////////////

CParseTag::CParseTag(const char* tagName, unsigned char parseTypes,
					 unsigned char minimumNumber,unsigned long maxLength,
					 CAttributes attributes, ParseReadProc* readProc)
{
	this->tagName			= tagName;
	this->parseTypes		= parseTypes;
	this->minimumNumber		= minimumNumber;
	this->maxParseLength	= maxLength;
	this->attributes		= attributes;
	this->readProc			= readProc;
}


//////////////////////////////////////////////////////////////////////////
// CHCParser::ParseError
//////////////////////////////////////////////////////////////////////////

void CHCParser::ParseError::clear()
{
	m_oss.str(L"");
}

bool CHCParser::ParseError::empty()
{
	return m_oss.str().size() == 0;
}

string CHCParser::ParseError::str()
{
	char buffer[512];

	// Convert the wchar_t to ANSI characters
	if (!WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
		m_oss.str().c_str(), -1, buffer, sizeof(buffer), NULL, NULL))
	{
        // may fail on early OSes so fall back to some older behaviour
	    if (!WideCharToMultiByte(CP_ACP, WC_DEFAULTCHAR,
		    m_oss.str().c_str(), -1, buffer, sizeof(buffer), NULL, NULL))
	    {
    		return std::string("Unicode to multi-byte conversion error.");
        }
	}
	return std::string(buffer);
}

wstring CHCParser::ParseError::wstr()
{
	return m_oss.str();
}

CHCParser::ParseError& CHCParser::ParseError::operator << (const char* s)
{
	wchar_t wbuffer[512];

	// Convert const char* to wchar_t*
	if (!MultiByteToWideChar(CP_ACP, 0, s, -1, wbuffer, sizeof(wbuffer)))
	{
		m_oss << L"Multi-byte to unicode conversion error.";
	}

	m_oss << wbuffer;
	return *this;
}
