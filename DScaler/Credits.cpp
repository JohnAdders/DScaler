/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 1998-2001 Mark Findlay.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file Credits.cpp Credits Dialog
 */


// NOTE: Be sure you have defined your bitmaps by assigning them a resource
//       id of a quoted tstring (i.e. _T("BITMAP1") instead of ID_BITMAP1) since
//       you will be designating the bitmaps by their quoted resource tstring
//       in this file.


//      Thanks to Gerry Shaw for submitting code to fix the redraw so that only
//      the actual display window refreshes each time instead of the entire
//      dialog which was causing annoying flicker.

#include "stdafx.h"
#include "Credits.h"
#include "..\DScalerRes\resource.h"
#include "DScaler.h"

using namespace std;

#define DISPLAY_TIMER_ID  150       // timer id

#define SCROLLAMOUNT                -1
#define DISPLAY_SLOW                70
#define DISPLAY_MEDIUM                40
#define DISPLAY_FAST                10
#define DISPLAY_SPEED                DISPLAY_MEDIUM

#define RED                        RGB(255,0,0)
#define GREEN                    RGB(0,255,0)
#define BLUE                    RGB(0,0,255)
#define WHITE                   RGB(255,255,255)
#define YELLOW                  RGB(255,255,0)
#define TURQUOISE                 RGB(0,255,255)
#define PURPLE                  RGB(255,0,255)
#define BLACK                   RGB(0,0,0)

#define BACKGROUND_COLOR        BLACK
#define TOP_LEVEL_TITLE_COLOR    RED
#define TOP_LEVEL_GROUP_COLOR   YELLOW
#define GROUP_TITLE_COLOR       TURQUOISE
#define NORMAL_TEXT_COLOR        WHITE

// You can set font heights here to suit your taste
#define TOP_LEVEL_TITLE_HEIGHT    21
#define TOP_LEVEL_GROUP_HEIGHT  19
#define GROUP_TITLE_HEIGHT        17
#define    NORMAL_TEXT_HEIGHT        15



// these define the escape sequences to suffix each array line with. They
// are listed in order of descending font size.
#define TOP_LEVEL_TITLE            '\n'
#define TOP_LEVEL_GROUP         '\r'
#define GROUP_TITLE               '\t'
#define NORMAL_TEXT                '\f'
#define DISPLAY_BITMAP            '\b'

// Enter the text and bitmap resource tstring identifiers which are to
// appear in the scrolling list. Append each non-empty tstring entry with
// an escape character corresponding to the desired font effect. Empty
// strings do not get a font assignment.
// (see #defines above)

