/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.h,v 1.4 2001-08-08 10:53:30 adcockj Exp $
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

#define  osPortReadByte(port,data) data = READ_PORT_UCHAR ( (PUCHAR) &port )
#define  osPortReadWord(port,data) data = READ_PORT_USHORT( (PUSHORT) &port)
#define  osPortReadLong(port,data) data = READ_PORT_ULONG( (PULONG) &port )
#define  osPortWriteByte(port,data) WRITE_PORT_UCHAR ( (PUCHAR) &port, (UCHAR) data)
#define  osPortWriteWord(port,data) WRITE_PORT_USHORT( (PUSHORT) &port, (USHORT) data)
#define  osPortWriteLong(port,data) WRITE_PORT_ULONG ( (PULONG) &port, (DWORD) data)

#define  osMemoryReadDWORD(port,data) data = READ_REGISTER_ULONG( (PULONG) port)
#define  osMemoryWriteDWORD(port,data) WRITE_REGISTER_ULONG( (PULONG) port, (DWORD) data)
#define  osMemoryReadWORD(port,data) data = READ_REGISTER_USHORT( (PUSHORT) port)
#define  osMemoryWriteWORD(port,data) WRITE_REGISTER_USHORT( (PUSHORT) port, (USHORT) data)
#define  osMemoryReadBYTE(port,data) data = READ_REGISTER_UCHAR( (PUCHAR) port)
#define  osMemoryWriteBYTE(port,data) WRITE_REGISTER_UCHAR( (PUCHAR) port, (UCHAR) data)

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
    NTSTATUS pciGetDeviceConfig(DWORD dwBusNumber, DWORD dwSlotNumber, PPCI_COMMON_CONFIG pciConfiguration);
    NTSTATUS allocMemory(DWORD ulLength,DWORD ulFlags,DWORD ulUserAddress, PMemStruct pMemStruct);
    NTSTATUS freeMemory(PMemStruct pMemStruct);
    void freeMemory(PMemoryNode node);
    DWORD mapMemory(DWORD dwBaseAddress, DWORD ulLength);
    void unmapMemory(void);

protected:
    MemoryNode memoryList[MAX_FREE_MEMORY_NODES];
    DWORD GetPhysAddr(DWORD UserAddr);
    DWORD dwBusNumber;
    DWORD dwSlotNumber;
    DWORD dwMemoryBase;            // mapped memory base
    DWORD dwMappedMemoryLength;
};

#endif



