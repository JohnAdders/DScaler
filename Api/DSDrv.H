/////////////////////////////////////////////////////////////////////////////
// DSDrv.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
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

#if ! defined (__DSDRVDEF_H)
#define __DSDRVDEF_H

#define ALLOC_MEMORY_CONTIG 1

typedef struct _PageStruct
{
	DWORD dwSize;
	DWORD dwPhysical;
} TPageStruct, * PPageStruct;

typedef struct _MemStruct
{
	DWORD dwTotalSize;
	DWORD dwPages;
	DWORD dwHandle;
	DWORD dwFlags;
	void* dwUser;
} TMemStruct, * PMemStruct;


#if defined (WIN32) && !defined (_NTKERNEL_)

#include <winioctl.h>

#elif defined(WIN95)



//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) )

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Define the access check value for any access
//
//
// The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
// ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
// constants *MUST* always be in sync.
//


#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe


#elif defined (_NTKERNEL_)

extern "C" {

#include <devioctl.h>

}

//
// Extract transfer type
//

#define IOCTL_TRANSFER_TYPE( _iocontrol)   (_iocontrol & 0x3)


#endif

#ifdef __cplusplus
extern "C" {
#endif


#define FILE_DEVICE_DSCALER 0x8002
#define DSDRV_BASE 0x800

#define DSDRV_READ_BYTE (DSDRV_BASE + 1)
#define DSDRV_READ_WORD (DSDRV_BASE + 2)
#define DSDRV_READ_DWORD (DSDRV_BASE + 3)

#define DSDRV_WRITE_BYTE (DSDRV_BASE + 4)
#define DSDRV_WRITE_WORD (DSDRV_BASE + 5)
#define DSDRV_WRITE_DWORD (DSDRV_BASE + 6)

#define DSDRV_GET_PCI_INFO (DSDRV_BASE + 7)

#define DSDRV_ALLOC_MEMORY (DSDRV_BASE + 8)
#define DSDRV_FREE_MEMORY (DSDRV_BASE + 9)

#define DSDRV_MAP_MEMORY (DSDRV_BASE + 10)
#define DSDRV_UNMAP_MEMORY (DSDRV_BASE + 11)

#define DSDRV_READ_MEMORY_DWORD (DSDRV_BASE + 12)
#define DSDRV_WRITE_MEMORY_DWORD (DSDRV_BASE + 13)

#define DSDRV_READ_MEMORY_WORD (DSDRV_BASE + 14)
#define DSDRV_WRITE_MEMORY_WORD (DSDRV_BASE + 15)

#define DSDRV_READ_MEMORY_BYTE (DSDRV_BASE + 16)
#define DSDRV_WRITE_MEMORY_BYTE (DSDRV_BASE + 17)



//
// The wrapped control codes as required by the system
//

#define DSDRV_CTL_CODE(function,method) CTL_CODE( FILE_DEVICE_DSCALER,function,method,FILE_ANY_ACCESS)


#define ioctlReadBYTE DSDRV_CTL_CODE( DSDRV_READ_BYTE, METHOD_OUT_DIRECT)
#define ioctlReadWORD DSDRV_CTL_CODE( DSDRV_READ_WORD, METHOD_OUT_DIRECT)
#define ioctlReadDWORD DSDRV_CTL_CODE( DSDRV_READ_DWORD, METHOD_OUT_DIRECT)
#define ioctlWriteBYTE DSDRV_CTL_CODE( DSDRV_WRITE_BYTE, METHOD_IN_DIRECT)
#define ioctlWriteWORD DSDRV_CTL_CODE( DSDRV_WRITE_WORD, METHOD_IN_DIRECT)
#define ioctlWriteDWORD DSDRV_CTL_CODE( DSDRV_WRITE_DWORD, METHOD_IN_DIRECT)
#define ioctlAllocMemory DSDRV_CTL_CODE( DSDRV_ALLOC_MEMORY, METHOD_BUFFERED)
#define ioctlFreeMemory DSDRV_CTL_CODE( DSDRV_FREE_MEMORY, METHOD_IN_DIRECT)
#define ioctlGetPCIInfo DSDRV_CTL_CODE( DSDRV_GET_PCI_INFO, METHOD_OUT_DIRECT)
#define ioctlMapMemory DSDRV_CTL_CODE( DSDRV_MAP_MEMORY, METHOD_BUFFERED)
#define ioctlUnmapMemory DSDRV_CTL_CODE( DSDRV_UNMAP_MEMORY, METHOD_BUFFERED)
#define ioctlReadMemoryDWORD DSDRV_CTL_CODE( DSDRV_READ_MEMORY_DWORD, METHOD_OUT_DIRECT)
#define ioctlWriteMemoryDWORD DSDRV_CTL_CODE( DSDRV_WRITE_MEMORY_DWORD, METHOD_IN_DIRECT)
#define ioctlReadMemoryWORD DSDRV_CTL_CODE( DSDRV_READ_MEMORY_WORD, METHOD_OUT_DIRECT)
#define ioctlWriteMemoryWORD DSDRV_CTL_CODE( DSDRV_WRITE_MEMORY_WORD, METHOD_IN_DIRECT)
#define ioctlReadMemoryBYTE DSDRV_CTL_CODE( DSDRV_READ_MEMORY_BYTE, METHOD_OUT_DIRECT)
#define ioctlWriteMemoryBYTE DSDRV_CTL_CODE( DSDRV_WRITE_MEMORY_BYTE, METHOD_IN_DIRECT)


typedef struct tagDSDrvParam
{
	DWORD   dwAddress;
	DWORD   dwValue;
	DWORD   dwFlags;
} TDSDrvParam, * PDSDrvParam;

//---------------------------------------------------------------------------
// This structure is taken from NTDDK.H, we use this only in WIN32 user mode
//---------------------------------------------------------------------------
#if (defined (WIN32) || defined (WIN95) ) && !defined (_NTKERNEL_)

typedef struct _PCI_COMMON_CONFIG
{
	USHORT  VendorID;                   // (ro)
	USHORT  DeviceID;                   // (ro)
	USHORT  Command;                    // Device control
	USHORT  Status;
	UCHAR   RevisionID;                 // (ro)
	UCHAR   ProgIf;                     // (ro)
	UCHAR   SubClass;                   // (ro)
	UCHAR   BaseClass;                  // (ro)
	UCHAR   CacheLineSize;              // (ro+)
	UCHAR   LatencyTimer;               // (ro+)
	UCHAR   HeaderType;                 // (ro)
	UCHAR   BIST;                       // Built in self test

	union
	{
        struct _PCI_HEADER_TYPE_0
		{
            DWORD   BaseAddresses[6];
            DWORD   CIS;
            USHORT  SubVendorID;
            USHORT  SubSystemID;
            DWORD   ROMBaseAddress;
            DWORD   Reserved2[2];

            UCHAR   InterruptLine;      //
            UCHAR   InterruptPin;       // (ro)
            UCHAR   MinimumGrant;       // (ro)
            UCHAR   MaximumLatency;     // (ro)
        } type0;
    } u;
    UCHAR   DeviceSpecific[192];
} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;

#if defined (WIN32)

//---------------------------------------------------------------------------
// The DSDrv DLL application interface
//---------------------------------------------------------------------------

int WINAPI isDriverOpened (void);

BYTE WINAPI readPort(WORD address);
WORD WINAPI readPortW(WORD address);
DWORD WINAPI readPortL(WORD address);
void WINAPI writePort(WORD address, BYTE bValue);
void WINAPI writePortW(WORD address, WORD uValue);
void WINAPI writePortL(WORD address, DWORD dwValue);

DWORD WINAPI memoryAlloc(DWORD  dwLength,
                          DWORD  dwFlags,
						  PMemStruct* ppMemStruct);

DWORD WINAPI memoryFree(PMemStruct pMemStruct);

DWORD WINAPI pciGetHardwareResources(DWORD   dwVendorID,
                                      DWORD   dwDeviceID,
                                      PDWORD  pdwMemoryAddress,
                                      PDWORD  pdwMemoryLength,
                                      PDWORD  pdwSubSystemId);

DWORD WINAPI memoryMap(DWORD dwAddress, DWORD dwLength);
void WINAPI memoryUnmap(DWORD dwAddress, DWORD dwLength);
void WINAPI memoryWriteDWORD(DWORD dwAddress, DWORD dwValue);
DWORD WINAPI memoryReadDWORD(DWORD dwAddress);
void WINAPI memoryWriteWORD(DWORD dwAddress, WORD wValue);
WORD WINAPI memoryReadWORD(DWORD dwAddress);
void WINAPI memoryWriteBYTE(DWORD dwAddress, BYTE ucValue);
BYTE WINAPI memoryReadBYTE(DWORD dwAddress);


typedef int (WINAPI * PIsDriverOpened)(void);
typedef DWORD (WINAPI * PMemoryRead)(DWORD dwAddress);
typedef DWORD (WINAPI * PMemoryWrite)(DWORD dwAddress, DWORD dwValue);

typedef DWORD (WINAPI * PPCIGetHardwareResources)(DWORD   dwVendorID,
                                                  DWORD   dwDeviceID,
                                                  PDWORD  pdwMemoryAddress,
                                                  PDWORD  pdwMemoryLength,
                                                  PDWORD  pdwSubSystemId);

typedef DWORD (WINAPI * PMemoryAlloc)(DWORD  dwLength,
                                        DWORD  dwFlags,
										PMemStruct pMemStruct);

typedef DWORD (WINAPI * PMemoryFree)(DWORD dwUserAddress);
typedef DWORD (WINAPI * PMemoryMap)(DWORD dwAddress, DWORD dwLength);
typedef void * (WINAPI * PMemoryMapEx)(DWORD dwAddress, DWORD dwLength);
typedef DWORD (WINAPI * PMemoryRead) (DWORD dwAddress);
typedef DWORD (WINAPI * PMemoryWrite) (DWORD dwAddress, DWORD dwValue);

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif