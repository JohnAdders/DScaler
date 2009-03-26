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

    NTSTATUS Status(STATUS_SUCCESS);
    *pBytesWritten = 0;

    switch(ioControlCode)
    {
    case IOCTL_DEPRICATED_READPORTBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READPORTBYTE:
        {
            UCHAR* pByte = (UCHAR*)outputBuffer;
            *pByte = _inp((PORTADDRTYPE)ioParam->dwAddress);
            *pBytesWritten = 1;
        }
        break;

    case IOCTL_DEPRICATED_READPORTWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READPORTWORD:
        {
            USHORT* pWord = (USHORT*)outputBuffer;
            *pWord = _inpw((PORTADDRTYPE)ioParam->dwAddress);
            *pBytesWritten = 2;
        }
        break;

    case IOCTL_DEPRICATED_READPORTDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READPORTDWORD:
        {
            ULONG* pDword = (ULONG*)outputBuffer;
            *pDword = _inpd((PORTADDRTYPE)ioParam->dwAddress);
            *pBytesWritten = 4;
        }
        break;

    case IOCTL_DEPRICATED_WRITEPORTBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTBYTE:
        _outp((PORTADDRTYPE)ioParam->dwAddress, (UCHAR)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_WRITEPORTWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTWORD:
        _outpw((PORTADDRTYPE)ioParam->dwAddress, (USHORT)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_WRITEPORTDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTDWORD:
        _outpd((PORTADDRTYPE)ioParam->dwAddress, (ULONG)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_GETPCIINFO:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_GETPCIINFO:
        if (isValidAddress(outputBuffer))
        {
            TPCICARDINFO* pPCICardInfo = (TPCICARDINFO*)outputBuffer;
            Status = pciFindDevice(
                                       (ULONG)ioParam->dwAddress,
                                       (ULONG)ioParam->dwValue,
                                       ioParam->dwFlags,
                                       &(pPCICardInfo->dwBusNumber),
                                       &(pPCICardInfo->dwSlotNumber)
                                  );

            if (Status == STATUS_SUCCESS)
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
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_ALLOCMEMORY:
        {
            PMemStruct pMem = (PMemStruct)outputBuffer;
            Status = allocMemory((ULONG)ioParam->dwValue, ioParam->dwFlags, (ULONG)ioParam->dwAddress, pMem, false);
            *pBytesWritten = sizeof(TMemStruct) + pMem->dwPages * sizeof(TPageStruct);
        }
        break;

    case IOCTL_DSDRV_ALLOCMEMORY64:
        {
            PMemStruct pMem = (PMemStruct)outputBuffer;
            Status = allocMemory((ULONG)ioParam->dwValue, ioParam->dwFlags, (ULONG)ioParam->dwAddress, pMem, true);
            *pBytesWritten = sizeof(TMemStruct) + pMem->dwPages * sizeof(TPageStruct64);
        }
        break;

    case IOCTL_DEPRICATED_FREEMEMORY:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
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
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_MAPMEMORY:
        *outputBuffer = mapMemory((ULONG)ioParam->dwValue, ioParam->dwFlags);
        *pBytesWritten = 4;
        break;

    case IOCTL_DEPRICATED_UNMAPMEMORY:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_UNMAPMEMORY:
        unmapMemory();
        break;

    case IOCTL_DEPRICATED_READMEMORYDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYDWORD:
        if (ioParam->dwAddress)
        {
            ULONG* Address = (ULONG*)ioParam->dwAddress;
            *outputBuffer = *Address;
            *pBytesWritten = 4;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYDWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYDWORD:
        if (ioParam->dwAddress)
        {
            ULONG* Address = (ULONG*)ioParam->dwAddress;
            *Address = (ULONG)ioParam->dwValue;
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_READMEMORYWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYWORD:
        if (ioParam->dwAddress)
        {
            USHORT* Address = (USHORT*)ioParam->dwAddress;
            USHORT* pWord = (USHORT*)outputBuffer;
            *pWord = *Address;
            *pBytesWritten = 2;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *pWord);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYWORD:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYWORD:
        if (ioParam->dwAddress)
        {
            USHORT* Address = (USHORT*)ioParam->dwAddress;
            *Address = (USHORT)ioParam->dwValue;
            debugOut(dbTrace,"memory %X write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_READMEMORYBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYBYTE:
        if (ioParam->dwAddress)
        {
            UCHAR* Address = (UCHAR*)ioParam->dwAddress;
            UCHAR* pByte = (UCHAR*)outputBuffer;
            *pByte = *Address;
            *pBytesWritten = 1;
            debugOut(dbTrace,"memory %X read %X",ioParam->dwAddress, *pByte);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYBYTE:
        if(m_AllowDepricatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYBYTE:
        if (ioParam->dwAddress)
        {
            UCHAR* Address = (UCHAR*)ioParam->dwAddress;
            *Address = (UCHAR)ioParam->dwValue;
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
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_GETPCICONFIG:
        if (isValidAddress(outputBuffer))
        {
            PCI_COMMON_CONFIG *pPCIConfig = (PCI_COMMON_CONFIG*)outputBuffer;

            Status = pciGetDeviceConfig(pPCIConfig, (ULONG)ioParam->dwAddress, (ULONG)ioParam->dwValue);

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
            return STATUS_SUCCESS;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_SETPCICONFIG:
        if (isValidAddress(outputBuffer))
        {
            PCI_COMMON_CONFIG *pPCIConfig = (PCI_COMMON_CONFIG*)outputBuffer;

            Status = pciSetDeviceConfig(pPCIConfig, (ULONG)ioParam->dwAddress, (ULONG)ioParam->dwValue);

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

            Status = pciGetDeviceConfigOffset(pPCIConfig, ioParam->dwFlags, (ULONG)ioParam->dwAddress, (ULONG)ioParam->dwValue);

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

            Status = pciSetDeviceConfigOffset(pPCIConfig, ioParam->dwFlags, (ULONG)ioParam->dwAddress, (ULONG)ioParam->dwValue);

            *pBytesWritten = 1;
        }
        else
        {
            debugOut(dbError,"! invalid system address %X",outputBuffer);
        }
        break;

    default:
        debugOut(dbError,"unknown command %lX",ioControlCode);
        break;
    }
    return Status;
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
NTSTATUS CIOAccessDevice::allocMemory(DWORD dwLength, DWORD dwFlags, DWORD dwUserAddress, PMemStruct pMemStruct, bool is64)
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

        if (is64)
            ntStatus = buildPageStruct64(pMemStruct, node, PhysAddress);
        else
            ntStatus = buildPageStruct32(pMemStruct, node, PhysAddress);
    }
    else
    {
        DWORD nPages;

        // Calculate # of pages to lock
        nPages = ((dwUserAddress + dwLength) >> 12) - (dwUserAddress >> 12) + 1;

        // lock the pages
        if(_LinPageLock(dwUserAddress >> 12, nPages, 0) == 0)
        {
            debugOut(dbTrace,"! cannot lock memory");
            return  STATUS_INSUFFICIENT_RESOURCES;
        }

        node->dwFlags = dwFlags;
        node->dwPages = nPages;
        node->dwSystemAddress = dwUserAddress;
        node->dwUserAddress = dwUserAddress;

        pMemStruct->dwTotalSize = dwLength;
        pMemStruct->dwHandle = (DWORD)node;
        pMemStruct->dwPages = nPages;

        if (is64)
            ntStatus = buildPageStruct64(pMemStruct, node, 0);
        else
            ntStatus = buildPageStruct32(pMemStruct, node, 0);
    }
    return ntStatus;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::buildPageStruct32(PMemStruct pMemStruct, PMemoryNode node, DWORD phys)
{
    PPageStruct pPages = (PPageStruct)(pMemStruct + 1);

    if (node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        pPages[0].dwSize = pMemStruct->dwTotalSize;
        pPages[0].dwPhysical = phys;
    }
    else
    {
        // work out the physical addresses for each distinct page in the
        // input buffer
        DWORD LinOffset = node->dwUserAddress & 0xfff; // page offset of memory to map
        DWORD SizeUsed = 0;
        pPages[0].dwPhysical = GetPhysAddr(node->dwUserAddress);
        if(pPages[0].dwPhysical == 0xFFFFFFFF)
        {
            debugOut(dbTrace,"! cannot get Physical Address");
            _LinPageUnLock(node->dwUserAddress >> 12, node->dwPages, 0);
            node->dwSystemAddress = 0;
            return  STATUS_INSUFFICIENT_RESOURCES;
        }
        pPages[0].dwSize = 4096 - LinOffset;
        SizeUsed = pPages[0].dwSize;
        for(DWORD i = 1; i < node->dwPages; i++)
        {
            pPages[i].dwPhysical = GetPhysAddr(node->dwUserAddress + SizeUsed);
            if(pPages[i].dwPhysical == 0xFFFFFFFF)
            {
                debugOut(dbTrace,"! cannot get Physical Address");
                _LinPageUnLock(node->dwUserAddress >> 12, node->dwPages, 0);
                node->dwSystemAddress = 0;
                return  STATUS_INSUFFICIENT_RESOURCES;
            }
            if((pMemStruct->dwTotalSize - SizeUsed) > 4096)
            {
                pPages[i].dwSize = 4096;
                SizeUsed += 4096;
            }
            else
            {
                pPages[i].dwSize = (pMemStruct->dwTotalSize - SizeUsed);
            }
        }
    }
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::buildPageStruct64(PMemStruct pMemStruct, PMemoryNode node, DWORD phys)
{
    PPageStruct64 pPages = (PPageStruct64)(pMemStruct + 1);

    if (node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        pPages[0].dwSize = pMemStruct->dwTotalSize;
        pPages[0].llPhysical = phys;
    }
    else
    {
        // work out the physical addresses for each distinct page in the
        // input buffer
        DWORD LinOffset = node->dwUserAddress & 0xfff; // page offset of memory to map
        DWORD SizeUsed = 0;
        pPages[0].llPhysical = GetPhysAddr(node->dwUserAddress);
        if(pPages[0].llPhysical == 0xFFFFFFFFU)
        {
            debugOut(dbTrace,"! cannot get Physical Address");
            _LinPageUnLock(node->dwUserAddress >> 12, node->dwPages, 0);
            node->dwSystemAddress = 0;
            return  STATUS_INSUFFICIENT_RESOURCES;
        }
        pPages[0].dwSize = 4096 - LinOffset;
        SizeUsed = pPages[0].dwSize;
        for(DWORD i = 1; i < node->dwPages; i++)
        {
            pPages[i].llPhysical = GetPhysAddr(node->dwUserAddress + SizeUsed);
            if(pPages[i].llPhysical == 0xFFFFFFFFU)
            {
                debugOut(dbTrace,"! cannot get Physical Address");
                _LinPageUnLock(node->dwUserAddress >> 12, node->dwPages, 0);
                node->dwSystemAddress = 0;
                return  STATUS_INSUFFICIENT_RESOURCES;
            }
            if((pMemStruct->dwTotalSize - SizeUsed) > 4096)
            {
                pPages[i].dwSize = 4096;
                SizeUsed += 4096;
    }
    else
    {
                pPages[i].dwSize = (pMemStruct->dwTotalSize - SizeUsed);
            }
        }
    }
    return STATUS_SUCCESS;
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
