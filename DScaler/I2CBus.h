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
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.4  2002/09/27 14:10:26  kooiman
// thread safe i2c bus access
//
// Revision 1.3  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
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
    /// Lock device
    virtual void Lock()=0;
    /// Unlock device
    virtual void Unlock()=0;
  
};

#endif // !defined(__I2CBUS_H__)
