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
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file OSD.cpp OSD Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "OSD.h"
#include "AspectRatio.h"
#include "OverlayOutput.h"
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
#include "SettingsMaster.h"

using namespace std;

#define OSD_COLOR_TITLE     RGB(255,150,150)
#define OSD_COLOR_SECTION   RGB(150,150,255)
#define OSD_COLOR_CURRENT   RGB(200,150,0)

SettingStringValue OSD_szFontName;


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
    TCHAR           szText[512];        // Text of OSD
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
    TCHAR   name[24];       // Name of the screen
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
int OSD_CutLines(LPTSTR, long);
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
    { _T("Card calibration screen"), TRUE,  FALSE, 250,                     TRUE,  TRUE,  OSD_RefreshCalibrationScreen },
    { _T("Short prog screen"),       TRUE,  TRUE,  0,                       TRUE,  FALSE, OSD_DisplayProgrammeInfos },
    { _T("Detailed prog screen"),    TRUE,  FALSE, 500,                     TRUE,  TRUE,  OSD_RefreshCurrentProgrammeScreen },
    { _T("Prog browser screen"),     TRUE,  FALSE, 250,                     TRUE,  TRUE,  OSD_RefreshProgrammesScreen },
    { _T("General screen"),          FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, TRUE,  FALSE, OSD_RefreshGeneralScreen },
    { _T("Statistics screen"),       FALSE, TRUE,  1000,                    TRUE,  FALSE, OSD_RefreshStatisticsScreen },
    { _T("WSS decoding screen"),     FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, TRUE,  FALSE, OSD_RefreshWSSScreen },
//  { _T("AR screen"),               FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, FALSE, FALSE, OSD_RefreshARScreen },
#ifdef USE_PERFORMANCE_STATS
    { _T("Developer screen"),        FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, FALSE, FALSE, OSD_RefreshDeveloperScreen },
#endif
};


// OSD background names
static const char* OSD_szBackgroundNames[OSDB_LASTONE] =
{
    "Transparent",
    "Block",
    "Shaded",
};


static TCHAR TextLines[128][512];


