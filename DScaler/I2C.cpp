/////////////////////////////////////////////////////////////////////////////
// $Id: I2C.cpp,v 1.5 2001-11-02 16:30:08 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved I2C Functions in here
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.4  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.3  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "i2c.h"


CI2C::CI2C()
{
    InitializeCriticalSection(&m_hCritSect);
}

CI2C::~CI2C()
{
    DeleteCriticalSection(&m_hCritSect);
}

//----------------------------------------------------------------
BOOL CI2C::I2C_AddDevice(BYTE I2C_Port)
{
    BOOL bAck;

    // Test whether device exists
    EnterCriticalSection(&m_hCritSect);
    I2C_Start();
    bAck = I2C_SendByte(I2C_Port, 0);
    I2C_Stop();
    LeaveCriticalSection(&m_hCritSect);
    if (bAck)
        return TRUE;
    else
        return FALSE;
}

void CI2C::I2C_Start()
{
    I2C_SetLine(0, 1);
    I2C_SetLine(1, 1);
    I2C_SetLine(1, 0);
    I2C_SetLine(0, 0);
}

void CI2C::I2C_Stop()
{
    I2C_SetLine(0, 0);
    I2C_SetLine(1, 0);
    I2C_SetLine(1, 1);
}

void CI2C::I2C_One()
{
    I2C_SetLine(0, 1);
    I2C_SetLine(1, 1);
    I2C_SetLine(0, 1);
}

void CI2C::I2C_Zero()
{
    I2C_SetLine(0, 0);
    I2C_SetLine(1, 0);
    I2C_SetLine(0, 0);
}

BOOL CI2C::I2C_Ack()
{
    BOOL bAck;

    I2C_SetLine(0, 1);
    I2C_SetLine(1, 1);
    bAck = !I2C_GetLine();
    I2C_SetLine(0, 1);
    return bAck;
}

BOOL CI2C::I2C_SendByte(BYTE nData, int nWaitForAck)
{
    I2C_SetLine(0, 0);
    nData & 0x80 ? I2C_One() : I2C_Zero();
    nData & 0x40 ? I2C_One() : I2C_Zero();
    nData & 0x20 ? I2C_One() : I2C_Zero();
    nData & 0x10 ? I2C_One() : I2C_Zero();
    nData & 0x08 ? I2C_One() : I2C_Zero();
    nData & 0x04 ? I2C_One() : I2C_Zero();
    nData & 0x02 ? I2C_One() : I2C_Zero();
    nData & 0x01 ? I2C_One() : I2C_Zero();
    if (nWaitForAck)
        I2C_Wait(nWaitForAck);
    return I2C_Ack();
}

BYTE CI2C::I2C_ReadByte(BOOL bLast)
{
    int i;
    BYTE bData = 0;

    I2C_SetLine(0, 1);
    for (i = 7; i >= 0; i--)
    {
        I2C_SetLine(1, 1);
        if (I2C_GetLine())
            bData |= (1 << i);
        I2C_SetLine(0, 1);
    }

    bLast ? I2C_One() : I2C_Zero();
    return bData;
}

BYTE CI2C::I2C_Read(BYTE nAddr)
{
    BYTE bData;

    I2C_Start();
    I2C_SendByte(nAddr, 0);
    bData = I2C_ReadByte(TRUE);
    I2C_Stop();
    return bData;
}

BOOL CI2C::I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth)
{
    BOOL bAck;

    I2C_Start();
    I2C_SendByte(nAddr, 0);
    bAck = I2C_SendByte(nData1, 0);
    if (bSendBoth)
        bAck = I2C_SendByte(nData2, 0);
    I2C_Stop();
    return bAck;
}

void CI2C::I2C_Wait(int us)
{
    if (us > 0)
    {
        Sleep(us);
        return;
    }
    Sleep(0);
    Sleep(0);
    Sleep(0);
    Sleep(0);
    Sleep(0);
}

void CI2C::I2C_Lock()
{
    EnterCriticalSection(&m_hCritSect);
}

void CI2C::I2C_Unlock()
{
    LeaveCriticalSection(&m_hCritSect);
}
