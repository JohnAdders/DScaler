/////////////////////////////////////////////////////////////////////////////
// $Id: VTDrawer.cpp,v 1.22 2004-04-24 08:35:15 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2002 Mike Temperton.  All rights reserved.
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
//  GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
//  Portions Copyright (C) 2000 John Adcock
//
/////////////////////////////////////////////////////////////////////////////
//
// Change Log
//
// Date          Developer             Changes
//
// 02 Jan 2003   Atsushi Nakagawa      Redid the CVTDrawer class to tie in
//                                     with the new CVTCommon class.  CVS
//                                     Log entries prior to and including
//                                     rev 1.16 may no longer have direct
//                                     validity but their cumulative changes
//                                     still remain valid.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.21  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.20  2003/01/24 01:55:17  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.19  2003/01/05 16:09:44  atnak
// Updated TopText for new teletext
//
// Revision 1.18  2003/01/02 21:25:41  atnak
// Fixes for row24 find page number
//
// Revision 1.17  2003/01/01 20:43:13  atnak
// New drawer for new videotext layout
//
// Revision 1.16  2002/10/30 03:31:47  atnak
// Made mixed mode use nonantialias fonts iff there is a transparent background.
//
// Revision 1.15  2002/10/23 16:57:12  atnak
// Added TOP-Text support
//
// Revision 1.14  2002/10/23 02:52:33  atnak
// Added Alpha Black and Mosaic Black for level 2.5
//
// Revision 1.13  2002/10/20 09:29:59  atnak
// minor fixes
//
// Revision 1.12  2002/10/15 11:53:38  atnak
// Added UI feedback for some videotext stuff
//
// Revision 1.11  2002/10/12 18:43:32  atnak
// some changes for tranparency and boxed background
//
// Revision 1.10  2002/10/12 00:38:07  atnak
// Changed Draw() to be BBC specs compatible
//
// Revision 1.9  2002/08/06 21:35:08  robmuller
// Don't pause the image when VideoText contains transparency.
//
// Revision 1.8  2002/06/20 20:00:32  robmuller
// Implemented videotext search highlighting.
//
// Revision 1.7  2002/05/29 18:44:50  robmuller
// Added option to disable font anti-aliasing in Teletext.
//
// Revision 1.6  2002/05/24 14:49:10  robmuller
// Patch from PietOO:
// Non antialiased font when in mixed mode. Shadowed text in mixed mode.
//
// Revision 1.5  2002/05/23 22:16:32  robmuller
// Applied patch #559111 by PietOO.
// Teletext: less sparse look for ttf fonts.
//
// Revision 1.4  2002/02/24 16:41:40  temperton
// Bug fixes
//
// Revision 1.3  2002/01/19 19:52:47  temperton
// Transparent background in mix mode fix
//
// Revision 1.2  2002/01/19 12:53:00  temperton
// Teletext pages updates at correct time.
// Teletext can use variable-width font.
//
// Revision 1.1  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file VTDrawer.cpp CVTDrawer Implementation
 */

#include "stdafx.h"
#include "VTDrawer.h"
#include "VTCharacterSet.h"
#include "VTMosaicGraphics.h"

/*
 *  This class is NOT multi-thread safe
 */


/// The name of the font used to draw
const char CVTDrawer::m_szFontName[32] = "Arial";

/// The definition of constant colours
COLORREF CVTDrawer::m_ColourTable[8] =
{
    0x00000000,       // Black
    0x000000FF,       // Red
    0x0000FF00,       // Green
    0x0000FFFF,       // Yellow
    0x00FF0000,       // Blue
    0x00FF00FC,       // Magenta
    0x00FFFF00,       // Cyan
    0x00FFFFFF,       // White
};


CVTDrawer::CVTDrawer()
{
    m_hNormalFont = NULL;
    m_hDoubleFont = NULL;
    m_hNormalFontSmaller = NULL;
    m_hDoubleFontSmaller = NULL;

    m_hLowQNormalFont = NULL;
    m_hLowQDoubleFont = NULL;
    m_hLowQNormalFontSmaller = NULL;
    m_hLowQDoubleFontSmaller = NULL;

    m_TransparencyColour = 0x00FF0000;
    m_TransparencyBrush = NULL;

    m_bAntialias = FALSE;
    m_CurrentCodepage = VTCODEPAGE_NONE;

    SetRectEmpty(&m_BoundsRect);

    for (int i = 0; i < 8; i++)
    {
        m_hColourBrush[i] = CreateSolidBrush(m_ColourTable[i]);
    }
}


