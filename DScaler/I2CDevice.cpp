//
// $Id$
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

/**
 * @file I2CDevice.cpp CI2CDevice Implementation
 */

#include "stdafx.h"
#include "I2CDevice.h"


//////////////////////////////////////////////////////////////////////////
// CI2CDevice
//////////////////////////////////////////////////////////////////////////

CI2CDevice::CI2CDevice()
{
    this->m_I2CBus = 0;
    this->m_DeviceAddress = 0;
}

void CI2CDevice::SetI2CBus(CI2CBus* i2cBus, BYTE address)
{
    _ASSERTE(i2cBus != NULL);

    this->m_I2CBus = i2cBus;
    this->m_DeviceAddress = (address == 0x00) ? GetDefaultAddress() : address;
}

CI2CBus* CI2CDevice::GetI2CBus()const
{
    return m_I2CBus;
}

BYTE CI2CDevice::GetDeviceAddress()const
{
    return m_DeviceAddress;
}

BOOL CI2CDevice::WriteToSubAddress(BYTE subAddress, BYTE writeByte)
{
    return WriteToSubAddress(subAddress, &writeByte, 1);
}

BOOL CI2CDevice::WriteToSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize)
{
    _ASSERTE(m_I2CBus != 0);

    BYTE* buffer = (BYTE*)malloc(writeBufferSize + 2);
    buffer[0] = m_DeviceAddress << 1;
    buffer[1] = subAddress;
    memcpy(&buffer[2], writeBuffer, writeBufferSize);

    BOOL result = m_I2CBus->Write(buffer, writeBufferSize + 2);
    free(buffer);
    return result;
}

BOOL CI2CDevice::ReadFromSubAddress(BYTE subAddress, BYTE* readBuffer, size_t readBufferSize)
{
    return this->ReadFromSubAddress(subAddress, NULL, 0, readBuffer, readBufferSize);
}

BOOL CI2CDevice::ReadFromSubAddress(BYTE subAddress, const BYTE* writeBuffer, size_t writeBufferSize, BYTE* readBuffer, size_t readBufferSize)
{
    _ASSERTE(m_I2CBus != 0);

    if (writeBuffer == 0)
    {
        writeBufferSize = 0;
    }
    BYTE* buffer = (BYTE*)malloc(writeBufferSize + 2);
    buffer[0] = m_DeviceAddress << 1;
    buffer[1] = subAddress;
    if (writeBufferSize != 0)
    {
        memcpy(&buffer[2], writeBuffer, writeBufferSize);
    }

    BOOL result = m_I2CBus->Read(buffer, writeBufferSize + 2, readBuffer, readBufferSize);
    free(buffer);
    return result;
}


//////////////////////////////////////////////////////////////////////////
// CGenericI2CDevice
//////////////////////////////////////////////////////////////////////////

CGenericI2CDevice::CGenericI2CDevice()
{
}

CGenericI2CDevice::CGenericI2CDevice(CI2CBus* i2cBus, BYTE address)
{
    SetI2CBus(i2cBus, address);
}

CGenericI2CDevice::~CGenericI2CDevice()
{
}

void CGenericI2CDevice::SetI2CBus(CI2CBus* i2cBus, BYTE address)
{
    CI2CDevice::SetI2CBus(i2cBus, address);
}

