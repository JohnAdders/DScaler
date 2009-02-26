/////////////////////////////////////////////////////////////////////////////
// $Id$
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
//  Copyright (C) 2004 Mika Laine.  All rights reserved.
//  Copyright (C) 2000-2005 Quenotte  All rights reserved.
//  Copyright (C) 2000 - 2002 by Eduardo José Tagle.
/////////////////////////////////////////////////////////////////////////////

/**
 * @file PIC16C54.cpp CPIC16C54 Implementation
 */

#include "stdafx.h"
#include "I2C.h"
#include "pic16c54.h"

// need to switch unmute from PIC one time only
// used only for PV951 (Hercules SmartTV)

// \Todo: Add more audio routing for LineIn, Tuner, Muting...

CPIC16C54::CPIC16C54(void)
{
}

bool CPIC16C54::IsDevicePresent()
{
    // unmute and switch audio source to tuner
    BYTE buffer = PIC16C54_MISC_SND_NOTMUTE | PIC16C54_MISC_SWITCH_TUNER;
    return CI2CDevice::WriteToSubAddress(PIC16C54_REG_MISC, &buffer, sizeof(buffer));
}

BYTE CPIC16C54::GetDefaultAddress() const
{
    return I2C_PIC16C54 >> 1;
}

CPIC16C54::~CPIC16C54(void)
{
}
