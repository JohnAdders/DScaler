/////////////////////////////////////////////////////////////////////////////
// $Id: OSD.cpp,v 1.42 2001-11-29 17:30:52 adcockj Exp $
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

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "OSD.h"
#include "AspectRatio.h"
#include "Other.h"
#include "Status.h"
#include "ProgramList.h"
#include "Audio.h"
#include "MixerDev.h"
#include "OutThreads.h"
#include "FD_60Hz.h"
#include "Filter.h"
#include "Dialogs.h"
#include "DScaler.h"
#include "VBI_WSSdecode.h"
#include "Calibration.h"
#include "Providers.h"
#include "PaintingHDC.h"

extern long NumFilters;
extern FILTER_METHOD* Filters[];

#define OSD_COLOR_TITLE     RGB(255,150,150)
#define OSD_COLOR_SECTION   RGB(150,150,255)
#define OSD_COLOR_CURRENT   RGB(200,150,0)

char szFontName[128] = "Arial";
long OutlineColor = RGB(0,0,0);
long TextColor = RGB(0,255,0);
long DefaultSizePerc = 10;
long DefaultSmallSizePerc = 4;
BOOL bAntiAlias = TRUE;
BOOL bOutline = TRUE;
eOSDBackground Background;
BOOL bAutoHide = TRUE;

CPaintingHDC PaintingHDC; //Not a good way to instantiate it here, but it only temporary..
//---------------------------------------------------------------------------
// Global OSD Information structure
TOsdInfo    grOSD[OSD_MAX_TEXT];
static int  NbText = 0;
static struct 
{
    char    name[24];       // Name of the screen
    BOOL    managed_by_app; // TRUE if only app can display the screen
    BOOL    auto_hide;      // TRUE if the screen must disappear after a delay
    int     refresh_delay;  // Refresh period in ms (0 means no refresh)
    BOOL    active;         // TRUE if user can show the screen
    BOOL    lock;           // TRUE if display of the screen should lock OSD
} ActiveScreens[] = {
    { "General screen",          FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, TRUE,  FALSE },
    { "Statistics screen",       FALSE, TRUE,  1000,                    FALSE, FALSE },
    { "WSS decoding screen",     FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, FALSE, FALSE },
    { "AR autodetection screen", FALSE, TRUE,  OSD_TIMER_REFRESH_DELAY, FALSE, FALSE },
    { "Card calibration screen", TRUE,  FALSE, 250,                     TRUE,  TRUE  },
};
static int  IdxCurrentScreen = -1;  // index of the current displayed OSD screen
static BOOL bRestoreScreen = FALSE; // Restore Info screen when clear OSD

BOOL        bOverride = FALSE;

//---------------------------------------------------------------------------
// Clean the list of texts for OSD
void OSD_ClearAllTexts()
{
    NbText = 0;
}

//---------------------------------------------------------------------------
// Add a new text to the list of texts for OSD
void OSD_AddText(LPCTSTR szText, double Size, long NewTextColor, long BackgroundColor, eOSDBackground BackgroundMode, eOSDTextXPos TextXPos, double XPos, double YPos)
{
    if ( (strlen(szText) == 0) || (NbText >= OSD_MAX_TEXT) )
    {
        return;
    }

    if (Size == 0)
    {
        grOSD[NbText].Size = DefaultSizePerc;
    }
    else
    {
        grOSD[NbText].Size = Size;
    }
    if (NewTextColor == -1)
    {
        grOSD[NbText].TextColor = TextColor;
    }
    else
    {
        grOSD[NbText].TextColor = NewTextColor;
    }
    if (BackgroundColor == -1)
    {
        grOSD[NbText].BackgroundColor = OutlineColor;
    }
    else
    {
        grOSD[NbText].BackgroundColor = BackgroundColor;
    }
    if (BackgroundMode == OSDBACK_LASTONE)
    {
        grOSD[NbText].BackgroundMode = Background;
    }
    else
    {
        grOSD[NbText].BackgroundMode = BackgroundMode;
    }
    grOSD[NbText].TextXPos = TextXPos;
    grOSD[NbText].XPos = XPos;
    grOSD[NbText].YPos = YPos;
    strncpy(grOSD[NbText].szText, szText, sizeof(grOSD[NbText].szText));

    NbText++;
}

//---------------------------------------------------------------------------
// Display defined OSD texts
void OSD_Show(HWND hWnd, int ShowType, int refresh_delay)
{
    RECT        winRect;
    HDC         hDC;

    if (bOverride)
    {
        return;
    }
    if (ShowType == OSD_PERSISTENT)
    {
        KillTimer(hWnd, OSD_TIMER_ID);
    }
    KillTimer(hWnd, OSD_TIMER_REFRESH_ID);
    hDC = GetDC(hWnd);
    GetClientRect(hWnd,&winRect);

    LARGE_INTEGER count, count1, pfreq;
    BOOL bPerformanceCounter = QueryPerformanceCounter(&count);

    if (IsStatusBarVisible())
    {
        winRect.bottom -= StatusBar_Height();
    }

    //
    //  Uncomment line below to get double buffering temporary disabled
    //
    //PaintingHDC.SetEnabled(false);

    HDC hDBDC = PaintingHDC.BeginPaint(hDC, &winRect);
    PaintColorkey(hWnd, TRUE, hDBDC, &winRect);
    OSD_Redraw(hWnd, hDBDC);
    PaintingHDC.EndPaint();

    if(bPerformanceCounter)
    {
        QueryPerformanceFrequency(&pfreq);
        QueryPerformanceCounter(&count1);
        char temp[200];
        sprintf(temp, "OSD_Show in %d ms", (int) (count1.QuadPart-count.QuadPart)/(pfreq.QuadPart/1000));
        OutputDebugString(temp);
    }

    ReleaseDC(hWnd, hDC);
    if (ShowType == OSD_AUTOHIDE)
    {
        SetTimer(hWnd, OSD_TIMER_ID, OSD_TIMER_DELAY, NULL);
    }
    if (refresh_delay > 0)
    {
        SetTimer(hWnd, OSD_TIMER_REFRESH_ID, refresh_delay, NULL);
    }
    StatusBar_Repaint();
}

