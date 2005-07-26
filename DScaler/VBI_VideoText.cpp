/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_VideoText.cpp,v 1.77 2005-07-26 23:13:59 laurentg Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 02 Jan 2003   Atsushi Nakagawa      Redid VBI_VideoText.cpp to link in
//                                     with the new and updated CVTDecoder and
//                                     CVTDrawer classes.  CVS Log entries
//                                     prior to and including rev 1.52 may
//                                     no longer have direct validity but
//                                     their cumulative changes still remain
//                                     valid.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.76  2005/07/26 22:14:17  laurentg
// Function to search network names using P8/30/1,2 and a table of registered CNI codes
// Table not yet fully formatted
//
// Revision 1.75  2005/03/23 14:21:02  adcockj
// Test fix for threading issues
//
// Revision 1.74  2004/11/08 18:15:24  atnak
// Made UxTheme dynamically load for backwards compatibility
//
// Revision 1.73  2004/11/08 16:12:27  atnak
// Fix to strange problem with vertical tab control when using XP visual style
//
// Revision 1.72  2004/04/24 11:34:51  atnak
// minor fix
//
// Revision 1.71  2004/04/24 08:54:18  atnak
// reverted part of last change because there was no need for a new setting
// variable for input timeout, used ChannelEnterTime instead
//
// Revision 1.70  2004/04/24 08:36:28  atnak
// new: user customizable teletext page number input timeout
//
// Revision 1.69  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.68  2003/03/31 16:13:23  atnak
// Changed default for page lines caching control
//
// Revision 1.67  2003/02/05 06:58:28  atnak
// Added Danish codepage submitted by Kristian Trenskow (trenskow)
//
// Revision 1.66  2003/01/26 12:34:50  adcockj
// Fixed crash with PAL60
//
// Revision 1.65  2003/01/24 01:55:17  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.64  2003/01/12 17:12:45  atnak
// Added hex pages display and goto dialog
//
// Revision 1.63  2003/01/07 18:40:18  adcockj
// Fixed silly bug in new teletext code
//
// Revision 1.62  2003/01/07 16:49:11  adcockj
// Changes to allow variable sampling rates for VBI
//
// Revision 1.61  2003/01/07 07:37:38  atnak
// Fixed page subcodes
//
// Revision 1.60  2003/01/05 18:35:45  laurentg
// Init function for VBI added
//
// Revision 1.59  2003/01/05 16:09:44  atnak
// Updated TopText for new teletext
//
// Revision 1.58  2003/01/03 14:44:19  robmuller
// Removed wPageSubCode from VT_PerformFlofKey() as suggested by Atsushi.
//
// Revision 1.57  2003/01/02 21:26:33  atnak
// Only redraw the clock if it changed
//
// Revision 1.56  2003/01/02 14:48:50  atnak
// Added Teletext Settings page
//
// Revision 1.55  2003/01/02 11:05:24  atnak
// Added missing InitialTextPage implementation
//
// Revision 1.54  2003/01/01 21:34:11  atnak
// Added missing subtitles force double height filter
//
// Revision 1.53  2003/01/01 20:49:03  atnak
// Updated VBI_VideoText.* files for new videotext structure
//
// Revision 1.52  2002/10/30 13:37:52  atnak
// Added "Single key teletext toggle" option. (Enables mixed mode menu item)
//
// Revision 1.51  2002/10/23 16:57:13  atnak
// Added TOP-Text support
//
// Revision 1.50  2002/10/15 11:53:38  atnak
// Added UI feedback for some videotext stuff
//
// Revision 1.49  2002/10/15 03:36:29  atnak
// removed rounding from last commit - it wasn't necessary
//
// Revision 1.48  2002/10/15 02:02:58  atnak
// Added rounding in VT decoding to improve accuracy
//
// Revision 1.47  2002/10/13 08:36:14  atnak
// fix to portions of page not updated after subtitle/newsflash change
//
// Revision 1.46  2002/10/12 04:33:53  atnak
// flashing conceal teletext fix, header clear on channel change
//
// Revision 1.45  2002/09/07 20:59:45  kooiman
// Small fixes.
//
// Revision 1.44  2002/06/20 20:00:35  robmuller
// Implemented videotext search highlighting.
//
// Revision 1.43  2002/05/30 19:09:06  robmuller
// Clear VisiblePage on channel change.
//
// Revision 1.42  2002/05/30 10:13:43  robmuller
// Don't search the header line.
//
// Revision 1.41  2002/05/29 18:44:52  robmuller
// Added option to disable font anti-aliasing in Teletext.
//
// Revision 1.40  2002/05/28 20:04:26  robmuller
// Addition to patch #561180 by PietOO:
// In auto code page mode the code page options are grayed and the current code page is checked.
//
// Revision 1.39  2002/05/27 20:17:05  robmuller
// Patch #561180  by PietOO:
// Autodetection of teletext code page.
//
// Revision 1.38  2002/05/24 16:49:00  robmuller
// VideoText searching improved.
//
// Revision 1.37  2002/05/23 18:45:03  robmuller
// Patch #559554 by PietOO.
// Teletext: + text search ctrl-F & next F3
//
// Revision 1.36  2002/03/12 23:29:44  robmuller
// Implemented functions VT_GetNextPage() and VT_GetPreviousPage().
//
// Revision 1.35  2002/02/24 08:18:03  temperton
// TIMER_VTFLASHER set only when displayed page contains flashed elements and only in teletext modes.
//
// Revision 1.34  2002/02/07 13:04:54  temperton
// Added Spanish and Polish teletext code pages. Thanks to Jazz (stawiarz).
//
// Revision 1.33  2002/01/19 17:50:41  robmuller
// Clear more data on channel change.
//
// Revision 1.32  2002/01/19 12:53:00  temperton
// Teletext pages updates at correct time.
// Teletext can use variable-width font.
//
// Revision 1.31  2002/01/15 20:25:45  adcockj
// removed old bitmap code
//
// Revision 1.30  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
// Revision 1.29  2002/01/12 16:56:21  adcockj
// Series of fixes to bring 4.0.0 into line with 3.1.1
//
// Revision 1.28  2001/11/26 12:48:01  temperton
// Teletext corrections
//
// Revision 1.27  2001/11/23 10:45:32  adcockj
// Added Hebrew and Hungarian codepages
//
// Revision 1.26  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.25  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.24  2001/10/22 17:28:40  temperton
// Removed tabs to comply with coding standards
//
// Revision 1.23  2001/10/22 05:55:07  temperton
// Teletext improvements
//
// Revision 1.22  2001/10/06 17:04:26  adcockj
// Fixed teletext crashing problems
//
// Revision 1.21  2001/09/22 11:09:43  adcockj
// Fixed crashing problems with new code with noisy input
//
// Revision 1.20  2001/09/21 16:43:54  adcockj
// Teletext improvements by Mike Temperton
//
// Revision 1.19  2001/09/21 15:39:02  adcockj
// Added Russian and German code pages
// Corrected UK code page
//
// Revision 1.18  2001/09/05 16:22:34  adcockj
// Fix for new teletext painting overwritting other apps
//
// Revision 1.17  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.16  2001/09/05 06:59:13  adcockj
// Teletext fixes
//
// Revision 1.15  2001/09/02 14:17:51  adcockj
// Improved teletext code
//
// Revision 1.14  2001/08/21 09:39:46  adcockj
// Added Greek teletext Codepage
//
// Revision 1.13.2.2  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.13.2.1  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.13  2001/08/13 18:07:24  adcockj
// Added Czech code page for teletext
//
// Revision 1.12  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.11  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.10  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.9  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

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
//#include "DebugLog.h"


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


