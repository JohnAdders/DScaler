/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard_Types.cpp,v 1.19 2004-02-21 21:47:06 to_see Exp $
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
// Revision 1.18  2004/01/27 22:48:57  robmuller
// Use correct GPIO settings for TV@nywhere Master.
//
// Revision 1.17  2004/01/19 14:31:20  adcockj
// Test code for Hauppauge cards inspired by Torsten's diagram
//
// Revision 1.16  2004/01/07 10:08:12  adcockj
// Added setting GPIO pins for sound support and added ATI card
//
// Revision 1.15  2004/01/05 13:12:24  adcockj
// Added patch from Lavrenov Dmitrij (midimaker)
//
// Revision 1.14  2003/12/17 08:28:57  adcockj
// Added Leadtek Winfast Expert (Thanks to Tom Zoerner)
//
// Revision 1.13  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.12  2003/07/18 09:41:23  adcockj
// Added PDI input to holo3d (doesn't yet work)
//
// Revision 1.11  2003/06/17 12:45:19  adcockj
// Added new card for use with PDI
//
// Revision 1.10  2003/03/10 17:43:19  adcockj
// Added support for Hauppauge WinTV PCI-FM : patch from Tom Zoerner
//
// Revision 1.9  2003/02/24 11:19:51  adcockj
// Futher fix for issues with PlayHD
//
// Revision 1.8  2003/02/03 17:25:23  adcockj
// Fixes for PlayHD
//
// Revision 1.7  2002/12/05 17:11:11  adcockj
// Sound fixes
//
// Revision 1.6  2002/12/04 17:43:49  adcockj
// Contrast and Brightness adjustments so that h3d card behaves in expected way
//
// Revision 1.5  2002/11/15 17:10:51  adcockj
// Setting of GPIO pins for MSI TV@nywhere
//
// Revision 1.4  2002/11/03 18:38:32  adcockj
// Fixes for cx2388x and PAL & NTSC
//
// Revision 1.3  2002/11/03 15:54:10  adcockj
// Added cx2388x register tweaker support
//
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

