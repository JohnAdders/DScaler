/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Audio.cpp,v 1.20 2004-04-08 17:44:32 to_see Exp $
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
// Revision 1.19  2004/03/28 19:34:11  to_see
// added TVFormat for an call to CCX2388xCard::AudioInitxxx
// more on chip sound; BTSC and Nicam for PAL(I)
// set to LOG(2) when an tuner in CCX2388xCard::AutoDetectTuner was found
// added GPIO Settings for Leadtek WinFast TV2000 XP Expert
//
// Revision 1.18  2004/03/07 12:20:12  to_see
// added 2 Cards
// working Nicam-Sound
// Submenus in CX-Card for Soundsettings
// Click in "Autodetect" in "Setup card CX2388x" is now working
// added "Automute if no Tunersignal" in CX2388x Advanced
//
// Revision 1.17  2004/02/27 20:50:59  to_see
// -more logging in CCX2388xCard::StartStopConexxantDriver
// -handling for IDC_AUTODETECT in CX2388xSource_UI.cpp
// -renamed eAudioStandard to eCX2388xAudioStandard,
//  eStereoType to eCX2388xStereoType and moved from
//  cx2388xcard.h to cx2388x_defines.h
// -moved Audiodetecting from CX2388xCard_Audio.cpp
//  to CX2388xSource_Audio.cpp
// -CCX2388xCard::AutoDetectTuner read
//  at first from Registers
//
// Revision 1.16  2004/02/21 14:11:30  to_see
// more A2-code
//
// Revision 1.15  2004/02/15 21:47:48  to_see
// Oops, forget an 'else'...
//
// Revision 1.14  2004/02/15 21:18:14  to_see
// New A2 code.
//
// Revision 1.13  2004/01/27 22:56:26  robmuller
// New A2 code.
//
// Revision 1.12  2004/01/23 11:26:42  adcockj
// Dematrix fixes
//
// Revision 1.11  2004/01/14 20:01:31  robmuller
// Fix balance.
//
// Revision 1.10  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.9  2003/03/05 13:54:11  adcockj
// Possible fixes for Asus sound
//
// Revision 1.8  2002/12/10 14:53:16  adcockj
// Sound fixes for cx2388x
//
// Revision 1.7  2002/12/05 17:11:11  adcockj
// Sound fixes
//
// Revision 1.6  2002/11/29 17:19:09  adcockj
// extra logging
//
// Revision 1.5  2002/11/29 17:09:46  adcockj
// Some sound fixes (hopefully)
//
// Revision 1.4  2002/11/28 18:07:37  adcockj
// Fixed overflowing registers
//
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

/**
 * @file CX2388xCard.cpp CCX2388xCard Implementation (Audio)
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"

void CCX2388xCard::AudioInit(int nInput, eVideoFormat TVFormat, eCX2388xAudioStandard Standard, eCX2388xStereoType StereoType)
{
    // stop the audio and wait for buffers to clear
    WriteDword(MO_AUD_DMACNTRL, 0x00000000);
    ::Sleep(100);

    // \todo sort this out
    // most of what's below is probably rubbish

    if(Standard == AUDIO_STANDARD_AUTO)
    {
        switch (TVFormat)
        {

        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_I:
        case VIDEOFORMAT_SECAM_B:
        case VIDEOFORMAT_SECAM_D:
        case VIDEOFORMAT_SECAM_G:
        case VIDEOFORMAT_SECAM_H:
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
        case VIDEOFORMAT_SECAM_L:
        case VIDEOFORMAT_SECAM_L1:
            Standard = AUDIO_STANDARD_NICAM;
            break;

        case VIDEOFORMAT_NTSC_M:
            Standard = AUDIO_STANDARD_BTSC;
            break;

        case VIDEOFORMAT_NTSC_M_Japan:
            Standard = AUDIO_STANDARD_EIAJ;
            break;

        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_M:
        case VIDEOFORMAT_PAL_N:
        case VIDEOFORMAT_PAL_N_COMBO:
        case VIDEOFORMAT_PAL_60:
        case VIDEOFORMAT_NTSC_50:
        default:
            Standard = AUDIO_STANDARD_A2;
            break;
        }
    }

    switch(Standard)
    {
    case AUDIO_STANDARD_BTSC:
        AudioInitBTSC(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_EIAJ:
        AudioInitEIAJ(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_A2:
        AudioInitA2(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_BTSC_SAP:
        AudioInitBTSCSAP(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_NICAM:
        AudioInitNICAM(TVFormat, StereoType);
        break;
    case AUDIO_STANDARD_FM:
    default:
        AudioInitFM(TVFormat, StereoType);
        break;
    }

    // start the audio running
    AudioInitDMA();

	m_CurrentAudioStandard	= Standard;
	m_CurrentStereoType		= StereoType;
}

void CCX2388xCard::SetAudioVolume(WORD nVolume)
{
    // Unmute the audio and set volume
    DWORD dwval = 63 - MulDiv(nVolume, 63, 1000);
    WriteDword(AUD_VOL_CTL,dwval);
}

void CCX2388xCard::SetAudioBalance(WORD nBalance)
{
    short val = (short) nBalance;

    if(val <=0)
    {
        val = ((-val) >> 1);
    }
    else
    {
        val = (val >> 1);
        val |= (1 << 6);
    }
    WriteDword(AUD_BAL_CTL, val);
}


void CCX2388xCard::SetAudioMute()
{
	// Causes no pop every time
	if(m_CardType == CX2388xCARD_HAUPPAUGE_PCI_FM)
	{
		// set U1 (4052) pin INH to 1
		DWORD dwval = ReadDword(MO_GP0_IO) | 0x04;
		WriteDword(MO_GP0_IO, dwval);
	}

	// Mute the audio
	DWORD dwval = ReadDword(AUD_VOL_CTL) & 0x1FF;
	dwval |= EN_DAC_MUTE_EN;
	WriteDword(AUD_VOL_CTL, dwval);
}

void CCX2388xCard::SetAudioUnMute(WORD nVolume)
{
	// Unmute the audio and set volume
	DWORD dwval = 63 - MulDiv(nVolume, 63, 1000);
	WriteDword(AUD_VOL_CTL,dwval);

	// Causes no pop every time
	if(m_CardType == CX2388xCARD_HAUPPAUGE_PCI_FM)
	{
		// set U1 (4052) pin INH to 0
		DWORD dwval = ReadDword(MO_GP0_IO) & ~0x04;
		WriteDword(MO_GP0_IO, dwval);
	}
}

void CCX2388xCard::AudioInitDMA()
{
    WriteDword(MO_AUDD_LNGTH,SRAM_FIFO_AUDIO_BUFFER_SIZE);
    WriteDword(MO_AUDR_LNGTH,SRAM_FIFO_AUDIO_BUFFER_SIZE);
    WriteDword(MO_AUD_DMACNTRL, 0x00000003);
}

void CCX2388xCard::AudioInitBTSC(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
	WriteDword(AUD_AFE_12DB_EN,			0x00000001);
	WriteDword(AUD_INIT,				0x00000001);
    WriteDword(AUD_INIT_LD,				0x00000001);
    WriteDword(AUD_SOFT_RESET,			0x00000001);
	WriteDword(AUD_OUT1_SEL,			0x00000013);
	WriteDword(AUD_OUT1_SHIFT,			0x00000000);
	WriteDword(AUD_POLY0_DDS_CONSTANT,	0x0012010c);
	WriteDword(AUD_DMD_RA_DDS,			0x00c3e7aa);
	WriteDword(AUD_DBX_IN_GAIN,			0x00004734);
	WriteDword(AUD_DBX_WBE_GAIN,		0x00004640);
	WriteDword(AUD_DBX_SE_GAIN,			0x00008d31);
	WriteDword(AUD_DCOC_0_SRC,			0x0000001a);
	WriteDword(AUD_IIR1_4_SEL,			0x00000021);
	WriteDword(AUD_DCOC_PASS_IN,		0x00000003);
	WriteDword(AUD_DCOC_0_SHIFT_IN0,	0x0000000a);
	WriteDword(AUD_DCOC_0_SHIFT_IN1,	0x00000008);
	WriteDword(AUD_DCOC_1_SHIFT_IN0,	0x0000000a);
	WriteDword(AUD_DCOC_1_SHIFT_IN1,	0x00000008);
	WriteDword(AUD_DN0_FREQ,			0x0000283b);
	WriteDword(AUD_DN2_SRC_SEL,			0x00000008);
	WriteDword(AUD_DN2_FREQ,			0x00003000);
	WriteDword(AUD_DN2_AFC,				0x00000002);
	WriteDword(AUD_DN2_SHFT,			0x00000000);
	WriteDword(AUD_IIR2_2_SEL,			0x00000020);
	WriteDword(AUD_IIR2_2_SHIFT,		0x00000000);
	WriteDword(AUD_IIR2_3_SEL,			0x0000001f);
	WriteDword(AUD_IIR2_3_SHIFT,		0x00000000);
	WriteDword(AUD_CRDC1_SRC_SEL,		0x000003ce);
	WriteDword(AUD_CRDC1_SHIFT,			0x00000000);
	WriteDword(AUD_CORDIC_SHIFT_1,		0x00000007);
	WriteDword(AUD_DCOC_1_SRC,			0x0000001b);
	WriteDword(AUD_DCOC1_SHIFT,			0x00000000);
	WriteDword(AUD_RDSI_SEL,			0x00000008);
	WriteDword(AUD_RDSQ_SEL,			0x00000008);
	WriteDword(AUD_RDSI_SHIFT,			0x00000000);
	WriteDword(AUD_RDSQ_SHIFT,			0x00000000);
	WriteDword(AUD_POLYPH80SCALEFAC,	0x00000003);
    
	DWORD dwVal;
	switch (StereoType)
	{
	case STEREOTYPE_MONO:
	case STEREOTYPE_ALT1:
	case STEREOTYPE_ALT2:
		// exactly taken from driver, don't know why to set EN_FMRADIO_EN_RDS
		dwVal = EN_DAC_ENABLE|EN_FMRADIO_EN_RDS|EN_BTSC_FORCE_MONO;
		break;
	
	case STEREOTYPE_STEREO:
		dwVal = EN_DAC_ENABLE|EN_FMRADIO_EN_RDS|EN_BTSC_FORCE_STEREO;
		break;
	
	case STEREOTYPE_AUTO:
		dwVal = EN_DAC_ENABLE|EN_FMRADIO_EN_RDS|EN_BTSC_AUTO_STEREO;
		break;
	}

    WriteDword(AUD_CTL,			dwVal     );
    WriteDword(AUD_SOFT_RESET,	0x00000000);
    WriteDword(AUD_VOL_CTL,		0x00000000);
	
/*	old code:

	//\todo handle StereoType

    // increase level of input by 12dB
    WriteDword(AUD_AFE_12DB_EN,          0x0001);

    // initialize BTSC
    WriteDword(AUD_INIT,                 0x0001);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);

    WriteDword(AUD_CTL,                  EN_DAC_ENABLE | EN_DMTRX_SUMDIFF | EN_BTSC_AUTO_SAP);
    
    // These dbx values should be right....
    //WriteDword(AUD_DBX_IN_GAIN,          0x6dc0);
    //WriteDword(AUD_DBX_WBE_GAIN,         0x4003);
    //WriteDword(AUD_DBX_SE_GAIN,          0x7030);
    
    // ....but these values work a lot better
    //WriteDword(AUD_DBX_IN_GAIN,          0x92c0);
    //WriteDword(AUD_DBX_WBE_GAIN,         0x3e83);
    //WriteDword(AUD_DBX_SE_GAIN,          0x854a);

    //WriteDword(AUD_OUT1_SHIFT,           0x0007);
    //WriteDword(AUD_PHASE_FIX_CTL,        0x0020);
    //WriteDword(AUD_RATE_ADJ1,            0x0100);
    //WriteDword(AUD_RATE_ADJ2,            0x0200);
    //WriteDword(AUD_RATE_ADJ3,            0x0300);
    //WriteDword(AUD_RATE_ADJ4,            0x0400);
    //WriteDword(AUD_RATE_ADJ5,            0x0500);
    //WriteDword(AUD_POLY0_DDS_CONSTANT,   0x121116);

    // These are from the Asus TV
    WriteDword(AUD_DBX_IN_GAIN,          0x4734);
    WriteDword(AUD_DBX_WBE_GAIN,         0x4640);
    WriteDword(AUD_DBX_SE_GAIN,          0x8d31);
    WriteDword(AUD_PHASE_FIX_CTL,        0x0020);


    // turn down gain to iir4's...
    WriteDword(AUD_IIR4_0_SHIFT,         0x0006);
    WriteDword(AUD_IIR4_1_SHIFT,         0x0006);
    WriteDword(AUD_IIR4_2_SHIFT,         0x0006);
    
    // ...and turn up iir3 gains
    WriteDword(AUD_IIR3_0_SHIFT,         0x0000);
    WriteDword(AUD_IIR3_1_SHIFT,         0x0000);

    // Completely ditch AFC feedback
    //WriteDword(AUD_DCOC_0_SRC,           0x0021);
    //WriteDword(AUD_DCOC_1_SRC,           0x001a);
    //WriteDword(AUD_DCOC1_SHIFT,          0x0000);
    //WriteDword(AUD_DCOC_1_SHIFT_IN0,     0x000a);
    //WriteDword(AUD_DCOC_1_SHIFT_IN1,     0x0008);
    //WriteDword(AUD_DCOC_PASS_IN,         0x0000);
    //WriteDword(AUD_IIR1_4_SEL,           0x0023);

    // setup Audio PLL
    //WriteDword(AUD_PLL_PRESCALE,         0x0002);
    //WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);  // Causes a pop every time
*/
}

