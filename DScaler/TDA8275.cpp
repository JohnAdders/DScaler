/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Atsushi Nakagawa.  All rights reserved.
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
//
// Sections in this code were ported from video4linux project's "tda8290c"
// file.  Copyright is unspecified.  List of contributers, as reported by
// CVS accounts follows: kraxel (revision 1.6)
//
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA8290.cpp CTDA8290 Implementation
*/

#include "stdafx.h"
#include "TDA8275.h"
#include "TDA8290.h"
#include "DebugLog.h"


// TDA8275 analog TV frequency dependent parameters (prgTab)
const CTDA8275::tProgramingParam CTDA8275::k_programmingTable[] =
{
    // LOmin,    LOmax,    spd,    BS,        BP,        CP,        GC3,    div1p5
    { 55,        62,        3,        2,        0,        0,        3,        1    },
    { 62,        66,        3,        3,        0,        0,        3,        1    },
    { 66,        76,        3,        1,        0,        0,        3,        0    },
    { 76,        84,        3,        2,        0,        0,        3,        0    },
    { 84,        93,        3,        2,        0,        0,        1,        0    },
    { 93,        98,        3,        3,        0,        0,        1,        0    },
    { 98,        109,    3,        3,        1,        0,        1,        0    },
    { 109,        123,    2,        2,        1,        0,        1,        1    },
    { 123,        133,    2,        3,        1,        0,        1,        1    },
    { 133,        151,    2,        1,        1,        0,        1,        0    },
    { 151,        154,    2,        2,        1,        0,        1,        0    },
    { 154,        181,    2,        2,        1,        0,        0,        0    },
    { 181,        185,    2,        2,        2,        0,        1,        0    },
    { 185,        217,    2,        3,        2,        0,        1,        0    },
    { 217,        244,    1,        2,        2,        0,        1,        1    },
    { 244,        265,    1,        3,        2,        0,        1,        1    },
    { 265,        302,    1,        1,        2,        0,        1,        0    },
    { 302,        324,    1,        2,        2,        0,        1,        0    },
    { 324,        370,    1,        2,        3,        0,        1,        0    },
    { 370,        454,    1,        3,        3,        0,        1,        0    },
    { 454,        493,    0,        2,        3,        0,        1,        1    },
    { 493,        530,    0,        3,        3,        0,        1,        1    },
    { 530,        554,    0,        1,        3,        0,        1,        0    },
    { 554,        604,    0,        1,        4,        0,        0,        0    },
    { 604,        696,    0,        2,        4,        0,        0,        0    },
    { 696,        740,    0,        2,        4,        1,        0,        0    },
    { 740,        820,    0,        3,        4,        0,        0,        0    },
    { 820,        865,    0,        3,        4,        1,        0,        0    }
};

// TDA8275A analog TV frequency dependent parameters (prgTab)
const CTDA8275::tProgramingParam2 CTDA8275::k_programmingTable2[] =
{
    // loMin,   loMax,  SVCO,   SPD,    SCR,    SBS,    GC3
    { 49,       56,     3,      4,      0,      0,      3   },
    { 56,       67,     0,      3,      0,      0,      3   },
    { 67,       81,     1,      3,      0,      0,      3   },
    { 81,       97,     2,      3,      0,      0,      3   },
    { 97,       113,    3,      3,      0,      1,      1   },
    { 113,      134,    0,      2,      0,      1,      1   },
    { 134,      154,    1,      2,      0,      1,      1   },
    { 154,      162,    1,      2,      0,      1,      1   },
    { 162,      183,    2,      2,      0,      1,      1   },
    { 183,      195,    2,      2,      0,      2,      1   },
    { 195,      227,    3,      2,      0,      2,      3   },
    { 227,      269,    0,      1,      0,      2,      3   },
    { 269,      325,    1,      1,      0,      2,      1   },
    { 325,      390,    2,      1,      0,      3,      3   },
    { 390,      455,    3,      1,      0,      3,      3   },
    { 455,      520,    0,      0,      0,      3,      1   },
    { 520,      538,    0,      0,      1,      3,      1   },
    { 538,      554,    1,      0,      0,      3,      1   },
    { 554,      620,    1,      0,      0,      4,      0   },
    { 620,      650,    1,      0,      1,      4,      0   },
    { 650,      700,    2,      0,      0,      4,      0   },
    { 700,      780,    2,      0,      1,      4,      0   },
    { 780,      820,    3,      0,      0,      4,      0   },
    { 820,      870,    3,      0,      1,      4,      0   },
    { 870,      911,    3,      0,      2,      4,      0   }
};

