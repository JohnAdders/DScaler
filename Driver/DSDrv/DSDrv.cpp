/////////////////////////////////////////////////////////////////////////////
// $Id: DSDrv.cpp,v 1.5 2001-08-08 16:37:49 adcockj Exp $
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
// 05 Jan 2001   John Adcock           Added SetCurrentDirectoryCall
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2001/08/08 10:53:30  adcockj
// Preliminary changes to driver to support multiple cards
//
// Revision 1.3  2001/07/13 16:13:53  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "KernelDriver.h"
#include "DSDrv.h"
#include "debugout.h"

class CDSDriver : public CKernelDriver
{
public:
    CDSDriver();

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
    DWORD pciGetHardwareResources(DWORD dwVendorID, DWORD  dwDeviceID, DWORD dwCardIndex, TPCICARDINFO* pPCICardInfo);
    DWORD memoryMap(DWORD dwBusNumber, DWORD dwAddress, DWORD dwLength);
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


CDSDriver DSDriver;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
int WINAPI isDriverOpened(void)
{
    return DSDriver.isDriverOpen();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BYTE WINAPI readPort(WORD address)
{
    return DSDriver.readPort( (DWORD) address);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
WORD WINAPI readPortW(WORD address)
{
    return DSDriver.readPortW( (DWORD) address);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI readPortL(WORD address)
{
    return DSDriver.readPortL( (DWORD) address);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI writePort(WORD address, BYTE bValue)
{
    DSDriver.writePort( (DWORD) address, (DWORD) bValue);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI writePortW(WORD address, WORD uValue)
{
    DSDriver.writePortW( (DWORD) address, (DWORD) uValue);
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI writePortL(WORD address, DWORD dwValue)
{
    DSDriver.writePortL( (DWORD) address, (DWORD) dwValue);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI memoryAlloc(DWORD dwLength, DWORD dwFlags, PMemStruct* ppMemStruct)
{
    return DSDriver.allocMemory(dwLength, dwFlags, ppMemStruct);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI memoryFree(PMemStruct pMemStruct)
{
    if(pMemStruct)
    {
        return DSDriver.freeMemory(pMemStruct);
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD WINAPI memoryMap(DWORD dwBusNumber, DWORD dwAddress, DWORD dwLength)
{
    return DSDriver.memoryMap(dwBusNumber, dwAddress, dwLength);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI memoryUnmap(DWORD dwAddress, DWORD dwLength)
{
    DSDriver.memoryUnmap(dwAddress, dwLength);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void WINAPI memoryWriteDWORD(DWORD dwAddress, DWORD dwValue)
{
    DSDriver.memoryWriteDWORD(dwAddress, dwValue);
}

DWORD WINAPI memoryReadDWORD(DWORD dwAddress)
{
    return DSDriver.memoryReadDWORD(dwAddress);
}

void WINAPI memoryWriteWORD(DWORD dwAddress, WORD wValue)
{
    DSDriver.memoryWriteWORD(dwAddress, wValue);
}

WORD WINAPI memoryReadWORD(DWORD dwAddress)
{
    return DSDriver.memoryReadWORD(dwAddress);
}

void WINAPI memoryWriteBYTE(DWORD dwAddress, BYTE ucValue)
{
    DSDriver.memoryWriteBYTE(dwAddress, ucValue);
}

BYTE WINAPI memoryReadBYTE(DWORD dwAddress)
{
    return DSDriver.memoryReadBYTE(dwAddress);
}


//---------------------------------------------------------------------------
// Get back the PCI hardware resources, we assumes that only mapped memory
// is avail.
//---------------------------------------------------------------------------
DWORD WINAPI pciGetHardwareResources(
                                         DWORD   dwVendorID,
                                         DWORD   dwDeviceID,
                                         DWORD   dwCardIndex,
                                         TPCICARDINFO* pPCICardInfo
                                     )
{
    return DSDriver.pciGetHardwareResources(
                                                dwVendorID,
                                                dwDeviceID,
                                                dwCardIndex,
                                                pPCICardInfo
                                            );
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CDSDriver::CDSDriver(void) : CKernelDriver()
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
BYTE CDSDriver::readPort(DWORD dwAddress)
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
WORD CDSDriver::readPortW(DWORD dwAddress)
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
DWORD CDSDriver::readPortL(DWORD dwAddress)
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
void CDSDriver::writePort(DWORD dwAddress, DWORD dwValue)
{
    TDSDrvParam  param;

    param.dwAddress = dwAddress;
    param.dwValue = dwValue;

    deviceControl(ioctlWriteBYTE, &param, sizeof(param));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CDSDriver::writePortW(DWORD dwAddress, DWORD dwValue)
{
    TDSDrvParam  param;

    param.dwAddress = dwAddress;
    param.dwValue = dwValue;

    deviceControl(ioctlWriteWORD, &param, sizeof(param));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CDSDriver::writePortL(DWORD dwAddress, DWORD dwValue)
{
    TDSDrvParam  param;

    param.dwAddress = dwAddress;
    param.dwValue = dwValue;

    deviceControl(ioctlWriteDWORD, &param, sizeof(param));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CDSDriver:: allocMemory(DWORD dwLength, DWORD dwFlags, PMemStruct* ppMemStruct)
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
DWORD CDSDriver::freeMemory(PMemStruct pMemStruct)
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
DWORD CDSDriver::memoryMap(DWORD dwBusNumber, DWORD dwAddress, DWORD dwLength)
{
    TDSDrvParam hwParam;
    DWORD       dwMappedAddress;
    DWORD       dwReturnedLength;

    hwParam.dwAddress = dwBusNumber;
    hwParam.dwValue   = dwAddress;
    dwMappedAddress = dwLength;

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
void CDSDriver::memoryUnmap(DWORD dwAddress, DWORD dwLength)
{
    TDSDrvParam hwParam;

    hwParam.dwAddress = dwAddress;
    hwParam.dwValue   = dwLength;

    deviceControl(ioctlUnmapMemory, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CDSDriver::memoryWriteDWORD(DWORD dwAddress, DWORD dwValue)
{
    TDSDrvParam hwParam;

    hwParam.dwAddress = dwAddress;
    hwParam.dwValue = dwValue;

    deviceControl(ioctlWriteMemoryDWORD, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CDSDriver::memoryReadDWORD(DWORD dwAddress)
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
void CDSDriver::memoryWriteWORD(DWORD dwAddress, WORD wValue)
{
    TDSDrvParam hwParam;

    hwParam.dwAddress = dwAddress;
    hwParam.dwValue = wValue;

    deviceControl(ioctlWriteMemoryWORD, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
WORD CDSDriver::memoryReadWORD(DWORD dwAddress)
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
void CDSDriver::memoryWriteBYTE(DWORD dwAddress, BYTE ucValue)
{
    TDSDrvParam hwParam;

    hwParam.dwAddress = dwAddress;
    hwParam.dwValue = ucValue;

    deviceControl(ioctlWriteMemoryBYTE, &hwParam, sizeof(hwParam));
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BYTE CDSDriver::memoryReadBYTE(DWORD dwAddress)
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    BYTE ucValue(0);

    hwParam.dwAddress = dwAddress;
    hwParam.dwValue = dwAddress;
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
DWORD CDSDriver::pciGetHardwareResources(
                                            DWORD   dwVendorID,
                                            DWORD   dwDeviceID,
                                            DWORD   dwCardIndex,
                                            TPCICARDINFO* pPCICardInfo
                                        )
{
    TDSDrvParam hwParam;
    DWORD dwStatus;
    DWORD dwLength;

    if (!isDriverOpened())
    {
        return ERROR_ACCESS_DENIED;
    }

    hwParam.dwAddress = dwVendorID;
    hwParam.dwValue = dwDeviceID;
    hwParam.dwFlags = dwCardIndex;

    dwStatus = deviceControl(ioctlGetPCIInfo,
                                &hwParam,
                                sizeof(hwParam),
                                pPCICardInfo,
                                sizeof(TPCICARDINFO),
                                &dwLength);

    if ( dwStatus != ERROR_SUCCESS)
    {
        debugOut(dbTrace,"pciGetHardwareResource for %X %X failed",dwVendorID, dwDeviceID);
        memset(pPCICardInfo, 0, sizeof(TPCICARDINFO));
    }

    return dwStatus;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CDSDriver::DSDrvStartDriver(void)
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

    if(dwResult == ERROR_SUCCESS)
    {
        // OK so we've loaded the driver 
        // we had better check that it's the same version as we are
        // otherwise all sorts of nasty things could happen
        // n.b. note that if someone else has already loaded our driver this may
        // happen.  Hopefully any highup apps will just get the
        // access denied message and then fail reasonable gracefully
        DWORD dwReturnedLength;
        DWORD dwVersion(0);

        deviceControl(ioctlGetVersion,
                        NULL,
                        0,
                        &dwVersion,
                        sizeof(dwVersion),
                        &dwReturnedLength);


        if(dwVersion != DSDRV_VERSION)
        {
            DSDrvStopDriver();
            dwResult = ERROR_PRODUCT_VERSION;
        }
    }
    bDriverRunning = (dwResult == ERROR_SUCCESS);

    return dwResult;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CDSDriver::DSDrvStopDriver(void)
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
    return DSDriver.DSDrvStartDriver(  );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD closeDriver(void)
{
    return DSDriver.DSDrvStopDriver();
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
