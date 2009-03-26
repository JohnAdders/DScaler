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
 * @file Dialogs.h  Dialogs Header file
 */


#ifndef __DIALOGS_H___
#define __DIALOGS_H___

BOOL APIENTRY SplashProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY VPSInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY AboutProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void SetHorSliderInt(HWND hDlgItem, int m_YPos, int Value, int nMin, int nMax);
int GetHorSliderInt(int MouseX, int nMin, int nMax);
LPCSTR GetProductNameAndVersion();


#endif

