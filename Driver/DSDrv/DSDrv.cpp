/////////////////////////////////////////////////////////////////////////////
// DSDrv.cpp
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
// 05 Jan 2001   John Adcock           Added SetCurrentDirectoryCall
//
/////////////////////////////////////////////////////////////////////////////

#include "KernelDriver.h"
#include "DSDrv.h"
#include "debugout.h"

class CdTVDriver : public CKernelDriver
{
public:
	CdTVDriver();

	DWORD DSDrvStartDriver(void);
	DWORD DSDrvStopDriver(void);

	BOOL isDriverOpen(void) { return bDriverRunning; }

	BYTE readPort(DWORD dwAddress);
	WORD readPortW(DWORD dwAddress);
	DWORD readPortL(DWORD dwAddress);
	void writePort(DWORD dwAddress, DWORD dwValue);
	void writePortW(DWORD dwAddress, DWORD dwValue);
	void writePortL(DWORD dwAddress, DWORD dwValue);
	DWORD allocMemory(DWORD dwLength, DWORD dwFlags, PMemStruct* ppMemStruct);
	DWORD freeMemory(PMemStruct pMemStruct);
	DWORD pciGetHardwareResources(DWORD dwVendorID, DWORD  dwDeviceID, PDWORD pdwMemoryAddress, PDWORD pdwMemoryLength, PDWORD pdwSubSystemId);
	DWORD memoryMap(DWORD dwAddress, DWORD dwLength);
	void memoryUnmap(DWORD dwAddress, DWORD dwLength);
	void memoryWriteDWORD(DWORD dwAddress, DWORD dwValue);
	DWORD memoryReadDWORD(DWORD dwAddress);
	void memoryWriteWORD(DWORD dwAddress, WORD dwValue);
	WORD memoryReadWORD(DWORD dwAddress);
	void memoryWriteBYTE(DWORD dwAddress, BYTE dwValue);
	BYTE memoryReadBYTE(DWORD dwAddress);


protected:
	BOOL    bWindows95;
	BOOL    bDriverRunning;
};


