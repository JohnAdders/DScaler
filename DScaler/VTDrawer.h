/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file vtdrawer.h vtdrawer Header file
 */

#ifndef __VTDRAWER_H__
#define __VTDRAWER_H__

#include "stdafx.h"
#include "VTCommon.h"


// DrawPage flags
enum
{
    VTDF_HIDDEN             = 0x0001,
    VTDF_HIDDENONLY         = 0x0002,
    VTDF_FLASH              = 0x0004,
    VTDF_FLASHONLY          = 0x0008,
    VTDF_MIXEDMODE          = 0x0010,
    VTDF_UPDATEDONLY        = 0x0020,
    VTDF_FORCEHEADER        = 0x0040,
    VTDF_HEADERONLY         = 0x0080,
    VTDF_CLOCKONLY          = 0x0100,
    VTDF_ROW24ONLY          = 0x0200,
};


class CVTDrawer : CVTCommon
{
public:
    CVTDrawer();
    ~CVTDrawer();

    /// Sets the text codepage to use when drawing
    void SetCodepage(eVTCodepage Codepage);
    /// Turns on or off display font antialiasing
    void SetAntialias(BOOL bAntialias);
    /// Sets the colour to be used as the transparent colour
    void SetTransparencyColour(COLORREF ColorRef);

    /// Draws the page to the given rect and device context
    void DrawPage(HDC hDC, LPRECT lpRect, TVTPage* pPage, WORD uFlags,
                  TVTLeftRight* pHilightList[25], BYTE pDoubleProfile[25] = NULL,
                  RECT pRefreshProfile[25] = NULL, BYTE* pRefreshCount = NULL);

    /// Works out the row and column given a display point
    WORD GetRowColAtPoint(LPRECT lpDisplayRect, LPPOINT DisplayPoint);

    /// Gets the hex page number found at the pixel point
    WORD FindPageNumberAtPoint(TVTPage* pPage, LPRECT lpDisplayRect,
                               LPPOINT DisplayPoint);

    /// Gets the hex page number found at the point
    WORD FindPageNumberAtRowCol(TVTPage* pPage, WORD wSearchRowCol);

private:
    COLORREF GetColourRef(BYTE VTColour);
    HBRUSH GetColourBrush(BYTE VTColour);

    void SetBounds(HDC hDC, LPRECT lpRect);

    void CreateMissingStandardFonts(HDC hDC);
    void CreateMissingLowQFonts(HDC hDC);
    void DeleteFonts();

    HFONT MakeFont(HDC hDC, double iSize, double iWidth, LPCTSTR szFaceName,
                   BOOL bAntiAliased, BOOL bWidenFont = FALSE);

    static BYTE DrawPageProc(TVTPage*, WORD wPoint, LPWORD lpFlags,
                             WORD wColour, BYTE uChar, BYTE uMode, LPVOID lpParam);

    static BYTE FindPageNumberProc(TVTPage*, WORD wPoint, LPWORD, WORD,
                                   BYTE uChar, BYTE uMode, LPVOID lpParam);

    static BYTE FindRow24PageNumberProc(TVTPage*, WORD wPoint, LPWORD, WORD wColour,
                                        BYTE uChar, BYTE uMode, LPVOID lpParam);

private:
    COLORREF    m_TransparencyColour;
    HBRUSH      m_TransparencyBrush;

    HBRUSH      m_hColourBrush[8];

    BOOL        m_bAntialias;
    RECT        m_BoundsRect;

    eVTCodepage m_CurrentCodepage;
    WORD        m_CharacterSet[96];

    HFONT       m_hNormalFont;
    HFONT       m_hDoubleFont;
    HFONT       m_hNormalFontSmaller;
    HFONT       m_hDoubleFontSmaller;

    HFONT       m_hLowQNormalFont;
    HFONT       m_hLowQDoubleFont;
    HFONT       m_hLowQNormalFontSmaller;
    HFONT       m_hLowQDoubleFontSmaller;

    BOOL        m_bFixedPitch;

    double      m_dAvgWidth;
    double      m_dAvgHeight;

    static COLORREF     m_ColourTable[8];
    static const char   m_szFontName[32];
};

#endif


/*
 *
 *  Constructor:
 *
 *  CVTDrawer();
 *
 *
 *  Setup:
 *
 *  Set the code page.  (Default: Undefined.  Must be called before Main)
 *  - SetCodepage(eVTCodepage Codepage);
 *
 *  Set anti alias option (Default: Antialias off)
 *  - SetAntialias(BOOL bAntialias);
 *
 *  Set the colour to be used as the tranparency colour:  (Default: Blue)
 *  - SetTransparencyColour(COLORREF ColorRef);
 *
 *
 *  Main:
 *
 *  Draw a page.
 *  - DrawPage(HDC hDC, LPRECT lpRect, TVTPage* pPage, BYTE uFlags,
 *            LPPOINT lpStartRowCol = NULL, LPPOINT lpStopRowCol = NULL);
 *
 *    hDC             - Device context in which to draw
 *    lpRect          - The rect in which the page is drawn in
 *    pPage           - The page to be drawn.
 *    uFlags          - Various flags for DrawPage (see: enum { VTDF_* })
 *    pHilightList    - List of Left-Right spans that should be hilighted.
 *    pDoubleProfile  - Used to receive the list of double heights lines.
 *    pRefreshProfile - Receives all the RECTs that were painted to.
 *    pRefreshCount   - Receives the number of RECTs that were used.
 *
 *
 *  Utility:
 *
 *  Gets out the row and column given a display point
 *  - GetRowColAtPoint(LPRECT lpDisplayRect, LPPOINT DisplayPoint);
 *
 *    lpDisplayRect  - The rect in which the page was drawn.
 *    DisplayPoint   - The coordinates, with the same (0,0) reference
 *                     point as lpDisplayRect.
 *
 *    Return type: WORD
 *
 *  Get a string of three number at a screen coordinate.  Return the
 *  number as a hexidecimal page number (mask 0xFFF).
 *  - FindPageNumberAtPoint(TVTPage* pPage, LPRECT lpDisplayRect,
 *                           LPPOINT lpDisplayPoint);
 *
 *    pPage          - The page in which to search
 *    lpDisplayRect  - The rect in which the page was drawn.
 *    DisplayPoint   - The coordinates, with the same (0,0) reference
 *                     point as lpDisplayRect.
 *
 *    Return type: WORD
 *
 *  Get a string of three number at a specific row and column.  Return
 *  the number as a hexidecimal page number (mask 0xFFF).
 *  - FindPageNumberAtRowCol(TVTPage* pPage, WORD wSearchRowCol);
 *
 *    wSearchRowCol  - LOBYTE is Row, HIBYTE is Col.
 *
 *    Return type: WORD
 */

