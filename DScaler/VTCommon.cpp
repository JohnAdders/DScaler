/////////////////////////////////////////////////////////////////////////////
// $Id: VTCommon.cpp,v 1.3 2003-01-12 17:12:45 atnak Exp $
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
// Revision 1.2  2003/01/05 16:09:44  atnak
// Updated TopText for new teletext
//
// Revision 1.1  2003/01/01 20:40:48  atnak
// Inital release of videotext common + utilities class.
//
//
//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "VTCommon.h"


BYTE CVTCommon::m_DecodeTable[256];
BOOL CVTCommon::m_DecodeTableInitialized = FALSE;


/// The codepages used in various regions
eVTCodepage CVTCommon::m_RegionCodepage[VTREGION_LASTONE][8] =
{
    // VTREGION_DEFAULT
    { VTCODEPAGE_ENGLISH,   VTCODEPAGE_FRENCH,  VTCODEPAGE_SWEDISH,     VTCODEPAGE_CZECH,   VTCODEPAGE_GERMAN,  VTCODEPAGE_PORTUGUESE,  VTCODEPAGE_ITALIAN,     VTCODEPAGE_NONE     },

    // VTREGION_CENTRALEUROPE
    { VTCODEPAGE_POLISH,    VTCODEPAGE_SAME,    VTCODEPAGE_SAME,        VTCODEPAGE_SAME,    VTCODEPAGE_SAME,    VTCODEPAGE_NONE,        VTCODEPAGE_SAME,        VTCODEPAGE_NONE     },

    // VTREGION_SOUTHERNEUROPE
    { VTCODEPAGE_SAME,      VTCODEPAGE_SAME,    VTCODEPAGE_SAME,        VTCODEPAGE_TURKISH, VTCODEPAGE_SAME,    VTCODEPAGE_SAME,        VTCODEPAGE_SAME,        VTCODEPAGE_NONE     },

    // VTREGION_BALKANS
    { VTCODEPAGE_NONE,      VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,    VTCODEPAGE_NONE,    VTCODEPAGE_SLOVENIAN,   VTCODEPAGE_NONE,        VTCODEPAGE_RUMANIAN },

    // VTREGION_EASTERNEUROPE
    { VTCODEPAGE_SERBIAN,   VTCODEPAGE_RUSSIAN, VTCODEPAGE_ESTONIAN,    VTCODEPAGE_SAME,    VTCODEPAGE_SAME,    VTCODEPAGE_UKRAINIAN,   VTCODEPAGE_LETTISH,     VTCODEPAGE_NONE     },

    // VTREGION_RESERVED1
    { VTCODEPAGE_NONE,      VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,    VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,        VTCODEPAGE_NONE     },

    // VTREGION_MEDITERRANEAN
    { VTCODEPAGE_NONE,      VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_TURKISH, VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,        VTCODEPAGE_GREEK    },

    // VTREGION_RESERVED2
    { VTCODEPAGE_NONE,      VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,    VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,        VTCODEPAGE_NONE     },

    // VTREGION_NORTHAFRICA
    { VTCODEPAGE_ENGLISHA,  VTCODEPAGE_FRENCHA, VTCODEPAGE_NONE,        VTCODEPAGE_NONE,    VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,        VTCODEPAGE_ARABIC   },

    // VTREGION_RESERVED3
    { VTCODEPAGE_NONE,      VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,    VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,        VTCODEPAGE_NONE     },

    // VTREGION_MIDDLEEAST
    { VTCODEPAGE_NONE,      VTCODEPAGE_NONE,    VTCODEPAGE_NONE,        VTCODEPAGE_NONE,    VTCODEPAGE_NONE,    VTCODEPAGE_HEBREW,      VTCODEPAGE_NONE,        VTCODEPAGE_ARABIC   },
};


CVTCommon::CVTCommon()
{
    if (m_DecodeTableInitialized == FALSE)
    {
        InitializeDecodeTable();
    }
}


CVTCommon::~CVTCommon()
{
}


/// Reverses the bits in a byte
BYTE CVTCommon::ReverseBits(BYTE Byte)
{
    Byte = (Byte & 0x0F) << 4 | (Byte & 0xF0) >> 4;
    Byte = (Byte & 0x33) << 2 | (Byte & 0xCC) >> 2;
    Byte = (Byte & 0x55) << 1 | (Byte & 0xAA) >> 1;

    return Byte;
}