void CCX2388xCard::AudioInitBTSCSAP(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    //\todo code not started this is a copy of BTSC
    //\todo handle StereoType

    // increase level of input by 12dB
    WriteDword(AUD_AFE_12DB_EN,          0x0001);

    // initialize BTSC
    WriteDword(AUD_INIT,                 0x0001);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);

    WriteDword(AUD_CTL,                  EN_DAC_ENABLE | EN_DMTRX_SUMDIFF | EN_BTSC_AUTO_STEREO);
    
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
    //WriteDword(AUD_PLL_PRESCALE,         0x0002);
    //WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);  // Causes a pop every time

}

void CCX2388xCard::AudioInitFM(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    //\todo code not started this is a copy of BTSC

    //\todo handle StereoType

    // increase level of input by 12dB
    WriteDword(AUD_AFE_12DB_EN,          0x0001);

    // initialize BTSC
    WriteDword(AUD_INIT,                 0x0001);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);

    WriteDword(AUD_CTL,                  EN_DAC_ENABLE | EN_DMTRX_SUMDIFF | EN_BTSC_AUTO_STEREO);
    
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
    //WriteDword(AUD_PLL_PRESCALE,         0x0002);
    //WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);  // Causes a pop every time

}

void CCX2388xCard::AudioInitEIAJ(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    //\todo handle StereoType

    // increase level of input by 12dB
    WriteDword(AUD_AFE_12DB_EN,          0x0001);

    // EIAJ stereo -> RAM -> DAC standard setup
    // xtal = 28.636 MHz

    // initialize EIAJ
    WriteDword(AUD_INIT,                 0x0002);
    WriteDword(AUD_INIT_LD,              0x0001);
    WriteDword(AUD_SOFT_RESET,           0x0001);

    WriteDword(AUD_CTL,                  EN_DAC_ENABLE | EN_DMTRX_SUMDIFF | EN_EIAJ_AUTO_STEREO);
    
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
    WriteDword(AUD_AFE_12DB_EN,          0x0001);
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
    //WriteDword(AUD_PLL_PRESCALE,         0x0002);
    //WriteDword(AUD_PLL_INT,              0x001f);

    // de-assert Audio soft reset
    WriteDword(AUD_SOFT_RESET,           0x0000);
}


