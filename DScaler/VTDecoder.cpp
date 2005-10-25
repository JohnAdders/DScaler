/////////////////////////////////////////////////////////////////////////////
// $Id: VTDecoder.cpp,v 1.19 2005-10-25 08:17:59 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Atsushi Nakagawa.  All rights reserved.
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
//
// Portions were based on VBI_VideoText.cpp rev 1.52.  Those
// portions are Copyright (c) 2000 John Adcock.  It may also
// incur the Copyrights:
//
// Copyright (c) 2002 Mike Temperton
//
// Copyright (c) 2002 Rob Muller
//
// Copyright (c) 2002 Jeroen Kooiman 
//
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
// Copyright (C) 1999-2000 Zoltán Sinkovics and Szabolcs Seláf
//
/////////////////////////////////////////////////////////////////////////////
//
// Change Log
//
// Date          Developer             Changes
//
// 21 Dec 2002   Atsushi Nakagawa      Remodularized videotext elements
//                                     Moved and redid decoding functionality
//                                     into this VTDecoder.cpp file.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.18  2005/07/27 22:49:33  laurentg
// Use WORD instead of DWORD
//
// Revision 1.17  2005/07/26 19:36:03  laurentg
// New functions to get network ID from P8/30/1,2
// History of 2 consecutive values for NetworkIDCode inside P8/30/1
//
// Revision 1.16  2005/07/25 22:43:38  laurentg
// m_PDC buffer cleared when decoder reset
//
// Revision 1.15  2005/07/25 22:32:51  laurentg
// Mutex added to access m_BroadcastServiceData and m_PDC
//
// Revision 1.14  2005/07/25 21:57:13  laurentg
// Bug fixed in PDC CNI decoding
//
// Revision 1.13  2004/10/11 22:21:45  atnak
// Added cautionary notes for 1-based vs 0-based indexing.
//
// Revision 1.12  2004/10/11 21:57:12  atnak
// Corrected parsing offset error in 8/30 packets.  (Thanks Rani Feldman)
//
// Revision 1.11  2004/05/16 19:54:54  atnak
// changed to not accept duplicate lines for same page in one reception
//
// Revision 1.10  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.9  2003/01/12 22:58:32  atnak
// Small bug fix
//
// Revision 1.8  2003/01/12 17:12:45  atnak
// Added hex pages display and goto dialog
//
// Revision 1.6  2003/01/08 00:23:40  atnak
// Bug fix
//
// Revision 1.5  2003/01/07 07:37:38  atnak
// Fixed page subcodes
//
// Revision 1.4  2003/01/05 16:09:44  atnak
// Updated TopText for new teletext
//
// Revision 1.3  2003/01/03 13:46:10  atnak
// Fixed editorial link page number
//
// Revision 1.2  2003/01/02 23:36:24  robmuller
// Small bug fix.
//
// Revision 1.1  2003/01/01 20:38:11  atnak
// New videotext decoder
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file VTDecoder.cpp CVTDecoder Implementation
 */

#include "stdafx.h"
#include "VTCommon.h"
#include "VTDecoder.h"
//#include "DebugLog.h"


/*
 *  This class is programmed to be multi-thread safe
 */


CVTDecoder::CVTDecoder(TDecoderCallback* fnDecoderEventProc)
{
    m_NonVisiblePageList = NULL;
    m_DecoderEventProc = fnDecoderEventProc;

    m_CachingControl = DECODERCACHE_NORMAL;
    m_bHighGranularityCaching = FALSE;

    m_bSubstituteSpacesForError = FALSE;

    InitializeCriticalSection(&m_CommonHeaderMutex);
    InitializeCriticalSection(&m_MagazineStateMutex);
    InitializeCriticalSection(&m_PageStoreMutex);
    InitializeCriticalSection(&m_ServiceDataStoreMutex);

    m_pVTTopText = new CVTTopText;

    ResetDecoder();
}


CVTDecoder::~CVTDecoder()
{
    EnterCriticalSection(&m_PageStoreMutex);
    FreePageStore();
    LeaveCriticalSection(&m_PageStoreMutex);

    delete m_pVTTopText;

    DeleteCriticalSection(&m_ServiceDataStoreMutex);
    DeleteCriticalSection(&m_PageStoreMutex);
    DeleteCriticalSection(&m_MagazineStateMutex);
    DeleteCriticalSection(&m_CommonHeaderMutex);
}


void CVTDecoder::ResetDecoder()
{
    EnterCriticalSection(&m_CommonHeaderMutex);
    {
        FillMemory(m_CommonHeader, 32, 0x20);
    }
    LeaveCriticalSection(&m_CommonHeaderMutex);

    EnterCriticalSection(&m_MagazineStateMutex);
    {
        for (int i(0); i < 8; i++)
        {
            // Doing this is enough for zeroing the state
            m_MagazineState[i].bReceiving = FALSE;
        }
        m_LastMagazine = 0;
    }
    LeaveCriticalSection(&m_MagazineStateMutex);

    EnterCriticalSection(&m_PageStoreMutex);
    {
        ResetPageStore();
        m_ReceivedPages = 0;
    }
    LeaveCriticalSection(&m_PageStoreMutex);

    m_pVTTopText->Reset();

	EnterCriticalSection(&m_ServiceDataStoreMutex);
    m_BroadcastServiceData.InitialPage = 0UL;
	int i;
	for (i = 0; i < 2; i++)
	{
	    m_BroadcastServiceData.NetworkIDCode[i] = 0;
	}
    m_BroadcastServiceData.TimeOffset = 0;
    m_BroadcastServiceData.ModifiedJulianDate = 45000;
    m_BroadcastServiceData.UTCHours = 0;
    m_BroadcastServiceData.UTCMinutes = 0;
    m_BroadcastServiceData.UTCSeconds = 0;
    FillMemory(m_BroadcastServiceData.StatusDisplay, 20, 0x20);
	for (i = 0; i < 4; i++)
	{
		m_PDC[i].LCI = 0;
		m_PDC[i].LUF = 0;
		m_PDC[i].PRF = 0;
		m_PDC[i].PCS = 0;
		m_PDC[i].MI = 0;
		m_PDC[i].CNI = 0;
		m_PDC[i].PIL = 0;
		m_PDC[i].PTY = 0;
	}
	LeaveCriticalSection(&m_ServiceDataStoreMutex);

    m_bMagazineSerial = FALSE;
    m_CharacterSubset = 0;
}


