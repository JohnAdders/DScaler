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
 * @file DebugLog.cpp Debug Logging functions
 */

#include "StdAfx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DebugLog.h"

using namespace std;

#define DEBUGLOGFILENAME "DScaler.log"

static FILE* debugLog = NULL;
char* DebugLogFilename = NULL;
BOOL DebugLogEnabled = FALSE;
long gDebugLogLevel = 1;
BOOL FlushAfterEachWrite = FALSE;

#if !defined(NOLOGGING)

void LOG(int DebugLevel, LPCSTR Format, ...)
{
    DWORD SysTime;
    struct _timeb TimeB;
    struct tm* Time;
    char Stamp[100];
    va_list Args;

    if (DebugLogEnabled == FALSE && DebugLevel > 0)
    {
        return;
    }

    if (DebugLevel > gDebugLogLevel)
    {
        return;
    }

    if (debugLog == NULL && DebugLogFilename!=NULL && DebugLogFilename[0] != '\0')
        debugLog = fopen(DebugLogFilename, "w");

    if (debugLog == NULL)
        return;

    SysTime = timeGetTime();

    _ftime(&TimeB);
    Time = localtime(&TimeB.time);
    strftime(Stamp, sizeof(Stamp), "%y%m%d %H%M%S", Time);
    fprintf(debugLog, "%s.%03d(%03d)", Stamp, TimeB.millitm, SysTime % 1000);

    for(int i(0); i < DebugLevel + 1; ++i)
    {
        fputc(' ', debugLog);
    }

    va_start(Args, Format);
    vfprintf(debugLog, Format, Args);
    va_end(Args);

    fputc('\n', debugLog);
    if(FlushAfterEachWrite || DebugLevel == 0)
    {
        fflush(debugLog);
    }
}
#endif

#ifdef _DEBUG
void LOGD(LPCSTR Format, ...)
{
    char szMessage[2048];
    va_list Args;

    va_start(Args, Format);
    int result=_vsnprintf(szMessage,2048, Format, Args);
    va_end(Args);
    if(result==-1)
    {
        OutputDebugString("DebugString too long, truncated!!\n");
    }
    OutputDebugString(szMessage);
}
#endif

BOOL FlushAfterEachWrite_OnChange(long NewValue)
{
    FlushAfterEachWrite = (BOOL)NewValue;
    if(FlushAfterEachWrite && (debugLog != NULL))
    {
        fflush(debugLog);
    }
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DebugSettings[DEBUG_SETTING_LASTONE] =
{
    {
        "Debug Log", ONOFF, 0, (long*)&DebugLogEnabled,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Files", "DebugLogEnabled", NULL,
    },
    {
        "Debug Level", SLIDER, 0, (long*)&gDebugLogLevel,
        1, 0, 5, 1, 1,
        NULL,
        "Files", "DebugLevel", NULL,
    },
    {
        "Flush After Each Write", ONOFF, 0, (long*)&FlushAfterEachWrite,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Files", "FlushAfterEachWrite", FlushAfterEachWrite_OnChange,
    },
    {
        "Debug Log File", CHARSTRING, 0, (long*)&DebugLogFilename,
        (long)DEBUGLOGFILENAME, 0, 0, 0, 0,
        NULL,
        "Files", "DebugLogFilename", NULL,
    },
};

SETTING* Debug_GetSetting(DEBUG_SETTING Setting)
{
    if(Setting > -1 && Setting < DEBUG_SETTING_LASTONE)
    {
        return &(DebugSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Debug_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < DEBUG_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(DebugSettings[i]));
    }
}

void Debug_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < DEBUG_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(DebugSettings[i]), bOptimizeFileAccess);
    }
}

CTreeSettingsGeneric* Debug_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Logging Settings",DebugSettings, DEBUG_SETTING_LASTONE);
}

void Debug_FreeSettings()
{
    int i;
    for(i = 0; i < DEBUG_SETTING_LASTONE; i++)
    {
        Setting_Free(&DebugSettings[i]);
    }
}

