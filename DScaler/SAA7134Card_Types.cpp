/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card_Types.cpp,v 1.49 2004-06-21 06:08:59 atnak Exp $
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
// Revision 1.48  2004/05/11 08:11:52  atnak
// added new card "Genius Video Wonder PRO III"
//
// Revision 1.47  2004/05/07 17:24:24  atnak
// added new card "Dazzle My TV"
//
// Revision 1.46  2004/03/26 14:17:52  atnak
// Tuner Changes
//
// Revision 1.45  2004/03/11 03:48:32  atnak
// Updated Tevion MD 9717
//
// Revision 1.44  2004/02/24 05:35:18  atnak
// Renamed Much TV Plus
//
// Revision 1.43  2004/02/24 05:02:19  atnak
// Renamed Much TV Plus IT005
//
// Revision 1.42  2004/02/24 04:18:42  atnak
// Added new card Manli M-TV005
//
// Revision 1.41  2004/02/18 06:39:47  atnak
// Changed Setup Card / Tuner so that only cards of the same device are
// shown in the card list.
// Added new card Chronos Video Shuttle II (saa7134 version)
//
// Revision 1.40  2004/02/17 07:25:42  atnak
// Updated Compro VideoMate TV Gold Plus
//
// Revision 1.39  2004/02/17 06:29:14  atnak
// Added new card Elitegroup EZ-TV
// Added new card ST Lab PCI-TV7130
// Added new card Lifeview FlyTV Platinum
// Added new card Compro VideoMate TV Gold Plus (saa7134)
//
// Revision 1.38  2004/02/15 04:07:54  atnak
// Added new card FlyVideo FlyView 3100
// Added new card Pinnacle PCTV Stereo
// Added new card AverMedia AverTV Studio 305
// Added new card Chronos Video Shuttle II FM
//
// Revision 1.37  2004/02/14 04:33:48  atnak
// Updated card Medion MD-2819 PC-TV-radio card
//
// Revision 1.36  2004/02/14 04:03:44  atnak
// Put GPIO settings and AutoDetect IDs into the main card definition
// to remove the need for extra tables and custom functions.
// Added card Medion Philips 7134 Chipset
//
// Revision 1.35  2003/10/27 10:39:53  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.34  2003/08/23 10:00:33  atnak
// Added a missing break in a switch statement
//
// Revision 1.33  2003/07/31 05:01:38  atnak
// Added ASK Select AS-VCV300/PCI card
//
// Revision 1.32  2003/06/27 08:05:41  atnak
// Added AOPEN VA1000 Lite2
//
// Revision 1.31  2003/04/28 06:28:05  atnak
// Added ASUS TV/FM
//
// Revision 1.30  2003/04/17 09:17:46  atnak
// Added V-Gear MyTV SAP PK
//
// Revision 1.29  2003/04/16 15:11:50  atnak
// Fixed Medion TV-Tuner 7134 MK2/3 audio clock and default tuner
//
// Revision 1.28  2003/04/16 14:43:16  atnak
// Updated FlyVideo2000 radio GPIO setting
//
// Revision 1.27  2003/02/14 09:10:40  atnak
// Updated Manli M-TV002
//
// Revision 1.26  2003/02/14 08:59:15  atnak
// Updated Manli M-TV002
//
// Revision 1.25  2003/02/12 22:09:46  atnak
// Added M-TV002
//
// Revision 1.24  2003/02/06 21:30:44  ittarnavsky
// changes to support primetv 7133
//
// Revision 1.23  2003/02/03 07:00:52  atnak
// Added Typhoon TV-Radio 90031
//
// Revision 1.22  2003/01/30 07:19:47  ittarnavsky
// fixed the autodetect
//
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