void CCX2388xCard::AudioInitNICAM(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
	WriteDword(AUD_INIT,					0x00000010);
    WriteDword(AUD_INIT_LD,					0x00000001);
    WriteDword(AUD_SOFT_RESET,				0x00000001);
	WriteDword(AUD_AFE_12DB_EN,				0x00000001);
	WriteDword(AUD_RATE_ADJ1,				0x00000010);
	WriteDword(AUD_RATE_ADJ2,				0x00000040);
	WriteDword(AUD_RATE_ADJ3,				0x00000100);
	WriteDword(AUD_RATE_ADJ4,				0x00000400);
	WriteDword(AUD_RATE_ADJ5,				0x00001000);

	// Deemphasis 1:
	WriteDword(AUD_DEEMPHGAIN_R,			0x000023c2);
	WriteDword(AUD_DEEMPHNUMER1_R,			0x0002a7bc);
	WriteDword(AUD_DEEMPHNUMER2_R,			0x0003023e);
	WriteDword(AUD_DEEMPHDENOM1_R,			0x0000f3d0);
	WriteDword(AUD_DEEMPHDENOM2_R,			0x00000000);

	// Deemphasis 2: please test this for other TVFormat
	/*
	WriteDword(AUD_DEEMPHGAIN_R,			0x0000c600);
	WriteDword(AUD_DEEMPHNUMER1_R,			0x00066738);
	WriteDword(AUD_DEEMPHNUMER2_R,			0x00066739);
	WriteDword(AUD_DEEMPHDENOM1_R,			0x0001e88c);
	WriteDword(AUD_DEEMPHDENOM2_R,			0x0001e88c);
	*/
	
	WriteDword(AUD_DEEMPHDENOM2_R,			0x00000000);
	WriteDword(AUD_ERRLOGPERIOD_R,			0x00000fff);
	WriteDword(AUD_ERRINTRPTTHSHLD1_R,		0x000003ff);
	WriteDword(AUD_ERRINTRPTTHSHLD2_R,		0x000000ff);
	WriteDword(AUD_ERRINTRPTTHSHLD3_R,		0x0000003f);
	WriteDword(AUD_POLYPH80SCALEFAC,		0x00000003);

	WriteByte(AUD_PDF_DDS_CNST_BYTE2,		0x06);
	WriteByte(AUD_PDF_DDS_CNST_BYTE1,		0x82);
	WriteByte(AUD_QAM_MODE,					0x05);

	// set QAM
	switch (TVFormat)
	{
	// 6.552 MHz
	case VIDEOFORMAT_PAL_I:
		WriteByte(AUD_PDF_DDS_CNST_BYTE0,	0x12);
		WriteByte(AUD_PHACC_FREQ_8MSB,		0x3a);
		WriteByte(AUD_PHACC_FREQ_8LSB,		0x93);
		break;

	// 5.85 MHz
	default:
		WriteByte(AUD_PDF_DDS_CNST_BYTE0,	0x16);
		WriteByte(AUD_PHACC_FREQ_8MSB,		0x34);
		WriteByte(AUD_PHACC_FREQ_8LSB,		0x4c);
		break;
	}

	switch(StereoType)
	{
	case STEREOTYPE_MONO:
	case STEREOTYPE_ALT1:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_LR|EN_DMTRX_BYPASS|EN_NICAM_FORCE_MONO1);
		break;

	case STEREOTYPE_ALT2:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_LR|EN_DMTRX_BYPASS|EN_NICAM_FORCE_MONO2);
		break;

	case STEREOTYPE_STEREO:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_LR|EN_DMTRX_BYPASS|EN_NICAM_FORCE_STEREO);
		break;

	case STEREOTYPE_AUTO:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_LR|EN_DMTRX_BYPASS|EN_NICAM_AUTO_STEREO);
		break;
	}
    
	WriteDword(AUD_SOFT_RESET,				0x00000000);  // Causes a pop every time/**/
}   