void CVTDecoder::DecodeLine(BYTE* data)
{
    BOOL bError = FALSE;
    BYTE magazine;
    BYTE packetNumber;
    BYTE designationCode = 0;
    BYTE s1, s2, s3, s4;
    BYTE c7_14;
    WORD wPageHex;
    WORD wPageSubCode;
    WORD wControlBits;
    DWORD dwPageCode;

    // Caution: Remember that these offsets are zero based while indexes
    // in the ETS Teletext specification (ETS 300 706) are one based.
    packetNumber = UnhamTwo84_LSBF(data + 3, &bError);

    if (bError != FALSE)
    {
        // This error is unrecoverable
        return;
    }

    magazine = (packetNumber & 0x07);
    packetNumber >>= 3;

    // A page header terminates the last page reception
    if (packetNumber == 0)
    {
        EnterCriticalSection(&m_MagazineStateMutex);

        // Check if there is a finished page
        BYTE processMagazine = m_bMagazineSerial ? m_LastMagazine : magazine;
        if (m_MagazineState[processMagazine].bReceiving)
        {
            m_MagazineState[processMagazine].bReceiving = FALSE;
            CompleteMagazine(&m_MagazineState[processMagazine]);
        }

        LeaveCriticalSection(&m_MagazineStateMutex);
    }

    // A packets 26 or higher uses a designation code
    if (packetNumber >= 26)
    {
        designationCode = Unham84(data + 5, &bError);

        if (bError != FALSE)
        {
            // We can't continue without this
            return;
        }
    }

    switch (packetNumber)
    {
    case 0: // page header

        // Initialize the magazine
        EnterCriticalSection(&m_MagazineStateMutex);
        m_MagazineState[magazine].bReceiving = FALSE;
        LeaveCriticalSection(&m_MagazineStateMutex);

        // Work out the page number
        wPageHex = UnhamTwo84_LSBF(data + 5, &bError);
        wPageHex |= (magazine == 0 ? 0x800 : magazine * 0x100);

        // Caution: Remember that these offsets are zero based while indexes
        // in the ETS Teletext specification (ETS 300 706) are one based.
        s1 = Unham84(data + 7, &bError);
        s2 = Unham84(data + 8, &bError);
        s3 = Unham84(data + 9, &bError);
        s4 = Unham84(data + 10, &bError);

        c7_14 = UnhamTwo84_LSBF(data + 11, &bError);

        if (bError != FALSE)
        {
            // Cannot proceed without a valid page number
            return;
        }
        // Work out the page sub-code
        wPageSubCode = (s1 | ((s2 & 0x7) << 4) | (s3 << 8) | ((s4 & 0x3) << 12));

        // Get the page control bits
        wControlBits = (magazine | (s2 & 0x8) | ((s4 & 0xC) << 2) | (c7_14 << 6));

        m_bMagazineSerial = (wControlBits & VTCONTROL_MAGSERIAL) != 0;
        m_CharacterSubset = (wControlBits & VTCONTROL_CHARSUBSET) >> 11;

        // Update the rolling header if this header is usable
        if ((wControlBits & VTCONTROL_SUPRESSHEADER) == 0 &&
            (wControlBits & VTCONTROL_INTERRUPTED) == 0)
        {
            // Don't use non-visible page headers because
            // some on them are missing the clock.
            if (IsNonVisiblePage(wPageHex) == FALSE)
            {
                if (CheckParity(data + 13, 32) != FALSE)
                {
                    EnterCriticalSection(&m_CommonHeaderMutex);
                    CopyMemory(m_CommonHeader, data + 13, 32);
                    LeaveCriticalSection(&m_CommonHeaderMutex);

                    NotifyDecoderEvent(DECODEREVENT_HEADERUPDATE, 0);
                }
            }
        }

        // See if this is an invalid page
        if ((wPageHex & 0xFF) == 0xFF)
        {
            // Time filling page
            return;
        }

        EnterCriticalSection(&m_MagazineStateMutex);

        m_MagazineState[magazine].bReceiving = TRUE;
        m_MagazineState[magazine].wPageHex = wPageHex;
        m_MagazineState[magazine].wPageSubCode = wPageSubCode;
        m_MagazineState[magazine].wControlBits = wControlBits;

        CopyMemory(m_MagazineState[magazine].Header, data + 13, 32);
        ZeroMemory(m_MagazineState[magazine].bLineReceived, 25);
        m_MagazineState[magazine].LinkReceived = 0x00;

        m_LastMagazine = magazine;

        LeaveCriticalSection(&m_MagazineStateMutex);

        NotifyDecoderEvent(DECODEREVENT_PAGEBEGIN, MAKELONG(wPageHex, wPageSubCode));
        break;

    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
        {
            DWORD dwTTUpdate = 0UL;

            EnterCriticalSection(&m_MagazineStateMutex);

            if (m_MagazineState[magazine].bReceiving != FALSE)
            {
                BYTE line = packetNumber - 1;

                // 2004/05/10 atnak: do not override an existing line
                if (m_MagazineState[magazine].bLineReceived[line] != FALSE)
                {
                    // Assume that a new header was missed and close the magazine
                    m_MagazineState[magazine].bReceiving = FALSE;
                    CompleteMagazine(&m_MagazineState[magazine]);
                }
                else
                {
                    CopyMemory(m_MagazineState[magazine].Line[line], data + 5, 40);
                    m_MagazineState[magazine].bLineReceived[line] = TRUE;

                    dwPageCode = m_MagazineState[magazine].dwPageCode;

                    if (m_pVTTopText->IsTopTextPage(dwPageCode))
                    {
                        dwTTUpdate = m_pVTTopText->DecodePageRow(dwPageCode,
                            packetNumber, m_MagazineState[magazine].Line[line]);
                    }
                }
            }

            LeaveCriticalSection(&m_MagazineStateMutex);

            if (dwTTUpdate != 0UL)
            {
                if (m_bTopTextForComment != FALSE)
                {
                    NotifyDecoderEvent(DECODEREVENT_COMMENTUPDATE, dwTTUpdate);
                }
                else
                {
                    NotifyDecoderEvent(DECODEREVENT_PAGEUPDATE, dwTTUpdate);
                }
            }
        }
        break;

    case 26:
        break;

    case 27:
        EnterCriticalSection(&m_MagazineStateMutex);

        if (m_MagazineState[magazine].bReceiving != FALSE)
        {
            // Packet X/27/0 for FLOF Editorial Linking
            if (designationCode == 0)
            {
                // 2004/05/10 atnak: do not override existing information
                if (m_MagazineState[magazine].LinkReceived != 0x00)
                {
                    // Assume that a new header was missed and close the magazine
                    m_MagazineState[magazine].bReceiving = FALSE;
                    CompleteMagazine(&m_MagazineState[magazine]);
                }
                else
                {
                    for (int n(0); n < 6; n++)
                    {
                        bError = FALSE;
                        s1 = Unham84(data + 6*n + 8, &bError);
                        s2 = Unham84(data + 6*n + 9, &bError);
                        s3 = Unham84(data + 6*n + 10, &bError);
                        s4 = Unham84(data + 6*n + 11, &bError);

                        wPageHex = UnhamTwo84_LSBF(data + 6*n + 6, &bError);

                        if (bError != FALSE)
                        {
                            continue;
                        }

                        BYTE linkMagazine = magazine ^ (s2 >> 3 | (s4 & 0xC) >> 1);

                        wPageHex |= (linkMagazine == 0 ? 0x800 : linkMagazine * 0x100);
                        wPageSubCode = (s1 | ((s2 & 0x7) << 4));
                        wPageSubCode |= ((s3 << 8) | ((s4 & 0x3) << 12));

                        dwPageCode = MAKELONG(wPageHex, wPageSubCode);

                        m_MagazineState[magazine].EditorialLink[n] = dwPageCode;
                        m_MagazineState[magazine].LinkReceived |= (1 << n);
                    }

                    BYTE linkControlByte = Unham84(data + 42, &bError);

                    if (bError == FALSE)
                    {
                        if ((linkControlByte & 0x08) != 0)
                        {
                            m_MagazineState[magazine].LinkReceived |= LINKRECV_SHOW24;
                        }
                        else
                        {
                            m_MagazineState[magazine].LinkReceived |= LINKRECV_HIDE24;
                        }
                    }
                }
            }
        }

        LeaveCriticalSection(&m_MagazineStateMutex);
        break;

    case 28:
        /* don't see it; keep level 1 only
        {
            // X/28/0
            BYTE c1 = *(dat + 5);
            BYTE c2 = *(dat + 6);
            BYTE c3 = *(dat + 7);

            BYTE DesignationCode = ((c1 & 0x02) >> 1 )|((c1 & 0x08) >> 2 )|((c1 & 0x20) >> 3 )|((c1 & 0x80) >> 4 );
            BYTE PageFunction = c2 & 0x0F;

            //  format 1 and            basic level 1
            if( DesignationCode != 0 || PageFunction != 0 )
            {
                VTCodePageX28 = -1;
                C12C13C14 = -1;
            }
            else
            {
                C12C13C14 = ((c2 & 0x80) >> 6)|((c3 & 0x01) << 1)|((c3 & 0x02) >> 1); // 1>(8 9 10)>16 => 7>(2 1 0)>0
                VTCodePageX28 = ((c3 & 0x04) >> 2)|((c3 & 0x08) >> 2)|((c3 & 0x10) >> 2)|((c3 & 0x20) >> 2);
            }
            char xx[40];
            LOG(1, xx, "TELETEXT CO 28 %X %X %X - %X %X %X %X", c1, c2, c3, DesignationCode, PageFunction, VTCodePageX28, C12C13C14);
        }
        */
        break;

    case 29:
        /* don't see it; keep level 1 only
        LOG(1, xx, "TELETEXT CO 29");
        */
        break;

    case 30:
		EnterCriticalSection(&m_ServiceDataStoreMutex);

        // 8/30/0,1 is Format 1 Broadcast Service Data
        // 8/30/2,3 is Format 2 Broadcast Service Data
        if (magazine == 0 && (designationCode & 0xC) == 0x0)
        {
            // Caution: Remember that these offsets are zero based while indexes
            // in the ETS Teletext specification (ETS 300 706) are one based.
            s1 = Unham84(data + 8, &bError);
            s2 = Unham84(data + 9, &bError);
            s3 = Unham84(data + 10, &bError);
            s4 = Unham84(data + 11, &bError);

            wPageHex = UnhamTwo84_LSBF(data + 6, &bError);

            if (bError == FALSE)
            {
                BYTE initialMagazine = (s2 >> 3 | (s4 & 0xC) >> 1);

                wPageHex |= (initialMagazine == 0 ? 0x800 : initialMagazine * 0x100);
                wPageSubCode = (s1 | ((s2 & 0x7) << 4));
                wPageSubCode |= ((s3 << 8) | ((s4 & 0x3) << 12));

                dwPageCode = (DWORD)MAKELONG(wPageHex, wPageSubCode);

                m_BroadcastServiceData.InitialPage = dwPageCode;
            }

            // Format 1 and Format 2 are different from here
            if ((designationCode & 0x2) == 0)
            {
                // The Network Identification Code has bits in reverse
                WORD wNetworkIDCode = ReverseBits(data[13]) | ReverseBits(data[12]) << 8;
                m_BroadcastServiceData.NetworkIDCode[1] = m_BroadcastServiceData.NetworkIDCode[0];
                m_BroadcastServiceData.NetworkIDCode[0] = wNetworkIDCode;
				/*
				LOG(1, "m_BroadcastServiceData.NetworkIDCode[0] %x", m_BroadcastServiceData.NetworkIDCode[0]);
				*/

                // Time offset from UTC in half hour units
                char timeOffset = (data[14] >> 1) * ((data[14] & 0x40) ? -1 : 1);
                m_BroadcastServiceData.TimeOffset = timeOffset;

                // Modified Julian Date 45000 is 31 January 1982
                ULONG modifiedJulianDate = 0L;

                modifiedJulianDate += ((data[15] & 0x0F) >> 0) * 10000;
                modifiedJulianDate += ((data[16] & 0xF0) >> 4) * 1000;
                modifiedJulianDate += ((data[16] & 0x0F) >> 0) * 100;
                modifiedJulianDate += ((data[17] & 0xF0) >> 4) * 10;
                modifiedJulianDate += ((data[17] & 0x0F) >> 0) * 1;

                m_BroadcastServiceData.ModifiedJulianDate = modifiedJulianDate;

                BYTE Hours   = (data[18] & 0x0F) + ((data[18] & 0xF0) >> 4) * 10;
                BYTE Minutes = (data[19] & 0x0F) + ((data[19] & 0xF0) >> 4) * 10;
                BYTE Seconds = (data[20] & 0x0F) + ((data[20] & 0xF0) >> 4) * 10;

                m_BroadcastServiceData.UTCHours = Hours;
                m_BroadcastServiceData.UTCMinutes = Minutes;
                m_BroadcastServiceData.UTCSeconds = Seconds;
            }
            else
            {
                // Programme Identification Data

                BYTE LCI = Unham84(data + 12, &(bError = FALSE));
                if (bError == FALSE)
                {
                    BYTE LUF = (LCI & 0x4) != 0;
                    BYTE PRF = (LCI & 0x8) != 0;
                    LCI &= 0x3;

                    m_PDC[LCI].LCI = LCI;
                    m_PDC[LCI].LUF = LUF;
                    m_PDC[LCI].PRF = PRF;

                    BYTE PCS = Unham84(data + 13, &(bError = FALSE));
                    if (bError == FALSE)
                    {
                        BYTE MI = (PCS & 0x4) != 0;
                        PCS &= 0x3;

                        m_PDC[LCI].PCS = PCS;
                        m_PDC[LCI].MI = MI;
                    }

                    bError = FALSE;
                    BYTE b16 = Unham84(data + 15, &bError);
                    BYTE b21 = Unham84(data + 20, &bError);

                    if (bError == FALSE)
                    {
                        BYTE b22 = Unham84(data + 21, &bError);
                        WORD CNI = Unham84(data + 14, &bError);

                        CNI |= ((b21 & 0xC) << 2) | ((b22 & 0x3) << 6);
                        CNI |= ((b16 & 0x3) << 8) | ((b22 & 0xC) << 8);
                        CNI |= Unham84(data + 22, &bError) << 12;

                        if (bError == FALSE)
                        {
                            // Reverse the bits in the WORD
                            CNI = (CNI & 0x00FF) << 8 | (CNI & 0xFF00) >> 8;
                            CNI = (CNI & 0x0F0F) << 4 | (CNI & 0xF0F0) >> 4;
                            CNI = (CNI & 0x3333) << 2 | (CNI & 0xCCCC) >> 2;
                            CNI = (CNI & 0x5555) << 1 | (CNI & 0xAAAA) >> 1;

                            m_PDC[LCI].CNI = CNI;
                        }

                        bError = FALSE;

                        DWORD PIL = (b16 >> 2);
                        PIL |= Unham84(data + 16, &bError) << 2;
                        PIL |= Unham84(data + 17, &bError) << 6;
                        PIL |= Unham84(data + 18, &bError) << 10;
                        PIL |= Unham84(data + 19, &bError) << 14;
                        PIL |= (b21 & 0x3) << 18;

                        if (bError == FALSE)
                        {
                            m_PDC[LCI].PIL = PIL;
                        }
                    }

                    BYTE PTY = UnhamTwo84_LSBF(data + 23, &(bError = FALSE));
                    if (bError == FALSE)
                    {
                        m_PDC[LCI].PTY = PTY;
                    }

					/*
					LOG(1, "LCI = %d, Country = %x, Network = %x", LCI, (m_PDC[LCI].CNI >> 8) & 0xFF, (m_PDC[LCI].CNI & 0xFF));
					LOG(1, "Day = %d, Month = %d, %d:%d", (m_PDC[LCI].PIL & 0x1F), (m_PDC[LCI].PIL >> 5) & 0x0F,
														  (m_PDC[LCI].PIL >> 9) & 0x1F, (m_PDC[LCI].PIL >> 14) & 0x3F);
					LOG(1, "Program Type = %d", m_PDC[LCI].PTY);
					*/

                    NotifyDecoderEvent(DECODEREVENT_PDCUPDATE, 0);

                }
            }

            CopyMemory(m_BroadcastServiceData.StatusDisplay, data + 25, 20);
        }

		LeaveCriticalSection(&m_ServiceDataStoreMutex);
        break;

    case 31:  // Independent data services
        break;

    default:
        break;
    }
}


