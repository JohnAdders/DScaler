//
// $Id: MSP34x0.cpp,v 1.5 2001-12-18 23:36:01 adcockj Exp $
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

#define MSP_CONTROL 0x00 // Software reset
#define MSP_TEST    0x01 // Internal use
#define MSP_WR_DEM  0x10 // Write demodulator
#define MSP_RD_DEM  0x11 // Read demodulator
#define MSP_WR_DSP  0x12 // Write DSP
#define MSP_RD_DSP  0x13 // Read DSP

// This macro is allowed for *constants* only, gcc must calculate it
// at compile time.  Remember -- no floats in kernel Mode
#define MSP_CARRIER(freq) ((int)((float)(freq/18.432)*(1<<24)))

typedef struct
{
    int fir1[6];
    int fir2[6];
    int cdo1;
    int cdo2;
    int ad_cv;
    int mode_reg;
    int dfp_src;
    int dfp_matrix;
    int autodetect; // MAE 8 Dec 2000
} TMSPInitData;

static TMSPInitData m_MSPInitData[] =
{
    // AM (for carrier detect / msp3400)
    { 
        { 75, 19, 36, 35, 39, 40 }, 
        { 75, 19, 36, 35, 39, 40 },
        MSP_CARRIER(5.5), MSP_CARRIER(5.5),
        0x00d0, 0x0500,   0x0020, 0x3000, 1
    },
    
    // AM (for carrier detect / msp3410) 
    {
        { -1, -1, -8, 2, 59, 126 }, 
        { -1, -1, -8, 2, 59, 126 },
        MSP_CARRIER(5.5), MSP_CARRIER(5.5),
        0x00d0, 0x0100,   0x0020, 0x3000, 0
    },
    // FM Radio 
    {
        { -8, -8, 4, 6, 78, 107 }, 
        { -8, -8, 4, 6, 78, 107 },
        MSP_CARRIER(10.7), MSP_CARRIER(10.7),
        0x00d0, 0x0480, 0x0020, 0x3000, 0 
    },
    // Terrestial FM-mono + FM-stereo 
    { 
        {  3, 18, 27, 48, 66, 72 }, 
        {  3, 18, 27, 48, 66, 72 },
        MSP_CARRIER(5.5), MSP_CARRIER(5.5),
        0x00d0, 0x0480,   0x0030, 0x3000, 0
    }, 
    // Sat FM-mono 
    { 
        {  1,  9, 14, 24, 33, 37 }, 
        {  3, 18, 27, 48, 66, 72 },
        MSP_CARRIER(6.5), MSP_CARRIER(6.5),
        0x00c6, 0x0480,   0x0000, 0x3000, 0
    },
    // NICAM B/G, D/K 
    {
        { -2, -8, -10, 10, 50, 86 }, 
        {  3, 18, 27, 48, 66, 72 },
        MSP_CARRIER(5.5), MSP_CARRIER(5.5),
        0x00d0, 0x0040,   0x0120, 0x3000, 0
    },
    // NICAM I 
    { 
        {  2, 4, -6, -4, 40, 94 }, 
        {  3, 18, 27, 48, 66, 72 },
        MSP_CARRIER(6.0), MSP_CARRIER(6.0),
        0x00d0, 0x0040,   0x0120, 0x3000, 0
    },
    // NICAM/AM -- L (6.5/5.85) 
    {
        {  -2, -8, -10, 10, 50, 86 }, 
        {  -4, -12, -9, 23, 79, 126 },
        MSP_CARRIER(6.5), MSP_CARRIER(6.5),
        0x00c6, 0xA170,   0x0200, 0x7000, 0
    },
};

static int m_CarrierDetectMajor[4] = 
{
    // main carrier 
    MSP_CARRIER(4.5),   // 4.5   NTSC
    MSP_CARRIER(5.5),   // 5.5   PAL B/G
    MSP_CARRIER(6.0),   // 6.0   PAL I
    MSP_CARRIER(6.5),   // 6.5   PAL D/K + SAT + SECAM
};

