/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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

/** 
 * @file BitmapAsButton.h  BitmapAsButton Header file
 */

#ifndef _BITMAPASBUTTON_H_
#define _BITMAPASBUTTON_H_

#include "Bitmap.h"

typedef LRESULT (__cdecl BUTTONPROC)(std::string sID, void *pThis, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

enum eBitmapAsButtonType
{
    BITMAPASBUTTON_PUSH = 0,
    BITMAPASBUTTON_CHECKBOX,
    BITMAPASBUTTON_SLIDER,
    BITMAPASBUTTON_3STATE,
    BITMAPASBUTTON_4STATE
};

/** Simple button with different bitmaps for mouse over and click
    Calls user defined function for window message processing

    Button types:

     push button
        state: 0 = normal
               1 = mouse over
               2 = mouse down
     check box
        state: 0 = normal,       unchecked
               1 = mouse over,  unchecked
               2 = mouse down,  unchecked
               3 = normal,      checked
               4 = mouse over,  checked
               5 = mouse down,  checked
        
    slider bar:
        state: 0 = unselected bar
        state: 1 = unselected bar, mouse over
        state: 2 = unselected bar, mouse down
        state: 3 = selected bar
        state: 4 = selected bar, mouse over
        state: 5 = selected bar, mouse down        
        state: 6 = slider
        state: 7 = slider, mouse over
        state: 8 = slider, mouse down        
  
    3state/4state:
        state: 0 = state 0
        state: 1 = state 0, mouse over
        state: 2 = state 0, mouse click
        state: 3 = state 1
        state: 4 = state 1, mouse over
        state: 5 = state 1, mouse click
        ...
*/

class CBitmapAsButton {
protected:
    std::string sID;

    eBitmapAsButtonType  ButtonType;
    int  ButtonState;

    HWND hWndParent;
    HWND hWndButton;
    BOOL bFailed;

    CBitmapHolder m_bhBmp;
    
    int ButtonWidth;
    int ButtonHeight;
    
    BUTTONPROC* pfnButtonProc;
    void *pfnButtonProc_pThis;
    void *pOriginalProc;
    int OriginalWidth;
    int OriginalHeight;
    
    HCURSOR hCursorHand;
    //
    BOOL m_mouseldown;
    BOOL m_mouserdown;
    BOOL m_mouseover;
    BOOL m_trackmouse;
    // Checkbox/3state/4state specific
    
    // Slider specific
    int SliderPos;
    int SliderRangeMin;
    int SliderRangeMax;
protected:

    void Draw(HDC hDC, LPRECT lpRect);
    void SetWindowRegion();
    
    LRESULT ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK StaticButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);    
public:
    CBitmapAsButton(eBitmapAsButtonType ButtonType);
    ~CBitmapAsButton();

    HWND hWnd() { if (bFailed) return NULL; return hWndButton; }
    int Width() { return ButtonWidth; }
    int Height() { return ButtonHeight; }
    std::string GetID() { return sID; }

    void AddBitmap(int State, SmartPtr<CBitmapState> BitmapState);

    HRGN SetRegion();
    
    void SetProcessMessage(void *pThis, BUTTONPROC* pfnButtonProc) 
    { 
        this->pfnButtonProc_pThis = pThis;
        this->pfnButtonProc = pfnButtonProc; 
    }

    BOOL Create(std::string sID, HWND hWndParent, int x, int y, HINSTANCE hInst);
    BOOL TakeOver(HWND hWnd, std::string sID, HWND hWndParent);
    BOOL RestoreBack(HWND hWnd = NULL);
};


#endif