/**
 * @file SAA7134Card.cpp CSAA7134Card Implementation (Types)
 */

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
        0x0000,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_PROTEUSPRO - Proteus Pro [philips reference design]
    {
        "Proteus Pro [philips reference design]",
        0x7134,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x20011131,
    },
    // LifeView FlyVideo 3000
    // Chronos Video Shuttle II (Based on FlyVideo 3000, Stereo)
    // Thanks "Velizar Velinov" <veli_velinov2001@ya...>
    {
        "LifeView FlyVideo3000 / Chronos Video Shuttle II",
        0x7134,
        6,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
                //0xE000, 0x8000,
                0xE000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
                //0xE000, 0x0000,
                0xE000, 0x2000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0xE000, 0x8000,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        0x0018e700,
        NULL,
        StandardSAA7134InputSelect,
        0x01384e42,
    },
    // LifeView FlyVideo2000 (saa7130)
    // Chronos Video Shuttle II (Based on FlyVideo 2000)
    {
        "LifeView FlyVideo2000 / Chronos Video Shuttle II",
        0x7130,
        6,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x2000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0xE000, 0x8000,
            },
        },
        // Some variations of this card use differnet tuners.  E.g. Temic PAL
        TUNER_LG_TAPCNEW_PAL,
        AUDIOCRYSTAL_NONE,
        0x0018e700,
        NULL,
        StandardSAA7134InputSelect,
        0x01385168,
    },
    // SAA7134CARDID_EMPRESS - EMPRESS (has TS, i2srate=48000, has CCIR656 video out)
    {
        "EMPRESS",
        0x7134,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x67521131,
    },
    // SAA7134CARDID_MONSTERTV - SKNet Monster TV
    {
        "SKNet Monster TV",
        0x7134,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x4E851131,
    },
    // SAA7134CARDID_TEVIONMD9717 - Tevion MD 9717
    {
        "Tevion MD 9717",
        0x7134,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_KNC1RDS - KNC One TV-Station RDS
    {
        "KNC One TV-Station RDS",
        0x7134,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_CINERGY400 - Terratec Cinergy 400 TV
    {
        "Terratec Cinergy 400 TV",
        0x7134,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x1142153B,
    },
    // SAA7134CARDID_MEDION5044 - Medion 5044
    {
        "Medion 5044",
        0x7134,
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
                0x6000, 0x4000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0x6000, 0x0000,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
                0x6000, 0x0000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0x6000, 0x0000,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
                0x6000, 0x0000,
            },
        },
        TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_24576Hz,
        0x00006000,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_KWTV713XRF - KWORLD KW-TV713XRF (saa7130)
    // Thanks "b" <b@ki...>
    // this card probably needs GPIO changes but I don't know what they are
   {
        "KWORLD KW-TV713XRF / KUROUTO SHIKOU",
        0x7130,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_MANLIMTV001 - Manli M-TV001 (saa7130)
    // Thanks "Bedo" Bedo@dscaler.forums
    {
        "Manli M-TV001",
        0x7130,
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
                0x6000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
                0x6000, 0x0000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
                0x6000, 0x0000,
            },
        },
        TUNER_LG_B11D_PAL,  // Should be LG TPI8PSB12P PAL B/G
        AUDIOCRYSTAL_NONE,
        0x00006000,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_PRIMETV7133 - PrimeTV 7133 (saa7133)
    // Thanks "Shin'ya Yamaguchi" <yamaguchi@no...>
    {
        "PrimeTV 7133",
        0x7133,
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x2000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x4000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0xE000, 0x8000,
            },
        },
        TUNER_PHILIPS_FI1286_NTSC_M_J,  // Should be TCL2002NJ or Philips FI1286 (NTSC M-J)
        AUDIOCRYSTAL_24576Hz,
        0x0018e700,
        NULL,
        StandardSAA7134InputSelect,
        0x01385168,
    },
    // SAA7134CARDID_CINERGY600 - Terratec Cinergy 600 TV
    // Thanks "Michel de Glace" <mglace@my...>
    {
        "Terratec Cinergy 600 TV",
        0x7134,
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
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x1143153b,
    },
    // SAA7134CARDID_MEDION7134 - Medion TV-Tuner 7134 MK2/3
    // Thanks "DavidbowiE" Guest@dscaler.forums
    // Thanks "Josef Schneider" <josef@ne...>
    {
        "Medion TV-Tuner 7134 MK2/3",
        0x7134,
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
        TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_32110Hz,
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x000316be,
    },
    // SAA7134CARDID_TYPHOON90031 - Typhoon TV+Radio (Art.Nr. 90031)
    // Thanks "Tom Zoerner" <tomzo@ne...>
    {
        "Typhoon TV-Radio 90031",
        0x7134,
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
                AUDIOINPUTSOURCE_LINE1,  // MUTE, card has no audio in
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,  // MUTE, card has no audio in
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
        0,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_MANLIMTV002 - Manli M-TV002 (saa7130)
    // Thanks "Patrik Gloncak" <gloncak@ho...>
    {
        "Manli M-TV002",
        0x7130,
        4,
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
        TUNER_LG_B11D_PAL,  // Should be LG TPI8PSB02P PAL B/G
        AUDIOCRYSTAL_NONE,
        0,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_VGEAR_MYTV_SAP - V-Gear MyTV SAP PK
    // Thanks "Ken Chung" <kenchunghk2000@ya...>
    {
        "V-Gear MyTV SAP PK",
        0x7134,
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
                0x4400, 0x0400,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0x4400, 0x0400,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
                0x4400, 0x0400,
            },
        },
        TUNER_PHILIPS_PAL_I,
        AUDIOCRYSTAL_32110Hz,
        0x00004400,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_ASUS_TVFM - ASUS TV/FM
    // Thanks "Wolfgang Scholz" <wolfgang.scholz@ka...>
    {
        "ASUS TV/FM",
        0x7134,
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
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
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
        AUDIOCRYSTAL_32110Hz,
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x48421043,
    },
    // SAA7134CARDID_AOPEN_VA1000_L2 - Aopen VA1000 Lite2 (saa7130)
    // Thanks "stu" <ausstu@ho...>
    {
        "Aopen VA1000 Lite2",
        0x7130,
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
                0x40, 0x70,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
                0x20, 0x70,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
                0x20, 0x70,
            },
        },
        TUNER_LG_TAPCNEW_PAL,
        AUDIOCRYSTAL_NONE,
        0x00000060,
        NULL,
        StandardSAA7134InputSelect,
    },
    // SAA7134CARDID_ASK_ASVCV300_PCI (saa7130)
    // Thanks "Tetsuya Takahashi" <tetsu_64k@zer...>
    //  - may have Videoport
    //  - may have Transport Stream
    {
        "ASK SELECT AS-VCV300/PCI",
        0x7130,
        2,
        {
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
            },
        },
        TUNER_ABSENT,
        AUDIOCRYSTAL_NONE,
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x226e1048,
    },
    // Medion MD-2819 PC-TV-radio card
    // Thanks "Sanel.B" <vlasenica@ya...>
    // Thanks "Mc" <michel.heusinkveld2@wa...>
    // Thanks "Ing. Arno Pucher" <eolruin@ch...>
    {
        "Medion MD-2819 PC-TV-radio card",
        0x7134,
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
                0x00040007, 0x00000006,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0x00040007, 0x00000006,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE1,
                0x00040007, 0x00000006,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE1,
                0x00040007, 0x00000005,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0x00040007, 0x00000004,
            },
        },
        TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_32110Hz,
        0x00040007,
        NULL,
        StandardSAA7134InputSelect,
        0xa70b1461,
    },
    // FlyVideo FlyView 3100 (NTSC Version - United States)
    // Thanks "Ryan N. Datsko" <MysticWhiteDragon@ho...>
    // SAA7133 -- not supported
    {
        "FlyVideo FlyView 3100 (no audio)",
        0x7133,
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x4000,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x0000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0xE000, 0x8000,
            },
        },
        TUNER_PHILIPS_NTSC,
        AUDIOCRYSTAL_NONE,
        0x018e700,
        NULL,
        StandardSAA7134InputSelect,
        0x01385168,
    },
    // Pinnacle PCTV Stereo
    // Thanks "Fabio Maione" <maione@ma...>
    // Thanks "Dr. Uwe Zettl" <uwe.zettl@t...>
    // Thanks "Aristarco" <aristarco@ar...>
    // I2S audio may need to be enabled for this card to work.
    {
        "Pinnacle PCTV Stereo",
        0x7134,
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_DAC,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_MT2050_PAL,
        AUDIOCRYSTAL_32110Hz,
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x002b11bd,
    },
    // AverMedia AverTV Studio 305
    // Thanks "Oeoeeia Aieodee" <sid16@ya...>
    {
        "AverMedia AverTV Studio 305",
        0x7130,
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
                0x00040007, 0x00000005,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0x00040007, 0x00000006,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE1,
                0x00040007, 0x00000006,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE1,
                0x00040007, 0x00000005,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_DAC,
                0x00040007, 0x00000004,
            },
        },
        TUNER_PHILIPS_FM1216ME_MK3,
        AUDIOCRYSTAL_NONE,
        0x00040007,
        NULL,
        StandardSAA7134InputSelect,
        0x21151461,
    },
    // Elitegroup EZ-TV
    // Thanks "Arturo Garcia" <argabulk@ho...>
    // + Card "Grandmars PV951P4TF" is same except ID and Tuner chip(?)
    //   Thanks Kwok Kelvin <kelvin002@ho...>
    {
        "Elitegroup EZ-TV",
        0x7134,
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
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_32110Hz,
        0,
        NULL,
        StandardSAA7134InputSelect,
        0x4cb41019,
    },
    // ST Lab PCI-TV7130
    // Thanks "Aidan Gill" <schmookoo@ho...>
    {
        "ST Lab PCI-TV7130",
        0x7130,
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
                0x7000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0x7000, 0x2000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE1,
                0x7000, 0x2000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_DAC,
                0x7000, 0x3000,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_NONE,
        0x00007000,
        NULL,
        StandardSAA7134InputSelect,
        0x20011131,
    },
    // Lifeview FlyTV Platinum
    // Thanks "Chousw" <chousw@ms...>
    // SAA7133 -- not supported
    {
        "Lifeview FlyTV Platinum (no audio)",
        0x7133,
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x4000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0xE000, 0x8000,
            },
        },
        TUNER_PHILIPS_NTSC,
        AUDIOCRYSTAL_NONE,
        0x018e700,
        NULL,
        StandardSAA7134InputSelect,
        0x02145168,
    },
    // Compro VideoMate TV Gold Plus
    // Thanks "Stephen McCormick" <sdmcc@pa...>
    {
        "Compro VideoMate TV Gold Plus",
        0x7134,
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
                0x1ce780, 0x008080,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0x1ce780, 0x008080,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE1,
                0x1ce780, 0x008080,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0x1ce780, 0x0c8000,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_32110Hz,
        0x001ce780,
        NULL,
        StandardSAA7134InputSelect,
        0xc200185b,
    },
