/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134I2CBus.cpp,v 1.2 2002-10-26 05:24:23 atnak Exp $
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
// 13 Sep 2002   Atsushi Nakagawa      Moved some I2C stuff around
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2002/09/14 19:40:48  atnak
// various changes
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "I2CBus.h"
#include "SAA7134I2CBus.h"
#include "SAA7134I2CInterface.h"

#include "DebugLog.h"


CSAA7134I2CBus::CSAA7134I2CBus(ISAA7134I2CInterface* pSAA7134I2C)
{
    ASSERT(pSAA7134I2C != 0);
    this->m_pSAA7134I2C = pSAA7134I2C;

    InitializeSleep();
}


bool CSAA7134I2CBus::Read(const BYTE *writeBuffer,
                  size_t writeBufferSize,
                  BYTE *readBuffer,
                  size_t readBufferSize)
{
    ASSERT(m_pSAA7134I2C != 0);
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT(readBuffer != 0 || readBufferSize == 0);

    if (readBufferSize == 0)
    {
        return true;
    }

    if (!IsBusReady())
    {
        // Try to ready the bus
        if (!I2CStop() || !IsBusReady())
        {
            return false;
        }
    }

    BYTE address = writeBuffer[0];

    if (writeBufferSize != 1)
    {
        ASSERT(writeBufferSize > 1);

        SetData(address & ~1);

        // Send the address
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

    // \todo I don't know if the CONTINUE before reading is right.
    // STOP might be needed before the last grab to NAK the bus
    for (size_t i = 0; i < readBufferSize; i++)
    {
        I2CContinue();
        readBuffer[i] = GetData();
    }

    I2CStop();

    return true;
}

bool CSAA7134I2CBus::Write(const BYTE *writeBuffer, size_t writeBufferSize)
{
    ASSERT(m_pSAA7134I2C != NULL);
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT((writeBuffer[0] & 1) == 0);

    if (!IsBusReady())
    {
        // Try to ready the bus
        if (!I2CStop() || !IsBusReady())
        {
            return false;
        }
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


void CSAA7134I2CBus::Sleep()
{
    for (volatile ULONG i = m_I2CSleepCycle; i > 0; i--);
}


bool CSAA7134I2CBus::BusyWait()
{
    ASSERT(m_pSAA7134I2C != NULL);

    int Retries = 0;
    BYTE Status;

    while ((Status = m_pSAA7134I2C->GetI2CStatus()) ==
        ISAA7134I2CInterface::STATUS_BUSY)
    {
        if (Retries++ > MAX_BUSYWAIT_RETRIES)
        {
            return false;
        }
        Sleep();
    }

    return !IsError(Status);
}


void CSAA7134I2CBus::SetData(BYTE Data)
{
    ASSERT(m_pSAA7134I2C != NULL);

    m_pSAA7134I2C->SetI2CData(Data);
}


BYTE CSAA7134I2CBus::GetData()
{
    ASSERT(m_pSAA7134I2C != 0);

    return m_pSAA7134I2C->GetI2CData();
}

bool CSAA7134I2CBus::IsBusReady()
{
    BYTE Status = m_pSAA7134I2C->GetI2CStatus();

    return Status == ISAA7134I2CInterface::STATUS_IDLE ||
        Status == ISAA7134I2CInterface::STATUS_DONE_STOP;
}


bool CSAA7134I2CBus::IsError(BYTE Status)
{
    switch (Status)
    {
    case ISAA7134I2CInterface::STATUS_NO_DEVICE:
    case ISAA7134I2CInterface::STATUS_NO_ACKN:
    case ISAA7134I2CInterface::STATUS_BUS_ERR:
    case ISAA7134I2CInterface::STATUS_ARB_LOST:
    case ISAA7134I2CInterface::STATUS_SEQ_ERR:
    case ISAA7134I2CInterface::STATUS_ST_ERR:
    case ISAA7134I2CInterface::STATUS_SW_ERR:
        return true;

    default:
        // do nothing
        break;
    }
    return false;
}


bool CSAA7134I2CBus::I2CStart()
{
    ASSERT(m_pSAA7134I2C != 0);

    m_pSAA7134I2C->SetI2CCommand(ISAA7134I2CInterface::COMMAND_START);
    return BusyWait();
}


bool CSAA7134I2CBus::I2CStop()
{
    ASSERT(m_pSAA7134I2C != 0);

    m_pSAA7134I2C->SetI2CCommand(ISAA7134I2CInterface::COMMAND_STOP);
    return BusyWait();
}


bool CSAA7134I2CBus::I2CContinue()
{
    ASSERT(m_pSAA7134I2C != 0);

    m_pSAA7134I2C->SetI2CCommand(ISAA7134I2CInterface::COMMAND_CONTINUE);
    return BusyWait();
}


void CSAA7134I2CBus::InitializeSleep()
{
    m_I2CSleepCycle = 10000L;
    ULONG elapsed = 0L;

    // get a stable reading
    while (elapsed < 5)
    {
        m_I2CSleepCycle *= 10;
        ULONG start = GetTickCount();
        for (volatile ULONG i = m_I2CSleepCycle; i > 0; i--);
        elapsed = GetTickCount() - start;
    }
    // calculate how many cycles a 50kHZ is (half I2C bus cycle)
    m_I2CSleepCycle = m_I2CSleepCycle / elapsed * 1000L / 50000L;
}


ULONG CSAA7134I2CBus::GetTickCount()
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);
    ticks = (ticks & 0xFFFFFFFF00000000) / frequency * 10000000 +
            (ticks & 0xFFFFFFFF) * 10000000 / frequency;
    return (ULONG)(ticks / 10000);
}


void CSAA7134I2CBus::Start()
{
    // Unsupported
}

void CSAA7134I2CBus::Stop()
{
    // Unsupported
}

bool CSAA7134I2CBus::Write(BYTE byte)
{
    // Unsupported
    return false;
}

BYTE CSAA7134I2CBus::Read(bool last)
{
    // Unsupported
    return 0x00;
}

bool CSAA7134I2CBus::GetAcknowledge()
{
    // Unsupported
    return false;
}

void CSAA7134I2CBus::SendACK()
{
    // Unsupported
}

void CSAA7134I2CBus::SendNAK()
{
    // Unsupported
}