static int m_CarrierDetectMinor[8] = 
{
    // PAL B/G 
    MSP_CARRIER(5.7421875), // 5.742 PAL B/G FM-stereo
    MSP_CARRIER(5.85),      // 5.85  PAL B/G NICAM
    // PAL SAT / SECAM 
    MSP_CARRIER(5.85),      // 5.85  PAL D/K NICAM
    MSP_CARRIER(6.2578125), //6.25  PAL D/K1 FM-stereo
    MSP_CARRIER(6.7421875), //6.74  PAL D/K2 FM-stereo
    MSP_CARRIER(7.02),      //7.02  PAL SAT FM-stereo s/b
    MSP_CARRIER(7.20),      //7.20  PAL SAT FM-stereo s
    MSP_CARRIER(7.38),      //7.38  PAL SAT FM-stereo b
};

CMSP34x0::CMSP34x0()
{
    m_bNicam = FALSE;
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
    return ((WORD)result[0]) << 8 | result[1];
}
void CMSP34x0::SetRegister(BYTE subAddress, WORD reg, WORD value)
{
    BYTE write[] = {(reg >> 8) & 0xFF, reg & 0xFF, (value >> 8) & 0xFF, value & 0xFF};
    WriteToSubAddress(subAddress, write, sizeof(write));
}

CMSP34x0Controls::CMSP34x0Controls() : CMSP34x0()
{
    m_Muted = false;
    m_Volume = 900;
    m_Balance = 0;
    m_Bass = 0;
    m_Treble = 0;
}


void CMSP34x0Controls::Reset()
{
    BYTE reset[2] = {0x80, 0};
    WriteToSubAddress(MSP_CONTROL, reset, sizeof(reset));
    reset[0] = 0;
    WriteToSubAddress(MSP_CONTROL, reset, sizeof(reset));
	reset[0] = 1;
}

WORD CMSP34x0::GetVersion()
{
    return GetRegister(MSP_RD_DSP, 0x1E);
}

WORD CMSP34x0::GetProductCode()
{
    WORD result = GetRegister(MSP_RD_DSP, 0x1F);
    m_bNicam = (((result >> 8) & 0xff) != 00) ? true : false;
    return result;
}

void CMSP34x0Controls::SetLoudnessAndSuperBass(long nLoudness, bool bSuperBass)
{
    if (nLoudness > 68)
    {
        return;
    }
    SetRegister(MSP_WR_DSP, 0x04, ((nLoudness & 0xFF) << 8) | (bSuperBass ? 0x4 : 0));
    SetRegister(MSP_WR_DSP, 0x33, ((nLoudness & 0xFF) << 8) | (bSuperBass ? 0x4 : 0));
}

void CMSP34x0Controls::SetSpatialEffects(long nSpatial)
{
    // Mode A, Automatic high pass gain
    SetRegister(MSP_WR_DSP, 0x05, ((nSpatial & 0xFF) << 8) | 0x8);
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
    SetRegister(MSP_WR_DSP, 0x21 + EqIndex, (nLevel & 0xFF) << 8);
}

CMSP34x0Decoder::CMSP34x0Decoder() : CAudioDecoder(), CMSP34x0()
{
    m_nMode = 0;
    m_nMajorMode = MSP34x0_MAJORMODE_NTSC;
    m_nMinorMode = MSP34x0_MINORMODE_PAL_BG_FM;
}

void CMSP34x0Decoder::SetCarrier(int cdo1, int cdo2)
{
    SetRegister(MSP_WR_DEM, 0x93, cdo1 & 0xfff);
    SetRegister(MSP_WR_DEM, 0x9b, cdo1 >> 12);
    SetRegister(MSP_WR_DEM, 0xa3, cdo2 & 0xfff);
    SetRegister(MSP_WR_DEM, 0xab, cdo2 >> 12);
    SetRegister(MSP_WR_DEM, 0x56, 0);
}

