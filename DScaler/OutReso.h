/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
//
// This module uses code from Kristian Trenskow provided as DScaler patch
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file outreso.h outreso Header file
 */

#ifndef __OUTRESO_H___
#define __OUTRESO_H___

#define    MAX_NUMBER_RESO        100

// For interface with PowerStrip
#define UM_SETPSTRIPTIMING            (WM_USER+200)
#define UM_GETPSTRIPTIMING            (WM_USER+205)
#define PSTRIP_TIMING_STRING_SIZE    64

extern int OutputReso;
extern SettingStringValue PStrip576i;
extern SettingStringValue PStrip480i;

void OutReso_UpdateMenu(HMENU hMenu);
void OutReso_SetMenu(HMENU hMenu);
BOOL ProcessOutResoSelection(HWND hWnd, WORD wMenuID);
void OutReso_Change(HWND hWnd, HWND hPSWnd, BOOL bUseRegistrySettings, BOOL bCaptureRunning, LPTSTR lTimingString, BOOL bApplyPStripTimingString);

#endif

