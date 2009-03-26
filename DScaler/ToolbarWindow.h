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
 * @file ToolbarWindow.h ToolbarWindow Header file
 */

#ifndef _TOOLBARWINDOW_H_
#define _TOOLBARWINDOW_H_

#include "BitmapAsButton.h"
#include "WindowBorder.h"

enum eToolbarRowAlign
{
    TOOLBARCHILD_ALIGN_LEFTTOP = 0,
    TOOLBARCHILD_ALIGN_LEFTCENTER,
    TOOLBARCHILD_ALIGN_LEFTBOTTOM,

    TOOLBARCHILD_ALIGN_RIGHTTOP,
    TOOLBARCHILD_ALIGN_RIGHTCENTER,
    TOOLBARCHILD_ALIGN_RIGHTBOTTOM,
};

#define TOOLBARBUTTON_ICON_HALIGN_LEFT 0
#define TOOLBARBUTTON_ICON_HALIGN_RIGHT 1
#define TOOLBARBUTTON_ICON_HALIGN_CENTER 2
#define TOOLBARBUTTON_ICON_VALIGN_TOP 0
#define TOOLBARBUTTON_ICON_VALIGN_BOTTOM 4
#define TOOLBARBUTTON_ICON_VALIGN_CENTER 8

#define TOOLBARBUTTON_TEXT_HALIGN_LEFT 0
#define TOOLBARBUTTON_TEXT_HALIGN_RIGHT 16
#define TOOLBARBUTTON_TEXT_HALIGN_CENTER 32
#define TOOLBARBUTTON_TEXT_VALIGN_TOP 0
#define TOOLBARBUTTON_TEXT_VALIGN_BOTTOM 64
#define TOOLBARBUTTON_TEXT_VALIGN_CENTER 128

class CToolbarChild;

class CToolbarWindow : public CWindowBorder
{
protected:
    HWND hWndToolbar;
    HWND hWndParent;

    typedef struct {
        BOOL bShow;
        int  Order;
        int  Row;
        eToolbarRowAlign  Align;
        CToolbarChild* pChild;
        CToolbarChild* pBarLeft;
        CToolbarChild* pBarRight;
    } TChildInfo;
    std::vector<TChildInfo> vChildList;

    std::vector<int> vChildOrder;
    int  ChildOrderRightPos;
    BOOL bChildOrderChanged;
    int  FitHeight;
    std::vector<int> vRowHeight;

    int TopMargin;
    int BottomMargin;
    int LeftMargin;
    int RightMargin;
    int ChildLeftRightMargin;
    int ChildTopBottomMargin;

    int IsToolbarVisible;
    int MainToolbarPosition;

    int m_lastWindowPos_x;
    int m_lastWindowPos_y;
    int m_lastWindowPos_w;
    int m_lastWindowPos_h;

    BOOL SetPos(int x, int y, int w, int h);
    BOOL GetPos(LPRECT rc);
public:
    int  Width();
    int  Height();

    HWND GethWnd() { return hWndToolbar; };
    HWND GethWndParent() { return hWndParent; };

    BOOL AttachBar(CToolbarChild *pChild, int Left, CToolbarChild *pBar);
    BOOL DetachBar(CToolbarChild *pChild, int Left);
    BOOL Add(CToolbarChild *pChild, eToolbarRowAlign Align, int Order, int Row);
    void Remove(CToolbarChild *pChild);
    CToolbarWindow(HWND hWndParent, HINSTANCE hInst, int Child);
    ~CToolbarWindow();

    BOOL Visible() { return IsToolbarVisible; }
    void AdjustArea(RECT *ar, int Crop);

    void PaintToolbar(HWND hWnd, HDC hDC, LPRECT lpRect, POINT *pPShift = NULL);
    void PaintChildBG(HWND hWndChild, HDC hDC, LPRECT lpRect);

    void ShowChild(CToolbarChild *pChild);
    void HideChild(CToolbarChild *pChild);

    void SetChildPosition(CToolbarChild *pChild, int Order, int Row);
    void SetChildRow(CToolbarChild *pChild, int Row);

    void SetPosition(int Pos);
    int GetPosition();

    void Margins(int l,int t,int r,int b, int child_lr, int child_tb);

    BOOL Show();
    BOOL Hide();
    void UpdateWindowPosition(HWND hParentWnd);
    void ForceUpdateWindowPosition(HWND hParentWnd);

    virtual BOOL LoadSkin(const char *szSkinIniFile,  const char *szSection, std::vector<int> *Results);
    virtual void ClearSkin();

    static LRESULT CALLBACK ToolbarProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

class CToolbarChild
{
protected:
    HWND hWnd;
    HINSTANCE hResourceInst;
    CToolbarWindow *m_pToolbar;
    std::vector<CBitmapAsButton*> Buttons;
    CBitmapsFromIniSection BitmapsFromIniSection;

    virtual LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;

    int m_PosX;
    int m_PosY;
    int m_PosW;
    int m_PosH;

    HBRUSH m_BgBrush;
    HPEN m_hPen3DShadow;
    HPEN m_hPen3DLight;

    int m_Visible;
public:
    int Visible();

    BOOL Show();
    BOOL Hide();

    int  Width();
    int  Height();

    BOOL SetPos(int x,int y,int w, int h, BOOL bUpdate);
    BOOL GetPos(LPRECT lpRect);

    HWND GethWnd() { return hWnd; };
    HWND GethWndParent() { return m_pToolbar->GethWnd(); }
    HWND GethWndParentOfToolbar() { return m_pToolbar->GethWndParent(); }

    void DrawItem(DRAWITEMSTRUCT* pDrawItemStruct, HICON hIcon, LPCSTR szText, int Width, int Height, int Align);

    virtual void UpdateWindow() {;};

    CToolbarChild(CToolbarWindow *pToolbar);
    ~CToolbarChild();

    virtual HWND Create(LPCTSTR szClassName, HINSTANCE hResourceInst);
    virtual HWND CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst);

    virtual BOOL SkinWindow(HWND hWnd, std::string sID, std::string sIniEntry, eBitmapAsButtonType ButtonType, std::string sSection, std::string sIniFile);
    virtual BOOL RemoveSkin(std::string sID);

    virtual BOOL SkinDlgItem(UINT uItemID, std::string sIniEntry, eBitmapAsButtonType ButtonType, std::string sSection, std::string sIniFile);
    virtual BOOL RemoveSkinDlgItem(UINT uItemID);

    virtual LRESULT ButtonChildProc(std::string sID, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK StaticToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK StaticToolbarChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT StaticToolbarChildButtonProc(std::string sID, void *pThis, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif
