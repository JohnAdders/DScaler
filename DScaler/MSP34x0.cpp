//
// $Id: MSP34x0.cpp,v 1.10 2001-12-28 12:16:53 adcockj Exp $
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
    LOG(1, "MSP: %02X %02X %02X=%02X %02X",
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
    LOG(1, "MSP: %02X %02X %02X %02X %02X",
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

CMSP34x0Controls::CMSP34x0Controls() : CMSP34x0()
{
    m_Muted = false;
    m_Volume = 900;
    m_Balance = 0;
    m_Bass = 0;
    m_Treble = 0;
}

void CMSP34x0::Reset()
{
    BYTE reset[2] = {0x80, 0};
    LOG(1, "MSP: %02X %02X %02X",
        0,
        reset[0],
        reset[1]);
    WriteToSubAddress(0, reset, sizeof(reset));
    reset[0] = 0;
    LOG(1, "MSP: %02X %02X %02X",
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


void CMSP34x0Controls::SetLoudnessAndSuperBass(long nLoudness, bool bSuperBass)
{
    if (nLoudness > 68)
    {
        return;
    }
    SetDSPRegister(DSP_WR_LDSPK_LOUDNESS, ((nLoudness & 0xFF) << 8) | (bSuperBass ? 0x4 : 0));
    SetDSPRegister(DSP_WR_HEADPH_LOUDNESS, ((nLoudness & 0xFF) << 8) | (bSuperBass ? 0x4 : 0));
}

void CMSP34x0Controls::SetSpatialEffects(long nSpatial)
{
    // Mode A, Automatic high pass gain
    SetDSPRegister(DSP_WR_LDSPK_SPATIALEFF, ((nSpatial & 0xFF) << 8) | 0x8);
}

void CMSP34x0Controls::SetEqualizer(long EqIndex, long nLevel)
{
    if (EqIndex < 0 || EqIndex > 4)
    {
        return;
    }
    if (nLevel < -96 || nLevel > 96)
    {
        return;
    }
    SetDSPRegister((eDSPWriteRegister)(DSP_WR_LDSPK_EQ1 + EqIndex), (nLevel & 0xFF) << 8);
}

void CMSP34x0Controls::SetMute(bool mute)
{
    m_Muted = mute;
    if (m_Muted)
    {
        SetDSPRegister(DSP_WR_LDSPK_VOLUME, 0xFF00);
        SetDSPRegister(DSP_WR_HEADPH_VOLUME, 0xFF00);
    }
    else
    {
        SetVolume(m_Volume);
    }
}

bool CMSP34x0Controls::IsMuted()
{
    return m_Muted;
}

void CMSP34x0Controls::SetVolume(WORD nVolume)
{
    if (nVolume < 0 || nVolume > 1000)
    {
        return;
    }
    m_Volume = nVolume;
    nVolume = MulDiv(nVolume, 0x7f0, 1000);
    // Use Mute if less than 0x10
    if(nVolume < 0x10)
    {
        nVolume = 0;
    }
    SetDSPRegister(DSP_WR_LDSPK_VOLUME, nVolume << 4);
    SetDSPRegister(DSP_WR_HEADPH_VOLUME, nVolume << 4);
}

WORD CMSP34x0Controls::GetVolume()
{
    return m_Volume;
}

void CMSP34x0Controls::SetBalance(WORD nBalance)
{
    m_Balance = nBalance;
    SetDSPRegister(DSP_WR_LDSPK_BALANCE, (nBalance & 0xFF) << 8);
    SetDSPRegister(DSP_WR_HEADPH_BALANCE, (nBalance & 0xFF) << 8);
}

WORD CMSP34x0Controls::GetBalance()
{
    return m_Balance;
}

void CMSP34x0Controls::SetBass(WORD nBass)
{
    if (nBass < -96)
    {
        return;
    }
    m_Bass = nBass;
    SetDSPRegister(DSP_WR_LDSPK_BASS, (nBass & 0xFF) << 8);
    SetDSPRegister(DSP_WR_HEADPH_BASS, (nBass & 0xFF) << 8);
}

WORD CMSP34x0Controls::GetBass()
{
    return m_Bass;
}

void CMSP34x0Controls::SetTreble(WORD nTreble)
{
    if (nTreble < -96)
    {
        return;
    }
    m_Treble = nTreble;
    SetDSPRegister(DSP_WR_LDSPK_TREBLE, (nTreble & 0xFF) << 8);
    SetDSPRegister(DSP_WR_HEADPH_TREBLE, (nTreble & 0xFF) << 8);
}

WORD CMSP34x0Controls::GetTreble()
{
    return m_Treble;
}

////////////////////////////////////////////////////////////////////////////////////////////

CMSP34x0Decoder::CMSP34x0Decoder() : CAudioDecoder(), CMSP34x0()
{
    m_bRevisionChecked = false;
}

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
};

WORD CMSP34x0Decoder::m_ScartMasks[MSP34x0_SCARTOUTPUT_LASTONE][MSP34x0_SCARTINPUT_LASTONE + 1] = 
{
  /* MUTE    MONO    IN1     IN1_DA  IN2     IN2_DA  IN3     IN4     MASK*/
  {  0x0320, 0x0100, 0x0000, -1,     0x0200, -1,     0x0300, 0x0020, 0x0320, },
  {  0x0c40, 0x0800, 0x0440, 0x0c00, 0x0400, 0x0040, 0x0000, 0x0840, 0x0c40, },
  {  0x3000, 0x2000, 0x1000, 0x0000, 0x1080, 0x0080, 0x2080, 0x3080, 0x3080, },
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

void CMSP34x0Decoder::ReconfigureRevD()
{
    /// SCART Signal Path
    switch (m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_2);
        break;
    case AUDIOINPUT_EXTERNAL:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_1);
        break;
    case AUDIOINPUT_MUTE:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_MUTE);
        break;
    }

    /// Demodulator programming

    WORD modus = 0x0003;
    WORD standard = MSP34x0_STANDARD_AUTO;

    if (m_AudioInput == AUDIOINPUT_RADIO)
    {
        standard = MSP34x0_STANDARD_FM_RADIO;
    }
    else
    {
        switch(m_VideoFormat)
        {
        case VIDEOFORMAT_PAL_D:
            // make the autodetection prefer the NICAM or DK2 modes
            modus = 0x1003;
            break;
        case VIDEOFORMAT_NTSC_M:
            modus = 0x2003;
            standard = MSP34x0_STANDARD_M_BTSC;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            modus = 0x4003;
            break;
        case VIDEOFORMAT_PAL_I:
            modus = 0x0003;
            standard = MSP34x0_STANDARD_I_NICAM_FM;
            break;        
		default:
            break;
        }
    }

    // 1. MODUS register
    SetDEMRegister(DEM_WR_MODUS, modus);

    // set up so that we fall back to AM/FM if there is no NICAM
    // required on D series chips
    SetDEMRegister(DEM_WR_AUTO_FMAM, 0x0001);

    SetDSPRegister(DSP_WR_LDSPK_SOURCE, 0x0320);
    SetDSPRegister(DSP_WR_HEADPH_SOURCE, 0x0420);
    SetDSPRegister(DSP_WR_SCART1_SOURCE, 0x0120);

    // 2. FM and NICAM prescale
    SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x2403);
    SetDSPRegister(DSP_WR_NICAM_PRESCALE, 0x5A00);
    
    // 3. STANDARD SELECT register
    SetDEMRegister(DEM_WR_STANDARD_SELECT, standard);

    if (standard == MSP34x0_STANDARD_AUTO)
    {
        while (1)
        {
            ::Sleep(100);
            standard = GetDEMRegister(DEM_RD_STANDARD_RESULT);
            LOG(1, "MSP: std:0x%04X", standard);
            if (standard < MSP34x0_STANDARD_AUDODETECTION_IN_PROGRESS)
            {
                break;
            }
        }
    }
    LOG(1, "MSP: status:0x%04X", GetDEMRegister(DEM_RD_STATUS));

    // 4. FM matrix
    WORD fmMatrix = 0x3000;
    if (m_SoundChannel == SOUNDCHANNEL_STEREO)
    {
        if (m_AudioInput == AUDIOINPUT_TUNER || IsNTSCVideoFormat(m_VideoFormat))
        {
            fmMatrix |= 2;
        }
        else
        {
            fmMatrix |= 1;
        }
    }
    else if (m_SoundChannel = SOUNDCHANNEL_LANGUAGE1)
    {
        fmMatrix |= 3;
    }
    else if (m_SoundChannel = SOUNDCHANNEL_LANGUAGE2)
    {
        fmMatrix |= 4;
    }
    SetDSPRegister(DSP_WR_FMAM_PRESCALE, fmMatrix);

    /// SCART and I2S inputs

    // 1. SCART prescale
    SetDSPRegister(DSP_WR_SCART_PRESCALE, 0x1900);

    // 2. I2S inputs prescale

    /// Ouput channels

    // 1. sources and matrix
    WORD source = 0x0100;
    if (m_AudioInput == AUDIOINPUT_RADIO)
    {
        source = 0x0220;
    }
    else
    {
        switch (m_SoundChannel)
        {
        case SOUNDCHANNEL_MONO:
            source |= 0x30;
            break;
        case SOUNDCHANNEL_STEREO:
            source |= 0x20;
            break;
        case SOUNDCHANNEL_LANGUAGE2:
            source |= 0x10;
        }
    }

    SetDSPRegister(DSP_WR_LDSPK_SOURCE, source);
    SetDSPRegister(DSP_WR_HEADPH_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART1_SOURCE, source);
    SetDSPRegister(DSP_WR_I2S_SOURCE, source);
    // 2. audio baseband

    // 3. volume
    /// \todo FIXME
}

