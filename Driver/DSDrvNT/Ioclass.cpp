/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.cpp,v 1.3 2001-07-13 16:13:53 adcockj Exp $
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
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CIOAccessDevice::CIOAccessDevice(void)
{
    dwBusNumber = 0;
    dwSlotNumber = 0;
    dwMemoryBase = 0;
    dwMappedMemoryLength = 0;
    memset(&memoryList, 0, sizeof(memoryList));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CIOAccessDevice::~CIOAccessDevice()
{
    int Index;
    PMemoryNode node;

    //
    // Free allocated memory
    //

    for (Index = 0; Index < MAX_FREE_MEMORY_NODES; Index++)
    {
        node = &memoryList[Index];

        if(node->dwSystemAddress)
        {
            freeMemory (node);
        }
    }
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::deviceIOControl(PIRP irp)
{
    PIO_STACK_LOCATION      irpStack;
    PVOID                   inputBuffer;
    PVOID                   outputBuffer;
    DWORD                   inputBufferLength;
    DWORD                   outputBufferLength;
    NTSTATUS                ntStatus;
    DWORD                   ioControlCode;

    //
    // Get a pointer to the current location in the Irp. This is where
    //     the function codes and parameters are located.
    //

    irpStack        = IoGetCurrentIrpStackLocation (irp);
    ioControlCode   = irpStack->Parameters.DeviceIoControl.IoControlCode;

    //
    // Get the pointer to the input/output buffer and its length
    //

    switch ( IOCTL_TRANSFER_TYPE(ioControlCode) )
    {
    case METHOD_IN_DIRECT:
        if ( irp->MdlAddress )
        {
            inputBuffer       = MmGetSystemAddressForMdl( irp->MdlAddress );
        }
        else
        {
            inputBuffer        = irp->AssociatedIrp.SystemBuffer;
        }

        inputBufferLength   = irpStack->Parameters.DeviceIoControl.InputBufferLength;
        outputBuffer        = irp->AssociatedIrp.SystemBuffer;
        outputBufferLength  = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        break;

    case METHOD_OUT_DIRECT:
        if ( irp->MdlAddress )
        {
            outputBuffer      = MmGetSystemAddressForMdl( irp->MdlAddress );
        }
        else
        {
            debugOut(dbError,"mdl for ioControl %X expected",ioControlCode);
        }

        inputBuffer         = irp->AssociatedIrp.SystemBuffer;
        inputBufferLength   = irpStack->Parameters.DeviceIoControl.InputBufferLength;
        outputBufferLength  = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        break;

    default:
        inputBuffer        = irp->AssociatedIrp.SystemBuffer;
        inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
        outputBuffer       = irp->AssociatedIrp.SystemBuffer;
        outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        break;
    }

    ntStatus = deviceControl(ioControlCode,
                            (PDSDrvParam) inputBuffer,
                            (PULONG) outputBuffer,
                            &irp->IoStatus.Information);

    return ntStatus;
}

//---------------------------------------------------------------------------
// The READ/WRITE_PORT_* calls manipulate I/O registers in PORT space.
// (Use x86 in/out instructions.)
//
// The READ/WRITE_REGISTER_* calls manipulate I/O registers in MEMORY space.
// (Use x86 move instructions)
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::deviceControl(DWORD ioControlCode, PDSDrvParam ioParam, DWORD* outputBuffer, DWORD* pBytesWritten)
{
    NTSTATUS status;

    status = STATUS_SUCCESS;
    *pBytesWritten = 0;

    switch ( ioControlCode )
    {
    case ioctlReadBYTE:
        osPortReadByte((ULONG)ioParam->dwAddress, *outputBuffer);
        *pBytesWritten = 1;
        break;

    case ioctlReadWORD:
        osPortReadWord((ULONG)ioParam->dwAddress, *outputBuffer);
        *pBytesWritten = 2;
        break;

    case ioctlReadDWORD:
        osPortReadLong((ULONG)ioParam->dwAddress, *outputBuffer);
        *pBytesWritten = 4;
        break;

    case ioctlWriteBYTE:
        osPortWriteByte((ULONG)ioParam->dwAddress, ioParam->dwValue);
        break;

    case ioctlWriteWORD:
        osPortWriteWord((ULONG)ioParam->dwAddress, (USHORT) ioParam->dwValue);
        break;

    case ioctlWriteDWORD:
        osPortWriteLong((ULONG)ioParam->dwAddress, ioParam->dwValue);
        break;

    case ioctlGetPCIInfo:
        status = pciFindDevice((USHORT) ioParam->dwAddress,
                             (USHORT) ioParam->dwValue,
                             &dwBusNumber,
                             &dwSlotNumber);

        if ( status == STATUS_SUCCESS)
        {
            if ( ! isValidAddress(outputBuffer ) )
            {
                debugOut(dbError,"! invalid system address %X",outputBuffer);
            }
            else
            {
                status = pciGetDeviceConfig(dwBusNumber,
                                          dwSlotNumber,
                                          (PPCI_COMMON_CONFIG) outputBuffer);
            }
        }
        else
        {
            debugOut(dbTrace,"pci device for vendor %lX deviceID %lX not found",ioParam->dwAddress,ioParam->dwValue);
        }
        *pBytesWritten = sizeof(PCI_COMMON_CONFIG);
        break;

    case ioctlAllocMemory:
        {
            PMemStruct pMem = (PMemStruct)outputBuffer;
            status = allocMemory(ioParam->dwValue, ioParam->dwFlags, ioParam->dwAddress,  pMem);
            *pBytesWritten = sizeof(TMemStruct) + pMem->dwPages * sizeof(TPageStruct);
        }
        break;

    case ioctlFreeMemory:
        {
            PMemStruct pMem = (PMemStruct)ioParam;
            status = freeMemory(pMem);
        }
        break;

    case ioctlMapMemory:
        *outputBuffer = mapMemory( ioParam->dwAddress, ioParam->dwValue);
        *pBytesWritten = 4;
        break;

    case ioctlUnmapMemory:
        unmapMemory();
        break;

    case ioctlReadMemoryDWORD:
        if (dwMemoryBase)
        {
            DWORD Address = ioParam->dwAddress + dwMemoryBase;
            osMemoryReadDWORD( Address, *outputBuffer);
            *pBytesWritten = 4;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *outputBuffer);
        }
        break;

    case ioctlWriteMemoryDWORD:
        if (dwMemoryBase)
        {
            DWORD Address = ioParam->dwAddress + dwMemoryBase;
            osMemoryWriteDWORD( Address, ioParam->dwValue );
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case ioctlReadMemoryWORD:
        if (dwMemoryBase)
        {
            DWORD Address = ioParam->dwAddress + dwMemoryBase;
            osMemoryReadWORD( Address, *outputBuffer);
            *pBytesWritten = 2;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, (DWORD)*outputBuffer);
        }
        break;

    case ioctlWriteMemoryWORD:
        if (dwMemoryBase)
        {
            DWORD Address = ioParam->dwAddress + dwMemoryBase;
            osMemoryWriteWORD( Address, ioParam->dwValue );
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case ioctlReadMemoryBYTE:
        if (dwMemoryBase)
        {
            DWORD Address = ioParam->dwAddress + dwMemoryBase;
            osMemoryReadBYTE( Address, *outputBuffer);
            *pBytesWritten = 1;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, (DWORD)*outputBuffer);
        }
        break;

    case ioctlWriteMemoryBYTE:
        if (dwMemoryBase)
        {
            DWORD Address = ioParam->dwAddress + dwMemoryBase;
            osMemoryWriteBYTE( Address, ioParam->dwValue );
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    default:
        debugOut(dbError,"unknown command %lX",ioControlCode);
        status=STATUS_INVALID_PARAMETER;
        break;
    }
    return status;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int CIOAccessDevice::isValidAddress(void * pvAddress)
{
    return MmIsAddressValid( pvAddress);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::allocMemory(DWORD dwLength, DWORD dwFlags, DWORD dwUserAddress, PMemStruct pMemStruct)
{
    NTSTATUS      ntStatus;
    PMemoryNode   node;
    DWORD         dwIndex;
    PPageStruct pPages = (PPageStruct)(pMemStruct + 1);

    // Initialize the MemStruct
    pMemStruct->dwFlags = dwFlags;
    pMemStruct->dwHandle = 0;
    pMemStruct->dwPages = 0;
    pMemStruct->dwTotalSize = 0;
    pMemStruct->dwUser = (void*)dwUserAddress;

    //
    //  First alloc our own free memory descriptor
    //
    debugOut(dbTrace,"allocMemory %lu ",dwLength);

    for ( dwIndex = 0; dwIndex < MAX_FREE_MEMORY_NODES; dwIndex++)
    {
        node = &memoryList[ dwIndex ];

        if ( ! node->dwSystemAddress )
        {
            break;
        }
    }

    if ( dwIndex >= MAX_FREE_MEMORY_NODES)
    {
        debugOut(dbTrace," ! no free memory descriptor available");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ntStatus = STATUS_SUCCESS;

    PHYSICAL_ADDRESS highestAcceptableAddress;

    highestAcceptableAddress.LowPart  =  -1;
    highestAcceptableAddress.HighPart =  -1;

    if(dwFlags & ALLOC_MEMORY_CONTIG)
    {
        node->dwSystemAddress = (DWORD) MmAllocateContiguousMemory(dwLength, highestAcceptableAddress);
        if (!node->dwSystemAddress)
        {
            debugOut(dbTrace,"! cannot alloc ContiguousMemory");
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        dwUserAddress = node->dwSystemAddress;
    }

    debugOut(dbTrace,"alloc %lu bytes of system memory %X", dwLength, dwUserAddress);

    //
    // build the MDL to desribe the memory pages
    //
    node->pMdl = IoAllocateMdl((void*)dwUserAddress, dwLength, FALSE, FALSE, NULL);
    if (!node->pMdl)
    {
        if(dwFlags & ALLOC_MEMORY_CONTIG)
        {
            MmFreeContiguousMemory((PVOID) node->dwSystemAddress);
            node->dwSystemAddress = 0;
        }
        debugOut(dbTrace,"! cannot alloc MDL");
        return  STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        debugOut(dbTrace,"node->pMdl %X", node->pMdl);
        //
        // Map locked pages into process's user address space
        //
        MmProbeAndLockPages(node->pMdl, KernelMode, IoModifyAccess);
        debugOut(dbTrace,"Locked");

        // OK so we've got some memory and we can fill
        // in the return structure

        // need to store this so we know to deallocate
        // any contig memory
        node->dwFlags = dwFlags;

        pMemStruct->dwTotalSize = dwLength;
        pMemStruct->dwHandle = (DWORD)node;
        if(dwFlags & ALLOC_MEMORY_CONTIG)
        {
            node->dwUserAddress = (DWORD) MmMapLockedPages(node->pMdl, UserMode );
            pMemStruct->dwPages = 1;
            pMemStruct->dwUser = (void*)node->dwUserAddress;
            pPages[0].dwSize = dwLength;
            pPages[0].dwPhysical = GetPhysAddr(node->dwUserAddress);
        }
        else
        {
            node->dwSystemAddress = GetPhysAddr(dwUserAddress);
            node->dwUserAddress = dwUserAddress;
            DWORD LastUserAddr = dwUserAddress;
            int Pages(1);
            pPages[0].dwPhysical = GetPhysAddr(dwUserAddress); 
            for(DWORD i = dwUserAddress; i < dwUserAddress + dwLength; i++)
            {
                if(i % 4096 == 0)
                {
                    pPages[Pages].dwPhysical = GetPhysAddr(i); 
                    pPages[Pages - 1].dwSize = i - LastUserAddr;
                    Pages++;
                    LastUserAddr = i;
                }
            }
            pPages[Pages - 1].dwSize = i - LastUserAddr;
            pMemStruct->dwPages = Pages;
        }
        debugOut(dbTrace,"node->dwUserAddress %X", node->dwUserAddress);
        debugOut(dbTrace,"Pages %d", (node->pMdl->Size - sizeof(MDL))/4);
    }
    return ntStatus;
}



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS
CIOAccessDevice::freeMemory(PMemStruct pMemStruct)
{
    PMemoryNode node;
    DWORD       dwIndex;

    //
    // First search memory node with given dwUserAddress
    //

    debugOut(dbTrace,"freeMemory()");

    for( dwIndex = 0; dwIndex < MAX_FREE_MEMORY_NODES; dwIndex++)
    {
        node = &memoryList[ dwIndex ];

        if ((DWORD)node == pMemStruct->dwHandle)
        {
            freeMemory(node);
            return STATUS_SUCCESS;
        }
    }

    debugOut(dbError," ! memory address not found");
    return STATUS_ACCESS_DENIED;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CIOAccessDevice::freeMemory(PMemoryNode node)
{
    debugOut(dbTrace,"free node");
    if(node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        MmUnmapLockedPages((PVOID) node->dwUserAddress, node->pMdl);
    }
    
    MmUnlockPages(node->pMdl);

    IoFreeMdl(node->pMdl);

    if(node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        MmFreeContiguousMemory((PVOID)node->dwSystemAddress);
    }
    memset(node, 0, sizeof(MemoryNode));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CIOAccessDevice::GetPhysAddr(DWORD UserAddr)
{
    PHYSICAL_ADDRESS phys;
    phys = MmGetPhysicalAddress((void*)UserAddr);
    return phys.LowPart;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CIOAccessDevice::mapMemory(DWORD dwPhysicalAddress, DWORD dwLength)
{
    if (!dwMemoryBase)
    {
        dwMappedMemoryLength = dwLength;

        debugOut(dbTrace,"mapMemory for %X", dwPhysicalAddress);

        PHYSICAL_ADDRESS translatedAddress;
        PHYSICAL_ADDRESS busAddress;
        DWORD addressSpace;
        BOOLEAN bTranslate;

        KIRQL irql;

        KeRaiseIrql(PASSIVE_LEVEL, &irql);

        busAddress.LowPart          = dwPhysicalAddress | 8;
        busAddress.HighPart         = 0;
        translatedAddress.LowPart   = 0;
        translatedAddress.HighPart  = 0;
        addressSpace                = 0xFF;

        bTranslate = HalTranslateBusAddress(PCIBus,
                                       dwBusNumber,
                                       busAddress,
                                       &addressSpace,
                                       &translatedAddress);

        if (!bTranslate)
        {
            debugOut(dbError,"HalTranslateBusAddress() failed, addressSpace %X",addressSpace);
            translatedAddress.LowPart = dwPhysicalAddress;
        }
        //else
        {
            //
            // memory space
            //
            dwMemoryBase = (DWORD) MmMapIoSpace(translatedAddress, dwLength, MmNonCached);

            debugOut(dbTrace,"map pysical address %X to memory base %X,length %d",translatedAddress.LowPart, dwMemoryBase,dwLength);
        }

        KeLowerIrql(irql);
    }
    else
    {
        debugOut(dbError,"! mapMemory failed, already mapped to %lX",dwMemoryBase);
        return dwMemoryBase;
    }

    return dwMemoryBase;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void
CIOAccessDevice::unmapMemory(void)
{
    if ( dwMemoryBase )
    {
        debugOut(dbTrace,"MMUnmapIoSpace() %X",dwMemoryBase);
        MmUnmapIoSpace( (PVOID) dwMemoryBase, dwMappedMemoryLength);
        dwMemoryBase = 0;
    }
}