void CVTDecoder::CompleteMagazine(TMagazineState* magazineState)
{
    WORD wPageHex = magazineState->wPageHex;
    WORD wPageSubCode = magazineState->wPageSubCode;
    WORD wControlBits = magazineState->wControlBits;

    BOOL bErasePage = (wControlBits & VTCONTROL_ERASEPAGE) != 0;
    BOOL bPageUpdate = (wControlBits & VTCONTROL_UPDATE) != 0;
    BOOL bFlushLines = FALSE;

    BOOL bLinesAdded = FALSE;
    BOOL bPageChanged = FALSE;

    EnterCriticalSection(&m_PageStoreMutex);

    TVTPage *pPage = GetPageStore(magazineState->dwPageCode, TRUE);

    // Erase the storage if this is the first receive
    if (pPage->bReceived == FALSE)
    {
        bErasePage = TRUE;
    }

    // Erase the page if inhibit display is set
    if ((wControlBits & VTCONTROL_INHIBITDISP) != 0)
    {
        bErasePage = TRUE;
    }

    // Make sure old lines aren't retained if erase page
    if (bErasePage != FALSE)
    {
        bPageUpdate = TRUE;
    }

    if (bPageUpdate != FALSE)
    {
        bFlushLines = TRUE;
    }

    // Copy the header if it's not suppressed
    if ((wControlBits & VTCONTROL_SUPRESSHEADER) != 0)
    {
        if ((pPage->LineState[0] & CACHESTATE_HASDATA) != 0)
        {
            FillMemory(&pPage->Frame[0][8], 32, 0x20);
            pPage->LineState[0] = CACHESTATE_UPDATED;
            bPageChanged = TRUE;
        }
    }
    else
    {
        // Only copy the header if it is an update or
        // if the previous reception had an error
        if (bFlushLines != FALSE ||
            (pPage->LineState[0] & CACHESTATE_HASDATA) == 0 ||
            (pPage->LineState[0] & CACHESTATE_HASERROR) != 0)
        {
            pPage->LineState[0] = CACHESTATE_HASDATA | CACHESTATE_UPDATED;

            if (CheckParity(magazineState->Header, 32) == FALSE)
            {
                pPage->LineState[0] |= CACHESTATE_HASERROR;
            }

            CopyMemory(&pPage->Frame[0][8], magazineState->Header, 32);
            bPageChanged = TRUE;
        }
        else if (m_CachingControl == DECODERCACHE_SECONDCHANCE)
        {
            // If second chance error correction is on, copy the
            // line regardless, as long as the new line is error
            // free.
            if (CheckParity(magazineState->Header, 32) != FALSE)
            {
                pPage->LineState[0] = CACHESTATE_HASDATA | CACHESTATE_UPDATED;
                CopyMemory(&pPage->Frame[0][8], magazineState->Header, 32);
                bPageChanged = TRUE;
            }
        }
    }

    // Some broadcasters may not explicitly set update for
    // subtitles but it needs to be set for subtitles to change
    if ((wControlBits & VTCONTROL_SUBTITLE) != 0)
    {
        bFlushLines = TRUE;
    }

    if (m_CachingControl == DECODERCACHE_ALWAYSUPDATE)
    {
        bFlushLines = TRUE;
    }

    // Copy the display lines
    for (int i = 1; i < 26; i++)
    {
        if (magazineState->bLineReceived[i-1] != FALSE &&
            (wControlBits & VTCONTROL_INHIBITDISP) == 0)
        {
            // Only copy the line if it is an update or
            // if the previous reception had an error
            if (bFlushLines != FALSE ||
                (pPage->LineState[i] & CACHESTATE_HASDATA) == 0 ||
                (pPage->LineState[i] & CACHESTATE_HASERROR) != 0)
            {
                if (m_bHighGranularityCaching != FALSE && bPageUpdate == FALSE)
                {
                    if (HighGranularityLineCache(pPage, i, magazineState->Line[i-1]))
                    {
                        bLinesAdded = TRUE;
                    }
                }
                else
                {
                    pPage->LineState[i] = CACHESTATE_HASDATA | CACHESTATE_UPDATED;

                    if (CheckParity(magazineState->Line[i-1], 40) == FALSE)
                    {
                        pPage->LineState[i] |= CACHESTATE_HASERROR;
                    }

                    CopyMemory(pPage->Frame[i], magazineState->Line[i-1], 40);
                    bLinesAdded = TRUE;
                }
            }
            else if (m_CachingControl == DECODERCACHE_SECONDCHANCE)
            {
                // If second chance error correction is on, copy the
                // line regardless, as long as the new line is error
                // free.

                if (m_bHighGranularityCaching != FALSE)
                {
                    if (HighGranularityLineCache(pPage, i, magazineState->Line[i-1]))
                    {
                        bLinesAdded = TRUE;
                    }
                }
                else if (CheckParity(magazineState->Line[i-1], 40) != FALSE)
                {
                    pPage->LineState[i] = CACHESTATE_HASDATA | CACHESTATE_UPDATED;
                    CopyMemory(pPage->Frame[i], magazineState->Line[i-1], 40);
                    bLinesAdded = TRUE;
                }
            }
        }
        else if (bErasePage != FALSE)
        {
            if ((pPage->LineState[i] & CACHESTATE_HASDATA) != 0)
            {
                FillMemory(pPage->Frame[i], 40, 0x20);
                pPage->LineState[i] = CACHESTATE_UPDATED;
                bPageChanged = TRUE;
            }
        }
    }

    // Copy packet X/27/0 (FLOF) stuff
    for (int n = 0; n < 6; n++)
    {
        if (magazineState->LinkReceived & (1 << n))
        {
            pPage->EditorialLink[n] = magazineState->EditorialLink[n];
        }
    }

    if (magazineState->LinkReceived & LINKRECV_SHOW24)
    {
        pPage->bShowRow24 = TRUE;
    }
    else if (magazineState->LinkReceived & LINKRECV_HIDE24)
    {
        pPage->bShowRow24 = FALSE;
    }


    // Mark all lines as updated if the subtitle or newsflash status changed
    if ((wControlBits & (VTCONTROL_NEWSFLASH | VTCONTROL_SUBTITLE)) !=
        (pPage->wControlBits & (VTCONTROL_NEWSFLASH | VTCONTROL_SUBTITLE)))
    {
        for (int i = 0; i < 25; i++)
        {
            pPage->LineState[i] |= CACHESTATE_UPDATED;
        }
    }

    // Copy the page control bits
    pPage->wControlBits = wControlBits;


    // Update the cache count
    if (pPage->bReceived == FALSE)
    {
        // There's no point keeping the page
        // if it came with no display lines.
        if (bLinesAdded == FALSE)
        {
            LeaveCriticalSection(&m_PageStoreMutex);
            return;
        }

        pPage->bReceived = TRUE;
        m_ReceivedPages++;
    }

    LeaveCriticalSection(&m_PageStoreMutex);

    if (bLinesAdded != FALSE || bPageChanged != FALSE)
    {
        NotifyDecoderEvent(DECODEREVENT_PAGEUPDATE, magazineState->dwPageCode);
    }
    else
    {
        NotifyDecoderEvent(DECODEREVENT_PAGEREFRESH, magazineState->dwPageCode);
    }
}