/**
 * @file CX2388xCard.cpp CCX2388xCard Implementation (Types)
 */

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
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Colour Bars",
                INPUTTYPE_COLOURBARS,
                0,
                0x00000000,
            },
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
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
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Colour Bars",
                INPUTTYPE_COLOURBARS,
                0,
                0x00000000,
            },
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
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
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Colour Bars",
                INPUTTYPE_COLOURBARS,
                0,
                0x00000000,
            },
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_PHILIPS_PAL,
        IDC_CX2388X,
    },
    {
        "Holo 3d Graph",
        9,
        {
            {
                "Component",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "RGsB",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "SDI",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "Composite G",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "Composite B",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "Composite R",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "Composite BNC",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
            {
                "PDI",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
        },
        InitH3D,
        NULL,
        H3DInputSelect,
        SetH3DContrastBrightness,
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
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
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
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
                0x00000000,
            },
        },
        NULL,
        NULL,
        MSIInputSelect,
        SetAnalogContrastBrightness,
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
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
                0x00000000,
            },
        },
        NULL,
        NULL,
        MSIPalInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_MT2032_PAL,
        IDC_CX2388X,
    },
    {
        "Asus TV Tuner 880 NTSC",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
                0x00000000,
            },
        },
        NULL,
        NULL,
        AsusInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_USER_SETUP,
        IDC_CX2388X,
    },
    {
        "Prolink PlayTV HD",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
                0x00000000,
            },
        },
        NULL,
        PlayHDStopCapture,
        PlayHDInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_USER_SETUP,
        IDC_CX2388X,
    },
    // Card info from Tom Zoerner
    {
        "Hauppauge WinTV 34xxx models",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
    			0x0000ff00,

            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
    			0x0000ff02,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
    			0x0000ff02,
            },
            {   // card has no composite in, but comes with a Cinch to S-Video adapter
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
    			0x0000ff02,
            },
            // FM radio input omitted
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        // \todo add eeprom read functionality
        // these cards seem similar to the bt848 except that
        // the contents are shifted by 8 bytes
		// ...fixed 21.02.2004 to_see
        TUNER_AUTODETECT,
        IDC_CX2388X,
    },
    {
        "PixelView XCapture With PDI Mod",
        3,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "PDI",
                INPUTTYPE_CCIR,
                3,
                0x00000000,
            },
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_ABSENT,
        IDC_CX2388X,
    },
    // CX2388xCARD_LEADTEK_WINFAST_EXPERT
    // video and audio input switching is not yet verified.
    {
        "Leadtek WinFast TV2000 XP Expert",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_PHILIPS_FM1216ME_MK3,
        IDC_CX2388X,
    },
    {
        "MSI TV@nywhere Master",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
            {
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
                0x00000000,
            },
        },
        NULL,
        NULL,
        MSIPalInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_MT2050_PAL,
        IDC_CX2388X,
    },
    {
        "ATI TV Wonder Pro",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
                0x00000000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
                0x00000000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
                0x00000000,
            },
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        TUNER_USER_SETUP,
        IDC_CX2388X,
    },
    // Card info from Tom Zoerner
    {
        "Hauppauge WinTV 34xxx models (Mono Tuner Sound)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                0,
    			0x0000ff01,

            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                1,
    			0x0000ff02,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
    			0x0000ff02,
            },
            {   // card has no composite in, but comes with a Cinch to S-Video adapter
                "Composite Over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
    			0x0000ff02,
            },
            // FM radio input omitted
        },
        NULL,
        NULL,
        StandardInputSelect,
        SetAnalogContrastBrightness,
        SetAnalogHue,
        SetAnalogSaturationU,
        SetAnalogSaturationV,
        StandardSetFormat,
        // \todo add eeprom read functionality
        // these cards seem similar to the bt848 except that
        // the contents are shifted by 8 bytes
		// ...fixed 21.02.2004 to_see
        TUNER_AUTODETECT,
        IDC_CX2388X,
    },
};

const CCX2388xCard::TAutoDectect CCX2388xCard::m_AutoDectect[] =
{
    { 0x006614F1, CX2388xCARD_CONEXANT_EVK, "Conexant CX23880 TV/FM EVK" },
    { 0x00f81002, CX2388xCARD_ATI_WONDER_PRO, "ATI Wonder Pro"},
    //Tee Added support for PAL EVK and also added support for SSVID
    { 0x016614F1, CX2388xCARD_CONEXANT_EVK_PAL, "Conexant CX23880 PAL TV/FM EVK" },
    { 0x48201043, CX2388xCARD_ASUS, "Asus 880" },
    { 0x34010070, CX2388xCARD_HAUPPAUGE_PCI_FM, "Hauppauge" },
    { 0x34000070, CX2388xCARD_HAUPPAUGE_PCI_FM, "Hauppauge" },
    { 0x6611107D, CX2388xCARD_LEADTEK_WINFAST_EXPERT, "Leadtek WinFast TV2000 XP Expert" },
    { 0x86061462, CX2388xCARD_MSI_TV_ANYWHERE_MASTER_PAL, "MSI TV@nywhere Master"},
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

                break;
            
            case INPUTTYPE_CCIR:
                VideoInput |= CX2388X_VIDEO_INPUT_PE_SRCSEL;
                VideoInput |= CX2388X_VIDEO_INPUT_SVID_C_SEL; 
                break;
        
            case INPUTTYPE_TUNER:
            case INPUTTYPE_COMPOSITE:
            default:
                // Switch chroma DAC to audio
                AndDataDword(MO_AFECFG_IO, 0xFFFFFFFE);
                break;
        }
        
        WriteDword(CX2388X_VIDEO_INPUT, VideoInput);

    }

    // set up any sound stuff
    WriteDword(MO_GP0_IO, m_TVCards[m_CardType].Inputs[nInput].GPIOFlags);
}

