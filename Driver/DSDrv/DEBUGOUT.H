/////////////////////////////////////////////////////////////////////////////
// $Id: DEBUGOUT.H,v 1.2 2001-07-13 16:13:53 adcockj Exp $
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

#if ! defined (__DEBUGOUT_H)
#define __DEBUGOUT_H

#include <windows.h>

#if defined (_DEBUG) && ! defined (DEBUG)
#define DEBUG           1
#endif


#if defined (__cplusplus)
extern "C" {
#endif

typedef void (WINAPI * PDebugOutHandler)(PCHAR lpszMessage);

typedef void (WINAPI * PDebugSetOutputHandler)(PDebugOutHandler debugHandler);

void traceOut(DWORD debugLevel, PCHAR format, ... );

#define errorOut traceOut

#if defined (DEBUG)
#define debugOut traceOut
#else
#define debugOut
#endif

void WINAPI debugInitialize(PCHAR devName, DWORD dwDebugLevel);

void WINAPI debugResetTimeBase(void);
DWORD WINAPI debugGetSystemTime(void);
void WINAPI dumpHex(PCHAR data, int size);
void WINAPI debugSetOutputHandler(PDebugOutHandler outputHandler);

#define debugDump(a,b) dumpHex( (PCHAR)a, (int)b)

#if defined (__cplusplus)
}
#endif


//---------------------------------------------------------------------------
// DEBUG ID setting
//   31..28  - Reserved
//
//   00..07  - Debug module ID
//---------------------------------------------------------------------------

#define dbModuleFlags         0xFF

#define  dbLevel1             0x00000100L
#define  dbLevel2             0x00000200L
#define  dbLevel4             0x00000400L
#define  dbLevel8             0x00000800L
#define  dbLevel10            0x00001000L
#define  dbLevel20            0x00002000L
#define  dbLevel40            0x00004000L
#define  dbLevel80            0x00008000L


// common stuff

#define  dbInitPhase          0x80000000L
#define  dbFunctionEnter      0x40000000L
#define  dbFunctionExit       0x20000000L
#define  dbBinaries           0x10000000L


#define  dbTrace              0x01000000L
#define  dbDebug              0x02000000L
#define  dbWarning            0x04000000L
#define  dbError              0x08000000L

#define  dbEnter              dbTrace|dbFunctionEnter
#define  dbExit               dbTrace|dbFunctionExit

#define  dbMessage            dbDebug
#define  dbMessage2           dbTrace


#if defined (WIN95)

#define ID_Get_System_Time     0x0001003F

#define vxdCall(service) \
    _asm _emit 0xcd \
    _asm _emit 0x20 \
    _asm _emit ((service) & 0xff) \
    _asm _emit ((service) >> 8) & 0xff \
    _asm _emit ((service) >> 16) & 0xff \
    _asm _emit ((service) >> 24) & 0xff \


#pragma warning(disable : 4035)
DWORD inline
getSystemTime(void)
{
  _asm _emit 0xcd
  _asm _emit 0x20
  _asm _emit 0x3f
  _asm _emit 0x00
  _asm _emit 0x01
  _asm _emit 0x00
  vxdCall(ID_Get_System_Time)
}

#endif

#endif










