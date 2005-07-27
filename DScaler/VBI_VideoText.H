/////////////////////////////////////////////////////////////////////////////
// $Id: VBI_VideoText.H,v 1.37 2005-07-27 19:34:01 laurentg Exp $
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
// 02 Jan 2003   Atsushi Nakagawa      Redid VBI_VideoText.* to link in
//                                     with the new and updated CVTDecoder and
//                                     CVTDrawer classes.  CVS Log entries
//                                     prior to and including rev 1.27 may
//                                     no longer have direct validity but
//                                     their cumulative changes still remain
//                                     valid.
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

void VT_GetStationFromIDP8301(LPSTR lpBuffer, LONG nLength);
void VT_GetStationFromPDC(LPSTR lpBuffer, LONG nLength);
void VT_GetStation(LPSTR lpBuffer, LONG nLength);

BOOL APIENTRY VTSearchProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VTInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VTGotoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

SETTING* VT_GetSetting(VT_SETTING Setting);
void VT_ReadSettingsFromIni();
void VT_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void VT_UpdateMenu(HMENU hMenu);
void VT_SetMenu(HMENU hMenu);

CTreeSettingsGeneric* VideoText_GetTreeSettingsPage();

BOOL ProcessVTCodepageSelection(HWND hWnd, WORD wMenuID);

BOOL VT_HilightSearchOnChange(long NewValue);

void VBI_DecodeLine_VT(BYTE* VBI_Buffer);


#endif
