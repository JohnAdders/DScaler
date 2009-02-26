/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth. All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HardwareDriver.h"

#include <aclapi.h>
#include <devguid.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CHardwareDriver::CHardwareDriver()
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_hService = NULL;

    OSVERSIONINFO ov;

    ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &ov);
    m_bWindows95 = (ov.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
    m_WeStartedDriver = TRUE;
}

CHardwareDriver::~CHardwareDriver()
{
}

BOOL CHardwareDriver::LoadDriver()
{
    UnloadDriver();

    SC_HANDLE hSCManager    = NULL;
    BOOL      bError        = FALSE;


    if (!m_bWindows95)
    {
        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        
        if(hSCManager == NULL)
        {
            bError = TRUE;
        }

        if(!bError)
        {

            m_hService = OpenService(hSCManager, NTDriverName, SERVICE_START|SERVICE_STOP);

            if(m_hService == NULL)
            {
                if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    if(!InstallNTDriver())
                    {
                        bError = TRUE;
                    }
                }

                else
                {
                    bError = TRUE;
                }
            }
        }

        if(hSCManager != NULL)
        {
            if(!CloseServiceHandle(hSCManager))
            {
                bError = TRUE;
            }

            hSCManager = NULL;
        }

        if(!bError)
        {
            if(StartService(m_hService, 0, NULL) == FALSE)
            {
                DWORD Err = GetLastError();
                
                if(Err == ERROR_PATH_NOT_FOUND || Err == ERROR_FILE_NOT_FOUND)
                {
                    UnInstallNTDriver();
                    bError = TRUE;
                }

                else if(Err == ERROR_SERVICE_ALREADY_RUNNING)
                {
                    m_WeStartedDriver = FALSE;
                }

                else
                {
                    bError = TRUE;
                }

            }
            
            if(!bError)
            {
                m_hFile = CreateFile("\\\\.\\DSDrv4", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, INVALID_HANDLE_VALUE);
            }
        }
    }

    else
    {
        m_hFile = CreateFile("\\\\.\\DSDrv4.VXD", 0, 0, NULL, 0, FILE_FLAG_OVERLAPPED|FILE_FLAG_DELETE_ON_CLOSE, NULL);
    }
    
    if(!bError)
    {
        if(m_hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwReturnedLength;
            DWORD dwVersion(0);

            SendCommand(IOCTL_DSDRV_GETVERSION, NULL, 0, &dwVersion, sizeof(dwVersion), &dwReturnedLength);

            if(dwVersion < 0x4002)
            {
                bError = TRUE;
                UnInstallNTDriver();
            }
        }
        
        else
        {
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
        return TRUE;
    }
}

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
                    //LOG(0,"SERVICE_CONTROL_STOP failed, error 0x%X", GetLastError());
                }
            }

            if(m_hService != NULL)
            {
                if(CloseServiceHandle(m_hService) == FALSE)
                {
                    //LOG(0,"CloseServiceHandle failed, error 0x%X", GetLastError());
                }

                m_hService = NULL;
            }
        }
    }
}

BOOL CHardwareDriver::InstallNTDriver()
{
    LPSTR       pszName;
    char        szDriverPath[MAX_PATH];
    SC_HANDLE   hSCManager = NULL;
    BOOL        bError = FALSE;

    UnloadDriver();

    if (m_bWindows95)
    {
        return TRUE;
    }

    if (!GetModuleFileName(NULL, szDriverPath, sizeof(szDriverPath)))
    {
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
            bError = TRUE;
        }
    }
    
    if(!bError)
    {
        GetShortPathName(szDriverPath, szDriverPath, MAX_PATH);

        m_hService = CreateService(    hSCManager,            // SCManager database
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
                                    NULL);                 // no password
        
        if(m_hService == NULL)
        {
            if(GetLastError() == ERROR_SERVICE_EXISTS)
            {              
                m_hService = OpenService(hSCManager, NTDriverName, SERVICE_ALL_ACCESS);
                if(DeleteService(m_hService) == FALSE)
                {
                    bError = TRUE;
                }

                if(m_hService != NULL)
                {
                    CloseServiceHandle(m_hService);
                    m_hService = NULL;
                }

                if(!bError)
                {
                    m_hService = CreateService( hSCManager,            // SCManager database
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
                                                NULL);                 // no password
                    
                    if(m_hService == NULL)
                    {
                        bError = TRUE;
                    }
                }
            }
           
            else
            {
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
            bError = TRUE;
        }
        
        hSCManager = NULL;
    }
    
    if(bError)
    {
        UnloadDriver();
        return FALSE;
    }
    
    else
    {
        return TRUE;
    }

}

