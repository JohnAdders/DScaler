/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Types.cpp,v 1.3 2002-11-03 15:54:10 adcockj Exp $
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
// Revision 1.2  2002/10/29 16:20:30  adcockj
// Added card setup for MSI TV@nywhere (no work done on sound)
//
// Revision 1.1  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// 
// CVS Log while file was called CT2388xCard_Types.cpp
//
// Revision 1.11  2002/10/27 19:17:25  adcockj
// Fixes for cx2388x - PAL & NTSC tested
//
// Revision 1.10  2002/10/25 14:44:26  laurentg
// filter setup updated to have something working even for SECAM
//
// Revision 1.9  2002/10/24 16:03:00  adcockj
// Minor tidy up
//
// Revision 1.8  2002/10/23 20:26:53  adcockj
// Bug fixes for cx2388x
//
// Revision 1.7  2002/10/23 16:10:50  adcockj
// Fixed some filter setting bugs and added SECAM tests for Laurent
//
// Revision 1.6  2002/10/21 16:07:26  adcockj
// Added H & V delay options for CX2388x cards
//
// Revision 1.5  2002/10/21 07:19:33  adcockj
// Preliminary Support for PixelView XCapture
//
// Revision 1.4  2002/10/17 13:31:37  adcockj
// Give Holo3d different menu and updated settings
//
// Revision 1.3  2002/09/22 17:47:04  adcockj
// Fixes for holo3d
//
// Revision 1.2  2002/09/11 19:33:06  adcockj
// a few tidy ups
//
// Revision 1.1  2002/09/11 18:19:37  adcockj
// Prelimainary support for CX2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "DebugLog.h"

const CCX2388xCard::TCardType CCX2388xCard::m_TVCards[CX2388xCARD_LASTONE] = 
{
    // Card Number 0 - Unknown
    {
        "Unknown",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
            {
                "Colour Bars",
                INPUTTYPE_COLOURBARS,
                0,
            },
        },
        NULL,
        StandardInputSelect,
        SetAnalogBrightness,
        SetAnalogContrast,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_PHILIPS_NTSC,
        IDC_CX2388X,
    },
    {
        "Conexant CX23880 TV/FM EVK",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
            {
                "Colour Bars",
                INPUTTYPE_COLOURBARS,
                0,
            },
        },
        NULL,
        StandardInputSelect,
        SetAnalogBrightness,
        SetAnalogContrast,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_PHILIPS_NTSC,
        IDC_CX2388X,
    },
    {
        "Conexant CX23880 TV/FM EVK (PAL)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
            {
                "Colour Bars",
                INPUTTYPE_COLOURBARS,
                0,
            },
        },
        NULL,
        StandardInputSelect,
        SetAnalogBrightness,
        SetAnalogContrast,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_PHILIPS_PAL,
        IDC_CX2388X,
    },
    {
        "Holo 3d Graph",
        8,
        {
            {
                "Component",
                INPUTTYPE_CCIR,
                3,
            },
            {
                "RGsB",
                INPUTTYPE_CCIR,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_CCIR,
                3,
            },
            {
                "SDI",
                INPUTTYPE_CCIR,
                3,
            },
            {
                "Composite G",
                INPUTTYPE_CCIR,
                3,
            },
            {
                "Composite B",
                INPUTTYPE_CCIR,
                3,
            },
            {
                "Composite R",
                INPUTTYPE_CCIR,
                3,
            },
            {
                "Composite BNC",
                INPUTTYPE_CCIR,
                3,
            },
        },
        InitH3D,
        H3DInputSelect,
        SetH3DBrightness,
        SetH3DContrast,
        SetH3DHue,
        SetH3DSaturationU,
        SetH3DSaturationV,
        H3DSetFormat,
        TUNER_ABSENT,
        IDC_CX2388X_H3D,
    },
    {
        "PixelView XCapture",
        2,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
        },
        NULL,
        StandardInputSelect,
        SetAnalogBrightness,
        SetAnalogContrast,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_ABSENT,
        IDC_CX2388X,
    },
    {
        "MSI TV@nywhere (NTSC)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
            {
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
            },
        },
        NULL,
        StandardInputSelect,
        SetAnalogBrightness,
        SetAnalogContrast,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_MT2032,
        IDC_CX2388X,
    },
    {
        "MSI TV@nywhere (PAL)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
            {
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
            },
        },
        NULL,
        StandardInputSelect,
        SetAnalogBrightness,
        SetAnalogContrast,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_MT2032_PAL,
        IDC_CX2388X,
    },
};

const CCX2388xCard::TAutoDectect CCX2388xCard::m_AutoDectect[] =
{
    { 0x006614F1, CX2388xCARD_CONEXANT_EVK, "Conexant CX23880 TV/FM EVK" },
    //Tee Added support for PAL EVK and also added support for SSVID
    { 0x016614F1, CX2388xCARD_CONEXANT_EVK_PAL, "Conexant CX23880 PAL TV/FM EVK" },
    { 0, (eCX2388xCardId)-1, NULL }
};

int CCX2388xCard::GetNumInputs()
{
    return m_TVCards[m_CardType].NumInputs;
}

