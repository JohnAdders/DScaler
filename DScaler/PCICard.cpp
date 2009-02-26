/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file PCICard.cpp CPCICard Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "PCICard.h"
#include "HardwareDriver.h"
#include "DebugLog.h"

extern "C" unsigned long gBuildNum;

CPCICard::CPCICard(CHardwareDriver* pDriver) :
            m_pDriver(pDriver),
            m_MemoryAddress(0),
            m_MemoryLength(0),
            m_SubSystemId(0),
            m_BusNumber(0),
            m_SlotNumber(0),
            m_MemoryBase(0),
            m_bOpen(FALSE),
            m_hStateFile(INVALID_HANDLE_VALUE),
            m_bStateIsReading(false),
            m_SupportsACPI(false),
            m_InitialACPIStatus(0)
{
    InitializeCriticalSection(&m_CriticalSection);
}

CPCICard::~CPCICard()
{
    if(m_bOpen)
    {
        ClosePCICard();
    }
    DeleteCriticalSection(&m_CriticalSection);
}

DWORD CPCICard::GetSubSystemId()
{
    return m_SubSystemId;
}

WORD CPCICard::GetDeviceId()
{
    return m_DeviceId;
}

WORD CPCICard::GetVendorId()
{
    return m_VendorId;
}

BOOL CPCICard::OpenPCICard(WORD VendorID, WORD DeviceID, int DeviceIndex)
{
    if(m_bOpen)
    {
        ClosePCICard();
    }
    m_DeviceId = DeviceID;
    m_VendorId = VendorID;

    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    DWORD dwStatus;
    DWORD dwLength;
    TPCICARDINFO PCICardInfo;

    hwParam.dwAddress = VendorID;
    hwParam.dwValue = DeviceID;
    hwParam.dwFlags = DeviceIndex;

    dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_GETPCIINFO,
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

        // we need to map much more memory for the CX2388x
        // \todo should make this a parameter
        if((VendorID == 0x14F1) && (DeviceID == 0x8800))
        {
            m_MemoryLength = 0x400000;
        }

        hwParam.dwFlags = m_MemoryLength;

        dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_MAPMEMORY,
                                            &hwParam,
                                            sizeof(hwParam),
                                            &(m_MemoryBase),
                                            sizeof(DWORD),
                                            &dwReturnedLength);

        if (dwStatus == ERROR_SUCCESS)
        {
            m_bOpen = TRUE;

            m_InitialACPIStatus = GetACPIStatus();
            // if the chip is powered down we need to power it up
            if(m_InitialACPIStatus != 0)
            {
                SetACPIStatus(0);
            }

            SaveState();
        }
        else
        {
            LOG(1, "MapMemory failed 0x%x", dwStatus);
        }
   
    }
    else
    {
        LOG(1, "GetPCIInfo failed for %X %X failed 0x%x", VendorID, DeviceID, dwStatus);
    }


    return m_bOpen;
}

void CPCICard::ClosePCICard()
{
    if(m_MemoryBase != NULL)
    {
        RestoreState();

        // if the chip was not in D0 state we restore the original ACPI power state
        if(m_InitialACPIStatus != 0)
        {
            SetACPIStatus(m_InitialACPIStatus);
        }


        TDSDrvParam hwParam;

        hwParam.dwAddress = m_MemoryBase;
        hwParam.dwValue   = m_MemoryLength;

        DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_UNMAPMEMORY,
                                                &hwParam,
                                                sizeof(hwParam));

        if (dwStatus != ERROR_SUCCESS)
        {
            LOG(1, "UnmapMemory failed 0x%x", dwStatus);
        }

        m_bOpen = FALSE;
    }
}

void CPCICard::WriteByte(DWORD Offset, BYTE Data)
{
    LockCard();
    TDSDrvParam hwParam;

    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_WRITEMEMORYBYTE,
                                            &hwParam,
                                            sizeof(hwParam));

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "WriteMemoryBYTE failed 0x%x", dwStatus);
    }
    UnlockCard();
}

void CPCICard::WriteWord(DWORD Offset, WORD Data)
{
    LockCard();
    TDSDrvParam hwParam;

    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_WRITEMEMORYWORD,
                                            &hwParam,
                                            sizeof(hwParam));

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "WriteMemoryWORD failed 0x%x", dwStatus);
    }
    UnlockCard();
}

