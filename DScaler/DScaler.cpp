////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.cpp,v 1.131 2002-02-18 20:51:51 laurentg Exp $
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
// 21 Dec 2000   John Adcock           Stopped Timer after ini write
//
// 26 Dec 2000   Eric Schmidt          Fixed remember-last-audio-input-at-start.
//
// 02 Jan 2001   John Adcock           Added pVBILines
//                                     Removed bTV plug-in
//                                     Added Scaled BOB method
//
// 03 Jan 2001   Michael Eskin         Added MSP muting
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing
//                                     Changed display and handling of
//                                     change deinterlacing method
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 20 Feb 2001   Michael Samblanet     Added bounce timer - see AspectRatio.c
//                                     Corrected bug in SaveWindowPos - length 
//                                     not set in placement structure
//
// 23 Feb 2001   Michael Samblanet     Added orbit timer, Expierementaly removed
//                                     2 lines from WM_PAINT which should not be
//                                     needed and may have caused flashing.
//
// 31 Mar 2001   Laurent Garnier       Single click replaced by double click
//
// 04 Apr 2001   Laurent Garnier       Automatic hide cursor
//
// 26 May 2001   Eric Schmidt          Added Custom Channel Order.
//
// 24 Jul 2001   Eric Schmidt          Added TimeShift stuff.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.130  2002/02/17 21:41:03  laurentg
// Action "Find and Lock Film mode" added
//
// Revision 1.129  2002/02/17 20:32:34  laurentg
// Audio input display suppressed from the OSD main screen
// GetStatus modified to display the video input name in OSD main screen even when there is no signal
//
// Revision 1.128  2002/02/11 21:28:19  laurentg
// Popup menu updated
//
// Revision 1.127  2002/02/10 21:38:04  laurentg
// Default value for "Autohide Cursor" is now ON
//
// Revision 1.126  2002/02/09 15:30:19  laurentg
// Card calibration menus revisited.
//
// Revision 1.125  2002/02/09 13:08:41  laurentg
// New menu items to access to UI for calibration and OSD settings
//
// Revision 1.124  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.123  2002/02/08 08:14:21  adcockj
// Select saved channel on startup if in tuner mode
//
// Revision 1.122  2002/02/07 13:04:54  temperton
// Added Spanish and Polish teletext code pages. Thanks to Jazz (stawiarz).
//
// Revision 1.121  2002/02/03 22:48:21  robmuller
// Added command line parameters /driverinstall and /driveruninstall.
//
// Revision 1.120  2002/02/03 10:31:22  tobbej
// fixed so its posibel to open popup menu from keyboard
//
// Revision 1.119  2002/02/02 01:31:18  laurentg
// Access to the files of the playlist added in the menus
// Save Playlist added
// "Video Adjustments ..." restored in the popup menu
//
// Revision 1.118  2002/01/31 18:07:15  robmuller
// Fixed crash when using command line parameter /c.
//
// Revision 1.117  2002/01/31 13:02:46  robmuller
// Improved accuracy and reliability of the performance statistics.
//
// Revision 1.116  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.115  2002/01/22 14:50:10  robmuller
// Added keyboard lock option.
//
// Revision 1.114  2002/01/20 10:05:02  robmuller
// On channel setup prevent switch to tuner mode if already in tuner mode.
//
// Revision 1.113  2002/01/20 09:59:32  robmuller
// In tuner mode STATUS_TEXT in statusbar shows channel number if channel name is not available.
//
// Revision 1.112  2002/01/19 12:53:00  temperton
// Teletext pages updates at correct time.
// Teletext can use variable-width font.
//
// Revision 1.111  2002/01/16 19:02:17  adcockj
// Fixed window style and context menu fullscreen check
//
// Revision 1.110  2002/01/15 19:53:36  adcockj
// Fix for window creep with toolbar at top or left
//
// Revision 1.109  2002/01/15 11:16:03  temperton
// New teletext drawing code.
//
// Revision 1.108  2002/01/12 16:56:21  adcockj
// Series of fixes to bring 4.0.0 into line with 3.1.1
//
// Revision 1.107  2001/12/22 13:18:04  adcockj
// Tuner bugfixes
//
// Revision 1.106  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.105  2001/12/16 18:40:28  laurentg
// Reset statistics
//
// Revision 1.104  2001/12/16 16:31:43  adcockj
// Bug fixes
//
// Revision 1.103  2001/12/16 13:13:34  laurentg
// New statistics
//
// Revision 1.102  2001/12/08 14:22:19  laurentg
// Bug fix regarding Sources submenu in the right mouse menu
//
// Revision 1.101  2001/12/08 13:43:20  adcockj
// Fixed logging and memory leak bugs
//
// Revision 1.100  2001/12/03 19:33:59  adcockj
// Bug fixes for settings and memory
//
// Revision 1.99  2001/12/03 17:14:42  adcockj
// Added command line patch from Arie van Wijngaarden
//
// Revision 1.98  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.97  2001/11/29 14:04:06  adcockj
// Added Javadoc comments
//
// Revision 1.96  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.95  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.94  2001/11/26 12:48:01  temperton
// Teletext corrections
//
// Revision 1.93  2001/11/25 21:29:50  laurentg
// Take still, Open file, Close file callbacks updated
//
// Revision 1.92  2001/11/24 22:54:25  laurentg
// Close file added for still source
//
// Revision 1.91  2001/11/24 18:01:39  laurentg
// Still source
//
// Revision 1.90  2001/11/23 10:47:44  adcockj
// Added Hebrew and Hungarian codepages
//
// Revision 1.89  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.88  2001/11/19 14:02:48  adcockj
// Apply patches from Sandu Turcan
//
// Revision 1.87  2001/11/17 18:15:57  adcockj
// Bugfixes (including very silly performance bug)
//
// Revision 1.86  2001/11/14 11:28:03  adcockj
// Bug fixes
//
// Revision 1.85  2001/11/09 14:19:34  adcockj
// Bug fixes
//
// Revision 1.84  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.83  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.82  2001/11/02 10:15:20  temperton
// Removed unnecessary painting of color key in middle part of screen in teletext mode.
//
// Revision 1.81  2001/10/25 12:59:48  temperton
// Fixed problem, when DScaler hangs on exit if we forgot to stop record.
//
// Revision 1.80  2001/10/22 05:55:07  temperton
// Teletext improvements
//
// Revision 1.79  2001/10/06 17:04:26  adcockj
// Fixed teletext crashing problems
//
// Revision 1.78  2001/10/06 12:36:10  laurentg
// New shortcut keys added to adjust left and right player cropping during calibration
//
// Revision 1.77  2001/10/04 12:39:16  adcockj
// Added Teletext colour buttons to UI and switch to using accelerator rather than keyup message
//
// Revision 1.76  2001/09/29 10:51:09  laurentg
// O and Shift+O to adjust specific overscan when in calibration mode
// Enter and Backspace to show and hide calibration OSD when in calibration mode
//
// Revision 1.75  2001/09/21 20:47:12  laurentg
// SaveStill modified to return the name of the written file
// Name of the file added in the OSD text when doing a snapshot
//
// Revision 1.74  2001/09/21 16:43:54  adcockj
// Teletext improvements by Mike Temperton
//
// Revision 1.73  2001/09/21 15:39:01  adcockj
// Added Russian and German code pages
// Corrected UK code page
//
// Revision 1.72  2001/09/11 12:03:52  adcockj
// Updated Help menu to go to help page
//
// Revision 1.71  2001/09/08 19:18:46  laurentg
// Added new specific dialog box to set the overlay settings
//
// Revision 1.70  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.69  2001/09/05 06:59:12  adcockj
// Teletext fixes
//
// Revision 1.68  2001/09/03 13:46:06  adcockj
// Added PAL-NC thanks to Eduardo José Tagle
//
// Revision 1.67  2001/09/02 14:17:51  adcockj
// Improved teletext code
//
// Revision 1.66  2001/09/02 12:13:21  adcockj
// Changed dscaler webiste
// Tidied up resource spelling
//
// Revision 1.65  2001/08/24 21:36:46  adcockj
// Menu bug fix
//
// Revision 1.64  2001/08/23 18:54:21  adcockj
// Menu and Settings fixes
//
// Revision 1.63  2001/08/23 16:03:26  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.62  2001/08/21 09:39:46  adcockj
// Added Greek teletext Codepage
//
// Revision 1.61  2001/08/16 21:17:34  laurentg
// Automatic calibration improved with a fine adjustment
//
// Revision 1.60  2001/08/15 17:50:11  laurentg
// UseRGB ini parameter suppressed
// OSD screen concerning card calibration fully modified
// Automatic calibration added (not finished)
//
// Revision 1.59.2.9  2001/08/24 12:35:09  adcockj
// Menu handling changes
//
// Revision 1.59.2.8  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.59.2.7  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.59.2.6  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.59.2.5  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.59.2.4  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.59.2.3  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
// Revision 1.59.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.59.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
// Revision 1.59  2001/08/13 18:07:24  adcockj
// Added Czech code page for teletext
//
// Revision 1.58  2001/08/09 22:18:23  laurentg
// Improvments in relation with calibration
//
// Revision 1.57  2001/08/09 21:34:59  adcockj
// Fixed bugs raise by Timo and Keld
//
// Revision 1.56  2001/08/08 18:03:20  adcockj
// Moved status timer start till after hardware init
//
// Revision 1.55  2001/08/05 16:32:12  adcockj
// Added brackets
//
// Revision 1.54  2001/08/03 14:36:05  adcockj
// Added menu for sharpness filter
//
// Revision 1.53  2001/08/03 14:24:32  adcockj
// added extra info to splash screen and log
//
// Revision 1.52  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.51  2001/07/30 22:44:04  laurentg
// Bug fixed concerning saturation V accelerator
//
// Revision 1.50  2001/07/28 13:24:40  adcockj
// Added UI for Overlay Controls and fixed issues with SettingsDlg
//
// Revision 1.49  2001/07/27 16:11:32  adcockj
// Added support for new Crash dialog
//
// Revision 1.48  2001/07/26 22:26:24  laurentg
// New menu for card calibration
//
// Revision 1.47  2001/07/24 12:19:00  adcockj
// Added code and tools for crash logging from VirtualDub
//
// Revision 1.46  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
// Revision 1.45  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.44  2001/07/13 18:13:24  adcockj
// Changed Mute to not be persisted and to work properly
//
// Revision 1.43  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.42  2001/07/13 07:04:43  adcockj
// Attemp 1 at fixing MSP muting
//
// Revision 1.41  2001/07/12 19:28:03  adcockj
// Limit VT display to valid pages
//
// Revision 1.40  2001/07/12 16:20:07  adcockj
// Fixed typo in $Id
//
// Revision 1.39  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Other.h"
#include "CPU.h"
#include "MixerDev.h"
#include "VBI_VideoText.h"
#include "AspectRatio.h"
#include "DScaler.h"
#include "Settings.h"
#include "ProgramList.h"
#include "Dialogs.h"
#include "OutThreads.h"
#include "OSD.h"
#include "Audio.h"
#include "Status.h"
#include "VBI.h"
#include "FD_60Hz.H"
#include "FD_50Hz.H"
#include "FD_Common.H"
#include "Filter.h"
#include "Splash.h"
#include "VideoSettings.h"
#include "VBI_CCdecode.h"
#include "VBI_VideoText.h"
#include "Deinterlace.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "TimeShift.h"
#include "Crash.h"
#include "Calibration.h"
#include "Providers.h"
#include "OverlaySettings.h"
#include "Perf.h"
#include "hardwaredriver.h"

