/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134I2CBusInterface.h,v 1.2 2002-09-09 14:25:16 atnak Exp $
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
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134I2CBUSINTERFACE_H__
#define __SAA7134I2CBUSINTERFACE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SAA7134I2CInterface.h"
#include "I2CBus.h"


class CSAA7134I2CBusInterface : public CI2CBus
{
public:
    CSAA7134I2CBusInterface(ISAA7134I2CInterface *saa7134Interface);

public:
    /**
    @return true if sucessful
    */
    virtual bool Read(
                        const BYTE *writeBuffer,
                        size_t writeBufferSize,
                        BYTE *readBuffer,
                        size_t readBufferSize
                     );
    
    /**
    @return true if sucessful
    */
    virtual bool Write(const BYTE *writeBuffer, size_t writeBufferSize);

protected:
    virtual void Sleep();
	virtual void SetData(BYTE Data);
	virtual BYTE GetData();
	virtual bool I2CStart();
	virtual bool I2CContinue();
	virtual bool I2CStop();
	virtual bool BusyWait();
	virtual bool IsReady();

    virtual void Start() {};
    virtual void Stop() {};
    virtual bool Write(BYTE byte) { return FALSE; };
    virtual BYTE Read(bool last=true) { return 0x00; };
    virtual bool GetAcknowledge() { return FALSE; };
    virtual void SendACK() {};
    virtual void SendNAK() {};

private:
    ISAA7134I2CInterface *m_SAA7134Interface;
};


#endif
