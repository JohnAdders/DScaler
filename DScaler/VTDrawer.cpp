/////////////////////////////////////////////////////////////////////////////
// $Id: VTDrawer.cpp,v 1.4 2002-02-24 16:41:40 temperton Exp $
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

#include "stdafx.h"
#include "VTDrawer.h"
#include "VTMosaicGraphics.h"
#include "VTConvTables.h"
#include "VBI_VideoText.h"

#define CLOCK_COL           32
#define CLOCK_COL_WIDTH     8

//---------------------------------------------------------------------------
// VideoText
unsigned long VTColourTable[9] =
{
    0x000000,       //Black
    0x0000FF,       //Red
    0x00FF00,       //Green
    0x00FFFF,       //Yellow
    0xFF0000,       //Blue
    0xFF00FC,       //Magenta
    0xFFFF00,       //Cyan
    0xFFFFFF,       //White
    0xFF00FF        //Transparent
};

char VTDrawerFontName[32] = "Arial";


CVTDrawer::CVTDrawer()
    :m_hFont(0),
    m_hDoubleFont(0),
    m_AvgWidth(0),
    m_AvgHeight(0),
    m_dAvgWidth(0),
    m_dAvgHeight(0)
{
    for(int a=0; a<9; a++)
    {
        m_hBrushes[a] = CreateSolidBrush(VTColourTable[a]);
    }
}

CVTDrawer::~CVTDrawer()
{
    for(int a=0; a<9; a++)
    {
        DeleteObject(m_hBrushes[a]);
    }
    DestroyFonts();
}

