/////////////////////////////////////////////////////////////////////////////
// $Id: HierarchicalConfigParser.h,v 1.1 2004-11-19 23:51:04 atnak Exp $
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
* @file HierarchicalConfigParser.h HierarchicalConfigParser header file
*/

#ifndef __HIERARCHICALCONFIGPARSER_H___
#define __HIERARCHICALCONFIGPARSER_H___

#include <fstream>
#include <sstream>
#include <list>
#include <vector>

/**
* A configuration file parser for parsing text files similar to standard
* INI files used in 16-bit Windows.
*/
namespace HCParser
{

//
// Grammar:
//
//  file := [\[value\]\n] tag-list
//
//  tag-list := value-tag|parent-tag[ tag-list]
//
//  value-tag := tag [= value\n|[=] (value)]
//  parent-tag := tag [= sub-tag-value-list\n|[=] ( sub-tag-value-list ) \
//    [= tag-list\n|tag-list|{ tag-list }]]
//
//  sub-tag-value-list := value[, sub-tag-value-list]
//
//  tag := string
//  value := "string"|number|constant
//
//
// Examples:
//
//  tag = value
//  tag [=] ( value )
//
//  tag
//    sub-tag1 ( value )
//  tag ( sub-tag1-value, sub-tag2-value )
//    ...
//  tag ( sub-tag1-value ) = sub-tag2 ( value )
//    ...
//  tag ( ) = sub-tag ( value )
//    ...
//  tag = sub-tag1-value, sub-tag2-value
//

//
// Example of use:
//
// const ParseConstant constants[] =
// {
//   { "Constant1", "Value1" },
//   { "Constant2", "Value2" },
//   { NULL } // Terminating NULL entry.
// };
//
// const ParseTag tagList[] =
// {
//   { "Foo",			PARSE_STRING|PARSE_CONSTANT,	1, 127, NULL, constants, ReportFooProc },
//   { "Bar",			PARSE_CHILDREN,					1, 127, tagList2, NULL, ReportBarProc },
//   { NULL } // Terminating NULL entry.
// };
//
// CHCParser parser(tagList);
//
// if (!parser.ParseFile("file.ini", NULL))
// {
//   cout << parser.GetError();
// }
//
//
// In this example, ReportFooProc() is called with REPORT_OPEN, REPORT_VALUE and REPORT_CLOSE
// when a "Foo" is parsed.  REPORT_VALUE may not be called if there is no value specified.
// In this case, the default value should be used (or an error thrown).  ReportBarProc() is
// is called when a "Bar" is parsed.  For a PARSE_CHILDREN entry, a REPORT_OPEN will be
// called, and a REPORT_CLOSE only after all child entries have been parsed.  There is no
// REPORT_VALUE for a PARSE_CHILDREN.
//
//
// Public functions:
//
// CHCParser.ParseFile(filename, context)
// CHCParser.ParseFile(filepointer, context)
// CHCParser.ParseLocalFile(filename, context);
//
// Parses the full content of the file given.  If the parsing encounters an error, false is
// returned and CHCParser.GetError() returns a std::string value indicating the error.
// 'filename' is the path to the filename that should be read.  'context' is an arbitrary
// pointer that will be given to the report callback and it is not used by CHCParser for
// any other purpose.  If 'filepointer' (FILE*) is specified instead, parsing starts at the
// current position in the file and continues to the end of the file.  The file is not
// closed afterwards.  ParseLocalFile() will look in the same directory as the program if
// the file is not found in the working directory.  'filename' can be specified as const
// char* or const wchar_t* for unicode file names.
//
//
// CHCParser.GetError()
// CHCParser.GetErrorUnicode()
//
// Only after CHCParser.ParseFile(...) returns false, CHCParser.GetError() returns a valid
// std::string variable containing the description of the error.  GetErrorUnicode() returns
// a std::wstring type.  Since the error string is stored internally as unicode, a
// conversion to multi-byte characters takes place if GetError() is used.
//
//
// CHCParser::Str2Long(text)
//
// Converts const char* to long.  Provided the specified text holds a number in a textual
// representation, either in decimal (negative or positive) or hexadecimal with a leading
// '0x', the return value is a long value of the number.  If the number cannot be parsed,
// the return value is 0.
//
//
// ParseReportProc(reason, tag, type, value, context)
//
// A callback of this function type is called every time an entry in a file is parsed.
// 'reason' can be either REPORT_OPEN, REPORT_CLOSE or REPORT_VALUE and signifies the
// various stages of parsing the entry.  'tag' is a pointer to the significant ParseTag
// element in the ParseTag array given to CHCParse at construction.  'type' can be one of
// PARSE_STRING, PARSE_NUMERIC, PARSE_CONSTANT or any combination of the three bit-wise
// ORed together.  It specifies how the CHCParser managed to read the value and may be
// necessary if there are more than one way of parsing a value, 'value' is a textual
// representation (const char*) of the value read.  CHCParser::Str2Long() can be used to
// convert any numeric text values to a long representation.  For REPORT_OPEN and
// REPORT_CLOSE, the 'type' and 'value' arguments are 0 and NULL respectively. 'context'
// is the pointer passed in the second argument of CHCParser.Parse().
//
// The application can stop the CHCParser from continuing the parsing by throwing a type
// std::string or std::wstring inside the callback.  In this case, the parsing will stop
// immediately and CHCParser.ParseFile() will return with false.  CHCParser.GetError()
// will return an error message of format:
//
// "Error on line N character n: message"
//
// Where 'message' is the text in the std::string object that was thrown.  If any other
// type is thrown in the callback, it will not be caught by CHCParser.
//
//
// CHCParser::IsUnicodeOS()
//
// This function is provided as a utility and can be used to determine at runtime if the
// version of Windows running the program supports unicode.  The check is merely whether
// or not the version of Windows is NT or above (NT, 2000 and XP).  Since many of the
// 'W' functions will fail on 95/98/Me, this function can be used to make an informed
// decision as to whether or not such functions should be used.  For some information
// regarding this, see: http://msdn.microsoft.com/library/en-us/win9x/chilimit_5alv.asp
//


// parseTypes for ParseTag
enum
{
	PARSE_STRING		= 1 << 0,
	PARSE_NUMERIC		= 1 << 1,
	PARSE_CONSTANT		= 1 << 2,
	PARSE_CHILDREN		= 1 << 3,
};

// ParseReadProc reasons
enum
{
	REPORT_OPEN,
	REPORT_CLOSE,
	REPORT_VALUE,
};

typedef struct _ParseConstant ParseConstant;
typedef struct _ParseTag ParseTag;

// Callback function type for ParseTag
typedef void (ParseReportProc)(int reason, const ParseTag* tag, unsigned char type,
							   const char* value, void* context) throw(...);

// Constants definition for ParseTag
struct _ParseConstant
{
	const char*		constant;
	const char*		value;
};

struct _ParseTag
{
	const char*				tagName;
	unsigned char			parseTypes;
	unsigned long			minimumNumber;
	unsigned long			maxParseLength;
	const ParseTag*			subTags;
	const ParseConstant*	constants;
	ParseReportProc*		readProc;
};

//
// CHCParser
//
// Main parsing class.
//
class CHCParser
{
public:
	CHCParser(const ParseTag* tagList);
	virtual ~CHCParser();

