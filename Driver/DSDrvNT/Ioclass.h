/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.h,v 1.10 2002-10-22 16:01:46 adcockj Exp $
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

#if ! defined (__IOCLASS_H)
#define __IOCLASS_H

#include "precomp.h"

#if !defined(__cplusplus)
#error C++ compiler required.
#endif

#define MAX_FREE_MEMORY_NODES   32

class MemoryNode
{
public:
    DWORD dwSystemAddress;
    DWORD dwUserAddress;
    DWORD dwFlags;
    PMDL pMdl;
};
typedef MemoryNode * PMemoryNode;



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
    NTSTATUS deviceControl(DWORD ioControlCode, PDSDrvParam ioParam, DWORD* outputBuffer, DWORD* pulBytesWritten);
    NTSTATUS pciFindDevice(DWORD vendorID, DWORD deviceID, DWORD dwCardIndex, DWORD* pdwBusNumber, DWORD* pdwSlotNumber);
    NTSTATUS pciGetDeviceInfo(TPCICARDINFO* pPCICardInfo);
    NTSTATUS allocMemory(DWORD ulLength,DWORD ulFlags,DWORD ulUserAddress, PMemStruct pMemStruct);
    NTSTATUS freeMemory(PMemStruct pMemStruct);
    void freeMemory(PMemoryNode node);
    DWORD mapMemory(DWORD dwBusNumber, DWORD dwBaseAddress, DWORD ulLength);
    void unmapMemory(DWORD dwMemoryBase, DWORD dwMappedMemoryLength);

protected:
    NTSTATUS pciGetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot);
    NTSTATUS pciSetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot);
    MemoryNode memoryList[MAX_FREE_MEMORY_NODES];
    DWORD GetPhysAddr(DWORD UserAddr);
private:
    bool m_AllowDepricatedIOCTLs;
};

#endif



