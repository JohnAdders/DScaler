//
// $Id: MSP34x0.cpp,v 1.30 2002-10-02 10:52:35 kooiman Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2001 itt@myself.com
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
// $Log: not supported by cvs2svn $
// Revision 1.29  2002/09/28 15:04:58  kooiman
// Added object to raiseevent
//
// Revision 1.28  2002/09/28 14:49:39  tobbej
// fixed thread init/deinit for crashloging
//
// Revision 1.27  2002/09/27 14:14:22  kooiman
// MSP34xx fixes.
//
// Revision 1.26  2002/09/26 11:29:52  kooiman
// Split MSP code in 3 parts.
//
// Revision 1.25  2002/09/17 17:28:58  kooiman
// Couple of small fixes.
//
// Revision 1.24  2002/09/16 14:38:59  kooiman
// Added stereo autodetection.
//
// Revision 1.23  2002/09/16 14:37:35  kooiman
// Added stereo autodetection.
//
// Revision 1.22  2002/09/15 19:52:22  kooiman
// Adressed some NICAM AM issues.
//
// Revision 1.21  2002/09/15 15:58:33  kooiman
// Added Audio standard detection & some MSP fixes.
//
// Revision 1.20  2002/09/12 21:44:44  ittarnavsky
// split the MSP34x0 in two files one for the AudioControls the other foe AudioDecoder
//
// Revision 1.19  2002/09/07 20:54:49  kooiman
// Added equalizer, loudness, spatial effects for MSP34xx
//
// Revision 1.18  2002/07/02 20:00:09  adcockj
// New setting for MSP input pin selection
//
// Revision 1.17  2002/03/04 20:48:52  adcockj
// Reversed incorrect change
//
// Revision 1.15  2002/02/01 04:43:55  ittarnavsky
// some more audio related fixes
// removed the handletimermessages and getaudioname methods
// which break the separation of concerns oo principle
//
// Revision 1.14  2002/01/27 23:54:32  robmuller
// Removed the Auto Standard Detect of the rev G chips. + some reorganization of code.
//
// Revision 1.13  2002/01/23 22:57:28  robmuller
// Revision D/G improvements. The code is following the documentation much closer now.
//
// Revision 1.12  2002/01/21 12:06:33  robmuller
// RevA improvements.
//
// Revision 1.11  2002/01/16 19:24:28  adcockj
// Added Rob Muller's msp patch #504469
//
// Revision 1.10  2001/12/28 12:16:53  adcockj
// Sound fixes
//
// Revision 1.9  2001/12/21 11:07:31  adcockj
// Even more RevA fixes
//
// Revision 1.8  2001/12/20 23:46:20  ittarnavsky
// further RevA programming changes
//
// Revision 1.7  2001/12/20 12:55:54  adcockj
// First stab at supporting older MSP chips
//
// Revision 1.6  2001/12/19 19:26:17  ittarnavsky
// started rewrite of the sound standard selection
//
// Revision 1.5  2001/12/18 23:36:01  adcockj
// Split up the MSP chip support into two parts to avoid probelms when deleting objects
//
// Revision 1.4  2001/12/05 21:45:11  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.3  2001/12/03 17:27:56  adcockj
// SECAM NICAM patch from Quenotte
//
// Revision 1.2  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.1  2001/11/25 02:03:21  ittarnavsky
// initial checkin of the new I2C code
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MSP34x0.h"
#include "DebugLog.h"
#include "DScaler.h"
#include "Crash.h"
#include "Events.h"
#include "Providers.h"

#define MSP_UNCARRIER(carrier) ((double)(carrier)*18.432/(double)(1<<24))
#define MSP_CARRIER_HZ(freq) ((int)((double)(freq/18432000.0)*(1<<24)))
#define MSP_UNCARRIER_HZ(carrier) ((long)((double)(carrier)*18432000.0/double(1<<24)))

CMSP34x0::CMSP34x0()
{
}

BYTE CMSP34x0::GetDefaultAddress()const
{
    return 0x80>>1;
}

WORD CMSP34x0::GetRegister(BYTE subAddress, WORD reg)
{
    BYTE write[] = {(reg >> 8) & 0xFF, reg & 0xFF};
    BYTE result[2] = {0, 0};
    ReadFromSubAddress(subAddress, write, sizeof(write), result, sizeof(result));
    LOG(2, "MSP: %02X %02X %02X=%02X %02X",
        subAddress,
        write[0],
        write[1],
        result[0],
        result[1]);
    return ((WORD)result[0]) << 8 | result[1];
}

void CMSP34x0::SetRegister(BYTE subAddress, WORD reg, WORD value)
{
    BYTE write[] = {(reg >> 8) & 0xFF, reg & 0xFF, (value >> 8) & 0xFF, value & 0xFF};
    LOG(2, "MSP: %02X %02X %02X %02X %02X",
        subAddress,
        write[0],
        write[1],
        write[2],
        write[3]);

    WriteToSubAddress(subAddress, write, sizeof(write));
}

