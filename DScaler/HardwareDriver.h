/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareDriver.h,v 1.9 2003-10-27 10:39:51 adcockj Exp $
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
 * @file hardwaredriver.h hardwaredriver Header file
 */
 
#ifndef __HARDWAREDRIVER_H___
#define __HARDWAREDRIVER_H___

#include "DSDrv.h"
#include <winsvc.h>

/** Allows access to the DSDrv4 driver
*/
class CHardwareDriver
{
public:
	BOOL UnInstallNTDriver();
	BOOL InstallNTDriver();
    CHardwareDriver();
    ~CHardwareDriver();

    BOOL LoadDriver();
    void UnloadDriver();

    DWORD SendCommand(
                        DWORD dwIOCommand,
                        LPVOID pvInput,
                        DWORD dwInputLength,
                        LPVOID pvOutput,
                        DWORD dwOutputLength,
                        LPDWORD pdwReturnedLength
                     );

    DWORD SendCommand(
                        DWORD dwIOCommand,
                        LPVOID pvInput,
                        DWORD dwInputLength
                     );

    BOOL DoesThisPCICardExist(
                                 WORD VendorID, 
                                 WORD DeviceID, 
                                 int DeviceIndex, 
                                 DWORD& SubSystemId
                              );

private:
	BOOL AdjustAccessRights();
    SC_HANDLE   m_hService;
    HANDLE      m_hFile;
    BOOL        m_bWindows95;
    BOOL        m_WeStartedDriver;
};


#endif