void CCX2388xCard::AudioInitA2(eVideoFormat TVFormat, eCX2388xStereoType StereoType)
{
    // exactly taken from conexant-driver
	WriteDword(AUD_INIT,					0x00000004);
    WriteDword(AUD_INIT_LD,					0x00000001);
    WriteDword(AUD_SOFT_RESET,				0x00000001);
    WriteByte(AUD_PDF_DDS_CNST_BYTE2,		0x06);
    WriteByte(AUD_PDF_DDS_CNST_BYTE1,		0x82);
    WriteByte(AUD_PDF_DDS_CNST_BYTE0,		0x12);
    WriteByte(AUD_QAM_MODE,					0x05);
    WriteByte(AUD_PHACC_FREQ_8MSB,			0x34);
    WriteByte(AUD_PHACC_FREQ_8LSB,			0x4c);
    WriteDword(AUD_RATE_ADJ1,				0x00001000);
    WriteDword(AUD_RATE_ADJ2,				0x00002000);
    WriteDword(AUD_RATE_ADJ3,				0x00003000);
    WriteDword(AUD_RATE_ADJ4,				0x00004000);
    WriteDword(AUD_RATE_ADJ5,				0x00005000);
	WriteDword(AUD_THR_FR,					0x00000000);
	WriteDword(AAGC_HYST,					0x0000001a);
	WriteDword(AUD_PILOT_BQD_1_K0,			0x0000755b);
	WriteDword(AUD_PILOT_BQD_1_K1,			0x00551340);
	WriteDword(AUD_PILOT_BQD_1_K2,			0x006d30be);
	WriteDword(AUD_PILOT_BQD_1_K3,			0xffd394af);
	WriteDword(AUD_PILOT_BQD_1_K4,			0x00400000);
	WriteDword(AUD_PILOT_BQD_2_K0,			0x00040000);
	WriteDword(AUD_PILOT_BQD_2_K1,			0x002a4841);
	WriteDword(AUD_PILOT_BQD_2_K2,			0x00400000);
	WriteDword(AUD_PILOT_BQD_2_K3,			0x00000000);
	WriteDword(AUD_PILOT_BQD_2_K4,			0x00000000);
	WriteDword(AUD_MODE_CHG_TIMER,			0x00000040);
	WriteDword(AUD_START_TIMER,				0x00000200);
	WriteDword(AUD_AFE_12DB_EN,				0x00000000);
	WriteDword(AUD_CORDIC_SHIFT_0,			0x00000007);
	WriteDword(AUD_CORDIC_SHIFT_1,			0x00000007);
	WriteDword(AUD_DEEMPH0_G0,				0x00000380);
	WriteDword(AUD_DEEMPH1_G0,				0x00000380);
	WriteDword(AUD_DCOC_0_SRC,				0x0000001a);
	WriteDword(AUD_DCOC0_SHIFT,				0x00000000);
	WriteDword(AUD_DCOC_0_SHIFT_IN0,		0x0000000a);
	WriteDword(AUD_DCOC_0_SHIFT_IN1,		0x00000008);
	WriteDword(AUD_DCOC_PASS_IN,			0x00000003);
	WriteDword(AUD_IIR3_0_SEL,				0x00000021);
	WriteDword(AUD_DN2_AFC,					0x00000002);
	WriteDword(AUD_DCOC_1_SRC,				0x0000001b);
	WriteDword(AUD_DCOC1_SHIFT,				0x00000000);
	WriteDword(AUD_DCOC_1_SHIFT_IN0,		0x0000000a);
	WriteDword(AUD_DCOC_1_SHIFT_IN1,		0x00000008);
	WriteDword(AUD_IIR3_1_SEL,				0x00000023);
	WriteDword(AUD_RDSI_SEL,				0x00000017);
	WriteDword(AUD_RDSI_SHIFT,				0x00000000);
	WriteDword(AUD_RDSQ_SEL,				0x00000017);
	WriteDword(AUD_RDSQ_SHIFT,				0x00000000);
	WriteDword(AUD_POLYPH80SCALEFAC,		0x00000001);
   

	switch (TVFormat)
	{
	// 5.5 MHz
	case VIDEOFORMAT_PAL_B:
	case VIDEOFORMAT_PAL_G:
		WriteDword(AUD_DMD_RA_DDS,			0x002a73bd);
		WriteDword(AUD_C1_UP_THR,			0x00007000);
		WriteDword(AUD_C1_LO_THR,			0x00005400);
		WriteDword(AUD_C2_UP_THR,			0x00005400);
		WriteDword(AUD_C2_LO_THR,			0x00003000);
		break;

	// 6.5 MHz
	case VIDEOFORMAT_PAL_D:
	case VIDEOFORMAT_SECAM_K:
	case VIDEOFORMAT_SECAM_D:
		WriteDword(AUD_DMD_RA_DDS,			0x002a73bd);
		WriteDword(AUD_C1_UP_THR,			0x00007000);
		WriteDword(AUD_C1_LO_THR,			0x00005400);
		WriteDword(AUD_C2_UP_THR,			0x00005400);
		WriteDword(AUD_C2_LO_THR,			0x00003000);
		WriteDword(AUD_DN0_FREQ,			0x00003a1c);
		WriteDword(AUD_DN2_FREQ,			0x0000d2e0);
		break;

	// ?.?? Mhz ,untested !
	case VIDEOFORMAT_PAL_I:
		WriteDword(AUD_DMD_RA_DDS,			0x002a2873);
		WriteDword(AUD_C1_UP_THR,			0x00003c00);
		WriteDword(AUD_C1_LO_THR,			0x00003000);
		WriteDword(AUD_C2_UP_THR,			0x00006000);
		WriteDword(AUD_C2_LO_THR,			0x00003c00);
		WriteDword(AUD_DN0_FREQ,			0x00002836);
		WriteDword(AUD_DN1_FREQ,			0x00003418);
		WriteDword(AUD_DN2_FREQ,			0x000029c7);
		WriteDword(AUD_POLY0_DDS_CONSTANT,	0x000a7540);
		break;
	}

	switch(StereoType)
	{
	case STEREOTYPE_MONO:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_MONO|EN_A2_FORCE_MONO1);
		break;

	case STEREOTYPE_ALT1:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_MONO|EN_A2_FORCE_MONO1);
		break;

	case STEREOTYPE_ALT2:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_MONO|EN_A2_FORCE_MONO2);
		break;

	case STEREOTYPE_STEREO:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_LR|EN_A2_FORCE_STEREO);
		break;

	case STEREOTYPE_AUTO:
		WriteDword(AUD_CTL, EN_DAC_ENABLE|EN_DMTRX_LR|EN_A2_AUTO_STEREO);
		// Start Autodetecting with mono
		SetAutoA2StereoToMono();
		break;
	}
    
	WriteDword(AUD_SOFT_RESET,				0x00000000);  // Causes a pop every time/**/
}

