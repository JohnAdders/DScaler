/////////////////////////////////////////////////////////////////////////////
// $Id: VTTopText.h,v 1.5 2003-10-27 10:39:54 adcockj Exp $
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
// Revision 1.4  2003/01/05 16:09:44  atnak
// Updated TopText for new teletext
//
// Revision 1.3  2003/01/01 20:30:12  atnak
// Commented out everything so I can redo TopText for the new CVTCommon
// functions later
//
// Revision 1.2  2002/10/24 01:38:32  atnak
// Added subtitle message
//
// Revision 1.1  2002/10/23 16:51:01  atnak
// TOP-Text support initial release
//
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file vttoptext.h vttoptext Header file
 */
 
#ifndef __VTTOPTEXT_H__
#define __VTTOPTEXT_H__

#include "stdafx.h"
#include "VTCommon.h"


class CVTTopText : CVTCommon
{
private:

    // Variable constants
    enum
    {
        TOPMAX_EXTRAPAGES   = 10,
    };

    // Bits used in m_BTTable[800]
    enum
    {
        TOP_VALUEMASK       = 0x0F,
        TOP_UNRECEIVED      = 1 << 4,
    };

    enum eTopLevel
    {
        TOPLEVEL_SUBTITLE   = 0x01,
        TOPLEVEL_PROGRAM    = 0x02,
        TOPLEVEL_BLOCK      = 0x04,
        TOPLEVEL_GROUP      = 0x06,
        TOPLEVEL_NORMAL     = 0x08,
        TOPLEVEL_LASTLEVEL  = 0x0B,
    };

    enum
    {
        TOPTYPE_MPT         = 0x01,
        TOPTYPE_AIT         = 0x02,
    };

    enum
    {
        TOPWAIT_GREEN       = 0,
        TOPWAIT_YELLOW,
        TOPWAIT_BLUE,
        TOPWAIT_YELLOW_AIT,
        TOPWAIT_BLUE_AIT,
        TOPWAIT_MPT,
        TOPWAIT_LASTONE
    };
    
    typedef struct
    {
        BYTE    uType;
        DWORD   dwPageCode;
    } TExtraTopPage;

public:
    CVTTopText();
    ~CVTTopText();

    void Reset();

    DWORD DecodePageRow(DWORD dwPageCode, BYTE nRow, BYTE* pData);

    BOOL IsTopTextPage(DWORD dwPageCode);
    BOOL GetTopTextDetails(DWORD dwPageCode, TVTPage* pBuffer, BOOL bWaitMessage = FALSE);

private:
    BOOL DecodeBTTPageRow(BYTE nRow, BYTE* pData);
    BOOL DecodeMPTPageRow(BYTE nRow, BYTE* pData);
    BOOL DecodeAITPageRow(BYTE nRow, BYTE* pData);

    BYTE GetExtraPageType(DWORD dwPageCode);

    BOOL IsWaitingBTTPage(short Page);
    BOOL IsWaitingMPTPage(short Page);
    BOOL IsWaitingAITPage(short Page);

    void ResetWaitingPages();

    BOOL IsMultiPage(short Page);

    short GetFirstInBlock(short Page, short* MissingPage);
    short GetFirstInGroup(short Page, short* MissingPage);
    short GetNextBlock(short Page, short* MissingPage);
    short GetNextGroup(short Page, short* MissingPage);
    short GetNextGroupInBlock(short Page, short* MissingPage);
    short GetNextPage(short Page, short* MissingPage);
    short GetNextPageInGroup(short Page, short* MissingPage);

    short PageHex2Page(WORD wPageHex);
    WORD  Page2PageHex(short Page);

private:
    BYTE                    m_BTTable[800];         // Basic TOP
    BYTE                    m_MPTable[800];         // Multi-Page
    BYTE*                   m_AITable[800];         // Additional Information

    BYTE                    m_AITReceived[800];

    TExtraTopPage           m_ExtraTopPages[TOPMAX_EXTRAPAGES];
    BYTE                    m_ExtraPagesHead;

    short                   m_WaitingPage[TOPWAIT_LASTONE];

    CRITICAL_SECTION        m_IntegrityMutex;

    DWORD                   m_LastPageCode;
    BYTE                    m_LastBuffer[40];

    static char*            m_WaitMessage;
    static char*            m_NoneMessage;
    static char*            m_MultiMessage;
    static char*            m_SubtitleMessage;
    static char*            m_EmptyTitleText;
};

#endif
