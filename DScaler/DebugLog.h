#ifndef __DEBUGLOG_H__
#define __DEBUGLOG_H__

extern char DebugLogFilename[];
extern boolean DebugLogEnabled;

void LOG(LPCSTR format, ...);
void LOGD(LPCSTR format, ...);

#endif