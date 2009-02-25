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
// Revision 1.6  2002/06/16 10:04:04  adcockj
// Changed driver to not be exclusive
//
// Revision 1.5  2001/11/02 16:36:54  adcockj
// Merge code from Multiple cards into main trunk
//
// Revision 1.3.2.1  2001/08/14 16:41:37  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.3  2001/07/13 16:13:53  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "DSDrvNT.h"


//----------------------------------------------------------------------
// In this routine we handle requests to our own device. The only
// requests we care about handling explicitely are IOCTL commands that
// we will get from the GUI. We also expect to get Create and Close
// commands when the GUI opens and closes communications with us.
//----------------------------------------------------------------------
NTSTATUS DSDrvDispatch( IN PDEVICE_OBJECT deviceObject, IN PIRP irp)
{
    CIOAccessDevice *ioDevice;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    //
    // Go ahead and set the request up as successful
    //
    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;
    irpStack = IoGetCurrentIrpStackLocation (irp);
    
    ioDevice = (CIOAccessDevice *) irpStack->FileObject->FsContext;

    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:
        debugOut(dbTrace,"IRP_MJ_CREATE");
        
        //create a CIOAccessDevice and assosiate it with this filehandle
        irpStack->FileObject->FsContext=new CIOAccessDevice();

        break;
    case IRP_MJ_CLOSE:
        debugOut(dbTrace,"IRP_MJ_CLOSE");
        
        //remove CIOAccessDevice
        //if thers any memory currently alloocated CIOAccessDevice's destructor
        //unlocks the memory and frees it
        delete (CIOAccessDevice*)irpStack->FileObject->FsContext;
        irpStack->FileObject->FsContext=NULL;

        break;
    case IRP_MJ_DEVICE_CONTROL:
        debugOut(dbTrace,"IRP_MJ_DEVICE_CONTROL");
        ntStatus = ioDevice->deviceIOControl( irp );
        break;
    default:
        debugOut(dbWarning,"Unknown IRP!!!");
        //oops dont know how to handle this IRP, we shoud never get here
        ntStatus=STATUS_NOT_IMPLEMENTED;
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
extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT driverObject, IN PUNICODE_STRING registryPath)
{
    NTSTATUS                ntStatus;
    UNICODE_STRING          deviceNameUnicodeString;
    UNICODE_STRING          deviceLinkUnicodeString;
    PDEVICE_OBJECT          deviceObject;

    debugInitialize( "DSDrv4",0xFFFFFFFFL);

    debugOut(dbTrace,"DriverEntry");

    //
    // Create adevice object
    //

    RtlInitUnicodeString (&deviceNameUnicodeString, DSDRVNT_DEVNAME);

    deviceObject      = NULL;
    ntStatus          = IoCreateDevice (driverObject,
                                        0,                         // No space for extension
                                        &deviceNameUnicodeString,
                                        FILE_DEVICE_UNKNOWN,
                                        0,
                                        FALSE,
                                        &deviceObject);

    if (NT_SUCCESS(ntStatus))
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

        RtlInitUnicodeString(&deviceLinkUnicodeString, DSDRVNT_LNKNAME);

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

    }
    return ntStatus;
}


//----------------------------------------------------------------------
// Our job is done - time to leave.
//----------------------------------------------------------------------
VOID DSDrvUnload(IN PDRIVER_OBJECT driverObject)
{
    UNICODE_STRING deviceLinkUnicodeString;

    debugOut(dbTrace,"DSDrv4 Unload");

    RtlInitUnicodeString(&deviceLinkUnicodeString, DSDRVNT_LNKNAME);
    IoDeleteSymbolicLink(&deviceLinkUnicodeString);

    //
    // Delete the device object
    //
    IoDeleteDevice( driverObject->DeviceObject );
}
