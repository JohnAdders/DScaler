/////////////////////////////////////////////////////////////////////////////
// i2c.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved I2C Functions in here
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "i2c.h"

CRITICAL_SECTION cCritSect;

void I2C_SetLine(BOOL bCtrl, BOOL bData)
{
	BT848_WriteDword(BT848_I2C, (bCtrl << 1) | bData);
	I2CBus_wait(I2C_DELAY);
}

BOOL I2C_GetLine()
{
	return BT848_ReadDword(BT848_I2C) & 1;
}

BYTE I2C_Read(BYTE nAddr)
{
	DWORD i;
	volatile DWORD stat;

	BT848_WriteDword(BT848_INT_STAT, BT848_INT_I2CDONE);
	BT848_WriteDword(BT848_I2C, (nAddr << 24) | I2C_COMMAND);

	for (i = 0x7fffffff; i; i--)
	{
		stat = BT848_ReadDword(BT848_INT_STAT);
		if (stat & BT848_INT_I2CDONE)
			break;
	}

	if (!i)
		return (BYTE) - 1;
	if (!(stat & BT848_INT_RACK))
		return (BYTE) - 2;

	return (BYTE) ((BT848_ReadDword(BT848_I2C) >> 8) & 0xFF);
}

BOOL I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth)
{
	DWORD i;
	DWORD data;
	DWORD stat;

	BT848_WriteDword(BT848_INT_STAT, BT848_INT_I2CDONE);

	data = (nAddr << 24) | (nData1 << 16) | I2C_COMMAND;
	if (bSendBoth)
		data |= (nData2 << 8) | BT848_I2C_W3B;
	BT848_WriteDword(BT848_I2C, data);

	for (i = 0x7fffffff; i; i--)
	{
		stat = BT848_ReadDword(BT848_INT_STAT);
		if (stat & BT848_INT_I2CDONE)
			break;
	}

	if (!i)
		return FALSE;
	if (!(stat & BT848_INT_RACK))
		return FALSE;

	return TRUE;
}

//----------------------------------------------------------------

BOOL I2CBus_AddDevice(BYTE I2C_Port)
{
	BOOL bAck;

	// Test whether device exists
	I2CBus_Lock();
	I2CBus_Start();
	bAck = I2CBus_SendByte(I2C_Port, 0);
	I2CBus_Stop();
	I2CBus_Unlock();
	if (bAck)
		return TRUE;
	else
		return FALSE;
}

BOOL I2CBus_Lock()
{
	InitializeCriticalSection(&cCritSect);
	EnterCriticalSection(&cCritSect);
	return TRUE;
}

BOOL I2CBus_Unlock()
{
	LeaveCriticalSection(&cCritSect);
	DeleteCriticalSection(&cCritSect);
	return TRUE;
}

void I2CBus_Start()
{
	I2C_SetLine(0, 1);
	I2C_SetLine(1, 1);
	I2C_SetLine(1, 0);
	I2C_SetLine(0, 0);
}

void I2CBus_Stop()
{
	I2C_SetLine(0, 0);
	I2C_SetLine(1, 0);
	I2C_SetLine(1, 1);
}

void I2CBus_One()
{
	I2C_SetLine(0, 1);
	I2C_SetLine(1, 1);
	I2C_SetLine(0, 1);
}

void I2CBus_Zero()
{
	I2C_SetLine(0, 0);
	I2C_SetLine(1, 0);
	I2C_SetLine(0, 0);
}

BOOL I2CBus_Ack()
{
	BOOL bAck;

	I2C_SetLine(0, 1);
	I2C_SetLine(1, 1);
	bAck = !I2C_GetLine();
	I2C_SetLine(0, 1);
	return bAck;
}

BOOL I2CBus_SendByte(BYTE nData, int nWaitForAck)
{
	I2C_SetLine(0, 0);
	nData & 0x80 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x40 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x20 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x10 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x08 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x04 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x02 ? I2CBus_One() : I2CBus_Zero();
	nData & 0x01 ? I2CBus_One() : I2CBus_Zero();
	if (nWaitForAck)
		I2CBus_wait(nWaitForAck);
	return I2CBus_Ack();
}

BYTE I2CBus_ReadByte(BOOL bLast)
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

	bLast ? I2CBus_One() : I2CBus_Zero();
	return bData;
}

BYTE I2CBus_Read(BYTE nAddr)
{
	BYTE bData;

	I2CBus_Start();
	I2CBus_SendByte(nAddr, 0);
	bData = I2CBus_ReadByte(TRUE);
	I2CBus_Stop();
	return bData;
}

BOOL I2CBus_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth)
{
	BOOL bAck;

	I2CBus_Start();
	I2CBus_SendByte(nAddr, 0);
	bAck = I2CBus_SendByte(nData1, 0);
	if (bSendBoth)
		bAck = I2CBus_SendByte(nData2, 0);
	I2CBus_Stop();
	return bAck;
}

void I2CBus_wait(int us)
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
