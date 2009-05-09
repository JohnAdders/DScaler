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
 * @file BT848Card_PMS.cpp CBT848Card Implementation (PMS)
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "DebugLog.h"
#include "Providers.h"

#include "status.h"

#include "Source.h"

#include "DS_Filter.h"

// enum sequence aligned to the UI entry sequence!
enum ePMSInputs
{
    PMS_COMPONENT,
    PMS_SVIDEO,
    PMS_SVIDEO2,

    PMS_COMPOSITE1,
    PMS_COMPOSITE2,
    PMS_COMPOSITE3,

    PMS_XPORT,

    PMS_COMPOSITE_SV_1, // Composite over S-Video
    PMS_COMPOSITE_SV_2, // Composite over S-Video
    PMS_COMPOSITE_SV_3, // Composite over S-Video
    PMS_COMPOSITE_SV_4, // Composite over S-Video

    PMS_RGBS,

    PMS_SDI // not yet

};


void CBT848Card::InitPMSDeluxe()
{
    //InitRSBT();

    m_SAA7118 = new CSAA7118();

    m_SAA7118->SetI2CBus(m_I2CBus.GetRawPointer());

    #ifdef _DEBUG
    if(m_SAA7118->GetVersion() > 0)
    {
        m_SAA7118->DumpSettings(_T("SAA7118_PMS.txt"));
    }
    #endif

    int i;

    BYTE dBrightness = 135, dContrast = 62, dSat = 64, dHue = 0, // Other analog
      cBrightness = 176, cContrast = 90, cSat = 64, // Component
      ABrightness = 128, AContrast = 64, ASat = 64, // Post
      verticalChromaPhaseOffset = 0, verticalLumaPhaseOffset = 0,
      horizontalChromaPhaseOffset = 0, horizontalLumaPhaseOffset = 0;

    m_SAA7118->SetRegister(0x01,0x47);
    //0x02 is input selection
    m_SAA7118->SetRegister(0x03,0x20);
    m_SAA7118->SetRegister(0x04,0x90); //not on datasheet
    m_SAA7118->SetRegister(0x05,0x90); //not on datasheet
    m_SAA7118->SetRegister(0x06,0xeb);
    m_SAA7118->SetRegister(0x07,0xe0);
    m_SAA7118->SetRegister(0x08,0xe8);  //enforce even/odd toggle even if non-interlaced detected //0x98);

    /* Decoder Brightness(Lumin), Contrast(Lumin), Saturation(Chromin) & Hue(Chromin) */
    m_SAA7118->SetRegister(0x0a,dBrightness);
    m_SAA7118->SetRegister(0x0b,dContrast);
    m_SAA7118->SetRegister(0x0c,dSat);
    m_SAA7118->SetRegister(0x0d,dHue);

    m_SAA7118->SetRegister(0x11,0x00);
    m_SAA7118->SetRegister(0x12,0x00);
    m_SAA7118->SetRegister(0x13,0x00);
    m_SAA7118->SetRegister(0x14,0x00);
    m_SAA7118->SetRegister(0x15,0x11);
    m_SAA7118->SetRegister(0x16,0xfe);
    m_SAA7118->SetRegister(0x17,0xc0);
    m_SAA7118->SetRegister(0x18,0x40);
    m_SAA7118->SetRegister(0x19,0x80);
    m_SAA7118->SetRegister(0x23,0x00);
    m_SAA7118->SetRegister(0x24,0x90);
    m_SAA7118->SetRegister(0x25,0x90);
    m_SAA7118->SetRegister(0x29,0x00);

    /* Component Brightness(Lumin), Contrast(Lunin) & Satutration(Chromin) */
    m_SAA7118->SetRegister(0x2a,cBrightness);
    m_SAA7118->SetRegister(0x2b,cContrast);
    m_SAA7118->SetRegister(0x2c,cSat);

    m_SAA7118->SetRegister(0x2d,0x00);
    m_SAA7118->SetRegister(0x2e,0x00);
    m_SAA7118->SetRegister(0x2f,0x00);

    //0x30-0x3A = default
    m_SAA7118->SetRegister(0x40,0x22);
    for (i=0x41; i<=0x57; i++)
    {
        m_SAA7118->SetRegister(i,0xff);
    }

    m_SAA7118->SetRegister(0x58,0x00);
    m_SAA7118->SetRegister(0x59,0x00);
    m_SAA7118->SetRegister(0x5a,0x07);
    m_SAA7118->SetRegister(0x5b,0x00);
    m_SAA7118->SetRegister(0x5c,0x00);
    m_SAA7118->SetRegister(0x5d,0x00);
    m_SAA7118->SetRegister(0x5e,0x00);
    m_SAA7118->SetRegister(0x5f,0x00);

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
    //m_SAA7118->SetRegister(0x9e,0xe0);//V=480 //ha e0
    //m_SAA7118->SetRegister(0x9f,0x1); //ha 1


    /* Scaler Settings */
    m_SAA7118->SetRegister(0xa0,0x1); // hori prescaling
    m_SAA7118->SetRegister(0xa1,0x00);
    m_SAA7118->SetRegister(0xa2,0x00);
    m_SAA7118->SetRegister(0xa4,ABrightness);//0x80);//Register A Brightness(Lumin)
    m_SAA7118->SetRegister(0xa5,AContrast);//0x40);//Register A Contrast(Lumin)
    m_SAA7118->SetRegister(0xa6,ASat);//0x40);//Register A Saturation(Chromin)
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

    //m_SAA7118->SetRegister(0xaa,horizontalLumaPhaseOffset);
    //m_SAA7118->SetRegister(0xae,horizontalChromaPhaseOffset);
    m_SAA7118->SetRegister(0xb8,0x00);
    m_SAA7118->SetRegister(0xbc,0x00);

    //m_SAA7118->SetRegister(0x0E, 137); Problem! can't set D7 here, but only in PMSDeluxeInputSelect()
}

