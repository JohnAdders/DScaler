/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xProvider.cpp,v 1.1 2002-09-11 18:19:37 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CT2388xProvider.h"
#include "CT2388xSource.h"

CCT2388xProvider::CCT2388xProvider(CHardwareDriver* pHardwareDriver)
{
    char szSection[12];
    BOOL IsMemoryInitialized = FALSE;
    int i;
    DWORD SubSystemId;
    int CardsFound(0);

    // initilize memory pointers to NULL
    // so that if we fail we can check 
    // for NULL before destroying
    m_RiscDMAMem = NULL;
    for(i = 0; i < 5; ++i)
    {
        m_DisplayDMAMem[i] = NULL;
    }


    while(pHardwareDriver->DoesThisPCICardExist( 
                                                    0x14F1, 
                                                    0x8800, 
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

        sprintf(szSection, "%s%d", "CT23880", CardsFound + 1);
        CCT2388xSource* pNewSource = CreateCorrectSource(
                                                            pHardwareDriver,
                                                            szSection, 
                                                            0x14F1, 
                                                            0x8800, 
                                                            CardsFound, 
                                                            SubSystemId
                                                      );
        if(pNewSource != NULL)
        {
            m_Sources.push_back(pNewSource);
        }
        ++CardsFound;
    }
}

CCT2388xProvider::~CCT2388xProvider()
{
    MemoryFree();
    for(vector<CCT2388xSource*>::iterator it = m_Sources.begin();
        it != m_Sources.end();
        ++it)
    {
        delete *it;
    }
}


CCT2388xSource* CCT2388xProvider::CreateCorrectSource(CHardwareDriver* pHardwareDriver, LPCSTR szSection, WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD SubSystemId)
{
    // \todo use the subsystem id to create the correct specilized version of the card
    CCT2388xCard* pNewCard = new CCT2388xCard(pHardwareDriver);
    if(pNewCard->OpenPCICard(VendorID, DeviceID, DeviceIndex))
    {
		// \todo remove this as it's just for testing
		pNewCard->DumpChipStatus();
        CCT2388xSource* pNewSource = new CCT2388xSource(pNewCard, m_RiscDMAMem, m_DisplayDMAMem, szSection);
        return pNewSource;
    }
    else
    {
        delete pNewCard;
        return NULL;
    }
}


int CCT2388xProvider::GetNumberOfSources()
{
    return m_Sources.size();
}

CSource* CCT2388xProvider::GetSource(int SourceIndex)
{
    if(SourceIndex >= 0 && SourceIndex < m_Sources.size())
    {
        return m_Sources[SourceIndex];
    }
    else
    {
        return NULL;
    }
}

BOOL CCT2388xProvider::MemoryInit(CHardwareDriver* pHardwareDriver)
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
            m_DisplayDMAMem[i] = new CUserMemory(pHardwareDriver, 1024 * 576 * 2);
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

void CCT2388xProvider::MemoryFree()
{
    if(m_RiscDMAMem != NULL)
    {
        delete m_RiscDMAMem;
        m_RiscDMAMem = NULL;
    }

    for(int i(0); i < 5; i++)
    {
        if(m_DisplayDMAMem[i] != NULL)
        {
            delete m_DisplayDMAMem[i];
            m_DisplayDMAMem[i] = NULL;
        }
    }
}
