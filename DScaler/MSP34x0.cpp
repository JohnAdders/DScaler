//
// $Id: MSP34x0.cpp,v 1.22 2002-09-15 19:52:22 kooiman Exp $
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
        FIR_BG_DK_DUAL_FM,
    },
    { 
        "SAT-Mono",
        MSP34x0_STANDARD_SAT_MONO,            
        MSP34x0_CARRIER_6_5,  
        MSP34x0_CARRIER_6_5,        
        MONO_FM,
        STEREO_NONE,
        FIR_BG_DK_DUAL_FM,
    },
    { 
        "SAT-Stereo",
        MSP34x0_STANDARD_SAT,                 
        MSP34x0_CARRIER_7_02, 
        MSP34x0_CARRIER_7_20,       
        MONO_FM,
        STEREO_FM,
        FIR_BG_DK_DUAL_FM,
    },
    { 
        "SAT ADR",
        MSP34x0_STANDARD_SAT_ADR,             
        MSP34x0_CARRIER_6_12, 
        MSP34x0_CARRIER_6_12,       
        MONO_FM,
        STEREO_ADR,
        FIR_BG_DK_DUAL_FM,
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
        { -2, -8, -10, 10, 50, 86 }, 
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        {  2, 4, -6, -4, 40, 94 }, 
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        {  -2, -8, -10, 10, 50, 86 }, 
        {  -4, -12, -9, 23, 79, 126 },
    },
    {
        {  3, 18, 27, 48, 66, 72 }, 
        {  3, 18, 27, 48, 66, 72 },
    },
    {
        { 75, 19, 36, 35, 39, 40 }, 
        { 75, 19, 36, 35, 39, 40 },
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
                              MSP34x0_CARRIER_4_5,                             
                              MSP34x0_CARRIER_4_724212,
                              MSP34x0_NOCARRIER
                          }
  },
  {
    MSP34x0_CARRIER_5_5,  {  
                              MSP34x0_CARRIER_5_7421875,
                              MSP34x0_CARRIER_5_85,
                              MSP34x0_NOCARRIER
                          }
  },
  {
    MSP34x0_CARRIER_6_0,  (  
                              MSP34x0_CARRIER_6_0,
                              MSP34x0_NOCARRIER
                          )                             
  },
  {
    MSP34x0_CARRIER_6_12, (  
                              MSP34x0_CARRIER_6_12,
                              MSP34x0_NOCARRIER
                          ) 
   
  },
  {                             
    MSP34x0_CARRIER_6_5,  (  
                              MSP34x0_CARRIER_5_7421875,
                              MSP34x0_CARRIER_5_85,
                              MSP34x0_CARRIER_6_2578125,
                              MSP34x0_CARRIER_6_5,
                              MSP34x0_CARRIER_6_7421875,
                              MSP34x0_NOCARRIER
                          )       
  },
  {
    MSP34x0_CARRIER_7_20, (  
                              MSP34x0_CARRIER_7_02,
                              MSP34x0_NOCARRIER
                          ) 
  },
/*  {
    MSP34x0_CARRIER_7_38, ( 
                              MSP34x0_CARRIER_7_02, 
                              MSP34x0_NOCARRIER
                          ) 
  },*/
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

    m_DetectInterval10ms = 20; //Detect audio standard in steps of 200 milliseconds 
}

