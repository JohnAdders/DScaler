////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.cpp,v 1.61 2001-08-16 21:17:34 laurentg Exp $
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
// Revision 1.60  2001/08/15 17:50:11  laurentg
// UseRGB ini parameter suppressed
// OSD screen concerning card calibration fully modified
// Automatic calibration added (not finished)
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
#include "resource.h"
#include "Other.h"
#include "BT848.h"
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
#include "Tuner.h"
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

HWND hWnd = NULL;
HINSTANCE hInst = NULL;

// Used to call MainWndOnInitBT
#define INIT_BT 1800

BOOL bDoResize = FALSE;

HWND VThWnd;

long WStyle;

BOOL    bShowMenu=TRUE;
HMENU   hMenu;
HMENU   hMenuPopup;
HACCEL  hAccel;

char ChannelString[10];

int MainProcessor=0;
int DecodeProcessor=0;
int PriorClassId = 0;
int ThreadClassId = 1;

BOOL bShowCursor = TRUE;
BOOL bAutoHideCursor = FALSE;

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

HFONT hCurrentFont = NULL;

BOOL bInMenuOrDialogBox = FALSE;
BOOL bIgnoreMouse = FALSE;
BOOL bShowCrashDialog = FALSE;

UINT MsgWheel;

BOOL IsFullScreen_OnChange(long NewValue);
BOOL DisplayStatusBar_OnChange(long NewValue);
void Cursor_UpdateVisibility();
void Cursor_SetVisibility(BOOL bVisible);
const char* GetSourceName(int nVideoSource);
void MainWndOnDestroy();


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

    hInst = hInstance;
    
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetUnhandledExceptionFilter(UnexpectedCrashHandler);

    CPU_SetupFeatureFlag();
    // if we are already runninmg then start up old version
    hPrevWindow = FindWindow((LPCTSTR) DSCALER_APPNAME, NULL);
    if (hPrevWindow != NULL)
    {
        SetFocus(hPrevWindow);
        SetActiveWindow(hPrevWindow);
        SetForegroundWindow(hPrevWindow);
        return FALSE;
    }

    // JA 07/01/2001
    // Required to use slider control
    InitCommonControls();

    // JA 21/12/2000
    // Added single place to setup ini file name
    SetIniFileForSettings(lpCmdLine);
    LoadSettingsFromIni();

    if(bDisplaySplashScreen)
    {
        ShowSpashScreen();
    }

    wc.style = CS_DBLCLKS;      // Allow double click
    wc.lpfnWndProc = (WNDPROC) MainWndProcSafe;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(LONG);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DSCALER));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(0);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = DSCALER_APPNAME;

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }
    hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_DSCALERMENU));

    hMenuPopup = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenuPopup != NULL)
        hMenuPopup = GetSubMenu(hMenuPopup,0);
    if (hMenuPopup != NULL && hMenu != NULL)
    {
        MENUITEMINFO MenuItemInfo;
        HMENU hSubMenu;

        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fMask = MIIM_SUBMENU;

        hSubMenu = GetSubMenu(hMenu, 4);
        if(hSubMenu != NULL)
            hSubMenu = GetSubMenu(hSubMenu, 3);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,0,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 4);
        if(hSubMenu != NULL)
            hSubMenu = GetSubMenu(hSubMenu, 2);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,1,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 4);
        if(hSubMenu != NULL)
            hSubMenu = GetSubMenu(hSubMenu, 4);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,2,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetChannelsSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,3,TRUE,&MenuItemInfo);
        }
        
        hSubMenu = GetVideoDeinterlaceSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,4,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 3);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,5,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetFiltersSubmenu();
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,6,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 4);
        if(hSubMenu != NULL)
            hSubMenu = GetSubMenu(hSubMenu, 5);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,8,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 6);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,9,TRUE,&MenuItemInfo);
        }

        hSubMenu = GetSubMenu(hMenu, 2);
        if(hSubMenu != NULL)
            hSubMenu = GetSubMenu(hSubMenu, 7);
        if(hSubMenu != NULL)
        {
            MenuItemInfo.hSubMenu = hSubMenu;
            SetMenuItemInfo(hMenuPopup,10,TRUE,&MenuItemInfo);
        }
    }

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
    if (!(hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_DSCALER))))
    {
        ErrorBox("Accelerators not Loaded");
    }

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