	bool ParseLocalFile(const char* filename, void* reportContext = NULL);
	bool ParseLocalFile(const wchar_t* filename, void* reportContext = NULL);

	bool ParseFile(const char* filename, void* reportContext = NULL, bool localFile = false);
	bool ParseFile(const wchar_t* filename, void* reportContext = NULL, bool localFile = false);
	bool ParseFile(FILE* fp, void* reportContext = NULL);

	std::string GetError();
	std::wstring GetErrorUnicode();

	static long Str2Long(const char* text);

	static bool IsUnicodeOS();
	static FILE* OpenLocalFile(const char* filename);
	static FILE* OpenLocalFile(const wchar_t* filename);

private:
	enum
	{
		MAX_LINE_LENGTH		= 512,
	};

	enum
	{
		EXPECT_SECTION		= 1 << 0,
		EXPECT_TAG			= 1 << 1,
		EXPECT_COMMENT		= 1 << 2,
		EXPECT_VALUE		= 1 << 3,
		EXPECT_EQUAL		= 1 << 4,
		EXPECT_NEXT_COMMA	= 1 << 5,
		EXPECT_OPEN_V		= 1 << 6,
		EXPECT_CLOSE_V		= 1 << 7,
		EXPECT_OPEN_L		= 1 << 8,
		EXPECT_CLOSE_L		= 1 << 9,
		EXPECT_CLOSE_EOL	= 1 << 10,
		EXPECT_CLOSE_COMMA	= 1 << 11,
		EXPECT_MAX			= 12,
	};

