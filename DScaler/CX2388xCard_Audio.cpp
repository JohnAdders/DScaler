/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Audio.cpp,v 1.4 2002-11-28 18:07:37 adcockj Exp $
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2002/11/27 17:41:57  adcockj
// Fixed setting PLL registers in DumpRegister
//
// Revision 1.2  2002/11/25 12:20:33  adcockj
// Changed audio init
//
// Revision 1.1  2002/10/31 15:55:48  adcockj
// Moved audio code from Connexant dTV version
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"

void CCX2388xCard::AudioInit(eVideoFormat Format)
{
    // \todo sort this out
    // most of what's below is probably rubbish
    switch (Format)
    {
    case VIDEOFORMAT_PAL_B:
    case VIDEOFORMAT_PAL_H:
    case VIDEOFORMAT_PAL_M:
        AudioInitA2();
        break;

    case VIDEOFORMAT_PAL_D:
    case VIDEOFORMAT_PAL_G:
    case VIDEOFORMAT_PAL_I:
        AudioInitNICAM();
        break;

    case VIDEOFORMAT_PAL_N:
    case VIDEOFORMAT_PAL_N_COMBO:
        AudioInitA2();
        break;

    case VIDEOFORMAT_SECAM_B:
    case VIDEOFORMAT_SECAM_D:
    case VIDEOFORMAT_SECAM_G:
    case VIDEOFORMAT_SECAM_H:
    case VIDEOFORMAT_SECAM_K:
    case VIDEOFORMAT_SECAM_K1:
    case VIDEOFORMAT_SECAM_L:
    case VIDEOFORMAT_SECAM_L1:
        AudioInitNICAM();
        break;

    case VIDEOFORMAT_NTSC_M:
        AudioInitBTSC();
        break;
    case VIDEOFORMAT_NTSC_M_Japan:
        AudioInitEIAJ();
        break;

    case VIDEOFORMAT_PAL_60:
    case VIDEOFORMAT_NTSC_50:
    default:
        AudioInitA2();
        break;
    }
}

void CCX2388xCard::SetAudioMute()
{
    DWORD dwval;

    // Mute the audio
    dwval = ReadDword(AUD_VOL_CTL) & 0x1FF;
    dwval |= 0x100;
    WriteDword(AUD_VOL_CTL,dwval);
}

void CCX2388xCard::SetAudioUnMute()
{
    DWORD dwval;
    // Unmute the audio
    dwval = ReadDword(AUD_VOL_CTL) & 0x0FF;
    WriteDword(AUD_VOL_CTL,dwval);
}


void CCX2388xCard::AudioInitDMA()
{
    WriteDword(MO_AUDD_LNGTH,SRAM_FIFO_AUDIO_BUFFER_SIZE);
    //WriteDword(MO_AUDR_LNGTH,SRAM_FIFO_AUDIO_BUFFER_SIZE);
    WriteDword(MO_AUD_DMACNTRL, 0x00000003);
}

void CCX2388xCard::AudioInitBTSC()
{
    // initialize BTSC
    WriteDword(AUD_INIT,                 0x0001);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);
    
    // These dbx values should be right....
    //WriteDword(AUD_DBX_IN_GAIN,          0x6dc0);
    //WriteDword(AUD_DBX_WBE_GAIN,         0x4003);
    //WriteDword(AUD_DBX_SE_GAIN,          0x7030);
    
    // ....but these values work a lot better
    WriteDword(AUD_DBX_IN_GAIN,          0x92c0);
    WriteDword(AUD_DBX_WBE_GAIN,         0x3e83);
    WriteDword(AUD_DBX_SE_GAIN,          0x854a);
    WriteDword(AUD_OUT1_SHIFT,           0x0007);
    WriteDword(AUD_PHASE_FIX_CTL,        0x0020);
    WriteDword(AUD_RATE_ADJ1,            0x0100);
    WriteDword(AUD_RATE_ADJ2,            0x0200);
    WriteDword(AUD_RATE_ADJ3,            0x0300);
    WriteDword(AUD_RATE_ADJ4,            0x0400);
    WriteDword(AUD_RATE_ADJ5,            0x0500);
    WriteDword(AUD_POLY0_DDS_CONSTANT,   0x121116);

    // turn down gain to iir4's...
    WriteDword(AUD_IIR4_0_SHIFT,         0x0006);
    WriteDword(AUD_IIR4_1_SHIFT,         0x0006);
    WriteDword(AUD_IIR4_2_SHIFT,         0x0006);
    
    // ...and turn up iir3 gains
    WriteDword(AUD_IIR3_0_SHIFT,         0x0000);
    WriteDword(AUD_IIR3_1_SHIFT,         0x0000);

    // Completely ditch AFC feedback
    WriteDword(AUD_DCOC_0_SRC,           0x0021);
    WriteDword(AUD_DCOC_1_SRC,           0x001a);
    WriteDword(AUD_DCOC1_SHIFT,          0x0000);
    WriteDword(AUD_DCOC_1_SHIFT_IN0,     0x000a);
    WriteDword(AUD_DCOC_1_SHIFT_IN1,     0x0008);
    WriteDword(AUD_DCOC_PASS_IN,         0x0000);
    WriteDword(AUD_IIR1_4_SEL,           0x0023);

    // setup Audio PLL
    WriteDword(AUD_PLL_PRESCALE,         0x0002);
    WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);  // Causes a pop every time

    ::Sleep(100);

    // adjust volume to max, unmute
    WriteDword(AUD_VOL_CTL,0x0000);

}