CVTDrawer::~CVTDrawer()
{
    for (int i = 0; i < 8; i++)
    {
        DeleteObject(m_hColourBrush[i]);
    }

    DeleteFonts();
}


void CVTDrawer::SetCodepage(eVTCodepage Codepage)
{
    if (Codepage != m_CurrentCodepage)
    {
        CVTCharacterSet::GetCharacterSet(Codepage, m_CharacterSet);
        m_CurrentCodepage = Codepage;
    }
}


void CVTDrawer::SetAntialias(BOOL bAntialias)
{
    if (bAntialias != m_bAntialias)
    {
        m_bAntialias = bAntialias;
        DeleteFonts();
    }
}


void CVTDrawer::SetTransparencyColour(COLORREF ColorRef)
{
    if (ColorRef != m_TransparencyColour)
    {
        m_TransparencyColour = ColorRef;
        DeleteObject(m_TransparencyBrush);
        m_TransparencyBrush = NULL;
    }
}


void CVTDrawer::DrawPage(HDC hDC, LPRECT lpRect, TVTPage* pPage, WORD uFlags,
                         TVTLeftRight* pHilightList[25], BYTE pDoubleProfile[25],
                         RECT pRefreshProfile[25], BYTE* pRefreshCount)
{
    WORD wParsePageFlags = PARSE_REPEATDOUBLE;

    if (uFlags & VTDF_UPDATEDONLY)
    {
        wParsePageFlags |= PARSE_UPDATEDONLY;
    }
    if (uFlags & VTDF_FORCEHEADER)
    {
        wParsePageFlags |= PARSE_FORCEHEADER;
    }
    if (uFlags & VTDF_CLOCKONLY)
    {
        uFlags |= VTDF_HEADERONLY;
    }

    if (pRefreshCount != NULL)
    {
        *pRefreshCount = 0;
    }
    else
    {
        pRefreshProfile = NULL;
    }

    SetBounds(hDC, lpRect);

    HFONT hSelectedFont = m_hNormalFont;

    LPVOID lpParams[9] = { hDC, lpRect, (LPVOID)uFlags, &hSelectedFont,
                           pHilightList, pDoubleProfile, pRefreshProfile,
                           pRefreshCount, this };

    HGDIOBJ hSaveFont = SelectObject(hDC, hSelectedFont);
    HGDIOBJ hSaveBrush = SelectObject(hDC, GetColourBrush(VTCOLOR_BLACK));
    
    SetBkMode(hDC, TRANSPARENT);

    ParsePageElements(pPage, &wParsePageFlags,
        (TParserCallback*)DrawPageProc, lpParams);

    SelectObject(hDC, hSaveBrush);
    SelectObject(hDC, hSaveFont);
}