TCHAR* ArrCredits[] = {  _T("DScaler\n"),
                        _T(""),
                        _T("Copyright © 2012\f"),
                        _T("DScaler Team\f"),
                        _T("All Rights Reserved\f"),
                        _T(""),
                        _T("SMALLLOGO\b"),
                        _T(""),
                        _T(""),
                        _T("Project Lead\t"),
                        _T(""),
                        _T("John Adcock\f"),
                        _T(""),
                        _T(""),
                        _T("Current Active Developers\t"),
                        _T(""),
                        _T("Torbjörn Jansson\f"),
                        _T("Atsushi Nakagawa\f"),
                        _T("Torsten Seeboth\f"),
                        _T(""),
                        _T(""),
                        _T("Other Developers\t"),
                        _T(""),
                        _T("Tom Barry\f"),
                        _T("Denis Balazuc\f"),
                        _T("Muljadi Budiman\f"),
                        _T("Lindsey Dubb\f"),
                        _T("Michael Eskin\f"),
                        _T("Laurent Garnier\f"),
                        _T("Steve Grimm\f"),
                        _T("Jeroen Kooiman\f"),
                        _T("Rob Muller\f"),
                        _T("Mark Rejhon\f"),
                        _T("Michael Samblanet\f"),
                        _T("Eric Schmidt\f"),
                        _T("Ivan Tarnavsky\f"),
                        _T("Jochen Trenner\f"),
                        _T("Mike Temperton\f"),
                        _T(""),
                        _T(""),
                        _T("Documentation \t"),
                        _T(""),
                        _T("Yvon Quere\f"),
                        _T(""),
                        _T(""),
                        _T("Web Sites & Forums\t"),
                        _T(""),
                        _T("Dan Schmelzer\f"),
                        _T(""),
                        _T(""),
                        _T("Graphics\t"),
                        _T(""),
                        _T("Erik Språng\f"),
                        _T("Robin Zalek\f"),
                        _T(""),
                        _T(""),
                        _T("Patches From\t"),
                        _T(""),
                        _T("Olivier Borca\f"),
                        _T("Billy Chang\f"),
                        _T("Tom Fojta\f"),
                        _T("Péter Gulyás\f"),
                        _T("John Isar\f"),
                        _T("Piet Orye\f"),
                        _T("Kevin Radke\f"),
                        _T("Eduardo José Tagle\f"),
                        _T("Kristian Trenskow\f"),
                        _T("Arjan Zipp\f"),
                        _T("Tom Zoerner\f"),
                        _T(""),
                        _T(""),
                        _T("Projects we've borrowed from\t"),
                        _T(""),
                        _T("bttv - Copyright © Ralph Metzler & Marcus Metzler\f"),
                        _T("Credits screen - Copyright © Mark Findlay\f"),
                        _T("FreeTV Driver - Copyright © Mathias Ellinger\f"),
                        _T("I2C Interface - Copyright © Gerd Knorr\f"),
                        _T("MultiDec - Copyright © Espresso\f"),
                        _T("saa713x - Copyright © Gerd Knorr\f"),
                        _T("VirtualDub - Copyright © Avery Lee\f"),
                        _T("LinuxTV - www.linuxtv.org \f"),
                        _T(""),
                        _T(""),
                        _T("Libraries we use\t"),
                        _T(""),
                        _T("LibJpeg - A work of the Independent JPEG Group\f"),
                        _T("LibTiff - Copyright © Sam Leffler & Silicon Graphics\f"),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        _T(""),
                        };

#define ARRAYCOUNT        (sizeof(ArrCredits)/sizeof(char*))


/////////////////////////////////////////////////////////////////////////////
// CCredits dialog


CCredits::CCredits() : CDSDialog(MAKEINTRESOURCE(IDD_CREDITS))
{
}

INT_PTR CCredits::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
        HANDLE_MSG(hDlg, WM_TIMER, OnTimer);
    default:
        return FALSE;
    }
}

void CCredits::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDCANCEL:
    case IDOK:
        // clean up after ourselves
        KillTimer(hDlg, DISPLAY_TIMER_ID);
        if (m_bProcessingBitmap)
        {
            SelectObject(m_dcMem, m_BmpOld);
            DeleteObject(m_BmpWork);
        }
        DeleteDC(m_dcMem);
        RemoveProp(m_pDisplayFrame, _T("DSSubClass"));
        EndDialog(hDlg, id);
        break;
    }
}

INT_PTR CALLBACK CCredits::StaticWndProc(HWND hStatic, UINT message, WPARAM wParam, LPARAM lParam)
{
    CCredits* myDialogObject = (CCredits*)GetProp(hStatic, _T("DSSubClass"));
    if(myDialogObject)
    {
        if(message != WM_PAINT)
        {
            return myDialogObject->m_OldWndProc(hStatic, message, wParam, lParam);
        }
        else
        {
            myDialogObject->OnPaint(hStatic);
        }
    }
    return FALSE;
}


