/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file HardwareDriver.cpp CHardwareDriver Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "HardwareDriver.h"
#include "DebugLog.h"
#include <aclapi.h>
#include "ErrorBox.h"
#include "PathHelpers.h"

using namespace std;

// define this to force uninstallation of the NT driver on every destruction of the class.
//#define ALWAYS_UNINSTALL_NTDRIVER

// the access rights that are needed to just use DScaler (no (un)installation).
#define DRIVER_ACCESS_RIGHTS (SERVICE_START | SERVICE_STOP)

CHardwareDriver::CHardwareDriver()
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_hService = NULL;

    OSVERSIONINFO ov;

    ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &ov);
    m_bWindows95 = (ov.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
    m_WeStartedDriver = TRUE;
    _tcscpy(m_NTDriverName, _T("DSDrv4"));

    // we are a 32bit program possibly running on 64bit hardware
    // if this is the case then we need to load up an arch
    // specific driver, we just append the arch to the standard name
    const TCHAR* arch64Bit = _tgetenv(_T("PROCESSOR_ARCHITEW6432"));
    if(arch64Bit)
    {
        _tcscat(m_NTDriverName, arch64Bit);
    }
}

CHardwareDriver::~CHardwareDriver()
{
    // just in case the driver hasn't been closed properly
    UnloadDriver();

#if defined ALWAYS_UNINSTALL_NTDRIVER
    LOG(1,_T("(NT driver) ALWAYS_UNINSTALL_NTDRIVER"));
    UnInstallNTDriver();
#endif
}

BOOL CHardwareDriver::LoadDriver()
{
    SC_HANDLE hSCManager = NULL;
    BOOL      bError = FALSE;

    // make sure we start with nothing open
    UnloadDriver();

    if (!m_bWindows95)
    {
        // get handle of the Service Control Manager

        // This function fails on WinXP when not logged on as an administrator.
        // The following note comes from the updated Platform SDK documentation:

        // Windows 2000 and earlier: All processes are granted SC_MANAGER_CONNECT,
        // SC_MANAGER_ENUMERATE_SERVICE, and SC_MANAGER_QUERY_LOCK_STATUS access to all service control
        // manager databases. This enables any process to open a service control manager database handle
        // that it can use in the OpenService, EnumServicesStatus, and QueryServiceLockStatus functions.
        //
        // Windows XP: Only authenticated users are granted SC_MANAGER_CONNECT,
        // SC_MANAGER_ENUMERATE_SERVICE, and SC_MANAGER_QUERY_LOCK_STATUS access to all service control
        // manager databases.

        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if(hSCManager == NULL)
        {
            LOG(0, _T("OpenSCManager returned an error = 0x%X"), GetLastError());
            bError = TRUE;
        }

        if(!bError)
        {
            m_hService = OpenService(hSCManager, m_NTDriverName, DRIVER_ACCESS_RIGHTS);

            if(m_hService == NULL)
            {
                if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    LOG(1,_T("(NT driver) Service does not exist"));
                    if(!InstallNTDriver())
                    {
                        LOG(1,_T("Failed to install NT driver."));
                        bError = TRUE;
                    }
                }
                else
                {
                    LOG(0,_T("(NT driver) Unable to open service. 0x%X"),GetLastError());
                    bError = TRUE;
                }
            }
        }

        if(hSCManager != NULL)
        {
            if(!CloseServiceHandle(hSCManager))
            {
                LOG(0,_T("(NT driver) Failed to close handle to service control manager. 0x%X"),GetLastError());
                bError = TRUE;
            }
            hSCManager = NULL;
        }

        // try to start service
        if(!bError)
        {
            if(StartService(m_hService, 0, NULL) == FALSE)
            {
                DWORD Err = GetLastError();

                // uninstall the driver if the driver location in the registry is incorrect.
                if(Err == ERROR_PATH_NOT_FOUND || Err == ERROR_FILE_NOT_FOUND)
                {
                    LOG(0, _T("Driver location in the registry is incorrect.0x%X"), Err);

                    ErrorBox(_T("Please exit and restart DScaler"));

                    UnInstallNTDriver();
                    bError = TRUE;
                }
                else if(Err == ERROR_SERVICE_ALREADY_RUNNING)
                {
                    LOG(1, _T("Service already started"));
                    m_WeStartedDriver = FALSE;
                }
                else
                {
                    LOG(0, _T("StartService() failed 0x%X"), Err);
                    bError = TRUE;
                }
            }
            if(!bError)
            {
                m_hFile = CreateFile(
                                     _T("\\\\.\\DSDrv4"),
                                     GENERIC_READ | GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     INVALID_HANDLE_VALUE
                                    );
            }

        }

    }
    else
    {
        // it's much easier in windows 95, 98 and me
        m_hFile = CreateFile(
                                _T("\\\\.\\DSDrv4.VXD"),
                                0,
                                0,
                                NULL,
                                0,
                                FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE,
                                NULL
                            );
    }

    if(!bError)
    {
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
                        IOCTL_DSDRV_GETVERSION,
                        NULL,
                        0,
                        &dwVersion,
                        sizeof(dwVersion),
                        &dwReturnedLength
                       );

            // we should try and force the user to have the
            // latest possible driver installed
            if(dwVersion < DSDRV_VERSION)
            {
                ErrorBox(_T("Detected old driver version, rerun setup program"));
                LOG(0, _T("We've loaded up the wrong version of the driver"));
                bError = TRUE;

                // Maybe another driver from an old DScaler version is still installed.
                // Try to uninstall it.
                UnInstallNTDriver();
            }

            // if we create any incomaptibilities
            // in the driver may need to have more checks here
            if(dwVersion > DSDRV_VERSION)
            {
                LOG(0, _T("Running with newer driver - hopefully this will be OK"));
            }
        }
        else
        {
            LOG(0, _T("CreateFile on Driver failed 0x%X"), GetLastError());
            bError = TRUE;
        }
    }

    if(bError)
    {
        UnloadDriver();
        return FALSE;
    }
    else
    {
        LOG(1, _T("Hardware driver loaded successfully."));
        return TRUE;
    }
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
            if(m_WeStartedDriver)
            {
                SERVICE_STATUS ServiceStatus;
                if(ControlService(m_hService, SERVICE_CONTROL_STOP, &ServiceStatus ) == FALSE)
                {
                    LOG(0,_T("SERVICE_CONTROL_STOP failed, error 0x%X"), GetLastError());
                }
            }

            if(m_hService != NULL)
            {
                if(CloseServiceHandle(m_hService) == FALSE)
                {
                    LOG(0,_T("CloseServiceHandle failed, error 0x%X"), GetLastError());
                }
                m_hService = NULL;
            }
        }
    }
}

