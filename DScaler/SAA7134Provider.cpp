/////////////////////////////////////////////////////////////////////////////
// $id$
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
//
// This software was based on BT848Provider.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $log$
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Provider.h"
#include "SAA7134Source.h"

typedef struct
{
    DWORD VendorId;
    DWORD DeviceId;
    char* szName;
} TSAA7134Chip;

TSAA7134Chip SAA7134Chips[4] = 
{
    {
        0x1131,
        0x7134,
        "BT848",
    }

};

CSAA7134Provider::CSAA7134Provider(CHardwareDriver* pHardwareDriver)
{
    char szSection[12];
    DWORD SubSystemId;
    BOOL IsMemoryInitialized = FALSE;
    int i;

    for(i = 0; i < 2; ++i)
    {
        m_VBIDMAMem[i] = NULL;
        m_DisplayDMAMem[i] = NULL;
    }

    for(i = 0; i < sizeof(SAA7134Chips)/sizeof(TSAA7134Chip); ++i)
    {
        int CardsFound(0);

        while(pHardwareDriver->DoesThisPCICardExist( 
                                                        SAA7134Chips[i].VendorId, 
                                                        SAA7134Chips[i].DeviceId, 
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

            sprintf(szSection, "%s%d", SAA7134Chips[i].szName, CardsFound + 1);
            CSAA7134Source* pNewSource = CreateCorrectSource(
                                                                pHardwareDriver,
                                                                szSection, 
                                                                SAA7134Chips[i].VendorId, 
                                                                SAA7134Chips[i].DeviceId, 
                                                                CardsFound, 
                                                                SubSystemId,
                                                                SAA7134Chips[i].szName
                                                          );
            if(pNewSource != NULL)
            {
                m_SAA7134Sources.push_back(pNewSource);
            }
            ++CardsFound;
        }
    }
}

CSAA7134Provider::~CSAA7134Provider()
{
    MemoryFree();
    for(vector<CSAA7134Source*>::iterator it = m_SAA7134Sources.begin();
        it != m_SAA7134Sources.end();
        ++it)
    {
        delete *it;
    }
}


CSAA7134Source* CSAA7134Provider::CreateCorrectSource(CHardwareDriver* pHardwareDriver, LPCSTR szSection, WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD SubSystemId, char* ChipName)
{
    /// \todo use the subsystem id to create the correct specilized version of the card
    CSAA7134Card* pNewCard = new CSAA7134Card(pHardwareDriver);
    if(pNewCard->OpenPCICard(VendorID, DeviceID, DeviceIndex))
    {
        CSAA7134Source* pNewSource = new CSAA7134Source(pNewCard, m_PagelistDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection, ChipName, DeviceIndex);
        return pNewSource;
    }
    else
    {
        delete pNewCard;
        return NULL;
    }
}


int CSAA7134Provider::GetNumberOfSources()
{
    return m_SAA7134Sources.size();
}

CSource* CSAA7134Provider::GetSource(int SourceIndex)
{
    if(SourceIndex >= 0 && SourceIndex < m_SAA7134Sources.size())
    {
        return m_SAA7134Sources[SourceIndex];
    }
    else
    {
        return NULL;
    }
}

BOOL CSAA7134Provider::MemoryInit(CHardwareDriver* pHardwareDriver)
{
    try
    {
        for (int i(0); i < 4; i++)
        {
            m_PagelistDMAMem[i] = new CContigMemory(pHardwareDriver, 4096);
        }
    }
    catch(...)
    {
        MemoryFree();
        ErrorBox("Can't create PageTable Memory");
        return FALSE;
    }

    try
    {
        for (int i(0); i < 2; i++)
        {
            m_DisplayDMAMem[i] = new CUserMemory(pHardwareDriver, 1024 * 576 * 2);
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
        for (int i(0); i < 2; i++)
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

void CSAA7134Provider::MemoryFree()
{
    for(int i(0); i < 2; i++)
    {
        if(m_PagelistDMAMem[i] != NULL)
        {
            delete m_PagelistDMAMem[i];
            m_PagelistDMAMem[i] = NULL;
        }

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
