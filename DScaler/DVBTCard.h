/////////////////////////////////////////////////////////////////////////////
// $Id: DVBTCard.h,v 1.1 2001-11-02 16:30:07 adcockj Exp $
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

#ifndef __DVBTCARD_H___
#define __DVBTCARD_H___

#include "PCICard.h"
#include "I2C.h"
#include "TVFormats.h"
#include "DVB_Defines.h"

class CDVBTCard : public CPCICard, 
                   public CI2C
{
public:
    CDVBTCard(CHardwareDriver* pDriver);
	~CDVBTCard();

	BOOL FindCard(int CardIndex);
	void CloseCard();
    
    void ResetHardware();

    LPCSTR GetChipType();

    BOOL IsVideoPresent();
    void StopCapture();
    void StartCapture();

    BOOL SetTunerFrequency(long FrequencyId);
    BOOL InitTuner();

protected:
    void I2C_SetLine(BOOL bCtrl, BOOL bData);
    BOOL I2C_GetLine();
    BYTE I2C_Read(BYTE nAddr);
    BOOL I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth);

private:
};


#endif
