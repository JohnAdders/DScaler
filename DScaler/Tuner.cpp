/////////////////////////////////////////////////////////////////////////////
// $Id: Tuner.cpp,v 1.8 2001-10-19 18:44:25 ittarnavsky Exp $
/////////////////////////////////////////////////////////////////////////////
// Code sourced from bttv linux driver tuner.c file
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// Copyright (c) 1999,2000 Gerd Knorr <kraxel@goldbach.in-berlin.de>
// Copyright (C) 1997 Markus Schroeder (schroedm@uni-duesseldorf.de)
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.6  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

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
    TunerDeviceRead = j | 1;
    TunerDeviceWrite = j;

    while ((j <= 0xce) && (I2CBus_AddDevice((BYTE) j) == FALSE))
    {
        j++;
        TunerDeviceRead = j | 1;
        TunerDeviceWrite = j;
    }

    if (j > 0xce)
    {
        return (FALSE);
    }
    sprintf(TunerStatus, "Tuner I2C-Bus I/O 0x%02x/0x%02x", TunerDeviceRead, TunerDeviceWrite);
    return (TRUE);
}


/********************************************************************
    Support for the MT2032 tuner
********************************************************************/
#define MT2032_OPTIMIZE_VCO 1   // perform VCO optimizations
static int  MT2032_XOGC = 4;    // holds the value of XOGC register after init
static int  MT2032_Initialized = 0; // MT2032 needs intialization?

static BYTE MT2032_ReadRegister(BYTE regNum)
{
    I2CBus_Write(TunerDeviceWrite, regNum, 0, FALSE);
    return I2CBus_Read(TunerDeviceRead);
}

static void MT2032_WriteRegister(BYTE regNum, BYTE data)
{
    I2CBus_Write(TunerDeviceWrite, regNum, data, TRUE);
}

static void MT2032_Init(void)
{
    unsigned char   buf[21];
    int             xogc, xok = 0;

    for (int i = 0; i < 21; i++)
    {
        buf[i] = MT2032_ReadRegister(i);
    }

    /* Initialize Registers per spec. */
    MT2032_WriteRegister(2, 0xff);
    MT2032_WriteRegister(3, 0x0f);
    MT2032_WriteRegister(4, 0x1f);
    MT2032_WriteRegister(6, 0xe4);
    MT2032_WriteRegister(7, 0x8f);
    MT2032_WriteRegister(8, 0xc3);
    MT2032_WriteRegister(9, 0x4e);
    MT2032_WriteRegister(10, 0xec);
    MT2032_WriteRegister(13, 0x32);

    /* Adjust XOGC (register 7), wait for XOK */
    xogc = 7;
    do
    {
        Sleep(10);
        xok = MT2032_ReadRegister(0x0e) & 0x01;
        if (xok == 1)
        {
            break;
        }
        xogc--;
        if (xogc == 3)
        {
            xogc = 4;   /* min. 4 per spec */
            break;
        }
        MT2032_WriteRegister(7, 0x88 + xogc);
    } while (xok != 1);
    MT2032_XOGC = xogc;
    MT2032_Initialized = 0;
}

static int MT2032_SpurCheck(int f1, int f2, int spectrum_from, int spectrum_to)
{
    int n1 = 1, n2, f;

    f1 = f1 / 1000;     /* scale to kHz to avoid 32bit overflows */
    f2 = f2 / 1000;
    spectrum_from /= 1000;
    spectrum_to /= 1000;

    do
    {
        n2 = -n1;
        f = n1 * (f1 - f2);
        do
        {
            n2--;
            f = f - f2;
            if ((f > spectrum_from) && (f < spectrum_to))
            {
                return 1;
            }
        } while ((f > (f2 - spectrum_to)) || (n2 > -5));
        n1++;
    } while (n1 < 5);

    return 0;
}