BYTE CVTDrawer::DrawPageProc(TVTPage*, WORD wPoint, LPWORD lpFlags, WORD wColour,
                             BYTE uChar, BYTE uMode, LPVOID lpParam)
{
    HDC     hDC             = (HDC)     ((LPVOID*)lpParam)[0];
    LPRECT  lpRect          = (LPRECT)  ((LPVOID*)lpParam)[1];
    WORD    uVTDFlags       = (WORD)    ((LPVOID*)lpParam)[2];
    HFONT*  hSelectedFont   = (HFONT*)  ((LPVOID*)lpParam)[3];

    TVTLeftRight** pHilightList    = (TVTLeftRight**)((LPVOID*)lpParam)[4];
    BYTE*          pDoubleProfile  = (BYTE*)         ((LPVOID*)lpParam)[5];
    RECT*          pRefreshProfile = (RECT*)         ((LPVOID*)lpParam)[6];
    BYTE*          pRefreshCount   = (BYTE*)         ((LPVOID*)lpParam)[7];
    CVTDrawer*     pThis           = (CVTDrawer*)    ((LPVOID*)lpParam)[8];

    if ((uVTDFlags & VTDF_CLOCKONLY) && HIBYTE(wPoint) < 32)
    {
        return PARSE_CONTINUE;
    }

    if ((uVTDFlags & VTDF_HEADERONLY) && LOBYTE(wPoint) > 0)
    {
        return PARSE_STOPPAGE;
    }

    if ((uVTDFlags & VTDF_ROW24ONLY) && LOBYTE(wPoint) < 24)
    {
        return PARSE_CONTINUE;
    }

    /*
     * REPEATDOUBLE:
     *
     * First row:     dddddddDDDDDDDDDDDDD
     * Repeat row:    -------DDDDDDDDDDDDD
     *                       ^
     *                       |- DOUBLEHEIGHT
     *
     * 1. Do the first row normally until DOUBLEHEIGHT then
     *    double the background height from there on.  The
     *    foreground height is as defined by VTMODE_DOUBLE.
     * 2. Do the repeat row to fill the missing backgrounds
     *    until DOUBLEHEIGHT is encountered.
     *
     */

    if ((*lpFlags & PARSE_DOUBLEREPEAT) && (*lpFlags & PARSE_DOUBLEHEIGHT))
    {
        return PARSE_STOPLINE;
    }

    if (((uVTDFlags & VTDF_FLASHONLY) && !(uMode & VTMODE_FLASH)) ||
        ((uVTDFlags & VTDF_HIDDENONLY) && !(uMode & VTMODE_CONCEAL)))
    {
        return PARSE_CONTINUE;
    }

    BYTE nRow = LOBYTE(wPoint);
    BYTE nCol = HIBYTE(wPoint);

    RECT DrawRect = { nCol * pThis->m_dAvgWidth + 0.5,
                      nRow * pThis->m_dAvgHeight + 0.5,
                      (nCol + 1) * pThis->m_dAvgWidth + 0.5,
                      (nRow + 1) * pThis->m_dAvgHeight + 0.5 };

    if (*lpFlags & PARSE_DOUBLEHEIGHT)
    {
        DrawRect.bottom = (nRow + 2) * pThis->m_dAvgHeight + 0.5;
    }

    OffsetRect(&DrawRect, lpRect->left, lpRect->top);

    BYTE Foreground = LOBYTE(wColour);
    BYTE Background = HIBYTE(wColour);

    if ((uVTDFlags & VTDF_MIXEDMODE) && !(uMode & VTMODE_BOXED))
    {
        Background = VTCOLOR_NONE;
    }

    if (pHilightList != NULL)
    {
        TVTLeftRight* pHilight;

        if (*lpFlags & PARSE_DOUBLEREPEAT)
        {
            pHilight = pHilightList[nRow - 1];
        }
        else
        {
            pHilight = pHilightList[nRow];
        }

        while (pHilight != NULL && nCol >= pHilight->Left)
        {
            if (nCol <= pHilight->Right)
            {
                BYTE TempColour;
                TempColour = Foreground;
                Foreground = Background;
                Background = TempColour;
                break;
            }
            pHilight = pHilight->Next;
        }
    }

    FillRect(hDC, &DrawRect, pThis->GetColourBrush(Background));

    if (pRefreshProfile != NULL)
    {
        if (*pRefreshCount == 0 ||
            pRefreshProfile[*pRefreshCount-1].top != DrawRect.top)
        {
            (*pRefreshCount)++;
            pRefreshProfile[*pRefreshCount-1].top = DrawRect.top;
            pRefreshProfile[*pRefreshCount-1].left = DrawRect.left;
        }

        pRefreshProfile[*pRefreshCount-1].right = DrawRect.right;
        pRefreshProfile[*pRefreshCount-1].bottom = DrawRect.bottom;
    }

    if (*lpFlags & PARSE_DOUBLEREPEAT)
    {
        return PARSE_CONTINUE;
    }

    if (pDoubleProfile != NULL)
    {
        // Record all the lines where doubles are drawn
        pDoubleProfile[nRow] = ((*lpFlags & PARSE_DOUBLEHEIGHT) != 0);
    }

    if (uChar == 0x20)
    {
        return PARSE_CONTINUE;
    }

    if(((uMode & VTMODE_FLASH) && !(uVTDFlags & VTDF_FLASH)) ||
       ((uMode & VTMODE_CONCEAL) && !(uVTDFlags & VTDF_HIDDEN)))
    {
        return PARSE_CONTINUE;
    }

    if ((*lpFlags & PARSE_DOUBLEHEIGHT) && !(uMode & VTMODE_DOUBLE))
    {
        DrawRect.bottom = (nRow + 1) * pThis->m_dAvgHeight + 0.5;
        DrawRect.bottom += lpRect->top;
    }

    if ((uMode & VTMODE_GRAPHICS) && (uChar & 0x20))
    {
        CVTMosaicGraphics::DrawG1Mosaic(hDC, &DrawRect, uChar,
            pThis->GetColourBrush(Foreground), (uMode & VTMODE_SEPARATED) != 0);
    }
    else
    {
        HFONT hNormalFont;
        HFONT hDoubleFont;
        HFONT hNormalFontSmaller;
        HFONT hDoubleFontSmaller;

        if (Background == VTCOLOR_NONE && pThis->m_bAntialias)
        {
            pThis->CreateMissingLowQFonts(hDC);

            hNormalFont         = pThis->m_hLowQNormalFont;
            hDoubleFont         = pThis->m_hLowQDoubleFont;
            hNormalFontSmaller  = pThis->m_hLowQNormalFontSmaller;
            hDoubleFontSmaller  = pThis->m_hLowQDoubleFontSmaller;
        }
        else
        {
            pThis->CreateMissingStandardFonts(hDC);

            hNormalFont         = pThis->m_hNormalFont;
            hDoubleFont         = pThis->m_hDoubleFont;
            hNormalFontSmaller  = pThis->m_hNormalFontSmaller;
            hDoubleFontSmaller  = pThis->m_hDoubleFontSmaller;
        }

        WORD UnicodeChar = pThis->m_CharacterSet[uChar - 0x20];

        // Select correct font
        if (uMode & VTMODE_DOUBLE)
        {
            if (*hSelectedFont != hDoubleFont)
            {
                SelectObject(hDC, *hSelectedFont = hDoubleFont);
            }
        }
        else
        {
            if (*hSelectedFont != hNormalFont)
            {
                SelectObject(hDC, *hSelectedFont = hNormalFont);
            }
        }

        int LeftOffset = 0;

        if (!pThis->m_bFixedPitch)
        {
            SIZE Size;
            GetTextExtentPoint32W(hDC, (LPCWSTR)&UnicodeChar, 1, &Size);

            // Use smaller font if it doesn't fit
            if (Size.cx > DrawRect.right - DrawRect.left)
            {
                if (uMode & VTMODE_DOUBLE)
                {
                    SelectObject(hDC, *hSelectedFont = hDoubleFontSmaller);
                }
                else
                {
                    SelectObject(hDC, *hSelectedFont = hNormalFontSmaller);
                }

                GetTextExtentPoint32W(hDC, (LPCWSTR)&UnicodeChar, 1, &Size);
            }

            LeftOffset = (pThis->m_dAvgWidth - Size.cx) / 2;

            if (nCol == 0)
            {
                // This is 1 and not 0 to compensate for
                // antialiasing
                if (LeftOffset < 1)
                {
                    LeftOffset = 1;
                }
            }
            else if (nCol == 39)
            {
                if (LeftOffset + Size.cx > DrawRect.right - DrawRect.left - 1)
                {
                    LeftOffset = DrawRect.right - DrawRect.left - Size.cx - 1;
                }
            }
        }

        // Draw a shadow if there is no background
        if (Background == VTCOLOR_NONE)
        {
            SetTextColor(hDC, 0x000000);
            TextOutW(hDC, DrawRect.left + LeftOffset + 1,
                DrawRect.top, (LPCWSTR)&UnicodeChar, 1);
        }

        SetTextColor(hDC, pThis->GetColourRef(Foreground));
        TextOutW(hDC, DrawRect.left + LeftOffset,
            DrawRect.top - 1, (LPCWSTR)&UnicodeChar, 1);
    }

    return PARSE_CONTINUE;
}