// TDA8275A DVB-T frequency dependent parameters (prgTab)
const CTDA8275::tProgramingParam3 CTDA8275::k_programmingTable3[] =
{
    // loMin,   loMax,  SVCO,   SPD,    SCR,    SBS,    GC3
    { 49,       57,     3,      4,      0,      0,      1   },
    { 57,       67,     0,      3,      0,      0,      1   },
    { 67,       81,     1,      3,      0,      0,      1   },
    { 81,       98,     2,      3,      0,      0,      1   },
    { 98,       114,    3,      3,      0,      1,      1   },
    { 114,      135,    0,      2,      0,      1,      1   },
    { 135,      154,    1,      2,      0,      1,      1   },
    { 154,      163,    1,      2,      0,      1,      1   },
    { 163,      183,    2,      2,      0,      1,      1   },
    { 183,      195,    2,      2,      0,      2,      1   },
    { 195,      228,    3,      2,      0,      2,      1   },
    { 228,      269,    0,      1,      0,      2,      1   },
    { 269,      290,    1,      1,      0,      2,      1   },
    { 290,      325,    1,      1,      0,      3,      1   },
    { 325,      390,    2,      1,      0,      3,      1   },
    { 390,      455,    3,      1,      0,      3,      1   },
    { 455,      520,    0,      0,      0,      3,      1   },
    { 520,      538,    0,      0,      1,      3,      1   },
    { 538,      550,    1,      0,      0,      3,      1   },
    { 550,      620,    1,      0,      0,      4,      0   },
    { 620,      650,    1,      0,      1,      4,      0   },
    { 650,      700,    2,      0,      0,      4,      0   },
    { 700,      780,    2,      0,      1,      4,      0   },
    { 780,      820,    3,      0,      0,      4,      0   },
    { 820,      870,    3,      0,      1,      4,      0   }
};

const CTDA8275::tStandardParam CTDA8275::k_standardParamTable[TDA8290_STANDARD_LASTONE] =
{
    // sgIFkHz, sgIFLPFilter
    { 5750,    1 },
    { 6750,    0 },
    { 7750,    0 },
    { 7750,    0 },
    { 7750,    0 },
    { 7750,    0 },
    { 1250,    0 },
    { 4750,    0 },  // FM Radio (this value is a guess)
};


CTDA8275::CTDA8275()
{
}

CTDA8275::~CTDA8275()
{
}

BOOL CTDA8275::InitializeTuner()
{
    WriteTDA8275Initialization();
    return TRUE;
}

eVideoFormat CTDA8275::GetDefaultVideoFormat()
{
    return VIDEOFORMAT_NTSC_M;
}

BOOL CTDA8275::HasRadio() const
{
    return TRUE;
}

BOOL CTDA8275::SetTVFrequency(long frequencyHz, eVideoFormat videoFormat)
{
    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->TunerSet(TRUE, videoFormat);
    }

    BOOL success = SetFrequency(frequencyHz, CTDA8290::GetTDA8290Standard(videoFormat));
    m_Frequency = frequencyHz;

    if (m_ExternalIFDemodulator != NULL)
    {
        m_ExternalIFDemodulator->TunerSet(FALSE, videoFormat);
    }

    return success;
}

BOOL CTDA8275::SetRadioFrequency(long frequencyHz)
{
    // This tuning of radio has not been tested AT ALL;
    return SetTVFrequency(frequencyHz, (eVideoFormat)(VIDEOFORMAT_LASTONE+1));
}