/// Returns the bit value required to make Byte Odd Parity
BYTE CVTCommon::Parity(BYTE Byte)
{
    return (m_DecodeTable[Byte] & DECODE_PARITY_BIT) != 0;
}


/// Check the odd parity-ness of a string
BOOL CVTCommon::CheckParity(BYTE* Data, ULONG Length, BOOL bStripParityBits)
{
    BOOL ErrorFree = TRUE;

    for (ULONG i(0); i < Length; i++)
    {
        if (Parity(Data[i]) != 0)
        {
            ErrorFree = FALSE;
        }

        if (bStripParityBits != FALSE)
        {
            Data[i] &= 0x7F;
        }
    }

    return ErrorFree;
}


BYTE CVTCommon::Unham84(BYTE* pByte, BOOL* Error)
{
    return Unham84(*pByte, Error);
}


BYTE CVTCommon::Unham84(BYTE Byte, BOOL* Error)
{
    if (m_DecodeTable[Byte] & DECODE_8_4_ERROR)
    {
        *Error = TRUE;
    }

    return m_DecodeTable[Byte] & DECODE_8_4_RESULT;
}


BYTE CVTCommon::UnhamTwo84_LSBF(BYTE Byte[2], BOOL* Error)
{
    BYTE Result;

    Result = Unham84(Byte[0], Error);
    Result |= Unham84(Byte[1], Error) << 4;

    return Result;
}


BYTE CVTCommon::UnhamTwo84_MSBF(BYTE Byte[2], BOOL* Error)
{
    BYTE Result;

    Result = Unham84(Byte[0], Error) << 4;
    Result |= Unham84(Byte[1], Error);

    return Result;
}


DWORD CVTCommon::Unham2418(BYTE Byte[3], BOOL* Error)
{
    BYTE b1, b2, b3;
    BYTE Syndrome = 0;
    DWORD Data = 0UL;

    b1 = Byte[0];
    b2 = Byte[1];
    b3 = Byte[2];

    Syndrome += (Parity(b1 & 0x55) ^ Parity(b2 & 0x55) ^ Parity(b3 & 0x55)) * 0x01;
    Syndrome += (Parity(b1 & 0x66) ^ Parity(b2 & 0x66) ^ Parity(b3 & 0x66)) * 0x02;
    Syndrome += (Parity(b1 & 0x78) ^ Parity(b2 & 0x78) ^ Parity(b3 & 0x78)) * 0x04;
    Syndrome += (Parity(b1 & 0x80) ^ Parity(b2 & 0x7F)) * 0x08;
    Syndrome += (Parity(b2 & 0x80) ^ Parity(b3 & 0x7F)) * 0x10;

    if (Syndrome != 0 && (Parity(b1) ^ Parity(b2) ^ Parity(b3)) == 0)
    {
        // Double error
        *Error = TRUE;
    }
    else if (Syndrome > 24)
    {
        // Error
        *Error = TRUE;
    }
    else
    {
        Syndrome--;
        switch ((BYTE) (Syndrome / 8))
        {
        case 0:
            b1 ^= (1 << (Syndrome % 8));
            break;
        case 2:
            b2 ^= (1 << (Syndrome % 8));
            break;
        case 3:
            b3 ^= (1 << (Syndrome % 8));
            break;
        default:
            break;
        }

        Data |= (b3 & 0x7F) << 11;
        Data |= (b2 & 0x7F) << 4;
        Data |= (b1 & 0x70) >> 3;
        Data |= (b1 & 0x04) >> 2;
    }

    return Data;
}


void CVTCommon::Unham2418(BYTE Source[3], BYTE* Dest, BYTE BitOffset, BOOL* Error)
{
    // Zero out the destination
    for (int BitsLeft = BitOffset, i = 0; BitsLeft > 0; BitsLeft -= 8, i++)
    {
        Dest[i] &= ~(0xff << BitsLeft);
    }
    for (BitsLeft += 18; BitsLeft > 0; BitsLeft -= 8, i++)
    {
        Dest[i] &= (0xff << BitsLeft);
    }

    DWORD Data = Unham2418(Source, Error);

    *((DWORD*)&Dest[BitOffset / 8]) |= (Data << (BitOffset % 8));
}


