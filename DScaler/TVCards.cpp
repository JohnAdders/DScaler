/////////////////////////////////////////////////////////////////////////////
// $Id: TVCards.cpp,v 1.19 2001-07-16 18:07:50 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// The structures where taken from bttv driver version 7.37
// bttv - Bt848 frame grabber driver
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// (c) 1999,2000 Gerd Knorr <kraxel@goldbach.in-berlin.de>
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
// Change Log
//
// Date          Developer                Changes
//
// 15 Aug 2000   John Adcock             Added structures from bttv
// 20 Nov 2000   Michael Eskin, Conexant Added support for Conexant and Rockwell Bt878XEVKs
//  5 Dec 2000   Michael Eskin, Conexant Added support for Conexant Foghorn ATSC reference designs 
//                                       and PHILIPS 1236D tuner
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.18  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.17  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "tvcards.h"
#include "bt848.h"
#include "i2c.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "Audio.h"

eTVCardId CardType = TVCARD_UNKNOWN;
eTunerId TunerType = TUNER_ABSENT;
long ProcessorSpeed = 1;
long TradeOff = 1;

void hauppauge_boot_msp34xx();
void init_PXC200();

void GVBCTV3PCI_SetAudio(int StereoMode);
void LT9415_SetAudio(int StereoMode);
void TERRATV_SetAudio(int StereoMode);
void AVER_TVPHONE_SetAudio(int StereoMode);
void WINFAST2000_SetAudio(int StereoMode);

