/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Audio.cpp,v 1.2 2001-11-02 16:30:07 adcockj Exp $
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
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BT848Card.h"
#include "BT848_Defines.h"

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

TMSPInitData MSP_init_data[] =
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
        0x00c6, 0x0140,   0x0120, 0x7c03
    },
};


int carrier_detect_main[4] = 
{
    // main carrier 
    MSP_CARRIER(4.5),   // 4.5   NTSC
    MSP_CARRIER(5.5),   // 5.5   PAL B/G
    MSP_CARRIER(6.0),   // 6.0   PAL I
    MSP_CARRIER(6.5),   // 6.5   PAL D/K + SAT + SECAM
};

int CarrierDetect[8] = 
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


BOOL CBT848Card::HasMSP()
{
    return m_bHasMSP;
}

void CBT848Card::WriteMSP(BYTE bSubAddr, int wAddr, int wData)
{
    if(m_bHasMSP)
    {
        I2C_Lock();
        I2C_Start();
        I2C_SendByte(BT848_MSP_WRITE, 0);
        I2C_SendByte(bSubAddr, 0);
        if (bSubAddr != MSP_CONTROL && bSubAddr != MSP_TEST)
        {
            I2C_SendByte((BYTE) (wAddr >> 8), 0);
            I2C_SendByte((BYTE) (wAddr & 0xFF), 0);
        }
        I2C_SendByte((BYTE) (wData >> 8), 0);
        I2C_SendByte((BYTE) (wData & 0xFF), 0);
        I2C_Stop();
        I2C_Unlock();
    }
}

WORD CBT848Card::ReadMSP(BYTE bSubAddr, WORD wAddr)
{
    if(m_bHasMSP)
    {
        WORD wResult;
        BYTE B0, B1;

        B0 = (BYTE) (wAddr >> 8);
        B1 = (BYTE) (wAddr & 0xFF);
        I2C_Lock();
        I2C_Start();
        I2C_SendByte(BT848_MSP_WRITE, 2);
        I2C_SendByte(bSubAddr, 0);
        I2C_SendByte(B0, 0);
        I2C_SendByte(B1, 0);
        I2C_Start();
        if (I2C_SendByte(BT848_MSP_READ, 2))
        {
            B0 = I2C_ReadByte(0);
            B1 = I2C_ReadByte(1);
            wResult = B0 << 8 | B1;
        }
        else
        {
            wResult = -1;
        }
        I2C_Stop();
        I2C_Unlock();
        return wResult;
    }
    else
    {
        return -1;
    }
}

void CBT848Card::InitMSP()
{
    if (!I2C_AddDevice(BT848_MSP_READ))
    {
        m_bHasMSP = FALSE;
        return;
    }

    if (!I2C_AddDevice(BT848_MSP_WRITE))
    {
        m_bHasMSP = FALSE;
        return;
    }

    m_bHasMSP = TRUE;

    ResetMSP();
    Sleep(4);

    // set volume to Mute level
    WriteMSP(MSP_WR_DSP, 0, 0x00 << 4);
    WriteMSP(MSP_WR_DSP, 6, 0x00 << 4);

    // setup version information
    int rev1, rev2;

    m_MSPNicam = FALSE;

    rev1 = ReadMSP(MSP_WR_DSP, 0x1e);
    rev2 = ReadMSP(MSP_WR_DSP, 0x1f);
    if (0 == rev1 && 0 == rev2)
    {
        return;
    }

    sprintf(m_MSPVersion, "MSP34%02d%c-%c%d", (rev2 >> 8) & 0xff, (rev1 & 0xff) + '@', ((rev1 >> 8) & 0xff) + '@', rev2 & 0x1f);
    m_MSPNicam = (((rev2 >> 8) & 0xff) != 00) ? 1 : 0;
}

BOOL CBT848Card::ResetMSP()
{
    BOOL ret = TRUE;

    I2C_Lock();
    I2C_Start();
    I2C_SendByte(BT848_MSP_WRITE, 5);
    I2C_SendByte(0x00, 0);
    I2C_SendByte(0x80, 0);
    I2C_SendByte(0x00, 0);
    I2C_Stop();
    I2C_Start();
    if ((I2C_SendByte(BT848_MSP_WRITE, 5) == FALSE) ||
        (I2C_SendByte(0x00, 0) == FALSE) ||
        (I2C_SendByte(0x00, 0) == FALSE) ||
        (I2C_SendByte(0x00, 0) == FALSE))
    {
        ret = FALSE;
    }

    I2C_Stop();
    I2C_Unlock();

    return ret;
}

void CBT848Card::Mute()
{
    if(m_bHasMSP)
    {
        int nVolume;
        // use fast mute
        // see MSP docs
        nVolume = 0xFFE;
        WriteMSP(MSP_WR_DSP, 0, nVolume << 4);
        WriteMSP(MSP_WR_DSP, 6, nVolume << 4);
    }
}