void CVTCommon::InitializeDecodeTable()
{
    BYTE Parity;
    BYTE Data;
    int i;

    // Create the Odd Parity checker
    for (i = 0; i < 256; i++)
    {
        Parity = i ^ (i >> 4);
        Parity ^= Parity >> 2;
        Parity ^= Parity >> 1;

        m_DecodeTable[i] = (Parity & 1) ? 0 : DECODE_PARITY_BIT;
    }

    // Create the Hamming 8/4 decoder
    for (i = 0; i < 256; i++)
    {
        Parity = 0x80;
        Parity >>= ((m_DecodeTable[i & 0xA3] & DECODE_PARITY_BIT) != 0) * 1;
        Parity >>= ((m_DecodeTable[i & 0x8E] & DECODE_PARITY_BIT) != 0) * 2;
        Parity >>= ((m_DecodeTable[i & 0x3A] & DECODE_PARITY_BIT) != 0) * 4;

        if ((Parity != 0x80) && (m_DecodeTable[i] & DECODE_PARITY_BIT) == 0)
        {
            // Double bit error
            m_DecodeTable[i] |= DECODE_8_4_ERROR | DECODE_8_4_RESULT;
            continue;
        }

        if (Parity >> 3)
        {
            Parity &= 0x10;
            Parity >>= 1;
        }

        Data = (i & (1 << 1)) >> 1;
        Data |= (i & (1 << 3)) >> 2;
        Data |= (i & (1 << 5)) >> 3;
        Data |= (i & (1 << 7)) >> 4;

        // Correct an incorrect bit
        Data ^= Parity;

        m_DecodeTable[i] |= Data;
    }

    m_DecodeTableInitialized = TRUE;
}


BOOL CVTCommon::IsNonVisiblePage(WORD wPageHex)
{
    if ((wPageHex & 0xFF00) < 0x0100 ||
        (wPageHex & 0xFF00) > 0x0800)
    {
        return FALSE;
    }

    if ((wPageHex & 0x00F0) >= 0x00A0 ||
        (wPageHex & 0x000F) >= 0x000A)
    {
        return TRUE;
    }

    return FALSE;
}


BOOL CVTCommon::IsBoxedOnlyPage(TVTPage* pPage)
{
    // If newsflash or subtitle is set, only boxed text
    // is to be displayed
    if ((pPage->wControlBits & VTCONTROL_NEWSFLASH) != 0 ||
        (pPage->wControlBits & VTCONTROL_SUBTITLE) != 0)
    {
        return TRUE;
    }
    return FALSE;
}


BOOL CVTCommon::HasDoubleElement(TVTPage* pPage, BYTE nRow)
{
    if ((pPage->LineState[nRow] & CACHESTATE_HASDATA) == 0)
    {
        return FALSE;
    }

    for (BYTE nCol = 0; nCol < 40; nCol++)
    {
        if ((pPage->Frame[nRow][nCol] & 0x7F) == 0x0D)
        {
            return TRUE;
        }
    }
    return FALSE;
}


BOOL CVTCommon::HasFlashElement(TVTPage* pPage)
{
    WORD wFlags = PARSE_HASDATAONLY;

    return ParsePageElements(pPage, &wFlags,
        (TParserCallback*)HasFlashElementProc, NULL) == PARSE_STOPPAGE;
}


eVTCodepage CVTCommon::GetRegionCodepage(eVTRegion VTRegion,
                                                    BYTE uCharacterSubsetCode,
                                                    BOOL bCorrectMissing)
{
    if (uCharacterSubsetCode > 0x07)
    {
        return VTCODEPAGE_ENGLISH;
    }

    eVTCodepage Codepage = m_RegionCodepage[VTRegion][uCharacterSubsetCode];

    // Same indicates it's the same as the first
    if (Codepage == VTCODEPAGE_SAME)
    {
        Codepage = m_RegionCodepage[VTREGION_DEFAULT][uCharacterSubsetCode];
    }

    // If it's none, the region is wrong.  Pick the
    // first valid one we come accross
    if (bCorrectMissing != FALSE && Codepage == VTCODEPAGE_NONE)
    {
        for (int i = VTREGION_DEFAULT; i < VTREGION_LASTONE; i++)
        {
            Codepage = m_RegionCodepage[i][uCharacterSubsetCode];

            if (Codepage != VTCODEPAGE_NONE &&
                Codepage != VTCODEPAGE_SAME)
            {
                break;
            }
        }
    }

    return Codepage;
}