void CCX2388xCard::SetAutoA2StereoToMono()
{
	// set timer to an lower value for faster detection
	// of bit 0 + 1 in AUD_CTL
	WriteDword (AUD_MODE_CHG_TIMER,		0x00000040);
	AndOrDataDword(AUD_CTL,				0, ~0x00000800);
	AndOrDataDword(AUD_DEEMPH1_SRC_SEL,	0, ~0x00000002);
}

void CCX2388xCard::SetAutoA2StereoToStereo()
{
	// set timer to this value makes
	// bit 0 + 1 in AUD_CTL stable
	WriteDword(AUD_MODE_CHG_TIMER,		0x00000120);
	OrDataDword(AUD_CTL,				0x00000800);
	OrDataDword(AUD_DEEMPH1_SRC_SEL,	0x00000002);
}

void CCX2388xCard::AudioSoftReset()
{
	// needed when switching from channel to channel
	WriteDword(AUD_SOFT_RESET, 1);	
	WriteDword(AUD_SOFT_RESET, 0);
}

DWORD CCX2388xCard::GetAudioStatusRegister()
{
	DWORD dwVal = ReadDword(AUD_STATUS) & 0x0000002f;
	return dwVal;
}

eCX2388xAudioStandard CCX2388xCard::GetCurrentAudioStandard()
{
	return m_CurrentAudioStandard;
}

eCX2388xStereoType CCX2388xCard::GetCurrentStereoType()
{
	return m_CurrentStereoType;
}