/*
 *  ROUNDING theory:
 *
 *  4 pixels:
 *  .__ __ __ __.
 *  | 0| 1| 2| 3|
 *  *--*--*--*--*
 *
 *  Divided into 3 columns:
 *  .___ ___ ___.
 *  | c0| c1| c2|
 *  *---*---*---*
 *
 *  Each would have a width of (4 / 3) = 1.33 pixels.
 *
 *  Column 1: 0.00 - 1.33
 *  Column 2: 1.33 - 2.66
 *  Column 3: 2.66 - 3.99
 *
 *  ROUND the ranges:
 *
 *  Column 1: 0 - 1
 *  Column 2: 1 - 3
 *  Column 3: 3 - 4
 *
 *  The 4 pixels will divided as:
 *  .__ __ __ __.
 *  |c0|c1|c1|c2|
 *  *--*--*--*--*
 * ------------------------------
 *
 *  Obtaining the column from a pixel:
 *  .__ __ __ __.
 *  |  | P|  |  |    P = 1
 *  *--*--*--*--*
 *
 *  DON'T ROUND: Add 1 to P then divide by 1.33.
 *
 *  (P + 1) / 1.33 = 1.50 => 1
 */


WORD CVTDrawer::GetRowColAtPoint(LPRECT lpDisplayRect, LPPOINT DisplayPoint)
{
    double dWidth = (lpDisplayRect->right - lpDisplayRect->left) / 40.0;
    double dHeight = (lpDisplayRect->bottom - lpDisplayRect->top) / 25.0;

    BYTE nRow = (DisplayPoint->y - lpDisplayRect->top + 1) / dHeight;
    BYTE nCol = (DisplayPoint->x - lpDisplayRect->left + 1) / dWidth;

    if (nRow >= 25 || nCol >= 40)
    {
        return 0xFEFE;
    }

    return MAKEWORD(nRow, nCol);
}