/*
 *  ParsePageElements
 *
 *  Parses all the elements in a Teletext page and calls the
 *  supplied callback function for all the characters.  This
 *  function understands the exclusive control characters in
 *  Teletext pages and should be used rather than attempting
 *  to parse pages with normal string manipulation functions.
 *
 *  These are the relavent uFlags:
 *
 *    PARSE_HASDATAONLY    - Only parse lines with HASDATA set
 *    PARSE_UPDATEDONLY    - Only parse lines with UPDATED set
 *    PARSE_REPEATDOUBLE   - Repeat lines with double in it as
 *                           the contents of the following row
 *    PARSE_FORCEDHEADER   - Always perform parsing for header
 *    PARSE_ALSOKEYWORDS   - Include keywords line for parsing
 *
 *  The parser callback function should return one of these:
 *
 *    PARSE_CONTINUE      - Continue normally to the next char
 *    PARSE_STOPLINE      - Stop the currently processing line
 *    PARSE_STOPPAGE      - Stop parsing the rest of this page
 *
 *  The return value of ParsePageElements is the value returned
 *  by the last call of ParseLineElements.  This may or may not
 *  be the same value returned by the last call to the callback
 *  function.
 *
 *  ParseLineElements may return one of these values:
 *
 *    PARSE_STOPLINE        - The callback returned this value
 *    PARSE_STOPPAGE        - The callback returned this value
 *    PARSE_NODATA          - PARSE_HASDATAONLY is set and the
 *                            line does not have received data
 *    PARSE_SUPPRESSHEADER  - PARSE_HASDATAONLY is set and the
 *                            header is suppressed by the page
 *    PARSE_INHIBITDISP     - PARSE_HASDATAONLY is set and the
 *                            display is inhibited by the page
 *    PARSE_ROW24HIDDEN     - PARSE_HASDATAONLY is set and the
 *                            line 24 is not shown by the page
 *    PARSE_SKIPKEYWORDS    - PARSE_ALSOKEYWORDS bit is not on
 *                            and the line was navigation line
 *    PARSE_NOTUPDATED      - PARSE_UPDATEDONLY is set and the
 *                            line state is not set to updated
 *    PARSE_DOUBLESKIP      - PARSE_REPEATDOUBLE bit is not on
 *                            and this line is invalid because
 *                            of a double on the previous line
 *    PARSE_CONFIGERROR     - Bad flags were set with the call
 *
 */
BYTE CVTCommon::ParsePageElements(TVTPage* pPage, LPWORD lpFlags,
                                  TParserCallback* fnParseProc, LPVOID lpParam)
{
    BYTE LastRow;
    LONG uResult = PARSE_CONTINUE;
    WORD wFlags = *lpFlags;

    if ((*lpFlags & PARSE_ALSOKEYWORDS) != 0)
    {
        LastRow = 25;
    }
    else
    {
        LastRow = 24;
    }

    *lpFlags &= ~(PARSE_DOUBLEHEIGHT | PARSE_DOUBLEREPEAT);

    // Loop through the rows
    for (BYTE nRow = 0; nRow <= LastRow; nRow++)
    {
        wFlags = *lpFlags | PARSE_SKIPDOUBLECHECK;

        // Call the line elements parser
        if ((uResult = ParseLineElements(pPage, nRow, &wFlags,
            fnParseProc, lpParam)) == PARSE_STOPPAGE)
        {
            break;
        }

        if (uResult == PARSE_NOTUPDATED)
        {
            // The line was not updated so the necessary double
            // check was not performed.  Do it here.
            if (nRow >= 1 && nRow < 23 && HasDoubleElement(pPage, nRow))
            {
                wFlags |= PARSE_DOUBLEHEIGHT;
            }
        }

        if ((wFlags & PARSE_DOUBLEHEIGHT) != 0)
        {
            wFlags = *lpFlags | PARSE_DOUBLEREPEAT | PARSE_SKIPDOUBLECHECK;

            // Call the line parser again for the repeat
            if ((uResult = ParseLineElements(pPage, nRow, &wFlags,
                fnParseProc, lpParam)) == PARSE_STOPPAGE)
            {
                break;
            }
            nRow++;
        }
    }

    *lpFlags = wFlags;

    return uResult;
}