CMSP34x0Decoder::~CMSP34x0Decoder()
{
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

    if ((GetVersion() & 0xFF) >= 0x03)
	{
		// Equalizer supported by revisions C and higher
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

    if (m_AutoDetecting == 1)
    {
        return;
    }

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
    return desiredAudioChannel;
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

long CMSP34x0Decoder::GetAudioStandardCurrent()
{
    return CAudioDecoder::GetAudioStandardCurrent();
}

void CMSP34x0Decoder::SetAudioStandard(long Standard, eVideoFormat VideoFormat)
{
    CAudioDecoder::SetAudioStandard(Standard, VideoFormat);

    /*if(m_AudioInput != AUDIOINPUT_RADIO && m_AudioInput != AUDIOINPUT_TUNER)
    {
        return;
    }*/

    if(!m_IsInitialized)
    {
        Initialize();
    }

    if (Standard == MSP34x0_STANDARD_NONE)
    {
        m_AudioStandardMajorCarrier = 0;
        m_AudioStandardMinorCarrier = 0;
    }
    else
    {
        int nIndex = 0;
        while (m_MSPStandards[nIndex].Name != NULL)
        {
            if (m_MSPStandards[nIndex].Standard == Standard)
            {
                m_AudioStandardMajorCarrier = m_MSPStandards[nIndex].MajorCarrier;            
                m_AudioStandardMinorCarrier = m_MSPStandards[nIndex].MinorCarrier;
            }
            nIndex++;
        }
    }

    if (m_MSPVersion == MSPVersionG)
    {
        SetStandard34x1G((eStandard)Standard, VideoFormat);
    }
    else
    {
        SetStandard3400((eStandard)Standard, VideoFormat);
    }
    SetSoundChannel(m_SoundChannel);
}

const char* CMSP34x0Decoder::GetAudioStandardName(long Standard)
{
    if (Standard == MSP34x0_STANDARD_NONE)
    {
        return "<No standard>";
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
        return m_AudioStandardMajorCarrier;
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
        return m_AudioStandardMajorCarrier;
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


void CMSP34x0Decoder::DetectAudioStandard(long Interval, void *pThis, void (*pfnDetected)(void *pThis, long Standard))
{
    CAudioDecoder::DetectAudioStandard(Interval, pThis,pfnDetected);

    m_DetectInterval10ms = (Interval/10);

    if (m_AutoDetecting != 1)
    {
        if(m_MSPVersion == MSPVersionG)
        {
            SetStandard34x1G(MSP34x0_STANDARD_AUTO, m_VideoFormat);
        }
        else
        {
            SetStandard3400(MSP34x0_STANDARD_AUTO, m_VideoFormat);
        }
        
        m_AutoDetecting = 1;
        StartThread();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/// MSP 3400 specific ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

CMSP34x0Decoder::eStandard CMSP34x0Decoder::DetectStandard3400()
{
    static int m_IndexVal1;
    static int m_IndexVal2;
    static int m_IndexMax1;
    static int m_IndexMax2;
    static int m_MajorIndex;
    static int m_MinorIndex;

    if (m_CarrierDetect_Phase == 0) //Setup
    {                
        if (m_ForceAMSound && IsSECAMVideoFormat(m_VideoFormat))
        {
		   // autodetect doesn't work well with AM ...		   
           return MSP34x0_STANDARD_L_NICAM_AM;
        }

        m_IndexVal1 = m_IndexVal2 = 0;
	    m_IndexMax1 = m_IndexMax2 = -1;

        m_MajorIndex = 0;
        m_MinorIndex = 0;

        SetCarrier3400(CarrierDetectTable[m_MajorIndex].Major, CarrierDetectTable[m_MajorIndex].Major);

        m_CarrierDetect_Phase = 1;
        return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
    }

    if (m_CarrierDetect_Phase == 1) //Detect Major
    {
        // Check carrier
        int val = GetDSPRegister(DSP_RD_FM1_DCLVL);
        if (val > 32768) val-= 65536;
        if (m_IndexVal1 < val)
        {
		   m_IndexVal1 = val;
           m_IndexMax1 = m_MajorIndex;
        }

        LOG(2,"MSP3400: Detecting standard. Major carrier = %g Mhz, val=%d",MSP_UNCARRIER(CarrierDetectTable[m_MajorIndex].Major),val);
        
        m_MajorIndex++;
        if (CarrierDetectTable[m_MajorIndex].Major != MSP34x0_NOCARRIER)
        {
            SetCarrier3400(CarrierDetectTable[m_MajorIndex].Major, CarrierDetectTable[m_MajorIndex].Major);
            return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
        }
        else
        {                          
            LOG(2,"MSP3400: Detecting standard. Found Major carrier = %g Mhz. Setup Minor.",MSP_UNCARRIER(CarrierDetectTable[m_IndexMax1].Major));
            
            // Found Major carrier.
            m_MajorIndex = m_IndexMax1;

            // Now setup the detection for the minor carrier                        
            m_MinorIndex = 0;
            if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] == 
                  CarrierDetectTable[m_MajorIndex].Major)
            {
               //Did that already
               m_MinorIndex++;
            }
            
            if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] == MSP34x0_NOCARRIER)
            {
                m_CarrierDetect_Phase = 3;  //Finished
            }
            else
            {
                SetCarrier3400(CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex], CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex]);
                m_CarrierDetect_Phase = 2;
                
                int n = 0;
                while (CarrierDetectTable[m_MajorIndex].Minor[n] != MSP34x0_NOCARRIER)
                {
                    if (CarrierDetectTable[m_MajorIndex].Minor[n] == CarrierDetectTable[m_MajorIndex].Major)
                    {
                        m_IndexVal2 = m_IndexVal1;
                        m_IndexMax2 = n;

                        LOG(2,"MSP3400: Detecting standard. Minor carrier = %g Mhz, val=%d",MSP_UNCARRIER(CarrierDetectTable[m_MajorIndex].Minor[n]),m_IndexVal2);
                        break;
                    }
                    n++;
                }
                return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
            }
        }        
    }

    if (m_CarrierDetect_Phase == 2) //Detect minor
    {
        // Check carrier
        int val = GetDSPRegister(DSP_RD_FM1_DCLVL);
        if (val > 32768) val-= 65536;
        if (m_IndexVal2 < val)
        {
		    m_IndexVal2 = val;
            m_IndexMax2 = m_MinorIndex;
        }
        
        LOG(2,"MSP3400: Detecting standard. Minor carrier = %g Mhz, val=%d",MSP_UNCARRIER(CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex]),val);

        m_MinorIndex++;
        if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] == 
                  CarrierDetectTable[m_MajorIndex].Major)
        {
            //Did that already
            m_MinorIndex++;
        }

        if (CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex] != MSP34x0_NOCARRIER)
        {
            SetCarrier3400(CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex], CarrierDetectTable[m_MajorIndex].Minor[m_MinorIndex]);
            return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
        }
        else
        {
            m_CarrierDetect_Phase = 3;        
        }
    }
    if (m_CarrierDetect_Phase == 3) //Found carriers, determine standard
    {
          eCarrier MajorCarrier = CarrierDetectTable[ m_IndexMax1 ].Major;
          eCarrier MinorCarrier = MajorCarrier;
          if (m_IndexMax2 >= 0)
          {
              MinorCarrier = CarrierDetectTable[ m_IndexMax1 ].Minor[ m_IndexMax2 ];
          }

          LOG(2,"MSP3400: Detecting standard. Found Major/Minor : %g / %g Mhz",MSP_UNCARRIER(MajorCarrier),(MSP_UNCARRIER(MinorCarrier)));

          eStandard Standard = MSP34x0_STANDARD_NONE;
          int n = 0;
          while (m_MSPStandards[n].Name != NULL)
          {
              if (    (m_MSPStandards[n].MajorCarrier == MajorCarrier)
                   && (m_MSPStandards[n].MinorCarrier == MinorCarrier)
                 )
              {
                  Standard = m_MSPStandards[n].Standard;                  
                  break;
              }
              n++;
          }

          //Duplicates
          if ((MajorCarrier == MSP34x0_CARRIER_4_5) && (MinorCarrier == MSP34x0_CARRIER_4_5))
          {
             // BTSC or Japan Stereo?
          }            
          else 
          if ((MajorCarrier == MSP34x0_CARRIER_6_5) && (MinorCarrier == MSP34x0_CARRIER_5_85))
          {
             Standard = MSP34x0_STANDARD_DK_NICAM_FM;
             if (IsSECAMVideoFormat(m_VideoFormat))
             {
                Standard = MSP34x0_STANDARD_L_NICAM_AM;
             }             
          }
          else
          if ((MajorCarrier == MSP34x0_CARRIER_6_5) && (MinorCarrier == MSP34x0_CARRIER_6_5))
          {
             // D/K FM-Mono(HDEV3) or Sat?
          }

          LOG(2,"MSP3400: Detected standard: %d",Standard);

          m_AudioStandardMajorCarrier = MajorCarrier;
          m_AudioStandardMinorCarrier = MinorCarrier;
          if (Standard != MSP34x0_STANDARD_NONE)
          {              
              return Standard;
          }
          else
          {              
              //Try this one
              return MSP34x0_STANDARD_M_DUAL_FM;              
          }
    }

    return MSP34x0_STANDARD_NONE;    
}