long CTDA8275::GetFrequency()
{
    return m_Frequency;
}

eTunerLocked CTDA8275::IsLocked()
{
    BYTE Result(0);
    if (ReadFromSubAddress(TDA8275_SR1, &Result, 1))
    {
        // LOCK = bit 22
        return (Result & 0x40) != 0 ? TUNER_LOCK_ON : TUNER_LOCK_OFF;
    }

    return TUNER_LOCK_NOTSUPPORTED;
}

eTunerAFCStatus CTDA8275::GetAFCStatus(long &nFreqDeviation)
{
    if (m_ExternalIFDemodulator != NULL)
    {
        return m_ExternalIFDemodulator->GetAFCStatus(nFreqDeviation);
    }
    return TUNER_AFC_NOTSUPPORTED;
}

BYTE CTDA8275::GetDefaultAddress() const
{
    return I2C_ADDR_TDA8275_1;
}

BOOL CTDA8275::IsTDA8275A()
{
    BYTE Result(0);

    // Read HID Bit's (18:21) : 0000 = TDA8275
    //                          0010 = TDA8275A

    if (ReadFromSubAddress(TDA8275_SR1, &Result, 1))
    {
        if ((Result & 0x3C) == 0x08)
        {
            LOG(1, "TDA8275: TDA8275A revision found.");
            return TRUE;
        }
        else
        {
            LOG(1, "TDA8275: Found.");
            return FALSE;
        }
    }

    LOG(0, "TDA8275: Error while detecting chip revision.");
    return FALSE;
}

BOOL CTDA8275::IsDvbMode()
{
    // \TODO
    return FALSE;
}

void CTDA8275::WriteTDA8275Initialization()
{
    // 2 TDA8275A Initialization
    if (IsTDA8275A())
    {
        WriteToSubAddress(TDA8275_DB1, 0x00);
        WriteToSubAddress(TDA8275_DB2, 0x00);
        WriteToSubAddress(TDA8275_DB3, 0x00);
        WriteToSubAddress(TDA8275_AB1, 0xAB);
        WriteToSubAddress(TDA8275_AB2, 0x3C);
        WriteToSubAddress(TDA8275A_IB1, 0x04);
        WriteToSubAddress(TDA8275A_AB3, 0x24);
        WriteToSubAddress(TDA8275A_IB2, 0xFF);
        WriteToSubAddress(TDA8275A_CB2, 0x40);
        WriteToSubAddress(TDA8275A_IB3, 0x00);
        WriteToSubAddress(TDA8275A_CB3, 0x3B);

        /*
        // These values come from the "2/ TDA8275A Initialization"
        // code in the data-sheet.  Those that weren't specified
        // there were substituted with default values from the
        // default column of the data-sheet.

        BYTE initializationBytes[13] = {
            // DB1,  DB2,  DB3,  CB1
               0x00, 0x00, 0x00, 0xDC,
            // BB,   AB1,  AB2,  IB1
               0x05, 0xAB, 0x3C, 0x04,
            // AB3,  IB2,  CB2,  IB3,  CB3
               0x24, 0xFF, 0x40, 0x00, 0x3B };

        WriteToSubAddress(TDA8275A_DB1, initializationBytes, 13);
        */
    }
    else
    {
        WriteToSubAddress(TDA8275_DB1, 0x00);
        WriteToSubAddress(TDA8275_DB2, 0x00);
        WriteToSubAddress(TDA8275_DB3, 0x40);
        WriteToSubAddress(TDA8275_AB3, 0x2A);
        WriteToSubAddress(TDA8275_GB, 0xFF);
        WriteToSubAddress(TDA8275_TB, 0x00);
        WriteToSubAddress(TDA8275_SDB3, 0x00);
        WriteToSubAddress(TDA8275_SDB4, 0x40);

        /*
        // These values come from the "2/ TDA827x Initialization"
        // code in the data-sheet.  Those that weren't specified
        // there were substituted with default values from the
        // default column of the data-sheet.

        BYTE initializationBytes[13] = {
            // DB1,  DB2,  DB3,  CB1
               0x00, 0x00, 0x40, 0x70,
            // BB,   AB1,  AB2,  AB3
               0x00, 0x83, 0x3F, 0x2A,
            // AB4,  GB,   TB,   SDB3, SDB4
               0x04, 0xFF, 0x00, 0x00, 0x40 };

        WriteToSubAddress(TDA8275_DB1, initializationBytes, 13);
        */
    }
}