HMENU CCX2388xCard::GetCardSpecificMenu()
{
    return LoadMenu(hResourceInst, MAKEINTRESOURCE(m_TVCards[m_CardType].MenuId));
}

void CCX2388xCard::MSIInputSelect(int nInput)
{
    StandardInputSelect(nInput);
	if(nInput == 0)
	{
		// GPIO pins set according to values supplied by
		// Ryan Griffin - Stegink
		WriteDword(MO_GP0_IO, 0x000000ff);
		WriteDword(MO_GP1_IO, 0x00008040);
		WriteDword(MO_GP2_IO, 0x0000fc1f); 
		WriteDword(MO_GP3_IO, 0x00000000); 
	}
	else
	{
		// Turn off anything audio if we're not the tuner
		WriteDword(MO_GP0_IO, 0x00000000);
		WriteDword(MO_GP1_IO, 0x00008000);
		WriteDword(MO_GP2_IO, 0x0000ff80); 
		WriteDword(MO_GP3_IO, 0x00000000); 
	}
}

void CCX2388xCard::MSIPalInputSelect(int nInput)
{
    StandardInputSelect(nInput);
	if(nInput == 0)
	{
		WriteDword(MO_GP0_IO, 0x000040bf);
		WriteDword(MO_GP1_IO, 0x000080c0);
		WriteDword(MO_GP2_IO, 0x0000ff40); 
		WriteDword(MO_GP3_IO, 0x00000000); 
	}
	else
	{
		// Turn off anything audio if we're not the tuner
		WriteDword(MO_GP0_IO, 0x000040bf);
		WriteDword(MO_GP1_IO, 0x000080c0);
		WriteDword(MO_GP2_IO, 0x0000ff20); 
		WriteDword(MO_GP3_IO, 0x00000000); 
	}
}

void CCX2388xCard::PlayHDInputSelect(int nInput)
{
    StandardInputSelect(nInput);
    if(nInput == 0)
    {
        // GPIO pins set according to values supplied by
        // Tom Fotja
        WriteDword(MO_GP0_IO, 0x0000ff00);
    }
    else
    {
        // Turn off anything audio if we're not the tuner
        WriteDword(MO_GP0_IO, 0x00000ff1);
    }
}

void CCX2388xCard::PlayHDStopCapture()
{
    // Turn off audio
    // JA 24/Feb/2003 Tuned off as it doesn't work
    //WriteDword(MO_GP0_IO, 0x00000ff1);
}


void CCX2388xCard::AsusInputSelect(int nInput)
{
    StandardInputSelect(nInput);
    if(nInput == 0)
    {
        // GPIO pins set according to values supplied by
        // Phil Rasmussen 
        WriteDword(MO_GP0_IO, 0x000080ff);
        WriteDword(MO_GP1_IO, 0x000001ff);
        WriteDword(MO_GP2_IO, 0x000000ff); 
        WriteDword(MO_GP3_IO, 0x00000000); 
    }
    else
    {
        // Turn off anything audio if we're not the tuner
        WriteDword(MO_GP0_IO, 0x0000ff00);
        WriteDword(MO_GP1_IO, 0x0000ff00);
        WriteDword(MO_GP2_IO, 0x0000ff00); 
        WriteDword(MO_GP3_IO, 0x00000000); 
    }
}

