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
 * @file mixerdev.h mixerdev Header file
 */

#ifndef __MIXERDEV_H___
#define __MIXERDEV_H___

#include "settings.h"


//  Mixer functions

void    Mixer_Init();
void    Mixer_Exit();

BOOL    Mixer_IsEnabled();
BOOL    Mixer_IsNoHardwareMute();

void    Mixer_SetMute(BOOL bEnabled);
BOOL    Mixer_GetMute(void);

void    Mixer_SetVolume(long newVolume);
long    Mixer_GetVolume();
void    Mixer_AdjustVolume(long delta);

void    Mixer_Mute();
void    Mixer_UnMute();

void    Mixer_Volume_Up();
void    Mixer_Volume_Down();

void    Mixer_SetupDlg(HWND hWndParent);

void    MixerDev_SetMenu(HMENU hMenu);

#endif
