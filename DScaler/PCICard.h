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
 * @file pcicard.h pcicard Header file
 */

#ifndef __PCICARD_H___
#define __PCICARD_H___

#include "HardwareDriver.h"
#include "BitVector.h"

/** This class is used to provide access to the low level function provided
    by the drivers.  To use these function derive your card specific class
    from this one.
*/
class CPCICard
{
public:
    BOOL GetPCIConfigOffset(BYTE* pbPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot);
    BOOL SetPCIConfigOffset(BYTE* pbPCIConfig, DWORD Offset, DWORD Bus, DWORD Slot);
    DWORD GetSubSystemId();
    WORD GetDeviceId();
    WORD GetVendorId();
    /**  Try to find card with given attributes on system
         @return TRUE is device is found
    */
    BOOL OpenPCICard(WORD dwVendorID, WORD dwDeviceID, int dwDeviceIndex);
protected:
    CPCICard(SmartPtr<CHardwareDriver> pDriver);
    ~CPCICard();
    /// Close the card and unmap memory
    void ClosePCICard();

    /*** Must be defined in derived classes
         This function tells the state management code which
         Memory addresses to save and restore by calling the
         ManageXXXX functions.
    */
    virtual void ManageMyState() = 0;
    /** Tells the state management code to save and restore
        the DWORD at Offset.
        This function must only be called from ManageMyState
    */

    virtual BOOL SupportsACPI() = 0;
    virtual void ResetChip() = 0;

    void ManageDword(DWORD Offset);
    /** Tells the state management code to save and restore
        the WORD at Offset
        This function must only be called from ManageMyState
    */
    void ManageWord(DWORD Offset);
    /** Tells the state management code to save and restore
        the BYTE at Offset
        This function must only be called from ManageMyState
    */
    void ManageByte(DWORD Offset);

public:
    /// prevent other threads from using the card while we are in the middle of something
    /// must be called in pairs with UnlockCard
    void LockCard();
    /// release card and allow other threads to use the card
    /// must be called in pairs with LockCard
    void UnlockCard();

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

    /// Write masked data of size defined by Mask to shared memory
    void WriteData(DWORD Offset, DWORD Mask, CBitVector Data);
    /// Read masked data of size defined by Mask from shared memory
    CBitVector ReadData(DWORD Offset, DWORD Mask);
    /// State management interface for Offset and Mask register definitions
    void ManageData(DWORD Offset, DWORD Mask, CBitMask DataMask = CBitMask(0xFFFFFFFF));

    void MaskDataByte(DWORD Offset, BYTE Data, BYTE Mask);
    void MaskDataWord(DWORD Offset, WORD Data, WORD Mask);
    void MaskDataDword(DWORD Offset, DWORD Data, DWORD Mask);
    void AndOrDataByte(DWORD Offset, DWORD Data, BYTE Mask);
    void AndOrDataWord(DWORD Offset, DWORD Data, WORD Mask);
    void AndOrDataDword(DWORD Offset, DWORD Data, DWORD Mask);
    void AndDataByte(DWORD Offset, BYTE Data);
    void AndDataWord(DWORD Offset, WORD Data);
    void AndDataDword(DWORD Offset, DWORD Data);
    void OrDataByte(DWORD Offset, BYTE Data);
    void OrDataWord(DWORD Offset, WORD Data);
    void OrDataDword(DWORD Offset, DWORD Data);

private:
    /// Called from OpenPCICard to save the state of the card
    void SaveState();
    /// Called from ClosePCICard to restore the state of the card
    void RestoreState();

    void SetACPIStatus(int ACPIStatus);
    int GetACPIStatus();

protected:
    DWORD   m_SubSystemId;
    WORD    m_DeviceId;
    WORD    m_VendorId;
    DWORD   m_BusNumber;
    DWORD   m_SlotNumber;
    BOOL    m_bOpen;
    BOOL    m_SupportsACPI;

private:
    DWORD   m_MemoryAddress;
    DWORD   m_MemoryLength;
    DWORD   m_MemoryBase;
    SmartPtr<CHardwareDriver> m_pDriver;
    HANDLE  m_hStateFile;
    boolean m_bStateIsReading;
    int     m_InitialACPIStatus;
    CRITICAL_SECTION m_CriticalSection;

    BOOL GetPCIConfig(PCI_COMMON_CONFIG* pPCI_COMMON_CONFIG, DWORD Bus, DWORD Slot);
    BOOL SetPCIConfig(PCI_COMMON_CONFIG* pPCI_COMMON_CONFIG, DWORD Bus, DWORD Slot);
};


#endif