WORD CMSP34x0::GetDEMRegister(eDEMReadRegister reg)
{
    return GetRegister(0x11, (WORD)reg);
}

void CMSP34x0::SetDEMRegister(eDEMWriteRegister reg, WORD value)
{
    SetRegister(0x10, (WORD)reg, value);
}

WORD CMSP34x0::GetDSPRegister(eDSPReadRegister reg)
{
    return GetRegister(0x13, (WORD)reg);
}

void CMSP34x0::SetDSPRegister(eDSPWriteRegister reg, WORD value)
{
    SetRegister(0x12, (WORD)reg, value);
}

void CMSP34x0::Reset()
{
    BYTE reset[2] = {0x80, 0};
    LOG(2, "MSP: %02X %02X %02X",
        0,
        reset[0],
        reset[1]);
    WriteToSubAddress(0, reset, sizeof(reset));
    reset[0] = 0;
    LOG(2, "MSP: %02X %02X %02X",
        0,
        reset[0],
        reset[1]);
    WriteToSubAddress(0, reset, sizeof(reset));
}

WORD CMSP34x0::GetVersion()
{
    WORD result = GetDSPRegister(DSP_RD_VERSION_CODE);
    return result;
}

WORD CMSP34x0::GetProductCode()
{
    WORD result = GetDSPRegister(DSP_RD_PRODUCT_CODE);
    return result;
}


////////////////////////////////////////////////////////////////////////////////////////////

