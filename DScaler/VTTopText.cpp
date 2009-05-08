/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file VTTopText.cpp CVTTopText Implementation
 */

#include "stdafx.h"
#include "VTCommon.h"
#include "VTTopText.h"


char* CVTTopText::m_WaitMessage     = "Please wait";
char* CVTTopText::m_NoneMessage     = "Page not included";
char* CVTTopText::m_MultiMessage    = "Multi-page with %d sub-pages";
char* CVTTopText::m_SubtitleMessage = "Subtitle page";
char* CVTTopText::m_EmptyTitleText  = "    ????    ";


CVTTopText::CVTTopText()
{
    InitializeCriticalSection(&m_IntegrityMutex);

    for (int i(0); i < 800; i++)
    {
        m_AITable[i] = NULL;
    }

    Reset();
}


CVTTopText::~CVTTopText()
{
    for (int i(0); i < 800; i++)
    {
        if (m_AITable[i] != NULL)
        {
            delete [] m_AITable[i];
        }
    }

    DeleteCriticalSection(&m_IntegrityMutex);
}


void CVTTopText::Reset()
{
    EnterCriticalSection(&m_IntegrityMutex);

    m_ExtraPagesHead = 1;
    m_ExtraTopPages[0].uType = 0xFF;

    ResetWaitingPages();

    memset(m_BTTable, TOP_UNRECEIVED, sizeof(BYTE) * 800);
    memset(m_MPTable, 0x00, sizeof(BYTE) * 800);

    for (int i = 0; i < 800; i++)
    {
        if (m_AITable[i] != NULL)
        {
            delete [] m_AITable[i];
        }
        m_AITable[i] = NULL;
        m_AITReceived[i] = FALSE;
    }

    m_LastPageCode = 0UL;
    memset(m_LastBuffer, 0x00, sizeof(BYTE) * 40);

    LeaveCriticalSection(&m_IntegrityMutex);
}


void CVTTopText::ResetWaitingPages()
{
    for (int i = 0; i < TOPWAIT_LASTONE; i++)
    {
        m_WaitingPage[i] = -1;
    }
}


BOOL CVTTopText::IsWaitingBTTPage(short Page)
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


BOOL CVTTopText::IsWaitingAITPage(short Page)
{
    for (int i = TOPWAIT_YELLOW_AIT; i <= TOPWAIT_BLUE_AIT; i++)
    {
        if (m_WaitingPage[i] == Page)
        {
            return TRUE;
        }
    }
    return FALSE;
}


BOOL CVTTopText::IsWaitingMPTPage(short Page)
{
    if (m_WaitingPage[TOPWAIT_MPT] == Page)
    {
        return TRUE;
    }
    return FALSE;
}


DWORD CVTTopText::DecodePageRow(DWORD dwPageCode, BYTE nRow, BYTE* pData)
{
    // 0x1F0 is always the Basic TOP page
    if (LOWORD(dwPageCode) == 0x1F0)
    {
        if (DecodeBTTPageRow(nRow, pData))
        {
            return m_LastPageCode;
        }
        return 0UL;
    }

    // Determine if this page is a TOP page
    BYTE Type = GetExtraPageType(dwPageCode);

    if (Type == 0xFF)
    {
        return 0UL;
    }

    switch (Type)
    {
    case TOPTYPE_MPT:
        if (DecodeMPTPageRow(nRow, pData))
        {
            return m_LastPageCode;
        }
        break;

    case TOPTYPE_AIT:
        if (DecodeAITPageRow(nRow, pData))
        {
            return m_LastPageCode;
        }
        break;
    }

    return 0UL;
}