void CBT848Card::UnMute(long nVolume)
{
    if(m_bHasMSP)
    {
        // go back from mute to same volums as before
        SetMSPVolume(nVolume);
    }
}


void CBT848Card::SetMSPVolume(long nVolume)
{
    if(m_bHasMSP == TRUE)
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
        WriteMSP(MSP_WR_DSP, 0, nVolume << 4);
        WriteMSP(MSP_WR_DSP, 6, nVolume << 4);
    }
}


void CBT848Card::SetMSPBalance(long nBalance)
{
    if(m_bHasMSP == TRUE)
    {
        WriteMSP(MSP_WR_DSP, 1, nBalance << 8);
        WriteMSP(MSP_WR_DSP, 0x30, nBalance << 8);
    }
}

void CBT848Card::SetMSPBass(long nBass)
{
    if(m_bHasMSP == TRUE)
    {
        if (nBass < -96)
        {
            return;
        }
        WriteMSP(MSP_WR_DSP, 2, nBass << 8);
        WriteMSP(MSP_WR_DSP, 0x31, nBass << 8);
    }
}

void CBT848Card::SetMSPTreble(long nTreble)
{
    if(m_bHasMSP == TRUE)
    {
        if (nTreble < -96)
        {
            return;
        }
        WriteMSP(MSP_WR_DSP, 3, nTreble << 8);
        WriteMSP(MSP_WR_DSP, 0x32, nTreble << 8);
    }
}

void CBT848Card::SetMSPSuperBassLoudness(long nLoudness, BOOL bSuperBass)
{
    if(m_bHasMSP == TRUE)
    {
        if (nLoudness > 68)
        {
            return;
        }
        WriteMSP(MSP_WR_DSP, 4, (nLoudness << 8) + (bSuperBass ? 0x4 : 0));
        WriteMSP(MSP_WR_DSP, 0x33, (nLoudness << 8) + (bSuperBass ? 0x4 : 0));
    }
}


void CBT848Card::SetMSPSpatial(long nSpatial)
{
    if(m_bHasMSP == TRUE)
    {
        WriteMSP(MSP_WR_DSP, 0x5, (nSpatial << 8) + 0x8);   // Mode A, Automatic high pass gain
    }
}

void CBT848Card::SetMSPEqualizer(long EqIndex, long nLevel)
{
    if(m_bHasMSP == TRUE)
    {
        if(EqIndex < 0 || EqIndex > 4)
        {
            return;
        }
        if (nLevel < -96 || nLevel > 96)
        {
            return;
        }
        WriteMSP(MSP_WR_DSP, 0x21 + EqIndex, nLevel << 8);
    }
}

void CBT848Card::SetMSPCarrier(int cdo1, int cdo2)
{
    if(m_bHasMSP == TRUE)
    {
        WriteMSP(MSP_WR_DEM, 0x93, cdo1 & 0xfff);
        WriteMSP(MSP_WR_DEM, 0x9b, cdo1 >> 12);
        WriteMSP(MSP_WR_DEM, 0xa3, cdo2 & 0xfff);
        WriteMSP(MSP_WR_DEM, 0xab, cdo2 >> 12);
        WriteMSP(MSP_WR_DEM, 0x56, 0);
    }
}

void CBT848Card::SetMSPMode(long MSPMode)
{
    if(m_bHasMSP == TRUE)
    {
        int i;

        WriteMSP(MSP_WR_DEM, 0xbb, MSP_init_data[MSPMode].ad_cv);

        for (i = 5; i >= 0; i--)
        {
            WriteMSP(MSP_WR_DEM, 0x01, MSP_init_data[MSPMode].fir1[i]);
        }

        WriteMSP(MSP_WR_DEM, 0x05, 0x0004);     // fir 2 
        WriteMSP(MSP_WR_DEM, 0x05, 0x0040);
        WriteMSP(MSP_WR_DEM, 0x05, 0x0000);

        for (i = 5; i >= 0; i--)
        {
            WriteMSP(MSP_WR_DEM, 0x05, MSP_init_data[MSPMode].fir2[i]);
        }

        WriteMSP(MSP_WR_DEM, 0x83, MSP_init_data[MSPMode].mode_reg);

        SetMSPCarrier(MSP_init_data[MSPMode].cdo1, MSP_init_data[MSPMode].cdo2);

        WriteMSP(MSP_WR_DSP, 0x08, MSP_init_data[MSPMode].dfp_src);
        WriteMSP(MSP_WR_DSP, 0x09, MSP_init_data[MSPMode].dfp_src);
        WriteMSP(MSP_WR_DSP, 0x0a, MSP_init_data[MSPMode].dfp_src);
        WriteMSP(MSP_WR_DSP, 0x0e, MSP_init_data[MSPMode].dfp_matrix);

        // msp3410 needs some more initialization
        if (m_MSPNicam)
        {
            WriteMSP(MSP_WR_DSP, 0x10, 0x5a00);
        }
    }
}

