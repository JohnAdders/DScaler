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
 * @file PIC16C54.h PIC16C54 Header file
 */

#ifndef _PIC16C54_H
#define _PIC16C54_H

#include "i2cdevice.h"


/* ---------------------------------------------------------------------- */
/* audio chip descriptions - defines+functions for pic16c54 (PV951)       */

/* the registers of 16C54, I2C sub address. */
#define PIC16C54_REG_KEY_CODE     0x01           /* Not use. */
#define PIC16C54_REG_MISC         0x02

/* bit definition of the RESET register, I2C data. */
#define PIC16C54_MISC_RESET_REMOTE_CTL 0x01 /* bit 0, Reset to receive the key */
                                            /*        code of remote controller */
#define PIC16C54_MISC_MTS_MAIN         0x02 /* bit 1 */
#define PIC16C54_MISC_MTS_SAP          0x04 /* bit 2 */
#define PIC16C54_MISC_MTS_BOTH         0x08 /* bit 3 */
#define PIC16C54_MISC_SND_MUTE         0x10 /* bit 4, Mute Audio(Line-in and Tuner) */
#define PIC16C54_MISC_SND_NOTMUTE      0x20 /* bit 5 */
#define PIC16C54_MISC_SWITCH_TUNER     0x40 /* bit 6    , Switch to Tuner */
#define PIC16C54_MISC_SWITCH_LINE      0x80 /* bit 7    , Switch to Line-In */

class CPIC16C54 : public CI2CDevice
{
public:
    CPIC16C54(void);

    BYTE GetDefaultAddress() const;
    bool IsDevicePresent();

    virtual ~CPIC16C54(void);

};

#endif
