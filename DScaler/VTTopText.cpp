/////////////////////////////////////////////////////////////////////////////
// $Id: VTTopText.cpp,v 1.5 2003-01-01 20:30:12 atnak Exp $
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
//  GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2002/10/24 09:34:58  atnak
// Changed '-' dash to minus to fix codepage difference
//
// Revision 1.3  2002/10/24 01:38:32  atnak
// Added subtitle message
//
// Revision 1.2  2002/10/23 16:53:43  atnak
// small adjustment
//
// Revision 1.1  2002/10/23 16:51:01  atnak
// TOP-Text support initial release
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "VTTopText.h"
/*
extern unsigned char UnhamTab[256];

char* CVTTopText::m_WaitMessage     = "Please wait";
char* CVTTopText::m_NoneMessage     = "Page not included";
char* CVTTopText::m_MultiMessage    = "Multi-page with %d sub-pages";
char* CVTTopText::m_SubtitleMessage = "Subtitle page";
char* CVTTopText::m_EmptyADIPText   = "    ????    ";


CVTTopText::CVTTopText()
{
    for (int i(0); i < 800; i++)
    {
        m_ADIP[i] = NULL;
    }

    Reset();
}

CVTTopText::~CVTTopText()
{
    for (int i(0); i < 800; i++)
    {
        if (m_ADIP[i] != NULL)
        {
            delete [] m_ADIP[i];
        }
    }
}

void CVTTopText::Reset()
{
    int i;

    m_ExtraPagesHead = 1;
    m_ExtraTopPages[0].Type = 0xFF;

    m_LastPagesHead = 0;
    m_LastPages[0] = -1;

    ClearWaitingPages();

    memset(m_TOP, TOP_UNRECEIVED, sizeof(BYTE) * 800);
    memset(m_MultiPage, 0, sizeof(BYTE) * 800);

    for (i = 0; i < 800; i++)
    {
        if (m_ADIP[i] != NULL)
        {
            delete [] m_ADIP[i];
        }
        m_ADIP[i] = NULL;
    }
}

void CVTTopText::ClearWaitingPages()
{
    for (int i = 0; i < TOPWAIT_LASTONE; i++)
    {
        m_WaitingPage[i] = -1;
    }
}

BOOL CVTTopText::IsWaitingTOPPage(short Page)
{
    for (int i = TOPWAIT_GREEN; i <= TOPWAIT_BLUE; i++)
    {
        if (m_WaitingPage[i] == Page)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CVTTopText::IsWaitingADIPPage(short Page)
{
    for (int i = TOPWAIT_YELLOWADIP; i <= TOPWAIT_BLUEADIP; i++)
    {
        if (m_WaitingPage[i] == Page)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CVTTopText::IsWaitingMultiPage(short Page)
{
    if (m_WaitingPage[TOPWAIT_MULTIPAGE] == Page)
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CVTTopText::DecodePageRow(TVTPage* pPage, unsigned char* pData, int nRow)
{
    BYTE Type;

    if (pPage->Page == 0x1F0)
    {
        return DecodeTOPPageRow(pData, nRow);
    }

    Type = GetExtraPageType(pPage->Page, pPage->SubPage);
    if (Type == 0xFF)
    {
        return FALSE;
    }

    switch (Type)
    {
    case TOPTYPE_MULTIPAGE:
        return DecodeMultiPageRow(pData, nRow);

    case TOPTYPE_ADIP:
        return DecodeADIPPageRow(pData, nRow);
    }
    return FALSE;
}

BOOL CVTTopText::DecodeTOPPageRow(unsigned char* pData, int nRow)
{
    int     i;
    int     Page;
    BOOL    bCallRedraw = FALSE;

    if (nRow >= 1 && nRow <= 20)
    {
        for (i = 0, Page = ((nRow - 1) * 40); i < 40; i++, Page++)
        {
            m_TOP[Page] = UnhamTab[pData[i]];

            // See if a missing page was received
            if (IsWaitingTOPPage(Page))
            {
                ClearWaitingPages();
                bCallRedraw = TRUE;
            }
        }
    }
    else if (nRow >= 21 && nRow <= 22)
    {
        int     Mag;
        int     Type;
        int     Page;
        int     SubPage;

        for (i = 0; i < 5; i++)
        {
            Mag = UnhamTab[pData[i * 8]];

            if (Mag == 0x0F)
            {
                break;
            }
            if (Mag == 0x0E)
            {
                continue;
            }

            Type = UnhamTab[pData[i * 8 + 7]];

            if (Type != TOPTYPE_MULTIPAGE && Type != TOPTYPE_ADIP)
            {
                continue;
            }

            Page = (UnhamTab[pData[i * 8 + 1]] & 0x0F) << 4 |
                   (UnhamTab[pData[i * 8 + 2]] & 0x0F);

            SubPage = (UnhamTab[pData[i * 8 + 3]] & 0x03) << 11 |
                      (UnhamTab[pData[i * 8 + 4]] & 0x0F) << 07 |
                      (UnhamTab[pData[i * 8 + 5]] & 0x07) << 04 |
                      (UnhamTab[pData[i * 8 + 6]] & 0x0F);
            SubPage = (SubPage >> 4) * 10 + (SubPage & 0x0F);
            
            Page = VT_MakeHexPage(Mag & 0x07, Page);

            if (!IsTopTextPage(Page, SubPage))
            {
                m_ExtraTopPages[m_ExtraPagesHead].HexPage  = Page;
                m_ExtraTopPages[m_ExtraPagesHead].SubPage  = SubPage;
                m_ExtraTopPages[m_ExtraPagesHead].Type     = Type;

                m_ExtraPagesHead = (m_ExtraPagesHead + 1) % TOPMAX_EXTRAPAGES;
            }
        }
    }
    return bCallRedraw;
}

BOOL CVTTopText::DecodeMultiPageRow(unsigned char* pData, int nRow)
{
    BOOL    bCallRedraw = FALSE;

    if (nRow >= 1 && nRow <= 20)
    {
        for (int i = 0, Page = (nRow - 1) * 40; i < 40; i++, Page++)
        {
            m_MultiPage[Page] = UnhamTab[pData[i]];
            
            if (IsWaitingMultiPage(Page))
            {
                ClearWaitingPages();
                bCallRedraw = TRUE;
            }
        }
    }
    return bCallRedraw;
}

BOOL CVTTopText::DecodeADIPPageRow(unsigned char* pData, int nRow)
{
    int     i, j;
    BOOL    bCallRedraw = FALSE;

    if (nRow >= 1 && nRow <= 22)
    {
        int    Mag;
        int    Page;

        for (i = 0; i < 2; i++)
        {
            Mag = UnhamTab[pData[i * 20]];
            
            if (Mag == 0x0F)
            {
                break;
            }
            if (Mag == 0x0E)
            {
                continue;
            }

            Page = (UnhamTab[pData[i * 20 + 1]] & 0x0F) << 4 |
                   (UnhamTab[pData[i * 20 + 2]] & 0x0F);

            Page = VT_MakePage(Mag & 0x07, Page) - 100;

            if (Page >= 0 && Page < 800)
            {
                if (m_ADIP[Page] != NULL)
                {
                    delete [] m_ADIP[Page];
                }
                else
                {
                    // See if a missing text was received
                    if (IsWaitingADIPPage(Page))
                    {
                        ClearWaitingPages();
                        bCallRedraw = TRUE;
                    }
                }

                m_ADIP[Page] = new BYTE[12];

                for (j = 0; j < 12; j++)
                {
                    m_ADIP[Page][j] = ((pData[i * 20 + 8 + j] & 0x7F) < 0x20) ?
                                        0x20 : pData[i * 20 + 8 + j];
                }
            }
        }
    }
    return bCallRedraw;
}

BYTE CVTTopText::GetExtraPageType(int HexPage, int SubPage)
{
    int i = m_ExtraPagesHead;

    while (m_ExtraTopPages[i = (TOPMAX_EXTRAPAGES + i - 1) %
        TOPMAX_EXTRAPAGES].Type != 0xFF)
    {
        if (m_ExtraTopPages[i].HexPage == HexPage &&
            m_ExtraTopPages[i].SubPage == SubPage)
        {
            break;
        }
        if (i == m_ExtraPagesHead)
        {
            return 0xFF;
        }
    }
    return m_ExtraTopPages[i].Type;
}

BOOL CVTTopText::IsTopTextPage(int HexPage, int SubPage)
{
    if (HexPage == 0x1F0)
    {
        return TRUE;
    }

    return GetExtraPageType(HexPage, SubPage) != 0xFF;
}

BOOL CVTTopText::IsMultiPage(int Page)
{
    switch (m_TOP[Page])
    {
    case 3:
    case 5:
    case 7:
    case 10:
    case 11:
        return TRUE;
    }
    return FALSE;
}

short CVTTopText::GetFirstInBlock(short Page, short* MissingPage)
{
    for ( ; Page > 0; Page--)
    {
        if (m_TOP[Page] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = Page;
            }
            return -1;
        }
        if (m_TOP[Page] >= TOPLEVEL_PROGRAM &&
            m_TOP[Page] < TOPLEVEL_GROUP)
        {
            return Page;
        }
    }
    return 0;
}

short CVTTopText::GetFirstInGroup(short Page, short* MissingPage)
{
    for ( ; Page > 0; Page--)
    {
        if (m_TOP[Page] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = Page;
            }
            return -1;
        }
        if (m_TOP[Page] >= TOPLEVEL_GROUP &&
            m_TOP[Page] < TOPLEVEL_NORMAL)
        {
            return Page;
        }
    }
    return 0;
}

short CVTTopText::GetNextBlock(short Page, short* MissingPage)
{
    int i;

    for (i = Page + 1; i != Page; i = (i + 1) % 800)
    {
        if (m_TOP[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }
        if (m_TOP[i] >= TOPLEVEL_PROGRAM &&
            m_TOP[i] < TOPLEVEL_GROUP)
        {
            break;
        }
    }
    return i;
}

short CVTTopText::GetNextGroup(short Page, short* MissingPage)
{
    int i;

    for (i = Page + 1; i != Page; i = (i + 1) % 800)
    {
        if (m_TOP[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }
        if (m_TOP[i] >= TOPLEVEL_PROGRAM &&
            m_TOP[i] < TOPLEVEL_NORMAL)
        {
            break;
        }
    }
    return i;
}

short CVTTopText::GetNextGroupInBlock(short Page, short* MissingPage)
{
    for (int i = Page + 1; i < 800; i++)
    {
        if (m_TOP[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }
        if (m_TOP[i] >= TOPLEVEL_SUBTITLE &&
            m_TOP[i] < TOPLEVEL_GROUP)
        {
            break;
        }
        if (m_TOP[i] >= TOPLEVEL_GROUP &&
            m_TOP[i] < TOPLEVEL_NORMAL)
        {
            return i;
        }
    }
    return GetFirstInBlock(Page, MissingPage);
}

short CVTTopText::GetNextPage(short Page, short* MissingPage)
{
    for (int i = Page + 1; i != Page; i = (i + 1) % 800)
    {
        if (m_TOP[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }
        if (m_TOP[i] >= TOPLEVEL_SUBTITLE &&
            m_TOP[i] <= TOPLEVEL_LASTLEVEL)
        {
            break;
        }
    }
    return i;
}

short CVTTopText::GetNextPageInGroup(short Page, short* MissingPage)
{
    for (int i = Page + 1; i < 800; i++)
    {
        if (m_TOP[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }
        if (m_TOP[i] >= TOPLEVEL_SUBTITLE &&
            m_TOP[i] < TOPLEVEL_NORMAL)
        {
            break;
        }
        if (m_TOP[i] >= TOPLEVEL_NORMAL &&
            m_TOP[i] <= TOPLEVEL_LASTLEVEL)
        {
            return i;
        }
    }
    return GetFirstInGroup(Page, MissingPage);
}

short CVTTopText::GetLastPage(short Page)
{
    int i = (TOPMAX_LASTPAGES + m_LastPagesHead - 1) % TOPMAX_LASTPAGES;

    if (m_LastPages[i] != -1)
    {
        Page = m_LastPages[i];
    }
    return Page;
}

void CVTTopText::WindBackLast()
{
    if (m_LastPages[m_LastPagesHead] != -1)
    {
        m_LastPages[m_LastPagesHead] = -1;
        m_LastPagesHead = (TOPMAX_LASTPAGES + m_LastPagesHead - 1) % TOPMAX_LASTPAGES;
    }
}

void CVTTopText::GetTopTextDetails(TVTPage* pPage)
{
    int     i;
    int     NextPage;
    BYTE*   pBuffer;
    LPSTR   Message;

    if (pPage == NULL || pPage->Page < 0 || pPage->Page > 799)
    {
        return;
    }

    // Add this page to the last pages list
    if (m_LastPages[m_LastPagesHead] != pPage->Page)
    {
        m_LastPagesHead = (m_LastPagesHead + 1) % TOPMAX_LASTPAGES;
        m_LastPages[m_LastPagesHead] = pPage->Page;
    }

    ClearWaitingPages();

    // We can't do much more if we don't have a TOP
    if (m_TOP[pPage->Page] == TOP_UNRECEIVED)
    {
        m_WaitingPage[TOPWAIT_GREEN] = pPage->Page;
        return;
    }

    pBuffer = pPage->Frame[24];
    pPage->CommentaryRow = 24;
    pPage->bFlofUpdated = 1;

    // Set the RED Flof key
    NextPage = GetLastPage(pPage->Page);
    pPage->FlofPage[FLOF_RED] = NextPage + 100;

    // Set the GREEN Flof key
    NextPage = GetNextPage(pPage->Page, &m_WaitingPage[TOPWAIT_GREEN]);
    if (NextPage != -1)
    {
        pPage->FlofPage[FLOF_GREEN] = NextPage + 100;
    }

    // If the page hasn't yet been received
    if (!pPage->bUpdated)
    {
        char buf[40];

        if (m_TOP[pPage->Page] == 0x00)
        {
            Message = m_NoneMessage;
        }
        else if (m_TOP[pPage->Page] == TOPLEVEL_SUBTITLE)
        {
            Message = m_SubtitleMessage;
        }
        else if (IsMultiPage(pPage->Page))
        {
            if (m_MultiPage[pPage->Page] != 0)
            {
                sprintf(buf, m_MultiMessage, m_MultiPage[pPage->Page]);
                Message = buf;
            }
            else
            {
                m_WaitingPage[TOPWAIT_MULTIPAGE] = pPage->Page + 100;
                Message = m_WaitMessage;
            }
        }
        else
        {
            Message = m_WaitMessage;
        }

        int Length = strlen(Message);
        int Start = (40 - Length) / 2;

        memset(pBuffer, 0x20, Start);
        memcpy(&pBuffer[Start], Message, Length);
        memset(&pBuffer[Start + Length], 0x20, 40 - Start - Length);
        return;
    }

    // Red section
    *pBuffer++ = 0x11;       // Red Mosaic
    *pBuffer++ = 0x3C;       // Box Mosaic
    *pBuffer++ = 0x01;       // Red text
    *pBuffer++ = 0x2D;       // '-'
    *pBuffer++ = 0x20;       // Space

    // Green section
    if (NextPage != -1)
    {
        *pBuffer++ = 0x12;       // Green Mosaic
        *pBuffer++ = 0x3C;       // Box Mosaic
        *pBuffer++ = 0x02;       // Green text
        *pBuffer++ = 0x2B;       // '+'
        *pBuffer++ = 0x20;       // Space
    }
    else
    {
        memset(pBuffer, 0x20, 5);
        pBuffer += 5;
    }

    // Yellow section
    NextPage = GetNextGroup(pPage->Page, &m_WaitingPage[TOPWAIT_YELLOW]);
    if (NextPage != -1)
    {
        if ((Message = (char*)m_ADIP[NextPage]) == NULL)
        {
            m_WaitingPage[TOPWAIT_YELLOWADIP] = NextPage;
            Message = m_EmptyADIPText;
        }

        // Set the YELLOW Flof key
        pPage->FlofPage[FLOF_YELLOW] = NextPage + 100;

        *pBuffer++ = 0x13;       // Yellow Mosaic
        *pBuffer++ = 0x3C;       // Box Mosaic
        *pBuffer++ = 0x03;       // Yellow text
        for (i = 0; i < 12; i++)
        {
            *pBuffer++ = Message[i];
        }
    }
    else
    {
        memset(pBuffer, 0x20, 15);
        pBuffer += 15;
    }

    // Blue section
    NextPage = GetNextBlock(pPage->Page, &m_WaitingPage[TOPWAIT_BLUE]);
    if (NextPage != -1)
    {
        if ((Message = (char*)m_ADIP[NextPage]) == NULL)
        {
            m_WaitingPage[TOPWAIT_BLUEADIP] = NextPage;
            Message = m_EmptyADIPText;
        }

        // Set the BLUE Flof key
        pPage->FlofPage[FLOF_BLUE] = NextPage + 100;

        *pBuffer++ = 0x16;       // Cyan Mosaic
        *pBuffer++ = 0x3C;       // Box Mosaic
        *pBuffer++ = 0x06;       // Cyan text

        for (i = 0; i < 12; i++)
        {
            *pBuffer++ = Message[i];
        }
    }
    else
    {
        memset(pBuffer, 0x20, 15);
        pBuffer += 15;
    }
}



/*  This creates the traditional red, green, yellow, blue
 *  box background TOP-Text commentary but it doesn't look
 *  very nice.

    // Red
    *pBuffer++ = 0x01;       // Red
    *pBuffer++ = 0x1d;       // Red Background
    *pBuffer++ = 0x07;       // White text
    *pBuffer++ = 0x60;       // '-'
    *pBuffer++ = 0x20;       // Space
    *pBuffer++ = 0x02;       // Green

    // Green
    *pBuffer++ = 0x1d;       // Green Background
    *pBuffer++ = 0x00;       // Black text
    *pBuffer++ = 0x2B;        // '+'
    *pBuffer++ = 0x20;       // Space
    *pBuffer++ = 0x03;       // Yellow

    // Yellow
    *pBuffer++ = 0x1d;       // Yellow Background
    *pBuffer++ = 0x00;       // Black text
    for (i = 0; i < 12; i++)
    {
        *pBuffer++ = (m_ADIP[Page] == NULL) ? ' ' : m_ADIP[Page][i];
    }
    *pBuffer++ = 0x04;       // Blue

    // Cyan
    *pBuffer++ = 0x1d;       // Blue Background
    *pBuffer++ = 0x07;       // White text
    for (i = 0; i < 12; i++)
    {
        *pBuffer++ = (m_ADIP[Page] == NULL) ? ' ' : m_ADIP[Page][i];
    }
*/

