/////////////////////////////////////////////////////////////////////////////
// $Id: KernelDriver.H,v 1.2 2001-07-13 16:13:53 adcockj Exp $
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

#if ! defined (__KERNELDRIVER_H)
#define __KERNELDRIVER_H

#if ! defined (__OBJWIN32)
#include "objwin32.h"
#endif

#include <winsvc.h>

#define DEVICE_PREFIX   "\\\\.\\"


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class CKernelDriver : public CWin32Object
{
public:
    CKernelDriver();
    ~CKernelDriver();

    virtual DWORD start(LPSTR pszDriverName, LPSTR pszBinaryPathName);

    virtual DWORD start(LPSTR pszDriverName);

    virtual DWORD stop(LPSTR pszDriverName, BOOL bRemove);
    virtual DWORD stop(void);
    virtual DWORD open(LPSTR pszDeviceName);
    virtual DWORD close(void);
    virtual DWORD read(PVOID pvBuffer, DWORD dwBufferSize, LPDWORD pdwBytesReading);
    virtual DWORD write(PVOID pvBuffer,DWORD dwBufferSize);
    virtual DWORD deviceControl(DWORD dwIOCommand,
                                    LPVOID pvInput,
                                    DWORD dwInputLength,
                                    LPVOID pvOutput,
                                    DWORD dwOutputLength,
                                    LPDWORD pdwReturnedLength);
    virtual DWORD  deviceControl(DWORD dwIOCommand, LPVOID pvInput, DWORD dwInputLength);

protected:
    virtual HANDLE openDevice(LPSTR pszDriverName);
    virtual HANDLE openFile(LPSTR pszDriverName);

    DWORD installDriver(LPSTR pszDriverName, LPSTR pszBinaryPathName);

    void removeDriver(LPSTR pszDriverName);
    void startDriver(LPSTR pszDriverName);
    DWORD stopDriver(LPSTR pszDriverName);
    SC_HANDLE openService(LPSTR pszDriverName);
    BOOL openSCM(void);

protected:
    SC_HANDLE   schSCManager;
    DWORD       serviceFlag;
    LPSTR       kdDriverName;
    HANDLE      kdFileHandle;
    BOOL        kdDeviceCreated;
    LPSTR       kdDOSDeviceName;
    BOOL        bWindows95;
};

typedef CKernelDriver * PKernelDriver;


#endif