const eTunerId CCX2388xCard::m_Tuners_Hauppauge_CX2388x_Card[]=
{
	TUNER_ABSENT,
	TUNER_ABSENT,						//"External"
	TUNER_ABSENT,						//"Unspecified"
	TUNER_PHILIPS_PAL,					//"Philips FI1216"
	//4
	TUNER_PHILIPS_SECAM,				//"Philips FI1216MF"
	TUNER_PHILIPS_NTSC,					//"Philips FI1236"
	TUNER_PHILIPS_PAL_I,				//"Philips FI1246"
	TUNER_PHILIPS_PAL_DK,				//"Philips FI1256"
	//8
	TUNER_PHILIPS_PAL,					//"Philips FI1216 MK2"
	TUNER_PHILIPS_SECAM,				//"Philips FI1216MF MK2"
	TUNER_PHILIPS_NTSC,					//"Philips FI1236 MK2"
	TUNER_PHILIPS_PAL_I,				//"Philips FI1246 MK2"
	//12
	TUNER_PHILIPS_PAL_DK,				//"Philips FI1256 MK2"
	TUNER_TEMIC_4032FY5_NTSC,			//"Temic 4032FY5"
	TUNER_TEMIC_4002FH5_PAL,			//"Temic 4002FH5"
	TUNER_TEMIC_4062FY5_PAL_I,			//"Temic 4062FY5"
	//16
	TUNER_PHILIPS_PAL,					//"Philips FR1216 MK2"
	TUNER_PHILIPS_SECAM,				//"Philips FR1216MF MK2"
	TUNER_PHILIPS_NTSC,					//"Philips FR1236 MK2"
	TUNER_PHILIPS_PAL_I,				//"Philips FR1246 MK2"
	//20
	TUNER_PHILIPS_PAL_DK,				//"Philips FR1256 MK2"
	TUNER_PHILIPS_PAL,					//"Philips FM1216"
	TUNER_PHILIPS_SECAM,				//"Philips FM1216MF"
	TUNER_PHILIPS_NTSC,					//"Philips FM1236"
	//24
	TUNER_PHILIPS_PAL_I,				//"Philips FM1246"
	TUNER_PHILIPS_PAL_DK,				//"Philips FM1256"
	TUNER_TEMIC_4036FY5_NTSC,			//"Temic 4036FY5"
	TUNER_ABSENT,						//"Samsung TCPN9082D"
	//28
	TUNER_ABSENT,						//"Samsung TCPM9092P"
	TUNER_TEMIC_4006FH5_PAL,			//"Temic 4006FH5"
	TUNER_ABSENT,						//"Samsung TCPN9085D"
	TUNER_ABSENT,						//"Samsung TCPB9085P"
	//32
	TUNER_ABSENT,						//"Samsung TCPL9091P"
	TUNER_TEMIC_4039FR5_NTSC,			//"Temic 4039FR5"
	TUNER_PHILIPS_MULTI,				//"Philips FQ1216 ME"
	TUNER_TEMIC_4066FY5_PAL_I,			//"Temic 4066FY5"
	//36
	TUNER_PHILIPS_NTSC,					//"Philips TD1536"
	TUNER_PHILIPS_NTSC,					//"Philips TD1536D"
	TUNER_PHILIPS_NTSC,					//"Philips FMR1236"
	TUNER_ABSENT,						//"Philips FI1256MP"
	//40
	TUNER_ABSENT,						//"Samsung TCPQ9091P"
	TUNER_TEMIC_4006FN5_PAL,			//"Temic 4006FN5"
	TUNER_TEMIC_4009FR5_PAL,			//"Temic 4009FR5"
	TUNER_TEMIC_4046FM5_MULTI,			//"Temic 4046FM5"
	//44
	TUNER_TEMIC_4009FN5_PAL,			//"Temic 4009FN5"
	TUNER_ABSENT,						//"Philips TD1536D_FH_44"
	TUNER_LG_R01F_NTSC,					//"LG TPI8NSR01F"}
	TUNER_LG_B01D_PAL,					//"LG TPI8PSB01D"}
	//48
	TUNER_LG_B11D_PAL,					//"LG TPI8PSB11D"}
	TUNER_LG_I001D_PAL_I,				//"LG TAPC-I001D"}
	TUNER_LG_I701D_PAL_I,				//"LG TAPC-I701D"}
};