/*    // Chronos Video Shuttle II Stereo
    // Thanks "Velizar Velinov" <veli_velinov2001@ya...>
    // Maybe exactly the same as FlyVideo 3000
    {
        "Chronos Video Shuttle II Stereo",
        0x7134,
        6,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_DAC,
                0xE000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Composite over S-Video",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x2000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0xE000, 0x8000,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_24576Hz,
        0x0000e000,
        NULL,
        StandardSAA7134InputSelect,
        0x01384e42,
    },*/
    // Much TV Plus IT005
    // Thanks "Norman Jonas" <normanjonas@ar...>
    {
        "Much TV Plus",
        0x7134,
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_DAC,
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
        TUNER_LG_B11D_PAL,  // Should be LG TPI8PSB02P PAL B/G
        AUDIOCRYSTAL_32110Hz,
        0,
        NULL,
        StandardSAA7134InputSelect,
    },
    // Dazzle My TV
    // Thanks <rockmong@ho...>
    // Looks like a FlyVideo 2000 clone (from Korean)
    {
        "Dazzle My TV",
        0x7130,
        3,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE1,
                0xE000, 0x4000,
            },
        },
        TUNER_PHILIPS_NTSC,
        AUDIOCRYSTAL_NONE,
        0x0018e700,
        NULL,
        StandardSAA7134InputSelect,
        0x01384e42,
    },
    // Genius Video Wonder PRO III
    // Thanks Michal Kueera <michalk@my...>
    // Looks like another FlyVideo 3000 clone (from Czech Republic)
    // Notice this card is a SAA7134 but the DAC appears not used
    {
        "Genius Video Wonder PRO III",
        0x7134,
        4,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_NONE,
                0xE000, 0x8000,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_32110Hz,
        0x0018e700,
        NULL,
        StandardSAA7134InputSelect,
        0x01385168,
    },
    // V-Gear MyTV2 Radio (saa7130)
    // Thanks "Daniel Kutin" <daniel.kutin@os...>
    // Another card by vendor 0x0138.
    {
        "V-Gear MyTV2 Radio",
        0x7130,
        5,
        {
            {
                "Tuner",
                INPUTTYPE_TUNER,
                VIDEOINPUTSOURCE_PIN1,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x0000,
            },
            {
                "Composite",
                INPUTTYPE_COMPOSITE,
                VIDEOINPUTSOURCE_PIN3,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "S-Video",
                INPUTTYPE_SVIDEO,
                VIDEOINPUTSOURCE_PIN0,          // (Might req mode 6)
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x4000,
            },
            {
                "Radio",
                INPUTTYPE_RADIO,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_LINE2,
                0xE000, 0x2000,
            },
            {
                NULL,
                INPUTTYPE_FINAL,
                VIDEOINPUTSOURCE_NONE,
                AUDIOINPUTSOURCE_DAC,
                0xE000, 0x8000,
            },
        },
        TUNER_PHILIPS_PAL,
        AUDIOCRYSTAL_NONE,
        0x0000e000,
        NULL,
        StandardSAA7134InputSelect,
        0x013819d0,
    },
};


