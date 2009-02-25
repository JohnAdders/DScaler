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
// Change Log
//
// Date          Developer             Changes
//
// 12 Jul 2001   John Adcock           added header block
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.26  2003/08/10 12:14:42  tobbej
// prevent crashing when trying to log when the debuglog filename is not loaded yet
//
// Revision 1.25  2003/06/02 13:15:31  adcockj
// Fixes for CHARSTRING problems
//
// Revision 1.24  2003/04/26 19:39:09  laurentg
// New character string settings
//
// Revision 1.23  2003/03/02 16:21:29  tobbej
// fixed crashing in LOGD if string is too long
//
// Revision 1.22  2003/01/10 17:38:04  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.21  2002/06/22 22:11:50  robmuller
// Fixed: DebugLogFilename was not written to ini file if bOptimizeFileAccess is true.
//
// Revision 1.20  2002/06/15 10:36:58  robmuller
// FlushAfterEachWrite defaults to TRUE.
//
// Revision 1.19  2002/06/13 12:10:21  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.18  2002/06/11 20:52:35  robmuller
// Enable debug logging by default. Renamed debug log file to debug.log.
//
// Revision 1.17  2002/05/26 19:04:13  robmuller
// Implemented debug log level 0 (for critical errors).
//
// Revision 1.16  2002/03/26 13:07:10  robmuller
// Flush file when FlushAfterEachWrite is enabled with the dialog.
//
// Revision 1.15  2001/12/16 17:04:37  adcockj
// Debug Log improvements
//
// Revision 1.14  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.13  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.12  2001/08/02 18:18:32  adcockj
// Fixed release build problem
//
// Revision 1.11  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.10  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.9  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.8  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file DebugLog.cpp Debug Logging functions
 */

#include "StdAfx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DebugLog.h"

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