//---------------------------------------------------------------------------
// Display specified OSD text with autohide
void OSD_ShowText(HWND hWnd, LPCTSTR szText, double Size)
{
    if (bOverride)
    {
        return;
    }

    if (strlen(szText))
    {
        OSD_ClearAllTexts();
        OSD_AddText(szText, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 0.9, 0.1);
        OSD_Show(hWnd, OSD_AUTOHIDE, 0);
        if (bAutoHide)
        {
            IdxCurrentScreen = -1;
        }
        bRestoreScreen = (IdxCurrentScreen != -1);
    }
    else
    {
        // If OSD message is blank, kill previous OSD message
        OSD_Clear(hWnd);
    }
}

//---------------------------------------------------------------------------
// Displayed specified OSD text without autohide timer.
// Stays on screen until a new OSD message replaces current OSD message.
void OSD_ShowTextPersistent(HWND hWnd, LPCTSTR szText, double Size)
{
    if (bOverride)
    {
        return;
    }

    if (strlen(szText))
    {
        OSD_ClearAllTexts();
        OSD_AddText(szText, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 0.9, 0.1);
        OSD_Show(hWnd, OSD_PERSISTENT, 0);
        if (bAutoHide)
        {
            IdxCurrentScreen = -1;
        }
        bRestoreScreen = (IdxCurrentScreen != -1);
    }
    else
    {
        // If OSD message is blank, kill previous OSD message
        OSD_Clear(hWnd);
    }
}

//---------------------------------------------------------------------------
// Override all previous OSD text, and force this current OSD text
// to override all other OSD text showings (done by the above functions).
// This is useful for external programs to override DScaler's own OSD text
// for its own controls.
void OSD_ShowTextOverride(HWND hWnd, LPCTSTR szText, double Size)
{
    bOverride = FALSE;
    OSD_ShowText(hWnd, szText, Size);
    if (strlen(szText))
    {
        bOverride = TRUE;
    }
}

//---------------------------------------------------------------------------
// Clear currently displayed OSD
void OSD_Clear(HWND hWnd)
{
    int i;

    KillTimer(hWnd, OSD_TIMER_ID);
    KillTimer(hWnd, OSD_TIMER_REFRESH_ID);
    bOverride = FALSE;
    for (i = 0 ; i < NbText ; i++)
    {
        InvalidateRect(hWnd, &(grOSD[i].CurrentRect), FALSE);
    }
    if (bRestoreScreen && (IdxCurrentScreen != -1) && ActiveScreens[IdxCurrentScreen].active)
    {
        OSD_RefreshInfosScreen(hWnd, 0, ActiveScreens[IdxCurrentScreen].auto_hide ? OSD_AUTOHIDE : OSD_PERSISTENT);
    }
    else
    {
        OSD_ClearAllTexts();
        IdxCurrentScreen = -1;
    }
    StatusBar_Repaint();
}

