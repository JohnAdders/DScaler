/////////////////////////////////////////////////////////////////////////////
// $Id: OutReso.h,v 1.6 2003-04-26 16:06:21 laurentg Exp $
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
// Change Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2003/04/15 13:07:10  adcockj
// Fixed memory leak
//
// Revision 1.4  2003/04/12 15:23:23  laurentg
// Interface with PowerStrip when changing resolution (code from Olivier Borca)
//
// Revision 1.3  2003/02/08 13:16:47  laurentg
// Change resolution in full screen mode slightly updated
//
// Revision 1.2  2003/02/07 11:28:25  laurentg
// Keep more ids for the output reso menus (100)
// New resolutions added (720x480 and 720x576)
//
// Revision 1.1  2003/02/06 00:58:53  laurentg
// Change output resolution (first step)
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __OUTRESO_H___
#define __OUTRESO_H___

#define	MAX_NUMBER_RESO		100

// For interface with PowerStrip
#define UM_SETPSTRIPTIMING			(WM_USER+200)
#define UM_GETPSTRIPTIMING			(WM_USER+205)
#define PSTRIP_TIMING_STRING_SIZE	64

extern int OutputReso;
extern LPSTR PStrip576i;
extern LPSTR PStrip480i;

void PStripTiming_CleanUp();
void OutReso_UpdateMenu(HMENU hMenu);
void OutReso_SetMenu(HMENU hMenu);
BOOL ProcessOutResoSelection(HWND hWnd, WORD wMenuID);
void OutReso_Change(HWND hWnd, HWND hPSWnd, BOOL bUseRegistrySettings, BOOL bCaptureRunning, LPSTR lTimingString, BOOL bApplyPStripTimingString);

#endif