// On success m_hService will contain the handle to the service.
BOOL CHardwareDriver::InstallNTDriver()
{
    tstring      szDriverPath(GetInstallationPath());
    SC_HANDLE   hSCManager = NULL;
    BOOL        bError = FALSE;

    LOG(1, _T("Attempting to install NT driver."));

    UnloadDriver();

    if (m_bWindows95)
    {
        LOG(1, _T("No need to install NT driver with win9x/ME."));
        return TRUE;
    }

    if(!bError)
    {
        if(GetDriveType(szDriverPath.c_str()) == DRIVE_REMOTE)
        {
            ErrorBox(_T("The DScaler device driver can't be installed from a network drive."));
            bError = TRUE;
        }
    }

    if(!bError)
    {
        szDriverPath +=  _T("\\");
        szDriverPath += m_NTDriverName;
        szDriverPath += _T(".sys");

        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if(hSCManager == NULL)
        {
            LOG(0, _T("OpenSCManager returned an error = 0x%X"), GetLastError());
            bError = TRUE;
        }
    }

    if(!bError)
    {
        // Make sure no spaces exist in the path since CreateService() does not like spaces.
        szDriverPath.reserve(MAX_PATH);
        GetShortPathName(szDriverPath.c_str(), (LPTSTR)szDriverPath.c_str(), MAX_PATH);

        m_hService = CreateService(
            hSCManager,            // SCManager database
            m_NTDriverName,        // name of service
            m_NTDriverName,        // name to display
            SERVICE_ALL_ACCESS,    // desired access
            SERVICE_KERNEL_DRIVER, // service type
            SERVICE_DEMAND_START,  // start type
            SERVICE_ERROR_NORMAL,  // error control type
            szDriverPath.c_str(),  // service's binary
            NULL,                  // no load ordering group
            NULL,                  // no tag identifier
            NULL,                  // no dependencies
            NULL,                  // LocalSystem account
            NULL                   // no password
            );

        if(m_hService == NULL)
        {
            // if the service already exists delete it and create it again.
            // this might prevent problems when the existing service points to another driver.
            if(GetLastError() == ERROR_SERVICE_EXISTS)
            {
                m_hService = OpenService(hSCManager, m_NTDriverName, SERVICE_ALL_ACCESS);
                if(DeleteService(m_hService) == FALSE)
                {
                    LOG(0,_T("DeleteService failed, error 0x%X"), GetLastError());
                    bError = TRUE;
                }
                if(m_hService != NULL)
                {
                    CloseServiceHandle(m_hService);
                    m_hService = NULL;
                }
                if(!bError)
                {
                    m_hService = CreateService(
                        hSCManager,            // SCManager database
                        m_NTDriverName,        // name of service
                        m_NTDriverName,        // name to display
                        SERVICE_ALL_ACCESS,    // desired access
                        SERVICE_KERNEL_DRIVER, // service type
                        SERVICE_DEMAND_START,  // start type
                        SERVICE_ERROR_NORMAL,  // error control type
                        szDriverPath.c_str(),  // service's binary
                        NULL,                  // no load ordering group
                        NULL,                  // no tag identifier
                        NULL,                  // no dependencies
                        NULL,                  // LocalSystem account
                        NULL                   // no password
                        );
                    if(m_hService == NULL)
                    {
                        LOG(0,_T("(NT driver) CreateService #2 failed. 0x%X"), GetLastError());
                        bError = TRUE;
                    }
                }
            }
            else
            {
                LOG(0,_T("(NT driver) CreateService #1 failed. 0x%X"), GetLastError());
                bError = TRUE;
            }
        }
    }

    if(!bError)
    {
        if(!AdjustAccessRights())
        {
            bError = TRUE;
        }
    }

    if(hSCManager != NULL)
    {
        if(!CloseServiceHandle(hSCManager))
        {
            LOG(0, _T("(NT driver) Failed to close handle to service control manager. 0x%X"),GetLastError());
            bError = TRUE;
        }
        hSCManager = NULL;
    }

    if(bError)
    {
        LOG(1, _T("(NT driver) Failed to install driver."));
        UnloadDriver();
        return FALSE;
    }
    else
    {
        LOG(1, _T("(NT driver) Install complete."));
        return TRUE;
    }
}


