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
// Sections in this code were ported from video4linux project's _T("tda8290c")
// file.  Copyright is unspecified.  List of contributers, as reported by
// CVS accounts follows: kraxel (revision 1.6)
//
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA8290.cpp CTDA8290 Implementation
*/

#include "stdafx.h"
#include "TDA8290.h"
#include "DebugLog.h"


CTDA8290::CTDA8290()
{
}

CTDA8290::~CTDA8290()
{
}

CTDA8290* CTDA8290::CreateDetectedTDA8290(IN CI2CBus* i2cBus)
{
    CTDA8290* pTDA8290 = new CTDA8290();

    if (!pTDA8290->SetDetectedI2CAddress(i2cBus))
    {
        delete pTDA8290;
        return NULL;
    }
    return pTDA8290;
}

void CTDA8290::Init(BOOL bPreInit, eVideoFormat videoFormat)
{
    if (bPreInit)
    {
        /* Write the default value into the CLEAR register. Sets soft reset and
           standby to the _T("normal operation") setting. There's no video or audio
           without this. */
        WriteToSubAddress(TDA8290_CLEAR, 0x1);

        // Set TDA8290 gate for TDA8275 communication
        /* DEBUG:
            Sets
            GP2_CF (Bits 0-3) (GPIO2 Pin Configuration) to 1
            IICSW_ON to 1 (I2C Switch Command)
            IICSW_EN to 1 (Enable GPIO_1,2 as I2C switch) */
        WriteToSubAddress(TDA8290_GPIO2, 0xC1);
    }
    else
    {
        // V-Sync: positive pulse, line 15, width 128us (data-sheet initialization)
        WriteToSubAddress(TDA8290_V_SYNC_DEL, 0x6F);
        // Set GP0 to VSYNC.  The data-sheet says this but the v4l2 code
        // doesn't do this for some reason.
        WriteToSubAddress(TDA8290_GPIO1, 0x0B);
        // Set TDA8290 gate to prevent TDA8275 communication
        WriteToSubAddress(TDA8290_GPIO2, 0x81);
    }
}

void CTDA8290::TunerSet(BOOL bPreSet, eVideoFormat videoFormat)
{
    eTDA8290Standard standard = GetTDA8290Standard(videoFormat);

    if (bPreSet)
    {
        // Set video system standard.
        SetVideoSystemStandard(standard);

        // _T("1.1 Set input ADC register") (data-sheet)
        WriteToSubAddress(TDA8290_ADC, 0x14);
        // _T("1.2 Increasing IF AGC speed") (data-sheet)
        // This is the register's default value.
        WriteToSubAddress(TDA8290_IF_AGC_SET, 0x88);
        // _T("1.3 Set ADC headroom (TDA8290) to nominal") (data-sheet)
        if (standard == TDA8290_STANDARD_L || standard == TDA8290_STANDARD_L2)
        {
            // 9dB ADC headroom if standard is L or L'
            WriteToSubAddress(TDA8290_ADC_HEADR, 0x2);
        }
        else
        {
            // 6dB ADC headroom
            WriteToSubAddress(TDA8290_ADC_HEADR, 0x4);
        }

        // 1.4 Set picture carrier PLL BW (TDA8290) to nominal (data-sheet)
        // This is the register's default value.
        WriteToSubAddress(TDA8290_PC_PLL_FUNC, 0x47);

        // The v4l code does this instead of 1.2, 1.3 and 1.4
        // WriteToSubAddress(TDA8290_CLEAR, 0x00);

        // Set TDA8290 gate for TDA8275 communication
        WriteToSubAddress(TDA8290_GPIO2, 0xC1);
    }
    else
    {
        BYTE readByte = 0x00;

        // 4 Switching AGCF gain to keep 8dB headroom for RF power variations
        if ((ReadFromSubAddress(TDA8290_IF_AGC_STAT, &readByte, 1) && readByte > 155 ||
            ReadFromSubAddress(TDA8290_AFC_REG, &readByte, 1) && (readByte & 0x80) == 0) &&
            ReadFromSubAddress(TDA8290_ADC_SAT, &readByte, 1) && readByte < 20)
        {
            // 1Vpp for TDA8290 ADC
            WriteToSubAddress(TDA8290_ADC, 0x54);
            Sleep(100);

            // This cannot be done because there's no interface from here to the TDA8275.
            //if ((ReadFromSubAddress(TDA8290_IF_AGC_STAT, &readByte, 1) && readByte > 155 ||
            //    ReadFromSubAddress(TDA8290_AFC_REG, &readByte, 1) && (readByte & 0x80) == 0))
            //{
            //    // AGCF gain is increased to 10-30dB.
            //    TDA8275->WriteToSubAddress(TDA8275_AB4, 0x08);
            //    Sleep(100);

                if ((ReadFromSubAddress(TDA8290_IF_AGC_STAT, &readByte, 1) && readByte > 155 ||
                    ReadFromSubAddress(TDA8290_AFC_REG, &readByte, 1) && (readByte & 0x80) == 0))
                {
                    // 12dB ADC headroom
                    WriteToSubAddress(TDA8290_ADC_HEADR, 0x1);
                    // 70kHz PC PLL BW to counteract striping in picture
                    WriteToSubAddress(TDA8290_PC_PLL_FUNC, 0x27);
                    // Wait for IF AGC
                    Sleep(100);
                }
            //}
        }

        // 5/ RESET for L/L' deadlock
        if (standard == TDA8290_STANDARD_L || standard == TDA8290_STANDARD_L2)
        {
            if (ReadFromSubAddress(TDA8290_ADC_SAT, &readByte, 1) && readByte > 20 ||
                ReadFromSubAddress(TDA8290_AFC_REG, &readByte, 1) && (readByte & 0x80) == 0)
            {
                // Reset AGC integrator
                WriteToSubAddress(TDA8290_AGC_FUNC, 0x0B);
                Sleep(40);
                // Normal mode
                WriteToSubAddress(TDA8290_AGC_FUNC, 0x09);
            }
        }

        // 6/ Set TDA8290 gate to prevent TDA8275 communication
        WriteToSubAddress(TDA8290_GPIO2, 0x81);
        // 7/ IF AGC control loop bandwidth
        WriteToSubAddress(TDA8290_IF_AGC_SET, 0x81);
    }
}