//
// Notes:
//
// "Might req mode 6": S-Video is listed with VIDEOINPUTSOURCE_PIN0 but what
// is actually used is not mode 0 but mode 8.  --This is due to an old design
// decision that I no longer remember why.  Mode 6 is exactly the same as mode
// 8 except the C-channel gain control is set with a register instead of
// automatic gain control that is linked to the Y-channel. "Might req mode 6"
// has been placed beside entries where the RegSpy dump showed the
// SAA7134_ANALOG_IN_CTRL1 register with xxxx0110(6) instead of xxxx1000(8).
//

//
// LifeView Clones:  (Actually, I don't know who supplies who)
//
//              0x7130                      0x7134                          0x7133
//
// 0x01384e42   Dazzle My TV                LifeView FlyVideo3000
//                                          Chronos Video Shuttle II
//
// 0x01385168   LifeView FlyVideo2000       Genius Video Wonder PRO III     PrimeTV 7133
//              Chronos Video Shuttle II
//
// 0x013819d0   V-Gear MyTV2 Radio
//
//
// Notes:
// - The auto detect ID 0x01384e42 is not used by LifeView FlyVideo3000 that I know
//   of.  This ID comes from the SAA7134 version of Chronos Video Shuttle II.
// - All cards above have an identical video input pin configuration.  They also use
//   the same 0x0018e700 GPIO mask.
//


