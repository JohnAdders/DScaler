/////////////////////////////////////////////////////////////////////////////
// $id$
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
// $log$
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I2CBus.h"
#include "SAA7134I2CInterface.h"
#include "SAA7134I2CBusInterface.h"

#include "DebugLog.h"


CSAA7134I2CBusInterface::CSAA7134I2CBusInterface(ISAA7134I2CInterface* saa7134Interface)
{
    ASSERT(saa7134Interface != 0);
    this->m_SAA7134Interface = saa7134Interface;
}


bool CSAA7134I2CBusInterface::Read(const BYTE *writeBuffer,
                  size_t writeBufferSize,
                  BYTE *readBuffer,
                  size_t readBufferSize)
{
    ASSERT(m_SAA7134Interface != 0);
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT(readBuffer != 0 || readBufferSize == 0);

    if (readBufferSize == 0)
	{
        return true;
	}

    if (!IsReady())
    {
        if (!I2CStop() || !IsReady())
            return false;
    }

    BYTE address = writeBuffer[0];

    if (writeBufferSize != 1)
    {
        ASSERT(writeBufferSize > 1);

        SetData(address & ~1);

        // send the address
        if (!I2CStart())
        {
            LOGD("I2CBus::write(0x%x) returned true for write address in CI2CBus::read\n", address & ~1);
            I2CStop();
            return false;
        }

        for (size_t i = 1; i < (writeBufferSize - 1); i++)
        {
            SetData(writeBuffer[i]);
            if(!I2CContinue())
            {
                I2CStop();
                return false;
            }
        }

        // The last byte may also create a positive acknowledge, indicating, that
        // the device is "full", which is not an error.
        if (writeBufferSize >= 2)
		{
            SetData(writeBuffer[writeBufferSize - 1]);
            I2CContinue();
        }
    }

    SetData(address | 1);

    // The read address requires a negative ack
    if (!I2CStart())
    {
        LOGD("I2CBus::write(0x%x) returned false for read address in CI2CBus::read\n", address | 1);
        I2CStop();
        return false;
    }

    // FIXME: I don't know if the CONTINUE before reading is right
    // STOP might be needed before grab the last BYTE with a NAK
    for (size_t i = 0; i < readBufferSize; i++)
	{
        I2CContinue();
        readBuffer[i] = GetData();
    }

    I2CStop();

    return true;
}

bool CSAA7134I2CBusInterface::Write(const BYTE *writeBuffer, size_t writeBufferSize)
{
    ASSERT(m_SAA7134Interface != 0);
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT((writeBuffer[0] & 1) == 0);

    if (!IsReady())
    {
        if (!I2CStop() || !IsReady())
            return false;
    }

    SetData(writeBuffer[0]);
    if (!I2CStart())
    {
        I2CStop();
        return FALSE;
    }

    for (size_t i = 1; i < writeBufferSize; i++)
    {
        SetData(writeBuffer[i]);
        if (!I2CContinue())
        {
            I2CStop();
            return false;
        }
    }

    I2CStop();

    return true;
}


void CSAA7134I2CBusInterface::Sleep()
{
    ASSERT(m_SAA7134Interface != 0);

    m_SAA7134Interface->I2CSleep();
}


bool CSAA7134I2CBusInterface::BusyWait()
{
    ASSERT(m_SAA7134Interface != 0);

    int Retries = 0;

    // 0x2 = BUSYs
    while (m_SAA7134Interface->GetI2CStatus() == 0x2)
    {
        if (Retries++ > 16)
            return FALSE;

        m_SAA7134Interface->I2CSleep();
    }
    return TRUE;
}


void CSAA7134I2CBusInterface::SetData(BYTE Data)
{
    ASSERT(m_SAA7134Interface != 0);

    m_SAA7134Interface->SetI2CData(Data);
}


BYTE CSAA7134I2CBusInterface::GetData()
{
    ASSERT(m_SAA7134Interface != 0);

    return m_SAA7134Interface->GetI2CData();
}



bool CSAA7134I2CBusInterface::I2CStart()
{
    ASSERT(m_SAA7134Interface != 0);

    m_SAA7134Interface->SetI2CStart();

    BusyWait();

    // 0x09 = first error
    return m_SAA7134Interface->GetI2CStatus() < 0x9;
}


bool CSAA7134I2CBusInterface::I2CContinue()
{
    ASSERT(m_SAA7134Interface != 0);

    m_SAA7134Interface->SetI2CContinue();

    BusyWait();

    // 0x09 = first error
    return m_SAA7134Interface->GetI2CStatus() < 0x9;
}

bool CSAA7134I2CBusInterface::I2CStop()
{
    ASSERT(m_SAA7134Interface != 0);

    m_SAA7134Interface->SetI2CStop();

    BusyWait();

    // 0x09 = first error
    return m_SAA7134Interface->GetI2CStatus() < 0x9;
}


bool CSAA7134I2CBusInterface::IsReady()
{
    // 0x00 = IDLE, 0x01 = DONE_STOP
    return m_SAA7134Interface->GetI2CStatus() <= 0x1;
}