WORD CVTDrawer::FindPageNumberAtPoint(TVTPage* pPage, LPRECT lpDisplayRect,
                                      LPPOINT DisplayPoint)
{
    WORD wRowCol = GetRowColAtPoint(lpDisplayRect, DisplayPoint);
    return FindPageNumberAtRowCol(pPage, wRowCol);
}


WORD CVTDrawer::FindPageNumberAtRowCol(TVTPage* pPage, WORD wSearchRowCol)
{
    WORD wFlags = PARSE_HASDATAONLY | PARSE_REPEATDOUBLE;
    WORD wPageHex = 0xFFFF;

    BYTE nRow = LOBYTE(wSearchRowCol);
    BYTE nCol = HIBYTE(wSearchRowCol);

    if (nRow >= 25 || nCol >= 40)
    {
        return 0;
    }

    LPVOID lpParam[2] = { &nCol, &wPageHex };

    if (nRow != 24)
    {
        ParseLineElements(pPage, nRow, &wFlags,
            (TParserCallback*)FindPageNumberProc, lpParam);
    }
    else
    {
        ParseLineElements(pPage, nRow, &wFlags,
            (TParserCallback*)FindRow24PageNumberProc, lpParam);
    }

    if ((wPageHex & 0xF00) == 0xF00 || (wPageHex & 0xF000) != 0xF000)
    {
        return 0;
    }

    return (WORD)(wPageHex & 0xFFF);
}


BYTE CVTDrawer::FindPageNumberProc(TVTPage*, WORD wPoint, LPWORD, WORD,
                                   BYTE uChar, BYTE uMode, LPVOID lpParam)
{
    BYTE  nSearchCol    = *(BYTE*)((LPVOID*)lpParam)[0];
    WORD* pPageHex      =  (WORD*)((LPVOID*)lpParam)[1];

    BYTE nCol = HIBYTE(wPoint);

    if ((uMode & VTMODE_GRAPHICS) ||
        (uChar & 0x70) != 0x30 || (uChar & 0x0F) > 9)
    {
        if (nCol > nSearchCol)
        {
            return PARSE_STOPPAGE;
        }
        *pPageHex = 0xFFFF;

        if (nCol == nSearchCol)
        {
            return PARSE_STOPPAGE;
        }
    }
    else
    {
        *pPageHex = (WORD)((*pPageHex << 4) | (uChar - 0x30));

        // Even after matching 3 digits in a row at the
        // point specified, we wait for one more to be
        // certain there isn't a fourth digit.  Hence
        // we add + 3 instead of + 2.
        if ((nCol >= nSearchCol && (*pPageHex & 0xF000) != 0xF000) ||
            nCol == nSearchCol + 3)
        {
            return PARSE_STOPPAGE;
        }
    }

    return PARSE_CONTINUE;
}


