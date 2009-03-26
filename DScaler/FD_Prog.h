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
 * @file fd_prog.h fd_prog Header file
 */
 
#ifndef __FD_PROG_H___
#define __FD_PROG_H___

#include "settings.h"
#include "deinterlace.h"

// Get Hold of the FD_50Hz.c file settings
SETTING* FDProg_GetSetting(FDPROG_SETTING Setting);
void FDProg_SetMenu(HMENU hMenu);
SmartPtr<CTreeSettingsGeneric> FDProg_GetTreeSettingsPage();


#define MAXMODESWITCHES 50  // Maximum number of switches to track in TrackModeSwitches()

void UpdateProgPulldownModeNTSC(TDeinterlaceInfo* pInfo);
void UpdateProgPulldownModePAL(TDeinterlaceInfo* pInfo);
BOOL ProgMode(TDeinterlaceInfo* pInfo);

#endif