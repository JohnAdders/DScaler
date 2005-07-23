/////////////////////////////////////////////////////////////////////////////
// $Id: OSD.cpp,v 1.112 2005-07-23 19:13:27 laurentg Exp $
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 8 Nov 2000    Michael Eskin         Initial onscreen display
//
// 28 Nov 2000   Mark Rejhon           Reorganization and visual improvements
//
// 23 Feb 2001   Michael Samblanet     Calculate OSD rect so we do not 
//                                     invalidate entire display to erase
//
// 24 Feb 2001   Michael Samblanet     Moved rect into OSDInfo structure
//                                     Should improve compatability with coming
//                                     OSD changes
//
// 25 Feb 2001   Laurent Garnier       Added management of multiple OSD texts
//
// 03 Mar 2001   Laurent Garnier       Added functions OSD_ShowInfosScreen
//                                     and OSD_GetLineYpos
//
// 10 Mar 2001   Laurent Garnier       Status bar height taken into account when
//                                     calculating texts placement
//
// 18 Mar 2001   Laurent Garnier       Added multiple screens feature
//                                     Added specific screen for WSS data decoding
//
// 22 Mar 2001   Laurent Garnier       Screen Refresh managed with a timer
//                                     Choice (in ini file) between persistent
//                                     screens and screens with autohide timer
//
// NOTICE FROM MARK: This code will probably be rewritten, but keeping 
// this code neat and architecturally well organized, will maximize code 
// recyclability.   There is a need for multiple independent OSD elements,
// such as persistent "MUTE" / "UNMUTE" with separate channel number that
// dissappears.   Perhaps some kind of a linked list of OSD's can be 
// maintained.   Keep a future multi-OSD architecture in mind when deciding
// to expand this code.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.111  2005/07/23 18:56:08  laurentg
// EPG with external tuner using teletext/VPS to get channel name
//
// Revision 1.110  2005/07/23 12:40:57  laurentg
// EPG: switch between browser view and details view improved
//
// Revision 1.109  2005/07/19 21:41:54  laurentg
// EPG: shift programme description at screen using Shift+PgUp and Shift+PgDn
//
// Revision 1.108  2005/07/11 22:11:29  laurentg
// Minor change for display of EPG in OSD
//
// Revision 1.107  2005/07/09 13:43:43  laurentg
// Two new EPG settings + Possibility to display next and previous programmes info
//
// Revision 1.106  2005/07/06 21:44:41  laurentg
// Cut on several lines for the display of the programme description ( EPG OSD)
//
// Revision 1.105  2005/07/06 19:48:33  laurentg
// Updated OSD for EPG
//
// Revision 1.104  2005/06/09 23:27:41  robmuller
// Enable Developer Statistics by default (this screen is never shown in a release build).
//
// Revision 1.103  2005/04/02 14:23:44  laurentg
// EPG: little bugs fixed
//
// Revision 1.102  2005/04/02 14:04:12  laurentg
// EPG: navigation between the browser view and the programme view improved
//
// Revision 1.101  2005/04/01 22:16:33  laurentg
// EPG: new menu "Hide EPG" + new setting to define the time frame duration
//
// Revision 1.100  2005/03/29 21:08:34  laurentg
// program renamed programme
//
// Revision 1.99  2005/03/28 17:48:10  laurentg
// Navigation into EPG + change of channel
//
// Revision 1.98  2005/03/28 13:42:02  laurentg
// EPG: preparation for when new data (category, sub-title, description) will be available
//
// Revision 1.97  2005/03/28 12:53:20  laurentg
// EPG: previous and next page to show programs
//
// Revision 1.96  2005/03/27 20:22:20  laurentg
// EPG: new improvements
//
// Revision 1.95  2005/03/26 22:07:28  laurentg
// EPG screens defined as not accessible in the View menu
//
// Revision 1.94  2005/03/26 18:53:23  laurentg
// EPG code improved
// => possibility to set the EPG channel name in the channel setup dialog box
// => automatic loading of new data when needed
// => OSD scrrens updated
// => first step for programs "browser"
//
// Revision 1.93  2005/03/23 14:20:57  adcockj
// Test fix for threading issues
//
// Revision 1.92  2005/03/21 22:39:15  laurentg
// EPG: changes regarding OSD
//
// Revision 1.91  2005/03/20 22:56:22  laurentg
// New OSD screens added for EPG
//
// Revision 1.90  2004/07/25 11:25:31  adcockj
// Patch for international lanugauges in OSD from Alexander Choporov
//
// Revision 1.89  2004/05/02 14:09:32  atnak
// Fixed possible problem of overlay colour getting dithered with < 32bit colour
//
// Revision 1.88  2003/11/12 22:14:30  robmuller
// Add some more info to the developers statistics screen.
//
// Revision 1.87  2003/11/11 22:16:30  robmuller
// Add ability to include the performance statistics in a release build.
//
// Revision 1.86  2003/10/27 10:39:52  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.85  2003/06/02 13:15:34  adcockj
// Fixes for CHARSTRING problems
//
// Revision 1.84  2003/05/26 22:04:17  laurentg
// Update of the OSD displayed when doing calibration
//
// Revision 1.83  2003/04/26 19:39:10  laurentg
// New character string settings
//
// Revision 1.82  2003/03/09 19:46:26  laurentg
// Updated field statistics
//
// Revision 1.81  2003/02/22 13:37:49  laurentg
// New statistics to check fields runnign late and no flip at time
//
// Revision 1.80  2003/01/24 02:20:37  atnak
// Put back performance counter for OSD screen redraw.
//
// Revision 1.79  2003/01/24 01:55:18  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.78  2003/01/19 11:09:10  laurentg
// New methods GetInitialWidth and GetInitialHeight to store the initial size before resizing in DScaler (for stills)
//
// Revision 1.77  2003/01/18 12:26:27  laurentg
// Menu items for OSD screens allow now to select/display the screen instead of enable/disable it
// The three OSD screens are now enabled by default
//
// Revision 1.76  2003/01/08 22:01:24  robmuller
// Fixed problem with multi-line OSD messages at default size.
//
// Revision 1.75  2002/11/10 09:42:01  laurentg
// Settings "Use OSD screen..."
//
// Revision 1.74  2002/10/29 23:38:35  laurentg
// Display of the calibration OSD
//
// Revision 1.73  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// Revision 1.72  2002/10/27 20:39:07  laurentg
// Performance statistics only computed in DEBUG buildd
// Developer OSD screen only present in DEBUG build
//
// Revision 1.71  2002/10/27 13:08:17  robmuller
// In addition to "\n" OSD now supports "\r\n" to start a new line.
//
// Revision 1.70  2002/09/18 11:38:05  kooiman
// Preparations for skinned dscaler look.
//
// Revision 1.69  2002/09/11 18:19:42  adcockj
// Prelimainary support for CX2388x based cards
//
// Revision 1.68  2002/08/02 20:16:43  laurentg
// Suppress call to RemoveMenu
//
// Revision 1.67  2002/07/30 21:20:59  laurentg
// Merge of menus View, AspectRatio and OSD
//
// Revision 1.66  2002/07/20 10:33:06  laurentg
// New settings to select the wished OSD screens
//
// Revision 1.65  2002/07/19 13:02:32  laurentg
// OSD menu simplified (one depth level less)
//
// Revision 1.64  2002/07/19 11:59:12  laurentg
// OSD settings added in the tree settings
//
// Revision 1.63  2002/06/13 12:10:22  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.62  2002/06/02 09:43:23  laurentg
// Settings restore at end of automatic calibration was broken
//
// Revision 1.61  2002/06/01 22:24:36  laurentg
// New calibration mode to compute YUV range
//
// Revision 1.60  2002/05/28 08:23:05  robmuller
// Fixed: OSD did never output non-antialiased.
// Changed: OSD font now defaults to non-antialiased to fix the OSD performance problems.
//
// Revision 1.59  2002/05/20 18:08:29  robmuller
// Fixed: screen only partly updated when in video text mode.
//
// Revision 1.58  2002/05/06 15:34:59  laurentg
// Key <i> to show source informations through OSD
//
// Revision 1.57  2002/04/27 14:08:07  laurentg
// Automatic square pixels mode handling updated
//
// Revision 1.56  2002/04/06 11:46:45  laurentg
// Check that the current source is not NULL to avoid DScaler exits
//
// Revision 1.55  2002/02/23 15:43:08  laurentg
// General screen (OSD) updated
//
// Revision 1.54  2002/02/23 13:56:12  laurentg
// Big switch in OSD_RefreshInfosScreen replaced by a call to one function for each screen
// AR statistics moved in the the statistics screen and AR screen disabled
// New screen added with our statistics regarding the time spent in each part of the DScaler code
//
// Revision 1.53  2002/02/23 12:02:40  laurentg
// % of time used by each AR added in the AR statistics
//
// Revision 1.52  2002/02/18 20:51:51  laurentg
// Statistics regarding deinterlace modes now takes into account the progressive mode
// Reset of the deinterlace statistics at each start of the decoding thread
// Reset action now resets the deinterlace statistics too
//
// Revision 1.51  2002/02/17 22:34:19  laurentg
// Bug in statistics OSD screen corrected
//
// Revision 1.50  2002/02/17 20:32:34  laurentg
// Audio input display suppressed from the OSD main screen
// GetStatus modified to display the video input name in OSD main screen even when there is no signal
//
// Revision 1.49  2002/02/09 14:46:04  laurentg
// OSD main screen updated to display the correct input name (or channel)
// OSD main screen updated to display only activated filters
// Menu label for the BT848 providers now displays the name of the card
//
// Revision 1.48  2002/02/09 13:01:57  laurentg
// Function OSD_ShowUI added
//
// Revision 1.47  2002/02/08 00:42:25  laurentg
// Support of a new type of file : DScaler patterns
//
// Revision 1.46  2002/02/02 12:44:01  laurentg
// "Pixel width" replaced by "Source size" in the main OSD
//
// Revision 1.45  2002/01/19 19:09:37  robmuller
// Added support for the '\n' character in OSD_AddText().
//
// Revision 1.44  2001/12/16 16:31:43  adcockj
// Bug fixes
//
// Revision 1.43  2001/12/16 13:13:34  laurentg
// New statistics
//
// Revision 1.42  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.41  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.40  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.39  2001/11/22 22:27:00  adcockj
// Bug Fixes
//
// Revision 1.38  2001/11/22 13:19:37  temperton
// Added CPaintingHDC class for double buffering painting
//
// Revision 1.37  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.36  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.35  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.34  2001/09/29 16:33:50  laurentg
// Background color and background mode can now be deifned sepaately for each text in OSD
// OSD for calibration updated
//
// Revision 1.33  2001/09/29 10:40:58  laurentg
// OSD for calibration updated
//
// Revision 1.32  2001/09/22 18:04:55  laurentg
// Minimum time to wait after each ajustment reduced
// Fine adjustments made with more frames (50)
// Brigthness now ajusted using several black bars
// Contrast now ajusted using several white bars
// Hue now ajusted using magenta, cyan and green bars
// Fine ajustment only tests combinations of correct settings found in previous steps
//
// Revision 1.31  2001/09/21 20:39:12  laurentg
// Text "auto" added in front of the source ratio in the OSD general screen when ratio automatic detection is activated
//
// Revision 1.30  2001/09/15 21:02:45  laurentg
// no message
//
// Revision 1.29  2001/09/09 17:46:30  laurentg
// no message
//
// Revision 1.28  2001/09/04 21:03:09  laurentg
// no message
//
// Revision 1.27  2001/09/02 22:07:42  laurentg
// OSD screen for automatic calibration modified
// Hue taken into account during automatic calibration
//
// Revision 1.26  2001/08/26 18:33:42  laurentg
// Automatic calibration improved
//
// Revision 1.25  2001/08/23 16:03:26  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.24  2001/08/16 21:17:34  laurentg
// Automatic calibration improved with a fine adjustment
//
// Revision 1.23  2001/08/15 17:50:11  laurentg
// UseRGB ini parameter suppressed
// OSD screen concerning card calibration fully modified
// Automatic calibration added (not finished)
//
// Revision 1.22.2.7  2001/08/24 12:35:09  adcockj
// Menu handling changes
//
// Revision 1.22.2.6  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.22.2.5  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.22.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.22.2.3  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.22.2.2  2001/08/16 06:43:34  adcockj
// moved more stuff into the new file (deonsn't compile)
//
// Revision 1.22.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
// Revision 1.22  2001/08/11 15:17:06  laurentg
// Bug fixed
//
// Revision 1.21  2001/08/09 22:18:23  laurentg
// Improvments in relation with calibration
//
// Revision 1.20  2001/08/08 21:58:16  laurentg
// Lot of comments added - not finished
//
// Revision 1.19  2001/08/05 20:14:49  laurentg
// New OSD screen added for AR autodetection
//
// Revision 1.18  2001/07/30 19:51:30  laurentg
// no message
//
// Revision 1.17  2001/07/29 22:51:09  laurentg
// OSD screen for card calibration improved
// Test patterns description added or corrected
//
// Revision 1.16  2001/07/28 16:15:15  laurentg
// New test patterns added
//
// Revision 1.15  2001/07/26 22:02:12  laurentg
// New entry in OSD section of ini file
// New OSD screen for card calibration
//
// Revision 1.14  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.13  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.12  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.11  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file OSD.cpp OSD Functions
 */

