/////////////////////////////////////////////////////////////////////////////
// $Id: VTDrawer.h,v 1.6 2002-10-12 00:38:07 atnak Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2002/06/20 20:00:31  robmuller
// Implemented videotext search highlighting.
//
// Revision 1.4  2002/05/23 22:16:32  robmuller
// Applied patch #559111 by PietOO.
// Teletext: less sparse look for ttf fonts.
//
// Revision 1.3  2002/02/24 16:41:40  temperton
// Bug fixes
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

#ifndef __VTDRAWER_H__
#define __VTDRAWER_H__

#include "stdafx.h"
#include "VBI_VideoText.h"

typedef char TVTHeaderLine[40], *LPTVTHeaderLine, *PTVTHeaderLine;

#define VTDF_HIDDEN          0x0001
#define VTDF_HIDDENONLY      0x0002
#define VTDF_FLASH           0x0004
#define VTDF_FLASHONLY       0x0008
#define VTDF_MIXMODE         0x0010
#define VTDF_CLOCKONLY       0x0020
#define VTDF_HEADERONLY      0x0040
#define VTDF_FORCEDRAW       0x0080
#define VTDF_CLEARFLASH      0x0100
#define VTDF_THISROWONLY     0x0200

#define VTDF_HIDDENMASK      0x0003
#define VTDF_FLASHMASK       0x000C

class
    CVTDrawer
{
public:
    CVTDrawer();
    ~CVTDrawer();
    void SetBounds(HDC hDC, RECT* Rect);
	void SetHiliteText(const char* Text);
    bool Draw(TVTPage* pPage, TVTHeaderLine* pHeader, HDC hDC, LPPOINT pOrigin, unsigned long ulFlags, eVTCodePage VTCodePage, int iRow);

    int GetAvgWidth();
    int GetAvgHeight();

private:
    BOOL DrawCharacterRect(HDC hDC, BYTE nRow, BYTE nCol, BYTE DisplayColour, BYTE BackgroundColour, BYTE DisplayModes, BYTE DisplayChar,
                            BOOL bHighLightChar, BOOL bHasDouble, int nTopOffset, int nLeftOffset, eVTCodePage VTCodePage, HFONT& hCurrentFont,
                            unsigned long ulFlags);
private:
    void DestroyFonts();  
    HFONT MakeFont(HDC hDC, double iSize, double iWidth, char* szFaceName, BOOL bWidenFont = FALSE);
	BOOL IsHiliteText(int col, const char* VTRow);
    HBRUSH m_hBrushes[9];
    RECT m_Rect;
    HFONT m_hFont;
    HFONT m_hDoubleFont;
	HFONT m_hFontSmall; // smaller to accomodate wide characters
    HFONT m_hDoubleFontSmall; // idem
    BOOL m_bFixedPitch;
    int m_AvgWidth, m_AvgHeight;
    double m_dAvgWidth, m_dAvgHeight;
    char m_HiliteText[41];
};

#endif
