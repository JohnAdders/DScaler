/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.h,v 1.1 2001-08-13 12:05:12 adcockj Exp $
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

#ifndef __BT848CARD_H___
#define __BT848CARD_H___

#include "PCICard.h"
#include "TVCards.h"
#include "BT848_Defines.h"

class CBT848Card : public CPCICard
{
public:
	CBT848Card(CHardwareDriver* pDriver);
	~CBT848Card();

	BOOL FindCard(int CardIndex);
	void CloseCard();
    
    void SetCardType(eTVCardId CardType);
    eTVCardId GetCardType();
    void SetTunerType(eTunerId TunerType);
    eTunerId GetTunerType();
    
    void SetVideoSource(eVideoSourceType nInput);

    void ResetHardware(eVideoSourceType nInput);

    void SetBrightness(BYTE Brightness);
    BYTE GetBrightness();
    void SetWhiteCrushUp(BYTE WhiteCrushUp);
    BYTE GetWhiteCrushUp();
    void SetWhiteCrushDown(BYTE WhiteCrushDown);
    BYTE GetWhiteCrushDown();
    void SetHue(BYTE Hue);
    BYTE GetHue();
    void SetContrast(WORD Contrast);
    WORD GetContrast(WORD Contrast);
    void SetSaturationU(WORD SaturationU);
    WORD GetSaturationU(WORD SaturationU);
    void SetSaturationV(WORD SaturationV);
    WORD GetSaturationV(WORD SaturationV);
    void SetBDelay(BYTE BDelay);
    BYTE GetBDelay();
private:
    void SetDMA(BOOL bState);
    eTVCardId m_CardType;
    eTunerId m_TunerType;
};

	
#endif