/////////////////////////////////////////////////////////////////////////////
// $Id: VTCommon.h,v 1.4 2003-10-27 10:39:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 21 Dec 2002   Atsushi Nakagawa      Remodularized videotext elements
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/02/05 06:58:28  atnak
// Added Danish codepage submitted by Kristian Trenskow (trenskow)
//
// Revision 1.2  2003/01/03 13:47:06  atnak
// Flof index key definition
//
// Revision 1.1  2003/01/01 20:40:48  atnak
// Inital release of videotext common + utilities class.
//
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file vtcommon.h vtcommon Header file
 */
 
#ifndef __VTCOMMON_H___
#define __VTCOMMON_H___


/// Control bits in a page header
enum
{
    VTCONTROL_MAGAZINE      = 7 << 0,
    VTCONTROL_ERASEPAGE     = 1 << 3,
    VTCONTROL_NEWSFLASH     = 1 << 4,
    VTCONTROL_SUBTITLE      = 1 << 5,
    VTCONTROL_SUPRESSHEADER = 1 << 6,
    VTCONTROL_UPDATE        = 1 << 7,
    VTCONTROL_INTERRUPTED   = 1 << 8,
    VTCONTROL_INHIBITDISP   = 1 << 9,
    VTCONTROL_MAGSERIAL     = 1 << 10,
    VTCONTROL_CHARSUBSET    = 7 << 11,
};


/// VideoText display modes
enum
{
    VTMODE_GRAPHICS     = 1 << 0,
    VTMODE_SEPARATED    = 1 << 1,
    VTMODE_FLASH        = 1 << 2,
    VTMODE_CONCEAL      = 1 << 3,
    VTMODE_BOXED        = 1 << 4,
    VTMODE_DOUBLE       = 1 << 5,
    VTMODE_HOLD         = 1 << 6,
};


/// VideoText display colours
enum eVTColor
{
    VTCOLOR_BLACK   = 0,
    VTCOLOR_RED     = 1,
    VTCOLOR_GREEN   = 2,
    VTCOLOR_YELLOW  = 3,
    VTCOLOR_BLUE    = 4,
    VTCOLOR_MAGENTA = 5,
    VTCOLOR_CYAN    = 6,
    VTCOLOR_WHITE   = 7,

    // These one is irregular
    VTCOLOR_NONE    = 8,
};


/// Coloured buttons on the remote
enum
{
    VTFLOF_RED      = 0,
    VTFLOF_GREEN    = 1,
    VTFLOF_YELLOW   = 2,
    VTFLOF_BLUE     = 3,
    VTFLOF_INDEX    = 4,
    VTFLOF_UNKN1    = 5
};


/// Videotext codepages
enum eVTCodepage
{
    VTCODEPAGE_SAME             = -2,   // Special internal use
    VTCODEPAGE_NONE             = -1,   // Special internal use
    VTCODEPAGE_ENGLISH          = 0,
    VTCODEPAGE_FRENCH,
    VTCODEPAGE_SWEDISH,                 // Also Finnish/Hungarian
    VTCODEPAGE_CZECH,                   // Also Slovak
    VTCODEPAGE_GERMAN,
    VTCODEPAGE_PORTUGUESE,              // Also Spanish
    VTCODEPAGE_ITALIAN,
    VTCODEPAGE_POLISH,
    VTCODEPAGE_TURKISH,
    VTCODEPAGE_SLOVENIAN,               // Also Serbian/Croation (Latin)
    VTCODEPAGE_RUMANIAN,
    VTCODEPAGE_SERBIAN,                 // Also Croatian (Cyrillic-1)
    VTCODEPAGE_RUSSIAN,                 // Also Bulgarian
    VTCODEPAGE_ESTONIAN,
    VTCODEPAGE_UKRAINIAN,
    VTCODEPAGE_LETTISH,                 // Also Lithuanian
    VTCODEPAGE_GREEK,
    VTCODEPAGE_ENGLISHA,                // English with Arabic G2
    VTCODEPAGE_FRENCHA,                 // French with Arabic G2
    VTCODEPAGE_ARABIC,
    VTCODEPAGE_HEBREW,
    VTCODEPAGE_DANISH,
    VTCODEPAGE_LASTONE
};


/// Videotext codepages
/// These names are guesses --AtNak
enum eVTRegion
{
    VTREGION_DEFAULT            = 0,
    VTREGION_CENTRALEUROPE,
    VTREGION_SOUTHERNEUROPE,
    VTREGION_BALKANS,
    VTREGION_EASTERNEUROPE,
    VTREGION_RESERVED1,
    VTREGION_MEDITERRANEAN,
    VTREGION_RESERVED2,
    VTREGION_NORTHAFRICA,
    VTREGION_RESERVED3,
    VTREGION_MIDDLEEAST,
    VTREGION_LASTONE
};


/// Special page and subpage values
enum
{
    /*
     * These values are local use and not from
     * any VT standards.  Where these are used
     * need to be under strict control because
     * not all path look out for these special
     * values.
     */

    VTPAGE_ERROR                = 0x000,
    
    VTPAGE_FLOFRED              = 0x010,
    VTPAGE_FLOFGREEN            = 0x011,
    VTPAGE_FLOFYELLOW           = 0x012,
    VTPAGE_FLOFBLUE             = 0x013,
    VTPAGE_PREVIOUS             = 0x020,

    VTPAGE_NULLMASK             = 0x0FF,

    VTSUBPAGE_NULL              = 0x3F7F,
    VTSUBPAGE_UNSET             = 0xFFFF,
};