void CMSP34x0Decoder::SetSoundChannel3400(eSoundChannel SoundChannel)
{       
    WORD nicam = 0;  // channel source: FM/AM or nicam
	WORD source = 0;

    if (m_AudioInput == AUDIOINPUT_EXTERNAL)
    {
	    nicam = 0x0200;
	}
    else
    {    
    	// switch demodulator
    	switch (m_AudioStandard)
        {
        case MSP34x0_STANDARD_M_DUAL_FM:
        case MSP34x0_STANDARD_BG_DUAL_FM:
        case MSP34x0_STANDARD_DK1_DUAL_FM:
        case MSP34x0_STANDARD_DK2_DUAL_FM:
        case MSP34x0_STANDARD_DK_FM_MONO:
        case MSP34x0_STANDARD_DK3_DUAL_FM:
        case MSP34x0_STANDARD_M_EIA_J:    	
    			if ( (SoundChannel == SOUNDCHANNEL_LANGUAGE1) || (SoundChannel == SOUNDCHANNEL_LANGUAGE2) )
                {
    				  SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3000);
    			} 
                else if (SoundChannel == SOUNDCHANNEL_STEREO)
                { 
    				  SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3001);
    			} 
                else 
                {
    				  SetDSPRegister( DSP_WR_FMAM_PRESCALE, 0x3000);
    			}
    			break;
    		case MSP34x0_STANDARD_SAT_MONO:          
            case MSP34x0_STANDARD_SAT:
            case MSP34x0_STANDARD_SAT_ADR:
    			if ( (SoundChannel == SOUNDCHANNEL_LANGUAGE1) || (SoundChannel == SOUNDCHANNEL_LANGUAGE2) )
                {
    				  SetCarrier3400(MSP34x0_CARRIER_7_38, MSP34x0_CARRIER_7_02);			                    
    			} 
                else if (SoundChannel == SOUNDCHANNEL_STEREO)
                {			
    				  SetCarrier3400(MSP34x0_CARRIER_7_20, MSP34x0_CARRIER_7_02);			                
    			} 
                else 
                {
    				  SetCarrier3400(MSP34x0_CARRIER_6_5, MSP34x0_CARRIER_6_5);              
    			}
    			break;
    		case MSP34x0_STANDARD_BG_NICAM_FM:
            case MSP34x0_STANDARD_I_NICAM_FM:
            case MSP34x0_STANDARD_DK_NICAM_FM:
            case MSP34x0_STANDARD_DK_NICAM_FM_HDEV2:
            case MSP34x0_STANDARD_DK_NICAM_FM_HDEV3:
            case MSP34x0_STANDARD_L_NICAM_AM:
    			SetCarrier3400(m_MSPStandards[m_AudioStandard].MinorCarrier,m_MSPStandards[m_AudioStandard].MajorCarrier);
    			//if (m_NicamOn)
                //{
    				  nicam=0x0100;
                //}
    			break;
    		case MSP34x0_STANDARD_M_BTSC:
            case MSP34x0_STANDARD_M_BTSC_MONO:
    			nicam = 0x0300;
    			break;
    		case MSP34x0_STANDARD_FM_RADIO:
    			break;
    		default:
    			return;
    	}
  }

	
    if (SoundChannel == SOUNDCHANNEL_LANGUAGE1) 
    {
	      source = 0x0000 | nicam;
	} 
    else if (SoundChannel == SOUNDCHANNEL_LANGUAGE2) 
    {
		  source = 0x0010 | nicam;		
	} 
    else if (SoundChannel == SOUNDCHANNEL_STEREO) 
    {
		  source = 0x0020 | nicam;
	} 
    else 
    {
        // Mono		
		source = 0x0000 | nicam;

        if (m_AudioStandard == MSP34x0_STANDARD_L_NICAM_AM)
        {
            source = 0x200;
            /// AM mono decoding is handled by tuner, not MSP chip
            SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_MONO);            
            SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);
		 }
	}
	
    SetDSPRegister(DSP_WR_LDSPK_SOURCE, source);
    SetDSPRegister(DSP_WR_HEADPH_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART1_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART2_SOURCE, source);
    SetDSPRegister(DSP_WR_I2S_SOURCE, source);    	
}


