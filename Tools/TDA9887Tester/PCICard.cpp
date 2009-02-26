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

#include "stdafx.h"
#include "PCICard.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CPCICard::CPCICard(CHardwareDriver* pDriver) :
    m_pDriver(pDriver),
    m_bOpen(FALSE)
{
    strcpy(m_szChipName,"n/a");
}

CPCICard::~CPCICard()
{
    ClosePCICard();
}

BOOL CPCICard::OpenPCICard(WORD VendorID, WORD DeviceID, int DeviceIndex)
{
    if(m_bOpen)
    {
        ClosePCICard();
    }

    m_DeviceId = DeviceID;
    m_VendorId = VendorID;

    DWORD dwReturnedLength;

    TDSDrvParam hwParam;
    hwParam.dwAddress    = VendorID;
    hwParam.dwValue        = DeviceID;
    hwParam.dwFlags        = DeviceIndex;

    TPCICARDINFO PCICardInfo;
    DWORD dwLength;
    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_GETPCIINFO, &hwParam, sizeof(hwParam), &PCICardInfo, sizeof(TPCICARDINFO), &dwLength);

    if (dwStatus == ERROR_SUCCESS)
    {
        m_MemoryAddress    = PCICardInfo.dwMemoryAddress;
        m_MemoryLength    = PCICardInfo.dwMemoryLength;
        m_SubSystemId    = PCICardInfo.dwSubSystemId;
        m_BusNumber        = PCICardInfo.dwBusNumber;
        m_SlotNumber    = PCICardInfo.dwSlotNumber;

        hwParam.dwAddress    = m_BusNumber;
        hwParam.dwValue        = m_MemoryAddress;

        if((VendorID == 0x14F1) && (DeviceID == 0x8800))
        {
            hwParam.dwFlags = 0x400000;
        }
        
        else
        {
            hwParam.dwFlags = m_MemoryLength;
        }

        dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_MAPMEMORY, &hwParam, sizeof(hwParam), &(m_MemoryBase), sizeof(DWORD), &dwReturnedLength);

        if (dwStatus == ERROR_SUCCESS)
        {
            m_bOpen = TRUE;
        }
    }

    return m_bOpen;
}

void CPCICard::ClosePCICard()
{
    if(m_MemoryBase != NULL)
    {
        TDSDrvParam hwParam;
        hwParam.dwAddress = m_MemoryBase;
        hwParam.dwValue   = m_MemoryLength;

        DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_UNMAPMEMORY, &hwParam, sizeof(hwParam));
        m_bOpen = FALSE;
    }
}

ULONG CPCICard::GetTickCount()
// an overflow happens after 21 days uptime on a 10GHz machine
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);

    ticks = ticks * 1000 / frequency;
    return (ULONG)ticks;
}

BYTE CPCICard::ReadByte(DWORD Offset)
{
    TDSDrvParam hwParam;
    hwParam.dwAddress = m_MemoryBase + Offset;

    DWORD dwReturnedLength;
    BYTE bValue(0);

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_READMEMORYBYTE, &hwParam, sizeof(hwParam.dwAddress), &bValue, sizeof(bValue), &dwReturnedLength);
    return bValue;
}

DWORD CPCICard::ReadDword(DWORD Offset)
{
    TDSDrvParam hwParam;
    hwParam.dwAddress = m_MemoryBase + Offset;
    
    DWORD dwReturnedLength;
    DWORD dwValue(0);

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_READMEMORYDWORD, &hwParam, sizeof(hwParam.dwAddress), &dwValue, sizeof(dwValue), &dwReturnedLength);
    return dwValue;
}

void CPCICard::WriteByte(DWORD Offset, BYTE Data)
{
    TDSDrvParam hwParam;
    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_WRITEMEMORYBYTE, &hwParam, sizeof(hwParam));
}

void CPCICard::WriteDword(DWORD Offset, DWORD Data)
{
    TDSDrvParam hwParam;
    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_WRITEMEMORYDWORD, &hwParam, sizeof(hwParam));
}

void CPCICard::MaskDataByte(DWORD Offset, BYTE Data, BYTE Mask)
{
    BYTE Result(ReadByte(Offset));
    Result = (Result & ~Mask) | (Data & Mask);
    WriteByte(Offset, Result);
}

void CPCICard::SetChipName(const char* ChipName)
{
    strcpy(m_szChipName, ChipName);
}

const char* CPCICard::GetChipName()
{
    return m_szChipName;
}
