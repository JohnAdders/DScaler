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
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file vbi_videotext.h vbi_videotext Header file
 */

#ifndef __VBI_VIDEOTEXT_H___
#define __VBI_VIDEOTEXT_H___

#include "settings.h"


enum eVTState
{
    VT_OFF,
    VT_BLACK,
    VT_MIXED
};


typedef struct {
    char*    sCountry;
    char*    sNetwork;
    WORD    wNI_P8301;
    BYTE    uC_P8302;
    BYTE    uNI_P8302;
    BYTE    uA_X26;
    BYTE    uB_X26;
    WORD    wCNI_VPS;
} TChannelIdentif;


extern TChannelIdentif RegisteredCNICodes[];
extern int iNbRegisteredCNICodes;


void VBI_VT_Init();
void VBI_VT_Exit();

void VT_Init_Data(double VBI_Frequency);

void VT_SetState(HDC hDC, LPRECT lpRect, eVTState State);
eVTState VT_GetState();

void VT_ChannelChange();

BOOL VT_SetAutoCodepage(HDC hDC, LPRECT lpRect, BOOL bAuto);
BOOL VT_GetAutoCodepage();
void VT_SetAntialias(HDC hDC, LPRECT lpRect, BOOL bAntialias);
BOOL VT_GetAntialias();
void VT_SetShowHidden(HDC hDC, LPRECT lpRect, BOOL bShowHidden);
BOOL VT_GetShowHidden();

void VT_SetOverlayColour(COLORREF ColorRef);

BOOL VT_SetPage(HDC hDC, LPRECT lpRect, WORD wPageHex, WORD wPageSubCode = 0xFFFF);
BOOL VT_PageScroll(HDC hDC, LPRECT lpRect, BOOL bForwards);
BOOL VT_SubPageScroll(HDC hDC, LPRECT lpRect, BOOL bForwards);
BOOL VT_PerformSearch(HDC hDC, LPRECT lpRect, BOOL bInclusive, BOOL bReverse = FALSE);
BOOL VT_ShowTestPage(HDC hDC, LPRECT lpRect);
BOOL VT_PerformFlofKey(HDC hDC, LPRECT lpRect, BYTE nFlofKey);

void VT_SetPageOSD(char OSD[3]);
void VT_SetPageOSD(char OSD[3], BOOL bOSDModeOn);
void VT_ShowHeader(HDC hDC, LPRECT lpRect, char OSD[3] = NULL);

BOOL VT_IsSearchStringValid(BOOL bBeepOnCorrection = FALSE);
BOOL VT_IsTransparencyInPage();

BOOL VT_OnInput(HDC hDC, LPRECT lpRect, char cInput);
void VT_ClearInput();
void VT_OnInputTimer(HDC hDC, LPRECT lpRect);

void VT_RedrawFlash(HDC hDC, LPRECT lpRect);
void VT_Redraw(HDC hDC, LPRECT lpRect, WORD uVTDFFlags = 0);

LONG VT_GetPaintedRects(RECT* pRectBuffer, LONG nBufferSize);
void VT_ResetPaintedRects();

void VT_ProcessHeaderUpdate(HDC hDC, LPRECT lpRect);
BOOL VT_ProcessCommentUpdate(HDC hDC, LPRECT lpRect, DWORD dwPageCode);
BOOL VT_ProcessPageUpdate(HDC hDC, LPRECT lpRect, DWORD dwPageCode);
BOOL VT_ProcessPageRefresh(HDC hDC, LPRECT lpRect, DWORD dwPageCode);

void VT_UpdateFlashTimer();

BOOL VT_ClickAtPosition(HDC hDC, LPRECT lpRect, WORD xPos, WORD yPos);
BOOL VT_IsPageNumberAtPosition(LPRECT lpRect, LPPOINT lpPoint);
BOOL VT_IsPageNumberAtPosition(LPRECT lpRect, WORD xPos, WORD yPos);

std::string VT_GetStationFromIDP8301();
std::string VT_GetStationFromPDC();
std::string VT_GetStation();

BOOL APIENTRY VTSearchProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VTGotoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

SETTING* VT_GetSetting(VT_SETTING Setting);
void VT_UpdateMenu(HMENU hMenu);
void VT_SetMenu(HMENU hMenu);

SmartPtr<CTreeSettingsGeneric> VideoText_GetTreeSettingsPage();

BOOL ProcessVTCodepageSelection(HWND hWnd, WORD wMenuID);

BOOL VT_HilightSearchOnChange(long NewValue);

void VBI_DecodeLine_VT(BYTE* VBI_Buffer);


#endif