void CMSP34x0Decoder::SetMode(long nMode)
{
    int i;
    
    m_nMode = nMode;
    SetRegister(MSP_WR_DEM, 0xbb, m_MSPInitData[m_nMode].ad_cv);
    
    for (i = 5; i >= 0; i--)
    {
        SetRegister(MSP_WR_DEM, 0x01, m_MSPInitData[m_nMode].fir1[i]);
    }
    
    SetRegister(MSP_WR_DEM, 0x05, 0x0004);     // fir 2 
    SetRegister(MSP_WR_DEM, 0x05, 0x0040);
    SetRegister(MSP_WR_DEM, 0x05, 0x0000);
    
    for (i = 5; i >= 0; i--)
    {
        SetRegister(MSP_WR_DEM, 0x05, m_MSPInitData[m_nMode].fir2[i]);
    }
    
    SetRegister(MSP_WR_DEM, 0x83, m_MSPInitData[m_nMode].mode_reg);
    
    SetCarrier(m_MSPInitData[m_nMode].cdo1, m_MSPInitData[m_nMode].cdo2);
    
    SetRegister(MSP_WR_DSP, 0x08, m_MSPInitData[m_nMode].dfp_src);
    SetRegister(MSP_WR_DSP, 0x09, m_MSPInitData[m_nMode].dfp_src);
    SetRegister(MSP_WR_DSP, 0x0a, m_MSPInitData[m_nMode].dfp_src);
    SetRegister(MSP_WR_DSP, 0x0e, m_MSPInitData[m_nMode].dfp_matrix);
    
    if (m_bNicam)
    {
        SetRegister(MSP_WR_DSP, 0x10, 0x5a00);
    }
}