CdTVDriver dTVDriver;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int WINAPI isDriverOpened(void)
{
	return dTVDriver.isDriverOpen();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BYTE WINAPI readPort(WORD address)
{
	return dTVDriver.readPort( (DWORD) address);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
WORD WINAPI readPortW(WORD address)
{
	return dTVDriver.readPortW( (DWORD) address);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI readPortL(WORD address)
{
	return dTVDriver.readPortL( (DWORD) address);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI writePort(WORD address, BYTE bValue)
{
	dTVDriver.writePort( (DWORD) address, (DWORD) bValue);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI writePortW(WORD address, WORD uValue)
{
	dTVDriver.writePortW( (DWORD) address, (DWORD) uValue);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI writePortL(WORD address, DWORD dwValue)
{
	dTVDriver.writePortL( (DWORD) address, (DWORD) dwValue);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI memoryAlloc(DWORD dwLength, DWORD dwFlags, PMemStruct* ppMemStruct)
{
	return dTVDriver.allocMemory(dwLength, dwFlags, ppMemStruct);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI memoryFree(PMemStruct pMemStruct)
{
	if(pMemStruct)
	{
		return dTVDriver.freeMemory(pMemStruct);
	}
	else
	{
		return 0;
	}
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI memoryMap(DWORD dwAddress, DWORD dwLength)
{
	return dTVDriver.memoryMap(dwAddress, dwLength);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI memoryUnmap(DWORD dwAddress, DWORD dwLength)
{
	dTVDriver.memoryUnmap(dwAddress, dwLength);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI memoryWriteDWORD(DWORD dwAddress, DWORD dwValue)
{
	dTVDriver.memoryWriteDWORD(dwAddress, dwValue);
}

DWORD WINAPI memoryReadDWORD(DWORD dwAddress)
{
	return dTVDriver.memoryReadDWORD(dwAddress);
}

void WINAPI memoryWriteWORD(DWORD dwAddress, WORD wValue)
{
	dTVDriver.memoryWriteWORD(dwAddress, wValue);
}

WORD WINAPI memoryReadWORD(DWORD dwAddress)
{
	return dTVDriver.memoryReadWORD(dwAddress);
}

void WINAPI memoryWriteBYTE(DWORD dwAddress, BYTE ucValue)
{
	dTVDriver.memoryWriteBYTE(dwAddress, ucValue);
}

BYTE WINAPI memoryReadBYTE(DWORD dwAddress)
{
	return dTVDriver.memoryReadBYTE(dwAddress);
}


//---------------------------------------------------------------------------
// Get back the PCI hardware resources, we assumes that only mapped memory
// is avail.
//---------------------------------------------------------------------------
DWORD WINAPI pciGetHardwareResources(DWORD   dwVendorID,
										DWORD dwDeviceID,
										PDWORD pdwMemoryAddress,
										PDWORD pdwMemoryLength,
										PDWORD pdwSubSystemId)
{
	return dTVDriver.pciGetHardwareResources(dwVendorID,
												dwDeviceID,
												pdwMemoryAddress,
												pdwMemoryLength,
												pdwSubSystemId);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CdTVDriver::CdTVDriver(void) : CKernelDriver()
{
	OSVERSIONINFO ov;

	ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx( &ov);
	bWindows95 = (ov.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);

	bDriverRunning = FALSE;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BYTE CdTVDriver::readPort(DWORD dwAddress)
{
	TDSDrvParam  param;
	DWORD       dwReturnedLength;

	param.dwAddress = dwAddress;
	deviceControl(ioctlReadBYTE,
					&param,
					sizeof(param.dwAddress),
					&param.dwValue,
					sizeof(param.dwValue),
					&dwReturnedLength);

	return (BYTE) param.dwValue;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
WORD CdTVDriver::readPortW(DWORD dwAddress)
{
	TDSDrvParam  param;
	DWORD       dwReturnedLength;

	param.dwAddress = dwAddress;
	deviceControl(ioctlReadWORD,
					&param,
					sizeof(param.dwAddress),
					&param.dwValue,
					sizeof(param.dwValue),
					&dwReturnedLength);
	return (WORD) param.dwValue;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver::readPortL(DWORD dwAddress)
{
	TDSDrvParam  param;
	DWORD       dwReturnedLength;

	param.dwAddress = dwAddress;
	deviceControl(ioctlReadDWORD,
					&param,
					sizeof(param.dwAddress),
					&param.dwValue,
					sizeof(param.dwValue),
					&dwReturnedLength);

	return param.dwValue;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CdTVDriver::writePort(DWORD dwAddress, DWORD dwValue)
{
	TDSDrvParam  param;

	param.dwAddress = dwAddress;
	param.dwValue = dwValue;

	deviceControl(ioctlWriteBYTE, &param, sizeof(param));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CdTVDriver::writePortW(DWORD dwAddress, DWORD dwValue)
{
	TDSDrvParam  param;

	param.dwAddress = dwAddress;
	param.dwValue = dwValue;

	deviceControl(ioctlWriteWORD, &param, sizeof(param));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CdTVDriver::writePortL(DWORD dwAddress, DWORD dwValue)
{
	TDSDrvParam  param;

	param.dwAddress = dwAddress;
	param.dwValue = dwValue;

	deviceControl(ioctlWriteDWORD, &param, sizeof(param));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver:: allocMemory(DWORD dwLength, DWORD dwFlags, PMemStruct* ppMemStruct)
{
	TDSDrvParam paramIn;
	DWORD dwReturnedLength;
	DWORD status;
	DWORD nPages = 0;

	if(dwFlags & ALLOC_MEMORY_CONTIG)
	{
		nPages = 1;
	}
	else
	{
		nPages = (dwLength + 4095) / 4096 + 1;
	}
	
	DWORD dwOutParamLength = sizeof(TMemStruct) + nPages * sizeof(TPageStruct);
	*ppMemStruct = (PMemStruct) malloc(dwOutParamLength);

	paramIn.dwValue = dwLength;
	paramIn.dwFlags = dwFlags;
	if(dwFlags & ALLOC_MEMORY_CONTIG)
	{
		paramIn.dwAddress = 0;
	}
	else
	{
		paramIn.dwAddress = (ULONG)malloc(dwLength);
		memset((void*)paramIn.dwAddress, 0, dwLength);
		if(paramIn.dwAddress == NULL)
		{
			free(*ppMemStruct);
			return ERROR_NOT_ENOUGH_MEMORY;
		}
	}
	status = deviceControl(ioctlAllocMemory,
							&paramIn,
							sizeof(paramIn),
							*ppMemStruct,
							dwOutParamLength,
							&dwReturnedLength);

	if(status != ERROR_SUCCESS || ppMemStruct == NULL || (*ppMemStruct)->dwUser == 0)
	{
		if(!(dwFlags & ALLOC_MEMORY_CONTIG))
		{
			free((void*)paramIn.dwAddress);
		}
		free(*ppMemStruct);
		*ppMemStruct = NULL;
	}

	return status;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver::freeMemory(PMemStruct pMemStruct)
{
	DWORD status = ERROR_SUCCESS;
	if(pMemStruct != NULL)
	{
		DWORD dwInParamLength = sizeof(TMemStruct) + pMemStruct->dwPages * sizeof(TPageStruct);
		status = deviceControl(ioctlFreeMemory, pMemStruct, dwInParamLength);
		if(!(pMemStruct->dwFlags & ALLOC_MEMORY_CONTIG))
		{
			free(pMemStruct->dwUser);
		}
		free(pMemStruct);
	}
	return status;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver::memoryMap(DWORD dwAddress, DWORD dwLength)
{
	TDSDrvParam hwParam;
	DWORD       dwMappedAddress;
	DWORD       dwReturnedLength;

	hwParam.dwAddress = dwAddress;
	hwParam.dwValue   = dwLength;
	dwMappedAddress = 0;

	deviceControl(ioctlMapMemory,
					&hwParam,
					sizeof(hwParam),
					&dwMappedAddress,
					sizeof(dwMappedAddress),
					&dwReturnedLength);

	return dwMappedAddress;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CdTVDriver::memoryUnmap(DWORD dwAddress, DWORD dwLength)
{
	TDSDrvParam hwParam;

	hwParam.dwAddress = dwAddress;
	hwParam.dwValue   = dwLength;

	deviceControl(ioctlUnmapMemory, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CdTVDriver::memoryWriteDWORD(DWORD dwAddress, DWORD dwValue)
{
	TDSDrvParam hwParam;

	hwParam.dwAddress = dwAddress;
	hwParam.dwValue = dwValue;

	deviceControl(ioctlWriteMemoryDWORD, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver::memoryReadDWORD(DWORD dwAddress)
{
	TDSDrvParam hwParam;
	DWORD dwReturnedLength;
	DWORD dwValue(0);

	hwParam.dwAddress = dwAddress;
	deviceControl(ioctlReadMemoryDWORD,
					&hwParam,
					sizeof(hwParam.dwAddress),
					&dwValue,
					sizeof(dwValue),
					&dwReturnedLength);

	return dwValue;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CdTVDriver::memoryWriteWORD(DWORD dwAddress, WORD wValue)
{
	TDSDrvParam hwParam;

	hwParam.dwAddress = dwAddress;
	hwParam.dwValue = wValue;

	deviceControl(ioctlWriteMemoryWORD, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
WORD CdTVDriver::memoryReadWORD(DWORD dwAddress)
{
	TDSDrvParam hwParam;
	DWORD dwReturnedLength;
	WORD wValue(0);

	hwParam.dwAddress = dwAddress;
	deviceControl(ioctlReadMemoryWORD,
					&hwParam,
					sizeof(hwParam.dwAddress),
					&wValue,
					sizeof(wValue),
					&dwReturnedLength);

	return wValue;
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CdTVDriver::memoryWriteBYTE(DWORD dwAddress, BYTE ucValue)
{
	TDSDrvParam hwParam;

	hwParam.dwAddress = dwAddress;
	hwParam.dwValue = ucValue;

	deviceControl(ioctlWriteMemoryBYTE, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BYTE CdTVDriver::memoryReadBYTE(DWORD dwAddress)
{
	TDSDrvParam hwParam;
	DWORD dwReturnedLength;
	BYTE ucValue(0);

	hwParam.dwAddress = dwAddress;
	deviceControl(ioctlReadMemoryBYTE,
					&hwParam,
					sizeof(hwParam.dwAddress),
					&ucValue,
					sizeof(ucValue),
					&dwReturnedLength);

	return ucValue;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver::pciGetHardwareResources(DWORD dwVendorID,
											DWORD dwDeviceID,
											PDWORD pdwMemoryAddress,
											PDWORD pdwMemoryLength,
											PDWORD pdwSubSystemId)
{
	PCI_COMMON_CONFIG pciConfig;
	TDSDrvParam hwParam;
	DWORD dwStatus;
	DWORD dwLength;

	if (!isDriverOpened())
	{
		return ERROR_ACCESS_DENIED;
	}

	hwParam.dwAddress = dwVendorID;
	hwParam.dwValue = dwDeviceID;

	dwStatus = deviceControl(ioctlGetPCIInfo,
								&hwParam,
								sizeof(hwParam),
								&pciConfig,
								sizeof(pciConfig),
								&dwLength);

	if ( dwStatus == ERROR_SUCCESS)
	{
		//
		// Ok, here is the simplification, we use only the first address
		// For our current project this is enough (I hope so)
		//

		*pdwMemoryAddress = pciConfig.u.type0.BaseAddresses[0] & 0xFFFFFFF0;
		*pdwMemoryLength = 0x1000;
		*pdwSubSystemId = (pciConfig.u.type0.SubSystemID << 16) + pciConfig.u.type0.SubVendorID;
	}
	else
	{
		debugOut(dbTrace,"pciGetHardewareResource for %X %X failed",dwVendorID,dwDeviceID);

		*pdwMemoryAddress = 0;
		*pdwMemoryLength = 0;
		*pdwSubSystemId = 0;
	}

	return dwStatus;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver::DSDrvStartDriver(void)
{
	LPSTR  pszDriverName;
	DWORD  dwResult;
	LPSTR  pszName;
	char   szDriverPath[MAX_PATH];

	pszDriverName = ( bWindows95 ) ? "DSDrv95.VXD" : "DSDrvNT";


	if (!GetModuleFileName(NULL, szDriverPath, sizeof(szDriverPath)))
	{
		debugOut(dbError, "cannot get module file name");
		szDriverPath[0] = '\0';
	}

	pszName = szDriverPath + strlen(szDriverPath);
	while (pszName >= szDriverPath && *pszName != '\\')
	{
		*pszName-- = 0;
	}

	// JA 05/01/2001
	// changes directory to be the same as the exe file
	// should cure problems with shortcuts and macros
	SetCurrentDirectory(szDriverPath);

	dwResult = start( pszDriverName );
	if (dwResult != ERROR_SUCCESS && !bWindows95)
	{
		//
		// OK the driver service is not installed.
		// Build full driver path name and start the driver again.
		// We assume the driver.sys is located in the same path
		//

		strcat(szDriverPath,pszDriverName);
		strcat(szDriverPath,".sys");

		dwResult = start(pszDriverName, szDriverPath);

		if ( dwResult != ERROR_SUCCESS)
		{
			char MsgBuf[256];
			getErrorMessage(MsgBuf);
			debugOut(dbError,"install driver %s failed",pszDriverName);
			debugOut(dbError,MsgBuf);
		}
	}

	if(dwResult == ERROR_SUCCESS)
	{
		//
		// Its time to open the device
		//
		strcpy(szDriverPath,pszDriverName);

		dwResult = open(szDriverPath);
		if (  dwResult )
		{
			debugOut(dbError,"open driver failed");
		}
	}

	bDriverRunning = ( dwResult == ERROR_SUCCESS);

	return dwResult;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CdTVDriver::DSDrvStopDriver(void)
{
	close();

	bDriverRunning = FALSE;

	return stop();
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD loadDriver(void)
{
	return dTVDriver.DSDrvStartDriver(  );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD closeDriver(void)
{
	return dTVDriver.DSDrvStopDriver();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
extern "C" int
APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	// DLL is attaching to a process, due to process initialization or a
	// call to LoadLibrary
	case DLL_PROCESS_ATTACH:
		debugInitialize("DSDrv",0xFFFFFFFFL);
		debugOut(dbEnter,"DLL_PROCESS_ATTACH DSDrv.dll, handle %X %s",hInstance, "1.0");
		loadDriver();
		debugOut(dbExit,"DLL_PROCCES_ATTACH, exit");
		break;

	case DLL_PROCESS_DETACH:
		debugOut(dbEnter,"DLL_PROCESS_DETACH, handle %X",hInstance);
		closeDriver();
		debugOut(dbExit,"DLL_PROCESS_DETACH, exit");
		break;
	}

	return 1;   // ok
}
