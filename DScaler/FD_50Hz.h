/////////////////////////////////////////////////////////////////////////////
// $Id: FD_50Hz.h,v 1.8 2001-11-21 15:21:39 adcockj Exp $
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

#ifndef __FD_50HZ_H___
#define __FD_50HZ_H___

#include "settings.h"
#include "deinterlace.h"

// Get Hold of the FD_50Hz.c file settings
SETTING* FD50_GetSetting(FD50_SETTING Setting);
void FD50_ReadSettingsFromIni();
void FD50_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void FD50_ShowUI();

void UpdatePALPulldownMode(TDeinterlaceInfo* pInfo);
BOOL FilmModePALEven(TDeinterlaceInfo* pInfo);
BOOL FilmModePALOdd(TDeinterlaceInfo* pInfo);
BOOL FilmModePALComb(TDeinterlaceInfo* pInfo);


#endif