BYTE CVTCommon::ParseLineElements(TVTPage* pPage, BYTE nRow, LPWORD lpFlags,
                                  TParserCallback* fnParseProc, LPVOID lpParam)
{
    BYTE DisplayChar;
    BYTE DisplayModes;
    BYTE DisplayColour;
    BYTE DisplayBkColour;
    BYTE DisplayRow;
    BYTE SetAfterModes;
    BYTE SetAfterColour;
    BYTE Background;
    BYTE nLastBoxPos;
    BYTE nLastUnboxPos;
    BYTE HeldGraphChar;
    BOOL bHeldGraphSeparated = FALSE;
    BOOL bBoxedElementsOnly = FALSE;
    LONG uResult;

    if ((*lpFlags & PARSE_DOUBLEREPEAT) != 0)
    {
        if (nRow < 1 || nRow >= 23)
        {
            // Programming error
            ASSERT(FALSE);
            return PARSE_CONFIGERROR;
        }
    }

    // Check if we are to look for doubles in the previous row
    if ((*lpFlags & PARSE_SKIPDOUBLECHECK) == 0 && nRow > 1 && nRow <= 23)
    {
        // Check if the previous line had a double
        if (HasDoubleElement(pPage, nRow - 1))
        {
            // Check that this isn't already a double repeated
            if ((*lpFlags & PARSE_DOUBLEREPEAT) != 0)
            {
                BYTE nConsecutiveDoubles = 1;

                // There are problematic doubles in the page
                for (DisplayRow = nRow - 2; DisplayRow >= 1 &&
                    HasDoubleElement(pPage, DisplayRow); DisplayRow--)
                {
                    nConsecutiveDoubles++;
                }

                // Check if this double repeat is valid
                if ((nConsecutiveDoubles % 2) == 1)
                {
                    // This row is invalid, so the double repeat
                    // is also invalid.  Redo the next row line
                    // as a non-repeat
                    *lpFlags &= ~PARSE_DOUBLEREPEAT;
                    nRow++;
                }
                else
                {
                    // Ignore the double on the previous line
                    // and continue repeating the double on
                    // this line as the next row line.
                }
            }
            else
            {
                // Repeat the previous as this line
                *lpFlags |= PARSE_DOUBLEREPEAT;
                return ParseLineElements(pPage, nRow - 1, lpFlags,
                                         fnParseProc, lpParam);
            }
        }
    }

    // If this row is a repeat, the actual row is the one below
    if ((*lpFlags & PARSE_DOUBLEREPEAT) != 0)
    {
        if ((*lpFlags & PARSE_REPEATDOUBLE) == 0)
        {
            return PARSE_DOUBLESKIP;
        }

        DisplayRow = nRow + 1;
    }
    else
    {
        DisplayRow = nRow;
    }

    // Check if we need to process this line
    uResult = CheckLineParsable(pPage, nRow, lpFlags);

    if (uResult != PARSE_CONTINUE)
    {
        if ((*lpFlags & PARSE_EMPTYLINE) != 0)
        {
            return ParseEmptyLine(pPage, nRow, lpFlags, fnParseProc, lpParam);
        }
        return uResult;
    }

    if (nRow != 25)
    {
        // If newsflash or subtitle is set, only boxed elements
        // should be displayed
        if ((pPage->wControlBits & VTCONTROL_NEWSFLASH) != 0 ||
            (pPage->wControlBits & VTCONTROL_SUBTITLE) != 0)
        {
            // Ignore if force header is on
            if (nRow != 0 || (*lpFlags & PARSE_FORCEHEADER) == 0)
            {
                bBoxedElementsOnly = TRUE;
            }
        }
    }

    DisplayModes = 0;
    DisplayColour = VTCOLOR_WHITE;
    Background = VTCOLOR_BLACK;
    HeldGraphChar = 0x20;

    nLastBoxPos = 40;
    nLastUnboxPos = 40;

    for(BYTE nCol = 0; nCol < 40; nCol++)
    {
        SetAfterModes = 0;
        SetAfterColour = DisplayColour;

        DisplayChar = pPage->Frame[nRow][nCol] & 0x7F;

        // Remember the most recent graphics character
        if ((DisplayModes & VTMODE_GRAPHICS) && (DisplayChar & 0x20))
        {
            HeldGraphChar = DisplayChar;
            bHeldGraphSeparated = (DisplayModes & VTMODE_SEPARATED);
        }

        // Check if the character is a control character
        if (DisplayChar < 0x20)
        {
            switch(DisplayChar)
            {
            case 0x00:  // NUL (reserved) (2.5+: Alpha Black)
                // Workaround to 1.x vs 2.5+ conflict
                if (Background == 0)
                {
                    break;
                }
            case 0x01:  // Alpha Red
            case 0x02:  // Alpha Green
            case 0x03:  // Alpha Yellow
            case 0x04:  // Alpha Blue
            case 0x05:  // Alpha Magenta
            case 0x06:  // Alpha Cyan
            case 0x07:  // Alpha White
                SetAfterColour = DisplayChar;
                SetAfterModes |= (DisplayModes & VTMODE_GRAPHICS);
                SetAfterModes |= (DisplayModes & VTMODE_CONCEAL);
                break;
            case 0x08:  // Flash
                SetAfterModes |= (DisplayModes & VTMODE_FLASH ^ VTMODE_FLASH);
                break;
            case 0x09:  // Steady (immediate)
                DisplayModes &= ~VTMODE_FLASH;
                break;
            case 0x0a:  // Unboxed (immediate on 2nd)
                if (nCol == (nLastUnboxPos + 1))
                {
                    DisplayModes &= ~VTMODE_BOXED;
                }
                else
                {
                    nLastUnboxPos = nCol;
                }
                break;
            case 0x0b:  // Boxed (immediate on 2nd)
                if (nCol == (nLastBoxPos + 1))
                {
                    DisplayModes |= VTMODE_BOXED;
                }
                else
                {
                    nLastBoxPos = nCol;
                }
                break;
            case 0x0c:  // Normal Height (immediate)
                DisplayModes &= ~VTMODE_DOUBLE;
                break;
            case 0x0d:  // Double Height
                // Double on rows 0, 23, 24, 25 not permitted
                if (nRow > 0 && nRow < 23)
                {
                    *lpFlags |= PARSE_DOUBLEHEIGHT;
                    SetAfterModes |= (DisplayModes & VTMODE_DOUBLE ^ VTMODE_DOUBLE);
                }
                break;
            case 0x0e:  // Shift Out (reserved)
            case 0x0f:  // Shift In (reserved
                break;
            case 0x10:  // DLE (reserved) (2.5+: Graphics Black)
                // Workaround to 1.x vs 2.5+ conflict
                if (Background == 0)
                {
                    break;
                }
            case 0x11:  // Graphics Red
            case 0x12:  // Graphics Green
            case 0x13:  // Graphics Yellow
            case 0x14:  // Graphics Blue
            case 0x15:  // Graphics Magenta
            case 0x16:  // Graphics Cyan
            case 0x17:  // Graphics White
                SetAfterColour = DisplayChar - 0x10;
                SetAfterModes |= (DisplayModes & VTMODE_GRAPHICS ^ VTMODE_GRAPHICS);
                SetAfterModes |= (DisplayModes & VTMODE_CONCEAL);
                break;
            case 0x18:  // Conceal (immediate)
                DisplayModes |= VTMODE_CONCEAL;
                break;
            case 0x19:  // Contiguous Graphics (either)
                DisplayModes &= ~VTMODE_SEPARATED;
                break;
            case 0x1a:  // Separated Graphics (either)
                DisplayModes |= VTMODE_SEPARATED;
                break;
            case 0x1b:  // ESC (reserved)
                break;
            case 0x1c:  // Black Background (immediate)
                Background = VTCOLOR_BLACK;
                break;
            case 0x1d:  // New Background (immediate)
                Background = DisplayColour;
                break;
            case 0x1e:  // Hold Graphics (immediate)
                DisplayModes |= VTMODE_HOLD;
                break;
            case 0x1f:  // Release Graphics
                SetAfterModes |= (DisplayModes & VTMODE_HOLD);
                break;
            }

            DisplayChar = 0x20;

            if ((DisplayModes & VTMODE_HOLD) != 0)
            {
                DisplayChar = HeldGraphChar;
                if (!bHeldGraphSeparated != !(DisplayModes & VTMODE_SEPARATED))
                {
                    DisplayModes ^= VTMODE_SEPARATED;
                    SetAfterModes ^= VTMODE_SEPARATED;
                }
            }
        }

        if ((bBoxedElementsOnly != FALSE) && !(DisplayModes & VTMODE_BOXED))
        {
            DisplayChar = 0x20;
            DisplayColour = VTCOLOR_NONE;
            DisplayBkColour = VTCOLOR_NONE;
        }
        else
        {
            DisplayBkColour = Background;
        }

        // Call the callback function
        uResult = (fnParseProc)(pPage, MAKEWORD(DisplayRow, nCol), lpFlags,
                                MAKEWORD(DisplayColour, DisplayBkColour),
                                DisplayChar, DisplayModes, lpParam);

        if (uResult == PARSE_STOPLINE || uResult == PARSE_STOPPAGE)
        {
            break;
        }

        DisplayModes ^= SetAfterModes;
        DisplayColour = SetAfterColour;

        // Forget the last graph character if one of these changed
        if (SetAfterModes & VTMODE_GRAPHICS || SetAfterModes & VTMODE_DOUBLE)
        {
            HeldGraphChar = 0x20;
        }
    }
    return uResult;
}


