/////////////////////////////////////////////////////////////////////////////
// $Id: My2388xCard.h,v 1.1 2004-04-14 08:15:44 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth.  All rights reserved.
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

#if !defined(AFX_MY2388XCARD_H__A8D4B201_8B94_498E_8958_D591C1D7449C__INCLUDED_)
#define AFX_MY2388XCARD_H__A8D4B201_8B94_498E_8958_D591C1D7449C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HardwareDriver.h"

class CMy2388xCard  
{
public:
	void WriteByte(DWORD Offset, BYTE Data);
	void WriteDword(DWORD Offset, DWORD Data);
	DWORD ReadDword(DWORD Offset);
	BOOL OpenPCICard(WORD VendorID, WORD DeviceID, int DeviceIndex);
	CMy2388xCard(CHardwareDriver* pDriver);
	~CMy2388xCard();

protected:
	void ClosePCICard();
    
	DWORD   m_SubSystemId;
    WORD    m_DeviceId;
    WORD    m_VendorId;
    DWORD   m_BusNumber;
    DWORD   m_SlotNumber;
	BOOL	m_bOpen;

private:
    DWORD   m_MemoryAddress;
    DWORD   m_MemoryLength;
    DWORD   m_MemoryBase;
    CHardwareDriver* m_pDriver;
};

#endif // !defined(AFX_MY2388XCARD_H__A8D4B201_8B94_498E_8958_D591C1D7449C__INCLUDED_)