BOOL CHardwareDriver::AdjustAccessRights()
{
    PSECURITY_DESCRIPTOR    psd = NULL;
    SECURITY_DESCRIPTOR     sd;
    DWORD                   dwSize = 0;
    DWORD                   dwError = 0;
    BOOL                    bError = FALSE;
    BOOL                    bDaclPresent = FALSE;
    BOOL                    bDaclDefaulted = FALSE;
    PACL                    pNewAcl = NULL;
    PACL                    pacl = NULL;
    EXPLICIT_ACCESS         ea;

    if(m_bWindows95)
    {
        return TRUE;
    }
    if(m_hService == NULL)
    {
        return FALSE;
    }

    // Find out how much memory to allocate for psd.
    // psd can't be NULL so we let it point to itself.

    psd = (PSECURITY_DESCRIPTOR)&psd;

    if(!QueryServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, psd, 0, &dwSize))
    {
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
            if(psd == NULL)
            {
                LOG(0, _T("HeapAlloc failed."));
                // note: HeapAlloc does not support GetLastError()
                bError = TRUE;
            }
        }
        else
        {
            LOG(0,_T("QueryServiceObjectSecurity #1 failed. 0x%X"), GetLastError());
            bError = TRUE;
        }
    }

    // Get the current security descriptor.
    if(!bError)
    {
        if(!QueryServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, psd,dwSize, &dwSize))
        {
            LOG(0,_T("QueryServiceObjectSecurity #2 failed. 0x%X"),GetLastError());
            bError = TRUE;
        }
    }

    // Get the DACL.
    if(!bError)
    {
        if(!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted))
        {
            LOG(0,_T("GetSecurityDescriptorDacl failed. 0x%X"),GetLastError());
            bError = TRUE;
        }
    }

    // Build the ACE.
    if(!bError)
    {
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
        PSID pSIDEveryone;

        // Create a SID for the Everyone group.
        if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0,
                     0, 0, 0, 0, 0, 0,
                     &pSIDEveryone))
        {
            LOG(0,_T("AllocateAndInitializeSid() failed. 0x%X"),GetLastError());
            bError = TRUE;
        }
        else
        {
            ea.grfAccessMode = SET_ACCESS;
            ea.grfAccessPermissions = DRIVER_ACCESS_RIGHTS;
            ea.grfInheritance = NO_INHERITANCE;
            ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            ea.Trustee.pMultipleTrustee = NULL;
            ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
            ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
            ea.Trustee.ptstrName = (TCHAR* )pSIDEveryone;

            dwError = SetEntriesInAcl(1, &ea, pacl, &pNewAcl);
            if(dwError != ERROR_SUCCESS)
            {
                LOG(0,_T("SetEntriesInAcl failed. %d"), dwError);
                bError = TRUE;
            }
        }
        FreeSid(pSIDEveryone);
    }

    // Initialize a new Security Descriptor.
    if(!bError)
    {
        if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
        {
            LOG(0,_T("InitializeSecurityDescriptor failed. 0x%X"),GetLastError());
            bError = TRUE;
        }
    }

    // Set the new DACL in the Security Descriptor.
    if(!bError)
    {
        if(!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))
        {
            LOG(0, _T("SetSecurityDescriptorDacl"), GetLastError());
            bError = TRUE;
        }
    }

    // Set the new DACL for the service object.
    if(!bError)
    {
        if (!SetServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, &sd))
        {
            LOG(0, _T("SetServiceObjectSecurity"), GetLastError());
            bError = TRUE;
        }
    }

    // Free buffers.
    LocalFree((HLOCAL)pNewAcl);
    HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

    return !bError;
}

