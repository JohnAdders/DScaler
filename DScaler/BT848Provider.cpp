/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Provider.cpp,v 1.4 2001-11-23 10:49:16 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.5  2001/08/19 14:43:47  adcockj
// Fixed memory leaks
//
// Revision 1.1.2.4  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.3  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.2  2001/08/16 06:43:34  adcockj
// moved more stuff into the new file (deonsn't compile)
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Provider.h"
#include "BT848Source.h"


CBT848Provider::CBT848Provider(CHardwareDriver* pHardwareDriver)
{
    char szSection[12];
    // need to allocate memory for display, VBI and RISC code
    if(MemoryInit(pHardwareDriver) == FALSE)
    {
        return;
    }

    int CardsFound(0);
    CBT848Card* NewCard = new CBT848Card(pHardwareDriver);

    while(NewCard->FindCard(CBT848Card::BT848, CardsFound) == TRUE)
    {
        sprintf(szSection, "BT848%d", CardsFound + 1);
        CBT848Source* pNewSource = new CBT848Source(NewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection);
        m_BT848Sources.push_back(pNewSource);
        NewCard = new CBT848Card(pHardwareDriver);
        ++CardsFound;
    }
    
    CardsFound = 0;
    while(NewCard->FindCard(CBT848Card::BT849, CardsFound) == TRUE)
    {
        sprintf(szSection, "BT849%d", CardsFound + 1);
        CBT848Source* pNewSource = new CBT848Source(NewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection);
        m_BT848Sources.push_back(pNewSource);
        NewCard = new CBT848Card(pHardwareDriver);
        ++CardsFound;
    }

    CardsFound = 0;
    while(NewCard->FindCard(CBT848Card::BT878, CardsFound) == TRUE)
    {
        sprintf(szSection, "BT878%d", CardsFound + 1);
        CBT848Source* pNewSource = new CBT848Source(NewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection);
        m_BT848Sources.push_back(pNewSource);
        NewCard = new CBT848Card(pHardwareDriver);
        ++CardsFound;
    }

    CardsFound = 0;
    while(NewCard->FindCard(CBT848Card::BT878A, CardsFound) == TRUE)
    {
        sprintf(szSection, "BT878A%d", CardsFound + 1);
        CBT848Source* pNewSource = new CBT848Source(NewCard, m_RiscDMAMem, m_DisplayDMAMem, m_VBIDMAMem, szSection);
        m_BT848Sources.push_back(pNewSource);
        NewCard = new CBT848Card(pHardwareDriver);
        ++CardsFound;
    }

    delete NewCard;
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
    // initilize pointers to NULL
    // so that if we fail we can check 
    // for NULL before destroying
    m_RiscDMAMem = NULL;
    for(int i(0); i < 5; ++i)
    {
        m_VBIDMAMem[i] = NULL;
        m_DisplayDMAMem[i] = NULL;
    }

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
        for (i = 0; i < 5; i++)
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
        for (i = 0; i < 5; i++)
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