#include "stdafx.h"
#include "OSD.h"
#include "AspectRatio.h"
#include "Other.h"
#include "Audio.h"
#include "OutThreads.h"
#include "FD_60Hz.h"
#include "Filter.h"
#include "Dialogs.h"
#include "DScaler.h"
#include "VBI_WSSdecode.h"
#include "Calibration.h"
#include "Providers.h"
#include "Perf.h"
#include "DebugLog.h"
#include "epg.h"
#include "ProgramList.h"
#include "VBI.h"
#include "VBI_VideoText.h"
#include "VBI_VPSdecode.h"


#define OSD_COLOR_TITLE     RGB(255,150,150)
#define OSD_COLOR_SECTION   RGB(150,150,255)
#define OSD_COLOR_CURRENT   RGB(200,150,0)

static char* OSD_szFontName = NULL;


// OSD placement values
enum eOSDTextXPos
{
    OSD_XPOS_LEFT = 0,
    OSD_XPOS_RIGHT,
    OSD_XPOS_CENTER,
};

// OSD Command values
enum
{
    OSDC_SHOW_TEXT,
    OSDC_SHOW_SCREEN,
    OSDC_SHOW_NEXT_SCREEN,
    OSDC_CLEAR,
};

// OSD Command flags
enum
{
    OSDF_PERSISTENT     = 0x01,
};


// OSD refresh function callback definition
typedef void (OSDREFRESHFUNCTION)(double Size);


// Stores the OSD data cache
typedef struct
{
    char            szText[512];        // Text of OSD
    double          Size;               // Size of OSD as percentage of screen height
    long            TextColor;          // Text color (RGB)
    long            BackgroundColor;    // Background color (RGB)
    eOSDBackground  BackgroundMode;     // Background mode
    eOSDTextXPos    TextXPos;           // Text position / Xpos
    double          XPos;               // X position (0 = left, 1 = right)
    double          YPos;               // Y position (0 = top, 1 = bottom)
    RECT            CurrentRect;        // MRS 2-24-01 Saves the current drawn rectangle (used to limit invalidation area)
} TOSDInfo;


// Used to define OSD screens
typedef struct 
{
    char    name[24];       // Name of the screen
    BOOL    managed_by_app; // TRUE if only app can display the screen
    BOOL    auto_hide;      // TRUE if the screen must disappear after a delay
    int     refresh_delay;  // Refresh period in ms (0 means no refresh)
    BOOL    active;         // TRUE if user can show the screen
    BOOL    lock;           // TRUE if display of the screen should lock OSD
    OSDREFRESHFUNCTION* RefreshFunction;    // Function to call to fill the screen
} TActiveScreen;


// Used for posting OSD commands
typedef struct
{
    BYTE            uCommand;
    BYTE            uFlags;
    DWORD           dwParam;
    DOUBLE          dSize;
} TOSDCommand;


void OSD_PostCommand(TOSDCommand* pOSDCommand);
void OSD_FreeCommand(TOSDCommand* pOSDCommand);

void OSD_AddTextSingleLine(LPCTSTR, double, long, long, eOSDBackground, eOSDTextXPos, double, double);
void OSD_AddText(LPCTSTR, double, long, long, eOSDBackground, eOSDTextXPos, double, double);
int OSD_CutLines(LPCTSTR, long);
void OSD_ClearAllTexts();


long            OSD_OutlineColor = RGB(0, 0, 0);
long            OSD_TextColor = RGB(0, 255, 0);
long            OSD_DefaultSizePerc = 10;
long            OSD_DefaultSmallSizePerc = 4;
BOOL            OSD_bAntiAlias = TRUE;
BOOL            OSD_bOutline = TRUE;
BOOL            OSD_bAutoHide = TRUE;
BOOL            OSD_bOverride = FALSE;
eOSDBackground  OSD_Background;
TOSDCommand*    OSD_pPendingCommand = NULL;

BOOL            OSD_bRedrawCalled;

TOSDInfo        OSD_Text[OSD_MAX_TEXT];         // Global OSD Information cache
long            OSD_nTextCount = 0;

static int      OSD_IdxCurrentScreen = -1;      // Index of the displayed OSD screen
static BOOL     OSD_bRestoreScreen = FALSE;     // Restore previous screen on clear OSD

static void OSD_RefreshGeneralScreen(double Size);
static void OSD_RefreshStatisticsScreen(double Size);
static void OSD_RefreshWSSScreen(double Size);
static void OSD_RefreshARScreen(double Size);
static void OSD_RefreshCalibrationScreen(double Size);
static void OSD_RefreshDeveloperScreen(double Size);
static void OSD_RefreshCurrentProgrammeScreen(double Size);
static void OSD_RefreshProgrammesScreen(double Size);
static void OSD_DisplayProgrammeInfos(double Size);


// Screens definition
static TActiveScreen ActiveScreens[] =
{
    { "Card calibration screen", TRUE,  FALSE, 250,                     TRUE,  TRUE,  OSD_RefreshCalibrationScreen },
    { "Short prog screen",       TRUE,  TRUE,  0,                       TRUE,  FALSE, OSD_DisplayProgrammeInfos },
    { "Detailed prog screen",    TRUE,  FALSE, 500,                     TRUE,  TRUE,  OSD_RefreshCurrentProgrammeScreen },
    { "Prog browser screen",     TRUE,  FALSE, 250,                     TRUE,  TRUE,  OSD_RefreshProgrammesScreen },
    { "General screen",          FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, TRUE,  FALSE, OSD_RefreshGeneralScreen },
    { "Statistics screen",       FALSE, TRUE,  1000,                    TRUE,  FALSE, OSD_RefreshStatisticsScreen },
    { "WSS decoding screen",     FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, TRUE,  FALSE, OSD_RefreshWSSScreen },
//  { "AR screen",               FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, FALSE, FALSE, OSD_RefreshARScreen },
#ifdef USE_PERFORMANCE_STATS
    { "Developer screen",        FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, FALSE, FALSE, OSD_RefreshDeveloperScreen },
#endif
};


// OSD background names
static const char* OSD_szBackgroundNames[OSDB_LASTONE] = 
{
    "Transparent",
    "Block",
    "Shaded",
};


static char TextLines[128][512];


void OSD_Init()
{
    // nothing to do
}


void OSD_Exit()
{
    TOSDCommand* pOSDCommand;

    // Clean up any unhandled commands
    pOSDCommand = (TOSDCommand*)InterlockedExchange(
        (LPLONG)&OSD_pPendingCommand, (LONG)NULL);

    if (pOSDCommand != NULL)
    {
        OSD_FreeCommand(pOSDCommand);
    }
}


//---------------------------------------------------------------------------
// External: Displays the specified OSD text
void OSD_ShowText(LPCTSTR szText, double dSize, BOOL bPersistent, BOOL bOverride)
{
    if (bOverride)
    {
        OSD_bOverride = (*szText != '\0');
    }
    else if (OSD_bOverride)
    {
        return;
    }

    if (*szText == '\0')
    {
        OSD_Clear();
        return;
    }

    TOSDCommand* pOSDCommand;

    pOSDCommand = (TOSDCommand*)malloc(sizeof(TOSDCommand));

    pOSDCommand->uCommand = OSDC_SHOW_TEXT;
    pOSDCommand->uFlags = 0;
    pOSDCommand->dSize = dSize;

    ULONG nDataLength = strlen(szText) + 1;

    LPSTR pszText = (LPSTR)malloc(nDataLength);
    memcpy(pszText, szText, nDataLength);

    pOSDCommand->dwParam = (DWORD)pszText;

    if (bPersistent != FALSE)
    {
        pOSDCommand->uFlags |= OSDF_PERSISTENT;
    }

    OSD_PostCommand(pOSDCommand);
}


//---------------------------------------------------------------------------
// External: Displayed specified OSD text without autohide timer.
// External: Stays on screen until a new OSD message replaces current OSD message.
void OSD_ShowTextPersistent(LPCTSTR szText, double dSize)
{
    OSD_ShowText(szText, dSize, TRUE);
}


//---------------------------------------------------------------------------
// External: Override all previous OSD text, and force this current OSD text
// External: to override all other OSD text showings (done by the above functions).
// External: This is useful for external programs to override DScaler's own OSD text
// External: for its own controls.
void OSD_ShowTextOverride(LPCTSTR szText, double dSize)
{
    OSD_ShowText(szText, dSize, FALSE, TRUE);
}


//---------------------------------------------------------------------------
// External: Display source comments in OSD with autohide
void OSD_ShowSourceComments()
{
    if (OSD_bOverride)
    {
        return;
    }

    CSource* pSource = Providers_GetCurrentSource();

    if (pSource != NULL)
    {
        LPSTR lpComments = pSource->GetComments();

        if (*lpComments != '\0')
        {
            OSD_ShowText(lpComments, OSD_DefaultSmallSizePerc);
        }
    }
}


//---------------------------------------------------------------------------
// External: Activate or deactivate the information screen
// External: whose number is given as parameter
void OSD_ActivateInfosScreen(INT IdxScreen, DOUBLE Size)
{
    int     NbScreens;      // number of OSD scrrens

    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    if (IdxScreen >= 0 && IdxScreen < NbScreens)
    {
        ActiveScreens[IdxScreen].active = !ActiveScreens[IdxScreen].active;
        if (IdxScreen == OSD_IdxCurrentScreen)
        {
            OSD_Clear();
        }
    }
}


//---------------------------------------------------------------------------
// External: Displays the specified OSD screen
void OSD_ShowInfosScreen(INT IdxScreen, DOUBLE dSize)
{
	// Laurent: change not very clean !!!
	// When the display of the current screen disables all other OSD
	// and we try to display another OSD screen managed by the application,
	// we accept it
    if (OSD_bOverride)
    {
		int nScreens = sizeof(ActiveScreens) / sizeof(ActiveScreens[0]);

		if (IdxScreen < 0 || IdxScreen >= nScreens)
		{
			IdxScreen = -1;
		}
		else if (!ActiveScreens[IdxScreen].active)
		{
			IdxScreen = -1;
		}
		if (   (IdxScreen == -1)
			|| (ActiveScreens[IdxScreen].lock == FALSE) )
	        return;
    }

    TOSDCommand* pOSDCommand;

    pOSDCommand = (TOSDCommand*)malloc(sizeof(TOSDCommand));

    pOSDCommand->uCommand = OSDC_SHOW_SCREEN;
    pOSDCommand->dwParam = (DWORD)IdxScreen;
    pOSDCommand->dSize = dSize;

    OSD_PostCommand(pOSDCommand);
}