CMSP34x0Decoder::TStandardDefinition CMSP34x0Decoder::m_MSPStandards[] =
{
    {
        "M-Dual FM-Stereo",
        MSP34x0_STANDARD_M_DUAL_FM,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_724212,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "B/G-Dual FM-Stereo",
        MSP34x0_STANDARD_BG_DUAL_FM,
        MSP34x0_CARRIER_5_5,
        MSP34x0_CARRIER_5_7421875,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "D/K1-Dual FM-Stereo",
        MSP34x0_STANDARD_DK1_DUAL_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_2578125,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "D/K2-Dual FM-Stereo",
        MSP34x0_STANDARD_DK2_DUAL_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_7421875,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "D/K-FM-Mono with HDEV3",
        MSP34x0_STANDARD_DK_FM_MONO,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_5,
        MONO_FM,
        STEREO_NONE,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "D/K3-Dual FM-Stereo",
        MSP34x0_STANDARD_DK3_DUAL_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_7421875,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM
    },
    {
        "B/G-NICAM-FM",
        MSP34x0_STANDARD_BG_NICAM_FM,
        MSP34x0_CARRIER_5_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        "L-NICAM-AM",
        MSP34x0_STANDARD_L_NICAM_AM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_AM,
        STEREO_NICAM,
        FIR_L_NICAM,
    },
    {
        "I-NICAM-FM",
        MSP34x0_STANDARD_I_NICAM_FM,
        MSP34x0_CARRIER_6_0,
        MSP34x0_CARRIER_6_552,
        MONO_FM,
        STEREO_NICAM,
        FIR_I_NICAM,
    },
    {
        "D/K-NICAM-FM",
        MSP34x0_STANDARD_DK_NICAM_FM,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        "D/K-NICAM-FM with HDEV2",
        MSP34x0_STANDARD_DK_NICAM_FM_HDEV2,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        "D/K-NICAM-FM with HDEV3",
        MSP34x0_STANDARD_DK_NICAM_FM_HDEV3,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_5_85,
        MONO_FM,
        STEREO_NICAM,
        FIR_BG_DK_NICAM,
    },
    {
        "M-BTSC-Stereo",
        MSP34x0_STANDARD_M_BTSC,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_5,
        MONO_FM,
        STEREO_BTSC,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "M-BTSC-Mono + SAP",
        MSP34x0_STANDARD_M_BTSC_MONO,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_5,
        MONO_FM,
        STEREO_MONO_SAP,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "M-EIA-J Japan Stereo",
        MSP34x0_STANDARD_M_EIA_J,
        MSP34x0_CARRIER_4_5,
        MSP34x0_CARRIER_4_5,
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    {
        "FM-Stereo Radio",
        MSP34x0_STANDARD_FM_RADIO,
        MSP34x0_CARRIER_10_7,
        MSP34x0_CARRIER_10_7,
        MONO_FM,
        STEREO_FM,
        FIR_FM_RADIO,
    },
    {
        "SAT-Mono",
        MSP34x0_STANDARD_SAT_MONO,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_5,
        MONO_FM,
        STEREO_NONE,
        FIR_SAT,
    },
    {
        "SAT-Stereo",
        MSP34x0_STANDARD_SAT,
        MSP34x0_CARRIER_7_02,
        MSP34x0_CARRIER_7_20,
        MONO_FM,
        STEREO_FM,
        FIR_SAT,
    },
    {
        "SAT ASTRA Digital Radio",
        MSP34x0_STANDARD_SAT_ADR,
        MSP34x0_CARRIER_6_12,
        MSP34x0_CARRIER_6_12,
        MONO_FM,
        STEREO_ADR,
        FIR_BG_DK_DUAL_FM, //??
    },
    {
        NULL,
        MSP34x0_STANDARD_NONE,
        MSP34x0_CARRIER_6_5,
        MSP34x0_CARRIER_6_5,
        MONO_FM,
        STEREO_NONE,
        FIR_BG_DK_DUAL_FM,
    },
};

CMSP34x0Decoder::TFIRType CMSP34x0Decoder::m_FIRTypes[] =
{
    {
        { -2, -8, -10, 10, 50, 86 },        //FIR_BG_DK_NICAM
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        {  2, 4, -6, -4, 40, 94 },          //FIR_I_NICAM
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        {  -2, -8, -10, 10, 50, 86 },       //FIR_L_NICAM
        {  -4, -12, -9, 23, 79, 126 },
    },
    {
        {  3, 18, 27, 48, 66, 72 },         //FIR_BG_DK_DUAL_FM
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        { 75, 19, 36, 35, 39, 40 },         //FIR_AM_DETECT
        { 75, 19, 36, 35, 39, 40 },
    },
    {
        {  1,  9, 14, 24, 33, 37 },         //FIR_SAT
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        { -8, -8, 4, 6, 78, 107 },          //FIR_FM_RADIO
        { -8, -8, 4, 6, 78, 107 },
    },
};

WORD CMSP34x0Decoder::m_ScartMasks[MSP34x0_SCARTOUTPUT_LASTONE][MSP34x0_SCARTINPUT_LASTONE + 1] =
{
  /* MUTE    MONO    IN1     IN1_DA  IN2     IN2_DA  IN3     IN4     MASK*/
  {  0x0320, 0x0100, 0x0000, -1,     0x0200, -1,     0x0300, 0x0020, 0x0320, },
  {  0x0c40, 0x0800, 0x0440, 0x0c00, 0x0400, 0x0040, 0x0000, 0x0840, 0x0c40, },
  {  0x3000, 0x2000, 0x1000, 0x0000, 0x1080, 0x0080, 0x2080, 0x3080, 0x3080, },
};

CMSP34x0Decoder::TCarrierDetect CMSP34x0Decoder::CarrierDetectTable[] =
{
  {
    MSP34x0_CARRIER_4_5,  {
                              MSP34x0_CARRIER_4_5,            //BTSC, Japan Stereo
                              MSP34x0_CARRIER_4_724212,       //M Dual FM Stereo
                              MSP34x0_NOCARRIER
                          }
  },
  {
    MSP34x0_CARRIER_5_5,  {
                              MSP34x0_CARRIER_5_7421875,      //B/G Dual FM-Stereo
                              MSP34x0_CARRIER_5_85,           //B/G NICAM FM
                              MSP34x0_NOCARRIER
                          }
  },
  {
    MSP34x0_CARRIER_6_0,  (
                              MSP34x0_CARRIER_6_552,            //I NICAM FM
                              MSP34x0_NOCARRIER
                          )
  },
  {
    MSP34x0_CARRIER_6_12, (
                              MSP34x0_CARRIER_6_12,             //SAT ADR
                              MSP34x0_NOCARRIER
                          )

  },
  {
    MSP34x0_CARRIER_6_5,  (
                              MSP34x0_CARRIER_5_7421875,        //D/K3 Dual FM-Stereo
                              MSP34x0_CARRIER_5_85,             //D/K NICAM FM, L NICAM AM
                              MSP34x0_CARRIER_6_2578125,        //D/L1 Dual FM-Stereo
                              MSP34x0_CARRIER_6_5,              //D/K FM-MONO, SAT-MONO
                              MSP34x0_CARRIER_6_7421875,        //D/K2 Dual FM-Stereo
                              MSP34x0_NOCARRIER
                          )
  },
  {
    MSP34x0_CARRIER_7_02, (
                              MSP34x0_CARRIER_7_20,             //SAT Stereo
                              MSP34x0_CARRIER_7_38,             //SAT Lang1/2
                              MSP34x0_NOCARRIER
                          )
  },
  {
    MSP34x0_NOCARRIER,    ( MSP34x0_NOCARRIER )
  }
};


void CMSP34x0Decoder::SetSCARTxbar(eScartOutput output, eScartInput input)
{
	if (-1 == m_ScartMasks[output][input])
		return;
    WORD acb = GetDSPRegister(DSP_RD_ACB);
	acb &= ~m_ScartMasks[output][MSP34x0_SCARTINPUT_LASTONE];
	acb |=  m_ScartMasks[output][input];
	SetDSPRegister(DSP_WR_ACB, acb);
}

CMSP34x0Decoder::CMSP34x0Decoder() : CAudioDecoder(), CMSP34x0()
{
    m_IsInitialized = false;
    m_bHasEqualizer = false;
    m_bHasDolby = false;
    m_bUseInputPin1 = false;
    m_ForceAMSound = false;

    m_MSP34xxThread = NULL;
    m_bStopThread = FALSE;
    m_AutoDetecting = 0;

    m_ForceVersionA = FALSE;
    m_ForceVersionD = FALSE;
    m_ForceVersionG = FALSE;
    m_ForceHasEqualizer = FALSE;

    m_KeepWatchingStereo = FALSE;

    m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_NONE;

    m_DetectInterval10ms = 20; //Detect audio standard in steps of 200 milliseconds

    InitializeCriticalSection(&MSP34xxCriticalSection);
}

CMSP34x0Decoder::~CMSP34x0Decoder()
{
    StopThread();
    DeleteCriticalSection(&MSP34xxCriticalSection);
}

void CMSP34x0Decoder::Initialize()
{
    Reset();

	m_bHasEqualizer = false;
	m_bHasDolby = false;

    if(GetVersion() & 0xFF >= 0x07)
    {
        m_MSPVersion = MSPVersionG;
		m_bHasDolby = true;
    }
    else if(GetVersion() & 0xFF >= 0x04)
    {
        m_MSPVersion = MSPVersionD;
    }
    else
    {
        m_MSPVersion = MSPVersionA;
    }

    if (m_ForceVersionA)
    {
        m_MSPVersion = MSPVersionA;
    }
    else if (m_ForceVersionD)
    {
        m_MSPVersion = MSPVersionD;
    }
    else if (m_ForceVersionG)
    {
        m_MSPVersion = MSPVersionG;
    }

    if ((GetVersion() & 0xFF) >= 0x03)
	{
		// Equalizer supported by revisions C and higher
		m_bHasEqualizer = true;
	}

    if (m_ForceHasEqualizer)
    {
        m_bHasEqualizer = true;
    }

    if(m_MSPVersion == MSPVersionG)
    {
        Initialize34x1G();
    }
    else
    {
        Initialize3400();
    }
    m_IsInitialized = true;
}


/*
All models:
    The demodulator only needs to be programmed if m_AudioInput is AUDIOINPUT_RADIO or
    AUDIOINPUT_TUNER.
Revision G:
    Features:
    - Automatic Standard Detection. TV sound standard is detected automatically.
    - Automatic Sound Select. Automatic setup of FM matrix. If NICAM or FM/stereo is available it is
        automatically enabled. No action needed when changing tuner channels.

    This code uses Automatic Sound Select.
    There is no action needed on channel changes. The Automatic Sound Select will take care
    of switching to NICAM or analog stereo and back to mono.

Revision D:
    Automatic Standard Detection (TV sound standard is detected automatically)
    FM matrix must be programmed manually.

    Uses currently Revision A code.

Revision A:
    Everything is set up manually.
*/


void CMSP34x0Decoder::SetSoundChannel(eSoundChannel soundChannel)
{
    CAudioDecoder::SetSoundChannel(soundChannel);

    EnterCriticalSection(&MSP34xxCriticalSection);
    if (m_AutoDetecting == 1)
    {
        LeaveCriticalSection(&MSP34xxCriticalSection);

        //Autodetect sets soundchannel if its done
		return;
    }
    LeaveCriticalSection(&MSP34xxCriticalSection);

    if(!m_IsInitialized)
    {
        Initialize();
    }

    if(m_MSPVersion == MSPVersionG)
    {
        SetSoundChannel34x1G(soundChannel);
    }
    else
    {
        SetSoundChannel3400(soundChannel);
    }
}


void CMSP34x0Decoder::SetAudioInput(eAudioInput audioInput)
{
    CAudioDecoder::SetAudioInput(audioInput);

    if(!m_IsInitialized)
    {
        Initialize();
    }

    switch (m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_4);
        break;
    case AUDIOINPUT_STEREO:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_3);
        break;
    case AUDIOINPUT_INTERNAL:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_2);
        break;
    case AUDIOINPUT_EXTERNAL:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_1);
        break;
    case AUDIOINPUT_MUTE:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_MUTE);
        break;
    default:
        break;
    }

    SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);

