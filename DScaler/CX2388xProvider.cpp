/////////////////////////////////////////////////////////////////////////////
// $Id$
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xProvider.cpp CCX2388xProvider Implementation
 */

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xProvider.h"
#include "CX2388xSource.h"

CCX2388xProvider::CCX2388xProvider(CHardwareDriver* pHardwareDriver)
{
    char szSection[12];
    BOOL IsMemoryInitialized = FALSE;
    DWORD SubSystemId;
    int CardsFound(0);

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

        sprintf(szSection, "%s%d", "CX23880", CardsFound + 1);
        SmartPtr<CCX2388xSource> pNewSource = CreateCorrectSource(
                                                            pHardwareDriver,
                                                            szSection, 
                                                            0x14F1, 
                                                            0x8800, 
                                                            CardsFound, 
                                                            SubSystemId
                                                                      );
        if(pNewSource)
        {
            m_Sources.push_back(pNewSource);
            ++CardsFound;
        }
    }
}

CCX2388xProvider::~CCX2388xProvider()
{
}


SmartPtr<CCX2388xSource> CCX2388xProvider::CreateCorrectSource(CHardwareDriver* pHardwareDriver, LPCSTR szSection, WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD SubSystemId)
{
    // \todo use the subsystem id to create the correct specilized version of the card
    SmartPtr<CCX2388xCard> pNewCard = new CCX2388xCard(pHardwareDriver);
    SmartPtr<CCX2388xSource> pNewSource;
    if(pNewCard->OpenPCICard(VendorID, DeviceID, DeviceIndex))
    {
        // \todo remove this as it's just for testing
        pNewCard->DumpChipStatus(szSection);
        pNewSource = new CCX2388xSource(pNewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection);
    }
    return pNewSource;
}


int CCX2388xProvider::GetNumberOfSources()
{
    return m_Sources.size();
}

SmartPtr<CSource> CCX2388xProvider::GetSource(int SourceIndex)
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

BOOL CCX2388xProvider::MemoryInit(CHardwareDriver* pHardwareDriver)
{
    try
    {
        m_RiscDMAMem = new CContigMemory(pHardwareDriver, 83968);
    }
    catch(...)
    { 
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
        ErrorBox("Can't create Display Memory");
        return FALSE;
    }

    return TRUE;
}

#endif // WANT_CX2388X_SUPPORT