void CCX2388xCard::AudioInitEIAJ()
{
    // EIAJ stereo -> RAM -> DAC standard setup
    // xtal = 28.636 MHz

    // initialize EIAJ
    WriteDword(AUD_INIT,                 0x0002);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);
    // fix pilot detection
    WriteDword(AUD_HP_PROG_IIR4_1,       0x0019);
    WriteDword(AUD_IIR4_0_CA0,           0x000392ad);
    WriteDword(AUD_IIR4_0_CA1,           0x00007543);
    WriteDword(AUD_IIR4_0_CA2,           0x00030dc3);
    WriteDword(AUD_IIR4_0_CB0,           0x00006439);
    WriteDword(AUD_IIR4_0_CB1,           0x00031e98);
    WriteDword(AUD_IIR4_1_CA0,           0x00004137);
    WriteDword(AUD_IIR4_1_CA1,           0x0000692d);
    WriteDword(AUD_IIR4_1_CA2,           0x0000bca5);
    WriteDword(AUD_IIR4_1_CB0,           0x00003bd4);
    WriteDword(AUD_IIR4_1_CB1,           0x000097f3);
    WriteDword(AUD_IIR4_2_CA0,           0x00004137);
    WriteDword(AUD_IIR4_2_CA1,           0x0000692d);
    WriteDword(AUD_IIR4_2_CA2,           0x0000bca5);
    WriteDword(AUD_IIR4_2_CB0,           0x00003bd4);
    WriteDword(AUD_IIR4_2_CB1,           0x000097f3);
    WriteDword(AUD_DN2_FREQ,             0x00003d71);

    // turn down gain to iir4's...
    WriteDword(AUD_IIR4_0_SHIFT,         0x0007);
    WriteDword(AUD_IIR4_1_SHIFT,         0x0000);
    WriteDword(AUD_IIR4_2_SHIFT,         0x0006);

    // fast-in, fast-out, stereo & dual
    WriteDword(AUD_THR_FR,               0x0000);
    WriteDword(AUD_PILOT_BQD_1_K0,       0x00008000);
    WriteDword(AUD_PILOT_BQD_1_K1,       0x00000000);
    WriteDword(AUD_PILOT_BQD_1_K2,       0x00000000);
    WriteDword(AUD_PILOT_BQD_1_K3,       0x00000000);
    WriteDword(AUD_PILOT_BQD_1_K4,       0x00000000);
    WriteDword(AUD_PILOT_BQD_2_K0,       0x00c00000);
    WriteDword(AUD_PILOT_BQD_2_K1,       0x00000000);
    WriteDword(AUD_PILOT_BQD_2_K2,       0x00000000);
    WriteDword(AUD_PILOT_BQD_2_K3,       0x00000000);
    WriteDword(AUD_PILOT_BQD_2_K4,       0x00000000);
    WriteDword(AUD_C2_UP_THR,            0x2600);
    WriteDword(AUD_C2_LO_THR,            0x0c00);
    WriteDword(AUD_C1_UP_THR,            0x3400);
    WriteDword(AUD_C1_LO_THR,            0x2c00);
    WriteDword(AUD_MODE_CHG_TIMER,       0x0050);
    WriteDword(AUD_START_TIMER,          0x0200);
    WriteDword(AUD_AFE_16DB_EN,          0x0001);
    WriteDword(AUD_CORDIC_SHIFT_0,       0x0006);

    // slow-in, fast-out stereo only
    //WriteDword(AUD_THR_FR,               0x0000);
    //WriteDword(AUD_PILOT_BQD_1_K0,       0x0000c000);
    //WriteDword(AUD_PILOT_BQD_1_K1,       0x00000000);
    //WriteDword(AUD_PILOT_BQD_1_K2,       0x00000000);
    //WriteDword(AUD_PILOT_BQD_1_K3,       0x00000000);
    //WriteDword(AUD_PILOT_BQD_1_K4,       0x00000000);
    //WriteDword(AUD_PILOT_BQD_2_K0,       0x00c00000);
    //WriteDword(AUD_C1_UP_THR,            0x0120);
    //WriteDword(AUD_C1_LO_THR,            0x0114);
    //WriteDword(AUD_C2_UP_THR,            0x0000);
    //WriteDword(AUD_C2_LO_THR,            0x0000);
    //WriteDword(AUD_MODE_CHG_TIMER,       0x0050);
    //WriteDword(AUD_START_TIMER,          0x0200);
    //WriteDword(AUD_AFE_16DB_EN,          0x0000);
    //WriteDword(AUD_CORDIC_SHIFT_0,       0x0006);

    WriteDword(AUD_RATE_ADJ1,            0x0100);
    WriteDword(AUD_RATE_ADJ2,            0x0200);
    WriteDword(AUD_RATE_ADJ3,            0x0300);
    WriteDword(AUD_RATE_ADJ4,            0x0400);
    WriteDword(AUD_RATE_ADJ5,            0x0500);
    WriteDword(AUD_POLY0_DDS_CONSTANT,   0x121116);
    WriteDword(AUD_DEEMPH0_SHIFT,        0x0000);
    WriteDword(AUD_DEEMPH1_SHIFT,        0x0000);
    WriteDword(AUD_DEEMPH0_G0,           0x0d9f); // change 5/4/01
    WriteDword(AUD_PHASE_FIX_CTL,        0x0009); // change 5/4/01
    WriteDword(AUD_CORDIC_SHIFT_1,       0x0006); // change 5/4/01

    // Completely ditch AFC feedback
    WriteDword(AUD_DCOC_0_SRC,           0x0021);
    WriteDword(AUD_DCOC_1_SRC,           0x001a);
    WriteDword(AUD_DCOC1_SHIFT,          0x0000);
    WriteDword(AUD_DCOC_1_SHIFT_IN0,     0x000a);
    WriteDword(AUD_DCOC_1_SHIFT_IN1,     0x0008);
    WriteDword(AUD_DCOC_PASS_IN,         0x0000);
    WriteDword(AUD_IIR1_4_SEL,           0x0023);

    // Completely ditch L-R AFC feedback
    WriteDword(AUD_DN1_AFC,              0x0000);
    WriteDword(AUD_DN1_FREQ,             0x4000);
    WriteDword(AUD_DCOC_2_SRC,           0x001b);
    WriteDword(AUD_DCOC2_SHIFT,          0x0000);
    WriteDword(AUD_DCOC_2_SHIFT_IN0,     0x000a);
    WriteDword(AUD_DCOC_2_SHIFT_IN1,     0x0008);
    WriteDword(AUD_DEEMPH1_SRC_SEL,      0x0025);

    // setup Audio PLL
    WriteDword(AUD_PLL_PRESCALE,         0x0002);
    WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);

    ::Sleep(100);

    // adjust volume for testing
    WriteDword(AUD_VOL_CTL,              0x0000);
}