BOOL CVTTopText::DecodeBTTPageRow(BYTE nRow, BYTE* pData)
{
    BOOL    bError;
    BOOL    bLastPageUpdated = FALSE;

    EnterCriticalSection(&m_IntegrityMutex);

    if (nRow >= 1 && nRow <= 20)
    {
        for (int i = 0, Page = ((nRow - 1) * 40); i < 40; i++, Page++)
        {
            BYTE uLevel = Unham84(pData[i], &(bError = FALSE));

            if (bError != FALSE)
            {
                continue;
            }

            m_BTTable[Page] = uLevel;

            // See if a missing page was received
            if (IsWaitingBTTPage(Page))
            {
                ResetWaitingPages();
                bLastPageUpdated = TRUE;
            }
        }
    }
    else if (nRow >= 21 && nRow <= 22)
    {
        BYTE    Type;
        BYTE    Magazine;
        WORD    wPageHex;
        WORD    wPageSubCode;
        BYTE    s1, s2, s3, s4;

        for (int n = 0; n < 5; n++)
        {
            Magazine = Unham84(pData[n*8], &(bError = FALSE));

            if (bError != FALSE || Magazine == 0x0F)
            {
                break;
            }
            if (Magazine >= 0x08)
            {
                continue;
            }

            Type = Unham84(pData[n*8 + 7], &bError);

            if (bError != FALSE ||
                (Type != TOPTYPE_MPT && Type != TOPTYPE_AIT))
            {
                continue;
            }

            // These units are MSB first
            wPageHex = UnhamTwo84_MSBF(&pData[n*8 + 1], &bError);

            s1 = Unham84(pData[n*8 + 3], &bError);
            s2 = Unham84(pData[n*8 + 4], &bError);
            s3 = Unham84(pData[n*8 + 5], &bError);
            s4 = Unham84(pData[n*8 + 6], &bError);

            if (bError != FALSE)
            {
                continue;
            }

            wPageHex |= (Magazine == 0 ? 0x800 : Magazine * 0x100);
            wPageSubCode = (s4 | ((s3 & 0x7) << 4));
            wPageSubCode |= ((s2 << 7) | ((s1 & 0x3) << 11));

            DWORD dwPageCode = MAKELONG(wPageHex, wPageSubCode);

            if (!IsTopTextPage(dwPageCode))
            {
                // Multi-thread protection
                m_ExtraTopPages[m_ExtraPagesHead].uType      = 0xFF;

                m_ExtraTopPages[m_ExtraPagesHead].dwPageCode = dwPageCode;
                m_ExtraTopPages[m_ExtraPagesHead].uType      = Type;

                m_ExtraPagesHead = (m_ExtraPagesHead + 1) % TOPMAX_EXTRAPAGES;
            }
        }
    }

    LeaveCriticalSection(&m_IntegrityMutex);

    return bLastPageUpdated;
}


BOOL CVTTopText::DecodeMPTPageRow(BYTE nRow, BYTE* pData)
{
    BOOL    bError;
    BOOL    bLastPageUpdated = FALSE;

    EnterCriticalSection(&m_IntegrityMutex);

    if (nRow >= 1 && nRow <= 20)
    {
        for (int i = 0, Page = (nRow - 1) * 40; i < 40; i++, Page++)
        {
            BYTE uPages = Unham84(pData[i], &(bError = FALSE));

            if (bError != FALSE)
            {
                continue;
            }

            m_MPTable[Page] = uPages;

            if (IsWaitingMPTPage(Page))
            {
                ResetWaitingPages();
                bLastPageUpdated = TRUE;
            }
        }
    }

    LeaveCriticalSection(&m_IntegrityMutex);

    return bLastPageUpdated;
}


BOOL CVTTopText::DecodeAITPageRow(BYTE nRow, BYTE* pData)
{
    BOOL    bError;
    BOOL    bLastPageUpdated = FALSE;
    BOOL    bTitleChanged;
    BYTE    uChar;

    EnterCriticalSection(&m_IntegrityMutex);

    if (nRow >= 1 && nRow <= 22)
    {
        BYTE    Magazine;
        WORD    wPageHex;
        short   Page;

        for (int n = 0; n < 2; n++)
        {
            Magazine = Unham84(pData[n*20], &(bError = FALSE));

            if (bError != FALSE || Magazine == 0x0F)
            {
                break;
            }
            if (Magazine >= 0x08)
            {
                continue;
            }

            wPageHex = UnhamTwo84_MSBF(&pData[n*20 + 1], &bError);
            wPageHex |= (Magazine == 0 ? 0x800 : Magazine * 0x100);

            if (bError != FALSE || (Page = PageHex2Page(wPageHex)) == -1)
            {
                continue;
            }

            if (m_AITable[Page] == NULL)
            {
                m_AITable[Page] = new BYTE[12];
            }

            bTitleChanged = FALSE;

            for (int i = 0; i < 12; i++)
            {
                uChar = pData[n*20 + 8 + i];

                if (m_AITReceived[Page] == FALSE)
                {
                    m_AITable[Page][i] = (uChar & 0x7F) < 0x20 ? 0x20 : (uChar & 0x7F);
                    bTitleChanged = TRUE;
                }
                else
                {
                    if (CheckParity(&uChar, 1, TRUE))
                    {
                        if (uChar != m_AITable[Page][i])
                        {
                            m_AITable[Page][i] = uChar;
                            bTitleChanged = TRUE;
                        }
                    }
                }
            }

            m_AITReceived[Page] = TRUE;

            if (bTitleChanged != FALSE)
            {
                if (IsWaitingAITPage(Page))
                {
                    ResetWaitingPages();
                    bLastPageUpdated = TRUE;
                }
            }
        }
    }

    LeaveCriticalSection(&m_IntegrityMutex);

    return bLastPageUpdated;
}