/*
    switch (m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
    case AUDIOINPUT_TUNER:
        if(m_MSPVersion != MSPVersionG)
        {
            ReconfigureRevA();
        }
        break;
    default:
        // todo: make sure the sound from the tuner/radio is muted.
        break;
    }
// todo:
    // Just some inputs are selected. DScaler does not yet support this correctly
    */
}

/*
LPCSTR CMSP34x0Decoder::GetAudioName()
{
    if(m_MSPVersion != MSPVersionG)
    {
        return "not implemented";
    }

    if(!m_IsInitialized)
    {
        Initialize();
    }

    WORD Status = GetDEMRegister(DEM_RD_STATUS);

    if((Status & 0x2) > 0 || (Status & 0x4) > 0)
    {
        return "No carrier";
    }
    // if bilingual sound mode present
    if((Status & 0x100) > 0)
    {
        switch(m_SoundChannel)
        {
        case SOUNDCHANNEL_LANGUAGE1:
            return "Mono L1";
        case SOUNDCHANNEL_LANGUAGE2:
            return "Mono L2";
        default:
            return "Mono (L1/L2)";
        }
    }
    if((Status & 0x20) > 0)
    {
        if((Status & 0x40) > 0)
        {
            return "NICAM Stereo";
        }
        else
        {
            return "NICAM Mono";
        }
    }
    if((Status & 0x40) > 0)
    {
        return "Stereo";
    }
    return "Mono";
}
*/