struct {
	char*	sCountry;
	char*	sNetwork;
	DWORD	dwNI_P8301;
	BYTE	uC_P8302;
	BYTE	uNI_P8302;
	BYTE	uA_X26;
	BYTE	uB_X26;
} RegisteredCNICodes[] = 
{
{	"Austria",		"ORF-1",					0x4301,	0,	0,	0,	0	},
{	"Austria",		"ORF-2",					0x4302,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x4303,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x4304,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x4305,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x4306,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x4307,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x4308,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x4309,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x430A,	0,	0,	0,	0	},
{	"Austria",		"ORF future use",			0x430B,	0,	0,	0,	0	},
{	"Austria",		"ATV",						0x430C,	0,	0,	0,	0	},
{	"Belgium",		"AB3",						0x320C,	0,	0,	0,	0	},
{	"Belgium",		"AB4e",						0x320D,	0,	0,	0,	0	},
{	"Belgium",		"VRT TV1",					0x3201,	0x16,	0x01,	0x36,	0x03	},
{	"Belgium",		"CANVAS",					0x3202,	0x16,	0x02,	0x36,	0x02	},
{	"Belgium",		"RTBF 1",					0x3203,	0,	0,	0,	0	},
{	"Belgium",		"RTBF 2",					0x3204,	0,	0,	0,	0	},
{	"Belgium",		"VTM",						0x3205,	0x16,	0x05,	0x36,	0x05	},
{	"Belgium",		"Kanaal2",					0x3206,	0x16,	0x06,	0x36,	0x06	},
{	"Belgium",		"RTBF Sat",					0x3207,	0,	0,	0,	0	},
{	"Belgium",		"RTBF future use",			0x3208,	0,	0,	0,	0	},
{	"Belgium",		"RTL-TVI",					0x3209,	0,	0,	0,	0	},
{	"Belgium",		"CLUB-RTL",					0x320A,	0,	0,	0,	0	},
{	"Belgium",		"VT4",						0x0404,	0x16,	0x04,	0x36,	0x04	},
{	"Belgium",		"JIM.tv",					0x320F,	0,	0,	0,	0	},
{	"Belgium",		"PLUG TV",					0x3225,	0,	0,	0,	0	},
{	"Belgium",		"RTV-Kempen",				0x3210,	0,	0,	0,	0	},
{	"Belgium",		"RTV-Mechelen",				0x3211,	0,	0,	0,	0	},
{	"Belgium",		"MCM Belgium",				0x3212,	0,	0,	0,	0	},
{	"Belgium",		"Vitaya",					0x3213,	0,	0,	0,	0	},
{	"Belgium",		"WTV",						0x3214,	0,	0,	0,	0	},
{	"Belgium",		"FocusTV",					0x3215,	0,	0,	0,	0	},
{	"Belgium",		"Be 1 ana",					0x3216,	0,	0,	0,	0	},
{	"Belgium",		"Be 1 num",					0x3217,	0,	0,	0,	0	},
{	"Belgium",		"Be Ciné 1",				0x3218,	0,	0,	0,	0	},
{	"Belgium",		"Be Sport 1",				0x3219,	0,	0,	0,	0	},
{	"Belgium",		"Be 1 + 1h",				0x32A7,	0,	0,	0,	0	},
{	"Belgium",		"Be Ciné 2",				0x32A8,	0,	0,	0,	0	},
{	"Belgium",		"Be Sport 2",				0x32A9,	0,	0,	0,	0	},
{	"Belgium",		"Canal+VL1",				0x321A,	0,	0,	0,	0	},
{	"Belgium",		"Canal+VL1",				0x321B,	0,	0,	0,	0	},
{	"Belgium",		"Canal+ Blau",				0x321C,	0,	0,	0,	0	},
{	"Belgium",		"Canal+ Rood",				0x321D,	0,	0,	0,	0	},
{	"Belgium",		"TV Limburg",				0x3221,	0,	0,	0,	0	},
{	"Belgium",		"Kanaal 3",					0x3222,	0,	0,	0,	0	},
{	"Belgium",		"Ring TV",					0x320E,	0,	0,	0,	0	},
{	"Belgium",		"TV Brussel",				0x321E,	0,	0,	0,	0	},
{	"Belgium",		"AVSe",						0x321F,	0,	0,	0,	0	},
{	"Belgium",		"ATV",						0x3223,	0,	0,	0,	0	},
{	"Belgium",		"ROB TV",					0x3224,	0,	0,	0,	0	},
{	"Belgium",		"Télé Bruxelles",			0x3230,	0,	0,	0,	0	},
{	"Belgium",		"Télésambre",				0x3231,	0,	0,	0,	0	},
{	"Belgium",		"TV Com",					0x3232,	0,	0,	0,	0	},
{	"Belgium",		"Canal Zoom",				0x3233,	0,	0,	0,	0	},
{	"Belgium",		"Vidéoscope",				0x3234,	0,	0,	0,	0	},
{	"Belgium",		"Canal C",					0x3235,	0,	0,	0,	0	},
{	"Belgium",		"Télé MB",					0x3236,	0,	0,	0,	0	},
{	"Belgium",		"Antenne Centre",			0x3237,	0,	0,	0,	0	},
{	"Belgium",		"Télévesdre",				0x3238,	0,	0,	0,	0	},
{	"Belgium",		"RTC Télé Liège",			0x3239,	0,	0,	0,	0	},
{	"Belgium",		"No tele",					0x3240,	0,	0,	0,	0	},
{	"Belgium",		"TV Lux",					0x3241,	0,	0,	0,	0	},
{	"Belgium",		"Kanaal Z - NL",			0x325A,	0,	0,	0,	0	},
{	"Belgium",		"CANAL Z - FR",				0x325B,	0,	0,	0,	0	},
{	"Belgium",		"CARTOON Network - NL",		0x326A,	0,	0,	0,	0	},
{	"Belgium",		"CARTOON Network - FR",		0x326B,	0,	0,	0,	0	},
{	"Belgium",		"LIBERTY CHANNEL - NL",		0x327A,	0,	0,	0,	0	},
{	"Belgium",		"LIBERTY CHANNEL - FR",		0x327B,	0,	0,	0,	0	},
{	"Belgium",		"TCM - NL",					0x328A,	0,	0,	0,	0	},
{	"Belgium",		"TCM - FR",					0x328B,	0,	0,	0,	0	},
{	"Belgium",		"Mozaiek/Mosaique",			0x3298,	0,	0,	0,	0	},
{	"Belgium",		"Info Kanaal/Canal Info",	0x3299,	0,	0,	0,	0	},
{	"Belgium",		"Sporza",					0x3226,	0,	0,	0,	0	},
{	"Belgium",		"VIJF tv",					0x3227,	0,	0,	0,	0	},
{	"Croatia",		"HRT",						0x0385,	0,	0,	0,	0	},
{	"Czech Republic","CT 1",					0x4201,	0x32,	0xC1,	0x3C,	0x21	},
{	"Czech Republic","CT 2",					0x4202,	0x32,	0xC2,	0x3C,	0x22	},
{	"Czech Republic","CT1 Regional",			0x4231,	0x32,	0xF1,	0x3C,	0x25	},
{	"Czech Republic","CT1 Regional, Brno",		0x4211,	0x32,	0xD1,	0x3B,	0x01	},
{	"Czech Republic","CT1 Regional, Ostravia",	0x4221,	0x32,	0xE1,	0x3B,	0x02	},
{	"Czech Republic","CT2 Regional",			0x4232,	0x32,	0xF2,	0x3B,	0x03	},
{	"Czech Republic","CT2 Regional, Brno",		0x4212,	0x32,	0xD2,	0x3B,	0x04	},
{	"Czech Republic","CT2 Regional, Ostravia",	0x4222,	0x32,	0xE2,	0x3B,	0x05	},
{	"Czech Republic","NOVA TV",					0x4203,	0x32,	0xC3,	0x3C,	0x23	},
{	"Czech Republic","Prima TV",				0x4204,	0x32,	0xC4,	0x3C,	0x04	},
{	"Czech Republic","TV Praha",				0x4205,	0,	0,	0,	0	},
{	"Czech Republic","TV HK",					0x4206,	0,	0,	0,	0	},
{	"Czech Republic","TV Pardubice",			0x4207,	0,	0,	0,	0	},
{	"Czech Republic","TV Brno",					0x4208,	0,	0,	0,	0	},
{	"Denmark",		"Discovery Denmark",		0x4504,	0,	0,	0,	0	},
{	"Denmark",		"DR1",						0x7392,	0x29,	0x01,	0x39,	0x01	},
{	"Denmark",		"DR2",						0x49CF,	0x29,	0x03,	0x39,	0x03	},
{	"Denmark",		"TV 2",						0x4502,	0x29,	0x02,	0x39,	0x02	},
{	"Denmark",		"TV 2 0xZulu",				0x4503,	0x29,	0x04,	0x39,	0x04	},
{	"Denmark",		"TV 2 0xCharlie",			0x4505,	0x29,	0x05,	0,	0	},
{	"Denmark",		"TV 2 Film",				0x4508,	0x29,	0x08,	0,	0	},
{	"Denmark",		"TV Danmark",				0x4506,	0x29,	0x06,	0,	0	},
{	"Denmark",		"Kanal 5",					0x4507,	0x29,	0x07,	0,	0	},
{	"Finland",		"OWL3",						0x358F,	0x26,	0x0F,	0x36,	0x14	},
{	"Finland",		"YLE future use",			0x3583,	0x26,	0x03,	0x36,	0x08	},
{	"Finland",		"YLE future use",			0x3584,	0x26,	0x04,	0x36,	0x09	},
{	"Finland",		"YLE future use",			0x3585,	0x26,	0x05,	0x36,	0x0A	},
{	"Finland",		"YLE future use",			0x3586,	0x26,	0x06,	0x36,	0x0B	},
{	"Finland",		"YLE future use",			0x3587,	0x26,	0x07,	0x36,	0x0C	},
{	"Finland",		"YLE future use",			0x3588,	0x26,	0x08,	0x36,	0x0D	},
{	"Finland",		"YLE future use",			0x3589,	0x26,	0x09,	0x36,	0x0E	},
{	"Finland",		"YLE future use",			0x358A,	0x26,	0x0A,	0x36,	0x0F	},
{	"Finland",		"YLE future use",			0x358B,	0x26,	0x0B,	0x36,	0x10	},
{	"Finland",		"YLE future use",			0x358C,	0x26,	0x0C,	0x36,	0x11	},
{	"Finland",		"YLE future use",			0x358D,	0x26,	0x0D,	0x36,	0x12	},
{	"Finland",		"YLE future use",			0x358E,	0x26,	0x0E,	0x36,	0x13	},
{	"Finland",		"YLE1",						0x3581,	0x26,	0x01,	0x36,	0x01	},
{	"Finland",		"YLE2",						0x3582,	0x26,	0x02,	0x36,	0x07	},
{	"France",		"AB1",						0x33C1,	0x2F,	0xC1,	0x3F,	0x41	},
{	"France",		"Aqui TV",					0x3320,	0x2F,	0x20,	0x3F,	0x20	},
{	"France",		"France 5 / Arte",			0x330A,	0x2F,	0x0A,	0x3F,	0x0A	},
{	"France",		"Canal J",					0x33C2,	0x2F,	0xC2,	0x3F,	0x42	},
{	"France",		"Canal Jimmy",				0x33C3,	0x2F,	0xC3,	0x3F,	0x43	},
{	"France",		"Canal+",					0x33F4,	0x2F,	0x04,	0x3F,	0x04	},
{	"France",		"Euronews",					0xFE01,	0x2F,	0xE1,	0x3F,	0x61	},
{	"France",		"Eurosport",				0xF101,	0x2F,	0xE2,	0x3F,	0x62	},
{	"France",		"France 2",					0x33F2,	0x2F,	0x02,	0x3F,	0x02	},
{	"France",		"France 3",					0x33F3,	0x2F,	0x03,	0x3F,	0x03	},
{	"France",		"La Chaîne Météo",			0x33C5,	0x2F,	0xC5,	0x3F,	0x45	},
{	"France",		"LCI",						0x33C4,	0x2F,	0xC4,	0x3F,	0x44	},
{	"France",		"M6",						0x33F6,	0x2F,	0x06,	0x3F,	0x06	},
{	"France",		"MCM",						0x33C6,	0x2F,	0xC6,	0x3F,	0x46	},
{	"France",		"Paris Première",			0x33C8,	0x2F,	0xC8,	0x3F,	0x48	},
{	"France",		"Planète",					0x33C9,	0x2F,	0xC9,	0x3F,	0x49	},
{	"France",		"RFO1",						0x3311,	0x2F,	0x11,	0x3F,	0x11	},
{	"France",		"RFO2",						0x3312,	0x2F,	0x12,	0x3F,	0x12	},
{	"France",		"Sailing Channel",			0x33B2,	0,	0,	0,	0	},
{	"France",		"Série Club",				0x33CA,	0x2F,	0xCA,	0x3F,	0x4A	},
{	"France",		"Télétoon",					0x33CB,	0x2F,	0xCB,	0x3F,	0x4B	},
{	"France",		"Téva",						0x33CC,	0x2F,	0xCC,	0x3F,	0x4C	},
{	"France",		"TF1",						0x33F1,	0x2F,	0x01,	0x3F,	0x01	},
{	"France",		"TLM",						0x3321,	0x2F,	0x21,	0x3F,	0x21	},
{	"France",		"TLT",						0x3322,	0x2F,	0x22,	0x3F,	0x22	},
{	"France",		"TMC Monte-Carlo",			0x33C7,	0x2F,	0xC7,	0x3F,	0x47	},
{	"France",		"TV5",						0xF500,	0x2F,	0xE5,	0x3F,	0x65	},
{	"Germany",		"3SAT",						0x49C7,	0,	0,	0,	0	},
{	"Germany",		"ARD",						0x4901,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49C1,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49C3,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49C4,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49C5,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49C6,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49CA,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49CC,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49CD,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49CE,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D0,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D1,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D2,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D3,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D5,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D6,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D7,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49D8,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49DA,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49DB,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49DD,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49DE,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49E0,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49E2,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49E3,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49E5,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49E7,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49E8,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49E9,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49EA,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49EB,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49EC,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49ED,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49EE,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49EF,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F0,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F1,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F2,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F3,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F4,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F5,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F6,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F7,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F8,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49F9,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49FA,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49FB,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49FC,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x49FD,	0,	0,	0,	0	},
{	"Germany",		"ARD future use",			0x4981,	0,	0,	0,	0	},
{	"Germany",		"Arte",						0x490A,	0,	0,	0,	0	},
{	"Germany",		"BR",						0x49CB,	0,	0,	0,	0	},
{	"Germany",		"BR-Alpha",					0x4944,	0,	0,	0,	0	},
{	"Germany",		"EXTRA",					0x4943,	0,	0,	0,	0	},
{	"Germany",		"Festival",					0x4941,	0,	0,	0,	0	},
{	"Germany",		"HR",						0x49FF,	0,	0,	0,	0	},
{	"Germany",		"Kinderkanal",				0x49C9,	0,	0,	0,	0	},
{	"Germany",		"MDR",						0x49FE,	0,	0,	0,	0	},
{	"Germany",		"MUXX",						0x4942,	0,	0,	0,	0	},
{	"Germany",		"NDR",						0x49D4,	0,	0,	0,	0	},
{	"Germany",		"ORB",						0x4982,	0,	0,	0,	0	},
{	"Germany",		"Phoenix",					0x4908,	0,	0,	0,	0	},
{	"Germany",		"QVC D Gmbh",				0x5C49,	0,	0,	0,	0	},
{	"Germany",		"RB",						0x49D9,	0,	0,	0,	0	},
{	"Germany",		"SFB",						0x49DC,	0,	0,	0,	0	},
{	"Germany",		"SR",						0x49DF,	0,	0,	0,	0	},
{	"Germany",		"SWR-BW",					0x49E1,	0,	0,	0,	0	},
{	"Germany",		"SWR-RP",					0x49E4,	0,	0,	0,	0	},
{	"Germany",		"1-2-3.TV",					0x49BD,	0,	0,	0,	0	},
{	"Germany",		"TELE-5",					0x49BE,	0,	0,	0,	0	},
{	"Germany",		"Home Shopping Europe",		0x49BF,	0,	0,	0,	0	},
{	"Germany",		"VOX Television",			0x490C,	0,	0,	0,	0	},
{	"Germany",		"WDR",						0x49E6,	0,	0,	0,	0	},
{	"Germany",		"ZDF",						0x4902,	0,	0,	0,	0	},
/*
{	"Greece ET future use 3004 21 04 31 04
{	"Greece ET future use 3005 21 05 31 05
{	"Greece ET future use 3006 21 06 31 06
{	"Greece ET future use 3007 21 07 31 07
{	"Greece ET future use 3008 21 08 31 08
{	"Greece ET future use 3009 21 09 31 09
{	"Greece ET future use 300A 21 0A 31 0A
{	"Greece ET future use 300B 21 0B 31 0B
{	"Greece ET future use 300C 21 0C 31 0C
{	"Greece ET future use 300D 21 0D 31 0D
{	"Greece ET future use 300E 21 0E 31 0E
{	"Greece ET future use 300F 21 0F 31 0F
{	"Greece ET-1 3001 21 01 31 01
{	"Greece ET-3 3003 21 03 31 03
{	"Greece NET 3002 21 02 31 02
{	"Hungary Duna Televizio 3636
{	"Hungary MTV1 3601
{	"Hungary MTV1 future use 3681
{	"Hungary MTV1 regional, Budapest 3611
{	"Hungary MTV1 regional, Debrecen 3651
{	"Hungary MTV1 regional, Miskolc 3661
{	"Hungary MTV1 regional, Pécs 3621
{	"Hungary MTV1 regional, Szeged 3631
{	"Hungary MTV1 regional, Szombathely 3641
{	"Hungary MTV2 3602
{	"Hungary MTV2 future use 3682
{	"Hungary tv2 3622
{	"Hungary tv2 future use 3620
{	"Iceland Rikisutvarpid-Sjonvarp 3541
{	"Iceland Network 2 3532 42 02 32 02
{	"Iceland RTE future use 3534 42 04 32 04
{	"Iceland RTE future use 3535 42 05 32 05
{	"Iceland RTE future use 3536 42 06 32 06
{	"Iceland RTE future use 3537 42 07 32 07
{	"Iceland RTE future use 3538 42 08 32 08
{	"Iceland RTE future use 3539 42 09 32 09
{	"Iceland RTE future use 353A 42 0A 32 0A
{	"Iceland RTE future use 353B 42 0B 32 0B
{	"Iceland RTE future use 353C 42 0C 32 0C
{	"Iceland RTE future use 353D 42 0D 32 0D
{	"Iceland RTE future use 353E 42 0E 32 0E
{	"Iceland RTE future use 353F 42 0F 32 0F
{	"Iceland RTE1 3531 42 01 32 01
{	"Iceland Teilifis na Gaeilge 3533 42 03 32 03
{	"Iceland TV3 3333
{	"Italy RAI 1 3901
{	"Italy RAI 2 3902
{	"Italy RAI 3 3903
{	"Italy Rete A 3904
{	"Italy Canale Italia 3905 15 05
{	"Italy Telenova 3909
{	"Italy Arte 390A
{	"Italy TRS TV 3910
{	"Italy Sky Cinema Classic 3911 15 11
{	"Italy Sky Future use (canale 109) 3912 15 12
{	"Italy Sky Calcio 1 3913 15 13
{	"Italy Sky Calcio 2 3914 15 14
{	"Italy Sky Calcio 3 3915 15 15
{	"Italy Sky Calcio 4 3916 15 16
{	"Italy Sky Calcio 5 3917 15 17
{	"Italy Sky Calcio 6 3918 15 18
{	"Italy Sky Calcio 7 3919 15 19
{	"Italy RaiNews24 3920
{	"Italy RAI Med 3921
{	"Italy RAI Sport 3922
{	"Italy RAI Educational 3923
{	"Italy RAI Edu Lab 3924
{	"Italy RAI Nettuno 1 3925
{	"Italy RAI Nettuno 2 3926
{	"Italy Camera Deputati 3927
{	"Italy RAI Mosaico 3928
{	"Italy RAI future use 3929
{	"Italy RAI future use 392A
{	"Italy RAI future use 392B
{	"Italy RAI future use 392C
{	"Italy RAI future use 392D
{	"Italy RAI future use 392E
{	"Italy RAI future use 392F
{	"Italy Discovery Italy 3930
{	"Italy MTV Italia 3933
{	"Italy MTV Brand New 3934
{	"Italy MTV Hits 3935
{	"Italy RTV38 3938
{	"Italy GAY TV 3939
{	"Italy Video Italia 3940
{	"Italy SAT 2000 3941
{	"Italy Jimmy 3942 15 42
{	"Italy Planet 3943 15 43
{	"Italy Cartoon Network 3944 15 44
{	"Italy Boomerang 3945 15 45
{	"Italy CNN International 3946 15 46
{	"Italy Cartoon Network +1 3947 15 47
{	"Italy Sky Sports 3 3948 15 48
{	"Italy Sky Diretta Gol 3949 15 49
{	"Italy RAISat Album 3950
{	"Italy RAISat Art 3951
{	"Italy RAISat Cinema 3952
{	"Italy RAISat Fiction 3953
{	"Italy RAISat GamberoRosso 3954
{	"Italy RAISat Ragazzi 3955
{	"Italy RAISat Show 3956
{	"Italy RAISat G. Rosso interattivo 3957
{	"Italy RAISat future use 3958
{	"Italy RAISat future use 3959
{	"Italy RAISat future use 395A
{	"Italy RAISat future use 395B
{	"Italy RAISat future use 395C
{	"Italy RAISat future use 395D
{	"Italy RAISat future use 395E
{	"Italy RAISat future use 395F
{	"Italy SCI FI CHANNEL 3960 15 60
{	"Italy Discovery Civilisations 3961
{	"Italy Discovery Travel and Adventure 3962
{	"Italy Discovery Science 3963
{	"Italy Sky Meteo24 3968 15 68
{	"Italy Sky Cinema 2 3970
{	"Italy Sky Cinema 3 3971
{	"Italy Sky Cinema Autore 3972
{	"Italy Sky Cinema Max 3973
{	"Italy Sky Cinema 16:9 3974
{	"Italy Sky Sports 2 3975
{	"Italy Sky TG24 3976
{	"Italy Fox 3977 15 77
{	"Italy Foxlife 3978 15 78
{	"Italy National Geographic Channel 3979 15 79
{	"Italy A1 3980 15 80
{	"Italy History Channel 3981 15 81
{	"Italy FOX KIDS 3985
{	"Italy PEOPLE TV – RETE 7 3986
{	"Italy FOX KIDS +1 3987
{	"Italy LA7 3988
{	"Italy PrimaTV 3989
{	"Italy SportItalia 398A
{	"Italy STUDIO UNIVERSAL 3990 15 90
{	"Italy Marcopolo 3991 15 91
{	"Italy Alice 3992 15 92
{	"Italy Nuvolari 3993 15 93
{	"Italy Leonardo 3994 15 94
{	"Italy SUPERPIPPA CHANNEL 3996 15 96
{	"Italy Sky Sports 1 3997
{	"Italy Sky Cinema 1 3998
{	"Italy Tele+3 3999
{	"Italy Sky Calcio 8 39A0 15 A0
{	"Italy Sky Calcio 9 39A1 15 A1
{	"Italy Sky Calcio 10 39A2 15 A2
{	"Italy Sky Calcio 11 39A3 15 A3
{	"Italy Sky Calcio 12 39A4 15 A4
{	"Italy Sky Calcio 13 39A5 15 A5
{	"Italy Sky Calcio 14 39A6 15 A6
{	"Italy Telesanterno 39A7 15 A7
{	"Italy Telecentro 39A8 15 A8
{	"Italy Telestense 39A9 15 A9
{	"Italy Disney Channel +1 39B0 15 B0
{	"Italy Sailing Channel 39B1
{	"Italy Disney Channel 39B2 15 B2
{	"Italy 7 Gold-Sestra Rete 39B3 15 B3
{	"Italy Rete 8-VGA 39B4 15 B4
{	"Italy Nuovarete 39B5 15 B5
{	"Italy Radio Italia TV 39B6 15 B6
{	"Italy Rete 7 39B7 15 B7
{	"Italy E! Entertainment Television 39B8 15 B8
{	"Italy Toon Disney 39B9 15 B9
{	"Italy Bassano TV 39C7 15 C7
{	"Italy ESPN Classic Sport 39C8 15 C8
{	"Italy VIDEOLINA 39CA
{	"Italy Mediaset Premium 1 39D2 15 D2
{	"Italy Mediaset Premium 2 39D3 15 D3
{	"Italy Mediaset Premium 3 39D4 15 D4
{	"Italy Mediaset Premium 4 39D5 15 D5
{	"Italy BOING 39D6 15 D6
{	"Italy Playlist Italia 39D7 15 D7
{	"Italy MATCH MUSIC 39D8 15 D8
{	"Italy National Geographic +1 39E1 15 E1
{	"Italy History Channel +1 39E2 15 E2
{	"Italy Sky TV 39E3 15 E3
{	"Italy GXT 39E4 15 E4
{	"Italy Playhouse Disney 39E5 15 E5
{	"Italy Sky Canale 224 39E6 15 E6
{	"Italy Rete 4 FA04
{	"Italy Canale 5 FA05
{	"Italy Italia 1 FA06
{	"Luxembourg RTL Télé Lëtzebuerg 4000
{	"Netherlands Nederland 1 3101 48 01 38 01
{	"Netherlands Nederland 2 3102 48 02 38 02
{	"Netherlands Nederland 3 3103 48 03 38 03
{	"Netherlands RTL 4 3104 48 04 38 04
{	"Netherlands RTL 5 3105 48 05 38 05
{	"Netherlands Yorin 3106 48 06 38 06
{	"Netherlands NOS future use 3110
{	"Netherlands NOS future use 3111
{	"Netherlands NOS future use 3112
{	"Netherlands NOS future use 3113
{	"Netherlands NOS future use 3114
{	"Netherlands NOS future use 3115
{	"Netherlands NOS future use 3116
{	"Netherlands NOS future use 3117
{	"Netherlands NOS future use 3118
{	"Netherlands NOS future use 3119
{	"Netherlands NOS future use 311A
{	"Netherlands NOS future use 311B
{	"Netherlands NOS future use 311C
{	"Netherlands NOS future use 311D
{	"Netherlands NOS future use 311E
{	"Netherlands NOS future use 311F
{	"Netherlands NOS future use 3107 48 07 38 07
{	"Netherlands NOS future use 3108 48 08 38 08
{	"Netherlands NOS future use 3109 48 09 38 09
{	"Netherlands NOS future use 310A 48 0A 38 0A
{	"Netherlands NOS future use 310B 48 0B 38 0B
{	"Netherlands NOS future use 310C 48 0C 38 0C
{	"Netherlands NOS future use 310D 48 0D 38 0D
{	"Netherlands NOS future use 310E 48 0E 38 0E
{	"Netherlands NOS future use 310F 48 0F 38 0F
{	"Netherlands The BOX 3120 48 20 38 20
{	"Netherlands Discovery Netherlands 3121
{	"Netherlands Nickelodeon 3122 48 22 38 22
{	"Netherlands Animal Planet Benelux 3123
{	"Netherlands TALPA TV 3124
{	"Netherlands NET5 3125
{	"Netherlands SBS6 3126
{	"Netherlands SBS future use 3127
{	"Netherlands V8 3128
{	"Netherlands SBS future use 3129
{	"Netherlands SBS future use 312A
{	"Netherlands SBS future use 312B
{	"Netherlands SBS future use 312C
{	"Netherlands SBS future use 312D
{	"Netherlands SBS future use 312E
{	"Netherlands SBS future use 312F
{	"Netherlands TMF (Netherlands service) 3130
{	"Netherlands TMF (Belgian Flanders service) 3131
{	"Netherlands MTV NL 3132
{	"Netherlands RNN7 3137
{	"Norway NRK1 4701
{	"Norway NRK2 4703
{	"Norway TV 2 4702
{	"Norway TV Norge 4704
{	"Norway Discovery Nordic 4720
{	"Poland Animal Planet 4831
{	"Poland Discovery Poland 4830
{	"Poland TV Polonia 4810
{	"Poland TVP1 4801
{	"Poland TVP2 4802
{	"Poland TVP Warszawa 4880
{	"Poland TVP Bialystok 4881
{	"Poland TVP Bydgoszcz 4882
{	"Poland TVP Gdansk 4883
{	"Poland TVP Katowice 4884
{	"Poland TVP Krakow 4886
{	"Poland TVP Lublin 4887
{	"Poland TVP Lodz 4888
{	"Poland TVP Rzeszow 4890
{	"Poland TVP Poznan 4891
{	"Poland TVP Szczecin 4892
{	"Poland TVP Wroclaw 4893
{	"Poland TVN 4820
{	"Poland TVN Siedem 4821
{	"Poland TVN24 4822
{	"Portugal Future use 3516
{	"Portugal Future use 3517
{	"Portugal Future use 3518
{	"Portugal Future use 3519
{	"Portugal RTP1 3510
{	"Portugal RTP2 3511
{	"Portugal RTPAF 3512
{	"Portugal RTPAZ 3514
{	"Portugal RTPI 3513
{	"Portugal RTPM 3515
{	"San Marino RTV 3781
{	"Slovakia future use 42A7 35 A7 35 27
{	"Slovakia future use 42A8 35 A8 35 28
{	"Slovakia future use 42A9 35 A9 35 29
{	"Slovakia future use 42AA 35 AA 35 2A
{	"Slovakia future use 42AB 35 AB 35 2B
{	"Slovakia future use 42AC 35 AC 35 2C
{	"Slovakia future use 42AD 35 AD 35 2D
{	"Slovakia future use 42AE 35 AE 35 2E
{	"Slovakia future use 42AF 35 AF 35 2F
{	"Slovakia STV1 42A1 35 A1 35 21
{	"Slovakia STV1 Regional, B. Bystrica 42A5 35 A5 35 25
{	"Slovakia STV1 Regional, Košice 42A3 35 A3 35 23
{	"Slovakia STV2 42A2 35 A2 35 22
{	"Slovakia STV2 Regional, B. Bystrica 42A6 35 A6 35 26
{	"Slovakia STV2 Regional, Košice 42A4 35 A4 35 24
{	"Slovenia future use AAE5
{	"Slovenia future use AAE6
{	"Slovenia future use AAE7
{	"Slovenia future use AAE8
{	"Slovenia future use AAE9
{	"Slovenia future use AAEA
{	"Slovenia future use AAEB
{	"Slovenia future use AAEC
{	"Slovenia future use AAED
{	"Slovenia future use AAEE
{	"Slovenia future use AAEF
{	"Slovenia future use AAF2
{	"Slovenia future use AAF3
{	"Slovenia future use AAF4
{	"Slovenia future use AAF5
{	"Slovenia future use AAF6
{	"Slovenia future use AAF7
{	"Slovenia future use AAF8
{	"Slovenia future use AAF9
{	"Slovenia KC AAE3
{	"Slovenia SLO1 AAE1
{	"Slovenia SLO2 AAE2
{	"Slovenia SLO3 AAF1
{	"Slovenia TLM AAE4
{	"Spain Arte 340A
{	"Spain C33 CA33
{	"Spain ETB 1 BA01
{	"Spain ETB 2 3402
{	"Spain TV3 CA03
{	"Spain TVE1 3E00
{	"Spain TVE2 E100
{	"Spain TVE Internacional Europa E200
{	"Spain CANAL 9 3403
{	"Spain PUNT 2 3404
{	"Spain CCV 3405
{	"Spain CANAL 9 NEWS 24H Future use 3406
{	"Spain CANAL 9 Future Use 3407
{	"Spain CANAL 9 DVB Future Use 3408
{	"Spain CANAL 9 DVB Future Use 3409
{	"Spain CANAL 9 DVB Future Use 340B
{	"Spain CANAL 9 DVB Future Use 340C
{	"Spain CANAL 9 DVB Future Use 340D
{	"Spain CANAL 9 DVB Future Use 340E
{	"Spain CANAL 9 DVB Future Use 340F
{	"Spain CANAL 9 DVB Future Use 3410
{	"Spain CANAL 9 DVB Future Use 3411
{	"Spain CANAL 9 DVB Future Use 3412
{	"Spain CANAL 9 DVB Future Use 3413
{	"Spain CANAL 9 DVB Future Use 3414
{	"Spain Tele5 E500 1F E5
{	"Sweden SVT 1 4601 4E 01 3E 01
{	"Sweden SVT 2 4602 4E 02 3E 02
{	"Sweden SVT future use 4603 4E 03 3E 03
{	"Sweden SVT future use 4604 4E 04 3E 04
{	"Sweden SVT future use 4605 4E 05 3E 05
{	"Sweden SVT future use 4606 4E 06 3E 06
{	"Sweden SVT future use 4607 4E 07 3E 07
{	"Sweden SVT future use 4608 4E 08 3E 08
{	"Sweden SVT future use 4609 4E 09 3E 09
{	"Sweden SVT future use 460A 4E 0A 3E 0A
{	"Sweden SVT future use 460B 4E 0B 3E 0B
{	"Sweden SVT future use 460C 4E 0C 3E 0C
{	"Sweden SVT future use 460D 4E 0D 3E 0D
{	"Sweden SVT future use 460E 4E 0E 3E 0E
{	"Sweden SVT future use 460F 4E 0F 3E 0F
{	"Sweden SVT Test Txmns 4600 4E 00 3E 00
{	"Sweden TV 4 4640 4E 40 3E 40
{	"Sweden TV 4 future use 4641 4E 41 3E 41
{	"Sweden TV 4 future use 4642 4E 42 3E 42
{	"Sweden TV 4 future use 4643 4E 43 3E 43
{	"Sweden TV 4 future use 4644 4E 44 3E 44
{	"Sweden TV 4 future use 4645 4E 45 3E 45
{	"Sweden TV 4 future use 4646 4E 46 3E 46
{	"Sweden TV 4 future use 4647 4E 47 3E 47
{	"Sweden TV 4 future use 4648 4E 48 3E 48
{	"Sweden TV 4 future use 4649 4E 49 3E 49
{	"Sweden TV 4 future use 464A 4E 4A 3E 4A
{	"Sweden TV 4 future use 464B 4E 4B 3E 4B
{	"Sweden TV 4 future use 464C 4E 4C 3E 4C
{	"Sweden TV 4 future use 464D 4E 4D 3E 4D
{	"Sweden TV 4 future use 464E 4E 4E 3E 4E
{	"Sweden TV 4 future use 464F 4E 4F 3E 4F
*/
{	"Switzerland",	"SAT ACCESS",				0x410A,	0x24,	0xCA,	0x34,	0x4A	},
{	"Switzerland",	"SF 1",						0x4101,	0x24,	0xC1,	0x34,	0x41	},
{	"Switzerland",	"SF 2",						0x4107,	0x24,	0xC7,	0x34,	0x47	},
{	"Switzerland",	"TSI 1",					0x4103,	0x24,	0xC3,	0x34,	0x43	},
{	"Switzerland",	"TSI 2",					0x4109,	0x24,	0xC9,	0x34,	0x49	},
{	"Switzerland",	"TSR 1",					0x4102,	0x24,	0xC2,	0x34,	0x42	},
{	"Switzerland",	"TSR 2",					0x4108,	0x24,	0xC8,	0x34,	0x48	},
{	"Switzerland",	"U1",						0x4121,	0x24,	0x21,	0,	0	},
/*
{	"Turkey ATV 900A
{	"Turkey AVRASYA 9006 43 06 33 06
{	"Turkey BRAVO TV 900E
{	"Turkey Cine 5 9008
{	"Turkey EKO TV 900D
{	"Turkey EURO D 900C
{	"Turkey FUN TV 9010
{	"Turkey GALAKSI TV 900F
{	"Turkey KANAL D 900B
{	"Turkey KANAL D future use 9012
{	"Turkey KANAL D future use 9013
{	"Turkey Show TV 9007
{	"Turkey STAR TV 9020
{	"Turkey STARMAX 9021
{	"Turkey KANAL 6 9022
{	"Turkey STAR 4 9023
{	"Turkey STAR 5 9024
{	"Turkey STAR 6 9025
{	"Turkey STAR 7 9026
{	"Turkey STAR 8 9027
{	"Turkey STAR TV future use 9028
{	"Turkey STAR TV future use 9029
{	"Turkey STAR TV future use 9030
{	"Turkey STAR TV future use 9031
{	"Turkey STAR TV future use 9032
{	"Turkey STAR TV future use 9033
{	"Turkey STAR TV future use 9034
{	"Turkey STAR TV future use 9035
{	"Turkey STAR TV future use 9036
{	"Turkey STAR TV future use 9037
{	"Turkey STAR TV future use 9038
{	"Turkey STAR TV future use 9039
{	"Turkey Super Sport 9009
{	"Turkey TEMPO TV 9011
{	"Turkey TGRT 9014
{	"Turkey TRT-1 9001 43 01 33 01
{	"Turkey TRT-2 9002 43 02 33 02
{	"Turkey TRT-3 9003 43 03 33 03
{	"Turkey TRT-4 9004 43 04 33 04
{	"Turkey TRT-INT 9005 43 05 33 05
{	"UK ANGLIA TV FB9C 2C 1C 3C 1C
{	"UK ANGLIA TV future use FB9F 2C 1F 3C 1F
{	"UK ANGLIA TV future use FB9D 5B CD 3B 4D
{	"UK ANGLIA TV future use FB9E 5B CE 3B 4E
{	"UK BBC News 24 4469 2C 69 3C 69
{	"UK BBC Prime 4468 2C 68 3C 68
{	"UK BBC World 4457 2C 57 3C 57
{	"UK BBC Worldwide future 01 4458 2C 58 3C 58
{	"UK BBC Worldwide future 02 4459 2C 59 3C 59
{	"UK BBC Worldwide future 03 445A 2C 5A 3C 5A
{	"UK BBC Worldwide future 04 445B 2C 5B 3C 5B
{	"UK BBC Worldwide future 05 445C 2C 5C 3C 5C
{	"UK BBC Worldwide future 06 445D 2C 5D 3C 5D
{	"UK BBC Worldwide future 07 445E 2C 5E 3C 5E
{	"UK BBC Worldwide future 08 445F 2C 5F 3C 5F
{	"UK BBC Worldwide future 09 4460 2C 60 3C 60
{	"UK BBC Worldwide future 10 4461 2C 61 3C 61
{	"UK BBC Worldwide future 11 4462 2C 62 3C 62
{	"UK BBC Worldwide future 12 4463 2C 63 3C 63
{	"UK BBC Worldwide future 13 4464 2C 64 3C 64
{	"UK BBC Worldwide future 14 4465 2C 65 3C 65
{	"UK BBC Worldwide future 15 4466 2C 66 3C 66
{	"UK BBC Worldwide future 16 4467 2C 67 3C 67
{	"UK BBC1 447F 2C 7F 3C 7F
{	"UK BBC1 future 01 4443 2C 43 3C 43
{	"UK BBC1 future 02 4445 2C 45 3C 45
{	"UK BBC1 future 03 4479 2C 79 3C 79
{	"UK BBC1 future 04 4447 2C 47 3C 47
{	"UK BBC1 future 05 4477 2C 77 3C 77
{	"UK BBC1 future 06 4449 2C 49 3C 49
{	"UK BBC1 future 07 4475 2C 75 3C 75
{	"UK BBC1 future 08 444B 2C 4B 3C 4B
{	"UK BBC1 future 09 4473 2C 73 3C 73
{	"UK BBC1 future 10 444D 2C 4D 3C 4D
{	"UK BBC1 future 11 4471 2C 71 3C 71
{	"UK BBC1 future 12 444F 2C 4F 3C 4F
{	"UK BBC1 future 13 446F 2C 6F 3C 6F
{	"UK BBC1 future 14 4451 2C 51 3C 51
{	"UK BBC1 future 15 446D 2C 6D 3C 6D
{	"UK BBC1 future 16 4453 2C 53 3C 53
{	"UK BBC1 future 17 446B 2C 6B 3C 6B
{	"UK BBC1 future 18 4455 2C 55 3C 55
{	"UK BBC1 NI 4441 2C 41 3C 41
{	"UK BBC1 Scotland 447B 2C 7B 3C 7B
{	"UK BBC1 Wales 447D 2C 7D 3C 7D
{	"UK BBC2 4440 2C 40 3C 40
{	"UK BBC2 future 01 447C 2C 7C 3C 7C
{	"UK BBC2 future 02 447A 2C 7A 3C 7A
{	"UK BBC2 future 03 4446 2C 46 3C 46
{	"UK BBC2 future 04 4478 2C 78 3C 78
{	"UK BBC2 future 05 4448 2C 48 3C 48
{	"UK BBC2 future 06 4476 2C 76 3C 76
{	"UK BBC2 future 07 444A 2C 4A 3C 4A
{	"UK BBC2 future 08 4474 2C 74 3C 74
{	"UK BBC2 future 09 444C 2C 4C 3C 4C
{	"UK BBC2 future 10 4472 2C 72 3C 72
{	"UK BBC2 future 11 444E 2C 4E 3C 4E
{	"UK BBC2 future 12 4470 2C 70 3C 70
{	"UK BBC2 future 13 4450 2C 50 3C 50
{	"UK BBC2 future 14 446E 2C 6E 3C 6E
{	"UK BBC2 future 15 4452 2C 52 3C 52
{	"UK BBC2 future 16 446C 2C 6C 3C 6C
{	"UK BBC2 future 17 4454 2C 54 3C 54
{	"UK BBC2 future 18 446A 2C 6A 3C 6A
{	"UK BBC2 future 19 4456 2C 56 3C 56
{	"UK BBC2 NI 447E 2C 7E 3C 7E
{	"UK BBC2 Scotland 4444 2C 44 3C 44
{	"UK BBC2 Wales 4442 2C 42 3C 42
{	"UK BORDER TV B7F7 2C 27 3C 27
{	"UK BRAVO 4405 5B EF 3B 6F
{	"UK CARLTON SEL. future use 82E2 2C 06 3C 06
{	"UK CARLTON SELECT 82E1 2C 05 3C 05
{	"UK CARLTON TV 82DD 2C 1D 3C 1D
{	"UK CARLTON TV future use 82DE 5B CF 3B 4F
{	"UK CARLTON TV future use 82DF 5B D0 3B 50
{	"UK CARLTON TV future use 82E0 5B D1 3B 51
{	"UK CENTRAL TV 2F27 2C 37 3C 37
{	"UK CENTRAL TV future use 5699 2C 16 3C 16
{	"UK CHANNEL 4 FCD1 2C 11 3C 11
{	"UK CHANNEL 5 (1) 9602 2C 02 3C 02
{	"UK CHANNEL 5 (2) 1609 2C 09 3C 09
{	"UK CHANNEL 5 (3) 28EB 2C 2B 3C 2B
{	"UK CHANNEL 5 (4) C47B 2C 3B 3C 3B
{	"UK CHANNEL TV FCE4 2C 24 3C 24
{	"UK CHILDREN'S CHANNEL 4404 5B F0 3B 70
{	"UK CNNI 01F2 5B F1 3B 71
{	"UK DISCOVERY 4407 5B F2 3B 72
{	"UK Discovery Home & Leisure 4420
{	"UK Animal Planet 4421
{	"UK DISNEY CHANNEL UK 44D1 5B CC 3B 4C
{	"UK FAMILY CHANNEL 4408 5B F3 3B 73
{	"UK FilmFour C4F4 42 F4 32 74
{	"UK GMTV ADDC 5B D2 3B 52
{	"UK GMTV future use ADDD 5B D3 3B 53
{	"UK GMTV future use ADDE 5B D4 3B 54
{	"UK GMTV future use ADDF 5B D5 3B 55
{	"UK GMTV future use ADE0 5B D6 3B 56
{	"UK GMTV future use ADE1 5B D7 3B 57
{	"UK GRAMPIAN TV F33A 2C 3A 3C 3A
{	"UK GRANADA PLUS 4D5A 5B F4 3B 74
{	"UK GRANADA Timeshare 4D5B 5B F5 3B 75
{	"UK GRANADA TV ADD8 2C 18 3C 18
{	"UK GRANADA TV future use ADD9 5B D8 3B 58
{	"UK HISTORY Ch. FCF4 5B F6 3B 76
{	"UK HTV 5AAF 2C 3F 3C 3F
{	"UK HTV future use F258 2C 38 3C 38
{	"UK ITV NETWORK C8DE 2C 1E 3C 1E
{	"UK LEARNING CHANNEL 4406 5B F7 3B 77
{	"UK Live TV 4409 5B F8 3B 78
{	"UK LWT 884B 2C 0B 3C 0B
{	"UK LWT future use 884C 5B D9 3B 59
{	"UK LWT future use 884D 5B DA 3B 5A
{	"UK LWT future use 884F 5B DB 3B 5B
{	"UK LWT future use 8850 5B DC 3B 5C
{	"UK LWT future use 8851 5B DD 3B 5D
{	"UK LWT future use 8852 5B DE 3B 5E
{	"UK LWT future use 8853 5B DF 3B 5F
{	"UK LWT future use 8854 5B E0 3B 60
{	"UK MERIDIAN 10E4 2C 34 3C 34
{	"UK MERIDIAN future use DD50 2C 10 3C 10
{	"UK MERIDIAN future use DD51 5B E1 3B 61
{	"UK MERIDIAN future use DD52 5B E2 3B 62
{	"UK MERIDIAN future use DD53 5B E3 3B 63
{	"UK MERIDIAN future use DD54 5B E4 3B 64
{	"UK MERIDIAN future use DD55 5B E5 3B 65
{	"UK MOVIE CHANNEL FCFB 2C 1B 3C 1B
{	"UK MTV 4D54 2C 14 3C 14
{	"UK MTV future use 4D55 2C 33 3C 33
{	"UK MTV future use 4D56 2C 36 3C 36
{	"UK National Geographic Channel 320B
{	"UK NBC Europe 8E71 2C 31 3C 31
{	"UK NBC Europe future use 5343 2C 03 3C 03
{	"UK NBC Europe future use 8E79 2C 23 3C 23
{	"UK NBC Europe future use 8E78 2C 26 3C 26
{	"UK NBC Europe future use 8E77 2C 28 3C 28
{	"UK NBC Europe future use 8E76 2C 29 3C 29
{	"UK NBC Europe future use 8E75 2C 2A 3C 2A
{	"UK NBC Europe future use 8E74 2C 2E 3C 2E
{	"UK NBC Europe future use 8E73 2C 32 3C 32
{	"UK CNBC Europe 8E72 2C 35 3C 35
{	"UK Nickelodeon UK A460
{	"UK Paramount Comedy Channel UK A465
{	"UK QVC future use 5C33
{	"UK QVC future use 5C34
{	"UK QVC future use 5C39
{	"UK QVC UK 5C44
{	"UK RACING Ch. FCF3 2C 13 3C 13
{	"UK S4C B4C7 2C 07 3C 07
{	"UK SCI FI CHANNEL FCF5 2C 15 3C 15
{	"UK SCOTTISH TV F9D2 2C 12 3C 12
{	"UK SKY GOLD FCF9 2C 19 3C 19
{	"UK SKY MOVIES PLUS FCFC 2C 0C 3C 0C
{	"UK SKY NEWS FCFD 2C 0D 3C 0D
{	"UK SKY ONE FCFE 2C 0E 3C 0E
{	"UK SKY SOAPS FCF7 2C 17 3C 17
{	"UK SKY SPORTS FCFA 2C 1A 3C 1A
{	"UK SKY SPORTS 2 FCF8 2C 08 3C 08
{	"UK SKY TRAVEL FCF6 5B F9 3B 79
{	"UK SKY TWO FCFF 2C 0F 3C 0F
{	"UK SSVC 37E5 2C 25 3C 25
{	"UK TNT / Cartoon Network 44C1
{	"UK TYNE TEES TV A82C 2C 2C 3C 2C
{	"UK TYNE TEES TV future use A82D 5B E6 3B 66
{	"UK TYNE TEES TV future use A82E 5B E7 3B 67
{	"UK UK GOLD 4401 5B FA 3B 7A
{	"UK UK GOLD future use 4411 5B FB 3B 7B
{	"UK UK GOLD future use 4412 5B FC 3B 7C
{	"UK UK GOLD future use 4413 5B FD 3B 7D
{	"UK UK GOLD future use 4414 5B FE 3B 7E
{	"UK UK GOLD future use 4415 5B FF 3B 7F
{	"UK UK LIVING 4402 2C 01 3C 01
{	"UK ULSTER TV 833B 2C 3D 3C 3D
{	"UK VH-1 4D58 2C 20 3C 20
{	"UK VH-1 (German language) 4D59 2C 21 3C 21
{	"UK VH-1 future use 4D57 2C 22 3C 22
{	"UK WESTCOUNTRY future use 25D1 5B E8 3B 68
{	"UK WESTCOUNTRY future use 25D2 5B E9 3B 69
{	"UK WESTCOUNTRY TV 25D0 2C 30 3C 30
{	"UK WIRE TV 4403 2C 3C 3C 3C
{	"UK YORKSHIRE TV FA2C 2C 2D 3C 2D
{	"UK YORKSHIRE TV future use FA2D 5B EA 3B 6A
{	"UK YORKSHIRE TV future use FA2E 5B EB 3B 6B
{	"UK YORKSHIRE TV future use FA2F 5B EC 3B 6C
{	"UK YORKSHIRE TV future use FA30 5B ED 3B 6D
{	"UK YORKSHIRE TV future use FA31 5B EE 3B 6E
*/
{	"Ukraine",		"1+1",						0x7700,	0,	0,	0,	0	},
{	"Ukraine",		"1+1 future use",			0x7701,	0,	0,	0,	0	},
{	"Ukraine",		"1+1 future use",			0x7702,	0,	0,	0,	0	},
{	"Ukraine",		"1+1 future use",			0x7703,	0,	0,	0,	0	},
{	"Ukraine",		"M1",						0x7705,	0,	0,	0,	0	},
{	"Ukraine",		"ICTV",						0x7707,	0,	0,	0,	0	},
{	"Ukraine",		"Novy Kanal",				0x7708,	0,	0,	0,	0	},
};


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
                    for (int j = i + 2; j <= 23; j += 2)
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
        for (int j = 1; j < nIndex; j++)
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


void VT_GetStationFromID(LPSTR lpBuffer, LONG nLength)
{
    ASSERT(nLength > 0);

    lpBuffer[0] = '\0';

	// Search first with CNI from PDC (P8/30/2)
	DWORD dwCode = VTDecoder.GetCNIFromPDC();
	if (dwCode != 0)
	{
		// dwCode is a CNI from PDC
		BYTE uCountry = (dwCode >> 8) & 0xFF;
		BYTE uNetwork = dwCode & 0xFF;
		//LOG(1, "PDC Country %x Network %x", uCountry, uNetwork);
		int iNbCodes = sizeof(RegisteredCNICodes) / sizeof(RegisteredCNICodes[0]);
		for (int i(0); i < iNbCodes; i++)
		{
			if (   (RegisteredCNICodes[i].uC_P8302 == uCountry)
				&& (RegisteredCNICodes[i].uNI_P8302 == uNetwork) )
			{
				strncpy(lpBuffer, RegisteredCNICodes[i].sNetwork, nLength-1);
				lpBuffer[nLength] = '\0';
				break;
			}
		}
	}

	// If not found, then search with network ID ocde from P8/30/1
    if (*lpBuffer == '\0')
	{
		dwCode = VTDecoder.GetNetworkIDFromP8301();
		if (dwCode != 0)
		{
			// dwCode is a network ID code
			//LOG(1, "P8/30/1 Network ID Code %x", dwCode);
			int iNbCodes = sizeof(RegisteredCNICodes) / sizeof(RegisteredCNICodes[0]);
			for (int i(0); i < iNbCodes; i++)
			{
				if (RegisteredCNICodes[i].dwNI_P8301 == dwCode)
				{
					strncpy(lpBuffer, RegisteredCNICodes[i].sNetwork, nLength-1);
					lpBuffer[nLength] = '\0';
					break;
				}
			}
		}
	}
}