BOOL CTDA8275::SetFrequency(long frequencyHz, eTDA8290Standard standard)
{
    BYTE sgIFLPFilter = k_standardParamTable[(int)standard].sgIFLPFilter;
    LONG sgIFHz = k_standardParamTable[(int)standard].sgIFkHz * 1000;

    // sgRFHz + sgIFHz
    LONG freqRFIFHz = frequencyHz + sgIFHz;

    // The data-sheet says:
    // N11toN0=round((2^spd)*Flo*1000000*(16MHz/(2^6))
    //
    // Then uses this code to get the n11ton0 value:
    // lgN11toN0 = Round((2 ^ prgTab(c, 3)) * (sgRFMHz + sgIFMHz) * 1000000 / (16000000 / 2 ^ 6))
    //
    // Notice the discrepancy with division of (16MHz/2^6).  'prgTab(c, 3)' is row->spd,
    // (sgRFMHz + sgIFMHz) is (freqRFIFHz / 1000000).

    // 0.5 is added for rounding.

    BOOL success = TRUE;

    if (IsTDA8275A())
    {
        if (IsDvbMode())
        {
            // For TDA8275A in DVB Mode
            const tProgramingParam3* row = k_programmingTable3;
            const tProgramingParam3* last = (const tProgramingParam3*)((size_t)row + sizeof(k_programmingTable3)) - 1;
            for ( ; row != last && freqRFIFHz > row->loMax * 1000000; row++) ;
            WORD n11ton0 = (WORD)((double)(1 << row->SPD) * ((double)freqRFIFHz / 250000) + 0.5);

            BYTE channelBytes[12];
            channelBytes[0]  = (n11ton0 >> 6) & 0x3F;
            channelBytes[1]  = (n11ton0 << 2) & 0xFC;
            channelBytes[2]  = 0x00;
            channelBytes[3]  = 0x16;
            channelBytes[4]  = (row->SPD << 5) | (row->SVCO << 3) | (row->SBS);
            channelBytes[5]  = 0x01 << 6 | (row->GC3 << 4) | 0x0B;
            channelBytes[6]  = 0x0C;
            channelBytes[7]  = 0x06;
            channelBytes[8]  = 0x24;
            channelBytes[9]  = 0xFF;
            channelBytes[10] = 0x60;
            channelBytes[11] = 0x00;
            channelBytes[12] = sgIFLPFilter ? 0x3B : 0x39; // 7MHz (US) / 9Mhz (Europe);

            if (!WriteToSubAddress(TDA8275_DB1, channelBytes, 12))
            {
                return FALSE;
            }

            // 2.2 Re-initialize PLL and gain path
            WriteToSubAddress(TDA8275_AB2, 0x3C);
            WriteToSubAddress(TDA8275A_CB2, 0x40);
            Sleep(2);
            WriteToSubAddress(TDA8275_CB1, (0x04 << 2) | (row->SCR >> 2));
            Sleep(550); // 550ms delay required.
            WriteToSubAddress(TDA8275_AB1, (0x02 << 6) | (row->GC3 << 4) | 0x0F);
        }
        else
        {
            // For TDA8275A in analog TV Mode
            const tProgramingParam2* row = k_programmingTable2;
            const tProgramingParam2* last = (const tProgramingParam2*)((size_t)row + sizeof(k_programmingTable2)) - 1;

            // Find the matching row of the programming table for this frequency.
            for ( ; row != last && freqRFIFHz > row->loMax * 1000000; row++) ;

            WORD n11ton0 = (WORD)((double)(1 << row->SPD) * ((double)freqRFIFHz / 250000) + 0.5);

            BYTE channelBytes[12];
            channelBytes[0]  = (n11ton0 >> 6) & 0x3F;
            channelBytes[1]  = (n11ton0 << 2) & 0xFC;
            channelBytes[2]  = 0x00;
            channelBytes[3]  = 0x16;
            channelBytes[4]  = (row->SPD << 5) | (row->SVCO << 3) | (row->SBS);
            channelBytes[5]  = 0x02 << 6 | (row->GC3 << 4) | 0x0B;
            channelBytes[6]  = 0x0C;
            channelBytes[7]  = 0x04;
            channelBytes[8]  = 0x20;
            channelBytes[9]  = 0xFF;
            channelBytes[10] = 0xE0;
            channelBytes[11] = 0x00;
            channelBytes[12] = sgIFLPFilter ? 0x3B : 0x39; // 7MHz (US) / 9Mhz (Europe);

            if (!WriteToSubAddress(TDA8275_DB1, channelBytes, 12))
            {
                return FALSE;
            }

            // 2.2 Re-initialize PLL and gain path
            WriteToSubAddress(TDA8275_AB2,  0x3C);
            WriteToSubAddress(TDA8275A_CB2, 0xC0);
            Sleep(2);
            WriteToSubAddress(TDA8275_CB1, (0x04 << 2) | (row->SCR >> 2));
            Sleep(550); // 550ms delay required.
            WriteToSubAddress(TDA8275_AB1, (0x02 << 6) | (row->GC3 << 4) | 0x0F);
            // 3 Enabling VSYNC only for analog TV
            WriteToSubAddress(TDA8275A_AB3, 0x28);
            WriteToSubAddress(TDA8275A_IB3, 0x01);
            WriteToSubAddress(TDA8275A_CB3, sgIFLPFilter ? 0x3B : 0x39); // 7MHz (US) / 9Mhz (Europe);
        }
    }
    else
    {
        // For TDA8275
        const tProgramingParam* row = k_programmingTable;
        const tProgramingParam* last = (const tProgramingParam*)((size_t)row + sizeof(k_programmingTable)) - 1;

        // Find the matching row of the programming table for this frequency.
        for ( ; row != last && freqRFIFHz > row->loMax * 1000000; row++) ;

        WORD n11ton0 = (WORD)((double)(1 << row->spd) * ((double)freqRFIFHz / 250000) + 0.5);

        BYTE channelBytes[7];
        channelBytes[0] = (n11ton0 >> 6) & 0x3F;
        channelBytes[1] = (n11ton0 << 2) & 0xFC;
        channelBytes[2] = 0x40;
        channelBytes[3] = sgIFLPFilter ? 0x72 : 0x52; // 7MHz (US) / 9Mhz (Europe)
        channelBytes[4] = (row->spd << 6)|(row->div1p5 << 5)|(row->BS << 3)|row->BP;
        channelBytes[5] = 0x8F | (row->GC3 << 4);
        channelBytes[6] = 0x8F;

        if (!WriteToSubAddress(TDA8275_DB1, channelBytes, 7) ||
            !WriteToSubAddress(TDA8275_AB4, 0x00))
        {
            return FALSE;
        }


        // 2.2 Re-initialize PLL and gain path
        success &= WriteToSubAddress(TDA8275_AB2, 0xBF);
        // This puts a delay that may not be necessary.
        //    success &= WriteToSubAddress(TDA8275_CB1, 0xD2);
        //    Sleep(1);
        //    success &= WriteToSubAddress(TDA8275_CB1, 0x56);
        //    Sleep(1); // Only 550us required.
        //    success &= WriteToSubAddress(TDA8275_CB1, 0x52);
        //    Sleep(550); // 550ms delay required.
        success &= WriteToSubAddress(TDA8275_CB1, 0x50|row->CP);

        // 3 Enabling VSYNC mode for AGC2
        WriteToSubAddress(TDA8275_AB2, 0x7F);
        WriteToSubAddress(TDA8275_AB4, 0x08);
    }

    return success;
}



