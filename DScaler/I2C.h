/////////////////////////////////////////////////////////////////////////////
// $Id: I2C.h,v 1.3 2001-07-12 16:16:40 adcockj Exp $
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved i2c Functions in here
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __I2C_H___
#define __I2C_H___

#include "bt848.h"

BOOL I2CBus_AddDevice(BYTE I2C_Port);
BOOL I2CBus_Lock();
BOOL I2CBus_Unlock();
void I2CBus_Start();
void I2CBus_Stop();
void I2CBus_One();
void I2CBus_Zero();
BOOL I2CBus_Ack();
BOOL I2CBus_SendByte(BYTE nData, int nWaitForAck);
BYTE I2CBus_ReadByte(BOOL bLast);
BYTE I2CBus_Read(BYTE nAddr);
BOOL I2CBus_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth);
void I2CBus_wait(int us);
void I2C_SetLine(BOOL bCtrl, BOOL bData);
BOOL I2C_GetLine();
BYTE I2C_Read(BYTE nAddr);
BOOL I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth);

#define I2C_DELAY 0
#define I2C_TIMING (0x7<<4)
#define I2C_COMMAND (I2C_TIMING | BT848_I2C_SCL | BT848_I2C_SDA)

#define I2C_TSA5522        0xc2
#define I2C_TDA7432        0x8a
#define I2C_TDA8425        0x82
#define I2C_TDA9840        0x84
#define I2C_TDA9850        0xb6 /* also used by 9855,9873 */
#define I2C_TDA9875        0xb0
#define I2C_HAUPEE         0xa0
#define I2C_STBEE          0xae
#define I2C_VHX            0xc0
#define I2C_MSP3400        0x80
#define I2C_TEA6300        0x80
#define I2C_DPL3518    0x84


#endif
