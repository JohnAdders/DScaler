/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file SAA7134I2CBus.cpp CSAA7134I2CBus Implementation
 */

#include "stdafx.h"

#ifdef WANT_SAA713X_SUPPORT

#include "I2CBus.h"
#include "SAA7134I2CBus.h"
#include "SAA7134I2CInterface.h"

#include "DebugLog.h"


CSAA7134I2CBus::CSAA7134I2CBus(ISAA7134I2CInterface* pSAA7134I2C) :
    m_InitializedSleep(FALSE)
{
    _ASSERTE(pSAA7134I2C != 0);
    this->m_pSAA7134I2C = pSAA7134I2C;
}


BOOL CSAA7134I2CBus::Read(const BYTE *writeBuffer,
                  size_t writeBufferSize,
                  BYTE *readBuffer,
                  size_t readBufferSize)
{
    _ASSERTE(m_pSAA7134I2C != 0);
    _ASSERTE(writeBuffer != 0);
    _ASSERTE(writeBufferSize >= 1);
    _ASSERTE(readBuffer != 0 || readBufferSize == 0);

    if (readBufferSize == 0)
    {
        return TRUE;
    }

    Lock();
    if (!IsBusReady())
    {
        // Try to ready the bus
        if (!I2CStop() || !IsBusReady())
        {
            Unlock();
            return FALSE;
        }
    }

    BYTE address = writeBuffer[0];

    if (writeBufferSize != 1)
    {
        _ASSERTE(writeBufferSize > 1);

        SetData(address & ~1);

        // Send the address
        if (!I2CStart())
        {
            LOGD(_T("I2CBus::write(0x%x) returned TRUE for write address in CI2CBus::read\n"), address & ~1);
            I2CStop();
            Unlock();
            return FALSE;
        }

        for (size_t i = 1; i < (writeBufferSize - 1); i++)
        {
            SetData(writeBuffer[i]);
            if(!I2CContinue())
            {
                I2CStop();
                Unlock();
                return FALSE;
            }
        }

        // The last byte may also create a positive acknowledge, indicating, that
        // the device is _T("full"), which is not an error.
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
        LOGD(_T("I2CBus::write(0x%x) returned FALSE for read address in CI2CBus::read\n"), address | 1);
        I2CStop();
        Unlock();
        return FALSE;
    }

    // \todo I don't know if the CONTINUE before reading is right.
    // STOP might be needed before the last grab to NAK the bus
    for (size_t i = 0; i < readBufferSize; i++)
    {
        I2CContinue();
        readBuffer[i] = GetData();
    }

    I2CStop();
    Unlock();

    return TRUE;
}

BOOL CSAA7134I2CBus::Write(const BYTE *writeBuffer, size_t writeBufferSize)
{
    _ASSERTE(m_pSAA7134I2C != NULL);
    _ASSERTE(writeBuffer != 0);
    _ASSERTE(writeBufferSize >= 1);
    _ASSERTE((writeBuffer[0] & 1) == 0);

    Lock();

    if (!IsBusReady())
    {
        // Try to ready the bus
        if (!I2CStop() || !IsBusReady())
        {
            Unlock();
            return FALSE;
        }
    }

    SetData(writeBuffer[0]);
    if (!I2CStart())
    {
        I2CStop();
        Unlock();
        return FALSE;
    }

    for (size_t i = 1; i < writeBufferSize; i++)
    {
        SetData(writeBuffer[i]);
        if (!I2CContinue())
        {
            I2CStop();
            Unlock();
            return FALSE;
        }
    }

    I2CStop();
    Unlock();

    return TRUE;
}


void CSAA7134I2CBus::Sleep()
{
    ULONGLONG ticks = 0;
    ULONGLONG start;

    QueryPerformanceCounter((PLARGE_INTEGER)&start);

    while(start + m_I2CSleepCycle > ticks)
    {
        QueryPerformanceCounter((PLARGE_INTEGER)&ticks);
    }
}


BOOL CSAA7134I2CBus::BusyWait()
{
    _ASSERTE(m_pSAA7134I2C != NULL);

    int Retries = 0;
    BYTE Status;

    if (m_InitializedSleep == FALSE)
    {
        InitializeSleep();
    }

    while ((Status = m_pSAA7134I2C->GetI2CStatus()) ==
        ISAA7134I2CInterface::STATUS_BUSY)
    {
        if (Retries++ > MAX_BUSYWAIT_RETRIES)
        {
            return FALSE;
        }
        Sleep();
    }

    return !IsError(Status);
}


void CSAA7134I2CBus::SetData(BYTE Data)
{
    _ASSERTE(m_pSAA7134I2C != NULL);

    m_pSAA7134I2C->SetI2CData(Data);
}


BYTE CSAA7134I2CBus::GetData()
{
    _ASSERTE(m_pSAA7134I2C != 0);

    return m_pSAA7134I2C->GetI2CData();
}

BOOL CSAA7134I2CBus::IsBusReady()
{
    BYTE Status = m_pSAA7134I2C->GetI2CStatus();

    return Status == ISAA7134I2CInterface::STATUS_IDLE ||
        Status == ISAA7134I2CInterface::STATUS_DONE_STOP;
}


BOOL CSAA7134I2CBus::IsError(BYTE Status)
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
        return TRUE;

    default:
        // do nothing
        break;
    }
    return FALSE;
}


BOOL CSAA7134I2CBus::I2CStart()
{
    _ASSERTE(m_pSAA7134I2C != 0);

    m_pSAA7134I2C->SetI2CCommand(ISAA7134I2CInterface::COMMAND_START);
    return BusyWait();
}


BOOL CSAA7134I2CBus::I2CStop()
{
    _ASSERTE(m_pSAA7134I2C != 0);

    m_pSAA7134I2C->SetI2CCommand(ISAA7134I2CInterface::COMMAND_STOP);
    return BusyWait();
}


BOOL CSAA7134I2CBus::I2CContinue()
{
    _ASSERTE(m_pSAA7134I2C != 0);

    m_pSAA7134I2C->SetI2CCommand(ISAA7134I2CInterface::COMMAND_CONTINUE);
    return BusyWait();
}


void CSAA7134I2CBus::InitializeSleep()
{
    ULONGLONG frequency;
    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);

    m_I2CSleepCycle = (unsigned long)(frequency / 50000);

    m_InitializedSleep = TRUE;
}


ULONG CSAA7134I2CBus::GetTickCount()
// an overflow happens after 21 days uptime on a 10GHz machine
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);

    ticks = ticks * 1000 / frequency;
    return (ULONG)ticks;
}

void CSAA7134I2CBus::Start()
{
    // Unsupported
}

void CSAA7134I2CBus::Stop()
{
    // Unsupported
}

BOOL CSAA7134I2CBus::Write(BYTE byte)
{
    // Unsupported
    return FALSE;
}

BYTE CSAA7134I2CBus::Read(BOOL last)
{
    // Unsupported
    return 0x00;
}

BOOL CSAA7134I2CBus::GetAcknowledge()
{
    // Unsupported
    return FALSE;
}

void CSAA7134I2CBus::Lock()
{
    m_pSAA7134I2C->I2CLock();
}

void CSAA7134I2CBus::Unlock()
{
    m_pSAA7134I2C->I2CUnlock();
}


void CSAA7134I2CBus::SendACK()
{
    // Unsupported
}

void CSAA7134I2CBus::SendNAK()
{
    // Unsupported
}

#endif//xxx
