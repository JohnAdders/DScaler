/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Torsten Seeboth.  All rights reserved.
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
* @file TDA5767.cpp TEA5767 chip related implementations
*/

#include "stdafx.h"
#include "TEA5767.h"
#include "I2C.h"
#include "DebugLog.h"


BOOL IsTEA5767PresentAtC0(IN CI2CBus* i2cBus)
{
    BYTE buffer[7] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    BYTE addr = I2C_TEA5767;

    // Sub addresses are not supported so read all five bytes.
    if (!i2cBus->Read(&addr, sizeof(addr), buffer, sizeof(buffer)))
    {
        LOG(2, _T("TEA5767: No I2C device at 0xC0."));
        return FALSE;
    }

    // If all bytes are the same then it's a tuner and not a tea5767 chip.
    if (buffer[0] == buffer[1] &&
        buffer[0] == buffer[2] &&
        buffer[0] == buffer[3] &&
        buffer[0] == buffer[4])
    {
        LOG(2, _T("TEA5767: Not Found. All bytes are equal."));
        return FALSE;
    }

    // Status bytes:
    // Byte 4: bit 3:1 : CI (Chip Identification) == 0
    //         bit 0   : internally set to 0
    // Byte 5: bit 7:0 : == 0
    if (((buffer[3] & 0x0f) != 0x00) || (buffer[4] != 0x00))
    {
        LOG(2, _T("TEA5767: Not Found. Chip ID is not zero."));
        return FALSE;
    }

    // It seems that tea5767 returns 0xff after the 5th byte
    if ((buffer[5] != 0xff) || (buffer[6] != 0xff))
    {
        LOG(2, _T("TEA5767: Not Found. Returned more than 5 bytes."));
        return FALSE;
    }

    LOG(1, _T("TEA5767: Found. 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X"),
        buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);

    return TRUE;
}