eSoundChannel CMSP34x0Decoder::IsAudioChannelDetected(eSoundChannel desiredAudioChannel)
{
    switch (desiredAudioChannel)
    {
    case SOUNDCHANNEL_STEREO:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_STEREO)
        {
            return desiredAudioChannel;
        }
        else
        {
            return SOUNDCHANNEL_MONO;
        }
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG1)
        {
            return desiredAudioChannel;
        }
        else
        {
            return SOUNDCHANNEL_MONO;
        }
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        if (m_SupportedSoundChannels & SUPPORTEDSOUNDCHANNEL_LANG2)
        {
            return desiredAudioChannel;
        }
        else
        {
            return SOUNDCHANNEL_MONO;
        }
        break;
    default:
        return SOUNDCHANNEL_MONO;
    }
}


long CMSP34x0Decoder::GetAudioStandardFromVideoFormat(eVideoFormat videoFormat)
{
    /// Guess the correct format
    WORD standard = MSP34x0_STANDARD_NONE;
    switch(videoFormat)
    {
    case VIDEOFORMAT_PAL_B:
        standard = MSP34x0_STANDARD_BG_DUAL_FM;
        break;
    case VIDEOFORMAT_PAL_D:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_PAL_G:
        standard = MSP34x0_STANDARD_BG_NICAM_FM;
        break;
    case VIDEOFORMAT_PAL_H:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_I:
        standard = MSP34x0_STANDARD_I_NICAM_FM;
        break;
    case VIDEOFORMAT_PAL_M:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_N:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_PAL_N_COMBO:
        // \todo FIXME
        standard = MSP34x0_STANDARD_NONE;
        break;
    case VIDEOFORMAT_SECAM_B:
        standard = MSP34x0_STANDARD_BG_DUAL_FM;
        break;
    case VIDEOFORMAT_SECAM_D:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_G:
        standard = MSP34x0_STANDARD_BG_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_H:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_K:
        standard = MSP34x0_STANDARD_DK_NICAM_FM;
        break;
    case VIDEOFORMAT_SECAM_K1:
        standard = MSP34x0_STANDARD_DK1_DUAL_FM;
        break;
    case VIDEOFORMAT_SECAM_L:
        standard = MSP34x0_STANDARD_L_NICAM_AM;
        break;
    case VIDEOFORMAT_SECAM_L1:
        standard = MSP34x0_STANDARD_L_NICAM_AM;
        break;
    case VIDEOFORMAT_NTSC_M:
        standard = MSP34x0_STANDARD_M_BTSC;
        break;
    case VIDEOFORMAT_NTSC_M_Japan:
        standard = MSP34x0_STANDARD_M_EIA_J;
        break;
    default:
    case VIDEOFORMAT_PAL_60:
    case VIDEOFORMAT_NTSC_50:
        standard = MSP34x0_STANDARD_NONE;
        break;
    }
    return standard;
}

LPCSTR CMSP34x0Decoder::GetAudioDecoderID()
{
    return "MSP34xx";
}

int CMSP34x0Decoder::SetAudioDecoderValue(int What, long Val)
{
    if (What==0)
    {
        long Flags = Val;
        if (Flags&0x01)
        {
            if (!m_ForceVersionA)
            {
                m_ForceVersionA = TRUE;
                Initialize();
            }
        }
        else if (Flags&0x02)
        {
            if (!m_ForceVersionD)
            {
                m_ForceVersionD = TRUE;
                Initialize();
            }
        }
        else if (Flags&0x04)
        {
            if (!m_ForceVersionG)
            {
                m_ForceVersionG = TRUE;
                Initialize();
            }
        }

        m_ForceAMSound = ((Flags & 0x08)!=0);
        m_ForceHasEqualizer = ((Flags & 0x10)!=0);
        return 1;
    }
    return 0;
}

