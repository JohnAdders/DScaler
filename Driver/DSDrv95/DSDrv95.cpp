/////////////////////////////////////////////////////////////////////////////
// $Id: DSDrv95.cpp,v 1.4 2001-11-02 16:36:54 adcockj Exp $
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
// 19 Nov 1998   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.2  2001/07/13 16:13:53  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "ioclass.h"
#include "debugout.h"

extern "C" {

DWORD _stdcall DSDrvW32DeviceIOControl(
                                      DWORD           dwService,
                                      DWORD           dwDDB,
                                      DWORD           hDevice,
                                      PDIOCPARAMETERS ioParam
                                     );


DWORD _stdcall DSDrvDynamicInit       (void);
DWORD _stdcall DSDrvDynamicExit       (void);
DWORD _stdcall DSDrvInitComplete      (void);

}


CIOAccessDevice * ioDevice;


//---------------------------------------------------------------------------
// Perform device IO control
//---------------------------------------------------------------------------
DWORD _stdcall DSDrvW32DeviceIOControl(DWORD           dwService,
                        DWORD           dwDDB,
                        DWORD           hDevice,
                        PDIOCPARAMETERS ioParam)
{
  debugOut(dbTrace,"DSDrvW32DeviceIOControl() %X",dwService);

  ioParam->dwIoControlCode = dwService;

  return ioDevice->deviceIOControl(ioParam);
 }

//---------------------------------------------------------------------------
// This function is called when the driver was loaded by Windows 95
// a static or dynamic vxd call your own startup code function from
// this function
//---------------------------------------------------------------------------
DWORD _stdcall DSDrvDynamicInit(void)
 {
  debugInitialize("DSDrv4",0xFFFFFFFFL);

  debugOut(dbTrace,"DSDrvDynamicInit()");

  ioDevice = new CIOAccessDevice();

  return( ioDevice != 0);
 }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD _stdcall DSDrvDynamicExit(void)
 {
  debugOut(dbTrace,"DSDrvDynamicExit()");

  delete ioDevice;

  return 1;
 }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD _stdcall DSDrvInitComplete(void)
 {
  debugOut(dbTrace,"DSDrvInitComplete()");

  return 1;
}


