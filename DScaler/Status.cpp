/////////////////////////////////////////////////////////////////////////////
// $Id: Status.cpp,v 1.7 2001-08-15 09:41:16 adcockj Exp $
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
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.5  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Status.h"
#include "DScaler.h"

#if !defined (APIENTRY)
#define APIENTRY FAR PASCAL
#endif

#define MAXSTATUS 5
typedef struct
{
    HWND hWnd;
    int MaxWidth;
    int MinWidth;
    int GivenWidth;
} TStatus;

TStatus StatusFields[MAXSTATUS];

HWND hwndStatusBar = NULL;

int NumStatusFields = 0;
int StatusY, StatusBorderX, StatusBorderY, FrameX, FrameY, FieldY;
HFONT hfontStatus;
TEXTMETRIC tmStatusFont;
HBRUSH hbrBtnFace;

LONG APIENTRY StatusProc(HWND, UINT, UINT, LONG);
LONG APIENTRY StatusFieldProc(HWND, UINT, UINT, LONG);

HWND hwndTextField;
HWND hwndPalField;
HWND hwndKeyField;
HWND hwndFPSField;
HWND hwndAudioField;

#define ID_STATUSBAR    1700
#define ID_TEXTFIELD       ID_STATUSBAR+1
#define ID_KENNUNGFFIELD   ID_STATUSBAR+2
#define ID_CODEFIELD       ID_STATUSBAR+3
#define ID_FPSFIELD        ID_STATUSBAR+4
#define ID_AUDIOFIELD      ID_STATUSBAR+5

BOOL StatusBar_Init()
{
    WNDCLASS wndclass;

    hbrBtnFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = (WNDPROC) StatusProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInst;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = hbrBtnFace;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "SamplerStatus";

    if (!RegisterClass(&wndclass))
        return FALSE;

    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = (WNDPROC) StatusFieldProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInst;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = hbrBtnFace;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = "StatusField";

    if (!RegisterClass(&wndclass))
        return FALSE;

    StatusBorderX = GetSystemMetrics(SM_CXBORDER);
    StatusBorderY = GetSystemMetrics(SM_CYBORDER);

    hwndStatusBar = CreateWindow("SamplerStatus", "SamplerStatus", WS_CHILD | WS_BORDER | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)ID_STATUSBAR, hInst, NULL);

    if (!hwndStatusBar)
    {
        return FALSE;
    }
    
    hwndTextField = StatusBar_AddField(ID_TEXTFIELD, 110, 0, FALSE);
    hwndAudioField = StatusBar_AddField(ID_AUDIOFIELD, 110, 0, FALSE);
    hwndPalField = StatusBar_AddField(ID_CODEFIELD, 110, 0, FALSE);
    hwndKeyField = StatusBar_AddField(ID_KENNUNGFFIELD, 90, 50, FALSE);
    hwndFPSField = StatusBar_AddField(ID_FPSFIELD, 45, 45, TRUE);

    StatusBar_Adjust(hWnd);

    return (TRUE);
}

void StatusBar_ShowText(eStatusBarBox Box, LPCTSTR szText)
{
    if (IsWindowVisible(hwndStatusBar))
    {
        switch(Box)
        {
        case STATUS_KEY:
            SetWindowText(hwndKeyField, szText);
            break;
        case STATUS_FPS:
            SetWindowText(hwndFPSField, szText);
            break;
        case STATUS_PAL:
            SetWindowText(hwndPalField, szText);
            break;
        case STATUS_TEXT:
            SetWindowText(hwndTextField, szText);
            break;
        case STATUS_AUDIO:
            SetWindowText(hwndAudioField, szText);
            break;
        default:
            break;
        }
    }
}

HWND StatusBar_GetHWnd(eStatusBarBox Box)
{
    switch(Box)
    {
    case STATUS_KEY:
        return hwndKeyField;
        break;
    case STATUS_FPS:
        return hwndFPSField;
        break;
    case STATUS_PAL:
        return hwndPalField;
        break;
    case STATUS_TEXT:
        return hwndTextField;
        break;
    case STATUS_AUDIO:
        return hwndAudioField;
        break;
    case STATUS_BAR:
        return hwndStatusBar;
        break;
    default:
        break;
    }
    return NULL;
}

BOOL StatusBar_Adjust(HWND hWnd)
{
    RECT rect;

    GetClientRect(hWnd, &rect);
    MoveWindow(hwndStatusBar, rect.left - StatusBorderX, rect.bottom - StatusY + StatusBorderY, rect.right - rect.left + (StatusBorderX * 2), StatusY, TRUE);
    InvalidateRect(hwndStatusBar, NULL, FALSE);
    return TRUE;
}