const TCardSetup TVCards[TVCARD_LASTONE] =
{
    {
        "Unknown Card",
        3, 1, 0, 2, 0,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "MIRO PCTV",
        4, 1, 0, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 2, 0, 0, 0, 10, 0},
        0,
        PLL_NONE,
        TUNER_AUTODETECT,
        NULL,
    },
    {
        "Hauppauge old",
        4, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "STB",
        3, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 4, 0, 2, 3, 1, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Intel",
        3, 1, 0, -1, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Diamond DTV2000",
        3, 1, 0, 2, 3,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 0, 1, 3, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "AVerMedia TVPhone",
        3, 1, 0, 3, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {12, 4, 11, 11, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        AVER_TVPHONE_SetAudio,
    },
    {
        "MATRIX-Vision MV-Delta",
        5, 1, -1, 3, 0,
        { 2, 3, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
    },
    {
        "TurboTV",
        3, 1, 0, 2, 3,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 1, 1, 2, 3, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Standard BT878",
        4, 1, 0, 2, 7,
        { 2, 0, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "MIRO PCTV pro",
        3, 1, 0, 2, 65551,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {1, 65537, 0, 0, 10, 0},
        0,
        PLL_NONE,
        TUNER_AUTODETECT,
        NULL,
    },
    {
        "ADS Technologies Channel Surfer TV",
        3, 1, 2, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 13, 14, 11, 7, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "AVerMedia TVCapture 98",
        3, 4, 0, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 13, 14, 11, 7, 0, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Aimslab VHX",
        3, 1, 0, 2, 7,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Zoltrix TV-Max",
        3, 1, 0, 2, 15,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {0, 0, 1 , 0, 10, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
    },
    {
        "Leadtek WinView 601",
        3, 1, 0, 2, 0x8300f8,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x4fa007,0xcfa007,0xcfa007,0xcfa007,0xcfa007,0xcfa007},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "AVEC Intercapture",
        3, 2, 0, 2, 0,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "LifeView FlyKit w/o Tuner",
        3, 1, -1, -1, 0x8dff00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },

    {
        "CEI Raffles Card",
        3, 3, 0, 2, 0,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {0, 0, 0, 0 ,0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Lucky Star Image World ConferenceTV",
        3, 1, 0, 2, 0x00fffe07,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 131072, 1, 1638400, 3, 4, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        NULL,
    },
    {
        "Phoebe Tv Master + FM",
        3, 1, 0, 2, 0xc00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {0, 1, 0x800, 0x400, 0xc00, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Modular Technology MM205 PCTV, bt878",
        2, 1, 0, -1, 7,
        { 2, 3 , 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
    },
    {
        "Terratec/Vobis TV-Boostar",
        3, 1, 0, 2, 16777215,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        {131072, 1, 1638400, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Newer Hauppauge WinCam (bt878)",
        4, 1, 0, 3, 7,
        { 2, 0, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 4, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "MAXI TV Video PCI2",
        3, 1, 0, 2, 0xffff,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 1, 2, 3, 0xc00, 0},
        0,
        PLL_NONE,
        TUNER_PHILIPS_SECAM,
        NULL,
    },
    {
        "Terratec TerraTV+",
        3, 1, 0, 2, 0x70000,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x20000, 0x30000, 0x00000, 0x10000, 0x40000, 0x00000},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        TERRATV_SetAudio,
    },
    {
        "Imagenation PXC200",
        5, 1, -1, 4, 0,
        { 2, 3, 1, 0, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "FlyVideo 98",
        3, 1, 0, 2, 0x8dfe00,
        {2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0x8dff00, 0x8df700, 0x8de700, 0x8dff00, 0 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "iProTV",
        3, 1, 0, 2, 1,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 1, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
    },
    {
        "Terratec TerraTValue",
        3, 1, 0, 2, 0xffff00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x500, 0, 0x300, 0x900, 0x900, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Leadtek WinFast 2000",
        3, 1, 0, 2, 0xfff000,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x621000,0x620100,0x621100,0x620000,0xE210000,0x620000},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        WINFAST2000_SetAudio,
    },
    {
        "Chronos Video Shuttle II",
        3, 3, 0, 2, 0x1800,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0x1000, 0x1000, 0x0800, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Typhoon TView TV/FM Tuner",
        3, 3, 0, 2, 0x1800,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0x800, 0, 0, 0x1800, 0 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "PixelView PlayTV pro",
        3, 1, 0, 2, 0xff,
        { 2, 3, 1, 1, 0, 0, 0, 0 },
        { 0x21, 0x20, 0x24, 0x2c, 0x29, 0x29 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "TView99 CPH063",
        3, 1, 0, 2, 0x551e00,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x551400, 0x551200, 0, 0, 0, 0x551200 },
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Pinnacle PCTV Rave",
        3, 1, 0, 2, 0x03000F,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 2, 0, 0, 0, 1, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
    },
    {
        "AVerMedia TVPhone 98",
        3, 4, 0, 2, 4,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 13, 14, 11, 7, 0, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
        NULL,
    },
    {
        "ProVideo PV951", // pic16c54 
        3, 1, 0, 2, 0,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        NULL,
    },
    {
        "Little OnAir TV",
        3, 1, 0, 2, 0xe00b,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {0xff9ff6, 0xff9ff6, 0xff1ff7, 0, 0xff3ffc, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Sigma TVII-FM",
        2, 1, 0, -1, 3,
        {2, 3, 1, 1, 0, 0, 0, 0},
        {1, 1, 0, 2, 3, 0},
        0,
        PLL_NONE,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "MATRIX-Vision MV-Delta 2",
        5, 1, -1, 3, 0,
        { 2, 3, 1, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
    },
    {
        "Zoltrix Genie TV",
        3, 1, 0, 2, 0xbcf03f,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0xbc803f, 0, 0xbcb03f, 0, 0xbcb03f, 0},
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
        NULL,
    },
    {
        "Terratec TV/Radio+", // Radio ?? 
        3, 1, 0, 2, 0x1f0000,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0xe2ffff, 0, 0, 0, 0xe0ffff, 0xe2ffff },
        0,
        PLL_35,
        TUNER_PHILIPS_PAL_I,
        NULL,
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
        NULL,
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
        NULL,
    },
    {
        "Rockwell Bt878 NTSC XEVK",
        3, 1, 0, 2, 0xFFFEFF,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x001000, 0x001000, 0x000000, 0x000000, 0x003000, 0x000000},
        0,
        PLL_NONE,
        TUNER_PHILIPS_NTSC,
        NULL,
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
        NULL,
    },
    {
        "Conexant Foghorn NTSC/ATSC-B",
        3, 1, 0, 2, 0xFF00F8,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
        0,
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
        NULL,
    },
    {
        "Conexant Foghorn NTSC/ATSC-C",
        3, 1, 0, 2, 0xFF00F8,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
        0,
        PLL_NONE,
        TUNER_PHILIPS_1236D_NTSC_INPUT1,
        NULL,
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
        NULL,
    },
    {
        "Cybermail AV",
        3, 1, -1, 2, 0xFFFEFF,
        { 2, 3, 1, 2, 0, 0, 0, 0},
        { 0x001000, 0x001000, 0x000000, 0x000000, 0x003000, 0x000000},
        0,
        PLL_NONE,
        TUNER_ABSENT,
        NULL,
    },
    {
        "Viewcast Osprey",
        3, 1, -1, 1, 0,
        { 2, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 0, 0},
        0,
        PLL_NONE,
        TUNER_ABSENT,
        NULL,
    },
    {
        // Lukas Gebauer <geby@volny.cz>
        "ATI TV-Wonder",
        3, 1, 0, 2, 0xf03f,
        { 2, 3, 0, 1, 0, 0, 0, 0},
        { 0xbffe, 0, 0xbfff, 0, 0xbffe, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
     },
     {
        "GV-BCTV3",
        3, 1, 0, 2, 0x010f00,
        {2, 3, 0, 0},
        {0x10000, 0, 0x10000, 0, 0, 0},
        0,
        PLL_28,
        TUNER_ALPS_TSCH6_NTSC,
        GVBCTV3PCI_SetAudio,
    },
    {
        "Prolink PV-BT878P+4E (PixelView PlayTV PAK)",
        4, 1, 0, 2, 0xAA0000,
        { 2, 3, 1, 1 },
        { 0x20000, 0, 0x80000, 0x80000, 0xa8000, 0x46000  },
        0,
        PLL_28,
        TUNER_PHILIPS_PAL_I,
        NULL,
    },
    {
        "Eagle Wireless Capricorn2 (bt878A)",
        4, 1, 0, 2, 7,
        { 2, 0, 1, 1},
        { 0, 1, 2, 3, 4},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
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
        NULL,
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
        LT9415_SetAudio,
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
        NULL,
    },
    {
        "FlyVideo '98/FM / 2000S",
        3,3,0,2,0x18e0,
        { 2, 3, 0, 1},
        { 0,0x18e0,0x1000,0x1000,0x1080, 0x1080 },
        0,
        PLL_28,
        TUNER_PHILIPS_PAL,
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
    },
};

const TAutoDectect878 AutoDectect878[] =
{
    // There are taken from bttv vesrion 7.68
    { 0x00011002, TVCARD_ATI_TVWONDER,  "ATI TV Wonder" },
    { 0x00011461, TVCARD_AVPHONE98,     "AVerMedia TVPhone98" },
    { 0x00021461, TVCARD_AVERMEDIA98,   "Avermedia TVCapture 98" },
    { 0x00031002, TVCARD_ATI_TVWONDERVE,"ATI TV Wonder/VE" },
    { 0x00031461, TVCARD_AVPHONE98,     "AVerMedia TVPhone98" },
    { 0x00041461, TVCARD_AVERMEDIA98,   "AVerMedia TVCapture 98" },
    { 0x001211bd, TVCARD_PINNACLERAVE,  "Pinnacle PCTV" },
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
    { 0x405010fc, TVCARD_GVBCTV3PCI,    "I-O Data Co. GV-BCV4/PCI" },
    { 0x45000070, TVCARD_HAUPPAUGE878,  "Hauppauge WinTV/PVR" },
    { 0x217d6606, TVCARD_WINFAST2000,   "Leadtek WinFast TV 2000" },
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
    { 0, (eTVCardId)-1, NULL }
};

const TTunerSetup Tuners[TUNER_LASTONE] =
{
    // TUNER_ABSENT
    { 
        "NoTuner", NOMFTR, NOTTYPE,     
        0, 0, 0x00, 0x00, 0x00, 0x00, 0x00 
    },
    // TUNER_PHILIPS_PAL_I
    { 
        "PHILIPS PAL_I", PHILIPS, PAL_I, 
         16*140.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_PHILIPS_NTSC
    { 
        "PHILIPS NTSC", PHILIPS, NTSC, 
        16*157.25, 16*451.25, 0xA0, 0x90, 0x30, 0x8e, 732
    },
    // TUNER_PHILIPS_SECAM
    { 
        "PHILIPS SECAM", PHILIPS, SECAM, 
        16*168.25, 16*447.25, 0xA7, 0x97, 0x37, 0x8e, 623
    },
    // TUNER_PHILIPSFY5_PAL     
    { 
        "PHILIPS PAL", PHILIPS, PAL, 
        16*168.25, 16*447.25, 0xA0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_TEMIC_4002FH5_PAL
    { 
        "Temic 4002 FH5 PAL", TEMIC, PAL, 
        16*140.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 623
    },
    // TUNER_TEMIC_4032FY5_NTSC
    {
        "Temic 4036 FY5 NTSC", TEMIC, NTSC, 
        16*157.25, 16*463.25, 0x02, 0x04, 0x01, 0x8e, 732
    },
    // TUNER_TEMIC_4062FY5_PAL_I
    {
        "Temic PAL_I (4062 FY5)", TEMIC, PAL_I, 
        16*170.00, 16*450.00, 0x02, 0x04, 0x01, 0x8e, 623
    },
    // TUNER_TEMIC_4036FY5_NTSC
    {
        "Temic 4036 FY5 NTSC", TEMIC, NTSC, 
        16*157.25, 16*463.25, 0xa0, 0x90, 0x30, 0x8e, 732
    },
    // TUNER_ALPS_TSBH1_NTSC    
    {
        "ALPS HSBH1", TEMIC, NTSC, 
        16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732
    },
    // TUNER_ALPS_TSBE1_PAL     
    {
        "ALPS TSBE1", TEMIC, PAL, 
        16*137.25, 16*385.25, 0x01, 0x02, 0x08, 0x8e, 732
    },
    // TUNER_ALPS_TSBB5_PAL_I
    {
        "ALPS TSBB5", ALPS, PAL_I, 
        16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 632
    },
    // TUNER_ALPS_TSBE5_PAL 
    {
        "ALPS TSBE5", ALPS, PAL, 
        16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 622
    },
    // TUNER_ALPS_TSBC5_PAL
    {
        "ALPS TSBC5", ALPS, PAL, 
        16*133.25, 16*351.25, 0x01, 0x02, 0x08, 0x8e, 608
    },
    // TUNER_TEMIC_4006FH5_PAL
    {
        "Temic 4006FH5", TEMIC, PAL_I, 
        16*170.00,16*450.00, 0xa0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_PHILIPS_1236D_NTSC_INPUT1
    {
        "PHILIPS 1236D ATSC/NTSC Input 1", PHILIPS, NTSC, 
        2516, 7220, 0xA3, 0x93, 0x33, 0xCE, 732
    },
    // TUNER_PHILIPS_1236D_NTSC_INPUT2
    {
        "PHILIPS 1236D ATSC/NTSC Input 2", PHILIPS, NTSC, 
        2516, 7220, 0xA2, 0x92, 0x32, 0xCE, 732
    },
    // TUNER_ALPS_TSCH6_NTSC
    {
        "ALPS TSCH6",ALPS,NTSC,
        16*137.25, 16*385.25, 0x14, 0x12, 0x11, 0x8e, 732
    },
    // TUNER_TEMIC_4016FY5_PAL
    {
        "Temic PAL_DK (4016 FY5)",TEMIC,PAL,
        16*136.25, 16*456.25, 0xa0, 0x90, 0x30, 0x8e, 623
    },
    // TUNER_PHILIPS_MK2_NTSC
    {
        "PHILIPS NTSC_M (MK2)",PHILIPS,NTSC,
        16*160.00,16*454.00,0xa0,0x90,0x30,0x8e,732
    },
    // TUNER_TEMIC_4066FY5_PAL_I
    {
        "Temic PAL_I (4066 FY5)", TEMIC, PAL_I,
        16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4006FN5_PAL
    {
        "Temic PAL* auto (4006 FN5)", TEMIC, PAL,
        16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4009FR5_PAL
    { 
        "Temic PAL (4009 FR5)", TEMIC, PAL,
        16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4039FR5_NTSC
    {
        "Temic NTSC (4039 FR5)", TEMIC, NTSC,
        16*158.00, 16*453.00, 0xa0,0x90,0x30,0x8e,732
    },
    // TUNER_TEMIC_4046FM5_MULTI
    { 
        "Temic PAL/SECAM multi (4046 FM5)", TEMIC, PAL,
        16*169.00, 16*454.00, 0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_PHILIPS_PAL_DK
    { 
        "PHILIPS PAL_DK", PHILIPS, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_PHILIPS_MULTI
    { 
        "PHILIPS PAL/SECAM multi (FQ1216ME)", PHILIPS, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_I001D_PAL_I
    { 
        "LG PAL_I+FM (TAPC-I001D)", LGINNOTEK, PAL_I,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_I701D_PAL_I
    { 
        "LG PAL_I (TAPC-I701D)", LGINNOTEK, PAL_I,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_R01F_NTSC
    { 
        "LG NTSC+FM (TPI8NSR01F)", LGINNOTEK, NTSC,
        16*210.00,16*497.00,0xa0,0x90,0x30,0x8e,732
    },
    // TUNER_LG_B01D_PAL
    { 
        "LG PAL_BG+FM (TPI8PSB01D)", LGINNOTEK, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_LG_B11D_PAL
    { 
        "LG PAL_BG (TPI8PSB11D)", LGINNOTEK, PAL,
        16*170.00,16*450.00,0xa0,0x90,0x30,0x8e,623
    },
    // TUNER_TEMIC_4009FN5_PAL
    { 
        "Temic PAL* auto + FM (4009 FN5)", TEMIC, PAL,
        16*141.00, 16*464.00, 0xa0,0x90,0x30,0x8e,623
    }
};

// do any specific card related initilaisation
void Card_Init()
{
    switch(CardType)
    {
    case TVCARD_HAUPPAUGE:
    case TVCARD_HAUPPAUGE878:
        //hauppauge_readee(btv,eeprom_data,0xa0);
        //hauppauge_eeprom(btv);
        hauppauge_boot_msp34xx();
        break;
    case TVCARD_PXC200:
        init_PXC200();
        break;
    default:
        break;
    }
}

// reset/enable the MSP on some Hauppauge cards 
// Thanks to Kyösti Mälkki (kmalkki@cc.hut.fi)! 
void hauppauge_boot_msp34xx()
{
    // reset/enable the MSP on some Hauppauge cards 
    // Thanks to Kyösti Mälkki (kmalkki@cc.hut.fi)! 
    BT848_AndOrDataDword(BT848_GPIO_DATA, 32, ~32);
    BT848_AndOrDataDword(BT848_GPIO_DATA, 0, ~32);
    Sleep(10);
    BT848_AndOrDataDword(BT848_GPIO_DATA, 32, ~32);
}


// ----------------------------------------------------------------------- 
//  Imagenation L-Model PXC200 Framegrabber 
//  This is basically the same procedure as
//  used by Alessandro Rubini in his pxc200
//  driver, but using BTTV functions 

void init_PXC200()
{
    const BYTE vals[] =
    {
        0x08, 0x09, 0x0a, 0x0b, 0x0d, 0x0d,
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x00
    };
    int i;

    // Initialise GPIO-connevted stuff 
    BT848_WriteWord(BT848_GPIO_OUT_EN, 1<<13); // Reset pin only 
    BT848_WriteWord(BT848_GPIO_DATA, 0);
    Sleep(30);
    BT848_WriteWord(BT848_GPIO_DATA, 1<<13);
    // GPIO inputs are pulled up, so no need to drive
    // reset pin any longer 
    BT848_WriteWord(BT848_GPIO_OUT_EN, 0);

    //  we could/should try and reset/control the AD pots? but
    //  right now  we simply  turned off the crushing.  Without
    //  this the AGC drifts drifts
    //  remember the EN is reverse logic -->
    //  setting BT848_ADC_AGC_EN disable the AGC
    //  tboult@eecs.lehigh.edu
    
    BT848_WriteByte(BT848_ADC, BT848_ADC_RESERVED|BT848_ADC_AGC_EN);

    //  Initialise MAX517 DAC 
    I2CBus_Lock();
    I2CBus_Write(0x5E, 0, 0x80, 1);

    //  Initialise 12C508 PIC 
    //  The I2CWrite and I2CRead commmands are actually to the
    //  same chips - but the R/W bit is included in the address
    //  argument so the numbers are different 
    for (i = 0; i < sizeof(vals)/sizeof(int); i++)
    {
        I2CBus_Write(0x1E, vals[i], 0, 1);
        I2CBus_Read(0x1F);
    }
    I2CBus_Unlock();
}

void GVBCTV3PCI_SetAudio(int StereoMode)
{
    BT848_OrDataDword(BT848_GPIO_DATA, 0x300);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x200, ~0x300);
        break;
    case VIDEO_SOUND_LANG2:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x300, ~0x300);
        break;
    default:
    case VIDEO_SOUND_LANG1:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x000, ~0x300);
        break;
    }
}

void LT9415_SetAudio(int StereoMode)
{
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x0880, ~0x0880);
        break;
    case VIDEO_SOUND_LANG2:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x0080, ~0x0880);
        break;
    default:
    case VIDEO_SOUND_LANG1:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x0000, ~0x0880);
        break;
    }
}

void TERRATV_SetAudio(int StereoMode)
{
    BT848_OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x180000, ~0x180000);
        break;
    case VIDEO_SOUND_LANG2:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x080000, ~0x180000);
        break;
    default:
    case VIDEO_SOUND_LANG1:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x000000, ~0x180000);
        break;
    }
}

void AVER_TVPHONE_SetAudio(int StereoMode)
{
    BT848_OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x01, ~0x03);
        break;
    case VIDEO_SOUND_LANG1:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x02, ~0x03);
        break;
    default:
        break;
    }
}

void WINFAST2000_SetAudio(int StereoMode)
{
    BT848_OrDataDword(BT848_GPIO_DATA, 0x180000);
    switch(StereoMode)
    {
    case VIDEO_SOUND_STEREO:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x020000, ~0x430000);
        break;
    case VIDEO_SOUND_LANG1:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    case VIDEO_SOUND_LANG2:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x410000, ~0x430000);
        break;
    default:
        BT848_AndOrDataDword(BT848_GPIO_DATA, 0x420000, ~0x430000);
        break;
    }
}

int Card_AutoDetectTuner(eTVCardId CardId)
{
    eTunerId Tuner = TUNER_ABSENT;
    switch(CardId)
    {
    case TVCARD_MIRO:
    case TVCARD_MIROPRO:
        Tuner = (eTunerId)(((BT848_ReadWord(BT848_GPIO_DATA)>>10)-1)&7);
        break;
    default:
        break;
    }
    return Tuner;
}

LPCSTR TVCard_AutoDetectID()
{
    static char CardId[9] = "None    ";

    // look for normal eeprom address
    if(I2CBus_AddDevice(I2C_HAUPEE))
    {
        DWORD Id = BT848_GetSubSystemID();
        if (Id != 0 && Id != 0xffffffff)
        {
            sprintf(CardId, "%8X", Id);
        }
    }
    return CardId;
}


eTVCardId Card_AutoDetect()
{
    // look for normal eeprom address
    if(I2CBus_AddDevice(I2C_HAUPEE))
    {
        DWORD Id = BT848_GetSubSystemID();
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

    // look for STB eeprom address
    if(I2CBus_AddDevice(I2C_STBEE))
    {
        return TVCARD_STB;
    }

    return TVCARD_UNKNOWN;
}

const TCardSetup* GetCardSetup()
{
    return TVCards + CardType;
}

const TTunerSetup* GetTunerSetup()
{
    if(TunerType >= 0)
    {
        return Tuners + TunerType;
    }
    else
    {
        return NULL;
    }
}

void TVCard_FirstTimeSetupHardware(HINSTANCE hInst, HWND hWnd)
{
    // try to detect the card
    CardType = Card_AutoDetect();
    Card_AutoDetectTuner(CardType);

    // then display the hardware setup dialog
    DialogBox(hInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc);
}

void TVCard_ChangeDefault(SETTING* pSetting, long Default)
{
    pSetting->Default = Default;
    *pSetting->pValue = Default;
}

void ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(TunerType != TUNER_ABSENT)
    {
        switch(Tuners[TunerType].Type)
        {
        case PAL:
        case PAL_I:
            TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_PAL_BDGHI);
            break;
        case SECAM:
            TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_SECAM);
            break;
        case NTSC:
        default:
            TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_NTSC);
            break;
        }
    }
}