BOOL CCX2388xCard::IsInputATuner(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_TUNER);
    }
    else
    {
        return FALSE;
    }
}

void CCX2388xCard::SetVideoSource(int nInput)
{
    // call correct function
    // this funny syntax is the only one that works
    // if you want help understanding what is going on
    // I suggest you read http://www.newty.de/
    (*this.*m_TVCards[m_CardType].pInputSwitchFunction)(nInput);
}

const CCX2388xCard::TCardType* CCX2388xCard::GetCardSetup()
{
    return &(m_TVCards[m_CardType]);
}

eCX2388xCardId CCX2388xCard::AutoDetectCardType()
{
    DWORD Id = m_SubSystemId;
    if (Id != 0 && Id != 0xffffffff)
    {
        int i;
        for (i = 0; m_AutoDectect[i].ID != 0; i++)
        {
            if (m_AutoDectect[i].ID  == Id)
            {
                return m_AutoDectect[i].CardId;
            }
        }
    }
    return CX2388xCARD_UNKNOWN;
}

void CCX2388xCard::StandardInputSelect(int nInput)
{
    if(nInput >= m_TVCards[m_CardType].NumInputs)
    {
        LOG(1, "Input Select Called for invalid input");
        nInput = m_TVCards[m_CardType].NumInputs - 1;
    }
    if(nInput < 0)
    {
        LOG(1, "Input Select Called for invalid input");
        nInput = 0;
    }
    m_CurrentInput = nInput;

    if(m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_COLOURBARS)
    {
        // Enable color bars
        OrDataDword(CX2388X_VIDEO_COLOR_FORMAT, 0x00004000);
    }
    else
    {
        // disable color bars
        AndDataDword(CX2388X_VIDEO_COLOR_FORMAT, 0xFFFFBFFF);
    
        // Read and mask the video input register
        DWORD VideoInput = ReadDword(CX2388X_VIDEO_INPUT);
        // zero out mux and svideo bit
        // and force auto detect
        // also turn off CCIR input
        // also VERTEN & SPSPD
        VideoInput &= 0x0F;
        
        // start with default values except turn of CFILT
        DWORD FilterSetup(1 << 19);

        // set the Mux up from the card setup
        VideoInput |= (m_TVCards[m_CardType].Inputs[nInput].MuxSelect << CX2388X_VIDEO_INPUT_MUX_SHIFT);

        // set the comp bit for svideo
        switch (m_TVCards[m_CardType].Inputs[nInput].InputType)
        {
            case INPUTTYPE_SVIDEO: // SVideo
                VideoInput |= CX2388X_VIDEO_INPUT_SVID_C_SEL; 
                VideoInput |= CX2388X_VIDEO_INPUT_SVID;

                // Switch chroma DAC to chroma channel
                OrDataDword(MO_AFECFG_IO, 0x00000001);

                // switch off luma notch
                // Luma notch is 1 = off
                FilterSetup |= CX2388X_FILTER_LNOTCH;
                // turn off Comb Filter
                FilterSetup |= 3 << 5;
                // Disbale luma dec
                FilterSetup |= 1 << 12;
                break;
            
            case INPUTTYPE_CCIR:
                VideoInput |= CX2388X_VIDEO_INPUT_PE_SRCSEL;
                VideoInput |= CX2388X_VIDEO_INPUT_SVID_C_SEL; 
                break;
        
            case INPUTTYPE_TUNER:
            case INPUTTYPE_COMPOSITE:
            default:

                // test for Laurent
                // Try out SECAM Notch Filter
				// Comments from Laurent
				// It seems that these SECAM Notch Filters are not necessary
                if(false)
                {
                    // May have to switch off normal luma notch
                    // to see any effect
                    //FilterSetup |= CX2388X_FILTER_LNOTCH;
                    
                    // SECAM Luma notch is 1 = on
                    //FilterSetup |= CX2388X_FILTER_SNOTCH;
                }

                // Switch chroma DAC to audio
                AndDataDword(MO_AFECFG_IO, 0xFFFFFFFE);
                break;
        }
        
        WriteDword(CX2388X_VIDEO_INPUT, VideoInput);

        // test for Laurent
        // other stuff that may be required
		// Comments from Laurent
		// Bits 12, 16, and 18 must be set to 1 for SECAM
		// It seems to work even for PAL with these bits
		// TODO : check that they must be set for all the video formats
        if(true)
        {
            // QCIF HFilter
            FilterSetup |= (1<<11);

            // 29 Tap first chroma demod
            FilterSetup |= (1<<15);

			// Laurent : very important for Secam
            FilterSetup |= (1<<17);

//            FilterSetup |= (1<<3);
//            FilterSetup |= (1<<16);
        }

        //FilterSetup = 0x8268;
        WriteDword(CX2388X_FILTER_EVEN, FilterSetup);
        WriteDword(CX2388X_FILTER_ODD, FilterSetup);
    }
}

HMENU CCX2388xCard::GetCardSpecificMenu()
{
    return LoadMenu(hResourceInst, MAKEINTRESOURCE(m_TVCards[m_CardType].MenuId));
}
