/////////////////////////////////////////////////////////////////////////////
// $Id: PCICard.h,v 1.1 2001-08-09 16:44:50 adcockj Exp $
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
protected:
    CPCICard(CHardwareDriver* pDriver);
    ~CPCICard();

    BOOL OpenPCICard(DWORD dwVendorID, DWORD dwDeviceID, DWORD dwDeviceIndex);
    void ClosePCICard();

    void WriteByte(DWORD Offset, BYTE Data);
    void WriteWord(DWORD Offset, WORD Data);
    void WriteDword(DWORD Offset, DWORD Data);

    BYTE ReadByte(DWORD Offset);
    WORD ReadWord(DWORD Offset);
    DWORD ReadDword(DWORD Offset);
    BOOL bIsOpen();

private:
    DWORD  m_MemoryAddress;
    DWORD  m_MemoryLength;
    DWORD  m_SubSystemId;
    DWORD  m_BusNumber;
    DWORD  m_SlotNumber;
    DWORD  m_MemoryBase;
    CHardwareDriver* m_pDriver;
    BOOL   m_bOpen;
};


#endif