void VT_GetStation(LPSTR lpBuffer, LONG nLength)
{
    VTDecoder.GetStatusDisplay(lpBuffer, nLength);
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


typedef HRESULT (__stdcall *PFNSETWINDOWTHEME)(HWND, LPCWSTR, LPCWSTR);
typedef BOOL (__stdcall *PFNISAPPTHEMED)();

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
            static HMODULE hThemeDll = LoadLibrary(_T("UxTheme.dll"));

            if (hThemeDll != NULL)
            {
                PFNISAPPTHEMED pfnIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(hThemeDll, "IsAppThemed");
                PFNSETWINDOWTHEME pfnSetWindowThemed = (PFNSETWINDOWTHEME)GetProcAddress(hThemeDll, "SetWindowTheme");

                if (pfnIsAppThemed != NULL && pfnSetWindowThemed != NULL)
                {
                    if ((pfnIsAppThemed)())
                    {
                        (pfnSetWindowThemed)(hItem, L" ", L" ");
                    }
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


void VT_ReadSettingsFromIni()
{
    for (int i = 0; i < VT_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&VTSettings[i]);
    }

    if (!VT_GetAutoCodepage())
    {
        VT_SetCodepage(NULL, NULL, VTUserCodepage);
    }

    VTDecoder.SetCachingControl(VTCachingControl);
    VTDecoder.SetHighGranularityCaching(VTHighGranularityCaching);
    VTDecoder.SetSubstituteSpacesForError(VTSubstituteErrorsWithSpaces);
}


void VT_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    for (int i = 0; i < VT_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&VTSettings[i], bOptimizeFileAccess);
    }
}


CTreeSettingsGeneric* VideoText_GetTreeSettingsPage()
{
    // Teletext Settings
    SETTING* VideoTextSettings[] =
    {
        &VTSettings[VT_LANGUAGE_REGION              ],
        &VTSettings[VT_SHOW_SUBCODE_OSD             ],
        &VTSettings[VT_LINES_CACHINGCONTROL         ],
        &VTSettings[VT_HIGH_GRANULARITY_CACHING     ],
        &VTSettings[VT_SUBSTITUTE_ERROR_SPACES      ],
        &VTSettings[VT_SUBTITLE_DUPLICATION_FILTER  ],
        &VTSettings[VT_DOUBLEHEIGHT_SUBTITLES_FILTER],
    };

    WORD nCount = sizeof(VideoTextSettings)/sizeof(SETTING*);
    return new CTreeSettingsGeneric("Teletext Settings", VideoTextSettings, nCount);
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
    VTShowSubcodeInOSD = NewValue;
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