long GetTunersTVFormat()
{
    if(TunerType != TUNER_ABSENT)
    {
        switch(Tuners[TunerType].Type)
        {
        case PAL:
        case PAL_I:
            return FORMAT_PAL_BDGHI;
            break;
        case SECAM:
            return FORMAT_SECAM;
            break;
        case NTSC:
            return FORMAT_NTSC;
            break;
        }
    }
    return FORMAT_NTSC;
}

    
void ChangeDefaultsBasedOnHardware()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(TunerType != TUNER_ABSENT)
    {
        switch(Tuners[TunerType].Type)
        {
        case PAL:
        case PAL_I:
            TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_PAL_BDGHI);
            break;
        case SECAM:
            TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_SECAM);
            break;
        case NTSC:
        default:
            TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_NTSC);
            break;
        }
    }
    // now do defaults based on the processor speed selected
    if(ProcessorSpeed == 1 && TradeOff == 0)
    {
        // User has selected 300-500 MHz and low judder
        TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 720);
    }
    else if(ProcessorSpeed == 1 && TradeOff == 1)
    {
        // User has selected 300-500 MHz and best picture
        TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), TRUE);
        TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 720);
    }
    else if(ProcessorSpeed == 2 && TradeOff == 0)
    {
        // User has selected below 300 MHz and low judder
        TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 640);
    }
    else if(ProcessorSpeed == 2 && TradeOff == 1)
    {
        // User has selected below 300 MHz and best picture
        TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), TRUE);
        TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 640);
    }
    else
    {
        // user has fast processor use best defaults
        TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_ADAPTIVE);
        TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY2FRAME);
        TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 720);
    }
}

BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int i;
    static long OrigProcessorSpeed;
    static long OrigTradeOff;
    static long OrigTuner;

    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < TVCARD_LASTONE; i++)
        {
            int nIndex;
            nIndex = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)TVCards[i].szName);
            SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETITEMDATA, nIndex, i);
            if(i == CardType)
            {
                SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETCURSEL, nIndex, 0);
            }
        }

        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_RESETCONTENT, 0, 0);
        for(i = 0; i < TUNER_LASTONE; i++)
        {
            SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_ADDSTRING, 0, (LONG)Tuners[i].szName);
        }
        SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TunerType, 0);

        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Above 500 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"300 - 500 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Below 300 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_SETCURSEL, ProcessorSpeed, 0);
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Show all frames - Lowest judder");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Best picture quality");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_SETCURSEL, TradeOff, 0);
        OrigProcessorSpeed = ProcessorSpeed;
        OrigTradeOff = TradeOff;
        OrigTuner = TunerType;
        SetFocus(hDlg);
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            TunerType = (eTunerId)SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0);
            i =  SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
            CardType = (eTVCardId)SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETITEMDATA, i, 0);
            ProcessorSpeed = SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_GETCURSEL, 0, 0);
            TradeOff = SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_GETCURSEL, 0, 0);
            if(OrigProcessorSpeed != ProcessorSpeed || 
                OrigTradeOff != TradeOff)
            {
                ChangeDefaultsBasedOnHardware();
            }
            if(OrigTuner != TunerType)
            {
                ChangeTVSettingsBasedOnTuner();
            }
			WriteSettingsToIni(FALSE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_CARDSSELECT:
            i = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
            i = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETITEMDATA, i, 0);
            if(TVCards[i].TunerId == TUNER_USER_SETUP)
            {
                SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TUNER_ABSENT, 0);
            }
            else if(TVCards[i].TunerId == TUNER_AUTODETECT)
            {
                SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, Card_AutoDetectTuner((eTVCardId)i), 0);
            }
            else
            {
                SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TVCards[i].TunerId, 0);
            }
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return (FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING TVCardSettings[TVCARD_SETTING_LASTONE] =
{
    {
        "Card Type", SLIDER, 0, (long*)&CardType,
         TVCARD_UNKNOWN, TVCARD_UNKNOWN, TVCARD_LASTONE - 1, 1, 1,
         NULL,
        "Hardware", "CardType", NULL,
    },
    {
        "Tuner Type", SLIDER, 0, (long*)&TunerType,
         TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, 1, 1,
         NULL,
        "Hardware", "TunerType", NULL,
    },
    {
        "Processor Speed", SLIDER, 0, (long*)&ProcessorSpeed,
         1, 0, 2, 1, 1,
         NULL,
        "Hardware", "ProcessorSpeed", NULL,
    },
    {
        "Trade Off", SLIDER, 0, (long*)&TradeOff,
         1, 0, 1, 1, 1,
         NULL,
        "Hardware", "TradeOff", NULL,
    },
};


SETTING* TVCard_GetSetting(TVCARD_SETTING Setting)
{
    if(Setting > -1 && Setting < TVCARD_SETTING_LASTONE)
    {
        return &(TVCardSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void TVCard_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < TVCARD_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(TVCardSettings[i]));
    }
    ChangeDefaultsBasedOnHardware();
}

void TVCard_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < TVCARD_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(TVCardSettings[i]), bOptimizeFileAccess);
    }
}

