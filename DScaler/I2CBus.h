/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
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
 * @file i2cbus.h i2cbus Header
 */

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
    @return TRUE if sucessful
    */
    virtual BOOL Read(
                        const BYTE *writeBuffer,
                        size_t writeBufferSize,
                        BYTE *readBuffer,
                        size_t readBufferSize
                     );

    /**
    @return TRUE if sucessful
    */
    virtual BOOL Write(const BYTE *writeBuffer, size_t writeBufferSize);

protected:
    /// Prepare the bus for comminications
    virtual void Start()=0;
    /// Return the bus to off state
    virtual void Stop()=0;
    /// Output a single byte
    virtual BOOL Write(BYTE byte)=0;
    /// Input a single byte
    virtual BYTE Read(BOOL last=TRUE)=0;
    /// Wait for ACK signal
    virtual BOOL GetAcknowledge()=0;
    /// Wait for small amount of time linked to clock speed
    virtual void Sleep()=0;
    /// Send ACK
    virtual void SendACK()=0;
    /// Send NAK
    virtual void SendNAK()=0;
    /// Lock device
    virtual void Lock()=0;
    /// Unlock device
    virtual void Unlock()=0;

};

#endif // !defined(__I2CBUS_H__)