BOOL CVTDecoder::HighGranularityLineCache(TVTPage* pPage, BYTE nRow, BYTE* pSource)
{
    BOOL bLineUpdated = FALSE;
    BOOL bHasError = FALSE;

    for (int i = 0; i < 40; i++)
    {
        if (CheckParity(&pSource[i]) != FALSE)
        {
            pPage->Frame[nRow][i] = pSource[i];
            bLineUpdated = TRUE;
        }
        else
        {
            if (CheckParity(&pPage->Frame[nRow][i]) == FALSE)
            {
                bHasError = TRUE;
            }
        }
    }

    pPage->LineState[nRow] = CACHESTATE_HASDATA;

    if (bLineUpdated != FALSE)
    {
        pPage->LineState[nRow] |= CACHESTATE_UPDATED;
    }

    if (bHasError != FALSE)
    {
        pPage->LineState[nRow] |= CACHESTATE_HASERROR;
    }

    return bLineUpdated;
}


WORD CVTDecoder::PageHex2ArrayIndex(WORD wPageHex)
{
    if ((wPageHex & 0xFF00) < 0x0100 ||
        (wPageHex & 0xFF00) > 0x0800 ||
        (wPageHex & 0x00F0) > 0x0090 ||
        (wPageHex & 0x000F) > 0x0009)
    {
        return 0xFFFF;
    }

    WORD wArrayIndex;

    wArrayIndex = (((wPageHex & 0xF00) >> 8) * 100) - 100;
    wArrayIndex += ((wPageHex & 0x0F0) >> 4) * 10;
    wArrayIndex += ((wPageHex & 0x00F));

    return wArrayIndex;
}