//************************************************************************
//     InitDialog
//
//         Setup the display rect and start the timer.
//************************************************************************
BOOL CCredits::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    UINT_PTR nRet;

    m_nCurrentFontHeight = NORMAL_TEXT_HEIGHT;

    m_bProcessingBitmap=FALSE;

    m_nArrIndex = 0;
    m_nCounter = 1;
    m_nClip=0;
    m_bFirstTime=TRUE;
    m_BmpOld = 0;

    m_pDisplayFrame = GetDlgItem(hDlg, IDC_CREDITS_STATIC);

    m_dcMem = CreateCompatibleDC(GetDC(m_pDisplayFrame));

    // If you assert here, you did not assign your static display control
    // the IDC_ value that was used in the GetDlgItem(...). This is the
    // control that will display the credits.
    _ASSERTE(m_pDisplayFrame);

    GetClientRect(m_pDisplayFrame, &m_ScrollRect);

    nRet = SetTimer(hDlg, DISPLAY_TIMER_ID, DISPLAY_SPEED, NULL);
    _ASSERTE(nRet != 0);

    SetProp(m_pDisplayFrame, _T("DSSubClass"), (HANDLE)this);
    m_OldWndProc = (WNDPROC)SetWindowLongPtr(m_pDisplayFrame, GWLP_WNDPROC, (LONG_PTR)StaticWndProc);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//************************************************************************
//     OnTimer
//
//         On each of the display timers, scroll the window 1 unit. Each 20
//      units, fetch the next array element and load into work tstring. Call
//      Invalidate and UpdateWindow to invoke the OnPaint which will paint
//      the contents of the newly updated work tstring.
//************************************************************************
void CCredits::OnTimer(HWND hDlg, UINT nIDEvent)
{
    if (!m_bProcessingBitmap)
    {
        if (m_nCounter++ % m_nCurrentFontHeight == 0)     // every x timer events, show new line
        {
            m_nCounter=1;
            m_szWork = ArrCredits[m_nArrIndex++];

            if (m_nArrIndex > ARRAYCOUNT-1)
            {
                m_nArrIndex = 0;
            }
            m_nClip = 0;
        }
    }

    ScrollWindow(m_pDisplayFrame, 0,SCROLLAMOUNT,&m_ScrollRect,&m_ScrollRect);
    m_nClip = m_nClip + abs(SCROLLAMOUNT);

    InvalidateRect(m_pDisplayFrame, NULL, FALSE); // FALSE does not erase background
}


HFONT CreateCreditsFont(int Height, BOOL Bold, BOOL Underline)
{
    return CreateFont(Height, 0, 0, 0,
                       Bold?FW_BOLD:FW_THIN, 
                       FALSE, Underline, 0,
                       ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS,
                       PROOF_QUALITY,
                       VARIABLE_PITCH | 0x04 | FF_DONTCARE,
                       _T("Arial"));
}

