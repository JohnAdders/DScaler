/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Provider.h,v 1.2 2001-11-02 16:30:07 adcockj Exp $
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

#ifndef __BT848PROVIDER_H___
#define __BT848PROVIDER_H___

#include "SourceProvider.h"
#include "HardwareDriver.h"
#include "HardwareMemory.h"
#include "BT848Source.h"

class CBT848Provider : public CSourceProvider
{
public:
    CBT848Provider(CHardwareDriver* pHardwareDriver);
    ~CBT848Provider();
    int GetNumberOfSources();
    CInterlacedSource* GetSource(int SourceIndex);
private:
    BOOL MemoryInit(CHardwareDriver* pHardwareDriver);
    void MemoryFree();
    vector<CBT848Source*> m_BT848Sources;
    CContigMemory* m_RiscDMAMem;
    CUserMemory* m_VBIDMAMem[5];
    CUserMemory* m_DisplayDMAMem[5];
};

#endif