/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Types.cpp,v 1.31 2003-02-20 10:39:59 adcockj Exp $
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
// Revision 1.30  2002/10/29 11:05:27  adcockj
// Renamed CT2388x to CX2388x
//
// Revision 1.29  2002/10/27 13:27:13  adcockj
// Fix osprey svideo
//
// Revision 1.28  2002/10/22 09:22:23  adcockj
// Fixed PLL problem with Leadtek WinView 601 (fix from bugs list)
//
// Revision 1.27  2002/10/16 21:40:19  kooiman
// Improved detection for new Pinnacle/Miro cards.
//
// Revision 1.26  2002/10/11 21:41:32  ittarnavsky
// changes due to the eAudioDecoderType move
//
// Revision 1.25  2002/10/11 13:35:49  kooiman
// Improved Pinnacle/Miro pro detection.
//
// Revision 1.24  2002/09/12 21:58:13  ittarnavsky
// Added new card AventMedia TVPhone Old
// Changes due to the new AudioDecoder handling
//
// Revision 1.23  2002/09/11 18:19:36  adcockj
// Prelimainary support for CX2388x based cards
//
// Revision 1.22  2002/09/04 11:58:45  kooiman
// Added new tuners & fix for new Pinnacle cards with MT2032 tuner.
//
// Revision 1.21  2002/08/03 17:57:52  kooiman
// Added new cards & tuners. Changed the tuner combobox into a sorted list.
//
// Revision 1.20  2002/07/29 13:10:20  dschmelzer
// Thanks to Laurent for spotting the typo on c-video jumper
//
// Revision 1.19  2002/07/25 05:22:18  dschmelzer
// Added SDI Silk 200 Support
//
// Revision 1.18  2002/07/23 18:11:48  adcockj
// Tuner autodetect patch from Jeroen Kooiman
//
// Revision 1.17  2002/06/05 20:02:27  adcockj
// Applied old RS BT card patch
//
// Revision 1.16  2002/03/24 19:40:14  adcockj
// Added Skywell magic card
//
// Revision 1.15  2002/03/01 06:37:47  dschmelzer
// Update Silk Settings for Latest Drivers
//
// Revision 1.14  2002/02/26 18:31:44  adcockj
// Added extra composite mux to standard card
//
// Revision 1.13  2002/02/26 18:28:49  adcockj
// Fix s-video in on voodoo cards
//
// Revision 1.12  2002/02/03 18:14:40  adcockj
// Fixed SDI Silk & Sasem cards
//
// Revision 1.11  2002/01/16 19:07:04  adcockj
// Corrected voodoo cards
//
// Revision 1.10  2001/12/21 11:07:31  adcockj
// Even more RevA fixes
//
// Revision 1.9  2001/12/19 19:24:44  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.8  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.7  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.6  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.5  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.4  2001/11/18 02:47:08  ittarnavsky
// added all v3.1 suported cards
//
// Revision 1.3  2001/11/13 17:06:10  adcockj
// Bug Fixes
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "DebugLog.h"