BYTE CVTCommon::CheckLineParsable(TVTPage* pPage, BYTE nRow, LPWORD lpFlags)
{
    if ((*lpFlags & PARSE_EMPTYLINE) != 0)
    {
        return PARSE_NODATA;
    }

    if (nRow == 0 && (*lpFlags & PARSE_FORCEHEADER) != 0)
    {
        if ((pPage->LineState[nRow] & CACHESTATE_HASDATA) != 0)
        {
            return PARSE_CONTINUE;
        }
    }

    // Check if the caller wants updated rows only
    if ((*lpFlags & PARSE_UPDATEDONLY) != 0)
    {
        if ((pPage->LineState[nRow] & CACHESTATE_UPDATED) == 0)
        {
            return PARSE_NOTUPDATED;
        }
    }

    BYTE uCheckResult = PARSE_CONTINUE;

    // If this row has no data
    if ((pPage->LineState[nRow] & CACHESTATE_HASDATA) == 0)
    {
        uCheckResult = PARSE_NODATA;
    }
    // If this row has data and is the header row
    else if (nRow == 0)
    {
        if ((pPage->wControlBits & VTCONTROL_SUPRESSHEADER) != 0)
        {
            uCheckResult = PARSE_SUPPRESSHEADER;
        }
    }
    // If this row has data and is the commentary row
    else if (nRow == 24)
    {
        if (pPage->bShowRow24 == FALSE)
        {
            uCheckResult = PARSE_ROW24HIDDEN;
        }
    }
    // If this row has data and is the keywords row
    else if (nRow == 25)
    {
        if ((*lpFlags & PARSE_ALSOKEYWORDS) == 0)
        {
            // Has data only flag doesn't affect his row
            return PARSE_SKIPKEYWORDS;
        }
    }
    // For all other rows
    else
    {
        if ((pPage->wControlBits & VTCONTROL_INHIBITDISP) != 0)
        {
            uCheckResult = PARSE_INHIBITDISP;
        }
    }

    if (uCheckResult != PARSE_CONTINUE)
    {
        if ((*lpFlags & PARSE_HASDATAONLY) == 0)
        {
            *lpFlags = PARSE_EMPTYLINE;
        }
    }

    return uCheckResult;
}


