/////////////////////////////////////////////////////////////////////////////
// $Id: FD_60Hz.h,v 1.6 2001-07-12 16:16:40 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 09 Jan 2001   John Adcock           Split into new file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __FD_60HZ_H___
#define __FD_60HZ_H___

#include "settings.h"
#include "deinterlace.h"

// Get Hold of the FD_50Hz.c file settings
SETTING* FD60_GetSetting(FD60_SETTING Setting);
void FD60_ReadSettingsFromIni();
void FD60_WriteSettingsToIni();
void FD60_SetMenu(HMENU hMenu);
void FD60_ShowUI();


#define MAXMODESWITCHES 50  // Maximum number of switches to track in TrackModeSwitches()

void UpdateNTSCPulldownMode(DEINTERLACE_INFO *pInfo);
eFILMPULLDOWNMODES GetFilmModeFromPosition(DEINTERLACE_INFO *pInfo);
BOOL DoWeWantToFlipNTSC(DEINTERLACE_INFO *pInfo);
BOOL FilmModeNTSC1st(DEINTERLACE_INFO *info);
BOOL FilmModeNTSC2nd(DEINTERLACE_INFO *info);
BOOL FilmModeNTSC3rd(DEINTERLACE_INFO *info);
BOOL FilmModeNTSC4th(DEINTERLACE_INFO *info);
BOOL FilmModeNTSC5th(DEINTERLACE_INFO *info);
BOOL FilmModeNTSCComb(DEINTERLACE_INFO *info);
BOOL DoWeWantToFlip(DEINTERLACE_INFO *pInfo);

#endif