const CBT848Card::TCardType CBT848Card::m_TVCards[TVCARD_LASTONE] = 
{
    // Card Number 0 - Unknown Card
    {
        "*Unknown Card*",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 1 - MIRO PCTV
    {
        "MIRO PCTV",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF,
        {0x2, 0, 0, 0, 0xA, 0, }
    },
    // Card Number 2 - Hauppauge old
    {
        "Hauppauge old",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        InitHauppauge,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x1, 0x2, 0x3, 0x4, 0, }
    },
    // Card Number 3 - STB
    {
        "STB",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0x4, 0, 0x2, 0x3, 0x1, 0, }
    },
    // Card Number 4 - Intel
    {
        "Intel",
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x1, 0x2, 0x3, 0x4, 0, }
    },
    // Card Number 5 - Diamond DTV2000
    {
        "Diamond DTV2000",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x3,
        {0, 0x1, 0, 0x1, 0x3, 0, }
    },
    // Card Number 6 - AVerMedia TVPhone
    {
        "AVerMedia TVPhone",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_AVER_TVPHONE_NEW,
        0xF,
        {0xC, 0x4, 0xB, 0xB, 0, 0, }
    },
    // Card Number 7 - MATRIX-Vision MV-Delta
    {
        "MATRIX-Vision MV-Delta",
        5,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
                CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 8 - Fly Video II
    {
        "Fly Video II",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xC00,
        {0, 0xC00, 0x800, 0x400, 0xC00, 0, }
    },
    // Card Number 9 - TurboTV
    {
        "TurboTV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x3,
        {0x1, 0x1, 0x2, 0x3, 0, 0, }
    },
    // Card Number 10 - Standard BT878
    {
        "Standard BT878 / Hauppauge BT878",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_MSP,
        InitHauppauge,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x1, 0x2, 0x3, 0x4, 0, }
    },
    // Card Number 11 - MIRO PCTV pro
    {
        "MIRO PCTV pro",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1000F,
        {0x1, 0x10001, 0, 0, 0xA, 0, }
    },
    // Card Number 12 - ADS Technologies Channel Surfer TV
    {
        "ADS Technologies Channel Surfer TV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                1,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF,
        {0xD, 0xE, 0xB, 0x7, 0, 0, }
    },
    // Card Number 13 - AVerMedia TVCapture 98
    {
        "AVerMedia TVCapture 98",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF,
        {0xD, 0xE, 0xB, 0x7, 0, 0, }
    },
    // Card Number 14 - Aimslab VHX
    {
        "Aimslab VHX",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x1, 0x2, 0x3, 0x4, 0, }
    },
    // Card Number 15 - Zoltrix TV-Max
    {
        "Zoltrix TV-Max",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF,
        {0, 0, 0x1, 0, 0xA, 0, }
    },
    // Card Number 16 - Pixelview PlayTV (bt878)
    {
        "Pixelview PlayTV (bt878)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1FE00,
        {0x1C000, 0, 0x18000, 0x14000, 0x2000, 0, }
    },
    // Card Number 17 - Leadtek WinView 601
    {
        "Leadtek WinView 601",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x8300F8,
        {0x4FA007, 0xCFA007, 0xCFA007, 0xCFA007, 0xCFA007, 0xCFA007, }
    },
    // Card Number 18 - AVEC Intercapture
    {
        "AVEC Intercapture",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0x1, 0, 0, 0, 0, 0, }
    },
    // Card Number 19 - LifeView FlyKit w/o Tuner
    {
        "LifeView FlyKit w/o Tuner",
        3,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x8DFF00,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 20 - CEI Raffles Card
    {
        "CEI Raffles Card",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 21 - Lucky Star Image World ConferenceTV
    {
        "Lucky Star Image World ConferenceTV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFE07,
        {0x20000, 0x1, 0x190000, 0x3, 0x4, 0, }
    },
    // Card Number 22 - Phoebe Tv Master + FM
    {
        "Phoebe Tv Master + FM",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xC00,
        {0, 0x1, 0x800, 0x400, 0xC00, 0, }
    },
    // Card Number 23 - Modular Technology MM205 PCTV, bt878
    {
        "Modular Technology MM205 PCTV, bt878",
        2,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 24 - Askey/Typhoon/Anubis Magic TView CPH051/061 (bt878)
    {
        "Askey/Typhoon/Anubis Magic TView CPH051/061 (bt878)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xE00,
        {0x400, 0x400, 0x400, 0x400, 0, 0, }
    },
    // Card Number 25 - Terratec/Vobis TV-Boostar
    {
        "Terratec/Vobis TV-Boostar",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFFFF,
        {0x20000, 0x1, 0x190000, 0x3, 0x4, 0, }
    },
    // Card Number 26 - Newer Hauppauge WinCam (bt878)
    {
        "Newer Hauppauge WinCam (bt878)",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x1, 0x2, 0x3, 0x4, 0, }
    },
    // Card Number 27 - MAXI TV Video PCI2
    {
        "MAXI TV Video PCI2",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_PHILIPS_SECAM,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFF,
        {0, 0x1, 0x2, 0x3, 0xC00, 0, }
    },
    // Card Number 28 - Terratec TerraTV+
    {
        "Terratec TerraTV+",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_TERRATV,
        0x70000,
        {0x20000, 0x30000, 0, 0x10000, 0x40000, 0, }
    },
    // Card Number 29 - Imagenation PXC200
    {
        "Imagenation PXC200",
        5,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        InitPXC200,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 30 - FlyVideo 98
    {
        "FlyVideo 98",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x8DFE00,
        {0, 0x8DFF00, 0x8DF700, 0x8DE700, 0x8DFF00, 0, }
    },
    // Card Number 31 - iProTV
    {
        "iProTV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1,
        {0x1, 0, 0, 0, 0, 0, }
    },
    // Card Number 32 - Intel Create and Share PCI
    {
        "Intel Create and Share PCI",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0x4, 0x4, 0x4, 0x4, 0x4, 0x4, }
    },
    // Card Number 33 - Terratec TerraTValue
    {
        "Terratec TerraTValue",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFF00,
        {0x500, 0, 0x300, 0x900, 0x900, 0, }
    },
    // Card Number 34 - Leadtek WinFast 2000
    {
        "Leadtek WinFast 2000",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_WINFAST2000,
        0xFFF000,
        {0x621000, 0x620100, 0x621100, 0x620000, 0xE210000, 0x620000, }
    },
    // Card Number 35 - Chronos Video Shuttle II
    {
        "Chronos Video Shuttle II",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1800,
        {0, 0, 0x1000, 0x1000, 0x800, 0, }
    },
    // Card Number 36 - Typhoon TView TV/FM Tuner
    {
        "Typhoon TView TV/FM Tuner",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1800,
        {0, 0x800, 0, 0, 0x1800, 0, }
    },
    // Card Number 37 - PixelView PlayTV pro
    {
        "PixelView PlayTV pro",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFF,
        {0x21, 0x20, 0x24, 0x2C, 0x29, 0x29, }
    },
    // Card Number 38 - TView99 CPH063
    {
        "TView99 CPH063",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x551E00,
        {0x551400, 0x551200, 0, 0, 0, 0x551200, }
    },
    // Card Number 39 - Pinnacle PCTV Rave
    {
        "Pinnacle PCTV Rave",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x3000F,
        {0x2, 0, 0, 0, 0x1, 0, }
    },
    // Card Number 40 - STB2
    {
        "STB2",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0x4, 0, 0x2, 0x3, 0x1, 0, }
    },
    // Card Number 41 - AVerMedia TVPhone 98
    {
        "AVerMedia TVPhone 98",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x4,
        {0xD, 0xE, 0xB, 0x7, 0, 0, }
    },
    // Card Number 42 - ProVideo PV951
    {
        "ProVideo PV951",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 43 - Little OnAir TV
    {
        "Little OnAir TV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xE00B,
        {0xFF9FF6, 0xFF9FF6, 0xFF1FF7, 0, 0xFF3FFC, 0, }
    },
    // Card Number 44 - Sigma TVII-FM
    {
        "Sigma TVII-FM",
        2,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
        },
        PLL_NONE,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x3,
        {0x1, 0x1, 0, 0x2, 0x3, 0, }
    },
    // Card Number 45 - MATRIX-Vision MV-Delta 2
    {
        "MATRIX-Vision MV-Delta 2",
        5,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 46 - Zoltrix Genie TV
    {
        "Zoltrix Genie TV",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xBCF03F,
        {0xBC803F, 0, 0xBCB03F, 0, 0xBCB03F, 0, }
    },
    // Card Number 47 - Terratec TV/Radio+
    {
        "Terratec TV/Radio+",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_35,
        TUNER_PHILIPS_PAL_I,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1F0000,
        {0xE2FFFF, 0, 0, 0, 0xE0FFFF, 0xE2FFFF, }
    },
    // Card Number 48 - Dynalink Magic TView
    {
        "Dynalink Magic TView",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF,
        {0x2, 0, 0, 0, 0x1, 0, }
    },
    // Card Number 49 - Conexant Bt878 NTSC XEVK
    {
        "Conexant Bt878 NTSC XEVK",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_PHILIPS_NTSC,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFEFF,
        {0x1000, 0x1000, 0, 0, 0x3000, 0, }
    },
    // Card Number 50 - Rockwell Bt878 NTSC XEVK
    {
        "Rockwell Bt878 NTSC XEVK",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_PHILIPS_NTSC,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFEFF,
        {0x1000, 0x1000, 0, 0, 0x3000, 0, }
    },
    // Card Number 51 - Conexant Foghorn NTSC/ATSC-A
    {
        "Conexant Foghorn NTSC/ATSC-A",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFF00F8,
        {0x48, 0x48, 0x48, 0x48, 0x48, 0x48, }
    },
    // Card Number 52 - Conexant Foghorn NTSC/ATSC-B
    {
        "Conexant Foghorn NTSC/ATSC-B",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFF00F8,
        {0x48, 0x48, 0x48, 0x48, 0x48, 0x48, }
    },
    // Card Number 53 - Conexant Foghorn NTSC/ATSC-C
    {
        "Conexant Foghorn NTSC/ATSC-C",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFF00F8,
        {0x48, 0x48, 0x48, 0x48, 0x48, 0x48, }
    },
    // Card Number 54 - RS BT Card
    {
        "RS BT Card",
        7,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
            {
                "CCIR 1",
                INPUTTYPE_CCIR,
                0,
            },
            {
                "CCIR 2",
                INPUTTYPE_CCIR,
                0,
            },
            {
                "CCIR 3",
                INPUTTYPE_CCIR,
                0,
            },
            {
                "CCIR 4",
                INPUTTYPE_CCIR,
                0,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        InitRSBT,
        RSBTCardInputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 55 - Cybermail AV
    {
        "Cybermail AV",
        4,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFEFF,
        {0x1000, 0x1000, 0, 0, 0x3000, 0, }
    },
    // Card Number 56 - Viewcast Osprey
    {
        "Viewcast Osprey",
        4,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 57 - ATI TV-Wonder
    {
        "ATI TV-Wonder",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF03F,
        {0xBFFE, 0, 0xBFFF, 0, 0xBFFE, 0, }
    },
    // Card Number 58 - ATI TV-Wonder VE
    {
        "ATI TV-Wonder VE",
        2,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1,
        {0, 0, 0x1, 0, 0, 0, }
    },
    // Card Number 59 - GV-BCTV3
    {
        "GV-BCTV3",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
            },
        },
        PLL_28,
        TUNER_ALPS_TSCH6_NTSC,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_GVBCTV3,
        0x10F00,
        {0x10000, 0, 0x10000, 0, 0, 0, }
    },
    // Card Number 60 - Prolink PV-BT878P+4E (PixelView PlayTV PAK)
    {
        "Prolink PV-BT878P+4E (PixelView PlayTV PAK)",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xAA0000,
        {0x20000, 0, 0x80000, 0x80000, 0xA8000, 0x46000, }
    },
    // Card Number 61 - Eagle Wireless Capricorn2 (bt878A)
    {
        "Eagle Wireless Capricorn2 (bt878A)",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x1, 0x2, 0x3, 0x4, 0, }
    },
    // Card Number 62 - Pinnacle PCTV Studio Pro
    {
        "Pinnacle PCTV Studio Pro",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x3000F,
        {0x1, 0x10001, 0, 0, 0xA, 0, }
    },
    // Card Number 63 - Typhoon TView RDS / FM Stereo
    {
        "Typhoon TView RDS / FM Stereo",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1C,
        {0, 0, 0x10, 0x8, 0x4, 0, }
    },
    // Card Number 64 - Lifetec LT 9415 TV
    {
        "Lifetec LT 9415 TV",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_LT9415,
        0x18E0,
        {0, 0x800, 0x1000, 0x1000, 0x18E0, 0, }
    },
    // Card Number 65 - BESTBUY Easy TV
    {
        "BESTBUY Easy TV",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_TEMIC_4002FH5_PAL,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF,
        {0x2, 0, 0, 0, 0xA, 0, }
    },
    // Card Number 66 - FlyVideo '98/FM / 2000S
    {
        "FlyVideo '98/FM / 2000S",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x18E0,
        {0, 0x18E0, 0x1000, 0x1000, 0x1080, 0x1080, }
    },
    // Card Number 67 - GrandTec 'Grand Video Capture'
    {
        "GrandTec 'Grand Video Capture'",
        3,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_35,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 68 - Phoebe TV Master Only (No FM)
    {
        "Phoebe TV Master Only (No FM)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_TEMIC_4036FY5_NTSC,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xE00,
        {0x400, 0x400, 0x400, 0x400, 0x800, 0x400, }
    },
    // Card Number 69 - TV Capturer
    {
        "TV Capturer",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x3000F,
        {0x2, 0, 0, 0, 0x1, 0, }
    },
    // Card Number 70 - MM100PCTV
    {
        "MM100PCTV",
        2,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
        },
        PLL_NONE,
        TUNER_TEMIC_4002FH5_PAL,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xB,
        {0x2, 0, 0, 0x1, 0x8, 0, }
    },
    // Card Number 71 - AG Electronics GMV1
    {
        "AG Electronics GMV1",
        3,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xF,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 72 - BESTBUY Easy TV (bt878)
    {
        "BESTBUY Easy TV (bt878)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFF,
        {0x1, 0, 0x4, 0x4, 0x9, 0, }
    },
    // Card Number 73 - Sasem 4-Channel Dev Board (S-Video Jumper)
    {
        "Sasem 4-Channel Dev Board (S-Video Jumper)",
        4,
        {
            {
                "Composite 1",
                INPUTTYPE_COMPOSITE,
                0x00,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0x01,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                0x03,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0xD2
            },
        },
        PLL_NONE,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        NULL,
        Sasem4ChannelInputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 74 - Pinnacle PCTV Sat
    {
        "Pinnacle PCTV Sat",
        3,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x3000F,
        {0, 0, 0, 0, 0x1, 0, }
    },
    // Card Number 75 - Aimslab VideoHighway Extreme (not 98)
    {
        "Aimslab VideoHighway Extreme (not 98)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x2, 0x1, 0x3, 0x4, 0, }
    },
    // Card Number 76 - 3dfx `TV 200 (USA)
    {
        "3dfx VoodooTV 200 (USA)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_MT2032,
        SOUNDCHIP_NONE,
        InitVoodoo,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x4F8A00,
        {0x957FFF, 0x997FFF, 0x957FFF, 0x957FFF, 0, 0, }
    },
    // Card Number 77 - 3dfx VoodooTV FM (Europa)
    {
        "3dfx VoodooTV FM (Europa)",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_MT2032,
        SOUNDCHIP_NONE,
        InitVoodoo,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x4F8A00,
        {0x947FFF, 0x987FFF, 0x947FFF, 0x947FFF, 0, 0, }
    },
    // Card Number 78 - Standard BT878 (No Init MSP)
    {
        "Standard BT878 (No Init MSP)",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x7,
        {0, 0x1, 0x2, 0x3, 0x4, 0, }
    },
    // Card Number 79 - Terratec TValueRadio
    {
        "Terratec TValueRadio",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xFFFF00,
        {0x500, 0x500, 0x300, 0x900, 0x900, 0, }
    },
    // Card Number 80 - Flyvideo 98EZ (capture only)
    {
        "Flyvideo 98EZ (capture only)",
        5,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                2,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 81 - Active Imaging AIMMS
    {
        "Active Imaging AIMMS",
        2,
        {
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                2,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                2,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 82 - FlyVideo 2000S
    {
        "FlyVideo 2000S",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
            },
        },
        PLL_28,
        TUNER_AUTODETECT,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x18E0,
        {0, 0x18E0, 0x1000, 0x1000, 0x1080, 0x1080, }
    },
    // Card Number 83 - GV-BCTV4/PCI
    {
        "GV-BCTV4/PCI",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                0,
            },
        },
        PLL_28,
        TUNER_SHARP_2U5JF5540_NTSC,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_GVBCTV3,
        0x10F00,
        {0x10000, 0, 0x10000, 0, 0, 0, }
    },
    // Card Number 84 - Prolink PV-BT878P+4E / PixelView PlayTV PAK / Lenco MXTV-9578 CP
    {
        "Prolink PV-BT878P+4E / PixelView PlayTV PAK / Lenco MXTV-9578 CP",
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0xAA0000,
        {0x20000, 0, 0x80000, 0x80000, 0xA8000, 0x46000, }
    },
    // Card Number 85 - Typhoon TView RDS + FM Stereo / KNC1 TV Station RDS
    {
        "Typhoon TView RDS + FM Stereo / KNC1 TV Station RDS",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        SOUNDCHIP_NONE,
        InitSasem,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1C,
        {0, 0, 0x10, 0x8, 0x4, 0, }
    },
    // Card Number 86 - Sasem 4-Channel Dev Board (C-Video Jumper)
    {
        "Sasem 4-Channel Dev Board (C-Video Jumper)",
        4,
        {
            {
                "Composite 1",
                INPUTTYPE_COMPOSITE,
                0x00,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0x01,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                0x02,
            },
            {
                "Composite 4",
                INPUTTYPE_COMPOSITE,
                0x03,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        InitSasem,
        Sasem4ChannelInputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x0,
        {0, 0, 0, 0, 0, 0, }
    },
    // Card Number 87 - SDI Silk 100 (S-Video Jumper)
    {
        "SDI Silk 100 (S-Video Jumper)",
        5,
        {
            {
                "Composite 1",
                INPUTTYPE_COMPOSITE,
                0x00,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0x01,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                0x03,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0xD2,
            },
            {
                "SDI",
                INPUTTYPE_CCIR,
                0x00,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        InitSasem,
        Sasem4ChannelInputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1F800,
        {0xD, 0xE, 0xB, 0x7, 0, 0, }
    },
    // Card Number 88 - SDI Silk 100 (C-Video Jumper)
    {
        "SDI Silk 100 (C-Video Jumper)",
        5,
        {
            {
                "Composite 1",
                INPUTTYPE_COMPOSITE,
                0x00,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0x01,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                0x02,
            },
            {
                "Composite 4",
                INPUTTYPE_COMPOSITE,
                0x03,
            },
            {
                "SDI",
                INPUTTYPE_CCIR,
                0x00,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        InitSasem,
        Sasem4ChannelInputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1F800,
        {0xD, 0xE, 0xB, 0x7, 0, 0, }
    },
    // Card Number 89 - Skywell Magic TV Card
    {
        "Skywell Magic TV Card",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_28,
        TUNER_USER_SETUP,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x07,
        { 0, 0, 2, 0, 1, 0}
    },
    // Card Number 90 - SDI Silk 200 (S-Video Jumper)
    {
        "SDI Silk 200 (S-Video Jumper)",
        7,
        {
            {
                "Composite 1",
                INPUTTYPE_COMPOSITE,
                0x00,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0x01,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                0x03,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                0xD2,
            },
            {
                "SDI 1",
                INPUTTYPE_CCIR,
                0x00,
            },
            {
                "SDI 2",
                INPUTTYPE_CCIR,
                0x01,
            },
            {
                "SDI 3",
                INPUTTYPE_CCIR,
                0x02,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        InitSasem,
        Silk200InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1F800,
        {0xD, 0xE, 0xB, 0x7, 0, 0, }
    },

    // Card Number 91 - SDI Silk 200 (C-Video Jumper)
    {
        "SDI Silk 200 (C-Video Jumper)",
        7,
        {
            {
                "Composite 1",
                INPUTTYPE_COMPOSITE,
                0x00,
            },
            {
                "Composite 2",
                INPUTTYPE_COMPOSITE,
                0x01,
            },
            {
                "Composite 3",
                INPUTTYPE_COMPOSITE,
                0x02,
            },
            {
                "Composite 4",
                INPUTTYPE_COMPOSITE,
                0x03,
            },
            {
                "SDI 1",
                INPUTTYPE_CCIR,
                0x00,
            },
            {
                "SDI 2",
                INPUTTYPE_CCIR,
                0x01,
            },
            {
                "SDI 3",
                INPUTTYPE_CCIR,
                0x02,
            },
        },
        PLL_28,
        TUNER_ABSENT,
        SOUNDCHIP_NONE,
        InitSasem,
        Silk200InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_DETECT,
        0x1F800,
        {0xD, 0xE, 0xB, 0x7, 0, 0, }
    },
    // Card Number 92 - Sensoray 311                                                                   
    {                                                                                                  
        "Sensoray 311",                                                                                  
    5,                                                                                               
        {                                                                                                
            {                                                                                              
                "Composite 1",                                                                               
                INPUTTYPE_COMPOSITE,                                                                         
                2,                                                                                           
            },                                                                                             
            {                                                                                              
                "Composite 2",                                                                               
                INPUTTYPE_COMPOSITE,                                                                         
                3,                                                                                           
            },                                                                                             
            {                                                                                              
                "Composite 3",                                                                               
                INPUTTYPE_COMPOSITE,                                                                         
                1,                                                                                           
            },                                                                                             
            {                                                                                              
                "S-Video",                                                                                   
                INPUTTYPE_SVIDEO,                                                                            
                0,                                                                                           
            },                                                                                             
       {                                                                                              
                "Composite over S-Video",                                                                    
                INPUTTYPE_COMPOSITE,                                                                         
                0,                                                                                           
            },                                                                                             
        },                                                                                               
        PLL_NONE,                                                                                        
        TUNER_ABSENT,                                                                                    
        SOUNDCHIP_NONE,                                                                                  
        NULL,                                                                                            
        StandardBT848InputSelect,                                                                        
        CAudioDecoder::AUDIODECODERTYPE_DETECT,                                                                                            
        0,                                                                                               
        { 0, 0, 0, 0, 0, 0}                                                                              
    },                                                                                                 
  // Card Number 91 - Canopus WinDVR PCI (COMPAQ Presario 3524JP, 5112JP)
    {                                                                                                  
        "Canopus WinDVR PCI (COMPAQ Presario 3524JP, 5112JP)",                                           
     4,                                                                                               
        {                                                                                                
            {                                                                                              
                "Tuner",                                                                                     
                INPUTTYPE_TUNER,                                                                             
                2,                                                                                           
            },                                                                                             
            {                                                                                              
                "Composite",                                                                                 
                INPUTTYPE_COMPOSITE,                                                                         
                3,                                                                                           
            },                                                                                                 
            {                                                                                              
                "S-Video",                                                                                   
                INPUTTYPE_SVIDEO,                                                                            
                1,                                                                                           
            },                                                                                             
       {                                                                                              
                "Composite over S-Video",                                                                    
                INPUTTYPE_COMPOSITE,                                                                         
                1,                                                                                           
            },                                                                                             
        },                                                                                               
        PLL_NONE,                                                                                        
        TUNER_PHILIPS_NTSC,                                                                              
        SOUNDCHIP_NONE,                                                                                  
        NULL,                                                                                            
        StandardBT848InputSelect,                                                                        
        CAudioDecoder::AUDIODECODERTYPE_WINDVR,                                                                                  
        0x140007,                                                                                        
        { 0, 1, 2, 3, 4, 0 }                                                                             
    },
    {
        /* szName */ "AVerMedia TVPhone (old)",
        /* NumInputs */ 4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        /* PLLFreq */ PLL_35,
        /* TunerId */ TUNER_AUTODETECT,
        /* SoundChip */ SOUNDCHIP_NONE,
        /* pInitCardFunction */ NULL,
        /* pInputSwitchFunction */ StandardBT848InputSelect,
        /* AudioDecoderType */ CAudioDecoder::AUDIODECODERTYPE_AVER_TVPHONE_OLD,
        /* GPIOMask */ 0x703F,
        /* AudioMuxSelect */
        /* TUNER   RADIO     EXTERNAL  INTERNAL  MUTE      STEREO */
        {0x00600e, 0x006006, 0x00600a, 0x00600a, 0x006002, 0, }
    },
    // Patch thanks to Kevin Radke 
    {
        "KWorld MPEGTV RF Pro",
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                2,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                3,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                1,
            },
       {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                1,
            },
        },
        PLL_NONE,
        TUNER_PHILIPS_NTSC,
        SOUNDCHIP_NONE,
        NULL,
        StandardBT848InputSelect,
        CAudioDecoder::AUDIODECODERTYPE_WINDVR,
        0x140007,
        { 0, 1, 2, 3, 4, 0 }
    },
};

const CBT848Card::TAutoDectect878 CBT848Card::m_AutoDectect878[] =
{
    { 0x00011002, TVCARD_ATI_TVWONDER,  "ATI TV Wonder" },
    { 0x00011461, TVCARD_AVPHONE98,     "AVerMedia TVPhone98" },
    { 0x00021461, TVCARD_AVERMEDIA98,   "Avermedia TVCapture 98" },
    { 0x00031002, TVCARD_ATI_TVWONDERVE,"ATI TV Wonder/VE" },
    { 0x00031461, TVCARD_AVPHONE98,     "AVerMedia TVPhone98" },
    { 0x00041461, TVCARD_AVERMEDIA98,   "AVerMedia TVCapture 98" },
    { 0x001211bd, TVCARD_PINNACLERAVE,  "Pinnacle PCTV" },
    { 0x001c11bd, TVCARD_PINNACLESAT,   "Pinnacle PCTV Sat" },
    { 0x10b42636, TVCARD_HAUPPAUGE878,  "STB ???" },
    { 0x1118153b, TVCARD_TERRATVALUE,   "Terratec TV Value" },
    { 0x1123153b, TVCARD_TERRATVRADIO,  "Terratec TV/Radio+" },
    { 0x1200bd11, TVCARD_PINNACLERAVE,  "Pinnacle PCTV Rave" },
    { 0x13eb0070, TVCARD_HAUPPAUGE878,  "Hauppauge WinTV" },
    { 0x18501851, TVCARD_CHRONOS_VS2,   "Chronos Video Shuttle II" },
    { 0x18521852, TVCARD_TYPHOON_TVIEW, "Typhoon TView TV/FM Tuner" },
    { 0x217d6606, TVCARD_WINFAST2000,   "Leadtek WinFast TV 2000" },
    { 0x263610b4, TVCARD_STB2,          "STB TV PCI FM, P/N 6000704" },
    { 0x3000144f, TVCARD_MAGICTVIEW063, "TView 99 (CPH063)" },
    { 0x300014ff, TVCARD_MAGICTVIEW061, "TView 99 (CPH061)" },
    { 0x3002144f, TVCARD_MAGICTVIEW061, "Askey Magic TView" },
    { 0x300214ff, TVCARD_PHOEBE_TVMAS,  "Phoebe TV Master" },
    { 0x39000070, TVCARD_HAUPPAUGE878,  "Hauppauge WinTV-D" },
    { 0x400a15b0, TVCARD_ZOLTRIX_GENIE, "Zoltrix Genie TV" },
    { 0x400d15b0, TVCARD_ZOLTRIX_GENIE, "Zoltrix Genie TV / Radio" },
    { 0x401015b0, TVCARD_ZOLTRIX_GENIE, "Zoltrix Genie TV / Radio" },
    { 0x402010fc, TVCARD_GVBCTV3PCI,    "I-O Data Co. GV-BCV3/PCI" },
    { 0x405010fc, TVCARD_GVBCTV4PCI,    "I-O Data Co. GV-BCV4/PCI" },
    { 0x45000070, TVCARD_HAUPPAUGE878,  "Hauppauge WinTV/PVR" },
    { 0xff000070, TVCARD_VIEWCAST,      "Osprey-100" },
    { 0xff010070, TVCARD_VIEWCAST,      "Osprey-200" },
    { 0x010115cb, TVCARD_GMV1,          "AG GMV1" },
    { 0x14610002, TVCARD_AVERMEDIA98,   "Avermedia TVCapture 98" },
    { 0x182214F1, TVCARD_CONEXANTNTSCXEVK,  "Conexant Bt878A NTSC XEVK" },
    { 0x1322127A, TVCARD_ROCKWELLNTSCXEVK,  "Rockwell Bt878A NTSC XEVK" },
    { 0x013214F1, TVCARD_CONEXANTFOGHORNREVA,  "Conexant Foghorn NTSC/ATSC-A" },
    { 0x023214F1, TVCARD_CONEXANTFOGHORNREVB,  "Conexant Foghorn NTSC/ATSC-B" },
    { 0x033214F1, TVCARD_CONEXANTFOGHORNREVC,  "Conexant Foghorn NTSC/ATSC-C" },
    { 0x3000121A, TVCARD_VOODOOTV_200, "3dfx VoodooTV 200 (USA) / FM (Europa)"},
    { 0x3100121A, TVCARD_VOODOOTV_200, "3dfx VoodooTV 200 (USA) / FM (Europa) (OEM)"},
    // { 0x3060121A, TVCARD_VOODOOTV_100, "3dfx VoodooTV 100"},
    { 0x010114c7, TVCARD_MODTEC_205,    "Modular Technology PCTV" },
    { 0x1117153b, TVCARD_TERRATVALUE,   "Terratec TValue" },
    { 0x1119153b, TVCARD_TERRATVALUE,   "Terratec TValue" },
    { 0x111a153b, TVCARD_TERRATVALUE,   "Terratec TValue" },
    { 0x1127153b, TVCARD_TERRATV,       "Terratec TV+"    },
    { 0x1134153b, TVCARD_TERRATVALUE,   "Terratec TValue" },
    { 0x1135153b, TVCARD_TERRATVALUER,  "Terratec TValue Radio" },
    { 0x18511851, TVCARD_FLYVIDEO98EZ,  "Flyvideo 98EZ (LR51)/ CyberMail AV" },
    { 0x3005144f, TVCARD_MAGICTVIEW061, "(Askey Magic/others) TView99 CPH061/06L (T1/LC)" },
    { 0x401615b0, TVCARD_ZOLTRIX_GENIE, "Zoltrix Genie TV / Radio" },
    { 0x6606107d, TVCARD_WINFAST2000,   "Leadtek WinFast TV 2000" },
    { 0x5018153b, TVCARD_TERRATVALUE,   "Terratec TValue" },
    { 0x03116000, TVCARD_SENSORAY311,   "Sensoray 311" },
    { 0x00790e11, TVCARD_WINDVR,        "Canopus WinDVR PCI" },
    { 0xa0fca1a0, TVCARD_ZOLTRIX,       "Face to Face Tvmax" },    
    { 0x31323334, TVCARD_GRANDTEC,      "GrandTec XCapture" },
    { 0x109e306e, TVCARD_KWORLD_MPEGTV, "KWorld MPEGTV RF Pro" },
    { 0, (eTVCardId)-1, NULL }
};


const eTunerId CBT848Card::m_Tuners_miro[] = 
{
  TUNER_TEMIC_4002FH5_PAL,    
  TUNER_TEMIC_4032FY5_NTSC,
  TUNER_PHILIPS_NTSC,         
  TUNER_PHILIPS_SECAM,   
    TUNER_ABSENT,               
    TUNER_PHILIPS_PAL,
    TUNER_TEMIC_4032FY5_NTSC,   
    TUNER_TEMIC_4002FH5_PAL,  
    TUNER_PHILIPS_SECAM,        
    TUNER_TEMIC_4002FH5_PAL,
    TUNER_ABSENT,                 
    TUNER_PHILIPS_PAL,  
    TUNER_PHILIPS_PAL,            
    TUNER_PHILIPS_NTSC,
    TUNER_TEMIC_4016FY5_PAL,    
    TUNER_PHILIPS_PAL_I,
                
    TUNER_TEMIC_4006FH5_PAL,    
    TUNER_PHILIPS_NTSC,
    TUNER_PHILIPS_MK2_NTSC,     
    TUNER_PHILIPS_PAL_I, 
    TUNER_ABSENT,               
    TUNER_PHILIPS_PAL_I,
    TUNER_ABSENT,               
    TUNER_PHILIPS_SECAM,  
    TUNER_PHILIPS_PAL_I,        
    TUNER_PHILIPS_NTSC,
    TUNER_TEMIC_4016FY5_PAL,    
    TUNER_PHILIPS_PAL_I, 
    TUNER_ABSENT,               
    TUNER_ABSENT,
    TUNER_ABSENT,               
    TUNER_ABSENT  
};

const bool CBT848Card::m_Tuners_miro_fm[] = 
{ 
    false   ,false  ,false  ,false  ,   
    false   ,false  ,false  ,false  ,  
    false   ,false  ,false  ,false  ,  
    false   ,false  ,false  ,true   ,
    true    ,true     ,true   ,true ,   
    true    ,true     ,true   ,false    ,  
    false   ,false  ,false  ,false  ,  
    false   ,false   ,false  ,false 
};

const eTunerId CBT848Card::m_Tuners_hauppauge[]=
{
    TUNER_ABSENT,                           /* ""                                 */
    TUNER_ABSENT,                           /* "External"                     */
    TUNER_ABSENT,                           /* "Unspecified"                */
    TUNER_PHILIPS_PAL,                  /* "Philips FI1216"           */
    /*4*/   
    TUNER_PHILIPS_SECAM,                /* "Philips FI1216MF"       */
    TUNER_PHILIPS_NTSC,                 /* "Philips FI1236"           */
    TUNER_PHILIPS_PAL_I,                /* "Philips FI1246"         */
    TUNER_PHILIPS_PAL_DK,               /* "Philips FI1256"           */
    /*8*/
    TUNER_PHILIPS_PAL,                  /* "Philips FI1216 MK2"     */
    TUNER_PHILIPS_SECAM,                /* "Philips FI1216MF MK2"   */
    TUNER_PHILIPS_NTSC,                 /* "Philips FI1236 MK2"     */
    TUNER_PHILIPS_PAL_I,                /* "Philips FI1246 MK2"     */
    /*12*/
    TUNER_PHILIPS_PAL_DK,               /* "Philips FI1256 MK2"     */
    TUNER_TEMIC_4032FY5_NTSC,       /* "Temic 4032FY5"            */
    TUNER_TEMIC_4002FH5_PAL,        /* "Temic 4002FH5"          */
    TUNER_TEMIC_4062FY5_PAL_I,  /* "Temic 4062FY5"          */
    /*16*/
    TUNER_PHILIPS_PAL,                  /* "Philips FR1216 MK2"     */
    TUNER_PHILIPS_SECAM,                /* "Philips FR1216MF MK2"   */
    TUNER_PHILIPS_NTSC,                 /* "Philips FR1236 MK2"     */
    TUNER_PHILIPS_PAL_I,                /* "Philips FR1246 MK2"     */
    /*20*/
    TUNER_PHILIPS_PAL_DK,               /* "Philips FR1256 MK2"     */
    TUNER_PHILIPS_PAL,                  /* "Philips FM1216"         */
    TUNER_PHILIPS_SECAM,                /* "Philips FM1216MF"       */
    TUNER_PHILIPS_NTSC,                 /* "Philips FM1236"         */
    /*24*/
    TUNER_PHILIPS_PAL_I,                /* "Philips FM1246"         */
    TUNER_PHILIPS_PAL_DK,               /* "Philips FM1256"           */
    TUNER_TEMIC_4036FY5_NTSC,       /* "Temic 4036FY5"            */
    TUNER_ABSENT,                           /* "Samsung TCPN9082D"      */
    /*28*/
    TUNER_ABSENT,                           /* "Samsung TCPM9092P"      */
    TUNER_TEMIC_4006FH5_PAL,        /* "Temic 4006FH5"          */
    TUNER_ABSENT,                           /* "Samsung TCPN9085D"      */
    TUNER_ABSENT,                           /* "Samsung TCPB9085P"      */
    /*32*/
    TUNER_ABSENT,                           /* "Samsung TCPL9091P"      */
    TUNER_TEMIC_4039FR5_NTSC,       /* "Temic 4039FR5"            */
    TUNER_PHILIPS_MULTI,                /* "Philips FQ1216 ME"      */
    TUNER_TEMIC_4066FY5_PAL_I,  /* "Temic 4066FY5"          */
    /*36*/
    TUNER_ABSENT,                           /* "Philips TD1536"           */
    TUNER_ABSENT,                           /* "Philips TD1536D"        */
    TUNER_PHILIPS_NTSC,                 /* "Philips FMR1236"          */
    TUNER_ABSENT,                           /* "Philips FI1256MP"         */
    /*40*/
    TUNER_ABSENT,                           /* "Samsung TCPQ9091P"      */
    TUNER_TEMIC_4006FN5_PAL,        /* "Temic 4006FN5"            */
    TUNER_TEMIC_4009FR5_PAL,        /* "Temic 4009FR5"            */
    TUNER_TEMIC_4046FM5_MULTI,  /* "Temic 4046FM5"            */
    /*44*/
    TUNER_TEMIC_4009FN5_PAL,      /* "Temic 4009FN5"              */
    TUNER_ABSENT,                           /* "Philips TD1536D_FH_44"*/
    TUNER_LG_R01F_NTSC,                 /* "LG TP18NSR01F"          */
    TUNER_LG_B01D_PAL,                  /* "LG TP18PSB01D"            */
    TUNER_LG_B11D_PAL,                  /* "LG TP18PSB11D"            */    
    TUNER_LG_I001D_PAL_I,               /* "LG TAPC-I001D"          */
    TUNER_LG_I701D_PAL_I                /* "LG TAPC-I701D"          */
};

const eTunerId CBT848Card::m_Tuners_avermedia_0[] = 
{
    TUNER_PHILIPS_NTSC,  
    TUNER_PHILIPS_PAL,
    TUNER_PHILIPS_PAL,   
    TUNER_PHILIPS_PAL,
    TUNER_PHILIPS_PAL,   
    TUNER_PHILIPS_PAL,
    TUNER_PHILIPS_SECAM, 
    TUNER_PHILIPS_SECAM,
    TUNER_PHILIPS_SECAM, 
    TUNER_PHILIPS_PAL
};

/*
const eTunerId CBT848Card::m_Tuners_avermedia_0_fm[] = 
{
    PHILIPS_FR1236_NTSC,  
    PHILIPS_FR1216_PAL,
    PHILIPS_FR1216_PAL,   
    PHILIPS_FR1216_PAL,
    PHILIPS_FR1216_PAL,   
    PHILIPS_FR1216_PAL,
    PHILIPS_FR1236_SECAM, 
    PHILIPS_FR1236_SECAM,
    PHILIPS_FR1236_SECAM, 
    PHILIPS_FR1216_PAL
};
*/

const eTunerId CBT848Card::m_Tuners_avermedia_1[] = 
{
    TUNER_TEMIC_4032FY5_NTSC,  
    TUNER_TEMIC_4002FH5_PAL,    
    TUNER_TEMIC_4002FH5_PAL,   
    TUNER_TEMIC_4002FH5_PAL,
    TUNER_TEMIC_4002FH5_PAL,   
    TUNER_TEMIC_4002FH5_PAL,
    TUNER_TEMIC_4012FY5, 
    TUNER_TEMIC_4012FY5, //TUNER_TEMIC_SECAM
    TUNER_TEMIC_4012FY5, 
    TUNER_TEMIC_4002FH5_PAL
};


eTunerId CBT848Card::AutoDetectTuner(eTVCardId CardId)
{
   eTunerId TunerId=TUNER_ABSENT;
  
    // Maybe for future use
    bool TVTunerDoesFM=false;
    bool HasRemoteControl=false;
    // Not implemented yet
    bool HasTEA5757=false;

    if(m_TVCards[CardId].TunerId == TUNER_USER_SETUP)
    {
        return TUNER_ABSENT;
    }
    else if(m_TVCards[CardId].TunerId == TUNER_AUTODETECT)
    {
        eTunerId Tuner = TUNER_ABSENT;
         /* The same way btwincap 5.3.2 uses to detect the tuners.

           Tested:
            - Livetec:        (only properties) no
            - Miro/Pinnacle:  (full)            no
            - Flyvideo cards: (full)            no
            - Hauppauge:      (full)            Wintv-Go
            - Avermedia:      (full)            no
            - VHX:            (only properties) no
            - Magic TView:    (only properties) no 

            Properties (FM, Remote control) are useless as of now.            
        */   
        switch(CardId)
        {
          case TVCARD_LIFETEC:
            {                
                    DWORD id;                   
                    WriteDword(BT848_GPIO_OUT_EN,( 0x18e0 )&0x00FFFFFFL);
                id = ReadDword(BT848_GPIO_DATA);
                LOG(2, "AutoDetectTuner: Lifetec card. ID: %08x",id);
                    if (id & 0x4000) 
                    {
                        TVTunerDoesFM = true;                  
                    } 
            }
            break;
          case TVCARD_MIRO:
          case TVCARD_MIROPRO:
          case TVCARD_PINNACLERAVE:
          case TVCARD_PINNACLEPRO:            
            {
                //Tuner = (eTunerId)(((ReadWord(BT848_GPIO_DATA)>>10)-1)&7);                            
                DWORD Id;
                DWORD Val;
                
                // Read ID                
                WriteDword(BT848_GPIO_OUT_EN,( 0x0000 )&0x00FFFFFFL);
                Id = ReadDword(BT848_GPIO_DATA);
                //Id = ((Id >> 10) & 31) - 1;            
                Id = ((Id >> 10) & 63) - 1;
                
                LOG(2, "AutoDetectTuner: Miro/Pinnacle card. ID: 0x%08X",Id);
                
                if (Id < 32)
                {
                    // Get tuner from list
                    Tuner = m_Tuners_miro[Id];

                    // Get additional data
                    Val = ReadDword(BT848_GPIO_DATA);
                    
                    LOG(2, "AutoDetectTuner: Miro/Pinnacle card. Val: 0x%08X",Val);
                    
                    if (Val & 0x20) 
                    {
                        TVTunerDoesFM = true;
                        if (m_Tuners_miro_fm[Id]) 
                        {
                            HasTEA5757 = true;
                            TVTunerDoesFM = false;
                        }
                    }
                 }
                 else
                 {
                     Id = 63-Id;
                     TVTunerDoesFM = false;
                     switch (Id)
                     {
                     case 1: //PAL / mono
                         Tuner = TUNER_MT2032_PAL;
                         break;
                     case 2: //PAL+SECAM / stereo
                         Tuner = TUNER_MT2032_PAL;
                         break;
                     case 3: //NTSC / stereo
                         Tuner = TUNER_MT2032;
                         break;
                     case 4: //PAL+SECAM / mono
                         Tuner = TUNER_MT2032_PAL;
                         break;
                     case 5: //NTSC / mono
                         Tuner = TUNER_MT2032;
                         break;
                     case 6: //NTSC / stereo
                         Tuner = TUNER_MT2032;
                         break;
                     default:
                         //unknown, try mt2032
                         Tuner = TUNER_MT2032;                         
                         break;
                     }
                 }
            }
            break;          
          case TVCARD_FLYVIDEO_98:
              case TVCARD_TYPHOON_TVIEW:
              case TVCARD_CHRONOS_VS2:
            case TVCARD_FLYVIDEO_98FM:
              case TVCARD_FLYVIDEO2000:           
            case TVCARD_FLYVIDEO98EZ:       
            {            
                DWORD Out;
                DWORD Gpio;
                DWORD TType;

                Out = ReadDword(BT848_GPIO_OUT_EN)&0x00FFFFFFL;            
                // Set to inputs the appropiate lines 
                WriteDword(BT848_GPIO_OUT_EN,( 0x0000 )&0x00FFFFFFL);                                               
                 // without this we would see the 0x1800 mask               
                ::Sleep(8);
                    Gpio = ReadDword(BT848_GPIO_DATA);              
                WriteDword(BT848_GPIO_OUT_EN,( Out )&0x00FFFFFFL);            
                    // all cards provide GPIO info, some have an additional eeprom

                LOG(2, "AutoDetectTuner: Flyvideo type card. Id: 0x%08X",Gpio);

                // lowest 3 bytes are remote control codes (no handshake needed)
                    TType = (Gpio & 0x0f0000)>>16;
                switch (TType) 
                {
                  case 0x0: 
                    Tuner = TUNER_ABSENT; 
                    break;
                  case 0x4: 
                    Tuner = TUNER_PHILIPS_PAL; 
                    break;
                  case 0x6: 
                    Tuner = TUNER_LG_TAPCH701P_NTSC; 
                    break;
                  case 0xC: 
                    Tuner = TUNER_PHILIPS_SECAM; 
                    break;
                  default:  
                    // Unknown
                    Tuner = TUNER_ABSENT; 
                    break;
                }
                TVTunerDoesFM = (Gpio&0x400000) != 0;
                    HasRemoteControl = (Gpio&0x800000) != 0;
            }  
            break;
          case TVCARD_HAUPPAUGE:
          case TVCARD_HAUPPAUGE878:
            {
                // Read EEPROM
                BYTE Eeprom[256];
                BYTE Out[] = { 0xA0 , 0 };
                Eeprom[0] = 0;
                m_I2CBus->Read(Out,2,Eeprom,256);

                if (Eeprom[0] != 0x84 || Eeprom[2] != 0) 
                {
                        //Hauppage EEPROM invalid
                        LOG(2, "AutoDetectTuner: Hauppage card. EEPROM error");
                    break;
                }
                
                LOG(2, "AutoDetectTuner: Hauppage card. Id: 0x%02X",Eeprom[9]);
                
                if (Eeprom[9] < sizeof(m_Tuners_hauppauge)/sizeof(m_Tuners_hauppauge[0])) 
                {
                  Tuner = m_Tuners_hauppauge[Eeprom[9]];
                }
                
                LOG(2, "AutoDetectTuner: Hauppage card. Block 2: 0x%02X at %d+3",Eeprom[ Eeprom[1]+3 ],Eeprom[1]);
                
                /* Block 2 starts after len+3 bytes header */
                int blk2 = Eeprom[1] + 3;
                int radio = Eeprom[blk2-1] & 0x01;
                    int infrared = Eeprom[blk2-1] & 0x04;
                    TVTunerDoesFM = false;
                    if (radio) 
                    {
                        TVTunerDoesFM = true;
                    }
                    HasRemoteControl = false;
                    if (infrared) 
                    {
                        HasRemoteControl = true;
                    }
            }
            break;                                  
          case TVCARD_AVERMEDIA98:
          case TVCARD_AVPHONE98:
          case TVCARD_AVERMEDIA:
            {
                BYTE Eeprom[256];
                BYTE Out[] = { 0xA0 , 0 };                
                m_I2CBus->Read(Out,2,Eeprom,256);

                BYTE tuner_make;
                BYTE tuner_tv_fm;
                BYTE tuner_format;
                BYTE tuner=0;

                    tuner_make   = (Eeprom[0x41] & 0x7);
                  tuner_tv_fm  = (Eeprom[0x41] & 0x18) >> 3;
                    tuner_format = (Eeprom[0x42] & 0xf0) >> 4;
                    
                    LOG(2, "AutoDetectTuner: Avermedia card. Id: 0x%02X 0x%02X",Eeprom[0x41],Eeprom[0x42]);

                if (tuner_make == 0 || tuner_make == 2) 
                {
                        if (tuner_format <=9) 
                        {
                          Tuner = m_Tuners_avermedia_0[tuner_format];
                        }  
                }
                if (tuner_make == 1) 
                {
                        if (tuner_format <= 9) 
                        {
                          Tuner = m_Tuners_avermedia_1[tuner_format];
                        }   
                }
                // We assume we have a remote control
                    HasRemoteControl = true;
            }
            break;
                  case TVCARD_VHX:            
            {
                TVTunerDoesFM = false;
                    HasTEA5757 = true;
            }
            break;
          case TVCARD_MAGICTVIEW061:
            {
                BYTE Eeprom[256];
                BYTE Out[] = { 0xA0 , 0 };
                m_I2CBus->Read(Out,2,Eeprom,256);

                UINT Id = (UINT(Eeprom[252]) << 24) |
                                (UINT(Eeprom[253]) << 16) |
                                (UINT(Eeprom[254]) << 8)  |
                                (UINT(Eeprom[255]));                

                LOG(2, "AutoDetectTuner: Magic TView card. Id: 0x%08X",Id);
                
                if (Id == 0x4002144f) 
                {
                         TVTunerDoesFM = true;
                }
            }
            break;
          case TVCARD_PXELVWPLTVPRO:
          case TVCARD_WINFAST2000:
            {
                HasRemoteControl = true;
            }
            break;
          default:
            break;
        }
        
        if (HasTEA5757) 
        {
           //not supported yet
           return TUNER_ABSENT;
        }   
        return Tuner;
    }
    else
    {
        return m_TVCards[CardId].TunerId;
    }
}

int CBT848Card::GetNumInputs()
{
    return m_TVCards[m_CardType].NumInputs;
}

BOOL CBT848Card::IsInputATuner(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_TUNER);
}


void CBT848Card::SetVideoSource(int nInput)
{
    // call correct function
    // this funny syntax is the only one that works
    // if you want help understanding what is going on
    // I suggest you read http://www.newty.de/
    (*this.*m_TVCards[m_CardType].pInputSwitchFunction)(nInput);
}

const CBT848Card::TCardType* CBT848Card::GetCardSetup()
{
    return &(m_TVCards[m_CardType]);
}

eTVCardId CBT848Card::AutoDetectCardType()
{
    /*  This is the way btwincap gets the card ID.
        Maybe for future use.
  
        BYTE Eeprom[256];
        BYTE Out[] = { 0xA0 , 0 };
        if (m_I2CBus->Read(Out,2,Eeprom,256)) 
        {
            DWORD Id = (DWORD(Eeprom[252]) << 24) |
                           (DWORD(Eeprom[253]) << 16) |
                               (DWORD(Eeprom[254]) << 8)  |
                               (DWORD(Eeprom[255]));
        }
    */

//    if(I2C_AddDevice(I2C_HAUPEE))
    {
        DWORD Id = m_SubSystemId;
        if (Id != 0 && Id != 0xffffffff)
        {
        int i;
        for (i = 0; m_AutoDectect878[i].ID != 0; i++)
        {
            if (m_AutoDectect878[i].ID  == Id)
            {
                //Try to detect PRO models
                eTVCardId CardID = m_AutoDectect878[i].CardId;
                switch (CardID)
                {
                case TVCARD_MIRO:                
                    if (AutoDetectMSP3400())
                    {
                         CardID = TVCARD_MIROPRO;
                    }
                    break;
                case TVCARD_PINNACLERAVE:
                    if (AutoDetectMSP3400())
                    {
                         CardID = TVCARD_PINNACLEPRO;
                    }
                    break;
                case TVCARD_FLYVIDEO2000:                            
                    //if (TDA9874)
                    //{
                    //     CardID = TVCARD_FLYVIDEO2000S;
                    //}
                    break;
                }
                
                return CardID;
            }
        }
    }
    }
    
    //if(I2C_AddDevice(I2C_STBEE))
    //{
        //return TVCARD_STB;
    //}

    // STB cards have a eeprom @ 0xae (old bt848) 
    BYTE Out[] = { 0xAE , 0 };
    BYTE Val = 0;
        if (m_I2CBus->Read(Out,2,&Val,1)) 
        {
            return TVCARD_STB;
        }
    
    return TVCARD_UNKNOWN;
}

#define I2C_MSP3400C_0 0x80

bool CBT848Card::AutoDetectMSP3400()
{
    BYTE writebf[4];
	BYTE readbf[3];
    
    bool HasMSP34xx = false;

    writebf[0] = I2C_MSP3400C_0; //address
			
    if (m_I2CBus->Read(writebf, 1, readbf, 1))
    {
     	// It´s responding ... Try a better check!
		// Reset MSP!

        static BYTE reset_off[4] = { I2C_MSP3400C_0, 0x00, 0x80, 0x00 };
        static BYTE reset_on[4]  = { I2C_MSP3400C_0, 0x00, 0x00, 0x00 };	
        
        m_I2CBus->Write(reset_off, 4);		
		if (m_I2CBus->Write(reset_on, 4)) 
        {
			// If Reset went ok, Get revision            			
            writebf[1] = 0x12 + 1;       //DFP
			writebf[2] = 0x1e >> 8;
			writebf[3] = 0x1e & 0xff;            
            if (m_I2CBus->Read(writebf, 4, readbf, 2))
            {
				WORD rev1;
                WORD rev2;

                rev1 = ( WORD(readbf[0]) << 8) | readbf[1] ;
				writebf[2] = 0x1f >> 8;
				writebf[3] = 0x1f & 0xff;
				if (m_I2CBus->Read(writebf, 4, readbf, 2))
                {
					rev2 = ( WORD(readbf[0]) << 8) | readbf[1] ;
					if ( (rev1 != rev2) || (rev1 != 0 && rev1 != 0xFFFF) ) 
                    {
						// Revision seems to be OK! ... Accept!
						HasMSP34xx = true;					
					}
				}
			}
		}
	}
    return HasMSP34xx;
}

#undef I2C_MSP3400C_0
        
void CBT848Card::RSBTCardInputSelect(int nInput)
{
    StandardBT848InputSelect(nInput);
    switch(nInput)
    {
    case 3:
        AndOrDataDword(BT848_GPIO_DATA, 0x1000, ~0x1F800);
        ::Sleep(50);
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    case 4:
        AndOrDataDword(BT848_GPIO_DATA, 0x9000, ~0x1F800);
        ::Sleep(50);
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    case 5:
        AndOrDataDword(BT848_GPIO_DATA, 0x11000, ~0x1F800);
        ::Sleep(50);
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    case 6:
        AndOrDataDword(BT848_GPIO_DATA, 0x19000, ~0x1F800);
        ::Sleep(50);
        AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x1000);
        break;
    default:
        break;
    }
}


void CBT848Card::StandardBT848InputSelect(int nInput)
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
    
    AndDataByte(BT848_IFORM, (BYTE)~BT848_IFORM_MUXSEL);

    // set the comp bit for svideo
    switch (m_TVCards[m_CardType].Inputs[nInput].InputType)
    {
    case INPUTTYPE_SVIDEO:
        OrDataByte(BT848_E_CONTROL, BT848_CONTROL_COMP);
        OrDataByte(BT848_O_CONTROL, BT848_CONTROL_COMP);
        break;
    case INPUTTYPE_TUNER:
    case INPUTTYPE_COMPOSITE:
    case INPUTTYPE_CCIR:
    default:
        AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        break;
    }

    DWORD MuxSel = m_TVCards[m_CardType].Inputs[nInput].MuxSelect;
    MaskDataByte(BT848_IFORM, (BYTE) (MuxSel << 5), BT848_IFORM_MUXSEL);
}

void CBT848Card::Sasem4ChannelInputSelect(int nInput)
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

    // these cards always use mux 2
    MaskDataByte(BT848_IFORM, (BYTE) (2 << 5), BT848_IFORM_MUXSEL);

    if(m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_SVIDEO)
    {
        OrDataByte(BT848_E_CONTROL, BT848_CONTROL_COMP);
        OrDataByte(BT848_O_CONTROL, BT848_CONTROL_COMP);
        // Set the Philips TDA8540 4x4 switch matrix for s-video input
        // 0xD2 SW1 choose OUT3=IN3; OUT2=IN1; OUT1=IN0; OUT0=IN2
        // 0x07 GCO choose (0000) gain; (01) clamp; (11) aux    
        // 0x03 OEN choose OUT0 and OUT1 high (s-video)
        CtrlTDA8540(0x90, 0x00, 0xD2, 0x07, 0x03);
    }
    else
    {
        AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        // Set the Philips TDA8540 4x4 switch matrix for composite switch 1 input
        // 0x00 SW1 choose the MuxSelect setting
        // 0x87 GCO choose (1000) gain; (01) clamp; (11) aux    
        // 0x01 OEN choose OUT0 high
        BYTE OutputSelect = m_TVCards[m_CardType].Inputs[nInput].MuxSelect;
        CtrlTDA8540(0x90, 0x00, OutputSelect, 0x87, 0x01);
    }
}

void CBT848Card::Silk200InputSelect(int nInput)
{
    //Is this an analog input?
    if(nInput < 0x4)
    {
        Sasem4ChannelInputSelect (nInput);
    }
    else
    //No, it's an SDI input
    {
        BYTE OutputSelect = m_TVCards[m_CardType].Inputs[nInput].MuxSelect;
        CtrlSilkSDISwitch(0xB0, OutputSelect);
    }
}

void CBT848Card::InitHauppauge()
{
    BootMSP34xx(5);
}

void CBT848Card::InitVoodoo()
{
    BootMSP34xx(20);
}

void  CBT848Card::InitRSBT()
{
    WriteDword(BT848_GPIO_OUT_EN, 0xFFFFFF);
    WriteDword(BT848_GPIO_DATA, 0xFFFFFF);
    ::Sleep(50);
    WriteDword(BT848_GPIO_DATA, 0x000000);
    ::Sleep(50);
    WriteDword(BT848_GPIO_DATA, 0xFFFFFF);
    ::Sleep(50);
    WriteDword(BT848_GPIO_OUT_EN, 0x1F800);
}

void CBT848Card::InitSasem()
{
    // Initialize and set the Philips TDA8540 4x4 switch matrix
    // 0xD2 SW1 choose OUT3=IN3; OUT2=IN1; OUT1=IN0; OUT0=IN2
    // 0x07 GCO choose (0000) gain; (01) clamp; (11) aux    
    // 0x03 OEN choose OUT0 and OUT1 high (i.e., s-video)
    CtrlTDA8540(0x90, 0x00, 0xD2, 0x07, 0x03);
}

// reset/enable the MSP on some Hauppauge cards 
// Thanks to Kyösti Mälkki (kmalkki@cc.hut.fi)! 
void CBT848Card::BootMSP34xx(int pin)
{
    int mask = 1 << pin;
    AndOrDataDword(BT848_GPIO_OUT_EN, mask, ~mask);
    AndOrDataDword(BT848_GPIO_DATA, 0, ~mask);
    ::Sleep(10);
    AndOrDataDword(BT848_GPIO_DATA, mask, ~mask);
}


// ----------------------------------------------------------------------- 
//  Imagenation L-Model PXC200 Framegrabber 
//  This is basically the same procedure as
//  used by Alessandro Rubini in his pxc200
//  driver, but using BTTV functions 

void CBT848Card::InitPXC200()
{
    //I2C_Lock();
    //I2C_Write(0x5E, 0, 0x80, 1);
    const BYTE InitDac[] =
    {
        0x5E, 0, 
        0x80, 1,
    };

    const BYTE InitPic[] =
    {
        0x1E, 0,
        0x08, 0x09, 
        0x0a, 0x0b, 
        0x0d, 0x0d,
        0x01, 0x02, 
        0x03, 0x04, 
        0x05, 0x06,
        0x00
    };

    // Initialise GPIO-connevted stuff 
    WriteWord(BT848_GPIO_OUT_EN, 1<<13); // Reset pin only 
    WriteWord(BT848_GPIO_DATA, 0);
    ::Sleep(30);
    WriteWord(BT848_GPIO_DATA, 1<<13);
    // GPIO inputs are pulled up, so no need to drive
    // reset pin any longer 
    WriteWord(BT848_GPIO_OUT_EN, 0);

    //  we could/should try and reset/control the AD pots? but
    //  right now  we simply  turned off the crushing.  Without
    //  this the AGC drifts drifts
    //  remember the EN is reverse logic -->
    //  setting BT848_ADC_AGC_EN disable the AGC
    //  tboult@eecs.lehigh.edu
    
    WriteByte(BT848_ADC, BT848_ADC_RESERVED | BT848_ADC_AGC_EN);

    //  Initialise MAX517 DAC 
    m_I2CBus->Write(InitDac, sizeof(InitDac));

    //  Initialise 12C508 PIC 
    //  The I2CWrite and I2CRead commmands are actually to the
    //  same chips - but the R/W bit is included in the address
    //  argument so the numbers are different 
    m_I2CBus->Write(InitPic, sizeof(InitPic));
}

// ----------------------------------------------------------------------- 
// TDA8540 Control Code
// Philips composite/s-video 4x4 switch IC
// 19 Jul 2001 Dan Schmelzer
//
// See datasheet at:
// http://www.semiconductors.philips.com/acrobat/datasheets/TDA8540_3.pdf
//
// Slave address byte (SLV) possibilities for the TDA8540 chip
// See page 5 of 2/6/95 datasheet
// 0x90, 0x92, 0x94, 0x96, 0x98, 0x9A, 0x9C
//
// Subcommand byte (SUB) possibilities choosing function
// See page 6 of 2/6/95 datasheet
// 0x00 = Access to switch control (SW1)
// 0x01 = Access to gain/clamp/auxilliary pins control (GCO)
// 0x02 = Access to output enable control (OEN)
// If commands are given in succession, then SUB is automatically incremented
// and the next register is written to
//
// Switch Control Register (SW1)
// See page 6 of 2/6/95 datasheet
// Four output bit pairs P1-4 make up control byte; input chosen for each output
// 00 for Input 0; 01 for Input 1; 10 for Input 2; 11 for input 3
//
// Gain & Clamp Control Register (GCO)
// See page 6 of 2/6/95 datasheet
// MS 4 bits control gain on outputs[3:0] (low is 1x, high is 2x)
// 2 bits control clamp action or mean value on inputs [1:0] ; LS 2 bits
// control value of auxilliary outputs D1, D0
//
// Output Enable Control Register (OEN)
// See page 7 of 2/6/95 datasheet
// MS 4 bits reserved; LS 4 bits controls which output(s) from 3 to 0 are
// active (high)
//
// Upon reset, the outputs are set to active and connected to IN0; the gains
// are set at 2x and inputs IN0 and IN1 are clamped.

void CBT848Card::CtrlTDA8540(BYTE SLV, BYTE SUB, BYTE SW1, BYTE GCO, BYTE OEN)
{
    BYTE Buffer[] = {SLV, SUB, SW1, GCO, OEN};
    m_I2CBus->Write(Buffer, 5);
}

// ----------------------------------------------------------------------- 
// SDI Silk SDI Input Switch
// 21 May 2002 Dan Schmelzer
// 
// Slave addres (SLV) = 0xB0 for the Silk 200; write only
// Input enable (IEN) = from 0 to x
//
// You can never have too many inputs!

void CBT848Card::CtrlSilkSDISwitch(BYTE SLV, BYTE IEN)
{
    BYTE Buffer[] = {SLV, IEN};
    // The switch is flakey sometimes; it will return true, but won't
    // switch it; if you do multiple times in a row, however, it switches
    // fine.  Not sure whether it's DScaler or the switch.  Guessing the
    // switch.
    m_I2CBus->Write(Buffer, 2);
    m_I2CBus->Write(Buffer, 2);
    m_I2CBus->Write(Buffer, 2);
}