static int MT2032_ComputeFreq
(
    int             rfin,
    int             if1,
    int             if2,
    int             spectrum_from,
    int             spectrum_to,
    unsigned char   *buf,
    int             *ret_sel,
    int             xogc
)   /* all in Hz */
{
    int fref, lo1, lo1n, lo1a, s, sel, lo1freq, desired_lo1, desired_lo2, lo2, lo2n, lo2a, lo2num,
        lo2freq;

    fref = 5250 * 1000; /* 5.25MHz */

    /* per spec 2.3.1 */
    desired_lo1 = rfin + if1;
    lo1 = (2 * (desired_lo1 / 1000) + (fref / 1000)) / (2 * fref / 1000);
    lo1freq = lo1 * fref;
    desired_lo2 = lo1freq - rfin - if2;

    /* per spec 2.3.2 */
    for (int nLO1adjust = 1; nLO1adjust < 3; nLO1adjust++)
    {
        if (!MT2032_SpurCheck(lo1freq, desired_lo2, spectrum_from, spectrum_to))
        {
            break;
        }

        if (lo1freq < desired_lo1)
        {
            lo1 += nLO1adjust;
        }
        else
        {
            lo1 -= nLO1adjust;
        }

        lo1freq = lo1 * fref;
        desired_lo2 = lo1freq - fref - if2;
    }

    /* per spec 2.3.3 */
    s = lo1freq / 1000 / 1000;

    if (MT2032_OPTIMIZE_VCO)
    {
        if (s > 1890)
        {
            sel = 0;
        }
        else if (s > 1720)
        {
            sel = 1;
        }
        else if (s > 1530)
        {
            sel = 2;
        }
        else if (s > 1370)
        {
            sel = 3;
        }
        else
        {
            sel = 4;    /* >1090 */
        }
    }
    else
    {
        if (s > 1790)
        {
            sel = 0;    /* <1958 */
        }
        else if (s > 1617)
        {
            sel = 1;
        }
        else if (s > 1449)
        {
            sel = 2;
        }
        else if (s > 1291)
        {
            sel = 3;
        }
        else
        {
            sel = 4;    /* >1090 */
        }
    }

    *ret_sel = sel;

    /* per spec 2.3.4 */
    lo1n = lo1 / 8;
    lo1a = lo1 - (lo1n * 8);
    lo2 = desired_lo2 / fref;
    lo2n = lo2 / 8;
    lo2a = lo2 - (lo2n * 8);
    lo2num = ((desired_lo2 / 1000) % (fref / 1000)) * 3780 / (fref / 1000); /* scale to fit in 32bit arith */
    lo2freq = (lo2a + 8 * lo2n) * fref + lo2num * (fref / 1000) / 3780 * 1000;

    if (lo1a < 0 ||  
        lo1a > 7 ||  
        lo1n < 17 ||  
        lo1n > 48 ||  
        lo2a < 0 ||  
        lo2a > 7 ||  
        lo2n < 17 ||  
        lo2n > 30)
    {
        return -1;
    }

    /* set up MT2032 register map for transfer over i2c */
    buf[0] = lo1n - 1;
    buf[1] = lo1a | (sel << 4);
    buf[2] = 0x86;                  /* LOGC */
    buf[3] = 0x0f;                  /* reserved */
    buf[4] = 0x1f;
    buf[5] = (lo2n - 1) | (lo2a << 5);
    if (rfin > 400 * 1000 * 1000)
    {
        buf[6] = 0xe4;
    }
    else
    {
        buf[6] = 0xf4;              /* set PKEN per rev 1.2 */
    }

    buf[7] = 8 + xogc;
    buf[8] = 0xc3;                  /* reserved */
    buf[9] = 0x4e;                  /* reserved */
    buf[10] = 0xec;                 /* reserved */
    buf[11] = (lo2num & 0xff);
    buf[12] = (lo2num >> 8) | 0x80; /* Lo2RST */

    return 0;
}

static int MT2032_CheckLOLock(void)
{
    int t, lock = 0;
    for (t = 0; t < 10; t++)
    {
        lock = MT2032_ReadRegister(0x0e) & 0x06;
        if (lock == 6)
        {
            break;
        }
        Sleep(1);
    }
    return lock;
}