long CMSP34x0Decoder::GetAudioDecoderValue(int What)
{
    if (What==0)
    {
        long Flags = 0;
        if (m_ForceVersionA)
        {
            Flags |= 0x01;
        }
        else if (m_ForceVersionD)
        {
            Flags |= 0x02;
        }
        else if (m_ForceVersionG)
        {
            Flags |= 0x04;
        }
        if (m_ForceAMSound)
        {
            Flags |= 0x08;
        }
        if (m_ForceHasEqualizer)
        {
             Flags |= 0x10;
        }
        return Flags;
    }
    return 0;
}

long CMSP34x0Decoder::GetAudioStandardCurrent()
{
    return CAudioDecoder::GetAudioStandardCurrent();
}

void CMSP34x0Decoder::SetAudioStandard(long Standard, eVideoFormat VideoFormat)
{
    CAudioDecoder::SetAudioStandard(Standard, VideoFormat);

    //Only for tuner & radio
	if(m_AudioInput != AUDIOINPUT_RADIO && m_AudioInput != AUDIOINPUT_TUNER)
    {
        return;
    }

    if(!m_IsInitialized)
    {
        Initialize();
    }

    // Find default carrier frequencies
	if (Standard == MSP34x0_STANDARD_NONE)
    {
        m_AudioStandardMajorCarrier = 0;
        m_AudioStandardMinorCarrier = 0;
    }
    else
    {
        m_AudioStandardMajorCarrier = 0;
        m_AudioStandardMinorCarrier = 0;

        int nIndex = 0;
        while (m_MSPStandards[nIndex].Name != NULL)
        {
            if (m_MSPStandards[nIndex].Standard == Standard)
            {
                m_AudioStandardMajorCarrier = MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MajorCarrier);
                m_AudioStandardMinorCarrier = MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MinorCarrier);
            }
            nIndex++;
        }
    }

    if (m_MSPVersion == MSPVersionG)
    {
        SetStandard34x1G((eStandard)Standard, VideoFormat);
        SetSoundChannel(m_SoundChannel);
    }
    else
    {
        SetStandard3400((eStandard)Standard, VideoFormat, TRUE, m_SoundChannel);
    }
}

const char* CMSP34x0Decoder::GetAudioStandardName(long Standard)
{
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        return "<No standard>";
    }
	if (Standard >= MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
    {
        return "Detecting...";
    }
    int nIndex = 0;
    while (m_MSPStandards[nIndex].Name != NULL)
    {
        if (m_MSPStandards[nIndex].Standard == Standard)
        {
            return m_MSPStandards[nIndex].Name;
        }
        nIndex++;
    }
    return NULL;
}

int CMSP34x0Decoder::GetNumAudioStandards()
{
    return ((sizeof(m_MSPStandards)/sizeof(TStandardDefinition)));
}

long CMSP34x0Decoder::GetAudioStandard(int nIndex)
{
    if ((nIndex < 0) || (nIndex >= GetNumAudioStandards()) )
    {
        return 0;
    }
    if (nIndex == 0)
    {
        return MSP34x0_STANDARD_NONE;
    }
    return m_MSPStandards[nIndex-1].Standard;
}

long CMSP34x0Decoder::GetAudioStandardMajorCarrier(long Standard)
{
    if (Standard < 0) //Current
    {
        if (m_AudioStandardMajorCarrier == 0)
        {
            Standard = m_AudioStandard; //default carrier
        }
        else
        {
            return m_AudioStandardMajorCarrier;
        }
    }
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        return 0; //No standard
    }
    int nIndex = 0;
    while (m_MSPStandards[nIndex].Name != NULL)
    {
        if (m_MSPStandards[nIndex].Standard == Standard)
        {
            return MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MajorCarrier);
        }
        nIndex++;
    }
    return 0;
}

long CMSP34x0Decoder::GetAudioStandardMinorCarrier(long Standard)
{
    if (Standard < 0) //Current
    {
        if (m_AudioStandardMinorCarrier == 0)
        {
            Standard = m_AudioStandard; //default carrier
        }
        else
        {
            return m_AudioStandardMinorCarrier;
        }
    }
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        return 0; //No standard
    }
    int nIndex = 0;
    while (m_MSPStandards[nIndex].Name != NULL)
    {
        if (m_MSPStandards[nIndex].Standard == Standard)
        {
            return MSP_UNCARRIER_HZ(m_MSPStandards[nIndex].MinorCarrier);
        }
        nIndex++;
    }
    return 0;
}


void CMSP34x0Decoder::SetAudioStandardCarriers(long MajorCarrier, long MinorCarrier)
{
    if ((MajorCarrier != 0) && (MinorCarrier!=0) &&
        ((MajorCarrier != m_AudioStandardMajorCarrier) || (MinorCarrier != m_AudioStandardMinorCarrier)))
    {
        CAudioDecoder::SetAudioStandardCarriers(MajorCarrier, MinorCarrier);

        if (m_MSPVersion == MSPVersionG)
        {
            SetCarrier34x1G((eCarrier)MSP_CARRIER_HZ(MajorCarrier), (eCarrier)MSP_CARRIER_HZ(MinorCarrier));
        }
        else
        {
            SetCarrier3400((eCarrier)MSP_CARRIER_HZ(MajorCarrier), (eCarrier)MSP_CARRIER_HZ(MinorCarrier));
        }
    }
    else
    {
        CAudioDecoder::SetAudioStandardCarriers(MajorCarrier, MinorCarrier);
    }
}


