/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9875.h,v 1.1 2004-01-05 13:25:25 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 John Adcock.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TDA9875.h TDA9875 Header
 */


#if !defined(__TDA9875_H__)
#define __TDA9875_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "I2CDevice.h"

#define TDA9875_STA         0x00  /*Status register*/
#define TDA9875_MUT         0x12  /*General mute*/
#define TDA9875_CFG         0x01  /*Config register*/
#define TDA9875_DACOS       0x13  /*DAC i/o select (ADC)*/
#define TDA9875_LOSR        0x16  /*Line output select register*/

#define TDA9875_CH1V        0x0c  /*Channel 1 volume*/
#define TDA9875_CH2V        0x0d  /*Channel 2 volume*/
#define TDA9875_NCHV		0x0f  /*NICAM volume*/
#define TDA9875_SC1         0x14  /*SCART 1 in (mono)*/
#define TDA9875_SC2         0x15  /*SCART 2 in (mono)*/

#define TDA9875_ADCIS       0x17  /*ADC input select (mono)*/
#define TDA9875_AER         0x19  /*Audio effect (AVL+Pseudo)*/
#define TDA9875_MCS         0x18  /*Main channel select (DAC)*/
#define TDA9875_MVL         0x1a  /*Main volume gauche */
#define TDA9875_MVR         0x1b  /*Main volume droite */
#define TDA9875_LOU         0x1c  /*Loudness*/
#define TDA9875_MBA         0x1d  /*Main Basse*/
#define TDA9875_MTR         0x1e  /*Main treble*/
#define TDA9875_ACS         0x1f  /*Auxilary channel select (FM)*/
#define TDA9875_AVL         0x20  /*Auxilary volume gauche*/
#define TDA9875_AVR         0x21  /*Auxilary volume droite*/
#define TDA9875_ABA         0x22  /*Auxilary Basse*/
#define TDA9875_ATR         0x23  /*Auxilary treble*/
#define TDA9875_FIC         0x24  /*Feature Iinterface Configuration*/
#define TDA9875_BBO         0x2D  /*Bass Boost*/

#define TDA9875_MSR         0x02  /*Monitor select register*/
#define TDA9875_C1MSB       0x03  /*Carrier 1 (FM) frequency register MSB*/
#define TDA9875_C1MIB       0x04  /*Carrier 1 (FM) frequency register (16-8]b*/
#define TDA9875_C1LSB       0x05  /*Carrier 1 (FM) frequency register LSB*/
#define TDA9875_C2MSB       0x06  /*Carrier 2 (nicam) frequency register MSB*/
#define TDA9875_C2MIB       0x07  /*Carrier 2 (nicam) frequency register (16-8]b*/
#define TDA9875_C2LSB       0x08  /*Carrier 2 (nicam) frequency register LSB*/
#define TDA9875_DCR         0x09  /*Demodulateur configuration regirter*/
#define TDA9875_DEEM        0x0a  /*FM de-emphasis regirter*/
#define TDA9875_FMAT        0x0b  /*FM Matrix regirter*/

/* values */
#define TDA9875_MUTE_ON	    0xff /*general mute*/
#define TDA9875_MUTE_OFF    0xcc /*general no mute*/

#define TDA9875_L1MSB       0x05  /*LEVEL READ-OUT REGISTERS MSB*/
#define TDA9875_L2LSB       0x06  /*LEVEL READ-OUT REGISTERS LSB*/

class CTDA9875 : public CI2CDevice
{
public:

    CTDA9875();
	virtual ~CTDA9875() {};
    void Reset();

	bool IsDevicePresent(int& dic, int& rev);

protected:

	void WriteToSubAddress(BYTE subAddress, BYTE value);

    virtual BYTE GetDefaultAddress() const;
};

#endif // !defined(__TDA9875_H__)