void CBT848Card::SetMSPStereo(int MajorMode, int MinorMode, int StereoMode, int MSPMode)
{
    if(m_bHasMSP == TRUE)
    {
        int nicam = 0;
        int src = 0;

        // switch demodulator
        switch (MSPMode)
        {
        case MSP_MODE_FM_TERRA:
            SetMSPCarrier(CarrierDetect[MinorMode], carrier_detect_main[MajorMode]);
            switch (StereoMode)
            {
            case VIDEO_SOUND_STEREO:
                WriteMSP(MSP_WR_DSP, 0x15, 0x0000); // HC 22/Feb/2001 Identification Mode B/G
                WriteMSP(MSP_WR_DSP, 0x0e, 0x3001); // HC 22/Feb/2001 
                WriteMSP(MSP_WR_DSP, 0xbb, 0x00d0); // HC 22/Feb/2001 AGC On
                break;
            case VIDEO_SOUND_MONO:
            case VIDEO_SOUND_LANG1:
            case VIDEO_SOUND_LANG2:
                WriteMSP(MSP_WR_DSP, 0x0e, 0x3000); // MAE 8 Dec 2000
                break;
            }
            break;
        case MSP_MODE_FM_SAT:
            switch (StereoMode)
            {
            case VIDEO_SOUND_MONO:
                SetMSPCarrier(MSP_CARRIER(6.5), MSP_CARRIER(6.5));
                break;
            case VIDEO_SOUND_STEREO:
                SetMSPCarrier(MSP_CARRIER(7.2), MSP_CARRIER(7.02));
                break;
            case VIDEO_SOUND_LANG1:
                SetMSPCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
                break;
            case VIDEO_SOUND_LANG2:
                SetMSPCarrier(MSP_CARRIER(7.38), MSP_CARRIER(7.02));
                break;
            }
            break;
        case MSP_MODE_FM_NICAM1:
        case MSP_MODE_FM_NICAM2:
            SetMSPCarrier(CarrierDetect[MinorMode], carrier_detect_main[MajorMode]);
            nicam = 0x0100;
            break;
        default:
            // can't do stereo - abort here
            return;
        }

        // switch audio
        switch (StereoMode)
        {
        case VIDEO_SOUND_STEREO:
            src = 0x0020 | nicam;
            break;
        case VIDEO_SOUND_MONO:
        case VIDEO_SOUND_LANG1:
            src = 0x0000 | nicam;
            break;
        case VIDEO_SOUND_LANG2:
            src = 0x0010 | nicam;
            break;
        }
        WriteMSP(MSP_WR_DSP, 0x08, src);
        WriteMSP(MSP_WR_DSP, 0x09, src);
        WriteMSP(MSP_WR_DSP, 0x0a, src);
    }
    else
    {
        switch(m_CardType)
        {
        case TVCARD_GVBCTV3PCI:
            SetAudioGVBCTV3PCI(StereoMode);
            break;
        case TVCARD_LIFETEC:
            SetAudioLT9415(StereoMode);
            break;
        case TVCARD_TERRATV:
            SetAudioTERRATV(StereoMode);
            break;
        case TVCARD_AVERMEDIA:
            SetAudioAVER_TVPHONE(StereoMode);
            break;
        case TVCARD_WINFAST2000:
            SetAudioWINFAST2000(StereoMode);
            break;
        default:
            break;
        }
    }
}


void CBT848Card::SetMSPMajorMinorMode(int MajorMode, int MinorMode, int MSPMode)
{
    if(m_bHasMSP == TRUE)
    {
        switch (MajorMode)
        {
        case 1:                 // 5.5
            if (MinorMode == 0)
            {
                // B/G FM-stereo
                SetMSPStereo(MajorMode, MinorMode, VIDEO_SOUND_MONO, MSPMode);
            }
            else if (MinorMode == 1 && m_MSPNicam)
            {
                // B/G NICAM
                SetMSPCarrier(CarrierDetect[MinorMode], carrier_detect_main[MajorMode]);
            }
            else
            {
                SetMSPCarrier(CarrierDetect[MinorMode], carrier_detect_main[MajorMode]);
            }
            break;
        case 2:                 // 6.0
            // PAL I NICAM
            SetMSPCarrier(MSP_CARRIER(6.55), carrier_detect_main[MajorMode]);
            break;
        case 3:                 // 6.5
            if (MinorMode == 1 || MinorMode == 2)
            {
                // D/K FM-stereo
                SetMSPStereo(MajorMode, MinorMode, VIDEO_SOUND_MONO, MSPMode);
            }
            else if (MinorMode == 0 && m_MSPNicam)
            {
                // D/K NICAM
                SetMSPCarrier(CarrierDetect[MinorMode], carrier_detect_main[MajorMode]);
            }
            else
            {
                SetMSPCarrier(CarrierDetect[MinorMode], carrier_detect_main[MajorMode]);
            }
            break;
        case 0:                 // 4.5
        default:
            SetMSPCarrier(CarrierDetect[MinorMode], carrier_detect_main[MajorMode]);
            break;
        }
    }
}



