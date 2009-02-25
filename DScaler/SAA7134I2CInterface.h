/////////////////////////////////////////////////////////////////////////////
// $Id$
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
//
// This software was based on I2CBus.cpp.  Those portions are
// copyleft 2001 itt@myself.com.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 13 Sep 2002   Atsushi Nakagawa      Changed some I2C stuff
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2005/03/24 17:57:58  adcockj
// Card access from one thread at a time
//
// Revision 1.4  2003/10/27 10:39:53  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.3  2002/09/14 19:40:48  atnak
// various changes
//
//
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file saa7134i2cinterface.h saa7134i2cinterface Header file
 */
 
#ifdef WANT_SAA713X_SUPPORT

#ifndef __SAA7134I2CINTERFACE_H__
#define __SAA7134I2CINTERFACE_H__


class ISAA7134I2CInterface
{
public:

    enum eCommand
    {
        COMMAND_STOP            = 0x40, // Stop transfer
        COMMAND_CONTINUE        = 0x80, // Continue transfer
        COMMAND_START           = 0xC0  // Start transfer (address device)
    };

    enum eStatus
    {
        STATUS_IDLE             = 0x0,  // no I2C command pending
        STATUS_DONE_STOP        = 0x1,  // I2C command done and STOP executed
        STATUS_BUSY             = 0x2,  // executing I2C command
        STATUS_TO_SCL           = 0x3,  // executing I2C command, time out on clock stretching
        STATUS_TO_ARB           = 0x4,  // time out on arbitration trial, still trying
        STATUS_DONE_WRITE       = 0x5,  // I2C command done and awaiting next write command
        STATUS_DONE_READ        = 0x6,  // I2C command done and awaiting next read command
        STATUS_DONE_WRITE_TO    = 0x7,  // see 5, and time out on status echo
        STATUS_DONE_READ_TO     = 0x8,  // see 6, and time out on status echo
        STATUS_NO_DEVICE        = 0x9,  // no acknowledge on device slave address
        STATUS_NO_ACKN          = 0xA,  // no acknowledge after data byte transfer
        STATUS_BUS_ERR          = 0xB,  // bus error
        STATUS_ARB_LOST         = 0xC,  // arbitration lost during transfer
        STATUS_SEQ_ERR          = 0xD,  // erroneous programming sequence
        STATUS_ST_ERR           = 0xE,  // wrong status echoing
        STATUS_SW_ERR           = 0xF   // software error
    };

    virtual void SetI2CData(BYTE Data)=0;
    virtual BYTE GetI2CData()=0;

    virtual BYTE GetI2CStatus()=0;
    virtual void SetI2CStatus(BYTE Status)=0;

    virtual void SetI2CCommand(BYTE Command)=0;

    virtual void I2CLock()=0;
    virtual void I2CUnlock()=0;
};

#endif

#endif//xxx