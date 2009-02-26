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

#include "stdafx.h"
#include "I2CBus.h"

CI2CBus::CI2CBus()
{
    InitializeCriticalSection(&I2CCriticalSection);
}

CI2CBus::~CI2CBus()
{
    DeleteCriticalSection(&I2CCriticalSection);
}

bool CI2CBus::Read(    const BYTE *writeBuffer,
                    size_t writeBufferSize,
                    BYTE *readBuffer,
                    size_t readBufferSize)
{
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT(readBuffer != 0 || readBufferSize == 0);

    if (readBufferSize == 0)
    {
        return true;
    }

    BYTE address = writeBuffer[0];

    if (writeBufferSize != 1)
    {
        ASSERT(writeBufferSize > 1);
        
        EnterCriticalSection(&I2CCriticalSection);
        Start();
        
        // send the address
        if (!Write(address & ~1))
        {
            Stop();
            LeaveCriticalSection(&I2CCriticalSection);
            return false;
        }

        for (size_t i = 1; i < (writeBufferSize - 1); i++)
        {
            if(!Write(writeBuffer[i]))
            {
                Stop();
                LeaveCriticalSection(&I2CCriticalSection);
                return false;
            }
        }

        // The last byte may also create a positive acknowledge, indicating, that
        // the device is "full", which is not an error.
        if (writeBufferSize >= 2)
        {
            Write(writeBuffer[writeBufferSize - 1]);
        }
        
        LeaveCriticalSection(&I2CCriticalSection);
    }

    EnterCriticalSection(&I2CCriticalSection);
    Start();

    // The read address requires a negative ack
    if (!Write(address | 1))
    {
        Stop();
        LeaveCriticalSection(&I2CCriticalSection);
        return false;
    }
    
    for (size_t i = 0; i < (readBufferSize - 1); i++)
    {
        readBuffer[i] = Read(false);
    }

    readBuffer[i] = Read(true);
    Stop();
    LeaveCriticalSection(&I2CCriticalSection);

    return true;
}

bool CI2CBus::Write(const BYTE *writeBuffer, size_t writeBufferSize)
{
    ASSERT(writeBuffer != 0);
    ASSERT(writeBufferSize >= 1);
    ASSERT((writeBuffer[0] & 1) == 0);
    
    EnterCriticalSection(&I2CCriticalSection);

    Start();

    for (size_t i = 0; i < writeBufferSize; i++)
    {
        if (!Write(writeBuffer[i]))
        {
            Stop();
            LeaveCriticalSection(&I2CCriticalSection);
            return false;
        }
    }

    Stop();
    LeaveCriticalSection(&I2CCriticalSection);

    return true;
}
