/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

/**
 * @file BT848Provider.cpp CBT848Provider Implementation
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Provider.h"
#include "BT848Source.h"

typedef struct
{
    WORD VendorId;
    WORD DeviceId;
    TCHAR* szName;
} TBT848Chip;

TBT848Chip BT848Chips[4] =
{
    {
        0x109e,
        0x0350,
        _T("BT848"),
    },
    {
        0x109e,
        0x0351,
        _T("BT849"),
    },
    {
        0x109e,
        0x036e,
        _T("BT878"),
    },
    {
        0x109e,
        0x036f,
        _T("BT878A"),
    },
};

CBT848Provider::CBT848Provider(SmartPtr<CHardwareDriver> pHardwareDriver)
{
    TCHAR szSection[12];
    DWORD SubSystemId;
    BOOL IsMemoryInitialized = FALSE;
    int i;

    for(i = 0; i < sizeof(BT848Chips)/sizeof(TBT848Chip); ++i)
    {
        int CardsFound(0);

        while(pHardwareDriver->DoesThisPCICardExist(
                                                        BT848Chips[i].VendorId,
                                                        BT848Chips[i].DeviceId,
                                                        CardsFound,
                                                        SubSystemId
                                                   ) == TRUE)
        {
            if(!IsMemoryInitialized)
            {
                // need to allocate memory for display, VBI and RISC code once
                if(MemoryInit(pHardwareDriver) == FALSE)
                {
                    return;
                }
                IsMemoryInitialized = TRUE;
            }

            _stprintf_s(szSection, 12, _T("%s%d"), BT848Chips[i].szName, CardsFound + 1);
            SmartPtr<CBT848Source> pNewSource = CreateCorrectSource(
                                                                pHardwareDriver,
                                                                szSection,
                                                                BT848Chips[i].VendorId,
                                                                BT848Chips[i].DeviceId,
                                                                CardsFound,
                                                                SubSystemId,
                                                                BT848Chips[i].szName
                                                          );
            if(pNewSource)
            {
                m_BT848Sources.push_back(pNewSource);
            }
            ++CardsFound;
        }
    }
}


SmartPtr<CBT848Source> CBT848Provider::CreateCorrectSource(SmartPtr<CHardwareDriver> pHardwareDriver, LPCTSTR szSection, WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD SubSystemId, TCHAR* ChipName)
{
    /// \todo use the subsystem id to create the correct specilized version of the card
    SmartPtr<CBT848Card> pNewCard = new CBT848Card(pHardwareDriver);
    SmartPtr<CBT848Source> pNewSource;
    if(pNewCard->OpenPCICard(VendorID, DeviceID, DeviceIndex))
    {
        pNewSource = new CBT848Source(pNewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection, ChipName, DeviceIndex);
    }
    return pNewSource;
}


int CBT848Provider::GetNumberOfSources()
{
    return m_BT848Sources.size();
}

SmartPtr<CSource> CBT848Provider::GetSource(int SourceIndex)
{
    if(SourceIndex >= 0 && SourceIndex < m_BT848Sources.size())
    {
        return m_BT848Sources[SourceIndex];
    }
    else
    {
        return NULL;
    }
}

BOOL CBT848Provider::MemoryInit(SmartPtr<CHardwareDriver> pHardwareDriver)
{
    try
    {
        m_RiscDMAMem = new CContigMemory(pHardwareDriver, 83968);
    }
    catch(...)
    {
        ErrorBox(_T("Can't create RISC Memory"));
        return FALSE;
    }

    try
    {
        for (int i(0); i < 5; i++)
        {
            m_DisplayDMAMem[i] = new CUserMemory(pHardwareDriver, 1024 * 768 * 2);
        }
    }
    catch(...)
    {
        ErrorBox(_T("Can't create Display Memory"));
        return FALSE;
    }

    try
    {
        for (int i(0); i < 5; i++)
        {
            m_VBIDMAMem[i] = new CUserMemory(pHardwareDriver, 2048 * 19 * 2);
        }
    }
    catch(...)
    {
        ErrorBox(_T("Can't create Display Memory"));
        return FALSE;
    }

    return TRUE;
}

#endif // WANT_BT8X8_SUPPORT