void CVTDecoder::InitializePage(TVTPage* pPage)
{
    pPage->dwPageCode   = MAKELONG(0x1FF, 0x3F7F);
    pPage->wControlBits = 0x0000;

    FillMemory(pPage->LineState, 26,
        CACHESTATE_HASDATA | CACHESTATE_HASERROR);

    for (int i = 0; i < 6; i++)
    {
        pPage->EditorialLink[i] = 0UL;
    }

    // The first eight bytes of the header never changes
    FillMemory(&pPage->Frame[0][0], 8, 0x20);

    pPage->bReceived    = FALSE;
    pPage->bShowRow24   = FALSE;
    // pNextPage must not be set NULL here
}


void CVTDecoder::ResetPageStore()
{
    TVTPage* pPage;

    for (pPage = m_NonVisiblePageList; pPage != NULL; pPage = pPage->pNextPage)
    {
        pPage->bReceived = FALSE;
        pPage->bBufferReserved = FALSE;
    }

    for (int i(0); i < 800; i++)
    {
        pPage = m_VisiblePageList[i];
        for ( ; pPage != NULL; pPage = pPage->pNextPage)
        {
            pPage->bReceived = FALSE;
            pPage->bBufferReserved = FALSE;
        }
    }
}


TVTPage* CVTDecoder::GetPageStore(DWORD dwPageCode, BOOL bUpdate)
{
    WORD wPageHex = LOWORD(dwPageCode);

    TVTPage* pPage;
    TVTPage** hPage;
    TVTPage** hList;

    if (IsNonVisiblePage(wPageHex))
    {
        if ((wPageHex & 0xFF) == 0xFF)
        {
            return NULL;
        }

        hList = &m_NonVisiblePageList;
    }
    else
    {
        WORD wPageIndex = PageHex2ArrayIndex(wPageHex);

        if (wPageIndex == 0xFFFF)
        {
            return NULL;
        }

        hList = &m_VisiblePageList[wPageIndex];
    }

    WORD nCount = 0;

    // Look for an available page buffer
    for (hPage = hList; *hPage != NULL; hPage = &(*hPage)->pNextPage)
    {
        if ((*hPage)->bBufferReserved == FALSE)
        {
            break;
        }
        else if ((*hPage)->dwPageCode == dwPageCode)
        {
            if (bUpdate != FALSE)
            {
                // Move the element to the front
                if (hPage != hList)
                {
                    pPage = *hPage;
                    *hPage = pPage->pNextPage;
                    pPage->pNextPage = *hList;
                    *hList = pPage;
                }
                return *hList;
            }

            return *hPage;
        }
        else if (++nCount == kMAX_PAGELIST)
        {
            break;
        }
    }

    if (bUpdate == FALSE)
    {
        return NULL;
    }

    // Create a new buffer if there isn't a spare
    if (*hPage == NULL)
    {
        *hPage = (TVTPage*)malloc(sizeof(TVTPage));
        (*hPage)->pNextPage = NULL;
        (*hPage)->bReceived = FALSE;
    }

    pPage = *hPage;

    // Move the new buffer to the front
    if (hPage != hList)
    {
        *hPage = pPage->pNextPage;
        pPage->pNextPage = *hList;
        *hList = pPage;
    }

    pPage->bBufferReserved = TRUE;
    
    InitializePage(pPage);
    pPage->dwPageCode = dwPageCode;

    return pPage;
}


void CVTDecoder::FreePageStore()
{
    TVTPage* pPage;

    while ((pPage = m_NonVisiblePageList) != NULL)
    {
        m_NonVisiblePageList = pPage->pNextPage;
        free(pPage);
    }

    for (int i(0); i < 800; i++)
    {
        while ((pPage = m_VisiblePageList[i]) != NULL)
        {
            m_VisiblePageList[i] = pPage->pNextPage;
            free(pPage);
        }
    }
}


DWORD CVTDecoder::GetProcessingPageCode()
{
    DWORD dwPageCode = 0UL;

    EnterCriticalSection(&m_MagazineStateMutex);
    if (m_MagazineState[m_LastMagazine].bReceiving != FALSE)
    {
        dwPageCode = m_MagazineState[m_LastMagazine].dwPageCode;
    }
    LeaveCriticalSection(&m_MagazineStateMutex);

    return dwPageCode;
}


ULONG CVTDecoder::GetReceivedPagesCount()
{
    return m_ReceivedPages;
}


BYTE CVTDecoder::GetCharacterSubsetCode()
{
    return m_CharacterSubset;
}


WORD CVTDecoder::GetVisiblePageNumbers(LPWORD lpNumberList, WORD nListSize)
{
    WORD nPagesCount = 0;
    TVTPage* pPage;

    EnterCriticalSection(&m_PageStoreMutex);

    for (WORD i = 0; i < 800 && nPagesCount < nListSize; i++)
    {
        pPage = FindReceivedPage(m_VisiblePageList[i]);
        if (pPage != NULL)
        {
            lpNumberList[nPagesCount] = LOWORD(pPage->dwPageCode);
            nPagesCount++;
        }
    }

    LeaveCriticalSection(&m_PageStoreMutex);

    return nPagesCount;
}


WORD CVTDecoder::GetNonVisiblePageNumbers(LPWORD lpNumberList, WORD nListSize)
{
    WORD nPagesCount = 0;
    TVTPage* pPage;

    ASSERT(nListSize != 0);

    EnterCriticalSection(&m_PageStoreMutex);

    for (pPage = FindReceivedPage(m_NonVisiblePageList);
        pPage != NULL && nPagesCount < nListSize;
        pPage = FindReceivedPage(pPage->pNextPage))
    {
        for (WORD i = 0; i < nPagesCount; i++)
        {
            if (lpNumberList[i] == LOWORD(pPage->dwPageCode))
            {
                break;
            }
        }

        if (i == nPagesCount)
        {
            lpNumberList[nPagesCount] = LOWORD(pPage->dwPageCode);
            nPagesCount++;
        }
    }

    LeaveCriticalSection(&m_PageStoreMutex);

    return nPagesCount;
}


void CVTDecoder::GetDisplayHeader(TVTPage* pBuffer, BOOL bClockOnly)
{
    if (bClockOnly != FALSE)
    {
        EnterCriticalSection(&m_CommonHeaderMutex);
        CopyMemory(&pBuffer->Frame[0][32], &m_CommonHeader[24], 8);
        CheckParity(&pBuffer->Frame[0][32], 8, TRUE);
        LeaveCriticalSection(&m_CommonHeaderMutex);
    }
    else
    {
        EnterCriticalSection(&m_CommonHeaderMutex);
        CopyMemory(&pBuffer->Frame[0][8], m_CommonHeader, 32);
        CheckParity(&pBuffer->Frame[0][8], 32, TRUE);
        LeaveCriticalSection(&m_CommonHeaderMutex);
    }
    pBuffer->LineState[0] |= CACHESTATE_HASDATA | CACHESTATE_UPDATED;
}


