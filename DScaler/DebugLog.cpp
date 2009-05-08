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
#include "SettingsMaster.h"

using namespace std;

#define DEBUGLOGFILENAME "DScaler.log"

static FILE* debugLog = NULL;
SettingStringValue DebugLogFilename;
BOOL DebugLogEnabled = FALSE;
BOOL StampEnabled = FALSE;
long gDebugLogLevel = 1;
BOOL FlushAfterEachWrite = FALSE;

#if !defined(NOLOGGING)

void LOG(int DebugLevel, LPCWSTR Format, ...)
{
    DWORD SysTime;
    struct _timeb TimeB;
    struct tm* Time;
    wchar_t Stamp[100];
    va_list Args;

    if (DebugLogEnabled == FALSE && DebugLevel > 0)
    {
        return;
    }

    if (DebugLevel > gDebugLogLevel)
    {
        return;
    }

    if (debugLog == NULL && DebugLogFilename)
    {
        tstring FileName(DebugLogFilename);
        debugLog = _tfopen(FileName.c_str(), _T("w"));
    }

    if (debugLog == NULL)
    {
        return;
    }

    if(StampEnabled)
    {
        SysTime = timeGetTime();

        _ftime(&TimeB);
        Time = localtime(&TimeB.time);
        wcsftime(Stamp, 100, L"%y%m%d %H%M%S", Time);
        fwprintf(debugLog, L"%s.%03d(%03d)", Stamp, TimeB.millitm, SysTime % 1000);
    }

    for(int i(0); i < DebugLevel + 1; ++i)
    {
        fputwc(' ', debugLog);
    }

    va_start(Args, Format);
    vfwprintf(debugLog, Format, Args);
    va_end(Args);

    fputwc('\n', debugLog);
    if(FlushAfterEachWrite || DebugLevel == 0)
    {
        fflush(debugLog);
    }
}

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

    if (debugLog == NULL && DebugLogFilename)
    {
        tstring FileName(DebugLogFilename);
        debugLog = _tfopen(FileName.c_str(), _T("w"));
    }

    if (debugLog == NULL)
    {
        return;
    }

    if(StampEnabled)
    {
        SysTime = timeGetTime();

        _ftime(&TimeB);
        Time = localtime(&TimeB.time);
        strftime(Stamp, 100, "%y%m%d %H%M%S", Time);
        fprintf(debugLog, "%s.%03d(%03d)", Stamp, TimeB.millitm, SysTime % 1000);
    }

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
void LOGD(LPCTSTR Format, ...)
{
    TCHAR szMessage[2048];
    va_list Args;

    va_start(Args, Format);
    int result=_vsntprintf(szMessage,2048, Format, Args);
    va_end(Args);
    if(result==-1)
    {
        OutputDebugString(_T("DebugString too long, truncated!!\n"));
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
        "Debug Log File", TCHARSTRING, 0, DebugLogFilename.GetPointer(),
        (long)DEBUGLOGFILENAME, 0, 0, 0, 0,
        NULL,
        "Files", "DebugLogFilename", NULL,
    },
    {
        "Put Time Stamp in Log", ONOFF, 0, (long*)&StampEnabled,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Files", "StampEnabled", NULL,
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
