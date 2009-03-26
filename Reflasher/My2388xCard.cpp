/////////////////////////////////////////////////////////////////////////////
// $Id$
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

#include "stdafx.h"
#include "HardwareDriver.h"
#include "My2388xCard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CMy2388xCard::CMy2388xCard(CHardwareDriver* pDriver)
{
    m_pDriver    = pDriver;
    m_bOpen        = FALSE;
}

CMy2388xCard::~CMy2388xCard()
{
    ClosePCICard();
}

BOOL CMy2388xCard::OpenPCICard(WORD VendorID, WORD DeviceID, int DeviceIndex)
{
    if(m_bOpen)
        ClosePCICard();

    m_DeviceId = DeviceID;
    m_VendorId = VendorID;

    DWORD        dwReturnedLength;

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
            hwParam.dwFlags = 0x400000;

        else
            hwParam.dwFlags = m_MemoryLength;

        dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_MAPMEMORY, &hwParam, sizeof(hwParam), &(m_MemoryBase), sizeof(DWORD), &dwReturnedLength);

        if (dwStatus == ERROR_SUCCESS)
            m_bOpen = TRUE;
    }

    return m_bOpen;
}

void CMy2388xCard::ClosePCICard()
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

DWORD CMy2388xCard::ReadDword(DWORD Offset)
{
    TDSDrvParam hwParam;
    hwParam.dwAddress = m_MemoryBase + Offset;

    DWORD dwReturnedLength;
    DWORD dwValue = NULL;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_READMEMORYDWORD, &hwParam, sizeof(hwParam.dwAddress), &dwValue, sizeof(dwValue), &dwReturnedLength);
    return dwValue;
}

void CMy2388xCard::WriteDword(DWORD Offset, DWORD Data)
{
    TDSDrvParam hwParam;
    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_WRITEMEMORYDWORD, &hwParam, sizeof(hwParam));
}

void CMy2388xCard::WriteByte(DWORD Offset, BYTE Data)
{
    TDSDrvParam hwParam;

    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_WRITEMEMORYBYTE, &hwParam, sizeof(hwParam));
}
