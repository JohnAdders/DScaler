/////////////////////////////////////////////////////////////////////////////
// $Id: VTTopText.h,v 1.1 2002-10-23 16:51:01 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __VTTOPTEXT_H__
#define __VTTOPTEXT_H__

#include "stdafx.h"
#include "VBI_VideoText.h"


class CVTTopText
{
private:

    // Variable constants
    enum
    {
        TOPMAX_LASTPAGES    = 64,
        TOPMAX_EXTRAPAGES   = 10,
    };

    // Bits used in m_TOP[800]
    enum
    {
        TOP_VALUEMASK       = 0x0F,
        TOP_UNRECEIVED      = 1 << 4,
    };

    enum eTopLevel
    {
        TOPLEVEL_SUBTITLE   = 1,
        TOPLEVEL_PROGRAM    = 2,
        TOPLEVEL_BLOCK      = 4,
        TOPLEVEL_GROUP      = 6,
        TOPLEVEL_NORMAL     = 8,
        TOPLEVEL_LASTLEVEL  = 11,
    };

    enum
    {
        TOPTYPE_MULTIPAGE   = 1,
        TOPTYPE_ADIP        = 2,
    };

    enum
    {
        TOPWAIT_GREEN       = 0,
        TOPWAIT_YELLOW,
        TOPWAIT_BLUE,
        TOPWAIT_YELLOWADIP,
        TOPWAIT_BLUEADIP,
        TOPWAIT_MULTIPAGE,
        TOPWAIT_LASTONE
    };
    
    typedef struct
    {
        BYTE    Type;
        WORD    HexPage;
        WORD    SubPage;
    } TExtraTopPage;

public:
    CVTTopText();
    ~CVTTopText();

    void Reset();

    BOOL DecodePageRow(TVTPage* pPage, unsigned char* pData, int nRow);
    BOOL IsTopTextPage(int HexPage, int SubPage);
    void GetTopTextDetails(TVTPage* pPage);
    void WindBackLast();

private:
    BOOL DecodeTOPPageRow(unsigned char* pData, int nRow);
    BOOL DecodeMultiPageRow(unsigned char* pData, int nRow);
    BOOL DecodeADIPPageRow(unsigned char* pData, int nRow);

    BYTE GetExtraPageType(int HexPage, int SubPage);

    BOOL IsWaitingTOPPage(short Page);
    BOOL IsWaitingADIPPage(short Page);
    BOOL IsWaitingMultiPage(short Page);
    void ClearWaitingPages();

    BOOL IsMultiPage(int Page);

    short GetFirstInBlock(short Page, short* MissingPage);
    short GetFirstInGroup(short Page, short* MissingPage);
    short GetNextBlock(short Page, short* MissingPage);
    short GetNextGroup(short Page, short* MissingPage);
    short GetNextGroupInBlock(short Page, short* MissingPage);
    short GetNextPage(short Page, short* MissingPage);
    short GetNextPageInGroup(short Page, short* MissingPage);
    short GetLastPage(short Page);

private:
    BYTE                    m_TOP[800];
    BYTE                    m_MultiPage[800];
    BYTE*                   m_ADIP[800];
    TExtraTopPage           m_ExtraTopPages[TOPMAX_EXTRAPAGES];
    BYTE                    m_ExtraPagesHead;
    short                   m_LastPages[TOPMAX_LASTPAGES];
    BYTE                    m_LastPagesHead;

    short                   m_WaitingPage[TOPWAIT_LASTONE];

    static char*            m_WaitMessage;
    static char*            m_NoneMessage;
    static char*            m_MultiMessage;
    static char*            m_EmptyADIPText;
};

#endif