//************************************************************************
//     OnPaint
//
//     Send the newly updated work tstring to the display rect.
//************************************************************************
void CCredits::OnPaint(HWND hDlg)
{
    PAINTSTRUCT ps;
    HDC Dc = BeginPaint(hDlg, &ps);

    SetBkMode(Dc, TRANSPARENT);


    //*********************************************************************
    //    FONT SELECTION
    HFONT fntArial = 0;
    HGDIOBJ pOldFont = 0;

    if (!m_szWork.empty())
    {
        switch(m_szWork[m_szWork.length()-1])
        {
        case NORMAL_TEXT:
        default:
            m_nCurrentFontHeight = NORMAL_TEXT_HEIGHT;
            fntArial = CreateCreditsFont(m_nCurrentFontHeight, FALSE, FALSE);
            SetTextColor(Dc, NORMAL_TEXT_COLOR);
            pOldFont = SelectObject(Dc, fntArial);
            break;

        case TOP_LEVEL_GROUP:
            m_nCurrentFontHeight = TOP_LEVEL_GROUP_HEIGHT;
            fntArial = CreateCreditsFont(m_nCurrentFontHeight, TRUE, FALSE);
            SetTextColor(Dc, TOP_LEVEL_GROUP_COLOR);
            pOldFont = SelectObject(Dc, fntArial);
            break;



        case GROUP_TITLE:
            m_nCurrentFontHeight = GROUP_TITLE_HEIGHT;
            fntArial = CreateCreditsFont(m_nCurrentFontHeight, TRUE, FALSE);
            SetTextColor(Dc, GROUP_TITLE_COLOR);
            pOldFont = SelectObject(Dc, fntArial);
            break;


        case TOP_LEVEL_TITLE:
            m_nCurrentFontHeight = TOP_LEVEL_TITLE_HEIGHT;
            fntArial = CreateCreditsFont(m_nCurrentFontHeight, TRUE, TRUE);
            SetTextColor(Dc, TOP_LEVEL_TITLE_COLOR);
            pOldFont = SelectObject(Dc, fntArial);
            break;

        case DISPLAY_BITMAP:
            if (!m_bProcessingBitmap)
            {
                BITMAP bmpInfo;
                tstring szBitmap = m_szWork.substr(0, m_szWork.length()-1);
                m_BmpWork = LoadBitmap(hResourceInst, szBitmap.c_str());
                if(m_BmpWork == NULL)
                {
                    tstring str(MakeString() << _T("Could not find bitmap resource ") << 
                                            szBitmap << _T(". ") << 
                                            _T("Be sure to assign the bitmap a QUOTED resource name"));
                    KillTimer(hDlg, DISPLAY_TIMER_ID);
                    ErrorBox(str.c_str());
                    return;
                }
                GetObject(m_BmpWork, sizeof(BITMAP), &bmpInfo);

                m_BitmapWidth = bmpInfo.bmWidth;    // width  of dest rect
                m_BitmapHeight = bmpInfo.bmHeight;
                RECT workRect;
                GetClientRect(m_pDisplayFrame, &workRect);
                // upper left point of dest
                m_DisplayPoint.x = (workRect.right -
                            ((workRect.right-workRect.left)/2) - (bmpInfo.bmWidth/2));
                m_DisplayPoint.y = workRect.bottom;

                m_BmpOld = SelectObject(m_dcMem, m_BmpWork);
                m_bProcessingBitmap = TRUE;
            }
            break;

        }
    }

    HBRUSH bBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    HGDIOBJ pOldBrush;
    pOldBrush  = SelectObject(Dc, bBrush);
    // Only fill rect comprised of gap left by bottom of scrolling window
    RECT r=m_ScrollRect;
    r.top = r.bottom-abs(SCROLLAMOUNT);
    DPtoLP(Dc, (LPPOINT)&r, 2);

    if (m_bFirstTime)
    {
        m_bFirstTime=FALSE;
        FillRect(Dc, &m_ScrollRect, bBrush);
    }
    else
    {
        FillRect(Dc, &r, bBrush);
    }

    r = m_ScrollRect;
    r.top = r.bottom - m_nClip;


    if (!m_bProcessingBitmap)
    {
        int x = DrawText(Dc, m_szWork.c_str(),m_szWork.length()-1,&r,DT_TOP|DT_CENTER|
                    DT_NOPREFIX | DT_SINGLELINE);

        SelectObject(Dc, pOldFont);
        DeleteObject(fntArial);
    }
    else
    {
        StretchBlt(Dc, m_DisplayPoint.x, m_DisplayPoint.y-m_nClip, m_BitmapWidth, m_nClip,
                           m_dcMem, 0, 0, m_BitmapWidth - 1, m_nClip,
                           SRCCOPY );
        if (m_nClip > m_BitmapHeight)
        {
            SelectObject(m_dcMem, m_BmpOld);
            DeleteObject(m_BmpWork);
            m_bProcessingBitmap = FALSE;
            m_nClip=0;
            m_szWork = _T("");
            m_nCounter=1;
        }
    }

    SelectObject(Dc, pOldBrush);

    EndPaint(hDlg, &ps);
}