//---------------------------------------------------------------------------
// External: Display on screen the first information screen if
// External: no screen is already displayed, or the next one
void OSD_ShowNextInfosScreen(DOUBLE dSize)
{
    if (OSD_bOverride)
    {    
        return;
    }

    TOSDCommand* pOSDCommand;

    pOSDCommand = (TOSDCommand*)malloc(sizeof(TOSDCommand));

    pOSDCommand->uCommand = OSDC_SHOW_NEXT_SCREEN;
    pOSDCommand->dSize = dSize;

    OSD_PostCommand(pOSDCommand);
}


//---------------------------------------------------------------------------
// External: Clears anything displayed in OSD
void OSD_Clear()
{
    TOSDCommand* pOSDCommand;

    pOSDCommand = (TOSDCommand*)malloc(sizeof(TOSDCommand));

    pOSDCommand->uCommand = OSDC_CLEAR;

    OSD_PostCommand(pOSDCommand);
}


//---------------------------------------------------------------------------
// Linking: Handles UWM_OSD OSDM_DISPLAYUPDATE messages
// Linking: to perform posted drawing commands
void OSD_ProcessDisplayUpdate(HDC hDC, LPRECT lpRect)
{
    TOSDCommand* pOSDCommand;

    // Get the command off the queue
    pOSDCommand = (TOSDCommand*)InterlockedExchange(
            (LPLONG)&OSD_pPendingCommand, (LONG)NULL);

    if (pOSDCommand == NULL)
    {
        return;
    }

    switch (pOSDCommand->uCommand)
    {
    case OSDC_SHOW_TEXT:
        {
            LPSTR pszText = (LPSTR)pOSDCommand->dwParam;

            OSD_ShowText(hDC, lpRect, pszText, pOSDCommand->dSize,
                (pOSDCommand->uFlags & OSDF_PERSISTENT) != 0);
        }
        break;

    case OSDC_SHOW_SCREEN:
        OSD_ShowInfosScreen(hDC, lpRect, (INT)pOSDCommand->dwParam, pOSDCommand->dSize);
        break;

    case OSDC_SHOW_NEXT_SCREEN:
        OSD_ShowNextInfosScreen(hDC, lpRect, pOSDCommand->dSize);
        break;

    case OSDC_CLEAR:
        OSD_Clear(hDC, lpRect);
        break;
    }

    OSD_FreeCommand(pOSDCommand);
}


//---------------------------------------------------------------------------
// Reserved: Draws the specified OSD text onto the screen and
// Reserved: starts the auto-hide timer
void OSD_ShowText(HDC hDC, LPRECT lpRect, LPCTSTR pszText, double dSize,
                  BOOL bPersistent)
{
    OSD_InvalidateTextsArea();
    OSD_ClearAllTexts();

    OSD_AddText(pszText, dSize, -1, -1,
        OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9, 0.1);

    OSD_Redraw(hDC, lpRect);

    KillTimer(GetMainWnd(), OSD_TIMER_REFRESH_ID);

    if (bPersistent == FALSE)
    {
        SetTimer(GetMainWnd(), OSD_TIMER_ID, OSD_TIMER_DELAY, NULL);
    }
    else
    {
        KillTimer(GetMainWnd(), OSD_TIMER_ID);
    }

    if (OSD_bAutoHide)
    {
        OSD_IdxCurrentScreen = -1;
    }
    OSD_bRestoreScreen = (OSD_IdxCurrentScreen != -1);
}


//---------------------------------------------------------------------------
// Reserved: Display on screen the information screen whose number
// Reserved: is given as parameter
void OSD_ShowInfosScreen(HDC hDC, LPRECT lpRect, INT IdxScreen, DOUBLE Size)
{
    int nScreens = sizeof(ActiveScreens) / sizeof(ActiveScreens[0]);

    if (IdxScreen < 0 || IdxScreen >= nScreens)
    {
        IdxScreen = -1;
    }
    else if (!ActiveScreens[IdxScreen].active)
    {
        IdxScreen = -1;
    }

    BOOL bScreenDisplayed = (OSD_IdxCurrentScreen != -1);

    OSD_IdxCurrentScreen = IdxScreen;
    OSD_bRestoreScreen = FALSE;

    // if no screen to display
    if (IdxScreen == -1)
    {
        // If there is a screen displayed
        if (bScreenDisplayed != FALSE)
        {
            OSD_Clear(hDC, lpRect);
        }
    }
    else
    {
        OSD_RefreshInfosScreen(hDC, lpRect, Size);

        if (ActiveScreens[IdxScreen].auto_hide)
        {
            SetTimer(GetMainWnd(), OSD_TIMER_ID, OSD_TIMER_DELAY, NULL);
        }
        else
        {
            KillTimer(GetMainWnd(), OSD_TIMER_ID);
        }
    }
}


//---------------------------------------------------------------------------
// Reserved: Display on screen the first information screen if no
// Reserved: screen is already displayed, or the next one
void OSD_ShowNextInfosScreen(HDC hDC, LPRECT lpRect, DOUBLE Size)
{
    int nScreens = sizeof(ActiveScreens) / sizeof(ActiveScreens[0]);

    // determine which screen to display
    for (int i = OSD_IdxCurrentScreen + 1; i < nScreens; i++)
    {
        if (ActiveScreens[i].active && !ActiveScreens[i].managed_by_app)
        {
            break;
        }
    }

    if (i == nScreens)
    {
        OSD_ShowInfosScreen(hDC, lpRect, -1, Size);
    }
    else
    {
        OSD_ShowInfosScreen(hDC, lpRect, i, Size);
    }
}


//---------------------------------------------------------------------------
// Reserved: Clear currently displayed OSD
void OSD_Clear(HDC hDC, LPRECT lpRect)
{
    KillTimer(GetMainWnd(), OSD_TIMER_ID);
    KillTimer(GetMainWnd(), OSD_TIMER_REFRESH_ID);

    OSD_bOverride = FALSE;

    OSD_InvalidateTextsArea();

    if (OSD_bRestoreScreen && OSD_IdxCurrentScreen != -1)
    {
        if (ActiveScreens[OSD_IdxCurrentScreen].active)
        {
            OSD_ShowInfosScreen(hDC, lpRect, OSD_IdxCurrentScreen, 0);
        }
    }
    else
    {
        OSD_ClearAllTexts();
        OSD_IdxCurrentScreen = -1;
    }

	MyEPG.HideOSD();
}


//---------------------------------------------------------------------------
// Reserved: Invalidates the RECTs used by the OSD texts
void OSD_InvalidateTextsArea()
{
    for (int i = 0 ; i < OSD_nTextCount ; i++)
    {
        InvalidateRect(GetMainWnd(), &(OSD_Text[i].CurrentRect), FALSE);
    }
}


//---------------------------------------------------------------------------
// Reserved: Refresh the current information screen
void OSD_RefreshInfosScreen(HDC hDC, LPRECT lpRect, double Size)
{
    // Case : no OSD screen
    if (OSD_IdxCurrentScreen == -1)
    {
        return;
    }

    OSD_InvalidateTextsArea();
    OSD_ClearAllTexts();

#ifdef USE_PERFORMANCE_STATS
    LARGE_INTEGER FirstCount, SecondCount, Frequency;
    BOOL bPerformanceCounter = QueryPerformanceCounter(&FirstCount);
#endif

    if (ActiveScreens[OSD_IdxCurrentScreen].RefreshFunction != NULL)
    {
        (*(ActiveScreens[OSD_IdxCurrentScreen].RefreshFunction))(Size);
    }

    if (ActiveScreens[OSD_IdxCurrentScreen].lock)
    {
        OSD_bOverride = TRUE;
    }

    OSD_Redraw(hDC, lpRect);

#ifdef USE_PERFORMANCE_STATS
    if (bPerformanceCounter)
    {
        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&SecondCount);
        LOGD("OSD_RefreshInfosScreen in %ld ms\n", (int)(SecondCount.QuadPart -
            FirstCount.QuadPart)/(Frequency.QuadPart/1000));
    }
#endif

	if (ActiveScreens[OSD_IdxCurrentScreen].refresh_delay)
	{
		SetTimer(GetMainWnd(), OSD_TIMER_REFRESH_ID,
			ActiveScreens[OSD_IdxCurrentScreen].refresh_delay, NULL);
	}
}


