/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareDriver.h,v 1.1 2001-08-09 16:44:50 adcockj Exp $
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

#ifndef __HARDWAREDRIVER_H___
#define __HARDWAREDRIVER_H___

#include "DSDrv.h"
#include <winsvc.h>

class CHardwareMemory
{
public:
    CHardwareMemory(size_t Bytes, size_t Align);
    ~CHardwareMemory();
    BYTE* GetUserPointer();
    DWORD TranslateToPhysical(BYTE* pUser);
private:

}

class CHardwareDriver
{
public:
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

private:
    SC_HANDLE   m_hSCManager;
    SC_HANDLE   m_hService;
    HANDLE      m_hFile;
    LPSTR       m_DOSDeviceName;
    BOOL        m_bWindows95;
};


#endif