void CPCICard::WriteDword(DWORD Offset, DWORD Data)
{
    LockCard();
    TDSDrvParam hwParam;

    hwParam.dwAddress = m_MemoryBase + Offset;
    hwParam.dwValue = Data;

    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_WRITEMEMORYDWORD,
                                            &hwParam,
                                            sizeof(hwParam));

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "WriteMemoryDWORD failed 0x%x", dwStatus);
    }
    UnlockCard();
}

BYTE CPCICard::ReadByte(DWORD Offset)
{
    LockCard();
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    BYTE bValue(0);

    hwParam.dwAddress = m_MemoryBase + Offset;
    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_READMEMORYBYTE,
                                            &hwParam,
                                            sizeof(hwParam.dwAddress),
                                            &bValue,
                                            sizeof(bValue),
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "ReadMemoryBYTE failed 0x%x", dwStatus);
    }
    UnlockCard();
    return bValue;
}

WORD CPCICard::ReadWord(DWORD Offset)
{
    LockCard();
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    WORD wValue(0);

    hwParam.dwAddress = m_MemoryBase + Offset;
    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_READMEMORYWORD,
                                            &hwParam,
                                            sizeof(hwParam.dwAddress),
                                            &wValue,
                                            sizeof(wValue),
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "ReadMemoryWORD failed 0x%x", dwStatus);
    }
    UnlockCard();
    return wValue;
}

DWORD CPCICard::ReadDword(DWORD Offset)
{
    LockCard();
    TDSDrvParam hwParam;
    DWORD dwReturnedLength;
    DWORD dwValue(0);

    hwParam.dwAddress = m_MemoryBase + Offset;
    DWORD dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_READMEMORYDWORD,
                                            &hwParam,
                                            sizeof(hwParam.dwAddress),
                                            &dwValue,
                                            sizeof(dwValue),
                                            &dwReturnedLength);

    if (dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "ReadMemoryDWORD failed 0x%x", dwStatus);
    }
    UnlockCard();
    return dwValue;
}

void CPCICard::WriteData(DWORD RegisterOffset, DWORD RegisterMask, CBitVector Data)
{
    unsigned long mask = Data.mask() & RegisterMask;
    unsigned long value = Data.value();

    if (RegisterMask & 0xFFFF0000)
    {
        if (mask == 0xFFFFFFFF)
        {
            WriteDword(RegisterOffset, static_cast<BYTE>(value));
        }
        else
        {
            MaskDataDword(RegisterOffset, static_cast<BYTE>(value), static_cast<BYTE>(mask));
        }
    }
    else if (RegisterMask & 0xFF00)
    {
        if (mask == 0xFFFF)
        {
            WriteWord(RegisterOffset, static_cast<BYTE>(value));
        }
        else
        {
            MaskDataWord(RegisterOffset, static_cast<BYTE>(value), static_cast<BYTE>(mask));
        }
    }
    else if (RegisterMask & 0xFF)
    {
        if (mask == 0xFF)
        {
            WriteByte(RegisterOffset, static_cast<BYTE>(value));
        }
        else
        {
            MaskDataByte(RegisterOffset, static_cast<BYTE>(value), static_cast<BYTE>(mask));
        }
    }
}

CBitVector CPCICard::ReadData(DWORD RegisterOffset, DWORD RegisterMask)
{
    unsigned long value = 0;

    if (RegisterMask & 0xFFFF0000)
    {
        value = static_cast<unsigned long>(ReadDword(RegisterOffset));
    }
    else if (RegisterMask & 0xFF00)
    {
        value = static_cast<unsigned long>(ReadWord(RegisterOffset));
    }
    else if (RegisterMask & 0xFF)
    {
        value = static_cast<unsigned long>(ReadByte(RegisterOffset));
    }
    return CBitVector(RegisterMask, value);
}

