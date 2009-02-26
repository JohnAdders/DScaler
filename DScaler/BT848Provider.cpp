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
    char* szName;
} TBT848Chip;

TBT848Chip BT848Chips[4] = 
{
    {
        0x109e,
        0x0350,
        "BT848",
    },
    {
        0x109e,
        0x0351,
        "BT849",
    },
    {
        0x109e,
        0x036e,
        "BT878",
    },
    {
        0x109e,
        0x036f,
        "BT878A",
    },
};

CBT848Provider::CBT848Provider(CHardwareDriver* pHardwareDriver)
{
    char szSection[12];
    DWORD SubSystemId;
    BOOL IsMemoryInitialized = FALSE;
    int i;

    // initilize memory pointers to NULL
    // so that if we fail we can check 
    // for NULL before destroying
    m_RiscDMAMem = NULL;
    for(i = 0; i < 5; ++i)
    {
        m_VBIDMAMem[i] = NULL;
        m_DisplayDMAMem[i] = NULL;
    }

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

            sprintf(szSection, "%s%d", BT848Chips[i].szName, CardsFound + 1);
            CBT848Source* pNewSource = CreateCorrectSource(
                                                                pHardwareDriver,
                                                                szSection, 
                                                                BT848Chips[i].VendorId, 
                                                                BT848Chips[i].DeviceId, 
                                                                CardsFound, 
                                                                SubSystemId,
                                                                BT848Chips[i].szName
                                                          );
            if(pNewSource != NULL)
            {
                m_BT848Sources.push_back(pNewSource);
            }
            ++CardsFound;
        }
    }
}

CBT848Provider::~CBT848Provider()
{
    MemoryFree();
    for(vector<CBT848Source*>::iterator it = m_BT848Sources.begin();
        it != m_BT848Sources.end();
        ++it)
    {
        delete *it;
    }
}


CBT848Source* CBT848Provider::CreateCorrectSource(CHardwareDriver* pHardwareDriver, LPCSTR szSection, WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD SubSystemId, char* ChipName)
{
    /// \todo use the subsystem id to create the correct specilized version of the card
    CBT848Card* pNewCard = new CBT848Card(pHardwareDriver);
    if(pNewCard->OpenPCICard(VendorID, DeviceID, DeviceIndex))
    {
        CBT848Source* pNewSource = new CBT848Source(pNewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection, ChipName, DeviceIndex);
        return pNewSource;
    }
    else
    {
        delete pNewCard;
        return NULL;
    }
}


int CBT848Provider::GetNumberOfSources()
{
    return m_BT848Sources.size();
}

CSource* CBT848Provider::GetSource(int SourceIndex)
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

BOOL CBT848Provider::MemoryInit(CHardwareDriver* pHardwareDriver)
{
    try
    {
        m_RiscDMAMem = new CContigMemory(pHardwareDriver, 83968);
    }
    catch(...)
    {
        MemoryFree();
        ErrorBox("Can't create RISC Memory");
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
        MemoryFree();
        ErrorBox("Can't create Display Memory");
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
        MemoryFree();
        ErrorBox("Can't create Display Memory");
        return FALSE;
    }

    return TRUE;
}

void CBT848Provider::MemoryFree()
{
    if(m_RiscDMAMem != NULL)
    {
        delete m_RiscDMAMem;
        m_RiscDMAMem = NULL;
    }

    for(int i(0); i < 5; i++)
    {
        if(m_VBIDMAMem[i] != NULL)
        {
            delete m_VBIDMAMem[i];
            m_VBIDMAMem[i] = NULL;
        }
        if(m_DisplayDMAMem[i] != NULL)
        {
            delete m_DisplayDMAMem[i];
            m_DisplayDMAMem[i] = NULL;
        }
    }
}

#endif // WANT_BT8X8_SUPPORT