int CSAA7134Card::GetMaxCards()
{
    return sizeof(m_SAA7134Cards)/sizeof(TCardType);
}


CSAA7134Card::eSAA7134CardId CSAA7134Card::AutoDetectCardType()
{
    WORD DeviceId           = GetDeviceId();
    DWORD SubSystemId        = GetSubSystemId();

    if (SubSystemId == 0x00001131)
    {
       LOG(0, "SAA713x: Autodetect found [0x00001131] *Unknown Card*.");
        return SAA7134CARDID_UNKNOWN;
    }

    int ListSize = GetMaxCards();

    for (int i = 0; i < ListSize; i++)
    {
        if (m_SAA7134Cards[i].DeviceId == DeviceId &&
            m_SAA7134Cards[i].dwAutoDetectId == SubSystemId)
        {
            LOG(0, "SAA713x: Autodetect found %s.", m_SAA7134Cards[i].szName);
            return (eSAA7134CardId)i;
        }
    }

    LOG(0, "SAA713x: Autodetect found an unknown card with the following");
    LOG(0, "SAA713x: properties.  Please email the author and quote the");
    LOG(0, "SAA713x: following numbers, as well as which card you have,");
    LOG(0, "SAA713x: so it can be added to the list:");
    LOG(0, "SAA713x: DeviceId: 0x%04x, AutoDetectId: 0x%08x",
        DeviceId, SubSystemId);

    return SAA7134CARDID_UNKNOWN;
}


