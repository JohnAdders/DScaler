#ifndef __DEBUGLOG_H__
#define __DEBUGLOG_H__

#include "settings.h"

// Get Hold of the DebugLog.cpp file settings
SETTING* Debug_GetSetting(DEBUG_SETTING Setting);
void Debug_ReadSettingsFromIni();
void Debug_WriteSettingsToIni();

#if !defined(NOLOGGING)
void LOG(LPCSTR format, ...);
void LOGD(LPCSTR format, ...);

#else

#define LOG 
#define LOGD

#endif

#endif