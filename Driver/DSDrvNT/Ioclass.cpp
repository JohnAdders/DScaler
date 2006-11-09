/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.cpp,v 1.19 2006-11-09 21:18:09 adcockj Exp $
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
// 13 Mar 2006   Michael Lutz          Modifications for Win64
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.18  2006/11/08 09:50:30  adcockj
// fix for i386 driver issues from Michael Lutz
//
// Revision 1.17  2006/10/31 13:42:09  to_see
// Added Michael's Patch (error on i386 platforms)
//
// Revision 1.16  2006/03/18 13:28:45  adcockj
// fixed compile issues with vs 6
//
// Revision 1.15  2006/03/16 17:20:56  adcockj
// Added Michael Lutz's 64 bit code
//
// Revision 1.14  2005/05/13 10:12:46  adcockj
// fixed uninitialised variable
//
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
#include "DSDrvNt.h"

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CIOAccessDevice::CIOAccessDevice(void) :
    m_AllowDeprecatedIOCTLs(false)
{
    memset(&memoryList, 0, sizeof(memoryList));
    memset(&mappingList, 0, sizeof(mappingList));
    
#ifndef _WIN64
    // only on win32
    m_PAEEnabled = ExIsProcessorFeaturePresent(PF_PAE_ENABLED) == TRUE;
    debugOut(dbTrace, m_PAEEnabled ? "PAE enabled" : "PAE not enabled");
#endif
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

        if(node->pSystemAddress)
        {
            freeMemory (node);
        }
    }
    
    //
    // Remove mappings
    //
    
    for (Index = 0; Index < MAX_FREE_MAPPING_NODES; Index++)
    {
        if ( mappingList[Index].pUser )
            unmapMemory(mappingList[Index].pUser, mappingList[Index].ulLength);
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
    TDSDrvParam             ioDrvParam;
    bool                    is32 = false;

#ifdef _WIN64
    is32 = IoIs32bitProcess( irp ) == TRUE;
#endif
    
    RtlZeroMemory( &ioDrvParam, sizeof(TDSDrvParam) );
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
            inputBuffer       = irp->AssociatedIrp.SystemBuffer;
        }

        inputBufferLength   = irpStack->Parameters.DeviceIoControl.InputBufferLength;
        outputBuffer        = irp->AssociatedIrp.SystemBuffer;
        outputBufferLength  = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
        break;

    case METHOD_OUT_DIRECT:
        if ( irp->MdlAddress )
        {
            outputBuffer      = MmGetSystemAddressForMdl( irp->MdlAddress );
            if (!outputBuffer)
                debugOut(dbError, "No address for MDL (io=%X)", ioControlCode);
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
    
    if ( ioControlCode == IOCTL_DSDRV_FREEMEMORY )
        // inputBuffer points to a PMemStruct
        ntStatus = deviceControl(ioControlCode,
                                (PDSDrvParam) inputBuffer,
                                (DWORD*) outputBuffer, outputBufferLength,
                                &irp->IoStatus.Information, is32);
    else
    {
#ifdef _WIN64
        if ( is32 )
        {
            debugOut(dbTrace, "thunking needed: size=%d", inputBufferLength);
            PDSDrvParam_32 in32 = (PDSDrvParam_32)inputBuffer;
            if ( inputBufferLength >= sizeof(DWORD) )
                ioDrvParam.dwAddress = (DWORD_PTR)(in32->dwAddress);
            if ( inputBufferLength >= (sizeof(DWORD) + FIELD_OFFSET(TDSDrvParam_32, dwValue)) )
                ioDrvParam.dwValue = (DWORD_PTR)(in32->dwValue);
            if ( inputBufferLength >= (sizeof(DWORD) + FIELD_OFFSET(TDSDrvParam_32, dwFlags)) )
                ioDrvParam.dwFlags = in32->dwFlags;
        }
        else
#endif
            if ( inputBufferLength > 0 )
                RtlCopyMemory( &ioDrvParam, inputBuffer,
                    inputBufferLength > sizeof(TDSDrvParam) ? sizeof(TDSDrvParam) : inputBufferLength );
                
        ntStatus = deviceControl(ioControlCode,
                                &ioDrvParam,
                                (DWORD*) outputBuffer, outputBufferLength,
                                &irp->IoStatus.Information, is32);
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
NTSTATUS CIOAccessDevice::deviceControl(DWORD ioControlCode, PDSDrvParam ioParam, DWORD* outputBuffer, DWORD outLen, DWORD_PTR* pBytesWritten, bool is32)
{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;
    *pBytesWritten = 0;

    switch ( ioControlCode )
    {
    case IOCTL_DEPRICATED_READPORTBYTE:
        if(m_AllowDeprecatedIOCTLs == false)
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
        if(m_AllowDeprecatedIOCTLs == false)
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
        if(m_AllowDeprecatedIOCTLs == false)
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
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTBYTE:
        WRITE_PORT_UCHAR((PUCHAR)&ioParam->dwAddress, (UCHAR)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_WRITEPORTWORD:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTWORD:
        WRITE_PORT_USHORT((PUSHORT)&ioParam->dwAddress, (USHORT)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_WRITEPORTDWORD:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEPORTDWORD:
        WRITE_PORT_ULONG((PULONG)&ioParam->dwAddress, (ULONG)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_GETPCIINFO:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_GETPCIINFO:
        if (isValidAddress(outputBuffer))
        {
            TPCICARDINFO tPCICardInfo;
            Status = pciFindDevice(
                                       (DWORD)ioParam->dwAddress,
                                       (DWORD)ioParam->dwValue,
                                       ioParam->dwFlags,
                                       &(tPCICardInfo.dwBusNumber),
                                       &(tPCICardInfo.dwSlotNumber)
                                  );

            if (Status == STATUS_SUCCESS)
            {
                Status = pciGetDeviceInfo(&tPCICardInfo);
                if (Status == STATUS_SUCCESS)
                {
#ifdef _WIN64
                    if ( is32 )
                    {
                        TPCICARDINFO_32 *pPCI32 = (TPCICARDINFO_32*)outputBuffer;
                        if ((UINT_PTR)tPCICardInfo.dwMemoryAddress > 0xFFFFFFFFU)
                        {
                            // Too bad, not in good address space
                            debugOut(dbError, "ioctlGetPCIInfo: hardware address not in range (0x%IX)", tPCICardInfo.dwMemoryAddress);
                            return STATUS_CONFLICTING_ADDRESSES;
                        }
                        pPCI32->dwMemoryAddress = (DWORD)tPCICardInfo.dwMemoryAddress;
                        pPCI32->dwMemoryLength = tPCICardInfo.dwMemoryLength;
                        pPCI32->dwSubSystemId = tPCICardInfo.dwSubSystemId;
                        pPCI32->dwBusNumber = tPCICardInfo.dwBusNumber;
                        pPCI32->dwSlotNumber = tPCICardInfo.dwSlotNumber;
                        *pBytesWritten = sizeof(TPCICARDINFO_32);
                    }
                    else
#endif
                    {
                        RtlCopyMemory(outputBuffer, &tPCICardInfo, sizeof(TPCICARDINFO));
                        *pBytesWritten = sizeof(TPCICARDINFO);
                    }
                }
            }
            else
            {
                debugOut(dbTrace,"ioctlGetPCIInfo: pci device for vendor %lX deviceID %lX not found", (DWORD)ioParam->dwAddress, (DWORD)ioParam->dwValue);
            }
        }
        else
        {
            debugOut(dbError,"! invalid system address %IX",outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_ALLOCMEMORY:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_ALLOCMEMORY:
        {
#ifdef _WIN64
            if (is32)
            {
                PMemStruct_32 pMem32 = (PMemStruct_32)outputBuffer;
                char *outBuf = new char[outLen+sizeof(DWORD)];
                if (!outBuf)
                {
                    debugOut(dbError, "allocMemory: not enough memory for buffer");
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                PMemStruct pMem = (PMemStruct)outBuf;
                Status = allocMemory((ULONG)ioParam->dwValue, ioParam->dwFlags, (PVOID)ioParam->dwAddress, pMem, false);
                if (Status == STATUS_SUCCESS)
                {
                    // thunk to 32-bit structure
                    pMem32->dwTotalSize = pMem->dwTotalSize;
                    pMem32->dwPages = pMem->dwPages;
                    pMem32->dwHandle = pMem->dwHandle;
                    pMem32->dwFlags = pMem->dwFlags;
                    if ((UINT_PTR)pMem->dwUser > 0xFFFFFFFFU)
                    {
                        // Too bad, not in address space range
                        debugOut(dbError, "allocMemory() returned 64-bit address: 0x%IX. User address was 0x%IX", pMem->dwUser, ioParam->dwAddress);
                        delete[] outBuf;
                        return STATUS_CONFLICTING_ADDRESSES;
                    }
                    pMem32->dwUser = PtrToUint(pMem->dwUser);
                    // copy page structs
                    RtlCopyMemory(pMem32+1, pMem+1, sizeof(TPageStruct) * pMem32->dwPages);
                    
                    *pBytesWritten = sizeof(TMemStruct_32) + pMem32->dwPages * sizeof(TPageStruct);
                }
                delete[] outBuf;
            }
            else
#endif
            {
                Status = allocMemory((ULONG)ioParam->dwValue, ioParam->dwFlags, (PVOID)ioParam->dwAddress, (PMemStruct)outputBuffer, false);
                *pBytesWritten = sizeof(TMemStruct) + ((PMemStruct)outputBuffer)->dwPages * sizeof(TPageStruct);
            }
            
        }
        break;

    case IOCTL_DSDRV_ALLOCMEMORY64:
        // allocate memory above 4GB
        // almost a copy of above, just with TPageStruct64s
        {
#ifdef _WIN64
            if (is32)
            {
                PMemStruct_32 pMem32 = (PMemStruct_32)outputBuffer;
                char *outBuf = new char[outLen+sizeof(DWORD)];
                if (!outBuf)
                {
                    debugOut(dbError, "allocMemory: not enough memory for buffer");
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                PMemStruct pMem = (PMemStruct)outBuf;
                Status = allocMemory((ULONG)ioParam->dwValue, ioParam->dwFlags, (PVOID)ioParam->dwAddress, pMem, true);
                if (Status == STATUS_SUCCESS)
                {
                    // thunk to 32-bit structure
                    pMem32->dwTotalSize = pMem->dwTotalSize;
                    pMem32->dwPages = pMem->dwPages;
                    pMem32->dwHandle = pMem->dwHandle;
                    pMem32->dwFlags = pMem->dwFlags;
                    if ((UINT_PTR)pMem->dwUser > 0xFFFFFFFFU)
                    {
                        // Too bad, not in address space range
                        debugOut(dbError, "allocMemory() returned 64-bit address: 0x%IX. User address was 0x%IX", pMem->dwUser, ioParam->dwAddress);
                        delete[] outBuf;
                        return STATUS_CONFLICTING_ADDRESSES;
                    }
                    pMem32->dwUser = PtrToUint(pMem->dwUser);
                    // copy page structs
                    RtlCopyMemory(pMem32+1, pMem+1, sizeof(TPageStruct64) * pMem32->dwPages);
                    
                    *pBytesWritten = sizeof(TMemStruct_32) + pMem32->dwPages * sizeof(TPageStruct64);
                }
                delete[] outBuf;
            }
            else
#endif
            {
                Status = allocMemory((ULONG)ioParam->dwValue, ioParam->dwFlags, (PVOID)ioParam->dwAddress, (PMemStruct)outputBuffer, true);
                *pBytesWritten = sizeof(TMemStruct) + ((PMemStruct)outputBuffer)->dwPages * sizeof(TPageStruct64);
            }
            
        }
        break;

    case IOCTL_DEPRICATED_FREEMEMORY:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_FREEMEMORY:
        {
#ifdef _WIN64
            if (is32)
            {
                PMemStruct_32   pMem32 = (PMemStruct_32)ioParam;
                TMemStruct      tMem;
                tMem.dwTotalSize = pMem32->dwTotalSize;
                tMem.dwPages = pMem32->dwPages;
                tMem.dwHandle = pMem32->dwHandle;
                tMem.dwFlags = pMem32->dwFlags;
                tMem.dwUser = UintToPtr(pMem32->dwUser);
                Status = freeMemory(&tMem);
            }
            else
#endif
            {
                PMemStruct pMem = (PMemStruct)ioParam;
                Status = freeMemory(pMem);
            }
        }
        break;

    case IOCTL_DEPRICATED_MAPMEMORY:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_MAPMEMORY:
#ifdef _WIN64
        if (is32)
        {
            PVOID   out = NULL;
            Status = mapMemory((DWORD)ioParam->dwAddress, ioParam->dwValue, ioParam->dwFlags, &out);
            if ( (UINT_PTR)out > 0xFFFFFFFFU ) {
                debugOut(dbError, "mapMemory() returned 64-bit address: 0x%IX. Input was 0x%IX", out, ioParam->dwValue);
                return STATUS_CONFLICTING_ADDRESSES;
            }
            *(DWORD*)outputBuffer = PtrToUint(out);
            *pBytesWritten = sizeof(DWORD);
            debugOut(dbTrace, "mapMemory() returned user address 0x%X", *(DWORD*)outputBuffer);
        }
        else
#endif
        {
            Status = mapMemory((DWORD)ioParam->dwAddress, ioParam->dwValue, ioParam->dwFlags, (PVOID*)outputBuffer);
            *pBytesWritten = sizeof(PVOID);
            debugOut(dbTrace, "mapMemory() returned user address 0x%IX", *(DWORD_PTR*)outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_UNMAPMEMORY:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_UNMAPMEMORY:
        unmapMemory((PVOID)ioParam->dwAddress, (ULONG)ioParam->dwValue);
        break;

    case IOCTL_DEPRICATED_READMEMORYDWORD:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYDWORD:
        if (ioParam->dwAddress)
        {
            *outputBuffer = READ_REGISTER_ULONG((PULONG)(ioParam->dwAddress));
            *pBytesWritten = 4;
            debugOut(dbTrace,"memory %IX read %X",ioParam->dwAddress, *outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYDWORD:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYDWORD:
        if (ioParam->dwAddress)
        {
            WRITE_REGISTER_ULONG((PULONG)ioParam->dwAddress, (ULONG)ioParam->dwValue);
            debugOut(dbTrace,"memory %IX write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_READMEMORYWORD:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYWORD:
        if (ioParam->dwAddress)
        {
            USHORT* pWord = (USHORT*)outputBuffer;
            *pWord = READ_REGISTER_USHORT((PUSHORT)(ioParam->dwAddress));
            *pBytesWritten = 2;
            debugOut(dbTrace,"memory %IX read %lX",ioParam->dwAddress, *pWord);
        }
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYWORD:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYWORD:
        if (ioParam->dwAddress)
        {
            WRITE_REGISTER_USHORT((PUSHORT)ioParam->dwAddress, (USHORT)ioParam->dwValue);
            debugOut(dbTrace,"memory %IX write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_READMEMORYBYTE:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_READMEMORYBYTE:
        if (ioParam->dwAddress)
        {
            UCHAR* pByte = (UCHAR*)outputBuffer;
            *pByte = READ_REGISTER_UCHAR((PUCHAR)(ioParam->dwAddress));
            *pBytesWritten = 1;
            debugOut(dbTrace, "memory %IX read %lX (to 0x%IX)", ioParam->dwAddress, *pByte, outputBuffer);
        } else
            debugOut(dbError, " ! memory read no address");
        break;

    case IOCTL_DEPRICATED_WRITEMEMORYBYTE:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_WRITEMEMORYBYTE:
        if (ioParam->dwAddress)
        {
            WRITE_REGISTER_UCHAR((PUCHAR)ioParam->dwAddress, (UCHAR)ioParam->dwValue);
            debugOut(dbTrace,"memory %IX write %X",ioParam->dwAddress, ioParam->dwValue);
        }
        break;

    case IOCTL_DEPRICATED_GETVERSION:
        // if we get called on this IOCTL
        // we must be being called by an older version
        // of dscaler so switch on the potentially unsafe
        // IOCTLS that we depricated because of problems in XP
        *outputBuffer = DSDRV_VERSION;
        *pBytesWritten = sizeof(DWORD);
        m_AllowDeprecatedIOCTLs = true;
        break;

    case IOCTL_DSDRV_GETVERSION:
        *outputBuffer = DSDRV_VERSION;
        *pBytesWritten = sizeof(DWORD);
        break;

    case IOCTL_DEPRICATED_GETPCICONFIG:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_GETPCICONFIG:
        if (isValidAddress(outputBuffer))
        {
            PCI_COMMON_CONFIG *pPCIConfig = (PCI_COMMON_CONFIG*)outputBuffer;

            Status = pciGetDeviceConfig(pPCIConfig, (DWORD)ioParam->dwAddress, (DWORD)ioParam->dwValue);

            *pBytesWritten = sizeof(PCI_COMMON_CONFIG);
        }
        else
        {
            debugOut(dbError,"! invalid system address %IX",outputBuffer);
        }
        break;

    case IOCTL_DEPRICATED_SETPCICONFIG:
        if(m_AllowDeprecatedIOCTLs == false)
        {
            debugOut(dbError,"Called on Deprecated Interface %lX",ioControlCode);
            return STATUS_INVALID_PARAMETER;
        }
        // ... deliberate drop through ...
    case IOCTL_DSDRV_SETPCICONFIG:
        if (isValidAddress(outputBuffer))
        {
            PCI_COMMON_CONFIG *pPCIConfig = (PCI_COMMON_CONFIG*)outputBuffer;

            Status = pciSetDeviceConfig(pPCIConfig, (DWORD)ioParam->dwAddress, (DWORD)ioParam->dwValue);

            *pBytesWritten = sizeof(PCI_COMMON_CONFIG);
        }
        else
        {
            debugOut(dbError,"! invalid system address %IX",outputBuffer);
        }
        break;

    case IOCTL_DSDRV_GETPCICONFIGOFFSET:
        if (isValidAddress(outputBuffer))
        {
            BYTE *pPCIConfig = (BYTE*)outputBuffer;

            Status = pciGetDeviceConfigOffset(pPCIConfig, ioParam->dwFlags, (DWORD)ioParam->dwAddress, (DWORD)ioParam->dwValue);

            *pBytesWritten = 1;
        }
        else
        {
            debugOut(dbError,"! invalid system address %IX",outputBuffer);
        }
        break;

    case IOCTL_DSDRV_SETPCICONFIGOFFSET:
        if (isValidAddress(outputBuffer))
        {
            BYTE *pPCIConfig = (BYTE*)outputBuffer;

            Status = pciSetDeviceConfigOffset(pPCIConfig, ioParam->dwFlags, (DWORD)ioParam->dwAddress, (DWORD)ioParam->dwValue);

            *pBytesWritten = 1;
        }
        else
        {
            debugOut(dbError,"! invalid system address %IX",outputBuffer);
        }
        break;

    default:
        debugOut(dbError,"unknown command %lX",ioControlCode);
        Status = STATUS_INVALID_PARAMETER;
        *pBytesWritten = 0;
        break;
    }
    debugOut(dbExit, "");
    return Status;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int CIOAccessDevice::isValidAddress(void * pvAddress)
{
    return MmIsAddressValid(pvAddress);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::allocMemory(ULONG ulLength, DWORD dwFlags, PVOID pUserAddress, PMemStruct pMemStruct, bool above4G)
{
    NTSTATUS      ntStatus;
    PMemoryNode   node;
    DWORD         dwIndex;
    PPageStruct   pPages = (PPageStruct)(pMemStruct + 1);

    // Initialize the MemStruct
    pMemStruct->dwFlags = dwFlags;
    pMemStruct->dwHandle = 0;
    pMemStruct->dwPages = 0;
    pMemStruct->dwTotalSize = 0;
    pMemStruct->dwUser = pUserAddress;

    // don't return memory above 4GB unless the app wants it.
    // it seems the only way to ensure that is via MmAllocateContiguousMemory().    
    // FIXME: determine highest physical address and use for decision
#ifdef _WIN64
    if (!above4G)
#else
    if (!above4G && m_PAEEnabled)
#endif
    {
        dwFlags |= ALLOC_MEMORY_CONTIG;
        debugOut(dbTrace, "allocMemory set contig b/c of 4gb limit");
    }

    //
    //  First alloc our own free memory descriptor
    //
    debugOut(dbTrace,"allocMemory %lu ",ulLength);

    for ( dwIndex = 0; dwIndex < MAX_FREE_MEMORY_NODES; dwIndex++)
    {
        node = &memoryList[ dwIndex ];

        if ( ! node->pSystemAddress )
        {
            node->pSystemAddress = (PVOID)1; // mark as used
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

    if (!above4G)
    {
        // clamp to 4GB
        highestAcceptableAddress.LowPart  = -1;
        highestAcceptableAddress.HighPart = 0;
    }
    else
    {
        // app doesn't care, use all we can get
        highestAcceptableAddress.LowPart  = -1;
        highestAcceptableAddress.HighPart = -1;
    }

    if (dwFlags & ALLOC_MEMORY_CONTIG)
    {
        debugOut(dbTrace, "allocating contigous memory");
        node->pSystemAddress = MmAllocateContiguousMemory(ulLength, highestAcceptableAddress);
        if (!node->pSystemAddress)
        {
            debugOut(dbTrace,"! cannot alloc ContiguousMemory");
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        pUserAddress = node->pSystemAddress;
    }

    debugOut(dbTrace,"alloc %lu bytes of system memory 0x%IX", ulLength, pUserAddress);

    //
    // build the MDL to desribe the memory pages
    //
    node->pMdl = IoAllocateMdl(pUserAddress, ulLength, FALSE, FALSE, NULL);
    if (!node->pMdl)
    {
        if(dwFlags & ALLOC_MEMORY_CONTIG)
        {
            MmFreeContiguousMemory(node->pSystemAddress);
        }
        node->pSystemAddress = NULL;
        debugOut(dbError, " ! cannot alloc MDL");
        return  STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        debugOut(dbTrace,"node->pMdl 0x%IX", node->pMdl);
        //
        // Map locked pages into process's user address space
        //
        if (dwFlags & ALLOC_MEMORY_CONTIG)
           MmBuildMdlForNonPagedPool(node->pMdl);
        else
        {
            __try {
                MmProbeAndLockPages(node->pMdl, KernelMode, IoModifyAccess);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                debugOut(dbError, " ! ProbeAndLock failed: user=0x%IX", node->pUserAddress);
                IoFreeMdl(node->pMdl);
                node->pSystemAddress = NULL;
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            debugOut(dbTrace,"Locked");
        }

        // OK so we've got some memory and we can fill
        // in the return structure

        // need to store this so we know to deallocate
        // any contig memory
        node->dwFlags = dwFlags;

        pMemStruct->dwTotalSize = ulLength;
        pMemStruct->dwHandle = PtrToUlong(node);
        if(dwFlags & ALLOC_MEMORY_CONTIG)
        {
            __try {
                node->pUserAddress = MmMapLockedPages(node->pMdl, UserMode);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                MmFreeContiguousMemory(node->pSystemAddress);
                IoFreeMdl(node->pMdl);
                debugOut(dbError, "MmMapLockedPages caused an exception (user=0x%IX)", node->pUserAddress);
                node->pSystemAddress = NULL;
                return STATUS_INSUFFICIENT_RESOURCES;       
            }
        }
        else
        {
            node->pUserAddress = pUserAddress;
        }
        
        if ( above4G )
            ntStatus = buildPageStruct64(pMemStruct, node);
        else
            ntStatus = buildPageStruct32(pMemStruct, node);
        
        if (ntStatus != STATUS_SUCCESS)
        {
            debugOut(dbError, "allocMemory: something went wrong, aborting!");
            freeMemory(node);
        }
        else
        {
            debugOut(dbTrace,"node->dwUserAddress 0x%IX", node->pUserAddress);
            debugOut(dbTrace,"Pages %d", (node->pMdl->Size - sizeof(MDL))/4);
        }
    }
    return ntStatus;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::buildPageStruct32(PMemStruct pMemStruct, PMemoryNode node)
{
    PPageStruct pPages = (PPageStruct)(pMemStruct + 1);
    
    if (node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        pMemStruct->dwPages = 1;
        pMemStruct->dwUser = node->pUserAddress;
        ULONGLONG phys = GetPhysAddr(node->pUserAddress).QuadPart;
        if (phys > 0xFFFFFFFFU)
        {
            // Too bad, not in address space range
            debugOut(dbError, "allocMemory() returned 64-bit address: 0x%I64X. User address was 0x%IX", phys, node->pUserAddress);
            return STATUS_CONFLICTING_ADDRESSES;
        }        
        pPages[0].dwPhysical = (DWORD)phys;
        pPages[0].dwSize = pMemStruct->dwTotalSize;
    }
    else
    {
        int Pages = 1;
        DWORD_PTR LastUserAddr = (DWORD_PTR)node->pUserAddress;
        ULONGLONG phys = GetPhysAddr(node->pUserAddress).QuadPart;
        if (phys > 0xFFFFFFFFU)
        {
            // Too bad, not in address space range
            debugOut(dbError, "allocMemory() returned 64-bit address: 0x%I64X. User address was 0x%IX", phys, node->pUserAddress);
            return STATUS_CONFLICTING_ADDRESSES;
        }        
        node->pSystemAddress = (PVOID)phys;
        pPages[0].dwPhysical = (DWORD)phys; 
        for(DWORD_PTR i = (DWORD_PTR)node->pUserAddress; i < (DWORD_PTR)node->pUserAddress + pMemStruct->dwTotalSize; i++)
        {
            if(i % 4096 == 0)
            {
                phys = GetPhysAddr((PVOID)i).QuadPart;
                if (phys > 0xFFFFFFFFU)
                {
                    // Too bad, not in address space range
                    debugOut(dbError, "allocMemory() returned 64-bit address: 0x%I64X. User address was 0x%IX", phys, i);
                    return STATUS_CONFLICTING_ADDRESSES;
                }        
                pPages[Pages].dwPhysical = (DWORD)phys;
                pPages[Pages - 1].dwSize = (DWORD)(i - LastUserAddr);
                Pages++;
                LastUserAddr = i;
            }
        }
        pPages[Pages - 1].dwSize = (DWORD)(i - LastUserAddr);
        pMemStruct->dwPages = Pages;
    }
    return STATUS_SUCCESS;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::buildPageStruct64(PMemStruct pMemStruct, PMemoryNode node)
{
    PPageStruct64 pPages = (PPageStruct64)(pMemStruct + 1);
    
    if (node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        pMemStruct->dwPages = 1;
        pMemStruct->dwUser = node->pUserAddress;
        pPages[0].llPhysical = GetPhysAddr(node->pUserAddress).QuadPart;
        pPages[0].dwSize = pMemStruct->dwTotalSize;
    }
    else
    {
        int Pages = 1;
        DWORD_PTR LastUserAddr = (DWORD_PTR)node->pUserAddress;
        pPages[0].llPhysical = GetPhysAddr(node->pUserAddress).QuadPart; 
        node->pSystemAddress = (PVOID)pPages[0].llPhysical;
        for(DWORD_PTR i = (DWORD_PTR)node->pUserAddress; i < (DWORD_PTR)node->pUserAddress + pMemStruct->dwTotalSize; i++)
        {
            if(i % 4096 == 0)
            {
                pPages[Pages].llPhysical = GetPhysAddr((PVOID)i).QuadPart;
                pPages[Pages - 1].dwSize = (DWORD)(i - LastUserAddr);
                Pages++;
                LastUserAddr = i;
            }
        }
        pPages[Pages - 1].dwSize = (DWORD)(i - LastUserAddr);
        pMemStruct->dwPages = Pages;
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

        if (PtrToUlong(node) == pMemStruct->dwHandle)
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
        MmUnmapLockedPages(node->pUserAddress, node->pMdl);
    }
    else
    {
        MmUnlockPages(node->pMdl);
    }

    IoFreeMdl(node->pMdl);

    if(node->dwFlags & ALLOC_MEMORY_CONTIG)
    {
        MmFreeContiguousMemory(node->pSystemAddress);
    }
    memset(node, 0, sizeof(MemoryNode));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
PHYSICAL_ADDRESS CIOAccessDevice::GetPhysAddr(PVOID UserAddr)
{
    return MmGetPhysicalAddress(UserAddr);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::mapMemory(DWORD dwBusNumber, DWORD_PTR dwPhysicalAddress, ULONG ulLength, PVOID *pUserMapping)
{
    debugOut(dbTrace,"mapMemory for 0x%IX", dwPhysicalAddress);

    PHYSICAL_ADDRESS translatedAddress;
    PHYSICAL_ADDRESS busAddress;
    DWORD            addressSpace;
    PVOID            pMemoryBase;
    BOOLEAN          bTranslate;
    PMappingNode     node = NULL;

    //KIRQL irql;

    // find unused node
    for ( DWORD i = 0; i < MAX_FREE_MAPPING_NODES; i++ )
        if ( mappingList[i].pUser == NULL )
        {
            node = &mappingList[i];	   
            break;
        }

    if ( !node )
    {
       debugOut(dbError," ! no free mapping descriptor available");
       return STATUS_INSUFFICIENT_RESOURCES;
    }

    // TODO: what's the reason for this??
    //KeRaiseIrql(PASSIVE_LEVEL, &irql);

    busAddress.QuadPart         = dwPhysicalAddress;
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
        debugOut(dbError,"HalTranslateBusAddress() failed for 0x%IX, addressSpace %d", dwPhysicalAddress, addressSpace);
        translatedAddress.QuadPart = dwPhysicalAddress;
    }

    //
    // kernel space
    //
    pMemoryBase = MmMapIoSpace(translatedAddress, ulLength, MmNonCached);
    debugOut(dbTrace, "MmMapIoSpace physical address 0x%I64X to memory base 0x%IX, length %d", translatedAddress.QuadPart, pMemoryBase, ulLength);
    node->pSystem = pMemoryBase;
    node->ulLength = ulLength;

    //KeLowerIrql(irql);
    
    //
    // user space
    //
    node->pMdl = IoAllocateMdl(pMemoryBase, ulLength, FALSE, FALSE, NULL);
    if (!(node->pMdl))
    {
        MmUnmapIoSpace(pMemoryBase, ulLength);
        debugOut(dbError, "IoAllocateMdl failed");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    MmBuildMdlForNonPagedPool(node->pMdl);
    __try {
        node->pUser = MmMapLockedPagesSpecifyCache(node->pMdl, UserMode, MmNonCached, NULL, FALSE, NormalPagePriority);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        node->pUser = NULL;
        IoFreeMdl(node->pMdl);
        MmUnmapIoSpace(pMemoryBase, ulLength);
        debugOut(dbError, "MmMapLockedPages caused an exception");
        return STATUS_INSUFFICIENT_RESOURCES;       
    }
    if (!(node->pUser))
    {
        IoFreeMdl(node->pMdl);
        MmUnmapIoSpace(pMemoryBase, ulLength);
        debugOut(dbError, "MmMapLockedPages faild for 0x%IX", pMemoryBase);
        return STATUS_INSUFFICIENT_RESOURCES;       
    }
    
    debugOut(dbTrace, "MmMapLockedPages to user address 0x%IX", node->pUser);
    
    *pUserMapping = node->pUser;
    return STATUS_SUCCESS;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void
CIOAccessDevice::unmapMemory(PVOID pMemoryBase, ULONG ulMappedMemoryLength)
{
    if (pMemoryBase)
    {
        for (int i = 0; i < MAX_FREE_MAPPING_NODES; i++)
            if (mappingList[i].pUser == pMemoryBase)
            {
                //KIRQL irql;
                
                if ( mappingList[i].ulLength != ulMappedMemoryLength )
                    debugOut(dbWarning, "unmapMemory: length argument invalid, using driver value");
                
                // free user mapping
                MmUnmapLockedPages(mappingList[i].pUser, mappingList[i].pMdl);
                IoFreeMdl(mappingList[i].pMdl);
                
                // TODO: what's the reason for this??
                //KeRaiseIrql(PASSIVE_LEVEL, &irql);
        
                debugOut(dbTrace,"MMUnmapIoSpace() %IX, length %d", mappingList[i].pSystem, mappingList[i].ulLength);
                MmUnmapIoSpace(mappingList[i].pSystem, mappingList[i].ulLength);
        
                //KeLowerIrql(irql);
                
                mappingList[i].pUser = NULL;
            }
    }
}
