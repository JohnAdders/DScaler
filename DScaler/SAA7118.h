//
// $Id: SAA7118.h,v 1.3 2003-10-27 10:39:53 adcockj Exp $
//
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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

/** 
 * @file saa7118.h saa7118 Header file
 */
 
#ifndef __SAA7118_H___
#define __SAA7118_H___


#include "I2CDevice.h"

/** Implemenation of a simple tuner connected via I2C
*/
class CSAA7118 : public CI2CDevice  
{
public:
    CSAA7118();
	void SetBrightness(BYTE Brightness);
	void SetContrast(BYTE Contrast);
	void SetHue(BYTE Hue);
	void SetSaturation(BYTE Saturation);
	void SetComponentBrightness(BYTE Brightness);
	void SetComponentContrast(BYTE Contrast);
	void SetComponentSaturation(BYTE Saturation);

	BYTE GetVersion();
	void DumpSettings();

    void SetRegister(BYTE Register, BYTE Value);
    BYTE GetRegister(BYTE Register);

protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const;

private:
};


#endif