HWND hWnd = NULL;
HINSTANCE hResourceInst = NULL;
HINSTANCE hDScalerInst = NULL;

// Used to call MainWndOnInitBT
#define INIT_BT 1800

BOOL bDoResize = FALSE;

HWND VThWnd;

long WStyle;

BOOL    bShowMenu=TRUE;
HMENU   hMenu;
HACCEL  hAccel;

char ChannelString[10];

int MainProcessor=0;
int DecodeProcessor=0;
int PriorClassId = 0;
int ThreadClassId = 1;

//Cursor defines and vars
#define CURSOR_DEFAULT 0x0000
#define CURSOR_HAND    0x0001
HCURSOR hCursorDefault = NULL;
HCURSOR hCursorHand = NULL;

BOOL bShowCursor = TRUE;
BOOL bAutoHideCursor = TRUE;

long MainWndWidth = 649;
long MainWndHeight = 547;
long MainWndLeft = 10;
long MainWndTop = 10;

BOOL bAlwaysOnTop = FALSE;
BOOL bAlwaysOnTopFull = TRUE;
BOOL bDisplayStatusBar = TRUE;
BOOL bDisplaySplashScreen = TRUE;
BOOL bIsFullScreen = FALSE;
BOOL bForceFullScreen = FALSE;
BOOL bUseAutoSave = FALSE;

BOOL bKeyboardLock = FALSE;
HHOOK hKeyboardHook = NULL;

HFONT hCurrentFont = NULL;

int InitialChannel = -1;
int InitialTextPage = -1;

BOOL bInMenuOrDialogBox = FALSE;
BOOL bIgnoreMouse = FALSE;
BOOL bShowCrashDialog = FALSE;

UINT MsgWheel;

BOOL IsFullScreen_OnChange(long NewValue);
BOOL DisplayStatusBar_OnChange(long NewValue);
void Cursor_UpdateVisibility();
void Cursor_SetVisibility(BOOL bVisible);
int Cursor_SetType(int type);
void Cursor_VTUpdate(bool PosValid, int x, int y);
void MainWndOnDestroy();
void SetDirectoryToExe();
int ProcessCommandLine(char* commandLine, char* argv[], int sizeArgv);
void SetKeyboardLock(BOOL Enabled);

///**************************************************************************
//
// FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
//
// PURPOSE: calls initialization function, processes message loop
//
///**************************************************************************

int APIENTRY WinMainOld(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    MSG msg;
    HWND hPrevWindow;

    hDScalerInst = hInstance;

    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(UnexpectedCrashHandler);

    SetDirectoryToExe();

    CPU_SetupFeatureFlag();
    // if we are already runninmg then start up old version
    hPrevWindow = FindWindow((LPCTSTR) DSCALER_APPNAME, NULL);
    if (hPrevWindow != NULL)
    {
        if (IsIconic(hPrevWindow))
        {
            SendMessage(hPrevWindow, WM_SYSCOMMAND, SC_RESTORE, NULL);
        }
        SetFocus(hPrevWindow);
        SetActiveWindow(hPrevWindow);
        SetForegroundWindow(hPrevWindow);
        return FALSE;
    }

    // JA 07/01/2001
    // Required to use slider control
    InitCommonControls();

    // setup default ini file
	SetIniFileForSettings("");
    
    LoadSettingsFromIni();

	// Process the command line arguments.
    // The following arguments are supported
    // -i<inifile>      specification of the ini file.
    // -c<channel>      the starting channel (0-x).
    // -p<page>         the starting videotext page (100-y).
    // -driverinstall   (un)install the NT driver. These arguments are mainly intended to be
    // -driveruninstall used by the installation program. When the user has not enough rights to
    //                  complete (un)installation a messagebox is shown. With other errors there
    //                  is no feedback.
    // For backwards compatibility an argument not starting with a - or / is
    // processed as -i<parameter> (ini file specification).
    char* ArgValues[20];
    int ArgCount = ProcessCommandLine(lpCmdLine, ArgValues, sizeof(ArgValues) / sizeof(char*));
    for (int i = 0; i < ArgCount; ++i)
    {
        if (ArgValues[i][0] != '-' && ArgValues[i][0] != '/')
        {
            SetIniFileForSettings(ArgValues[i]);
        }
        else if(strlen(ArgValues[i]) > 2)
        {
            char* szParameter = &ArgValues[i][2];
            switch (tolower(ArgValues[i][1]))
            {
            case 'd':
                if(strcmp(szParameter, "riverinstall") == 0)
                {
                    DWORD result = 0;                  
                    CHardwareDriver* HardwareDriver = NULL;
                    
                    HardwareDriver = new CHardwareDriver();
                    if(!HardwareDriver->InstallNTDriver())
                    {
                        // if access denied
                        if(GetLastError() == 5)
                        {
                            RealErrorBox("You must have administrative rights to install the driver.");
                        }
                        result = 1;
                    }
                    
                    delete HardwareDriver;
                    
                    return result;
                }
                else if(strcmp(szParameter, "riveruninstall") == 0)
                {
                    DWORD result = 0;                  
                    CHardwareDriver* HardwareDriver = NULL;
                    
                    HardwareDriver = new CHardwareDriver();
                    if(!HardwareDriver->UnInstallNTDriver())
                    {
                        // if access denied
                        if(GetLastError() == 5)
                        {
                            RealErrorBox("You must have administrative rights to uninstall the driver.");
                        }
                        result = 1;
                    }
                    
                    delete HardwareDriver;
                    
                    return result;
                }
            case 'i':
                SetIniFileForSettings(szParameter);              
                break;
            case 'c':
                sscanf(szParameter, "%d", &InitialChannel);
                break;
            case 'p':
                sscanf(szParameter, "%d", &InitialTextPage);
                break;
            default:
                // Unknown
                break;
            }
        }
    }
    
    if(bDisplaySplashScreen)
    {
        ShowSpashScreen();
    }


    // load up the cursors we want to use
    // we load up arrow as the default and try and load up
    // the hand cursor if we are running NT 5
    /// \todo add or find hand cursor for win 95 and up
    hCursorDefault = LoadCursor(NULL, IDC_ARROW);

    OSVERSIONINFO version;
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx(&version);
    if ((version.dwPlatformId == VER_PLATFORM_WIN32_NT) && (version.dwMajorVersion >= 5))
    {
        hCursorHand = LoadCursor(NULL, MAKEINTRESOURCE(32649));
    }
    else
    {
        hCursorHand = hCursorDefault;
    }

    wc.style = CS_DBLCLKS;      // Allow double click
    wc.lpfnWndProc = (WNDPROC) MainWndProcSafe;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DSCALER));
    wc.hCursor = hCursorDefault; 
    wc.hbrBackground = CreateSolidBrush(0);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = DSCALER_APPNAME;

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }
    hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_DSCALERMENU));


    // 2000-10-31 Added by Mark: Changed to WS_POPUP for more cosmetic direct-to-full-screen startup,
    // let UpdateWindowState() handle initialization of windowed dTV instead.


    hWnd = CreateWindow(DSCALER_APPNAME, DSCALER_APPNAME, WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);
    if (!hWnd) return FALSE;
    if (!bIsFullScreen) SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);

    if (!StatusBar_Init()) return FALSE;

    if (bDisplayStatusBar == FALSE)
    {
        StatusBar_ShowWindow(FALSE);
    }

    MsgWheel = RegisterWindowMessage("MSWHEEL_ROLLMSG");

    // 2000-10-31 Added by Mark Rejhon
    // Now show the window, directly to maximized or windowed right away.
    // That way, if the end user has configured dTV to startup maximized,
    // it won't flash a window right before maximizing.
    UpdateWindowState();

    PostMessage(hWnd, WM_SIZE, SIZENORMAL, MAKELONG(MainWndWidth, MainWndHeight));
    if (!(hAccel = LoadAccelerators(hResourceInst, MAKEINTRESOURCE(IDA_DSCALER))))
    {
        ErrorBox("Accelerators not Loaded");
    }

    SetTimer(hWnd, TIMER_VTFLASHER, TIMER_VTFLASHER_MS, NULL);

    // catch any serious errors during message handling
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hWnd, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return msg.wParam;
}

LONG APIENTRY MainWndProcSafe(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    return MainWndProc(hWnd, message, wParam, lParam);
}

HMENU CreateDScalerPopupMenu()
{
    HMENU hMenuPopup;
    hMenuPopup = LoadMenu(hResourceInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenuPopup != NULL)
    {
        hMenuPopup = GetSubMenu(hMenuPopup,0);
    }
    if (hMenuPopup != NULL && hMenu != NULL)
    {
        MENUITEMINFO MenuItemInfo;
        HMENU hSubMenu;

        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fMask = MIIM_SUBMENU;

        hSubMenu = GetSubMenu(hMenu, 5);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,0,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 6);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,1,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetChannelsSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,3, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 3);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,4,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 4);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,5, TRUE, &MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 7);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,6,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 2);
        if(hSubMenu != NULL)
            hSubMenu = GetSubMenu(hSubMenu, 8);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,7,TRUE,&MenuItemInfo);
        }
        CheckMenuItemBool(hMenuPopup, IDM_FULL_SCREEN, bIsFullScreen);
    }
    return hMenuPopup;
}


BOOL WINAPI OnContextMenu(HWND hWnd, int x, int y)
{ 
    RECT rc;                    // client area of window
    POINT pt = {0,0};           // location of mouse click
    HMENU hMenuPopup = CreateDScalerPopupMenu();

    // Get the bounding rectangle of the client area.
    GetClientRect(hWnd, &rc);

    //if the context menu is opend with a keypress, x and y is -1
    if(x!=-1 && y!=-1)
    {
        pt.x=x;
        pt.y=y;

        // Convert the mouse position to client coordinates.
        ScreenToClient(hWnd, &pt);
    }

    // If the position is in the client area, display a
    // shortcut menu.
    if (PtInRect(&rc, pt))
    {
        ClientToScreen(hWnd, &pt);
        // Display the shortcut menu. Track the right mouse
        // button.
        return TrackPopupMenuEx(hMenuPopup, 
                                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, 
                                pt.x, pt.y, hWnd, NULL); 
    }

    if (hMenuPopup != NULL)
    {
        DestroyMenu(hMenuPopup);
    }
 
    // Return FALSE if no menu is displayed.
    return FALSE; 
} 

void SetVTPage(int Page, int SubPage, bool SubPageValid, bool LockSubPage)
{
    if ((Page < 100) || (Page > 899))
        return;

    VTPage = Page;
    VTShowHidden = false;
    if (SubPageValid)
    {
        VTSubPage = SubPage;
        VTSubPageLocked = VT_CompleteSubPages(VTPage - 100) && LockSubPage;
        /** \todo It will be good idea to set timer for removing lock
                  after 2-3 mins and switching back to the most recent 
                  received subpage.
        */
    }
    else
    {
        //Find first available SubPage with most lowest index
        VTSubPage = VT_SubPageNext(VTPage - 100, -1, 1, false);
        VTSubPageLocked = false;
    }

    VT_PurgeRedrawCache();
    VT_DoUpdate_Page(VTPage - 100, VTSubPage);
    Cursor_VTUpdate(false, 0, 0);
    InvalidateRect(hWnd, NULL, FALSE);
}

