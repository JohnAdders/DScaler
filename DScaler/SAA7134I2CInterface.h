/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134I2CInterface.h,v 1.2 2002-09-09 14:25:16 atnak Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134I2CINTERFACE_H__
#define __SAA7134I2CINTERFACE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/** Interface for a SAA7134 I2C bus
*/
class ISAA7134I2CInterface  
{
public:

	enum eSAA7134I2CStatus {
		IDLE          = 0x0,  // no I2C command pending
		DONE_STOP     = 0x1,  // I2C command done and STOP executed
		BUSY          = 0x2,  // executing I2C command
		TO_SCL        = 0x3,  // executing I2C command, time out on clock stretching
		TO_ARB        = 0x4,  // time out on arbitration trial, still trying
		DONE_WRITE    = 0x5,  // I2C command done and awaiting next write command
		DONE_READ     = 0x6,  // I2C command done and awaiting next read command
		DONE_WRITE_TO = 0x7,  // see 5, and time out on status echo
		DONE_READ_TO  = 0x8,  // see 6, and time out on status echo
		NO_DEVICE     = 0x9,  // no acknowledge on device slave address
		NO_ACKN       = 0xA,  // no acknowledge after data byte transfer
		BUS_ERR       = 0xB,  // bus error
		ARB_LOST      = 0xC,  // arbitration lost during transfer
		SEQ_ERR       = 0xD,  // erroneous programming sequence
		ST_ERR        = 0xE,  // wrong status echoing
		SW_ERR        = 0xF   // software error
	};

	virtual void SetI2CData(BYTE Data)=0;
	virtual BYTE GetI2CData()=0;

	virtual void SetI2CStatus(BYTE Status)=0;
	virtual BYTE GetI2CStatus()=0;

	virtual void SetI2CStart()=0;
	virtual void SetI2CStop()=0;
	virtual void SetI2CContinue()=0;

	virtual void I2CSleep()=0;
};

#endif