BYTE CVTDrawer::FindRow24PageNumberProc(TVTPage*, WORD wPoint, LPWORD, WORD wColour,
                                        BYTE uChar, BYTE uMode, LPVOID lpParam)
{
    BYTE  nSearchCol    = *(BYTE*)((LPVOID*)lpParam)[0];
    WORD* pPageHex      =  (WORD*)((LPVOID*)lpParam)[1];

    BYTE nCol = HIBYTE(wPoint);

    if (nCol == nSearchCol)
    {
        BYTE Foreground = LOBYTE(wColour);
        BYTE Background = HIBYTE(wColour);

        switch (Background)
        {
        case VTCOLOR_RED:
            *pPageHex = VTPAGE_FLOFRED;
            break;

        case VTCOLOR_GREEN:
            *pPageHex = VTPAGE_FLOFGREEN;
            break;

        case VTCOLOR_YELLOW:
            *pPageHex = VTPAGE_FLOFYELLOW;
            break;

        case VTCOLOR_CYAN:
            *pPageHex = VTPAGE_FLOFBLUE;
            break;

        default:
            *pPageHex = 0xFFFF;
            break;
        }

        if (*pPageHex == 0xFFFF)
        {
            switch (Foreground)
            {
                case VTCOLOR_RED:
                    *pPageHex = VTPAGE_FLOFRED;
                    break;

                case VTCOLOR_GREEN:
                    *pPageHex = VTPAGE_FLOFGREEN;
                    break;

                case VTCOLOR_YELLOW:
                    *pPageHex = VTPAGE_FLOFYELLOW;
                    break;

                case VTCOLOR_CYAN:
                    *pPageHex = VTPAGE_FLOFBLUE;
                    break;
            }
        }

        // Nothing for character four
        *pPageHex |= 0xF000;

        return PARSE_STOPPAGE;
    }

    return PARSE_CONTINUE;
}


COLORREF CVTDrawer::GetColourRef(BYTE VTColour)
{
    if (VTColour == VTCOLOR_NONE)
    {
        return m_TransparencyColour;
    }

    return m_ColourTable[VTColour];
}


HBRUSH CVTDrawer::GetColourBrush(BYTE VTColour)
{
    if (VTColour == VTCOLOR_NONE)
    {
        if (m_TransparencyBrush == NULL)
        {
            m_TransparencyBrush = CreateSolidBrush(m_TransparencyColour);
        }
        return m_TransparencyBrush;
    }

    return m_hColourBrush[VTColour];
}


void CVTDrawer::SetBounds(HDC hDC, LPRECT lpRect)
{
    WORD wOldWidth = (WORD)(m_BoundsRect.right - m_BoundsRect.left);
    WORD wOldHeight = (WORD)(m_BoundsRect.bottom - m_BoundsRect.top);

    CopyRect(&m_BoundsRect, lpRect);

    if ((lpRect->right - lpRect->left) == wOldWidth &&
        (lpRect->bottom - lpRect->top) == wOldHeight)
    {
        return;
    }

    m_dAvgWidth = (lpRect->right - lpRect->left) / 40.0;
    m_dAvgHeight = (lpRect->bottom - lpRect->top) / 25.0;

    DeleteFonts();
}


void CVTDrawer::CreateMissingStandardFonts(HDC hDC)
{
    if (m_hNormalFont == NULL)
    {
        // Wide fonts for most charaters
        m_hNormalFont = MakeFont(hDC, m_dAvgHeight, m_dAvgWidth, m_szFontName, m_bAntialias, TRUE);
    }

    if (m_hDoubleFont == NULL)
    {
        // Wide double height fonts for most charaters
        m_hDoubleFont = MakeFont(hDC, m_dAvgHeight * 2, m_dAvgWidth, m_szFontName, m_bAntialias, TRUE);
    }

    if (m_hNormalFontSmaller == NULL)
    {
        // Smaller font for wide characters like 'W' (double iso int for lessening rounding artifacts)
        m_hNormalFontSmaller = MakeFont(hDC, m_dAvgHeight, m_dAvgWidth, m_szFontName, m_bAntialias, FALSE);
    }

    if (m_hDoubleFontSmaller == NULL)
    {
        // Smaller double height font for wide characters like 'W'
        m_hDoubleFontSmaller = MakeFont(hDC, m_dAvgHeight * 2, m_dAvgWidth, m_szFontName, m_bAntialias, FALSE);
    }

    TEXTMETRIC TextMetric;

    HGDIOBJ hSaveFont = SelectObject(hDC, m_hNormalFont);
    GetTextMetrics(hDC, &TextMetric);
    SelectObject(hDC, hSaveFont);

    m_bFixedPitch = !(TextMetric.tmPitchAndFamily & TMPF_FIXED_PITCH);
}


