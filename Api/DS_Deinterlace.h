/////////////////////////////////////////////////////////////////////////////
// DS_Deinterlace.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
// This header file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 27 Mar 2001   John Adcock           Separated code to support plug-ins
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DS_DEINTERLACE_H___
#define __DS_DEINTERLACE_H___

#include "DS_Control.h"
#include "DS_ApiCommon.h"

struct _DEINTERLACE_METHOD;

// Deinterlace functions return true if the overlay is ready to be displayed.
typedef BOOL (__cdecl DEINTERLACE_FUNC)(TDeinterlaceInfo *Info);
typedef void (__cdecl DEINTERLACEPLUGINSETSTATUS)(LPCSTR StatusText);
typedef void (__cdecl DEINTERLACEPLUGININIT)(void);
typedef void (__cdecl DEINTERLACEPLUGINSTART)(long NumPlugIns, struct _DEINTERLACE_METHOD** OtherPlugins, DEINTERLACEPLUGINSETSTATUS* SetStatus);
typedef void (__cdecl DEINTERLACEPLUGINSHOWUI)(HWND hwndMain);
typedef void (__cdecl DEINTERLACEPLUGINEXIT)(void);

// list of supported plugin versions
#define DEINTERLACE_VERSION_3 3

// The current version
#define DEINTERLACE_CURRENT_VERSION DEINTERLACE_VERSION_3

typedef struct _DEINTERLACE_METHOD
{
    // should be set up as sizeof(DEINTERLACE_METHOD)
    // used to test that the program is using the same
    // header as the plug-in
    size_t SizeOfStructure;
    // may be used in the future when backwards combatability may
    // be required
    // set to DEINTERLACE_CURRENT_VERSION
    long DeinterlaceStructureVersion;
    // What to display when selected
    char* szName;
    // Short Name
    // What to display when used in adaptive Mode (NULL to use szName)
    char* szShortName;
    // Do we need to shrink the overlay by half
    BOOL bIsHalfHeight;
    // Is this a film Mode
    BOOL bIsFilmMode;
    // Pointer to Algorithm function (cannot be NULL)
    DEINTERLACE_FUNC* pfnAlgorithm;
    // flip frequency in 50Hz Mode
    unsigned long FrameRate50Hz;
    // flip frequency in 60Hz Mode
    unsigned long FrameRate60Hz;
    // number of settings
    long nSettings;
    // pointer to start of Settings[nSettings]
    SETTING* pSettings;
    // Index Number (position in menu) should map to old enum Value
    // and should be unique
    long nMethodIndex;
    // call this if plugin needs to do anything on load up
    DEINTERLACEPLUGININIT* pfnPluginInit;
    // call this if plugin needs to do anything before it is used
    DEINTERLACEPLUGINSTART* pfnPluginStart;
    // call this to display any UI, if NULL no UI is supoprted
    DEINTERLACEPLUGINSHOWUI* pfnPluginShowUI;
    // call this if plugin needs to deallocate anything
    DEINTERLACEPLUGINEXIT* pfnPluginExit;
    // how many fields are required to run this plug-in
    long nFieldsRequired;
    // Track number of Mode Changes
    long ModeChanges;
    // Track Time in Mode
    long ModeTicks;
    // the offset used by the external settings API
    long nSettingsOffset;
    // Dll module so that we can unload the dll cleanly at the end
    HMODULE hModule;
    // Menu Id used for this plug-in, use 0 to automatically allocate one
    DWORD MenuId;
    // do we need FieldDiff filled in in Info
    BOOL bNeedFieldDiff;
    // do we need CombFactor filled in in Info
    BOOL bNeedCombFactor;
    // Help ID
    // needs to be in \help\helpids.h
    int HelpID;
} DEINTERLACE_METHOD;

// Call this function to init a plug-in
// On exit pDeintMethod is a pointer to deinterlace properties
typedef DEINTERLACE_METHOD* (__cdecl GETDEINTERLACEPLUGININFO)(long CpuFeatureFlags);


#endif
