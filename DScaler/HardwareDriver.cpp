/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareDriver.cpp,v 1.5 2001-11-23 10:49:17 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.2.2.3  2001/08/15 07:10:19  adcockj
// Fixed memory leak
//
// Revision 1.2.2.2  2001/08/14 21:25:57  adcockj
// Bug fixes to get new version working
//
// Revision 1.2.2.1  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.2  2001/08/13 12:05:12  adcockj
// Updated range for contrast and saturation
// Added more code for new driver interface
//
// Revision 1.1  2001/08/09 16:44:50  adcockj
// Added extra files (Unused) for better hardware handling
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "HardwareDriver.h"
#include "DebugLog.h"

static const LPSTR NTDriverName = "DSDrv4";

CHardwareDriver::CHardwareDriver()
{
    m_hSCManager = NULL;
    m_hFile = INVALID_HANDLE_VALUE;
    m_hService = NULL;

    OSVERSIONINFO ov;

    ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &ov);
    m_bWindows95 = (ov.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
}

CHardwareDriver::~CHardwareDriver()
{
    // just in case the driver hasn't been closed properly
    UnloadDriver();
}

BOOL CHardwareDriver::LoadDriver()
{
    // make sure we start with nothing open
    UnloadDriver();

    if (!m_bWindows95)
    {
        // get handle of the Service Control Manager
        m_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if(m_hSCManager == NULL)
        {
            LOG(1, "OpenSCManager returned an error = 0x%x", GetLastError());
            return FALSE;
        }

        m_hService = OpenService(m_hSCManager, NTDriverName, SERVICE_ALL_ACCESS);
        if(m_hService == NULL)
        {
            LPSTR  pszName;
            char   szDriverPath[MAX_PATH];

            if (!GetModuleFileName(NULL, szDriverPath, sizeof(szDriverPath)))
            {
                LOG(1, "cannot get module file name");
                szDriverPath[0] = '\0';
            }

            pszName = szDriverPath + strlen(szDriverPath);
            while (pszName >= szDriverPath && *pszName != '\\')
            {
                *pszName-- = 0;
            }

            strcat(szDriverPath, NTDriverName);
            strcat(szDriverPath, ".sys");

            m_hService = CreateService(
                                        m_hSCManager,          // SCManager database
                                        NTDriverName,             // name of service
                                        NTDriverName,             // name to display
                                        SERVICE_ALL_ACCESS,    // desired access
                                        SERVICE_KERNEL_DRIVER, // service type
                                        SERVICE_DEMAND_START,  // start type
                                        SERVICE_ERROR_NORMAL,  // error control type
                                        szDriverPath,          // service's binary
                                        NULL,                  // no load ordering group
                                        NULL,                  // no tag identifier
                                        NULL,                  // no dependencies
                                        NULL,                  // LocalSystem account
                                        NULL                   // no password
                                      );

            if (m_hService == NULL)
            {
                LOG(1, "CreateService() failed %X", GetLastError());
                UnloadDriver();
                return FALSE;
            }
        }
        else
        {
            LOG(1, "Service already exists");
        }
        
        // try to start service
        if(StartService(m_hService, 0, NULL) == FALSE)
        {
            DWORD Err = GetLastError();
            if(Err != ERROR_SERVICE_ALREADY_RUNNING)
            {
                LOG(1, "StartService() failed %X", Err);
                UnloadDriver();
                return FALSE;
            }
            else
            {
                LOG(1, "Service already started");
            }
        }

        m_hFile = CreateFile(
                                "\\\\.\\DSDrv4",
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                INVALID_HANDLE_VALUE
                            );
    
    }
    else
    {
        // it's much easier in windows 95, 98 and me
        m_hFile = CreateFile(
                                "\\\\.\\DSDrv4.VXD",
                                0,
                                0,
                                NULL,
                                0,
                                FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE,
                                NULL
                            );
    }

    if(m_hFile != INVALID_HANDLE_VALUE)
    {
        // OK so we've loaded the driver 
        // we had better check that it's the same version as we are
        // otherwise all sorts of nasty things could happen
        // n.b. note that if someone else has already loaded our driver this may
        // happen.

        DWORD dwReturnedLength;
        DWORD dwVersion(0);

        SendCommand(
                    ioctlGetVersion,
                    NULL,
                    0,
                    &dwVersion,
                    sizeof(dwVersion),
                    &dwReturnedLength
                   );


        if(dwVersion != DSDRV_VERSION)
        {
            LOG(1, "We've loaded up the wrong version of the driver");
            UnloadDriver();
            return FALSE;
        }
    }
    else
    {
        LOG(1, "CreateFile on Driver failed %X", GetLastError());
        UnloadDriver();
        return FALSE;
    }
    return TRUE;
}

// must make sure that this function copes with multiple calls
void CHardwareDriver::UnloadDriver()
{
    if(m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    if (!m_bWindows95)
    {
        if (m_hService != NULL)
        {
            SERVICE_STATUS ServiceStatus;
            if(ControlService(m_hService, SERVICE_CONTROL_STOP, &ServiceStatus ) == FALSE)
            {
                LOG(1,"SERVICE_CONTROL_STOP failed, error 0x%x", GetLastError());
            }

            if (DeleteService(m_hService) == FALSE)
            {
                LOG(1,"DeleteService failed, error 0x%x", GetLastError());
            }

            if(CloseServiceHandle(m_hService) == FALSE)
            {
                LOG(1,"CloseServiceHandle failed, error 0x%x", GetLastError());
            }
            m_hService = NULL;
        }

        if (m_hSCManager)
        {
            CloseServiceHandle(m_hSCManager);
            m_hSCManager = NULL;
        }
    }
}

DWORD CHardwareDriver::SendCommand(
                                    DWORD dwIOCommand,
                                    LPVOID pvInput,
                                    DWORD dwInputLength,
                                    LPVOID pvOutput,
                                    DWORD dwOutputLength,
                                    LPDWORD pdwReturnedLength
                                  )
{
    if(DeviceIoControl(
                        m_hFile,
                        dwIOCommand,
                        pvInput,
                        dwInputLength,
                        pvOutput,
                        dwOutputLength,
                        pdwReturnedLength,
                        NULL
                      ))
    {
        return 0;
    }
    else
    {
        LOG(1, "DeviceIoControl returned an error = 0x%x For Command 0x%x", GetLastError(), dwIOCommand);
        return GetLastError();
    }
}

DWORD CHardwareDriver::SendCommand(
                                    DWORD dwIOCommand,
                                    LPVOID pvInput,
                                    DWORD dwInputLength
                                  )
{
    DWORD dwDummy;

    if(DeviceIoControl(
                        m_hFile,
                        dwIOCommand,
                        pvInput,
                        dwInputLength,
                        NULL,
                        0,
                        &dwDummy,
                        NULL
                      ))
    {
        return 0;
    }
    else
    {
        LOG(1, "DeviceIoControl returned an error = 0x%x For Command 0x%x", GetLastError(), dwIOCommand);
        return GetLastError();
    }
}
