/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134I2CBus.h,v 1.1 2004-10-30 19:30:22 to_see Exp $
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

#ifndef __SAA7134I2CBUS_H__
#define __SAA7134I2CBUS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "I2CBus.h"
#include "..\..\DScaler\SAA7134I2CInterface.h"

class CSAA7134I2CBus : public CI2CBus
{
private:
    enum
    {
        MAX_BUSYWAIT_RETRIES    = 16,
    };

public:
    CSAA7134I2CBus(ISAA7134I2CInterface* pSAA7134I2C);

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
    /// Sets the data for the next command
    virtual void SetData(BYTE Data);
    /// Reads the data from the last command
    virtual BYTE GetData();
    /// Wait until the last command is finished.
    virtual bool BusyWait();
    /// Is the bus ready?
    virtual bool IsBusReady();

    /// Is the status an error?
    virtual bool IsError(BYTE Status);

    /// Addresses the device
    virtual bool I2CStart();
    /// Stops the transfer
    virtual bool I2CStop();
    /// Continue transfering the next byte
    virtual bool I2CContinue();

    /// Generic delay function
    virtual void Sleep();

    /// These are not supported
    virtual void Start();
    virtual void Stop();
    virtual bool Write(BYTE byte);
    virtual BYTE Read(bool last=true);
    virtual bool GetAcknowledge();
    virtual void SendACK();
    virtual void SendNAK();

private:
    void InitializeSleep();

private:
    ISAA7134I2CInterface*   m_pSAA7134I2C;
    BOOL                    m_InitializedSleep;
    ULONG                   m_I2CSleepCycle;
};


#endif
