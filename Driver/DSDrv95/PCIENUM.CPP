/////////////////////////////////////////////////////////////////////////////
// pcienum.cpp
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
//  1 Nov 2000   Michael Eskin         Fixed Win95 PCI device enum for systems
//               Conexant Systems      that have had several boards installed
//                                     over the life of the system.
//									   
/////////////////////////////////////////////////////////////////////////////

#include "ioclass.h"
#include <debugout.h>
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------------
//
// MAE 30 Oct 2000 - Original code only attempted to open the first match found
// in the registry. This fails if other boards have been installed in the past, and the
// registry entryies are still in ENUM\PCI, but not physically installed.
// Changed code to continue to enumerate if match found, but devnode open failed
//
//---------------------------------------------------------------------------
// We can find some PCI information for WINDOWS 9x in the registry
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::pciFindDevice(USHORT  vendorID, USHORT  deviceID, DWORD* pdwBusNumber, DWORD* pdwSlotNumber)
{
	CHAR                  devicePath[128];
	CHAR                  subkeyPath[128];
	CHAR				  workingPath[128];
	CHAR				  workingPath2[128];
	CHAR                  hardwareID[128];
	HKEY                  pciKey;
	HKEY				  deviceKey;
	DWORD                 status;
	DWORD				  status2;
	int                   iIndex,jIndex;
	CONFIGRET             cr;
	int                   bFound;

	sprintf(devicePath, "VEN_%04X&DEV_%04X", vendorID, deviceID);

	debugOut(dbTrace,"search for <%s>",devicePath);

	//
	// Typically this is the key, since some revision information are avail we must
	// enum all subkeys from \HKL\ENUM\PCI and compare with given registryKeyPath
	//

	*pdwBusNumber  = 0;
	*pdwSlotNumber = 0;
	bFound         = 0;

	status = _RegOpenKey(HKEY_LOCAL_MACHINE,"ENUM\\PCI",&pciKey);
	if ( status != STATUS_SUCCESS)
	{
		debugOut(dbError,"! cannot open HKL\\ENUM\\PCI error %X",status);
		return status;
	}
	
	bFound = FALSE;

	for ( iIndex = 0; ! bFound && (status == STATUS_SUCCESS); iIndex++)
	{
		status = _RegEnumKey(pciKey, iIndex, subkeyPath, sizeof(subkeyPath));

		if ( status == STATUS_SUCCESS)
		{
			debugOut(dbTrace," found <%s>", subkeyPath);
			if (strstr(subkeyPath, devicePath ) != NULL)
			{
				//
				// Now we have the full path name for given vendor ID
				//
				sprintf( workingPath, "ENUM\\PCI\\%s",subkeyPath );

				debugOut(dbTrace," found pci board %s, get configuration",workingPath);

				status = _RegOpenKey(HKEY_LOCAL_MACHINE, workingPath, &deviceKey );

				if ( status == STATUS_SUCCESS)
				{
					debugOut(dbTrace,"found hardware key <%s>",workingPath );

					status2 = STATUS_SUCCESS;

					for ( jIndex = 0; ! bFound && (status2 == STATUS_SUCCESS); jIndex++)
					{
						status2 = _RegEnumKey(deviceKey, jIndex, workingPath2, sizeof(workingPath2));

						if ( status2 == STATUS_SUCCESS)
						{
							debugOut(dbTrace," subkey <%s>", workingPath2);
							sprintf(hardwareID,"%s\\%s",&workingPath[5],workingPath2);
							debugOut(dbTrace," locate device node for <%s>",hardwareID);

							cr = _CONFIGMG_Locate_DevNode((DEVNODE *)pdwSlotNumber, hardwareID, 0);
							if (cr == STATUS_SUCCESS)
							{
								bFound = TRUE;
								debugOut(dbTrace," device node %X found", *pdwSlotNumber);
							}
						}
					}
					_RegCloseKey(deviceKey);
				}
				else
				{
					debugOut(dbError,"! cannot open key <%s>",workingPath);
				}
			}
		}
	}

	_RegCloseKey(pciKey);

	if ( *pdwSlotNumber == 0)
	{
		debugOut(dbError," key not found, PCI board not installed");
		return ERROR_ACCESS_DENIED;
	}

	return STATUS_SUCCESS;
}



//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
NTSTATUS CIOAccessDevice::pciGetDeviceConfig(DWORD dwBusNumber, DWORD dwSlotNumber, PPCI_COMMON_CONFIG pciConfiguration)
{
	CONFIGRET cr;

	cr = _CONFIGMG_Call_Enumerator_Function(dwSlotNumber,
											PCI_ENUM_FUNC_GET_DEVICE_INFO,
											0,
											pciConfiguration,
											sizeof(*pciConfiguration),
											0);

	debugOut(dbTrace,"get PCI device info for node %lX returns %X",dwSlotNumber,cr);

	return cr;
}