void CMSP34x0Decoder::Initialize3400()
{   
    //Everything is set up in SetStandard3400 and SetSoundChannel3400
}

void CMSP34x0Decoder::SetStandard3400(eStandard standard, eVideoFormat videoformat)
{
    TStandardDefinition StandardDefinition;

    if (standard == MSP34x0_STANDARD_AUTO)
    {
        StandardDefinition.Standard = MSP34x0_STANDARD_AUTO;
        StandardDefinition.FIRType = FIR_AM_DETECT;
        StandardDefinition.StereoType = STEREO_FM;
        StandardDefinition.MajorCarrier = MSP34x0_CARRIER_4_5;
        StandardDefinition.MinorCarrier = MSP34x0_CARRIER_4_5;

        m_CarrierDetect_Phase = 0;
        // Mute
        SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0x0000);
        SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0x0000);
        SetDSPRegister(DSP_WR_SCART1_VOLUME, 0x0000);
        SetDSPRegister(DSP_WR_SCART2_VOLUME, 0x0000);
    }
    else
    {        
        StandardDefinition.Standard = MSP34x0_STANDARD_NONE;

        int standardIndex = 0;        
        while (m_MSPStandards[standardIndex].Name != NULL)
        {
            if (m_MSPStandards[standardIndex].Standard == standard)
            {
                StandardDefinition = m_MSPStandards[standardIndex];
                break;
            }
            standardIndex++;
        }
    }

    if (StandardDefinition.Standard == MSP34x0_STANDARD_NONE)
    {
        //No standard, mute
        Reset();
        return;
    }

    /// Step 0: AD_CV
    if(StandardDefinition.StereoType == STEREO_SAT)
    {
        // set up for AGC bit 7
        // and bits 6..1  100011 for SAT
        SetDEMRegister(DEM_WR_AD_CV, 0xC6);
    }
    else if(StandardDefinition.StereoType == STEREO_NICAM && 
                StandardDefinition.MonoType == MONO_AM)
    {
        // set up for AGC bit 7
        // and bits 6..1  100011 AM and NICAM
        SetDEMRegister(DEM_WR_AD_CV, 0xC6);
    }
    else
    {
        // set up for AGC bit 7
        // and bits 6..1  101000 FM and NICAM
        // or Dual FM
        SetDEMRegister(DEM_WR_AD_CV, 0xD0);
    }
    
    // I have no idea what this is supposed to do.
    /// Step 1: AUDIO_PLL
    // FIXME what should this be may need to be 0 for NICAM
    // SetDEMRegister(DEM_WR_AUDIO_PLL, 1);

    /// Step 2: FAWCT_SOLL
    if(StandardDefinition.StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_FAWCT_SOLL, 12);
    }
    
    /// Step 3: FAW_ER_TOL
    if(StandardDefinition.StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_FAW_ER_TOL, 2);
    }

	
    // FIR data

    int FIRType = StandardDefinition.FIRType;
    int i;
    for (i = 5; i >= 0; i--)
    {
        SetDEMRegister(DEM_WR_FIR1, m_FIRTypes[FIRType].FIR1[i]);
    }

    SetDEMRegister(DEM_WR_FIR2, 0x0004);
    SetDEMRegister(DEM_WR_FIR2, 0x0040);
    SetDEMRegister(DEM_WR_FIR2, 0x0000);
    
    for (i = 5; i >= 0; i--)
    {
        SetDEMRegister(DEM_WR_FIR2, m_FIRTypes[FIRType].FIR2[i]);
    }

    // Mode reg
    /// Step 6: MODE_REG
    WORD ModeReg = 1 << 10;// bit 10 must be set according to documentation
    if(StandardDefinition.StereoType == STEREO_NICAM)
    {
        // set NICAM mode
        ModeReg |= 1 << 6;
        if(StandardDefinition.MonoType == MONO_AM)
        {
            // set MSP 1/2 to AM
            ModeReg |= 1 << 8;
        }
    }
    else
    {
        if(StandardDefinition.StereoType != STEREO_NONE)
        {
            // set Two carrier FM mode
            ModeReg |= 1 << 7;
        }
        if(StandardDefinition.MonoType == MONO_FM)
        {
            // set MSP channel1 to FM
            ModeReg |= 1 << 7;
        }
        else
        {
            // set MSP 1/2 to AM
            ModeReg |= 1 << 8;
        }

    }
    switch(standard)
    {
    case MSP34x0_STANDARD_BG_DUAL_FM:
    case MSP34x0_STANDARD_DK1_DUAL_FM:
        ModeReg |= 1 << 13;
        break;
    case MSP34x0_STANDARD_AUTO:
        ModeReg = 0x500;
        break;
    case MSP34x0_STANDARD_L_NICAM_AM:
        ModeReg = 0x140;
    default:
        break;
    }

    SetDEMRegister(DEM_WR_MODE_REG, ModeReg);

    
    // Set Carrier
    SetCarrier3400(StandardDefinition.MajorCarrier, StandardDefinition.MinorCarrier);

    /// Step 12 NICAM_START
    if(m_MSPStandards[m_AudioStandard].StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_SEARCH_NICAM, 0);
    }
    
    // Source
    SetSoundChannel3400(m_SoundChannel);

    // FM and NICAM prescale
    WORD fmprescale = 0x3000;
    switch(standard)
    {
    case MSP34x0_STANDARD_BG_DUAL_FM:
        fmprescale |= 1;
        break;
    case MSP34x0_STANDARD_M_DUAL_FM:
        fmprescale |= 2;
        break;
    case MSP34x0_STANDARD_L_NICAM_AM:
        fmprescale = 0x7c03;
    default:
        break;
    }
    
    SetDSPRegister(DSP_WR_FMAM_PRESCALE, fmprescale);
    SetDSPRegister(DSP_WR_NICAM_PRESCALE, 0x5A00);
    
    // reset the ident filter
    SetDSPRegister(DSP_WR_IDENT_MODE, 0x3f);    
    
    /*SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0x7300);
    SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0x7300);

    SetDSPRegister(DSP_WR_SCART1_VOLUME, 0x4000);
    SetDSPRegister(DSP_WR_SCART2_VOLUME, 0x4000);
    */    
}

