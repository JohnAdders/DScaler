/////////////////////////////////////////////////////////////////////////////
// $Id: PCICard.cpp,v 1.1 2001-08-09 16:44:50 adcockj Exp $
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
#include "PCICard.h"
#include "HardwareDriver.h"
#include "DebugLog.h"


CPCICard::CPCICard(CHardwareDriver* pDriver) :
            m_pDriver(pDriver),
            m_MemoryAddress(0),
            m_MemoryLength(0),
            m_SubSystemId(0),
            m_BusNumber(0),
            m_SlotNumber(0),
            m_MemoryBase(0),
            m_bOpen(FALSE)
{
}

CPCICard::~CPCICard()
{
    if(m_bOpen)
    {
        ClosePCICard();
    }
}

BOOL CPCICard::OpenPCICard(DWORD dwVendorID, DWORD dwDeviceID, DWORD dwDeviceIndex)
{
    if(m_bOpen)
    {
        ClosePCICard();
    }

    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    DWORD dwStatus;
    DWORD dwLength;
    TPCICARDINFO PCICardInfo;

    hwParam.dwAddress = dwVendorID;
    hwParam.dwValue = dwDeviceID;
    hwParam.dwFlags = dwDeviceIndex;

    dwStatus = m_pDriver->SendCommand(ioctlGetPCIInfo,
                                        &hwParam,
                                        sizeof(hwParam),
                                        &PCICardInfo,
                                        sizeof(TPCICARDINFO),
                                        &dwLength);

    if ( dwStatus == ERROR_SUCCESS)
    {
        m_MemoryAddress = PCICardInfo.dwMemoryAddress;
        m_MemoryLength = PCICardInfo.dwMemoryLength;
        m_SubSystemId = PCICardInfo.dwSubSystemId;
        m_BusNumber = PCICardInfo.dwBusNumber;
        m_SlotNumber = PCICardInfo.dwSlotNumber;

        hwParam.dwAddress = m_BusNumber;
        hwParam.dwValue = m_MemoryAddress;
        hwParam.dwFlags = m_MemoryLength;

        dwStatus = m_pDriver->SendCommand(ioctlMapMemory,
                                            &hwParam,
                                            sizeof(hwParam),
                                            &(m_MemoryBase),
                                            sizeof(DWORD),
                                            &dwReturnedLength);

        if (dwStatus == ERROR_SUCCESS)
        {
            m_bOpen = TRUE;
        }
        else
        {
            LOG(1, "MapMemory failed 0x%x", dwStatus);
        }
   
    }
    else
    {
        LOG(1, "GetPCIInfo failed for %X %X failed 0x%x",dwVendorID, dwDeviceID, dwStatus);
    }
    return m_bOpen;
}

void CPCICard::ClosePCICard()
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;

    hwParam.dwAddress = m_MemoryAddress;
    hwParam.dwValue   = m_MemoryLength;

    DWORD dwStatus = m_pDriver->SendCommand(ioctlUnmapMemory,
                                            &hwParam,
                                            sizeof(hwParam),
                                            NULL,
                                            0,
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "UnmapMemory failed 0x%x", dwStatus);
    }

    m_bOpen = FALSE;
}

void CPCICard::WriteByte(DWORD Offset, BYTE Data)
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;

    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(ioctlWriteMemoryBYTE,
                                            &hwParam,
                                            sizeof(hwParam),
                                            NULL,
                                            0,
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "WriteMemoryBYTE failed 0x%x", dwStatus);
    }
}

void CPCICard::WriteWord(DWORD Offset, WORD Data)
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;

    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(ioctlWriteMemoryWORD,
                                            &hwParam,
                                            sizeof(hwParam),
                                            NULL,
                                            0,
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "WriteMemoryWORD failed 0x%x", dwStatus);
    }
}

void CPCICard::WriteDword(DWORD Offset, DWORD Data)
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;

    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(ioctlWriteMemoryDWORD,
                                            &hwParam,
                                            sizeof(hwParam),
                                            NULL,
                                            0,
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "WriteMemoryDWORD failed 0x%x", dwStatus);
    }
}

BYTE CPCICard::ReadByte(DWORD Offset)
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    BYTE bValue(0);

    hwParam.dwAddress = m_MemoryBase + Offset;
    DWORD dwStatus = m_pDriver->SendCommand(ioctlReadMemoryBYTE,
                                            &hwParam,
                                            sizeof(hwParam.dwAddress),
                                            &bValue,
                                            sizeof(bValue),
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "ReadMemoryBYTE failed 0x%x", dwStatus);
    }
    return bValue;
}

WORD CPCICard::ReadWord(DWORD Offset)
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    WORD wValue(0);

    hwParam.dwAddress = m_MemoryBase + Offset;
    DWORD dwStatus = m_pDriver->SendCommand(ioctlReadMemoryWORD,
                                            &hwParam,
                                            sizeof(hwParam.dwAddress),
                                            &wValue,
                                            sizeof(wValue),
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "ReadMemoryWORD failed 0x%x", dwStatus);
    }
    return wValue;
}

DWORD CPCICard::ReadDword(DWORD Offset)
{
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    DWORD dwValue(0);

    hwParam.dwAddress = m_MemoryBase + Offset;
    DWORD dwStatus = m_pDriver->SendCommand(ioctlReadMemoryDWORD,
                                            &hwParam,
                                            sizeof(hwParam.dwAddress),
                                            &dwValue,
                                            sizeof(dwValue),
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "ReadMemoryDWORD failed 0x%x", dwStatus);
    }
    return dwValue;
}