//---------------------------------------------------------------------------
// External: Displays the specified OSD text
void OSD_ShowText(const tstring& Text, double dSize, BOOL bPersistent, BOOL bOverride)
{
    if (bOverride)
    {
        OSD_bOverride = !Text.empty();
    }
    else if (OSD_bOverride)
    {
        return;
    }

    if (Text.empty())
    {
        OSD_Clear();
        return;
    }

    TOSDCommand* pOSDCommand;

    pOSDCommand = (TOSDCommand*)malloc(sizeof(TOSDCommand));

    pOSDCommand->uCommand = OSDC_SHOW_TEXT;
    pOSDCommand->uFlags = 0;
    pOSDCommand->dSize = dSize;

    size_t nDataLength = Text.length() + 1;

    LPTSTR pszText = (LPTSTR)malloc(nDataLength);
    memcpy(pszText, Text.c_str(), nDataLength);

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
void OSD_ShowTextPersistent(const tstring& Text, double dSize)
{
    OSD_ShowText(Text, dSize, TRUE);
}


//---------------------------------------------------------------------------
// External: Override all previous OSD text, and force this current OSD text
// External: to override all other OSD text showings (done by the above functions).
// External: This is useful for external programs to override DScaler's own OSD text
// External: for its own controls.
void OSD_ShowTextOverride(const tstring& Text, double dSize)
{
    OSD_ShowText(Text, dSize, FALSE, TRUE);
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
        LPCTSTR lpComments = pSource->GetComments();

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
            LPTSTR pszText = (LPTSTR)pOSDCommand->dwParam;

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
    int i;
    for (i = OSD_IdxCurrentScreen + 1; i < nScreens; i++)
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
        LOGD(_T("OSD_RefreshInfosScreen in %ld ms\n"), (int)(SecondCount.QuadPart -
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
    tstring     szCurrentFont;
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
        if(OSD_szFontName)
        {
            szCurrentFont = OSD_szFontName;
            hOSDFont = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, dwQuality, DEFAULT_PITCH | FF_DONTCARE, szCurrentFont.c_str());
        }
        if (hOSDFont == NULL)
        {
            // Fallback to Arial
            szCurrentFont = _T("Arial");
            hOSDFont = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont.c_str());
            if (hOSDFont == NULL)
            {
                // Otherwise, fallback to any available font
                szCurrentFont = _T("");
                hOSDFont = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont.c_str());
            }
        }
        if (hOSDFont == NULL)
        {
            ErrorBox(_T("Failed To Create OSD Font"));
        }
        hOSDFontOutline = CreateFont(nFontSize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, szCurrentFont.c_str());

        if (hOSDFontOutline == NULL)
        {
            hOSDFontOutline = hOSDFont;
        }

        hSaveFont = (HFONT)SelectObject(hDC, hOSDFontOutline);
        if (hSaveFont != NULL)
        {
            GetTextExtentPoint32(hDC, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText), &sizeText);

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
                    SetBkColor(hDC, GetActiveOutput()->Overlay_GetCorrectedColor(hDC));

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
                TextOut(hDC, lpRect->left+nXpos - 2, lpRect->top+ nYpos, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos + 2, lpRect->top+ nYpos, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos, lpRect->top+ nYpos - 2, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos, lpRect->top+ nYpos + 2, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos - 1, lpRect->top+ nYpos - 1, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos + 1, lpRect->top+ nYpos - 1, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos - 1, lpRect->top+ nYpos + 1, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
                TextOut(hDC, lpRect->left+nXpos + 1, lpRect->top+ nYpos + 1, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));
            }

            // Draw OSD text
            if (SelectObject(hDC, hOSDFont))
            {
                SetTextColor(hDC, OSD_Text[i].TextColor);
                SetBkColor(hDC, OSD_Text[i].BackgroundColor);
                TextOut(hDC, lpRect->left+nXpos, lpRect->top+ nYpos, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText));

                // MRS 2-23-01 Calculate rectnagle for the entire OSD
                // so we do not invalidate the entire window to remove it.
                SIZE sz;
                GetTextExtentExPoint(hDC, OSD_Text[i].szText, _tcslen(OSD_Text[i].szText),
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

    int i;
    for (i = 0 ; i < nBufferSize && i < OSD_nTextCount ; i++)
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
        LPTSTR pszText = (LPTSTR)pOSDCommand->dwParam;

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


static void OSD_GetTextResult(int delta, TCHAR* text, long *color)
{
    if (delta <= 3)
    {
        _tcscpy (text, _T("*****"));
    }
    else if (delta <= 9)
    {
        _tcscpy (text, _T("****"));
    }
    else if (delta <= 18)
    {
        _tcscpy (text, _T("***"));
    }
    else if (delta <= 30)
    {
        _tcscpy (text, _T("**"));
    }
    else
    {
        _tcscpy (text, _T("*"));
    }
    *color = -1;
}


extern long NumFilters;
extern FILTER_METHOD* Filters[];

static void OSD_RefreshGeneralScreen(double Size)
{
    double  dfMargin = 0.02;    // 2% of screen height/width
    TCHAR   szInfo[64];
    int     nLine;
    int     i;
    CSource* pSource = Providers_GetCurrentSource();
    CSimpleSetting* pSetting = NULL;
    int     OverlaySetting;
    BOOL    UseOverlayCtrl;
    BOOL    DisplayTitle;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // DScaler version
    OSD_AddText(GetProductNameAndVersion().c_str(), Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    // Channel
    nLine = 3;

    OSD_AddText(_T("Source"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    // Video input
    OSD_AddText(pSource != NULL ? pSource->GetStatus().c_str() : _T(""), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    // Audio mute
    if (Setting_GetValue(WM_AUDIO_GETVALUE, SYSTEMINMUTE) == TRUE)
    {
        OSD_AddText(_T("Volume Mute"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    // Source size
    if (pSource != NULL)
    {
        if (pSource->GetWidth() != pSource->GetInitialWidth() || pSource->GetHeight() != pSource->GetInitialHeight())
        {
            _stprintf (szInfo, _T("Size %ux%u (%ux%u)"), pSource->GetWidth(), pSource->GetHeight(), pSource->GetInitialWidth(), pSource->GetInitialHeight());
        }
        else
        {
            _stprintf (szInfo, _T("Size %ux%u"), pSource->GetWidth(), pSource->GetHeight());
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    // Source ratio
    if (!AspectSettings.SquarePixels)
    {
        _stprintf(szInfo, _T("Ratio %.2f:1"), (double)Setting_GetValue(WM_ASPECT_GETVALUE, SOURCE_ASPECT) / 1000.0);
        if ( (Setting_GetValue(WM_ASPECT_GETVALUE, ASPECT_MODE)) == 1
          && (Setting_GetValue(WM_ASPECT_GETVALUE, SOURCE_ASPECT)) != 1333)
        {
            _tcscat(szInfo, _T(" Letterbox"));
        }
        else if (Setting_GetValue(WM_ASPECT_GETVALUE, ASPECT_MODE) == 2)
        {
            _tcscat(szInfo, _T(" Anamorphic"));
        }
        if (Setting_GetValue(WM_ASPECT_GETVALUE, AUTODETECTASPECT))
        {
            _tcscat(szInfo, _T(" auto"));
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    // Display ratio
    nLine++;
    OSD_AddText(_T("Display"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    if (Setting_GetValue(WM_ASPECT_GETVALUE, TARGET_ASPECT) == 0)
    {
        _tcscpy(szInfo, _T("Ratio from current resolution"));
    }
    else
    {
        _stprintf(szInfo, _T("Ratio %.2f:1"), (double)Setting_GetValue(WM_ASPECT_GETVALUE, TARGET_ASPECT) / 1000.0);
    }
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    // Video and overlay settings
    nLine = 4;
    DisplayTitle = FALSE;
    UseOverlayCtrl = Setting_GetValue(WM_OTHER_GETVALUE, USEOVERLAYCONTROLS);
    pSetting = pSource != NULL ? pSource->GetBrightness() : NULL;
    OverlaySetting = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS);
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText(_T("Brightness"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetContrast() : NULL;
    OverlaySetting = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYCONTRAST);
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText(_T("Contrast"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetHue() : NULL;
    OverlaySetting = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYHUE);
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText(_T("Hue"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetSaturation() : NULL;
    OverlaySetting = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSATURATION);
    if(pSetting != NULL || UseOverlayCtrl)
    {
        OSD_AddText(_T("Color"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (pSetting != NULL)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if (UseOverlayCtrl)
    {
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
    if(pSetting != NULL || UseOverlayCtrl)
    {
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetSaturationU() : NULL;
    if(pSetting != NULL)
    {
        OSD_AddText(_T("Color U"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    pSetting = pSource != NULL ? pSource->GetSaturationV() : NULL;
    if(pSetting != NULL)
    {
        OSD_AddText(_T("Color V"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    OverlaySetting = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYGAMMA);
    if(UseOverlayCtrl)
    {
        OSD_AddText(_T("Gamma"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    OverlaySetting = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSHARPNESS);
    if(UseOverlayCtrl)
    {
        OSD_AddText(_T("Sharpness"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.8 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        OSD_AddText(pSetting->GetDisplayValue().c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
        DisplayTitle = TRUE;
        nLine++;
    }
    if (DisplayTitle)
    {
        if(UseOverlayCtrl)
        {
            OSD_AddText(_T("Video & Overlay settings"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
        }
        else
        {
            OSD_AddText(_T("Video settings"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.9 - dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
        }
    }

    // Deinterlace Mode
    nLine = -1;
    if (Setting_GetValue(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS))
    {
        OSD_AddText(_T("Judder Terminator"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
    }
    if (!IsProgressiveMode())
    {
        if (Setting_GetValue(WM_FD60_GETVALUE, FALLBACKTOVIDEO))
        {
            OSD_AddText(_T("Fallback on Bad Pulldown"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        }
        if (Setting_GetValue(WM_OUTTHREADS_GETVALUE, AUTODETECT))
        {
            OSD_AddText(_T("Auto Pulldown Detect"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        }
    }
    OSD_AddText(MBCSToTString(GetDeinterlaceModeName()).c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
    OSD_AddText(_T("Deinterlacing"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));

    // Filters
    nLine = -1;
    DisplayTitle = FALSE;
    for (i = 0 ; i < NumFilters ; i++)
    {
        if (Filters[i]->bActive)
        {
            OSD_AddText(MBCSToTString(Filters[i]->szName).c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
            DisplayTitle = TRUE;
        }
    }
    if (DisplayTitle)
    {
        OSD_AddText(_T("Filters"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
    }
}


static void OSD_RefreshStatisticsScreen(double Size)
{
    double              dfMargin = 0.02;    // 2% of screen height/width
    TCHAR               szInfo[64];
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
    OSD_AddText(_T("Statistics"), Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 1;

    OSD_AddText(_T("Dropped fields"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number : %ld"), pPerf->GetNumberDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Last second : %d"), pPerf->GetDroppedFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    OSD_AddText(_T("Not waited"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number : %ld"), pPerf->GetNumberNotWaitedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Last second : %d"), pPerf->GetNotWaitedFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageNotWaitedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    OSD_AddText(_T("Processed late"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number : %ld"), pPerf->GetNumberLateFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Last second : %d"), pPerf->GetLateFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageLateFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

//    OSD_AddText(_T("Used fields"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//
//    _stprintf (szInfo, _T("Last second : %d"), pPerf->GetUsedFieldsLastSecond());
//    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//    _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageUsedFields());
//    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    if (Setting_GetValue(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS))
    {
        OSD_AddText(_T("No flip at time"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        _stprintf (szInfo, _T("Number : %ld"), pPerf->GetNumberNoFlipAtTime());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        _stprintf (szInfo, _T("Last second : %d"), pPerf->GetNoFlipAtTimeLastSecond());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageNoFlipAtTime());
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    nLine = 1;

    CurrentTicks = GetTickCount();

    OSD_AddText(_T("Deinterlace Modes"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number of changes : %ld"), nTotalDeintModeChanges);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//    OSD_AddText(_T("changes - % of time - Mode"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
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
                _stprintf (szInfo, _T("%03d - %05.1f %% - %s"), DeintMethod->ModeChanges, ticks * 100 / (double)(CurrentTicks - nInitialTicks), MBCSToTString(DeintMethod->szName).c_str());
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
                _stprintf (szInfo, _T("%03d - %05.1f %% - %s"), DeintMethod->ModeChanges, ticks * 100 / (double)(CurrentTicks - nInitialTicks), MBCSToTString(DeintMethod->szName).c_str());
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
                _stprintf (szInfo, _T("%03d - %05.1f %% - %s"), DeintMethod->ModeChanges, ticks * 100 / (double)(CurrentTicks - nInitialTicks), MBCSToTString(DeintMethod->szName).c_str());
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

    OSD_AddText(_T("Aspect Ratios"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number of changes : %d"), nNbRatioSwitch);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
//    OSD_AddText(_T("changes - % of time - Ratio"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

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
//                        OSD_AddText(_T("changes - % of time - Ratio"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, (nCol == 1) ? dfMargin : 0.5, OSD_GetLineYpos (nFirstLine-1, dfMargin, Size));
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
                    _stprintf (szInfo, _T("%03d - %05.1f %% - %.3f:1 %s"), RatioStatistics[i].switch_count, ticks * 100 / (double)(CurrentTicks - nARInitialTicks), RatioStatistics[i].ratio / 1000.0, RatioStatistics[i].mode == 2 ? _T("A") : _T(""));
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
    TCHAR       szInfo[64];
    int         nLine;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText(_T("WSS data decoding"), Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 3;

    OSD_AddText(_T("Status"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Errors : %d"), WSS_CtrlData.NbDecodeErr);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Ok : %d"), WSS_CtrlData.NbDecodeOk);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    if ((WSS_CtrlData.NbDecodeErr+WSS_CtrlData.NbDecodeOk) > 0)
    {
        _stprintf (szInfo, _T("Last : %s"), (WSS_CtrlData.DecodeStatus == WSS_STATUS_OK) ? _T("OK") : _T("ERROR"));
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    }

    if ((WSS_CtrlData.NbDecodeOk+WSS_CtrlData.NbDecodeErr) > 0)
    {

        nLine = -1;

        // Debug informations
        if (WSS_CtrlData.NbDecodeOk > 0)
        {
            _stprintf (szInfo, _T("Start position min / max : %d / %d"), WSS_CtrlData.MinPos, WSS_CtrlData.MaxPos);
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        }
        _stprintf (szInfo, _T("Errors searching start position : %d"), WSS_CtrlData.NbErrPos);
        OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        OSD_AddText(_T("Debug"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));

        if (WSS_CtrlData.DecodeStatus != WSS_STATUS_ERROR)
        {
            nLine = 3;

            OSD_AddText(_T("Data"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

            // WSS data
            if (WSS_Data.AspectRatio > 0)
            {
                _stprintf (szInfo, _T("Aspect ratio : %.3f"), WSS_Data.AspectRatio / 1000.0);
                if (WSS_Data.AspectMode == 1)
                    _tcscat (szInfo, _T(" Letterboxed"));
                else if (WSS_Data.AspectMode == 2)
                    _tcscat (szInfo, _T(" Anamorphic"));
            }
            else
            {
                _tcscpy (szInfo, _T("Aspect ratio : undefined"));
            }
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            _stprintf (szInfo, _T("Mode : %s"), WSS_Data.FilmMode ? _T("film Mode") : _T("camera Mode"));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            _stprintf (szInfo, _T("Helper signals : %s"), WSS_Data.HelperSignals ? _T("yes") : _T("no"));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            _stprintf (szInfo, _T("Color encoding : %s"), WSS_Data.ColorPlus ? _T("ColorPlus") : _T("normal"));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            _stprintf (szInfo, _T("Teletext subtitles : %s"), WSS_Data.TeletextSubtitle ? _T("yes") : _T("no"));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            switch (WSS_Data.OpenSubtitles)
            {
            case WSS625_SUBTITLE_NO:
                _tcscpy (szInfo, _T("Open subtitles : no"));
                break;
            case WSS625_SUBTITLE_INSIDE:
                _tcscpy (szInfo, _T("Open subtitles : inside picture"));
                break;
            case WSS625_SUBTITLE_OUTSIDE:
                _tcscpy (szInfo, _T("Open subtitles : outside picture"));
                break;
            default:
                _tcscpy (szInfo, _T("Open subtitles : ???"));
                break;
            }
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            _stprintf (szInfo, _T("Surround sound : %s"), WSS_Data.SurroundSound ? _T("yes") : _T("no"));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            _stprintf (szInfo, _T("Copyright asserted : %s"), WSS_Data.CopyrightAsserted ? _T("yes") : _T("no"));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            _stprintf (szInfo, _T("Copy protection : %s"), WSS_Data.CopyProtection ? _T("yes") : _T("no"));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }
    }
}


static void OSD_RefreshARScreen(double Size)
{
    double      dfMargin = 0.02;    // 2% of screen height/width
    TCHAR       szInfo[64];
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
    OSD_AddText(_T("Aspect Ratio"), Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 3;

    _stprintf (szInfo, _T("Number of changes : %d"), nNbRatioSwitch);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
    OSD_AddText(_T("changes - % of time - Ratio"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

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
                        OSD_AddText(_T("changes - % of time - Ratio"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, (nCol == 1) ? dfMargin : 0.5, OSD_GetLineYpos (4, dfMargin, Size));
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
                    _stprintf (szInfo, _T("%03d - %05.1f %% - %.3f:1 %s"), RatioStatistics[i].switch_count, ticks * 100 / (double)(CurrentTicks - nARInitialTicks), RatioStatistics[i].ratio / 1000.0, RatioStatistics[i].mode == 2 ? _T("A") : _T(""));
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
    TCHAR           szInfo[64];
    int             nLine;
    long            Color, BackColor;
    unsigned char   val1, val2, val3, val4, val5, val6;
    int             dif_val1, dif_val2, dif_val3;
    int             dif_total1, dif_total2;
    TCHAR           szResult[16];
    CTestPattern*   pTestPattern;
    CSubPattern*    pSubPattern;
    CSource*        pSource = Providers_GetCurrentSource();
    CSliderSetting* pSetting = NULL;
    BOOL            avail1, avail2;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText(_T("Card calibration"), Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    // Video settings
    if (pSource != NULL)
    {
        if ( (pCalibration->GetType() == CAL_MANUAL)
          || (pCalibration->GetType() == CAL_CHECK_YUV_RANGE) )
        {
            pSetting = pSource->GetBrightness();
            if(pSetting != NULL)
            {
                _stprintf (szInfo, _T("Brightness : %+04d"), pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (1, dfMargin, Size));
            }
            pSetting = pSource->GetContrast();
            if(pSetting != NULL)
            {
                _stprintf (szInfo, _T("Contrast : %03u"), pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (2, dfMargin, Size));
            }
            pSetting = pSource->GetHue();
            if(pSetting != NULL)
            {
                _stprintf (szInfo, _T("Hue : %+04d"), pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
            }
            pSetting = pSource->GetSaturation();
            if(pSetting != NULL)
            {
                _stprintf (szInfo, _T("Color : %03u"), pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (1, dfMargin, Size));
            }
            pSetting = pSource->GetSaturationU();
            if(pSetting != NULL)
            {
                _stprintf (szInfo, _T("Color U : %03u"), pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (2, dfMargin, Size));
            }
            pSetting = pSource->GetSaturationV();
            if(pSetting != NULL)
            {
                _stprintf (szInfo, _T("Color V : %03u"), pSetting->GetValue());
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
                _stprintf (szInfo, _T("Brightness : %+04d"), pSetting->GetValue());
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
                _stprintf (szInfo, _T("Contrast : %03u"), pSetting->GetValue());
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
                _stprintf (szInfo, _T("Color : %03u"), pSetting->GetValue());
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
                _stprintf (szInfo, _T("Color U : %03u"), pSetting->GetValue());
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
                _stprintf (szInfo, _T("Color V : %03u"), pSetting->GetValue());
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
                _stprintf (szInfo, _T("Hue : %+04d"), pSetting->GetValue());
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
        OSD_AddText(pTestPattern->GetName().c_str(), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (3, dfMargin, Size));
    }

    if (pCalibration->IsRunning() && (pTestPattern != NULL))
    {
        if ( (pCalibration->GetType() != CAL_MANUAL)
          && (pCalibration->GetType() != CAL_CHECK_YUV_RANGE) )
        {
            switch (pCalibration->GetCurrentStep())
            {
            case -1:
                _tcscpy(szInfo, _T("Calibration finished"));
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                _tcscpy(szInfo, _T("Adjusting Brightness ..."));
                break;
            case 5:
            case 6:
            case 7:
            case 8:
                _tcscpy(szInfo, _T("Adjusting Contrast ..."));
                break;
            case 10:
            case 11:
                _tcscpy(szInfo, _T("Fine tuning of brightness and contrast ..."));
                break;
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                _tcscpy(szInfo, _T("Adjusting Color ..."));
                break;
              case 20:
            case 21:
                _tcscpy(szInfo, _T("Adjusting Hue ..."));
                break;
            case 22:
            case 23:
                _tcscpy(szInfo, _T("Fine tuning of color ..."));
                break;
            default:
                _tcscpy(szInfo, _T(""));
                break;
            }
            OSD_AddText(szInfo, Size, OSD_COLOR_CURRENT, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (5, dfMargin, Size));
        }

        if ( (pCalibration->GetType() == CAL_MANUAL)
          || (pCalibration->GetType() == CAL_CHECK_YUV_RANGE)
          || (pCalibration->GetCurrentStep() == -1) )
        {
            nLine = 4;

            if ( Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_RGB_DELTA)
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                OSD_AddText(_T("Delta RGB"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
            }
            if ( Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_YUV_DELTA)
              && (pCalibration->GetType() == CAL_MANUAL) )
            {
                OSD_AddText(_T("Delta YUV"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
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
                            _stprintf (szInfo, _T("Y Range [%u,%u]"), val1, val4);
                            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            _stprintf (szInfo, _T("U Range [%u,%u]"), val2, val5);
                            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            _stprintf (szInfo, _T("V Range [%u,%u]"), val3, val6);
                            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                        }
                    }
                    else
                    {
                        (*it)->GetRefColor(FALSE, &val1, &val2, &val3);
                        BackColor = RGB(val1, val2, val3);

                        avail1 = (*it)->GetDeltaColor(FALSE, &dif_val1, &dif_val2, &dif_val3, &dif_total1);
                        if ( avail1
                          && Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_RGB_DELTA)
                          && (pCalibration->GetType() == CAL_MANUAL) )
                        {
                            _stprintf (szInfo, _T("(%+d,%+d,%+d) (%d)"), dif_val1, dif_val2, dif_val3, dif_total1);
                            OSD_AddText(szInfo, Size, -1, BackColor, OSDB_SHADED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                        }
                        avail2 = (*it)->GetDeltaColor(TRUE, &dif_val1, &dif_val2, &dif_val3, &dif_total2);
                        if ( avail2
                          && Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_YUV_DELTA)
                          && (pCalibration->GetType() == CAL_MANUAL) )
                        {
                            _stprintf (szInfo, _T("(%d) (%+d,%+d,%+d)"), dif_total2, dif_val1, dif_val2, dif_val3);
                            OSD_AddText(szInfo, Size, -1, BackColor, OSDB_SHADED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                        }

                        if (pCalibration->GetType() == CAL_MANUAL)
                        {
                            if ( avail1
                              && Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_RGB_DELTA)
                              && ! Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_YUV_DELTA) )
                            {
                                OSD_GetTextResult(dif_total1, szInfo, &Color);
                                OSD_AddText(szInfo, Size, Color, BackColor, OSDB_SHADED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                            }
                            else if ( avail2
                                   && ! Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_RGB_DELTA)
                                   && Setting_GetValue(WM_CALIBR_GETVALUE, SHOW_YUV_DELTA) )
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
                            _stprintf (szInfo, _T("%s (YUV %d)"), szResult, dif_total2);
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
    TCHAR       szInfo[64];
    int         nLine;
    int         i;
    double      pos;
    DWORD        Total = 0;
    DWORD        Cycles = 0;

    if (Size == 0)
    {
        Size = OSD_DefaultSmallSizePerc;
    }

    // Title
    OSD_AddText(_T("Information for developers"), Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

    nLine = 3;

    OSD_AddText(_T("Dropped fields"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number : %ld"), pPerf->GetNumberDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Last second : %d"), pPerf->GetDroppedFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageDroppedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    OSD_AddText(_T("Not waited"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number : %ld"), pPerf->GetNumberNotWaitedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Last second : %d"), pPerf->GetNotWaitedFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageNotWaitedFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    OSD_AddText(_T("Processed late"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    _stprintf (szInfo, _T("Number : %ld"), pPerf->GetNumberLateFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Last second : %d"), pPerf->GetLateFieldsLastSecond());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    _stprintf (szInfo, _T("Average / s : %.1f"), pPerf->GetAverageLateFields());
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

    nLine = 3;

    OSD_AddText(_T("Average Time per cycle (1/10 ms)"), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
    for (i = 0 ; i < PERF_TYPE_LASTONE ; ++i)
    {
        if (pPerf->IsValid((ePerfType)i))
        {
            pos = OSD_GetLineYpos (nLine, dfMargin, Size);
            _stprintf(szInfo, _T("%s :"), pPerf->GetName((ePerfType)i));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
            _stprintf(szInfo, _T("%d"), pPerf->GetAverageDuration((ePerfType)i));
            OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.45, pos);
            nLine++;
            Total += pPerf->GetAverageDuration((ePerfType)i);
        }
    }
    pos = OSD_GetLineYpos (nLine, dfMargin, Size);
    _stprintf(szInfo, _T("%s"), _T("Total"));
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
    _stprintf(szInfo, _T("%d"), Total);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.45, pos);
    nLine += 2;
    pos = OSD_GetLineYpos (nLine, dfMargin, Size);
    _stprintf(szInfo, _T("%s"), _T("Duration since start"));
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos);
    Cycles = pPerf->GetNbCycles(1);
    _stprintf(szInfo, _T("%d:%02d"), Cycles/60, Cycles % 60);
    OSD_AddText(szInfo, Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 0.45, pos);

}


static void OSD_DisplayProgrammeInfos(double Size)
{
    double    dfMargin = 0.02;    // 2% of screen height/width

    time_t StartTime;
    time_t EndTime;
    tstring ChannelName;
    tstring ProgrammeTitle;
    tstring Category;

    if (MyEPG.GetProgrammeMainData(-1, &StartTime, &EndTime, ChannelName, ProgrammeTitle, Category) == FALSE)
    {
        return;
    }

    OSD_AddText(ChannelName.c_str(), OSD_DefaultSizePerc, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (1, dfMargin, OSD_DefaultSizePerc));

    if (Size == 0)
    {
        Size = Setting_GetValue(WM_EPG_GETVALUE, (EPG_PERCENTAGESIZE));
    }

    struct tm *date_tm;
    TCHAR  StartTimeStr[6];
    TCHAR  EndTimeStr[6];
    date_tm = localtime(&StartTime);
    _stprintf(StartTimeStr, _T("%02u:%02u"), date_tm->tm_hour, date_tm->tm_min);
    date_tm = localtime(&EndTime);
    _stprintf(EndTimeStr, _T("%02u:%02u"), date_tm->tm_hour, date_tm->tm_min);

    double pos2 = OSD_GetLineYpos (2, dfMargin, OSD_DefaultSizePerc);
    double pos3 = pos2 + OSD_GetLineYpos (2, dfMargin, Size) - OSD_GetLineYpos (1, dfMargin, Size);
    double pos4 = pos3 + pos3 - pos2;

    OSD_AddText(ProgrammeTitle.c_str(), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos2);
    TCHAR  szInfo[16];
    _stprintf(szInfo, _T("%s - %s"), StartTimeStr, EndTimeStr);
    OSD_AddText(szInfo, Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos3);
    time_t TimeNow;
    time(&TimeNow);
    _stprintf(szInfo, _T("%.1f %%"), (double)(TimeNow - StartTime) * 100.0 / (double)(EndTime - StartTime));
    OSD_AddText(szInfo, Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos4);
}


static void OSD_RefreshCurrentProgrammeScreen(double Size)
{
    double    dfMargin = 0.02;    // 2% of screen height/width

    if (Size == 0)
    {
        Size = Setting_GetValue(WM_EPG_GETVALUE, (EPG_PERCENTAGESIZE));
    }

    int nLine = 1;

    double pos1 = OSD_GetLineYpos (nLine++, dfMargin, Size);
    double pos2 = OSD_GetLineYpos (nLine++, dfMargin, Size);

    tstring Channel;

    if (   (MyEPG.SearchForProgramme(Channel) == FALSE)
        || (Channel.length() == 0) )
    {
        OSD_AddText(_T("No EPG information"), Size, -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos1);
        return;
    }

    time_t StartTime;
    time_t EndTime;
    tstring ChannelName;
    tstring ProgrammeTitle;
    tstring SubTitle;
    tstring Category;
    tstring Description;

    MyEPG.GetProgrammeData(-1, &StartTime, &EndTime, ChannelName, ProgrammeTitle, SubTitle, Category, Description);

    struct tm *date_tm;
    TCHAR  StartTimeStr[6];
    TCHAR  EndTimeStr[6];
    date_tm = localtime(&StartTime);
    _stprintf(StartTimeStr, _T("%02u:%02u"), date_tm->tm_hour, date_tm->tm_min);
    date_tm = localtime(&EndTime);
    _stprintf(EndTimeStr, _T("%02u:%02u"), date_tm->tm_hour, date_tm->tm_min);

    OSD_AddText(ChannelName.c_str(), Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos1);
    TCHAR  szInfo[16];
    time_t TimeNow;
    time(&TimeNow);
    if (   (TimeNow >= StartTime)
        && (TimeNow < EndTime) )
    {
        _stprintf(szInfo, _T("%.1f %%"), (double)(TimeNow - StartTime) * 100.0 / (double)(EndTime - StartTime));
        OSD_AddText(szInfo, Size, OSD_COLOR_SECTION, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, pos1);
    }
    _stprintf(szInfo, _T("%s - %s"), StartTimeStr, EndTimeStr);
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
        int nb2 = OSD_CutLines((LPTSTR)Description.c_str(), Setting_GetValue(WM_EPG_GETVALUE, (EPG_MAXCHARSPERLINE)));

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
    double    dfMargin = 0.02;    // 2% of screen height/width

    time_t    TimeMin;
    time_t    TimeMax;
    int nb = MyEPG.GetSearchContext(NULL, &TimeMin, &TimeMax);

    if (Size == 0)
    {
        Size = Setting_GetValue(WM_EPG_GETVALUE, (EPG_PERCENTAGESIZE));
    }

    // Title
    double pos1 = OSD_GetLineYpos (1, dfMargin, Size*1.5);
    TCHAR  szInfo[64];
    struct tm *Time_tm = localtime(&TimeMin);
    _stprintf(szInfo, _T("%02u/%02u"), Time_tm->tm_mday, Time_tm->tm_mon+1);
    OSD_AddText(szInfo, Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos1);
    _stprintf(szInfo, _T("%02u:%02u/%02u:%02u"), Time_tm->tm_hour, Time_tm->tm_min);
    Time_tm = localtime(&TimeMax);
    _stprintf(&szInfo[5], _T(" / %02u:%02u"), Time_tm->tm_hour, Time_tm->tm_min);
    OSD_AddText(szInfo, Size*1.5, OSD_COLOR_TITLE, -1, OSDB_USERDEFINED, OSD_XPOS_CENTER, 0.5, pos1);

    if (nb == 0)
    {
        return;
    }

    time_t TimeNow;
    time(&TimeNow);

    tstring Channel;
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
        TCHAR StartTimeStr[6];
        TCHAR EndTimeStr[6];
        tstring ChannelName;
        tstring ProgrammeTitle;
        tstring Category;

        MyEPG.GetProgrammeMainData(i, &StartTime, &EndTime, ChannelName, ProgrammeTitle, Category);
        struct tm *date_tm;
        date_tm = localtime(&StartTime);
        _stprintf(StartTimeStr, _T("%02u:%02u"), date_tm->tm_hour, date_tm->tm_min);
        date_tm = localtime(&EndTime);
        _stprintf(EndTimeStr, _T("%02u:%02u"), date_tm->tm_hour, date_tm->tm_min);

        if (   (Channel.length() > 0)
            && !_tcsicmp(ChannelName.c_str(), Channel.c_str())
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
        _stprintf(szInfo, _T("%s - %s"), StartTimeStr, EndTimeStr);
        OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos2);
        OSD_AddText(ProgrammeTitle.c_str(), Size, (i == (IdxCur -1)) ? OSD_COLOR_CURRENT : -1, -1, OSDB_USERDEFINED, OSD_XPOS_LEFT, dfMargin, pos3);
        if (   (TimeNow >= StartTime)
            && (TimeNow < EndTime) )
        {
            _stprintf(szInfo, _T("%.1f %%"), (double)(TimeNow - StartTime) * 100.0 / (double)(EndTime - StartTime));
            OSD_AddText(szInfo, Size, Color, -1, OSDB_USERDEFINED, OSD_XPOS_RIGHT, 1 - dfMargin, pos3);
        }
    }

    MyEPG.SetDisplayIndexes(IdxMin, IdxMax, IdxCur);

    _stprintf(szInfo, _T("%u-%u / %u"), IdxMin, IdxMax, nb);
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
    if ( (_tcslen(szText) == 0) || (OSD_nTextCount >= OSD_MAX_TEXT) )
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
    _tcsncpy(OSD_Text[OSD_nTextCount].szText, szText, sizeof(OSD_Text[OSD_nTextCount].szText));

    OSD_nTextCount++;
}


//---------------------------------------------------------------------------
// Internal: Add a new line of text to the list of texts for OSD.
// Internal: _T("\n") and _T("\r\n") are supported for starting a new line.
void OSD_AddText(LPCTSTR szText, double Size, long NewTextColor, long BackgroundColor,
                 eOSDBackground BackgroundMode, eOSDTextXPos TextXPos, double XPos, double YPos)
{
    TCHAR     SingleLine[512];
    int       SingleLineIndex = 0;
    const TCHAR* s;

    // convert _T("\r\n") to _T("\n")
    while(s = _tcsstr(szText, _T("\r\n")))
    {
        _tcscpy((TCHAR*)&s[0], &s[1]);
    }

    for(int i = 0; i < _tcslen(szText); i++)
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
int OSD_CutLines(LPTSTR szText, long MaxCharsPerLine)
{
    int       IndexLine = 0;
    int       InLineIndex = 0;
    const TCHAR     *cs;
    int       i;

    // convert _T("\r\n") to _T("\n")
    while(cs = _tcsstr(szText, _T("\r\n")))
    {
        _tcscpy((TCHAR* )&cs[0], &cs[1]);
    }

    for(i = 0; i < _tcslen(szText); i++)
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
                    TCHAR* s = _tcsrchr(TextLines[IndexLine], ' ');
                    if (s)
                    {
                        *s = 0x00;
                        IndexLine++;
                        if (IndexLine == 128)
                        {
                            InLineIndex = 0;
                            break;
                        }
                        _tcscpy(&TextLines[IndexLine][0], s+1);
                        InLineIndex = _tcslen(TextLines[IndexLine]);
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
        if ((_tcslen (ActiveScreens[i].name) > 0) && !ActiveScreens[i].managed_by_app)
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
        "Outline Color", SLIDER, 0, (LONG_PTR*)&OSD_OutlineColor,
         RGB(0,0,0), 0, RGB(255,255,255), 1, 1,
         NULL,
        "OSD", "OutlineColor", NULL,
    },
    {
        "Text Color", SLIDER, 0, (LONG_PTR*)&OSD_TextColor,
         RGB(0,255,0), 0, RGB(255,255,255), 1, 1,
         NULL,
        "OSD", "TextColor", NULL,
    },
    {
        "Default Size", SLIDER, 0, (LONG_PTR*)&OSD_DefaultSizePerc,
         10, 2, 15, 1, 1,
         NULL,
        "OSD", "DefaultSizePerc", NULL,
    },
    {
        "Default Small Size", SLIDER, 0, (LONG_PTR*)&OSD_DefaultSmallSizePerc,
         5, 2, 7, 1, 1,
         NULL,
        "OSD", "DefaultSmallSizePerc", NULL,
    },
    {
        "Anti Alias", ONOFF, 0, (LONG_PTR*)&OSD_bAntiAlias,
         FALSE, 0, 1, 1, 1,
         NULL,
        "OSD", "AntiAlias", NULL,
    },
    {
        "Background Type", ITEMFROMLIST, 0, (LONG_PTR*)&OSD_Background,
         OSDB_TRANSPARENT, 0, OSDB_LASTONE - 1, 1, 1,
         OSD_szBackgroundNames,
        "OSD", "Background", NULL,
    },
    {
        "Outline Text", ONOFF, 0, (LONG_PTR*)&OSD_bOutline,
         TRUE, 0,  1, 1, 1,
         NULL,
        "OSD", "Outline", NULL,
    },
    {
        "Auto Hide Screens", ONOFF, 0, (LONG_PTR*)&OSD_bAutoHide,
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "AutoHide", OSD_AutoHide_OnChange,
    },
    {
        "Use General Screen", ONOFF, 0, (LONG_PTR*)&(ActiveScreens[4].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseGeneralScreen", NULL,
    },
    {
        "Use Statistics Screen", ONOFF, 0, (LONG_PTR*)&(ActiveScreens[5].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseStatisticsScreen", NULL,
    },
    {
        "Use WSS Decoding Screen", ONOFF, 0, (LONG_PTR*)&(ActiveScreens[6].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseWSSDecodingScreen", NULL,
    },
    {
        "Use Developer Screen", ONOFF, 0, (LONG_PTR*)&(ActiveScreens[7].active),
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "UseDeveloperScreen", NULL,
    },
    {
        "Font Name", TCHARSTRING, 0, OSD_szFontName.GetPointer(),
         (LONG_PTR)"Arial", 0, 0, 0, 0,
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

void OSD_Init()
{
    OSD_AutoHide_OnChange(OSD_bAutoHide);
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