void CPCICard::ManageData(DWORD RegisterOffset, DWORD RegisterMask, CBitMask DataMask)
{
    if (m_hStateFile != INVALID_HANDLE_VALUE)
    {
        DWORD minimalMask = RegisterMask & DataMask.mask();
        DWORD minimalSize;

        if (minimalMask & 0xFFFF0000)
        {
            minimalSize = 4;
        }
        else if (minimalMask & 0xFF00)
        {
            minimalSize = 2;
        }
        else if (minimalMask & 0xFF)
        {
            minimalSize = 1;
        }
        else
        {
            return;
        }

        if (m_bStateIsReading)
        {
            DWORD readData = 0;
            DWORD bytesRead = 0;
            if (ReadFile(m_hStateFile, &readData, minimalSize, &bytesRead, NULL))
            {
                if (bytesRead == minimalSize)
                {
                    WriteData(RegisterOffset, RegisterMask, CBitVector(DataMask.mask(), readData));
                }
            }
        }
        else
        {
            CBitVector data = ReadData(RegisterOffset, RegisterMask);
            DWORD writeData = data.value();
            DWORD bytesWritten = 0;
            WriteFile(m_hStateFile, &writeData, minimalSize, &bytesWritten, NULL);
        }
    }
}

void CPCICard::MaskDataByte(DWORD Offset, BYTE Data, BYTE Mask)
{
    LockCard();
    BYTE Result(ReadByte(Offset));
    Result = (Result & ~Mask) | (Data & Mask);
    WriteByte(Offset, Result);
    UnlockCard();
}

void CPCICard::MaskDataWord(DWORD Offset, WORD Data, WORD Mask)
{
    LockCard();
    WORD Result(ReadWord(Offset));
    Result = (Result & ~Mask) | (Data & Mask);
    WriteWord(Offset, Result);
    UnlockCard();
}

void CPCICard::MaskDataDword(DWORD Offset, DWORD Data, DWORD Mask)
{
    LockCard();
    DWORD Result(ReadDword(Offset));
    Result = (Result & ~Mask) | (Data & Mask);
    WriteDword(Offset, Result);
    UnlockCard();
}

void CPCICard::AndOrDataByte(DWORD Offset, DWORD Data, BYTE Mask)
{
    LockCard();
    BYTE Result(ReadByte(Offset));
    Result = static_cast<BYTE>((Result & Mask) | Data);
    WriteByte(Offset, Result);
    UnlockCard();
}

void CPCICard::AndOrDataWord(DWORD Offset, DWORD Data, WORD Mask)
{
    LockCard();
    WORD Result(ReadWord(Offset));
    Result = static_cast<BYTE>((Result & Mask) | Data);
    WriteWord(Offset, Result);
    UnlockCard();
}

void CPCICard::AndOrDataDword(DWORD Offset, DWORD Data, DWORD Mask)
{
    LockCard();
    DWORD Result(ReadDword(Offset));
    Result = (Result & Mask) | Data;
    WriteDword(Offset, Result);
    UnlockCard();
}

void CPCICard::AndDataByte(DWORD Offset, BYTE Data)
{
    LockCard();
    BYTE Result(ReadByte(Offset));
    Result &= Data;
    WriteByte(Offset, Result);
    UnlockCard();
}

void CPCICard::AndDataWord(DWORD Offset, WORD Data)
{
    LockCard();
    WORD Result(ReadWord(Offset));
    Result &= Data;
    WriteWord(Offset, Result);
    UnlockCard();
}

void CPCICard::AndDataDword(DWORD Offset, DWORD Data)
{
    LockCard();
    DWORD Result(ReadDword(Offset));
    Result &= Data;
    WriteDword(Offset, Result);
    UnlockCard();
}

void CPCICard::OrDataByte(DWORD Offset, BYTE Data)
{
    LockCard();
    BYTE Result(ReadByte(Offset));
    Result |= Data;
    WriteByte(Offset, Result);
    UnlockCard();
}

void CPCICard::OrDataWord(DWORD Offset, WORD Data)
{
    LockCard();
    WORD Result(ReadWord(Offset));
    Result |= Data;
    WriteWord(Offset, Result);
    UnlockCard();
}

void CPCICard::OrDataDword(DWORD Offset, DWORD Data)
{
    LockCard();
    DWORD Result(ReadDword(Offset));
    Result |= Data;
    WriteDword(Offset, Result);
    UnlockCard();
}


BOOL CPCICard::GetPCIConfig(PCI_COMMON_CONFIG* pPCI_COMMON_CONFIG, DWORD Bus, DWORD Slot)
{
    if(pPCI_COMMON_CONFIG == NULL)
    {
        LOG(1, "GetPCIConfig failed. pPCI_COMMON_CONFIG == NULL");
        return FALSE;
    }

    TDSDrvParam hwParam;
    DWORD dwStatus;
    DWORD dwLength;

    hwParam.dwAddress = Bus;
    hwParam.dwValue = Slot;

    dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_GETPCICONFIG,
                                        &hwParam,
                                        sizeof(hwParam),
                                        pPCI_COMMON_CONFIG,
                                        sizeof(PCI_COMMON_CONFIG),
                                        &dwLength);

    if(dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "GetPCIConfig failed for %X %X failed 0x%x", Bus, Slot, dwStatus);
        return FALSE;
    }
    return TRUE;
}