eTunerId CSAA7134Card::AutoDetectTuner(eSAA7134CardId CardId)
{
    return m_SAA7134Cards[CardId].TunerId;
}


int CSAA7134Card::GetNumInputs()
{
    int iInvalidInputs = 0;

    for (int i = 0; i < m_SAA7134Cards[m_CardType].NumInputs; i++)
    {
        if (m_SAA7134Cards[m_CardType].Inputs[i].szName == NULL)
        {
            iInvalidInputs++;
        }
    }

    return m_SAA7134Cards[m_CardType].NumInputs - iInvalidInputs;
}


LPCSTR CSAA7134Card::GetInputName(int nInput)
{
    if(nInput < m_SAA7134Cards[m_CardType].NumInputs && nInput >= 0)
    {
        if (m_SAA7134Cards[m_CardType].Inputs[nInput].szName == NULL)
        {
            return "";
        }
        return m_SAA7134Cards[m_CardType].Inputs[nInput].szName;
    }
    return "Error";
}


int CSAA7134Card::GetFinalInputNumber()
{
    for (int i = 0; i < m_SAA7134Cards[m_CardType].NumInputs; i++)
    {
        if (m_SAA7134Cards[m_CardType].Inputs[i].InputType == INPUTTYPE_FINAL)
        {
            return i;
        }
    }

    return -1;
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


WORD CSAA7134Card::GetCardDeviceId(eSAA7134CardId CardId)
{
    return m_SAA7134Cards[CardId].DeviceId;
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


/*
void CSAA7134Card::ManliMTV002CardInputSelect(int nInput)
{
    StandardSAA7134InputSelect(nInput);
    /*
    switch(nInput)
    {
    case 0: // Tuner
    case 1: // Composite
    case 2: // S-Video
    case -1: // Ending cleanup
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x8000, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x8000, 0x8000);
        break;
    case 3: // Radio
        // Unverified GPIO setup
        MaskDataDword(SAA7134_GPIO_GPMODE, 0x8000, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS, 0x0000, 0x8000);
        break;
    default:
        break;
    }
    */
/*}
*/


void CSAA7134Card::StandardSAA7134InputSelect(int nInput)
{
    eVideoInputSource VideoInput;

    // -1 for finishing clean up
    if (nInput == -1)
    {
        nInput = GetFinalInputNumber();
        if (nInput == -1)
        {
            // There are no cleanup specific changes
            return;
        }
    }

    if (nInput >= m_SAA7134Cards[m_CardType].NumInputs)
    {
        LOG(1, "Input Select Called for invalid input");
        nInput = m_SAA7134Cards[m_CardType].NumInputs - 1;
    }
    if (nInput < 0)
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

    // GPIO settings
    if (m_SAA7134Cards[m_CardType].dwGPIOMode != 0)
    {
        MaskDataDword(SAA7134_GPIO_GPMODE, m_SAA7134Cards[m_CardType].dwGPIOMode, 0x0EFFFFFF);
        MaskDataDword(SAA7134_GPIO_GPSTATUS,
            m_SAA7134Cards[m_CardType].Inputs[nInput].dwGPIOStatusBits,
            m_SAA7134Cards[m_CardType].Inputs[nInput].dwGPIOStatusMask);
    }
}

