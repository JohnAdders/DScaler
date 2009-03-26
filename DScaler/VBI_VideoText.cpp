/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
// Color buttons code was taken from GnomeTV. Those portions are
// Copyright (C) 1999-2000 Zoltán Sinkovics and Szabolcs Seláf
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file VBI_VideoText.cpp VBI teletext functions
 */

#include "stdafx.h"
#include "VTCommon.h"
#include "VTDecoder.h"
#include "VTDrawer.h"
#include "VBI_VideoText.h"
#include "VBI.h"
#include "DScaler.h"
#include "Providers.h"
#include "DynamicFunction.h"

using namespace std;

#define VT_MAXPAGEHISTORY               64

#define VT_OSD_DISPLAY_TIMEOUT          2000
#define VT_OSD_DEFAULT_INPUT_TIMEOUT    2


BOOL VT_SetCodepage(HDC hDC, LPRECT lpRect, eVTCodepage Codepage);

void VT_HistoryReset();
void VT_HistoryPushPage(WORD wPageHex);
WORD VT_HistoryPopLastPage(WORD wCurrentPageHex);

void VT_DecoderEventProc(BYTE uMsg, DWORD dwParam);
void VT_PerformUpscrollDuplicationFilter();
void VT_PerformDoubleHeightSubtitlesFilter();
void VT_DeleteHilightList(TVTLeftRight** pHilightList);
void VT_UpdateHilightList(BOOL bUpdatedOnly = FALSE);
BYTE VT_UpdateHilightListProc(TVTPage*, WORD, LPWORD, WORD, BYTE, BYTE, LPVOID);

WORD VT_Input2PageHex(char Input[3]);

BOOL VT_RegionOnChange(long NewValue);
BOOL VT_ShowSubcodeInOSDOnChange(long NewValue);
BOOL VT_CachingControlOnChange(long NewValue);
BOOL VT_HighGranularityCachingOnChange(long NewValue);
BOOL VT_SubstituteErrorsWithSpacesOnChange(long NewValue);


CVTDecoder          VTDecoder(VT_DecoderEventProc);
CVTDrawer           VTDrawer;

int                 VTStep;

eVTCodepage         VTCodepage = VTCODEPAGE_ENGLISH;
eVTCodepage         VTUserCodepage = VTCODEPAGE_ENGLISH;
eVTRegion           VTRegion = VTREGION_DEFAULT;
BOOL                VTAutoCodepage = TRUE;

eVTState            VTState = VT_OFF;
TVTPage             VTVisiblePage;

WORD                VTSavePageHex = 0x100;
WORD                VTSavePageSubCode = 0xFFFF;

WORD                VTPageHex;
WORD                VTPageSubCode;
DWORD               VTLoadedPageCode = 0UL;

BOOL                VTShowHidden = FALSE;
BOOL                VTShowFlash = FALSE;
BOOL                VTAntialias = TRUE;

BOOL                VTShowSubcodeInOSD = FALSE;

BYTE                VTCachingControl = 0;
BOOL                VTHighGranularityCaching = FALSE;
BOOL                VTSubstituteErrorsWithSpaces = FALSE;

BOOL                VTUpscrollDuplicationFilter = FALSE;
BOOL                VTDoubleHeightSubtitlesFilter = FALSE;

char                VTSearchString[41] = "";
BOOL                VTHilightSearch = TRUE;

UINT                VTFlashTimer = 0;

BOOL                VTShowOSD = FALSE;
char                VTOSDBuffer[4] = "";
char*               VTPageOSD = NULL;
char                VTPageInput[4] = "";

// This variable controls the display duration for VT_ShowHeader();
LONG                g_VTOSDTimeout = VT_OSD_DISPLAY_TIMEOUT;

WORD                VTPageHistoryHead;
WORD                VTPageHistory[VT_MAXPAGEHISTORY];

TVTLeftRight**      VTHilightListPtr = NULL;
TVTLeftRight*       VTHilightListArray[25];

BYTE                VTDoubleProfile[25];

RECT                VTRefreshProfile[25];
BYTE                VTRefreshCount;

WORD                VTCursorRowCol = 0xFFFF;
WORD                VTCursorPageHex = 0;

HWND                VTGotoProcDlg = NULL;

CRITICAL_SECTION    VTPageChangeMutex;


