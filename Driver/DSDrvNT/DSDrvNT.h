/////////////////////////////////////////////////////////////////////////////
// $Id: DSDrvNT.h,v 1.3 2001-11-02 10:45:51 adcockj Exp $
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
// 11 Jun 2001   Torbjörn Jansson      moved forward defines from DSDrvNT.cpp
//
/////////////////////////////////////////////////////////////////////////////

//#ifdef DBG
// #define DEBUGSTR(S) {DbgPrint("%s(%d) : ", __FILE__, __LINE__);DbgPrint S;}
//#else
// #define DEBUGSTR(S)
//#endif

//device name
#define DSDRVNT_DEVNAME L"\\Device\\DSDrv4"
//name of symbolic link
#define DSDRVNT_LNKNAME L"\\DosDevices\\DSDrv4"


NTSTATUS DSDrvDispatch(IN PDEVICE_OBJECT deviceObject, IN PIRP Irp);
VOID DSDrvUnload  (IN PDRIVER_OBJECT driverObject);
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT driverObject, IN PUNICODE_STRING registryPath);
