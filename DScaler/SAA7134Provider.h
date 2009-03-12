/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
//
// This software was based on BT848Provider.h.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////

/** 
 * @file saa7134provider.h saa7134provider Header file
 */
 
#ifdef WANT_SAA713X_SUPPORT

#ifndef __SAA7134PROVIDER_H___
#define __SAA7134PROVIDER_H___

#include "SourceProvider.h"
#include "HardwareDriver.h"
#include "HardwareMemory.h"
#include "SAA7134Common.h"
#include "SAA7134Source.h"

/** The provider detects all the cards with saa7134 PCI bridges and creates the
    appropriate sources for them.
*/
class CSAA7134Provider : public ISourceProvider,
                         public CSAA7134Common
{
public:
    CSAA7134Provider(SmartPtr<CHardwareDriver> pHardwareDriver);
    virtual ~CSAA7134Provider();
    int GetNumberOfSources();
    SmartPtr<CSource> GetSource(int SourceIndex);
private:
    ///  uses the subsystem id to determin the correct source to create
    SmartPtr<CSAA7134Source> CreateCorrectSource(
                                        SmartPtr<CHardwareDriver> pHardwareDriver, 
                                        LPCSTR szSection, 
                                        WORD VendorID, 
                                        WORD DeviceID, 
                                        int DeviceIndex, 
                                        DWORD SubSystemId,
                                        char* ChipName);
    /// creates the system accesable memory to be used by all cards
    BOOL MemoryInit(SmartPtr<CHardwareDriver> pHardwareDriver);
    void MemoryFree();
    vector< SmartPtr<CSAA7134Source> > m_SAA7134Sources;
    /// Memory used for DMA page table
    SmartPtr<CContigMemory> m_PageTableDMAMem[kMAX_VID_PAGETABLES + kMAX_VBI_PAGETABLES];
    /// Memory used for VBI
    SmartPtr<CUserMemory> m_VBIDMAMem[kMAX_FRAMEBUFFERS];
    /// Memory used for captured frames
    SmartPtr<CUserMemory> m_DisplayDMAMem[kMAX_FRAMEBUFFERS];
};

#endif

#endif//xxx