bool CVTDrawer::Draw(TVTPage* pPage, TVTHeaderLine* pHeader, HDC hDC, 
    LPPOINT pOrigin, unsigned long ulFlags, eVTCodePage VTCodePage, int iRow)
{
    BOOL bGraph, bHoldGraph, bSepGraph, bBox, bFlash, bDouble, bConceal;
    BOOL bHasDouble = false;
    int CurrentFg, CurrentBkg;
    int DefaultBkg = 0;
    int n;
    BYTE c, ch, nLastGraph;
    char tmp[41];

    int startrow = 0;
    int endrow = ((ulFlags & VTDF_HEADERONLY) || (ulFlags & VTDF_CLOCKONLY)) ? 1 : 25;

    int left = pOrigin ? pOrigin->x : m_Rect.left;
    int top = pOrigin ? pOrigin->y : m_Rect.top;

    if(ulFlags & VTDF_THISROWONLY)
    {
        if((iRow > 1) && pPage->bUpdated)
        {
            for(int n = 0; n < 40; n++)
            {
                if((pPage->Frame[iRow - 1][n] & 0x7F) == 0x0D)
                {
                    return true;
                }
            }
        }
        startrow = iRow;
        endrow = iRow + 1;
    }

    int SaveBkMode = SetBkMode(hDC, TRANSPARENT);

    HFONT hFontSave = (HFONT) SelectObject(hDC, m_hFont);
    HGDIOBJ hBrushSave = (HFONT) SelectObject(hDC, m_hBrushes[0]);

    for(int row = startrow; row < endrow; row++)
    {
        if (bHasDouble)
        {
            bHasDouble = FALSE;
            continue;
        }

        bGraph = bHoldGraph = bSepGraph = bBox = bFlash = bDouble = bConceal = bHasDouble = FALSE;
        SelectObject(hDC, m_hFont);
        CurrentFg = 7;
        CurrentBkg = DefaultBkg;

        if(row==0)
        {
            sprintf(tmp, "  P%-3d \x7", pPage->Page + 100);

            if ((pPage->bUpdated == FALSE) && (pHeader))
            {
                // if the current page hasn't yet been filled
                // show the last header
                for (n = 8; n < 40; n++)
                {
                    tmp[n] = (*pHeader)[n] & 0x7f;
                }              
                //DefaultBkg = 0;
                DefaultBkg = (ulFlags & VTDF_MIXMODE) ? 8 : 0;
            }
            else
            {
                // if the current page has been filled
                // show the original header
                for (n = 8; n < (pHeader ? CLOCK_COL : 40); n++)
                {
                    tmp[n] = pPage->Frame[row][n] & 0x7f;
                }
                
                // but the time from the most recent
                if(pHeader)
                {
                    for (n = CLOCK_COL; n < CLOCK_COL + CLOCK_COL_WIDTH; n++)
                    {
                        tmp[n] = (*pHeader)[n] & 0x7f;
                    }
                    memset(&tmp[CLOCK_COL + CLOCK_COL_WIDTH], 32, 40 - CLOCK_COL - CLOCK_COL_WIDTH);
                }

                DefaultBkg = (!(pPage->wCtrl & (3 << 4)) && !(ulFlags & VTDF_MIXMODE)) ? 0 : 8;

                if (pPage->wCtrl & (3 << 4))
                {
                    memset(tmp, 32, 40);
                }
            }
        }
        else
        {
            for (n = 0; n < 40; n++)
            {
                if((pPage->bUpdated) || (ulFlags & VTDF_FORCEDRAW))
                {
                    tmp[n] = pPage->Frame[row][n] & 0x7f;
                }
                else
                {
                    tmp[n] = 0x20;
                }

                if(tmp[n] == 0x0D)
                {
                    bHasDouble = TRUE;
                }

                DefaultBkg = ((pPage->wCtrl & (3 << 4)) || (ulFlags & VTDF_MIXMODE)) ? 8 : 0;
            }
        }

        CurrentBkg = DefaultBkg;

        for(int col = 0; col < 40; col++)
        {
            c = tmp[col];
            ch = c;
            if (c < 0x20)
            {
                switch(c)
                {
                case 0x00:
                case 0x01:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                case 0x06:
                case 0x07:
                    CurrentFg = c;
                    bGraph = FALSE;
                    bConceal = false;
                    break;
                case 0x08:
                    bFlash = TRUE;
                    break;
                case 0x09:
                    bFlash = FALSE;
                    break;
                case 0x0a:
                    CurrentBkg = DefaultBkg;
                    bBox = FALSE;
                    break;
                case 0x0b:
                    CurrentBkg = 0;
                    bBox = TRUE;
                    break;
                case 0x0c:
                    bDouble = FALSE;
                    SelectObject(hDC, m_hFont);
                    break;
                case 0x0d:
                    bDouble = TRUE;
                    SelectObject(hDC, m_hDoubleFont);
                    break;
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                case 0x16:
                case 0x17:
                    bGraph = TRUE;
                    CurrentFg = c - 0x10;
                    break;
                case 0x18:
                    bConceal = TRUE;
                    break;
                case 0x19:
                    bSepGraph = FALSE;
                    break;
                case 0x1a:
                    bSepGraph = TRUE;
                    break;
                case 0x1c:
                    CurrentBkg = bBox ? 0 : DefaultBkg;
                    break;
                case 0x1d:
                    if(!(ulFlags & VTDF_MIXMODE))
                    {
                        CurrentBkg = CurrentFg;
                    }
                    break;
                case 0x1e:
                    bHoldGraph = TRUE;
                    break;
                case 0x1f:
                    bHoldGraph = FALSE;
                    break;
                default:
                    break;
                }
                ch = bHoldGraph ? nLastGraph : 32;
            }

            if(((col<CLOCK_COL) && (ulFlags & VTDF_CLOCKONLY)) ||
               ((!bFlash) && (ulFlags & VTDF_FLASHONLY)) ||
               ((!bConceal) && (ulFlags & VTDF_HIDDENONLY)) ||
               ((!bFlash) && (ulFlags & VTDF_CLEARFLASH)))
            {
                continue;
            }

            RECT thischar;
            thischar.left = left + double(col) * m_dAvgWidth;
            thischar.right = left + double(double(col + 1) * m_dAvgWidth);
            thischar.top = top + double(row) * m_dAvgHeight;
            thischar.bottom = top + double(row + (bHasDouble ? 2 : 1)) * m_dAvgHeight;
            
            FillRect(hDC, &thischar, m_hBrushes[CurrentBkg]);

            if(!bDouble && bHasDouble)
            {
                thischar.bottom = top + double(row + (bDouble ? 2 : 1)) * m_dAvgHeight;
            }

            if((bFlash && !(ulFlags & VTDF_FLASHMASK)) ||
               (bConceal && !(ulFlags & VTDF_HIDDENMASK))) 
            {
                continue;
            }

            if((bGraph) && (ch & 0x20))
            {
                nLastGraph = ch;
                ch = (ch & 0x1f) | ((ch & 0x40) >> 1);

                //Draw mosaic graphics
                DrawGraphChar(hDC, m_hBrushes[CurrentFg], ch, bSepGraph, thischar.left, thischar.top, thischar.right - thischar.left, thischar.bottom - thischar.top);

            }
            else
            {
                //First, here should be VTCharToAnsi conversion routine call
                WORD UChar = VTCharToUnicode(VTCodePage, ch-32);

                //Draw char
                if(ch!=32)
                {    
                    int offset;
                    if(m_bFixedPitch)
                    {
                        offset = 0;
                    }
                    else
                    {
                        SIZE Size;
                        GetTextExtentPoint32W(hDC, (wchar_t*)&UChar, 1, &Size);
                        offset = (m_AvgWidth - Size.cx) / 2;
                    }
            
                    SetTextColor(hDC, VTColourTable[CurrentFg]);
                    TextOutW(hDC, thischar.left + offset, thischar.top, (wchar_t*) &UChar, 1);
                }
            }
        }
    }

    SelectObject(hDC, hBrushSave);
    SelectObject(hDC, hFontSave);
    SetBkMode(hDC, SaveBkMode);

    return true;
}

