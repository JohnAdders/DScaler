/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareDriver.cpp,v 1.16 2002-08-10 10:25:41 robmuller Exp $
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
// Revision 1.15  2002/07/20 11:43:32  robmuller
// Suppress error in log file when DoesThisPCICardExist() probes for a non-existing card.
//
// Revision 1.14  2002/07/02 19:59:02  adcockj
// Made a bit less strict with driver versions - should be happy with newer drivers
//
// Revision 1.13  2002/06/14 12:18:07  adcockj
// Fix to allow multiple programs access to the driver
//
// Revision 1.12  2002/05/27 11:27:37  robmuller
// Uninstall the driver if the driver location in the registry is incorrect.
//
// Revision 1.11  2002/05/27 07:22:09  robmuller
// Corrected detection of network drives.
// Use short file name for driver passed to CreateService().
//
// Revision 1.10  2002/05/26 19:04:13  robmuller
// Implemented debug log level 0 (for critical errors).
//
// Revision 1.9  2002/05/25 11:56:56  robmuller
// Show an error message if driver installation from a network drive is attempted.
//
// Revision 1.8  2002/02/13 16:37:16  tobbej
// fixed so LoadDriver dont fail if the service is already running
//
// Revision 1.7  2002/02/03 22:47:31  robmuller
// Added (un)installation of the NT/2000/XP driver, this includes code to change
// the access rights. The driver is not automatically deleted on exit anymore.
//
// Revision 1.6  2001/11/29 17:30:52  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.5  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
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
#include "aclapi.h"

// define this to force uninstallation of the NT driver on every destruction of the class.
//#define ALWAYS_UNINSTALL_NTDRIVER

static const LPSTR NTDriverName = "DSDrv4";

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
}

CHardwareDriver::~CHardwareDriver()
{
    // just in case the driver hasn't been closed properly
    UnloadDriver();

#if defined ALWAYS_UNINSTALL_NTDRIVER
    LOG(1,"(NT driver) ALWAYS_UNINSTALL_NTDRIVER");
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
        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if(hSCManager == NULL)
        {
            LOG(0, "OpenSCManager returned an error = 0x%X", GetLastError());
            bError = TRUE;
        }

        if(!bError)
        {
            m_hService = OpenService(hSCManager, NTDriverName, DRIVER_ACCESS_RIGHTS);

            if(m_hService == NULL)
            {
                if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    LOG(1,"(NT driver) Service does not exist");
                    if(!InstallNTDriver())
                    {
                        LOG(1,"Failed to install NT driver.");
                        bError = TRUE;
                    }
                }
                else
                {
                    LOG(0,"(NT driver) Unable to open service. 0x%X",GetLastError());
                    bError = TRUE;
                }
            }
        }

        if(hSCManager != NULL)
        {
            if(!CloseServiceHandle(hSCManager))
            {
                LOG(0,"(NT driver) Failed to close handle to service control manager. 0x%X",GetLastError());
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
                    LOG(0, "Driver location in the registry is incorrect.0x%X", Err);

                    ErrorBox("Please exit and restart DScaler");

                    UnInstallNTDriver();
                    bError = TRUE;
                }
                else if(Err == ERROR_SERVICE_ALREADY_RUNNING)
                {
                    LOG(1, "Service already started");
                }
                else
                {
                    LOG(0, "StartService() failed 0x%X", Err);
                    bError = TRUE;
                }
            }
            if(!bError)
            {
                m_hFile = CreateFile(
                                     "\\\\.\\DSDrv4",
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
                                "\\\\.\\DSDrv4.VXD",
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
                        ioctlGetVersion,
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
                ErrorBox("Detected old driver version, rerun setup program");
                LOG(0, "We've loaded up the wrong version of the driver");
                bError = TRUE;

                // Maybe another driver from an old DScaler version is still installed. 
                // Try to uninstall it.
                UnInstallNTDriver();
            }

            // if we create any incomaptibilities
            // in the driver may need to have more checks here
            if(dwVersion > DSDRV_VERSION)
            {
                LOG(0, "Running with newer driver - hopefully this will be OK");
            }
        }
        else
        {
            LOG(0, "CreateFile on Driver failed 0x%X", GetLastError());
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
        LOG(1, "Hardware driver loaded successfully.");
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
            SERVICE_STATUS ServiceStatus;
            if(ControlService(m_hService, SERVICE_CONTROL_STOP, &ServiceStatus ) == FALSE)
            {
                LOG(0,"SERVICE_CONTROL_STOP failed, error 0x%X", GetLastError());
            }

            if(m_hService != NULL)
            {
                if(CloseServiceHandle(m_hService) == FALSE)
                {
                    LOG(0,"CloseServiceHandle failed, error 0x%X", GetLastError());
                }
                m_hService = NULL;
            }
        }
    }
}

