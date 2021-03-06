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
                                            DWORD* pdwSlotNumber
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

                if(HalGetBusDataByOffset(PCIConfiguration,
                                    busNumber,
                                    slotNumber.u.AsULONG,
                                    &pciData, 0,
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
NTSTATUS CIOAccessDevice::pciGetDeviceInfo(TPCICARDINFO* pPCICardInfo)
{
    PCI_COMMON_CONFIG PCIConfig;
    NTSTATUS status;

    status = pciGetDeviceConfig(&PCIConfig, pPCICardInfo->dwBusNumber, pPCICardInfo->dwSlotNumber);

    debugOut(dbTrace,"found address %lX, irq %lX", PCIConfig.u.type0.BaseAddresses[0],
                                                    PCIConfig.u.type0.InterruptLine);

    pPCICardInfo->dwMemoryAddress = PCIConfig.u.type0.BaseAddresses[0] & 0xFFFFFFF0;
#ifdef _WIN64
    // TODO: test with actual hardware
    if ((PCIConfig.u.type0.BaseAddresses[0] & 0x6) == 0x4)
        pPCICardInfo->dwMemoryAddress |= (DWORD_PTR)(PCIConfig.u.type0.BaseAddresses[1] & 0xFFFFFFFF) << 32ULL;
#endif
    pPCICardInfo->dwMemoryLength = 0x1000;
    pPCICardInfo->dwSubSystemId = (PCIConfig.u.type0.SubSystemID << 16) + PCIConfig.u.type0.SubVendorID;

    return STATUS_SUCCESS;
}

NTSTATUS CIOAccessDevice::pciGetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot)
{
    debugOut(dbTrace,"get pci configuration for bus %X slot %X",Bus, Slot);

    return HalGetBusDataByOffset(PCIConfiguration, Bus, Slot, pPCIConfig, 0, sizeof(PCI_COMMON_CONFIG));
}

NTSTATUS CIOAccessDevice::pciSetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot)
{
    debugOut(dbTrace,"set pci configuration for bus %X slot %X",Bus, Slot);

    return HalSetBusDataByOffset(PCIConfiguration, Bus, Slot, pPCIConfig, 0, sizeof(PCI_COMMON_CONFIG));
}


NTSTATUS CIOAccessDevice::pciGetDeviceConfigOffset(BYTE* pPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot)
{
    debugOut(dbTrace,"get pci configuration offset for bus %X slot %X",Bus, Slot);

    return HalGetBusDataByOffset(PCIConfiguration, Bus, Slot, pPCIConfig, Offset, 1);
}

NTSTATUS CIOAccessDevice::pciSetDeviceConfigOffset(BYTE *pPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot)
{
    debugOut(dbTrace,"set pci configuration offset for bus %X slot %X",Bus, Slot);

    return HalSetBusDataByOffset(PCIConfiguration, Bus, Slot, pPCIConfig, Offset, 1);
}
