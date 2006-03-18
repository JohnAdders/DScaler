/////////////////////////////////////////////////////////////////////////////
// $Id: DSDrv.H,v 1.17 2006-03-18 16:38:23 adcockj Exp $
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
// 08 Aug 2001   John Adcock           Changed functionms to support multiple cards
//                                     Added driver version
//                                     Changed meaning of memory access parameters
//                                     (see note in header)
//
// 13 Mar 2006   Michael Lutz          Adapted for Win64 support
//                                     Bumped minor driver version
//
/////////////////////////////////////////////////////////////////////////////

#if ! defined (__DSDRVDEF_H)
#define __DSDRVDEF_H


#if !defined(_W64) 
	
	#if (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300 
		#define _W64 __w64 
	#else 
		#define _W64 
	#endif 
#endif

#if defined(_WIN64)
    typedef __int64 INT_PTR, *PINT_PTR;
    typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

    typedef __int64 LONG_PTR, *PLONG_PTR;
    typedef unsigned __int64 ULONG_PTR, *PULONG_PTR; 
#else
    typedef _W64 int INT_PTR, *PINT_PTR;
    typedef _W64 unsigned int UINT_PTR, *PUINT_PTR;

    typedef _W64 long LONG_PTR, *PLONG_PTR;
    typedef _W64 unsigned long ULONG_PTR, *PULONG_PTR; 
#endif

typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;


// define version number to be compiled into both files
// we use this to make sure that we are speaking the same language in both the 
// drivers and the dll
#define DSDRV_VERSION 0x4004

// allocate a continous block of memory
#define ALLOC_MEMORY_CONTIG 1

typedef struct
{
	DWORD dwSize;
	DWORD dwPhysical;
} TPageStruct, * PPageStruct;

// keep struct size a multiple of 8, simplifies things for Itanium
typedef struct
{
	ULONGLONG llPhysical;
	DWORD dwSize;
	DWORD dummy;
} TPageStruct64, * PPageStruct64;

// keep size of TMemStruct a multiple of 8 FOR 64-bit VERSIONS
// sizeof(void*)==8 then, so it's okay right now
typedef struct
{
	DWORD dwTotalSize;
	DWORD dwPages;
	DWORD dwHandle;
	DWORD dwFlags;
	void* dwUser;
} TMemStruct, * PMemStruct;


typedef struct
{
    DWORD_PTR  dwMemoryAddress;
    DWORD  dwMemoryLength;
    DWORD  dwSubSystemId;
    DWORD  dwBusNumber;
    DWORD  dwSlotNumber;
} TPCICARDINFO;

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

/** Define the IOCTL's used by DSDrv
    We need to ensure that we don't conflict with other drivers
    so move up both device and base for function code
*/


#define FILE_DEVICE_DSCALER 0x8D00
#define DSDRV_BASE          0xA00

#define IOCTL_DSDRV_READPORTBYTE  \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 1), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_READPORTWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 2), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_READPORTDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 3), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEPORTBYTE \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 4), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEPORTWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 5), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEPORTDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 6), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_ALLOCMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 7), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_FREEMEMORY  \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 8), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_GETPCIINFO \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 9), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_MAPMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 10), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_UNMAPMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 11), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_READMEMORYDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 12), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEMEMORYDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 13), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_READMEMORYWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 14), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEMEMORYWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 15), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_READMEMORYBYTE \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 16), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEMEMORYBYTE \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 17), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_GETVERSION \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 18), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_GETPCICONFIG \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 19), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_SETPCICONFIG \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 20), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_GETPCICONFIGOFFSET \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 21), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_SETPCICONFIGOFFSET \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 22), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_ALLOCMEMORY64 \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 23), METHOD_BUFFERED, FILE_ANY_ACCESS)


/** These are the old codes used by older version of the driver
    and should not be used in new code
    Support for these codes is enabled by requesting IOCTL_DEPRICATED_GETVERSION
    These IOCTLs will be desabled unless that is called first.
    This is done to maintain the backwards compatibility of the driver.
*/

#define FILE_DEVICE_DSCALER_OLD 0x8002
#define DSDRV_BASE_OLD 0x800

#define IOCTL_DEPRICATED_READPORTBYTE  \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 1), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_READPORTWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 2), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_READPORTDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 3), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_WRITEPORTBYTE \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 4), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_WRITEPORTWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 5), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_WRITEPORTDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 6), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_GETPCIINFO \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 7), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_ALLOCMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 8), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_FREEMEMORY  \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 9), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_MAPMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 10), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_UNMAPMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 11), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_READMEMORYDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 12), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_WRITEMEMORYDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 13), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_READMEMORYWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 14), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_WRITEMEMORYWORD \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 15), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_READMEMORYBYTE \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 16), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_WRITEMEMORYBYTE \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 17), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_GETVERSION \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 18), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_GETPCICONFIG \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 19), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DEPRICATED_SETPCICONFIG \
    CTL_CODE(FILE_DEVICE_DSCALER_OLD, (DSDRV_BASE_OLD + 20), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)


typedef struct tagDSDrvParam
{
	DWORD_PTR    dwAddress;
	DWORD_PTR    dwValue;
	DWORD     dwFlags;
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

#endif

#ifdef __cplusplus
}
#endif

#endif