void CMSP34x0Decoder::SetCarrier3400(eCarrier MajorCarrier, eCarrier MinorCarrier)
{
   /// Step 7, 8, 9 and 10: DCO1_LO, DCO1_HI, DCO2_LO and DCO2_HI    
    SetDEMRegister(DEM_WR_DCO1_LO, MinorCarrier & 0xfff);
    SetDEMRegister(DEM_WR_DCO1_HI, MinorCarrier >> 12);
    SetDEMRegister(DEM_WR_DCO2_LO, MajorCarrier & 0xfff);
    SetDEMRegister(DEM_WR_DCO2_HI, MajorCarrier >> 12);

    SetDEMRegister(DEM_WR_LOAD_REG_12, 0); 
}

/////////////////////////////////////////////////////////////////////////////////////////
/// MSP 34x1G specific //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

CMSP34x0Decoder::eStandard CMSP34x0Decoder::DetectStandard34x1G()
{
    if(m_MSPVersion != MSPVersionG) 
    {
        return MSP34x0_STANDARD_NONE;
    }
    WORD Result = GetDEMRegister(DEM_RD_STANDARD_RESULT);
    if (Result >= 0x07ff)
    {
        return MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS;
    }    
    return (eStandard)Result;
}

void CMSP34x0Decoder::SetSoundChannel34x1G(eSoundChannel soundChannel)
{
    LOG(2,"MSP34xx: Set sound channel: %d",soundChannel);
    
    WORD source = 0;
    //WORD nicam = 0;
    
    if (m_AudioStandard == MSP34x0_STANDARD_L_NICAM_AM)
    {
        SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x7c09);
    }
    else
    {
        if(soundChannel == SOUNDCHANNEL_MONO)
        {            
            SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3003);
        }
        else
        {
            SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x2403); //  0x3000);
        }
    }
    
    switch (soundChannel)
    {
    case SOUNDCHANNEL_MONO:
        source = 0x20;
        break;
    case SOUNDCHANNEL_STEREO:
        source = 0x120;
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        // if not bilingual then stereo is automatically enabled if available
        source = 0x320;
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        // if not bilingual then stereo is automatically enabled if available
        source = 0x420;
        break;
    default:
        break;
    }
    
    SetDSPRegister(DSP_WR_LDSPK_SOURCE, source);
    SetDSPRegister(DSP_WR_HEADPH_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART1_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART2_SOURCE, source);
    SetDSPRegister(DSP_WR_I2S_SOURCE, source);
}


