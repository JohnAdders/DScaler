/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xCard_H3D.cpp,v 1.3 2002-09-16 19:34:18 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/09/11 19:33:06  adcockj
// a few tidy ups
//
// Revision 1.1  2002/09/11 18:19:37  adcockj
// Prelimainary support for CT2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CT2388xCard.h"
#include "CT2388x_Defines.h"
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
};

void CCT2388xCard::InitH3D()
{
    m_SAA7118 = new CSAA7118();

    m_SAA7118->Attach(m_I2CBus);

    if(m_SAA7118->GetVersion() > 0)
    {
        m_SAA7118->DumpSettings();
    }

    FILE* hFile;

    hFile = fopen("FPGA.txt", "w");
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

void CCT2388xCard::H3DInputSelect(int nInput)
{
    StandardInputSelect(nInput);

    switch(nInput)
    {
    case H3D_COMPONENT:
        m_SAA7118->SetRegister(0x02, 0xef);
        m_SAA7118->SetRegister(0x03, 0x03);
        m_SAA7118->SetRegister(0x04, 0x00);
        m_SAA7118->SetRegister(0x05, 0x00);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_RGsB:
        m_SAA7118->SetRegister(0x02, 0xff);
        m_SAA7118->SetRegister(0x03, 0x03);
        m_SAA7118->SetRegister(0x04, 0x00);
        m_SAA7118->SetRegister(0x05, 0x00);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_SVIDEO:
        m_SAA7118->SetRegister(0x02, 0xc8);
        m_SAA7118->SetRegister(0x03, 0x00);
        m_SAA7118->SetRegister(0x04, 0x90);
        m_SAA7118->SetRegister(0x05, 0x90);
        m_SAA7118->SetRegister(0x09, 0xc0);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_SDI:
        WriteByte(0x390002, 0x14);
        break;
    case H3D_COMPOSITE1:
        m_SAA7118->SetRegister(0x02, 0xd5);
        m_SAA7118->SetRegister(0x03, 0x00);
        m_SAA7118->SetRegister(0x04, 0x90);
        m_SAA7118->SetRegister(0x05, 0x90);
        m_SAA7118->SetRegister(0x09, 0x40);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_COMPOSITE2:
        m_SAA7118->SetRegister(0x02, 0xd4);
        m_SAA7118->SetRegister(0x03, 0x00);
        m_SAA7118->SetRegister(0x04, 0x90);
        m_SAA7118->SetRegister(0x05, 0x90);
        m_SAA7118->SetRegister(0x09, 0x40);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_COMPOSITE3:
        m_SAA7118->SetRegister(0x02, 0xd3);
        m_SAA7118->SetRegister(0x03, 0x00);
        m_SAA7118->SetRegister(0x04, 0x90);
        m_SAA7118->SetRegister(0x05, 0x90);
        m_SAA7118->SetRegister(0x09, 0x40);
        WriteByte(0x390002, 0x04);
        break;
    case H3D_COMPOSITE4:
        m_SAA7118->SetRegister(0x02, 0xd2);
        m_SAA7118->SetRegister(0x03, 0x00);
        m_SAA7118->SetRegister(0x04, 0x90);
        m_SAA7118->SetRegister(0x05, 0x90);
        m_SAA7118->SetRegister(0x09, 0x40);
        WriteByte(0x390002, 0x04);
        break;
    }
}

void CCT2388xCard::H3DSetFormat(int nInput, eVideoFormat TVFormat, BOOL IsProgressive)
{
    BYTE ChrominanceControl(0);

    m_SAA7118->ReadFromSubAddress(0x0E, &ChrominanceControl, 1);

    ChrominanceControl &= 0x8F;

    if(GetTVFormat(TVFormat)->wCropHeight == 576)
    {
        WriteByte(0x390007, 0xd0);
    }
    else
    {
        WriteByte(0x390007, 0x50);
    }

    switch(nInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
    case H3D_SDI:
        // doesn't really matter for these inputs
        // but we'll set the default
        ChrominanceControl |= 0 << 4;
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_M:
            ChrominanceControl |= 3 << 4;
            break;
        case VIDEOFORMAT_PAL_60:
        case VIDEOFORMAT_NTSC_50:
            ChrominanceControl |= 1 << 4;
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            ChrominanceControl |= 2 << 4;
            break;
        case VIDEOFORMAT_SECAM_B:
        case VIDEOFORMAT_SECAM_D:
        case VIDEOFORMAT_SECAM_G:
        case VIDEOFORMAT_SECAM_H:
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
        case VIDEOFORMAT_SECAM_L:
        case VIDEOFORMAT_SECAM_L1:
            ChrominanceControl |= 5 << 4;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            ChrominanceControl |= 4 << 4;
            break;
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
        case VIDEOFORMAT_NTSC_M:
        default:
            ChrominanceControl |= 0 << 4;
            break;
        }
        break;
    }
    
    m_SAA7118->SetRegister(0x0E, ChrominanceControl);

    if(IsProgressive)
    {
        WriteByte(0x390005, 0x4a);
        WriteByte(0x390008, 0x02);
        WriteByte(0x39000e, 0x10);
        WriteByte(0x39000f, 0x01);
    }
    else
    {
        WriteByte(0x390005, 0x0a);
        WriteByte(0x390008, 0x03);
        WriteByte(0x39000e, 0x00);
        WriteByte(0x39000f, 0x02);
    }
}


void CCT2388xCard::SetH3DBrightness(BYTE Brightness)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        m_SAA7118->SetComponentBrightness(Brightness);
        SetVIPBrightness(0x80);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetBrightness(Brightness);
        SetVIPBrightness(0x80);
        break;
    case H3D_SDI:
    default:
        SetVIPBrightness(Brightness);
        break;
    }
}

void CCT2388xCard::SetH3DHue(BYTE Hue)
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
    default:
        break;
    }
}

void CCT2388xCard::SetH3DContrast(BYTE Contrast)
{
    switch(m_CurrentInput)
    {
    case H3D_COMPONENT:
    case H3D_RGsB:
        m_SAA7118->SetComponentContrast(Contrast);
        SetVIPContrast(0x80);
        break;
    case H3D_SVIDEO:
    case H3D_COMPOSITE1:
    case H3D_COMPOSITE2:
    case H3D_COMPOSITE3:
    case H3D_COMPOSITE4:
        m_SAA7118->SetContrast(Contrast);
        SetVIPContrast(0x80);
        break;
    case H3D_SDI:
    default:
        SetVIPContrast(Contrast);
        break;
    }
}

void CCT2388xCard::SetH3DSaturationU(BYTE Saturation)
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
    default:
        SetVIPSaturation(Saturation);
        break;
    }
}

void CCT2388xCard::SetH3DSaturationV(BYTE NotUsed)
{
    // we'll do nothing with this but it needs to be here
}