static int MT2032_OptimizeVCO(int sel, int lock)
{
    int tad1;

    tad1 = MT2032_ReadRegister(0x0f) & 0x07;

    if (tad1 == 0)
    {
        return lock;
    }
    if (tad1 == 1)
    {
        return lock;
    }
    if (tad1 == 2)
    {
        if (sel == 0)
        {
            return lock;
        }
        else
        {
            sel--;
        }
    }
    else
    {
        if (sel < 4)
        {
            sel++;
        }
        else
        {
            return lock;
        }
    }

    MT2032_WriteRegister(0x0f, sel);
    lock = MT2032_CheckLOLock();
    return lock;
}

static void MT2032_SetIFFreq(int rfin, int if1, int if2, int from, int to)
{
    unsigned char   buf[21];
    int             lint_try, ret, sel, lock = 0;

    for (int i = 0; i < 21; i++)
    {
        buf[i] = MT2032_ReadRegister(i);
    }

    ret = MT2032_ComputeFreq(rfin, if1, if2, from, to, &buf[0], &sel, MT2032_XOGC);
    if (ret < 0)
    {
        return;
    }

    /* send only the relevant registers per Rev. 1.2 */
    MT2032_WriteRegister(0, buf[0x00]);
    MT2032_WriteRegister(1, buf[0x01]);
    MT2032_WriteRegister(2, buf[0x02]);

    MT2032_WriteRegister(5, buf[0x05]);
    MT2032_WriteRegister(6, buf[0x06]);
    MT2032_WriteRegister(7, buf[0x07]);

    MT2032_WriteRegister(11, buf[0x0B]);
    MT2032_WriteRegister(12, buf[0x0C]);

    /* wait for PLLs to lock (per manual), retry LINT if not. */
    for (lint_try = 0; lint_try < 2; lint_try++)
    {
        lock = MT2032_CheckLOLock();

        if (MT2032_OPTIMIZE_VCO)
        {
            lock = MT2032_OptimizeVCO(sel, lock);
        }

        if (lock == 6)
        {
            break;
        }

        /* set LINT to re-init PLLs */
        MT2032_WriteRegister(7, 0x80 + 8 + MT2032_XOGC);
        Sleep(10);
        MT2032_WriteRegister(7, 8 + MT2032_XOGC);
    }

    MT2032_WriteRegister(2, 0x20);
}

static void MT2032_SetTVFreq(int freq, int norm)
{
    int if2, from, to;

    I2CBus_Lock();
    if (!MT2032_Initialized)
    {
        MT2032_Init();
    }

    /* signal bandwidth and picture carrier */
    if (norm == FORMAT_NTSC)
    {
        from = 40750 * 1000;
        to = 46750 * 1000;
        if2 = 45750 * 1000;
    }
    else
    {   /* PAL */
        from = 32900 * 1000;
        to = 39900 * 1000;
        if2 = 38900 * 1000;
    }

    MT2032_SetIFFreq(freq * 1000 / 16 * 1000, 1090 * 1000 * 1000, if2, from, to);
    I2CBus_Unlock();
}

// Set TSA5522 synthesizer frequency
 
BOOL Tuner_SetFrequency(int wFrequency)
{
    BYTE config;
    WORD div;
    BOOL bAck;

    if (GetTunerSetup() == NULL)
        return (TRUE);

    if (GetCardSetup()->TunerId == TUNER_MT2032) {
        MT2032_SetTVFreq(wFrequency, Setting_GetValue(BT848_GetSetting(TVFORMAT)));
        return (TRUE);
    }

    if (wFrequency < GetTunerSetup()->thresh1)
        config = GetTunerSetup()->VHF_L;
    else if (wFrequency < GetTunerSetup()->thresh2)
        config = GetTunerSetup()->VHF_H;
    else
        config = GetTunerSetup()->UHF;

    div = wFrequency + GetTunerSetup()->IFPCoff;

    // handle Mode on Philips SECAM tuners
    // they can also recive PAL if the Mode is set properly
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