// On success m_hService will contain the handle to the service.
BOOL CHardwareDriver::InstallNTDriver()
{
    LPSTR       pszName;
    char        szDriverPath[MAX_PATH];
    SC_HANDLE   hSCManager = NULL;
    BOOL        bError = FALSE;

    LOG(1, "Attempting to install NT driver.");

    UnloadDriver();

    if (m_bWindows95)
    {
        LOG(1, "No need to install NT driver with win9x/ME.");
        return TRUE;
    }

    if (!GetModuleFileName(NULL, szDriverPath, sizeof(szDriverPath)))
    {
        LOG(0, "cannot get module file name. 0x%X",GetLastError());
        szDriverPath[0] = '\0';
        bError = TRUE;
    }
    
    if(!bError)
    {
        pszName = szDriverPath + strlen(szDriverPath);
        while (pszName >= szDriverPath && *pszName != '\\')
        {
            *pszName-- = 0;
        }

        if(GetDriveType(szDriverPath) == DRIVE_REMOTE)
        {
            ErrorBox("The DScaler device driver can't be installed from a network drive.");
            bError = TRUE;
        }
    }

    if(!bError)
    {
        strcat(szDriverPath, NTDriverName);
        strcat(szDriverPath, ".sys");       
        
        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if(hSCManager == NULL)
        {
            LOG(0, "OpenSCManager returned an error = 0x%X", GetLastError());
            bError = TRUE;
        }   
    }
    
    if(!bError)
    {
        // Make sure no spaces exist in the path since CreateService() does not like spaces.
        GetShortPathName(szDriverPath, szDriverPath, MAX_PATH);

        m_hService = CreateService(
            hSCManager,            // SCManager database
            NTDriverName,          // name of service
            NTDriverName,          // name to display
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
        
        if(m_hService == NULL)
        {
            // if the service already exists delete it and create it again.
            // this might prevent problems when the existing service points to another driver.
            if(GetLastError() == ERROR_SERVICE_EXISTS)
            {              
                m_hService = OpenService(hSCManager, NTDriverName, SERVICE_ALL_ACCESS);
                if(DeleteService(m_hService) == FALSE)
                {
                    LOG(0,"DeleteService failed, error 0x%X", GetLastError());
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
                        NTDriverName,          // name of service
                        NTDriverName,          // name to display
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
                    if(m_hService == NULL)
                    {
                        LOG(0,"(NT driver) CreateService #2 failed. 0x%X", GetLastError());
                        bError = TRUE;
                    }
                }
            }
            else
            {
                LOG(0,"(NT driver) CreateService #1 failed. 0x%X", GetLastError());
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
            LOG(0, "(NT driver) Failed to close handle to service control manager. 0x%X",GetLastError());
            bError = TRUE;
        }
        hSCManager = NULL;
    }
    
    if(bError)
    {
        LOG(1, "(NT driver) Failed to install driver.");
        UnloadDriver();
        return FALSE;
    }
    else
    {
        LOG(1, "(NT driver) Install complete.");
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
                LOG(0, "HeapAlloc failed.");
                // note: HeapAlloc does not support GetLastError()
                bError = TRUE;
            }
        }
        else
        {
            LOG(0,"QueryServiceObjectSecurity #1 failed. 0x%X", GetLastError());
            bError = TRUE;
        }                
    }
    
    // Get the current security descriptor.
    if(!bError)
    {
        if(!QueryServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, psd,dwSize, &dwSize))
        {
            LOG(0,"QueryServiceObjectSecurity #2 failed. 0x%X",GetLastError());
            bError = TRUE;                       
        }
    }
    
    // Get the DACL.
    if(!bError)
    {
        if(!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted))
        {
            LOG(0,"GetSecurityDescriptorDacl failed. 0x%X",GetLastError());
            bError = TRUE;
        }
    }
    
    // Build the ACE.
    if(!bError)
    {
        BuildExplicitAccessWithName(&ea, "EVERYONE", DRIVER_ACCESS_RIGHTS, SET_ACCESS, 
            NO_INHERITANCE);
        
        dwError = SetEntriesInAcl(1, &ea, pacl, &pNewAcl);
        if(dwError != ERROR_SUCCESS)
        {
            LOG(0,"SetEntriesInAcl failed. 0x%X", GetLastError());
            bError = TRUE;
        }
    }
    
    // Initialize a new Security Descriptor.
    if(!bError)
    {
        if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
        {
            LOG(0,"InitializeSecurityDescriptor failed. 0x%X",GetLastError());
            bError = TRUE;
        }
    }
    
    // Set the new DACL in the Security Descriptor.
    if(!bError)
    {
        if(!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))
        {
            LOG(0, "SetSecurityDescriptorDacl", GetLastError());
            bError = TRUE;
        }
    }
    
    // Set the new DACL for the service object.
    if(!bError)
    {
        if (!SetServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, &sd))
        {
            LOG(0, "SetServiceObjectSecurity", GetLastError());
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

    LOG(1, "Attempting to uninstall NT driver.");

    if (m_bWindows95)
    {
        LOG(1,"(NT driver) Uninstall not needed with win9x/ME.");
    }
    else
    {
        UnloadDriver();

        // get handle of the Service Control Manager
        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if(hSCManager == NULL)
        {
            LOG(0, "OpenSCManager returned an error = 0x%X", GetLastError());
            bError = TRUE;
        }

        if(!bError)
        {
            m_hService = OpenService(hSCManager, NTDriverName, SERVICE_ALL_ACCESS);
            if(m_hService == NULL)
            {
                if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    LOG(0,"(NT driver) Service does not exist, no need to uninstall.");
                    CloseServiceHandle(m_hService);
                    m_hService = NULL;
                    return TRUE;
                }
                else
                {
                    LOG(0,"(NT driver) Unable to open service. 0x%X",GetLastError());
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
                LOG(0,"(NT driver) Failed to close handle to service control manager. 0x%X",GetLastError());
                bError = TRUE;
            }
            hSCManager = NULL;
        }

        if(!bError)
        {
            if (DeleteService(m_hService) == FALSE)
            {
                LOG(0,"DeleteService failed, error 0x%X", GetLastError());
                bError = TRUE;
            }
        }

        if(m_hService != NULL)
        {
            if(CloseServiceHandle(m_hService) == FALSE)
            {
                LOG(0,"CloseServiceHandle failed, error 0x%X", GetLastError());
                bError = TRUE;
            }
            m_hService = NULL;
        }

        if(bError)
        {
            UnloadDriver();
            LOG(1, "Failed to uninstall driver.");
            return FALSE;
        }
        LOG(1, "Uninstall NT driver complete.");
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
        if(dwIOCommand == ioctlGetPCIInfo)
        {
            LOG(2, "DeviceIoControl returned an error = 0x%X For Command ioctlGetPCIInfo. This is probably by design, do not worry.", GetLastError());
        }
        else
        {
            LOG(1, "DeviceIoControl returned an error = 0x%X For Command 0x%X", GetLastError(), dwIOCommand);
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
        LOG(1, "DeviceIoControl returned an error = 0x%X For Command 0x%X", GetLastError(), dwIOCommand);
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
                            ioctlGetPCIInfo,
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