	enum
	{
		PARSE_ERROR_GENERIC,
		PARSE_ERROR_FILE,
		PARSE_ERROR_LINE,
		PARSE_ERROR_POINT,
	};

	typedef struct _ParseState
	{
		const ParseTag*				parseTags;
		long						paramIndex;
		unsigned short				expect;
		bool						bracketOpened;
		bool						iterateValues;
		bool						passEOL;
		bool						valueOpened;
		std::vector<unsigned long>	openedTagParamCounts;
		std::vector<unsigned long>*	paramCounts;
	} ParseState;

	class ParseError
	{
	public:
		ParseError() { }
		virtual ~ParseError() { }

		virtual int type()
		{
			return PARSE_ERROR_GENERIC;
		}

		virtual void clear()
		{
			m_oss.str(L"");
		}

		virtual bool empty()
		{
			return m_oss.str().size() == 0;
		}

		virtual std::string str()
		{
			char buffer[512];

			// Convert the wchar_t to ANSI characters
			if (!WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
				m_oss.str().c_str(), -1, buffer, sizeof(buffer), NULL, NULL))
			{
				return std::string("Unicode to multi-byte conversion error.");
			}
			return std::string(buffer);
		}

		virtual std::wstring wstr()
		{
			return m_oss.str();
		}

		template <class T>
		ParseError& operator<< (const T& t)
		{
			m_oss << t;
			return *this;
		}

	private:
		std::wostringstream m_oss;
	};

	class FileError : public ParseError
	{
	public:
		virtual int type() { return PARSE_ERROR_FILE; }
	};

	class LineError : public ParseError
	{
	public:
		virtual int type() { return PARSE_ERROR_LINE; }
	};

	class PointError : public ParseError
	{
	public:
		virtual int type() { return PARSE_ERROR_POINT; }
	};

private:
	void InitializeParseState();

	bool ProcessStream(std::ifstream& ifs);
	bool ProcessLine();
	bool ProcessSection();
	bool ProcessTag();
	bool ProcessOpenValueBracket();
	bool ProcessCloseValueBracket();
	bool ProcessOpenTagListBracket();
	bool ProcessCloseTagListBracket();
	bool ProcessComma();
	bool ProcessEqual();
	bool ProcessValue();

	bool AcceptValue(const ParseTag* parseTag, unsigned char types,
		char* value, unsigned long length);

	bool OpenTag(long tagIndex);
	bool CloseTag(bool openNext = false);
	bool OpenValue(bool withBracket);
	bool OpenTagList(bool withBracket);
	bool CloseValue();

	void SetParseError(ParseError& error);
	void AddExpectLine(ParseError& error, bool debugging = false);

	bool ReportOpen(const ParseTag* parseTag);
	bool ReportClose(const ParseTag* parseTag);
	bool ReportValue(const ParseTag* parseTag, unsigned char type,
					 const char* value, int reason = REPORT_VALUE);

	void TrimLeft();
	bool TagTakesValues(const ParseTag* parseTag);
	long GetNextIterateValuesIndex();

	bool IsSpace(int c);
	bool IsAlpha(int c);
	bool IsAlnum(int c);
	bool IsDigit(int c);
	bool IsDelim(int c);
	bool IsHex(int c);

	enum
	{
		DEBUG_OUT_NONE		= 0,
		DEBUG_OUT_ERROR		= 1,
		DEBUG_OUT_REPORT	= 2,
		DEBUG_OUT_EXPECT	= 3,
	};

	void DebugOut(int level, const char* message, bool appendExpect = false);
	void DebugOut(int level, ParseError& error, bool appendExpect = false);

private:
	unsigned long				m_lineNumber;
	char						m_lineBuffer[MAX_LINE_LENGTH];
	char*						m_linePoint;

	ParseError					m_parseError;
	bool						m_parseErrorSet;
	int							m_debugOutLevel;
	void*						m_reportContext;

	const ParseTag*				m_tagListEntry;
	std::list<ParseState>		m_parseStates;
};

}; //namespace


#endif

