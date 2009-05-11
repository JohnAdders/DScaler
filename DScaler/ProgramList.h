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
 * @file programlist.h programlist Header file
 */

#ifndef __PROGRAMLIST_H___
#define __PROGRAMLIST_H___


// Get Hold of the OutThreads.c file settings
SETTING* Channels_GetSetting(CHANNELS_SETTING Setting);
void Channels_UpdateMenu(HMENU hMenu);
void Channels_SetMenu(HMENU hMenu);
BOOL ProcessProgramSelection(HWND hWnd, WORD wMenuID);

SETTING* AntiPlop_GetSetting(ANTIPLOP_SETTING Setting);

void Channels_Exit();

//TODO->Program listing file handling now sits in Channels.h
//This export should be removed and the corresponding impl changed
BOOL Load_Program_List_ASCII();


INT_PTR CALLBACK ProgramListProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void Channel_Change(int NewChannel, int DontStorePrevious=0);
void Channel_ChangeToNumber(int NewChannelNumber, int DontStorePrevious=0);
void Channel_Increment();
void Channel_Decrement();
void Channel_SetCurrent();
void Channel_Previous();
void Channel_Reset();
tstring Channel_GetName();
tstring Channel_GetEPGName();
tstring Channel_GetVBIName(BOOL bOnlyWithCodes=FALSE);

extern int PreSwitchMuteDelay;
extern int PostSwitchMuteDelay;

#endif