void CMSP34x0Decoder::Reconfigure()
{
    Reset();

    if (!m_bRevisionChecked)
    {
        m_bRevisionChecked = true;
        m_bHasRevD = (GetVersion() & 0x0F00 >= 0x0400);
    }

    if (m_bHasRevD)
    {
        ReconfigureRevA(); /// FIXME D();
    }
    else
    {
        ReconfigureRevA();
    }
}

void CMSP34x0Decoder::ReconfigureRevA()
{
    int i;

    /// SCART Signal Path
    switch (m_AudioInput)
    {
    case AUDIOINPUT_RADIO:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_2);
        break;
    case AUDIOINPUT_EXTERNAL:
		SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_SCART_1);
        break;
    case AUDIOINPUT_MUTE:
        SetSCARTxbar(MSP34x0_SCARTOUTPUT_DSP_INPUT, MSP34x0_SCARTINPUT_MUTE);
        break;
    }

    /// Guess the correct format
    WORD standard;
    switch(m_VideoFormat)
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

    // Find the correct Standard in list
    int MSPStandards(0);
    while(m_MSPStandards[MSPStandards].Standard != standard && 
            m_MSPStandards[MSPStandards].Name != NULL)
    {
        ++MSPStandards;
    }

    /// LOAD_SEQ_1/2: General Initialization followed by LOAD_REG_12

    /// Step 0: AD_CV
    if(m_MSPStandards[MSPStandards].StereoType == STEREO_SAT)
    {
        // set up for AGC bit 7
        // and bits 6..1  100011 for SAT
        SetDEMRegister(DEM_WR_AD_CV, 0xC6);
    }
    else if(m_MSPStandards[MSPStandards].StereoType == STEREO_NICAM && 
                m_MSPStandards[MSPStandards].MonoType == MONO_AM)
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

    /// Step 1: AUDIO_PLL
    // FIXME what should this be may need to be 0 for NICAM
    SetDEMRegister(DEM_WR_AUDIO_PLL, 1);

    /// Step 2: FAWCT_SOLL
    if(m_MSPStandards[MSPStandards].StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_FAWCT_SOLL, 12);
    }
    
    /// Step 3: FAW_ER_TOL
    if(m_MSPStandards[MSPStandards].StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_FAW_ER_TOL, 2);
    }

    /// Step 4: FIR_REG_1
    int FIRType = m_MSPStandards[MSPStandards].FIRType;
    for (i = 5; i >= 0; i--)
    {
        SetDEMRegister(DEM_WR_FIR1, m_FIRTypes[FIRType].FIR1[i]);
    }
    
    /// Step 5: FIR_REG_2
    SetDEMRegister(DEM_WR_FIR2, 0x0004);
    SetDEMRegister(DEM_WR_FIR2, 0x0040);
    SetDEMRegister(DEM_WR_FIR2, 0x0000);
    
    for (i = 5; i >= 0; i--)
    {
        SetDEMRegister(DEM_WR_FIR2, m_FIRTypes[FIRType].FIR2[i]);
    }

    /// Step 6: MODE_REG
    WORD ModeReg = 0x0000;
    if(m_MSPStandards[MSPStandards].StereoType == STEREO_NICAM)
    {
        // set NICAM mode
        ModeReg |= 1 << 6;
        if(m_MSPStandards[MSPStandards].MonoType == MONO_AM)
        {
            // set MSP 1/2 to AM
            ModeReg |= 1 << 8;
        }
    }
    else
    {
        if(m_MSPStandards[MSPStandards].StereoType != STEREO_NONE)
        {
            // set Two carrier FM mode
            ModeReg |= 1 << 10;
        }
        if(m_MSPStandards[MSPStandards].MonoType == MONO_FM)
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

    SetDEMRegister(DEM_WR_MODE_REG, ModeReg);

    /// Step 7, 8, 9 and 10: DCO1_LO, DCO1_HI, DCO2_LO and DCO2_HI
    SetDEMRegister(DEM_WR_DCO1_LO, m_MSPStandards[MSPStandards].MinorCarrier & 0xfff);
    SetDEMRegister(DEM_WR_DCO1_HI, m_MSPStandards[MSPStandards].MinorCarrier >> 12);
    SetDEMRegister(DEM_WR_DCO2_LO, m_MSPStandards[MSPStandards].MajorCarrier & 0xfff);
    SetDEMRegister(DEM_WR_DCO2_HI, m_MSPStandards[MSPStandards].MajorCarrier >> 12);
    
    /// Step 11: start LOAD_REG_12 process
    SetDEMRegister(DEM_WR_LOAD_REG_12, 0);

    /// Step 12 NICAM_START
    if(m_MSPStandards[MSPStandards].StereoType == STEREO_NICAM)
    {
        SetDEMRegister(DEM_WR_SEARCH_NICAM, 0);
    }

    WORD source = 0x0030;
    SetDSPRegister(DSP_WR_LDSPK_SOURCE, source);
    SetDSPRegister(DSP_WR_HEADPH_SOURCE, source);
    SetDSPRegister(DSP_WR_SCART1_SOURCE, source);

    // 2. FM and NICAM prescale
    SetDSPRegister(DSP_WR_FMAM_PRESCALE, 0x3000);
    SetDSPRegister(DSP_WR_NICAM_PRESCALE, 0x5A00);
}

void CMSP34x0Decoder::SetVideoFormat(eVideoFormat videoFormat)
{
    CAudioDecoder::SetVideoFormat(videoFormat);
    Reconfigure();
}

void CMSP34x0Decoder::SetSoundChannel(eSoundChannel soundChannel)
{
    CAudioDecoder::SetSoundChannel(soundChannel);
    Reconfigure();
}

void CMSP34x0Decoder::SetAudioInput(eAudioInput audioInput)
{
    CAudioDecoder::SetAudioInput(audioInput);
    Reconfigure();
}

eSoundChannel CMSP34x0Decoder::IsAudioChannelDetected(eSoundChannel desiredAudioChannel)
{
    eSoundChannel result = desiredAudioChannel;
    /// \todo FIXME
    return result; 
}
