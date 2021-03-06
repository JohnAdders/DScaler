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

/**
* @file HierarchicalConfigParser.h HierarchicalConfigParser header file
*/

#ifndef __HIERARCHICALCONFIGPARSER_H___
#define __HIERARCHICALCONFIGPARSER_H___

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
//  tag := tstring
//  value := "tstring"|number|constant
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
//   { "Foo",            PARSE_STRING|PARSE_CONSTANT,    1, 127, NULL, constants, ReadFooProc },
//   { "Bar",            PARSE_CHILDREN,                    1, 127, tagList2, NULL, ReadBarProc },
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
// In this example, ReadFooProc() is called with REPORT_OPEN, REPORT_VALUE and REPORT_CLOSE
// when a "Foo" is parsed.  REPORT_VALUE may not be called if there is no value specified.
// In this case, the default value should be used (or an error thrown).  ReadBarProc() is
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
// Parses the full content of the file given.  If the parsing encounters an error, FALSE is
// returned and CHCParser.GetError() returns a tstring value indicating the error.
// 'filename' is the path to the filename that should be read.  'context' is an arbitrary
// pointer that will be given to the read callback and it is not used by CHCParser for
// any other purpose.  If 'filepointer' (FILE*) is specified instead, parsing starts at the
// current position in the file and continues to the end of the file.  The file is not
// closed afterwards.  ParseLocalFile() will look in the same directory as the program if
// the file is not found in the working directory.  'filename' can be specified as const
// TCHAR* 
//
//
// CHCParser.GetError()
// CHCParser.GetErrorUnicode()
//
// Only after CHCParser.ParseFile(...) returns FALSE, CHCParser.GetError() returns a valid
// tstring.
//
//
// CHCParser::Str2Int(text)
//
// Converts const TCHAR* to int.  Provided the specified text holds a number in a textual
// representation, either in decimal (negative or positive) or hexadecimal with a leading
// '0x', the return value is a int value of the number.  If the number cannot be parsed,
// the return value is 0.
//
//
// ParseReadProc(report, tag, type, value, context)
//
// A callback of this function type is called every time an entry in a file is parsed.
// 'report' can be either REPORT_OPEN, REPORT_CLOSE or REPORT_VALUE and signifies the
// various stages of parsing the entry.  'tag' is a pointer to the significant ParseTag
// element in the ParseTag array given to CHCParse at construction.  'type' can be one of
// PARSE_STRING, PARSE_NUMERIC, PARSE_CONSTANT or any combination of the three bit-wise
// ORed together.  It specifies how the CHCParser managed to read the value and may be
// necessary if there are more than one way of parsing a value, 'value' is a textual
// representation (const char*) of the value read.  CHCParser::Str2Int() can be used to
// convert any numeric text values to a int representation.  For REPORT_OPEN and
// REPORT_CLOSE, the 'type' and 'value' arguments are 0 and NULL respectively. 'context'
// is the pointer passed in the second argument of CHCParser.Parse().
//
// The application can stop the CHCParser from continuing the parsing by throwing a type
// tstring inside the callback.  In this case, the parsing will stop
// immediately and CHCParser.ParseFile() will return with FALSE.  CHCParser.GetError()
// will return an error message of format:
//
// "Error on line N character n: message"
//
// Where 'message' is the text in the tstring object that was thrown.  If any other
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


// parseTypes for CParseTag
enum
{
    PARSE_STRING        = 1 << 0,
    PARSE_NUMERIC        = 1 << 1,
    PARSE_CONSTANT        = 1 << 2,
    PARSE_CHILDREN        = 1 << 3,

    PARSE_NUM_OR_CONST    = PARSE_NUMERIC|PARSE_CONSTANT,
};

// ParseReadProc reports
enum
{
    REPORT_TAG,
    REPORT_OPEN,
    REPORT_CLOSE,
    REPORT_VALUE,
};

// Value type used by HCParser
class CParseValue
{
public:
    CParseValue();
    CParseValue(const TCHAR* str);
    CParseValue(int number);
    virtual ~CParseValue();

    virtual const TCHAR* GetString() const;
    virtual int GetNumber() const;

protected:
    const TCHAR*    m_string;
    int            m_number;
};

// Short typedefs for initializer lists.
typedef class CParseConstant PC;
typedef class CParseTag PT;

// Callback function type for CParseTag
typedef void (ParseReadProc)(int report, const CParseTag* tag, unsigned char type,
                             const CParseValue* value, void* context);

// This PASS_TO_PARENT value can be specified as the 'readProc' value of
// CParseTag to have to parent tag's callback function called.
ParseReadProc PASS_TO_PARENT;

// Constants definition for CParseTag
class CParseConstant
{
public:
    CParseConstant(const TCHAR* constant = NULL,
        CParseValue value = CParseValue());

    const TCHAR*        constant;
    CParseValue        value;
};

class CParseTag
{
public:
    // This class allows the single attributes value to be
    // initialized for either PARSE_CHILDREN or PARSE_CONSTANTS.
    class CAttributes
    {
    public:
        CAttributes(int = NULL) :
          subTags(NULL), constants(NULL) { }
          CAttributes(const CParseTag* subTags) :
          subTags(subTags), constants(NULL) { };
          CAttributes(const CParseConstant* constants) :
          subTags(NULL), constants(constants) { };

          const CParseTag*        subTags;
          const CParseConstant*    constants;
    };

