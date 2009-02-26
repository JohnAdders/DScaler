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

#if !defined(__I2CBUS_H__)
#define __I2CBUS_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/** Abstract interface for I2C buses.
    This class abstracts the I2C bus used to control extra chips on cards.
    The bus used a serial interface using two lines.
*/
class CI2CBus  
{
public:
    CI2CBus();
    ~CI2CBus();

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
    /// Prepare the bus for comminications
    virtual void Start()=0;
    /// Return the bus to off state
    virtual void Stop()=0;
    /// Output a single byte
    virtual bool Write(BYTE byte)=0;
    /// Input a single byte
    virtual BYTE Read(bool last=true)=0;
    /// Wait for ACK signal
    virtual bool GetAcknowledge()=0;
    /// Wait for small amount of time linked to clock speed
    virtual void Sleep()=0;
    /// Send ACK
    virtual void SendACK()=0;
    /// Send NAK
    virtual void SendNAK()=0;
private:
    CRITICAL_SECTION I2CCriticalSection;    
};

#endif // !defined(__I2CBUS_H__)
