/////////////////////////////////////////////////////////////////////////////
// $Id: TVCards.cpp,v 1.32 2001-11-09 12:42:07 adcockj Exp $
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
// Revision 1.31  2001/10/28 16:40:01  ittarnavsky
// added card and tuner definitions from bttv-0.7.83
//
// Revision 1.30  2001/10/26 16:18:47  adcockj
// Added Start bt878 card with no MSP init as workaround for US cards
//
// Revision 1.29  2001/10/25 16:20:23  ittarnavsky
// added support for the VoodooTV FM (Europa)
//
// Revision 1.28  2001/10/20 18:34:12  ittarnavsky
// fixed the hardware setup dialog to properly select the default tuner provided in the card definition of the curently selected card
//
// Revision 1.27  2001/10/20 09:36:59  adcockj
// Fixed tuner in hardware dialog
//
// Revision 1.26  2001/10/19 18:44:25  ittarnavsky
// added support for the VoodooTV 200/FM cards and the MT2032 tuner
//
// Revision 1.25  2001/09/18 08:10:35  adcockj
// Added Aimslab VideoHighway Extreme (not 98) Thanks to Dax Sieger
//
// Revision 1.24  2001/09/09 17:46:29  laurentg
// no message
//
// Revision 1.23  2001/09/07 20:56:57  laurentg
// no message
//
// Revision 1.22  2001/09/07 20:39:43  laurentg
// New card added : Pinnacle PCTV Sat
//
// Revision 1.21  2001/07/27 10:54:23  adcockj
// Fix for s-video on ATI TV wonder
//
// Revision 1.20  2001/07/25 02:24:03  dschmelzer
// Add support for Sasem 4 Channel developer card with s-video jumper chosen
//
// Revision 1.19  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.18  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.17  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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
void ctrl_TDA8540(int SLV, int SUB, int SW1, int GCO, int OEN);

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
        // TODO: check this 3.0.0 { 2, 3, 0, 1, 0, 0, 0, 0},
        { 2, 3, 1, 0, 0, 0, 0, 0},
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
        NULL,
    },
    {
        "Pinnacle PCTV Sat",
        2, 1, -1, 2, 0x03000F,
        { 0, 3, 1, 1, 0, 0, 0, 0},
        { 0, 0, 0, 0, 1, 0},
        0,
        PLL_28,
        TUNER_USER_SETUP,
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        NULL,
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
        GVBCTV3PCI_SetAudio,
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
        NULL,
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
    },
    // TUNER_MT2032
    {
        "MT2032 universal", MICROTUNE, NOTTYPE,
        0, 0, 0, 0, 0, 0, 0
    },
    // TUNER_SHARP_2U5JF5540_NTSC
    {
        "SHARP NTSC_JP (2U5JF5540)", SHARP, NTSC,
        16*137.25, 16*317.25, 0x01, 0x02, 0x08, 0x8e, 940 // 940=16*58.75 NTSC@Japan
    },
};


// reset/enable the MSP on some Hauppauge cards
// Thanks to Kyösti Mälkki (kmalkki@cc.hut.fi)!
static void initialize_msp34xx(int pin)
{
    int mask = 1 << pin;
    BT848_AndOrDataDword(BT848_GPIO_DATA, mask, ~mask);
    BT848_AndOrDataDword(BT848_GPIO_DATA, 0, ~mask);
    Sleep(10);
    BT848_AndOrDataDword(BT848_GPIO_DATA, mask, ~mask);
}

// do any specific card related initilaisation
void Card_Init()
{
    switch(CardType)
    {
    case TVCARD_HAUPPAUGE:
    case TVCARD_HAUPPAUGE878:
        initialize_msp34xx(5);
        break;
    case TVCARD_VOODOOTV_200:
    case TVCARD_VOODOOTV_FM:
        initialize_msp34xx(20);
        break;
    case TVCARD_PXC200:
        init_PXC200();
        break;
    case TVCARD_SASEM4CHNLSVID:
        // Initialize and set the Philips TDA8540 4x4 switch matrix for s-video
        // 0xD2 SW1 choose OUT3=IN3; OUT2=IN1; OUT1=IN0; OUT0=IN2
        // 0x07 GCO choose (0000) gain; (01) clamp; (11) aux
        // 0x03 OEN choose OUT0 and OUT1 high (i.e., s-video)
        ctrl_TDA8540(0x90, 0x00, 0xD2, 0x07, 0x03);
        break;
    default:
        break;
    }
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

void ctrl_TDA8540(int SLV, int SUB, int SW1, int GCO, int OEN)
{
    I2CBus_Lock();
    I2CBus_Start();
    I2CBus_SendByte(SLV, 5);
    I2CBus_SendByte(SUB, 0);
    I2CBus_SendByte(SW1, 0);
    I2CBus_SendByte(GCO, 0);
    I2CBus_SendByte(OEN, 0);
    I2CBus_Stop();
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

eTunerId Card_AutoDetectTuner(eTVCardId CardId)
{
    eTunerId Tuner = TUNER_ABSENT;
    switch(CardId)
    {
    case TVCARD_MIRO:
    case TVCARD_MIROPRO:
        Tuner = (eTunerId)(((BT848_ReadWord(BT848_GPIO_DATA)>>10)-1)&7);
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

void TVCard_FirstTimeSetupHardware()
{
    // try to detect the card
    CardType = Card_AutoDetect();
    TunerType = Card_AutoDetectTuner(CardType);

    // then display the hardware setup dialog
    DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc);
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
        EnableMenuItem(hMenu, IDM_CHANNEL_PREVIOUS, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_ANALOGSCAN, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_CHANNEL_LIST, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_SOURCE_TUNER, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_AUDIO_0, MF_GRAYED);
    }
    else
    {
        EnableMenuItem(hMenu, IDM_CHANNELPLUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_CHANNELMINUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_CHANNEL_PREVIOUS, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_ANALOGSCAN, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_CHANNEL_LIST, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_SOURCE_TUNER, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_AUDIO_0, MF_ENABLED);
    }
    if(TVCards[CardType].SVideoInput != -1)
    {
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