BYTE CVTCommon::ParseEmptyLine(TVTPage* pPage, BYTE nRow, LPWORD lpFlags,
                               TParserCallback* fnParseProc, LPVOID lpParam)
{
    LONG uResult = PARSE_CONTINUE;

    WORD wColour = MAKEWORD(VTCOLOR_WHITE, VTCOLOR_BLACK);

    if ((pPage->wControlBits & VTCONTROL_NEWSFLASH) != 0 ||
        (pPage->wControlBits & VTCONTROL_SUBTITLE) != 0)
    {
        if (nRow != 0 || (*lpFlags & PARSE_FORCEHEADER) == 0)
        {
            wColour = MAKEWORD(VTCOLOR_WHITE, VTCOLOR_NONE);
        }
    }

    for(BYTE nCol = 0; nCol < 40; nCol++)
    {
        uResult = (fnParseProc)(pPage, MAKEWORD(nRow, nCol), lpFlags,
                                wColour, 0x20, 0x00, lpParam);

        if (uResult == PARSE_STOPLINE || uResult == PARSE_STOPPAGE)
        {
            break;
        }
    }

    return uResult;
}


BYTE CVTCommon::HasFlashElementProc(TVTPage*, WORD, LPWORD lpFlags, WORD,
                                    BYTE uChar, BYTE uMode, LPVOID)
{
    if ((uMode & VTMODE_FLASH) != 0)
    {
        if (uChar > 0x20)
        {
            return PARSE_STOPPAGE;
        }
    }
    return PARSE_CONTINUE;
}

