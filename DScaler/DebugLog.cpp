
/*
* Debug logging functions.
*/
#include <StdAfx.h>
#include "DebugLog.h"

static FILE *debugLog = NULL;
char DebugLogFilename[MAX_PATH] = "DScaler.txt";
boolean DebugLogEnabled = FALSE;

void
LOG(LPCSTR format, ...)
{
	DWORD systime;
	struct _timeb tb;
	struct tm *tm;
	char stamp[100];
	va_list args;

	if (DebugLogEnabled == FALSE)
		return;

	if (debugLog == NULL)
		debugLog = fopen(DebugLogFilename, "w");

	if (debugLog == NULL)
		return;

	systime = timeGetTime();

	_ftime(&tb);
	tm = localtime(&tb.time);
	strftime(stamp, sizeof(stamp), "%y%m%d %H%M%S", tm);
	fprintf(debugLog, "%s.%03d(%03d)", stamp, tb.millitm, systime % 1000);

	va_start(args, format);
	vfprintf(debugLog, format, args);
	va_end(args);

	fputc('\n', debugLog);
	fflush(debugLog);
}

void
LOGD(LPCSTR format, ...)
{
	char szMessage[2048];
	va_list args;

	va_start(args, format);
	vsprintf(szMessage, format, args);
	va_end(args);
	OutputDebugString(szMessage);
}