void CMSP34x0Decoder::DetectAudioStandard(long Interval, int SupportedSoundChannels, eSoundChannel TargetChannel)
{
    CAudioDecoder::DetectAudioStandard(Interval, SupportedSoundChannels, TargetChannel);

    m_DetectInterval10ms = (Interval/10);

    if (SupportedSoundChannels)
    {
        m_TargetSoundChannel = TargetChannel;
    }

    int AutoDetecting;

    EnterCriticalSection(&MSP34xxCriticalSection);
    AutoDetecting = m_AutoDetecting;
    if (m_AutoDetecting != 1)
    {
        m_DetectCounter = 0;
        m_ThreadWait = TRUE;

        if (SupportedSoundChannels == 2)
        {
            m_AutoDetecting = 2;
        }
        else
        {
            m_AutoDetecting = 1;
            m_DetectSupportedSoundChannels = (SupportedSoundChannels == 1);
        }
    }
    else
    {
        if (SupportedSoundChannels)
        {
            m_DetectSupportedSoundChannels = TRUE;
        }
    }
    LeaveCriticalSection(&MSP34xxCriticalSection);

    if (AutoDetecting != 1)
    {
        if (SupportedSoundChannels == 2)
        {
            if (TargetChannel != SOUNDCHANNEL_MONO)
            {
                if(m_MSPVersion == MSPVersionG)
                {
                    SetDEMRegister(DEM_WR_MODUS, 0x2003);
                }
            }
        }
        else
        {
            if(m_MSPVersion == MSPVersionG)
            {
                SetStandard34x1G(MSP34x0_STANDARD_AUTO, m_VideoFormat);
            }
            else
            {
                SetStandard3400(MSP34x0_STANDARD_AUTO, m_VideoFormat, FALSE, SOUNDCHANNEL_MONO);
            }
        }
        EnterCriticalSection(&MSP34xxCriticalSection);
        m_ThreadWait = FALSE;
        LeaveCriticalSection(&MSP34xxCriticalSection);
        StartThread();
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
/// Auto detect/monitoring thread ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MSP34xxThreadProc(LPVOID lpThreadParameter)
{
    DScalerInitializeThread("MSP34xxDetectThread");
    if (lpThreadParameter != NULL)
    {
        int Result;
        __try
        {
            Result = ((CMSP34x0Decoder*)lpThreadParameter)->DetectThread();
        }
        __except (CrashHandler((EXCEPTION_POINTERS*)_exception_info()))
        {
            LOG(1, "Crash in MSP34xx detect loop");
            DScalerDeinitializeThread();
            ExitThread(1);
            return 1;
        }
        DScalerDeinitializeThread();
        ExitThread(Result);
        return Result;
    }
    DScalerDeinitializeThread();
    ExitThread(1);
    return 1;
}

void CMSP34x0Decoder::StartThread()
{
    DWORD LinkThreadID;

    if (m_MSP34xxThread != NULL)
    {
        // Already started
        return;
    }

    m_bStopThread = FALSE;

    m_MSP34xxThread = CreateThread((LPSECURITY_ATTRIBUTES) NULL,  // No security.
                             (DWORD) 0,                     // Same stack size.
                             MSP34xxThreadProc,                  // Thread procedure.
                             (LPVOID)this,                          // Parameter.
                             (DWORD) 0,                     // Start immediatly.
                             (LPDWORD) & LinkThreadID);     // Thread ID.
}

void CMSP34x0Decoder::StopThread()
{
    DWORD ExitCode;
    int i;
    BOOL Thread_Stopped = FALSE;

    if (m_MSP34xxThread != NULL)
    {
        i = 10;
        m_bStopThread = TRUE;
        while(i-- > 0 && !Thread_Stopped)
        {
            if (GetExitCodeThread(m_MSP34xxThread, &ExitCode) == TRUE)
            {
                if (ExitCode != STILL_ACTIVE)
                {
                    Thread_Stopped = TRUE;
                }
                else
                {
                    Sleep(50);
                }
            }
            else
            {
                Thread_Stopped = TRUE;
            }
        }

        if (Thread_Stopped == FALSE)
        {
            TerminateThread(m_MSP34xxThread, 0);
            Sleep(50);
        }
        CloseHandle(m_MSP34xxThread);
        m_MSP34xxThread = NULL;
    }
}

int CMSP34x0Decoder::DetectThread()
{
    m_DetectCounter = 0;
    while (!m_bStopThread)
    {
        int AutoDetecting;
        BOOL bWait;

        EnterCriticalSection(&MSP34xxCriticalSection);
        AutoDetecting = m_AutoDetecting;
        bWait = m_ThreadWait;
        m_DetectSupportedSoundChannels;
        LeaveCriticalSection(&MSP34xxCriticalSection);

        if (bWait)
        {
            Sleep(0);
            continue;
        }

        if ( (AutoDetecting==1) //Detect standard
            && ((m_DetectCounter%m_DetectInterval10ms)==0))
        {
            if((m_MSPVersion == MSPVersionG)) //||(m_MSPVersion == MSPVersionD)
            {
                eStandard standard = DetectStandard34x1G();
                LOG(2,"MSP34x1G: Detect standard: 0x%04x",standard);

                if (standard != MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
                 {
                     m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;

                     if (m_DetectSupportedSoundChannels)
                     {
                        m_SoundChannel = m_TargetSoundChannel;
                     }

                     if (m_ForceAMSound && IsSECAMVideoFormat(m_VideoFormat))
                     {
		                // autodetect doesn't work well with AM ...
                        standard = MSP34x0_STANDARD_L_NICAM_AM;
                        SetStandard34x1G(standard, m_VideoFormat);
                     }

                     m_AudioStandard = standard;

                     SetStandard34x1G(standard, m_VideoFormat, TRUE);

                     AutoDetecting = 2;  //Detect stereo modes

					 SetSoundChannel34x1G(m_SoundChannel, TRUE);

                     EventCollector->RaiseEvent(this,EVENT_AUDIOSTANDARD_DETECTED, 0, standard);

					 EnterCriticalSection(&MSP34xxCriticalSection);
                     if (!m_DetectSupportedSoundChannels)
                     {
                        AutoDetecting = 0;
                     }
                     LeaveCriticalSection(&MSP34xxCriticalSection);
                 }
            }
            else
            {
                eStandard standard = DetectStandard3400();
                LOG(2,"MSP3400: Detect standard: 0x%04x",standard);
                if (standard != MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
                 {
                     m_SupportedSoundChannels = SUPPORTEDSOUNDCHANNEL_MONO;
                     m_AudioStandard = standard;

                     SetStandard3400(standard, m_VideoFormat, TRUE, SOUNDCHANNEL_MONO);

                     EventCollector->RaiseEvent(this,EVENT_AUDIOSTANDARD_DETECTED, 0, standard);

                     EnterCriticalSection(&MSP34xxCriticalSection);
                     if (m_DetectSupportedSoundChannels)
                     {
                         LeaveCriticalSection(&MSP34xxCriticalSection);

                         AutoDetecting = 2; //Try to find stereo
                     }
                     else
                     {
                         LeaveCriticalSection(&MSP34xxCriticalSection);

                         AutoDetecting = 0; //Finished
                         SetSoundChannel3400(m_SoundChannel);
                     }
                 }
            }
            EnterCriticalSection(&MSP34xxCriticalSection);
            m_AutoDetecting = AutoDetecting;
            m_DetectCounter = 1;
            LeaveCriticalSection(&MSP34xxCriticalSection);
        }

        EnterCriticalSection(&MSP34xxCriticalSection);
        AutoDetecting = m_AutoDetecting;
        LeaveCriticalSection(&MSP34xxCriticalSection);

        if (AutoDetecting==2) //Detect mono/stereo/lang1/lang2
        {
            if ((m_DetectCounter%m_DetectInterval10ms) == 0)
            {
                eSupportedSoundChannels Supported;
                if(m_MSPVersion == MSPVersionG)
                {
                    Supported = DetectSoundChannels34x1G();
                }
                else
                {
                    Supported = DetectSoundChannels3400();
                }

                if ((Supported != m_SupportedSoundChannels) || (m_DetectCounter>=150))
                {
                    m_SupportedSoundChannels = Supported;
                    LOG(2,"MSP34xx: Detect stereo: Supported: %s%s%s%s",
                        (Supported&SUPPORTEDSOUNDCHANNEL_MONO)?"Mono ":"",
                        (Supported&SUPPORTEDSOUNDCHANNEL_STEREO)?"Stereo ":"",
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG1)?"Lang1 ":"",
                        (Supported&SUPPORTEDSOUNDCHANNEL_LANG2)?"Lang2 ":"");

                    m_SoundChannel = IsAudioChannelDetected(m_TargetSoundChannel); //m_SoundChannel);
                    SetSoundChannel(m_SoundChannel);

                    EventCollector->RaiseEvent(this,EVENT_AUDIOCHANNELSUPPORT_DETECTED, 0, m_SupportedSoundChannels);

                    if (!m_KeepWatchingStereo)
                    {
                        //Finished
                        EnterCriticalSection(&MSP34xxCriticalSection);
                        m_AutoDetecting = 0;
                        LeaveCriticalSection(&MSP34xxCriticalSection);
                    }
                }
            }
        }

        Sleep(10);
        m_DetectCounter++;
    }
    return 0;
}