/// Bit vector for LineState
enum
{
    CACHESTATE_HASDATA       = 1 << 0,
    CACHESTATE_UPDATED       = 1 << 1,
    CACHESTATE_HASERROR      = 1 << 2,
};


/// The description of a VideoText page
typedef struct _VTPage
{
    DWORD   dwPageCode;
    WORD    wControlBits;
    BYTE    Frame[26][40];
    BYTE    LineState[26];
    DWORD   EditorialLink[6];
    BOOL    bBufferReserved;
    BOOL    bReceived;
    BOOL    bShowRow24;
    _VTPage*pNextPage;
} TVTPage;


/// Used to mark out selections
typedef struct _VTLeftRight
{
    BYTE            Left;
    BYTE            Right;
    _VTLeftRight*   Next;
} TVTLeftRight;


/// ParsePageElements and ParseLineElements flags
enum
{
    // ParsePageElements input
    PARSE_HASDATAONLY           = 0x0001,
    PARSE_UPDATEDONLY           = 0x0002,
    PARSE_FORCEHEADER           = 0x0004,
    PARSE_REPEATDOUBLE          = 0x0008,
    PARSE_ALSOKEYWORDS          = 0x0010,

    // Parse line input
    PARSE_SKIPDOUBLECHECK       = 0x0020,

    // ParseLineElements output
    PARSE_DOUBLEHEIGHT          = 0x0100,

    // Callback input
    PARSE_DOUBLEREPEAT          = 0x0040,
    PARSE_EMPTYLINE             = 0x0080,

    // Callback return values
    PARSE_CONTINUE              = 0,
    PARSE_STOPLINE              = 1,
    PARSE_STOPPAGE              = 2,

    // ParseLineElements return values

    PARSE_NODATA                = 3,
    PARSE_SUPPRESSHEADER        = 4,
    PARSE_INHIBITDISP           = 5,
    PARSE_ROW24HIDDEN           = 6,
    PARSE_SKIPKEYWORDS          = 7,
    PARSE_NOTUPDATED            = 8,
    PARSE_DOUBLESKIP            = 9,
    PARSE_CONFIGERROR           = 10,
};


class CVTCommon
{
public:
    /// The callback function used by ParsePageElements and ParseLineElements
    typedef BYTE (TParserCallback)(TVTPage* pPage, WORD wPoint, LPWORD lpFlags,
                WORD wColour, BYTE uChar, BYTE uMode, LPVOID lpParam);

public:
    CVTCommon();
    ~CVTCommon();

public:
    // Checks whether the page number is a non-visible page
    static BOOL IsNonVisiblePage(WORD wPageHex);

    // Checks if the page has the subtitle or newsflash bits
    static BOOL IsBoxedOnlyPage(TVTPage* pPage);
    // Check if the page contains flashing text or graphics
    static BOOL HasFlashElement(TVTPage* pPage);
    // Check if the line is a double height line
    static BOOL HasDoubleElement(TVTPage* pPage, BYTE nRow);

    // Reverses the bits in an 8-bit byte
    static BYTE ReverseBits(BYTE Byte);

    // Checks if the data is odd parity.  The parity bit is the 8th
    static BOOL CheckParity(BYTE* Data, ULONG Length = 1, BOOL bStripParityBits = FALSE);

    // Unhams a Hamming 8/4 coded byte
    static BYTE Unham84(BYTE Byte, BOOL* Error);
    static BYTE Unham84(BYTE* pByte, BOOL* Error);
    // Unhams two Hamming 8/4 coded bytes.  The first byte becomes the MSB
    static BYTE UnhamTwo84_MSBF(BYTE Byte[2], BOOL* Error);
    // Unhams two Hamming 8/4 coded bytes.  The first byte becomes the LSB
    static BYTE UnhamTwo84_LSBF(BYTE Byte[2], BOOL* Error);

    // Unhams a Hamming 24/18 coded triplet
    static DWORD Unham2418(BYTE Byte[3], BOOL* Error);
    static void Unham2418(BYTE* Source, BYTE* Dest, BYTE BitOffset, BOOL* Error);

    // Goes through the page and calls the callback on every character
    static BYTE ParsePageElements(TVTPage* pPage, LPWORD lpFlags,
                                  TParserCallback* fnParseProc, LPVOID lpParam);

    // Goes through the line and calls the callback on every character
    static BYTE ParseLineElements(TVTPage* pPage, BYTE nRow, LPWORD lpFlags,
                                  TParserCallback* fnParseProc, LPVOID lpParam);

    // Gets the codepage given the region and the character sub-set code
    static eVTCodepage GetRegionCodepage(eVTRegion VTRegion,
                                BYTE uCharacterSubsetCode, BOOL bCorrectMissing);

private:
    static BYTE CheckLineParsable(TVTPage* pPage, BYTE nRow, LPWORD lpFlags);

    static BYTE ParseEmptyLine(TVTPage* pPage, BYTE nRow, LPWORD lpFlags,
                               TParserCallback* fnParseProc, LPVOID lpParam);

    static BYTE HasFlashElementProc(TVTPage*, WORD, LPWORD, WORD, BYTE uChar,
                                    BYTE uMode, LPVOID);

private:
    /// Relevant bits in the DecodeTable
    enum
    {
        DECODE_8_4_RESULT   = 0x0F,
        DECODE_8_4_ERROR    = 0x10,
        DECODE_PARITY_BIT   = 0x20,
    };

    static BYTE Parity(BYTE Byte);
    static void InitializeDecodeTable();

private:
    static BYTE m_DecodeTable[256];
    static BOOL m_DecodeTableInitialized;

    static eVTCodepage m_RegionCodepage[VTREGION_LASTONE][8];
};

#endif
