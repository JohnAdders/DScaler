/////////////////////////////////////////////////////////////////////////////
// $Id: Audio.h,v 1.12 2002-12-07 16:06:54 adcockj Exp $
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
// 11 Aug 2000   John Adcock           Moved Audio_ Function in here
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_H___
#define __AUDIO_H___

#include "settings.h"

// Get Hold of the Audio.cpp file settings
SETTING* Audio_GetSetting(AUDIO_SETTING Setting);
void Audio_ReadSettingsFromIni();
void Audio_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void Audio_SetMenu(HMENU hMenu);

void Audio_SetMute(BOOL IsMute);
BOOL Audio_GetMute();

#endif
