/////////////////////////////////////////////////////////////////////////////
// $Id: DebugLog.cpp,v 1.14 2001-11-23 10:49:16 adcockj Exp $
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

#include "StdAfx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DebugLog.h"
#include "SettingsDlg.h"

static FILE* debugLog = NULL;
char DebugLogFilename[MAX_PATH] = "DScaler.txt";
BOOL DebugLogEnabled = FALSE;
long gDebugLogLevel = 1;

#if !defined(NOLOGGING)

void LOG(int DebugLevel, LPCSTR Format, ...)
{
    DWORD SysTime;
    struct _timeb TimeB;
    struct tm* Time;
    char Stamp[100];
    va_list Args;

    if (DebugLogEnabled == FALSE)
    {
        return;
    }

    if (DebugLevel > gDebugLogLevel)
    {
        return;
    }

    if (debugLog == NULL)
        debugLog = fopen(DebugLogFilename, "w");

    if (debugLog == NULL)
        return;

    SysTime = timeGetTime();

    _ftime(&TimeB);
    Time = localtime(&TimeB.time);
    strftime(Stamp, sizeof(Stamp), "%y%m%d %H%M%S", Time);
    fprintf(debugLog, "%s.%03d(%03d)", Stamp, TimeB.millitm, SysTime % 1000);

    for(int i(0); i < DebugLevel; ++i)
    {
        fputc(' ', debugLog);
    }

    va_start(Args, Format);
    vfprintf(debugLog, Format, Args);
    va_end(Args);

    fputc('\n', debugLog);
    fflush(debugLog);
}
#endif

#ifdef _DEBUG
void LOGD(LPCSTR Format, ...)
{
    char szMessage[2048];
    va_list Args;

    va_start(Args, Format);
    vsprintf(szMessage, Format, Args);
    va_end(Args);
    OutputDebugString(szMessage);
}
#endif


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DebugSettings[DEBUG_SETTING_LASTONE] =
{
    {
        "Debug Log", ONOFF, 0, (long*)&DebugLogEnabled,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Files", "DebugLogEnabled", NULL,
    },
    {
        "Debug Level", SLIDER, 0, (long*)&gDebugLogLevel,
        1, 0, 5, 1, 1,
        NULL,
        "Files", "DebugLevel", NULL,
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

    GetPrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, DebugLogFilename, MAX_PATH, GetIniFileForSettings());
}

void Debug_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < DEBUG_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(DebugSettings[i]), bOptimizeFileAccess);
    }
	if(bOptimizeFileAccess == FALSE)
	{
	    WritePrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, GetIniFileForSettings());
	}
}

void Debug_ShowUI()
{
    CSettingsDlg::ShowSettingsDlg("Logging Settings",DebugSettings, DEBUG_SETTING_LASTONE);
}