BOOL CHardwareDriver::UnInstallNTDriver()
{
    SC_HANDLE hSCManager = NULL;
    BOOL      bError = FALSE;

    LOG(1, _T("Attempting to uninstall NT driver."));

    if (m_bWindows95)
    {
        LOG(1,_T("(NT driver) Uninstall not needed with win9x/ME."));
    }
    else
    {
        UnloadDriver();

        // get handle of the Service Control Manager
        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if(hSCManager == NULL)
        {
            LOG(0, _T("OpenSCManager returned an error = 0x%X"), GetLastError());
            bError = TRUE;
        }

        if(!bError)
        {
            m_hService = OpenService(hSCManager, m_NTDriverName, SERVICE_ALL_ACCESS);
            if(m_hService == NULL)
            {
                if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    LOG(0,_T("(NT driver) Service does not exist, no need to uninstall."));
                    CloseServiceHandle(m_hService);
                    m_hService = NULL;
                    return TRUE;
                }
                else
                {
                    LOG(0,_T("(NT driver) Unable to open service. 0x%X"),GetLastError());
                    bError = TRUE;
                }
            }
        }

        if(!bError)
        {
            SERVICE_STATUS ServiceStatus;
            ControlService(m_hService, SERVICE_CONTROL_STOP, &ServiceStatus );
        }

        if(hSCManager != NULL)
        {
            if(!CloseServiceHandle(hSCManager))
            {
                LOG(0,_T("(NT driver) Failed to close handle to service control manager. 0x%X"),GetLastError());
                bError = TRUE;
            }
            hSCManager = NULL;
        }

        if(!bError)
        {
            if (DeleteService(m_hService) == FALSE)
            {
                LOG(0,_T("DeleteService failed, error 0x%X"), GetLastError());
                bError = TRUE;
            }
        }

        if(m_hService != NULL)
        {
            if(CloseServiceHandle(m_hService) == FALSE)
            {
                LOG(0,_T("CloseServiceHandle failed, error 0x%X"), GetLastError());
                bError = TRUE;
            }
            m_hService = NULL;
        }

        if(bError)
        {
            UnloadDriver();
            LOG(1, _T("Failed to uninstall driver."));
            return FALSE;
        }
        LOG(1, _T("Uninstall NT driver complete."));
    }
    return TRUE;
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
        // Suppress the error when DoesThisPCICardExist() probes for a non-existing card
        if(dwIOCommand == IOCTL_DSDRV_GETPCIINFO)
        {
            LOG(2, _T("DeviceIoControl returned an error = 0x%X For Command ioctlGetPCIInfo. This is probably by design, do not worry."), GetLastError());
        }
        else
        {
            LOG(1, _T("DeviceIoControl returned an error = 0x%X For Command 0x%X"), GetLastError(), dwIOCommand);
        }
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
        LOG(1, _T("DeviceIoControl returned an error = 0x%X For Command 0x%X"), GetLastError(), dwIOCommand);
        return GetLastError();
    }
}

BOOL CHardwareDriver::DoesThisPCICardExist(WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD& SubSystemId)
{
    TDSDrvParam hwParam;
    DWORD dwStatus;
    DWORD dwLength;
    TPCICARDINFO PCICardInfo;

    hwParam.dwAddress = VendorID;
    hwParam.dwValue = DeviceID;
    hwParam.dwFlags = DeviceIndex;

    dwStatus = SendCommand(
                            IOCTL_DSDRV_GETPCIINFO,
                            &hwParam,
                            sizeof(hwParam),
                            &PCICardInfo,
                            sizeof(TPCICARDINFO),
                            &dwLength
                          );

    if(dwStatus == ERROR_SUCCESS)
    {
        SubSystemId = PCICardInfo.dwSubSystemId;
        return TRUE;
    }
    else
    {
        SubSystemId = 0;
        return FALSE;
    }
}

