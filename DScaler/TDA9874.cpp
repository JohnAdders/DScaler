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
//  Copyright (C) 2000 - 2002 by Eduardo JosETagle.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TDA9874.h"
#include "DebugLog.h"
#include "I2C.h"


CTDA9874::CTDA9874()
{
    m_iMode        = 1;    /* 0: A2, 1: NICAM */
    m_iGCONR    = 0xc0;    /* default config. input pin: SIFSEL=0 */
    m_iNCONR    = 0x01;    /* default NICAM config.: AMSEL=0,AMUTE=1 */
    m_iESP        = 0x06;    /* default standard: NICAM B/G */
    m_iDic        = 0x11;    /* device id. code */
}

BYTE CTDA9874::GetDefaultAddress()const
{
    //TODO
    LOG(1,"CTDA9874::GetDefaultAddress()");
    return I2C_TDA9874 >> 1;
}

void CTDA9874::Reset()
{
    //TODO
}

BOOL CTDA9874::IsDevicePresent(int& dic, int& sic)
{
    // since 0 is a TRUE value make sure we get a change
    BYTE result[1] = {3};

    if(ReadFromSubAddress(TDA9874A_DIC, 0, 0, result, sizeof(result)) == FALSE)
    {
        return FALSE;
    }

    dic = result[0];

    if(ReadFromSubAddress(TDA9874A_SIC, 0, 0, result, sizeof(result)) == FALSE)
    {
        return FALSE;
    }

    sic = result[0];

    if (dic == 0x11 || dic == 0x07)
    {
        LOG(1, "TDA9874%s audio decoder found: dic=0x%x, sic=0x%x", (dic == 0x11) ? "a":"h", dic, sic);
        m_iDic = dic;
        return TRUE;
    }

    return FALSE;
}