BYTE CVTTopText::GetExtraPageType(DWORD dwPageCode)
{
    int i = m_ExtraPagesHead;

    while (1)
    {
        i = (TOPMAX_EXTRAPAGES + i - 1) % TOPMAX_EXTRAPAGES;

        if (m_ExtraTopPages[i].uType == 0xFF)
        {
            break;
        }
        if (m_ExtraTopPages[i].dwPageCode == dwPageCode)
        {
            break;
        }
        if (i == m_ExtraPagesHead)
        {
            return 0xFF;
        }
    }

    return m_ExtraTopPages[i].uType;
}


BOOL CVTTopText::IsTopTextPage(DWORD dwPageCode)
{
    if (LOWORD(dwPageCode) == 0x1F0)
    {
        return TRUE;
    }

    return GetExtraPageType(dwPageCode) != 0xFF;
}


BOOL CVTTopText::IsMultiPage(short Page)
{
    switch (m_BTTable[Page])
    {
    case 0x03:
    case 0x05:
    case 0x07:
    case 0x0A:
    case 0x0B:
        return TRUE;
    }
    return FALSE;
}


short CVTTopText::GetFirstInBlock(short Page, short* MissingPage)
{
    for ( ; Page > 0; Page--)
    {
        if (m_BTTable[Page] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = Page;
            }
            return -1;
        }

        if (m_BTTable[Page] >= TOPLEVEL_PROGRAM &&
            m_BTTable[Page] < TOPLEVEL_GROUP)
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
        if (m_BTTable[Page] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = Page;
            }
            return -1;
        }

        if (m_BTTable[Page] >= TOPLEVEL_GROUP &&
            m_BTTable[Page] < TOPLEVEL_NORMAL)
        {
            return Page;
        }
    }

    return 0;
}


