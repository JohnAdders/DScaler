/////////////////////////////////////////////////////////////////////////////
// deinterlace.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Put all my deinterlacing code into this
//                                     file
//
// 05 Jan 2001   John Adcock           Added flip frequencies to DeintMethods
//
// 07 Jan 2001   John Adcock           Added Adaptive deinterlacing method
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DEINTERLACE_H___
#define __DEINTERLACE_H___

#include "settings.h"
#include "DS_Deinterlace.h"

// Get Hold of the Deinterlace.c file settings
SETTING* Deinterlace_GetSetting(long nIndex, long Setting);
LONG Deinterlace_HandleSettingsMsg(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone);
void Deinterlace_ReadSettingsFromIni();
void Deinterlace_WriteSettingsToIni();
void Deinterlace_SetMenu(HMENU hMenu);


// Deinterlace modes.  Since these modes are referred to by number in the
// INI file, it's desirable to keep the numbers consistent between releases.
// Otherwise users will end up in the wrong modes when they upgrade.  If
// you renumber or add/remove modes, be sure to update inifile.htm, which
// documents the mode IDs!
typedef enum
{
	FILM_22_PULLDOWN_ODD = 0,
	FILM_22_PULLDOWN_EVEN = 1,
	FILM_32_PULLDOWN_0 = 2,
	FILM_32_PULLDOWN_1 = 3,
	FILM_32_PULLDOWN_2 = 4,
	FILM_32_PULLDOWN_3 = 5,
	FILM_32_PULLDOWN_4 = 6,
	FILM_32_PULLDOWN_COMB = 7,
	FILMPULLDOWNMODES_LAST_ONE = 8
} eFILMPULLDOWNMODES;

#define MAX_FIELD_HISTORY 5

BOOL LoadDeinterlacePlugins();
void UnloadDeinterlacePlugins();

DEINTERLACE_METHOD* GetCurrentDeintMethod();
DEINTERLACE_METHOD* GetVideoDeintMethod(int Mode);
DEINTERLACE_METHOD* GetVideoDeintIndex(int Index);
DEINTERLACE_METHOD* GetFilmDeintMethod(eFILMPULLDOWNMODES Mode);
BOOL ProcessDeinterlaceSelection(HWND hWnd, WORD wMenuID);

BOOL IsFilmMode();
eFILMPULLDOWNMODES GetFilmMode();
BOOL InHalfHeightMode();
void SetFilmDeinterlaceMode(eFILMPULLDOWNMODES mode);
void SetVideoDeinterlaceIndex(int index);
void ShowVideoModeUI();
char* GetDeinterlaceModeName();
void IncrementDeinterlaceMode();
void DecrementDeinterlaceMode();
void PrepareDeinterlaceMode();


#endif