void CMSP34x0Decoder::Initialize34x1G()
{
    SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3000);
    SetDSPRegister(DSP_WR_NICAM_PRESCALE, 0x5A00);
    SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);
    SetDSPRegister(DSP_WR_I2S1_PRESCALE, 0x1000);
    SetDSPRegister(DSP_WR_I2S2_PRESCALE, 0x1000);

    // set up so that we fall back to AM/FM if there is no NICAM
    // required on D series chips
    if(m_MSPVersion == MSPVersionD)
    {
        SetDEMRegister(DEM_WR_AUTO_FMAM, 0x0001);
    }
    
    // 3. reset volume to 0dB
    /*SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0x7300);
    SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0x7300);    
    SetDSPRegister(DSP_WR_SCART1_VOLUME, 0x4000);
    SetDSPRegister(DSP_WR_SCART2_VOLUME, 0x4000);    
    */
}

void CMSP34x0Decoder::SetStandard34x1G(eStandard standard, eVideoFormat videoformat)
{
    if(m_MSPVersion != MSPVersionG) return;

    LOG(2,"MSP34xx: Set standard: %d",standard);

    if (standard == MSP34x0_STANDARD_AUTO)
    {
        SetDEMRegister(DEM_WR_STANDARD_SELECT, MSP34x0_STANDARD_AUTO);    
        return;
    }

    
/*
    // the following is only valid for RevG since it uses Automatic Sound Select
    WORD modus = 0;

    // if mono output is forced, Automatic Sound Select must be disabled, otherwise enabled.
    if(m_SoundChannel == SOUNDCHANNEL_MONO)
    {
        modus = 0;
    }
    else
    {
        modus = 1;
    }

    // choose sound IF2 input pin if needed.
    // todo: Maybe some cards are using IF1?
    switch(m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
    case AUDIOINPUT_TUNER:
        if(!m_bUseInputPin1)
        {
            modus |= 0x100;
        }
        break;
    default:
        break;
    }

    SetDEMRegister(DEM_WR_MODUS, modus);
*/

    // Set mode    
    WORD mode = 0x3003;

    if (standard == MSP34x0_STANDARD_FM_RADIO) //m_AudioInput == AUDIOINPUT_RADIO
    {        
        mode = 0x0003; //autodetect        
    }
    else
    {   
        switch(videoformat)
        {
            case VIDEOFORMAT_SECAM_L:
            case VIDEOFORMAT_SECAM_L1:
                mode = 0x6003;
                break;
        
            case VIDEOFORMAT_PAL_M:
            case VIDEOFORMAT_PAL_60:
            case VIDEOFORMAT_NTSC_50:
                mode = 0x3003;
                break;
            case VIDEOFORMAT_PAL_B:
            case VIDEOFORMAT_PAL_D:
            case VIDEOFORMAT_PAL_G:
            case VIDEOFORMAT_PAL_H:
            case VIDEOFORMAT_PAL_I:
            case VIDEOFORMAT_PAL_N:
            case VIDEOFORMAT_PAL_N_COMBO:
                mode = 0x7003;
                break;
            case VIDEOFORMAT_NTSC_M_Japan:
                mode = 0x5003;
                break;
            case VIDEOFORMAT_NTSC_M:
                mode = 0x3003;
                break;
            case VIDEOFORMAT_SECAM_B:        
            case VIDEOFORMAT_SECAM_D:
            case VIDEOFORMAT_SECAM_G:
            case VIDEOFORMAT_SECAM_H:
            case VIDEOFORMAT_SECAM_K:
            case VIDEOFORMAT_SECAM_K1:
                mode = 0x7003;
                break;
        }
    }
    SetDEMRegister(DEM_WR_MODUS, mode);
    
    // Set standard
    SetDEMRegister(DEM_WR_STANDARD_SELECT, (int)standard);    
}