//---------------------------------------------------------------------------
// OSD Redrawing code.  Can be called from a paint event.
void OSD_Redraw(HWND hWnd, HDC hDC)
{
    CHAR        szCurrentFont[64];
    HFONT       hTmp, hOSDfont, hOSDfontOutline;
    int         nLen, nFontsize;
    int         nXpos, nYpos;
    int         nXWinSize, nYWinSize;
    TEXTMETRIC  tmOSDFont;
    SIZE        sizeText;
    RECT        winRect;
    DWORD       dwQuality = 0;
    int         i;

    nLen = strlen(grOSD[0].szText);
    if (nLen && hDC != NULL)
    {
        GetClientRect(hWnd,&winRect);
        nXWinSize = winRect.right  - winRect.left;
        nYWinSize = winRect.bottom - winRect.top;
        if (IsStatusBarVisible())
        {
            nYWinSize -= StatusBar_Height();
        }

        for (i = 0 ; i < NbText ; i++)
        {
            // LG 02/25/2001 This line is no more needed
            // if (grOSD[i].Size == 0) grOSD[i].Size = DefaultSizePerc;

            nFontsize = (int)((double)nYWinSize * (grOSD[i].Size / 100.00));

            // Set specified font
            if(bAntiAlias)
            {
                dwQuality = ANTIALIASED_QUALITY;
            }

            strcpy(szCurrentFont, szFontName);
            hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, dwQuality, DEFAULT_PITCH | FF_DONTCARE, szFontName);
            if (!hOSDfont)
            {
                // Fallback to Arial
                strcpy(szCurrentFont, "Arial");
                hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont);
                if (!hOSDfont)
                {
                    // Otherwise, fallback to any available font
                    strcpy(szCurrentFont, "");
                    hOSDfont = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, dwQuality, VARIABLE_PITCH | FF_SWISS, szCurrentFont);
                }
            }
            if (!hOSDfont)
            {
                ErrorBox("Failed To Create OSD Font");
            }
            hOSDfontOutline = CreateFont(nFontsize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, szCurrentFont);

            if (!hOSDfontOutline)
            {
                hOSDfontOutline = hOSDfont;
            }

            hTmp = (HFONT)SelectObject(hDC, hOSDfontOutline);
            if (hTmp)
            {
                GetTextMetrics(hDC, &tmOSDFont);
                GetTextExtentPoint32(hDC, grOSD[i].szText, strlen(grOSD[i].szText), &sizeText);

                switch (grOSD[i].TextXPos)
                {
                case OSD_XPOS_RIGHT:
                    nXpos = (int)((double)nXWinSize * grOSD[i].XPos) - sizeText.cx;
                    break;
                case OSD_XPOS_CENTER:
                    nXpos = (int)((double)nXWinSize * grOSD[i].XPos - (double)sizeText.cx / 2.0);
                    break;
                case OSD_XPOS_LEFT:
                default:
                    nXpos = (int)((double)nXWinSize * grOSD[i].XPos);
                    break;
                }

                nYpos = (int)((double)nYWinSize * grOSD[i].YPos);

                // Draw the requested background for the text
                switch(grOSD[i].BackgroundMode)
                {
                case OSDB_TRANSPARENT:
                    SetBkMode(hDC, TRANSPARENT);
                    SetBkColor(hDC, grOSD[i].BackgroundColor);
                    break;
            
                case OSDB_BLOCK:
                    SetBkMode(hDC, OPAQUE);
                    SetBkColor(hDC, grOSD[i].BackgroundColor);
                    break;
            
                case OSDB_SHADED:
                    {
                        HBRUSH hBrush;
                        HBRUSH hBrushOld;
                        HBITMAP hBM;
                        WORD bBrushBits[8] = {0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, };
                        SetBkMode(hDC, TRANSPARENT);
                        SetTextColor(hDC, grOSD[i].BackgroundColor);
                        SetBkColor(hDC, Overlay_GetColor());
                        hBM = CreateBitmap(8, 8, 1, 1, (LPBYTE)bBrushBits); 
                        hBrush = CreatePatternBrush(hBM); 
                        hBrushOld = (HBRUSH)SelectObject(hDC, hBrush);
                        if(bOutline)
                        {
                            PatBlt(hDC, nXpos - 2, nYpos - 2, sizeText.cx + 4, sizeText.cy + 4, PATCOPY);
                        }
                        else
                        {
                            PatBlt(hDC, nXpos, nYpos, sizeText.cx, sizeText.cy, PATCOPY);
                        }
                        SelectObject(hDC, hBrushOld);
                        DeleteObject(hBrush);
                        DeleteObject(hBM);
                    }
                    break;
                default:
                    break;
                }

                if(bOutline)
                {
                    // Draw OSD outline if required
                    SetTextColor(hDC, OutlineColor);
                    TextOut(hDC, nXpos - 2, nYpos, grOSD[i].szText, strlen(grOSD[i].szText));
                    TextOut(hDC, nXpos + 2, nYpos, grOSD[i].szText, strlen(grOSD[i].szText));
                    TextOut(hDC, nXpos, nYpos - 2, grOSD[i].szText, strlen(grOSD[i].szText));
                    TextOut(hDC, nXpos, nYpos + 2, grOSD[i].szText, strlen(grOSD[i].szText));
                    TextOut(hDC, nXpos - 1, nYpos - 1, grOSD[i].szText, strlen(grOSD[i].szText));
                    TextOut(hDC, nXpos + 1, nYpos - 1, grOSD[i].szText, strlen(grOSD[i].szText));
                    TextOut(hDC, nXpos - 1, nYpos + 1, grOSD[i].szText, strlen(grOSD[i].szText));
                    TextOut(hDC, nXpos + 1, nYpos + 1, grOSD[i].szText, strlen(grOSD[i].szText));
                }

                // Draw OSD text
                if (SelectObject(hDC, hOSDfont))
                {
                    SetTextColor(hDC, grOSD[i].TextColor);
                    SetBkColor(hDC, grOSD[i].BackgroundColor);
                    TextOut(hDC, nXpos, nYpos, grOSD[i].szText, strlen(grOSD[i].szText));

                    // MRS 2-23-01 Calculate rectnagle for the entire OSD 
                    // so we do not invalidate the entire window to remove it.
                    SIZE sz;
                    GetTextExtentExPoint(hDC, grOSD[i].szText, strlen(grOSD[i].szText), 
                                            32000, NULL, NULL, &sz);
                    grOSD[i].CurrentRect.left = nXpos-4; if (grOSD[i].CurrentRect.left < 0) grOSD[i].CurrentRect.left = 0;
                    grOSD[i].CurrentRect.right = nXpos + sz.cx + 4;
                    grOSD[i].CurrentRect.top = nYpos-4; if (grOSD[i].CurrentRect.top < 0) grOSD[i].CurrentRect.top = 0;
                    grOSD[i].CurrentRect.bottom = nYpos + sz.cy + 4;
                }

                SelectObject(hDC, hTmp);
                DeleteObject(hOSDfont);
                DeleteObject(hOSDfontOutline);
            }           
        }
    }
}

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
//        strcpy (text, "very good");
//        *color = RGB(0,192,0);
    }
    else if (delta <= 9)
    {
        strcpy (text, "****");
//        strcpy (text, "good");
//        *color = RGB(0,255,0);
    }
    else if (delta <= 18)
    {
        strcpy (text, "***");
//        strcpy (text, "medium");
//        *color = RGB(255,255,0);
    }
    else if (delta <= 30)
    {
        strcpy (text, "**");
//        strcpy (text, "bad");
//        *color = RGB(255,192,0);
    }
    else
    {
        strcpy (text, "*");
//        strcpy (text, "very bad");
//        *color = RGB(255,0,0);
    }
    *color = -1;
}