void CBT848Card::PMSDeluxeInputSelect(int nInput)
{
    //hcho, enforce to use those nInput with type INPUTTYPE_CCIR
    StandardBT848InputSelect(nInput);

    m_CurrentInput = nInput;

    switch(nInput)
    {
    case PMS_COMPONENT:
        m_SAA7118->SetRegister(0x02, 0xe0);
        break;
    case PMS_SVIDEO:
        m_SAA7118->SetRegister(0x02, 0xcc);
        break;
    case PMS_SVIDEO2:
        m_SAA7118->SetRegister(0x02, 0xcd);
        break;
    case PMS_COMPOSITE1:
        m_SAA7118->SetRegister(0x02, 0xd2);
        break;
    case PMS_COMPOSITE2:
        m_SAA7118->SetRegister(0x02, 0xc2);
        break;
    case PMS_COMPOSITE3:
        m_SAA7118->SetRegister(0x02, 0xd0);
        break;
    case PMS_COMPOSITE_SV_1:
        m_SAA7118->SetRegister(0x02, 0xce);
        break;
    case PMS_COMPOSITE_SV_2:
        m_SAA7118->SetRegister(0x02, 0xc4);
        break;
    case PMS_COMPOSITE_SV_3:
        m_SAA7118->SetRegister(0x02, 0xcf);
        break;
    case PMS_COMPOSITE_SV_4:
        m_SAA7118->SetRegister(0x02, 0xc5);
        break;
    case PMS_RGBS:
        m_SAA7118->SetRegister(0x02, 0xf0);
        break;
    default:
    case PMS_SDI:
        break;
    }
}


