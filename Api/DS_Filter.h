/////////////////////////////////////////////////////////////////////////////
// DS_Filter.h
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
// 03 Feb 2001   John Adcock           Initial Version
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DS_FILTER_H___
#define __DS_FILTER_H___

#include "DS_Control.h"
#include "DS_ApiCommon.h"

// filter functions return the aspect ratio change 1000 = 1.0
typedef long (__cdecl FILTER_FUNC)(DEINTERLACE_INFO *info);
typedef void (__cdecl FILTERPLUGINSTART)(void);
typedef void (__cdecl FILTERPLUGINEXIT)(void);

// list of supported plugin versions
#define FILTER_VERSION_1 1

// The current version
#define FILTER_CURRENT_VERSION FILTER_VERSION_1


typedef struct
{
	// should be set up as sizeof(FILTER_METHOD)
	// used to test that the program is using the same
	// header as the plug-in
	size_t SizeOfStructure;
	// may be used in the future when backwards combatability may
	// be required
	// set to FILTER_CURRENT_VERSION
	long FilterStructureVersion;
	// What to display when selected
	char* szName;
	// What to put in the Menu (NULL to use szName)
	char* szMenuName;
	// Are we active Initially FALSE
	BOOL bActive;
	// Do we get called on Input
	BOOL bOnInput;
    // Pointer to Algorithm function (cannot be NULL)
    FILTER_FUNC* pfnAlgorithm;
	// id of menu to display status
	int MenuId;
	// Always run - do we run if there has been an overrun
	BOOL bAlwaysRun;
	// call this if plugin needs to do anything before it is used
	FILTERPLUGINSTART* pfnPluginStart;
	// call this if plugin needs to deallocate anything
	FILTERPLUGINEXIT* pfnPluginExit;
	// Used to save the module Handle
	HMODULE hModule;
	// number of settings
	long nSettings;
	// pointer to start of Settings[nSettings]
	SETTING* pSettings;
	// the offset used by the external settings API
	long nSettingsOffset;
} FILTER_METHOD;


// Call this function to get plug-in info
typedef FILTER_METHOD* (__cdecl GETFILTERPLUGININFO)(long CpuFeatureFlags);

#endif