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
 * @file audio.h  audio Header file
 */

#ifndef __AUDIO_H___
#define __AUDIO_H___

#include "settings.h"

// Get Hold of the Audio.cpp file settings
SETTING* Audio_GetSetting(AUDIO_SETTING Setting);
void Audio_SetMenu(HMENU hMenu);

void Initialize_Mute();

void Audio_Mute(DWORD PostMuteDelay = 0UL);
void Audio_Unmute(DWORD PreUnmuteDelay = 0UL);
BOOL Audio_IsMute();

void Audio_SetUserMute(BOOL bMute);
BOOL Audio_GetUserMute();

#endif
