/////////////////////////////////////////////////////////////////////////////
// $Id: VTDecoder.h,v 1.2 2003-01-05 16:09:44 atnak Exp $
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
// Revision 1.1  2003/01/01 20:38:11  atnak
// New videotext decoder
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __VTDECODER_H___
#define __VTDECODER_H___

#include "VTCommon.h"
#include "VTTopText.h"


/// Decoder Event messages
enum
{
    DECODEREVENT_HEADERUPDATE   = 0,    // A new rolling header is ready
    DECODEREVENT_PAGEBEGIN      = 1,    // New page reception has began
    DECODEREVENT_PAGEUPDATE     = 2,    // Page update have been received
    DECODEREVENT_PAGEREFRESH    = 3,    // Page received with no update
    DECODEREVENT_PDCUPDATE      = 4,    // Program Delivery Control changed
    DECODEREVENT_COMMENTUPDATE  = 5,    // Row 24 commentary changed
};


/// Cache control settings
enum
{
    // Controls how internal page lines cache
    // is updated when a new line is received
    DECODERCACHE_NORMAL         = 0,    // Update if the cached line has errors
    DECODERCACHE_SECONDCHANCE   = 1,    // Update if the new line is error free
    DECODERCACHE_ALWAYSUPDATE   = 2,    // Always update the line
};


class CVTDecoder : CVTCommon
{
public:
    /// The callback proc for catching decoder events
    typedef void (TDecoderCallback)(BYTE uMsg, DWORD dwParam);

public:
    CVTDecoder(TDecoderCallback* fnDecoderEventProc);
    ~CVTDecoder();

    // The main input point for raw videotext lines
    void DecodeLine(BYTE* data);
    // Resets the internal cache
    void ResetDecoder();

    // Gets the page the decoder is currently docoding
    DWORD GetProcessingPageCode();
    // Gets the number of pages in the decoder cache
    ULONG GetReceivedPagesCount();
    // Get the currently detected national character set
    BYTE GetCharacterSubsetCode();

    // Get the status display in the broadcast service data
    void GetStatusDisplay(LPSTR lpBuffer, LONG nLength);

    // Gets the rolling header
    void GetDisplayHeader(TVTPage* pBuffer, BOOL bClockOnly = FALSE);
    // Gets a copy of the page described by dwPageCode
    DWORD GetDisplayPage(DWORD dwPageCode, TVTPage* pBuffer);

    // Gets a copy of the next page following dwPageCode
    DWORD GetNextDisplayPage(DWORD dwPageCode, TVTPage* pBuffer, BOOL bReverse);
    // Gets a copy of the next subpage following dwPageCode
    DWORD GetNextDisplaySubPage(DWORD dwPageCode, TVTPage* pBuffer, BOOL bReverse);

    // Gets a copy of the next page that matches the search string
    DWORD FindInDisplayPage(DWORD dwFromPageCode, BOOL bInclusive, LPSTR lpSearchString,
                            TVTPage* pBuffer, BOOL bReverse = FALSE);

    BOOL GetDisplayComment(DWORD dwPageCode, TVTPage* pBuffer);

    // Creates a test page into buffer pBuffer
    void CreateTestDisplayPage(TVTPage* pBuffer);

    // Sets the way lines are cached in the internal buffer
    void SetCachingControl(BYTE uCacheControl);

    // Turns on or off per byte error avoidance line caching
    void SetHighGranularityCaching(BOOL bEnable);

    // Sets whether errors in pages should be substituted with spaces
    void SetSubstituteSpacesForError(BOOL bEnable);

protected:

    // Bit vector used by LinkReceived
    enum
    {
        LINKRECV_RECVMASK            = 0x3F,
        LINKRECV_SHOW24              = 0x40,
        LINKRECV_HIDE24              = 0x80,
    };

    // The receiving buffer for new pages
    typedef struct _MagazineState
    {
        #pragma pack(push, 1)
        union
        {
            struct
            {
                WORD wPageHex;
                WORD wPageSubCode;
            };
            DWORD dwPageCode;
        };
        #pragma pack(pop)

        // Packet X/0
        BOOL bReceiving;
        WORD wControlBits;
        BYTE Header[32];

        // Packets X/1 to X/25
        BYTE Line[25][40];
        // This must be BYTE to be 1 byte big
        BYTE bLineReceived[25];

        // Packet X/27/0
        DWORD EditorialLink[6];
        BYTE LinkReceived;

    } TMagazineState;

    // Broadcast service data
    typedef struct _ServiceData
    {
        DWORD   InitialPage;
        WORD    NetworkIDCode;
        char    TimeOffset;
        DWORD   ModifiedJulianDate;
        BYTE    UTCHours;
        BYTE    UTCMinutes;
        BYTE    UTCSeconds;
        BYTE    StatusDisplay[20];
    } TServiceData;

    // Programme Delivery Control over Teletext
    typedef struct _PDC
    {
        BYTE  LCI   : 2;
        BYTE  LUF   : 1;
        BYTE  PRF   : 1;
        BYTE  PCS   : 2;
        BYTE  MI    : 1;
        WORD  CNI   : 16;
        DWORD PIL   : 20;
        BYTE  PTY   : 8;
    } TPDC;

protected:
    WORD PageHex2ArrayIndex(WORD wPageHex);

    void InitializePage(TVTPage* pPage);
    void ResetPageStore();
    TVTPage* GetPageStore(DWORD dwPageCode, BOOL bCreateMissing);
    void FreePageStore();

    void CopyPageForDisplay(TVTPage* pBuffer, TVTPage* pPage);
    void UnsetUpdatedStates(TVTPage* pPage);

    TVTPage* FindSubPage(TVTPage* pPageList, DWORD dwPageCode);
    TVTPage* FindNextSubPage(TVTPage* pPageList, DWORD dwPageCode,  BOOL bReverse);

    BOOL SearchPage(TVTPage* pPage, LPSTR lpSearchString, BOOL bIncludeRow25 = TRUE);

    void NotifyDecoderEvent(BYTE uMsg, DWORD dwParam);

private:
    void CompleteMagazine(TMagazineState* magazineState);
    BOOL HighGranularityLineCache(TVTPage* pPage, BYTE nRow, BYTE* pSource);

    static BYTE SearchPageProc(TVTPage*, WORD wPoint, LPWORD,
                               WORD, BYTE uChar, BYTE uMode, LPVOID lpParam);

private:
    BYTE                m_LastMagazine;
    TMagazineState      m_MagazineState[8];
    BOOL                m_bMagazineSerial;

    BYTE                m_CharacterSubset;
    BYTE                m_CommonHeader[32];
    ULONG               m_ReceivedPages;

    TVTPage*            m_NonVisiblePageList;
    TVTPage             m_VisiblePage[800];
    WORD                m_LatestSubPage[800];

    TServiceData        m_BroadcastServiceData;
    TPDC                m_PDC[4];

    TDecoderCallback*   m_DecoderEventProc;

    BYTE                m_CachingControl;
    BOOL                m_bHighGranularityCaching;

    BOOL                m_bSubstituteSpacesForError;

    BOOL                m_bTopTextForComment;
    CVTTopText*         m_pVTTopText;

    CRITICAL_SECTION    m_CommonHeaderMutex;
    CRITICAL_SECTION    m_MagazineStateMutex;
    CRITICAL_SECTION    m_PageStoreMutex;
    CRITICAL_SECTION    m_PDCStoreMutex;
};

#endif
