/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xCard_Types.cpp,v 1.5 2002-10-21 07:19:33 adcockj Exp $
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
// Prelimainary support for CT2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CT2388xCard.h"
#include "CT2388x_Defines.h"
#include "DScaler.h"
#include "DebugLog.h"

const CCT2388xCard::TCardType CCT2388xCard::m_TVCards[CT2388xCARD_LASTONE] = 
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
        IDC_CT2388X,
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
        IDC_CT2388X,
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
        IDC_CT2388X,
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
        IDC_CT2388X_H3D,
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
        IDC_CT2388X,
 	},
};

const CCT2388xCard::TAutoDectect CCT2388xCard::m_AutoDectect[] =
{
	{ 0x006614F1, CT2388xCARD_CONEXANT_EVK, "Conexant CX23880 TV/FM EVK" },
	//Tee Added support for PAL EVK and also added support for SSVID
	{ 0x016614F1, CT2388xCARD_CONEXANT_EVK_PAL, "Conexant CX23880 PAL TV/FM EVK" },
    { 0, (eCT2388xCardId)-1, NULL }
};

int CCT2388xCard::GetNumInputs()
{
    return m_TVCards[m_CardType].NumInputs;
}

BOOL CCT2388xCard::IsInputATuner(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_TUNER);
}

void CCT2388xCard::SetVideoSource(int nInput)
{
    // call correct function
    // this funny syntax is the only one that works
    // if you want help understanding what is going on
    // I suggest you read http://www.newty.de/
    (*this.*m_TVCards[m_CardType].pInputSwitchFunction)(nInput);
}

const CCT2388xCard::TCardType* CCT2388xCard::GetCardSetup()
{
    return &(m_TVCards[m_CardType]);
}

eCT2388xCardId CCT2388xCard::AutoDetectCardType()
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
    return CT2388xCARD_UNKNOWN;
}

void CCT2388xCard::StandardInputSelect(int nInput)
{
    m_CurrentInput = nInput;
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

    if(m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_COLOURBARS)
    {
        // Enable color bars
        OrDataDword(CT2388X_VIDEO_COLOR_FORMAT, 0x00004000);
    }
    else
    {
        // disable color bars
        AndDataDword(CT2388X_VIDEO_COLOR_FORMAT, 0xFFFFBFFF);
    
        // Read and mask the video input register
        DWORD dwVal = ReadDword(CT2388X_VIDEO_INPUT);
        
        // zero out mux and svideo bit
        // and force auto detect
		// also turn off CCIR input
		// also VERTEN & SPSPD
        dwVal &= 0xFFFC3F00;

		// set the Mux up from the card setup
        dwVal |= (m_TVCards[m_CardType].Inputs[nInput].MuxSelect << CT2388X_VIDEO_INPUT_MUX_SHIFT);

        // set the comp bit for svideo
        switch (m_TVCards[m_CardType].Inputs[nInput].InputType)
        {
            case INPUTTYPE_SVIDEO: // SVideo
                dwVal |= CT2388X_VIDEO_INPUT_SVID_C_SEL; 
                dwVal |= CT2388X_VIDEO_INPUT_SVID;
                WriteDword(CT2388X_VIDEO_INPUT, dwVal);

                // Switch chroma DAC to chroma channel
                OrDataDword(MO_AFECFG_IO, 0x00000001);
                break;
            case INPUTTYPE_CCIR:
				dwVal |= CT2388X_VIDEO_INPUT_PE_SRCSEL;
                dwVal |= CT2388X_VIDEO_INPUT_SVID_C_SEL; 
                WriteDword(CT2388X_VIDEO_INPUT, dwVal);
                break;
		
            case INPUTTYPE_TUNER:
            case INPUTTYPE_COMPOSITE:
            default:
                WriteDword(CT2388X_VIDEO_INPUT,dwVal);
                AndDataDword(MO_AFECFG_IO, 0xFFFFFFFE);
                break;
        }
    }
}

HMENU CCT2388xCard::GetCardSpecificMenu()
{
    return LoadMenu(hResourceInst, MAKEINTRESOURCE(m_TVCards[m_CardType].MenuId));
}
