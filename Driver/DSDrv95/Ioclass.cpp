/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.cpp,v 1.5 2001-08-14 10:30:39 adcockj Exp $
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
// Revision 1.4  2001/08/08 16:37:50  adcockj
// Made drivers stateless to support multiple cards
// Added version check
// Changed meaning of memory access functions so that you no longer pass just the offset
//
// Revision 1.3  2001/08/08 10:53:30  adcockj
// Preliminary changes to driver to support multiple cards
//
// Revision 1.2  2001/07/13 16:13:53  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "ioclass.h"
#include "debugout.h"

#ifdef WIN95
extern "C"
{
    void* __cdecl memset( void *dest, int c, size_t count);
}
#endif

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

NTSTATUS CIOAccessDevice::deviceIOControl(PIRP irp)
{
    NTSTATUS ntStatus;
    DWORD    dwBytesWritten;

    switch ( irp->dwIoControlCode )
    {
    case DIOC_OPEN:
        debugOut(dbTrace,"DIOC_OPEN");
        ntStatus = STATUS_SUCCESS;
        break;

    case DIOC_CLOSEHANDLE:
        debugOut(dbTrace,"DIOC_CLOSEHANDLE");
        ntStatus = STATUS_SUCCESS;
        break;

    default:
        ntStatus = deviceControl(irp->dwIoControlCode,
                                (PDSDrvParam) irp->lpvInBuffer,
                                (DWORD*) irp->lpvOutBuffer,
                                &dwBytesWritten);
        break;
    }

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
// 2000-09-11 Added by Mark Rejhon 
// Eliminates compiler warnings about data type conversion
#define PORTADDRTYPE  USHORT

    NTSTATUS status;

    status = STATUS_SUCCESS;
    *pBytesWritten = 0;

    switch ( ioControlCode )
    {
    case ioctlReadBYTE:
        osPortReadByte((PORTADDRTYPE)ioParam->dwAddress, *outputBuffer);
        *pBytesWritten = 1;
        break;

    case ioctlReadWORD:
        osPortReadWord((PORTADDRTYPE)ioParam->dwAddress, *outputBuffer);
        *pBytesWritten = 2;
        break;

    case ioctlReadDWORD:
        osPortReadLong((PORTADDRTYPE)ioParam->dwAddress, *outputBuffer);
        *pBytesWritten = 4;
        break;

    case ioctlWriteBYTE:
        osPortWriteByte((PORTADDRTYPE)ioParam->dwAddress, ioParam->dwValue);
        break;

    case ioctlWriteWORD:
        osPortWriteWord((PORTADDRTYPE)ioParam->dwAddress, (USHORT) ioParam->dwValue);
        break;

    case ioctlWriteDWORD:
        osPortWriteLong((PORTADDRTYPE)ioParam->dwAddress, ioParam->dwValue);
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
        break;
    }
    return status;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int CIOAccessDevice::isValidAddress(void * pvAddress)
{
    return 1;
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

    if(dwFlags & ALLOC_MEMORY_CONTIG)
    {
        DWORD nPages = (dwLength + 4093) >> 12;
        DWORD PhysAddress = NULL;
        node->dwSystemAddress =(DWORD)_PageAllocate(nPages, 
                                                    PG_SYS,
                                                    0,
                                                    0,
                                                    0,
                                                    0x100000,
                                                    (void**)&PhysAddress,
                                                    PAGECONTIG | PAGEFIXED | PAGEUSEALIGN);
        if(node->dwSystemAddress == 0)
        {
            debugOut(dbTrace,"! cannot alloc contig pages");
            return  STATUS_INSUFFICIENT_RESOURCES;
        }
        
        node->dwUserAddress = node->dwSystemAddress;
        node->dwFlags = dwFlags;
        node->dwPages = nPages;
        
        pMemStruct->dwTotalSize = dwLength;
        pMemStruct->dwHandle = (DWORD)node;
        pMemStruct->dwPages = 1;
        pMemStruct->dwUser = (void*)node->dwUserAddress;

        pPages[0].dwSize = dwLength;
        pPages[0].dwPhysical = PhysAddress;
    }
    else
    {
        DWORD LinOffset = dwUserAddress & 0xfff; // page offset of memory to map
        DWORD nPages;

        // Calculate # of pages to lock
        nPages = ((dwUserAddress + dwLength) >> 12) - (dwUserAddress >> 12) + 1;

        // lock the pages
        if(_LinPageLock(dwUserAddress >> 12, nPages, 0) == 0)
        {
            debugOut(dbTrace,"! cannot lock memory");
            return  STATUS_INSUFFICIENT_RESOURCES;
        }

        // work out the physical addresses for each distinct page in the
        // input buffer
        DWORD SizeUsed;
        pPages[0].dwPhysical = GetPhysAddr(dwUserAddress); 
        if(pPages[0].dwPhysical == 0xFFFFFFFF)
        {
            debugOut(dbTrace,"! cannot get Physical Address");
            _LinPageUnLock(dwUserAddress >> 12, nPages, 0);
            return  STATUS_INSUFFICIENT_RESOURCES;
        }
        pPages[0].dwSize = 4096 - LinOffset; 
        SizeUsed = pPages[0].dwSize;
        for(DWORD i = 1; i < nPages; i++)
        {
            pPages[i].dwPhysical = GetPhysAddr(dwUserAddress + SizeUsed);
            if(pPages[i].dwPhysical == 0xFFFFFFFF)
            {
                debugOut(dbTrace,"! cannot get Physical Address");
                _LinPageUnLock(dwUserAddress >> 12, nPages, 0);
                return  STATUS_INSUFFICIENT_RESOURCES;
            }
            if((dwLength - SizeUsed) > 4096)
            {
                pPages[i].dwSize = 4096;
                SizeUsed += 4096;
            }
            else
            {
                pPages[i].dwSize = (dwLength - SizeUsed);
            }
        }
        
        node->dwFlags = dwFlags;
        node->dwPages = nPages;
        node->dwSystemAddress = dwUserAddress;
        node->dwUserAddress = dwUserAddress;

        pMemStruct->dwTotalSize = dwLength;
        pMemStruct->dwHandle = (DWORD)node;
        pMemStruct->dwPages = nPages;
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
    if(node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        if(_PageFree((void*)node->dwSystemAddress, 0) == 0)
        {
            debugOut(dbError," ! _PageFree failed");
        }
    }
    else
    {
        if(_LinPageUnLock(node->dwSystemAddress >> 12, node->dwPages, 0) == 0)
        {
            debugOut(dbError," ! _LinPageUnLock failed");
        }
    }
    memset(node, 0, sizeof(TMemoryNode));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CIOAccessDevice::GetPhysAddr(DWORD UserAddr)
{
    DWORD PhysAddr;

    if(_CopyPageTable(UserAddr >> 12, 1, &PhysAddr, 0) == 0)
    {
        return 0xFFFFFFFF;
    }
    return (PhysAddr & 0xFFFFF000) | (UserAddr & 0x00000FFF);
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

        dwMemoryBase = (DWORD) _MapPhysToLinear( (void *)dwPhysicalAddress, dwLength, 0);
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
        dwMemoryBase = 0;
    }
}
