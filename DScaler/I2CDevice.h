//
// $Id: I2CDevice.h,v 1.7 2005-03-09 09:35:16 atnak Exp $
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
// Revision 1.6  2005/03/07 09:12:18  atnak
// Added a function for simplifying a single byte write to subaddress.
//
// Revision 1.5  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.4  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
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
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file i2cdevice.h i2cdevice Header
 */

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

    // Set the bus and the address that should use for reading
    // and writing to the device.
    void        SetI2CBus(CI2CBus* i2cBus, BYTE address = 0x00);
    CI2CBus*    GetI2CBus() const;
    BYTE        GetDeviceAddress() const;

    // Various functions for writing data to this device.
    bool WriteToSubAddress(BYTE subAddress, BYTE writeByte);
    bool WriteToSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize);
    // Various functions for reading data from this device.
    bool ReadFromSubAddress(BYTE subAddress, BYTE* readBuffer, size_t readBufferSize);
    bool ReadFromSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize, BYTE* readBuffer, size_t readBufferSize);
protected:
    // This should be overridden to provide the expected I2C
    // address of the device.
    virtual BYTE GetDefaultAddress() const { return 0x00; }

protected:
    CI2CBus *m_I2CBus;
    BYTE m_DeviceAddress;
};

#endif // !defined(__I2CDEVICE_H__)
