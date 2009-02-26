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
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on BT848Provider.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file SAA7134Provider.cpp CSAA7134Provider Implementation
 */

#include "stdafx.h"

#ifdef WANT_SAA713X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Provider.h"
#include "SAA7134Source.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

typedef struct
{
    WORD VendorId;
    WORD DeviceId;
    char* szName;
} TSAA7134Chip;

TSAA7134Chip SAA7134Chips[] =
{
    {
        0x1131,
        0x7130,
        "SAA7130",
    },
    {
        0x1131,
        0x7133,
        "SAA7133",
    },
    {
        0x1131,
        0x7134,
        "SAA7134",
    }
};

CSAA7134Provider::CSAA7134Provider(CHardwareDriver* pHardwareDriver)
{
    char szSection[12];
    DWORD SubSystemId;
    BOOL IsMemoryInitialized = FALSE;
    int i;

    for (i = 0; i < kMAX_PAGETABLES; i++)
    {
        m_PageTableDMAMem[i] = NULL;
    }

    for(i = 0; i < kMAX_FRAMEBUFFERS; ++i)
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
        CSAA7134Source* pNewSource = new CSAA7134Source(pNewCard, m_PageTableDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection, ChipName, DeviceIndex);
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
    if (SourceIndex >= 0 && SourceIndex < (int)m_SAA7134Sources.size())
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
        for (int i(0); i < kMAX_PAGETABLES; i++)
        {
            m_PageTableDMAMem[i] = new CContigMemory(pHardwareDriver, 4096);
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
        for (int i(0); i < kMAX_FRAMEBUFFERS; i++)
        {
            // 1024 width * 2 bytes per pixel * field height(288) * 2
            m_DisplayDMAMem[i] = new CUserMemory(pHardwareDriver,
                1024 * 2 * kMAX_VIDLINES  * 2);
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
        for (int i(0); i < kMAX_FRAMEBUFFERS; i++)
        {
            m_VBIDMAMem[i] = new CUserMemory(pHardwareDriver, 2048 * kMAX_VBILINES * 2);
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
    int i;

    for (i = 0; i < kMAX_PAGETABLES; i++)
    {
        if(m_PageTableDMAMem[i] != NULL)
        {
            delete m_PageTableDMAMem[i];
            m_PageTableDMAMem[i] = NULL;
        }
    }

    for (i = 0; i < kMAX_FRAMEBUFFERS; i++)
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

#endif//xxx