DWORD CVTDecoder::GetDisplayPage(DWORD dwPageCode, TVTPage* pBuffer)
{
    WORD wPageHex = LOWORD(dwPageCode);
    TVTPage** hPageList;
    TVTPage* pPage;

    if (IsNonVisiblePage(wPageHex))
    {
        if ((wPageHex & 0xFF) == 0xFF)
        {
            return 0UL;
        }

        hPageList = &m_NonVisiblePageList;
    }
    else
    {
        WORD wPageIndex = PageHex2ArrayIndex(wPageHex);

        if (wPageIndex == 0xFFFF)
        {
            return 0UL;
        }

        hPageList = &m_VisiblePageList[wPageIndex];
    }

    EnterCriticalSection(&m_PageStoreMutex);

    if (HIWORD(dwPageCode) >= 0x3F7F)
    {
        pPage = FindReceivedPage(*hPageList);

        while (pPage != NULL)
        {
            if (LOWORD(pPage->dwPageCode) == wPageHex)
            {
                break;
            }
            pPage = FindReceivedPage(pPage->pNextPage);
        }
    }
    else
    {
        pPage = FindSubPage(*hPageList, dwPageCode);
    }

    if (pPage != NULL)
    {
        CopyPageForDisplay(pBuffer, pPage);
        UnsetUpdatedStates(pPage);
    }

    LeaveCriticalSection(&m_PageStoreMutex);

    return pPage != NULL ? pPage->dwPageCode : 0UL;
}


DWORD CVTDecoder::GetNextDisplayPage(DWORD dwPageCode, TVTPage* pBuffer,
                                     BOOL bReverse)
{
    WORD wPageHex = LOWORD(dwPageCode);
    WORD wPageIndex = PageHex2ArrayIndex(wPageHex);
    
    TVTPage* pPage = NULL;
    DWORD dwNextPageCode = 0UL;

    EnterCriticalSection(&m_PageStoreMutex);

    if (wPageIndex == 0xFFFF)
    {
        // Find the closest visible page
        if (IsNonVisiblePage(wPageHex))
        {
            wPageIndex = ((wPageHex & 0xF00) >> 8) * 100;
        }
        else
        {
            wPageIndex = 800;
        }

        if (bReverse != FALSE)
        {
            wPageIndex--;
        }

        wPageIndex %= 800;

        pPage = FindReceivedPage(m_VisiblePageList[wPageIndex]);
    }

    if (pPage == NULL)
    {
        char delta = (bReverse ? -1 : 1);

        // Loop around the available pages to find the next or previous page
        for (WORD i = 800 + wPageIndex + delta; (i % 800) != wPageIndex; i += delta)
        {
            pPage = FindReceivedPage(m_VisiblePageList[i % 800]);
            if (pPage != NULL)
            {
                break;
            }
        }
    }

    if (pPage != NULL)
    {
        CopyPageForDisplay(pBuffer, pPage);
        UnsetUpdatedStates(pPage);

        dwNextPageCode = pPage->dwPageCode;
    }

    LeaveCriticalSection(&m_PageStoreMutex);

    return dwNextPageCode;
}


DWORD CVTDecoder::GetNextDisplaySubPage(DWORD dwPageCode, TVTPage* pBuffer,
                                        BOOL bReverse)
{
    WORD wPageHex = LOWORD(dwPageCode);
    WORD wPageIndex = PageHex2ArrayIndex(wPageHex);
    TVTPage* pSubPageList;

    if (wPageIndex == 0xFFFF)
    {
        if (!IsNonVisiblePage(wPageHex))
        {
            return 0UL;
        }

        pSubPageList = m_NonVisiblePageList;
    }
    else
    {
        pSubPageList = m_VisiblePageList[wPageIndex];
    }

    DWORD dwNextPageCode = 0UL;
    TVTPage* pPage = NULL;

    EnterCriticalSection(&m_PageStoreMutex);

    if (pSubPageList != NULL)
    {
        pPage = FindNextSubPage(pSubPageList, dwPageCode, bReverse);

        if (pPage == NULL)
        {
            if (bReverse == FALSE)
            {
                pPage = FindSubPage(pSubPageList, MAKELONG(wPageHex, 0));
            }

            if (pPage == NULL)
            {
                pPage = FindNextSubPage(pSubPageList,
                    MAKELONG(wPageHex, bReverse == FALSE ? 0 : 0xFFFF), bReverse);
            }

            if (pPage != NULL)
            {
                if (pPage->dwPageCode == dwPageCode)
                {
                    pPage = NULL;
                }
            }
        }
    }

    if (pPage != NULL)
    {
        if (pBuffer != NULL)
        {
            CopyPageForDisplay(pBuffer, pPage);
            UnsetUpdatedStates(pPage);
        }
        dwNextPageCode = pPage->dwPageCode;
    }

    LeaveCriticalSection(&m_PageStoreMutex);

    return dwNextPageCode;
}


DWORD CVTDecoder::FindInDisplayPage(DWORD dwFromPageCode, BOOL bInclusive,
                                    LPSTR lpSearchString, TVTPage* pBuffer,
                                    BOOL bReverse)
{
    WORD wPageHex = LOWORD(dwFromPageCode);
    WORD wPageIndex = PageHex2ArrayIndex(wPageHex);

    if (wPageIndex == 0xFFFF)
    {
        if (bReverse != FALSE)
        {
            wPageIndex = 0;
        }
        else
        {
            wPageIndex = 799;
        }
    }

    EnterCriticalSection(&m_PageStoreMutex);

    char delta = (bReverse ? -1 : 1);
    WORD i = (800 + wPageIndex);
    BOOL bFound = FALSE;
    TVTPage* pPage;

    do
    {
        if (bInclusive != FALSE)
        {
            pPage = FindSubPage(m_VisiblePageList[i % 800], dwFromPageCode);

            if (pPage != NULL)
            {
                bFound = SearchPage(pPage, lpSearchString);

                if (bFound != FALSE)
                {
                    break;
                }
            }
        }

        do
        {
            pPage = FindNextSubPage(m_VisiblePageList[i % 800], dwFromPageCode, bReverse);

            if (pPage == NULL)
            {
                break;
            }

            dwFromPageCode = pPage->dwPageCode;

            bFound = SearchPage(pPage, lpSearchString);
        }
        while (bFound == FALSE);

        if (bFound != FALSE)
        {
            break;
        }

        // Find the next page to search in
        for (i += delta; (i % 800) != wPageIndex; i += delta)
        {
            pPage = FindReceivedPage(m_VisiblePageList[i % 800]);
            if (pPage != NULL)
            {
                wPageHex = LOWORD(pPage->dwPageCode);
                dwFromPageCode = MAKELONG(wPageHex, (bReverse != FALSE) ? 0x3F7F : 0);
                bInclusive = TRUE;
                break;
            }
        }
    }
    while ((i % 800) != wPageIndex);

    if (bFound != FALSE)
    {
        if (pBuffer != NULL)
        {
            CopyPageForDisplay(pBuffer, pPage);
            UnsetUpdatedStates(pPage);
        }
    }
    else
    {
        dwFromPageCode = 0UL;
    }

    LeaveCriticalSection(&m_PageStoreMutex);

    return dwFromPageCode;
}


BOOL CVTDecoder::GetDisplayComment(DWORD dwPageCode, TVTPage* pBuffer)
{
    m_bTopTextForComment = TRUE;

    if (m_pVTTopText->GetTopTextDetails(dwPageCode, pBuffer, TRUE))
    {
        return TRUE;
    }

    return FALSE;
}