//
// ETSI TR 101 203
//
// The following table contains the registered values of the Network Identification (NI) field of the Teletext
// Broadcast Service Data Packet (extension data packet of type 8/30 format 1), the Country and Network
// Identification (CNI) fields of the Programme Delivery Control (PDC) data packets (extension data packets X/26 and
// 8/30 format 2), and the VPS CNI codes.
//
// Source: http://www.ebu.ch/CMSimages/en/tec_info_tr231-2005_tcm6-18471.pdf
//
TChannelIdentif RegisteredCNICodes[] =
{
{ "Austria",        "ORF-1",                                            0x4301, 0,    0,    0,    0,    0x0AC1 },
{ "Austria",        "ORF-2",                                            0x4302, 0,    0,    0,    0,    0x0AC2 },
{ "Austria",        "ORF future use",                                   0x4303, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x4304, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x4305, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x4306, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x4307, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x4308, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x4309, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x430A, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF future use",                                   0x430B, 0,    0,    0,    0,    0      },
{ "Austria",        "ATV",                                              0x430C, 0,    0,    0,    0,    0      },
{ "Austria",        "ORF - FS 3",                                       0,      0,    0,    0,    0,    0x0AC3 },
{ "Austria",        "ORF- FS 2: Lokalprogramm Burgenland",              0,      0,    0,    0,    0,    0x0ACB },
{ "Austria",        "ORF- FS 2: Lokalprogramm Kärnten",                 0,      0,    0,    0,    0,    0x0ACC },
{ "Austria",        "ORF- FS 2: Lokalprogramm Niederösterreich",        0,      0,    0,    0,    0,    0x0ACD },
{ "Austria",        "ORF- FS 2: Lokalprogramm Oberösterreich",          0,      0,    0,    0,    0,    0x0ACE },
{ "Austria",        "ORF- FS 2: Lokalprogramm Salzburg",                0,      0,    0,    0,    0,    0x0ACF },
{ "Austria",        "ORF- FS 2: Lokalprogramm Steiermark",              0,      0,    0,    0,    0,    0x0AD0 },
{ "Austria",        "ORF- FS 2: Lokalprogramm Tirol",                   0,      0,    0,    0,    0,    0x0AD1 },
{ "Austria",        "ORF- FS 2: Lokalprogramm Vorarlberg",              0,      0,    0,    0,    0,    0x0AD2 },
{ "Austria",        "ORF- FS 2: Lokalprogramm Wien",                    0,      0,    0,    0,    0,    0x0AD3 },

{ "Belgium",        "VT4",                                              0x0404, 0x16, 0x04, 0x36, 0x04, 0      },
{ "Belgium",        "VRT TV1",                                          0x3201, 0x16, 0x01, 0x36, 0x03, 0      },
{ "Belgium",        "CANVAS",                                           0x3202, 0x16, 0x02, 0x36, 0x02, 0      },
{ "Belgium",        "RTBF 1",                                           0x3203, 0,    0,    0,    0,    0      },
{ "Belgium",        "RTBF 2",                                           0x3204, 0,    0,    0,    0,    0      },
{ "Belgium",        "VTM",                                              0x3205, 0x16, 0x05, 0x36, 0x05, 0      },
{ "Belgium",        "Kanaal2",                                          0x3206, 0x16, 0x06, 0x36, 0x06, 0      },
{ "Belgium",        "RTBF Sat",                                         0x3207, 0,    0,    0,    0,    0      },
{ "Belgium",        "RTBF future use",                                  0x3208, 0,    0,    0,    0,    0      },
{ "Belgium",        "RTL-TVI",                                          0x3209, 0,    0,    0,    0,    0      },
{ "Belgium",        "CLUB-RTL",                                         0x320A, 0,    0,    0,    0,    0      },
{ "Belgium",        "AB3",                                              0x320C, 0,    0,    0,    0,    0      },
{ "Belgium",        "AB4e",                                             0x320D, 0,    0,    0,    0,    0      },
{ "Belgium",        "Ring TV",                                          0x320E, 0,    0,    0,    0,    0      },
{ "Belgium",        "JIM.tv",                                           0x320F, 0,    0,    0,    0,    0      },
{ "Belgium",        "RTV-Kempen",                                       0x3210, 0,    0,    0,    0,    0      },
{ "Belgium",        "RTV-Mechelen",                                     0x3211, 0,    0,    0,    0,    0      },
{ "Belgium",        "MCM Belgium",                                      0x3212, 0,    0,    0,    0,    0      },
{ "Belgium",        "Vitaya",                                           0x3213, 0,    0,    0,    0,    0      },
{ "Belgium",        "WTV",                                              0x3214, 0,    0,    0,    0,    0      },
{ "Belgium",        "FocusTV",                                          0x3215, 0,    0,    0,    0,    0      },
{ "Belgium",        "Be 1 ana",                                         0x3216, 0,    0,    0,    0,    0      },
{ "Belgium",        "Be 1 num",                                         0x3217, 0,    0,    0,    0,    0      },
{ "Belgium",        "Be Ciné 1",                                        0x3218, 0,    0,    0,    0,    0      },
{ "Belgium",        "Be Sport 1",                                       0x3219, 0,    0,    0,    0,    0      },
{ "Belgium",        "Canal+VL1",                                        0x321A, 0,    0,    0,    0,    0      },
{ "Belgium",        "Canal+VL1",                                        0x321B, 0,    0,    0,    0,    0      },
{ "Belgium",        "Canal+ Blau",                                      0x321C, 0,    0,    0,    0,    0      },
{ "Belgium",        "Canal+ Rood",                                      0x321D, 0,    0,    0,    0,    0      },
{ "Belgium",        "TV Brussel",                                       0x321E, 0,    0,    0,    0,    0      },
{ "Belgium",        "AVSe",                                             0x321F, 0,    0,    0,    0,    0      },
{ "Belgium",        "TV Limburg",                                       0x3221, 0,    0,    0,    0,    0      },
{ "Belgium",        "Kanaal 3",                                         0x3222, 0,    0,    0,    0,    0      },
{ "Belgium",        "Sporza",                                           0x3226, 0,    0,    0,    0,    0      },
{ "Belgium",        "VIJF tv",                                          0x3227, 0,    0,    0,    0,    0      },
{ "Belgium",        "ATV",                                              0x3223, 0,    0,    0,    0,    0      },
{ "Belgium",        "ROB TV",                                           0x3224, 0,    0,    0,    0,    0      },
{ "Belgium",        "PLUG TV",                                          0x3225, 0,    0,    0,    0,    0      },
{ "Belgium",        "Télé Bruxelles",                                   0x3230, 0,    0,    0,    0,    0      },
{ "Belgium",        "Télésambre",                                       0x3231, 0,    0,    0,    0,    0      },
{ "Belgium",        "TV Com",                                           0x3232, 0,    0,    0,    0,    0      },
{ "Belgium",        "Canal Zoom",                                       0x3233, 0,    0,    0,    0,    0      },
{ "Belgium",        "Vidéoscope",                                       0x3234, 0,    0,    0,    0,    0      },
{ "Belgium",        "Canal C",                                          0x3235, 0,    0,    0,    0,    0      },
{ "Belgium",        "Télé MB",                                          0x3236, 0,    0,    0,    0,    0      },
{ "Belgium",        "Antenne Centre",                                   0x3237, 0,    0,    0,    0,    0      },
{ "Belgium",        "Télévesdre",                                       0x3238, 0,    0,    0,    0,    0      },
{ "Belgium",        "RTC Télé Liège",                                   0x3239, 0,    0,    0,    0,    0      },
{ "Belgium",        "No tele",                                          0x3240, 0,    0,    0,    0,    0      },
{ "Belgium",        "TV Lux",                                           0x3241, 0,    0,    0,    0,    0      },
{ "Belgium",        "Kanaal Z - NL",                                    0x325A, 0,    0,    0,    0,    0      },
{ "Belgium",        "CANAL Z - FR",                                     0x325B, 0,    0,    0,    0,    0      },
{ "Belgium",        "CARTOON Network - NL",                             0x326A, 0,    0,    0,    0,    0      },
{ "Belgium",        "CARTOON Network - FR",                             0x326B, 0,    0,    0,    0,    0      },
{ "Belgium",        "LIBERTY CHANNEL - NL",                             0x327A, 0,    0,    0,    0,    0      },
{ "Belgium",        "LIBERTY CHANNEL - FR",                             0x327B, 0,    0,    0,    0,    0      },
{ "Belgium",        "TCM - NL",                                         0x328A, 0,    0,    0,    0,    0      },
{ "Belgium",        "TCM - FR",                                         0x328B, 0,    0,    0,    0,    0      },
{ "Belgium",        "Mozaiek/Mosaique",                                 0x3298, 0,    0,    0,    0,    0      },
{ "Belgium",        "Info Kanaal/Canal Info",                           0x3299, 0,    0,    0,    0,    0      },
{ "Belgium",        "Be 1 + 1h",                                        0x32A7, 0,    0,    0,    0,    0      },
{ "Belgium",        "Be Ciné 2",                                        0x32A8, 0,    0,    0,    0,    0      },
{ "Belgium",        "Be Sport 2",                                       0x32A9, 0,    0,    0,    0,    0      },

{ "Croatia",        "HRT",                                              0x0385, 0,    0,    0,    0,    0      },

{ "Czech Republic", "CT 1",                                             0x4201, 0x32, 0xC1, 0x3C, 0x21, 0      },
{ "Czech Republic", "CT 2",                                             0x4202, 0x32, 0xC2, 0x3C, 0x22, 0      },
{ "Czech Republic", "NOVA TV",                                          0x4203, 0x32, 0xC3, 0x3C, 0x23, 0      },
{ "Czech Republic", "Prima TV",                                         0x4204, 0x32, 0xC4, 0x3C, 0x04, 0      },
{ "Czech Republic", "TV Praha",                                         0x4205, 0,    0,    0,    0,    0      },
{ "Czech Republic", "TV HK",                                            0x4206, 0,    0,    0,    0,    0      },
{ "Czech Republic", "TV Pardubice",                                     0x4207, 0,    0,    0,    0,    0      },
{ "Czech Republic", "TV Brno",                                          0x4208, 0,    0,    0,    0,    0      },
{ "Czech Republic", "CT1 Regional, Brno",                               0x4211, 0x32, 0xD1, 0x3B, 0x01, 0      },
{ "Czech Republic", "CT2 Regional, Brno",                               0x4212, 0x32, 0xD2, 0x3B, 0x04, 0      },
{ "Czech Republic", "CT1 Regional, Ostravia",                           0x4221, 0x32, 0xE1, 0x3B, 0x02, 0      },
{ "Czech Republic", "CT2 Regional, Ostravia",                           0x4222, 0x32, 0xE2, 0x3B, 0x05, 0      },
{ "Czech Republic", "CT1 Regional",                                     0x4231, 0x32, 0xF1, 0x3C, 0x25, 0      },
{ "Czech Republic", "CT2 Regional",                                     0x4232, 0x32, 0xF2, 0x3B, 0x03, 0      },

{ "Denmark",        "TV 2",                                             0x4502, 0x29, 0x02, 0x39, 0x02, 0      },
{ "Denmark",        "TV 2 0xZulu",                                      0x4503, 0x29, 0x04, 0x39, 0x04, 0      },
{ "Denmark",        "Discovery Denmark",                                0x4504, 0,    0,    0,    0,    0      },
{ "Denmark",        "TV 2 0xCharlie",                                   0x4505, 0x29, 0x05, 0,    0,    0      },
{ "Denmark",        "TV Danmark",                                       0x4506, 0x29, 0x06, 0,    0,    0      },
{ "Denmark",        "Kanal 5",                                          0x4507, 0x29, 0x07, 0,    0,    0      },
{ "Denmark",        "TV 2 Film",                                        0x4508, 0x29, 0x08, 0,    0,    0      },
{ "Denmark",        "DR2",                                              0x49CF, 0x29, 0x03, 0x39, 0x03, 0      },
{ "Denmark",        "DR1",                                              0x7392, 0x29, 0x01, 0x39, 0x01, 0      },

{ "Finland",        "YLE1",                                             0x3581, 0x26, 0x01, 0x36, 0x01, 0      },
{ "Finland",        "YLE2",                                             0x3582, 0x26, 0x02, 0x36, 0x07, 0      },
{ "Finland",        "YLE future use",                                   0x3583, 0x26, 0x03, 0x36, 0x08, 0      },
{ "Finland",        "YLE future use",                                   0x3584, 0x26, 0x04, 0x36, 0x09, 0      },
{ "Finland",        "YLE future use",                                   0x3585, 0x26, 0x05, 0x36, 0x0A, 0      },
{ "Finland",        "YLE future use",                                   0x3586, 0x26, 0x06, 0x36, 0x0B, 0      },
{ "Finland",        "YLE future use",                                   0x3587, 0x26, 0x07, 0x36, 0x0C, 0      },
{ "Finland",        "YLE future use",                                   0x3588, 0x26, 0x08, 0x36, 0x0D, 0      },
{ "Finland",        "YLE future use",                                   0x3589, 0x26, 0x09, 0x36, 0x0E, 0      },
{ "Finland",        "YLE future use",                                   0x358A, 0x26, 0x0A, 0x36, 0x0F, 0      },
{ "Finland",        "YLE future use",                                   0x358B, 0x26, 0x0B, 0x36, 0x10, 0      },
{ "Finland",        "YLE future use",                                   0x358C, 0x26, 0x0C, 0x36, 0x11, 0      },
{ "Finland",        "YLE future use",                                   0x358D, 0x26, 0x0D, 0x36, 0x12, 0      },
{ "Finland",        "YLE future use",                                   0x358E, 0x26, 0x0E, 0x36, 0x13, 0      },
{ "Finland",        "OWL3",                                             0x358F, 0x26, 0x0F, 0x36, 0x14, 0      },

{ "France",         "France 5 / Arte",                                  0x330A, 0x2F, 0x0A, 0x3F, 0x0A, 0      },
{ "France",         "RFO1",                                             0x3311, 0x2F, 0x11, 0x3F, 0x11, 0      },
{ "France",         "RFO2",                                             0x3312, 0x2F, 0x12, 0x3F, 0x12, 0      },
{ "France",         "Aqui TV",                                          0x3320, 0x2F, 0x20, 0x3F, 0x20, 0      },
{ "France",         "TLM",                                              0x3321, 0x2F, 0x21, 0x3F, 0x21, 0      },
{ "France",         "TLT",                                              0x3322, 0x2F, 0x22, 0x3F, 0x22, 0      },
{ "France",         "Sailing Channel",                                  0x33B2, 0,    0,    0,    0,    0      },
{ "France",         "AB1",                                              0x33C1, 0x2F, 0xC1, 0x3F, 0x41, 0      },
{ "France",         "Canal J",                                          0x33C2, 0x2F, 0xC2, 0x3F, 0x42, 0      },
{ "France",         "Canal Jimmy",                                      0x33C3, 0x2F, 0xC3, 0x3F, 0x43, 0      },
{ "France",         "LCI",                                              0x33C4, 0x2F, 0xC4, 0x3F, 0x44, 0      },
{ "France",         "La Chaîne Météo",                                  0x33C5, 0x2F, 0xC5, 0x3F, 0x45, 0      },
{ "France",         "MCM",                                              0x33C6, 0x2F, 0xC6, 0x3F, 0x46, 0      },
{ "France",         "TMC Monte-Carlo",                                  0x33C7, 0x2F, 0xC7, 0x3F, 0x47, 0      },
{ "France",         "Paris Première",                                   0x33C8, 0x2F, 0xC8, 0x3F, 0x48, 0      },
{ "France",         "Planète",                                          0x33C9, 0x2F, 0xC9, 0x3F, 0x49, 0      },
{ "France",         "Série Club",                                       0x33CA, 0x2F, 0xCA, 0x3F, 0x4A, 0      },
{ "France",         "Télétoon",                                         0x33CB, 0x2F, 0xCB, 0x3F, 0x4B, 0      },
{ "France",         "Téva",                                             0x33CC, 0x2F, 0xCC, 0x3F, 0x4C, 0      },
{ "France",         "TF1",                                              0x33F1, 0x2F, 0x01, 0x3F, 0x01, 0      },
{ "France",         "France 2",                                         0x33F2, 0x2F, 0x02, 0x3F, 0x02, 0      },
{ "France",         "France 3",                                         0x33F3, 0x2F, 0x03, 0x3F, 0x03, 0      },
{ "France",         "Canal+",                                           0x33F4, 0x2F, 0x04, 0x3F, 0x04, 0      },
{ "France",         "M6",                                               0x33F6, 0x2F, 0x06, 0x3F, 0x06, 0      },
{ "France",         "Eurosport",                                        0xF101, 0x2F, 0xE2, 0x3F, 0x62, 0      },
{ "France",         "TV5",                                              0xF500, 0x2F, 0xE5, 0x3F, 0x65, 0      },
{ "France",         "Euronews",                                         0xFE01, 0x2F, 0xE1, 0x3F, 0x61, 0      },

{ "Germany",        "ARD",                                              0x4901, 0,    0,    0,    0,    0x0DC1 },
{ "Germany",        "ZDF",                                              0x4902, 0,    0,    0,    0,    0x0DC2 },
{ "Germany",        "Arte",                                             0x490A, 0,    0,    0,    0,    0x0D85 },
{ "Germany",        "VOX",                                              0x490C, 0,    0,    0,    0,    0x0D8E },
{ "Germany",        "HR",                                               0x4915, 0,    0,    0,    0,    0x0DCF },
{ "Germany",        "Phoenix",                                          0x4918, 0,    0,    0,    0,    0x0DC8 }, // 0x4908 in ETSI TR
{ "Germany",        "Festival",                                         0x4941, 0,    0,    0,    0,    0x0D41 },
{ "Germany",        "MUXX",                                             0x4942, 0,    0,    0,    0,    0x0D42 },
{ "Germany",        "EXTRA",                                            0x4943, 0,    0,    0,    0,    0x0D43 },
{ "Germany",        "BR-Alpha",                                         0x4944, 0,    0,    0,    0,    0x0D44 },
{ "Germany",        "ARD future use",                                   0x4981, 0,    0,    0,    0,    0      },
{ "Germany",        "ORB",                                              0x4982, 0,    0,    0,    0,    0x0D82 },
{ "Germany",        "1-2-3.TV",                                         0x49BD, 0,    0,    0,    0,    0x0D77 },
{ "Germany",        "Tele-5",                                           0x49BE, 0,    0,    0,    0,    0x0D7B },
{ "Germany",        "HSE",                                              0x49BF, 0,    0,    0,    0,    0x0D7F },
{ "Germany",        "ARD future use",                                   0x49C1, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49C3, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49C4, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49C5, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49C6, 0,    0,    0,    0,    0      },
{ "Germany",        "3sat",                                             0x49C7, 0,    0,    0,    0,    0x0DC7 },
{ "Germany",        "KiKa",                                             0x49C9, 0,    0,    0,    0,    0x0DC9 },
{ "Germany",        "ARD future use",                                   0x49CA, 0,    0,    0,    0,    0      },
{ "Germany",        "BR",                                               0x49CB, 0,    0,    0,    0,    0x0DCB },
{ "Germany",        "ARD future use",                                   0x49CC, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49CD, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49CE, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49D0, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49D1, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49D2, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49D3, 0,    0,    0,    0,    0      },
{ "Germany",        "NDR",                                              0x49D4, 0,    0,    0,    0,    0x0DD4 },
{ "Germany",        "ARD future use",                                   0x49D5, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49D6, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49D7, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49D8, 0,    0,    0,    0,    0      },
{ "Germany",        "RB-1",                                             0x49D9, 0,    0,    0,    0,    0x0DD9 },
{ "Germany",        "ARD future use",                                   0x49DA, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49DB, 0,    0,    0,    0,    0      },
{ "Germany",        "SFB",                                              0x49DC, 0,    0,    0,    0,    0x0DDC },
{ "Germany",        "ARD future use",                                   0x49DD, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49DE, 0,    0,    0,    0,    0      },
{ "Germany",        "SR-1",                                             0x49DF, 0,    0,    0,    0,    0x0DDF },
{ "Germany",        "ARD future use",                                   0x49E0, 0,    0,    0,    0,    0      },
{ "Germany",        "SWR-BW",                                           0x49E1, 0,    0,    0,    0,    0x0DE1 },
{ "Germany",        "ARD future use",                                   0x49E2, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49E3, 0,    0,    0,    0,    0      },
{ "Germany",        "SWR RP",                                           0x49E4, 0,    0,    0,    0,    0x0DE4 },
{ "Germany",        "ARD future use",                                   0x49E5, 0,    0,    0,    0,    0      },
{ "Germany",        "WDR",                                              0x49E6, 0,    0,    0,    0,    0x0DE6 },
{ "Germany",        "ARD future use",                                   0x49E7, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49E8, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49E9, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49EA, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49EB, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49EC, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49ED, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49EE, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49EF, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F0, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F1, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F2, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F3, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F4, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F5, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F6, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F7, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F8, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49F9, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49FA, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49FB, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49FC, 0,    0,    0,    0,    0      },
{ "Germany",        "ARD future use",                                   0x49FD, 0,    0,    0,    0,    0      },
{ "Germany",        "MDR",                                              0x49FE, 0,    0,    0,    0,    0x0DFE },
{ "Germany",        "HR-1 Regional",                                    0x49FF, 0,    0,    0,    0,    0x0DCE },
{ "Germany",        "QVC",                                              0x5C49, 0,    0,    0,    0,    0x0D7D },
{ "Germany",        "n24",                                              0,      0,    0,    0,    0,    0x0D7A },
{ "Germany",        "ONYX",                                             0,      0,    0,    0,    0,    0x0D7C },
{ "Germany",        "Nickelodeon",                                      0,      0,    0,    0,    0,    0x0D7E },
{ "Germany",        "ORB-1",                                            0,      0,    0,    0,    0,    0x0D81 },
{ "Germany",        "1A",                                               0,      0,    0,    0,    0,    0x0D87 },
{ "Germany",        "VIVA",                                             0,      0,    0,    0,    0,    0x0D88 },
{ "Germany",        "VIVA 2",                                           0,      0,    0,    0,    0,    0x0D89 },
{ "Germany",        "Super RTL",                                        0,      0,    0,    0,    0,    0x0D8A },
{ "Germany",        "RTL Club",                                         0,      0,    0,    0,    0,    0x0D8B },
{ "Germany",        "n-tv",                                             0,      0,    0,    0,    0,    0x0D8C },
{ "Germany",        "DSF",                                              0,      0,    0,    0,    0,    0x0D8D },
{ "Germany",        "RTL 2",                                            0,      0,    0,    0,    0,    0x0D8F },
{ "Germany",        "RTL 2 Regional",                                   0,      0,    0,    0,    0,    0x0D90 },
{ "Germany",        "Eurosport",                                        0,      0,    0,    0,    0,    0x0D91 },
{ "Germany",        "Kabel 1",                                          0,      0,    0,    0,    0,    0x0D92 },
{ "Germany",        "Pro 7",                                            0,      0,    0,    0,    0,    0x0D94 },
{ "Germany",        "SAT.1 Brandenburg",                                0,      0,    0,    0,    0,    0x0D95 },
{ "Germany",        "SAT.1 Thüringen",                                  0,      0,    0,    0,    0,    0x0D96 },
{ "Germany",        "SAT.1 Sachsen",                                    0,      0,    0,    0,    0,    0x0D97 },
{ "Germany",        "SAT.1 Mecklenburg-Vorpommern",                     0,      0,    0,    0,    0,    0x0D98 },
{ "Germany",        "SAT.1 Sachsen-Anhalt",                             0,      0,    0,    0,    0,    0x0D99 },
{ "Germany",        "RTL Regional",                                     0,      0,    0,    0,    0,    0x0D9A },
{ "Germany",        "RTL Schleswig-Holstein",                           0,      0,    0,    0,    0,    0x0D9B },
{ "Germany",        "RTL Hamburg",                                      0,      0,    0,    0,    0,    0x0D9C },
{ "Germany",        "RTL Berlin",                                       0,      0,    0,    0,    0,    0x0D9D },
{ "Germany",        "RTL Niedersachsen",                                0,      0,    0,    0,    0,    0x0D9E },
{ "Germany",        "RTL Bremen",                                       0,      0,    0,    0,    0,    0x0D9F },
{ "Germany",        "RTL Nordrhein-Westfalen",                          0,      0,    0,    0,    0,    0x0DA0 },
{ "Germany",        "RTL Hessen",                                       0,      0,    0,    0,    0,    0x0DA1 },
{ "Germany",        "RTL Rheinland-Pfalz",                              0,      0,    0,    0,    0,    0x0DA2 },
{ "Germany",        "RTL Baden-Württemberg",                            0,      0,    0,    0,    0,    0x0DA3 },
{ "Germany",        "RTL Bayern",                                       0,      0,    0,    0,    0,    0x0DA4 },
{ "Germany",        "RTL Saarland",                                     0,      0,    0,    0,    0,    0x0DA5 },
{ "Germany",        "RTL Sachsen-Anhalt",                               0,      0,    0,    0,    0,    0x0DA6 },
{ "Germany",        "RTL Mecklenburg-Vorpommern",                       0,      0,    0,    0,    0,    0x0DA7 },
{ "Germany",        "RTL Sachsen",                                      0,      0,    0,    0,    0,    0x0DA8 },
{ "Germany",        "RTL Thüringen",                                    0,      0,    0,    0,    0,    0x0DA9 },
{ "Germany",        "RTL Brandenburg",                                  0,      0,    0,    0,    0,    0x0DAA },
{ "Germany",        "RTL Plus",                                         0,      0,    0,    0,    0,    0x0DAB },
{ "Germany",        "Premiere",                                         0,      0,    0,    0,    0,    0x0DAC },
{ "Germany",        "SAT.1 Regional",                                   0,      0,    0,    0,    0,    0x0DAD },
{ "Germany",        "SAT.1 Schleswig-Holstein",                         0,      0,    0,    0,    0,    0x0DAE },
{ "Germany",        "SAT.1 Hamburg",                                    0,      0,    0,    0,    0,    0xDDAF },
{ "Germany",        "SAT.1 Berlin",                                     0,      0,    0,    0,    0,    0x0DB0 },
{ "Germany",        "SAT.1 Niedersachsen",                              0,      0,    0,    0,    0,    0x0DB1 },
{ "Germany",        "SAT.1 Bremen",                                     0,      0,    0,    0,    0,    0x0DB2 },
{ "Germany",        "SAT.1 Nordrhein-Westfalen",                        0,      0,    0,    0,    0,    0x0DB3 },
{ "Germany",        "SAT.1 Hessen",                                     0,      0,    0,    0,    0,    0x0DB4 },
{ "Germany",        "SAT.1 Rheinland-Pfalz",                            0,      0,    0,    0,    0,    0x0DB5 },
{ "Germany",        "SAT.1 Baden-Württemberg",                          0,      0,    0,    0,    0,    0x0DB6 },
{ "Germany",        "SAT.1 Bayern",                                     0,      0,    0,    0,    0,    0x0DB7 },
{ "Germany",        "SAT.1 Saarland",                                   0,      0,    0,    0,    0,    0x0DB8 },
{ "Germany",        "SAT.1",                                            0,      0,    0,    0,    0,    0x0DB9 },
{ "Germany",        "9live",                                            0,      0,    0,    0,    0,    0x0DBA },
{ "Germany",        "Deutsche Welle Berlin",                            0,      0,    0,    0,    0,    0x0DBB },
{ "Germany",        "Berlin-Offener Kanal",                             0,      0,    0,    0,    0,    0x0DBD },
{ "Germany",        "Berlin-Mix-Channel II",                            0,      0,    0,    0,    0,    0x0DBE },
{ "Germany",        "Berlin-Mix-Channel I",                             0,      0,    0,    0,    0,    0x0DBF },
{ "Germany",        "B1",                                               0,      0,    0,    0,    0,    0x0DCA },
{ "Germany",        "BR Süd",                                           0,      0,    0,    0,    0,    0x0DCC },
{ "Germany",        "BR Nord",                                          0,      0,    0,    0,    0,    0x0DCD },
{ "Germany",        "NDR dreiländerweit",                               0,      0,    0,    0,    0,    0x0DD0 },
{ "Germany",        "NDR Hamburg",                                      0,      0,    0,    0,    0,    0x0DD1 },
{ "Germany",        "NDR Niedersachsen",                                0,      0,    0,    0,    0,    0x0DD2 },
{ "Germany",        "NDR Schleswig-Holstein",                           0,      0,    0,    0,    0,    0x0DD3 },
{ "Germany",        "NDR dreiländerweit",                               0,      0,    0,    0,    0,    0x0DD5 },
{ "Germany",        "NDR Hamburg",                                      0,      0,    0,    0,    0,    0x0DD6 },
{ "Germany",        "NDR Niedersachsen",                                0,      0,    0,    0,    0,    0x0DD7 },
{ "Germany",        "NDR Schleswig-Holstein",                           0,      0,    0,    0,    0,    0x0DD8 },
{ "Germany",        "RB",                                               0,      0,    0,    0,    0,    0x0DDA },
{ "Germany",        "SFB-1",                                            0,      0,    0,    0,    0,    0x0DDB },
{ "Germany",        "SDR BW",                                           0,      0,    0,    0,    0,    0x0DDD },
{ "Germany",        "SWF-1 RP",                                         0,      0,    0,    0,    0,    0x0DDE },
{ "Germany",        "SWR",                                              0,      0,    0,    0,    0,    0x0DE0 },
{ "Germany",        "SWR Saarland",                                     0,      0,    0,    0,    0,    0x0DE2 },
{ "Germany",        "SWR BW Süd",                                       0,      0,    0,    0,    0,    0x0DE3 },
{ "Germany",        "WDR-1",                                            0,      0,    0,    0,    0,    0x0DE5 },
{ "Germany",        "WDR-3 Bielefeld",                                  0,      0,    0,    0,    0,    0x0DE7 },
{ "Germany",        "WDR-3 Dortmund",                                   0,      0,    0,    0,    0,    0x0DE8 },
{ "Germany",        "WDR-3 Düsseldorf",                                 0,      0,    0,    0,    0,    0x0DE9 },
{ "Germany",        "WDR-3 Köln",                                       0,      0,    0,    0,    0,    0x0DEA },
{ "Germany",        "WDR-3 Münster",                                    0,      0,    0,    0,    0,    0x0DEB },
{ "Germany",        "SDR-1",                                            0,      0,    0,    0,    0,    0x0DEC },
{ "Germany",        "SWR BW Nord",                                      0,      0,    0,    0,    0,    0x0DED },
{ "Germany",        "SWR Mannheim",                                     0,      0,    0,    0,    0,    0x0DEE },
{ "Germany",        "SDR-1 + SWF-1",                                    0,      0,    0,    0,    0,    0x0DEF },
{ "Germany",        "SWF-1",                                            0,      0,    0,    0,    0,    0x0DF0 },
{ "Germany",        "NDR-1 Mecklenburg-Vorpommern",                     0,      0,    0,    0,    0,    0x0DF1 },
{ "Germany",        "NDR Mecklenburg-Vorpommern",                       0,      0,    0,    0,    0,    0x0DF2 },
{ "Germany",        "MDR-1 Sachsen",                                    0,      0,    0,    0,    0,    0x0DF3 },
{ "Germany",        "MDR Sachsen",                                      0,      0,    0,    0,    0,    0x0DF4 },
{ "Germany",        "MDR Dresden",                                      0,      0,    0,    0,    0,    0x0DF5 },
{ "Germany",        "MDR-1 Sachsen-Anhalt",                             0,      0,    0,    0,    0,    0x0DF6 },
{ "Germany",        "WDR Dortmund",                                     0,      0,    0,    0,    0,    0x0DF7 },
{ "Germany",        "MDR Sachsen-Anhalt",                               0,      0,    0,    0,    0,    0x0DF8 },
{ "Germany",        "MDR Magdeburg",                                    0,      0,    0,    0,    0,    0x0DF9 },
{ "Germany",        "MDR-1 Thüringen",                                  0,      0,    0,    0,    0,    0x0DFA },
{ "Germany",        "MDR Thüringen",                                    0,      0,    0,    0,    0,    0x0DFB },
{ "Germany",        "MDR Erfurt",                                       0,      0,    0,    0,    0,    0x0DFC },
{ "Germany",        "MDR-1",                                            0,      0,    0,    0,    0,    0x0DFD },

{ "Greece",         "ET-1",                                             0x3001, 0x21, 0x01, 0x31, 0x01, 0      },
{ "Greece",         "NET",                                              0x3002, 0x21, 0x02, 0x31, 0x02, 0      },
{ "Greece",         "ET-3",                                             0x3003, 0x21, 0x03, 0x31, 0x03, 0      },
{ "Greece",         "ET future use",                                    0x3004, 0x21, 0x04, 0x31, 0x04, 0      },
{ "Greece",         "ET future use",                                    0x3005, 0x21, 0x05, 0x31, 0x05, 0      },
{ "Greece",         "ET future use",                                    0x3006, 0x21, 0x06, 0x31, 0x06, 0      },
{ "Greece",         "ET future use",                                    0x3007, 0x21, 0x07, 0x31, 0x07, 0      },
{ "Greece",         "ET future use",                                    0x3008, 0x21, 0x08, 0x31, 0x08, 0      },
{ "Greece",         "ET future use",                                    0x3009, 0x21, 0x09, 0x31, 0x09, 0      },
{ "Greece",         "ET future use",                                    0x300A, 0x21, 0x0A, 0x31, 0x0A, 0      },
{ "Greece",         "ET future use",                                    0x300B, 0x21, 0x0B, 0x31, 0x0B, 0      },
{ "Greece",         "ET future use",                                    0x300C, 0x21, 0x0C, 0x31, 0x0C, 0      },
{ "Greece",         "ET future use",                                    0x300D, 0x21, 0x0D, 0x31, 0x0D, 0      },
{ "Greece",         "ET future use",                                    0x300E, 0x21, 0x0E, 0x31, 0x0E, 0      },
{ "Greece",         "ET future use",                                    0x300F, 0x21, 0x0F, 0x31, 0x0F, 0      },

{ "Hungary",        "MTV1",                                             0x3601, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV2",                                             0x3602, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV1 regional, Budapest",                          0x3611, 0,    0,    0,    0,    0      },
{ "Hungary",        "tv2 future use",                                   0x3620, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV1 regional, Pécs",                              0x3621, 0,    0,    0,    0,    0      },
{ "Hungary",        "tv2",                                              0x3622, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV1 regional, Szeged",                            0x3631, 0,    0,    0,    0,    0      },
{ "Hungary",        "Duna Televizio",                                   0x3636, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV1 regional, Szombathely",                       0x3641, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV1 regional, Debrecen",                          0x3651, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV1 regional, Miskolc",                           0x3661, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV1 future use",                                  0x3681, 0,    0,    0,    0,    0      },
{ "Hungary",        "MTV2 future use",                                  0x3682, 0,    0,    0,    0,    0      },

{ "Iceland",        "TV3",                                              0x3333, 0,    0,    0,    0,    0      },
{ "Iceland",        "RTE1",                                             0x3531, 0x42, 0x01, 0x32, 0x01, 0      },
{ "Iceland",        "Network 2",                                        0x3532, 0x42, 0x02, 0x32, 0x02, 0      },
{ "Iceland",        "Teilifis na Gaeilge",                              0x3533, 0x42, 0x03, 0x32, 0x03, 0      },
{ "Iceland",        "RTE future use",                                   0x3534, 0x42, 0x04, 0x32, 0x04, 0      },
{ "Iceland",        "RTE future use",                                   0x3535, 0x42, 0x05, 0x32, 0x05, 0      },
{ "Iceland",        "RTE future use",                                   0x3536, 0x42, 0x06, 0x32, 0x06, 0      },
{ "Iceland",        "RTE future use",                                   0x3537, 0x42, 0x07, 0x32, 0x07, 0      },
{ "Iceland",        "RTE future use",                                   0x3538, 0x42, 0x08, 0x32, 0x08, 0      },
{ "Iceland",        "RTE future use",                                   0x3539, 0x42, 0x09, 0x32, 0x09, 0      },
{ "Iceland",        "RTE future use",                                   0x353A, 0x42, 0x0A, 0x32, 0x0A, 0      },
{ "Iceland",        "RTE future use",                                   0x353B, 0x42, 0x0B, 0x32, 0x0B, 0      },
{ "Iceland",        "RTE future use",                                   0x353C, 0x42, 0x0C, 0x32, 0x0C, 0      },
{ "Iceland",        "RTE future use",                                   0x353D, 0x42, 0x0D, 0x32, 0x0D, 0      },
{ "Iceland",        "RTE future use",                                   0x353E, 0x42, 0x0E, 0x32, 0x0E, 0      },
{ "Iceland",        "RTE future use",                                   0x353F, 0x42, 0x0F, 0x32, 0x0F, 0      },
{ "Iceland",        "Rikisutvarpid-Sjonvarp",                           0x3541, 0,    0,    0,    0,    0      },

{ "Italy",          "RAI 1",                                            0x3901, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI 2",                                            0x3902, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI 3",                                            0x3903, 0,    0,    0,    0,    0      },
{ "Italy",          "Rete A",                                           0x3904, 0,    0,    0,    0,    0      },
{ "Italy",          "Canale Italia",                                    0x3905, 0x15, 0x05, 0,    0,    0      },
{ "Italy",          "Telenova",                                         0x3909, 0,    0,    0,    0,    0      },
{ "Italy",          "Arte",                                             0x390A, 0,    0,    0,    0,    0      },
{ "Italy",          "TRS TV",                                           0x3910, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Cinema Classic",                               0x3911, 0x15, 0x11, 0,    0,    0      },
{ "Italy",          "Sky Future use (canale 109)",                      0x3912, 0x15, 0x12, 0,    0,    0      },
{ "Italy",          "Sky Calcio 1",                                     0x3913, 0x15, 0x13, 0,    0,    0      },
{ "Italy",          "Sky Calcio 2",                                     0x3914, 0x15, 0x14, 0,    0,    0      },
{ "Italy",          "Sky Calcio 3",                                     0x3915, 0x15, 0x15, 0,    0,    0      },
{ "Italy",          "Sky Calcio 4",                                     0x3916, 0x15, 0x16, 0,    0,    0      },
{ "Italy",          "Sky Calcio 5",                                     0x3917, 0x15, 0x17, 0,    0,    0      },
{ "Italy",          "Sky Calcio 6",                                     0x3918, 0x15, 0x18, 0,    0,    0      },
{ "Italy",          "Sky Calcio 7",                                     0x3919, 0x15, 0x19, 0,    0,    0      },
{ "Italy",          "RaiNews24",                                        0x3920, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI Med",                                          0x3921, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI Sport",                                        0x3922, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI Educational",                                  0x3923, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI Edu Lab",                                      0x3924, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI Nettuno 1",                                    0x3925, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI Nettuno 2",                                    0x3926, 0,    0,    0,    0,    0      },
{ "Italy",          "Camera Deputati",                                  0x3927, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI Mosaico",                                      0x3928, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI future use",                                   0x3929, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI future use",                                   0x392A, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI future use",                                   0x392B, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI future use",                                   0x392C, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI future use",                                   0x392D, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI future use",                                   0x392E, 0,    0,    0,    0,    0      },
{ "Italy",          "RAI future use",                                   0x392F, 0,    0,    0,    0,    0      },
{ "Italy",          "Discovery Italy",                                  0x3930, 0,    0,    0,    0,    0      },
{ "Italy",          "MTV Italia",                                       0x3933, 0,    0,    0,    0,    0      },
{ "Italy",          "MTV Brand New",                                    0x3934, 0,    0,    0,    0,    0      },
{ "Italy",          "MTV Hits",                                         0x3935, 0,    0,    0,    0,    0      },
{ "Italy",          "RTV38",                                            0x3938, 0,    0,    0,    0,    0      },
{ "Italy",          "GAY TV",                                           0x3939, 0,    0,    0,    0,    0      },
{ "Italy",          "Video Italia",                                     0x3940, 0,    0,    0,    0,    0      },
{ "Italy",          "SAT 2000",                                         0x3941, 0,    0,    0,    0,    0      },
{ "Italy",          "Jimmy",                                            0x3942, 0x15, 0x42, 0,    0,    0      },
{ "Italy",          "Planet",                                           0x3943, 0x15, 0x43, 0,    0,    0      },
{ "Italy",          "Cartoon Network",                                  0x3944, 0x15, 0x44, 0,    0,    0      },
{ "Italy",          "Boomerang",                                        0x3945, 0x15, 0x45, 0,    0,    0      },
{ "Italy",          "CNN International",                                0x3946, 0x15, 0x46, 0,    0,    0      },
{ "Italy",          "Cartoon Network +1",                               0x3947, 0x15, 0x47, 0,    0,    0      },
{ "Italy",          "Sky Sports 3",                                     0x3948, 0x15, 0x48, 0,    0,    0      },
{ "Italy",          "Sky Diretta Gol",                                  0x3949, 0x15, 0x49, 0,    0,    0      },
{ "Italy",          "RAISat Album",                                     0x3950, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat Art",                                       0x3951, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat Cinema",                                    0x3952, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat Fiction",                                   0x3953, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat GamberoRosso",                              0x3954, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat Ragazzi",                                   0x3955, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat Show",                                      0x3956, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat G. Rosso interattivo",                      0x3957, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x3958, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x3959, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x395A, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x395B, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x395C, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x395D, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x395E, 0,    0,    0,    0,    0      },
{ "Italy",          "RAISat future use",                                0x395F, 0,    0,    0,    0,    0      },
{ "Italy",          "SCI FI CHANNEL",                                   0x3960, 0x15, 0x60, 0,    0,    0      },
{ "Italy",          "Discovery Civilisations",                          0x3961, 0,    0,    0,    0,    0      },
{ "Italy",          "Discovery Travel and Adventure",                   0x3962, 0,    0,    0,    0,    0      },
{ "Italy",          "Discovery Science",                                0x3963, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Meteo24",                                      0x3968, 0x15, 0x68, 0,    0,    0      },
{ "Italy",          "Sky Cinema 2",                                     0x3970, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Cinema 3",                                     0x3971, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Cinema Autore",                                0x3972, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Cinema Max",                                   0x3973, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Cinema 16:9",                                  0x3974, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Sports 2",                                     0x3975, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky TG24",                                         0x3976, 0,    0,    0,    0,    0      },
{ "Italy",          "Fox",                                              0x3977, 0x15, 0x77, 0,    0,    0      },
{ "Italy",          "Foxlife",                                          0x3978, 0x15, 0x78, 0,    0,    0      },
{ "Italy",          "National Geographic Channel",                      0x3979, 0x15, 0x79, 0,    0,    0      },
{ "Italy",          "A1",                                               0x3980, 0x15, 0x80, 0,    0,    0      },
{ "Italy",          "History Channel",                                  0x3981, 0x15, 0x81, 0,    0,    0      },
{ "Italy",          "FOX KIDS",                                         0x3985, 0,    0,    0,    0,    0      },
{ "Italy",          "PEOPLE TV – RETE 7",                               0x3986, 0,    0,    0,    0,    0      },
{ "Italy",          "FOX KIDS +1",                                      0x3987, 0,    0,    0,    0,    0      },
{ "Italy",          "LA7",                                              0x3988, 0,    0,    0,    0,    0      },
{ "Italy",          "PrimaTV",                                          0x3989, 0,    0,    0,    0,    0      },
{ "Italy",          "SportItalia",                                      0x398A, 0,    0,    0,    0,    0      },
{ "Italy",          "STUDIO UNIVERSAL",                                 0x3990, 0x15, 0x90, 0,    0,    0      },
{ "Italy",          "Marcopolo",                                        0x3991, 0x15, 0x91, 0,    0,    0      },
{ "Italy",          "Alice",                                            0x3992, 0x15, 0x92, 0,    0,    0      },
{ "Italy",          "Nuvolari",                                         0x3993, 0x15, 0x93, 0,    0,    0      },
{ "Italy",          "Leonardo",                                         0x3994, 0x15, 0x94, 0,    0,    0      },
{ "Italy",          "SUPERPIPPA CHANNEL",                               0x3996, 0x15, 0x96, 0,    0,    0      },
{ "Italy",          "Sky Sports 1",                                     0x3997, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Cinema 1",                                     0x3998, 0,    0,    0,    0,    0      },
{ "Italy",          "Tele+3",                                           0x3999, 0,    0,    0,    0,    0      },
{ "Italy",          "Sky Calcio 8",                                     0x39A0, 0x15, 0xA0, 0,    0,    0      },
{ "Italy",          "Sky Calcio 9",                                     0x39A1, 0x15, 0xA1, 0,    0,    0      },
{ "Italy",          "Sky Calcio 10",                                    0x39A2, 0x15, 0xA2, 0,    0,    0      },
{ "Italy",          "Sky Calcio 11",                                    0x39A3, 0x15, 0xA3, 0,    0,    0      },
{ "Italy",          "Sky Calcio 12",                                    0x39A4, 0x15, 0xA4, 0,    0,    0      },
{ "Italy",          "Sky Calcio 13",                                    0x39A5, 0x15, 0xA5, 0,    0,    0      },
{ "Italy",          "Sky Calcio 14",                                    0x39A6, 0x15, 0xA6, 0,    0,    0      },
{ "Italy",          "Telesanterno",                                     0x39A7, 0x15, 0xA7, 0,    0,    0      },
{ "Italy",          "Telecentro",                                       0x39A8, 0x15, 0xA8, 0,    0,    0      },
{ "Italy",          "Telestense",                                       0x39A9, 0x15, 0xA9, 0,    0,    0      },
{ "Italy",          "Disney Channel +1",                                0x39B0, 0x15, 0xB0, 0,    0,    0      },
{ "Italy",          "Sailing Channel",                                  0x39B1, 0,    0,    0,    0,    0      },
{ "Italy",          "Disney Channel",                                   0x39B2, 0x15, 0xB2, 0,    0,    0      },
{ "Italy",          "7 Gold-Sestra Rete",                               0x39B3, 0x15, 0xB3, 0,    0,    0      },
{ "Italy",          "Rete 8-VGA",                                       0x39B4, 0x15, 0xB4, 0,    0,    0      },
{ "Italy",          "Nuovarete",                                        0x39B5, 0x15, 0xB5, 0,    0,    0      },
{ "Italy",          "Radio Italia TV",                                  0x39B6, 0x15, 0xB6, 0,    0,    0      },
{ "Italy",          "Rete 7",                                           0x39B7, 0x15, 0xB7, 0,    0,    0      },
{ "Italy",          "E! Entertainment Television",                      0x39B8, 0x15, 0xB8, 0,    0,    0      },
{ "Italy",          "Toon Disney",                                      0x39B9, 0x15, 0xB9, 0,    0,    0      },
{ "Italy",          "Bassano TV",                                       0x39C7, 0x15, 0xC7, 0,    0,    0      },
{ "Italy",          "ESPN Classic Sport",                               0x39C8, 0x15, 0xC8, 0,    0,    0      },
{ "Italy",          "VIDEOLINA",                                        0x39CA, 0,    0,    0,    0,    0      },
{ "Italy",          "Mediaset Premium 1",                               0x39D2, 0x15, 0xD2, 0,    0,    0      },
{ "Italy",          "Mediaset Premium 2",                               0x39D3, 0x15, 0xD3, 0,    0,    0      },
{ "Italy",          "Mediaset Premium 3",                               0x39D4, 0x15, 0xD4, 0,    0,    0      },
{ "Italy",          "Mediaset Premium 4",                               0x39D5, 0x15, 0xD5, 0,    0,    0      },
{ "Italy",          "BOING",                                            0x39D6, 0x15, 0xD6, 0,    0,    0      },
{ "Italy",          "Playlist Italia",                                  0x39D7, 0x15, 0xD7, 0,    0,    0      },
{ "Italy",          "MATCH MUSIC",                                      0x39D8, 0x15, 0xD8, 0,    0,    0      },
{ "Italy",          "National Geographic +1",                           0x39E1, 0x15, 0xE1, 0,    0,    0      },
{ "Italy",          "History Channel +1",                               0x39E2, 0x15, 0xE2, 0,    0,    0      },
{ "Italy",          "Sky TV",                                           0x39E3, 0x15, 0xE3, 0,    0,    0      },
{ "Italy",          "GXT",                                              0x39E4, 0x15, 0xE4, 0,    0,    0      },
{ "Italy",          "Playhouse Disney",                                 0x39E5, 0x15, 0xE5, 0,    0,    0      },
{ "Italy",          "Sky Canale 224",                                   0x39E6, 0x15, 0xE6, 0,    0,    0      },
{ "Italy",          "Rete 4",                                           0xFA04, 0,    0,    0,    0,    0      },
{ "Italy",          "Canale 5",                                         0xFA05, 0,    0,    0,    0,    0      },
{ "Italy",          "Italia 1",                                         0xFA06, 0,    0,    0,    0,    0      },

{ "Luxembourg",     "RTL Télé Lëtzebuerg",                              0x4000, 0,    0,    0,    0,    0      },

{ "Netherlands",    "Nederland 1",                                      0x3101, 0x48, 0x01, 0x38, 0x01, 0      },
{ "Netherlands",    "Nederland 2",                                      0x3102, 0x48, 0x02, 0x38, 0x02, 0      },
{ "Netherlands",    "Nederland 3",                                      0x3103, 0x48, 0x03, 0x38, 0x03, 0      },
{ "Netherlands",    "RTL 4",                                            0x3104, 0x48, 0x04, 0x38, 0x04, 0      },
{ "Netherlands",    "RTL 5",                                            0x3105, 0x48, 0x05, 0x38, 0x05, 0      },
{ "Netherlands",    "Yorin",                                            0x3106, 0x48, 0x06, 0x38, 0x06, 0      },
{ "Netherlands",    "NOS future use",                                   0x3107, 0x48, 0x07, 0x38, 0x07, 0      },
{ "Netherlands",    "NOS future use",                                   0x3108, 0x48, 0x08, 0x38, 0x08, 0      },
{ "Netherlands",    "NOS future use",                                   0x3109, 0x48, 0x09, 0x38, 0x09, 0      },
{ "Netherlands",    "NOS future use",                                   0x310A, 0x48, 0x0A, 0x38, 0x0A, 0      },
{ "Netherlands",    "NOS future use",                                   0x310B, 0x48, 0x0B, 0x38, 0x0B, 0      },
{ "Netherlands",    "NOS future use",                                   0x310C, 0x48, 0x0C, 0x38, 0x0C, 0      },
{ "Netherlands",    "NOS future use",                                   0x310D, 0x48, 0x0D, 0x38, 0x0D, 0      },
{ "Netherlands",    "NOS future use",                                   0x310E, 0x48, 0x0E, 0x38, 0x0E, 0      },
{ "Netherlands",    "NOS future use",                                   0x310F, 0x48, 0x0F, 0x38, 0x0F, 0      },
{ "Netherlands",    "NOS future use",                                   0x3110, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3111, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3112, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3113, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3114, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3115, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3116, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3117, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3118, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x3119, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x311A, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x311B, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x311C, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x311D, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x311E, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NOS future use",                                   0x311F, 0,    0,    0,    0,    0      },
{ "Netherlands",    "The BOX",                                          0x3120, 0x48, 0x20, 0x38, 0x20, 0      },
{ "Netherlands",    "Discovery Netherlands",                            0x3121, 0,    0,    0,    0,    0      },
{ "Netherlands",    "Nickelodeon",                                      0x3122, 0x48, 0x22, 0x38, 0x22, 0      },
{ "Netherlands",    "Animal Planet Benelux",                            0x3123, 0,    0,    0,    0,    0      },
{ "Netherlands",    "TALPA TV",                                         0x3124, 0,    0,    0,    0,    0      },
{ "Netherlands",    "NET5",                                             0x3125, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS6",                                             0x3126, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x3127, 0,    0,    0,    0,    0      },
{ "Netherlands",    "V8",                                               0x3128, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x3129, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x312A, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x312B, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x312C, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x312D, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x312E, 0,    0,    0,    0,    0      },
{ "Netherlands",    "SBS future use",                                   0x312F, 0,    0,    0,    0,    0      },
{ "Netherlands",    "TMF (Netherlands service)",                        0x3130, 0,    0,    0,    0,    0      },
{ "Netherlands",    "TMF (Belgian Flanders service)",                   0x3131, 0,    0,    0,    0,    0      },
{ "Netherlands",    "MTV NL",                                           0x3132, 0,    0,    0,    0,    0      },
{ "Netherlands",    "RNN7",                                             0x3137, 0,    0,    0,    0,    0      },

{ "Norway",         "NRK1",                                             0x4701, 0,    0,    0,    0,    0      },
{ "Norway",         "TV 2",                                             0x4702, 0,    0,    0,    0,    0      },
{ "Norway",         "NRK2",                                             0x4703, 0,    0,    0,    0,    0      },
{ "Norway",         "TV Norge",                                         0x4704, 0,    0,    0,    0,    0      },
{ "Norway",         "Discovery Nordic",                                 0x4720, 0,    0,    0,    0,    0      },

{ "Poland",         "TVP1",                                             0x4801, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP2",                                             0x4802, 0,    0,    0,    0,    0      },
{ "Poland",         "TV Polonia",                                       0x4810, 0,    0,    0,    0,    0      },
{ "Poland",         "TVN",                                              0x4820, 0,    0,    0,    0,    0      },
{ "Poland",         "TVN Siedem",                                       0x4821, 0,    0,    0,    0,    0      },
{ "Poland",         "TVN24",                                            0x4822, 0,    0,    0,    0,    0      },
{ "Poland",         "Discovery Poland",                                 0x4830, 0,    0,    0,    0,    0      },
{ "Poland",         "Animal Planet",                                    0x4831, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Warszawa",                                     0x4880, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Bialystok",                                    0x4881, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Bydgoszcz",                                    0x4882, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Gdansk",                                       0x4883, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Katowice",                                     0x4884, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Krakow",                                       0x4886, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Lublin",                                       0x4887, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Lodz",                                         0x4888, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Rzeszow",                                      0x4890, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Poznan",                                       0x4891, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Szczecin",                                     0x4892, 0,    0,    0,    0,    0      },
{ "Poland",         "TVP Wroclaw",                                      0x4893, 0,    0,    0,    0,    0      },

{ "Portugal",       "RTP1",                                             0x3510, 0,    0,    0,    0,    0      },
{ "Portugal",       "RTP2",                                             0x3511, 0,    0,    0,    0,    0      },
{ "Portugal",       "RTPAF",                                            0x3512, 0,    0,    0,    0,    0      },
{ "Portugal",       "RTPI",                                             0x3513, 0,    0,    0,    0,    0      },
{ "Portugal",       "RTPAZ",                                            0x3514, 0,    0,    0,    0,    0      },
{ "Portugal",       "RTPM",                                             0x3515, 0,    0,    0,    0,    0      },
{ "Portugal",       "Future use",                                       0x3516, 0,    0,    0,    0,    0      },
{ "Portugal",       "Future use",                                       0x3517, 0,    0,    0,    0,    0      },
{ "Portugal",       "Future use",                                       0x3518, 0,    0,    0,    0,    0      },
{ "Portugal",       "Future use",                                       0x3519, 0,    0,    0,    0,    0      },

{ "San Marino",     "RTV",                                              0x3781, 0,    0,    0,    0,    0      },

{ "Slovakia",       "STV1",                                             0x42A1, 0x35, 0xA1, 0x35, 0x21, 0      },
{ "Slovakia",       "STV2",                                             0x42A2, 0x35, 0xA2, 0x35, 0x22, 0      },
{ "Slovakia",       "STV1 Regional, Košice",                            0x42A3, 0x35, 0xA3, 0x35, 0x23, 0      },
{ "Slovakia",       "STV2 Regional, Košice",                            0x42A4, 0x35, 0xA4, 0x35, 0x24, 0      },
{ "Slovakia",       "STV1 Regional, B. Bystrica",                       0x42A5, 0x35, 0xA5, 0x35, 0x25, 0      },
{ "Slovakia",       "STV2 Regional, B. Bystrica",                       0x42A6, 0x35, 0xA6, 0x35, 0x26, 0      },
{ "Slovakia",       "future use",                                       0x42A7, 0x35, 0xA7, 0x35, 0x27, 0      },
{ "Slovakia",       "future use",                                       0x42A8, 0x35, 0xA8, 0x35, 0x28, 0      },
{ "Slovakia",       "future use",                                       0x42A9, 0x35, 0xA9, 0x35, 0x29, 0      },
{ "Slovakia",       "future use",                                       0x42AA, 0x35, 0xAA, 0x35, 0x2A, 0      },
{ "Slovakia",       "future use",                                       0x42AB, 0x35, 0xAB, 0x35, 0x2B, 0      },
{ "Slovakia",       "future use",                                       0x42AC, 0x35, 0xAC, 0x35, 0x2C, 0      },
{ "Slovakia",       "future use",                                       0x42AD, 0x35, 0xAD, 0x35, 0x2D, 0      },
{ "Slovakia",       "future use",                                       0x42AE, 0x35, 0xAE, 0x35, 0x2E, 0      },
{ "Slovakia",       "future use",                                       0x42AF, 0x35, 0xAF, 0x35, 0x2F, 0      },

{ "Slovenia",       "SLO1",                                             0xAAE1, 0,    0,    0,    0,    0      },
{ "Slovenia",       "SLO2",                                             0xAAE2, 0,    0,    0,    0,    0      },
{ "Slovenia",       "KC",                                               0xAAE3, 0,    0,    0,    0,    0      },
{ "Slovenia",       "TLM",                                              0xAAE4, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAE5, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAE6, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAE7, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAE8, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAE9, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAEA, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAEB, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAEC, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAED, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAEE, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAEF, 0,    0,    0,    0,    0      },
{ "Slovenia",       "SLO3",                                             0xAAF1, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF2, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF3, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF4, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF5, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF6, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF7, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF8, 0,    0,    0,    0,    0      },
{ "Slovenia",       "future use",                                       0xAAF9, 0,    0,    0,    0,    0      },

{ "Spain",          "ETB 2",                                            0x3402, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9",                                          0x3403, 0,    0,    0,    0,    0      },
{ "Spain",          "PUNT 2",                                           0x3404, 0,    0,    0,    0,    0      },
{ "Spain",          "CCV",                                              0x3405, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 NEWS 24H Future use",                      0x3406, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 Future Use",                               0x3407, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x3408, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x3409, 0,    0,    0,    0,    0      },
{ "Spain",          "Arte",                                             0x340A, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x340B, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x340C, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x340D, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x340E, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x340F, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x3410, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x3411, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x3412, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x3413, 0,    0,    0,    0,    0      },
{ "Spain",          "CANAL 9 DVB Future Use",                           0x3414, 0,    0,    0,    0,    0      },
{ "Spain",          "TVE1",                                             0x3E00, 0,    0,    0,    0,    0      },
{ "Spain",          "ETB 1",                                            0xBA01, 0,    0,    0,    0,    0      },
{ "Spain",          "TV3",                                              0xCA03, 0,    0,    0,    0,    0      },
{ "Spain",          "C33",                                              0xCA33, 0,    0,    0,    0,    0      },
{ "Spain",          "TVE2",                                             0xE100, 0,    0,    0,    0,    0      },
{ "Spain",          "TVE Internacional Europa",                         0xE200, 0,    0,    0,    0,    0      },
{ "Spain",          "Tele5",                                            0xE500, 0x1F, 0xE5, 0,    0,    0      },

{ "Sweden",         "SVT Test Txmns",                                   0x4600, 0x4E, 0x00, 0x3E, 0x00, 0      },
{ "Sweden",         "SVT 1",                                            0x4601, 0x4E, 0x01, 0x3E, 0x01, 0      },
{ "Sweden",         "SVT 2",                                            0x4602, 0x4E, 0x02, 0x3E, 0x02, 0      },
{ "Sweden",         "SVT future use",                                   0x4603, 0x4E, 0x03, 0x3E, 0x03, 0      },
{ "Sweden",         "SVT future use",                                   0x4604, 0x4E, 0x04, 0x3E, 0x04, 0      },
{ "Sweden",         "SVT future use",                                   0x4605, 0x4E, 0x05, 0x3E, 0x05, 0      },
{ "Sweden",         "SVT future use",                                   0x4606, 0x4E, 0x06, 0x3E, 0x06, 0      },
{ "Sweden",         "SVT future use",                                   0x4607, 0x4E, 0x07, 0x3E, 0x07, 0      },
{ "Sweden",         "SVT future use",                                   0x4608, 0x4E, 0x08, 0x3E, 0x08, 0      },
{ "Sweden",         "SVT future use",                                   0x4609, 0x4E, 0x09, 0x3E, 0x09, 0      },
{ "Sweden",         "SVT future use",                                   0x460A, 0x4E, 0x0A, 0x3E, 0x0A, 0      },
{ "Sweden",         "SVT future use",                                   0x460B, 0x4E, 0x0B, 0x3E, 0x0B, 0      },
{ "Sweden",         "SVT future use",                                   0x460C, 0x4E, 0x0C, 0x3E, 0x0C, 0      },
{ "Sweden",         "SVT future use",                                   0x460D, 0x4E, 0x0D, 0x3E, 0x0D, 0      },
{ "Sweden",         "SVT future use",                                   0x460E, 0x4E, 0x0E, 0x3E, 0x0E, 0      },
{ "Sweden",         "SVT future use",                                   0x460F, 0x4E, 0x0F, 0x3E, 0x0F, 0      },
{ "Sweden",         "TV 4",                                             0x4640, 0x4E, 0x40, 0x3E, 0x40, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4641, 0x4E, 0x41, 0x3E, 0x41, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4642, 0x4E, 0x42, 0x3E, 0x42, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4643, 0x4E, 0x43, 0x3E, 0x43, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4644, 0x4E, 0x44, 0x3E, 0x44, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4645, 0x4E, 0x45, 0x3E, 0x45, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4646, 0x4E, 0x46, 0x3E, 0x46, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4647, 0x4E, 0x47, 0x3E, 0x47, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4648, 0x4E, 0x48, 0x3E, 0x48, 0      },
{ "Sweden",         "TV 4 future use",                                  0x4649, 0x4E, 0x49, 0x3E, 0x49, 0      },
{ "Sweden",         "TV 4 future use",                                  0x464A, 0x4E, 0x4A, 0x3E, 0x4A, 0      },
{ "Sweden",         "TV 4 future use",                                  0x464B, 0x4E, 0x4B, 0x3E, 0x4B, 0      },
{ "Sweden",         "TV 4 future use",                                  0x464C, 0x4E, 0x4C, 0x3E, 0x4C, 0      },
{ "Sweden",         "TV 4 future use",                                  0x464D, 0x4E, 0x4D, 0x3E, 0x4D, 0      },
{ "Sweden",         "TV 4 future use",                                  0x464E, 0x4E, 0x4E, 0x3E, 0x4E, 0      },
{ "Sweden",         "TV 4 future use",                                  0x464F, 0x4E, 0x4F, 0x3E, 0x4F, 0      },

{ "Switzerland",    "SF 1",                                             0x4101, 0x24, 0xC1, 0x34, 0x41, 0x04C1 },
{ "Switzerland",    "TSR 1",                                            0x4102, 0x24, 0xC2, 0x34, 0x42, 0x04C2 },
{ "Switzerland",    "TSI 1",                                            0x4103, 0x24, 0xC3, 0x34, 0x43, 0x04C3 },
{ "Switzerland",    "SF 2",                                             0x4107, 0x24, 0xC7, 0x34, 0x47, 0x04C7 },
{ "Switzerland",    "TSI 2",                                            0x4109, 0x24, 0xC9, 0x34, 0x49, 0x04C9 },
{ "Switzerland",    "TSR 2",                                            0x4108, 0x24, 0xC8, 0x34, 0x48, 0x04C8 },
{ "Switzerland",    "SAT ACCESS",                                       0x410A, 0x24, 0xCA, 0x34, 0x4A, 0x04CA },
{ "Switzerland",    "U1",                                               0x4121, 0x24, 0x21, 0,    0,    0x0421 },
{ "Switzerland",    "TeleZüri",                                         0,      0,    0,    0,    0,    0x0481 },
{ "Switzerland",    "Teleclub Abonnements-Fernsehen",                   0,      0,    0,    0,    0,    0x0482 },
{ "Switzerland",    "TeleBern",                                         0,      0,    0,    0,    0,    0x0484 },
{ "Switzerland",    "Tele M1",                                          0,      0,    0,    0,    0,    0x0485 },
{ "Switzerland",    "Star TV",                                          0,      0,    0,    0,    0,    0x0486 },
{ "Switzerland",    "Pro Sieben",                                       0,      0,    0,    0,    0,    0x0487 },
{ "Switzerland",    "TopTV",                                            0,      0,    0,    0,    0,    0x0488 },

{ "Turkey",         "TRT-1",                                            0x9001, 0x43, 0x01, 0x33, 0x01, 0      },
{ "Turkey",         "TRT-2",                                            0x9002, 0x43, 0x02, 0x33, 0x02, 0      },
{ "Turkey",         "TRT-3",                                            0x9003, 0x43, 0x03, 0x33, 0x03, 0      },
{ "Turkey",         "TRT-4",                                            0x9004, 0x43, 0x04, 0x33, 0x04, 0      },
{ "Turkey",         "TRT-INT",                                          0x9005, 0x43, 0x05, 0x33, 0x05, 0      },
{ "Turkey",         "AVRASYA",                                          0x9006, 0x43, 0x06, 0x33, 0x06, 0      },
{ "Turkey",         "Show TV",                                          0x9007, 0,    0,    0,    0,    0      },
{ "Turkey",         "Cine 5",                                           0x9008, 0,    0,    0,    0,    0      },
{ "Turkey",         "Super Sport",                                      0x9009, 0,    0,    0,    0,    0      },
{ "Turkey",         "ATV",                                              0x900A, 0,    0,    0,    0,    0      },
{ "Turkey",         "KANAL D",                                          0x900B, 0,    0,    0,    0,    0      },
{ "Turkey",         "EURO D",                                           0x900C, 0,    0,    0,    0,    0      },
{ "Turkey",         "EKO TV",                                           0x900D, 0,    0,    0,    0,    0      },
{ "Turkey",         "BRAVO TV",                                         0x900E, 0,    0,    0,    0,    0      },
{ "Turkey",         "GALAKSI TV",                                       0x900F, 0,    0,    0,    0,    0      },
{ "Turkey",         "FUN TV",                                           0x9010, 0,    0,    0,    0,    0      },
{ "Turkey",         "KANAL D future use",                               0x9012, 0,    0,    0,    0,    0      },
{ "Turkey",         "KANAL D future use",                               0x9013, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV",                                          0x9020, 0,    0,    0,    0,    0      },
{ "Turkey",         "STARMAX",                                          0x9021, 0,    0,    0,    0,    0      },
{ "Turkey",         "KANAL 6",                                          0x9022, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR 4",                                           0x9023, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR 5",                                           0x9024, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR 6",                                           0x9025, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR 7",                                           0x9026, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR 8",                                           0x9027, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9028, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9029, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9030, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9031, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9032, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9033, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9034, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9035, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9036, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9037, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9038, 0,    0,    0,    0,    0      },
{ "Turkey",         "STAR TV future use",                               0x9039, 0,    0,    0,    0,    0      },
{ "Turkey",         "TEMPO TV",                                         0x9011, 0,    0,    0,    0,    0      },
{ "Turkey",         "TGRT",                                             0x9014, 0,    0,    0,    0,    0      },

{ "UK",             "CNNI",                                             0x01F2, 0x5B, 0xF1, 0x3B, 0x71, 0      },
{ "UK",             "MERIDIAN",                                         0x10E4, 0x2C, 0x34, 0x3C, 0x34, 0      },
{ "UK",             "CHANNEL 5 (2)",                                    0x1609, 0x2C, 0x09, 0x3C, 0x09, 0      },
{ "UK",             "WESTCOUNTRY TV",                                   0x25D0, 0x2C, 0x30, 0x3C, 0x30, 0      },
{ "UK",             "WESTCOUNTRY future use",                           0x25D1, 0x5B, 0xE8, 0x3B, 0x68, 0      },
{ "UK",             "WESTCOUNTRY future use",                           0x25D2, 0x5B, 0xE9, 0x3B, 0x69, 0      },
{ "UK",             "CHANNEL 5 (3)",                                    0x28EB, 0x2C, 0x2B, 0x3C, 0x2B, 0      },
{ "UK",             "CENTRAL TV",                                       0x2F27, 0x2C, 0x37, 0x3C, 0x37, 0      },
{ "UK",             "National Geographic Channel",                      0x320B, 0,    0,    0,    0,    0      },
{ "UK",             "SSVC",                                             0x37E5, 0x2C, 0x25, 0x3C, 0x25, 0      },
{ "UK",             "UK GOLD",                                          0x4401, 0x5B, 0xFA, 0x3B, 0x7A, 0      },
{ "UK",             "UK LIVING",                                        0x4402, 0x2C, 0x01, 0x3C, 0x01, 0      },
{ "UK",             "WIRE TV",                                          0x4403, 0x2C, 0x3C, 0x3C, 0x3C, 0      },
{ "UK",             "CHILDREN'S CHANNEL",                               0x4404, 0x5B, 0xF0, 0x3B, 0x70, 0      },
{ "UK",             "BRAVO",                                            0x4405, 0x5B, 0xEF, 0x3B, 0x6F, 0      },
{ "UK",             "LEARNING CHANNEL",                                 0x4406, 0x5B, 0xF7, 0x3B, 0x77, 0      },
{ "UK",             "DISCOVERY",                                        0x4407, 0x5B, 0xF2, 0x3B, 0x72, 0      },
{ "UK",             "Live TV",                                          0x4409, 0x5B, 0xF8, 0x3B, 0x78, 0      },
{ "UK",             "FAMILY CHANNEL",                                   0x4408, 0x5B, 0xF3, 0x3B, 0x73, 0      },
{ "UK",             "UK GOLD future use",                               0x4411, 0x5B, 0xFB, 0x3B, 0x7B, 0      },
{ "UK",             "UK GOLD future use",                               0x4412, 0x5B, 0xFC, 0x3B, 0x7C, 0      },
{ "UK",             "UK GOLD future use",                               0x4413, 0x5B, 0xFD, 0x3B, 0x7D, 0      },
{ "UK",             "UK GOLD future use",                               0x4414, 0x5B, 0xFE, 0x3B, 0x7E, 0      },
{ "UK",             "UK GOLD future use",                               0x4415, 0x5B, 0xFF, 0x3B, 0x7F, 0      },
{ "UK",             "Discovery Home & Leisure",                         0x4420, 0,    0,    0,    0,    0      },
{ "UK",             "Animal Planet",                                    0x4421, 0,    0,    0,    0,    0      },
{ "UK",             "BBC2",                                             0x4440, 0x2C, 0x40, 0x3C, 0x40, 0      },
{ "UK",             "BBC1 NI",                                          0x4441, 0x2C, 0x41, 0x3C, 0x41, 0      },
{ "UK",             "BBC2 Wales",                                       0x4442, 0x2C, 0x42, 0x3C, 0x42, 0      },
{ "UK",             "BBC1 future 01",                                   0x4443, 0x2C, 0x43, 0x3C, 0x43, 0      },
{ "UK",             "BBC2 Scotland",                                    0x4444, 0x2C, 0x44, 0x3C, 0x44, 0      },
{ "UK",             "BBC1 future 02",                                   0x4445, 0x2C, 0x45, 0x3C, 0x45, 0      },
{ "UK",             "BBC2 future 03",                                   0x4446, 0x2C, 0x46, 0x3C, 0x46, 0      },
{ "UK",             "BBC1 future 04",                                   0x4447, 0x2C, 0x47, 0x3C, 0x47, 0      },
{ "UK",             "BBC2 future 05",                                   0x4448, 0x2C, 0x48, 0x3C, 0x48, 0      },
{ "UK",             "BBC1 future 06",                                   0x4449, 0x2C, 0x49, 0x3C, 0x49, 0      },
{ "UK",             "BBC2 future 07",                                   0x444A, 0x2C, 0x4A, 0x3C, 0x4A, 0      },
{ "UK",             "BBC1 future 08",                                   0x444B, 0x2C, 0x4B, 0x3C, 0x4B, 0      },
{ "UK",             "BBC2 future 09",                                   0x444C, 0x2C, 0x4C, 0x3C, 0x4C, 0      },
{ "UK",             "BBC1 future 10",                                   0x444D, 0x2C, 0x4D, 0x3C, 0x4D, 0      },
{ "UK",             "BBC2 future 11",                                   0x444E, 0x2C, 0x4E, 0x3C, 0x4E, 0      },
{ "UK",             "BBC1 future 12",                                   0x444F, 0x2C, 0x4F, 0x3C, 0x4F, 0      },
{ "UK",             "BBC2 future 13",                                   0x4450, 0x2C, 0x50, 0x3C, 0x50, 0      },
{ "UK",             "BBC1 future 14",                                   0x4451, 0x2C, 0x51, 0x3C, 0x51, 0      },
{ "UK",             "BBC2 future 15",                                   0x4452, 0x2C, 0x52, 0x3C, 0x52, 0      },
{ "UK",             "BBC1 future 16",                                   0x4453, 0x2C, 0x53, 0x3C, 0x53, 0      },
{ "UK",             "BBC2 future 17",                                   0x4454, 0x2C, 0x54, 0x3C, 0x54, 0      },
{ "UK",             "BBC1 future 18",                                   0x4455, 0x2C, 0x55, 0x3C, 0x55, 0      },
{ "UK",             "BBC2 future 19",                                   0x4456, 0x2C, 0x56, 0x3C, 0x56, 0      },
{ "UK",             "BBC World",                                        0x4457, 0x2C, 0x57, 0x3C, 0x57, 0      },
{ "UK",             "BBC Worldwide future 01",                          0x4458, 0x2C, 0x58, 0x3C, 0x58, 0      },
{ "UK",             "BBC Worldwide future 02",                          0x4459, 0x2C, 0x59, 0x3C, 0x59, 0      },
{ "UK",             "BBC Worldwide future 03",                          0x445A, 0x2C, 0x5A, 0x3C, 0x5A, 0      },
{ "UK",             "BBC Worldwide future 04",                          0x445B, 0x2C, 0x5B, 0x3C, 0x5B, 0      },
{ "UK",             "BBC Worldwide future 05",                          0x445C, 0x2C, 0x5C, 0x3C, 0x5C, 0      },
{ "UK",             "BBC Worldwide future 06",                          0x445D, 0x2C, 0x5D, 0x3C, 0x5D, 0      },
{ "UK",             "BBC Worldwide future 07",                          0x445E, 0x2C, 0x5E, 0x3C, 0x5E, 0      },
{ "UK",             "BBC Worldwide future 08",                          0x445F, 0x2C, 0x5F, 0x3C, 0x5F, 0      },
{ "UK",             "BBC Worldwide future 09",                          0x4460, 0x2C, 0x60, 0x3C, 0x60, 0      },
{ "UK",             "BBC Worldwide future 10",                          0x4461, 0x2C, 0x61, 0x3C, 0x61, 0      },
{ "UK",             "BBC Worldwide future 11",                          0x4462, 0x2C, 0x62, 0x3C, 0x62, 0      },
{ "UK",             "BBC Worldwide future 12",                          0x4463, 0x2C, 0x63, 0x3C, 0x63, 0      },
{ "UK",             "BBC Worldwide future 13",                          0x4464, 0x2C, 0x64, 0x3C, 0x64, 0      },
{ "UK",             "BBC Worldwide future 14",                          0x4465, 0x2C, 0x65, 0x3C, 0x65, 0      },
{ "UK",             "BBC Worldwide future 15",                          0x4466, 0x2C, 0x66, 0x3C, 0x66, 0      },
{ "UK",             "BBC Worldwide future 16",                          0x4467, 0x2C, 0x67, 0x3C, 0x67, 0      },
{ "UK",             "BBC Prime",                                        0x4468, 0x2C, 0x68, 0x3C, 0x68, 0      },
{ "UK",             "BBC News 24",                                      0x4469, 0x2C, 0x69, 0x3C, 0x69, 0      },
{ "UK",             "BBC2 future 18",                                   0x446A, 0x2C, 0x6A, 0x3C, 0x6A, 0      },
{ "UK",             "BBC1 future 17",                                   0x446B, 0x2C, 0x6B, 0x3C, 0x6B, 0      },
{ "UK",             "BBC2 future 16",                                   0x446C, 0x2C, 0x6C, 0x3C, 0x6C, 0      },
{ "UK",             "BBC1 future 15",                                   0x446D, 0x2C, 0x6D, 0x3C, 0x6D, 0      },
{ "UK",             "BBC2 future 14",                                   0x446E, 0x2C, 0x6E, 0x3C, 0x6E, 0      },
{ "UK",             "BBC1 future 13",                                   0x446F, 0x2C, 0x6F, 0x3C, 0x6F, 0      },
{ "UK",             "BBC2 future 12",                                   0x4470, 0x2C, 0x70, 0x3C, 0x70, 0      },
{ "UK",             "BBC1 future 11",                                   0x4471, 0x2C, 0x71, 0x3C, 0x71, 0      },
{ "UK",             "BBC2 future 10",                                   0x4472, 0x2C, 0x72, 0x3C, 0x72, 0      },
{ "UK",             "BBC1 future 09",                                   0x4473, 0x2C, 0x73, 0x3C, 0x73, 0      },
{ "UK",             "BBC2 future 08",                                   0x4474, 0x2C, 0x74, 0x3C, 0x74, 0      },
{ "UK",             "BBC1 future 07",                                   0x4475, 0x2C, 0x75, 0x3C, 0x75, 0      },
{ "UK",             "BBC2 future 06",                                   0x4476, 0x2C, 0x76, 0x3C, 0x76, 0      },
{ "UK",             "BBC1 future 05",                                   0x4477, 0x2C, 0x77, 0x3C, 0x77, 0      },
{ "UK",             "BBC2 future 04",                                   0x4478, 0x2C, 0x78, 0x3C, 0x78, 0      },
{ "UK",             "BBC1 future 03",                                   0x4479, 0x2C, 0x79, 0x3C, 0x79, 0      },
{ "UK",             "BBC2 future 02",                                   0x447A, 0x2C, 0x7A, 0x3C, 0x7A, 0      },
{ "UK",             "BBC1 Scotland",                                    0x447B, 0x2C, 0x7B, 0x3C, 0x7B, 0      },
{ "UK",             "BBC2 future 01",                                   0x447C, 0x2C, 0x7C, 0x3C, 0x7C, 0      },
{ "UK",             "BBC1 Wales",                                       0x447D, 0x2C, 0x7D, 0x3C, 0x7D, 0      },
{ "UK",             "BBC2 NI",                                          0x447E, 0x2C, 0x7E, 0x3C, 0x7E, 0      },
{ "UK",             "BBC1",                                             0x447F, 0x2C, 0x7F, 0x3C, 0x7F, 0      },
{ "UK",             "TNT / Cartoon Network",                            0x44C1, 0,    0,    0,    0,    0      },
{ "UK",             "DISNEY CHANNEL UK",                                0x44D1, 0x5B, 0xCC, 0x3B, 0x4C, 0      },
{ "UK",             "MTV",                                              0x4D54, 0x2C, 0x14, 0x3C, 0x14, 0      },
{ "UK",             "MTV future use",                                   0x4D55, 0x2C, 0x33, 0x3C, 0x33, 0      },
{ "UK",             "MTV future use",                                   0x4D56, 0x2C, 0x36, 0x3C, 0x36, 0      },
{ "UK",             "VH-1 future use",                                  0x4D57, 0x2C, 0x22, 0x3C, 0x22, 0      },
{ "UK",             "VH-1",                                             0x4D58, 0x2C, 0x20, 0x3C, 0x20, 0      },
{ "UK",             "VH-1 (German language)",                           0x4D59, 0x2C, 0x21, 0x3C, 0x21, 0      },
{ "UK",             "GRANADA PLUS",                                     0x4D5A, 0x5B, 0xF4, 0x3B, 0x74, 0      },
{ "UK",             "GRANADA Timeshare",                                0x4D5B, 0x5B, 0xF5, 0x3B, 0x75, 0      },
{ "UK",             "NBC Europe future use",                            0x5343, 0x2C, 0x03, 0x3C, 0x03, 0      },
{ "UK",             "CENTRAL TV future use",                            0x5699, 0x2C, 0x16, 0x3C, 0x16, 0      },
{ "UK",             "HTV",                                              0x5AAF, 0x2C, 0x3F, 0x3C, 0x3F, 0      },
{ "UK",             "QVC future use",                                   0x5C33, 0,    0,    0,    0,    0      },
{ "UK",             "QVC future use",                                   0x5C34, 0,    0,    0,    0,    0      },
{ "UK",             "QVC future use",                                   0x5C39, 0,    0,    0,    0,    0      },
{ "UK",             "QVC UK",                                           0x5C44, 0,    0,    0,    0,    0      },
{ "UK",             "CARLTON TV",                                       0x82DD, 0x2C, 0x1D, 0x3C, 0x1D, 0      },
{ "UK",             "CARLTON TV future use",                            0x82DE, 0x5B, 0xCF, 0x3B, 0x4F, 0      },
{ "UK",             "CARLTON TV future use",                            0x82DF, 0x5B, 0xD0, 0x3B, 0x50, 0      },
{ "UK",             "CARLTON TV future use",                            0x82E0, 0x5B, 0xD1, 0x3B, 0x51, 0      },
{ "UK",             "CARLTON SELECT",                                   0x82E1, 0x2C, 0x05, 0x3C, 0x05, 0      },
{ "UK",             "CARLTON SEL. future use",                          0x82E2, 0x2C, 0x06, 0x3C, 0x06, 0      },
{ "UK",             "ULSTER TV",                                        0x833B, 0x2C, 0x3D, 0x3C, 0x3D, 0      },
{ "UK",             "LWT",                                              0x884B, 0x2C, 0x0B, 0x3C, 0x0B, 0      },
{ "UK",             "LWT future use",                                   0x884C, 0x5B, 0xD9, 0x3B, 0x59, 0      },
{ "UK",             "LWT future use",                                   0x884D, 0x5B, 0xDA, 0x3B, 0x5A, 0      },
{ "UK",             "LWT future use",                                   0x884F, 0x5B, 0xDB, 0x3B, 0x5B, 0      },
{ "UK",             "LWT future use",                                   0x8850, 0x5B, 0xDC, 0x3B, 0x5C, 0      },
{ "UK",             "LWT future use",                                   0x8851, 0x5B, 0xDD, 0x3B, 0x5D, 0      },
{ "UK",             "LWT future use",                                   0x8852, 0x5B, 0xDE, 0x3B, 0x5E, 0      },
{ "UK",             "LWT future use",                                   0x8853, 0x5B, 0xDF, 0x3B, 0x5F, 0      },
{ "UK",             "LWT future use",                                   0x8854, 0x5B, 0xE0, 0x3B, 0x60, 0      },
{ "UK",             "NBC Europe",                                       0x8E71, 0x2C, 0x31, 0x3C, 0x31, 0      },
{ "UK",             "CNBC Europe",                                      0x8E72, 0x2C, 0x35, 0x3C, 0x35, 0      },
{ "UK",             "NBC Europe future use",                            0x8E73, 0x2C, 0x32, 0x3C, 0x32, 0      },
{ "UK",             "NBC Europe future use",                            0x8E74, 0x2C, 0x2E, 0x3C, 0x2E, 0      },
{ "UK",             "NBC Europe future use",                            0x8E75, 0x2C, 0x2A, 0x3C, 0x2A, 0      },
{ "UK",             "NBC Europe future use",                            0x8E76, 0x2C, 0x29, 0x3C, 0x29, 0      },
{ "UK",             "NBC Europe future use",                            0x8E77, 0x2C, 0x28, 0x3C, 0x28, 0      },
{ "UK",             "NBC Europe future use",                            0x8E78, 0x2C, 0x26, 0x3C, 0x26, 0      },
{ "UK",             "NBC Europe future use",                            0x8E79, 0x2C, 0x23, 0x3C, 0x23, 0      },
{ "UK",             "CHANNEL 5 (1)",                                    0x9602, 0x2C, 0x02, 0x3C, 0x02, 0      },
{ "UK",             "Nickelodeon UK",                                   0xA460, 0,    0,    0,    0,    0      },
{ "UK",             "Paramount Comedy Channel UK",                      0xA465, 0,    0,    0,    0,    0      },
{ "UK",             "TYNE TEES TV",                                     0xA82C, 0x2C, 0x2C, 0x3C, 0x2C, 0      },
{ "UK",             "TYNE TEES TV future use",                          0xA82D, 0x5B, 0xE6, 0x3B, 0x66, 0      },
{ "UK",             "TYNE TEES TV future use",                          0xA82E, 0x5B, 0xE7, 0x3B, 0x67, 0      },
{ "UK",             "GMTV",                                             0xADDC, 0x5B, 0xD2, 0x3B, 0x52, 0      },
{ "UK",             "GMTV future use",                                  0xADDD, 0x5B, 0xD3, 0x3B, 0x53, 0      },
{ "UK",             "GMTV future use",                                  0xADDE, 0x5B, 0xD4, 0x3B, 0x54, 0      },
{ "UK",             "GMTV future use",                                  0xADDF, 0x5B, 0xD5, 0x3B, 0x55, 0      },
{ "UK",             "GMTV future use",                                  0xADE0, 0x5B, 0xD6, 0x3B, 0x56, 0      },
{ "UK",             "GMTV future use",                                  0xADE1, 0x5B, 0xD7, 0x3B, 0x57, 0      },
{ "UK",             "GRANADA TV",                                       0xADD8, 0x2C, 0x18, 0x3C, 0x18, 0      },
{ "UK",             "GRANADA TV future use",                            0xADD9, 0x5B, 0xD8, 0x3B, 0x58, 0      },
{ "UK",             "S4C",                                              0xB4C7, 0x2C, 0x07, 0x3C, 0x07, 0      },
{ "UK",             "BORDER TV",                                        0xB7F7, 0x2C, 0x27, 0x3C, 0x27, 0      },
{ "UK",             "CHANNEL 5 (4)",                                    0xC47B, 0x2C, 0x3B, 0x3C, 0x3B, 0      },
{ "UK",             "FilmFour",                                         0xC4F4, 0x42, 0xF4, 0x32, 0x74, 0      },
{ "UK",             "ITV NETWORK",                                      0xC8DE, 0x2C, 0x1E, 0x3C, 0x1E, 0      },
{ "UK",             "MERIDIAN future use",                              0xDD50, 0x2C, 0x10, 0x3C, 0x10, 0      },
{ "UK",             "MERIDIAN future use",                              0xDD51, 0x5B, 0xE1, 0x3B, 0x61, 0      },
{ "UK",             "MERIDIAN future use",                              0xDD52, 0x5B, 0xE2, 0x3B, 0x62, 0      },
{ "UK",             "MERIDIAN future use",                              0xDD53, 0x5B, 0xE3, 0x3B, 0x63, 0      },
{ "UK",             "MERIDIAN future use",                              0xDD54, 0x5B, 0xE4, 0x3B, 0x64, 0      },
{ "UK",             "MERIDIAN future use",                              0xDD55, 0x5B, 0xE5, 0x3B, 0x65, 0      },
{ "UK",             "HTV future use",                                   0xF258, 0x2C, 0x38, 0x3C, 0x38, 0      },
{ "UK",             "GRAMPIAN TV",                                      0xF33A, 0x2C, 0x3A, 0x3C, 0x3A, 0      },
{ "UK",             "SCOTTISH TV",                                      0xF9D2, 0x2C, 0x12, 0x3C, 0x12, 0      },
{ "UK",             "YORKSHIRE TV",                                     0xFA2C, 0x2C, 0x2D, 0x3C, 0x2D, 0      },
{ "UK",             "YORKSHIRE TV future use",                          0xFA2D, 0x5B, 0xEA, 0x3B, 0x6A, 0      },
{ "UK",             "YORKSHIRE TV future use",                          0xFA2E, 0x5B, 0xEB, 0x3B, 0x6B, 0      },
{ "UK",             "YORKSHIRE TV future use",                          0xFA2F, 0x5B, 0xEC, 0x3B, 0x6C, 0      },
{ "UK",             "YORKSHIRE TV future use",                          0xFA30, 0x5B, 0xED, 0x3B, 0x6D, 0      },
{ "UK",             "YORKSHIRE TV future use",                          0xFA31, 0x5B, 0xEE, 0x3B, 0x6E, 0      },
{ "UK",             "ANGLIA TV",                                        0xFB9C, 0x2C, 0x1C, 0x3C, 0x1C, 0      },
{ "UK",             "ANGLIA TV future use",                             0xFB9D, 0x5B, 0xCD, 0x3B, 0x4D, 0      },
{ "UK",             "ANGLIA TV future use",                             0xFB9E, 0x5B, 0xCE, 0x3B, 0x4E, 0      },
{ "UK",             "ANGLIA TV future use",                             0xFB9F, 0x2C, 0x1F, 0x3C, 0x1F, 0      },
{ "UK",             "CHANNEL 4",                                        0xFCD1, 0x2C, 0x11, 0x3C, 0x11, 0      },
{ "UK",             "CHANNEL TV",                                       0xFCE4, 0x2C, 0x24, 0x3C, 0x24, 0      },
{ "UK",             "RACING Ch.",                                       0xFCF3, 0x2C, 0x13, 0x3C, 0x13, 0      },
{ "UK",             "HISTORY Ch.",                                      0xFCF4, 0x5B, 0xF6, 0x3B, 0x76, 0      },
{ "UK",             "SCI FI CHANNEL",                                   0xFCF5, 0x2C, 0x15, 0x3C, 0x15, 0      },
{ "UK",             "SKY TRAVEL",                                       0xFCF6, 0x5B, 0xF9, 0x3B, 0x79, 0      },
{ "UK",             "SKY SOAPS",                                        0xFCF7, 0x2C, 0x17, 0x3C, 0x17, 0      },
{ "UK",             "SKY SPORTS 2",                                     0xFCF8, 0x2C, 0x08, 0x3C, 0x08, 0      },
{ "UK",             "SKY GOLD",                                         0xFCF9, 0x2C, 0x19, 0x3C, 0x19, 0      },
{ "UK",             "SKY SPORTS",                                       0xFCFA, 0x2C, 0x1A, 0x3C, 0x1A, 0      },
{ "UK",             "MOVIE CHANNEL",                                    0xFCFB, 0x2C, 0x1B, 0x3C, 0x1B, 0      },
{ "UK",             "SKY MOVIES PLUS",                                  0xFCFC, 0x2C, 0x0C, 0x3C, 0x0C, 0      },
{ "UK",             "SKY NEWS",                                         0xFCFD, 0x2C, 0x0D, 0x3C, 0x0D, 0      },
{ "UK",             "SKY ONE",                                          0xFCFE, 0x2C, 0x0E, 0x3C, 0x0E, 0      },
{ "UK",             "SKY TWO",                                          0xFCFF, 0x2C, 0x0F, 0x3C, 0x0F, 0      },

{ "Ukraine",        "1+1",                                              0x7700, 0,    0,    0,    0,    0x07C0 },
{ "Ukraine",        "1+1 future use",                                   0x7701, 0,    0,    0,    0,    0      },
{ "Ukraine",        "1+1 future use",                                   0x7702, 0,    0,    0,    0,    0      },
{ "Ukraine",        "1+1 future use",                                   0x7703, 0,    0,    0,    0,    0      },
{ "Ukraine",        "M1",                                               0x7705, 0,    0,    0,    0,    0x07C5 },
{ "Ukraine",        "ICTV",                                             0x7707, 0,    0,    0,    0,    0      },
{ "Ukraine",        "Novy Kanal",                                       0x7708, 0,    0,    0,    0,    0x07C8 },
};

int iNbRegisteredCNICodes = sizeof(RegisteredCNICodes) / sizeof(RegisteredCNICodes[0]);


void VBI_VT_Init()
{
    InitializeCriticalSection(&VTPageChangeMutex);

    VTDrawer.SetAntialias(VTAntialias);

    VTSavePageHex = 0x100;
    VTSavePageSubCode = 0xFFFF;

    VT_HistoryReset();

    for (int i = 0; i < 25; i++)
    {
        VTHilightListArray[i] = NULL;
    }

    if (!VT_GetAutoCodepage())
    {
        VT_SetCodepage(NULL, NULL, VTUserCodepage);
    }

    VTDecoder.SetCachingControl(VTCachingControl);
    VTDecoder.SetHighGranularityCaching(VTHighGranularityCaching);
    VTDecoder.SetSubstituteSpacesForError(VTSubstituteErrorsWithSpaces);
}


void VBI_VT_Exit()
{
    for (int i = 0; i < 25; i++)
    {
        VT_DeleteHilightList(&VTHilightListArray[i]);
    }

    DeleteCriticalSection(&VTPageChangeMutex);
}


void VT_Init_Data(double VBI_Frequency)
{
    TTVFormat* TVFormat = GetTVFormat(Providers_GetCurrentSource()->GetFormat());
    // set up the desired Teletext step frequency
    // there are two different Teletext frequencies
    // one for PAL type and one for NTSC type formats
    if(TVFormat->wCropHeight == 576)
    {
        VTStep = (int) ((VBI_Frequency / 6.9375) * FPFAC + 0.5);
    }
    else
    {
        VTStep = (int) ((VBI_Frequency / 5.7272) * FPFAC + 0.5);
    }
}


void VT_SetState(HDC hDC, LPRECT lpRect, eVTState State)
{
    if (State == VTState)
    {
        return;
    }

    VT_ClearInput();

    if (State != VT_OFF && VTState != VT_OFF)
    {
        VTState = State;
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(GetMainWnd(), TIMER_VTINPUT, g_VTOSDTimeout, NULL);
        return;
    }

    VTState = State;

    if (State == VT_OFF)
    {
        if ((VTPageHex & 0xFF00) < 0x0100 ||
            (VTPageHex & 0xFF00) > 0x0800)
        {
            VTSavePageHex = 0x100;
            VTSavePageSubCode = 0xFFFF;
        }
        else
        {
            VTSavePageHex = VTPageHex;
            VTSavePageSubCode = VTPageSubCode;
        }

        VTPageHex = 0xFFFF;
        VTPageSubCode = 0xFFFF;

        KillTimer(GetMainWnd(), VTFlashTimer);
        VTFlashTimer = 0;

        KillTimer(GetMainWnd(), TIMER_VTINPUT);
        VTShowOSD = FALSE;

        VTHilightListPtr = NULL;
        VTCursorRowCol = 0xFFFF;
    }
    else
    {
        if (VT_GetAutoCodepage())
        {
            VT_SetCodepage(NULL, NULL, VTCODEPAGE_NONE);
        }
        VT_SetPage(hDC, lpRect, VTSavePageHex, VTSavePageSubCode);
    }
}


eVTState VT_GetState()
{
    return VTState;
}


void VT_ChannelChange()
{
    VTDecoder.ResetDecoder();

    VT_HistoryReset();

    VTHilightListPtr = NULL;
    *VTSearchString = '\0';

    if (VTState != VT_OFF)
    {
        VT_SetPage(NULL, NULL, 0x100);
    }
    else
    {
        VTSavePageHex = 0x100;
        VTSavePageSubCode = 0xFFFF;
    }
}


BOOL VT_SetAutoCodepage(HDC hDC, LPRECT lpRect, BOOL bAuto)
{
    if (bAuto != VTAutoCodepage)
    {
        VTAutoCodepage = bAuto;

        if (bAuto)
        {
            return VT_SetCodepage(hDC, lpRect, VTCODEPAGE_NONE);
        }
        else
        {
            return VT_SetCodepage(hDC, lpRect, VTUserCodepage);
        }
    }
    return FALSE;
}


BOOL VT_GetAutoCodepage()
{
    return VTAutoCodepage;
}


BOOL VT_SetCodepage(HDC hDC, LPRECT lpRect, eVTCodepage Codepage)
{
    if (Codepage == VTCODEPAGE_NONE)
    {
        BYTE SubsetCode = VTDecoder.GetCharacterSubsetCode();
        Codepage = CVTCommon::GetRegionCodepage(VTRegion, SubsetCode, TRUE);
    }
    else
    {
        VTUserCodepage = Codepage;
        VTAutoCodepage = FALSE;
    }

    if (Codepage != VTCodepage)
    {
        VTCodepage = Codepage;
        VT_Redraw(hDC, lpRect);
        return TRUE;
    }

    return FALSE;
}


void VT_SetAntialias(HDC hDC, LPRECT lpRect, BOOL bAntialias)
{
    if (bAntialias != VTAntialias)
    {
        VTAntialias = bAntialias;
        VTDrawer.SetAntialias(bAntialias);
        VT_Redraw(hDC, lpRect);
    }
}


BOOL VT_GetAntialias()
{
    return VTAntialias;
}


void VT_SetShowHidden(HDC hDC, LPRECT lpRect, BOOL bShowHidden)
{
    if (bShowHidden != VTShowHidden)
    {
        VTShowHidden = bShowHidden;
        VT_Redraw(hDC, lpRect, VTDF_HIDDENONLY);
    }
}


BOOL VT_GetShowHidden()
{
    return VTShowHidden;
}


void VT_SetOverlayColour(COLORREF ColorRef)
{
    VTDrawer.SetTransparencyColour(ColorRef);
}


BOOL VT_SetPage(HDC hDC, LPRECT lpRect, WORD wPageHex, WORD wPageSubCode)
{
    if (wPageHex < 0x100 ||
        wPageHex > 0x8FF)
    {
        return FALSE;
    }

    EnterCriticalSection(&VTPageChangeMutex);

    VTPageHex = wPageHex;
    VTPageSubCode = wPageSubCode;

    DWORD dwPageCode = MAKELONG(VTPageHex, VTPageSubCode);

    VTLoadedPageCode = VTDecoder.GetDisplayPage(dwPageCode, &VTVisiblePage);

    LeaveCriticalSection(&VTPageChangeMutex);

    VTShowHidden = FALSE;
    VTCursorRowCol = 0xFFFF;

    if (VTLoadedPageCode != 0)
    {
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(GetMainWnd(), TIMER_VTINPUT, g_VTOSDTimeout, NULL);
        VT_HistoryPushPage(VTPageHex);
    }
    else
    {
        VTHilightListPtr = NULL;
        KillTimer(GetMainWnd(), VTFlashTimer);
        VTFlashTimer = 0;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, FALSE);
        VTVisiblePage.wControlBits = VTCONTROL_INHIBITDISP;
        VTVisiblePage.bShowRow24 = FALSE;

        // Get the loading status message
        VTDecoder.GetDisplayComment(dwPageCode, &VTVisiblePage);

        VT_SetPageOSD(NULL, FALSE);
        VT_Redraw(hDC, lpRect);
    }

    return TRUE;
}


BOOL VT_PageScroll(HDC hDC, LPRECT lpRect, BOOL bForwards)
{
    DWORD dwPageCode = MAKELONG(VTPageHex, 0xFFFF);

    EnterCriticalSection(&VTPageChangeMutex);

    dwPageCode = VTDecoder.GetNextDisplayPage(dwPageCode, &VTVisiblePage, !bForwards);

    VTPageSubCode = 0xFFFF;

    if (dwPageCode != 0)
    {
        VTPageHex = LOWORD(dwPageCode);
        VTLoadedPageCode = dwPageCode;
    }

    LeaveCriticalSection(&VTPageChangeMutex);

    if (dwPageCode != 0)
    {
        VTShowHidden = FALSE;
        VTCursorRowCol = 0xFFFF;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(GetMainWnd(), TIMER_VTINPUT, g_VTOSDTimeout, NULL);
        VT_HistoryPushPage(VTPageHex);

        return TRUE;
    }
    else
    {
        VT_ShowHeader(hDC, lpRect);
    }

    return FALSE;
}


BOOL VT_SubPageScroll(HDC hDC, LPRECT lpRect, BOOL bForwards)
{
    DWORD dwPageCode;

    if (LOWORD(VTLoadedPageCode) == VTPageHex)
    {
        dwPageCode = VTLoadedPageCode;
    }
    else
    {
        dwPageCode = MAKELONG(VTPageHex, VTPageSubCode);
    }

    EnterCriticalSection(&VTPageChangeMutex);

    dwPageCode = VTDecoder.GetNextDisplaySubPage(dwPageCode, &VTVisiblePage, !bForwards);

    if (dwPageCode != 0)
    {
        VTPageHex = LOWORD(dwPageCode);
        VTPageSubCode = HIWORD(dwPageCode);
        VTLoadedPageCode = dwPageCode;
    }
    else
    {
        VTPageSubCode = 0xFFFF;
    }

    LeaveCriticalSection(&VTPageChangeMutex);

    if (dwPageCode != 0)
    {
        VTShowHidden = FALSE;
        VTCursorRowCol = 0xFFFF;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(GetMainWnd(), TIMER_VTINPUT, g_VTOSDTimeout, NULL);

        return TRUE;
    }
    else
    {
        // There's no point calling showing the header
        // if the page isn't loaded because the header
        // is always shown before the page loads.
        if (LOWORD(VTLoadedPageCode) == VTPageHex)
        {
            VT_ShowHeader(hDC, lpRect);
        }
    }

    return FALSE;
}


BOOL VT_IsSearchStringValid(BOOL bBeepOnCorrection)
{
    if (*VTSearchString == '\0')
    {
        return FALSE;
    }

    for (int i = 0; VTSearchString[i] != '\0'; i++)
    {
        if (VTSearchString[i] != ' ')
        {
            return TRUE;
        }
    }

    *VTSearchString = '\0';

    if (bBeepOnCorrection)
    {
        MessageBeep(MB_OK);
    }
    return FALSE;
}


BOOL VT_PerformSearch(HDC hDC, LPRECT lpRect, BOOL bInclusive, BOOL bReverse)
{
    if (!VT_IsSearchStringValid(TRUE))
    {
        if (VTHilightListPtr != NULL)
        {
            // Get rid of the existing hilighting
            VT_UpdateHilightList();
            VT_Redraw(hDC, lpRect);
        }
        return FALSE;
    }

    DWORD dwPageCode;

    if (!bInclusive && LOWORD(VTLoadedPageCode) == VTPageHex)
    {
        dwPageCode = VTLoadedPageCode;
    }
    else
    {
        dwPageCode = MAKELONG(VTPageHex, bReverse ? 0xFFFF : 0);
    }

    EnterCriticalSection(&VTPageChangeMutex);

    dwPageCode = VTDecoder.FindInDisplayPage(dwPageCode, bInclusive,
        VTSearchString, &VTVisiblePage, bReverse);

    if (dwPageCode != 0)
    {
        VTPageHex = LOWORD(dwPageCode);
        VTPageSubCode = HIWORD(dwPageCode);
        VTLoadedPageCode = dwPageCode;
    }

    LeaveCriticalSection(&VTPageChangeMutex);

    if (dwPageCode != 0)
    {
        VTShowHidden = FALSE;
        VTCursorRowCol = 0xFFFF;
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
        VT_UpdateHilightList();
        VT_UpdateFlashTimer();
        VT_SetPageOSD(NULL, TRUE);
        VT_Redraw(hDC, lpRect);
        SetTimer(GetMainWnd(), TIMER_VTINPUT, g_VTOSDTimeout, NULL);
        VT_HistoryPushPage(VTPageHex);

        return TRUE;
    }
    else
    {
        // No pages found
        MessageBeep(MB_OK);
    }

    return FALSE;
}


BOOL VT_ShowTestPage(HDC hDC, LPRECT lpRect)
{
    EnterCriticalSection(&VTPageChangeMutex);

    VTPageHex = 0x900;
    VTPageSubCode = 0x0000;
    VTLoadedPageCode = MAKELONG(VTPageHex, VTPageSubCode);

    LeaveCriticalSection(&VTPageChangeMutex);

    VTDecoder.CreateTestDisplayPage(&VTVisiblePage);

    VTShowHidden = FALSE;
    VTCursorRowCol = 0xFFFF;
    VTHilightListPtr = NULL;
    VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);
    VT_UpdateFlashTimer();
    VT_SetPageOSD(NULL, TRUE);
    VT_Redraw(hDC, lpRect);
    SetTimer(GetMainWnd(), TIMER_VTINPUT, g_VTOSDTimeout, NULL);

    return TRUE;
}


BOOL VT_PerformFlofKey(HDC hDC, LPRECT lpRect, BYTE nFlofKey)
{
    DWORD dwPageCode = 0UL;

    if (!VTVisiblePage.bShowRow24)
    {
        MessageBeep(MB_OK);
        return FALSE;
    }

    dwPageCode = VTVisiblePage.EditorialLink[nFlofKey];

    WORD wPageHex = LOWORD(dwPageCode);
    WORD wPageSubCode = HIWORD(dwPageCode);

    if (wPageHex == VTPAGE_PREVIOUS)
    {
        wPageHex = VT_HistoryPopLastPage(VTPageHex);
    }

    if (wPageHex == 0 || (wPageHex & 0xFF) == 0xFF)
    {
        MessageBeep(MB_OK);
        return FALSE;
    }

    if (wPageSubCode >= 0x3F7F)
    {
        wPageSubCode = 0xFFFF;
    }

    return VT_SetPage(hDC, lpRect, wPageHex, wPageSubCode);
}


void VT_SetPageOSD(char OSD[3])
{
    char szOSD[9];

    if (OSD == NULL)
    {
        VTPageOSD = NULL;

        if (VTShowSubcodeInOSD != FALSE &&
            LOWORD(VTLoadedPageCode) == VTPageHex)
        {
            sprintf(szOSD, "%03X/%-4X", VTPageHex, HIWORD(VTLoadedPageCode));
        }
        else
        {
            sprintf(szOSD, "  P%03X \x07", VTPageHex);
        }
    }
    else
    {
        BOOL bSpace = FALSE;

        CopyMemory(VTOSDBuffer, OSD, 3);

        for (BYTE i = 0; i < 3; i++)
        {
            if (bSpace || VTOSDBuffer[i] == '\0')
            {
                VTOSDBuffer[i] = ' ';
                bSpace = TRUE;
            }
        }

        VTPageOSD = VTOSDBuffer;

        sprintf(szOSD, "  P%c%c%c \x07", VTPageOSD[0], VTPageOSD[1], VTPageOSD[2]);
    }

    CopyMemory(VTVisiblePage.Frame[0], szOSD, 8);
    VTVisiblePage.LineState[0] |= CACHESTATE_HASDATA;
}


void VT_SetPageOSD(char OSD[3], BOOL bOSDModeOn)
{
    VT_SetPageOSD(OSD);
    VTShowOSD = bOSDModeOn;
}


void VT_ShowHeader(HDC hDC, LPRECT lpRect, char OSD[3])
{
    VT_SetPageOSD(OSD, TRUE);
    VT_Redraw(hDC, lpRect, VTDF_HEADERONLY);
    SetTimer(GetMainWnd(), TIMER_VTINPUT, g_VTOSDTimeout, NULL);
}


BOOL VT_IsTransparencyInPage()
{
    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        return FALSE;
    }

    if (VTState == VT_MIXED ||
        (VTVisiblePage.wControlBits & VTCONTROL_NEWSFLASH) ||
        (VTVisiblePage.wControlBits & VTCONTROL_SUBTITLE))
    {
        return TRUE;
    }

    return FALSE;
}


void VT_HistoryReset()
{
    VTPageHistoryHead = 0;
    VTPageHistory[0] = 0;
}


void VT_HistoryPushPage(WORD wPageHex)
{
    if (VTPageHistory[VTPageHistoryHead] != wPageHex)
    {
        VTPageHistoryHead = (VTPageHistoryHead + 1) % VT_MAXPAGEHISTORY;
        VTPageHistory[VTPageHistoryHead] = wPageHex;
    }
}


WORD VT_HistoryPopLastPage(WORD wCurrentPageHex)
{
    WORD wPageHex;

    if (VTPageHistory[VTPageHistoryHead] == wCurrentPageHex)
    {
        WORD wLastHistoryHead = (VT_MAXPAGEHISTORY +
            VTPageHistoryHead - 1) % VT_MAXPAGEHISTORY;

        if (VTPageHistory[wLastHistoryHead] == 0)
        {
            return 0;
        }

        VTPageHistory[VTPageHistoryHead] = 0;
        VTPageHistoryHead = wLastHistoryHead;
    }

    wPageHex = VTPageHistory[VTPageHistoryHead];

    if (VTPageHistory[VTPageHistoryHead] != 0)
    {
        VTPageHistory[VTPageHistoryHead] = 0;
        VTPageHistoryHead = (VT_MAXPAGEHISTORY +
            VTPageHistoryHead - 1) % VT_MAXPAGEHISTORY;
    }

    return wPageHex;
}


// Global defined in DScaler.cpp
extern int ChannelEnterTime;

BOOL VT_OnInput(HDC hDC, LPRECT lpRect, char cInput)
{
    BYTE nLength = strlen(VTPageInput);
    BOOL bPageChanged = FALSE;

    if (nLength == 3)
    {
        VTPageInput[0] = VTPageInput[1];
        VTPageInput[1] = VTPageInput[2];
        VTPageInput[2] = toupper(cInput);
    }
    else
    {
        VTPageInput[nLength] = toupper(cInput);
        VTPageInput[++nLength] = '\0';
    }

    g_VTOSDTimeout = ChannelEnterTime;

    if (nLength == 3)
    {
        WORD wPageHex = VT_Input2PageHex(VTPageInput);

        if (wPageHex != 0)
        {
            bPageChanged = VT_SetPage(hDC, lpRect, wPageHex);
        }
        else
        {
            VT_ShowHeader(hDC, lpRect, VTPageInput);
        }
    }
    else
    {
        VT_ShowHeader(hDC, lpRect, VTPageInput);
    }

    g_VTOSDTimeout = VT_OSD_DISPLAY_TIMEOUT;

    return bPageChanged;
}


void VT_ClearInput()
{
    *VTPageInput = '\0';
}


void VT_OnInputTimer(HDC hDC, LPRECT lpRect)
{
    KillTimer(GetMainWnd(), TIMER_VTINPUT);
    VT_ClearInput();

    if (VTState == VT_OFF)
    {
        return;
    }

    VT_SetPageOSD(NULL, FALSE);
    VT_Redraw(hDC, lpRect, VTDF_HEADERONLY);
}


WORD VT_Input2PageHex(char Input[3])
{
    if (Input[0] < '1' || Input[0] > '8')
    {
        return 0;
    }

    WORD wPageHex = 0;

    wPageHex |= (Input[0] - '0') * 0x100;

    for (int i = 1; i < 3; i++)
    {
        if (Input[i] >= '0' &&
            Input[i] <= '9')
        {
            wPageHex |= (Input[i] - '0') * (i == 1 ? 0x10 : 1);
        }
        else if (Input[i] >= 'A' &&
                 Input[i] <= 'F')
        {
            wPageHex |= (Input[i] - 'A' + 0xA) * (i == 1 ? 0x10 : 1);
        }
    }

    return wPageHex;
}


void VT_RedrawFlash(HDC hDC, LPRECT lpRect)
{
    VTShowFlash = !VTShowFlash;
    VT_Redraw(hDC, lpRect, VTDF_FLASHONLY);
}


void VT_Redraw(HDC hDC, LPRECT lpRect, WORD uVTDFFlags)
{
    if (VTState == VT_OFF || hDC == NULL || lpRect == NULL)
    {
        return;
    }

    WORD uFlags = uVTDFFlags;

    if (VTShowHidden)
    {
        uFlags |= VTDF_HIDDEN;
    }
    if (VTShowFlash)
    {
        uFlags |= VTDF_FLASH;
    }
    if (VTState == VT_MIXED)
    {
        uFlags |= VTDF_MIXEDMODE;
    }
    if (VTShowOSD)
    {
        uFlags |= VTDF_FORCEHEADER;
    }

    VTDrawer.SetCodepage(VTCodepage);
    VTDrawer.DrawPage(hDC, lpRect, &VTVisiblePage, uFlags,
        VTHilightListPtr, VTDoubleProfile, VTRefreshProfile,
        &VTRefreshCount);
}


LONG VT_GetPaintedRects(RECT* pRectBuffer, LONG nBufferSize)
{
    LONG nCopyCount;

    nCopyCount = (nBufferSize < VTRefreshCount) ? nBufferSize : VTRefreshCount;

    if (nCopyCount > 0)
    {
        memcpy(pRectBuffer, VTRefreshProfile, sizeof(RECT) * nCopyCount);
    }

    return nCopyCount;
}


void VT_ResetPaintedRects()
{
    VTRefreshCount = 0;
}


void VT_DecoderEventProc(BYTE uMsg, DWORD dwParam)
{
    switch (uMsg)
    {
    case DECODEREVENT_HEADERUPDATE:
        PostMessageToMainWindow(UWM_VIDEOTEXT, VTM_VTHEADERUPDATE, NULL);
        break;

    case DECODEREVENT_COMMENTUPDATE:
        PostMessageToMainWindow(UWM_VIDEOTEXT, VTM_VTCOMMENTUPDATE, dwParam);
        break;

    case DECODEREVENT_PAGEUPDATE:
        /* // TryEnterCriticalSection is not defined in my API
        if (TryEnterCriticalSection(&VTPageChangeMutex))
        {
            if (LOWORD(dwParam) == VTPageHex)
            {
                if (VTPageSubCode == 0xFFFF || HIWORD(dwParam) == VTPageSubCode)
                {
                    PostMessageToMainWindow(WM_VIDEOTEXT, VTM_VTPAGEUPDATE, dwParam);
                }
            }
            LeaveCriticalSection(&VTPageChangeMutex);
        }
        else*/
        {
            PostMessageToMainWindow(UWM_VIDEOTEXT, VTM_VTPAGEUPDATE, dwParam);
        }
        break;

    case DECODEREVENT_PAGEREFRESH:
        /* // TryEnterCriticalSection is not defined in my API
        if (TryEnterCriticalSection(&VTPageChangeMutex))
        {
            if (LOWORD(dwParam) == VTPageHex)
            {
                if (VTPageSubCode == 0xFFFF)
                {
                    PostMessageToMainWindow(WM_VIDEOTEXT, VTM_VTPAGEREFRESH, dwParam);
                }
            }
            LeaveCriticalSection(&VTPageChangeMutex);
        }
        else*/
        {
            PostMessageToMainWindow(UWM_VIDEOTEXT, VTM_VTPAGEREFRESH, dwParam);
        }
        break;
    }
}


void VT_ProcessHeaderUpdate(HDC hDC, LPRECT lpRect)
{
    if (VTState == VT_OFF)
    {
        return;
    }

    if (VT_GetAutoCodepage())
    {
        if (VT_SetCodepage(hDC, lpRect, VTCODEPAGE_NONE))
        {
            return;
        }
    }

    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        VTDecoder.GetDisplayHeader(&VTVisiblePage);
        VT_Redraw(hDC, lpRect, VTDF_HEADERONLY);
    }
    else
    {
        char szOldClock[8];

        CopyMemory(szOldClock, &VTVisiblePage.Frame[0][32], 8);
        VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);

        // Only redraw the clock if it changed
        if (memcmp(&VTVisiblePage.Frame[0][32], szOldClock, 8) != 0)
        {
            VT_Redraw(hDC, lpRect, VTDF_CLOCKONLY);
        }
    }
}


BOOL VT_ProcessCommentUpdate(HDC hDC, LPRECT lpRect, DWORD dwPageCode)
{
    if (VTState == VT_OFF)
    {
        return FALSE;
    }

    if (LOWORD(dwPageCode) != VTPageHex ||
        HIWORD(dwPageCode) != VTPageSubCode)
    {
        return FALSE;
    }

    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        if (VTDecoder.GetDisplayComment(dwPageCode, &VTVisiblePage))
        {
            if (VTVisiblePage.LineState[24] & CACHESTATE_UPDATED)
            {
                if (LOBYTE(VTCursorRowCol) == 24)
                {
                    VTCursorRowCol = 0xFFFF;
                }

                VT_UpdateFlashTimer();
                VT_Redraw(hDC, lpRect, VTDF_ROW24ONLY);
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL VT_ProcessPageUpdate(HDC hDC, LPRECT lpRect, DWORD dwPageCode)
{
    if (VTGotoProcDlg != NULL)
    {
        // Update the goto dialog
        SendMessage(VTGotoProcDlg, UWM_VIDEOTEXT, VTM_VTPAGEUPDATE, dwPageCode);
    }

    if (VTState == VT_OFF)
    {
        return FALSE;
    }

    if (LOWORD(dwPageCode) == VTPageHex)
    {
        if (VTPageSubCode == 0xFFFF || HIWORD(dwPageCode) == VTPageSubCode)
        {
            dwPageCode = VTDecoder.GetDisplayPage(dwPageCode, &VTVisiblePage);
            VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);

            if (VTUpscrollDuplicationFilter != FALSE)
            {
                VT_PerformUpscrollDuplicationFilter();
            }

            if (VTDoubleHeightSubtitlesFilter != FALSE)
            {
                VT_PerformDoubleHeightSubtitlesFilter();
            }

            VTCursorRowCol = 0xFFFF;

            if (dwPageCode != VTLoadedPageCode)
            {
                VTLoadedPageCode = dwPageCode;
                // Reburn the OSD into the reloaded page
                VT_SetPageOSD(VTPageOSD);
                VT_UpdateHilightList();
                VT_UpdateFlashTimer();
                VT_Redraw(hDC, lpRect);
                VT_HistoryPushPage(VTPageHex);
            }
            else
            {
                for (int i = 0; i < 23; i++)
                {
                    // If an updated line was previously drawn as
                    // double height, we need to force an update
                    // on the line proceeding incase the new line
                    // is no longer double height.

                    if ((VTVisiblePage.LineState[i] & CACHESTATE_UPDATED) != 0)
                    {
                        if (VTDoubleProfile[i] != FALSE)
                        {
                            VTVisiblePage.LineState[i + 1] |= CACHESTATE_UPDATED;
                        }
                    }
                }

                // Reburn the OSD into the reloaded page
                VT_SetPageOSD(VTPageOSD);
                VT_UpdateHilightList(TRUE);
                VT_UpdateFlashTimer();
                VT_Redraw(hDC, lpRect, VTDF_UPDATEDONLY);
            }
            return TRUE;
        }
    }

    return FALSE;
}


BOOL VT_ProcessPageRefresh(HDC hDC, LPRECT lpRect, DWORD dwPageCode)
{
    if (VTState == VT_OFF)
    {
        return FALSE;
    }

    if (LOWORD(dwPageCode) == VTPageHex)
    {
        if (VTPageSubCode == 0xFFFF && dwPageCode != VTLoadedPageCode)
        {
            VTLoadedPageCode = VTDecoder.GetDisplayPage(dwPageCode, &VTVisiblePage);
            VTDecoder.GetDisplayHeader(&VTVisiblePage, TRUE);

            // Reburn the OSD into the reloaded page
            VT_SetPageOSD(VTPageOSD);

            VTCursorRowCol = 0xFFFF;
            VT_UpdateHilightList();
            VT_UpdateFlashTimer();
            VT_Redraw(hDC, lpRect);

            return TRUE;
        }
    }

    return FALSE;
}


void VT_PerformUpscrollDuplicationFilter()
{
    if ((VTVisiblePage.wControlBits & VTCONTROL_SUBTITLE) == 0)
    {
        return;
    }

    for (int i = 1; i <= 21; i++)
    {
        if ((VTVisiblePage.LineState[i] & CACHESTATE_HASDATA) &&
            (VTVisiblePage.LineState[i] & CACHESTATE_UPDATED) &&
            (VTVisiblePage.LineState[i] & CACHESTATE_HASERROR) == 0)
        {
            if ((VTVisiblePage.LineState[i + 2] & CACHESTATE_HASDATA) &&
                (VTVisiblePage.LineState[i + 2] & CACHESTATE_UPDATED) == 0 &&
                (VTVisiblePage.LineState[i + 2] & CACHESTATE_HASERROR) == 0)
            {
                if (memcmp(VTVisiblePage.Frame[i + 2], VTVisiblePage.Frame[i], 40) == 0)
                {
                    int j;
                    for (j = i + 2; j <= 23; j += 2)
                    {
                        if ((VTVisiblePage.LineState[j + 2] & CACHESTATE_HASDATA) == 0 ||
                            (VTVisiblePage.LineState[j + 2] & CACHESTATE_UPDATED))
                        {
                            break;
                        }
                        memcpy(VTVisiblePage.Frame[j], VTVisiblePage.Frame[j + 2], 40);
                        VTVisiblePage.LineState[j] |= CACHESTATE_UPDATED;
                    }

                    memset(&VTVisiblePage.Frame[j][0], 0x0d, 1);
                    memset(&VTVisiblePage.Frame[j][1], 0x20, 39);
                    VTVisiblePage.LineState[j] |= CACHESTATE_UPDATED;
                }
            }
        }
    }
}


void VT_PerformDoubleHeightSubtitlesFilter()
{
    if ((VTVisiblePage.wControlBits & VTCONTROL_SUBTITLE) == 0)
    {
        return;
    }

    for (int i = 1; i <= 22; i++)
    {
        if (VTVisiblePage.LineState[i] & CACHESTATE_HASDATA)
        {
            for (int j = 0; j < 40; j++)
            {
                if (VTVisiblePage.Frame[i][j] == 0x20)
                {
                    // Replace the first occurance of a space
                    // to a double height control character
                    VTVisiblePage.Frame[i][j] = 0x0D;
                    break;
                }
                else if (VTVisiblePage.Frame[i][j] == 0x0D)
                {
                    break;
                }
            }
        }
    }
}


void VT_UpdateFlashTimer()
{
    if (CVTCommon::HasFlashElement(&VTVisiblePage))
    {
        if (VTFlashTimer == 0)
        {
            VTFlashTimer = SetTimer(GetMainWnd(), TIMER_VTFLASHER, TIMER_VTFLASHER_MS, NULL);
        }
    }
    else
    {
        if (VTFlashTimer != 0)
        {
            if (KillTimer(GetMainWnd(), VTFlashTimer))
            {
                VTFlashTimer = 0;
            }
        }
    }
}


BOOL VT_ClickAtPosition(HDC hDC, LPRECT lpRect, WORD xPos, WORD yPos)
{
    if (VT_IsPageNumberAtPosition(lpRect, xPos, yPos))
    {
        if (VTCursorPageHex <= VTPAGE_FLOFBLUE &&
            VTCursorPageHex >= VTPAGE_FLOFRED)
        {
            BYTE nFlofKey = VTCursorPageHex - VTPAGE_FLOFRED;
            return VT_PerformFlofKey(hDC, lpRect, nFlofKey);
        }

        return VT_SetPage(hDC, lpRect, VTCursorPageHex);
    }

    return FALSE;
}


BOOL VT_IsPageNumberAtPosition(LPRECT lpRect, LPPOINT lpPoint)
{
    if (LOWORD(VTLoadedPageCode) != VTPageHex)
    {
        return FALSE;
    }

    WORD wRowCol = VTDrawer.GetRowColAtPoint(lpRect, lpPoint);

    if (wRowCol != VTCursorRowCol)
    {
        VTCursorRowCol = wRowCol;
        VTCursorPageHex =
            VTDrawer.FindPageNumberAtRowCol(&VTVisiblePage, VTCursorRowCol);
    }

    return VTCursorPageHex != 0;
}


BOOL VT_IsPageNumberAtPosition(LPRECT lpRect, WORD xPos, WORD yPos)
{
    POINT Point = { xPos, yPos };
    return VT_IsPageNumberAtPosition(lpRect, &Point);
}


void VT_DeleteHilightList(TVTLeftRight** pHilightList)
{
    if (*pHilightList != NULL)
    {
        VT_DeleteHilightList(&(*pHilightList)->Next);
        free(*pHilightList);
        *pHilightList = NULL;
    }
}


void VT_UpdateHilightList(BOOL bUpdatedOnly)
{
    if (*VTSearchString == '\0' || !VTHilightSearch)
    {
        VTHilightListPtr = NULL;
        return;
    }

    VTHilightListPtr = VTHilightListArray;

    WORD wFlags = bUpdatedOnly ? PARSE_UPDATEDONLY : 0;

    CVTCommon::ParsePageElements(&VTVisiblePage, &wFlags,
        VT_UpdateHilightListProc, NULL);
}


BYTE VT_UpdateHilightListProc(TVTPage*, WORD wPoint, LPWORD lpFlags,
                              WORD, BYTE uChar, BYTE uMode, LPVOID lpParam)
{
    static BYTE           nIndex;
    static TVTLeftRight** hHilightListTail;

    BYTE nRow               = LOBYTE(wPoint);
    BYTE nCol               = HIBYTE(wPoint);

    // Don't search the header or row 24
    if (nRow == 0 || nRow == 24)
    {
        return PARSE_CONTINUE;
    }

    if (nCol == 0)
    {
        nIndex = 0;
        VT_DeleteHilightList(&VTHilightListArray[nRow]);
        hHilightListTail = &VTHilightListArray[nRow];
    }

    if (*lpFlags & PARSE_EMPTYLINE)
    {
        return PARSE_STOPLINE;
    }

    if ((uMode & VTMODE_GRAPHICS) != 0 && (uChar & 0x20))
    {
        uChar = 0x00;
    }

    if (toupper(uChar) == toupper(VTSearchString[nIndex]))
    {
        // Check if the last character was matched
        if (VTSearchString[++nIndex] == '\0')
        {
            // Add the hilight and continue
            *hHilightListTail = (TVTLeftRight*)malloc(sizeof(TVTLeftRight));
            (*hHilightListTail)->Left = nCol + 1 - nIndex;
            (*hHilightListTail)->Right = nCol;
            *(hHilightListTail = &(*hHilightListTail)->Next) = NULL;
            nIndex = 0;
            return PARSE_CONTINUE;
        }
    }
    else
    {
        BYTE nLength = strlen(&VTSearchString[nIndex]);

        // If the string has repetitive substrings, it is
        // necessary check if the shifting the starting point
        // to the start of the substrings will find a match.
        int j;
        for (j = 1; j < nIndex; j++)
        {
            if ((nLength + j) > (40 - nCol))
            {
                // These is not enough chars left in the
                // buffer to complete the match
                return PARSE_STOPLINE;
            }

            if (_strnicmp(VTSearchString, &VTSearchString[j], nIndex - j) == 0)
            {
                // We found a possible substring, try to match
                // the failed character again.
                if (toupper(uChar) == toupper(VTSearchString[nIndex - j]))
                {
                    nIndex -= j;
                    if (VTSearchString[++nIndex] == '\0')
                    {
                        // Add the highlight and continue
                        *hHilightListTail = (TVTLeftRight*)malloc(sizeof(TVTLeftRight));
                        (*hHilightListTail)->Left = nCol + 1 - nIndex;
                        (*hHilightListTail)->Right = nCol;
                        *(hHilightListTail = &(*hHilightListTail)->Next) = NULL;
                        nIndex = 0;
                        return PARSE_CONTINUE;
                    }
                    break;
                }
            }
        }

        // No luck finding a valid substring.  Start
        // the matching from the first char again.
        if (j >= nIndex)
        {
            if ((nLength + nIndex) > (40 - nCol))
            {
                // These is not enough chars left in the
                // buffer to complete the match
                return PARSE_STOPLINE;
            }

            if (nIndex != 0)
            {
                // Start again
                nIndex = 0;

                // We have restarted the matching, try to match
                // the failed character one last time.
                if (toupper(uChar) == toupper(VTSearchString[nIndex]))
                {
                    nIndex++;
                }
            }
        }
    }
    return PARSE_CONTINUE;
}


std::string VT_GetStationFromIDP8301()
{
    std::string RetVal;

    WORD wCode = VTDecoder.GetNetworkIDFromP8301();
    if (wCode != 0)
    {
        // dwCode is a network ID code
        //LOG(1, "P8/30/1 Network ID Code %x", dwCode);
        for (int i(0); i < iNbRegisteredCNICodes; i++)
        {
            if (RegisteredCNICodes[i].wNI_P8301 == wCode)
            {
                RetVal = RegisteredCNICodes[i].sNetwork;
                break;
            }
        }
    }
    return RetVal;
}


string VT_GetStationFromPDC()
{
    string RetVal;
    // Search first with CNI from PDC (P8/30/2)
    WORD wCode = VTDecoder.GetCNIFromPDC();
    if (wCode != 0)
    {
        // dwCode is a CNI from PDC
        BYTE uCountry = (wCode >> 8) & 0xFF;
        BYTE uNetwork = wCode & 0xFF;
        //LOG(1, "PDC Country %x Network %x", uCountry, uNetwork);
        for (int i(0); i < iNbRegisteredCNICodes; i++)
        {
            if (   (RegisteredCNICodes[i].uC_P8302 == uCountry)
                && (RegisteredCNICodes[i].uNI_P8302 == uNetwork) )
            {
                RetVal = RegisteredCNICodes[i].sNetwork;
                break;
            }
        }
    }
    return RetVal;
}


string VT_GetStation()
{
    return VTDecoder.GetStatusDisplay();
}


BOOL APIENTRY VTSearchProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_COMBO1, VTSearchString);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            GetDlgItemText(hDlg, IDC_COMBO1, VTSearchString, 40);
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            break;

        default:
            ; // do nothing
            break;

        }
        break;
    }

    return (FALSE);
}


BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    DWORD dwPageCode;
    char buffer[8];

    switch (message)
    {
    case WM_INITDIALOG:
        SetTimer(hDlg, 0, 200, NULL);
        // no break

    case WM_TIMER:
        SetDlgItemInt(hDlg, IDC_TEXT1, VTDecoder.GetReceivedPagesCount(), FALSE);

        if ((dwPageCode = VTDecoder.GetProcessingPageCode()) != 0)
        {
            sprintf(buffer, "%03x", LOWORD(dwPageCode));
            SetDlgItemText(hDlg, IDC_TEXT2, buffer);

            sprintf(buffer, "%X", HIWORD(dwPageCode));
            SetDlgItemText(hDlg, IDC_TEXT3, buffer);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_TEXT2, "null");
            SetDlgItemText(hDlg, IDC_TEXT3, "null");
        }
        sprintf(buffer, "%x", VTDecoder.GetNetworkIDFromP8301());
        SetDlgItemText(hDlg, IDC_TEXT4, buffer);
        sprintf(buffer, "%x", VTDecoder.GetCNIFromPDC());
        SetDlgItemText(hDlg, IDC_TEXT5, buffer);
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            KillTimer(hDlg, 0);
            EndDialog(hDlg, TRUE);
        }
        break;
    }

    return FALSE;
}



BOOL APIENTRY VTGotoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    char szBuffer[4];
    HWND hItem;

    switch (message)
    {
    case WM_INITDIALOG:
        {
            hItem = GetDlgItem(hDlg, IDC_VTPAGEGROUP);

            // Disable the use of XP visual styles on the IDC_VTPAGEGROUP
            // tab control because the vertical tabs aren't supported by
            // XP visual styles (comctl32.dll version 6).  Dynamically load
            // UxTheme.dll so its compatible with OSes before XP.
            DynamicFunctionS0<BOOL> pfnIsAppThemed("UxTheme.dll", "IsAppThemed");
            DynamicFunctionS3<HRESULT, HWND, LPCWSTR, LPCWSTR> pfnSetWindowThemed("UxTheme.dll", "SetWindowTheme");

            if (pfnIsAppThemed && pfnSetWindowThemed)
            {
                if (pfnIsAppThemed())
                {
                    pfnSetWindowThemed(hItem, L" ", L" ");
                }
            }

            TCITEM TCItem;
            TCItem.mask = TCIF_TEXT;

            TCItem.pszText = "All";
            TabCtrl_InsertItem(hItem, 0, &TCItem);

            TCItem.pszText = "Hex Pages";
            TabCtrl_InsertItem(hItem, 0, &TCItem);

            TCItem.pszText = "Normal Pages";
            int iItem = TabCtrl_InsertItem(hItem, 0, &TCItem);
            TabCtrl_SetCurSel(hItem, iItem);

            VTGotoProcDlg = hDlg;
            SendMessage(hDlg, WM_COMMAND, IDC_VTPAGEGROUP, 0);

            hItem = GetDlgItem(hDlg, IDC_VTPAGESELECT);
            SendMessage(hItem, CB_LIMITTEXT, 3, 0);

            SetFocus(hItem);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            {
                GetDlgItemText(hDlg, IDC_VTPAGESELECT, szBuffer, 4);
                SendDlgItemMessage(hDlg, IDC_VTPAGESELECT,
                    CB_SETEDITSEL, 0, MAKELPARAM(0, -1));

                WORD wPageHex = VT_Input2PageHex(szBuffer);

                if (wPageHex != 0)
                {
                    if (VT_GetState() == VT_OFF)
                    {
                        SendMessage(GetMainWnd(), WM_COMMAND, IDM_CALL_VIDEOTEXT, 0);
                    }

                    if (VT_SetPage(NULL, NULL, wPageHex))
                    {
                        InvalidateDisplayAreaRect(GetMainWnd(), NULL, FALSE);
                    }
                }
            }
            break;

        case IDCANCEL:
            VTGotoProcDlg = NULL;
            EndDialog(hDlg, TRUE);
            break;

        case IDC_VTPAGEGROUP:
            {
                int iItem = TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_VTPAGEGROUP));

                WORD wPageHexList[800];
                WORD nPagesCount;

                hItem = GetDlgItem(hDlg, IDC_VTPAGESELECT);
                SendMessage(hItem, CB_RESETCONTENT, 0, 0);

                if (iItem != 1)
                {
                    nPagesCount = VTDecoder.GetVisiblePageNumbers(wPageHexList, 800);
                    for (int i = 0; i < nPagesCount; i++)
                    {
                        sprintf(szBuffer, "%03X", wPageHexList[i] & 0xFFF);
                        SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)szBuffer);
                    }
                }

                if (iItem != 0)
                {
                    nPagesCount = VTDecoder.GetNonVisiblePageNumbers(wPageHexList, 800);
                    for (int i = 0; i < nPagesCount; i++)
                    {
                        sprintf(szBuffer, "%03X", wPageHexList[i] & 0xFFF);
                        SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)szBuffer);
                    }
                }
            }
            break;

        case IDC_VTPAGESELECT:
            if (HIWORD(wParam) == CBN_DBLCLK)
            {
                SendMessage(hDlg, WM_COMMAND, IDOK, 0);
            }
            break;
        }
        break;

    case WM_NOTIFY:
        if (wParam == IDC_VTPAGEGROUP)
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            if (pnmh->code == TCN_SELCHANGE)
            {
                SendMessage(hDlg, WM_COMMAND, IDC_VTPAGEGROUP, 0);
            }
        }
        break;

    case UWM_VIDEOTEXT:
        if (wParam == VTM_VTPAGEUPDATE)
        {
            WORD wPageHex = (WORD)(lParam & 0xFFF);

            int iItem = TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_VTPAGEGROUP));

            if (iItem == 2 || CVTCommon::IsNonVisiblePage(wPageHex) == (iItem == 1))
            {
                sprintf(szBuffer, "%03X", wPageHex);

                hItem = GetDlgItem(hDlg, IDC_VTPAGESELECT);
                if (SendMessage(hItem, CB_FINDSTRINGEXACT, 0, (LPARAM)szBuffer) == CB_ERR)
                {
                    SendMessage(hItem, CB_ADDSTRING, 0, (LPARAM)szBuffer);
                }
            }
        }
        break;
    }

    return FALSE;
}