HWND StatusBar_AddField(int iId, int iMin, int iMax, BOOL bNewGroup)
{
    LONG lStyle;

    if (NumStatusFields >= MAXSTATUS)
        return (HWND) 0;        // No room left in our fixed array

    StatusFields[NumStatusFields].hWnd = CreateWindow("StatusField", "", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwndStatusBar, (HMENU) iId, hInst, NULL);

    if (!StatusFields[NumStatusFields].hWnd)
        return (HWND) 0;        // CreateWindow failed for some reason

    if (iMin < 0)
    {
        StatusFields[NumStatusFields].MinWidth = tmStatusFont.tmAveCharWidth * abs(iMin);
    }
    else
    {
        StatusFields[NumStatusFields].MinWidth = iMin;
    }

    if (iMax < 0)
    {
        StatusFields[NumStatusFields].MaxWidth = tmStatusFont.tmAveCharWidth * abs(iMax);
    }
    else
    {
        StatusFields[NumStatusFields].MaxWidth = iMax;
    }

    if (bNewGroup)
    {
        lStyle = GetWindowLong(StatusFields[NumStatusFields].hWnd, GWL_STYLE);
        lStyle |= WS_GROUP;
        SetWindowLong(StatusFields[NumStatusFields].hWnd, GWL_STYLE, lStyle);
    }

    return StatusFields[NumStatusFields++].hWnd;
}

LONG APIENTRY StatusProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    int x, y, i;
    int wAvailWidth, wFlexWidth, cntFlexWidth, wNeedWidth, cntNeedWidth;
    RECT rect, border;
    HBRUSH hBrush;

    switch (msg)
    {
    case WM_CREATE:
        hfontStatus = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        if (!hfontStatus)
        {
            ErrorBox("Failed To Create Statusbar Font");
        }
        hdc = GetDC(hWnd);
        SelectObject(hdc, hfontStatus);
        GetTextMetrics(hdc, &tmStatusFont);
        StatusBorderX = GetSystemMetrics(SM_CXBORDER);
        StatusBorderY = GetSystemMetrics(SM_CYBORDER);
        FrameX = 3 * StatusBorderX;
        FrameY = 3 * StatusBorderY;
        FieldY = tmStatusFont.tmHeight + (2 * StatusBorderY);
        StatusY = FieldY + (2 * FrameY);
        ReleaseDC(hWnd, hdc);
        return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_DESTROY:
        if (hfontStatus)
        {
            DeleteObject(hfontStatus);
        }
        break;

    case WM_SIZE:
        if (NumStatusFields)
        {
            GetClientRect(hWnd, &rect);
            wAvailWidth = rect.right - rect.left - (StatusBorderX * 8);
            wNeedWidth = 0;
            cntNeedWidth = 0;
            cntFlexWidth = 0;

            // First Pass: Dole out to fields that have a minimum need 
            for (i = 0; i < NumStatusFields; i++)
            {
                StatusFields[i].GivenWidth = 0;  // Make sure all are initialized to 0
                if (StatusFields[i].MinWidth)
                {
                    // (n, ?) 
                    StatusFields[i].GivenWidth = StatusFields[i].MinWidth;
                    wAvailWidth -= (StatusFields[i].GivenWidth + StatusBorderX * 2);
                    if (GetWindowLong(StatusFields[i].hWnd, GWL_STYLE) & WS_GROUP)
                    {
                        wAvailWidth -= StatusBorderX * 4;
                    }
                }
                else
                {
                    // They didn't specify a minimum... don't give them anything yet 
                    // (0, ?) 
                    StatusFields[i].GivenWidth = 0;
                }

                // For those that have a minimum, but can grow to be as large as possible... 
                // (n, 0) 
                if ((StatusFields[i].MinWidth > 0) && (StatusFields[i].MaxWidth == 0))
                {
                    ++cntFlexWidth;
                }

                // For those that have a max that is greater then their min... 
                // Includes (0,n) and (n,>n) 
                if (StatusFields[i].MaxWidth > StatusFields[i].GivenWidth)
                {
                    wNeedWidth += (StatusFields[i].MaxWidth - StatusFields[i].GivenWidth);
                    ++cntNeedWidth;
                }
            }

            // Second Pass: Dole out to fields that have a stated maximum need 
            // This will also hit those who had no minimum, but did have a maximum 
            // It will still not give anything to those with no min, no max 
            if ((cntNeedWidth > 0) && (wAvailWidth > 0))
            {
                if (wNeedWidth > wAvailWidth)
                {
                    wNeedWidth = wAvailWidth;
                }
                wNeedWidth = wNeedWidth / cntNeedWidth;
                for (i = 0; i < NumStatusFields; i++)
                {
                    if (StatusFields[i].MaxWidth > StatusFields[i].GivenWidth)
                    {
                        StatusFields[i].GivenWidth += wNeedWidth;
                        wAvailWidth -= (StatusFields[i].GivenWidth + StatusBorderX * 2);
                        if (GetWindowLong(StatusFields[i].hWnd, GWL_STYLE) & WS_GROUP)
                        {
                            wAvailWidth -= StatusBorderX * 4;
                        }
                    }
                }
            }

            // Third Pass: Dole out the remaining to fields that want all they can get 
            // This includes those who had a minimum, but no maximum 
            if ((cntFlexWidth > 0) && (wAvailWidth > 0))
            {
                wFlexWidth = wAvailWidth / cntFlexWidth;
                for (i = 0; i < NumStatusFields; i++)
                {
                    if (StatusFields[i].MaxWidth == 0)
                    {
                        StatusFields[i].GivenWidth += wFlexWidth;
                        wAvailWidth -= ((wFlexWidth - StatusFields[i].MinWidth) + StatusBorderX * 2);
                        if (GetWindowLong(StatusFields[i].hWnd, GWL_STYLE) & WS_GROUP)
                        {
                            wAvailWidth -= StatusBorderX * 4;
                        }
                    }
                }
            }

            x = StatusBorderX * 4;
            y = rect.top + (2 * StatusBorderY);
            for (i = 0; i < NumStatusFields; i++)
            {
                if (GetWindowLong(StatusFields[i].hWnd, GWL_STYLE) & WS_GROUP)
                {
                    x += (StatusBorderX * 4);
                }
                MoveWindow(StatusFields[i].hWnd, x, y, StatusFields[i].GivenWidth, FieldY, TRUE);
                x += StatusFields[i].GivenWidth + (StatusBorderX * 2);
            }
        }
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);

        hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        border = rect;
        border.bottom = border.top + StatusBorderY;
        FillRect(hdc, &border, hBrush);
        DeleteObject(hBrush);

        hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
        border = rect;
        border.top = border.bottom - StatusBorderY;
        FillRect(hdc, &border, hBrush);
        DeleteObject(hBrush);

        EndPaint(hWnd, &ps);

        return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_USER + 10:
        StatusBar_ShowText(STATUS_PAL, (LPCTSTR)lParam);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0L;
}