BOOL CPCICard::SetPCIConfig(PCI_COMMON_CONFIG* pPCI_COMMON_CONFIG, DWORD Bus, DWORD Slot)
{
    if(pPCI_COMMON_CONFIG == NULL)
    {
        LOG(1, "SetPCIConfig failed. pPCI_COMMON_CONFIG == NULL");
        return FALSE;
    }

    TDSDrvParam hwParam;
    DWORD dwStatus;
    DWORD dwLength;

    hwParam.dwAddress = Bus;
    hwParam.dwValue = Slot;

    dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_SETPCICONFIG,
                                        &hwParam,
                                        sizeof(hwParam),
                                        pPCI_COMMON_CONFIG,
                                        sizeof(PCI_COMMON_CONFIG),
                                        &dwLength);

    if(dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "SetPCIConfig failed for %X %X failed 0x%x", Bus, Slot, dwStatus);
        return FALSE;
    }
    return TRUE;
}

BOOL CPCICard::GetPCIConfigOffset(BYTE* pbPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot)
{
    if(pbPCIConfig == NULL)
    {
        LOG(1, "GetPCIConfigOffset failed. pbPCIConfig == NULL");
        return FALSE;
    }

    TDSDrvParam hwParam;
    DWORD dwStatus;
    DWORD dwLength;

    hwParam.dwAddress = Bus;
    hwParam.dwValue = Slot;
    hwParam.dwFlags = Offset;

    dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_GETPCICONFIGOFFSET,
                                        &hwParam,
                                        sizeof(hwParam),
                                        pbPCIConfig,
                                        1,
                                        &dwLength);

    if(dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "GetPCIConfigOffet failed for %X %X failed 0x%x", Bus, Slot, dwStatus);
        return FALSE;
    }
    return TRUE;
}

BOOL CPCICard::SetPCIConfigOffset(BYTE* pbPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot)
{
    if(pbPCIConfig == NULL)
    {
        LOG(1, "SetPCIConfigOffset failed. pbPCIConfig == NULL");
        return FALSE;
    }

    TDSDrvParam hwParam;
    DWORD dwStatus;
    DWORD dwLength;

    hwParam.dwAddress = Bus;
    hwParam.dwValue = Slot;
    hwParam.dwFlags = Offset;

    dwStatus = m_pDriver->SendCommand(IOCTL_DSDRV_SETPCICONFIGOFFSET,
                                        &hwParam,
                                        sizeof(hwParam),
                                        pbPCIConfig,
                                        1,
                                        &dwLength);

    if(dwStatus != ERROR_SUCCESS)
    {
        LOG(1, "SetPCIConfigOffset failed for %X %X failed 0x%x", Bus, Slot, dwStatus);
        return FALSE;
    }
    return TRUE;
}


void CPCICard::SaveState()
{
    char TempPath[MAX_PATH];
    char TempFileName[MAX_PATH];

    if(GetTempPath(MAX_PATH, TempPath) == 0)
    {
        LOG(0, "Can't create Temp path for saving state");
        return;
    }

    if(GetTempFileName(TempPath, "DS4", 0, TempFileName) == 0)
    {
        LOG(0, "Can't create Temp file name for saving state");
        return;
    }

    m_hStateFile = CreateFile(
                                TempFileName, 
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                                NULL
                             );
    
    if(m_hStateFile == INVALID_HANDLE_VALUE)
    {
        LOG(0, "Can't create Temp file for saving state");
        return;
    }

    DWORD BytesWritten(0);

    WriteFile(m_hStateFile, &gBuildNum, 4, &BytesWritten, NULL);

    m_bStateIsReading = false;
    ManageMyState();
}

