/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Types.cpp,v 1.22 2003-01-30 07:19:47 ittarnavsky Exp $
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
// Revision 1.21  2003/01/28 03:41:21  atnak
// Updated MEDION TV-TUNER 7134 MK2/3
//
// Revision 1.20  2003/01/27 12:32:37  atnak
// Updated Medion TV-Tuner 7134 MK2/3
//
// Revision 1.19  2003/01/27 03:17:46  atnak
// Added card support for "MEDION TV-TUNER 7134 MK2/3"
//
// Revision 1.18  2002/12/26 05:06:17  atnak
// Added Terratec Cinergy 600 TV
//
// Revision 1.17  2002/12/24 08:22:14  atnak
// Added Prime 7133 card
//
// Revision 1.16  2002/12/22 04:03:58  atnak
// Added FlyVideo2000 autodetect tab
//
// Revision 1.15  2002/12/22 03:52:11  atnak
// Fixed FlyVideo2000 GPIO settings for input change
//
// Revision 1.14  2002/12/14 00:29:35  atnak
// Added Manli M-TV001 card
//
// Revision 1.13  2002/12/10 11:05:46  atnak
// Fixed FlyVideo 3000 audio for external inputs
//
// Revision 1.12  2002/11/12 01:26:25  atnak
// Changed the define name of a card
//
// Revision 1.11  2002/10/28 11:10:13  atnak
// Various changes and revamp to settings
//
// Revision 1.10  2002/10/26 05:24:23  atnak
// Minor cleanups
//
// Revision 1.9  2002/10/26 04:41:44  atnak
// Clean up + added auto card detection
//
// Revision 1.8  2002/10/16 22:10:56  atnak
// fixed some cards to available FM1216ME_MK3 tuner
//
// Revision 1.7  2002/10/16 11:40:09  atnak
// Added KWORLD KW-TV713XRF card.  Thanks "b"
//
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


