/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file CX2388xCard.cpp CCX2388xCard Implementation (Holo3d)
 */

#include "stdafx.h"

#ifdef WANT_CX2388X_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "DebugLog.h"

enum eHolo3DInputs
{
    H3D_COMPONENT,
    H3D_RGsB,
    H3D_SVIDEO,
    H3D_SDI,
    H3D_COMPOSITE1,
    H3D_COMPOSITE2,
    H3D_COMPOSITE3,
    H3D_COMPOSITE4,
    H3D_PDI,
};

void CCX2388xCard::InitH3D()
{
    m_SAA7118 = new CSAA7118();

    m_SAA7118->SetI2CBus(m_I2CBus);

    if(m_SAA7118->GetVersion() > 0)
    {
        m_SAA7118->DumpSettings("SAA7118_H3D.txt");
    }

    FILE* hFile = fopen("FPGA.txt", "w");
    if(!hFile)
    {
        return;
    }

    DWORD i;
    for(i = 0x390000; i <= 0x39000F; ++i)
    {
        fprintf(hFile, "%06x\t%02x\n", i, ReadByte(i));
    }
    for(i = 0x39000d; i <= 0x390FFd; i += 0x10)
    {
        fprintf(hFile, "%06x\t%02x\n", i, ReadByte(i));
    }

    fclose(hFile);

    m_SAA7118->SetRegister(0x01, 0x47);
    m_SAA7118->SetRegister(0x03, 0x20);
    m_SAA7118->SetRegister(0x04, 0x90);
    m_SAA7118->SetRegister(0x05, 0x90);
    m_SAA7118->SetRegister(0x06, 0xeb);
    m_SAA7118->SetRegister(0x07, 0xe0);
    m_SAA7118->SetRegister(0x08, 0xb8);
    m_SAA7118->SetRegister(0x11, 0x00);
    m_SAA7118->SetRegister(0x12, 0x00);
    m_SAA7118->SetRegister(0x13, 0x00);
    m_SAA7118->SetRegister(0x14, 0x00);
    m_SAA7118->SetRegister(0x15, 0x11);
    m_SAA7118->SetRegister(0x16, 0xfe);
    m_SAA7118->SetRegister(0x17, 0xc0);
    m_SAA7118->SetRegister(0x18, 0x40);
    m_SAA7118->SetRegister(0x19, 0x80);

    m_SAA7118->SetRegister(0x23, 0x00);
    m_SAA7118->SetRegister(0x24, 0x90);
    m_SAA7118->SetRegister(0x25, 0x90);
    m_SAA7118->SetRegister(0x29, 0x00);
    m_SAA7118->SetRegister(0x2D, 0x00);
    m_SAA7118->SetRegister(0x2E, 0x00);
    m_SAA7118->SetRegister(0x2F, 0x00);

    m_SAA7118->SetRegister(0x40, 0x40);
    for(BYTE j(0x41); j <= 0x57; ++j)
    {
        m_SAA7118->SetRegister(j, 0xFF);
    }
    m_SAA7118->SetRegister(0x58, 0x00);
    m_SAA7118->SetRegister(0x59, 0x00);
    m_SAA7118->SetRegister(0x5C, 0x00);
    m_SAA7118->SetRegister(0x5D, 0x00);
    m_SAA7118->SetRegister(0x5E, 0x00);
    m_SAA7118->SetRegister(0x5F, 0x00);

    // 2
    // stop the scaler
    m_SAA7118->SetRegister(0x88,0x00);
    m_SAA7118->SetRegister(0x83,0x00); // Xport Output Disable (i.e. Enable Input)

    m_SAA7118->SetRegister(0x84,0x00);
    m_SAA7118->SetRegister(0x85,0x00);
    m_SAA7118->SetRegister(0x86,0x40);
    m_SAA7118->SetRegister(0x87,0x01);

    m_SAA7118->SetRegister(0x90,0x00);

    m_SAA7118->SetRegister(0x93,0x80);

    /* Acquisition Window Settings */
    m_SAA7118->SetRegister(0x94,0x00); // horizontal input offset, min >= 2
    m_SAA7118->SetRegister(0x95,0x00);
    m_SAA7118->SetRegister(0x96,0xd0); // horizontal input window length = 720
    m_SAA7118->SetRegister(0x97,0x02);

    m_SAA7118->SetRegister(0x9c,0xd0); // horizontal output window length = 720
    m_SAA7118->SetRegister(0x9d,0x02);

    /* Scaler Settings */
    m_SAA7118->SetRegister(0xa0,0x1); // hori prescaling
    m_SAA7118->SetRegister(0xa1,0x00);
    m_SAA7118->SetRegister(0xa2,0x00);
    m_SAA7118->SetRegister(0xa4,0x80);//Register A Brightness(Lumin)
    m_SAA7118->SetRegister(0xa5,0x40);//Register A Contrast(Lumin)
    m_SAA7118->SetRegister(0xa6,0x40);//Register A Saturation(Chromin)
    m_SAA7118->SetRegister(0xa8,0x00);
    m_SAA7118->SetRegister(0xa9,0x04);
    m_SAA7118->SetRegister(0xaa,0x20);
    m_SAA7118->SetRegister(0xac,0x00);
    m_SAA7118->SetRegister(0xad,0x02);
    m_SAA7118->SetRegister(0xae,0x00);
    m_SAA7118->SetRegister(0xb0,0x00);
    m_SAA7118->SetRegister(0xb1,0x04);
    m_SAA7118->SetRegister(0xb2,0x00);
    m_SAA7118->SetRegister(0xb3,0x04);
    m_SAA7118->SetRegister(0xb4,0x00);

    m_SAA7118->SetRegister(0xb8,0x00);
    m_SAA7118->SetRegister(0xbc,0x00);

}