void TVCard_SetMenu(HMENU hMenu)
{
    int NumExtraInputs;

    EnableMenuItem(hMenu, IDM_CHANNELPLUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenu, IDM_CHANNELMINUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenu, IDM_ANALOGSCAN, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SOURCE_TUNER, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);

    // need to keep track of how many other inputs there are
    // there is always one composite input
    NumExtraInputs = TVCards[CardType].nVideoInputs  - 1;

    if(TVCards[CardType].SVideoInput == -1)
    {
        EnableMenuItem(hMenu, IDM_SOURCE_SVIDEO, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_SOURCE_SVIDEO, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_ENABLED);
        // we've used up one more input
        --NumExtraInputs;
    }

    if(TVCards[CardType].TunerInput == -1 || TunerType == TUNER_ABSENT)
    {
        EnableMenuItem(hMenu, IDM_CHANNELPLUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_CHANNELMINUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_ANALOGSCAN, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_SOURCE_TUNER, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_CHANNELPLUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_CHANNELMINUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_ANALOGSCAN, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_SOURCE_TUNER, MF_ENABLED);
        // we've used up one more input
        --NumExtraInputs;
    }

    EnableMenuItem(hMenu, IDM_SOURCE_OTHER1, (NumExtraInputs > 0)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SOURCE_OTHER2, (NumExtraInputs > 1)?MF_ENABLED:MF_GRAYED);

    EnableMenuItem(hMenu, IDM_SOURCE_CCIR656_1, (CardType == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SOURCE_CCIR656_2, (CardType == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SOURCE_CCIR656_3, (CardType == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
    EnableMenuItem(hMenu, IDM_SOURCE_CCIR656_4, (CardType == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
}
