/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xProvider.cpp,v 1.4 2003-10-27 10:39:51 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2002/11/02 09:47:36  adcockj
// Removed test code commited by accident
//
// Revision 1.2  2002/11/02 09:30:08  adcockj
// Fixed SRAM overflow
//
// Revision 1.1  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// 
// CVS Log while file was called CT2388xProvider.cpp
//
// Revision 1.3  2002/10/27 19:17:25  adcockj
// Fixes for cx2388x - PAL & NTSC tested
//
// Revision 1.2  2002/10/23 15:18:07  adcockj
// Added preliminary code for VBI
//
// Revision 1.1  2002/09/11 18:19:37  adcockj
// Prelimainary support for CX2388x based cards
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xProvider.cpp CCX2388xProvider Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xProvider.h"
#include "CX2388xSource.h"

CCX2388xProvider::CCX2388xProvider(CHardwareDriver* pHardwareDriver)
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
        m_VBIDMAMem[i] = NULL;
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

        sprintf(szSection, "%s%d", "CX23880", CardsFound + 1);
        CCX2388xSource* pNewSource = CreateCorrectSource(
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

CCX2388xProvider::~CCX2388xProvider()
{
    MemoryFree();
    for(vector<CCX2388xSource*>::iterator it = m_Sources.begin();
        it != m_Sources.end();
        ++it)
    {
        delete *it;
    }
}


CCX2388xSource* CCX2388xProvider::CreateCorrectSource(CHardwareDriver* pHardwareDriver, LPCSTR szSection, WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD SubSystemId)
{
    // \todo use the subsystem id to create the correct specilized version of the card
    CCX2388xCard* pNewCard = new CCX2388xCard(pHardwareDriver);
    if(pNewCard->OpenPCICard(VendorID, DeviceID, DeviceIndex))
    {
		// \todo remove this as it's just for testing
		pNewCard->DumpChipStatus(szSection);
        CCX2388xSource* pNewSource = new CCX2388xSource(pNewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection);
        return pNewSource;
    }
    else
    {
        delete pNewCard;
        return NULL;
    }
}


int CCX2388xProvider::GetNumberOfSources()
{
    return m_Sources.size();
}

CSource* CCX2388xProvider::GetSource(int SourceIndex)
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

void CCX2388xProvider::MemoryFree()
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
