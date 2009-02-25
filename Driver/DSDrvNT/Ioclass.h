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
// 13 Mar 2006   Michael Lutz          Modifications for Win64
//
/////////////////////////////////////////////////////////////////////////////

#if ! defined (__IOCLASS_H)
#define __IOCLASS_H

#include "precomp.h"

#if !defined(__cplusplus)
#error C++ compiler required.
#endif

#define MAX_FREE_MEMORY_NODES   32
#define MAX_FREE_MAPPING_NODES   8

class MemoryNode
{
public:
    PVOID  pSystemAddress;
    PVOID  pUserAddress;
    DWORD  dwFlags;
    PMDL   pMdl;
};
typedef MemoryNode * PMemoryNode;

class MappingNode
{
public:
    PVOID  pUser;
    PVOID  pSystem;
    ULONG  ulLength;
    PMDL   pMdl;
};
typedef MappingNode * PMappingNode;


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class CIOAccessDevice
{
public:
    CIOAccessDevice();
    ~CIOAccessDevice();

    NTSTATUS deviceIOControl(PIRP irp);

protected:
    int isValidAddress(void * pvAddress);
    NTSTATUS deviceControl(DWORD ioControlCode, PDSDrvParam ioParam, DWORD *outputBuffer, DWORD outLen, DWORD_PTR *pulBytesWritten, bool is32);
    NTSTATUS pciFindDevice(DWORD vendorID, DWORD deviceID, DWORD dwCardIndex, DWORD* pdwBusNumber, DWORD *pdwSlotNumber);
    NTSTATUS pciGetDeviceInfo(TPCICARDINFO* pPCICardInfo);
    NTSTATUS allocMemory(ULONG ulLength, DWORD ulFlags, PVOID ulUserAddress, PMemStruct pMemStruct, bool above4G);
    NTSTATUS CIOAccessDevice::buildPageStruct32(PMemStruct pMemStruct, PMemoryNode node);
    NTSTATUS CIOAccessDevice::buildPageStruct64(PMemStruct pMemStruct, PMemoryNode node);
    NTSTATUS freeMemory(PMemStruct pMemStruct);
    void freeMemory(PMemoryNode node);
    MemoryNode memoryList[MAX_FREE_MEMORY_NODES];
    
    NTSTATUS mapMemory(DWORD dwBusNumber, DWORD_PTR dwBaseAddress, ULONG ulLength, PVOID *pUserMapping);
    void unmapMemory(PVOID dwMemoryBase, ULONG ulMappedMemoryLength);
    MappingNode mappingList[MAX_FREE_MAPPING_NODES];

protected:
    NTSTATUS pciGetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot);
    NTSTATUS pciSetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot);
    NTSTATUS pciGetDeviceConfigOffset(BYTE *pPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot);
    NTSTATUS pciSetDeviceConfigOffset(BYTE *pPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot);
    PHYSICAL_ADDRESS GetPhysAddr(PVOID UserAddr);
private:
    bool m_AllowDeprecatedIOCTLs;
#ifndef _WIN64
    bool m_PAEEnabled;
#endif
};

#endif


