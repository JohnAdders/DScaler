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
 * @file filter.h filter Header file
 */

#ifndef __FILTER_H___
#define __FILTER_H___

#include "DS_Filter.h"

class CSettingsHolder;

BOOL LoadFilterPlugins();
void UnloadFilterPlugins();
BOOL ProcessFilterSelection(HWND hWnd, WORD wMenuID);
void GetFilterSettings(std::vector< SmartPtr<CSettingsHolder> >& Holders,std::vector< std::string >& Names);
void Filter_SetMenu(HMENU hMenu);
BOOL Filter_WillWeDoOutput();


long Filter_DoInput(TDeinterlaceInfo* Info, int History, BOOL HurryUp);
void Filter_DoOutput(TDeinterlaceInfo* Info, int History, BOOL HurryUp);

#endif