/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.cpp,v 1.14 2005-05-13 10:12:46 adcockj Exp $
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
// Revision 1.13  2004/04/14 10:02:02  adcockj
// Added new offset functions for manipulating PCI config space
//
// Revision 1.12  2002/10/22 16:01:45  adcockj
// Changed definition of IOCTLs
//
// Revision 1.11  2002/06/16 18:53:36  robmuller
// Renamed pciGetDeviceConfig() to pciGetDeviceInfo().
// Implemented pciGetDeviceConfig() and pciSetDeviceConfig().
//
// Revision 1.10  2001/11/02 16:36:54  adcockj
// Merge code from Multiple cards into main trunk
//
// Revision 1.7.2.1  2001/08/15 08:54:28  adcockj
// Tidy up driver code
//
// Revision 1.7  2001/08/11 13:46:03  adcockj
// Fix for driver problem
//
// Revision 1.6  2001/08/11 12:47:12  adcockj
// Driver fixes
//
// Revision 1.5  2001/08/08 16:37:50  adcockj
// Made drivers stateless to support multiple cards
// Added version check
// Changed meaning of memory access functions so that you no longer pass just the offset
//
// Revision 1.4  2001/08/08 10:53:30  adcockj
// Preliminary changes to driver to support multiple cards
//
// Revision 1.3  2001/07/13 16:13:53  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CIOAccessDevice::CIOAccessDevice(void) :
    m_AllowDepricatedIOCTLs(false)
{
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
    PVOID                   outputBuffer = NULL;
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
    NTSTATUS Status;

    Status = STATUS_SUCCESS;
    *pBytesWritten = 0;

    switch ( ioControlCode )
    {
    case IOCTL_DEPRICATED_READPORTBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READPORTBYTE:
        {
            UCHAR* pByte = (UCHAR*)outputBuffer;
            *pByte = READ_PORT_UCHAR ((PUCHAR)&ioParam->dwAddress);
            *pBytesWritten = 1;
        }
        break;

    case IOCTL_DEPRICATED_READPORTWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READPORTWORD:
        {
            USHORT* pWord = (USHORT*)outputBuffer;
            *pWord = READ_PORT_USHORT ((PUSHORT)&ioParam->dwAddress);
            *pBytesWritten = 2;
        }
        break;

    case IOCTL_DEPRICATED_READPORTDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READPORTDWORD:
        {
            *outputBuffer = READ_PORT_ULONG ((PULONG)&ioParam->dwAddress);
            *pBytesWritten = 4;
        }
        break;

    case IOCTL_DEPRICATED_WRITEPORTBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTBYTE:
        WRITE_PORT_UCHAR((PUCHAR)&ioParam->dwAddress, (UCHAR)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_WRITEPORTWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTWORD:
        WRITE_PORT_USHORT((PUSHORT)&ioParam->dwAddress, (USHORT)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_WRITEPORTDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTDWORD:
        WRITE_PORT_ULONG(&ioParam->dwAddress, (ULONG)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_GETPCIINFO:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_GETPCIINFO:
        if (isValidAddress(outputBuffer))
        {
            TPCICARDINFO* pPCICardInfo = (TPCICARDINFO*)outputBuffer;
            Status = pciFindDevice(
                                       ioParam->dwAddress,
                                       ioParam->dwValue,
                                       ioParam->dwFlags,
                                       &(pPCICardInfo->dwBusNumber),
                                       &(pPCICardInfo->dwSlotNumber)
                                  );

            if ( Status == STATUS_SUCCESS)
            {
                Status = pciGetDeviceInfo(pPCICardInfo);
            }
            else
            {
                debugOut(dbTrace,"ioctlGetPCIInfo: pci device for vendor %lX deviceID %lX not found",ioParam->dwAddress,ioParam->dwValue);
            }
            *pBytesWritten = sizeof(TPCICARDINFO);
        }
        else
        {
            debugOut(dbError,"! invalid system address %X",outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_ALLOCMEMORY:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_ALLOCMEMORY:
        {
            PMemStruct pMem = (PMemStruct)outputBuffer;
            Status = allocMemory(ioParam->dwValue, ioParam->dwFlags, ioParam->dwAddress,  pMem);
            *pBytesWritten = sizeof(TMemStruct) + pMem->dwPages * sizeof(TPageStruct);
        }
        break;

    case IOCTL_DEPRICATED_FREEMEMORY:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_FREEMEMORY:
        {
            PMemStruct pMem = (PMemStruct)ioParam;
            Status = freeMemory(pMem);
        }
        break;

    case IOCTL_DEPRICATED_MAPMEMORY:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_MAPMEMORY:
        *outputBuffer = mapMemory(ioParam->dwAddress,  ioParam->dwValue, ioParam->dwFlags);
        *pBytesWritten = 4;
        break;

    case IOCTL_DEPRICATED_UNMAPMEMORY:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_UNMAPMEMORY:
        unmapMemory(ioParam->dwAddress,  ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_READMEMORYDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYDWORD:
        if (ioParam->dwAddress)
        {
            *outputBuffer = READ_REGISTER_ULONG((PULONG)ioParam->dwAddress);
            *pBytesWritten = 4;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYDWORD:
        if (ioParam->dwAddress)
        {
            WRITE_REGISTER_ULONG((PULONG)ioParam->dwAddress, ioParam->dwValue);
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_READMEMORYWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYWORD:
        if (ioParam->dwAddress)
        {
            USHORT* pWord = (USHORT*)outputBuffer;
            *pWord = READ_REGISTER_USHORT((PUSHORT)ioParam->dwAddress);
            *pBytesWritten = 2;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *pWord);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYWORD:
        if (ioParam->dwAddress)
        {
            WRITE_REGISTER_USHORT((PUSHORT)ioParam->dwAddress, (USHORT)ioParam->dwValue);
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_READMEMORYBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYBYTE:
        if (ioParam->dwAddress)
        {
            UCHAR* pByte = (UCHAR*)outputBuffer;
            *pByte = READ_REGISTER_UCHAR((PUCHAR)ioParam->dwAddress);
            *pBytesWritten = 1;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *pByte);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYBYTE:
        if (ioParam->dwAddress)
        {
            WRITE_REGISTER_UCHAR((PUCHAR)ioParam->dwAddress, (UCHAR)ioParam->dwValue);
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_GETVERSION:
        // if we get called on this IOCTL
        // we must be being called by an older version
        // of dscaler so switch on the potentially unsafe
        // IOCTLS that we depricated because of problems in XP
        *outputBuffer = DSDRV_VERSION;
        *pBytesWritten = sizeof(DWORD);
        m_AllowDepricatedIOCTLs = true;
        break;

    case IOCTL_DSDRV_GETVERSION:
        *outputBuffer = DSDRV_VERSION;
        *pBytesWritten = sizeof(DWORD);
        break;

    case IOCTL_DEPRICATED_GETPCICONFIG:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_GETPCICONFIG:
        if (isValidAddress(outputBuffer))
        {
            PCI_COMMON_CONFIG *pPCIConfig = (PCI_COMMON_CONFIG*)outputBuffer;

            Status = pciGetDeviceConfig(pPCIConfig, ioParam->dwAddress, ioParam->dwValue);

            *pBytesWritten = sizeof(PCI_COMMON_CONFIG);
        }
        else
        {
            debugOut(dbError,"! invalid system address %X",outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_SETPCICONFIG:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_SETPCICONFIG:
        if (isValidAddress(outputBuffer))
        {
            PCI_COMMON_CONFIG *pPCIConfig = (PCI_COMMON_CONFIG*)outputBuffer;

            Status = pciSetDeviceConfig(pPCIConfig, ioParam->dwAddress, ioParam->dwValue);

            *pBytesWritten = sizeof(PCI_COMMON_CONFIG);
        }
        else
        {
            debugOut(dbError,"! invalid system address %X",outputBuffer);
        }
        break;

    case IOCTL_DSDRV_GETPCICONFIGOFFSET:
        if (isValidAddress(outputBuffer))
        {
            BYTE *pPCIConfig = (BYTE*)outputBuffer;

            Status = pciGetDeviceConfigOffset(pPCIConfig, ioParam->dwFlags, ioParam->dwAddress, ioParam->dwValue);

            *pBytesWritten = 1;
        }
        else
        {
            debugOut(dbError,"! invalid system address %X",outputBuffer);
        }
        break;

    case IOCTL_DSDRV_SETPCICONFIGOFFSET:
        if (isValidAddress(outputBuffer))
        {
            BYTE *pPCIConfig = (BYTE*)outputBuffer;

            Status = pciSetDeviceConfigOffset(pPCIConfig, ioParam->dwFlags, ioParam->dwAddress, ioParam->dwValue);

            *pBytesWritten = 1;
        }
        else
        {
            debugOut(dbError,"! invalid system address %X",outputBuffer);
        }
        break;

    default:
        debugOut(dbError,"unknown command %lX",ioControlCode);
        Status = STATUS_INVALID_PARAMETER;
        *pBytesWritten = 0;
        break;
    }
    return Status;
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
DWORD CIOAccessDevice::mapMemory(DWORD dwBusNumber, DWORD dwPhysicalAddress, DWORD dwLength)
{
    debugOut(dbTrace,"mapMemory for %X", dwPhysicalAddress);

    PHYSICAL_ADDRESS translatedAddress;
    PHYSICAL_ADDRESS busAddress;
    DWORD addressSpace;
    DWORD dwMemoryBase;
    BOOLEAN bTranslate;

    KIRQL irql;

    KeRaiseIrql(PASSIVE_LEVEL, &irql);

    busAddress.LowPart          = dwPhysicalAddress;
    busAddress.HighPart         = 0;
    translatedAddress.LowPart   = 0;
    translatedAddress.HighPart  = 0;
    addressSpace                = 0x00;

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

    //
    // memory space
    //
    dwMemoryBase = (DWORD) MmMapIoSpace(translatedAddress, dwLength, MmNonCached);

    debugOut(dbTrace,"MmMapIoSpace physical address %X to memory base %X, length %d",translatedAddress.LowPart, dwMemoryBase, dwLength);

    KeLowerIrql(irql);

    return dwMemoryBase;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void
CIOAccessDevice::unmapMemory(DWORD dwMemoryBase, DWORD dwMappedMemoryLength)
{
    if(dwMemoryBase)
    {
        KIRQL irql;
        KeRaiseIrql(PASSIVE_LEVEL, &irql);

        debugOut(dbTrace,"MMUnmapIoSpace() %X, length %d",dwMemoryBase, dwMappedMemoryLength);
        MmUnmapIoSpace( (PVOID) dwMemoryBase, dwMappedMemoryLength);

        KeLowerIrql(irql);
    }
}