void CCX2388xCard::AudioInitNICAM()
{
    // initialize BTSC
    WriteDword(AUD_INIT,                 0x0001);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);

    //; NICAM stereo -> RAM -> DAC bogus xtal setup
    //; xtal = 28.636 MHz

    //; WARNING!!!! Stereo mode is FORCED!!!!

    //; initialize NICAM
    WriteDword(AUD_INIT,                 0x0010);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);
    WriteDword(AUD_CTL,                  0x1922);
    WriteDword(AUD_RATE_ADJ1,            0x1000);
    WriteDword(AUD_RATE_ADJ2,            0x2000);
    WriteDword(AUD_RATE_ADJ3,            0x3000);
    WriteDword(AUD_RATE_ADJ4,            0x4000);
    WriteDword(AUD_RATE_ADJ5,            0x5000);
    WriteDword(AUD_DMD_RA_DDS,           0xc0d5ce);


    //; setup QAM registers
    WriteByte(0x320d01,                  0x06);
    WriteByte(0x320d02,                  0x82);
    WriteByte(0x320d03,                  0x16);
    WriteByte(0x320d04,                  0x05);
    WriteByte(0x320d2a,                  0x34);
    WriteByte(0x320d2b,                  0x4c);

    // setup Audio PLL
    WriteDword(AUD_PLL_PRESCALE,         0x0002);
    WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);  // Causes a pop every time

    ::Sleep(100);

    // adjust volume to max, unmute
    WriteDword(AUD_VOL_CTL,0x0000);

}   

