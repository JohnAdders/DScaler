//
// $Id: I2CDevice.h,v 1.4 2001-12-18 23:36:01 adcockj Exp $
//
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
// Revision 1.3  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(__I2CDEVICE_H__)
#define __I2CDEVICE_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "I2CBus.h"

/** Base class for devices that support control via I2C
*/
class CI2CDevice
{
public:
    CI2CDevice();
    virtual ~CI2CDevice() {};
    /** Attach the device to a bus, note that this does not
        test for the presence of a device.  Use the address 
        parameter to overrride the default address
    */
    virtual void Attach(CI2CBus* i2cBus, BYTE address = 0);
    CI2CBus* GetI2CBus() const;
    BYTE GetDeviceAddress() const;
    bool WriteToSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize);
    bool ReadFromSubAddress(BYTE subAddress, BYTE* readBuffer, size_t readBufferSize);
    bool ReadFromSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize, BYTE* readBuffer, size_t readBufferSize);
protected:
    /// must override this to provide expected location on I2C bus
    virtual BYTE GetDefaultAddress() const = 0;

protected:
    CI2CBus *m_I2CBus;
    BYTE m_DeviceAddress;
};

#endif // !defined(__I2CDEVICE_H__)
