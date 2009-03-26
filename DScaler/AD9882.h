//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2006 Curtiss-Wright Controls, Inc..  All rights reserved.
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
 * @file AD9882.h AD9882 Header file
 */

#ifndef __AD9882_H___
#define __AD9882_H___


#include "Settings.h"
#include "I2CDevice.h"
#include "TVFormats.h"

/** Implemenation of a RGB decoder connected via I2C
*/
class CAD9882 : public CI2CDevice
{
public:
    CAD9882();

    BYTE GetVersion();
    void DumpSettings(LPCSTR Filename);

    void SetRegister(BYTE Register, BYTE Value);
    BYTE GetRegister(BYTE Register);

    void Suspend();
    void Wakeup();
    void calcAD9882PLL(int htotal, int vtotal, int vf, int ilace,
                        int &vco_range, int &cpump);

    void SetPLL(WORD Value);
    void SetVCO(BYTE Value);
    void SetPump(BYTE Value);
    void SetPhase(BYTE Value);
    void SetPreCoast(BYTE Value);
    void SetPostCoast(BYTE Value);
    void SetHSync(BYTE Value);
    void SetSyncSep(BYTE Value);
    void SetSOGThresh(BYTE Value);
    void SetSOG(BOOLEAN YesNo);
    void SetCoastSel(BOOLEAN IntExt);
    void SetCoastOvr(BOOLEAN ManAuto);
    void SetCoastPol(BOOLEAN HighLow);

protected:
    // from CI2CDevice
    virtual BYTE GetDefaultAddress() const;

};

#endif