void CVTDrawer::CreateMissingLowQFonts(HDC hDC)
{
    if (m_hLowQNormalFont == NULL)
    {
        m_hLowQNormalFont = MakeFont(hDC, m_dAvgHeight, m_dAvgWidth, m_szFontName, FALSE, TRUE);
    }

    if (m_hLowQDoubleFont == NULL)
    {
        m_hLowQDoubleFont = MakeFont(hDC, m_dAvgHeight * 2, m_dAvgWidth, m_szFontName, FALSE, TRUE);
    }

    if (m_hLowQNormalFontSmaller == NULL)
    {
        m_hLowQNormalFontSmaller = MakeFont(hDC, m_dAvgHeight, m_dAvgWidth, m_szFontName, FALSE, FALSE);
    }

    if (m_hLowQDoubleFontSmaller == NULL)
    {
        m_hLowQDoubleFontSmaller = MakeFont(hDC, m_dAvgHeight * 2, m_dAvgWidth, m_szFontName, FALSE, FALSE);
    }

    TEXTMETRIC TextMetric;

    HGDIOBJ hSaveFont = SelectObject(hDC, m_hLowQNormalFont);
    GetTextMetrics(hDC, &TextMetric);
    SelectObject(hDC, hSaveFont);

    m_bFixedPitch = !(TextMetric.tmPitchAndFamily & TMPF_FIXED_PITCH);
}


void CVTDrawer::DeleteFonts()
{
    if (m_hNormalFont != NULL)
    {
        DeleteObject(m_hNormalFont);
        m_hNormalFont = NULL;
    }

    if (m_hDoubleFont != NULL)
    {
        DeleteObject(m_hDoubleFont);
        m_hDoubleFont = NULL;
    }

    if (m_hNormalFontSmaller != NULL)
    {
        DeleteObject(m_hNormalFontSmaller);
        m_hNormalFontSmaller = NULL;
    }

    if (m_hDoubleFontSmaller != NULL)
    {
        DeleteObject(m_hDoubleFontSmaller);
        m_hDoubleFontSmaller = NULL;
    }

    if (m_hLowQNormalFont != NULL)
    {
        DeleteObject(m_hLowQNormalFont);
        m_hLowQNormalFont = NULL;
    }

    if (m_hLowQDoubleFont != NULL)
    {
        DeleteObject(m_hLowQDoubleFont);
        m_hLowQDoubleFont = NULL;
    }

    if (m_hLowQNormalFontSmaller != NULL)
    {
        DeleteObject(m_hLowQNormalFontSmaller);
        m_hLowQNormalFontSmaller = NULL;
    }

    if (m_hLowQDoubleFontSmaller != NULL)
    {
        DeleteObject(m_hLowQDoubleFontSmaller);
        m_hLowQDoubleFontSmaller = NULL;
    }
}


HFONT CVTDrawer::MakeFont(HDC hDC, double iSize, double iWidth, LPCTSTR szFaceName, BOOL bAntiAliased, BOOL bWidenFont)
{
    // WidenFont = make X as wide as the average width of W and X
    BOOL bWiden = bWidenFont && (iWidth > 10);

    DWORD dwQuality = bAntiAliased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;

    // Small fonts blur when too heavy
    LONG bWeight = (iWidth > 10) ? FW_SEMIBOLD : FW_NORMAL; //bWiden ? FW_BOLD : FW_SEMIBOLD;

    HFONT hFont = CreateFont(-96, 96, 0, 0, bWeight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, dwQuality, DEFAULT_PITCH, szFaceName);
    HGDIOBJ hSave = SelectObject(hDC, hFont);

    SIZE Size;
    GetTextExtentPoint32(hDC, "W", 1, &Size);
    if (bWiden)
    {
        SIZE SizeSmall;
        GetTextExtentPoint32(hDC, "X", 1, &SizeSmall);
        Size.cx = (Size.cx + 2 * SizeSmall.cx) / 3;
    }
    iWidth = iWidth * 96 / Size.cx;

    GetTextExtentPoint32(hDC, "Wy", 1, &Size);
    iSize = iSize * 96 / Size.cy;

    SelectObject(hDC, hSave);
    DeleteObject(hFont);

    hFont = CreateFont(-iSize, iWidth, 0, 0, bWeight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, dwQuality, DEFAULT_PITCH, szFaceName);

    return hFont;
}

