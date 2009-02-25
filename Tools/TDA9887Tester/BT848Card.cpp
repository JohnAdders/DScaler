/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth. All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BT848Card.h"
#include "..\..\DScaler\Bt848_Defines.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CBT848Card::CBT848Card(CHardwareDriver* pDriver) : CPCICard(pDriver)
{
    m_I2CInitialized = false;
    m_I2CBus = new CI2CBusForLineInterface(this);
}

CBT848Card::~CBT848Card()
{
    delete m_I2CBus;
}

void CBT848Card::InitializeI2C()
{
    WriteDword(BT848_I2C, 1);
    m_I2CRegister = ReadDword(BT848_I2C);

    m_I2CSleepCycle = 10000L;
    DWORD elapsed = 0L;
    // get a stable reading
    while (elapsed < 5)
    {
        m_I2CSleepCycle *= 10;
        DWORD start = GetTickCount();
        for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
        elapsed = GetTickCount() - start;
    }
    // calculate how many cycles a 50kHZ is (half I2C bus cycle)
    m_I2CSleepCycle = m_I2CSleepCycle / elapsed * 1000L / 50000L;
	m_I2CInitialized = true;
}

void CBT848Card::Sleep()
{
    for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
}

void CBT848Card::SetSDA(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }

    if (value)
    {
        m_I2CRegister |= BT848_I2C_SDA;
    }

    else
    {
        m_I2CRegister &= ~BT848_I2C_SDA;
    }

    WriteDword(BT848_I2C, m_I2CRegister);
}

void CBT848Card::SetSCL(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }

    if (value)
    {
        m_I2CRegister |= BT848_I2C_SCL;
    }

    else
    {
        m_I2CRegister &= ~BT848_I2C_SCL;
    }

    WriteDword(BT848_I2C, m_I2CRegister);
}

bool CBT848Card::GetSDA()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }

    bool state = ReadDword(BT848_I2C) & BT848_I2C_SDA ? true : false;
    return state;
}

bool CBT848Card::GetSCL()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }

    bool state = ReadDword(BT848_I2C) & BT848_I2C_SCL ? true : false;
    return state;
}

bool CBT848Card::WriteToI2C(const BYTE *writeBuffer, size_t writeBufferSize)
{
	if(m_I2CBus->Write(writeBuffer, writeBufferSize))
	{
		return true;
	}

	return false;
}