short CVTTopText::GetNextBlock(short Page, short* MissingPage)
{
    int i = Page;

    while ((i = (i + 1) % 800) != Page)
    {
        if (m_BTTable[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }

        if (m_BTTable[i] >= TOPLEVEL_PROGRAM &&
            m_BTTable[i] < TOPLEVEL_GROUP)
        {
            break;
        }
    }

    return i;
}


short CVTTopText::GetNextGroup(short Page, short* MissingPage)
{
    int i = Page;

    while ((i = (i + 1) % 800) != Page)
    {
        if (m_BTTable[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }

        if (m_BTTable[i] >= TOPLEVEL_PROGRAM &&
            m_BTTable[i] < TOPLEVEL_NORMAL)
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
        if (m_BTTable[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }

        if (m_BTTable[i] >= TOPLEVEL_SUBTITLE &&
            m_BTTable[i] < TOPLEVEL_GROUP)
        {
            break;
        }

        if (m_BTTable[i] >= TOPLEVEL_GROUP &&
            m_BTTable[i] < TOPLEVEL_NORMAL)
        {
            return i;
        }
    }

    return GetFirstInBlock(Page, MissingPage);
}


short CVTTopText::GetNextPage(short Page, short* MissingPage)
{
    int i = Page;

    while ((i = (i + 1) % 800) != Page)
    {
        if (m_BTTable[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }

        if (m_BTTable[i] >= TOPLEVEL_SUBTITLE &&
            m_BTTable[i] <= TOPLEVEL_LASTLEVEL)
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
        if (m_BTTable[i] & TOP_UNRECEIVED)
        {
            if (MissingPage != NULL)
            {
                *MissingPage = i;
            }
            return -1;
        }

        if (m_BTTable[i] >= TOPLEVEL_SUBTITLE &&
            m_BTTable[i] < TOPLEVEL_NORMAL)
        {
            break;
        }

        if (m_BTTable[i] >= TOPLEVEL_NORMAL &&
            m_BTTable[i] <= TOPLEVEL_LASTLEVEL)
        {
            return i;
        }
    }

    return GetFirstInGroup(Page, MissingPage);
}


short CVTTopText::PageHex2Page(WORD wPageHex)
{
    if ((wPageHex & 0xF00) < 0x100 ||
        (wPageHex & 0xF00) > 0x800 ||
        (wPageHex & 0x0F0) > 0x090 ||
        (wPageHex & 0x00F) > 0x009)
    {
        return -1;
    }

    short Page;

    Page = (((wPageHex & 0xF00) >> 8) * 100) - 100;
    Page += ((wPageHex & 0x0F0) >> 4) * 10;
    Page += ((wPageHex & 0x00F));

    return Page;
}


WORD CVTTopText::Page2PageHex(short Page)
{
    if (Page < 0 || Page > 799)
    {
        return 0;
    }

    WORD wPageHex = 0;

    wPageHex |= Page / 100 * 0x100 + 0x100;
    wPageHex |= Page % 100 / 10 * 0x10;
    wPageHex |= Page % 10;

    return wPageHex;
}


BOOL CVTTopText::GetTopTextDetails(DWORD dwPageCode, TVTPage* pBuffer, BOOL bWaitMessage)
{
    short   Page;
    short   LinkPage;
    short   MissingPage;
    BYTE*   pLineInput;
    char*   pMessage;

    if ((Page = PageHex2Page(LOWORD(dwPageCode))) == -1)
    {
        return FALSE;
    }

    ResetWaitingPages();

    m_LastPageCode = dwPageCode;

    // We can't do much more if we don't have a Basic TOP
    if (m_BTTable[Page] == TOP_UNRECEIVED)
    {
        m_WaitingPage[TOPWAIT_GREEN] = Page;

        // Multi-thread protection
        if (m_BTTable[Page] == TOP_UNRECEIVED)
        {
            return FALSE;
        }
        else
        {
            m_WaitingPage[TOPWAIT_GREEN] = -1;
        }
    }

    // Set the RED Flof key
    pBuffer->EditorialLink[VTFLOF_RED] = VTPAGE_PREVIOUS;

    // Get the GREEN Flof link
    LinkPage = GetNextPage(Page, &MissingPage);

    while (LinkPage == -1)
    {
        m_WaitingPage[TOPWAIT_GREEN] = MissingPage;

        // Multi-thread protection
        if (m_BTTable[MissingPage] == TOP_UNRECEIVED)
        {
            break;
        }

        m_WaitingPage[TOPWAIT_GREEN] = -1;
        LinkPage = GetNextPage(Page, &MissingPage);
    }

    // Set the GREEN Flof key
    pBuffer->EditorialLink[VTFLOF_GREEN] = Page2PageHex(LinkPage);

    pBuffer->bShowRow24 = TRUE;
    pBuffer->LineState[24] = CACHESTATE_HASDATA;

    pLineInput = pBuffer->Frame[24];

    // See if a wait message is required instead
    if (bWaitMessage != FALSE)
    {
        char   szBuffer[40];

        // Invalidate the YELLOW and BLUE Flof keys
        pBuffer->EditorialLink[VTFLOF_YELLOW] = 0UL;
        pBuffer->EditorialLink[VTFLOF_BLUE] = 0UL;

        if (m_BTTable[Page] == 0x00)
        {
            pMessage = m_NoneMessage;
        }
        else if (m_BTTable[Page] == TOPLEVEL_SUBTITLE)
        {
            pMessage = m_SubtitleMessage;
        }
        else if (IsMultiPage(Page))
        {
            if (m_MPTable[Page] == 0)
            {
                m_WaitingPage[TOPWAIT_MPT] = Page;

                // Multi-thread protection
                if (m_MPTable[Page] != 0)
                {
                    m_WaitingPage[TOPWAIT_MPT] = -1;
                }
            }

            if (m_MPTable[Page] != 0)
            {
                sprintf(szBuffer, m_MultiMessage, m_MPTable[Page]);
                pMessage = szBuffer;
            }
            else
            {
                pMessage = m_WaitMessage;
            }
        }
        else
        {
            pMessage = m_WaitMessage;
        }

        int nLength = strlen(pMessage);
        int nStart = (40 - nLength) / 2;

        memset(pLineInput, 0x20, nStart);
        memcpy(&pLineInput[nStart], pMessage, nLength);
        memset(&pLineInput[nStart + nLength], 0x20, 40 - nStart - nLength);
    }
    else
    {
        // Red section
        *pLineInput++ = 0x11;       // Red Mosaic
        *pLineInput++ = 0x3C;       // Box Mosaic
        *pLineInput++ = 0x01;       // Red text
        *pLineInput++ = 0x2D;       // '-'
        *pLineInput++ = 0x20;       // Space

        // Green section
        if (LinkPage != -1)
        {
            *pLineInput++ = 0x12;       // Green Mosaic
            *pLineInput++ = 0x3C;       // Box Mosaic
            *pLineInput++ = 0x02;       // Green text
            *pLineInput++ = 0x2B;       // '+'
            *pLineInput++ = 0x20;       // Space
        }
        else
        {
            memset(pLineInput, 0x20, 5);
            pLineInput += 5;
        }

        // Yellow Flof link
        LinkPage = GetNextGroup(Page, &MissingPage);

        while (LinkPage == -1)
        {
            m_WaitingPage[TOPWAIT_YELLOW] = MissingPage;

            // Multi-thread protection
            if (m_BTTable[MissingPage] == TOP_UNRECEIVED)
            {
                break;
            }

            m_WaitingPage[TOPWAIT_YELLOW] = -1;
            LinkPage = GetNextGroup(Page, &MissingPage);
        }

        pBuffer->EditorialLink[VTFLOF_YELLOW] = Page2PageHex(LinkPage);

        if (LinkPage != -1)
        {
            m_WaitingPage[TOPWAIT_YELLOW_AIT] = LinkPage;

            if (m_AITReceived[LinkPage] != FALSE)
            {
                pMessage = (char*)m_AITable[LinkPage];
            }
            else
            {
                pMessage = m_EmptyTitleText;
            }

            *pLineInput++ = 0x13;       // Yellow Mosaic
            *pLineInput++ = 0x3C;       // Box Mosaic
            *pLineInput++ = 0x03;       // Yellow text

            memcpy(pLineInput, pMessage, 12);
            pLineInput += 12;
        }
        else
        {
            memset(pLineInput, 0x20, 15);
            pLineInput += 15;
        }

        // Blue section
        LinkPage = GetNextBlock(Page, &MissingPage);

        while (LinkPage == -1)
        {
            m_WaitingPage[TOPWAIT_BLUE] = MissingPage;

            // Multi-thread protection
            if (m_BTTable[MissingPage] == TOP_UNRECEIVED)
            {
                break;
            }

            m_WaitingPage[TOPWAIT_BLUE] = -1;
            LinkPage = GetNextBlock(Page, &MissingPage);
        }

        pBuffer->EditorialLink[VTFLOF_BLUE] = Page2PageHex(LinkPage);

        if (LinkPage != -1)
        {
            m_WaitingPage[TOPWAIT_BLUE_AIT] = LinkPage;

            if (m_AITReceived[LinkPage] != FALSE)
            {
                pMessage = (char*)m_AITable[LinkPage];
            }
            else
            {
                pMessage = m_EmptyTitleText;
            }

            *pLineInput++ = 0x16;       // Cyan Mosaic
            *pLineInput++ = 0x3C;       // Box Mosaic
            *pLineInput++ = 0x06;       // Cyan text

            memcpy(pLineInput, pMessage, 12);
            pLineInput += 12;
        }
        else
        {
            memset(pLineInput, 0x20, 15);
            pLineInput += 15;
        }
    }

    pBuffer->EditorialLink[VTFLOF_INDEX] = 0x0100;

    for (int i = VTFLOF_RED; i <= VTFLOF_INDEX; i++)
    {
        // All TOP-Text pages have unspecified subcodes
        pBuffer->EditorialLink[i] |= (0x3F7F << 16);
    }

    if (memcmp(pBuffer->Frame[24], m_LastBuffer, 40) != 0)
    {
        memcpy(m_LastBuffer, pBuffer->Frame[24], 40);
        pBuffer->LineState[24] |= CACHESTATE_UPDATED;
    }

    return TRUE;
}


/*  This creates the traditional red, green, yellow, blue
 *  box background TOP-Text commentary but it doesn't look
 *  very nice.  This code is outdated.

    // Red
    *pLineInput++ = 0x01;       // Red
    *pLineInput++ = 0x1d;       // Red Background
    *pLineInput++ = 0x07;       // White text
    *pLineInput++ = 0x60;       // '-'
    *pLineInput++ = 0x20;       // Space
    *pLineInput++ = 0x02;       // Green

    // Green
    *pLineInput++ = 0x1d;       // Green Background
    *pLineInput++ = 0x00;       // Black text
    *pLineInput++ = 0x2B;        // '+'
    *pLineInput++ = 0x20;       // Space
    *pLineInput++ = 0x03;       // Yellow

    // Yellow
    *pLineInput++ = 0x1d;       // Yellow Background
    *pLineInput++ = 0x00;       // Black text
    for (i = 0; i < 12; i++)
    {
        *pLineInput++ = (m_AITable[Page] == NULL) ? ' ' : m_AITable[Page][i];
    }
    *pLineInput++ = 0x04;       // Blue

    // Cyan
    *pLineInput++ = 0x1d;       // Blue Background
    *pLineInput++ = 0x07;       // White text
    for (i = 0; i < 12; i++)
    {
        *pLineInput++ = (m_AITable[Page] == NULL) ? ' ' : m_AITable[Page][i];
    }
*/

