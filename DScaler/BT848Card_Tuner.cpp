/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Tuner.cpp,v 1.2 2001-11-02 16:30:07 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1.2.2  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.5  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.4  2001/08/17 16:35:13  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.3  2001/08/15 07:10:19  adcockj
// Fixed memory leak
//
// Revision 1.1.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.1.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
// Revision 1.1  2001/08/13 12:05:12  adcockj
// Updated range for contrast and saturation
// Added more code for new driver interface
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BT848Card.h"
#include "BT848_Defines.h"


const TTunerSetup Tuners[TUNER_LASTONE] =
{
    // TUNER_ABSENT
    { 
        "NoTuner", NOMFTR, NOTTYPE,     
        0, 0, 0x00, 0x00, 0x00, 0x00, 0x00 
    },
    // TUNER_PHILIPS_PAL_I
    { 
        "PHILIPS PAL_I", PHILIPS, PAL_I, 
         16*140.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_PHILIPS_NTSC
    { 
        "PHILIPS NTSC", PHILIPS, NTSC, 
        16*157.25, 16*451.25, 0xA0, 0x90, 0x30, 0x8e, 732
    },
    // TUNER_PHILIPS_SECAM
    { 
        "PHILIPS SECAM", PHILIPS, SECAM, 
        16*168.25, 16*447.25, 0xA7, 0x97, 0x37, 0x8e, 623
    },
    // TUNER_PHILIPSFY5_PAL     
    { 
        "PHILIPS PAL", PHILIPS, PAL, 
        16*168.25, 16*447.25, 0xA0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_TEMIC_4002FH5_PAL
    { 
        "Temic 4002 FH5 PAL", TEMIC, PAL, 
        16*140.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 623
    },
    // TUNER_TEMIC_4032FY5_NTSC
    {
        "Temic 4036 FY5 NTSC", TEMIC, NTSC, 
        16*157.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 732
    },
    // TUNER_TEMIC_4062FY5_PAL_I
    {
        "Temic PAL_I (4062 FY5)", TEMIC, PAL_I, 
        16*170.00, 16*450.00, 0x02, 0x04, 0x01, 0x8e, 623
    },
    // TUNER_TEMIC_4036FY5_NTSC
    {
        "Temic 4036 FY5 NTSC", TEMIC, NTSC, 
        16*157.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 732
    },
    // TUNER_ALPS_TSBH1_NTSC    
    {
        "ALPS HSBH1", TEMIC, NTSC, 
        16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732
    },
    // TUNER_ALPS_TSBE1_PAL     
    {
        "ALPS TSBE1", TEMIC, PAL, 
        16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732
    },
    // TUNER_ALPS_TSBB5_PAL_I
    {
        "ALPS TSBB5", ALPS, PAL_I, 
        16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 632
    },
    // TUNER_ALPS_TSBE5_PAL 
    {
        "ALPS TSBE5", ALPS, PAL, 
        16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 622
    },
    // TUNER_ALPS_TSBC5_PAL
    {
        "ALPS TSBC5", ALPS, PAL, 
        16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 608
    },
    // TUNER_TEMIC_4006FH5_PAL
    {
        "Temic 4006FH5", TEMIC, PAL_I, 
        16*170.00,16*450.00, 0xa0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_PHILIPS_1236D_NTSC_INPUT1
    {
        "PHILIPS 1236D ATSC/NTSC Input 1", PHILIPS, NTSC, 
        2516, 7220, 0xA3, 0x93, 0x33, 0xCE, 732
    },
    // TUNER_PHILIPS_1236D_NTSC_INPUT2
    {
        "PHILIPS 1236D ATSC/NTSC Input 2", PHILIPS, NTSC, 
        2516, 7220, 0xA2, 0x92, 0x32, 0xCE, 732
    },
    // TUNER_ALPS_TSCH6_NTSC
    {
        "ALPS TSCH6",ALPS,NTSC,
        16*137.25, 16*385.25, 0x14, 0x12, 0x11, 0x8e, 732
    },
    // TUNER_TEMIC_4016FY5_PAL
    {
        "Temic PAL_DK (4016 FY5)",TEMIC,PAL,
        16*136.25, 16*456.25, 0xa0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_PHILIPS_MK2_NTSC
    {
        "PHILIPS NTSC_M (MK2)",PHILIPS,NTSC,
        16*160.00,16*454.00,0xa0,0x90,0x30,0x8e,732
    },
    // TUNER_TEMIC_4066FY5_PAL_I
    {
        "Temic PAL_I (4066 FY5)", TEMIC, PAL_I,
        16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4006FN5_PAL
    {
        "Temic PAL* auto (4006 FN5)", TEMIC, PAL,
        16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4009FR5_PAL
    { 
        "Temic PAL (4009 FR5)", TEMIC, PAL,
        16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4039FR5_NTSC
    {
        "Temic NTSC (4039 FR5)", TEMIC, NTSC,
        16*158.00, 16*453.00, 0xa0,0x90,0x30,0x8e,732
    },
    // TUNER_TEMIC_4046FM5_MULTI
    { 
        "Temic PAL/SECAM multi (4046 FM5)", TEMIC, PAL,
        16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_PHILIPS_PAL_DK
    { 
        "PHILIPS PAL_DK", PHILIPS, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_PHILIPS_MULTI
    { 
        "PHILIPS PAL/SECAM multi (FQ1216ME)", PHILIPS, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_I001D_PAL_I
    { 
        "LG PAL_I+FM (TAPC-I001D)", LGINNOTEK, PAL_I,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_I701D_PAL_I
    { 
        "LG PAL_I (TAPC-I701D)", LGINNOTEK, PAL_I,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_R01F_NTSC
    { 
        "LG NTSC+FM (TPI8NSR01F)", LGINNOTEK, NTSC,
        16*210.00,16*497.00,0xa0,0x90,0x30,0x8e,732
    },
    // TUNER_LG_B01D_PAL
    { 
        "LG PAL_BG+FM (TPI8PSB01D)", LGINNOTEK, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_B11D_PAL
    { 
        "LG PAL_BG (TPI8PSB11D)", LGINNOTEK, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4009FN5_PAL
    { 
        "Temic PAL* auto + FM (4009 FN5)", TEMIC, PAL,
        16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623
    }
};

void CBT848Card::SetTunerType(eTunerId TunerType)
{
    m_TunerType = TunerType;
}

eTunerId CBT848Card::GetTunerType()
{
    return m_TunerType;
}

const char* CBT848Card::GetTunerStatus()
{
    return m_TunerStatus;
}

BOOL CBT848Card::InitTuner()
{
    BYTE j;

    j = 0xc0;
    m_TunerDevice = j;

    while ((j <= 0xce) && (I2C_AddDevice(j) == FALSE))
    {
        j++;
        m_TunerDevice = j;
    }

    if (j > 0xce)
    {
        m_TunerDevice = 0;
        return FALSE;
    }
    sprintf(m_TunerStatus, "Tuner I2C-Bus I/O 0x%02x", j);
    return TRUE;
}

// Set TSA5522 synthesizer frequency
 
BOOL CBT848Card::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat, eTunerId TunerId)
{
    BYTE config;
    WORD div;
    BOOL bAck;

    if (m_TunerDevice == 0 || m_TunerType <= 0 || m_TunerType >= TUNER_LASTONE)
    {
        return FALSE;
    }

    if (FrequencyId < Tuners[m_TunerType].thresh1)
    {
        config = Tuners[m_TunerType].VHF_L;
    }
    else if (FrequencyId < Tuners[m_TunerType].thresh2)
    {
        config = Tuners[m_TunerType].VHF_H;
    }
    else
    {
        config = Tuners[m_TunerType].UHF;
    }

    div = FrequencyId + Tuners[m_TunerType].IFPCoff;

    // handle Mode on Philips SECAM tuners
    // they can also recive PAL if the Mode is set properly
    if(TunerId == TUNER_PHILIPS_SECAM)
    {
        if(VideoFormat == FORMAT_SECAM)
        {
            config |= 0x02;
        }
        else
        {
            config &= ~0x02;
        }
    }

    div &= 0x7fff;
    I2C_Lock();
    if (!I2C_Write(m_TunerDevice, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
    {
        Sleep(1);
        if (!I2C_Write(m_TunerDevice, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
        {
            Sleep(1);
            if (!I2C_Write(m_TunerDevice, (BYTE) ((div >> 8) & 0x7f), (BYTE) (div & 0xff), TRUE))
            {
                ErrorBox("Tuner Device : Error Writing (1)");
                I2C_Unlock();
                return FALSE;
            }
        }
    }
    if (!(bAck = I2C_Write(m_TunerDevice, Tuners[m_TunerType].config, config, TRUE)))
    {
        Sleep(1);
        if (!(bAck = I2C_Write(m_TunerDevice, Tuners[m_TunerType].config, config, TRUE)))
        {
            Sleep(1);
            if (!(bAck = I2C_Write(m_TunerDevice, Tuners[m_TunerType].config, config, TRUE)))
            {
                ErrorBox("Tuner Device : Error Writing (2)");
            }
        }
    }
    I2C_Unlock();
    return bAck;
}

