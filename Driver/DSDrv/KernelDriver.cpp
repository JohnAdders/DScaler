/////////////////////////////////////////////////////////////////////////////
// $Id: KernelDriver.cpp,v 1.2 2001-07-13 16:13:53 adcockj Exp $
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
// 15 Apr 1995   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "KernelDriver.h"
#include "debugout.h"


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CKernelDriver::CKernelDriver(void) : 
        CWin32Object(),
        serviceFlag(SERVICE_ALL_ACCESS)
{
    setError (ERROR_SUCCESS);

    schSCManager    = NULL;
    kdFileHandle    = NULL;
    kdDriverName    = NULL;
    kdDOSDeviceName = NULL;

    OSVERSIONINFO ov;

    ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx( &ov);
    bWindows95 = (ov.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
CKernelDriver::~CKernelDriver()
{
    if ( kdFileHandle )
    {
        close();
    }

    if (kdDOSDeviceName)
    {
        //
        // The driver wasn't visible in the Win32 name space so we created
        // a link.  Now we have to delete it.
        //

        if(!DefineDosDevice(DDD_RAW_TARGET_PATH | DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE,
                            kdDriverName, kdDOSDeviceName) )
        {
            debugOut(dbMessage,"DefineDosDevice returned an error removing the device = 0x%x",::GetLastError());
        }

        delete kdDOSDeviceName;
    }

    stop();

    if (schSCManager)
    {
        CloseServiceHandle (schSCManager);
        schSCManager = NULL;
    }
}

//---------------------------------------------------------------------------
// Install and start a driver
//---------------------------------------------------------------------------
DWORD CKernelDriver::start(LPSTR pszDriverName, LPSTR pszBinaryPathName)
{
    if(bWindows95)
    {
        setError( ERROR_SUCCESS );
    }
    else
    {
        if(openSCM())
        {
            if(installDriver(pszDriverName,pszBinaryPathName) == ERROR_SUCCESS)
            {
                startDriver(pszDriverName);
            }
        }
    }
    return getLastError();
}


//---------------------------------------------------------------------------
// Start a installed driver.
//---------------------------------------------------------------------------
DWORD CKernelDriver::start(LPSTR pszDriverName)
{
    if ( bWindows95 )
    {
        setError( ERROR_SUCCESS );
    }
    else
    {
        if ( openSCM() )
        {
            startDriver(pszDriverName);
        }
    }
    return getLastError();
}

//---------------------------------------------------------------------------
// Open kernel device as file
//---------------------------------------------------------------------------
DWORD CKernelDriver::open(LPSTR pszDeviceName)
{
    if (!kdFileHandle)
    {
        kdFileHandle = openDevice( pszDeviceName );
    }
    else
    {
        debugOut(dbTrace,"device already open");
    }

    return getLastError();
}

//---------------------------------------------------------------------------
// Close kernel device file
//---------------------------------------------------------------------------
DWORD CKernelDriver::close(void)
{
    if (kdFileHandle)
    {
        debugOut(dbTrace,"Close file handle %X",kdFileHandle);

        CloseHandle( kdFileHandle );
        kdFileHandle = NULL;
    }

    return ERROR_SUCCESS;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CKernelDriver::read(PVOID pvBuffer, DWORD dwBufferSize, LPDWORD pdwBytesReading)
{
    if (!ReadFile(kdFileHandle,
                  pvBuffer,
                  dwBufferSize,
                  pdwBytesReading,
                  NULL))
    {
        setError();
    }
    else
    {
        setError(ERROR_SUCCESS);
    }

    return getLastError();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD
CKernelDriver::write(PVOID pvBuffer, DWORD dwBufferSize)
{
    DWORD dwBytesWritten;

    if (!WriteFile(kdFileHandle,
                    pvBuffer,
                    dwBufferSize,
                    &dwBytesWritten,
                    NULL))
    {
        setError();
    }
    else
    {
        setError(ERROR_SUCCESS);
    }

    return getLastError();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CKernelDriver::deviceControl(DWORD dwIOCommand,
                                    LPVOID pvInput,
                                    DWORD dwInputLength,
                                    LPVOID pvOutput,
                                    DWORD dwOutputLength,
                                    LPDWORD pdwReturnedLength)
{
    if(!DeviceIoControl(kdFileHandle,
                        dwIOCommand,
                        pvInput,
                        dwInputLength,
                        pvOutput,
                        dwOutputLength,
                        pdwReturnedLength,
                        NULL))
    {
        setError();
    }
    else
    {
        setError(ERROR_SUCCESS);
    }

    return getLastError();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CKernelDriver::deviceControl(DWORD dwIOCommand, LPVOID pvInput, DWORD dwInputLength)
{
    DWORD dwDummy;

    if (!DeviceIoControl(kdFileHandle,
                        dwIOCommand,
                        pvInput,
                        dwInputLength,
                        NULL,
                        0,
                        &dwDummy,
                        NULL))
    {
        setError();
        debugOut(dbError," DeviceIoControl() failed %X",getLastError());
    }
    else
    {
        setError(ERROR_SUCCESS);
    }

    return getLastError();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CKernelDriver::stop(void)
{
    DWORD dwResult;

    if (!kdDriverName)
    {
        return ERROR_SUCCESS;
    }

    dwResult = stop(kdDriverName, TRUE);

    kdDriverName = NULL;

    return dwResult;
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CKernelDriver::stop(LPSTR pszDriverName, BOOL bRemove)
{
    if (bWindows95)
    {
        setError(ERROR_SUCCESS);
    }
    else
    {
        if (openSCM())
        {
            if (stopDriver(pszDriverName) == ERROR_SUCCESS)
            {
                if (bRemove)
                {
                    removeDriver(pszDriverName);
                }
            }
        }
    }

    return getLastError();
}



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
SC_HANDLE CKernelDriver::openService(LPSTR pszDriverName)
{
    SC_HANDLE schService;

    schService = ::OpenService(schSCManager, pszDriverName, serviceFlag);
    if (schService == NULL)
    {
        setError();
        debugOut(dbError,"OpenService() <%s> failed %X",pszDriverName,getLastError());
    }
    else
    {
        setError( ERROR_SUCCESS);
    }

    return schService;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CKernelDriver::installDriver(LPSTR pszDriverName, LPSTR pszBinaryPathName)
{
    SC_HANDLE  schService;

    if ( bWindows95 )
    {
        return ERROR_SUCCESS;
    }

    debugOut(dbTrace,"CKernelDriver::installDriver(), <%s> <%s>",pszDriverName,pszBinaryPathName);

    //
    // NOTE: This creates an entry for a standalone driver. If this
    //       is modified for use with a driver that requires a Tag,
    //       Group, and/or Dependencies, it may be necessary to
    //       query the registry for existing driver information
    //       (in order to determine a unique Tag, etc.).
    //


    schService = ::CreateService (schSCManager,          // SCManager database
                                   pszDriverName,         // name of service
                                   pszDriverName,         // name to display
                                   SERVICE_ALL_ACCESS,    // desired access
                                   SERVICE_KERNEL_DRIVER, // service type
                                   SERVICE_DEMAND_START,  // start type
                                   SERVICE_ERROR_NORMAL,  // error control type
                                   pszBinaryPathName,     // service's binary
                                   NULL,                  // no load ordering group
                                   NULL,                  // no tag identifier
                                   NULL,                  // no dependencies
                                   NULL,                  // LocalSystem account
                                   NULL);                   // no password

    if (schService == NULL)
    {
        setError();
        debugOut(dbError,"CreateService() <%s> failed %X",pszDriverName,getLastError());
    }
    else
    {
        debugOut(dbError,"driver <%s> successfully installed",pszDriverName);
        setError( ERROR_SUCCESS);
        CloseServiceHandle (schService);
    }

    if ( getLastError() == ERROR_SERVICE_EXISTS)
    {
        debugOut(dbTrace," service exists, assume no error");
        setError(ERROR_SUCCESS);
    }

    return getLastError();
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CKernelDriver::removeDriver(LPSTR pszDriverName)
{
    SC_HANDLE  schService;

    if (!bWindows95)
    {
        debugOut(dbTrace,"CKernelDriver::removeDriver(), <%s>",pszDriverName);

        schService = openService(pszDriverName);

        if (schService)
        {
            if (::DeleteService(schService) == FALSE)
            {
                setError();
                debugOut(dbError,"DeleteService() failed %X",getLastError() );
            }

            ::CloseServiceHandle(schService);
        }
    }
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void CKernelDriver::startDriver(LPSTR pszDriverName)
{
    SC_HANDLE schService;

    if (!bWindows95)
    {
        debugOut(dbTrace,"CKernelDriver::startDriver(), <%s>",pszDriverName);

        schService = openService(pszDriverName);

        if (schService)
        {
            if (::StartService(schService,    // service identifier
                                0,             // number of arguments
                                NULL)           // pointer to arguments
                    == FALSE)
            {
                setError();
            }

            switch (getLastError())
            {
            case ERROR_SERVICE_ALREADY_RUNNING:
                debugOut(dbTrace,"StartService(), service already running");
                setError( ERROR_SUCCESS);
                // fall trough
            case ERROR_SUCCESS:
                kdDriverName = pszDriverName;
                break;

            default:
                debugOut(dbError, "StartService() <%s> failed, %X",pszDriverName,getLastError());
                break;
            }       

            ::CloseServiceHandle (schService);
        }
    }
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD CKernelDriver::stopDriver(LPSTR pszDriverName)
{
    SC_HANDLE      schService;
    SERVICE_STATUS serviceStatus;

    if (bWindows95)
    {
        return ERROR_SUCCESS;
    }

    debugOut(dbTrace,"CKernelDriver::stopDriver(), <%s>",pszDriverName);

    schService = openService(pszDriverName);

    if (schService)
    {
        if (::ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus ) == FALSE)
        {
            setError();
            debugOut(dbError,"SERVICE_CONTROL_STOP failed, error %lX",getLastError());
        }
        else
        {
            setError(ERROR_SUCCESS);
        }

        ::CloseServiceHandle(schService);
    }

    return getLastError();
}

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HANDLE CKernelDriver::openDevice(LPSTR pszDriverName)
{
    char szLinkName[MAX_PATH];

    if ( ! bWindows95)
    {
        //
        // Check to see if the DOS name for the device driver already exists.
        // Its not created automatically in version 3.1 but may be later.
        //
        setError(ERROR_SUCCESS);

        if (QueryDosDevice(pszDriverName, (LPTSTR) szLinkName, sizeof(szLinkName)))
        {
            debugOut(dbMessage,"DOS device <%s> exist",szLinkName);
        }
        else
        {
            setError();
            if (getLastError() == ERROR_FILE_NOT_FOUND)
            {
                debugOut(dbMessage,"DOS device <%s> not exist",pszDriverName);
                kdDOSDeviceName = new char [ strlen(pszDriverName) + 32 ];
                if (!kdDOSDeviceName)
                {
                    setError( ERROR_NOT_ENOUGH_MEMORY);
                    return NULL;
                }

                strcpy(kdDOSDeviceName, "\\Device\\");
                strcat(kdDOSDeviceName, pszDriverName);

                debugOut(dbMessage,"define a dos device <%s>",kdDOSDeviceName);

                //
                // It doesn't exist so create it.
                //

                if (!::DefineDosDevice(DDD_RAW_TARGET_PATH, pszDriverName, kdDOSDeviceName))
                {
                    setError();
                    debugOut(dbMessage,"DefineDosDevice returned an error creating the device = %d", getLastError() );

                    delete kdDOSDeviceName;
                    kdDOSDeviceName = NULL;

                    return NULL;
                }
            }
            else
            {
                setError();
                debugOut(dbTrace,"QueryDosDevice <%s> returned an error = %d",pszDriverName, getLastError() );
                return NULL;
            }
        }
    }

    //
    // Construct a device name to pass to CreateFile
    //

    strcpy(szLinkName, DEVICE_PREFIX);
    strcat(szLinkName, pszDriverName);

    debugOut(dbMessage,"Open file with link name <%s>",szLinkName);

    return openFile( szLinkName );
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
HANDLE CKernelDriver::openFile(LPSTR pszFileName)
{
    HANDLE hDevice;

    if(bWindows95)
    {
        hDevice = CreateFile(pszFileName,
                                0,
                                0,
                                NULL,
                                0,
                                FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE,
                                NULL);
    }
    else
    {
        hDevice = CreateFile(pszFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                INVALID_HANDLE_VALUE);
    }

    if(hDevice == INVALID_HANDLE_VALUE)
    {
        setError();
        hDevice = NULL;
        debugOut(dbMessage,"open file failed %X", getLastError());
    }
    else
    {
        debugOut(dbMessage," file %s is open, handle %X",pszFileName,hDevice);
        setError ( ERROR_SUCCESS );
    }
    
    return hDevice;
}


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
BOOL CKernelDriver::openSCM(void)
{
    if (bWindows95) return TRUE;

    if (!schSCManager)
    {
        //
        // Open the service control manager data base
        //
        schSCManager = ::OpenSCManager(NULL,                    // machine (NULL == local)
                                        NULL,                   // database (NULL == default)
                                        SC_MANAGER_ALL_ACCESS);   // access required
        if (!schSCManager)
        {
            setError();
        }
    }

    return schSCManager ? TRUE : FALSE;
}





