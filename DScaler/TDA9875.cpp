//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004 Robert Milharcic.  All rights reserved.
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
 * @file TDA9875.cpp CTDA9875 Implementation
 */

#include "stdafx.h"
#include "TDA9875.h"
#include "I2C.h"


CTDA9875::CTDA9875()
{
}

BYTE CTDA9875::GetDefaultAddress()const
{
    return I2C_TDA9875 >> 1;
}

void CTDA9875::Reset()
{

    WriteToSubAddress(TDA9875_CFG, 0xd0); /* reg de config 0 (reset)*/
    WriteToSubAddress(TDA9875_CH1V, 22 );  /* Channel volume 1 -10db */
    WriteToSubAddress(TDA9875_CH2V, 22 );  /* Channel volume 2 -10db */ 
    WriteToSubAddress(TDA9875_NCHV, 22 );  /* NICAM volume -10db */ 
    WriteToSubAddress(TDA9875_MUT, TDA9875_MUTE_ON);   /* General mute  */
        
}

BOOL CTDA9875::IsDevicePresent(int& dic, int& rev)
{
    // since 0 is a TRUE valuie make sure we get a change
    BYTE result[1] = {3};

    if(ReadFromSubAddress(254, 0, 0, result, sizeof(result)) == FALSE)
    {
        return FALSE;
    }
    dic = result[0];
    if(ReadFromSubAddress(255, 0, 0, result, sizeof(result)) == FALSE)
    {
        return FALSE;
    }

    rev = result[0];

    if(dic==0 || dic==2) // tda9875 and tda9875A
        return TRUE;

    return FALSE; 
}