///////
///////
void CMSP34x0Controls::SetMute(bool mute)
{
    m_Muted = mute;
    if (m_Muted)
    {
        SetRegister(MSP_WR_DSP, 0, 0xFF00);
        SetRegister(MSP_WR_DSP, 6, 0xFF00);
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
    SetRegister(MSP_WR_DSP, 0x00, nVolume << 4);
    SetRegister(MSP_WR_DSP, 0x06, nVolume << 4);
}

WORD CMSP34x0Controls::GetVolume()
{
    return m_Volume;
}

void CMSP34x0Controls::SetBalance(WORD nBalance)
{
    m_Balance = nBalance;
    SetRegister(MSP_WR_DSP, 0x01, (nBalance & 0xFF) << 8);
    SetRegister(MSP_WR_DSP, 0x30, (nBalance & 0xFF) << 8);
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
    SetRegister(MSP_WR_DSP, 0x02, (nBass & 0xFF) << 8);
    SetRegister(MSP_WR_DSP, 0x31, (nBass & 0xFF) << 8);
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
    SetRegister(MSP_WR_DSP, 0x03, (nTreble & 0xFF) << 8);
    SetRegister(MSP_WR_DSP, 0x32, (nTreble & 0xFF) << 8);
}

WORD CMSP34x0Controls::GetTreble()
{
    return m_Treble;
}

///////
///////

void CMSP34x0Decoder::SetVideoFormat(eVideoFormat videoFormat)
{
    CAudioDecoder::SetVideoFormat(videoFormat);
/// \todo FIXME FIXME FIXME
    /*
    switch (m_VideoFormat)
    {
    case VIDEOFORMAT_NTSC_M:
    m_nMajorMode = ???;
    m_nMinorMode = ???;
    break;
    }
    */
}


void CMSP34x0Decoder::SetSoundChannel(eSoundChannel soundChannel)
{
    int nicam = 0;
    int src = 0;
    
    CAudioDecoder::SetSoundChannel(soundChannel);
    
    // switch demodulator
    switch (m_nMode)
    {
    case MSP_MODE_FM_TERRA:
        SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        switch (m_SoundChannel)
        {
        case STEREO:
            SetRegister(MSP_WR_DSP, 0x15, 0x0000); // HC 22/Feb/2001 Identification Mode B/G
            SetRegister(MSP_WR_DSP, 0x0e, 0x3001); // HC 22/Feb/2001 
            SetRegister(MSP_WR_DSP, 0xbb, 0x00d0); // HC 22/Feb/2001 AGC On
            break;
        case MONO:
        case LANGUAGE1:
        case LANGUAGE2:
            SetRegister(MSP_WR_DSP, 0x0e, 0x3000); // MAE 8 Dec 2000
            break;
        }
        break;
        case MSP_MODE_FM_SAT:
            switch (m_SoundChannel)
            {
            case MONO:
                SetCarrier(MSP_CARRIER(6.5), MSP_CARRIER(6.5));
                break;
            case STEREO:
                SetCarrier(MSP_CARRIER(7.2), MSP_CARRIER(7.02));
                break;
            case LANGUAGE1:
                SetCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
                break;
            case LANGUAGE2:
                SetCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
                break;
            }
            break;
            case MSP_MODE_FM_NICAM1:
            case MSP_MODE_FM_NICAM2:
            case MSP_MODE_AM_NICAM:
                SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
                nicam = 0x0100;
                break;
            default:
                // can't do stereo - abort here
                return;
    }
    
    // switch audio
    switch (m_SoundChannel)
    {
    case STEREO:
        src = 0x0020 | nicam;
        break;
    case MONO:
        if (m_nMode == MSP_MODE_AM_NICAM) // AM Sound on Hauppauge french Nicam cards
        {
            // AM mono decoding is handled by tuner, not MSP chip
            // so let's redirect sound from tuner via SCART
            // volume prescale for SCART
            SetRegister(MSP_WR_DSP, 0x0d, 0x1000);
            // SCART switching control register
            SetRegister(MSP_WR_DSP, 0x13, 0xe900);
            src=0x0200;
            break;
        } 
         // otherwise drop down 
    case LANGUAGE1:
        src = 0x0000 | nicam;
        break;
    case LANGUAGE2:
        src = 0x0010 | nicam;
        break;
    }
    SetRegister(MSP_WR_DSP, 0x08, src);
    SetRegister(MSP_WR_DSP, 0x09, src);
    SetRegister(MSP_WR_DSP, 0x0a, src);
}


void CMSP34x0Decoder::SetMajorMinorMode(eMajorMode majorMode, eMinorMode minorMode)
{
    m_nMajorMode = majorMode;
    m_nMinorMode = minorMode;

    switch (m_nMajorMode)
    {
    case MSP34x0_MAJORMODE_PAL_BG:                 // 5.5 (1)
        if (m_nMinorMode == MSP34x0_MINORMODE_PAL_BG_FM)
        {
            // B/G FM-stereo
            SetSoundChannel(MONO);
        }
        else if (m_nMinorMode == MSP34x0_MINORMODE_PAL_BG_NICAM && m_bNicam)
        {
            // B/G NICAM
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        else
        {
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        break;
    case MSP34x0_MAJORMODE_PAL_I:                 // 6.0
        // PAL I NICAM
        SetCarrier(MSP_CARRIER(6.55), m_CarrierDetectMajor[m_nMajorMode]);
        break;
    case MSP34x0_MAJORMODE_PAL_DK_SECAM_SAT:                 // 6.5
        if (m_nMinorMode == MSP34x0_MINORMODE_PAL_BG_NICAM || m_nMinorMode == MSP34x0_MINORMODE_PAL_DK_NICAM)
        {
            // D/K FM-stereo
            SetSoundChannel(MONO);
        }
        else if (m_nMinorMode == MSP34x0_MINORMODE_PAL_BG_FM && m_bNicam)
        {
            // D/K NICAM
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        else
        {
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        break;
    case MSP34x0_MAJORMODE_NTSC:                 // 4.5
    default:
        SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        break;
    }
}


void CMSP34x0Decoder::SetAudioInput(eAudioInput audioInput)
{
    CAudioDecoder::SetAudioInput(audioInput);
}

eSoundChannel CMSP34x0Decoder::IsAudioChannelDetected(eSoundChannel desiredAudioChannel)
{
    eSoundChannel result = desiredAudioChannel;
    int val;
    
    switch (m_nMode)
    {
    case MSP_MODE_FM_TERRA:
        val = GetRegister(MSP_RD_DSP, 0x18);
        if (val > 4096)
        {
            result = STEREO;
        }
        else if (val < -4096)
        {
            result = LANGUAGE1;
        }
        else
        {
            result = MONO;
        }
        break;
    case MSP_MODE_FM_NICAM1:
    case MSP_MODE_FM_NICAM2:
        val = GetRegister(MSP_RD_DSP, 0x23);
        switch ((val & 0x1e) >> 1)
        {
        case 0:
        case 8:
            result = STEREO;
            break;
        default:
            result = MONO;
            break;
        }
        break;
    case MSP_MODE_AM_NICAM:
        result = MONO;
        if(GetRegister(MSP_RD_DEM, 0x57) < 700)
        {
            result = STEREO;
        }
        break;
    }
    return result; 
}