void CBT848Card::SetPMSDeluxeFormat(int nInput, eVideoFormat TVFormat)
{
    BYTE ChrominanceControl(0x81);
    BYTE ChrominanceControl2(0x00);
    BYTE ChrominanceGainControl(0x2A);
    BYTE LuminanceControl;

    // get existing sharpness and set luma comb filter
    LuminanceControl = (m_SAA7118->GetRegister(0x09) & 0x0F) | 0x40;

    if(GetTVFormat(TVFormat)->wCropHeight == 576)
    {
        m_SAA7118->SetRegister(0x08, 0x28);
        m_SAA7118->SetRegister(0x5A, 0x03);
        m_SAA7118->SetRegister(0x5B, 0x00);

        m_SAA7118->SetRegister(0x98,0x0d); // vertical input offset
        m_SAA7118->SetRegister(0x99,0x00);

        m_SAA7118->SetRegister(0x9A, 0x38);
        m_SAA7118->SetRegister(0x9B, 0x01);
        m_SAA7118->SetRegister(0x9E, 0x40-2);
        m_SAA7118->SetRegister(0x9F, 0x02);
    }
    else
    {
        m_SAA7118->SetRegister(0x08, 0x68);
        m_SAA7118->SetRegister(0x5A, 0x06);
        m_SAA7118->SetRegister(0x5B, 0x83);

        m_SAA7118->SetRegister(0x98,0x0a); // vertical input offset
        m_SAA7118->SetRegister(0x99,0x00);

        m_SAA7118->SetRegister(0x9A, 0x06);
        m_SAA7118->SetRegister(0x9B, 0x01);
        m_SAA7118->SetRegister(0x9E, 0xE0 - 2);
        m_SAA7118->SetRegister(0x9F, 0x01);
    }

    if(nInput == PMS_XPORT)
    {
        m_SAA7118->SetRegister(0x80, 0x11);
        m_SAA7118->SetRegister(0x91, 0x18);
        m_SAA7118->SetRegister(0x92, 0x19);
    }
    else
    {
        m_SAA7118->SetRegister(0x80, 0x10);
        m_SAA7118->SetRegister(0x91, 0x08);
        m_SAA7118->SetRegister(0x92, 0x10);
    }


    switch(nInput)
    {
    case PMS_COMPONENT:
    case PMS_RGBS:
    case PMS_XPORT:
    case PMS_SDI:
        // doesn't really matter for these inputs
        // but we'll set the default
        ChrominanceControl = 0x89;
        m_SAA7118->SetRegister(0x29, 0x40);
        break;
    case PMS_SVIDEO:
    case PMS_SVIDEO2:
    case PMS_COMPOSITE1:
    case PMS_COMPOSITE2:
    case PMS_COMPOSITE3:
    case PMS_COMPOSITE_SV_1: // Composite over S-Video
    case PMS_COMPOSITE_SV_2: // Composite over S-Video
    case PMS_COMPOSITE_SV_3: // Composite over S-Video
    case PMS_COMPOSITE_SV_4: // Composite over S-Video
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
            LuminanceControl = 0x1B;
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

    if(nInput == PMS_SVIDEO || nInput == PMS_SVIDEO2)
    {
        LuminanceControl |= 0x80;
    }


    m_SAA7118->SetRegister(0x09, LuminanceControl);
    m_SAA7118->SetRegister(0x0E, ChrominanceControl);
    m_SAA7118->SetRegister(0x0F, ChrominanceGainControl);
    m_SAA7118->SetRegister(0x10, ChrominanceControl2);

    // reset the scaler
    m_SAA7118->SetRegister(0x88,0x00);
    m_SAA7118->SetRegister(0x88,0x20);

}


void CBT848Card::SetPMSDeluxeContrastBrightness(WORD Contrast, WORD Brightness)
{
    BYTE ContrastLimited;
    if(Contrast > 255)
    {
        ContrastLimited = 255;
    }
    else
    {
        ContrastLimited = (BYTE)Contrast;
    }

    switch(m_CurrentInput)
    {
    case PMS_COMPONENT:
    case PMS_RGBS:
        {
            // The SAA7118 has really wierd brighness and contrast
            // this messing about below makes the settings similar in
            // operation to the bt848 which means that the calibration code
            // may work properly

            int NewBrightness = Brightness + ContrastLimited;
            if(NewBrightness < 0)
            {
                NewBrightness = 0;
            }
            if(NewBrightness > 255)
            {
                NewBrightness = 255;
            }
            m_SAA7118->SetComponentBrightness((BYTE)NewBrightness);
            m_SAA7118->SetComponentContrast(ContrastLimited);

            SetAnalogContrastBrightness(0x80, 0x00);
        }
       break;
    case PMS_SVIDEO:
    case PMS_SVIDEO2:
    case PMS_COMPOSITE1:
    case PMS_COMPOSITE2:
    case PMS_COMPOSITE3:
    case PMS_COMPOSITE_SV_1: // Composite over S-Video
    case PMS_COMPOSITE_SV_2: // Composite over S-Video
    case PMS_COMPOSITE_SV_3: // Composite over S-Video
    case PMS_COMPOSITE_SV_4: // Composite over S-Video
        {
            // The SAA7118 has really wierd brighness and contrast
            // this messing about below makes the settings similar in
            // operation to the bt848 which means that the calibration code
            // may work properly

            int NewBrightness = Brightness + ContrastLimited;
            if(NewBrightness < 0)
            {
                NewBrightness = 0;
            }
            if(NewBrightness > 255)
            {
                NewBrightness = 255;
            }
            m_SAA7118->SetBrightness((BYTE)NewBrightness);
            m_SAA7118->SetContrast(ContrastLimited);
            SetAnalogContrastBrightness(0x80, 0x00);
        }
        break;
    case PMS_XPORT:
    case PMS_SDI:
    default:
        SetAnalogContrastBrightness(Contrast, Brightness);
        break;
    }
}

void CBT848Card::SetPMSDeluxeHue(BYTE Hue)
{
    switch(m_CurrentInput)
    {
    case PMS_COMPONENT:
    case PMS_RGBS:
        break;
    case PMS_SVIDEO:
    case PMS_SVIDEO2:
    case PMS_COMPOSITE1:
    case PMS_COMPOSITE2:
    case PMS_COMPOSITE3:
    case PMS_COMPOSITE_SV_1: // Composite over S-Video
    case PMS_COMPOSITE_SV_2: // Composite over S-Video
    case PMS_COMPOSITE_SV_3: // Composite over S-Video
    case PMS_COMPOSITE_SV_4: // Composite over S-Video
        m_SAA7118->SetHue(Hue + 0x80);
        break;
    case PMS_XPORT:
    case PMS_SDI:
    default:
        break;
    }
}

void CBT848Card::SetPMSChannelGain(int ChannelNum, WORD Gain)
{
    BYTE TempReg;
    switch(ChannelNum)
    {
    case 1:
        TempReg = m_SAA7118->GetRegister(0x03) & ~1;
        TempReg |= ((Gain & 0x100) >> 8);
        m_SAA7118->SetRegister(0x03, TempReg);
        m_SAA7118->SetRegister(0x04, Gain & 0xFF);
        break;
    case 2:
        TempReg = m_SAA7118->GetRegister(0x03) & ~2;
        TempReg |= ((Gain & 0x100) >> 7);
        m_SAA7118->SetRegister(0x03, TempReg);
        m_SAA7118->SetRegister(0x05, Gain & 0xFF);
        break;
    case 3:
        TempReg = m_SAA7118->GetRegister(0x23) & ~1;
        TempReg |= ((Gain & 0x100) >> 8);
        m_SAA7118->SetRegister(0x23, TempReg);
        m_SAA7118->SetRegister(0x24, Gain & 0xFF);
        break;
    case 4:
        TempReg = m_SAA7118->GetRegister(0x23) & ~2;
        TempReg |= ((Gain & 0x100) >> 7);
        m_SAA7118->SetRegister(0x23, TempReg);
        m_SAA7118->SetRegister(0x25, Gain & 0xFF);
        break;
    default:
        break;
    }
}


void CBT848Card::SetPMSDeluxeSaturationU(WORD Saturation)
{
    BYTE SaturationLimited;
    if(Saturation > 255)
    {
        SaturationLimited = 255;
    }
    else
    {
        SaturationLimited = (BYTE)Saturation;
    }

    switch(m_CurrentInput)
    {
    case PMS_COMPONENT:
    case PMS_RGBS:
        m_SAA7118->SetComponentSaturation(SaturationLimited);
        SetAnalogSaturationU(0x80);
        break;
    case PMS_SVIDEO:
    case PMS_SVIDEO2:
    case PMS_COMPOSITE1:
    case PMS_COMPOSITE2:
    case PMS_COMPOSITE3:
    case PMS_COMPOSITE_SV_1: // Composite over S-Video
    case PMS_COMPOSITE_SV_2: // Composite over S-Video
    case PMS_COMPOSITE_SV_3: // Composite over S-Video
    case PMS_COMPOSITE_SV_4: // Composite over S-Video
        m_SAA7118->SetSaturation(SaturationLimited);
        SetAnalogSaturationU(0x80);
        break;
    case PMS_XPORT:
    case PMS_SDI:
    default:
        SetAnalogSaturationU(Saturation);
        break;
    }
}

void CBT848Card::SetPMSDeluxeSaturationV(WORD Saturation)
{
    switch(m_CurrentInput)
    {
    case PMS_COMPONENT:
    case PMS_RGBS:
        SetAnalogSaturationV(0x80);
        break;
    case PMS_SVIDEO:
    case PMS_SVIDEO2:
    case PMS_COMPOSITE1:
    case PMS_COMPOSITE2:
    case PMS_COMPOSITE3:
    case PMS_COMPOSITE_SV_1: // Composite over S-Video
    case PMS_COMPOSITE_SV_2: // Composite over S-Video
    case PMS_COMPOSITE_SV_3: // Composite over S-Video
    case PMS_COMPOSITE_SV_4: // Composite over S-Video
        SetAnalogSaturationV(0x80);
        break;
    case PMS_XPORT:
    case PMS_SDI:
    default:
        SetAnalogSaturationV(Saturation);
        break;
    }
}

#endif // WANT_BT8X8_SUPPORT