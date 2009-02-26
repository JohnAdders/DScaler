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
// This software was based on hwiodrv from the FreeTV project Those portions are
// Copyright (C) Mathias Ellinger
//
/////////////////////////////////////////////////////////////////////////////

#if ! defined (__DEBUGOUT_H)
#define __DEBUGOUT_H


#if defined (_DEBUG) && ! defined (DEBUG)
#define DEBUG           1
#endif


#if defined (__cplusplus)
extern "C" {
#endif

typedef void (* PDebugOutHandler)(PCHAR lpszMessage);

typedef void (* PDebugSetOutputHandler)(PDebugOutHandler debugHandler);

void traceOut(DWORD debugLevel, PCHAR format, ... );

#define errorOut traceOut

#if defined (DEBUG)
#define debugOut traceOut
#else
#define debugOut
#endif

void debugInitialize(PCHAR devName, DWORD dwDebugLevel);

void debugResetTimeBase(void);
DWORD debugGetSystemTime(void);
void dumpHex(PCHAR data, int size);
void debugSetOutputHandler(PDebugOutHandler outputHandler);

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

#endif