BOOL WINAPI OnContextMenu(HWND hWnd, int x, int y)
{ 
    RECT rc;                    // client area of window
    POINT pt = { x, y };        // location of mouse click

    // Get the bounding rectangle of the client area.
    GetClientRect(hWnd, &rc);

    // Convert the mouse position to client coordinates.
    ScreenToClient(hWnd, &pt);

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
 
    // Return FALSE if no menu is displayed.
    return FALSE; 
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
    char Text1[128];
    int i;
    long nValue;
    BOOL bDone;

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

        if ( (LOWORD(wParam) > IDM_CHANNEL_SELECT) && (LOWORD(wParam) <= (IDM_CHANNEL_SELECT+MAXPROGS)) )
        {
            PostMessage(hWnd, WM_COMMAND, IDM_CHANNEL_SELECT, LOWORD(wParam)-IDM_CHANNEL_SELECT-1);
            break;
        }
        if ( (LOWORD(wParam) > IDM_OSDSCREEN_SHOW) && (LOWORD(wParam) <= (IDM_OSDSCREEN_SHOW+10)) )
        {
            PostMessage(hWnd, WM_COMMAND, IDM_OSDSCREEN_SHOW, LOWORD(wParam)-IDM_OSDSCREEN_SHOW-1);
            break;
        }
        if ( (LOWORD(wParam) > IDM_OSDSCREEN_ACTIVATE) && (LOWORD(wParam) <= (IDM_OSDSCREEN_ACTIVATE+10)) )
        {
            PostMessage(hWnd, WM_COMMAND, IDM_OSDSCREEN_ACTIVATE, LOWORD(wParam)-IDM_OSDSCREEN_ACTIVATE-1);
            break;
        }
        if ( (LOWORD(wParam) > IDM_PATTERN_SELECT) && (LOWORD(wParam) <= (IDM_PATTERN_SELECT+MAX_TEST_PATTERNS)) )
        {
            PostMessage(hWnd, WM_COMMAND, IDM_PATTERN_SELECT, LOWORD(wParam)-IDM_PATTERN_SELECT-1);
            break;
        }

        switch (LOWORD(wParam))
        {
        case IDM_SETUPCARD:
            Stop_Capture();
            DialogBox(hInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc);
            Card_Init();
            Tuner_Init();
            Reset_Capture();
            break;

        case IDM_VT_PAGE_MINUS:
            if(VTState != VT_OFF)
            {
                if(VTPage >= 100)
                {
                    VTPage--;
                    VT_DoUpdate_Page(VTPage - 100);
                    InvalidateRect(hWnd,NULL,FALSE);
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
                    VTPage++;
                    VT_DoUpdate_Page(VTPage - 100);
                    InvalidateRect(hWnd,NULL,FALSE);
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
                // not yet implemented
            }
            else
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_VT_PAGE_DOWN:
            if(VTState != VT_OFF)
            {
                // not yet implemented
            }
            else
            {
                ProcessAspectRatioSelection(hWnd, LOWORD(wParam));
            }
            break;

        case IDM_CHANNELPLUS:
            if (Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)) == SOURCE_TUNER)
            {
                Channel_Increment();
            }

            break;

        case IDM_CHANNELMINUS:
            if (Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)) == SOURCE_TUNER)
            {
                Channel_Decrement();
            }
            break;

        case IDM_CHANNEL_PREVIOUS:
            if (Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)) == SOURCE_TUNER)
            {
                Channel_Previous();
            }
            break;

        case IDM_CHANNEL_SELECT:
            if (Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)) == SOURCE_TUNER)
            {
                Channel_Change(lParam);
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
            Audio_SetSource(AudioSource);
            break;

        case IDM_TOGGLE_MENU:
            bShowMenu = !bShowMenu;
            WorkoutOverlaySize();
            break;

        case IDM_AUTODETECT:
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
            if(BT848_GetTVFormat()->Is25fps)
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD50_GetSetting(PALFILMFALLBACKMODE)));
            }
            else
            {
                SetVideoDeinterlaceIndex(Setting_GetValue(FD60_GetSetting(NTSCFILMFALLBACKMODE)));
            }
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
            Setting_SetValue(OutThreads_GetSetting(AUTODETECT), FALSE);
            SetFilmDeinterlaceMode((eFilmPulldownMode)(LOWORD(wParam) - IDM_22PULLODD));
            ShowText(hWnd, GetDeinterlaceModeName());
            break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
            break;

        case IDM_BRIGHTNESS_PLUS:
            Setting_Up(BT848_GetSetting(BRIGHTNESS));
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_MINUS:
            Setting_Down(BT848_GetSetting(BRIGHTNESS));
            SendMessage(hWnd, WM_COMMAND, IDM_BRIGHTNESS_CURRENT, 0);
            break;

        case IDM_BRIGHTNESS_CURRENT:
            Setting_OSDShow(BT848_GetSetting(BRIGHTNESS), hWnd);
            break;

        case IDM_KONTRAST_PLUS:
            Setting_Up(BT848_GetSetting(CONTRAST));
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_MINUS:
            Setting_Down(BT848_GetSetting(CONTRAST));
            SendMessage(hWnd, WM_COMMAND, IDM_KONTRAST_CURRENT, 0);
            break;

        case IDM_KONTRAST_CURRENT:
            Setting_OSDShow(BT848_GetSetting(CONTRAST), hWnd);
            break;

        case IDM_USATURATION_PLUS:
            Setting_Up(BT848_GetSetting(SATURATIONU));
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;
        
        case IDM_USATURATION_MINUS:
            Setting_Down(BT848_GetSetting(SATURATIONU));
            SendMessage(hWnd, WM_COMMAND, IDM_USATURATION_CURRENT, 0);
            break;

        case IDM_USATURATION_CURRENT:
            Setting_OSDShow(BT848_GetSetting(SATURATIONU), hWnd);
            break;
        
        case IDM_VSATURATION_PLUS:
            Setting_Up(BT848_GetSetting(SATURATIONV));
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_MINUS:
            Setting_Down(BT848_GetSetting(SATURATIONV));
            SendMessage(hWnd, WM_COMMAND, IDM_VSATURATION_CURRENT, 0);
            break;

        case IDM_VSATURATION_CURRENT:
            Setting_OSDShow(BT848_GetSetting(SATURATIONV), hWnd);
            break;

        case IDM_COLOR_PLUS:
            Setting_Up(BT848_GetSetting(SATURATION));
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_MINUS:
            Setting_Down(BT848_GetSetting(SATURATION));
            SendMessage(hWnd, WM_COMMAND, IDM_COLOR_CURRENT, 0);
            break;

        case IDM_COLOR_CURRENT:
            Setting_OSDShow(BT848_GetSetting(SATURATION), hWnd);
            break;

        case IDM_HUE_PLUS:
            Setting_Up(BT848_GetSetting(HUE));
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_MINUS:
            Setting_Down(BT848_GetSetting(HUE));
            SendMessage(hWnd, WM_COMMAND, IDM_HUE_CURRENT, 0);
            break;

        case IDM_HUE_CURRENT:
            Setting_OSDShow(BT848_GetSetting(HUE), hWnd);
            break;

        case IDM_OVERSCAN_PLUS:
            Setting_Up(Aspect_GetSetting(OVERSCAN));
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_MINUS:
            Setting_Down(Aspect_GetSetting(OVERSCAN));
            SendMessage(hWnd, WM_COMMAND, IDM_OVERSCAN_CURRENT, 0);
            break;

        case IDM_OVERSCAN_CURRENT:
            Setting_OSDShow(Aspect_GetSetting(OVERSCAN), hWnd);
            break;

        case IDM_BDELAY_PLUS:
            Setting_Up(BT848_GetSetting(BDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
            break;

        case IDM_BDELAY_MINUS:
            Setting_Down(BT848_GetSetting(BDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_BDELAY_CURRENT, 0);
            break;

        case IDM_BDELAY_CURRENT:
            Setting_OSDShow(BT848_GetSetting(BDELAY), hWnd);
            break;

        case IDM_HDELAY_PLUS:
            Setting_Up(BT848_GetSetting(HDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_MINUS:
            Setting_Down(BT848_GetSetting(HDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_HDELAY_CURRENT, 0);
            break;

        case IDM_HDELAY_CURRENT:
            Setting_OSDShow(BT848_GetSetting(HDELAY), hWnd);
            break;

        case IDM_VDELAY_PLUS:
            Setting_Up(BT848_GetSetting(VDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_MINUS:
            Setting_Down(BT848_GetSetting(VDELAY));
            SendMessage(hWnd, WM_COMMAND, IDM_VDELAY_CURRENT, 0);
            break;

        case IDM_VDELAY_CURRENT:
            Setting_OSDShow(BT848_GetSetting(VDELAY), hWnd);
            break;

        case IDM_PIXELWIDTH_PLUS:
            Setting_Up(BT848_GetSetting(CURRENTX));
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_MINUS:
            Setting_Down(BT848_GetSetting(CURRENTX));
            SendMessage(hWnd, WM_COMMAND, IDM_PIXELWIDTH_CURRENT, 0);
            break;

        case IDM_PIXELWIDTH_CURRENT:
            Setting_OSDShow(BT848_GetSetting(CURRENTX), hWnd);
            break;

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

        case IDM_L_BALANCE:
            if (bUseMixer == FALSE)
            {
                Setting_Down(Audio_GetSetting(BALANCE));
                sprintf(Text, "BT-Balance %d", Setting_GetValue(Audio_GetSetting(BALANCE)));
                ShowText(hWnd, Text);
            }
            break;

        case IDM_R_BALANCE:
            if (bUseMixer == FALSE)
            {
                Setting_Up(Audio_GetSetting(BALANCE));
                sprintf(Text, "BT-Balance %d", Setting_GetValue(Audio_GetSetting(BALANCE)));
                ShowText(hWnd, Text);
            }
            break;

        case IDM_VOLUMEPLUS:
            if (bUseMixer == FALSE)
            {
                if(Audio_MSP_IsPresent())
                {
                    Setting_Up(Audio_GetSetting(VOLUME));
                    sprintf(Text, "BT-Volume %d", Setting_GetValue(Audio_GetSetting(VOLUME))/ 10);
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
                if(Audio_MSP_IsPresent())
                {
                    Setting_Down(Audio_GetSetting(VOLUME));
                    sprintf(Text, "BT-Volume %d", Setting_GetValue(Audio_GetSetting(VOLUME))/ 10);
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

        case IDM_MSPMODE_3:
        case IDM_MSPMODE_2:
        case IDM_MSPMODE_4:
        case IDM_MSPMODE_5:
        case IDM_MSPMODE_6:
            Setting_SetValue(Audio_GetSetting(MSPMODE), LOWORD(wParam) - (IDM_MSPMODE_2 - 2));
            break;

        case IDM_MAJOR_CARRIER_0:
        case IDM_MAJOR_CARRIER_1:
        case IDM_MAJOR_CARRIER_2:
        case IDM_MAJOR_CARRIER_3:
            Setting_SetValue(Audio_GetSetting(MSPMAJORMODE), LOWORD(wParam) - IDM_MAJOR_CARRIER_0);
            break;

        case IDM_MINOR_CARRIER_0:
        case IDM_MINOR_CARRIER_1:
        case IDM_MINOR_CARRIER_2:
        case IDM_MINOR_CARRIER_3:
        case IDM_MINOR_CARRIER_4:
        case IDM_MINOR_CARRIER_5:
        case IDM_MINOR_CARRIER_6:
        case IDM_MINOR_CARRIER_7:
            Setting_SetValue(Audio_GetSetting(MSPMINORMODE), LOWORD(wParam) - IDM_MINOR_CARRIER_0);
            break;

        case IDM_MSPSTEREO_1:
        case IDM_MSPSTEREO_2:
        case IDM_MSPSTEREO_3:
        case IDM_MSPSTEREO_4:
            Setting_SetValue(Audio_GetSetting(MSPSTEREO), LOWORD(wParam) - (IDM_MSPSTEREO_1 - 1));
            break;

        case IDM_AUTOSTEREO:
            Setting_SetValue(Audio_GetSetting(AUTOSTEREOSELECT), 
                !Setting_GetValue(Audio_GetSetting(AUTOSTEREOSELECT)));
            break;

        case IDM_AUDIO_0:
        case IDM_AUDIO_1:
        case IDM_AUDIO_2:
        case IDM_AUDIO_3:
        case IDM_AUDIO_4:
        case IDM_AUDIO_5:
            AudioSource = (eAudioMuxType)(LOWORD(wParam) - IDM_AUDIO_0);
            switch (AudioSource)
            {
            case AUDIOMUX_TUNER:     ShowText(hWnd, "Audio Input - Tuner");     break;
            case AUDIOMUX_MSP_RADIO: ShowText(hWnd, "Audio Input - MSP/Radio"); break;
            case AUDIOMUX_EXTERNAL:  ShowText(hWnd, "Audio Input - External");  break;
            case AUDIOMUX_INTERNAL:  ShowText(hWnd, "Audio Input - Internal");  break;
            case AUDIOMUX_MUTE:      ShowText(hWnd, "Audio Input - Disabled");  break;
            case AUDIOMUX_STEREO:    ShowText(hWnd, "Audio Input - Stereo");    break;
            }
            Stop_Capture();
            Audio_SetSource(AudioSource);
            Start_Capture();
            break;

        case IDM_SOURCE_TUNER:
        case IDM_SOURCE_COMPOSITE:
        case IDM_SOURCE_SVIDEO:
        case IDM_SOURCE_OTHER1:
        case IDM_SOURCE_OTHER2:
        case IDM_SOURCE_COMPVIASVIDEO:
        case IDM_SOURCE_CCIR656_1:
        case IDM_SOURCE_CCIR656_2:
        case IDM_SOURCE_CCIR656_3:
        case IDM_SOURCE_CCIR656_4:
            nValue = LOWORD(wParam) - IDM_SOURCE_TUNER;
            OSD_ShowText(hWnd, GetSourceName(nValue), 0);
            StatusBar_ShowText(STATUS_KEY, GetSourceName(nValue));
            Setting_SetValue(BT848_GetSetting(VIDEOSOURCE), nValue);
            break;
                
        case IDM_HWINFO:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_HWINFO), hWnd, (DLGPROC) ChipSettingProc);
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
            WorkoutOverlaySize();

            InvalidateRect(hWnd,NULL,FALSE);
            break;

        case IDM_VT_RESET:
            VT_ChannelChange();
            break;

        case IDM_AUDIOSETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_AUDIOSETTINGS), hWnd, AudioSettingProc);
            break;

        case IDM_AUDIOSETTINGS1:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_AUDIOEQUALIZER), hWnd, AudioSettingProc1);
            break;

        case IDM_VIDEOSETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_VIDEOSETTINGS), hWnd, VideoSettingProc);
            break;

        case IDM_ADV_VIDEOSETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ADV_VIDEOSETTINGS), hWnd, AdvVideoSettingProc);
            break;

        case IDM_VPS_OUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_VPSSTATUS), hWnd, VPSInfoProc);
            break;

        case IDM_VT_OUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_VTSTATUS), hWnd, VTInfoProc);
            break;

        case IDM_VBI:
            Stop_Capture();
            Setting_SetValue(VBI_GetSetting(CAPTURE_VBI), 
                !Setting_GetValue(VBI_GetSetting(CAPTURE_VBI)));
            Start_Capture();
            break;

        case IDM_AUTO_FORMAT:
            Setting_SetValue(Timing_GetSetting(AUTOFORMATDETECT), 
                !Setting_GetValue(Timing_GetSetting(AUTOFORMATDETECT)));
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
            WorkoutOverlaySize();
            break;

        case IDM_ALWAYONTOPFULLSCREEN:
            bAlwaysOnTopFull = !bAlwaysOnTopFull;
            WorkoutOverlaySize();
            break;

        case IDM_SPLASH_ON_STARTUP:
            bDisplaySplashScreen = !bDisplaySplashScreen;
            break;

        case IDM_CHANNEL_LIST:
            SendMessage(hWnd, WM_COMMAND, IDM_SOURCE_TUNER, 0);
            DialogBox(hInst, MAKEINTRESOURCE(IDD_CHANNELLIST), hWnd, (DLGPROC) ProgramListProc);
            Channels_UpdateMenu(hMenu);
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

        case IDM_TYPEFORMAT_0:
        case IDM_TYPEFORMAT_1:
        case IDM_TYPEFORMAT_2:
        case IDM_TYPEFORMAT_3:
        case IDM_TYPEFORMAT_4:
        case IDM_TYPEFORMAT_5:
        case IDM_TYPEFORMAT_6:
        case IDM_TYPEFORMAT_7:
            // Video format (NTSC, PAL, etc)
            Setting_SetValue(BT848_GetSetting(TVFORMAT), LOWORD(wParam) - IDM_TYPEFORMAT_0);
            ShowText(hWnd, BT848_GetTVFormat()->szDesc);
            break;

        case ID_SETTINGS_PIXELWIDTH_768:
            Setting_SetValue(BT848_GetSetting(CURRENTX), 768);
            break;

        case ID_SETTINGS_PIXELWIDTH_754:
            Setting_SetValue(BT848_GetSetting(CURRENTX), 754);
            break;

        case ID_SETTINGS_PIXELWIDTH_720:
            Setting_SetValue(BT848_GetSetting(CURRENTX), 720);
            break;
        
        case ID_SETTINGS_PIXELWIDTH_640:
            Setting_SetValue(BT848_GetSetting(CURRENTX), 640);
            break;
        
        case ID_SETTINGS_PIXELWIDTH_384:
            Setting_SetValue(BT848_GetSetting(CURRENTX), 384);
            break;
        
        case ID_SETTINGS_PIXELWIDTH_320:
            Setting_SetValue(BT848_GetSetting(CURRENTX), 320);
            break;
        
        case ID_SETTINGS_PIXELWIDTH_CUSTOM:
            Setting_SetValue(BT848_GetSetting(CURRENTX), 
                Setting_GetValue(BT848_GetSetting(CUSTOMPIXELWIDTH)));
            break;

        case IDM_JUDDERTERMINATOR:
            Stop_Capture();
            Setting_SetValue(OutThreads_GetSetting(DOACCURATEFLIPS), 
                !Setting_GetValue(OutThreads_GetSetting(DOACCURATEFLIPS)));
            Start_Capture();
            break;

        case IDM_VT_UK:
            Stop_Capture();
            VT_SetCodePage(VT_UK_CODE_PAGE);
            Start_Capture();
            break;

        case IDM_VT_FRENCH:
            Stop_Capture();
            VT_SetCodePage(VT_FRENCH_CODE_PAGE);
            Start_Capture();
            break;

        case IDM_VT_CZECH:
            Stop_Capture();
            VT_SetCodePage(VT_CZECH_CODE_PAGE);
            Start_Capture();
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

        case IDM_TAKESTILL:
            Pause_Capture();
            Sleep(100);
            SaveStill();
            ShowText(hWnd, "Snapshot");
            UnPause_Capture();
            Sleep(100);
            break;

        case IDM_TAKESTREAMSNAP:
            RequestStreamSnap();
            break;

        case IDM_TSOPTIONS:
            TimeShift::OnOptions();
            break;

        case IDM_TSRECORD:
            if (TimeShift::OnRecord())
            {
                ShowText(hWnd, "Recording");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSSTOP:
            if (TimeShift::OnStop())
            {
                ShowText(hWnd, "Stopped");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPLAY:
            if (TimeShift::OnPlay())
            {
                ShowText(hWnd, "Playing");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPAUSE:
            if (TimeShift::OnPause())
            {
                ShowText(hWnd, "Paused");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSFFWD:
            if (TimeShift::OnFastForward())
            {
                ShowText(hWnd, "Scanning >>>");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSRWND:
            if (TimeShift::OnFastBackward())
            {
                ShowText(hWnd, "Scanning <<<");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSNEXT:
            if (TimeShift::OnGoNext())
            {
                ShowText(hWnd, "Next Clip");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_TSPREV:
            if (TimeShift::OnGoPrev())
            {
                ShowText(hWnd, "Previous Clip");
                TimeShift::OnSetMenu(hMenu);
            }
            break;

        case IDM_SHOW_OSD:
            OSD_ShowNextInfosScreen(hWnd, 0);
            break;

        case IDM_OSDSCREEN_SHOW:
            OSD_ShowInfosScreen(hWnd, lParam, 0);
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
                    VT_Redraw(hWnd, sPaint.hdc);
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

        case IDM_BT848_SETTINGS:
            BT848_ShowUI();
            break;
        
        case IDM_TIMING_SETTNGS:
            Timing_ShowUI();
            break;

        case IDM_OVERLAY_SETTINGS:
            Other_ShowUI();
            break;

        case IDM_GAMMA_SETTINGS:
            Filter_ShowUI("Gamma");
            break;

        case IDM_NOISE_SETTINGS:
            Filter_ShowUI("Noise");
            break;

        case IDM_LOGOKILL_SETTINGS:
            Filter_ShowUI("Logo Killer");
            break;

        case IDM_LINCORR_SETTINGS:
            Filter_ShowUI("Linear Correction");
            break;

        case IDM_SHARPNESS_SETTINGS:
            Filter_ShowUI("Sharpness");
            break;

        case IDM_ASPECT_SETTINGS:
            Aspect_ShowUI();
            break;

        case IDM_HELP_HOMEPAGE:
            ShellExecute(hWnd, "open", "http://deinterlace.sourceforge.net/", NULL, NULL, SW_SHOWNORMAL);
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
            HtmlHelp(hWnd, "DScaler.chm::/DScaler_Readme.htm", HH_DISPLAY_TOPIC, 0);
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
//      Overlay_Clean();
//      BT848_ResetHardware();
//      BT848_SetGeoSize();
//      WorkoutOverlaySize();
//      Start_Capture();
//      Sleep(100);
//      Audio_SetSource(AudioSource);
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
        
        switch (LOWORD(wParam))
        {
        //-------------------------------
        case TIMER_MSP:
            Audio_MSP_Watch_Mode();
            if (bDisplayStatusBar == TRUE)
            {
                Audio_MSP_Print_Mode();
            }
            break;
        //-------------------------------
        case TIMER_STATUS:
            if (!BT848_IsVideoPresent())
            {
                StatusBar_ShowText(STATUS_TEXT, "No Video Signal Found");
            }
            else
            {
                Text[0] = 0x00;
                if (*VT_GetStation() != 0x00)
                {
                    sprintf(Text, "%s ", VT_GetStation());
                    VT_ResetStation();
                }
                else if (VPSLastName[0] != 0x00)
                {
                    sprintf(Text, "%s ", VPSLastName);
                    VPSLastName[0] = 0x00;
                }

                strcpy(Text1, Text);

                if (Setting_GetValue(Audio_GetSetting(SYSTEMINMUTE)) == TRUE)
				{
                    sprintf(Text1, "Volume Mute");
				}
                StatusBar_ShowText(STATUS_TEXT, Text1);
            }
            break;
        //-------------------------------
        case TIMER_KEYNUMBER:
            KillTimer(hWnd, TIMER_KEYNUMBER);
            if(VTState != VT_OFF)
            {
                VTPage = atoi(ChannelString);
                if(VTPage >= 100 && VTPage < 900)
                {
                    VT_DoUpdate_Page(VTPage - 100);
                    InvalidateRect(hWnd, NULL, FALSE);
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
            WorkoutOverlaySize(); // Takes care of everything...
            break;
        //-------------------------------
        case TIMER_HIDECURSOR:
            KillTimer(hWnd, TIMER_HIDECURSOR);
            if (!bInMenuOrDialogBox)
                Cursor_SetVisibility(FALSE);
            break;
        }
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
                    WorkoutOverlaySize();
                }
                break;
            case SIZE_MINIMIZED:
                Overlay_Update(NULL, NULL, DDOVER_HIDE);
                break;
            case SIZE_RESTORED:
                InvalidateRect(hWnd, NULL, FALSE);
                WorkoutOverlaySize();
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
            WorkoutOverlaySize();
        }
        break;

    case WM_CHAR:
        if (Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)) == SOURCE_TUNER || VTState != VT_OFF)
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
                PaintColorkey(hWnd, TRUE, sPaint.hdc, &sPaint.rcPaint);
                VT_Redraw(hWnd, sPaint.hdc);
            }
            else
            {
                PaintColorkey(hWnd, TRUE, sPaint.hdc, &sPaint.rcPaint);
                OSD_Redraw(hWnd, sPaint.hdc);
            }
            EndPaint(hWnd, &sPaint);
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
    }
}


//---------------------------------------------------------------------------
void MainWndOnInitBT(HWND hWnd)
{
    int i;
    BOOL bInitOK = FALSE;

    AddSplashTextLine("Hardware Init");

    if (BT848_FindTVCard(hWnd) == TRUE)
    {
        AddSplashTextLine(BT848_ChipType());
        if(InitDD(hWnd) == TRUE)
        {
            if(Overlay_Create() == TRUE)
            {
                if (BT848_MemoryInit() == TRUE)
                {
                    bInitOK = TRUE;
                }
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
        AddSplashTextLine("Check TV card");
        if(Setting_GetValue(TVCard_GetSetting(CURRENTCARDTYPE)) == TVCARD_UNKNOWN)
        {
            HideSplashScreen();
            TVCard_FirstTimeSetupHardware(hInst, hWnd);
        }

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

        if (Tuner_Init() == TRUE)
        {
            AddSplashTextLine("Tuner OK");
        }
        else
        {
            AddSplashTextLine("No Tuner");
        }

        // do any card related startup routines
        Card_Init();

        // MAE 8 Dec 2000 Start of change
        // JA 8 Jan 2001 Tidied up

        if (Audio_MSP_Init(0x80, 0x81) == TRUE)
        {
            AddSplashTextLine("MSP Device OK");
        }
        else
        {
            AddSplashTextLine("No MSP Device");
        }

        AddSplashTextLine("Setup Mixer");
        Mixer_Init();

        AddSplashTextLine("Start Timers");
        if (Audio_MSP_IsPresent() == TRUE)
        {
            SetTimer(hWnd, TIMER_MSP, TIMER_MSP_MS, NULL);
        }
        if(bIsFullScreen == FALSE && bDisplayStatusBar == TRUE)
        {
            SetTimer(hWnd, TIMER_STATUS, TIMER_STATUS_MS, NULL);
        }

        if(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)) == SOURCE_TUNER)
        {
            AddSplashTextLine("Set Channel");
            Channel_SetCurrent();
        }

        StatusBar_ShowText(STATUS_KEY, GetSourceName(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE))));

        // do final setup routines for any files
        // basically where we need the hWnd to be set
        AddSplashTextLine("Setup Aspect Ratio");
        Aspect_FinalSetup();

        // OK we're ready to go
        AddSplashTextLine("Reset hardware");
        BT848_ResetHardware();
        BT848_SetGeoSize();
        WorkoutOverlaySize();
        Audio_SetSource(AudioSource);
        
        AddSplashTextLine("Update Menu");
        Channels_UpdateMenu(hMenu);
        OSD_UpdateMenu(hMenu);
        pCalibration->UpdateMenu(hMenu);
        SetMenuAnalog();

        bDoResize = TRUE;

        AddSplashTextLine("Start Video");
        Start_Capture();
    }
    else
    {
        CleanUpMemory();
        BT848_Close();
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
    KillTimer(hWnd, TIMER_MSP);
    KillTimer(hWnd, TIMER_STATUS);
    KillTimer(hWnd, OSD_TIMER_ID);
    KillTimer(hWnd, OSD_TIMER_REFRESH_ID);
    KillTimer(hWnd, TIMER_HIDECURSOR);
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

    // Kill timeshift before muting since it always exits unmuted on cleanup.
	__try
	{
		LOG(1, "Try TimeShift::OnDestroy");

        TimeShift::OnDestroy();
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error TimeShift::OnDestroy");}

    __try
    {
        LOG(1, "Try Mute");
        Audio_Mute();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Mute");}

    __try
    {
        LOG(1, "Try Stop_Capture");
        Stop_Capture();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Stop_Capture");}

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
        LOG(1, "Try BT848_Close");
        BT848_Close();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error BT848_Close");}

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
        // save settings
        LOG(1, "WriteSettingsToIni");
        WriteSettingsToIni(FALSE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error WriteSettingsToIni");}

    __try
    {
        // unload plug-ins
        UnloadDeinterlacePlugins();
        UnloadFilterPlugins();
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {LOG(1, "Error Unload plug-ins");}

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
    CheckMenuItemBool(hMenu, IDM_ON_TOP, bAlwaysOnTop);
    CheckMenuItemBool(hMenu, IDM_ALWAYONTOPFULLSCREEN, bAlwaysOnTopFull);
    CheckMenuItemBool(hMenu, IDM_SPLASH_ON_STARTUP, bDisplaySplashScreen);
    CheckMenuItemBool(hMenu, IDM_AUTOHIDE_OSD, Setting_GetValue(OSD_GetSetting(OSD_AUTOHIDE_SCREEN)));

    AspectRatio_SetMenu(hMenu);
    FD60_SetMenu(hMenu);
    OutThreads_SetMenu(hMenu);
    Deinterlace_SetMenu(hMenu);
    Filter_SetMenu(hMenu);
    BT848_SetMenu(hMenu);
    TVCard_SetMenu(hMenu);
    VBI_SetMenu(hMenu);
    Channels_SetMenu(hMenu);
    OSD_SetMenu(hMenu);
    FD_Common_SetMenu(hMenu);
    Timing_SetMenu(hMenu);
    MixerDev_SetMenu(hMenu);
    Audio_SetMenu(hMenu);
    VT_SetMenu(hMenu);
    TimeShift::OnSetMenu(hMenu);
    if(pCalibration)
    {
        pCalibration->SetMenu(hMenu);
    }
}

HMENU GetFiltersSubmenu()
{
    HMENU hSubMenu;

    if(hMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hMenu, 4);
    if(hSubMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 6);
    return hSubMenu;
}

HMENU GetVideoDeinterlaceSubmenu()
{
    HMENU hSubMenu;

    if(hMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hMenu, 4);
    if(hMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 1);
    return hSubMenu;
}

HMENU GetChannelsSubmenu()
{
    HMENU hSubMenu;

    if(hMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hMenu, 1);
    if(hSubMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 1);
    return hSubMenu;
}

HMENU GetOSDSubmenu1()
{
    HMENU hSubMenu;

    if(hMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hMenu, 2);
    if(hSubMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 7);
    if(hSubMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 2);
    return hSubMenu;
}

HMENU GetOSDSubmenu2()
{
    HMENU hSubMenu;

    if(hMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hMenu, 2);
    if(hSubMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 7);
    if(hSubMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 3);
    return hSubMenu;
}

HMENU GetPatternsSubmenu()
{
    HMENU hSubMenu;

    if(hMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hMenu, 5);
    if(hSubMenu == NULL) return NULL;
    hSubMenu = GetSubMenu(hSubMenu, 6);
    return hSubMenu;
}

//---------------------------------------------------------------------------
void CleanUpMemory()
{
    Mixer_Exit();
    VBI_Exit();
    BT848_MemoryFree();
    if ((hMenu != NULL) && (GetMenu(hWnd) == NULL))
    {
        DestroyMenu(hMenu);
    }
    if (hMenuPopup != NULL)
    {
        DestroyMenu(hMenuPopup);
    }
    Channels_Exit();
    delete pCalibration;
    pCalibration = NULL;
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

const char* GetSourceName(int nVideoSource)
{
    switch (nVideoSource)
    {
    case SOURCE_TUNER:         return Channel_GetName(); break;
    case SOURCE_COMPOSITE:     return "Composite"; break;
    case SOURCE_SVIDEO:        return "S-Video"; break;
    case SOURCE_OTHER1:        return "Other 1"; break;
    case SOURCE_OTHER2:        return "Other 2"; break;
    case SOURCE_COMPVIASVIDEO: return "Composite via S-Video"; break;
    case SOURCE_CCIR656_1:     return "CCIR656 1"; break;
    case SOURCE_CCIR656_2:     return "CCIR656 2"; break;
    case SOURCE_CCIR656_3:     return "CCIR656 3"; break;
    case SOURCE_CCIR656_4:     return "CCIR656 4"; break;
    }
    return "Unknown";
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
        SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        SetMenu(hWnd, (bShowMenu == TRUE)?hMenu:NULL);
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
            Cursor_SetVisibility(FALSE);
        else
            Cursor_SetVisibility(bShowCursor);
    }
    else
    {
        Cursor_SetVisibility(TRUE);
        SetTimer(hWnd, TIMER_HIDECURSOR, TIMER_HIDECURSOR_MS, NULL);
    }
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
        WorkoutOverlaySize();
    }
    bDoResize = TRUE;
    return FALSE;
}

BOOL AlwaysOnTop_OnChange(long NewValue)
{
    bAlwaysOnTop = (BOOL)NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL AlwaysOnTopFull_OnChange(long NewValue)
{
    bAlwaysOnTopFull = (BOOL)NewValue;
    WorkoutOverlaySize();
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
        WorkoutOverlaySize();
    }
    return FALSE;
}

BOOL ShowMenu_OnChange(long NewValue)
{
    bShowMenu = (BOOL)NewValue;
    if(bIsFullScreen == FALSE)
    {
        WorkoutOverlaySize();
    }
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
        FALSE, 0, 1, 1, 1,
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
}

void DScaler_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < DSCALER_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(DScalerSettings[i]), bOptimizeFileAccess);
    }
}