const CSAA7134Card::TCardType CSAA7134Card::m_SAA7134Cards[] =
{
    // SAA7134CARDID_UNKNOWN - Unknown Card
    {
        "*Unknown Card*",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_ABSENT,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_PROTEUSPRO - Proteus Pro [philips reference design]
    {
        "Proteus Pro [philips reference design]",
        2,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_FLYVIDEO3000 - LifeView FlyVIDEO3000
    {
        "LifeView FlyVIDEO3000",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        FLYVIDEO3000CardInputSelect,
    },
    // SAA7134CARDID_FLYVIDEO2000 - LifeView FlyVIDEO2000 (saa7130)
    {
        "LifeView FlyVIDEO2000",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_LG_TAPCNEW_PAL,
        AUDIOCRYSTAL_NONE,
        NULL,
        FLYVIDEO2000CardInputSelect,
    },
    // SAA7134CARDID_EMPRESS - EMPRESS (has TS, i2srate=48000, has CCIR656 video out)
    {
        "EMPRESS",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_MONSTERTV - SKNet Monster TV
    {
        "SKNet Monster TV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_MK2_NTSC,
        AUDIOCRYSTAL_32110Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_TEVIONMD9717 - Tevion MD 9717
    {
        "Tevion MD 9717",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN2,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_KNC1RDS - KNC One TV-Station RDS
    {
        "KNC One TV-Station RDS",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN2,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_CINERGY400 - Terratec Cinergy 400 TV
    {
        "Terratec Cinergy 400 TV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN4,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_MEDION5044 - Medion 5044
    {
        "Medion 5044",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        MEDION5044CardInputSelect,
    },
    // SAA7134CARDID_KWTV713XRF - KWORLD KW-TV713XRF (saa7130)
    // Thanks "b" <b@ki...>
    {
        "KWORLD KW-TV713XRF / KUROUTO SHIKOU",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_NTSC,
        AUDIOCRYSTAL_NONE,
        NULL,
        KWTV713XRFCardInputSelect,
    },
    // SAA7134CARDID_MANLIMTV001 - Manli M-TV001 (saa7130)
    // Thanks "Bedo" Bedo@dscaler.forums
    {
        "Manli M-TV001",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_LG_B11D_PAL,  // Should be LG TPI8PSB12P PAL B/G
        AUDIOCRYSTAL_NONE,
        NULL,
        ManliMTV001CardInputSelect,
    },
    // SAA7134CARDID_PRIMETV7133 - PrimeTV 7133 (saa7133)
    // Thanks "Shin'ya Yamaguchi" <yamaguchi@no...>
    {
        "PrimeTV 7133",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_PHILIPS_NTSC,  // Should be TCL2002NJ or Philips FI1286 (NTSC M-J)
        AUDIOCRYSTAL_NONE,
        NULL,
        PrimeTV7133CardInputSelect,
    },
    // SAA7134CARDID_CINERGY600 - Terratec Cinergy 600 TV
    // Thanks "Michel de Glace" <mglace@my...>
    {
        "Terratec Cinergy 600 TV",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN4,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,          
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_MEDION7134 - Medion TV-Tuner 7134 MK2/3
    // Thanks "DavidbowiE" Guest@dscaler.forums
    // Thanks "Josef Schneider" <josef@ne...>
    {
        "Medion TV-Tuner 7134 MK2/3",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        NULL,
        StandardSAA7134InputSelect,
    },
};


const CSAA7134Card::TAutoDetectSAA7134 CSAA7134Card::m_AutoDetectSAA7134[] =
{
    // DeviceId, Subsystem vendor Id, Subsystem Id, Card Id
    { 0x7134, 0x1131, 0x0000, SAA7134CARDID_UNKNOWN      },
    { 0x7130, 0x1131, 0x0000, SAA7134CARDID_UNKNOWN      },
    { 0x7134, 0x1131, 0x2001, SAA7134CARDID_PROTEUSPRO   },
    { 0x7134, 0x1131, 0x6752, SAA7134CARDID_EMPRESS      },
    { 0x7134, 0x1131, 0x4E85, SAA7134CARDID_MONSTERTV    },
    { 0x7134, 0x153B, 0x1142, SAA7134CARDID_CINERGY400   },
    { 0x7130, 0x5168, 0x0138, SAA7134CARDID_FLYVIDEO2000 },
    { 0x7133, 0x5168, 0x0138, SAA7134CARDID_PRIMETV7133  },
    { 0x7134, 0x153b, 0x1143, SAA7134CARDID_CINERGY600   },
    { 0x7134, 0x16be, 0x0003, SAA7134CARDID_MEDION7134   },
};


int CSAA7134Card::GetMaxCards()
{
    return sizeof(m_SAA7134Cards)/sizeof(TCardType);
}


CSAA7134Card::eSAA7134CardId CSAA7134Card::AutoDetectCardType()
{
    WORD DeviceId           = GetDeviceId();
    WORD SubSystemId        = (GetSubSystemId() & 0xFFFF0000) >> 16;
    WORD SubSystemVendorId  = (GetSubSystemId() & 0x0000FFFF);

    int ListSize = sizeof(m_AutoDetectSAA7134)/sizeof(TAutoDetectSAA7134);

    for (int i(0); i < ListSize; i++)
    {
        if (m_AutoDetectSAA7134[i].DeviceId == DeviceId &&
            m_AutoDetectSAA7134[i].SubSystemId == SubSystemId &&
            m_AutoDetectSAA7134[i].SubSystemVendorId == SubSystemVendorId)
        {
            LOG(0, "SAA713x: Autodetect found %s.",
                GetCardName(m_AutoDetectSAA7134[i].CardId));
            return m_AutoDetectSAA7134[i].CardId;
        }
    }

    LOG(0, "SAA713x: Autodetect found an unknown card with the following");
    LOG(0, "SAA713x: properties.  Please email the author and quote the");
    LOG(0, "SAA713x: following numbers, as well as which card you have,");
    LOG(0, "SAA713x: so it can be added to the list:");
    LOG(0, "SAA713x: DeviceId: 0x%04x, SubVendorSystemId: 0x%04x%04x,",
        DeviceId, SubSystemVendorId, SubSystemId);

    return SAA7134CARDID_UNKNOWN;
}

eTunerId CSAA7134Card::AutoDetectTuner(eSAA7134CardId CardId)
{
    return m_SAA7134Cards[CardId].TunerId;
}


int CSAA7134Card::GetNumInputs()
{
    return m_SAA7134Cards[m_CardType].NumInputs;
}


LPCSTR CSAA7134Card::GetInputName(int nInput)
{
    if(nInput < m_SAA7134Cards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_SAA7134Cards[m_CardType].Inputs[nInput].szName;
    }
    return "Error";
}


BOOL CSAA7134Card::IsInputATuner(int nInput)
{
    return (m_SAA7134Cards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_TUNER);
}


BOOL CSAA7134Card::IsCCIRSource(int nInput)
{
    return (m_SAA7134Cards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
}


LPCSTR CSAA7134Card::GetCardName(eSAA7134CardId CardId)
{
    return m_SAA7134Cards[CardId].szName;
}


void CSAA7134Card::SetVideoSource(int nInput)
{
    // call correct function
    // this funny syntax is the only one that works
    // if you want help understanding what is going on
    // I suggest you read http://www.newty.de/
    (*this.*m_SAA7134Cards[m_CardType].pInputSwitchFunction)(nInput);
}


const CSAA7134Card::TCardType* CSAA7134Card::GetCardSetup()
{
    return &(m_SAA7134Cards[m_CardType]);
}


/*
 *  LifeView's audio chip connected accross GPIO mask 0xE000.
 *  Used by FlyVideo3000, FlyVideo2000 and PrimeTV 7133.
 *  (Below information is an unverified guess --AtNak)
 *
 *  NNNx
 *  ^^^
 *  |||- 0 = Normal, 1 = BTSC processing on ?
 *  ||-- 0 = Internal audio, 1 = External line pass through
 *  |--- 0 = Audio processor ON, 1 = Audio processor OFF
 *
 *  Use Normal/Internal/Audio Processor ON for FM Radio
 */


void CSAA7134Card::FLYVIDEO3000CardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);
    switch(nInput)
    {
    case 0: // Tuner
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x8000, 0xE000);
        break;
    case 1: // Composite
    case 2: // S-Video
    case 3: // Composite over S-Video
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x4000, 0xE000);
        break;
    case 4: // Radio
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x0000, 0xE000);
        break;
    case -1: // Ending cleanup
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x8000, 0xE000);
        break;
    default:
        break;
    }
}


void CSAA7134Card::FLYVIDEO2000CardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);
    switch(nInput)
    {
    case 0: // Tuner
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x0000, 0xE000);
        break;
    case 1: // Composite
    case 2: // S-Video
    case 3: // Composite over S-Video
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x4000, 0xE000);
        break;
    case 4: // Radio
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x0000, 0xE000);
        break;
    case -1: // Ending cleanup
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x8000, 0xE000);
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
    // know what they are
}


void CSAA7134Card::PrimeTV7133CardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);
    switch(nInput)
    {
    case 0: // Tuner
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x2000, 0xE000);
        break;
    case 1: // Composite
    case 2: // S-Video
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x4000, 0xE000);
        break;
    case -1: // Ending cleanup
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x0018e700, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x8000, 0xE000);
        break;
    default:
        break;
    }
}


void CSAA7134Card::ManliMTV001CardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);
    switch(nInput)
    {
    case 0:
    case 1:
    case 2:
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x6000, 0x6000);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x0000, 0x6000);
        break;
    default:
        break;
    }
}


void CSAA7134Card::StandardSAA7134InputSelect(int nInput)
{
    eVideoInputSource VideoInput;

    // -1 for finishing clean up
    if(nInput == -1)
    {
        // do nothing
        return;
    }

    if(nInput >= m_SAA7134Cards[m_CardType].NumInputs)
    {
        LOG(1, "Input Select Called for invalid input");
        nInput = m_SAA7134Cards[m_CardType].NumInputs - 1;
    }
    if(nInput < 0)
    {
        LOG(1, "Input Select Called for invalid input");
        nInput = 0;
    }

    VideoInput = m_SAA7134Cards[m_CardType].Inputs[nInput].VideoInputPin;

    /// There is a 1:1 correlation between (int)eVideoInputSource
    /// and SAA7134_ANALOG_IN_CTRL1_MODE
    BYTE Mode = (VideoInput == VIDEOINPUTSOURCE_NONE) ? 0x00 : VideoInput;

    switch (m_SAA7134Cards[m_CardType].Inputs[nInput].InputType)
    {
    case INPUTTYPE_SVIDEO:
        OrDataByte(SAA7134_LUMA_CTRL, SAA7134_LUMA_CTRL_BYPS);

        switch (VideoInput)
        {
        // This new mode sets Y-channel with automatic
        // gain control and gain control for C-channel
        // linked to Y-channel
        case VIDEOINPUTSOURCE_PIN0: Mode = 0x08; break;
        case VIDEOINPUTSOURCE_PIN1: Mode = 0x09; break;
        default:
            // NEVER_GET_HERE;
            break;
        }
        break;
    case INPUTTYPE_TUNER:
    case INPUTTYPE_COMPOSITE:
    case INPUTTYPE_CCIR:
    default:
        AndDataByte(SAA7134_LUMA_CTRL, ~SAA7134_LUMA_CTRL_BYPS);
        break;
    }

    MaskDataByte(SAA7134_ANALOG_IN_CTRL1, Mode, 0x0F);
}

