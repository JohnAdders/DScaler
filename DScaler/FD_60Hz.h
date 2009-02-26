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

/** 
 * @file fd_60hz.h fd_60hz Header file
 */
 
#ifndef __FD_60HZ_H___
#define __FD_60HZ_H___

#include "settings.h"
#include "deinterlace.h"

// Get Hold of the FD_50Hz.c file settings
SETTING* FD60_GetSetting(FD60_SETTING Setting);
void FD60_ReadSettingsFromIni();
void FD60_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void FD60_SetMenu(HMENU hMenu);
CTreeSettingsGeneric* FD60_GetTreeSettingsPage();


#define MAXMODESWITCHES 50  // Maximum number of switches to track in TrackModeSwitches()

void UpdateNTSCPulldownMode(TDeinterlaceInfo* pInfo);
eFilmPulldownMode GetFilmModeFromPosition(TDeinterlaceInfo* pInfo);
BOOL DoWeWantToFlipNTSC(TDeinterlaceInfo* pInfo);
BOOL FilmModeNTSC1st(TDeinterlaceInfo* pInfo);
BOOL FilmModeNTSC2nd(TDeinterlaceInfo* pInfo);
BOOL FilmModeNTSC3rd(TDeinterlaceInfo* pInfo);
BOOL FilmModeNTSC4th(TDeinterlaceInfo* pInfo);
BOOL FilmModeNTSC5th(TDeinterlaceInfo* pInfo);
BOOL FilmModeNTSCComb(TDeinterlaceInfo* pInfo);
BOOL DoWeWantToFlip(TDeinterlaceInfo* pInfo);

#endif