/////////////////////////////////////////////////////////////////////////////
// Tuner.cpp
/////////////////////////////////////////////////////////////////////////////
// Code sourced from bttv linux driver tuner.c file
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// Copyright (c) 1999,2000 Gerd Knorr <kraxel@goldbach.in-berlin.de>
// Copyright (C) 1997 Markus Schroeder (schroedm@uni-duesseldorf.de)
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Moved Tuner Functions in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 28 Jun 2001   John Adcock           Added support for Philips SECAM tuner
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "i2c.h"
#include "Tuner.h"

BYTE TunerDeviceWrite, TunerDeviceRead;
char TunerStatus[30] = "No Device on I2C-Bus";

const char* Tuner_Status()
{
    return TunerStatus;
}

BOOL Tuner_Init()
{
    unsigned char j;

    if (GetTunerSetup() == NULL)
        return (TRUE);

    j = 0xc0;
    TunerDeviceRead = j;
    TunerDeviceWrite = j;

    while ((j <= 0xce) && (I2CBus_AddDevice((BYTE) j) == FALSE))
    {
        j++;
        TunerDeviceRead = j;
        TunerDeviceWrite = j;
    }

    if (j > 0xce)
    {
        return (FALSE);
    }
    sprintf(TunerStatus, "Tuner I2C-Bus I/O 0x%02x", j);
    return (TRUE);
}

/*
 *  Set TSA5522 synthesizer frequency
 */
BOOL Tuner_SetFrequency(int wFrequency)
{
    BYTE config;
    WORD div;
    BOOL bAck;

    if (GetTunerSetup() == NULL)
        return (TRUE);

    if (wFrequency < GetTunerSetup()->thresh1)
        config = GetTunerSetup()->VHF_L;
    else if (wFrequency < GetTunerSetup()->thresh2)
        config = GetTunerSetup()->VHF_H;
    else
        config = GetTunerSetup()->UHF;

    div = wFrequency + GetTunerSetup()->IFPCoff;

    // handle mode on Philips SECAM tuners
    // they can also recive PAL if the mode is set properly
    if(Setting_GetValue(TVCard_GetSetting(CURRENTTUNERTYPE)) == TUNER_PHILIPS_SECAM)
    {
        if(Setting_GetValue(BT848_GetSetting(TVFORMAT)) == FORMAT_SECAM)
        {
            config |= 0x02;
        }
        else
        {
            config &= ~0x02;
        }
    }

    div &= 0x7fff;
    I2CBus_Lock();              // Lock/wait
    if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
    {
        Sleep(1);
        if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
        {
            Sleep(1);
            if (!I2CBus_Write((BYTE) TunerDeviceWrite, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
            {
                ErrorBox("Tuner Device : Error Writing (1)");
                I2CBus_Unlock();    // Unlock
                return (FALSE);
            }
        }
    }
    if (!(bAck = I2CBus_Write(TunerDeviceWrite, GetTunerSetup()->config, config, TRUE)))
    {
        Sleep(1);
        if (!(bAck = I2CBus_Write(TunerDeviceWrite, GetTunerSetup()->config, config, TRUE)))
        {
            Sleep(1);
            if (!(bAck = I2CBus_Write(TunerDeviceWrite, GetTunerSetup()->config, config, TRUE)))
            {
                ErrorBox("Tuner Device : Error Writing (2)");
            }
        }
    }
    I2CBus_Unlock();            // Unlock
    if (!bAck)
        return FALSE;
    return TRUE;
}

