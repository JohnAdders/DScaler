/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareMemory.cpp,v 1.2 2001-08-09 16:46:48 adcockj Exp $
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
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HardwareMemory.h"
#include "DebugLog.h"

CHardwareMemory::CHardwareMemory(CHardwareDriver* pDriver) :
                    m_pDriver(m_pDriver)
{
    m_pMemStruct = NULL;
}

CHardwareMemory::~CHardwareMemory()
{

}

void* CHardwareMemory::GetUserPointer()
{
    return NULL;
}

DWORD CHardwareMemory::TranslateToPhysical(void* pUser)
{
    return 0;
}

BOOL CHardwareMemory::IsValid()
{
    return (m_pMemStruct != NULL);
}

CUserMemory::CUserMemory(CHardwareDriver* m_pDriver, size_t Bytes, size_t Align)
{
}

CUserMemory::~CUserMemory()
{
}

CContigMemory::CContigMemory(CHardwareDriver* m_pDriver, size_t Bytes)
{
}

CContigMemory::~CContigMemory()
{
}
