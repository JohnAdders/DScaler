/////////////////////////////////////////////////////////////////////////////
// DSDrvNT.cpp
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
// 19 Nov 1998   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////

#include "ioclass.h"
#include <stdarg.h>
#include <stdio.h>
#include "debugout.h"

//----------------------------------------------------------------------
//                         FORWARD DEFINES
//----------------------------------------------------------------------
NTSTATUS DSDrvDispatch(IN PDEVICE_OBJECT deviceObject, IN PIRP Irp);
VOID DSDrvUnload  (IN PDRIVER_OBJECT driverObject);

//----------------------------------------------------------------------
//                         GLOBALS
// our user-inteface device object
//---------------------------------------------------------------------------
WCHAR deviceLinkBuffer[] = L"\\DosDevices\\DSDrvNT";
PDEVICE_OBJECT guiDevice = NULL;


//----------------------------------------------------------------------
// In this routine we handle requests to our own device. The only
// requests we care about handling explicitely are IOCTL commands that
// we will get from the GUI. We also expect to get Create and Close
// commands when the GUI opens and closes communications with us.
//----------------------------------------------------------------------
NTSTATUS DSDrvDispatch( IN PDEVICE_OBJECT deviceObject, IN PIRP irp)
{
	CIOAccessDevice *       ioDevice;
	PIO_STACK_LOCATION      irpStack;
	NTSTATUS                ntStatus;

	//
	// Go ahead and set the request up as successful
	//
	ioDevice = (CIOAccessDevice *) deviceObject->DeviceExtension;
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	irpStack = IoGetCurrentIrpStackLocation (irp);

	switch (irpStack->MajorFunction)
	{
	case IRP_MJ_CREATE:
		debugOut(dbTrace,"IRP_MJ_CREATE");
		break;
	case IRP_MJ_CLOSE:
		debugOut(dbTrace,"IRP_MJ_CLOSE");
		break;
	case IRP_MJ_DEVICE_CONTROL:
		ntStatus = ioDevice->deviceIOControl( irp );
		break;
	}

	//
	// Ok our driver complete request immediatly
	//
	IoCompleteRequest( irp, IO_NO_INCREMENT );
	return ntStatus;
}

//----------------------------------------------------------------------
// DriverEntry the Installable driver initialization. Here we just set
// ourselves up.
//----------------------------------------------------------------------
NTSTATUS DriverEntry(IN PDRIVER_OBJECT driverObject, IN PUNICODE_STRING registryPath)
{
	NTSTATUS                ntStatus;
	WCHAR                   deviceNameBuffer[]  = L"\\Device\\DSDrvNT";
	UNICODE_STRING          deviceNameUnicodeString;
	UNICODE_STRING          deviceLinkUnicodeString;
	PDEVICE_OBJECT          deviceObject;
	CIOAccessDevice *       ioDeviceExtension;

	debugInitialize( "DSDrvNT",0xFFFFFFFFL);

	debugOut(dbTrace,"DriverEntry");

	//
	// Create an EXCLUSIVE device object (only 1 thread at a time
	// can make requests to this device)
	//

	RtlInitUnicodeString (&deviceNameUnicodeString, deviceNameBuffer);

	deviceObject      = NULL;
	ioDeviceExtension = NULL;
	ntStatus          = IoCreateDevice (driverObject,
										0,                         // No space for extension
										&deviceNameUnicodeString,
										FILE_DEVICE_UNKNOWN,
										0,
										TRUE,                      // exclusiv open
										&deviceObject);

	if (NT_SUCCESS(ntStatus))
	{
		//
		// Its time to create our device extension object itself
		//

		ioDeviceExtension = new CIOAccessDevice();
		if (ioDeviceExtension)
		{
			//
			// Create dispatch points for device control, create, close.
			//

			driverObject->MajorFunction[IRP_MJ_CREATE] = DSDrvDispatch;
			driverObject->MajorFunction[IRP_MJ_CLOSE] = DSDrvDispatch;
			driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DSDrvDispatch;
			driverObject->DriverUnload = DSDrvUnload;

			//
			// Create a symbolic link, e.g. a name that a Win32 app can specify
			// to open the device
			//

			RtlInitUnicodeString(&deviceLinkUnicodeString, deviceLinkBuffer);

			ntStatus = IoCreateSymbolicLink (&deviceLinkUnicodeString, &deviceNameUnicodeString);

			if (!NT_SUCCESS(ntStatus))
			{
				//
				// Symbolic link creation failed- note this & then delete the
				// device object (it's useless if a Win32 app can't get at it).
				//
				debugOut(dbError,"!IoCreateSymbolicLink failed");
			}
		}
	}
	else
	{
		debugOut(dbError,"! IoCreateDevice failed");
	}


	if (ntStatus)
	{
		if (deviceObject)
		{
			IoDeleteDevice(deviceObject);
		}

		if (ioDeviceExtension)
		{
			delete ioDeviceExtension;
		}
	}
	else
	{
		guiDevice = deviceObject;
		deviceObject->DeviceExtension = ioDeviceExtension;          // create back link
	}
	return ntStatus;
}


//----------------------------------------------------------------------
// Our job is done - time to leave.
//----------------------------------------------------------------------
VOID DSDrvUnload(IN PDRIVER_OBJECT driverObject)
{
	UNICODE_STRING deviceLinkUnicodeString;

	debugOut(dbTrace,"DSDrvNT Unload");

	RtlInitUnicodeString(&deviceLinkUnicodeString, deviceLinkBuffer);
	IoDeleteSymbolicLink(&deviceLinkUnicodeString);

	CIOAccessDevice* ioDevice;

	ioDevice = (CIOAccessDevice *)driverObject->DeviceObject->DeviceExtension;

	delete ioDevice;

	//
	// Delete the device object
	//
	IoDeleteDevice( driverObject->DeviceObject );
}