//---------------------------------------------------------------------------
// Display/Refresh on screen the current information screen
void OSD_RefreshInfosScreen(HWND hWnd, double Size, int ShowType)
{
    double          dfMargin = 0.02;    // 2% of screen height/width
    char            szInfo[64];
    int             nLine, nCol;
    int             i;
    long            Color, BackColor;
    double          pos;
    DEINTERLACE_METHOD* DeintMethod;
    unsigned char   val1, val2, val3;
    int             dif_val1, dif_val2, dif_val3;
    int             dif_total1, dif_total2;
    char            szResult[16];
    CTestPattern *pTestPattern;
    CSubPattern *pSubPattern;
    CColorBar* pColorBar;
    CSource* pSource = Providers_GetCurrentSource();
    ISetting* pSetting = NULL;

    // Case : no OSD screen
    if (IdxCurrentScreen == -1)
    {
        return;
    }

    if (Size == 0)
    {
        Size = DefaultSmallSizePerc;
    }

    OSD_ClearAllTexts();

    switch (IdxCurrentScreen)
    {
    // GENERAL SCREEN
    case 0:
        // DScaler version
        OSD_AddText(GetProductNameAndVersion(), Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size));

        // Channel
        nLine = 2;

        // Video input + video format
        OSD_AddText(pSource->GetStatus(), Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

		/// \todo fix this to get correct details
        strcpy (szInfo, "Input ??? -");
        if (Setting_GetValue(Audio_GetSetting(SYSTEMINMUTE)) == TRUE)
        {
            strcat (szInfo, " - MUTE");
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        // Pixel width
        sprintf (szInfo, "Pixel width : %u", pSource->GetWidth());
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        // Source ratio
        sprintf(szInfo, "Source %.2f:1", (double)Setting_GetValue(Aspect_GetSetting(SOURCE_ASPECT)) / 1000.0);
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
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        // Display ratio
        if (Setting_GetValue(Aspect_GetSetting(TARGET_ASPECT)) == 0)
        {
            strcpy(szInfo, "Display ratio from current resolution");
        }
        else
        {
            sprintf(szInfo, "Display %.2f:1", (double)Setting_GetValue(Aspect_GetSetting(TARGET_ASPECT)) / 1000.0);
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        // Video settings
        nLine = 2;
        pSetting = pSource->GetBrightness();
        if(pSetting != NULL)
        {
            sprintf (szInfo, "Brightness : %03d", pSetting->GetValue());
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }
        pSetting = pSource->GetContrast();
        if(pSetting != NULL)
        {
            sprintf (szInfo, "Contrast : %03u", pSetting->GetValue());
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }
        pSetting = pSource->GetHue();
        if(pSetting != NULL)
        {
            sprintf (szInfo, "Hue : %03d", pSetting->GetValue());
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }
        pSetting = pSource->GetSaturation();
        if(pSetting != NULL)
        {
            sprintf (szInfo, "Color : %03u", pSetting->GetValue());
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }
        pSetting = pSource->GetSaturationU();
        if(pSetting != NULL)
        {
            sprintf (szInfo, "Color U : %03u", pSetting->GetValue());
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }
        pSetting = pSource->GetSaturationV();
        if(pSetting != NULL)
        {
            sprintf (szInfo, "Color V : %03u", pSetting->GetValue());
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }

        // Deinterlace Mode
        nLine = -1;
        if (Setting_GetValue(OutThreads_GetSetting(DOACCURATEFLIPS)))
        {
            strcpy(szInfo, "Judder Terminator ON");
        }
        else
        {
            strcpy(szInfo, "Judder Terminator OFF");
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        if (Setting_GetValue(FD60_GetSetting(FALLBACKTOVIDEO)))
        {
            strcpy(szInfo, "Fallback on Bad Pulldown ON");
        }
        else
        {
            strcpy(szInfo, "Fallback on Bad Pulldown OFF");
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        if (Setting_GetValue(OutThreads_GetSetting(AUTODETECT)))
        {
            strcpy(szInfo, "Auto Pulldown Detect ON");
        }
        else
        {
            strcpy(szInfo, "Auto Pulldown Detect OFF");
        }
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        OSD_AddText(GetDeinterlaceModeName(), Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));

        // Filters
        nLine = -1;
        for (i = 0 ; i < NumFilters ; i++)
        {
            strcpy(szInfo, Filters[i]->szName);
            if (Filters[i]->bActive)
            {
                strcat(szInfo, " ON");
            }
            else
            {
                strcat(szInfo, " OFF");
            }
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
        }
        break;

    // WSS DATA DECODING SCREEN
    case 2:
        // Title
        OSD_AddText("WSS data decoding", Size*1.5, OSD_COLOR_TITLE, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

        nLine = 3;

        OSD_AddText("Status", Size, OSD_COLOR_SECTION, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        sprintf (szInfo, "Errors : %d", WSS_CtrlData.NbDecodeErr);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        sprintf (szInfo, "Ok : %d", WSS_CtrlData.NbDecodeOk);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        if ((WSS_CtrlData.NbDecodeErr+WSS_CtrlData.NbDecodeOk) > 0)
        {
            sprintf (szInfo, "Last : %s", (WSS_CtrlData.DecodeStatus == WSS_STATUS_OK) ? "OK" : "ERROR");
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        }

        if ((WSS_CtrlData.NbDecodeOk+WSS_CtrlData.NbDecodeErr) > 0)
        {

            nLine = -1;

            // Debug informations
            if (WSS_CtrlData.NbDecodeOk > 0)
            {
                sprintf (szInfo, "Start position min / max : %d / %d", WSS_CtrlData.MinPos, WSS_CtrlData.MaxPos);
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
            }
            sprintf (szInfo, "Errors searching start position : %d", WSS_CtrlData.NbErrPos);
            OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));
            OSD_AddText("Debug", Size, OSD_COLOR_SECTION, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine--, dfMargin, Size));

            if (WSS_CtrlData.DecodeStatus != WSS_STATUS_ERROR)
            {
                nLine = 3;

                OSD_AddText("Data", Size, OSD_COLOR_SECTION, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

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
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                sprintf (szInfo, "Mode : %s", WSS_Data.FilmMode ? "film Mode" : "camera Mode");     
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                sprintf (szInfo, "Helper signals : %s", WSS_Data.HelperSignals ? "yes" : "no");     
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                sprintf (szInfo, "Color encoding : %s", WSS_Data.ColorPlus ? "ColorPlus" : "normal");
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                sprintf (szInfo, "Teletext subtitles : %s", WSS_Data.TeletextSubtitle ? "yes" : "no");      
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
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
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                sprintf (szInfo, "Surround sound : %s", WSS_Data.SurroundSound ? "yes" : "no");     
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                sprintf (szInfo, "Copyright asserted : %s", WSS_Data.CopyrightAsserted ? "yes" : "no");     
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                sprintf (szInfo, "Copy protection : %s", WSS_Data.CopyProtection ? "yes" : "no");       
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
            }
        }
        break;

    // STATISTICS SCREEN
    case 1:
        // Title
        OSD_AddText("Statistics", Size*1.5, OSD_COLOR_TITLE, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

        nLine = 3;

        OSD_AddText("Dropped fields", Size, OSD_COLOR_SECTION, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        sprintf (szInfo, "Number : %ld", nTotalDropFields);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        sprintf (szInfo, "Last second : %d", (int)ceil(nDropFieldsLastSec - 0.5));
//      sprintf (szInfo, "Last second : %.1f", nDropFieldsLastSec);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        sprintf (szInfo, "Average / s : %.1f", (double)nTotalDropFields * 1000 / (double)nSecTicks);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        OSD_AddText("Used fields", Size, OSD_COLOR_SECTION, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        sprintf (szInfo, "Last second : %d", (int)ceil(nUsedFieldsLastSec - 0.5));
//      sprintf (szInfo, "Last second : %.1f", nUsedFieldsLastSec);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        sprintf (szInfo, "Average / s : %.1f", (double)nTotalUsedFields * 1000.0 / (double)nSecTicks);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        nLine = 3;

        OSD_AddText("Deinterlace Modes", Size, OSD_COLOR_SECTION, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));

        sprintf (szInfo, "Number of changes : %ld", nTotalDeintModeChanges);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
        nLine++;
        OSD_AddText("changes - % of time - Mode", Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
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
                    }
                    else
                    {
                        Color = -1;
                    }
                    sprintf (szInfo, "%04d - %05.1f %% - %s", DeintMethod->ModeChanges, DeintMethod->ModeTicks * 100 / (double)(nLastTicks - nInitialTicks), DeintMethod->szName);
                    OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, pos);
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
                    }
                    else
                    {
                        Color = -1;
                    }
                    sprintf (szInfo, "%04d - %05.1f %% - %s", DeintMethod->ModeChanges, DeintMethod->ModeTicks * 100 / (double)(nLastTicks - nInitialTicks), DeintMethod->szName);
                    OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, pos);
                    nLine++;
                }
            }
            i++;
            DeintMethod = GetVideoDeintMethod(i);
        }
        break;


    // ASPECT RATIO AUTODETECTION SCREEN
    case 3:
        // Title
        OSD_AddText("Aspect Ratio Autodetection", Size*1.5, OSD_COLOR_TITLE, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

        nLine = 3;
        sprintf (szInfo, "Number of switch : %d", nNbRatioSwitch);
        OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine, dfMargin, Size));

        if (nNbRatioSwitch > 0)
        {
            nLine = 4;
            nCol = 1;

            for (i = 0 ; i < MAX_RATIO_STATISTICS ; i++)
            {
                if (RatioStatistics[i].switch_count > 0)
                {
                    pos = OSD_GetLineYpos (nLine, dfMargin, Size);
                    if (pos == 0)
                    {
                        nCol++;
                        nLine = 4;
                        if (nCol <= 2)
                        {
                            pos = OSD_GetLineYpos (nLine, dfMargin, Size);
                        }
                    }
                    if (pos > 0)
                    {
                        if ((RatioStatistics[i].mode == AspectSettings.AspectMode) && (RatioStatistics[i].ratio == AspectSettings.SourceAspect))
                        {
                            Color = OSD_COLOR_CURRENT;
                        }
                        else
                        {
                            Color = -1;
                        }
                        sprintf (szInfo, "%04d - %.3f:1 %s", RatioStatistics[i].switch_count, RatioStatistics[i].ratio / 1000.0, RatioStatistics[i].mode == 2 ? "Anamorphic" : "Letterbox");
                        OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, (nCol == 1) ? dfMargin : 0.5, pos);
                        nLine++;
                    }
                }
            }
        }
        break;

    // CARD CALIBRATION SCREEN
    case 4:
        // Title
        OSD_AddText("Card calibration", Size*1.5, OSD_COLOR_TITLE, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (1, dfMargin, Size*1.5));

        // Video settings
        if (pCalibration->GetType() == CAL_MANUAL)
        {
            pSetting = pSource->GetBrightness();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Brightness : %03d", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (1, dfMargin, Size));
            }
            pSetting = pSource->GetContrast();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Contrast : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (2, dfMargin, Size));
            }
            pSetting = pSource->GetSaturationU();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color U : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (1, dfMargin, Size));
            }
            pSetting = pSource->GetSaturationV();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Color V : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (2, dfMargin, Size));
            }
            pSetting = pSource->GetHue();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Hue : %03u", pSetting->GetValue());
                OSD_AddText(szInfo, Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (3, dfMargin, Size));
            }
        }
        else
        {
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
            }
            else
            {
                strcpy(szInfo, "Not Supported on this Source");
            }

            OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (8, dfMargin, Size));

            // do Contrast
            pSetting = pSource->GetContrast();
            if(pSetting != NULL)
            {
                sprintf (szInfo, "Contrast : %03d", pSetting->GetValue());
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
            }
            else
            {
                strcpy(szInfo, "Not Supported on this Source");
            }
            OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (9, dfMargin, Size));



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
            }
            else
            {
                strcpy(szInfo, "Not Supported on this Source");
            }
            OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (10, dfMargin, Size));


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
            }
            else
            {
                strcpy(szInfo, "Not Supported on this Source");
            }
            OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (11, dfMargin, Size));

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
            }
            else
            {
                strcpy(szInfo, "Not Supported on this Source");
            }
            OSD_AddText(szInfo, Size, Color, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (12, dfMargin, Size));
        }

		// Name of the test pattern
		pTestPattern = pCalibration->GetCurrentTestPattern();
        if (pTestPattern != NULL)
		{
            OSD_AddText(pTestPattern->GetName(), Size, OSD_COLOR_SECTION, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (3, dfMargin, Size));
		}

        if (pCalibration->IsRunning() && (pTestPattern != NULL))
		{
            if (pCalibration->GetType() != CAL_MANUAL)
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
                    strcpy(szInfo, "Adjusting Saturation U ...");
                    break;
                case 16:
                case 17:
                case 18:
                case 19:
                    strcpy(szInfo, "Adjusting Saturation V ...");
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
                OSD_AddText(szInfo, Size, OSD_COLOR_CURRENT, -1, OSDBACK_LASTONE, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (5, dfMargin, Size));
            }

            if ( (pCalibration->GetType() == CAL_MANUAL)
              || (pCalibration->GetCurrentStep() == -1) )
            {
                nLine = 4;

                if ( Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
                  && (pCalibration->GetType() == CAL_MANUAL) )
                {
                    OSD_AddText("Delta RGB", Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                }
                if ( Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA))
                  && (pCalibration->GetType() == CAL_MANUAL) )
                {
                    OSD_AddText("Delta YUV", Size, -1, -1, OSDBACK_LASTONE, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                }
                nLine++;

        		pSubPattern = pCalibration->GetCurrentSubPattern();
                if (pSubPattern != NULL)
                {
                    pColorBar = pSubPattern->GetFirstColorBar();
                }
                else
                {
                    pColorBar = NULL;
                }
                while (pColorBar != NULL)
	    		{
                    pColorBar->GetRefColor(FALSE, &val1, &val2, &val3);
	    			BackColor = RGB(val1, val2, val3);

                    pColorBar->GetDeltaColor(FALSE, &dif_val1, &dif_val2, &dif_val3, &dif_total1);
                    if ( Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
                      && (pCalibration->GetType() == CAL_MANUAL) )
                    {
                        sprintf (szInfo, "(%+d,%+d,%+d) (%d)", dif_val1, dif_val2, dif_val3, dif_total1);
                        OSD_AddText(szInfo, Size, -1, BackColor, OSDB_SHADED, OSD_XPOS_LEFT, dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                    }
                    pColorBar->GetDeltaColor(TRUE, &dif_val1, &dif_val2, &dif_val3, &dif_total2);
                    if ( Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA))
                      && (pCalibration->GetType() == CAL_MANUAL) )
                    {
                        sprintf (szInfo, "(%d) (%+d,%+d,%+d)", dif_total2, dif_val1, dif_val2, dif_val3);
                        OSD_AddText(szInfo, Size, -1, BackColor, OSDB_SHADED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine, dfMargin, Size));
                    }

                    if (pCalibration->GetType() == CAL_MANUAL)
                    {
                        if ( Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
                          && ! Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA)) )
                        {
                            OSD_GetTextResult(dif_total1, szInfo, &Color);
                        }
                        else if ( ! Setting_GetValue(Calibr_GetSetting(SHOW_RGB_DELTA))
                               && Setting_GetValue(Calibr_GetSetting(SHOW_YUV_DELTA)) )
                        {
                            OSD_GetTextResult(dif_total2, szInfo, &Color);
                        }
                        else
                        {
                            OSD_GetTextResult((dif_total1 < dif_total2) ? dif_total1 : dif_total2, szInfo, &Color);
                        }
		    		    OSD_AddText(szInfo, Size, Color, BackColor, OSDB_SHADED, OSD_XPOS_CENTER, 0.5, OSD_GetLineYpos (nLine++, dfMargin, Size));
                    }
                    else
                    {
                        OSD_GetTextResult(dif_total2, szResult, &Color);
                        sprintf (szInfo, "%s (YUV %d)", szResult, dif_total2);
		    		    OSD_AddText(szInfo, Size, Color, BackColor, OSDB_SHADED, OSD_XPOS_RIGHT, 1 - dfMargin, OSD_GetLineYpos (nLine++, dfMargin, Size));
                    }

                    pColorBar = pSubPattern->GetNextColorBar();
	    		}
            }
		}
        break;

    default:
        break;
    }

    if (ActiveScreens[IdxCurrentScreen].lock)
    {
        bOverride = FALSE;
    }
    OSD_Show(hWnd, ShowType, ActiveScreens[IdxCurrentScreen].refresh_delay);
    bRestoreScreen = FALSE;
    if (ActiveScreens[IdxCurrentScreen].lock)
    {
        bOverride = TRUE;
    }
}

