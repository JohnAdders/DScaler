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

#if ! defined (__IOCLASS_H)
#define __IOCLASS_H

#include "Basetype.h"
#include "DSDrv.h"

#include <vmmreg.h>

#define PCI_ENUM_FUNC_GET_DEVICE_INFO  0
#define PCI_ENUM_FUNC_SET_DEVICE_INFO  1
#define PIRP PDIOCPARAMETERS

typedef DWORD DEVNODE;
typedef DWORD CONFIGRET;
typedef DWORD ENUMFUNC;
typedef DWORD HKEY, * PHKEY;

extern "C" 
{

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


LONG __cdecl _RegCloseKey(HKEY hkey);
LONG __cdecl _RegOpenKey(HKEY hkey, PCHAR lpszSubKey, PHKEY phkResult) ;
LONG __cdecl _RegEnumKey(HKEY hkey, DWORD iSubKey, PCHAR lpszName, DWORD cchName) ;
LONG __cdecl _RegCloseKey(HKEY hkey) ;


CONFIGRET __cdecl _CONFIGMG_Locate_DevNode (DEVNODE * pNode, PCHAR szDevId, DWORD flags) ;
CONFIGRET __cdecl _CONFIGMG_Call_Enumerator_Function (DEVNODE dnDevNode, ENUMFUNC efFunc, DWORD RefData, void* pBuffer, DWORD ulBufferSize, DWORD ulFlags);
}


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
    NTSTATUS pciGetDeviceInfo(TPCICARDINFO* pPCICardInfo);
    NTSTATUS pciGetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot);
    NTSTATUS pciSetDeviceConfig(PCI_COMMON_CONFIG *pPCIConfig, DWORD Bus, DWORD Slot);
    NTSTATUS pciGetDeviceConfigOffset(BYTE* pPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot);
    NTSTATUS pciSetDeviceConfigOffset(BYTE *pPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot);
    NTSTATUS allocMemory(DWORD ulLength,DWORD ulFlags,DWORD ulUserAddress, PMemStruct pMemStruct, bool is64);
    NTSTATUS buildPageStruct32(PMemStruct pMemStruct, PMemoryNode node, DWORD phys);
    NTSTATUS buildPageStruct64(PMemStruct pMemStruct, PMemoryNode node, DWORD phys);
    NTSTATUS freeMemory(PMemStruct pMemStruct);
    void freeMemory(PMemoryNode node);
    DWORD mapMemory(DWORD dwBaseAddress, DWORD ulLength);
    void unmapMemory(void);

protected:
    TMemoryNode memoryList[MAX_FREE_MEMORY_NODES];
    DWORD GetPhysAddr(DWORD UserAddr);
private:
    bool m_AllowDepricatedIOCTLs;
};


#endif



