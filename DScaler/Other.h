/////////////////////////////////////////////////////////////////////////////
// $Id: Other.h,v 1.14 2001-11-28 16:04:50 adcockj Exp $
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

#ifndef __OTHER_H___
#define __OTHER_H___

#include "settings.h"

// Get Hold of the Other.c file settings
SETTING* Other_GetSetting(OTHER_SETTING Setting);
void Other_ReadSettingsFromIni();
void Other_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void Other_ShowUI();


#define DSCALER_MAX_WIDTH 768
#define DSCALER_MAX_HEIGHT 576

void ExitDD(void);
BOOL InitDD(HWND hWnd);
BOOL OverlayActive();
BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags);
BOOL Overlay_Create();
void Overlay_Clean();
BOOL Overlay_Destroy();
DWORD Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb);
COLORREF Overlay_GetColor();
void Overlay_SetColorControls();
BOOL Overlay_Lock(TDeinterlaceInfo* pInfo);
BOOL Overlay_Unlock();
BOOL Overlay_Flip(DWORD FlipFlag);
HDC Overlay_GetDC();
void Overlay_ReleaseDC(HDC hDC);

extern DWORD DestSizeAlign;
extern DWORD SrcSizeAlign;

#endif
