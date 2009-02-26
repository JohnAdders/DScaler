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
 * @file debuglog.h debuglog Header file
 */
 
#ifndef __DEBUGLOG_H__
#define __DEBUGLOG_H__

#include "settings.h"

// Get Hold of the DebugLog.cpp file settings
SETTING* Debug_GetSetting(DEBUG_SETTING Setting);
void Debug_ReadSettingsFromIni();
void Debug_WriteSettingsToIni(BOOL bOptimizeFileAccess);
void Debug_FreeSettings();
CTreeSettingsGeneric* Debug_GetTreeSettingsPage();

#if !defined(NOLOGGING)
void LOG(int DebugLevel, LPCSTR format, ...);
#else
#define LOG 
#endif

// LOGD outputs a string to the debug screen while debugging
#if defined(_DEBUG)
void LOGD(LPCSTR format, ...);
#else
#define LOGD
#endif

#endif