static const char* VTCodepageName[VTCODEPAGE_LASTONE] =
{
    "English",
    "French",
    "Swedish/Finnish/Hungarian",
    "Czesh/Slovak",
    "German",
    "Portuguese/Spanish",
    "Italian",
    "Polish",
    "Turkish",
    "Serbian/Croation/Slovenian",
    "Rumanian",
    "Serbian/Croatian",
    "Russian/Bulgarian",
    "Estonian",
    "Ukrainian",
    "Lettish/Lithuanian",
    "Greek",
    NULL,
    NULL,
    "Arabic",
    "Hebrew",
    "Danish/Norwegian"
};


static const char* VTRegionName[VTREGION_LASTONE] =
{
    "0: Default",
    "1: Central Europe",
    "2: Southern Europe",
    "3: Balkans",
    "4: Eastern Europe",
    NULL,
    "5: Mediterranian",
    NULL,
    "6: North Africa",
    NULL,
    "7: Middle East",
};


static const char* VTCachingControlName[3] =
{
    "0: Normal Error Correction",
    "1: Second Chance Error Correction",
    "2: Always Update",
};


void VT_UpdateMenu(HMENU hMenu)
{
    hMenu = GetVTCodepageSubmenu();

    for (int i = 0; i < VTCODEPAGE_LASTONE; i++)
    {
        if (VTCodepageName[i] != NULL)
        {
            AppendMenu(hMenu, MF_STRING,
                IDM_VT_CODEPAGE_FIRST + i, VTCodepageName[i]);
        }
    }
}