//---------------------------------------------------------------------------
// Display on screen the first information screen if no screen is already
// displayed, or the next one
void OSD_ShowNextInfosScreen(HWND hWnd, double Size)
{
    int     NbScreens;          // number of OSD scrrens
    int     IdxScreen;
    int     PrevIdxScreen;
    int     i;

    if (bOverride)
    {    
        return;
    }

    // determine which screen to display
    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    PrevIdxScreen = IdxCurrentScreen;
    IdxScreen = IdxCurrentScreen + 1;
    IdxCurrentScreen = -1;
    for (i = IdxScreen ; i < NbScreens ; i++)
    {
        if (ActiveScreens[i].active && !ActiveScreens[i].managed_by_app)
        {
            if (IdxCurrentScreen == -1)
            {
                IdxCurrentScreen = i;
            }
        }
    }
    // Case : no screen to display
    if (IdxCurrentScreen == -1)
    {
        // If there was a screen displayed
        if (PrevIdxScreen != -1)
        {
            OSD_Clear(hWnd);
        }
        return;
    }

    OSD_RefreshInfosScreen(hWnd, Size, ActiveScreens[IdxCurrentScreen].auto_hide ? OSD_AUTOHIDE : OSD_PERSISTENT);
}

//---------------------------------------------------------------------------
// Display on screen the information screen whose number is given as parameter
void OSD_ShowInfosScreen(HWND hWnd, int IdxScreen, double Size)
{
    int     NbScreens;      // number of OSD scrrens
    int     PrevIdxScreen;

    if (bOverride)
    {    
        return;
    }

    PrevIdxScreen = IdxCurrentScreen;
    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    if ((IdxScreen < 0) || (IdxScreen >= NbScreens))
        IdxCurrentScreen = -1;
    else if (! ActiveScreens[IdxScreen].active)
        IdxCurrentScreen = -1;
    else
        IdxCurrentScreen = IdxScreen;

    // Case : no screen to display
    if (IdxCurrentScreen == -1)
    {
        // If there was a screen displayed
        if (PrevIdxScreen != -1)
            OSD_Clear(hWnd);
        return;
    }

    OSD_RefreshInfosScreen(hWnd, Size, ActiveScreens[IdxScreen].auto_hide ? OSD_AUTOHIDE : OSD_PERSISTENT);
}

