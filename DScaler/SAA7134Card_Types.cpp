/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Types.cpp,v 1.4 2002-10-03 23:36:23 atnak Exp $
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
        NULL,
        0,
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
        NULL,
        0,
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
        NULL,
        0,
    },
    // Card Number 3 - LifeView FlyVIDEO2000
    {
        "LifeView FlyVIDEO2000",
        4,
        {
            {
                "Tuner",  // gpio: 0x0000
                INPUTTYPE_TUNER,
                1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",  // gpio: 0x4000
                INPUTTYPE_COMPOSITE,
                0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite 2",  // gpio: 0x4000
                INPUTTYPE_COMPOSITE,
                3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",  // gpio: 0x4000
                INPUTTYPE_SVIDEO,
                8,
                AUDIOINPUTSOURCE_LINE2,
            },
/*          {
                "Mute",
                INPUTTYPE_MUTE,
                0,
                AUDIOINPUTSOURCE_LINE1,
            },*/
        },
        TUNER_LG_TAPCNEW_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
        NULL,
        0x6000,
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
        NULL,
        0,
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
        NULL,
        0,
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
        NULL,
        0,
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
        NULL,
        0,
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
        NULL,
        0,
    },
    // Card Number 9 - Medion 5044
    {
        "Medion 5044",
        5,
        {
            {
                "Tuner", // gpio: 0x4000
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
        StandardSAA7134InputSelect,
        NULL,
        0x6000,
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
/*

void CBT848Card::RSBTCardInputSelect(int nInput)
{
    StandardBT848InputSelect(nInput);
    switch(nInput)
    {
    case 3:
        AndOrDataDword(BT848_GPIO_DATA, 0x1000, ~0x1F800);
        Sleep();
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    case 4:
        AndOrDataDword(BT848_GPIO_DATA, 0x9000, ~0x1F800);
        Sleep();
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    case 5:
        AndOrDataDword(BT848_GPIO_DATA, 0x11000, ~0x1F800);
        Sleep();
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    case 6:
        AndOrDataDword(BT848_GPIO_DATA, 0x19000, ~0x1F800);
        Sleep();
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    default:
        break;
    }
}
*/

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


// ----------------------------------------------------------------------- 
// Card specifc settings for those cards that
// use GPIO pinds to control the stereo output
// from the tuner
/*
void CBT848Card::SetAudioGVBCTV3PCI(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x300);
    switch(soundChannel)
    {
    case SOUNDCHANNEL_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x200, ~0x300);
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x300, ~0x300);
        break;
    default:
    case SOUNDCHANNEL_LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x000, ~0x300);
        break;
    }
}

void CBT848Card::SetAudioLT9415(eSoundChannel soundChannel)
{
    switch(soundChannel)
    {
    case SOUNDCHANNEL_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x0880, ~0x0880);
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x0080, ~0x0880);
        break;
    default:
    case SOUNDCHANNEL_LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x0880);
        break;
    }
}

void CBT848Card::SetAudioTERRATV(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(soundChannel)
    {
    case SOUNDCHANNEL_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x180000, ~0x180000);
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x080000, ~0x180000);
        break;
    default:
    case SOUNDCHANNEL_LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x000000, ~0x180000);
        break;
    }
}

void CBT848Card::SetAudioAVER_TVPHONE(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(soundChannel)
    {
    case SOUNDCHANNEL_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x01, ~0x03);
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x02, ~0x03);
        break;
    default:
        break;
    }
}

void CBT848Card::SetAudioWINFAST2000(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(soundChannel)
    {
    case SOUNDCHANNEL_STEREO:
        AndOrDataDword(BT848_GPIO_DATA, 0x020000, ~0x430000);
        break;
    case SOUNDCHANNEL_LANGUAGE1:
        AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x410000, ~0x430000);
        break;
    default:
        AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    }
}

void CBT848Card::SetAudioWINDVR(eSoundChannel soundChannel)
{
    OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(soundChannel)                                                                   
    {
    case SOUNDCHANNEL_MONO:
        AndOrDataDword(BT848_GPIO_DATA, 0x040000, ~0x140000);
        break;
    case SOUNDCHANNEL_LANGUAGE2:
        AndOrDataDword(BT848_GPIO_DATA, 0x100000, ~0x140000);
        break;
    default:
        //
        break;
    }
}
*/