    CParseTag(const TCHAR* tagName = NULL, unsigned char parseTypes = 0,
        unsigned char minimumNumber = 0, unsigned long maxParseLength = 0,
        CAttributes attributes = CAttributes(), ParseReadProc* readProc = NULL);

    // Name of the tag to be parsed.
    const TCHAR*                tagName;
    // Parsing mode for the tag.
    unsigned char            parseTypes;
    // Minimum number of the same type of tags that can be parsed.
    unsigned long            minimumNumber;
    // Maximum length in characters the value of the tag can be or
    // maximum tags of the same kind for PARSE_CHILDREN.
    unsigned long            maxParseLength;
    // Either a list of child tags or a list of constants.
    CAttributes                attributes;
    // The callback to be called when the tag is parsed.
    ParseReadProc*            readProc;
};

//
// CHCParser
//
// Main parsing class.
//
class CHCParser
{
public:
    CHCParser(const CParseTag* tagList);
    virtual ~CHCParser();

    BOOL ParseFile(const TCHAR* filename, void* reportContext = NULL);
    BOOL ParseFile(FILE* fp, void* reportContext = NULL);

    tstring GetError();

    static int Str2Int(const TCHAR* text);

public:
    enum
    {
        PARSE_ERROR_GENERIC,
        PARSE_ERROR_FILE,
        PARSE_ERROR_LINE,
        PARSE_ERROR_POINT,
    };

private:
    enum
    {
        MAX_LINE_LENGTH        = 512,
        MAX_READ_BUFFER        = 4096,
    };

    enum
    {
        EXPECT_SECTION        = 1 << 0,
        EXPECT_TAG            = 1 << 1,
        EXPECT_VALUE        = 1 << 2,
        EXPECT_EQUAL        = 1 << 3,
        EXPECT_NEXT_COMMA    = 1 << 4,
        EXPECT_OPEN_V        = 1 << 5,
        EXPECT_CLOSE_V        = 1 << 6,
        EXPECT_OPEN_L        = 1 << 7,
        EXPECT_CLOSE_L        = 1 << 8,
        EXPECT_CLOSE_EOL    = 1 << 9,
        EXPECT_CLOSE_COMMA    = 1 << 10,
        EXPECT_MAX            = 11,
    };

    typedef struct _ParseState
    {
        const CParseTag*            parseTags;
        long                        paramIndex;
        unsigned short                expect;
        BOOL                        bracketOpened;
        BOOL                        iterateValues;
        BOOL                        passEOL;
        BOOL                        valueOpened;
        std::vector<unsigned long>    openedTagParamCounts;
        std::vector<unsigned long>*    paramCounts;
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

        virtual void clear();
        virtual BOOL empty();

        virtual tstring str();

        template <class T>
        ParseError& operator<< (const T& t)
        {
            m_oss << t;
            return *this;
        }

    private:
        tostringstream m_oss;
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

    long ReadLineIntoBuffer(FILE* fp);

    BOOL ProcessStream(FILE* fp);
    BOOL ProcessLine();
    BOOL ProcessSection();
    BOOL ProcessTag();
    BOOL ProcessOpenValueBracket();
    BOOL ProcessCloseValueBracket();
    BOOL ProcessOpenTagListBracket();
    BOOL ProcessCloseTagListBracket();
    BOOL ProcessComma();
    BOOL ProcessEqual();
    BOOL ProcessValue();

    BOOL AcceptValue(const CParseTag* parseTag, unsigned char types,
        TCHAR* value, unsigned long length);

    BOOL OpenTag(long tagIndex);
    BOOL CloseTag(BOOL openNext = FALSE);
    BOOL OpenValue(BOOL withBracket);
    BOOL OpenTagList(BOOL withBracket);
    BOOL CloseValue();

    void SetParseError(ParseError& error);
    void AddExpectLine(ParseError& error, BOOL debugging = FALSE);

    BOOL ReportTag(const CParseTag* parseTag);
    BOOL ReportOpen(const CParseTag* parseTag);
    BOOL ReportClose(const CParseTag* parseTag);
    BOOL ReportValue(const CParseTag* parseTag, unsigned char type,
                     const CParseValue* value, int report = REPORT_VALUE);

    void TrimLeft();
    BOOL TagTakesValues(const CParseTag* parseTag);
    long GetNextIterateValuesIndex();

    BOOL IsSpace(int c);
    BOOL IsAlpha(int c);
    BOOL IsAlnum(int c);
    BOOL IsDigit(int c);
    BOOL IsDelim(int c);
    BOOL IsHex(int c);

    enum
    {
        DEBUG_OUT_NONE        = 0,
        DEBUG_OUT_ERROR        = 1,
        DEBUG_OUT_REPORT    = 2,
        DEBUG_OUT_EXPECT    = 3,
    };

    void DebugOut(int level, const TCHAR* message, BOOL appendExpect = FALSE);
    void DebugOut(int level, ParseError& error, BOOL appendExpect = FALSE);

private:
    char*                        m_readBuffer;
    size_t                       m_bufferPosition;
    size_t                       m_bufferLength;
    TCHAR                        m_newlineChar;

    unsigned long                m_lineNumber;
    TCHAR                        m_lineBuffer[MAX_LINE_LENGTH];
    TCHAR*                       m_linePoint;

    ParseError                   m_parseError;
    BOOL                         m_parseErrorSet;
    int                          m_debugOutLevel;
    void*                        m_reportContext;

    const CParseTag*             m_tagListEntry;
    CParseTag                    m_rootParent[2];
    std::list<ParseState>        m_parseStates;
};

}; //namespace


#endif

