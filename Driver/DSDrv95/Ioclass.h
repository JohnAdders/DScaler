/////////////////////////////////////////////////////////////////////////////
// $Id: Ioclass.h,v 1.4 2001-08-08 16:37:50 adcockj Exp $
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

#include "Basetype.h"
#include "DSDrv.h"

#if defined (WIN95)

#include <vmmreg.h>

#define PCI_ENUM_FUNC_GET_DEVICE_INFO  0
#define PIRP PDIOCPARAMETERS

typedef DWORD DEVNODE;
typedef DWORD CONFIGRET;
typedef DWORD ENUMFUNC;
typedef DWORD HKEY, * PHKEY;

extern "C" {

int  __cdecl _inp(unsigned short);
unsigned short __cdecl _inpw(unsigned short);
unsigned long __cdecl _inpd(unsigned short);

int __cdecl _outp(unsigned short, int);
unsigned short __cdecl _outpw(unsigned short, unsigned short);
unsigned long __cdecl _outpd(unsigned short, unsigned long);

DWORD __cdecl _LinPageLock(DWORD HLinPgNum, DWORD nPages, DWORD Flags) ;
int  __cdecl  _LinPageUnLock(DWORD HLinPgNum, DWORD nPages, DWORD Flags) ;
PVOID _cdecl _PageAllocate(DWORD nPages, DWORD pType, HVM hvm, DWORD AlignMask, DWORD minPhys, DWORD maxPhys, PVOID *PhysAddr, DWORD flags);
BOOL _cdecl _PageFree(PVOID hMem, DWORD flags);
void * __cdecl  _MapPhysToLinear(const void * PhysAddr, DWORD nBytes, DWORD Flags) ;
void * __cdecl _MapLinearToPhys(HANDLE,DWORD);
BOOL __cdecl _CopyPageTable(DWORD PageNumber,DWORD nPages, PDWORD ppte,DWORD flags);

#pragma intrinsic(_outp)
#pragma intrinsic(_outpw)
#pragma intrinsic(_outpd)
#pragma intrinsic(_inp)
#pragma intrinsic(_inpw)
#pragma intrinsic(_inpd)


#define  osPortReadByte(port,data)       data=_inp(port)
#define  osPortReadWord(port,data)       data=_inpw(port)
#define  osPortReadLong(port,data)       data=_inpw(port)

#define  osPortWriteByte(port,data)      _outp(port,data)
#define  osPortWriteWord(port,data)      _outpw(port,data)
#define  osPortWriteLong(port,data)      _outpd(port,data)

#define  osMemoryReadDWORD(port,data)         data = *((PDWORD)port)
#define  osMemoryWriteDWORD(port,data)        *((PDWORD)port) = (DWORD)data
#define  osMemoryReadWORD(port,data)         data = *((PWORD)port)
#define  osMemoryWriteWORD(port,data)        *((PWORD)port) = (WORD)data
#define  osMemoryReadBYTE(port,data)         data = *((PBYTE)port)
#define  osMemoryWriteBYTE(port,data)        *((PBYTE)port) = (BYTE)data

LONG __cdecl _RegCloseKey(HKEY hkey);
LONG __cdecl _RegOpenKey(HKEY hkey, PCHAR lpszSubKey, PHKEY phkResult) ;
LONG __cdecl _RegEnumKey(HKEY hkey, DWORD iSubKey, PCHAR lpszName, DWORD cchName) ;
LONG __cdecl _RegCloseKey(HKEY hkey) ;


CONFIGRET __cdecl _CONFIGMG_Locate_DevNode (DEVNODE * pNode, PCHAR szDevId, DWORD flags) ;
CONFIGRET __cdecl _CONFIGMG_Call_Enumerator_Function (DEVNODE dnDevNode, ENUMFUNC efFunc, DWORD RefData, void* pBuffer, DWORD ulBufferSize, DWORD ulFlags);
}

#elif defined (_NTKERNEL_)

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

#endif


#define MAX_FREE_MEMORY_NODES   32

typedef struct tagMemoryNode
{
    DWORD dwSystemAddress;
    DWORD dwUserAddress;
    DWORD dwFlags;
#if defined (WIN95)
    DWORD dwPages;
#elif defined (_NTKERNEL_)
    PMDL pMdl;
#endif
} TMemoryNode, * PMemoryNode;


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
    NTSTATUS pciGetDeviceConfig(TPCICARDINFO* pPCICardInfo);
    NTSTATUS allocMemory(DWORD ulLength,DWORD ulFlags,DWORD ulUserAddress, PMemStruct pMemStruct);
    NTSTATUS freeMemory(PMemStruct pMemStruct);
    void freeMemory(PMemoryNode node);
    DWORD mapMemory(DWORD dwBaseAddress, DWORD ulLength);
    void unmapMemory(void);

protected:
    TMemoryNode memoryList[MAX_FREE_MEMORY_NODES];
    DWORD GetPhysAddr(DWORD UserAddr);
};


#endif



