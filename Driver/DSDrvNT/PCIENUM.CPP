/////////////////////////////////////////////////////////////////////////////
// $Id: PCIENUM.CPP,v 1.4 2001-08-08 10:53:30 adcockj Exp $
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
//  1 Nov 2000   Michael Eskin         Fixed Win95 PCI device enum for systems
//               Conexant Systems      that have had several boards installed
//                                     over the life of the system.
//                                     
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2001/07/13 16:13:53  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

//---------------------------------------------------------------------------
// Find the PCI devices for the VendorId and DeviceId specified by the
// caller.
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::pciFindDevice(
                                            DWORD vendorID, 
                                            DWORD deviceID, 
                                            DWORD dwCardIndex,
                                            DWORD* pdwBusNumber, 
                                            DWORD*  pdwSlotNumber
                                       )
{
    PCI_SLOT_NUMBER         slotNumber;
    PCI_COMMON_CONFIG       pciData;
    DWORD                   busNumber;
    DWORD                   deviceNumber;
    DWORD                   functionNumber;
    DWORD                   CardCount(0);

    // Remeber ...
    //
    // typedef struct _PCI_SLOT_NUMBER {
    //     union {
    //         struct {
    //             DWORD   DeviceNumber:5;
    //             DWORD   FunctionNumber:3;
    //             DWORD   Reserved:24;
    //         } bits;
    //         DWORD   AsULONG;
    //     } u;
    // } PCI_SLOT_NUMBER, *PPCI_SLOT_NUMBER;
    //
    slotNumber.u.AsULONG = 0;

    //
    // Scan each bus.
    //
    for (busNumber = 0; busNumber < 256; busNumber++)
    {
        //
        // Scan each device.
        //
        for(deviceNumber = 0; deviceNumber < PCI_MAX_DEVICES; deviceNumber++)
        {
            slotNumber.u.bits.DeviceNumber = deviceNumber;

            //
            // Scan each function.
            //

            for(functionNumber = 0; functionNumber < PCI_MAX_FUNCTION; functionNumber++)
            {
                slotNumber.u.bits.FunctionNumber = functionNumber;

                //
                // Check what's in the current slot.
                // Read the complete 256 bytes of configuration information
                //

                if(HalGetBusData(PCIConfiguration,
                                     busNumber,
                                     slotNumber.u.AsULONG,
                                     &pciData,
                                     sizeof(DWORD)) == 0)
                {
                    //
                    // The specified PCI bus does not exist.  We are done
                    // with this bus.  Set the device number to the maximum
                    // and break out of the function loop.  This will scan
                    // the next system bus.  There is no guarantee that the
                    // system buses are sequentially ordered.
                    //
                    deviceNumber = PCI_MAX_DEVICES;
                    break;
                }

                if(pciData.VendorID != 0xFFFF)
                {
                    debugOut(dbTrace,"found pci device %lX %lX",pciData.VendorID,pciData.DeviceID);
                }

                if((pciData.VendorID == vendorID) && (pciData.DeviceID == deviceID))
                {
                    //
                    // At this point, we've found a valid PCI device.
                    // check if it is the one we want
                    if(CardCount == dwCardIndex)
                    {
                        // Get back the information for the located device.
                        *pdwBusNumber  = busNumber;
                        *pdwSlotNumber = slotNumber.u.AsULONG;

                        return STATUS_SUCCESS;
                    }
                    else
                    {
                        ++CardCount;
                    }
                }
            } // functionNumber
        }   // deviceNumber
    }   // busNumber

    return STATUS_DEVICE_DOES_NOT_EXIST;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::pciGetDeviceConfig(DWORD dwBusNumber, DWORD dwSlotNumber, PPCI_COMMON_CONFIG pciConfiguration)
{
    NTSTATUS status;

    debugOut(dbTrace,"get pci configuration for bus %X slot %X",dwBusNumber,dwSlotNumber);

    status = HalGetBusData(PCIConfiguration,
                            dwBusNumber,
                            dwSlotNumber,
                            pciConfiguration,
                            sizeof(*pciConfiguration));

    debugOut(dbTrace,"found address %lX, irq %lX", pciConfiguration->u.type0.BaseAddresses[0],
                                                    pciConfiguration->u.type0.InterruptLine);
    return STATUS_SUCCESS;
}