//---------------------------------------------------------------------------
// Activate or desactivate the information screen whose number is given as parameter
void OSD_ActivateInfosScreen(HWND hWnd, int IdxScreen, double Size)
{
    int     NbScreens;      // number of OSD scrrens

    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    if ((IdxScreen >= 0) && (IdxScreen < NbScreens))
    {
        ActiveScreens[IdxScreen].active = ! ActiveScreens[IdxScreen].active;
        if (IdxScreen == IdxCurrentScreen)
        {
            OSD_Clear(hWnd);
        }
    }
}

void OSD_UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuOSD1;
    HMENU           hMenuOSD2;
    int             i;
    int             NbScreens;

    hMenuOSD1 = GetOSDSubmenu1();
    hMenuOSD2 = GetOSDSubmenu2();
    if ((hMenuOSD1 == NULL) || (hMenuOSD2 == NULL))
    {
        return;
    }

    i = GetMenuItemCount(hMenuOSD1);
    while (i)
    {
        i--;
        RemoveMenu(hMenuOSD1, i, MF_BYPOSITION);
    }
    i = GetMenuItemCount(hMenuOSD2);
    while (i)
    {
        i--;
        RemoveMenu(hMenuOSD2, i, MF_BYPOSITION);
    }
    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    for (i=0; i < NbScreens ; i++)
    {
        if ((strlen (ActiveScreens[i].name) > 0) && !ActiveScreens[i].managed_by_app)
        {
            UINT Flags(MF_STRING);
            AppendMenu(hMenuOSD1, Flags, IDM_OSDSCREEN_SHOW + i, ActiveScreens[i].name);

            Flags | ActiveScreens[i].active ? MF_CHECKED : MF_ENABLED;
            AppendMenu(hMenuOSD2, MF_STRING, IDM_OSDSCREEN_ACTIVATE + i, ActiveScreens[i].name);
        }
    }
}