void VT_SetMenu(HMENU hMenu)
{
    for (int i = 0; i < VTCODEPAGE_LASTONE; i++)
    {
        if (VTCodepageName[i] != NULL)
        {
            EnableMenuItem(hMenu, IDM_VT_CODEPAGE_FIRST + i,
                VTAutoCodepage ? MF_GRAYED : MF_ENABLED);
            CheckMenuItemBool(hMenu, IDM_VT_CODEPAGE_FIRST + i,
                i == VTCodepage);
        }
    }
}


BOOL ProcessVTCodepageSelection(HWND hWnd, WORD wMenuID)
{
    if (wMenuID >= IDM_VT_CODEPAGE_FIRST &&
        wMenuID < (IDM_VT_CODEPAGE_FIRST + VTCODEPAGE_LASTONE))
    {
        int i = wMenuID - IDM_VT_CODEPAGE_FIRST;

        if (VTCodepageName[i] != NULL)
        {
            VTUserCodepage = (eVTCodepage)i;

            if (!VT_GetAutoCodepage())
            {
                if (VTUserCodepage != VTCodepage)
                {
                    VTCodepage = VTUserCodepage;

                    if (VT_GetState() != VT_OFF)
                    {
                        InvalidateDisplayAreaRect(GetMainWnd(), NULL, FALSE);
                    }
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}


SETTING VTSettings[VT_SETTING_LASTONE] =
{
    {
        "Code Page", SLIDER, 0, (long*)&VTUserCodepage,
        VTCODEPAGE_ENGLISH, 0, VTCODEPAGE_LASTONE - 1, 1, 1,
        NULL,
        "VT", "CodePage", NULL,
    },
    {
        "Language Region", ITEMFROMLIST, 0, (long*)&VTRegion,
        VTREGION_DEFAULT, 0, VTREGION_LASTONE - 1, 1, 1,
        VTRegionName,
        "VT", "LanguageRegion", VT_RegionOnChange,
    },
    {
        "Show Page Subcode in OSD", ONOFF, 0, (long*)&VTShowSubcodeInOSD,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "ShowPageSubcodeInOSD", VT_ShowSubcodeInOSDOnChange,
    },
    {
        "Page Lines Caching Control", ITEMFROMLIST, 0, (long*)&VTCachingControl,
        1, 0, 2, 1, 1,
        VTCachingControlName,
        "VT", "PageLinesCachingControl", VT_CachingControlOnChange,
    },
    {
        "High Granularity Caching", ONOFF, 0, (long*)&VTHighGranularityCaching,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "HighGranularityCaching", VT_HighGranularityCachingOnChange,
    },
    {
        "Substitute Errors with Spaces", ONOFF, 0, (long*)&VTSubstituteErrorsWithSpaces,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "SubstituteErrorsWithSpaces", VT_SubstituteErrorsWithSpacesOnChange,
    },
    {
        "Filter: Upscroll Subtitle Duplication Remover", ONOFF, 0,
        (long*)&VTUpscrollDuplicationFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "UpscrollSubtitleDuplicationFilter", NULL,
    },
    {
        "Filter: Force Double Height Subtitles", ONOFF, 0,
        (long*)&VTDoubleHeightSubtitlesFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "VT", "ForceDoubleHeightSubtitlesFilter", NULL,
    },
};


SETTING* VT_GetSetting(VT_SETTING Setting)
{
    if (Setting > -1 && Setting < VT_SETTING_LASTONE)
    {
        return &VTSettings[Setting];
    }
    else
    {
        return NULL;
    }
}

SmartPtr<CTreeSettingsGeneric> VideoText_GetTreeSettingsPage()
{
    // Teletext Settings
    SmartPtr<CSettingsHolder> Holder(new CSettingsHolder);

    Holder->AddSetting(&VTSettings[VT_LANGUAGE_REGION              ]);
    Holder->AddSetting(&VTSettings[VT_SHOW_SUBCODE_OSD             ]);
    Holder->AddSetting(&VTSettings[VT_LINES_CACHINGCONTROL         ]);
    Holder->AddSetting(&VTSettings[VT_HIGH_GRANULARITY_CACHING     ]);
    Holder->AddSetting(&VTSettings[VT_SUBSTITUTE_ERROR_SPACES      ]);
    Holder->AddSetting(&VTSettings[VT_SUBTITLE_DUPLICATION_FILTER  ]);
    Holder->AddSetting(&VTSettings[VT_DOUBLEHEIGHT_SUBTITLES_FILTER]);

    return new CTreeSettingsGeneric("Teletext Settings", Holder);
}


BOOL VT_RegionOnChange(long NewValue)
{
    VTRegion = (eVTRegion)NewValue;

    if (VT_GetState() != VT_OFF)
    {
        if (VT_GetAutoCodepage())
        {
            if (VT_SetCodepage(NULL, NULL, VTCODEPAGE_NONE))
            {
                InvalidateDisplayAreaRect(GetMainWnd(), NULL, FALSE);
            }
        }
    }
    return FALSE;
}


BOOL VT_ShowSubcodeInOSDOnChange(long NewValue)
{
    VTShowSubcodeInOSD = NewValue ;
    VT_SetPageOSD(VTPageOSD);

    if (VT_GetState() != VT_OFF)
    {
        InvalidateDisplayAreaRect(GetMainWnd(), NULL, FALSE);
    }

    return FALSE;
}


BOOL VT_CachingControlOnChange(long NewValue)
{
    VTCachingControl = (BYTE)NewValue;
    VTDecoder.SetCachingControl(VTCachingControl);
    return FALSE;
}


BOOL VT_HighGranularityCachingOnChange(long NewValue)
{
    VTHighGranularityCaching = NewValue;
    VTDecoder.SetHighGranularityCaching(NewValue);
    return FALSE;
}


BOOL VT_SubstituteErrorsWithSpacesOnChange(long NewValue)
{
    VTSubstituteErrorsWithSpaces = NewValue;
    VTDecoder.SetSubstituteSpacesForError(NewValue);
    return FALSE;
}


BOOL VT_HilightSearchOnChange(long NewValue)
{
    bSearchHighlight = (BOOL)NewValue;
    VTHilightSearch = (BOOL)NewValue;
    VT_UpdateHilightList();
    return FALSE;
}


//////////////////////////////////////////////////////////////////////
void VBI_DecodeLine_VT(BYTE* VBI_Buffer)
{
    unsigned char data[45];
    unsigned char min, max;
    int dt[512], hi[6], lo[6];
    int i, n, sync, thr;

    // remove DC. edge-detector
    // \todo is this really required????
    // shouldn't we just use a better clock signal
    // detector
    // this seems to operate on the region betweens 8 and 48 bit worth of the
    // incoming signal
    // these values seem quite wide and may not really suit all cards
    // also may possible corrupt data real data
    for (i = (VTStep * 8 / FPFAC); i < (VTStep * 48 / FPFAC); ++i)
    {
        dt[i] = VBI_Buffer[i + VTStep / FPFAC] - VBI_Buffer[i]; // amplifies the edges best.
    }


    // find 6 rising and falling edges
    for (i = (VTStep * 8 / FPFAC), n = 0; n < 6 && i < (VTStep * 48 / FPFAC); ++n)
    {
        while (dt[i] < 32 && i < (VTStep * 48 / FPFAC))
        {
            i++;
        }
        hi[n] = i;
        while (dt[i] > -32 && i < (VTStep * 48 / FPFAC))
        {
            i++;
        }
        lo[n] = i;
    }
    // If we exited look after looking too far
    // then we haven't found the clock run-in
    if (i >= (VTStep * 48 / FPFAC))
    {
        return; // not enough periods found
    }

    // length of 4 periods (8 bits)
    // normally 40 ish for PAL @ 8*fsc
    // or 31 ish for PAL 27Mhz
    i = hi[5] - hi[1];

    // check that the found frequency is very close to what we expect it
    // to be.  We will use the precalculated one rather than the one we've
    // just locked to
    if ( (i - (VTStep * 8 / FPFAC)) < -1 || (i - (VTStep * 8 / FPFAC)) > 1)
    {
        return; // bad frequency
    }

    // AGC and sync-reference
    min = 255;
    max = 0;
    sync = 0;

    for (i = hi[4]; i < hi[5]; ++i)
    {
        if (VBI_Buffer[i] > max)
        {
            max = VBI_Buffer[i];
            sync = i;
        }
    }
    for (i = lo[4]; i < lo[5]; ++i)
    {
        if (VBI_Buffer[i] < min)
        {
            min = VBI_Buffer[i];
        }
    }

    thr = (min + max) / 2;

    // search start-byte 11100100
    for (i = 4 * VTStep; i < (int)(16*VTStep); i += VTStep)
    {
        if (VBI_Buffer[sync + i/FPFAC] > thr && VBI_Buffer[sync + (i+VTStep)/FPFAC] > thr) // two ones is enough...
        {
            // got it...
            memset(data, 0, sizeof(data));
            data[0] = 0x55;
            data[1] = 0x55;
            for (n = 0; n < 43 * 8; ++n, i += VTStep)
            {
                if (VBI_Buffer[sync + i/FPFAC] +
                    VBI_Buffer[sync + i/FPFAC - 1] +
                    VBI_Buffer[sync + i/FPFAC + 1] > 3 * thr)
                {
                    data[2 + n/8] |= 1 << (n % 8);
                }
            }

            if (data[2] != 0x27)    // really 11100100? (rev order!)
            {
                return;
            }

            VTDecoder.DecodeLine(data);
        }
    }
    return;
}