void CMSP34x0Decoder::SetCarrier34x1G(eCarrier MajorCarrier, eCarrier MinorCarrier)
{
    // Not necessary because rev G detects the carrier?
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Auto detect/monitoring thread ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI MSP34xxThreadProc(LPVOID lpThreadParameter)
{
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
            ExitThread(1);
            return 1;
        }
        ExitThread(Result);
        return Result;
    }
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
        if ( (m_AutoDetecting==1) //Detect standard
            && ((m_DetectCounter%m_DetectInterval10ms)==0))
        {
            if(m_MSPVersion == MSPVersionG)
            {
                eStandard standard = DetectStandard34x1G(); 
                LOG(2,"MSP34x1G: Detect standard: %d",standard);
                if (standard == MSP34x0_STANDARD_NONE)
                {
                    SetStandard34x1G(MSP34x0_STANDARD_AUTO, m_VideoFormat);
                }
                else  if (standard != MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
                 {
                     CAudioDecoder::SetAudioStandard(standard, m_VideoFormat);
                     SetStandard34x1G(standard, m_VideoFormat);
                     SetSoundChannel34x1G(m_SoundChannel);
                     m_AutoDetecting = 0;

                     if (m_pfnDetected != NULL)
                     {
                        m_pfnDetected(m_pfnDetected_pThis, standard);
                     }
                     m_AutoDetecting = 2;
                 }
            }
            else
            {
                eStandard standard = DetectStandard3400(); 
                LOG(2,"MSP3400: Detect standard: %d",standard);
                if (standard != MSP34x0_STANDARD_AUTODETECTION_IN_PROGRESS)
                 {
                     CAudioDecoder::SetAudioStandard(standard, m_VideoFormat);
                     SetStandard3400(standard, m_VideoFormat);                     
                     SetAudioStandardCarriers(m_AudioStandardMajorCarrier, m_AudioStandardMinorCarrier);                     
                     m_AutoDetecting = 0;

                     if (m_pfnDetected != NULL)
                     {
                         // Notify that autodetection is finished..
                         m_pfnDetected(m_pfnDetected_pThis, standard);
                     }
                     m_AutoDetecting = 2;
                 }    
            }
        }
        if (m_AutoDetecting==2) //Detect mono/stereo/lang1/lang2
        {
            //
            m_AutoDetecting = 0;
        }

        Sleep(10);
        m_DetectCounter++;
    }
    return 0;
}
