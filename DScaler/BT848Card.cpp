/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.cpp,v 1.1 2001-08-13 12:05:12 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "DebugLog.h"

CBT848Card::CBT848Card(CHardwareDriver* pDriver) :
	CPCICard(pDriver)
{
}

CBT848Card::~CBT848Card()
{
}

BOOL CBT848Card::FindCard(int CardIndex)
{
	return TRUE;
}

void CloseCard()
{
}

void CBT848Card::SetDMA(BOOL bState)
{
    if (bState)
    {
        OrDataWord(BT848_GPIO_DMA_CTL, 3);
    }
    else
    {
        AndDataWord(BT848_GPIO_DMA_CTL, ~3);
    }
}


void CBT848Card::SetVideoSource(eVideoSourceType nInput)
{
    DWORD MuxSel;
    // 0= Tuner,
    // 1= Composite,
    // 2= SVideo,
    // 3= Other 1
    // 4= Other 2
    // 5= Composite via SVideo

    AndOrDataDword(BT848_GPIO_OUT_EN, GetCardSetup()->GPIOMuxMask, ~GetCardSetup()->GPIOMuxMask);
    AndDataByte(BT848_IFORM, ~BT848_IFORM_MUXSEL);

    // set the comp bit for svideo
    switch (nInput)
    {
    case SOURCE_TUNER:
        AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[GetCardSetup()->TunerInput & 7];
        break;
    case SOURCE_SVIDEO:
        OrDataByte(BT848_E_CONTROL, BT848_CONTROL_COMP);
        OrDataByte(BT848_O_CONTROL, BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[GetCardSetup()->SVideoInput & 7];
        break;
    case SOURCE_COMPVIASVIDEO:
        AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[GetCardSetup()->SVideoInput & 7];
        break;
    case SOURCE_COMPOSITE:
    case SOURCE_OTHER1:
    case SOURCE_OTHER2:
    case SOURCE_CCIR656_1:
    case SOURCE_CCIR656_2:
    case SOURCE_CCIR656_3:
    case SOURCE_CCIR656_4:
    default:
        AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup()->MuxSelect[nInput];
        break;
    }
    
    MaskDataByte(BT848_IFORM, (BYTE) (((MuxSel) & 3) << 5), BT848_IFORM_MUXSEL);
    AndOrDataDword(BT848_GPIO_DATA, MuxSel >> 4, ~GetCardSetup()->GPIOMuxMask);
}

void CBT848Card::SetCardType(eTVCardId CardType)
{
    m_CardType = CardType;
}

eTVCardId CBT848Card::GetCardType()
{
    return m_CardType;
}

void CBT848Card::SetTunerType(eTunerId TunerType)
{
    m_TunerType = TunerType;
}

eTunerId CBT848Card::GetTunerType()
{
    return m_TunerType;
}


void CBT848Card::SetBrightness(BYTE Brightness)
{
    WriteByte(BT848_BRIGHT, Brightness);
}

BYTE CBT848Card::GetBrightness()
{
    return ReadByte(BT848_BRIGHT);
}

void CBT848Card::SetWhiteCrushUp(BYTE WhiteCrushUp)
{
    WriteByte(BT848_WC_UP, WhiteCrushUp);
}

BYTE CBT848Card::GetWhiteCrushUp()
{
    return ReadByte(BT848_WC_UP);
}

void CBT848Card::SetWhiteCrushDown(BYTE WhiteCrushDown)
{
    WriteByte(BT848_WC_DOWN, WhiteCrushDown);
}

BYTE CBT848Card::GetWhiteCrushDown()
{
    return ReadByte(BT848_WC_DOWN);
}

void CBT848Card::SetHue(BYTE Hue)
{
    WriteByte(BT848_HUE, Hue);
}

BYTE CBT848Card::GetHue()
{
    return ReadByte(BT848_HUE);
}

void CBT848Card::SetContrast(WORD Contrast)
{
    WriteByte(BT848_CONTRAST_LO, (BYTE) (Contrast & 0xff));
    if(Contrast > 0xff)
    {
        MaskDataByte(BT848_E_CONTROL, BT848_CONTROL_CON_MSB, BT848_CONTROL_CON_MSB);
        MaskDataByte(BT848_O_CONTROL, BT848_CONTROL_CON_MSB, BT848_CONTROL_CON_MSB);
    }
    else
    {
        MaskDataByte(BT848_E_CONTROL, 0, BT848_CONTROL_CON_MSB);
        MaskDataByte(BT848_O_CONTROL, 0, BT848_CONTROL_CON_MSB);
    }
}

WORD CBT848Card::GetContrast(WORD Contrast)
{
    if(ReadByte(BT848_E_CONTROL) & BT848_CONTROL_CON_MSB)
    {
        return 0x100 + ReadByte(BT848_CONTRAST_LO);
    }
    else
    {
        return ReadByte(BT848_CONTRAST_LO);
    }
}

void CBT848Card::SetSaturationU(WORD SaturationU)
{
    WriteByte(BT848_SAT_U_LO, (BYTE) (SaturationU & 0xff));
    if(SaturationU > 0xff)
    {
        MaskDataByte(BT848_E_CONTROL, BT848_CONTROL_SAT_U_MSB, BT848_CONTROL_SAT_U_MSB);
        MaskDataByte(BT848_O_CONTROL, BT848_CONTROL_SAT_U_MSB, BT848_CONTROL_SAT_U_MSB);
    }
    else
    {
        MaskDataByte(BT848_E_CONTROL, 0, BT848_CONTROL_SAT_U_MSB);
        MaskDataByte(BT848_O_CONTROL, 0, BT848_CONTROL_SAT_U_MSB);
    }
}

WORD CBT848Card::GetSaturationU(WORD SaturationU)
{
    if(ReadByte(BT848_E_CONTROL) & BT848_CONTROL_SAT_U_MSB)
    {
        return 0x100 + ReadByte(BT848_SAT_U_LO);
    }
    else
    {
        return ReadByte(BT848_SAT_U_LO);
    }
}

void CBT848Card::SetSaturationV(WORD SaturationV)
{
    WriteByte(BT848_SAT_V_LO, (BYTE) (SaturationV & 0xff));
    if(SaturationV > 0xff)
    {
        MaskDataByte(BT848_E_CONTROL, BT848_CONTROL_SAT_V_MSB, BT848_CONTROL_SAT_V_MSB);
        MaskDataByte(BT848_O_CONTROL, BT848_CONTROL_SAT_V_MSB, BT848_CONTROL_SAT_V_MSB);
    }
    else
    {
        MaskDataByte(BT848_E_CONTROL, 0, BT848_CONTROL_SAT_V_MSB);
        MaskDataByte(BT848_O_CONTROL, 0, BT848_CONTROL_SAT_V_MSB);
    }
}

WORD CBT848Card::GetSaturationV(WORD SaturationV)
{
    if(ReadByte(BT848_E_CONTROL) & BT848_CONTROL_SAT_V_MSB)
    {
        return 0x100 + ReadByte(BT848_SAT_V_LO);
    }
    else
    {
        return ReadByte(BT848_SAT_V_LO);
    }
}

void CBT848Card::SetBDelay(BYTE BDelay)
{
    WriteByte(BT848_BDELAY, BDelay);
}

BYTE CBT848Card::GetBDelay()
{
    return ReadByte(BT848_BDELAY);
}