BOOL CTDA8290::Detect()
{
    BYTE readBuffer;

    // I'm not sure about this.  Maybe the value of identify needs to be tested
    // too, and maybe it's necessary to perform read tests on other registers.
    if (!ReadFromSubAddress(TDA8290_IDENTITY, &readBuffer, 1))
    {
        LOG(1, _T("TDA8290: not detected"));
        return FALSE;
    }

    LOG(1, _T("TDA8290: $1F = 02x"), readBuffer);
    return TRUE;
}

eTunerAFCStatus CTDA8290::GetAFCStatus(long &nFreqDeviation)
{
    // Bits: 0..6 := AFC readout, 7 := 1=PLL locked to picture carrier
    BYTE afc_reg = 0x00;

    if (ReadFromSubAddress(TDA8290_AFC_REG, &afc_reg, 1))
    {
        // I couldn't find it specified how to interpret the AFC readout.  It says in
        // the data-sheet:
        //
        // "- Wide TDA8290 AFC window (-840...830 kHz) and very accurate AFC information
        // (13.2 kHz step) provided via I2C for channel fine tuning."
        //
        // And:
        // "The second one is the high precise AFC readout (7 bit) which delivers the IF
        // frequency deviation from nominal.  The AFC information is valid at [-840...830 kHz]
        // around the nominal frequency, ..."
        //
        // I guess the 13.2 kHz step is a rounded approximation because (13.2 x 127 - 840kHz)
        // doesn't work out exactly to 830 kHz.  Using this reasoning, the step value is
        // (830 + 840) / 127 = 13.1496.......  I'm also guessing it's not in 2's complement
        // because there's no way to get step x readout to fall on zero.

        // Deviation in kHz (need Hz): AFC readout x ((830kHz + 840kHz) / 127) - 840kHz.
        // (0.5 is added for rounding.)
        nFreqDeviation = (long)((afc_reg & 0x7F) * ((double)1670000 / 127) - 840000 + 0.5);

        return (afc_reg & 0x80) != 0 ? TUNER_AFC_CARRIER : TUNER_AFC_NOCARRIER;
    }
    return TUNER_AFC_NOTSUPPORTED;
}

void CTDA8290::SetVideoSystemStandard(eTDA8290Standard standard)
{
    const BYTE sgStandard[TDA8290_STANDARD_LASTONE] = { 1, 2, 4, 8, 16, 32, 64, 0 };

    if (standard == TDA8290_STANDARD_RADIO)
    {
        return;
    }

    // Bits: 0..6 := standard, 7 := expert mode
    WriteToSubAddress(TDA8290_STANDARD_REG, sgStandard[(int)standard]);
    // Activate expert mode.  I think it might need to be broken into two
    // calls like this so the first call sets everything up in easy mode
    // before expert mode is switched on.
    WriteToSubAddress(TDA8290_STANDARD_REG, sgStandard[(int)standard]|0x80);
}

eTDA8290Standard CTDA8290::GetTDA8290Standard(eVideoFormat videoFormat)
{
    eTDA8290Standard standard = TDA8290_STANDARD_MN;

    switch (videoFormat)
    {
    case VIDEOFORMAT_PAL_B:
    case VIDEOFORMAT_SECAM_B:
        standard = TDA8290_STANDARD_B;
        break;
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_H:
    case VIDEOFORMAT_PAL_N:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
        standard = TDA8290_STANDARD_GH;
        break;
    case VIDEOFORMAT_PAL_I:
        standard = TDA8290_STANDARD_I;
        break;
    case VIDEOFORMAT_PAL_D:
    case VIDEOFORMAT_SECAM_D:
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
        standard = TDA8290_STANDARD_DK;
        break;
    case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
        standard = TDA8290_STANDARD_L;
        break;
    case VIDEOFORMAT_PAL_60:
        // Unsupported
        break;
    case VIDEOFORMAT_PAL_M:
    case VIDEOFORMAT_PAL_N_COMBO:
    case VIDEOFORMAT_NTSC_M:
        standard = TDA8290_STANDARD_MN;
        break;
    case VIDEOFORMAT_NTSC_50:
    case VIDEOFORMAT_NTSC_M_Japan:
        standard = TDA8290_STANDARD_MN;
        break;
    // This value is used among ITuner and IExternalIFDemodulator for radio.
    case (VIDEOFORMAT_LASTONE+1):
        standard = TDA8290_STANDARD_RADIO;
        break;
    }

    return standard;
}