BOOL CVTDecoder::SearchPage(TVTPage* pPage, LPSTR lpSearchString,
                            BOOL bIncludeRow25)
{
    WORD wFlags = PARSE_HASDATAONLY;

    if (bIncludeRow25 != FALSE)
    {
        wFlags |= PARSE_ALSOKEYWORDS;
    }

    LPVOID lpParam[2] = { lpSearchString, 0 };

    return ParsePageElements(pPage, &wFlags,
        (TParserCallback*)SearchPageProc, lpParam) == PARSE_STOPPAGE;
}


BYTE CVTDecoder::SearchPageProc(TVTPage*, WORD wPoint, LPWORD,
                               WORD, BYTE uChar, BYTE uMode, LPVOID lpParam)
{
    LPSTR lpSearchString    = (LPSTR)((LPVOID*)lpParam)[0];
    LPINT pIndex            = (LPINT)&((LPVOID*)lpParam)[1];

    BYTE nRow               = LOBYTE(wPoint);
    BYTE nCol               = HIBYTE(wPoint);

    // Don't search the header or row 24
    if (nRow == 0 || nRow == 24)
    {
        return PARSE_CONTINUE;
    }

    if (nCol == 0)
    {
        *pIndex = 0;
    }

    if ((uMode & VTMODE_GRAPHICS) != 0 && (uChar & 0x20))
    {
        uChar = 0x00;
    }

    if (toupper(uChar) == toupper(lpSearchString[*pIndex]))
    {
        // Check if the last character was matched
        if (lpSearchString[++*pIndex] == '\0')
        {
            return PARSE_STOPPAGE;
        }
    }
    else
    {
        BYTE nLength = strlen(&lpSearchString[*pIndex]);

        // If the string has repetitive substrings, it is
        // necessary check if the shifting the starting point
        // to the start of the substrings will find a match.
        for (int j = 1; j < *pIndex; j++)
        {
            if ((nLength + j) > (40 - nCol))
            {
                // These is not enough chars left in the
                // buffer to complete the match
                return PARSE_STOPLINE;
            }

            if (_strnicmp(lpSearchString, &lpSearchString[j], *pIndex - j) == 0)
            {
                // We found a possible substring, try to match
                // the failed character again.
                if (toupper(uChar) == toupper(lpSearchString[*pIndex - j]))
                {
                    *pIndex -= j;
                    if (lpSearchString[++*pIndex] == '\0')
                    {
                        return PARSE_STOPPAGE;
                    }
                    break;
                }
            }
        }

        // No luck finding a valid substring.  Start
        // the matching from the first char again.
        if (j >= *pIndex)
        {
            if ((nLength + *pIndex) > (40 - nCol))
            {
                // These is not enough chars left in the
                // buffer to complete the match
                return PARSE_STOPLINE;
            }

            if (*pIndex != 0)
            {
                // Start again
                *pIndex = 0;

                // We have restarted the matching, try to match
                // the failed character one last time.
                if (toupper(uChar) == toupper(lpSearchString[*pIndex]))
                {
                    *pIndex++;
                }
            }
        }
    }
    return PARSE_CONTINUE;
}


void CVTDecoder::CreateTestDisplayPage(TVTPage* pBuffer)
{
    pBuffer->dwPageCode = MAKELONG(0x900, 0x0000);
    pBuffer->wControlBits = VTCONTROL_INTERRUPTED;

    BYTE nCol;

    for (BYTE nRow = 0; nRow < 26; nRow++)
    {
        if (nRow == 0)
        {
            FillMemory(pBuffer->Frame[nRow], 8, 0x20);
            CopyMemory(&pBuffer->Frame[nRow][8], " DScaler Charset Test  \x03", 24);
            FillMemory(&pBuffer->Frame[nRow][32], 8, 0x20);
        }
        else if (nRow == 2)
        {
            CopyMemory(pBuffer->Frame[nRow], "\x17 \x1es\x13\x10\x16\x10\x1f\x18\x04\x0d\x1d\x03""ENGINEERING\x1a\x12\x1c\x0c\x1es\x15\x0e\x11\x10\x14\x10\x07", 38);

            pBuffer->Frame[nRow][38] = '0' + (nRow / 10);
            pBuffer->Frame[nRow][39] = '0' + (nRow % 10);
        }
        else if (nRow == 5)
        {
            CopyMemory(pBuffer->Frame[nRow], "\x14\x1a\x1es\x11\x19\x15\x00\x15\x01\x01\x15\x0d\x1d\x02Test Page  \x1c\x0c\x1e\x12s\x16\x18\x13\x00\x17\x18\x01", 38);

            pBuffer->Frame[nRow][38] = '0' + (nRow / 10);
            pBuffer->Frame[nRow][39] = '0' + (nRow % 10);
        }
        else if (nRow == 7)
        {
            CopyMemory(pBuffer->Frame[nRow], "\x01\x00\x01\x00\x01\x00\x01\x00\x01\x00\x17\x1e,\x13\x10\x16\x10\x12\x1e,\x15\x10\x11\x10\x14\x10\x1f\x00\x01\x00\x01\x00\x01\x00\x01\x00\x01\x00", 38);

            pBuffer->Frame[nRow][38] = '0' + (nRow / 10);
            pBuffer->Frame[nRow][39] = '0' + (nRow % 10);
        }
        else if (nRow <= 16)
        {
            for (nCol = 0; nCol < 38; nCol++)
            {
                if (nRow & 0x1)
                {
                    pBuffer->Frame[nRow][nCol] = (nCol & 0x1) ? 0x00 : 0x01;
                }
                else
                {
                    pBuffer->Frame[nRow][nCol] = (nCol & 0x1) ? 0x7f : 0x7e;
                }
            }

            pBuffer->Frame[nRow][38] = '0' + (nRow / 10);
            pBuffer->Frame[nRow][39] = '0' + (nRow % 10);
        }
        else if (nRow == 17)
        {
            CopyMemory(pBuffer->Frame[nRow], "White\x03""Yellow\x06""Cyan\x02""Green\x05""Magenta\x01""Red\x04""Blue", 40);
        }
        else if (nRow == 18)
        {
            for (nCol = 0; nCol < 40; nCol++)
            {
                if ((nCol % 5) == 0)
                {
                    switch (nCol)
                    {
                    case 0: pBuffer->Frame[nRow][nCol] = 0x17; break;
                    case 5: pBuffer->Frame[nRow][nCol] = 0x13; break;
                    case 10: pBuffer->Frame[nRow][nCol] = 0x16; break;
                    case 15: pBuffer->Frame[nRow][nCol] = 0x12; break;
                    case 20: pBuffer->Frame[nRow][nCol] = 0x19; break;
                    case 25: pBuffer->Frame[nRow][nCol] = 0x15; break;
                    case 30: pBuffer->Frame[nRow][nCol] = 0x11; break;
                    case 35: pBuffer->Frame[nRow][nCol] = 0x14; break;
                    }
                }
                else if (nCol == 1)
                {
                    pBuffer->Frame[nRow][nCol] = 0x1a;
                }
                else
                {
                    pBuffer->Frame[nRow][nCol] = 0x1F + nCol - (nCol / 5);
                }
            }
        }
        else if (nRow >= 19 && nRow <= 21)
        {
            for (nCol = 0; nCol < 40; nCol++)
            {
                if ((nCol % 5) == 0)
                {
                    pBuffer->Frame[nRow][nCol] = 0x20;
                }
                else
                {
                    pBuffer->Frame[nRow][nCol] = (0x1F | ((nRow - 19) << 5)) + nCol - (nCol / 5);
                }
            }
        }
        else if (nRow == 22)
        {
            for (nCol = 0; nCol < 40; nCol++)
            {
                if ((nCol % 5) == 0)
                {
                    switch (nCol)
                    {
                    case 0: pBuffer->Frame[nRow][nCol] = 0x14; break;
                    case 5: pBuffer->Frame[nRow][nCol] = 0x11; break;
                    case 10: pBuffer->Frame[nRow][nCol] = 0x15; break;
                    case 15: pBuffer->Frame[nRow][nCol] = 0x12; break;
                    case 20: pBuffer->Frame[nRow][nCol] = 0x1a; break;
                    case 25: pBuffer->Frame[nRow][nCol] = 0x16; break;
                    case 30: pBuffer->Frame[nRow][nCol] = 0x13; break;
                    case 35: pBuffer->Frame[nRow][nCol] = 0x17; break;
                    }
                }
                else
                {
                    pBuffer->Frame[nRow][nCol] = 0x5F + nCol - (nCol / 5);
                }
            }
        }
        else if (nRow == 23)
        {
            CopyMemory(pBuffer->Frame[nRow], "\x03\x18""Conceal\x08""Flash\x03\x2a\x0b\x0b""Box\x09""Steady\x18""Gone\x0a\x0a?\x16^\x7f", 40);
        }
        else if (nRow == 24 || nRow == 25)
        {
            FillMemory(pBuffer->Frame[nRow], 40, 0x20);
        }

        pBuffer->LineState[nRow] = CACHESTATE_HASDATA | CACHESTATE_UPDATED;
    }

    ZeroMemory(pBuffer->EditorialLink, sizeof(DWORD) * 6);

    pBuffer->bShowRow24 = FALSE;
    pBuffer->bBufferReserved = TRUE;
    pBuffer->bReceived = TRUE;
    pBuffer->pNextPage = NULL;
}


