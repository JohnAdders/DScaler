/////////////////////////////////////////////////////////////////////////////
// $Id: DVBTProvider.h,v 1.3 2001-11-29 14:04:07 adcockj Exp $
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

#ifndef __DVBTPROVIDER_H___
#define __DVBTPROVIDER_H___

#include "SourceProvider.h"
#include "HardwareDriver.h"
#include "HardwareMemory.h"
#include "DVBTSource.h"

class CDVBTProvider : public ISourceProvider
{
public:
    CDVBTProvider(CHardwareDriver* pHardwareDriver);
    ~CDVBTProvider();
    int GetNumberOfSources();
    CSource* GetSource(int SourceIndex);
private:
    BOOL MemoryInit(CHardwareDriver* pHardwareDriver);
    void MemoryFree();
    vector<CDVBTSource*> m_DVBTSources;
};

#endif