void CCX2388xCard::H3DInputSelect(int nInput)
{
    StandardInputSelect(nInput);

    switch(nInput)
    {
    case H3D_COMPONENT:
        m_SAA7118->SetRegister(0x02, 0xef);
        WriteByte(0x390002, 0x8f);
        break;
    case H3D_RGsB:
        m_SAA7118->SetRegister(0x02, 0xff);
        WriteByte(0x390002, 0x8f);
        break;
    case H3D_SVIDEO:
        m_SAA7118->SetRegister(0x02, 0xc8);
        WriteByte(0x390002, 0x8f);
        break;
    case H3D_SDI:
        WriteByte(0x390002, 0x97);
        break;
    case H3D_COMPOSITE1:
        m_SAA7118->SetRegister(0x02, 0xc5);
        WriteByte(0x390002, 0x8f);
        break;
    case H3D_COMPOSITE2:
        m_SAA7118->SetRegister(0x02, 0xdf);
        WriteByte(0x390002, 0x8f);
        break;
    case H3D_COMPOSITE3:
        m_SAA7118->SetRegister(0x02, 0xd5);
        WriteByte(0x390002, 0x8f);
        break;
    case H3D_COMPOSITE4:
        m_SAA7118->SetRegister(0x02, 0xd0);
        WriteByte(0x390002, 0x8f);
        break;
    case H3D_PDI:
        WriteByte(0x390002, 0x87);
        break;
    }
}

void CCX2388xCard::H3DSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive)
{
    BYTE ChrominanceControl(0x81);
    BYTE ChrominanceControl2(0x00);
    BYTE ChrominanceGainControl(0x2A);
    BYTE LuminaceControl;

    // get existing sharpness and set luma comb filter
    LuminaceControl = (m_SAA7118->GetRegister(0x09) & 0x0F) | 0x40;

    if(GetTVFormat(TVFormat)->wCropHeight == 576)
    {
        m_SAA7118->SetRegister(0x5A, 0x03);
        m_SAA7118->SetRegister(0x5B, 0x03);
        WriteByte(0x390007, 0xc8);

        m_SAA7118->SetRegister(0x98,0x0d); // vertical input offset
        m_SAA7118->SetRegister(0x99,0x00);

        m_SAA7118->SetRegister(0x9A, 0x28);
        m_SAA7118->SetRegister(0x9B, 0x01);
        m_SAA7118->SetRegister(0x9E, 0x28);
        m_SAA7118->SetRegister(0x9F, 0x01);
    }
    else
    {
        m_SAA7118->SetRegister(0x5A, 0x06);
        m_SAA7118->SetRegister(0x5B, 0x83);
        WriteByte(0x390007, 0x50);

        m_SAA7118->SetRegister(0x98,0x0a); // vertical input offset
        m_SAA7118->SetRegister(0x99,0x00);

        m_SAA7118->SetRegister(0x9A, 0xf6);
        m_SAA7118->SetRegister(0x9B, 0x00);
        m_SAA7118->SetRegister(0x9E, 0xf6);
        m_SAA7118->SetRegister(0x9F, 0x00);
    }

    m_SAA7118->SetRegister(0x80, 0x10);
    m_SAA7118->SetRegister(0x91, 0x08);
    m_SAA7118->SetRegister(0x92, 0x10);


    switch(nInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
    case H3D_SDI:
    case H3D_PDI:
        // doesn't really matter for these inputs
        // but we'll set the default
        ChrominanceControl = 0x89;
        m_SAA7118->SetRegister(0x29, 0x40);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetRegister(0x29, 0x00);
        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_M:
            ChrominanceControl |= 3 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_PAL_60:
            ChrominanceControl |= 1 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_NTSC_50:
            ChrominanceControl |= 1 << 4;
            ChrominanceControl |= 1 << 3;
            ChrominanceControl2 = 0x0E;
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            ChrominanceControl |= 2 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_SECAM_B:
        case VIDEOFORMAT_SECAM_D:
        case VIDEOFORMAT_SECAM_G:
        case VIDEOFORMAT_SECAM_H:
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
        case VIDEOFORMAT_SECAM_L:
        case VIDEOFORMAT_SECAM_L1:
            ChrominanceControl = 0xD0;
            ChrominanceControl2 = 0x00;
            ChrominanceGainControl = 0x80;
            LuminaceControl = 0x1B;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            ChrominanceControl |= 4 << 4;
            ChrominanceControl |= 1 << 3;
            ChrominanceControl2 = 0x0E;
            break;
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
            ChrominanceControl |= 0 << 4;
            ChrominanceControl2 = 0x06;
            break;
        case VIDEOFORMAT_NTSC_M:
        default:
            ChrominanceControl |= 0 << 4;
            ChrominanceControl |= 1 << 3;
            ChrominanceControl2 = 0x0E;
            break;
        }
        break;
    }

    if(nInput == H3D_SVIDEO)
    {
        LuminaceControl |= 0x80;
    }

    m_SAA7118->SetRegister(0x09, LuminaceControl);
    m_SAA7118->SetRegister(0x0E, ChrominanceControl);
    m_SAA7118->SetRegister(0x0F, ChrominanceGainControl);
    m_SAA7118->SetRegister(0x10, ChrominanceControl2);

    if(IsProgressive)
    {
        WriteByte(0x390005, 0xca);
        WriteByte(0x390008, 0x12);
        WriteByte(0x39000a, 0x7e);
        WriteByte(0x39000e, 0x10);
        WriteByte(0x39000f, 0x02);
        BYTE test = ReadByte(0x39000f);
    }
    else
    {
        WriteByte(0x390005, 0x8a);
        WriteByte(0x390008, 0x13);
        WriteByte(0x39000e, 0x00);
        WriteByte(0x39000f, 0x01);
    }

    // reset the scaler
    m_SAA7118->SetRegister(0x88,0x00);
    m_SAA7118->SetRegister(0x88,0x20);

    FILE* hFile = fopen("FPGA.txt", "w");
    if(!hFile)
    {
        return;
    }

    for(DWORD i(0x390000); i <= 0x39000F; ++i)
    {
        fprintf(hFile, "%06x\t%02x\n", i, ReadByte(i));
    }

    fclose(hFile);

}