BOOL CHardwareDriver::UnInstallNTDriver()
{
    SC_HANDLE hSCManager = NULL;
    BOOL      bError = FALSE;

    if (m_bWindows95)
    {
        TRACE("(NT driver) Uninstall not needed with win9x/ME.\n");
    }
   
    else
    {
        UnloadDriver();

        // get handle of the Service Control Manager
        hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if(hSCManager == NULL)
        {
            bError = TRUE;
        }

        if(!bError)
        {
            m_hService = OpenService(hSCManager, NTDriverName, SERVICE_ALL_ACCESS);
            if(m_hService == NULL)
            {
                if(GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    CloseServiceHandle(m_hService);
                    m_hService = NULL;
                    return TRUE;
                }
                
                else
                {
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
                bError = TRUE;
            }

            hSCManager = NULL;
        }

        if(!bError)
        {
            if (DeleteService(m_hService) == FALSE)
            {
                bError = TRUE;
            }
        }

        if(m_hService != NULL)
        {
            if(CloseServiceHandle(m_hService) == FALSE)
            {
                bError = TRUE;
            }

            m_hService = NULL;
        }

        if(bError)
        {
            UnloadDriver();
            return FALSE;
        }

    }
    return TRUE;

}

DWORD CHardwareDriver::SendCommand(DWORD dwIOCommand, LPVOID pvInput, DWORD dwInputLength, LPVOID pvOutput, DWORD dwOutputLength, LPDWORD pdwReturnedLength)
{
    if(DeviceIoControl(m_hFile, dwIOCommand, pvInput, dwInputLength, pvOutput, dwOutputLength, pdwReturnedLength, NULL))
    {
        return 0;
    }
    
    else
    {
        return GetLastError();
    }
}

DWORD CHardwareDriver::SendCommand(DWORD dwIOCommand, LPVOID pvInput, DWORD dwInputLength)
{
    DWORD dwDummy;
    
    if(DeviceIoControl(m_hFile, dwIOCommand, pvInput, dwInputLength, NULL, 0, &dwDummy, NULL))
    {
        return 0;
    }
   
    else
    {
        return GetLastError();
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

    psd = (PSECURITY_DESCRIPTOR)&psd;
    
    if(!QueryServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, psd, 0, &dwSize))
    {
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            psd = (PSECURITY_DESCRIPTOR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
            if(psd == NULL)
            {
                bError = TRUE;
            }
        }
        
        else
        {
            bError = TRUE;
        }
    }
    
    // Get the current security descriptor.
    if(!bError)
    {
        if(!QueryServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, psd,dwSize, &dwSize))
        {
            bError = TRUE;                       
        }
    }

    // Get the DACL.
    if(!bError)
    {
        if(!GetSecurityDescriptorDacl(psd, &bDaclPresent, &pacl, &bDaclDefaulted))
        {
            bError = TRUE;
        }
    }

    // Build the ACE.
    if(!bError)
    {
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
        PSID pSIDEveryone;

        // Create a SID for the Everyone group.
        if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSIDEveryone))
        {
            bError = TRUE;
        }

        else
        {
            ea.grfAccessMode                    = SET_ACCESS;
            ea.grfAccessPermissions                = SERVICE_START|SERVICE_STOP;
            ea.grfInheritance                    = NO_INHERITANCE;
            ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
            ea.Trustee.pMultipleTrustee            = NULL;
            ea.Trustee.TrusteeForm                = TRUSTEE_IS_SID;
            ea.Trustee.TrusteeType                = TRUSTEE_IS_GROUP;
            ea.Trustee.ptstrName                = (char *)pSIDEveryone;
    
            dwError = SetEntriesInAcl(1, &ea, pacl, &pNewAcl);
            if(dwError != ERROR_SUCCESS)
            {
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
            bError = TRUE;
        }
    }
    
    // Set the new DACL in the Security Descriptor.
    if(!bError)
    {
        if(!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE))
        {
            bError = TRUE;
        }
    }
    
    // Set the new DACL for the service object.
    if(!bError)
    {
        if (!SetServiceObjectSecurity(m_hService, DACL_SECURITY_INFORMATION, &sd))
        {
            bError = TRUE;
        }
    }
    
    // Free buffers.
    LocalFree((HLOCAL)pNewAcl);
    HeapFree(GetProcessHeap(), 0, (LPVOID)psd);
    
    return !bError;
}

BOOL CHardwareDriver::DoesThisPCICardExist(WORD VendorID, WORD DeviceID, int DeviceIndex, DWORD &SubSystemId)
{
    TDSDrvParam hwParam;
    DWORD dwStatus;
    DWORD dwLength;
    
    TPCICARDINFO PCICardInfo;
    hwParam.dwAddress    = VendorID;
    hwParam.dwValue        = DeviceID;
    hwParam.dwFlags        = DeviceIndex;

    dwStatus = SendCommand( IOCTL_DSDRV_GETPCIINFO,
                            &hwParam,
                            sizeof(hwParam),
                            &PCICardInfo,
                            sizeof(TPCICARDINFO),
                            &dwLength);

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



