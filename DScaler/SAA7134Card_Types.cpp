/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Types.cpp,v 1.7 2002-10-16 11:40:09 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on v4l2 device driver for philips
// saa7134 based TV cards.  Those portions are
// Copyright (c) 2001,02 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
//
// This software was based on BT848Card_Types.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2002/10/06 11:09:48  atnak
// SoundChannel function from TCardType
//
// Revision 1.5  2002/10/04 13:24:46  atnak
// Audio mux select through GPIO added (for 7130 cards)
//
// Revision 1.4  2002/10/03 23:36:23  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.3  2002/09/14 19:40:48  atnak
// various changes
//
//
//
//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "DebugLog.h"


const CSAA7134Card::TCardType CSAA7134Card::m_TVCards[] = 
{
    // Card Number 0 - Unknown Card
    {
        "*Unknown Card*",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_USER_SETUP,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 1 - Proteus Pro [philips reference design]
    {
        "Proteus Pro [philips reference design]",
        2,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 2 - LifeView FlyVIDEO3000
    {
        "LifeView FlyVIDEO3000",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 3 - LifeView FlyVIDEO2000 (saa7130)
    {
        "LifeView FlyVIDEO2000",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_LG_TAPCNEW_PAL,
        AUDIOCRYSTAL_NONE,
        NULL,
        FLYVIDEO2000CardInputSelect,
    },
    // Card Number 4 - EMPRESS (has TS, i2srate=48000, has CCIR656 video out)
    {
        "EMPRESS",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 5 - SKNet Monster TV
    {
        "SKNet Monster TV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_MK2_NTSC,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 6 - Tevion MD 9717
    {
        "Tevion MD 9717",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 7 - KNC One TV-Station RDS  (needs tda9887)
    {
        "KNC One TV-Station RDS",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_USER_SETUP, //TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 8 - Terratec Cinergy 400 TV
    {
        "Terratec Cinergy 400 TV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                4,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Card Number 9 - Medion 5044
    {
        "Medion 5044",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                8,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_USER_SETUP, //TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        MEDION5044CardInputSelect,
    },
    // Card Number 10 - KWORLD KW-TV713XRF (saa7130)
    // Thanks to "b" <b@ki...>
    {
        "KWORLD KW-TV713XRF / KUROUTO SHIKOU",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_NTSC,
        AUDIOCRYSTAL_NONE,
        NULL,
        KWTV713XRFCardInputSelect,
    },
};


int CSAA7134Card::GetMaxCards()
{
    return sizeof(m_TVCards)/sizeof(TCardType);
}


/*
TUNER_PHILIPS_FM1216ME_MK3
    { "Philips PAL/SECAM multi (FM1216ME MK3)", Philips, PAL,
      16*160.00,16*442.00,0x01,0x02,0x04,0x8e,623 },
*/



eTunerId CSAA7134Card::AutoDetectTuner(eTVCardId CardId)
{
    return m_TVCards[CardId].TunerId;
}


int CSAA7134Card::GetNumInputs()
{
    return m_TVCards[m_CardType].NumInputs;
}


LPCSTR CSAA7134Card::GetInputName(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_TVCards[m_CardType].Inputs[nInput].szName;
    }
    return "Error";
}


BOOL CSAA7134Card::IsInputATuner(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_TUNER);
}


BOOL CSAA7134Card::IsCCIRSource(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
}


LPCSTR CSAA7134Card::GetCardName(eTVCardId CardId)
{
    return m_TVCards[CardId].szName;
}


void CSAA7134Card::SetVideoSource(int nInput)
{
    // call correct function
    // this funny syntax is the only one that works
    // if you want help understanding what is going on
    // I suggest you read http://www.newty.de/
    (*this.*m_TVCards[m_CardType].pInputSwitchFunction)(nInput);
}


const CSAA7134Card::TCardType* CSAA7134Card::GetCardSetup()
{
    return &(m_TVCards[m_CardType]);
}


void CSAA7134Card::FLYVIDEO2000CardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);
    switch(nInput)
    {
    case 0:
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x6000, 0x6000);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x0000, 0x6000);
        break;
    case 1:
    case 2:
    case 3:
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x6000, 0x6000);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x4000, 0x6000);
        break;
    default:
        break;
    }
}


void CSAA7134Card::MEDION5044CardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);
    switch(nInput)
    {
    case 0:
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x6000, 0x6000);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x4000, 0x6000);
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x6000, 0x6000);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x0000, 0x6000);
        break;
    default:
        break;
    }
}


void CSAA7134Card::KWTV713XRFCardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);

    // this card probably needs GPIO changes but I don't
    // know what to do
}


void CSAA7134Card::StandardSAA7134InputSelect(int nInput)
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

    switch (m_TVCards[m_CardType].Inputs[nInput].InputType)
    {
    case INPUTTYPE_SVIDEO:
        OrDataByte(SAA7134_LUMA_CTRL, SAA7134_LUMA_CTRL_BYPS);
        break;
    case INPUTTYPE_TUNER:
    case INPUTTYPE_COMPOSITE:
    case INPUTTYPE_CCIR:
    default:
        AndDataByte(SAA7134_LUMA_CTRL, ~SAA7134_LUMA_CTRL_BYPS);
        break;
    }

    BYTE MuxSelect = m_TVCards[m_CardType].Inputs[nInput].MuxSelect;
    MaskDataByte(SAA7134_ANALOG_IN_CTRL1, MuxSelect, 0x0F);
}