void CCX2388xCard::AudioInitA2()
{
    // initialize A2
    WriteDword(AUD_INIT,                 0x0004);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);
    

    WriteDword(AUD_DN0_FREQ,             0x0000312b);
    WriteDword(AUD_POLY0_DDS_CONSTANT,   0x000a62b4);
    WriteDword(AUD_IIR1_0_SEL,           0x00000000);
    WriteDword(AUD_IIR1_1_SEL,           0x00000001);
    WriteDword(AUD_IIR1_2_SEL,           0x0000001f);
    WriteDword(AUD_IIR1_3_SEL,           0x00000020);
    WriteDword(AUD_IIR1_4_SEL,           0x00000023);
    WriteDword(AUD_IIR1_5_SEL,           0x00000007);
    WriteDword(AUD_IIR1_0_SHIFT,         0x00000000);
    WriteDword(AUD_IIR1_1_SHIFT,         0x00000000);
    WriteDword(AUD_IIR1_2_SHIFT,         0x00000007);
    WriteDword(AUD_IIR1_3_SHIFT,         0x00000007);
    WriteDword(AUD_IIR1_4_SHIFT,         0x00000007);
    WriteDword(AUD_IIR1_5_SHIFT,         0x00000000);
    WriteDword(AUD_IIR2_0_SEL,           0x00000002);
    WriteDword(AUD_IIR2_1_SEL,           0x00000003);
    WriteDword(AUD_IIR2_2_SEL,           0x00000004);
    WriteDword(AUD_IIR2_3_SEL,           0x00000005);
    WriteDword(AUD_IIR3_0_SEL,           0x00000021);
    WriteDword(AUD_IIR3_1_SEL,           0x00000023);
    WriteDword(AUD_IIR3_2_SEL,           0x00000016);
    WriteDword(AUD_IIR3_0_SHIFT,         0x00000000);
    WriteDword(AUD_IIR3_1_SHIFT,         0x00000000);
    WriteDword(AUD_IIR3_2_SHIFT,         0x00000000);
    WriteDword(AUD_IIR4_0_SEL,           0x0000001d);
    WriteDword(AUD_IIR4_1_SEL,           0x00000019);
    WriteDword(AUD_IIR4_2_SEL,           0x00000008);
    WriteDword(AUD_IIR4_0_SHIFT,         0x00000000);
    WriteDword(AUD_IIR4_1_SHIFT,         0x00000000);
    WriteDword(AUD_IIR4_2_SHIFT,         0x00000001);
    WriteDword(AUD_IIR4_0_CA0,           0x0003e57e);
    WriteDword(AUD_IIR4_0_CA1,           0x00005e11);
    WriteDword(AUD_IIR4_0_CA2,           0x0003a7cf);
    WriteDword(AUD_IIR4_0_CB0,           0x00002368);
    WriteDword(AUD_IIR4_0_CB1,           0x0003bf1b);
    WriteDword(AUD_IIR4_1_CA0,           0x00006349);
    WriteDword(AUD_IIR4_1_CA1,           0x00006f27);
    WriteDword(AUD_IIR4_1_CA2,           0x0000e7a3);
    WriteDword(AUD_IIR4_1_CB0,           0x00005653);
    WriteDword(AUD_IIR4_1_CB1,           0x0000cf97);
    WriteDword(AUD_IIR4_2_CA0,           0x00006349);
    WriteDword(AUD_IIR4_2_CA1,           0x00006f27);
    WriteDword(AUD_IIR4_2_CA2,           0x0000e7a3);
    WriteDword(AUD_IIR4_2_CB0,           0x00005653);
    WriteDword(AUD_IIR4_2_CB1,           0x0000cf97);
    WriteDword(AUD_HP_MD_IIR4_1,         0x00000001);
    WriteDword(AUD_HP_PROG_IIR4_1,       0x00000017);
    WriteDword(AUD_DN1_FREQ,             0x00003618);
    WriteDword(AUD_DN1_SRC_SEL,          0x00000017);
    WriteDword(AUD_DN1_SHFT,             0x00000007);
    WriteDword(AUD_DN1_AFC,              0x00000000);
    WriteDword(AUD_DN1_FREQ_SHIFT,       0x00000000);
    WriteDword(AUD_DN2_SRC_SEL,          0x00000040);
    WriteDword(AUD_DN2_SHFT,             0x00000000);
    WriteDword(AUD_DN2_AFC,              0x00000002);
    WriteDword(AUD_DN2_FREQ,             0x0000caaf);
    WriteDword(AUD_DN2_FREQ_SHIFT,       0x00000000);
    WriteDword(AUD_PDET_SRC,             0x00000014);
    WriteDword(AUD_PDET_SHIFT,           0x00000000);
    WriteDword(AUD_DEEMPH0_SRC_SEL,      0x00000011);
    WriteDword(AUD_DEEMPH1_SRC_SEL,      0x00000013);
    WriteDword(AUD_DEEMPH0_SHIFT,        0x00000000);
    WriteDword(AUD_DEEMPH1_SHIFT,        0x00000000);
    WriteDword(AUD_DEEMPH0_G0,           0x000004da);
    WriteDword(AUD_DEEMPH0_A0,           0x0000777a);
    WriteDword(AUD_DEEMPH0_B0,           0x00000000);
    WriteDword(AUD_DEEMPH0_A1,           0x0003f062);
    WriteDword(AUD_DEEMPH0_B1,           0x00000000);
    WriteDword(AUD_DEEMPH1_G0,           0x000004da);
    WriteDword(AUD_DEEMPH1_A0,           0x0000777a);
    WriteDword(AUD_DEEMPH1_B0,           0x00000000);
    WriteDword(AUD_DEEMPH1_A1,           0x0003f062);
    WriteDword(AUD_DEEMPH1_B1,           0x00000000);
    WriteDword(AUD_PLL_EN,               0x00000000);
    WriteDword(AUD_DMD_RA_DDS,           0x002a4efb);
    WriteDword(AUD_RATE_ADJ1,            0x1000);
    WriteDword(AUD_RATE_ADJ2,            0x2000);
    WriteDword(AUD_RATE_ADJ3,            0x3000);
    WriteDword(AUD_RATE_ADJ4,            0x4000);
    WriteDword(AUD_RATE_ADJ5,            0x5000);
    WriteDword(AUD_C2_UP_THR,            0xffff);
    WriteDword(AUD_C2_LO_THR,            0xe800);
    WriteDword(AUD_C1_UP_THR,            0x8c00);
    WriteDword(AUD_C1_LO_THR,            0x6c00);
    WriteDword(AUD_CTL,                  0x100c);

    //  ; Completely ditch AFC feedback
    WriteDword(AUD_DCOC_0_SRC,           0x0021);
    WriteDword(AUD_DCOC_1_SRC,           0x001a);
    WriteDword(AUD_DCOC1_SHIFT,          0x0000);
    WriteDword(AUD_DCOC_1_SHIFT_IN0,     0x000a);
    WriteDword(AUD_DCOC_1_SHIFT_IN1,     0x0008);
    WriteDword(AUD_DCOC_PASS_IN,         0x0000);
    WriteDword(AUD_IIR4_0_SEL,           0x0023);

    // ; Completely ditc FM-2 AFC feedback
    WriteDword(AUD_DN1_AFC,              0x0000);
    WriteDword(AUD_DCOC_2_SRC,           0x001b);
    WriteDword(AUD_IIR4_1_SEL,           0x0025);

    //; WARNING!!! A2 STEREO DEMATRIX AS TO BE
    //; SET MANUALLY!!!  Value sould be 0x100c
    WriteDword(AUD_CTL,                  0x188c);

    //; WARNING!!! THIS CHANGE WAS NOT EXPECTED!!!
    //; Swap I & Q inputs into second rotator
    //; to reverse frequency and therefor invert
    //; phase from the cordic FM demodulator
    //; (frequency rotation must also be reversed
    WriteDword(AUD_DN2_SRC_SEL,          0x0001);
    WriteDword(AUD_DN2_FREQ,             0x00003551);


        // setup Audio PLL
    WriteDword(AUD_PLL_PRESCALE,         0x0002);
    WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);  // Causes a pop every time

    ::Sleep(100);

    // adjust volume to max, unmute
    WriteDword(AUD_VOL_CTL,0x0000);
}


