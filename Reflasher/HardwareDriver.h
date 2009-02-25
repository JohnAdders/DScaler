/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth.  All rights reserved.
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

#if !defined(AFX_HARDWAREDRIVER_H__2AC97E31_787D_47CD_A7A5_9131E7C6A6D5__INCLUDED_)
#define AFX_HARDWAREDRIVER_H__2AC97E31_787D_47CD_A7A5_9131E7C6A6D5__INCLUDED_

#include <winsvc.h>
#include <winioctl.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

static const LPSTR NTDriverName = "DSDrv4";

#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS				( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS				( 0x0002 )    // file & pipe

#define FILE_DEVICE_DSCALER				0x8D00
#define DSDRV_BASE						0xA00

#define IOCTL_DSDRV_GETPCIINFO \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 9), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_GETVERSION \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 18), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_MAPMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 10), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_UNMAPMEMORY \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 11), METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_READMEMORYDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 12), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEMEMORYDWORD \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 13), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

#define IOCTL_DSDRV_WRITEMEMORYBYTE \
    CTL_CODE(FILE_DEVICE_DSCALER, (DSDRV_BASE + 17), METHOD_IN_DIRECT, FILE_ANY_ACCESS)

typedef struct tagDSDrvParam
{
	DWORD   dwAddress;
	DWORD   dwValue;
	DWORD   dwFlags;
} TDSDrvParam, * PDSDrvParam;

typedef struct
{
	DWORD  dwMemoryAddress;
	DWORD  dwMemoryLength;
	DWORD  dwSubSystemId;
	DWORD  dwBusNumber;
	DWORD  dwSlotNumber;
} TPCICARDINFO;

class CHardwareDriver  
{
public:
	CHardwareDriver();
	virtual ~CHardwareDriver();

	BOOL DoesThisPCICardExist(WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD& SubSystemId);
	BOOL UnInstallNTDriver();
	BOOL InstallNTDriver();
	void UnloadDriver();
	BOOL LoadDriver();
   
	DWORD SendCommand(  DWORD dwIOCommand,
                        LPVOID pvInput,
                        DWORD dwInputLength,
                        LPVOID pvOutput,
                        DWORD dwOutputLength,
                        LPDWORD pdwReturnedLength
                     );

    DWORD SendCommand(  DWORD dwIOCommand,
                        LPVOID pvInput,
                        DWORD dwInputLength
                     );

private:
	DWORD		StopService( SC_HANDLE hSCM, SC_HANDLE hService, BOOL fStopDependencies, DWORD dwTimeout);
	BOOL		AdjustAccessRights();
    SC_HANDLE   m_hService;
    HANDLE      m_hFile;
    BOOL        m_bWindows95;
    BOOL        m_WeStartedDriver;
};

#endif // !defined(AFX_HARDWAREDRIVER_H__2AC97E31_787D_47CD_A7A5_9131E7C6A6D5__INCLUDED_)