void SetVTShowHidden(bool Enabled)
{
    if ((Enabled && !VTShowHidden) || (!Enabled && VTShowHidden))
    {
        VTShowHidden = Enabled;
        VT_DoUpdate_Page(VTPage - 100, VTSubPage);
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

LRESULT CALLBACK KeyboardHookProc(int code, UINT wParam, UINT lParam)
{
    if(code >= 0 && bKeyboardLock)
    {
        // if it is not Ctrl+Shift+L do not pass the message to the rest of the hook chain 
        // or the target window procedure
        if(!((char)wParam == 'L' && GetKeyState(VK_SHIFT) < 0 && GetKeyState(VK_CONTROL) < 0))
        {
            return 1;
        }
    }
   	return CallNextHookEx(hKeyboardHook, code, wParam, lParam);
}

void SetKeyboardLock(BOOL Enabled)
{
    if(Enabled)
    {
        bKeyboardLock = TRUE;
        if(hKeyboardHook == NULL)
        {
            hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardHookProc, NULL, 
                                             GetCurrentThreadId());
        }
    }
    else
    {
        bKeyboardLock = FALSE;
        if(hKeyboardHook != NULL)
        {
            UnhookWindowsHookEx(hKeyboardHook);
            hKeyboardHook = NULL;
        }
    }
}

///**************************************************************************
//
//    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
//
//    PURPOSE:  Processes messages
//
//    MESSAGES:
//
//        WM_COMMAND    - application menu (About dialog box)
//        WM_CREATE     - create window and objects
//        WM_PAINT      - update window, draw objects
//        WM_DESTROY    - destroy window
//
//    COMMENTS:
//
//        Handles to the objects you will use are obtained when the WM_CREATE
//        message is received, and deleted when the WM_DESTROY message is
//        received.  The actual drawing is done whenever a WM_PAINT message is
//        received.
//
//
///**************************************************************************
LONG APIENTRY MainWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    char Text[128];
    int i;
    BOOL bDone;
    ISetting* pSetting = NULL;

    if (message == MsgWheel)
    {
        // crack the mouse wheel delta
        // +ve is forward (away from user)
        // -ve is backward (towards user)
        if((short)wParam > 0)
        {
            PostMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
        }
        else
        {
            PostMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
        }
    }

    switch (message)
    {

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_MUTE:
            if (Setting_GetValue(Audio_GetSetting(SYSTEMINMUTE)) == FALSE)
            {
                Setting_SetValue(Audio_GetSetting(SYSTEMINMUTE), TRUE);
                ShowText(hWnd,"MUTE");
            }
            else
            {
                Setting_SetValue(Audio_GetSetting(SYSTEMINMUTE), FALSE);
                ShowText(hWnd,"UNMUTE");
            }
            break;

        case IDM_VOLUMEPLUS:
            if (bUseMixer == FALSE)
            {
                ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                if(pSetting != NULL)
                {
                    pSetting->ChangeValue(ADJUSTUP);
                    sprintf(Text, "BT-Volume %d", pSetting->GetValue() / 10);
                }
                else
                {
                    strcpy(Text, "Volume not supported");
                }
            }
            else
            {
                Mixer_Volume_Up();
                sprintf(Text, "Mixer-Volume %d", Mixer_GetVolume());
            }
            ShowText(hWnd, Text);
            break;

        case IDM_VOLUMEMINUS:
            if (bUseMixer == FALSE)
            {
                ISetting* pSetting = Providers_GetCurrentSource()->GetVolume();
                if(pSetting != NULL)
                {
                    pSetting->ChangeValue(ADJUSTDOWN);
                    sprintf(Text, "BT-Volume %d", pSetting->GetValue() / 10);
                }
                else
                {
                    strcpy(Text, "Volume not supported");
                }
            }
            else
            {
                Mixer_Volume_Down();
                sprintf(Text, "Mixer-Volume %d", Mixer_GetVolume());
            }
            ShowText(hWnd, Text);
            break;

        case IDM_AUTO_FORMAT:
            Setting_SetValue(Timing_GetSetting(AUTOFORMATDETECT), 
                !Setting_GetValue(Timing_GetSetting(AUTOFORMATDETECT)));
            break;

        case IDM_VT_PAGE_MINUS:
            if(VTState != VT_OFF)
            {
                if(VTPage >= 100)
                {
                    SetVTPage(VTPage - 1, 0, false, false);
                }
            }
            else
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_VT_PAGE_PLUS:
            if(VTState != VT_OFF)
            {
                if(VTPage < 899)
                {
                    SetVTPage(VTPage + 1, 0, false, false);
                }
            }
            else
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_VT_PAGE_UP:
            if(VTState != VT_OFF)
            {
                SetVTPage(VTPage, VT_SubPageNext(VTPage - 100, VTSubPage, -1, true), true, true);
            }
            else
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_VT_PAGE_DOWN:
            if(VTState != VT_OFF)
            {
                SetVTPage(VTPage, VT_SubPageNext(VTPage - 100, VTSubPage, +1, true), true, true);
            }
            else
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_CHANNEL_LIST:
            if(!Providers_GetCurrentSource()->IsInTunerMode())
            {
                SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_INPUT1, 0);
            }
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_CHANNELLIST), hWnd, (DLGPROC) ProgramListProc);
            Channels_UpdateMenu(hMenu);
            break;

        case IDM_CHANNELPLUS:
            if (Providers_GetCurrentSource()->IsInTunerMode())
            {
                Channel_Increment();
            }
            break;

        case IDM_CHANNELMINUS:
            if (Providers_GetCurrentSource()->IsInTunerMode())
            {
                Channel_Decrement();
            }
            break;

        case IDM_CHANNEL_PREVIOUS:
            if (Providers_GetCurrentSource()->IsInTunerMode())
            {
                Channel_Previous();
            }
            break;

        case IDM_PATTERN_SELECT:
            pCalibration->SelectTestPattern(lParam);
            break;

        case IDM_START_AUTO_CALIBRATION:
            pCalibration->Start(CAL_AUTO_FULL);
            break;

        case IDM_START_AUTO_CALIBRATION2:
            pCalibration->Start(CAL_AUTO_BRIGHT_CONTRAST);
            break;

        case IDM_START_AUTO_CALIBRATION3:
            pCalibration->Start(CAL_AUTO_COLOR);
            break;

        case IDM_START_MANUAL_CALIBRATION:
            pCalibration->Start(CAL_MANUAL);
            break;

        case IDM_STOP_CALIBRATION:
            pCalibration->Stop();
            break;

        case IDM_RESET:
            Reset_Capture();
            Sleep(100);
            Providers_GetCurrentSource()->UnMute();
            break;

        case IDM_TOGGLE_MENU:
            bShowMenu = !bShowMenu;
            WorkoutOverlaySize(TRUE);
            break;

        case IDM_AUTODETECT:
            KillTimer(hWnd, TIMER_FINDPULL);
            if(Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                ShowText(hWnd, "Auto Pulldown Detect OFF");
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
            }
            else
            {
                ShowText(hWnd, "Auto Pulldown Detect ON");
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), TRUE);
            }
            // Set Deinterlace Mode to film fallback in
            // either case
            if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
            }
            else
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
            }
            break;

        case IDM_FINDLOCK_PULL:
            if(!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), TRUE);
            }
            // Set Deinterlace Mode to film fallback in
            // either case
            if(GetTVFormat(Providers_GetCurrentSource()->GetFormat())->Is25fps)
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
            }
            else
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
            }
            SetTimer(hWnd, TIMER_FINDPULL, TIMER_FINDPULL_MS, NULL);
