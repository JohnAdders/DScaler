/////////////////////////////////////////////////////////////////////////////
// $Id: PCICard.h,v 1.7 2001-11-29 17:30:52 adcockj Exp $
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

/** This class is used to provide access to the low level function provided
    by the drivers.  To use these function derive your card specific class 
    from this one.
*/
class CPCICard
{
public:
    DWORD GetSubSystemId();
    /**  Try to find card with given attributes on system
         @return TRUE is device is found
    */
    BOOL OpenPCICard(WORD dwVendorID, WORD dwDeviceID, int dwDeviceIndex);
protected:
    CPCICard(CHardwareDriver* pDriver);
    ~CPCICard();
    /// Clos the card and unmap memory
    void ClosePCICard();

    /// Write a BYTE to shared memory
    void WriteByte(DWORD Offset, BYTE Data);
    /// Write a WORD to shared memory
    void WriteWord(DWORD Offset, WORD Data);
    /// Write a DWORD to shared memory
    void WriteDword(DWORD Offset, DWORD Data);

    /// Read a BYTE from shared memory
    BYTE ReadByte(DWORD Offset);
    /// Read a WORD from shared memory
    WORD ReadWord(DWORD Offset);
    /// Read a DWORD from shared memory
    DWORD ReadDword(DWORD Offset);

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