void CPCICard::RestoreState()
{
    if(m_hStateFile != INVALID_HANDLE_VALUE)
    {
        // reset the pointer to the start of the file
        SetFilePointer(m_hStateFile, 0, NULL, FILE_BEGIN);

        DWORD StoredBuildNumber(0);
        DWORD BytesRead(0);

        ReadFile(m_hStateFile, &StoredBuildNumber, 4, &BytesRead, NULL);

        if(StoredBuildNumber == gBuildNum)
        {
            m_bStateIsReading = true;
            ManageMyState();
        }
        else
        {
            LOG(0, "Inavlid version on state file");
        }

        CloseHandle(m_hStateFile);
    }
}

void CPCICard::ManageDword(DWORD Offset)
{
    if(m_hStateFile != INVALID_HANDLE_VALUE)
    {
        if(m_bStateIsReading)
        {
            DWORD ReadData(0);
            DWORD BytesRead(0);
            ReadFile(m_hStateFile, &ReadData, sizeof(DWORD), &BytesRead, NULL);
            if(BytesRead == sizeof(DWORD))
            {
                WriteDword(Offset, ReadData);
            }
        }
        else
        {
            DWORD WriteData(0);
            DWORD BytesWritten(0);

            WriteData = ReadDword(Offset);
            WriteFile(m_hStateFile, &WriteData, sizeof(DWORD), &BytesWritten, NULL);
        }
    }
}

void CPCICard::ManageWord(DWORD Offset)
{
    if(m_hStateFile != INVALID_HANDLE_VALUE)
    {
        if(m_bStateIsReading)
        {
            WORD ReadData(0);
            DWORD BytesRead(0);
            ReadFile(m_hStateFile, &ReadData, sizeof(WORD), &BytesRead, NULL);
            if(BytesRead == sizeof(WORD))
            {
                WriteWord(Offset, ReadData);
            }
        }
        else
        {
            WORD WriteData(0);
            DWORD BytesWritten(0);

            WriteData = ReadWord(Offset);
            WriteFile(m_hStateFile, &WriteData, sizeof(WORD), &BytesWritten, NULL);
        }
    }
}

void CPCICard::ManageByte(DWORD Offset)
{
    if(m_hStateFile != INVALID_HANDLE_VALUE)
    {
        if(m_bStateIsReading)
        {
            BYTE ReadData(0);
            DWORD BytesRead(0);
            ReadFile(m_hStateFile, &ReadData, sizeof(BYTE), &BytesRead, NULL);
            if(BytesRead == sizeof(BYTE))
            {
                WriteByte(Offset, ReadData);
            }
        }
        else
        {
            BYTE WriteData(0);
            DWORD BytesWritten(0);

            WriteData = ReadByte(Offset);
            WriteFile(m_hStateFile, &WriteData, sizeof(BYTE), &BytesWritten, NULL);
        }
    }
}


// this functions returns 0 if the card is in ACPI state D0 or on error
// returns 3 if in D3 state (full off)
int CPCICard::GetACPIStatus()
{
    // only some cards are able to power down
    if(!SupportsACPI())
    {
        return 0;
    }
    
    BYTE ACPIStatus = 0;
    if(GetPCIConfigOffset(&ACPIStatus, 0x50, m_BusNumber, m_SlotNumber))
    {
        ACPIStatus = ACPIStatus & 3;

        LOG(1, "Bus %d Card %d ACPI status: D%d", m_BusNumber, m_SlotNumber, ACPIStatus);
        return ACPIStatus;
    }

    return 0;
}

// Set ACPIStatus to 0 for D0/full on state. 3 for D3/full off
void CPCICard::SetACPIStatus(int ACPIStatus)
{
    // only some cards are able to power down
    if(!SupportsACPI())
    {
        return;
    }

    BYTE ACPIStatusNew = 0;
    if(GetPCIConfigOffset(&ACPIStatusNew, 0x50, m_BusNumber, m_SlotNumber))
    {
        ACPIStatusNew &= ~3;
        ACPIStatusNew |= ACPIStatus;

        LOG(1, "Attempting to set Bus %d Card %d ACPI status to D%d", m_BusNumber, m_SlotNumber, ACPIStatusNew);

        SetPCIConfigOffset(&ACPIStatusNew, 0x50, m_BusNumber, m_SlotNumber);

        if(ACPIStatus == 0)
        {
            ::Sleep(500);
            ResetChip();
        }
        LOG(1, "Set ACPI status complete");
    }
}
void CPCICard::LockCard()
{
    EnterCriticalSection(&m_CriticalSection);
}

void CPCICard::UnlockCard()
{
    LeaveCriticalSection(&m_CriticalSection);
}

