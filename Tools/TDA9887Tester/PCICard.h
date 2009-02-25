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
// CVS Log
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PCICARD_H__94F7340C_9987_4F50_BCEA_A703531B1F54__INCLUDED_)
#define AFX_PCICARD_H__94F7340C_9987_4F50_BCEA_A703531B1F54__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HardwareDriver.h"

class CPCICard  
{
public:
	virtual bool WriteToI2C(const BYTE *writeBuffer, size_t writeBufferSize) = false;
	const char* GetChipName();
	void SetChipName(const char* ChipName);
	BOOL OpenPCICard(WORD VendorID, WORD DeviceID, int DeviceIndex);
	CPCICard(CHardwareDriver* pDriver);
	virtual ~CPCICard();

private:
    CHardwareDriver* m_pDriver;
    DWORD	m_MemoryAddress;
    DWORD	m_MemoryLength;
    DWORD	m_MemoryBase;
	DWORD   m_SubSystemId;
    WORD    m_DeviceId;
    WORD    m_VendorId;
    DWORD   m_BusNumber;
    DWORD   m_SlotNumber;
	BOOL	m_bOpen;

	char	m_szChipName[20];

protected:
	void	WriteByte(DWORD Offset, BYTE Data);
	BYTE	ReadByte(DWORD Offset);
	void	WriteDword(DWORD Offset, DWORD Data);
	DWORD	ReadDword(DWORD Offset);
    void	MaskDataByte(DWORD Offset, BYTE Data, BYTE Mask);

	ULONG	GetTickCount();

private:
	void ClosePCICard();

};

#endif // !defined(AFX_PCICARD_H__94F7340C_9987_4F50_BCEA_A703531B1F54__INCLUDED_)
