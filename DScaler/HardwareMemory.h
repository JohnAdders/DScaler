/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareMemory.h,v 1.2 2001-08-09 16:46:48 adcockj Exp $
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

#ifndef __HARDWAREMEMORY_H___
#define __HARDWAREMEMORY_H___

#include "HardwareDriver.h"

class CHardwareMemory
{
public:
    void* GetUserPointer();
    DWORD TranslateToPhysical(void* pUser);
    BOOL IsValid();
protected:
    CHardwareMemory(CHardwareDriver* m_pDriver);
    ~CHardwareMemory();
    TMemStruct* m_pMemStruct;
    CHardwareDriver* m_pDriver;
};

class CUserMemory
{
public:
    CUserMemory(CHardwareDriver* m_pDriver, size_t Bytes, size_t Align);
    ~CUserMemory();
};

class CContigMemory
{
public:
    CContigMemory(CHardwareDriver* m_pDriver, size_t Bytes);
    ~CContigMemory();
};

#endif