/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.cpp,v 1.7 2001-11-02 16:36:54 adcockj Exp $
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
// Revision 1.4.2.1  2001/08/15 08:54:28  adcockj
// Tidy up driver code
//
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
    case ioctlReadPortBYTE:
        {
            UCHAR* pByte = (UCHAR*)outputBuffer;
            *pByte = _inp((PORTADDRTYPE)ioParam->dwAddress);
            *pBytesWritten = 1;
        }
        break;

    case ioctlReadPortWORD:
        {
            USHORT* pWord = (USHORT*)outputBuffer;
            *pWord = _inpw((PORTADDRTYPE)ioParam->dwAddress);
            *pBytesWritten = 2;
        }
        break;

    case ioctlReadPortDWORD:
        {
            ULONG* pDword = (ULONG*)outputBuffer;
            *pDword = _inpd((PORTADDRTYPE)ioParam->dwAddress);
            *pBytesWritten = 4;
        }
        break;

    case ioctlWritePortBYTE:
        _outp((PORTADDRTYPE)ioParam->dwAddress, (UCHAR)ioParam->dwValue);
        break;

    case ioctlWritePortWORD:
        _outpw((PORTADDRTYPE)ioParam->dwAddress, (USHORT)ioParam->dwValue);
        break;

    case ioctlWritePortDWORD:
        _outpd((PORTADDRTYPE)ioParam->dwAddress, ioParam->dwValue);
        break;

    case ioctlGetPCIInfo:
        if (isValidAddress(outputBuffer))
        {
            TPCICARDINFO* pPCICardInfo = (TPCICARDINFO*)outputBuffer;
            status = pciFindDevice(
                                       ioParam->dwAddress,
                                       ioParam->dwValue,
                                       ioParam->dwFlags,
                                       &(pPCICardInfo->dwBusNumber),
                                       &(pPCICardInfo->dwSlotNumber)
                                  );

            if ( status == STATUS_SUCCESS)
            {
                status = pciGetDeviceConfig(pPCICardInfo);
            }
            else
            {
                debugOut(dbTrace,"pci device for vendor %lX deviceID %lX not found",ioParam->dwAddress,ioParam->dwValue);
            }
            *pBytesWritten = sizeof(TPCICARDINFO);
        }
        else
        {
            debugOut(dbError,"! invalid system address %X",outputBuffer);
        }
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
        *outputBuffer = mapMemory(ioParam->dwValue, ioParam->dwFlags);
        *pBytesWritten = 4;
        break;

    case ioctlUnmapMemory:
        unmapMemory();
        break;

    case ioctlReadMemoryDWORD:
        if (ioParam->dwAddress)
        {
            ULONG* Address = (ULONG*)ioParam->dwAddress;
            *outputBuffer = *Address;
            *pBytesWritten = 4;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *outputBuffer);
        }
        break;

    case ioctlWriteMemoryDWORD:
        if (ioParam->dwAddress)
        {
            ULONG* Address = (ULONG*)ioParam->dwAddress;
            *Address = ioParam->dwValue;
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case ioctlReadMemoryWORD:
        if (ioParam->dwAddress)
        {
            USHORT* Address = (USHORT*)ioParam->dwAddress;
            USHORT* pWord = (USHORT*)outputBuffer;
            *pWord = *Address;
            *pBytesWritten = 2;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *pWord);
        }
        break;

    case ioctlWriteMemoryWORD:
        if (ioParam->dwAddress)
        {
            USHORT* Address = (USHORT*)ioParam->dwAddress;
            *Address = (USHORT)ioParam->dwValue;
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case ioctlReadMemoryBYTE:
        if (ioParam->dwAddress)
        {
            UCHAR* Address = (UCHAR*)ioParam->dwAddress;
            UCHAR* pByte = (UCHAR*)outputBuffer;
            *pByte = *Address;
            *pBytesWritten = 1;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *pByte);
        }
        break;

    case ioctlWriteMemoryBYTE:
        if (ioParam->dwAddress)
        {
            UCHAR* Address = (UCHAR*)ioParam->dwAddress;
            *Address = (UCHAR)ioParam->dwValue;
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case ioctlGetVersion:
        *outputBuffer = DSDRV_VERSION;
        *pBytesWritten = sizeof(DWORD);
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
    return (DWORD) _MapPhysToLinear( (void *)dwPhysicalAddress, dwLength, 0);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void
CIOAccessDevice::unmapMemory(void)
{
    ; // do nothing
}