void OSD_SetMenu(HMENU hMenu)
{
    HMENU   hMenuOSD1;
    HMENU   hMenuOSD2;
    int     i;
    int     NbScreens;

    hMenuOSD1 = GetOSDSubmenu1();
    hMenuOSD2 = GetOSDSubmenu2();
    if ((hMenuOSD1 == NULL) || (hMenuOSD2 == NULL))
    {
        return;
    }

    NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);
    for (i=0 ; i < NbScreens ; i++)
    {
        if ((strlen (ActiveScreens[i].name) > 0) && !ActiveScreens[i].managed_by_app)
        {
            EnableMenuItem(hMenuOSD1, i, ActiveScreens[i].active ? MF_BYPOSITION | MF_ENABLED : MF_BYPOSITION | MF_GRAYED);
            CheckMenuItem(hMenuOSD2, i, ActiveScreens[i].active ? MF_BYPOSITION | MF_CHECKED : MF_BYPOSITION | MF_UNCHECKED);
        }
    }
}

BOOL ProcessOSDSelection(HWND hWnd, WORD wMenuID)
{
    if ( (wMenuID >= IDM_OSDSCREEN_ACTIVATE) && (wMenuID < (IDM_OSDSCREEN_ACTIVATE+10)) )
    {
        OSD_ActivateInfosScreen(hWnd, wMenuID - IDM_OSDSCREEN_ACTIVATE, 0);
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

    bRestoreScreen = FALSE;
    bAutoHide = NewValue;

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
        "OSD Outline Color", SLIDER, 0, (long*)&OutlineColor,
         RGB(0,0,0), 0, RGB(255,255,255), 1, 1,
         NULL,
        "OSD", "OutlineColor", NULL,
    },
    {
        "OSD Text Color", SLIDER, 0, (long*)&TextColor,
         RGB(0,255,0), 0, RGB(255,255,255), 1, 1,
         NULL,
        "OSD", "TextColor", NULL,
    },
    {
        "OSD Default Size", SLIDER, 0, (long*)&DefaultSizePerc,
         10, 0, 100, 1, 1,
         NULL,
        "OSD", "DefaultSizePerc", NULL,
    },
    {
        "OSD Default Small Size", SLIDER, 0, (long*)&DefaultSmallSizePerc,
         5, 0, 100, 1, 1,
         NULL,
        "OSD", "DefaultSmallSizePerc", NULL,
    },
    {
        "OSD Anti Alias", ONOFF, 0, (long*)&bAntiAlias,
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "AntiAlias", NULL,
    },
    {
        "OSD Background", SLIDER, 0, (long*)&Background,
         OSDB_TRANSPARENT, 0, OSDBACK_LASTONE - 1, 1, 1,
         NULL,
        "OSD", "Background", NULL,
    },
    {
        "OSD Outline Text", ONOFF, 0, (long*)&bOutline,
         TRUE, 0,  1, 1, 1,
         NULL,
        "OSD", "Outline", NULL,
    },
    {
        "OSD Auto Hide Texts", ONOFF, 0, (long*)&bAutoHide,
         TRUE, 0, 1, 1, 1,
         NULL,
        "OSD", "AutoHide", OSD_AutoHide_OnChange,
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
    int i;
    char szScreenSel[16];
    char szIniKey[16];
    int NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);

    for(i = 0; i < OSD_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(OSDSettings[i]));
    }
    OSD_AutoHide_OnChange(bAutoHide);
    GetPrivateProfileString("OSD", "FontName", "Arial", szFontName, sizeof(szFontName) , GetIniFileForSettings());
    for(i = 0; i < NbScreens; i++)
    {
        if (!ActiveScreens[i].managed_by_app)
        {
            sprintf(szIniKey, "Screen%uSelected", i+1);
            GetPrivateProfileString("OSD", szIniKey, "undefined", szScreenSel, sizeof(szScreenSel) , GetIniFileForSettings());
            if (!strcmp(szScreenSel, "0"))
            {
                ActiveScreens[i].active = FALSE;
            }
            else if (!strcmp(szScreenSel, "1"))
            {
                ActiveScreens[i].active = TRUE;
            }
        }
    }
}

void OSD_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    char szIniKey[16];
    int NbScreens = sizeof (ActiveScreens) / sizeof (ActiveScreens[0]);

    for(i = 0; i < OSD_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(OSDSettings[i]), bOptimizeFileAccess);
    }
    WritePrivateProfileString("OSD", "FontName", szFontName, GetIniFileForSettings());
    for(i = 0; i < NbScreens; i++)
    {
        if (!ActiveScreens[i].managed_by_app)
        {
            sprintf(szIniKey, "Screen%uSelected", i+1);
            WritePrivateProfileString("OSD", szIniKey, ActiveScreens[i].active ? "1" : "0", GetIniFileForSettings());
        }
    }
}
