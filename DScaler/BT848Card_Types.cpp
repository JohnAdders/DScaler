/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card_Types.cpp,v 1.4 2001-11-18 02:47:08 ittarnavsky Exp $
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
#include "BT848Card.h"
#include "BT848_Defines.h"

const TCardSetup TVCards[/*TVCARD_LASTONE*/] =
{
    {
        "Unknown Card",
        3, 1, 0, 2, 0,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "MIRO PCTV",
        4, 1, 0, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 2, 0, 0, 0, 10, 0},
        0,
        PLL_NONE,
        TUNER_AUTODETECT,
    },
    {
        "Hauppauge old",
        4, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "STB",
        3, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 4, 0, 2, 3, 1, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Intel",
        3, 1, 0, -1, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Diamond DTV2000",
        3, 1, 0, 2, 3,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 0, 1, 3, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "AVerMedia TVPhone",
        3, 1, 0, 3, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {12, 4, 11, 11, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "MATRIX-Vision MV-Delta",
        5, 1, -1, 3, 0,
        { 2, 3, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    // 0x08 
    {
        "Fly Video II",
        3, 1, 0, 2, 0xc00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0xc00, 0x800, 0x400, 0xc00, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "TurboTV",
        3, 1, 0, 2, 3,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 1, 1, 2, 3, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Standard BT878",
        4, 1, 0, 2, 7,
        { 2, 0, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "MIRO PCTV pro",
        3, 1, 0, 2, 65551,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {1, 65537, 0, 0, 10, 0},
        0,
        PLL_NONE,
        TUNER_AUTODETECT,
    },
    {
        "ADS Technologies Channel Surfer TV",
        3, 1, 2, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 13, 14, 11, 7, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "AVerMedia TVCapture 98",
        3, 4, 0, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 13, 14, 11, 7, 0, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Aimslab VHX",
        3, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Zoltrix TV-Max",
        3, 1, 0, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {0, 0, 1 , 0, 10, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    // 0x10 
    {
        "Pixelview PlayTV (bt878)",
        3, 1, 0, 2, 0x01fe00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x01c000, 0, 0x018000, 0x014000, 0x002000, 0 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Leadtek WinView 601",
        3, 1, 0, 2, 0x8300f8,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x4fa007,0xcfa007,0xcfa007,0xcfa007,0xcfa007,0xcfa007},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "AVEC Intercapture",
        3, 2, 0, 2, 0,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "LifeView FlyKit w/o Tuner",
        3, 1, -1, -1, 0x8dff00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "CEI Raffles Card",
        3, 3, 0, 2, 0,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0 ,0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Lucky Star Image World ConferenceTV",
        3, 1, 0, 2, 0x00fffe07,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 131072, 1, 1638400, 3, 4, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL_I,
    },
    {
        "Phoebe Tv Master + FM",
        3, 1, 0, 2, 0xc00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {0, 1, 0x800, 0x400, 0xc00, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Modular Technology MM205 PCTV, bt878",
        2, 1, 0, -1, 7,
        { 2, 3 , 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    // 0x18 
    {
        "Askey/Typhoon/Anubis Magic TView CPH051/061 (bt878)",
        3, 1, 0, 2, 0xe00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {0x400, 0x400, 0x400, 0x400, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Terratec/Vobis TV-Boostar",
        3, 1, 0, 2, 16777215,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {131072, 1, 1638400, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Newer Hauppauge WinCam (bt878)",
        4, 1, 0, 3, 7,
        { 2, 0, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "MAXI TV Video PCI2",
        3, 1, 0, 2, 0xffff,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 0xc00, 0},
        0,
        PLL_NONE,
        TUNER_PHILIPS_SECAM,
    },
    {
        "Terratec TerraTV+",
        3, 1, 0, 2, 0x70000,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x20000, 0x30000, 0x00000, 0x10000, 0x40000, 0x00000},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Imagenation PXC200",
        5, 1, -1, 4, 0,
        { 2, 3, 1, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "FlyVideo 98",
        3, 1, 0, 2, 0x8dfe00,
        {2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0x8dff00, 0x8df700, 0x8de700, 0x8dff00, 0 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "iProTV",
        3, 1, 0, 2, 1,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 1, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    // 0x20 
    {
        "Intel Create and Share PCI",
        4, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 4, 4, 4, 4, 4, 4},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Terratec TerraTValue",
        3, 1, 0, 2, 0xffff00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x500, 0, 0x300, 0x900, 0x900, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Leadtek WinFast 2000",
        3, 1, 0, 2, 0xfff000,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x621000,0x620100,0x621100,0x620000,0xE210000,0x620000},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Chronos Video Shuttle II",
        3, 3, 0, 2, 0x1800,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0x1000, 0x1000, 0x0800, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Typhoon TView TV/FM Tuner",
        3, 3, 0, 2, 0x1800,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0x800, 0, 0, 0x1800, 0 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "PixelView PlayTV pro",
        3, 1, 0, 2, 0xff,
        { 2, 3, 1, 1, 0, 0, 0, 0 },
        { 0x21, 0x20, 0x24, 0x2c, 0x29, 0x29 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "TView99 CPH063",
        3, 1, 0, 2, 0x551e00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x551400, 0x551200, 0, 0, 0, 0x551200 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Pinnacle PCTV Rave",
        3, 1, 0, 2, 0x03000F,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 2, 0, 0, 0, 1, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    // 0x28 
    {
        "STB2",
        3, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 4, 0, 2, 3, 1, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "AVerMedia TVPhone 98",
        3, 4, 0, 2, 4,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 13, 14, 11, 7, 0, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
    },
    {
        "ProVideo PV951", // pic16c54 
        3, 1, 0, 2, 0,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL_I,
    },
    {
        "Little OnAir TV",
        3, 1, 0, 2, 0xe00b,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {0xff9ff6, 0xff9ff6, 0xff1ff7, 0, 0xff3ffc, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "Sigma TVII-FM",
        2, 1, 0, -1, 3,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {1, 1, 0, 2, 3, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
    },
    {
        "MATRIX-Vision MV-Delta 2",
        5, 1, -1, 3, 0,
        { 2, 3, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Zoltrix Genie TV",
        3, 1, 0, 2, 0xbcf03f,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0xbc803f, 0, 0xbcb03f, 0, 0xbcb03f, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
    },
    {
        "Terratec TV/Radio+", // Radio ?? 
        3, 1, 0, 2, 0x1f0000,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0xe2ffff, 0, 0, 0, 0xe0ffff, 0xe2ffff },
        0,
        PLL_35,
        TUNER_PHILIPS_PAL_I,
    },
    // 0x30 
    {
        "Dynalink Magic TView",
        3, 1, 0, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {2, 0, 0, 0, 1, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    // MAE 20 Nov 2000 Start of change
    {
        "Conexant Bt878 NTSC XEVK",
        3, 1, 0, 2, 0xFFFEFF,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x001000, 0x001000, 0x000000, 0x000000, 0x003000, 0x000000},
        0,
        PLL_NONE,
        TUNER_PHILIPS_NTSC,
    },
    {
        "Rockwell Bt878 NTSC XEVK",
        3, 1, 0, 2, 0xFFFEFF,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x001000, 0x001000, 0x000000, 0x000000, 0x003000, 0x000000},
        0,
        PLL_NONE,
        TUNER_PHILIPS_NTSC,
    },
    // MAE 20 Nov 2000 End of change
    // MAE 5 Dec 2000 Start of change
    {
        "Conexant Foghorn NTSC/ATSC-A",
        3, 1, 0, 2, 0xFF00F8,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
        0,
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
    },
    {
        "Conexant Foghorn NTSC/ATSC-B",
        3, 1, 0, 2, 0xFF00F8,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
        0,
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
    },
    {
        "Conexant Foghorn NTSC/ATSC-C",
        3, 1, 0, 2, 0xFF00F8,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
        0,
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
    },
    // MAE 5 Dec 2000 End of change
    {
        "RS BT Card",
        3, 4, 0, 2, 0x1f800,
        { 0x010002, 0x010003, 0x010001, 0x00000000, 0x00000000, 0x00000000, 
            0x010000, 0x090000, 0x110000, 0x190000},
        { 13, 14, 11, 7, 0, 0 },
        0x1f800,
        PLL_28,
        TUNER_ABSENT,
    },
    {
        "Cybermail AV",
        3, 1, -1, 2, 0xFFFEFF,
        { 2, 3, 1, 2, 0, 0, 0, 0},
        { 0x001000, 0x001000, 0x000000, 0x000000, 0x003000, 0x000000},
        0,
        PLL_NONE,
        TUNER_ABSENT,
    },
    {
        "Viewcast Osprey",
        3, 1, -1, 1, 0,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_ABSENT,
    },
    {
        // Lukas Gebauer <geby@volny.cz>
        "ATI TV-Wonder",
        3, 1, 0, 2, 0xf03f,
        // TODO: check this 3.0.0 { 2, 3, 0, 1, 0, 0, 0, 0},
        { 2, 3, 1, 0, 0, 0, 0, 0},
        { 0xbffe, 0, 0xbfff, 0, 0xbffe, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        // Lukas Gebauer <geby@volny.cz>
        "ATI TV-Wonder VE",
        2, 1, 0, -1, 1,
        { 2, 3, 0, 1, 0, 0, 0, 0},
        { 0, 0, 1, 0, 0, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
     },
     {
        "GV-BCTV3",
        3, 1, 0, 2, 0x010f00,
        {2, 3, 0, 0},
        {0x10000, 0, 0x10000, 0, 0, 0},
        0,
        PLL_28,
        TUNER_ALPS_TSCH6_NTSC,
    },
    {
        "Prolink PV-BT878P+4E (PixelView PlayTV PAK)",
        4, 1, 0, 2, 0xAA0000,
        { 2, 3, 1, 1 },
        { 0x20000, 0, 0x80000, 0x80000, 0xa8000, 0x46000  },
        0,
        PLL_28,
        TUNER_PHILIPS_PAL_I,
    },
    {
        "Eagle Wireless Capricorn2 (bt878A)",
        4, 1, 0, 2, 7,
        { 2, 0, 1, 1},
        { 0, 1, 2, 3, 4},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        // David Härdeman <david@2gen.com>
        "Pinnacle PCTV Studio Pro",
        3,1,0,2,0x03000F,
        { 2, 3, 1, 1},
        { 1, 0x10001, 0, 0, 10},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        // Claas Langbehn <claas@bigfoot.com>,
        //   Sven Grothklags <sven@upb.de>
        "Typhoon TView RDS / FM Stereo",
        3, 3, 0, 2, 0x1c,
        { 2, 3, 1, 1},
        { 0, 0, 0x10, 8, 4 },
        0,
        PLL_28,
        TUNER_PHILIPS_PAL_I,
    },
    {
        // Tim Röstermundt <rosterm@uni-muenster.de>
        //   in de.comp.os.unix.linux.hardware:
        //  options bttv card=0 pll=1 radio=1 gpiomask=0x18e0
        //  audiomux=0x44c71f,0x44d71f,0,0x44d71f,0x44dfff
        //  options tuner Type=5 
        "Lifetec LT 9415 TV",
        4,1,0,2,0x18e0,
        { 2, 3, 1, 1},
        { 0x0000,0x0800,0x1000,0x1000,0x18e0 },
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
    },
    {
        // Miguel Angel Alvarez <maacruz@navegalia.com>
        //   old Easy TV BT848 version (model CPH031) 
        "BESTBUY Easy TV",
        4,1,0,2,0xF,
        { 2, 3, 1, 0},
        { 2, 0, 0, 0, 10},
        0,
        PLL_28,
        TUNER_TEMIC_4002FH5_PAL,
    },
    {
        "FlyVideo '98/FM / 2000S",
        3,3,0,2,0x18e0,
        { 2, 3, 0, 1},
        { 0,0x18e0,0x1000,0x1000,0x1080, 0x1080 },
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
    },
    {
        // Steve Hosgood <steve@equiinet.com>
        "GrandTec 'Grand Video Capture'",
        2, 0, -1, 1, 0,
        { 3, 1 },
        { 0 },
        0,
        PLL_35,
        TUNER_ABSENT,
    },
    {
        // Daniel Herrington <daniel.herrington@home.com>
        "Phoebe TV Master Only (No FM)",
        3,1,0,2,0x0e00,
        { 2, 3, 1, 1},
        { 0x400, 0x400, 0x400, 0x400, 0x800, 0x400 },
        0,
        PLL_NONE,
        TUNER_TEMIC_4036FY5_NTSC,
    },
    {
        // Matti Mottus <mottus@physic.ut.ee>
        "TV Capturer",
        4, 1, 0, 2, 0x03000F,
        { 2, 3, 1, 0},
        { 2,0,0,0,1 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        // Philip Blundell <philb@gnu.org>
        "MM100PCTV",
        2, 2, 0, -1, 11,
        { 2, 3, 1, 1},
        { 2, 0, 0, 1, 8},
        0,
        PLL_NONE,
        TUNER_TEMIC_4002FH5_PAL,
    },
    {
        // Adrian Cox <adrian@humboldt.co.uk>
        "AG Electronics GMV1",
        2,0,-1,1, 0xF,
        { 2, 2},
        {0, 0, 0, 0, 0, 0 },
        0,
        PLL_28,
        TUNER_ABSENT,
    },
    {
        // Miguel Angel Alvarez <maacruz@navegalia.com>
        // new Easy TV BT878 version (model CPH061) 
        // special thanks to Informatica Mieres for providing the card 
        "BESTBUY Easy TV (bt878)",
        3, 2, 0, 2, 0xFF,
        { 2, 3, 1, 0},
        { 1, 0, 4, 4, 9},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
    },
    // 24 Jul 2001 Dan Schmelzer
    // Sasem 4-Channel Developer Card (s-video jumper chosen)
    // TODO:  Add audio input support
    //        Add GPIO support
	//        Add composite jumpered card
    {
        "Sasem 4-Channel Dev Board (S-Video Jumper)",
        1, 1, -1, 0, 0,
        { 2, 2, 1, 2},
        { 0},
        0,
        PLL_NONE,
        TUNER_ABSENT,
    },
    {
        "Pinnacle PCTV Sat",
        2, 1, -1, 2, 0x03000F,
        { 0, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 1, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    // 17 Sep 2001 Dax Sieger
    // AIMS Labs VideoHighway Extreme Card (older - original version)
    // TODO:  ADD FM Radio Support
	{
        "Aimslab VideoHighway Extreme (not 98)",
        3, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 2, 1, 3, 4, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    // 3dfx VoodooTV 200 (USA) / FM (Europa)
	{
        "3dfx VoodooTV 200 (USA)", // szName
        4, // nVideoInputs
        1, // nAudioInputs
        0, // TunerInput
        -1, // SVideoInput
        0x4f8a00, // GPIOMask
        { 2, 3, 0, 1,}, // MuxSelect
        { 0x957fff, 0x997fff, 0x957fff, 0x957fff}, // AudioMuxSelect
        0, // GPIOMuxMask
        PLL_28, // ePLLFreq
        TUNER_MT2032, // eTunerID
    },
    // 3dfx VoodooTV FM (Europa)
	{
        "3dfx VoodooTV FM (Europa)", // szName
        4, // nVideoInputs
        1, // nAudioInputs
        0, // TunerInput
        -1, // SVideoInput
        0x4f8a00, // GPIOMask
        { 2, 3, 0, 1,}, // MuxSelect
        { 0x947fff, 0x987fff, 0x947fff, 0x947fff}, // AudioMuxSelect
        0, // GPIOMuxMask
        PLL_28, // ePLLFreq
        TUNER_MT2032, // eTunerID
    },
    // use this card so that the MSP chip is not reset on startup
    // should let Dscaler work as it did in 2.3.0
    {
        "Standard BT878 (No Init MSP)",
        4, 1, 0, 2, 7,
        { 2, 0, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
    },
    {
        "Terratec TValueRadio",                 // szName
        3,                                      // nVideoInputs
        1,                                      // nAudioInputs
        0,                                      // TunerInput
        2,                                      // SVideoInput
        0xffff00,                               // GPIOMask
        { 2, 3, 1, 1},                          // MuxSelect
        { 0x500, 0x500, 0x300, 0x900, 0x900},   // AudioMuxSelect
        0,                                      // GPIOMuxMask
        PLL_28,                                 // ePPLFreq
        TUNER_PHILIPS_PAL,                      // eTunerID
    },
    {
        "Flyvideo 98EZ (capture only)", // szName
        4,                              // nVideoInputs
        0,                              // nAudioInputs
        -1,                             // TunerInput
        2,                              // SVideoInput
        0,                              // GPIOMask
        { 2, 3, 1, 1},                  // MuxSelect: AV1, AV2, SVHS, CVid adapter on SVHS
        { 0 },                          // AudioMuxSelect
        0,                              // GPIOMuxMask
        PLL_28,                         // ePPLFreq
        TUNER_ABSENT,                   // eTunerID
    },
    {
        /* Philip Blundell <pb@nexus.co.uk> */
        "Active Imaging AIMMS", // szName
        1,                      // nVideoInputs
        0,                      // nAudioInputs
        -1,                     // TunerInput
        0,                      // SVideoInput
        0,                      // GPIOMask
        { 2},                   // MuxSelect
        { 0},                   // AudioMuxSelect
        0,                      // GPIOMuxMask
        PLL_28,                 // ePPLFreq
        TUNER_ABSENT,           // eTunerID
    },
    {
        /* DeeJay <deejay@westel900.net (2000S) */
        "FlyVideo 2000S",                               // szName
        3,                                              // nVideoInputs
        3,                                              // nAudioInputs
        0,                                              // TunerInput
        2,                                              // SVideoInput
        0x18e0,                                         // GPIOMask
        { 2, 3, 0, 1},                                  // MuxSelect
        { 0, 0x18e0, 0x1000, 0x1000, 0x1080, 0x1080},   // AudioMuxSelect
        0,                                              // GPIOMuxMask
        PLL_28,                                         // ePPLFreq
        TUNER_PHILIPS_PAL,                              // eTunerID
    },
    {
        /* TANAKA Kei <peg00625@nifty.com> */
        "GV-BCTV4/PCI",                     // szName
        3,                                  // nVideoInputs
        1,                                  // nAudioInputs
        0,                                  // TunerInput
        2,                                  // SVideoInput
        0x010f00,                           // GPIOMask
        { 2, 3, 0, 0},                      // MuxSelect
        { 0x10000, 0, 0x10000, 0, 0, 0},    // AudioMuxSelect
        0,                                  // GPIOMuxMask
        PLL_28,                             // ePPLFreq
        TUNER_SHARP_2U5JF5540_NTSC,         // eTunerID
    },
    {
        "Prolink PV-BT878P+4E / PixelView PlayTV PAK / Lenco MXTV-9578 CP", // szName
        4,                                                                  // nVideoInputs
        1,                                                                  // nAudioInputs
        0,                                                                  // TunerInput
        2,                                                                  // SVideoInput
        0xAA0000,                                                           // GPIOMask
        { 2, 3, 1, 1},                                                      // MuxSelect
        { 0x20000, 0, 0x80000, 0x80000, 0xa8000, 0x46000},                  // AudioMuxSelect
        0,                                                                  // GPIOMuxMask
        PLL_28,                                                             // ePPLFreq
        TUNER_PHILIPS_PAL_I,                                                // eTunerID
    },
    {
        /* Claas Langbehn <claas@bigfoot.com>,
           Sven Grothklags <sven@upb.de> */
        "Typhoon TView RDS + FM Stereo / KNC1 TV Station RDS",  // szName
        3,                                                      // nVideoInputs
        3,                                                      // nAudioInputs
        0,                                                      // TunerInput
        2,                                                      // SVideoInput
        0x1c,                                                   // GPIOMask
        { 2, 3, 1, 1},                                          // MuxSelect
        { 0, 0, 0x10, 8, 4},                                    // AudioMuxSelect
        0,                                                      // GPIOMuxMask
        PLL_28,                                                 // ePPLFreq
        TUNER_PHILIPS_PAL_I,                                    // eTunerID
    },
};

static const TAutoDectect878 AutoDectect878[] =
{
    // There are taken from bttv vesrion 7.68
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

    // below are additional cards that we have information about
    { 0x14610002, TVCARD_AVERMEDIA98,   "Avermedia TVCapture 98" },
    // MAE 20 Nov 2000 Start of change
    { 0x182214F1, TVCARD_CONEXANTNTSCXEVK,  "Conexant Bt878A NTSC XEVK" },
    { 0x1322127A, TVCARD_ROCKWELLNTSCXEVK,  "Rockwell Bt878A NTSC XEVK" },
    // MAE 20 Nov 2000 End of change
    // MAE 5 Dec 2000 Start of change
    { 0x013214F1, TVCARD_CONEXANTFOGHORNREVA,  "Conexant Foghorn NTSC/ATSC-A" },
    { 0x023214F1, TVCARD_CONEXANTFOGHORNREVB,  "Conexant Foghorn NTSC/ATSC-B" },
    { 0x033214F1, TVCARD_CONEXANTFOGHORNREVC,  "Conexant Foghorn NTSC/ATSC-C" },
    // MAE 5 Dec 2000 End of change
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
    { 0, (eTVCardId)-1, NULL }
};


eTunerId CBT848Card::AutoDetectTuner(eTVCardId CardId)
{
    eTunerId Tuner = TUNER_ABSENT;
    switch(CardId)
    {
    case TVCARD_MIRO:
    case TVCARD_MIROPRO:
        Tuner = (eTunerId)(((ReadWord(BT848_GPIO_DATA)>>10)-1)&7);
        break;
    default:
        if (TVCards[CardId].TunerId > TUNER_ABSENT && TVCards[CardId].TunerId < TUNER_LASTONE)
        {
            Tuner = TVCards[CardId].TunerId;
        }
        break;
    }
    return Tuner;
}

const TCardSetup* CBT848Card::GetCardSetup(eCardType BtCardType)
{
    return &(TVCards[BtCardType]);
}

eTVCardId CBT848Card::AutoDetectCardType()
{
    if(I2C_AddDevice(I2C_HAUPEE))
    {
        DWORD Id = m_SubSystemId;
        if (Id != 0 && Id != 0xffffffff)
        {
            int i;
            for (i = 0; AutoDectect878[i].ID != 0; i++)
            {
                if (AutoDectect878[i].ID  == Id)
                {
                    return AutoDectect878[i].CardId;
                }
            }
        }
    }
    if(I2C_AddDevice(I2C_STBEE))
    {
        return TVCARD_STB;
    }
    return TVCARD_UNKNOWN;
}

// do any specific card related initilaisation
void CBT848Card::CardSpecificInit(eTVCardId CardType)
{
    switch(CardType)
    {
    case TVCARD_HAUPPAUGE:
    case TVCARD_HAUPPAUGE878:
        HauppaugeBootMSP34xx(5);
        break;
    case TVCARD_VOODOOTV_200:
    case TVCARD_VOODOOTV_FM:
        HauppaugeBootMSP34xx(20);
        break;
    case TVCARD_PXC200:
        InitPXC200();
        break;
    case TVCARD_SASEM4CHNLSVID:
        // Initialize and set the Philips TDA8540 4x4 switch matrix for s-video
        // 0xD2 SW1 choose OUT3=IN3; OUT2=IN1; OUT1=IN0; OUT0=IN2
        // 0x07 GCO choose (0000) gain; (01) clamp; (11) aux    
        // 0x03 OEN choose OUT0 and OUT1 high (i.e., s-video)
        CtrlTDA8540(0x90, 0x00, 0xD2, 0x07, 0x03);
        break;
    default:
        break;
    }
}

// reset/enable the MSP on some Hauppauge cards 
// Thanks to Kyösti Mälkki (kmalkki@cc.hut.fi)! 
void CBT848Card::HauppaugeBootMSP34xx(int pin)
{
    int mask = 1 << pin;
    AndOrDataDword(BT848_GPIO_OUT_EN, mask, ~mask);
    AndOrDataDword(BT848_GPIO_DATA, 0, ~mask);
    Sleep(10);
    AndOrDataDword(BT848_GPIO_DATA, mask, ~mask);
}


// ----------------------------------------------------------------------- 
//  Imagenation L-Model PXC200 Framegrabber 
//  This is basically the same procedure as
//  used by Alessandro Rubini in his pxc200
//  driver, but using BTTV functions 

void CBT848Card::InitPXC200()
{
    const BYTE vals[] =
    {
        0x08, 0x09, 0x0a, 0x0b, 0x0d, 0x0d,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x00
    };
    int i;

    // Initialise GPIO-connevted stuff 
    WriteWord(BT848_GPIO_OUT_EN, 1<<13); // Reset pin only 
    WriteWord(BT848_GPIO_DATA, 0);
    Sleep(30);
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
    I2C_Lock();
    I2C_Write(0x5E, 0, 0x80, 1);

    //  Initialise 12C508 PIC 
    //  The I2CWrite and I2CRead commmands are actually to the
    //  same chips - but the R/W bit is included in the address
    //  argument so the numbers are different 
    for (i = 0; i < sizeof(vals)/sizeof(int); i++)
    {
        I2C_Write(0x1E, vals[i], 0, 1);
        I2C_Read(0x1F);
    }
    I2C_Unlock();
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

void CBT848Card::CtrlTDA8540(int SLV, int SUB, int SW1, int GCO, int OEN)
{
    I2C_Lock();
    I2C_Start();
    I2C_SendByte(SLV, 5); 
    I2C_SendByte(SUB, 0); 
    I2C_SendByte(SW1, 0); 
    I2C_SendByte(GCO, 0); 
    I2C_SendByte(OEN, 0); 
    I2C_Stop();
    I2C_Unlock();
}