//            ShowText(hWnd, "Searching Film mode ...");
            break;

        case IDM_FALLBACK:
            if(Setting_GetValue(FD60_GetSetting(FALLBACKTOVIDEO)))
            {
                ShowText(hWnd, "Fallback on Bad Pulldown OFF");
                Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), FALSE);
            }
            else
            {
                ShowText(hWnd, "Fallback on Bad Pulldown ON");
                Setting_SetValue(FD60_GetSetting(FALLBACKTOVIDEO), TRUE);
            }
            break;

        case IDM_22PULLODD:
        case IDM_22PULLEVEN:
        case IDM_32PULL1:
        case IDM_32PULL2:
        case IDM_32PULL3:
        case IDM_32PULL4:
        case IDM_32PULL5:
            KillTimer(hWnd, TIMER_FINDPULL);
            Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
            SetFilmDeinterlaceMode((eFilmPulldownMode)(LOWORD(wParam) - IDM_22PULLODD));
            ShowText(hWnd, GetDeinterlaceModeName());
            break;

        case IDM_ABOUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
            break;

        case IDM_BRIGHTNESS_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetBrightness()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Brightness Control");
            }
            break;

        case IDM_KONTRAST_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetContrast()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Contrast Control");
            }
            break;

        case IDM_USATURATION_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;
        
        case IDM_USATURATION_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;

        case IDM_USATURATION_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationU()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation U Control");
            }
            break;
        
        case IDM_VSATURATION_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturationV()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation V Control");
            }
            break;

        case IDM_COLOR_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetSaturation()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Saturation Control");
            }
            break;

        case IDM_HUE_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetHue()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Hue Control");
            }
            break;

        case IDM_OVERSCAN_PLUS:
            if((pSetting = Providers_GetCurrentSource()->GetOverscan()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTUP);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_MINUS:
            if((pSetting = Providers_GetCurrentSource()->GetOverscan()) != NULL)
            {
                pSetting->ChangeValue(ADJUSTDOWN);
            }
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_CURRENT:
            if((pSetting = Providers_GetCurrentSource()->GetOverscan()) != NULL)
            {
                pSetting->OSDShow();
            }
            else
            {
                ShowText(hWnd, "No Overscan Control");
            }
            break;

        case IDM_AUTOHIDE_CURSOR:
            bAutoHideCursor = !bAutoHideCursor;
            Cursor_UpdateVisibility();
            break;

        case IDM_TOGGLECURSOR:
            if(!bAutoHideCursor && bIsFullScreen == FALSE)
            {
                bShowCursor = !bShowCursor;
                Cursor_UpdateVisibility();
            }
            break;

        case IDM_END:
            ShowWindow(hWnd, SW_HIDE);
            PostMessage(hWnd, WM_DESTROY, wParam, lParam);
            break;

        case IDM_VBI_VT:
            Setting_SetValue(VBI_GetSetting(DOTELETEXT), 
                !Setting_GetValue(VBI_GetSetting(DOTELETEXT)));
            break;

        case IDM_CCOFF:
        case IDM_CC1:
        case IDM_CC2:
        case IDM_CC3:
        case IDM_CC4:
        case IDM_TEXT1:
        case IDM_TEXT2:
        case IDM_TEXT3:
        case IDM_TEXT4:
            Setting_SetValue(VBI_GetSetting(CLOSEDCAPTIONMODE), 
                LOWORD(wParam) - IDM_CCOFF);
            break;

        case IDM_VBI_VPS:
            Setting_SetValue(VBI_GetSetting(DOVPS), 
                !Setting_GetValue(VBI_GetSetting(DOVPS)));
            break;

        case IDM_VBI_WSS:
            Setting_SetValue(VBI_GetSetting(DOWSS), 
                !Setting_GetValue(VBI_GetSetting(DOWSS)));
            break;

        case IDM_CALL_VIDEOTEXT:
            switch(VTState)
            {
            case VT_OFF:
                VTState = VT_BLACK;
                break;
            case VT_BLACK:
                VTState = VT_MIX;
                break;
            case VT_MIX:
            default:
                VTState = VT_OFF;
                break;
            }
            
            if(VTState == VT_OFF)
            {
                ;
            }
            else if(!Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)) || !Setting_GetValue(VBI_GetSetting(DOTELETEXT)) )
            {
                Setting_SetValue(VBI_GetSetting(CAPTURE_VBI), TRUE);
                Setting_SetValue(VBI_GetSetting(DOTELETEXT), TRUE);
            }
            
            SetVTPage(VTPage, VTSubPage, true, false);

            WorkoutOverlaySize(TRUE);

            InvalidateRect(hWnd,NULL,FALSE);
            break;

        case IDM_VT_RESET:
            VT_ChannelChange();
            break;

        case IDM_VIDEOSETTINGS:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VIDEOSETTINGS), hWnd, VideoSettingProc);
            break;

        case IDM_VPS_OUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VPSSTATUS), hWnd, VPSInfoProc);
            break;

        case IDM_VT_OUT:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_VTSTATUS), hWnd, VTInfoProc);
            break;

        case IDM_VBI:
            Stop_Capture();
            Setting_SetValue(VBI_GetSetting(CAPTURE_VBI), 
                !Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)));
            Start_Capture();
            break;

        case IDM_CAPTURE_PAUSE:
            Pause_Toggle_Capture();
            break;

        case IDM_AUDIO_MIXER:
            Mixer_SetupDlg(hWnd);
            break;

        case IDM_STATUSBAR:
            DisplayStatusBar_OnChange(!bDisplayStatusBar);
            break;

        case IDM_ON_TOP:
            bAlwaysOnTop = !bAlwaysOnTop;
            WorkoutOverlaySize(FALSE);
            break;

        case IDM_ALWAYONTOPFULLSCREEN:
            bAlwaysOnTopFull = !bAlwaysOnTopFull;
            WorkoutOverlaySize(FALSE);
            break;

        case IDM_SPLASH_ON_STARTUP:
            bDisplaySplashScreen = !bDisplaySplashScreen;
            break;

        case IDM_KEYBOARDLOCK:
            bKeyboardLock = !bKeyboardLock;
            SetKeyboardLock(bKeyboardLock);
            if(bKeyboardLock)
            {
                OSD_ShowText(hWnd, "Keyboard lock on", 0);
            }
            else
            {
                OSD_ShowText(hWnd, "Keyboard lock off", 0);
            }
            
            break;

        case IDM_TREADPRIOR_0:
        case IDM_TREADPRIOR_1:
        case IDM_TREADPRIOR_2:
        case IDM_TREADPRIOR_3:
        case IDM_TREADPRIOR_4:
            ThreadClassId = LOWORD(wParam) - IDM_TREADPRIOR_0;
            Stop_Capture();
            Start_Capture();
            break;

        case IDM_PRIORCLASS_0:
        case IDM_PRIORCLASS_1:
        case IDM_PRIORCLASS_2:
            PriorClassId = LOWORD(wParam) - IDM_PRIORCLASS_0;
            strcpy(Text, "Can't set Priority");
            if (PriorClassId == 0)
            {
                if (SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS) == TRUE)
                    strcpy(Text, "Normal Priority");
            }
            else if (PriorClassId == 1)
            {
                if (SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS) == TRUE)
                    strcpy(Text, "High Priority");
            }
            else
            {
                if (SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS) == TRUE)
                    strcpy(Text, "Real-Time Priority");
            }
            ShowText(hWnd, Text);
            break;

        case IDM_JUDDERTERMINATOR:
            Stop_Capture();
            Setting_SetValue(OutThreads_GetSetting(DOACCURATEFLIPS), 
                !Setting_GetValue(OutThreads_GetSetting(DOACCURATEFLIPS)));
            Start_Capture();
            break;

        case IDM_VT_UK:
            VT_SetCodePage(VT_UK_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case IDM_VT_FRENCH:
            VT_SetCodePage(VT_FRENCH_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);            
            break;

        case IDM_VT_CZECH:
            VT_SetCodePage(VT_CZECH_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);            
            break;

        case IDM_VT_GREEK:
            VT_SetCodePage(VT_GREEK_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case IDM_VT_RUSSIAN:
            VT_SetCodePage(VT_RUSSIAN_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case IDM_VT_GERMAN:
            VT_SetCodePage(VT_GERMAN_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        
        case IDM_VT_HUNGARIAN:
            VT_SetCodePage(VT_HUNGARIAN_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);            
            break;

        case IDM_VT_HEBREW:
            VT_SetCodePage(VT_HEBREW_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);           
            break;

        case IDM_VT_SWEDISH:
            VT_SetCodePage(VT_SWEDISH_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case IDM_VT_ITALIAN:
            VT_SetCodePage(VT_ITALIAN_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case IDM_VT_SPANISH:
            VT_SetCodePage(VT_SPANISH_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case IDM_VT_POLISH:
            VT_SetCodePage(VT_POLISH_CODE_PAGE);
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case IDM_USECHROMA:
            Stop_Capture();
            Setting_SetValue(FD_Common_GetSetting(USECHROMA), 
                !Setting_GetValue(FD_Common_GetSetting(USECHROMA)));
            Start_Capture();
            break;

        case IDM_SPACEBAR:
            if(!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                IncrementDeinterlaceMode();
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;

        case IDM_SHIFT_SPACEBAR:
            if (!Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
            {
                DecrementDeinterlaceMode();
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;

        case IDM_FULL_SCREEN:
            IsFullScreen_OnChange(!bIsFullScreen);
            break;
        
        case IDM_RETURN_TO_WINDOW:
            if(bIsFullScreen)
            {
                IsFullScreen_OnChange(FALSE);
            }
            else
            {
                ShowWindow(hWnd, SW_MINIMIZE);
            }
            break;

        case IDM_TAKESTREAMSNAP:
            RequestStreamSnap();
            break;

        case IDM_TAKESTILL:
            RequestStill();
            break;

        case IDM_RESET_STATS:
            ResetDeinterlaceStats();
            pPerf->Reset();
            break;

        case IDM_TSOPTIONS:
            CTimeShift::OnOptions();
            break;

        case IDM_TSRECORD:
            if (CTimeShift::OnRecord())
            {
                ShowText(hWnd, "Recording");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSSTOP:
            if (CTimeShift::OnStop())
            {
                ShowText(hWnd, "Stopped");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPLAY:
            if (CTimeShift::OnPlay())
            {
                ShowText(hWnd, "Playing");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPAUSE:
            if (CTimeShift::OnPause())
            {
                ShowText(hWnd, "Paused");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSFFWD:
            if (CTimeShift::OnFastForward())
            {
                ShowText(hWnd, "Scanning >>>");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSRWND:
            if (CTimeShift::OnFastBackward())
            {
                ShowText(hWnd, "Scanning <<<");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSNEXT:
            if (CTimeShift::OnGoNext())
            {
                ShowText(hWnd, "Next Clip");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPREV:
            if (CTimeShift::OnGoPrev())
            {
                ShowText(hWnd, "Previous Clip");
                CTimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_SHOW_OSD:
            if (pCalibration->IsRunning())
            {
                OSD_ShowInfosScreen(hWnd, 4, 0);
            }
            else
            {
                OSD_ShowNextInfosScreen(hWnd, 0);
            }
            break;

        case IDM_OSDSCREEN_SHOW:
            if (!pCalibration->IsRunning())
            {
                OSD_ShowInfosScreen(hWnd, lParam, 0);
            }
            break;

        case IDM_OSDSCREEN_ACTIVATE:
            OSD_ActivateInfosScreen(hWnd, lParam, 0);
            break;

        case IDM_HIDE_OSD:
            OSD_Clear(hWnd);
            break;

        case IDM_AUTOHIDE_OSD:
            if (Setting_GetValue(OSD_GetSetting(OSD_AUTOHIDE_SCREEN)))
            {
                OSD_ShowText(hWnd, "Persistent OSD screens", 0);
                Setting_SetValue(OSD_GetSetting(OSD_AUTOHIDE_SCREEN), FALSE);
            }
            else
            {
                OSD_ShowText(hWnd, "Autohide OSD screens", 0);
                Setting_SetValue(OSD_GetSetting(OSD_AUTOHIDE_SCREEN), TRUE);
            }
            break;

        case IDM_SET_OSD_TEXT:
            // Useful for external programs for custom control of dTV's OSD display
            // Such as macros in software such as Girder, etc.
            if (lParam)
            {
                lstrcpy(Text, "");
                GlobalGetAtomName((ATOM) lParam, Text, sizeof(Text));
                OSD_ShowTextOverride(hWnd, Text, 0);
                GlobalDeleteAtom((ATOM) lParam);
            }
            else
            {
                OSD_ShowTextOverride(hWnd, "", 0);
            }
            break;

        case IDM_SAVE_SETTINGS_NOW:
            WriteSettingsToIni(FALSE);
            break;

        case IDM_OSD_CC_TEXT:
            {
                RECT winRect;
                RECT DestRect;
                PAINTSTRUCT sPaint;
                GetClientRect(hWnd, &winRect);
                if(bDisplayStatusBar == TRUE)
                {
                    winRect.bottom -= StatusBar_Height();
                }
                InvalidateRect(hWnd, &winRect, FALSE);
                BeginPaint(hWnd, &sPaint);
                PaintColorkey(hWnd, TRUE, sPaint.hdc, &winRect);
                GetDestRect(&DestRect);
                CC_PaintScreen(hWnd, (TCCScreen*)lParam, sPaint.hdc, &DestRect);
                EndPaint(hWnd, &sPaint);
                ValidateRect(hWnd, &winRect);
            }
            break;

        case IDM_OVERLAY_STOP:
            Overlay_Stop(hWnd);
            break;

        case IDM_OVERLAY_START:
            Overlay_Start(hWnd);
            break;

        case IDM_OVERLAYSETTINGS:
            DialogBox(hResourceInst, MAKEINTRESOURCE(IDD_OVERLAYSETTINGS), hWnd, OverlaySettingProc);
            break;

        case IDM_FAST_REPAINT:
            {
                RECT winRect;
                PAINTSTRUCT sPaint;
                GetClientRect(hWnd, &winRect);
                if(bDisplayStatusBar == TRUE)
                {
                    winRect.bottom -= StatusBar_Height();
                }
                BeginPaint(hWnd, &sPaint);
                if(VTState == VT_OFF)
                {
                    PaintColorkey(hWnd, TRUE, sPaint.hdc, &winRect);
                    OSD_Redraw(hWnd, sPaint.hdc);
                }
                else
                {
                    VT_Redraw(hWnd, sPaint.hdc, FALSE, FALSE);
                }
                EndPaint(hWnd, &sPaint);
                ValidateRect(hWnd, &sPaint.rcPaint);
            }
            break;

        case IDM_SHOWPLUGINUI:
            ShowVideoModeUI();
            break;

        case IDM_32PULLDOWN_SETTINGS:
            FD60_ShowUI();
            break;

        case IDM_22PULLDOWN_SETTINGS:
            FD50_ShowUI();
            break;

        case IDM_PULLDOWNCOMMON_SETTINGS:
            FD_Common_ShowUI();
            break;

        case IDM_DEBUG_SETTINGS:
            Debug_ShowUI();
            break;

        case IDM_TIMING_SETTNGS:
            Timing_ShowUI();
            break;

        case IDM_OVERLAY_SETTINGS:
            Other_ShowUI();
            break;

        case IDM_ASPECT_SETTINGS:
            Aspect_ShowUI();
            break;

        case IDM_CALIBR_SETTINGS:
            Calibr_ShowUI();
            break;

        case IDM_OSD_SETTINGS:
            OSD_ShowUI();
            break;

        case IDM_HELP_HOMEPAGE:
            ShellExecute(hWnd, "open", "http://www.dscaler.org/", NULL, NULL, SW_SHOWNORMAL);
            break;

        case IDM_HELP_FAQ:
            HtmlHelp(hWnd, "DScaler.chm::/FAQ.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_SUPPORT:
            HtmlHelp(hWnd, "DScaler.chm::/user_support.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_KEYBOARD:
            HtmlHelp(hWnd, "DScaler.chm::/keyboard.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_GPL:
            HtmlHelp(hWnd, "DScaler.chm::/COPYING.html", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_HELP_README:
            HtmlHelp(hWnd, "DScaler.chm::/Help.htm", HH_DISPLAY_TOPIC, 0);
            break;

        case IDM_TELETEXT_KEY1:
        case IDM_TELETEXT_KEY2:
        case IDM_TELETEXT_KEY3:
        case IDM_TELETEXT_KEY4:
        case IDM_TELETEXT_KEY5:
            if(VTState != VT_OFF)
            {
                if(VTPage >= 100)
                {
                    /// \todo VT_GetFlofPageNumber do not return SubPage...
                    i = VT_GetFlofPageNumber(VTPage-100, VTSubPage, LOWORD(wParam) - IDM_TELETEXT_KEY1);
                    if(i >= 100 && i <= 899) 
                    {
                        SetVTPage(i, 0, false, true);
                    }
                }
            }
            break;

        case IDM_TELETEXT_KEY6:
            if (VTState != VT_OFF)
            {
                SetVTShowHidden(!VTShowHidden);
            }
            break;

        case IDM_LEFT_CROP_PLUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Up(Calibr_GetSetting(LEFT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_LEFT_CROP_CURRENT, 0);
            }
            break;

        case IDM_LEFT_CROP_MINUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Down(Calibr_GetSetting(LEFT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_LEFT_CROP_CURRENT, 0);
            }
            break;

        case IDM_LEFT_CROP_CURRENT:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_OSDShow(Calibr_GetSetting(LEFT_SOURCE_CROPPING), hWnd);
            }
            break;

        case IDM_RIGHT_CROP_PLUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Up(Calibr_GetSetting(RIGHT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_RIGHT_CROP_CURRENT, 0);
            }
            break;

        case IDM_RIGHT_CROP_MINUS:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_Down(Calibr_GetSetting(RIGHT_SOURCE_CROPPING));
                SendMessage(hWnd, WM_COMMAND, IDM_RIGHT_CROP_CURRENT, 0);
            }
            break;

        case IDM_RIGHT_CROP_CURRENT:
            if ( pCalibration->IsRunning()
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                Setting_OSDShow(Calibr_GetSetting(RIGHT_SOURCE_CROPPING), hWnd);
            }
            break;

        case IDM_CHARSET_TEST:
            if(VTState != VT_OFF)
            {
                VT_CreateTestPage();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;

        default:
            // Check whether menu ID is an aspect ratio related item
            bDone = ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            if(!bDone)
            {
                bDone = ProcessDeinterlaceSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessFilterSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessProgramSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = ProcessOSDSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone && pCalibration != NULL)
            {
                bDone = pCalibration->ProcessSelection(hWnd, LOWORD(wParam));
            }
            if(!bDone)
            {
                bDone = Providers_HandleWindowsCommands(hWnd, wParam, lParam);
            }
            break;
        }

        //-------------------------------------------------------
        // The following code executes on all WM_COMMAND calls

        // Updates the menu checkbox settings
        SetMenuAnalog();

        if(bUseAutoSave)
        {
            // Set the configuration file autosave timer.
            // We use an autosave timer so that when the user has finished
            // making adjustments and at least a small delay has occured,
            // that the DTV.INI file is properly up to date, even if 
            // the system crashes or system is turned off abruptly.
            KillTimer(hWnd, TIMER_AUTOSAVE);
            SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);
        }
        break;

    case WM_CREATE:
        MainWndOnCreate(hWnd);
        break;

    case INIT_BT:
        MainWndOnInitBT(hWnd);
        break;

// 2000-10-31 Added by Mark Rejhon
// This was an attempt to allow dTV to run properly through
// computer resolution changes.  Alas, dTV still crashes and burns
// in a fiery dive if you try to change resolution while dTV is
// running.  We need to somehow capture a message that comes right
// before a resolution change, so we can destroy the video overlay
// on time beforehand.   This message seems to happen right after
// a resolution change.
//
//  case WM_DISPLAYCHANGE:
//      // Windows resolution changed while software running
//      Stop_Capture();
//      Overlay_Destroy();
//      Sleep(100);
//      Overlay_Create();
//      BT848_ResetHardware();
//      BT848_SetGeoSize();
//      WorkoutOverlaySize();
//      Start_Capture();
//      Sleep(100);
//      BT848_SetAudioSource(AudioSource);
//      break;

    case WM_POWERBROADCAST:
        // Handing to keep dTV running during computer suspend/resume
        switch ((DWORD) wParam)
        {
        case PBT_APMSUSPEND:
            // Stops video overlay upon suspend operation.
            Overlay_Stop(hWnd);
            break;
        case PBT_APMRESUMESUSPEND:
            // Restarts video overlay upon resume operation.
            // The following crashes unless we do either a HWND_BROADCAST
            // or a Sleep() operation.  To be on the safe side, I do both
            // here.  Perhaps the video overlay drivers needed to reinitialize.
            SendMessage(HWND_BROADCAST, WM_PAINT, 0, 0);
            Sleep(500);
            Overlay_Start(hWnd);
            break;
        }
        break;

//  case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
        SendMessage(hWnd, WM_COMMAND, IDM_FULL_SCREEN, 0);
        break;

//  case WM_RBUTTONUP:
//      if(!bAutoHideCursor && bIsFullScreen == FALSE)
//      {
//          bShowCursor = !bShowCursor;
//          Cursor_UpdateVisibility();
//      }
//      break;

    case WM_LBUTTONDOWN:
        if (VTState != VT_OFF) 
        {
            int a = VT_GetPageNumberAt(VTPage-100, VTSubPage, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            if ((a >= 100) && (a <= 899)) 
            {
                SetVTPage(a, 0, false, false);
                break;
            }
        }
        if(bShowMenu == FALSE && bIsFullScreen == FALSE)
        {
            // pretend we are hitting the caption bar
            // this will allow the user to move the window
            // when the menu and title bar are hidden
            return DefWindowProc(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
        }
        break;

    case WM_MOUSEMOVE:
        if (VTState != VT_OFF) 
            Cursor_VTUpdate(true, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_NCHITTEST:
        if (!bIgnoreMouse && !bInMenuOrDialogBox && bAutoHideCursor)
        {
            Cursor_UpdateVisibility();
        }
        bIgnoreMouse = FALSE;
        break;

    case WM_ENTERMENULOOP:
        bInMenuOrDialogBox = TRUE;
        Cursor_UpdateVisibility();
        break;

    case WM_EXITMENULOOP:
        bInMenuOrDialogBox = FALSE;
        Cursor_UpdateVisibility();
        break;

    case WM_INITMENU: 
        SetMenuAnalog();
        break;

    case WM_CONTEXTMENU: 
        if (!OnContextMenu(hWnd, GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)))
            return DefWindowProc(hWnd, message, wParam, lParam);
        break;

    case WM_KILLFOCUS:
        bInMenuOrDialogBox = TRUE;
        Cursor_UpdateVisibility();
        break;

    case WM_SETFOCUS:
        bInMenuOrDialogBox = FALSE;
        Cursor_UpdateVisibility();
        break;

    case WM_TIMER:
        pPerf->Suspend();

        switch (LOWORD(wParam))
        {
        //-------------------------------
        case TIMER_STATUS:
            if (IsStatusBarVisible())
            {
                if (Setting_GetValue(Audio_GetSetting(SYSTEMINMUTE)) == TRUE)
	    		{
                    strcpy(Text, "Volume Mute");
			    }
                else if (!Providers_GetCurrentSource()->IsVideoPresent())
                {
                    strcpy(Text, "No Video Signal Found");
                }
                else
                {
                    strcpy(Text, Providers_GetCurrentSource()->GetStatus());
                    if(Text[0] == 0x00)
                    {
                        if(Providers_GetCurrentSource()->IsInTunerMode())
                        {
                            strcpy(Text, Channel_GetName());
                        }
                    }
                }
                StatusBar_ShowText(STATUS_TEXT, Text);

                sprintf(Text, "%d DF/S", pPerf->GetDroppedFieldsLastSecond());
                StatusBar_ShowText(STATUS_FPS, Text);
            }
            break;
        //-------------------------------
        case TIMER_KEYNUMBER:
            KillTimer(hWnd, TIMER_KEYNUMBER);
            if(VTState != VT_OFF)
            {
				i = atoi(ChannelString);
                if(i >= 100 && i < 900) //This checking not needed now...
                {
                    SetVTPage(i, 0, false, false);
                }
            }
            else
            {
                i = atoi(ChannelString);
                Channel_ChangeToNumber(i);
            }
            ChannelString[0] = '\0';
            break;
        //-------------------------------
        case TIMER_AUTOSAVE:
            // JA 21/12/00 Added KillTimer so that settings are not
            // written repeatedly
            KillTimer(hWnd, TIMER_AUTOSAVE);
            WriteSettingsToIni(TRUE);
            break;
        //-------------------------------
        case OSD_TIMER_ID:
            OSD_Clear(hWnd);
            break;
        //-------------------------------
        case OSD_TIMER_REFRESH_ID:
            OSD_RefreshInfosScreen(hWnd, 0, OSD_REFRESH_DATA);
            break;
        //-------------------------------
        case TIMER_BOUNCE:
        case TIMER_ORBIT:
            // MRS 2-20-01 - Resetup the display for bounce and orbiting
            WorkoutOverlaySize(FALSE); // Takes care of everything...
            break;
        //-------------------------------
        case TIMER_HIDECURSOR:
            KillTimer(hWnd, TIMER_HIDECURSOR);
            if (!bInMenuOrDialogBox)
                Cursor_SetVisibility(FALSE);
            break;
        //-------------------------------
        case TIMER_VTFLASHER:
            {
                static bool Show = true;
                Show=!Show;
                if(VTState!=VT_OFF)
                {
                    HDC hDC = GetDC(hWnd);
                    //VT_Redraw(hWnd, hDC, FALSE, Show);
                    VT_RedrawFlash(hWnd, hDC, Show);
                    ReleaseDC(hWnd, hDC);
                }
            }
            break;
        //---------------------------------
        case TIMER_VTUPDATE:
            {
                HDC hDC = GetDC(hWnd);                    
                VT_ProcessRedrawCache(hWnd, hDC);
                ReleaseDC(hWnd, hDC);
            }
            break;
        //---------------------------------
        case TIMER_FINDPULL:
            {
                KillTimer(hWnd, TIMER_FINDPULL);
                Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
                ShowText(hWnd, GetDeinterlaceModeName());
            }
            break;
        default:
            Provider_HandleTimerMessages(LOWORD(wParam));
            break;
        }
        pPerf->Resume();

        return FALSE;
        break;
    
    // support for mouse wheel
    // the WM_MOUSEWHEEL message is not defined but this is it's Value
    case WM_MOUSELAST + 1:
        if ((wParam & (MK_SHIFT | MK_CONTROL)) == 0)
        {
            // crack the mouse wheel delta
            // +ve is forward (away from user)
            // -ve is backward (towards user)
            if((short)HIWORD(wParam) > 0)
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELPLUS, 0);
            }
            else
            {
                PostMessage(hWnd, WM_COMMAND, IDM_CHANNELMINUS, 0);
            }
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam & 0xFFF0)
        {
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
            return FALSE;
            break;
        }
        break;

    case WM_SIZE:
        StatusBar_Adjust(hWnd);
        if (bDoResize == TRUE)
        {
            switch(wParam)
            {
            case SIZE_MAXIMIZED:
                if(bIsFullScreen == FALSE)
                {
                    bIsFullScreen = TRUE;
                    Cursor_UpdateVisibility();
                    WorkoutOverlaySize(FALSE);
                }
                break;
            case SIZE_MINIMIZED:
                Overlay_Update(NULL, NULL, DDOVER_HIDE);
                break;
            case SIZE_RESTORED:
                InvalidateRect(hWnd, NULL, FALSE);
                WorkoutOverlaySize(FALSE);
                SetMenuAnalog();
                break;
            default:
                break;
            }
        }
        break;

    case WM_MOVE:
        StatusBar_Adjust(hWnd);
        if (bDoResize == TRUE && !IsIconic(hWnd) && !IsZoomed(hWnd))
        {
            WorkoutOverlaySize(FALSE);
        }
        break;

    case WM_CHAR:
        if (Providers_GetCurrentSource()->IsInTunerMode() || VTState != VT_OFF)
        {
            if (((char) wParam >= '0') && ((char) wParam <= '9'))
            {
                sprintf(Text, "%c", (char)wParam);
                strcat(ChannelString, Text);
                if(VTState == VT_OFF)
                {
                    OSD_ShowText(hWnd, ChannelString, 0);
                }
                if (strlen(ChannelString) >= 3)
                {
                    SetTimer(hWnd, TIMER_KEYNUMBER, 1, NULL);
                }
                else
                {
                    SetTimer(hWnd, TIMER_KEYNUMBER, TIMER_KEYNUMBER_MS, NULL);
                }
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT sPaint;
            BeginPaint(hWnd, &sPaint);
            if(VTState != VT_OFF)
            {
                PaintColorkey(hWnd, TRUE, sPaint.hdc, &sPaint.rcPaint, true);
                VT_Redraw(hWnd, sPaint.hdc, FALSE, FALSE);
            }
            else
            {
                PaintColorkey(hWnd, TRUE, sPaint.hdc, &sPaint.rcPaint);
                OSD_Redraw(hWnd, sPaint.hdc);
            }
            EndPaint(hWnd, &sPaint);
        }
        break;

    case WM_VIDEOTEXT:
        {
            if(VTState != VT_OFF)
            {
                switch(LOWORD(wParam))
                {
                case VTM_REDRAWHEADER:
                    {
                        HDC hDC = GetDC(hWnd);
                        VT_RedrawClock(hWnd, hDC, !VT_ContainsUpdatedPage());
                        ReleaseDC(hWnd, hDC);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        break;
    
    //TJ 010506 make sure we dont erase the background
    //if we do, it will cause flickering when resizing the window
    //in future we migth need to adjust this to only erase parts not covered by overlay
    case WM_ERASEBKGND:
        return TRUE;
    
    case WM_QUERYENDSESSION:
    case WM_DESTROY:
        MainWndOnDestroy();
        PostQuitMessage(0);
        break;

    default:
        {
            LONG RetVal = Settings_HandleSettingMsgs(hWnd, message, wParam, lParam, &bDone);
            if(!bDone)
            {
                RetVal = Deinterlace_HandleSettingsMsg(hWnd, message, wParam, lParam, &bDone);
            }
            if(!bDone)
            {
                RetVal = Filter_HandleSettingsMsg(hWnd, message, wParam, lParam, &bDone);
            }

            if(bDone)
            {
                // Updates the menu checkbox settings
                SetMenuAnalog();

                if(bUseAutoSave)
                {
                    // Set the configuration file autosave timer.
                    // We use an autosave timer so that when the user has finished
                    // making adjustments and at least a small delay has occured,
                    // that the DTV.INI file is properly up to date, even if 
                    // the system crashes or system is turned off abruptly.
                    KillTimer(hWnd, TIMER_AUTOSAVE);
                    SetTimer(hWnd, TIMER_AUTOSAVE, TIMER_AUTOSAVE_MS, NULL);
                }
                return RetVal;
            }
            else
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

//---------------------------------------------------------------------------
void SaveWindowPos(HWND hWnd)
{
    WINDOWPLACEMENT WndPlace;
    if(hWnd != NULL)
    {
        // MRS 2-20-01 - length must be set in WindowPlacement structure
        memset(&WndPlace,0,sizeof(WndPlace));
        WndPlace.length = sizeof(WndPlace);
        // End 2-20-01
        GetWindowPlacement(hWnd, &WndPlace);

        MainWndTop = WndPlace.rcNormalPosition.top;
        MainWndHeight = WndPlace.rcNormalPosition.bottom - WndPlace.rcNormalPosition.top;
        MainWndLeft = WndPlace.rcNormalPosition.left;
        MainWndWidth = WndPlace.rcNormalPosition.right - WndPlace.rcNormalPosition.left;

        // We need to adust these numbers by the workspace
        // offset so that we can later use them to set the
        // windows position
        RECT Workspace = {0,0,0,0};
        SystemParametersInfo(SPI_GETWORKAREA, 0, &Workspace, 0);
        MainWndTop += Workspace.top;
        MainWndLeft += Workspace.left;
    }
}


//---------------------------------------------------------------------------
void MainWndOnInitBT(HWND hWnd)
{
    int i;
    BOOL bInitOK = FALSE;

    AddSplashTextLine("Hardware Init");

    if (Providers_Load(hMenu) > 0)
    {
        if(InitDD(hWnd) == TRUE)
        {
            if(Overlay_Create() == TRUE)
            {
                bInitOK = TRUE;
            }
        }
    }
    else
    {
        AddSplashTextLine("");
        AddSplashTextLine("No");
        AddSplashTextLine("Suitable");
        AddSplashTextLine("Hardware");
    }

    if (bInitOK)
    {
        AddSplashTextLine("Load Plugins");
        if(!LoadDeinterlacePlugins())
        {
            AddSplashTextLine("");
            AddSplashTextLine("No");
            AddSplashTextLine("Plug-ins");
            AddSplashTextLine("Found");
            bInitOK = FALSE;
        }
        else
        {
            LoadFilterPlugins();
        }
    }
    
    if (bInitOK)
    {
        AddSplashTextLine("Position Window");
        WStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
        if (bAlwaysOnTop == FALSE)
        {
            WStyle = WStyle ^ 8;
            i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
            SetWindowPos(hWnd, HWND_NOTOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
        else
        {
            WStyle = WStyle | 8;
            i = SetWindowLong(hWnd, GWL_EXSTYLE, WStyle);
            SetWindowPos(hWnd, HWND_TOPMOST, 10, 10, 20, 20, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOSIZE | SWP_SHOWWINDOW);
        }

        if (bShowMenu == FALSE)
        {
            bShowMenu = TRUE;
            SendMessage(hWnd, WM_COMMAND, IDM_TOGGLE_MENU, 0);
        }


        AddSplashTextLine("Setup Mixer");
        Mixer_Init();

        AddSplashTextLine("Start Timers");
        if(bIsFullScreen == FALSE && bDisplayStatusBar == TRUE)
        {
            SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
        }

        // do final setup routines for any files
        // basically where we need the hWnd to be set
        AddSplashTextLine("Setup Aspect Ratio");
        Aspect_FinalSetup();

        // OK we're ready to go
        WorkoutOverlaySize(FALSE);
        
        AddSplashTextLine("Update Menu");
        OSD_UpdateMenu(hMenu);
        pCalibration->UpdateMenu(hMenu);
        Channels_UpdateMenu(hMenu);
        SetMenuAnalog();

        bDoResize = TRUE;
        
        // if we are in tuner mode
        // either set channel up as requested on the command line
        // or reset it to what is was last time
        if (Providers_GetCurrentSource()->IsInTunerMode())
        {
            if(InitialChannel >= 0)
            {
                Channel_Change(InitialChannel);
            }
            else
            {
                Channel_Reset();
            }
        }
        
        if (InitialTextPage >= 100)
        {
            Setting_SetValue(VBI_GetSetting(CAPTURE_VBI), TRUE);
            VTPage = InitialTextPage;
            VTState = VT_BLACK;
        }

        AddSplashTextLine("Start Video");
        Start_Capture();
    }
    else
    {
        Sleep(2000);
        PostQuitMessage(0);
    }
}

//---------------------------------------------------------------------------
void MainWndOnCreate(HWND hWnd)
{
    char Text[128];
    int i;
    int ProcessorMask;
    SYSTEM_INFO SysInfo;

    pCalibration = new CCalibration();

    pPerf = new CPerf();

    GetSystemInfo(&SysInfo);
    AddSplashTextLine("Table Build");
    AddSplashTextLine("VideoText");

    VBI_Init(); 
    
    Load_Program_List_ASCII();

    AddSplashTextLine("System Analysis");

    sprintf(Text, "Processor %d ", SysInfo.dwProcessorType);
    AddSplashTextLine(Text);
    sprintf(Text, "Number %d ", SysInfo.dwNumberOfProcessors);
    AddSplashTextLine(Text);

    if (SysInfo.dwNumberOfProcessors > 1)
    {
        if (DecodeProcessor == 0)
        {
            if (SysInfo.dwNumberOfProcessors == 2)
            {
                MainProcessor = 0;
                DecodeProcessor = 1;
            }
            if (SysInfo.dwNumberOfProcessors == 3)
            {
                MainProcessor = 0;
                DecodeProcessor = 2;
            }
            if (SysInfo.dwNumberOfProcessors > 3)
            {
                DecodeProcessor = 3;
            }

        }

        AddSplashTextLine("Multi-Processor");
        sprintf(Text, "Main-CPU %d ", MainProcessor);
        AddSplashTextLine(Text);
        sprintf(Text, "DECODE-CPU %d ", DecodeProcessor);
        AddSplashTextLine(Text);
    }

    ProcessorMask = 1 << (MainProcessor);
    i = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);

    Cursor_UpdateVisibility();

    PostMessage(hWnd, INIT_BT, 0, 0);
}

void KillTimers()
{
    KillTimer(hWnd, TIMER_BOUNCE);
    KillTimer(hWnd, TIMER_ORBIT);
    KillTimer(hWnd, TIMER_AUTOSAVE);
    KillTimer(hWnd, TIMER_KEYNUMBER);
    KillTimer(hWnd, TIMER_STATUS);
    KillTimer(hWnd, OSD_TIMER_ID);
    KillTimer(hWnd, OSD_TIMER_REFRESH_ID);
    KillTimer(hWnd, TIMER_HIDECURSOR);
    KillTimer(hWnd, TIMER_VTFLASHER);
    KillTimer(hWnd, TIMER_VTUPDATE);
    KillTimer(hWnd, TIMER_FINDPULL);
}


// basically we want do make sure everything that needs to be done on exit gets 
// done even if one of the functions crashes we should just carry on with the rest
// of the functions
void MainWndOnDestroy()
{
    __try
    {
        KillTimers();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Kill Timers");}

    // stop capture before stopping timneshift to avoid crash
    __try
    {
        LOG(1, "Try Stop_Capture");
        Stop_Capture();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Stop_Capture");}
      
    __try
    {
        LOG(1, "Try CTimeShift::OnStop");

        CTimeShift::OnStop();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error CTimeShift::OnStop");}

    
    // Kill timeshift before muting since it always exits unmuted on cleanup.
    __try
    {
        LOG(1, "Try CTimeShift::OnDestroy");

        CTimeShift::OnDestroy();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error CTimeShift::OnDestroy");}

    __try
    {
        LOG(1, "Try Mute");
        Audio_Mute();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Mute");}

    __try
    {
        LOG(1, "Try CleanUpMemory");
        CleanUpMemory();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error CleanUpMemory");}

    __try
    {
        if(bIsFullScreen == FALSE)
        {
            LOG(1, "Try SaveWindowPos");
            SaveWindowPos(hWnd);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error SaveWindowPos");}
    
    __try
    {
        // save settings
        // must be done before providers are unloaded
        LOG(1, "WriteSettingsToIni");
        WriteSettingsToIni(TRUE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error WriteSettingsToIni");}

    __try
    {
        LOG(1, "Try Providers_Unload");
        Providers_Unload();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Providers_Unload");}

    __try
    {
        LOG(1, "Try StatusBar_Destroy");
        StatusBar_Destroy();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error StatusBar_Destroy");}
    
    __try
    {
        LOG(1, "Try ExitDD");
        ExitDD();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error ExitDD");}

    __try
    {
        // unload plug-ins
        UnloadDeinterlacePlugins();
        UnloadFilterPlugins();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Unload plug-ins");}

    __try
    {
        SetKeyboardLock(FALSE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error SetKeyboardLock(FALSE)");}

}

//---------------------------------------------------------------------------
void SetMenuAnalog()
{
    CheckMenuItem(hMenu, ThreadClassId + 1150, MF_CHECKED);
    CheckMenuItem(hMenu, PriorClassId + 1160, MF_CHECKED);

    CheckMenuItemBool(hMenu, IDM_TREADPRIOR_0, (ThreadClassId == 0));
    CheckMenuItemBool(hMenu, IDM_TREADPRIOR_1, (ThreadClassId == 1));
    CheckMenuItemBool(hMenu, IDM_TREADPRIOR_2, (ThreadClassId == 2));
    CheckMenuItemBool(hMenu, IDM_TREADPRIOR_3, (ThreadClassId == 3));
    CheckMenuItemBool(hMenu, IDM_TREADPRIOR_4, (ThreadClassId == 4));

    CheckMenuItemBool(hMenu, IDM_PRIORCLASS_0, (PriorClassId == 0));
    CheckMenuItemBool(hMenu, IDM_PRIORCLASS_1, (PriorClassId == 1));
    CheckMenuItemBool(hMenu, IDM_PRIORCLASS_2, (PriorClassId == 2));

    CheckMenuItemBool(hMenu, IDM_TOGGLECURSOR, bShowCursor);
    EnableMenuItem(hMenu,IDM_TOGGLECURSOR, bAutoHideCursor?MF_GRAYED:MF_ENABLED);
    CheckMenuItemBool(hMenu, IDM_AUTOHIDE_CURSOR, bAutoHideCursor);
    CheckMenuItemBool(hMenu, IDM_STATUSBAR, bDisplayStatusBar);
    CheckMenuItemBool(hMenu, IDM_TOGGLE_MENU, bShowMenu);
    CheckMenuItemBool(hMenu, IDM_ON_TOP, bAlwaysOnTop);
    CheckMenuItemBool(hMenu, IDM_ALWAYONTOPFULLSCREEN, bAlwaysOnTopFull);
    CheckMenuItemBool(hMenu, IDM_SPLASH_ON_STARTUP, bDisplaySplashScreen);
    CheckMenuItemBool(hMenu, IDM_AUTOHIDE_OSD, Setting_GetValue(OSD_GetSetting(OSD_AUTOHIDE_SCREEN)));
    CheckMenuItemBool(hMenu, IDM_KEYBOARDLOCK, bKeyboardLock);

    AspectRatio_SetMenu(hMenu);
    FD60_SetMenu(hMenu);
    OutThreads_SetMenu(hMenu);
    Deinterlace_SetMenu(hMenu);
    Filter_SetMenu(hMenu);
    VBI_SetMenu(hMenu);
    Channels_SetMenu(hMenu);
    OSD_SetMenu(hMenu);
    FD_Common_SetMenu(hMenu);
    Timing_SetMenu(hMenu);
    MixerDev_SetMenu(hMenu);
    Audio_SetMenu(hMenu);
    VT_SetMenu(hMenu);
    Providers_SetMenu(hMenu);

    CTimeShift::OnSetMenu(hMenu);
    if(pCalibration)
    {
        pCalibration->SetMenu(hMenu);
    }
}

HMENU GetOrCreateSubSubMenu(int SubId, int SubSubId, LPCSTR szMenuText)
{
    if(hMenu != NULL)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, SubId);
        if(hSubMenu != NULL)
        {
            HMENU hSubSubMenu = GetSubMenu(hSubMenu, SubSubId);
            if(hSubSubMenu != NULL)
            {
                return hSubSubMenu;
            }
            else
            {
                if(ModifyMenu(hSubMenu, SubSubId, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), szMenuText))
                {
                    return GetSubMenu(hSubMenu, SubSubId);
                }
                else
                {
                    return NULL;
                }
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}

HMENU GetOrCreateSubSubSubMenu(int SubId, int SubSubId, int SubSubSubId, LPCSTR szMenuText)
{
    if(hMenu != NULL)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, SubId);
        if(hSubMenu != NULL)
        {
            HMENU hSubSubMenu = GetSubMenu(hSubMenu, SubSubId);
            if(hSubSubMenu != NULL)
            {
                HMENU hSubSubSubMenu = GetSubMenu(hSubSubMenu, SubSubSubId);
                if(hSubSubSubMenu != NULL)
                {
                    return hSubSubSubMenu;
                }
                else
                {
                    if(ModifyMenu(hSubSubMenu, SubSubSubId, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT)CreatePopupMenu(), szMenuText))
                    {
                        return GetSubMenu(hSubSubMenu, SubSubSubId);
                    }
                    else
                    {
                        return NULL;
                    }
                }
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}


HMENU GetFiltersSubmenu()
{
    return GetOrCreateSubSubMenu(4, 2, "Select &Filters");
}

HMENU GetFilterSettingsSubmenu()
{
    return GetOrCreateSubSubMenu(4, 3, "Filter &Settings");
}

HMENU GetVideoDeinterlaceSubmenu()
{
    return GetOrCreateSubSubMenu(4, 1, "&Deinterlace Mode");
}

HMENU GetChannelsSubmenu()
{
    return GetOrCreateSubSubMenu(1, 1, "Channel S&elect");
}

HMENU GetOSDSubmenu1()
{
    return GetOrCreateSubSubSubMenu(2, 8, 2, "&Show Screen");
}

HMENU GetOSDSubmenu2()
{
    return GetOrCreateSubSubSubMenu(2, 8, 3, "A&ctivate Screen");
}

HMENU GetPatternsSubmenu()
{
    return GetOrCreateSubSubSubMenu(4, 7, 0, "Test &Patterns");
}

//---------------------------------------------------------------------------
void CleanUpMemory()
{
    Mixer_Exit();
    VBI_Exit();
    if ((hMenu != NULL) && (GetMenu(hWnd) == NULL))
    {
        DestroyMenu(hMenu);
    }
    Channels_Exit();
    delete pCalibration;
    pCalibration = NULL;
    delete pPerf;
    pPerf = NULL;
}

//---------------------------------------------------------------------------
// Stops video overlay - 2000-10-31 Added by Mark Rejhon
// This ends the video overlay from operating, so that end users can
// write scripts that sends this special message to safely stop the video
// before switching computer resolutions or timings.
// This is also called during a Suspend operation
void Overlay_Stop(HWND hWnd)
{
    RECT winRect;
    HDC hDC;
    GetClientRect(hWnd, &winRect);
    hDC = GetDC(hWnd);
    PaintColorkey(hWnd, FALSE, hDC, &winRect);
    ReleaseDC(hWnd,hDC);
    Stop_Capture();
    Overlay_Destroy();
    InvalidateRect(hWnd, NULL, FALSE);
}

//---------------------------------------------------------------------------
// Restarts video overlay - 2000-10-31 Added by Mark Rejhon
// This reinitializes the video overlay to continue operation,
// so that end users can write scripts that sends this special message
// to safely restart the video after a resolution or timings change.
// This is also called during a Resume operation
void Overlay_Start(HWND hWnd)
{
    InvalidateRect(hWnd, NULL, FALSE);
    Overlay_Create();
    Reset_Capture();
}

//---------------------------------------------------------------------------
// Show text on both OSD and statusbar
void ShowText(HWND hWnd, LPCTSTR szText)
{
    StatusBar_ShowText(STATUS_TEXT, szText);
    OSD_ShowText(hWnd, szText, 0);
}

//----------------------------------------------------------------------------
// Updates the window position/window state and enable/disable titlebar 
// as necessary.  This function should be globally used for everytime 
// you want to update the window everytime you have enabled/disabled the 
// statusbar, menus, full screen state, etc.
//
// This allows for more cosmetic handling - including the ability to 
// startup directly to maximized without any intermediate cosmetic
// glitches during startup.
//
void UpdateWindowState()
{
    if(bIsFullScreen == TRUE)
    {
        SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE);
        SetMenu(hWnd, NULL);
        StatusBar_ShowWindow(FALSE);
        SetWindowPos(hWnd,
                    bAlwaysOnTopFull?HWND_TOPMOST:HWND_NOTOPMOST,
                    0,
                    0,
                    GetSystemMetrics(SM_CXSCREEN),
                    GetSystemMetrics(SM_CYSCREEN),
                    SWP_SHOWWINDOW);
    }
    else
    {
        if(bShowMenu == TRUE)
        {
            SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
            SetMenu(hWnd, hMenu);
        }
        else
        {
            SetWindowLong(hWnd, GWL_STYLE, WS_THICKFRAME | WS_POPUP | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
            SetMenu(hWnd, NULL);
        }
        StatusBar_ShowWindow(bDisplayStatusBar);
        SetWindowPos(hWnd,bAlwaysOnTop?HWND_TOPMOST:HWND_NOTOPMOST,
                    0,0,0,0,
                    SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
    }
}

BOOL IsStatusBarVisible()
{
    return (bDisplayStatusBar == TRUE && bIsFullScreen == FALSE);
}

///////////////////////////////////////////////////////////////////////////////
void SetThreadProcessorAndPriority()
{
    DWORD rc;
    int ProcessorMask;

    ProcessorMask = 1 << (DecodeProcessor);
    rc = SetThreadAffinityMask(GetCurrentThread(), ProcessorMask);
    
    if (ThreadClassId == 0)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    else if (ThreadClassId == 1)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    else if (ThreadClassId == 2)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    else if (ThreadClassId == 3)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    else if (ThreadClassId == 4)
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}

void Cursor_SetVisibility(BOOL bVisible)
{
    static int nCursorIndex = 1;
    if(bVisible)
    {
        while(nCursorIndex < 1)
        {
            ShowCursor(TRUE);
            nCursorIndex++;
        }
    }
    else
    {
        while(nCursorIndex > 0)
        {
            ShowCursor(FALSE);
            nCursorIndex--;
        }
    }
}

void Cursor_UpdateVisibility()
{
    KillTimer(hWnd, TIMER_HIDECURSOR);
    if (bInMenuOrDialogBox)
    {
        Cursor_SetVisibility(TRUE);
    }
    else if (!bAutoHideCursor)
    {
        if (bIsFullScreen)
        {
            Cursor_SetVisibility(FALSE);
        }
        else
        {
            Cursor_SetVisibility(bShowCursor);
        }
    }
    else
    {
        Cursor_SetVisibility(TRUE);
        SetTimer(hWnd, TIMER_HIDECURSOR, TIMER_HIDECURSOR_MS, NULL);
    }
}

int Cursor_SetType(int type)
{
    HCURSOR hCur;
    switch (type)
    {
    case CURSOR_HAND:
        hCur = hCursorHand;
        break;
    default:
        hCur = hCursorDefault;
        break;
    }

    SetClassLong(hWnd, GCL_HCURSOR, (long) hCur);

    return true;
}

void Cursor_VTUpdate(bool PosValid, int x, int y)
{
    POINT pt;
    int a;

    if (VTState == VT_OFF)
    {
        Cursor_SetType(CURSOR_DEFAULT);
    }
    else 
    {
        if (!PosValid) 
        {
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            x = pt.x; y = pt.y;
        }

        a = VT_GetPageNumberAt(VTPage-100, VTSubPage, x, y);
        if ((a >= 100) && (a <= 899))
        {
            Cursor_SetType(CURSOR_HAND);
        }
        else
        {
            Cursor_SetType(CURSOR_DEFAULT);
        }
    }
}

void SetDirectoryToExe()
{
    char szDriverPath[MAX_PATH];
    char* pszName;

    if (!GetModuleFileName(NULL, szDriverPath, sizeof(szDriverPath)))
    {
        ErrorBox("Cannot get module file name");
        return;
    }

    pszName = szDriverPath + strlen(szDriverPath);
    while (pszName >= szDriverPath && *pszName != '\\')
    {
        *pszName-- = 0;
    }

    SetCurrentDirectory(szDriverPath);
}

/** Process command line parameters and return them

    Routine process the command line and returns them in argv/argc format.
    Modifies the incoming string
*/
int ProcessCommandLine(char* CommandLine, char* ArgValues[], int SizeArgv)
{
   int ArgCount = 0;
   char* pCurrentChar = CommandLine;

   while (*pCurrentChar && ArgCount < SizeArgv)
   {
      // Skip any preceeding spaces.
      while (*pCurrentChar && isspace(*pCurrentChar))
      {
         pCurrentChar++;
      }
      // If the parameter starts with a double quote, copy until
      // the end quote.
      if (*pCurrentChar == '"')
      {
         pCurrentChar++;  // Skip the quote
         ArgValues[ArgCount++] = pCurrentChar;  // Save the start in the argument list.
         while (*pCurrentChar && *pCurrentChar != '"')
         {
            pCurrentChar++;
         }
         if (*pCurrentChar)
         {
             *pCurrentChar++ = '\0';   // Replace the end quote
         }
      }
      else if (*pCurrentChar) // Normal parameter, continue until white found (or end)
      {
         ArgValues[ArgCount++] = pCurrentChar++;
         while (*pCurrentChar && !isspace(*pCurrentChar))
         {
             ++pCurrentChar;
         }
         if (*pCurrentChar)
         {
             *pCurrentChar++ = '\0';
         }
      }
   }
   return ArgCount;
}


////////////////////////////////////////////////////////////////////////////
// On Change Functions for settings
/////////////////////////////////////////////////////////////////////////////

BOOL IsFullScreen_OnChange(long NewValue)
{
    bDoResize = FALSE;
    bIsFullScreen = (BOOL)NewValue;

    // make sure that the window has been created
    if(hWnd != NULL)
    {
        if(bIsFullScreen == FALSE)
        {
            SetWindowPos(hWnd, 0, MainWndLeft, MainWndTop, MainWndWidth, MainWndHeight, SWP_SHOWWINDOW);
            if (bDisplayStatusBar == TRUE)
            {
                SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
            }
        }
        else
        {
            SaveWindowPos(hWnd);
        }
        Cursor_UpdateVisibility();
        InvalidateRect(hWnd, NULL, FALSE);
        WorkoutOverlaySize(FALSE);
    }
    bDoResize = TRUE;
    return FALSE;
}

BOOL AlwaysOnTop_OnChange(long NewValue)
{
    bAlwaysOnTop = (BOOL)NewValue;
    WorkoutOverlaySize(FALSE);
    return FALSE;
}

BOOL AlwaysOnTopFull_OnChange(long NewValue)
{
    bAlwaysOnTopFull = (BOOL)NewValue;
    WorkoutOverlaySize(FALSE);
    return FALSE;
}

BOOL DisplayStatusBar_OnChange(long NewValue)
{
    bDisplayStatusBar = (BOOL)NewValue;
    if(bIsFullScreen == FALSE)
    {
        if(bDisplayStatusBar == TRUE)
        {
            SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
        }
        else
        {
            KillTimer(hWnd, TIMER_STATUS);
        }
        WorkoutOverlaySize(TRUE);
    }
    return FALSE;
}

BOOL ShowMenu_OnChange(long NewValue)
{
    bShowMenu = (BOOL)NewValue;
    if(bIsFullScreen == FALSE)
    {
        WorkoutOverlaySize(TRUE);
    }
    return FALSE;
}

BOOL KeyboardLock_OnChange(long NewValue)
{
    bKeyboardLock = (BOOL)NewValue;
    SetKeyboardLock(bKeyboardLock);
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DScalerSettings[DSCALER_SETTING_LASTONE] =
{
    {
        "Window Left", SLIDER, 0, (long*)&MainWndLeft,
        10, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartLeft", NULL,
    },
    {
        "Window Top", SLIDER, 0, (long*)&MainWndTop,
        10, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartTop", NULL,
    },
    {
        "Window Width", SLIDER, 0, (long*)&MainWndWidth,
        649, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartWidth", NULL,
    },
    {
        "Window Height", SLIDER, 0, (long*)&MainWndHeight,
        547, 0, 2048, 1, 1,
        NULL,
        "MainWindow", "StartHeight", NULL,
    },
    {
        "Always On Top (Window)", YESNO, 0, (long*)&bAlwaysOnTop,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysOnTop", AlwaysOnTop_OnChange,
    },
    {
        "Splash Screen", ONOFF, 0, (long*)&bDisplaySplashScreen,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "DisplaySplashScreen", NULL,
    },
    {
        "Full Screen", YESNO, 0, (long*)&bIsFullScreen,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "bIsFullScreen", IsFullScreen_OnChange,
    },
    {
        "Force Full Screen", ONOFF, 0, (long*)&bForceFullScreen,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysForceFullScreen", NULL,
    },
    {
        "Status Bar", ONOFF, 0, (long*)&bDisplayStatusBar,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "StatusBar", DisplayStatusBar_OnChange,
    },
    {
        "Menu", ONOFF, 0, (long*)&bShowMenu,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "Menu", ShowMenu_OnChange,
    },
    {
        "Auto Hide Cursor", ONOFF, 0, (long*)&bAutoHideCursor,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Show", "AutoHideCursor", NULL,
    },
    {
        "Window Processor", SLIDER, 0, (long*)&MainProcessor,
        0, 0, 3, 1, 1,
        NULL,
        "Threads", "WindowProcessor", NULL,
    },
    {
        "Thread Processor", SLIDER, 0, (long*)&DecodeProcessor,
        0, 0, 3, 1, 1,
        NULL,
        "Threads", "DecodeProcessor", NULL,
    },
    {
        "Window Priority", SLIDER, 0, (long*)&PriorClassId,
        0, 0, 2, 1, 1,
        NULL,
        "Threads", "WindowPriority", NULL,
    },
    {
        "Thread Priority", SLIDER, 0, (long*)&ThreadClassId,
        1, 0, 4, 1, 1,
        NULL,
        "Threads", "ThreadPriority", NULL,
    },
    {
        "Autosave settings", ONOFF, 0, (long*)&bUseAutoSave,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "UseAutoSave", NULL,
    },
    {
        "Always On Top (Full Screen)", YESNO, 0, (long*)&bAlwaysOnTopFull,
        TRUE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "AlwaysOnTopFull", AlwaysOnTopFull_OnChange,
    },
    {
        "Show Crash Dialog", ONOFF, 0, (long*)&bShowCrashDialog,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "ShowCrashDialog", NULL,
    },
    {
        "Lock keyboard", ONOFF, 0, (long*)&bKeyboardLock,
        FALSE, 0, 1, 1, 1,
        NULL,
        "MainWindow", "KeyboardLock", KeyboardLock_OnChange,
    },
};

SETTING* DScaler_GetSetting(DSCALER_SETTING Setting)
{
    if(Setting > -1 && Setting < DSCALER_SETTING_LASTONE)
    {
        return &(DScalerSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void DScaler_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(DScalerSettings[i]));
    }
    if(bForceFullScreen)
    {
        bIsFullScreen = TRUE;
    }
    if(bKeyboardLock)
    {
        SetKeyboardLock(TRUE);
    }
}

void DScaler_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(DScalerSettings[i]), bOptimizeFileAccess);
    }
}


