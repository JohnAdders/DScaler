/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Provider.h,v 1.8 2003-10-27 10:39:50 adcockj Exp $
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
 *@file bt848provider.h 
 */

#ifndef __BT848PROVIDER_H___
#define __BT848PROVIDER_H___

#include "SourceProvider.h"
#include "HardwareDriver.h"
#include "HardwareMemory.h"
#include "BT848Source.h"

/** The provider detects all the cards with bt848 PCI bridges and creates the
    appropriate sources for them.
*/
class CBT848Provider : public ISourceProvider
{
public:
    CBT848Provider(CHardwareDriver* pHardwareDriver);
    virtual ~CBT848Provider();
    int GetNumberOfSources();
    CSource* GetSource(int SourceIndex);
private:
    ///  uses the subsystem id to determin the correct source to create
    CBT848Source* CreateCorrectSource(
                                        CHardwareDriver* pHardwareDriver, 
                                        LPCSTR szSection, 
                                        WORD VendorID, 
                                        WORD DeviceID, 
                                        int DeviceIndex, 
                                        DWORD SubSystemId,
                                        char* ChipName);
    /// creates the system accesable memory to be used by all cards
    BOOL MemoryInit(CHardwareDriver* pHardwareDriver);
    void MemoryFree();
    vector<CBT848Source*> m_BT848Sources;
    /// Memory used for the RISC code
    CContigMemory* m_RiscDMAMem;
    /// Memory used for VBI
    CUserMemory* m_VBIDMAMem[5];
    /// Memory used for captured frames
    CUserMemory* m_DisplayDMAMem[5];
};

#endif