void CCX2388xCard::SetH3DContrastBrightness(BYTE Contrast, BYTE Brightness)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        {
            // The SAA7118 has really wierd brighness and contrast
            // this messing about below makes the settings similar in
            // operation to the bt848 which means that the calibration code
            // may work properly

            int NewBrightness = (Brightness - 128) + Contrast;
            if(NewBrightness < 0)
            {
                NewBrightness = 0;
            }
            if(NewBrightness > 255)
            {
                NewBrightness = 255;
            }
            m_SAA7118->SetComponentBrightness((BYTE)NewBrightness);
            SetVIPBrightness(0x80);
            m_SAA7118->SetComponentContrast(Contrast);
            SetVIPContrast(0x80);
        }
       break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        {
            // The SAA7118 has really wierd brighness and contrast
            // this messing about below makes the settings similar in
            // operation to the bt848 which means that the calibration code
            // may work properly

            int NewBrightness = (Brightness - 128) + Contrast;
            if(NewBrightness < 0)
            {
                NewBrightness = 0;
            }
            if(NewBrightness > 255)
            {
                NewBrightness = 255;
            }
            m_SAA7118->SetBrightness((BYTE)NewBrightness);
            SetVIPBrightness(0x80);
            m_SAA7118->SetContrast(Contrast);
            SetVIPContrast(0x80);
        }
        break;
    case H3D_SDI:
    case H3D_PDI:
    default:
        SetVIPBrightness(Brightness);
        SetVIPContrast(Contrast);
        break;
    }
}

void CCX2388xCard::SetH3DHue(BYTE Hue)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetHue(Hue);
        break;
    case H3D_SDI:
    case H3D_PDI:
    default:
        break;
    }
}

void CCX2388xCard::SetH3DSaturationU(BYTE Saturation)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        m_SAA7118->SetComponentSaturation(Saturation);
        SetVIPSaturation(0x80);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetSaturation(Saturation);
        SetVIPSaturation(0x80);
        break;
    case H3D_SDI:
    case H3D_PDI:
    default:
        SetVIPSaturation(Saturation);
        break;
    }
}

void CCX2388xCard::SetH3DSaturationV(BYTE NotUsed)
{
    // we'll do nothing with this but it needs to be here
}

void CCX2388xCard::SetSharpness(char Sharpness)
{
    // had better check that we've been initilaized here as
    // this is a public function
    if(m_SAA7118)
    {
        // get existing sharpness and set luma comb filter
        BYTE LuminaceControl = m_SAA7118->GetRegister(0x09) & 0xF0;
        LuminaceControl |= (BYTE)Sharpness;
        m_SAA7118->SetRegister(0x09, LuminaceControl);
    }
}

void CCX2388xCard::SetFLIFilmDetect(BOOL FLIFilmDetect)
{
    BYTE Buffer[3] = {0xE2, 0x05, 0x0a};
    if(FLIFilmDetect)
    {
        Buffer[2] = 0x0e;
    }
    m_I2CBus->Write(Buffer, 3);
}

#endif // WANT_CX2388X_SUPPORT