void CBT848Card::SetAudioGVBCTV3PCI(int StereoMode)
{
    OrDataDword(BT848_GPIO_DATA, 0x300);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x200, ~0x300);
        break;
    case VIDEO_SOUND_LANG2:
        AndOrDataDword(BT848_GPIO_DATA, 0x300, ~0x300);
        break;
    default:
    case VIDEO_SOUND_LANG1:
        AndOrDataDword(BT848_GPIO_DATA, 0x000, ~0x300);
        break;
    }
}

void CBT848Card::SetAudioLT9415(int StereoMode)
{
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x0880, ~0x0880);
        break;
    case VIDEO_SOUND_LANG2:
        AndOrDataDword(BT848_GPIO_DATA, 0x0080, ~0x0880);
        break;
    default:
    case VIDEO_SOUND_LANG1:
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x0880);
        break;
    }
}

void CBT848Card::SetAudioTERRATV(int StereoMode)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x180000, ~0x180000);
        break;
    case VIDEO_SOUND_LANG2:
        AndOrDataDword(BT848_GPIO_DATA, 0x080000, ~0x180000);
        break;
    default:
    case VIDEO_SOUND_LANG1:
        AndOrDataDword(BT848_GPIO_DATA, 0x000000, ~0x180000);
        break;
    }
}

void CBT848Card::SetAudioAVER_TVPHONE(int StereoMode)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x01, ~0x03);
        break;
    case VIDEO_SOUND_LANG1:
        AndOrDataDword(BT848_GPIO_DATA, 0x02, ~0x03);
        break;
    default:
        break;
    }
}

void CBT848Card::SetAudioWINFAST2000(int StereoMode)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x020000, ~0x430000);
        break;
    case VIDEO_SOUND_LANG1:
        AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    case VIDEO_SOUND_LANG2:
        AndOrDataDword(BT848_GPIO_DATA, 0x410000, ~0x430000);
        break;
    default:
        AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    }
}


void CBT848Card::GetMSPPrintMode(LPSTR Text, int MSPMajorMode, int MSPMinorMode, int MSPMode, int MSPStereo)
{
    if (m_bHasMSP == FALSE)
    {
        strcpy(Text, "No MSP Audio Device");
    }
    else
    {
        switch (MSPMode)
        {
        case 0:
            strcpy(Text, "AM (msp3400)+");
            break;
        case 1:
            strcpy(Text, "AM (msp3410)+");
            break;
        case 2:
            strcpy(Text, "FM Radio+");
            break;
        case 3:
            strcpy(Text, "TV Terrestial+");
            break;
        case 4:
            strcpy(Text, "TV Sat+");
            break;
        case 5:
            strcpy(Text, "NICAM B/G+");
            break;
        case 6:
            strcpy(Text, "NICAM I+");
            break;
        }

        switch (MSPMajorMode)
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

        switch (MSPMinorMode)
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

        switch (MSPStereo)
        {
        case 1:
            strcat(Text, "(Mono)");
            break;
        case 2:
            strcat(Text, "(Stereo)");
            break;
        case 3:
            strcat(Text, "(Channel 1)");
            break;
        case 4:
            strcat(Text, "(Channel 2)");
            break;
        }
    }
}

int CBT848Card::GetMSPWatchMode(int MSPMode, int MSPStereo)
{
    int newstereo = MSPStereo;
    int val;
    
    if(!m_bHasMSP)
    {
        return MSPStereo;
    }
    switch (MSPMode)
    {
    case MSP_MODE_FM_TERRA:

        val = ReadMSP(MSP_WR_DSP, 0x18);
        if (val > 4096)
        {
            newstereo = VIDEO_SOUND_STEREO;
        }
        else if (val < -4096)
        {
            newstereo = VIDEO_SOUND_LANG1;
        }
        else
        {
            newstereo = VIDEO_SOUND_MONO;
        }
        break;
    case MSP_MODE_FM_NICAM1:
    case MSP_MODE_FM_NICAM2:
        val = ReadMSP(MSP_WR_DSP, 0x23);
        switch ((val & 0x1e) >> 1)
        {
        case 0:
        case 8:
            newstereo = VIDEO_SOUND_STEREO;
            break;
        default:
            newstereo = VIDEO_SOUND_MONO;
            break;
        }
        break;
    }
    return newstereo; 
}