void CVTDecoder::GetStatusDisplay(LPSTR lpBuffer, LONG nLength)
{
    if (nLength > 21)
    {
        nLength = 21;
    }

    ASSERT(nLength > 0);

    lpBuffer[--nLength] = '\0';
	EnterCriticalSection(&m_ServiceDataStoreMutex);
    memcpy(lpBuffer, m_BroadcastServiceData.StatusDisplay, nLength);
	LeaveCriticalSection(&m_ServiceDataStoreMutex);
    CheckParity((BYTE*)lpBuffer, nLength, TRUE);

    while (nLength-- > 0 && lpBuffer[nLength] == 0x20)
    {
        lpBuffer[nLength] = '\0';
    }
}


WORD CVTDecoder::GetNetworkIDFromP8301()
{
	WORD wCode = 0;
	EnterCriticalSection(&m_ServiceDataStoreMutex);
	// Check that there are at leat two values received
	// and with same values
	if (   (m_BroadcastServiceData.NetworkIDCode[0] != 0)
		&& (m_BroadcastServiceData.NetworkIDCode[0] == m_BroadcastServiceData.NetworkIDCode[1]) )
	{
		wCode = m_BroadcastServiceData.NetworkIDCode[0];
	}
	LeaveCriticalSection(&m_ServiceDataStoreMutex);
	return wCode;
}


WORD CVTDecoder::GetCNIFromPDC()
{
	WORD wCode = 0;
	EnterCriticalSection(&m_ServiceDataStoreMutex);
	for (int i=0;i<4;i++)
	{
		if (m_PDC[i].CNI != 0)
		{
			wCode = m_PDC[i].CNI;
			break;
		}
	}
	LeaveCriticalSection(&m_ServiceDataStoreMutex);
	return wCode;
}


void CVTDecoder::SetCachingControl(BYTE uCachingControl)
{
    m_CachingControl = uCachingControl;
}


void CVTDecoder::SetHighGranularityCaching(BOOL bEnable)
{
    m_bHighGranularityCaching = bEnable;
}


void CVTDecoder::SetSubstituteSpacesForError(BOOL bEnable)
{
    m_bSubstituteSpacesForError = bEnable;
}


void CVTDecoder::NotifyDecoderEvent(BYTE uMsg, DWORD dwParam)
{
    if (m_DecoderEventProc != NULL)
    {
        (m_DecoderEventProc)(uMsg, dwParam);
    }
}


void CVTDecoder::CopyPageForDisplay(TVTPage* pBuffer, TVTPage* pPage)
{
    pBuffer->bShowRow24 = pPage->bShowRow24;
    pBuffer->dwPageCode = pPage->dwPageCode;
    pBuffer->wControlBits = pPage->wControlBits;

    for (int i = 0; i < 26; i++)
    {
        for (int j = 0; j < 40; j++)
        {
            if (CheckParity(&pPage->Frame[i][j]) == FALSE)
            {
                if (pPage->Frame[i][j] < 0x20)
                {
                    pPage->Frame[i][j] = 0x20;
                }
                else if (m_bSubstituteSpacesForError != FALSE)
                {
                    pBuffer->Frame[i][j] = 0x20;
                }
                else
                {
                    pBuffer->Frame[i][j] = pPage->Frame[i][j] & 0x7F;
                }
            }
            else
            {
                pBuffer->Frame[i][j] = pPage->Frame[i][j] & 0x7F;
            }
        }

        pBuffer->LineState[i] = pPage->LineState[i];
    }

    CopyMemory(pBuffer->EditorialLink, pPage->EditorialLink, sizeof(DWORD) * 6);

    if (pBuffer->bShowRow24 == FALSE)
    {
        m_bTopTextForComment = FALSE;
        m_pVTTopText->GetTopTextDetails(pBuffer->dwPageCode, pBuffer);
    }

    pBuffer->bBufferReserved = TRUE;
    pBuffer->bReceived = TRUE;
    pBuffer->pNextPage = NULL;
}


void CVTDecoder::UnsetUpdatedStates(TVTPage* pPage)
{
    for (int i(0); i < 26; i++)
    {
        pPage->LineState[i] &= ~CACHESTATE_UPDATED;
    }
}


TVTPage* CVTDecoder::FindReceivedPage(TVTPage* pPageList)
{
    if (pPageList == NULL || pPageList->bBufferReserved == FALSE)
    {
        return NULL;
    }

    if (pPageList->bReceived != FALSE)
    {
        return pPageList;
    }

    return FindReceivedPage(pPageList->pNextPage);
}


TVTPage* CVTDecoder::FindSubPage(TVTPage* pPageList, DWORD dwPageCode)
{
    TVTPage* pPage = pPageList;

    while (pPage != NULL && pPage->bBufferReserved != FALSE)
    {
        if (pPage->bReceived != FALSE &&
            pPage->dwPageCode == dwPageCode)
        {
            return pPage;
        }

        pPage = pPage->pNextPage;
    }

    return NULL;
}


TVTPage* CVTDecoder::FindNextSubPage(TVTPage* pPageList, DWORD dwPageCode,
                                     BOOL bReverse)
{
    TVTPage* pPage = pPageList;
    TVTPage* pNextPage = NULL;

    WORD wPageSubCode = HIWORD(dwPageCode);

    while (pPage != NULL && pPage->bBufferReserved != FALSE)
    {
        if (pPage->bReceived != FALSE &&
            LOWORD(pPage->dwPageCode) == LOWORD(dwPageCode))
        {
            if (bReverse == FALSE && HIWORD(pPage->dwPageCode) > wPageSubCode)
            {
                if (pNextPage == NULL ||
                    HIWORD(pPage->dwPageCode) < HIWORD(pNextPage->dwPageCode))
                {
                    pNextPage = pPage;
                }
            }
            else if (bReverse != FALSE && HIWORD(pPage->dwPageCode) < wPageSubCode)
            {
                if (pNextPage == NULL ||
                    HIWORD(pPage->dwPageCode) > HIWORD(pNextPage->dwPageCode))
                {
                    pNextPage = pPage;
                }
            }
        }

        pPage = pPage->pNextPage;
    }

    return pNextPage;
}

