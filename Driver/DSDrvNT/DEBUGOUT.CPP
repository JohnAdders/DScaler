/////////////////////////////////////////////////////////////////////////////
// debugout.cpp
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
//
// This software was based on hwiodrv from the FreeTV project Those portions are
// Copyright (C) Mathias Ellinger
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 20 Jun 1994   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include <stdio.h>
#include <stdarg.h>
//#include <string.h>

void WINAPI debugOutputProc(PCHAR lpszMessage);

#define timeStringLength 10

static int debugOption = 0xFFFFFFFFL;
static int debugIndent = 0;
static int deviceNameLength = 0;
static char debugDeviceName[9] = {0};
static int nameOffset = 2;
static int dwTimerStart = 0;
static int dwTimerTick = 0;
static PDebugOutHandler debugHandler = debugOutputProc;

static int bWin95 = 0;
static int bWin98 = 0;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI debugGetSystemTime(void)
{
	LARGE_INTEGER systemTime;

	KeQuerySystemTime(&systemTime);           // time in 1000ns
	systemTime =  RtlExtendedLargeIntegerDivide(systemTime, 10000,NULL);

	return( systemTime.u.LowPart );
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void traceOut(DWORD debugLevel, PCHAR format, ... )
{
	DWORD        i;
	va_list      argp;
	char         szMessage[160];
	char         szSystemTime[15];
	DWORD        offset;

	if (!(debugOption & debugLevel))
	{
		return;
	}

	if (debugLevel & dbFunctionEnter)
	{
		debugIndent++;
	}

	sprintf(szSystemTime,"%9lu ",debugGetSystemTime() - dwTimerStart);

	sprintf(szMessage,"%8s: %s                   ",debugDeviceName,szSystemTime);
	offset = nameOffset + timeStringLength + debugIndent;

	i = strlen(szMessage);
	if ( i < offset)
	{
		offset = i;
	}

	va_start(argp,format);
	vsprintf(&szMessage[offset],format,argp);
	va_end(argp);

	if ( debugLevel & dbFunctionExit)
	{
		debugIndent--;
		if ( debugIndent < 0)
		{
			debugIndent = 0;
		}
	}

	strcat(szMessage,"\n");

	debugHandler(szMessage);
}

//----------------------------------------------------------------------------
//    dumphex without sprintf
//----------------------------------------------------------------------------
void WINAPI dumpHex(PCHAR data, int size)
{
	int   i,j,k;
	const char hexDigit[17]="0123456789ABCDEF";
	char  szDebug[80];

	i = 0;
	memset(&szDebug[0],' ',8);

	while (i  < size)
	{
		j = 0;
		k = i;
		while ( (j < 16) && ( k < size) )
		{
			szDebug[8+(j*3)]=hexDigit[(data[k]>>4) & 0xF];
			szDebug[9+(j*3)]=hexDigit[data[k] & 0x0f];
			szDebug[10+(j*3)]=' ';
			j++;
			k++;
		}

		while  (j < 16)
		{
			memset( &szDebug[8+(j*3)],' ',3);
			j++;
		}

		szDebug[8+(j*3)]  = ' ';
		szDebug[9+(j*3)]  = '-';
		szDebug[10+(j*3)] = ' ';
		j = 0;
		k = i;
		while ( (j < 16) && ( k < size))
		{
			if ( data[k] > 31)
			szDebug[59+j]=data[k];
			else
			szDebug[59+j]='.';

			j++;
			k++;
		}

		szDebug[59+j++]='\r';
		szDebug[59+j++]='\n';
		szDebug[59+j++]='\0';

		debugHandler( szDebug );

		i += 16;
	}
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void WINAPI debugOutputProc(PCHAR lpszMessage)
{
	DbgPrint("%s",lpszMessage);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI debugInitialize(PCHAR devName, DWORD dwDebugLevel)
{
	debugOption = dwDebugLevel & 0x7FFFFFFF;         // ignore function enter/exit

	strncpy(debugDeviceName,devName,sizeof(debugDeviceName));
	nameOffset      = 10;

	debugResetTimeBase();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI debugResetTimeBase(void)
{
	dwTimerStart = debugGetSystemTime();
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI debugSetOutputHandler(PDebugOutHandler  newDebugHandler)
{
	if ( ! newDebugHandler )
	{
		debugHandler = debugOutputProc;
	}
	else
	{
		debugHandler = newDebugHandler;
	}
}






