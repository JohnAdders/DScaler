//
// $Id: MSP34x0.cpp,v 1.3 2001-12-03 17:27:56 adcockj Exp $
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

CMSP34x0::CMSP34x0() :
    m_bNicam(FALSE),
    m_nMode(0),
    m_nMajorMode(0), 
    m_nMinorMode(0),
    m_eSoundChannel(MONO)

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
    return ((WORD)result[0]) << 8 | result[1];
}
void CMSP34x0::SetRegister(BYTE subAddress, WORD reg, WORD value)
{
    BYTE write[] = {(reg >> 8) & 0xFF, reg & 0xFF, (value >> 8) & 0xFF, value & 0xFF};
    WriteToSubAddress(subAddress, write, sizeof(write));
}


void CMSP34x0::Reset()
{
    BYTE reset[2] = {0x80, 0};
    WriteToSubAddress(MSP_CONTROL, reset, sizeof(reset));
    reset[0] = 0;
    WriteToSubAddress(MSP_CONTROL, reset, sizeof(reset));
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

void CMSP34x0::SetMute()
{
    SetRegister(MSP_WR_DSP, 0, 0xFF00);
    SetRegister(MSP_WR_DSP, 6, 0xFF00);
}

void CMSP34x0::SetVolume(long nVolume)
{
    if (nVolume < 0 || nVolume > 1000)
    {
        return;
    }
    nVolume = MulDiv(nVolume, 0x7f0, 1000);
    // Use Mute if less than 0x10
    if(nVolume < 0x10)
    {
        nVolume = 0;
    }
    SetRegister(MSP_WR_DSP, 0x00, nVolume << 4);
    SetRegister(MSP_WR_DSP, 0x06, nVolume << 4);
}

void CMSP34x0::SetBalance(long nBalance)
{
    SetRegister(MSP_WR_DSP, 0x01, (nBalance & 0xFF) << 8);
    SetRegister(MSP_WR_DSP, 0x30, (nBalance & 0xFF) << 8);
}

void CMSP34x0::SetBass(long nBass)
{
    if (nBass < -96)
    {
        return;
    }
    SetRegister(MSP_WR_DSP, 0x02, (nBass & 0xFF) << 8);
    SetRegister(MSP_WR_DSP, 0x31, (nBass & 0xFF) << 8);
}

void CMSP34x0::SetTreble(long nTreble)
{
    if (nTreble < -96)
    {
        return;
    }
    SetRegister(MSP_WR_DSP, 0x03, (nTreble & 0xFF) << 8);
    SetRegister(MSP_WR_DSP, 0x32, (nTreble & 0xFF) << 8);
}

void CMSP34x0::SetLoudnessAndSuperBass(long nLoudness, bool bSuperBass)
{
    if (nLoudness > 68)
    {
        return;
    }
    SetRegister(MSP_WR_DSP, 0x04, ((nLoudness & 0xFF) << 8) | (bSuperBass ? 0x4 : 0));
    SetRegister(MSP_WR_DSP, 0x33, ((nLoudness & 0xFF) << 8) | (bSuperBass ? 0x4 : 0));
}

void CMSP34x0::SetSpatialEffects(long nSpatial)
{
    // Mode A, Automatic high pass gain
    SetRegister(MSP_WR_DSP, 0x05, ((nSpatial & 0xFF) << 8) | 0x8);
}

void CMSP34x0::SetEqualizer(long EqIndex, long nLevel)
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

void CMSP34x0::SetCarrier(int cdo1, int cdo2)
{
    SetRegister(MSP_WR_DEM, 0x93, cdo1 & 0xfff);
    SetRegister(MSP_WR_DEM, 0x9b, cdo1 >> 12);
    SetRegister(MSP_WR_DEM, 0xa3, cdo2 & 0xfff);
    SetRegister(MSP_WR_DEM, 0xab, cdo2 >> 12);
    SetRegister(MSP_WR_DEM, 0x56, 0);
}

void CMSP34x0::SetMode(long nMode)
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

void CMSP34x0::SetStereo(eSoundChannel soundChannel)
{
    int nicam = 0;
    int src = 0;
    
    m_eSoundChannel = soundChannel;

    // switch demodulator
    switch (m_nMode)
    {
    case MSP_MODE_FM_TERRA:
        SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        switch (m_eSoundChannel)
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
        switch (m_eSoundChannel)
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
    switch (m_eSoundChannel)
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


void CMSP34x0::SetMajorMinorMode(int majorMode, int minorMode)
{
    m_nMajorMode = majorMode;
    m_nMinorMode = minorMode;

    switch (m_nMajorMode)
    {
    case 1:                 // 5.5
        if (m_nMinorMode == 0)
        {
            // B/G FM-stereo
            SetStereo(MONO);
        }
        else if (m_nMinorMode == 1 && m_bNicam)
        {
            // B/G NICAM
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        else
        {
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        break;
    case 2:                 // 6.0
        // PAL I NICAM
        SetCarrier(MSP_CARRIER(6.55), m_CarrierDetectMajor[m_nMajorMode]);
        break;
    case 3:                 // 6.5
        if (m_nMinorMode == 1 || m_nMinorMode == 2)
        {
            // D/K FM-stereo
            SetStereo(MONO);
        }
        else if (m_nMinorMode == 0 && m_bNicam)
        {
            // D/K NICAM
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        else
        {
            SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        }
        break;
    case 0:                 // 4.5
    default:
        SetCarrier(m_CarrierDetectMinor[m_nMinorMode], m_CarrierDetectMajor[m_nMajorMode]);
        break;
    }
}

void CMSP34x0::GetPrintMode(LPSTR Text)
{
    switch (m_nMode)
    {
    case MSP_MODE_AM_DETECT:
        strcpy(Text, "AM (msp3400)+");
        break;
    case MSP_MODE_AM_DETECT2:
        strcpy(Text, "AM (msp3410)+");
        break;
    case MSP_MODE_FM_RADIO:
        strcpy(Text, "FM Radio+");
        break;
    case MSP_MODE_FM_TERRA:
        strcpy(Text, "TV Terrestial+");
        break;
    case MSP_MODE_FM_SAT:
        strcpy(Text, "TV Sat+");
        break;
    case MSP_MODE_FM_NICAM1:
        strcpy(Text, "NICAM B/G+");
        break;
    case MSP_MODE_FM_NICAM2:
        strcpy(Text, "NICAM I+");
        break;
    case MSP_MODE_AM_NICAM:
        strcpy(Text, "NICAM L+");
        break;
    }
    
    switch (m_nMajorMode)
    {
    case 0:
        strcat(Text, "NTSC+");
        break;
    case 1:
        strcat(Text, "PAL B/G+");
        break;
    case 2:
        strcat(Text, "PAL I+");
        break;
    case 3:
        strcat(Text, "PAL D/K (Sat+Secam)+");
        break;
    }
    
    switch (m_nMinorMode)
    {
    case 0:
        strcat(Text, "FM-stereo ");
        break;
    case 1:
        strcat(Text, "NICAM ");
        break;
    case 2:
        strcat(Text, "NICAM ");
        break;
    case 3:
        strcat(Text, "D/K1 FM-Stereo ");
        break;
    case 4:
        strcat(Text, "D/K2 FM-stereo ");
        break;
    case 5:
        strcat(Text, "SAT FM-stereo s/b ");
        break;
    case 6:
        strcat(Text, "SAT FM-stereo s ");
        break;
    case 7:
        strcat(Text, "SAT FM-stereo b ");
        break;
    }
    
    switch (m_eSoundChannel)
    {
    case MONO:
        strcat(Text, "(Mono)");
        break;
    case STEREO:
        strcat(Text, "(Stereo)");
        break;
    case LANGUAGE1:
        strcat(Text, "(Channel 1)");
        break;
    case LANGUAGE2:
        strcat(Text, "(Channel 2)");
        break;
    }
}

eSoundChannel CMSP34x0::GetWatchMode(eSoundChannel desiredSoundChannel)
{
    eSoundChannel result = desiredSoundChannel;
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
