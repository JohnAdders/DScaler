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
 * @file WindowBorder.h WindowBorder Header file
 */
 
#ifndef _WINDOWBORDER_H_
#define _WINDOWBORDER_H_

#include "bitmap.h"
#include "BitmapAsButton.h"

enum eWindowBorderPosition
{
    WINDOWBORDER_TOPLEFTCORNER = 0,
    WINDOWBORDER_TOPLEFT,
    WINDOWBORDER_TOPMIDDLE,
    WINDOWBORDER_TOPRIGHT,
    WINDOWBORDER_TOPRIGHTCORNER,
    WINDOWBORDER_RIGHTTOP,
    WINDOWBORDER_RIGHTMIDDLE,
    WINDOWBORDER_RIGHTBOTTOM,
    WINDOWBORDER_BOTTOMRIGHTCORNER,
    WINDOWBORDER_BOTTOMRIGHT,
    WINDOWBORDER_BOTTOMMIDDLE,
    WINDOWBORDER_BOTTOMLEFT,
    WINDOWBORDER_BOTTOMLEFTCORNER,
    WINDOWBORDER_LEFTBOTTOM,
    WINDOWBORDER_LEFTMIDDLE,
    WINDOWBORDER_LEFTTOP,
    WINDOWBORDER_LASTONE
};

//
//
//    0   1    -2-    3   5
//    
//   15                   4
//
//
//   |                    |
//   14                   6
//   |                    |
//    
//
//   13                   7
//
//   12   11   -10-    9  8



class CWindowBorder
{
protected:
    HWND hWnd;
    HINSTANCE hResourceInst;

    int IsBorderVisible;

    vector< SmartPtr<CBitmapHolder> > Bitmaps;
    int SolidTopSize;
    int SolidRightSize;
    int SolidBottomSize;
    int SolidLeftSize;

    
    typedef struct {
      string sID;
      SmartPtr<CBitmapAsButton> Button;
      POINT Location;
      int   RelativePosition;
      POINT LastLocation;

      string sIniEntryDefault;
      string sIniEntryMouseOver;
      string sIniEntryClick;
    } TButtonInfo;

    vector<TButtonInfo> Buttons;
    
    int TopSize;
    int RightSize;
    int BottomSize;
    int LeftSize;

    RECT rcWindow;

    COLORREF DefaultColorRef;
    HBRUSH m_hBrush;
    BOOL bBitmapsChanged;
    BOOL m_IsSkinned;

    vector<RECT> Locations;
    vector<RECT> RegionList;

    HRGN hLastRegion;

    void MergeLineRegion(int y,POINT *RowList,int RowListSize,int TotalWidth,vector<RECT>& AllRegions);
    void MergeBorderRegions(vector<RECT>& AllRegions, LPRECT lpRcExtra);    
    BOOL FindBorderSizes();
    BOOL FindLocations();

    void UpdateButtonLocations();

    BOOL (*CustomGetClientRect)(HWND hWnd, LPRECT lpRect);
    
    int BmpWidth(int Position);
    int BmpHeight(int Position);
    int BorderWidth(int Position);
    int BorderHeight(int Position);

    static const char* szBorderNames[];
public:
    CWindowBorder(HWND hWnd, HINSTANCE hInst, BOOL (*CustomGetClientRect)(HWND hWnd, LPRECT lpRect) = NULL);
    ~CWindowBorder();
    
    void AdjustArea(RECT *ar, int Crop);
    

    void DefaultColor(COLORREF Color);
    void SolidBorder(int left,int top, int right, int bottom, COLORREF Color);
    HRGN MakeRegion(LPRECT lpRcExtra);
    
    BOOL SetBorderBitmap(eWindowBorderPosition Position, int State, SmartPtr<CBitmapState> BitmapState, int DrawMode);
    BOOL SetButtonBitmap(string sID, int WhichBitmap, eWindowBorderPosition RelPos, int x, int y, SmartPtr<CBitmapState> BitmapState);
    
    BOOL RegisterButton(string sID, eBitmapAsButtonType ButtonType, string sIniEntryDefault, string sIniEntryMouseOver, string sIniEntryClick, BUTTONPROC *pfnButtonProc);
    
    void Paint(HWND hWnd, HDC hDC, LPRECT lpRect, POINT *pPShift = NULL);
    
    BOOL Visible() { return (IsBorderVisible>0); };
    BOOL Show() { IsBorderVisible=1; UpdateButtonLocations(); return TRUE; }
    BOOL Hide() { IsBorderVisible=0; UpdateButtonLocations(); return TRUE; }

    virtual BOOL LoadSkin(const char *szSkinIniFile,  const char *szSection, vector<int> *Results);
    virtual void ClearSkin();
    BOOL IsSkinned() { return m_IsSkinned; }
};



#endif

