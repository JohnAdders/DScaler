/////////////////////////////////////////////////////////////////////////////
// $Id: PCICard.h,v 1.5 2001-11-25 01:58:34 ittarnavsky Exp $
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

#ifndef __PCICARD_H___
#define __PCICARD_H___

#include "HardwareDriver.h"

/////////////////////////////////////////////////////////////////////////////
// CPCICard
/////////////////////////////////////////////////////////////////////////////
// This class is used to provide access to the low level function provided
// by the drivers
// To use these function derive your card specific class from this one.
/////////////////////////////////////////////////////////////////////////////

class CPCICard
{
public:
    DWORD GetSubSystemId();
    DWORD ReadDword(DWORD Offset);
protected:
    CPCICard(CHardwareDriver* pDriver);
    ~CPCICard();

    BOOL OpenPCICard(WORD dwVendorID, WORD dwDeviceID, int dwDeviceIndex);
    void ClosePCICard();

    void WriteByte(DWORD Offset, BYTE Data);
    void WriteWord(DWORD Offset, WORD Data);
    void WriteDword(DWORD Offset, DWORD Data);

    BYTE ReadByte(DWORD Offset);
    WORD ReadWord(DWORD Offset);

	void MaskDataByte(DWORD Offset, BYTE Data, BYTE Mask);
	void MaskDataWord(DWORD Offset, WORD Data, WORD Mask);
	void MaskDataDword(DWORD Offset, WORD Data, WORD Mask);
	void AndOrDataByte(DWORD Offset, DWORD Data, BYTE Mask);
	void AndOrDataWord(DWORD Offset, DWORD Data, WORD Mask);
	void AndOrDataDword(DWORD Offset, DWORD Data, DWORD Mask);
	void AndDataByte(DWORD Offset, BYTE Data);
	void AndDataWord(DWORD Offset, WORD Data);
	void AndDataDword(DWORD Offset, WORD Data);
	void OrDataByte(DWORD Offset, BYTE Data);
	void OrDataWord(DWORD Offset, WORD Data);
	void OrDataDword(DWORD Offset, DWORD Data);

protected:
    DWORD  m_SubSystemId;
    BOOL   m_bOpen;
private:
    DWORD  m_MemoryAddress;
    DWORD  m_MemoryLength;
    DWORD  m_BusNumber;
    DWORD  m_SlotNumber;
    DWORD  m_MemoryBase;
    CHardwareDriver* m_pDriver;
};


#endif