LONG APIENTRY StatusFieldProc(HWND hWnd, UINT msg, UINT wParam, LONG lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    RECT rect, border;
    HBRUSH hBrush;
    HFONT hTmp;
    char szText[80];
    int len;

    switch (msg)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rect);

        hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
        border = rect;
        border.bottom = border.top + StatusBorderY;
        FillRect(hdc, &border, hBrush);
        border = rect;
        border.right = border.left + StatusBorderX;
        FillRect(hdc, &border, hBrush);
        DeleteObject(hBrush);

        hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
        border = rect;
        border.top = border.bottom - StatusBorderY;
        FillRect(hdc, &border, hBrush);
        border = rect;
        border.left = border.right - StatusBorderX;
        FillRect(hdc, &border, hBrush);
        DeleteObject(hBrush);

        if (len = GetWindowText(hWnd, szText, sizeof(szText)))
        {
            hTmp = (HFONT)SelectObject(hdc, hfontStatus);

            SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
            SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));

            InflateRect(&rect, -(StatusBorderX * 2), -StatusBorderY);
            ExtTextOut(hdc, rect.left, rect.top, ETO_OPAQUE | ETO_CLIPPED, &rect, (LPSTR) szText, len, NULL);

            SelectObject(hdc, hTmp);
        }

        EndPaint(hWnd, &ps);
        break;

    case WM_SETTEXT:
        InvalidateRect(hWnd, NULL, TRUE);
        return DefWindowProc(hWnd, msg, wParam, lParam);

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0L;
}

void StatusBar_ShowWindow(BOOL bShow)
{
    ShowWindow(hwndStatusBar, (bShow)?SW_SHOW:SW_HIDE);
}

void StatusBar_Destroy(void)
{
    DestroyWindow(hwndStatusBar);   
    DeleteObject(hbrBtnFace);
}

void StatusBar_Repaint()
{
    if (IsWindowVisible(hwndStatusBar))
    {
        InvalidateRect(hwndStatusBar, NULL, TRUE);
    }
}

int StatusBar_Height()
{
    RECT rect;
    GetClientRect(hwndStatusBar, &rect);
    return (rect.bottom - rect.top);
}
