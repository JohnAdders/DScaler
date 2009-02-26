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
 * @file outthreads.h outthreads Header file
 */
 
#ifndef __OUTTHREADS_H___
#define __OUTTHREADS_H___

#include "settings.h"
#include "deinterlace.h"

typedef enum
{
    REQ_NONE,
    REQ_STILL,
    REQ_SNAPSHOT,
#ifdef WANT_DSHOW_SUPPORT
    REQ_DSHOW_CHANGERES,
    REQ_DSHOW_STOP,
#endif
} eRequestType;

typedef struct {
    eRequestType    type;
    int                param1;
    int                param2;
} TGUIRequest;

// Get Hold of the OutThreads.c file settings
SETTING* OutThreads_GetSetting(OUTTHREADS_SETTING Setting);
void OutThreads_ReadSettingsFromIni();
void OutThreads_WriteSettingsToIni(BOOL bOptimizeFileAccess);
CTreeSettingsGeneric* OutThreads_GetTreeSettingsPage();
void OutThreads_SetMenu(HMENU hMenu);

void ClearPictureHistory(TDeinterlaceInfo* pInfo);
void ShiftPictureHistory(TDeinterlaceInfo* pInfo, int NumFieldsValid);

void Start_Capture();
void Stop_Capture();
void Reset_Capture();
void Pause_Capture();
void UnPause_Capture();
void Pause_Toggle_Capture();
void Toggle_Vertical_Flip();
void PutRequest(TGUIRequest *req);

void Start_Thread();
void Stop_Thread();
void SetOutputThreadPriority();

void SetupCaptureFlags();

DWORD WINAPI YUVOutThread(LPVOID lpThreadParameter);

BOOL WaitForNextField(BOOL LastField, BOOL* RunningLate);
BOOL LockOverlay(TDeinterlaceInfo* pInfo);

#ifdef _DEBUG
    #define ASSERTONOUTTHREAD AssertOnOutThread()
    void AssertOnOutThread();
#else
    #define ASSERTONOUTTHREAD
#endif

extern BOOL    bCheckSignalPresent;
extern BOOL    bCheckSignalMissing;

#endif