void CVTDrawer::SetBounds(HDC hDC, RECT* Rect)
{
    m_dAvgWidth = double(Rect->right - Rect->left) / 40.0;
    m_dAvgHeight = double(Rect->bottom - Rect->top) / 25.0;
    int AvgWidth = (Rect->right - Rect->left) / 40;
    int AvgHeight = (Rect->bottom - Rect->top) / 25;
    int nHeight = AvgHeight;
    m_Rect = *Rect;

    if((m_AvgWidth == AvgWidth) && (m_AvgHeight == AvgHeight))
    {
        return;
    }

    DestroyFonts();

    m_AvgWidth = AvgWidth;
    m_AvgHeight = AvgHeight;

    m_hFont = MakeFont(hDC, nHeight, m_AvgWidth, (char*)&VTDrawerFontName);
    m_hDoubleFont = MakeFont(hDC, nHeight * 2, m_AvgWidth, (char*)&VTDrawerFontName);

    HGDIOBJ hSave = SelectObject(hDC, m_hFont);
    TEXTMETRIC TextMetric;
    GetTextMetrics(hDC, &TextMetric);
    SelectObject(hDC, hSave);

    m_bFixedPitch = !(TextMetric.tmPitchAndFamily & TMPF_FIXED_PITCH);
}

int CVTDrawer::GetAvgWidth()
{
    return m_AvgWidth;
}

int CVTDrawer::GetAvgHeight()
{
    return m_AvgHeight;
}

HFONT CVTDrawer::MakeFont(HDC hDC, int iSize, int iWidth, char* szFaceName)
{
    HFONT hFont = CreateFont(iSize, iWidth, 0, 0, 700, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, szFaceName);
    HGDIOBJ hSave = SelectObject(hDC, hFont);
    
    SIZE Size;
    GetTextExtentPoint(hDC, "W", 1, &Size);
    iWidth = iWidth * iWidth / Size.cx;

    GetTextExtentPoint(hDC, "Wy", 1, &Size);
    iSize = iSize * iSize / Size.cy;
        
    SelectObject(hDC, hSave);
    DeleteObject(hFont);

    hFont = CreateFont(iSize, iWidth, 0, 0, 700, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, szFaceName);
    return hFont;
}

void CVTDrawer::DestroyFonts()
{
    if(m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
    if(m_hDoubleFont)
    {
        DeleteObject(m_hDoubleFont);
        m_hDoubleFont = NULL;
    }
}
