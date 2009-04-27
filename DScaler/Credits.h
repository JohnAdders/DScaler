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
 * @file Credits.h  Credits Header file
 */

#if !defined(__CREDITS_H_)
#define __CREDITS_H_

#include "DSDialog.h"

/** Credits Dialog
    Creates a scolling credit screen
*/
class CCredits : public CDSDialog
{
// Construction
public:
    CCredits();   // standard constructor

private:

    RECT        m_ScrollRect;               // rect of Static Text frame
    int         m_nArrIndex;                // work ints
    int         m_nCounter;
    std::string m_szWork;                   // holds display line
    BOOL        m_bFirstTime;
    int         m_nClip;
    int         m_nCurrentFontHeight;

    HWND        m_pDisplayFrame;

    HBITMAP     m_BmpWork;
    HGDIOBJ     m_BmpOld;

    int          m_BitmapWidth;             // drawing helpers
    int          m_BitmapHeight;            // drawing helpers
    POINT        m_DisplayPoint;
    HDC          m_dcMem;
    BOOL         m_bProcessingBitmap;
    WNDPROC      m_OldWndProc;

    virtual BOOL DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    void OnPaint(HWND hDlg);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnTimer(HWND hDlg, UINT nIDEvent);
    static BOOL CALLBACK StaticWndProc(HWND hStatic, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