//---------------------------------------------------------------------------
// Reserved: OSD Redrawing code.
void OSD_Redraw(HDC hDC, LPRECT lpRect)
{
    CHAR        szCurrentFont[32];
    HFONT       hOSDFont;
    HFONT       hOSDFontOutline;
    HFONT       hSaveFont;
    int         nFontSize;
    int         nXpos, nYpos;
    int         nXWinSize, nYWinSize;
    SIZE        sizeText;
    DWORD       dwQuality;

    if (hDC == NULL || lpRect == NULL || *OSD_Text[0].szText == '\0')
    {
        return;
    }

    nXWinSize = lpRect->right  - lpRect->left;
    nYWinSize = lpRect->bottom - lpRect->top;

    dwQuality = (OSD_bAntiAlias ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY);

    OSD_bRedrawCalled = TRUE;

    for (int i = 0 ; i < OSD_nTextCount ; i++)
    {
        nFontSize = (int)((double)nYWinSize * (OSD_Text[i].Size / 100.00));

        // Set specified font
        strcpy(szCurrentFont, OSD_szFontName);
        hOSDFont = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, dwQuality, DEFAULT_PITCH | FF_DONTCARE, OSD_szFontName);
        if (hOSDFont == NULL)
        {
            // Fallback to Arial
            strcpy(szCurrentFont, "Arial");
            hOSDFont = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont);
            if (hOSDFont == NULL)
            {
                // Otherwise, fallback to any available font
                strcpy(szCurrentFont, "");
                hOSDFont = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont);
            }
        }
        if (hOSDFont == NULL)
        {
            ErrorBox("Failed To Create OSD Font");
        }
        hOSDFontOutline = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, szCurrentFont);

        if (hOSDFontOutline == NULL)
        {
            hOSDFontOutline = hOSDFont;
        }

        hSaveFont = (HFONT)SelectObject(hDC, hOSDFontOutline);
        if (hSaveFont != NULL)
        {
            GetTextExtentPoint32(hDC, OSD_Text[i].szText, strlen(OSD_Text[i].szText), &sizeText);

            switch (OSD_Text[i].TextXPos)
            {
            case OSD_XPOS_RIGHT:
                nXpos = (int)((double)nXWinSize * OSD_Text[i].XPos) - sizeText.cx;
                break;
            case OSD_XPOS_CENTER:
                nXpos = (int)((double)nXWinSize * OSD_Text[i].XPos - (double)sizeText.cx / 2.0);
                break;
            case OSD_XPOS_LEFT:
            default:
                nXpos = (int)((double)nXWinSize * OSD_Text[i].XPos);
                break;
            }

            nYpos = (int)((double)nYWinSize * OSD_Text[i].YPos);

            // Draw the requested background for the text
            switch(OSD_Text[i].BackgroundMode)
            {
            case OSDB_TRANSPARENT:
                SetBkMode(hDC, TRANSPARENT);
                SetBkColor(hDC, OSD_Text[i].BackgroundColor);
                break;
        
            case OSDB_BLOCK:
                SetBkMode(hDC, OPAQUE);
                SetBkColor(hDC, OSD_Text[i].BackgroundColor);
                break;
        
            case OSDB_SHADED:
                {
                    WORD bBrushBits[8] = { 0xAAAA, 0x5555, 0xAAAA, 0x5555,
                                           0xAAAA, 0x5555, 0xAAAA, 0x5555 };

                    SetBkMode(hDC, TRANSPARENT);
                    SetTextColor(hDC, OSD_Text[i].BackgroundColor);
                    SetBkColor(hDC, Overlay_GetCorrectedColor(hDC));

                    HBITMAP hBM = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBrushBits); 
                    HBRUSH hBrush = CreatePatternBrush(hBM);
                    HBRUSH hSaveBrush = (HBRUSH)SelectObject(hDC, hBrush);

                    if (OSD_bOutline)
                    {
                        PatBlt(hDC, lpRect->left+ nXpos - 2, lpRect->top+ nYpos - 2, sizeText.cx + 4, sizeText.cy + 4, PATCOPY);
                    }
                    else
                    {
                        PatBlt(hDC, lpRect->left+nXpos, lpRect->top+ nYpos, sizeText.cx, sizeText.cy, PATCOPY);
                    }

                    SelectObject(hDC, hSaveBrush);
                    DeleteObject(hBrush);
                    DeleteObject(hBM);
                }
                break;

            default:
                break;
            }

            if (OSD_bOutline)
            {
                // Draw OSD outline if required
                SetTextColor(hDC, OSD_OutlineColor);
                TextOut(hDC, lpRect->left+nXpos - 2, lpRect->top+ nYpos, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos + 2, lpRect->top+ nYpos, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos, lpRect->top+ nYpos - 2, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos, lpRect->top+ nYpos + 2, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos - 1, lpRect->top+ nYpos - 1, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos + 1, lpRect->top+ nYpos - 1, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos - 1, lpRect->top+ nYpos + 1, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos + 1, lpRect->top+ nYpos + 1, OSD_Text[i].szText, strlen(OSD_Text[i].szText));
            }

            // Draw OSD text
            if (SelectObject(hDC, hOSDFont))
            {
                SetTextColor(hDC, OSD_Text[i].TextColor);
                SetBkColor(hDC, OSD_Text[i].BackgroundColor);
                TextOut(hDC, lpRect->left+nXpos, lpRect->top+ nYpos, OSD_Text[i].szText, strlen(OSD_Text[i].szText));

                // MRS 2-23-01 Calculate rectnagle for the entire OSD 
                // so we do not invalidate the entire window to remove it.
                SIZE sz;
                GetTextExtentExPoint(hDC, OSD_Text[i].szText, strlen(OSD_Text[i].szText),
                    32000, NULL, NULL, &sz);

                SetRect(&OSD_Text[i].CurrentRect,
                    lpRect->left + nXpos - 4,
                    lpRect->top + nYpos - 4,
                    lpRect->left + nXpos + sz.cx + 4,
                    lpRect->top + nYpos + sz.cy + 4);

                if (OSD_Text[i].CurrentRect.left < 0)
                {
                    OSD_Text[i].CurrentRect.left = 0;
                }
                if (OSD_Text[i].CurrentRect.top < 0)
                {
                    OSD_Text[i].CurrentRect.top = 0;
                }
            }

            SelectObject(hDC, hSaveFont);
            DeleteObject(hOSDFont);
            DeleteObject(hOSDFontOutline);
        }           
    }
}


//---------------------------------------------------------------------------
// Reserved: Retreives all the RECTs we have painted too
LONG OSD_GetPaintedRects(RECT* pRectBuffer, LONG nBufferSize)
{
    if (!OSD_bRedrawCalled)
    {
        return 0;
    }

    for (int i = 0 ; i < nBufferSize && i < OSD_nTextCount ; i++)
    {
        CopyRect(&pRectBuffer[i], &OSD_Text[i].CurrentRect);        
    }

    return i;
}


//---------------------------------------------------------------------------
// Reserved: Forget the RECTs we have painted too
void OSD_ResetPaintedRects()
{
    OSD_bRedrawCalled = FALSE;
}


//---------------------------------------------------------------------------
// Internal: Posts a command to be processed later
void OSD_PostCommand(TOSDCommand* pOSDCommand)
{
    // Put the command onto the one element queue
    pOSDCommand = (TOSDCommand*)InterlockedExchange(
            (LPLONG)&OSD_pPendingCommand, (LONG)pOSDCommand);

    if (pOSDCommand != NULL)
    {
        // Free the buffer we overwrote
        OSD_FreeCommand(pOSDCommand);
    }
    else
    {
        // Start the queue
        PostMessage(GetMainWnd(), UWM_OSD, OSDM_DISPLAYUPDATE, 0);
    }
}


//---------------------------------------------------------------------------
// Internal: Frees the buffer used by a command
void OSD_FreeCommand(TOSDCommand* pOSDCommand)
{
    if (pOSDCommand->uCommand == OSDC_SHOW_TEXT)
    {
        LPSTR pszText = (LPSTR)pOSDCommand->dwParam;

        if (pszText != NULL)
        {
            free(pszText);
        }
    }

    free(pOSDCommand);
}


/////////////////////////////////////////////////////////////////////////////
// Start of screens data refresh handlers (Internal)
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// Calculate vertical position of line in OSD screen
//    Return Value between 0 (top) and 1 (bottom)
//    Use line number > 0 if reference is top
//    Use line number < 0 if reference is bottom
//    dfMargin is a percent of screen height/width and Value must be between 0 and 1
//    Size is a percent of screen height and Value must be between 0 and 100
static double OSD_GetLineYpos (int nLine, double dfMargin, double Size)
{
    double  dfY;
    double  dfH = Size / 100;

    // Line number 0 has no sense
    if (nLine == 0) return (0);

    if (nLine > 0)
    {
        dfY = dfMargin + ((double)nLine - 1) * dfH;
    }
    else
    {
        dfY = 1 - dfMargin + (double)nLine * dfH;
    }

    // Line outside screen
    if ( (dfY < 0) || ((dfY + dfH) > 1) )
    {
        dfY = 0;
    }

    return (dfY);
}


static void OSD_GetTextResult(int delta, char *text, long *color)
{
    if (delta <= 3)
    {
        strcpy (text, "*****");
    }
    else if (delta <= 9)
    {
        strcpy (text, "****");
    }
    else if (delta <= 18)
    {
        strcpy (text, "***");
    }
    else if (delta <= 30)
    {
        strcpy (text, "**");
    }
    else
    {
        strcpy (text, "*");
    }
    *color = -1;
}


extern long NumFilters;
extern FILTER_METHOD* Filters[];

static void OSD_RefreshGeneralScreen(double Size)
{
    double  dfMargin = 0.02;    // 2% of screen height/width
    char    szInfo[64];
    int     nLine;
    int     i;
    CSource* pSource = Providers_GetCurrentSource();
    ISetting* pSetting = NULL;
    int     OverlaySetting;
    BOOL    UseOverlayCtrl;
    BOOL    DisplayTitle;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // DScaler version
    OSD_AddText(GetProductNameAndVersion(), Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    // Channel
    nLine = 3;

    OSD_AddText("Source", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    // Video input
    OSD_AddText(pSource != NULL ? pSource->GetStatus() : "", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    // Audio mute
    if (Setting_GetValue(Audio_GetSetting(SYSTEMINMUTE)) == TRUE)
    {
        OSD_AddText("Volume Mute", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    // Source size
    if (pSource != NULL)
    {
		if (pSource->GetWidth() != pSource->GetInitialWidth() || pSource->GetHeight() != pSource->GetInitialHeight())
		{
	        sprintf (szInfo, "Size %ux%u (%ux%u)", pSource->GetWidth(), pSource->GetHeight(), pSource->GetInitialWidth(), pSource->GetInitialHeight());
		}
		else
		{
		    sprintf (szInfo, "Size %ux%u", pSource->GetWidth(), pSource->GetHeight());
		}
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    // Source ratio
    if (!AspectSettings.SquarePixels)
    {
        sprintf(szInfo, "Ratio %.2f:1", (double)Setting_GetValue(Aspect_GetSetting(SOURCE_ASPECT)) / 1000.0);
        if ( (Setting_GetValue(Aspect_GetSetting(ASPECT_MODE)) == 1)
          && (Setting_GetValue(Aspect_GetSetting(SOURCE_ASPECT)) != 1333) )
        {
            strcat(szInfo, " Letterbox");
        }
        else if (Setting_GetValue(Aspect_GetSetting(ASPECT_MODE)) == 2)
        {
            strcat(szInfo, " Anamorphic");
        }
        if (Setting_GetValue(Aspect_GetSetting(AUTODETECTASPECT)))
        {
            strcat(szInfo, " auto");
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    // Display ratio
    nLine++;
    OSD_AddText("Display", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    if (Setting_GetValue(Aspect_GetSetting(TARGET_ASPECT)) == 0)
    {
        strcpy(szInfo, "Ratio from current resolution");
    }
    else
    {
        sprintf(szInfo, "Ratio %.2f:1", (double)Setting_GetValue(Aspect_GetSetting(TARGET_ASPECT)) / 1000.0);
    }
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    // Video and overlay settings
    nLine = 4;
    DisplayTitle = FALSE;
    UseOverlayCtrl = Setting_GetValue(Other_GetSetting(USEOVERLAYCONTROLS));
    pSetting = pSource != NULL ? pSource->GetBrightness() : NULL;
    OverlaySetting = Setting_GetValue(Other_GetSetting(OVERLAYBRIGHTNESS));
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText("Brightness", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        sprintf (szInfo, "%d", pSetting->GetValue());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        sprintf (szInfo, "%d", OverlaySetting);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetContrast() : NULL;
    OverlaySetting = Setting_GetValue(Other_GetSetting(OVERLAYCONTRAST));
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText("Contrast", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        sprintf (szInfo, "%d", pSetting->GetValue());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        sprintf (szInfo, "%d", OverlaySetting);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetHue() : NULL;
    OverlaySetting = Setting_GetValue(Other_GetSetting(OVERLAYHUE));
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText("Hue", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        sprintf (szInfo, "%d", pSetting->GetValue());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        sprintf (szInfo, "%d", OverlaySetting);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetSaturation() : NULL;
    OverlaySetting = Setting_GetValue(Other_GetSetting(OVERLAYSATURATION));
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText("Color", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        sprintf (szInfo, "%d", pSetting->GetValue());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        sprintf (szInfo, "%d", OverlaySetting);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetSaturationU() : NULL;
    if(pSetting != NULL)
    {
        OSD_AddText("Color U", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        sprintf (szInfo, "%d", pSetting->GetValue());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetSaturationV() : NULL;
    if(pSetting != NULL)
    {
        OSD_AddText("Color V", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        sprintf (szInfo, "%d", pSetting->GetValue());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    OverlaySetting = Setting_GetValue(Other_GetSetting(OVERLAYGAMMA));
    if(UseOverlayCtrl)
    {
        OSD_AddText("Gamma", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        sprintf (szInfo, "%d", OverlaySetting);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    OverlaySetting = Setting_GetValue(Other_GetSetting(OVERLAYSHARPNESS));
    if(UseOverlayCtrl)
    {
        OSD_AddText("Sharpness", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        sprintf (szInfo, "%d", OverlaySetting);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    if (DisplayTitle)
    {
        if(UseOverlayCtrl)
        {
            OSD_AddText("Video & Overlay settings", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
        }
        else
        {
            OSD_AddText("Video settings", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
        }
    }

    // Deinterlace Mode
    nLine = -1;
    if (Setting_GetValue(OutThreads_GetSetting(DOACCURATEFLIPS)))
    {
        OSD_AddText("Judder Terminator", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
    }
    if (!IsProgressiveMode())
    {
        if (Setting_GetValue(FD60_GetSetting(FALLBACKTOVIDEO)))
        {
            OSD_AddText("Fallback on Bad Pulldown", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        }
        if (Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
        {
            OSD_AddText("Auto Pulldown Detect", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        }
    }
    OSD_AddText(GetDeinterlaceModeName(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
    OSD_AddText("Deinterlacing", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));

    // Filters
    nLine = -1;
    DisplayTitle = FALSE;
    for (i = 0 ; i < NumFilters ; i++)
    {
        strcpy(szInfo, Filters[i]->szName);
        if (Filters[i]->bActive)
        {
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
            DisplayTitle = TRUE;
        }
    }
    if (DisplayTitle)
    {
        OSD_AddText("Filters", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
}


static void OSD_RefreshStatisticsScreen(double Size)
{
    double              dfMargin = 0.02;    // 2% of screen height/width
    char                szInfo[64];
    int                 nLine, nCol, nFirstLine;
    int                 i;
    long                Color;
    double              pos;
    DEINTERLACE_METHOD* DeintMethod;
    DWORD               CurrentTicks;
    int                 ticks;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText("Statistics", Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 1;

    OSD_AddText("Dropped fields", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    sprintf (szInfo, "Number : %ld", pPerf->GetNumberDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Last second : %d", pPerf->GetDroppedFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Average / s : %.1f", pPerf->GetAverageDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    OSD_AddText("Not waited", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    sprintf (szInfo, "Number : %ld", pPerf->GetNumberNotWaitedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Last second : %d", pPerf->GetNotWaitedFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Average / s : %.1f", pPerf->GetAverageNotWaitedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    OSD_AddText("Processed late", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    sprintf (szInfo, "Number : %ld", pPerf->GetNumberLateFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Last second : %d", pPerf->GetLateFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Average / s : %.1f", pPerf->GetAverageLateFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

//    OSD_AddText("Used fields", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//
//    sprintf (szInfo, "Last second : %d", pPerf->GetUsedFieldsLastSecond());
//    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//    sprintf (szInfo, "Average / s : %.1f", pPerf->GetAverageUsedFields());
//    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    if (Setting_GetValue(OutThreads_GetSetting(DOACCURATEFLIPS)))
    {
		OSD_AddText("No flip at time", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

		sprintf (szInfo, "Number : %ld", pPerf->GetNumberNoFlipAtTime());
		OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
		sprintf (szInfo, "Last second : %d", pPerf->GetNoFlipAtTimeLastSecond());
		OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
		sprintf (szInfo, "Average / s : %.1f", pPerf->GetAverageNoFlipAtTime());
		OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
	}

    nLine = 1;

    CurrentTicks = GetTickCount();

    OSD_AddText("Deinterlace Modes", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    sprintf (szInfo, "Number of changes : %ld", nTotalDeintModeChanges);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//    OSD_AddText("changes - % of time - Mode", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    for (i = 0 ; i < PROGPULLDOWNMODES_LAST_ONE ; i++)
    {
        DeintMethod = GetProgressiveMethod(i);
        if (DeintMethod->ModeChanges > 0)
        {
            pos = OSD_GetLineYpos (nLine, dfMargin, Size);
            if (pos > 0)
            {
                if (DeintMethod == GetCurrentDeintMethod())
                {
                    Color = OSD_COLOR_CURRENT;
                    ticks = DeintMethod->ModeTicks + CurrentTicks - nLastTicks;
                }
                else
                {
                    Color = -1;
                    ticks = DeintMethod->ModeTicks;
                }
                sprintf (szInfo, "%03d - %05.1f %% - %s", DeintMethod->ModeChanges, ticks * 100 / (double)(CurrentTicks - nInitialTicks), DeintMethod->szName);
                OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
                nLine++;
            }
        }
    }
    for (i = 0 ; i < FILMPULLDOWNMODES_LAST_ONE ; i++)
    {
        DeintMethod = GetFilmDeintMethod((eFilmPulldownMode)i);
        if (DeintMethod->ModeChanges > 0)
        {
            pos = OSD_GetLineYpos (nLine, dfMargin, Size);
            if (pos > 0)
            {
                if (DeintMethod == GetCurrentDeintMethod())
                {
                    Color = OSD_COLOR_CURRENT;
                    ticks = DeintMethod->ModeTicks + CurrentTicks - nLastTicks;
                }
                else
                {
                    Color = -1;
                    ticks = DeintMethod->ModeTicks;
                }
                sprintf (szInfo, "%03d - %05.1f %% - %s", DeintMethod->ModeChanges, ticks * 100 / (double)(CurrentTicks - nInitialTicks), DeintMethod->szName);
                OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
                nLine++;
            }
        }
    }
    i = 0;
    DeintMethod = GetVideoDeintMethod(i);
    while(DeintMethod != NULL)
    {
        if (DeintMethod->ModeChanges > 0)
        {
            pos = OSD_GetLineYpos (nLine, dfMargin, Size);
            if (pos > 0)
            {
                if (DeintMethod == GetCurrentDeintMethod())
                {
                    Color = OSD_COLOR_CURRENT;
                    ticks = DeintMethod->ModeTicks + CurrentTicks - nLastTicks;
                }
                else
                {
                    Color = -1;
                    ticks = DeintMethod->ModeTicks;
                }
                sprintf (szInfo, "%03d - %05.1f %% - %s", DeintMethod->ModeChanges, ticks * 100 / (double)(CurrentTicks - nInitialTicks), DeintMethod->szName);
                OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
                nLine++;
            }
        }
        i++;
        DeintMethod = GetVideoDeintMethod(i);
    }

    nLine++;
    if (nLine < 7)
    {
        nLine = 7;
    }

    OSD_AddText("Aspect Ratios", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    sprintf (szInfo, "Number of changes : %d", nNbRatioSwitch);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//    OSD_AddText("changes - % of time - Ratio", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    if (nNbRatioSwitch > 0)
    {
        nFirstLine = nLine;
        nCol = 1;

        CurrentTicks = GetTickCount();

        for (i = 0 ; i < MAX_RATIO_STATISTICS ; i++)
        {
            if (RatioStatistics[i].switch_count > 0)
            {
                pos = OSD_GetLineYpos (nLine, dfMargin, Size);
                if (pos == 0)
                {
                    nCol++;
                    nLine = nFirstLine;
                    if (nCol <= 2)
                    {
//                        OSD_AddText("changes - % of time - Ratio", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, (nCol == 1) ? dfMargin : 0.5, OSD_GetLineYpos (nFirstLine-1, dfMargin, Size));
                        pos = OSD_GetLineYpos (nLine, dfMargin, Size);
                    }
                }
                if (pos > 0)
                {
                    if ((RatioStatistics[i].mode == AspectSettings.AspectMode) && (RatioStatistics[i].ratio == AspectSettings.SourceAspect))
                    {
                        Color = OSD_COLOR_CURRENT;
                        ticks = RatioStatistics[i].ticks + CurrentTicks - nARLastTicks;
                    }
                    else
                    {
                        Color = -1;
                        ticks = RatioStatistics[i].ticks;
                    }
                    sprintf (szInfo, "%03d - %05.1f %% - %.3f:1 %s", RatioStatistics[i].switch_count, ticks * 100 / (double)(CurrentTicks - nARInitialTicks), RatioStatistics[i].ratio / 1000.0, RatioStatistics[i].mode == 2 ? "A" : "");
                    OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, (nCol == 1) ? dfMargin : 0.5, pos);
                    nLine++;
                }
            }
        }
    }
}


static void OSD_RefreshWSSScreen(double Size)
{
    double      dfMargin = 0.02;    // 2% of screen height/width
    char        szInfo[64];
    int         nLine;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText("WSS data decoding", Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 3;

    OSD_AddText("Status", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    sprintf (szInfo, "Errors : %d", WSS_CtrlData.NbDecodeErr);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Ok : %d", WSS_CtrlData.NbDecodeOk);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    if ((WSS_CtrlData.NbDecodeErr+WSS_CtrlData.NbDecodeOk) > 0)
    {
        sprintf (szInfo, "Last : %s", (WSS_CtrlData.DecodeStatus == WSS_STATUS_OK) ? "OK" : "ERROR");
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    if ((WSS_CtrlData.NbDecodeOk+WSS_CtrlData.NbDecodeErr) > 0)
    {

        nLine = -1;

        // Debug informations
        if (WSS_CtrlData.NbDecodeOk > 0)
        {
            sprintf (szInfo, "Start position min / max : %d / %d", WSS_CtrlData.MinPos, WSS_CtrlData.MaxPos);
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        }
        sprintf (szInfo, "Errors searching start position : %d", WSS_CtrlData.NbErrPos);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        OSD_AddText("Debug", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));

        if (WSS_CtrlData.DecodeStatus != WSS_STATUS_ERROR)
        {
            nLine = 3;

            OSD_AddText("Data", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

            // WSS data
            if (WSS_Data.AspectRatio > 0)
            {
                sprintf (szInfo, "Aspect ratio : %.3f", WSS_Data.AspectRatio / 1000.0);
                if (WSS_Data.AspectMode == 1)
                    strcat (szInfo, " Letterboxed");
                else if (WSS_Data.AspectMode == 2)
                    strcat (szInfo, " Anamorphic");
            }
            else
            {
                strcpy (szInfo, "Aspect ratio : undefined");
            }
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            sprintf (szInfo, "Mode : %s", WSS_Data.FilmMode ? "film Mode" : "camera Mode");     
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            sprintf (szInfo, "Helper signals : %s", WSS_Data.HelperSignals ? "yes" : "no");     
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            sprintf (szInfo, "Color encoding : %s", WSS_Data.ColorPlus ? "ColorPlus" : "normal");
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            sprintf (szInfo, "Teletext subtitles : %s", WSS_Data.TeletextSubtitle ? "yes" : "no");      
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            switch (WSS_Data.OpenSubtitles)
            {
            case WSS625_SUBTITLE_NO:
                strcpy (szInfo, "Open subtitles : no");
                break;
            case WSS625_SUBTITLE_INSIDE:
                strcpy (szInfo, "Open subtitles : inside picture");
                break;
            case WSS625_SUBTITLE_OUTSIDE:
                strcpy (szInfo, "Open subtitles : outside picture");
                break;
            default:
                strcpy (szInfo, "Open subtitles : ???");
                break;
            }
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            sprintf (szInfo, "Surround sound : %s", WSS_Data.SurroundSound ? "yes" : "no");     
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            sprintf (szInfo, "Copyright asserted : %s", WSS_Data.CopyrightAsserted ? "yes" : "no");     
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            sprintf (szInfo, "Copy protection : %s", WSS_Data.CopyProtection ? "yes" : "no");       
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }
    }
}


static void OSD_RefreshARScreen(double Size)
{
    double      dfMargin = 0.02;    // 2% of screen height/width
    char        szInfo[64];
    int         nLine, nCol, nFirstLine;
    int         i;
    long        Color;
    double      pos;
    DWORD CurrentTicks;
    int ticks;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText("Aspect Ratio", Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 3;

    sprintf (szInfo, "Number of changes : %d", nNbRatioSwitch);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
    OSD_AddText("changes - % of time - Ratio", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    if (nNbRatioSwitch > 0)
    {
        nFirstLine = nLine;
        nCol = 1;

        CurrentTicks = GetTickCount();

        for (i = 0 ; i < MAX_RATIO_STATISTICS ; i++)
        {
            if (RatioStatistics[i].switch_count > 0)
            {
                pos = OSD_GetLineYpos (nLine, dfMargin, Size);
                if (pos == 0)
                {
                    nCol++;
                    nLine = nFirstLine;
                    if (nCol <= 2)
                    {
                        OSD_AddText("changes - % of time - Ratio", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, (nCol == 1) ? dfMargin : 0.5, OSD_GetLineYpos (4, dfMargin, Size));
                        pos = OSD_GetLineYpos (nLine, dfMargin, Size);
                    }
                }
                if (pos > 0)
                {
                    if ((RatioStatistics[i].mode == AspectSettings.AspectMode) && (RatioStatistics[i].ratio == AspectSettings.SourceAspect))
                    {
                        Color = OSD_COLOR_CURRENT;
                        ticks = RatioStatistics[i].ticks + CurrentTicks - nARLastTicks;
                    }
                    else
                    {
                        Color = -1;
                        ticks = RatioStatistics[i].ticks;
                    }
                    sprintf (szInfo, "%03d - %05.1f %% - %.3f:1 %s", RatioStatistics[i].switch_count, ticks * 100 / (double)(CurrentTicks - nARInitialTicks), RatioStatistics[i].ratio / 1000.0, RatioStatistics[i].mode == 2 ? "A" : "");
                    OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, (nCol == 1) ? dfMargin : 0.5, pos);
                    nLine++;
                }
            }
        }
    }
}


static void OSD_RefreshCalibrationScreen(double Size)
{
    double          dfMargin = 0.02;    // 2% of screen height/width
    char            szInfo[64];
    int             nLine;
    long            Color, BackColor;
    unsigned char   val1, val2, val3, val4, val5, val6;
    int             dif_val1, dif_val2, dif_val3;
    int             dif_total1, dif_total2;
    char            szResult[16];
    CTestPattern*   pTestPattern;
    CSubPattern*    pSubPattern;
    CSource*        pSource = Providers_GetCurrentSource();
    ISetting*       pSetting = NULL;
    BOOL            avail1, avail2;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText("Card calibration", Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    // Video settings
    if (pSource != NULL)
    {
        if ( (pCalibration->GetType() == CAL_MANUAL)
          || (pCalibration->GetType() == CAL_CHECK_YUV_RANGE) )
        {
            pSetting = pSource->GetBrightness();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Brightness : %+04d", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (1, dfMargin, Size));
            }
            pSetting = pSource->GetContrast();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Contrast : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (2, dfMargin, Size));
            }
            pSetting = pSource->GetHue();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Hue : %+04d", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
            }
            pSetting = pSource->GetSaturation();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (1, dfMargin, Size));
            }
            pSetting = pSource->GetSaturationU();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color U : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (2, dfMargin, Size));
            }
            pSetting = pSource->GetSaturationV();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color V : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
            }
        }
        else
        {
			nLine = 8;

            // do brightness
            pSetting = pSource->GetBrightness();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Brightness : %+04d", pSetting->GetValue());
                if ( pCalibration->IsRunning()
                  && ( (pCalibration->GetCurrentStep() == 1)
                    || (pCalibration->GetCurrentStep() == 2)
                    || (pCalibration->GetCurrentStep() == 3)
                    || (pCalibration->GetCurrentStep() == 4)
                    || (pCalibration->GetCurrentStep() == 9)
                    || (pCalibration->GetCurrentStep() == 10)
                    || (pCalibration->GetCurrentStep() == 11) ) )
                {
                    Color = OSD_COLOR_CURRENT;
                }
                else
                {
                    Color = -1;
                }
	            OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            }


            // do Contrast
            pSetting = pSource->GetContrast();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Contrast : %03u", pSetting->GetValue());
                if ( pCalibration->IsRunning()
                  && ( (pCalibration->GetCurrentStep() == 5)
                    || (pCalibration->GetCurrentStep() == 6)
                    || (pCalibration->GetCurrentStep() == 7)
                    || (pCalibration->GetCurrentStep() == 8)
                    || (pCalibration->GetCurrentStep() == 9)
                    || (pCalibration->GetCurrentStep() == 10)
                    || (pCalibration->GetCurrentStep() == 11) ) )
                {
                    Color = OSD_COLOR_CURRENT;
                }
                else
                {
                    Color = -1;
                }
	            OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            }


            // do Color
            pSetting = pSource->GetSaturation();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color : %03u", pSetting->GetValue());
                if ( pCalibration->IsRunning()
                  && ( (pCalibration->GetCurrentStep() == 12)
                    || (pCalibration->GetCurrentStep() == 13)
                    || (pCalibration->GetCurrentStep() == 14)
                    || (pCalibration->GetCurrentStep() == 15)
                    || (pCalibration->GetCurrentStep() == 16)
                    || (pCalibration->GetCurrentStep() == 17)
                    || (pCalibration->GetCurrentStep() == 18)
                    || (pCalibration->GetCurrentStep() == 19)
                    || (pCalibration->GetCurrentStep() == 22)
                    || (pCalibration->GetCurrentStep() == 23) ) )
                {
                    Color = OSD_COLOR_CURRENT;
                }
                else
                {
                    Color = -1;
                }
	            OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            }


            // do Color U
            pSetting = pSource->GetSaturationU();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color U : %03u", pSetting->GetValue());
                if ( pCalibration->IsRunning()
                  && ( (pCalibration->GetCurrentStep() == 12)
                    || (pCalibration->GetCurrentStep() == 13)
                    || (pCalibration->GetCurrentStep() == 14)
                    || (pCalibration->GetCurrentStep() == 15)
                    || (pCalibration->GetCurrentStep() == 22)
                    || (pCalibration->GetCurrentStep() == 23) ) )
                {
                    Color = OSD_COLOR_CURRENT;
                }
                else
                {
                    Color = -1;
                }
	            OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            }


            // do Color V
            pSetting = pSource->GetSaturationV();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color V : %03u", pSetting->GetValue());
                if ( pCalibration->IsRunning()
                  && ( (pCalibration->GetCurrentStep() == 16)
                    || (pCalibration->GetCurrentStep() == 17)
                    || (pCalibration->GetCurrentStep() == 18)
                    || (pCalibration->GetCurrentStep() == 19)
                    || (pCalibration->GetCurrentStep() == 22)
                    || (pCalibration->GetCurrentStep() == 23) ) )
                {
                    Color = OSD_COLOR_CURRENT;
                }
                else
                {
                    Color = -1;
                }
	            OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            }

            // do Hue
            pSetting = pSource->GetHue();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Hue : %+04d", pSetting->GetValue());
                if ( pCalibration->IsRunning()
                  && ( (pCalibration->GetCurrentStep() == 20)
                    || (pCalibration->GetCurrentStep() == 21)
                    || (pCalibration->GetCurrentStep() == 22)
                    || (pCalibration->GetCurrentStep() == 23) ) )
                {
                    Color = OSD_COLOR_CURRENT;
                }
                else
                {
                    Color = -1;
                }
	            OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            }
        }
    }

	// Name of the test pattern
	pTestPattern = pCalibration->GetCurrentTestPattern();
    if (pTestPattern != NULL)
	{
        OSD_AddText(pTestPattern->GetName(), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (3, dfMargin, Size));
	}

    if (pCalibration->IsRunning() && (pTestPattern != NULL))
	{
        if ( (pCalibration->GetType() != CAL_MANUAL)
          && (pCalibration->GetType() != CAL_CHECK_YUV_RANGE) )
        {
            switch (pCalibration->GetCurrentStep())
            {
            case -1:
                strcpy(szInfo, "Calibration finished");
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                strcpy(szInfo, "Adjusting Brightness ...");
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                strcpy(szInfo, "Adjusting Contrast ...");
                break;
            case 10:
            case 11:
                strcpy(szInfo, "Fine tuning of brightness and contrast ...");
                break;
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                strcpy(szInfo, "Adjusting Color ...");
                break;
              case 20:
            case 21:
                strcpy(szInfo, "Adjusting Hue ...");
                break;
            case 22:
            case 23:
                strcpy(szInfo, "Fine tuning of color ...");
                break;
            default:
                strcpy(szInfo, "");
                break;
            }
            OSD_AddText(szInfo, Size, OSD_COLOR_CURRENT, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (5, dfMargin, Size));
        }

        if ( (pCalibration->GetType() == CAL_MANUAL)
          || (pCalibration->GetType() == CAL_CHECK_YUV_RANGE)
          || (pCalibration->GetCurrentStep() == -1) )
        {
            nLine = 4;

            if ( Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                OSD_AddText("Delta RGB", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
            }
            if ( Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA))
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                OSD_AddText("Delta YUV", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
            }
            nLine++;

        	pSubPattern = pCalibration->GetCurrentSubPattern();
            if (pSubPattern != NULL)
            {
                for(vector<CColorBar*>::iterator it = pSubPattern->m_ColorBars.begin(); 
                    it != pSubPattern->m_ColorBars.end(); 
                    ++it)
    	    	{
                    if (pCalibration->GetType() == CAL_CHECK_YUV_RANGE)
                    {
                        if ( (*it)->GetMinColor(&val1, &val2, &val3)
                          && (*it)->GetMaxColor(&val4, &val5, &val6) )
                        {
                            sprintf (szInfo, "Y Range [%u,%u]", val1, val4);
                            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            sprintf (szInfo, "U Range [%u,%u]", val2, val5);
                            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            sprintf (szInfo, "V Range [%u,%u]", val3, val6);
                            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                        }
                    }
                    else
                    {
                        (*it)->GetRefColor(FALSE, &val1, &val2, &val3);
    	    		    BackColor = RGB(val1, val2, val3);

                        avail1 = (*it)->GetDeltaColor(FALSE, &dif_val1, &dif_val2, &dif_val3, &dif_total1);
                        if ( avail1
                          && Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
                          && (pCalibration->GetType() == CAL_MANUAL) )
                        {
                            sprintf (szInfo, "(%+d,%+d,%+d) (%d)", dif_val1, dif_val2, dif_val3, dif_total1);
                            OSD_AddText(szInfo, Size, -1, BackColor, OSDB_SHADED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                        }
                        avail2 = (*it)->GetDeltaColor(TRUE, &dif_val1, &dif_val2, &dif_val3, &dif_total2);
                        if ( avail2
                          && Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA))
                          && (pCalibration->GetType() == CAL_MANUAL) )
                        {
                            sprintf (szInfo, "(%d) (%+d,%+d,%+d)", dif_total2, dif_val1, dif_val2, dif_val3);
                            OSD_AddText(szInfo, Size, -1, BackColor, OSDB_SHADED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                        }

                        if (pCalibration->GetType() == CAL_MANUAL)
                        {
                            if ( avail1
                              && Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
                              && ! Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA)) )
                            {
                                OSD_GetTextResult(dif_total1, szInfo, &Color);
        		    		    OSD_AddText(szInfo, Size, Color, BackColor, OSDB_SHADED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            }
                            else if ( avail2
                                   && ! Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
                                   && Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA)) )
                            {
                                OSD_GetTextResult(dif_total2, szInfo, &Color);
        		    		    OSD_AddText(szInfo, Size, Color, BackColor, OSDB_SHADED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            }
                            else if (avail1 && avail2)
                            {
                                OSD_GetTextResult((dif_total1 < dif_total2) ? dif_total1 : dif_total2, szInfo, &Color);
        		    		    OSD_AddText(szInfo, Size, Color, BackColor, OSDB_SHADED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            }
                        }
                        else if (avail2)
                        {
                            OSD_GetTextResult(dif_total2, szResult, &Color);
                            sprintf (szInfo, "%s (YUV %d)", szResult, dif_total2);
		    		        OSD_AddText(szInfo, Size, Color, BackColor, OSDB_SHADED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                        }
                    }
    	    	}
            }
        }
	}
}


static void OSD_RefreshDeveloperScreen(double Size)
{
    double      dfMargin = 0.02;    // 2% of screen height/width
    char        szInfo[64];
    int         nLine;
    int         i;
    double      pos;
	DWORD		Total = 0;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText("Information for developers", Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 3;

    OSD_AddText("Dropped fields", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    sprintf (szInfo, "Number : %ld", pPerf->GetNumberDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Last second : %d", pPerf->GetDroppedFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    sprintf (szInfo, "Average / s : %.1f", pPerf->GetAverageDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    nLine = 3;

    OSD_AddText("Average Time per cycle (1/10 ms)", Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    for (i = 0 ; i < PERF_TYPE_LASTONE ; ++i)
    {
        if (pPerf->IsValid((ePerfType)i))
        {
            pos = OSD_GetLineYpos (nLine, dfMargin, Size);
            sprintf(szInfo, "%s :", pPerf->GetName((ePerfType)i));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
            sprintf(szInfo, "%d", pPerf->GetAverageDuration((ePerfType)i));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.45, pos);
            nLine++;
			Total += pPerf->GetAverageDuration((ePerfType)i);
        }
    }
    pos = OSD_GetLineYpos (nLine, dfMargin, Size);
    sprintf(szInfo, "%s", "Total");
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
    sprintf(szInfo, "%d", Total);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.45, pos);
}


static void OSD_DisplayProgrammeInfos(double Size)
{
    double	dfMargin = 0.02;    // 2% of screen height/width

	time_t StartTime;
	time_t EndTime;
	string ChannelName;
	string ProgrammeTitle;
	string Category;

	if (MyEPG.GetProgrammeMainData(-1, &StartTime, &EndTime, ChannelName, ProgrammeTitle, Category) == FALSE)
	{
		return;
	}

	OSD_AddText(ChannelName.c_str(), OSD_DefaultSizePerc, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (1, dfMargin, OSD_DefaultSizePerc));

    if (Size == 0)
    {
        Size = Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE));
    }

	struct tm *date_tm;
	char   StartTimeStr[6];
	char   EndTimeStr[6];
	date_tm = localtime(&StartTime);
	sprintf(StartTimeStr, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);
	date_tm = localtime(&EndTime);
	sprintf(EndTimeStr, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);

    double pos2 = OSD_GetLineYpos (2, dfMargin, OSD_DefaultSizePerc);
    double pos3 = pos2 + OSD_GetLineYpos (2, dfMargin, Size) - OSD_GetLineYpos (1, dfMargin, Size);
	double pos4 = pos3 + pos3 - pos2;

    OSD_AddText(ProgrammeTitle.c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos2);
	char   szInfo[16];
    sprintf(szInfo, "%s - %s", StartTimeStr, EndTimeStr);
    OSD_AddText(szInfo, Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos3);
	time_t TimeNow;
	time(&TimeNow);
    sprintf(szInfo, "%.1f %%", (double)(TimeNow - StartTime) * 100.0 / (double)(EndTime - StartTime));
    OSD_AddText(szInfo, Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos4);
}


static void OSD_RefreshCurrentProgrammeScreen(double Size)
{
    double	dfMargin = 0.02;    // 2% of screen height/width

    if (Size == 0)
    {
        Size = Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE));
    }

	int nLine = 1;

    double pos1 = OSD_GetLineYpos (nLine++, dfMargin, Size);
    double pos2 = OSD_GetLineYpos (nLine++, dfMargin, Size);

	string Channel;

	if (   (MyEPG.SearchForProgramme(Channel) == FALSE)
		|| (Channel.length() == 0) )
	{
		OSD_AddText("No EPG information", Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos1);
		return;
	}

	time_t StartTime;
	time_t EndTime;
	string ChannelName;
	string ProgrammeTitle;
	string SubTitle;
	string Category;
	string Description;

	MyEPG.GetProgrammeData(-1, &StartTime, &EndTime, ChannelName, ProgrammeTitle, SubTitle, Category, Description);

	struct tm *date_tm;
	char   StartTimeStr[6];
	char   EndTimeStr[6];
	date_tm = localtime(&StartTime);
	sprintf(StartTimeStr, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);
	date_tm = localtime(&EndTime);
	sprintf(EndTimeStr, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);

    OSD_AddText(ChannelName.c_str(), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos1);
	char   szInfo[16];
	time_t TimeNow;
	time(&TimeNow);
	if (   (TimeNow >= StartTime)
		&& (TimeNow < EndTime) )
	{
		sprintf(szInfo, "%.1f %%", (double)(TimeNow - StartTime) * 100.0 / (double)(EndTime - StartTime));
		OSD_AddText(szInfo, Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, pos1);
	}
    sprintf(szInfo, "%s - %s", StartTimeStr, EndTimeStr);
    OSD_AddText(szInfo, Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos1);
    OSD_AddText(ProgrammeTitle.c_str(), Size, OSD_COLOR_CURRENT, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, pos2);
	if (SubTitle.length() > 0)
	{
		pos2 = OSD_GetLineYpos (nLine++, dfMargin, Size);
		OSD_AddText(SubTitle.c_str(), Size, OSD_COLOR_CURRENT, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, pos2);
	}
	if (Description.length() > 0)
	{
		// Get the Y position of the first line of the description
		pos2 = OSD_GetLineYpos (nLine++, dfMargin, Size);

		// Determine how many lines can still enter in the OSD
		double LineHeight = (Size-1)/100;
		int nb = 0;
		pos1 = pos2;
		while ( (pos1+LineHeight) <= 1.0)
		{
            pos1 += LineHeight;
			nb++;
		} 

		// Cut the description test on several lines and
		// determine the number of necessary lines
		int nb2 = OSD_CutLines(Description.c_str(), Setting_GetValue(EPG_GetSetting(EPG_MAXCHARSPERLINE)));

		// Display the lines skipping first lines if required
		int IdxFirstLine;
		int nb3;
		if (nb2 <= nb)
		{
			IdxFirstLine = 0;
			nb3 = nb2;
		}
		else
		{
			IdxFirstLine = MyEPG.GetDisplayLineShift(nb2 - nb);
			nb3 = nb + IdxFirstLine;
		}
		for(int i = IdxFirstLine; i < nb3; i++)
		{
			OSD_AddTextSingleLine(TextLines[i], Size-1 ,-1 ,-1 , OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos2);
			pos2 += LineHeight;
		}
	}
}


static void OSD_RefreshProgrammesScreen(double Size)
{
    double	dfMargin = 0.02;    // 2% of screen height/width

	time_t	TimeMin;
	time_t	TimeMax;
	int nb = MyEPG.GetSearchContext(NULL, &TimeMin, &TimeMax);

    if (Size == 0)
    {
        Size = Setting_GetValue(EPG_GetSetting(EPG_PERCENTAGESIZE));
    }

    // Title
	double pos1 = OSD_GetLineYpos (1, dfMargin, Size*1.5);
	char   szInfo[64];
	struct tm *Time_tm = localtime(&TimeMin);
    sprintf(szInfo, "%02u/%02u", Time_tm->tm_mday, Time_tm->tm_mon+1);
    OSD_AddText(szInfo, Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos1);
    sprintf(szInfo, "%02u:%02u/%02u:%02u", Time_tm->tm_hour, Time_tm->tm_min);
	Time_tm = localtime(&TimeMax);
    sprintf(&szInfo[5], " / %02u:%02u", Time_tm->tm_hour, Time_tm->tm_min);
    OSD_AddText(szInfo, Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, pos1);

	if (nb == 0)
	{
		return;
	}

	time_t TimeNow;
	time(&TimeNow);

	string Channel;
	MyEPG.GetViewedChannelName(Channel);

	int IdxMin, IdxMax, IdxCur;
	MyEPG.GetDisplayIndexes(&IdxMin, &IdxMax, &IdxCur);

	BOOL NeedAdjust = ( (IdxCur != -1) && (IdxMin == -1) && (IdxMax == -1) ) ? TRUE : FALSE;

	if (IdxMax == -1)
	{
		IdxMax = nb;
	}

	int i;
	int nLine = 3;
	double pos2;
	double pos3;
	if (IdxMin == -1)
	{
		nLine++;
		for (i=IdxMax; i>=1; i--)
		{
			pos2 = OSD_GetLineYpos (nLine, dfMargin, Size);
			nLine += 2;
			if (pos2 == 0.0)
			{
				IdxMin = i+1;
				if (NeedAdjust == TRUE)
				{
					int NbPerPage = IdxMax - IdxMin + 1;
					int IdxPage = (IdxCur - 1) / NbPerPage;
					IdxMin = (IdxPage * NbPerPage) + 1;
					IdxMax = (IdxPage + 1) * NbPerPage;
					if (IdxMax > nb)
					{
						IdxMax = nb;
					}
				}
				break;
			}
		}
		if (IdxMin == -1)
		{
			IdxMin = 1;
		}
		nLine = 3;
	}

	for (i=IdxMin-1; i<IdxMax; i++)
	{
		long   Color;
		time_t StartTime;
		time_t EndTime;
		char StartTimeStr[6];
		char EndTimeStr[6];
		string ChannelName;
		string ProgrammeTitle;
		string Category;

		MyEPG.GetProgrammeMainData(i, &StartTime, &EndTime, ChannelName, ProgrammeTitle, Category);
		struct tm *date_tm;
		date_tm = localtime(&StartTime);
		sprintf(StartTimeStr, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);
		date_tm = localtime(&EndTime);
		sprintf(EndTimeStr, "%02u:%02u", date_tm->tm_hour, date_tm->tm_min);

		if (   (Channel.length() > 0) 
			&& !_stricmp(ChannelName.c_str(), Channel.c_str())
			&& (TimeNow >= StartTime)
			&& (TimeNow < EndTime) )
		{
			Color = OSD_COLOR_SECTION;
		}
		else
		{
			Color = -1;
		}
		pos2 = OSD_GetLineYpos (nLine++, dfMargin, Size);
		pos3 = OSD_GetLineYpos (nLine++, dfMargin, Size);
		if (pos3 == 0.0)
		{
			IdxMax = i;
			break;
		}
        OSD_AddText(ChannelName.c_str(), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos2);
		if (Category.length() > 0)
		{
			OSD_AddText(Category.c_str(), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, pos2);
		}
        sprintf(szInfo, "%s - %s", StartTimeStr, EndTimeStr);
        OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos2);
        OSD_AddText(ProgrammeTitle.c_str(), Size, (i == (IdxCur -1)) ? OSD_COLOR_CURRENT : -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos3);
		if (   (TimeNow >= StartTime)
			&& (TimeNow < EndTime) )
		{
			sprintf(szInfo, "%.1f %%", (double)(TimeNow - StartTime) * 100.0 / (double)(EndTime - StartTime));
			OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos3);
		}
	}

	MyEPG.SetDisplayIndexes(IdxMin, IdxMax, IdxCur);

    sprintf(szInfo, "%u-%u / %u", IdxMin, IdxMax, nb);
    OSD_AddText(szInfo, Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos1);
}


/////////////////////////////////////////////////////////////////////////////
// Start of OSD data cache manipulators (Internal)
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// Internal: Add a new single line of text to the list of texts for OSD
void OSD_AddTextSingleLine(LPCTSTR szText, double Size, long NewTextColor, long BackgroundColor, 
                           eOSDBackground BackgroundMode, eOSDTextXPos TextXPos, double XPos, 
                           double YPos)
{
    if ( (strlen(szText) == 0) || (OSD_nTextCount >= OSD_MAX_TEXT) )
    {
        return;
    }

	// Don't display lines that are outside or partially outside
	// the screen vertically
	double YPos2;
    if(Size == 0)
    {
        YPos2 = YPos + (double)OSD_DefaultSizePerc/100;
    }
    else
    {
        YPos2 = YPos + Size/100;
    }
	if (YPos < 0.0 || YPos2 > 1.0)
    {
        return;
    }

    if (Size == 0)
    {
        OSD_Text[OSD_nTextCount].Size = OSD_DefaultSizePerc;
    }
    else
    {
        OSD_Text[OSD_nTextCount].Size = Size;
    }
    if (NewTextColor == -1)
    {
        OSD_Text[OSD_nTextCount].TextColor = OSD_TextColor;
    }
    else
    {
        OSD_Text[OSD_nTextCount].TextColor = NewTextColor;
    }
    if (BackgroundColor == -1)
    {
        OSD_Text[OSD_nTextCount].BackgroundColor = OSD_OutlineColor;
    }
    else
    {
        OSD_Text[OSD_nTextCount].BackgroundColor = BackgroundColor;
    }
    if (BackgroundMode == OSDB_USERDEFINED)
    {
        OSD_Text[OSD_nTextCount].BackgroundMode = OSD_Background;
    }
    else
    {
        OSD_Text[OSD_nTextCount].BackgroundMode = BackgroundMode;
    }
    OSD_Text[OSD_nTextCount].TextXPos = TextXPos;
    OSD_Text[OSD_nTextCount].XPos = XPos;
    OSD_Text[OSD_nTextCount].YPos = YPos;
    strncpy(OSD_Text[OSD_nTextCount].szText, szText, sizeof(OSD_Text[OSD_nTextCount].szText));

    OSD_nTextCount++;
}


//---------------------------------------------------------------------------
// Internal: Add a new line of text to the list of texts for OSD.
// Internal: "\n" and "\r\n" are supported for starting a new line.
void OSD_AddText(LPCTSTR szText, double Size, long NewTextColor, long BackgroundColor, 
                 eOSDBackground BackgroundMode, eOSDTextXPos TextXPos, double XPos, double YPos)
{
    char      SingleLine[512];
    int       SingleLineIndex = 0;
    char      *s;

    // convert "\r\n" to "\n"
    while(s = strstr(szText, "\r\n"))
    {
        strcpy(&s[0], &s[1]);
    }

    for(int i = 0; i < strlen(szText); i++)
    {
        if(SingleLineIndex == 512)
        {
            SingleLineIndex--;
            break;
        }
		if( szText[i] != '\n' )
        {
            SingleLine[SingleLineIndex++] = szText[i];
        }
        else
        {
            SingleLine[SingleLineIndex] = 0x00;
            OSD_AddTextSingleLine(SingleLine, Size, NewTextColor, BackgroundColor, BackgroundMode,
                                  TextXPos, XPos, YPos);
            SingleLineIndex = 0;
            if(Size == 0)
            {
                YPos += (double)OSD_DefaultSizePerc/100;
            }
            else
            {
                YPos += Size/100;
            }
        }
    }
    SingleLine[SingleLineIndex] = 0x00;
    OSD_AddTextSingleLine(SingleLine,Size,NewTextColor,BackgroundColor,BackgroundMode,TextXPos,
                          XPos,YPos);
}


//---------------------------------------------------------------------------
// Same as OSD_AddText except that the text is cut on several lines
// as soon as one line has more than a certain number of characters.
int OSD_CutLines(LPCTSTR szText, long MaxCharsPerLine)
{
	int       IndexLine = 0;
    int       InLineIndex = 0;
    char      *s;
	int       i;

    // convert "\r\n" to "\n"
    while(s = strstr(szText, "\r\n"))
    {
        strcpy(&s[0], &s[1]);
    }

    for(i = 0; i < strlen(szText); i++)
    {
        if(InLineIndex == 512)
        {
            InLineIndex--;
            break;
        }
		if( (szText[i] != '\n') && (InLineIndex < MaxCharsPerLine))
        {
            TextLines[IndexLine][InLineIndex++] = szText[i];
        }
        else
        {
            TextLines[IndexLine][InLineIndex] = 0x00;

			// If the current character is a cariage return or a space,
			// then go to the next line and skip the current character
			if ((szText[i] == '\n') || (szText[i] == ' '))
			{
				IndexLine++;
				InLineIndex = 0;
				if(IndexLine == 128)
				{
					break;
				}
			}
			else
			{
				// The current character is neither a cariage return nor a space

				// If the last buffered character is a space,
				// then go to the next line and keep the current character
				// as first first character of the next line
				if (TextLines[IndexLine][InLineIndex-1] == ' ')
				{
					IndexLine++;
					InLineIndex = 0;
				}
				else
				{
					// Search the last space in the buffered line
					s = strrchr(TextLines[IndexLine], ' ');
					if (s)
					{
						*s = 0x00;
						IndexLine++;
						if (IndexLine == 128)
						{
							InLineIndex = 0;
							break;
						}
				        strcpy(&TextLines[IndexLine][0], s+1);
						InLineIndex = strlen(TextLines[IndexLine]);
					}
					else
					{
						IndexLine++;
						InLineIndex = 0;
					}
				}

				if (IndexLine == 128)
				{
					break;
				}

				// Add the current character to the line buffer
				TextLines[IndexLine][InLineIndex++] = szText[i];
			}
        }
    }
    if ( (IndexLine < 128) && (InLineIndex > 0) )
    {
		TextLines[IndexLine][InLineIndex] = 0x00;
		IndexLine++;
    }

	return IndexLine;
}


//---------------------------------------------------------------------------
// Internal: Clean the list of texts for OSD
void OSD_ClearAllTexts()
{
    OSD_nTextCount = 0;
}


/////////////////////////////////////////////////////////////////////////////
// Start of Menus related code
/////////////////////////////////////////////////////////////////////////////

void OSD_UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuOSD;
    int             i;
    int             NbScreens;

    hMenuOSD = GetOSDSubmenu();
    if (hMenuOSD == NULL)
    {
        return;
    }

    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    for (i=0; i < NbScreens ; i++)
    {
        if (*(ActiveScreens[i].name) != '\0' && !ActiveScreens[i].managed_by_app)
        {
            UINT Flags(MF_STRING);
            AppendMenu(hMenuOSD, Flags, IDM_OSDSCREEN_SHOW + i, ActiveScreens[i].name);
        }
    }
}


void OSD_SetMenu(HMENU hMenu)
{
    HMENU   hMenuOSD;
    int     i;
    int     NbScreens;

    hMenuOSD = GetOSDSubmenu();
    if (hMenuOSD == NULL)
    {
        return;
    }

    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    for (i=0 ; i < NbScreens ; i++)
    {
        if ((strlen (ActiveScreens[i].name) > 0) && !ActiveScreens[i].managed_by_app)
        {
//            CheckMenuItemBool(hMenuOSD, IDM_OSDSCREEN_SHOW + i, ActiveScreens[i].active);
            EnableMenuItem(hMenuOSD, IDM_OSDSCREEN_SHOW + i, ActiveScreens[i].active ? MF_ENABLED : MF_GRAYED);
        }
    }
}


BOOL ProcessOSDSelection(HWND hWnd, WORD wMenuID)
{
    if (wMenuID >= IDM_OSDSCREEN_SHOW && wMenuID < (IDM_OSDSCREEN_SHOW + 10))
    {
        if (!pCalibration->IsRunning())
        {
            OSD_ShowInfosScreen(wMenuID - IDM_OSDSCREEN_SHOW, 0);
        }

        // OSD_ActivateInfosScreen(hWnd, wMenuID - IDM_OSDSCREEN_SHOW, 0);

        return TRUE;
    }
    else if (wMenuID == IDM_SHOW_OSD)
    {
        if (pCalibration->IsRunning())
        {
            OSD_ShowInfosScreen(0, 0);
        }
        else
        {
            OSD_ShowNextInfosScreen(0);
        }
        return TRUE;
    }
    else if (wMenuID == IDM_HIDE_OSD)
    {
        OSD_Clear();
        return TRUE;
    }
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

BOOL OSD_AutoHide_OnChange(long NewValue)
{
    int NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);

    OSD_bRestoreScreen = FALSE;
    OSD_bAutoHide = NewValue;

    for(int i = 0; i < NbScreens; i++)
    {
        if (!ActiveScreens[i].managed_by_app)
        {
            ActiveScreens[i].auto_hide = NewValue;
        }
    }

    return TRUE;
}

SETTING OSDSettings[OSD_SETTING_LASTONE] =
{
    {
        "Outline Color", SLIDER, 0, (long*)&OSD_OutlineColor,
         RGB(0,0,0), 0, RGB(255,255,255), 1, 1,
         NULL,
        "OSD", "OutlineColor", NULL,
    },
    {
        "Text Color", SLIDER, 0, (long*)&OSD_TextColor,
         RGB(0,255,0), 0, RGB(255,255,255), 1, 1,
         NULL,
        "OSD", "TextColor", NULL,
    },
    {
        "Default Size", SLIDER, 0, (long*)&OSD_DefaultSizePerc,
         10, 2, 15, 1, 1,
         NULL,
        "OSD", "DefaultSizePerc", NULL,
    },
    {
        "Default Small Size", SLIDER, 0, (long*)&OSD_DefaultSmallSizePerc,
         5, 2, 7, 1, 1,
         NULL,
        "OSD", "DefaultSmallSizePerc", NULL,
    },
    {
        "Anti Alias", ONOFF, 0, (long*)&OSD_bAntiAlias,
         FALSE, 0, 1, 1, 1,
         NULL,
        "OSD", "AntiAlias", NULL,
    },
    {
        "Background Type", ITEMFROMLIST, 0, (long*)&OSD_Background,
         OSDB_TRANSPARENT, 0, OSDB_LASTONE - 1, 1, 1,
         OSD_szBackgroundNames,
        "OSD", "Background", NULL,
    },
    {
        "Outline Text", ONOFF, 0, (long*)&OSD_bOutline,
         TRUE, 0,  1, 1, 1,
         NULL,
        "OSD", "Outline", NULL,
    },
    {
        "Auto Hide Screens", ONOFF, 0, (long*)&OSD_bAutoHide,
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "AutoHide", OSD_AutoHide_OnChange,
    },
    {
        "Use General Screen", ONOFF, 0, (long*)&(ActiveScreens[2].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseGeneralScreen", NULL,
    },
    {
        "Use Statistics Screen", ONOFF, 0, (long*)&(ActiveScreens[5].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseStatisticsScreen", NULL,
    },
    {
        "Use WSS Decoding Screen", ONOFF, 0, (long*)&(ActiveScreens[6].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseWSSDecodingScreen", NULL,
    },
    {
        "Use Developer Screen", ONOFF, 0, (long*)&(ActiveScreens[7].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseDeveloperScreen", NULL,
    },
    {
        "Font Name", CHARSTRING, 0, (long*)&OSD_szFontName,
         (long)"Arial", 0, 0, 0, 0,
         NULL,
        "OSD", "FontName", NULL,
    },
};


SETTING* OSD_GetSetting(OSD_SETTING Setting)
{
    if(Setting > -1 && Setting < OSD_SETTING_LASTONE)
    {
        return &(OSDSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void OSD_ReadSettingsFromIni()
{
    for(int i = 0; i < OSD_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(OSDSettings[i]));
    }
    OSD_AutoHide_OnChange(OSD_bAutoHide);
}

void OSD_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    for(int i = 0; i < OSD_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(OSDSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* OSD_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("OSD Settings",OSDSettings, OSD_SETTING_LASTONE);
}

void OSD_FreeSettings()
{
    int i;
    for(i = 0; i < OSD_SETTING_LASTONE; i++)
    {
        Setting_Free(&OSDSettings[i]);
    }
}
