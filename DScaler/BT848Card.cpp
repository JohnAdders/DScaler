/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Card.cpp,v 1.4 2001-11-09 12:42:07 adcockj Exp $
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
// Revision 1.3  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.7  2001/08/22 11:12:48  adcockj
// Added VBI support
//
// Revision 1.1.2.6  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.5  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.4  2001/08/17 16:35:13  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.3  2001/08/15 07:10:19  adcockj
// Fixed memory leak
//
// Revision 1.1.2.2  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.1.2.1  2001/08/14 09:40:19  adcockj
// Interim version of code for multiple card support
//
// Revision 1.1  2001/08/13 12:05:12  adcockj
// Updated range for contrast and saturation
// Added more code for new driver interface
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "Audio.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"
// TODO: remove dependencies below
#include "ProgramList.h"
#include "OutThreads.h"

//===========================================================================
// CCIR656 Digital Input Support
//
// 13 Dec 2000 - Michael Eskin, Conexant Systems - Initial version
//
//===========================================================================
// Timing generator SRAM table values for CCIR601 720x480 NTSC
//===========================================================================

// For NTSC CCIR656 
static BYTE SRAMTable_NTSC[ 60 ] =
{
    // SRAM Timing Table for NTSC
    0x33, 
    0x0c, 0xc0, 0x00, 
    0x00, 0x90, 0xc2, 
    0x03, 0x10, 0x03, 
    0x06, 0x10, 0x34, 
    0x12, 0x12, 0x65, 
    0x02, 0x13, 0x24, 
    0x19, 0x00, 0x24, 
    0x39, 0x00, 0x96, 
    0x59, 0x08, 0x93, 
    0x83, 0x08, 0x97,
    0x03, 0x50, 0x30, 
    0xc0, 0x40, 0x30, 
    0x86, 0x01, 0x01, 
    0xa6, 0x0d, 0x62, 
    0x03, 0x11, 0x61, 
    0x05, 0x37, 0x30, 
    0xac, 0x21, 0x50
};

// For PAL CCIR656
static BYTE SRAMTable_PAL[ 60 ] =
{
    // SRAM Timing Table for PAL
    0x2d,
    0x36, 0x11, 0x01,
    0x00, 0x90, 0x02,
    0x05, 0x10, 0x04,
    0x16, 0x14, 0x05,
    0x11, 0x00, 0x04,
    0x12, 0xc0, 0x00,
    0x31, 0x00, 0x06,
    0x51, 0x08, 0x03,
    0x89, 0x08, 0x07,
    0xc0, 0x44, 0x00,
    0x81, 0x01, 0x01,
    0xa9, 0x0d, 0x02,
    0x02, 0x50, 0x03,
    0x37, 0x3d, 0x00,
    0xaf, 0x21, 0x00,
};



CBT848Card::CBT848Card(CHardwareDriver* pDriver) :
	CPCICard(pDriver),
    m_CardType(TVCARD_UNKNOWN),
    m_TunerType(TUNER_ABSENT),
    m_BtCardType(BT878),
    m_bHasMSP(FALSE),
    m_TunerDevice(0)

{
   strcpy(m_TunerStatus, "No Device on I2C-Bus");
   strcpy(m_MSPVersion,"No MSP");
}

CBT848Card::~CBT848Card()
{
    ClosePCICard();
}

BOOL CBT848Card::FindCard(eCardType BtCardType, int CardIndex)
{
    WORD VendorId(0);
    WORD DeviceId(0);
    m_BtCardType = BtCardType;

    switch(BtCardType)
    {
    case BT848:
        VendorId = 0x109e;
        DeviceId = 0x0350;
        break;
    case BT849:
        VendorId = 0x109e;
        DeviceId = 0x0351;
        break;
    case BT878:
        VendorId = 0x109e;
        DeviceId = 0x036e;
        break;
    case BT878A:
        VendorId = 0x109e;
        DeviceId = 0x036f;
        break;
    default:
        break;
    }

	BOOL bRetVal = OpenPCICard(VendorId, DeviceId, CardIndex);
    if(bRetVal == TRUE)
    {
        InitMSP();
    }

    return bRetVal;
}

void CBT848Card::CloseCard()
{
    ClosePCICard();
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


void CBT848Card::ResetHardware(DWORD RiscBasePhysical)
{
    SetDMA(FALSE);
    WriteByte(BT848_SRESET, 0);
    Sleep(100);

    WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
    WriteByte(BT848_CAP_CTL, 0x00);
    WriteByte(BT848_VBI_PACK_SIZE, (VBI_SPL / 4) & 0xff);
    WriteByte(BT848_VBI_PACK_DEL, (VBI_SPL / 4) >> 8);
    WriteWord(BT848_GPIO_DMA_CTL, 0xfc);
    WriteByte(BT848_IFORM, BT848_IFORM_MUX1 | BT848_IFORM_XTAUTO | BT848_IFORM_PAL_BDGHI);

    WriteByte(BT848_TDEC, 0x00);

    WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
    WriteDword(BT848_INT_MASK, 0);
    
    SetPLL(PLL_NONE);
}


void CBT848Card::SetVideoSource(eCardType BtCardType, eVideoSourceType nInput)
{
    DWORD MuxSel;
    // 0= Tuner,
    // 1= Composite,
    // 2= SVideo,
    // 3= Other 1
    // 4= Other 2
    // 5= Composite via SVideo

    AndOrDataDword(BT848_GPIO_OUT_EN, GetCardSetup(BtCardType)->GPIOMuxMask, ~GetCardSetup(BtCardType)->GPIOMuxMask);
    AndDataByte(BT848_IFORM, (BYTE)~BT848_IFORM_MUXSEL);

    // set the comp bit for svideo
    switch (nInput)
    {
    case SOURCE_TUNER:
        AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup(BtCardType)->MuxSelect[GetCardSetup(BtCardType)->TunerInput & 7];
        break;
    case SOURCE_SVIDEO:
        OrDataByte(BT848_E_CONTROL, BT848_CONTROL_COMP);
        OrDataByte(BT848_O_CONTROL, BT848_CONTROL_COMP);
        MuxSel = GetCardSetup(BtCardType)->MuxSelect[GetCardSetup(BtCardType)->SVideoInput & 7];
        break;
    case SOURCE_COMPVIASVIDEO:
        AndDataByte(BT848_E_CONTROL, ~BT848_CONTROL_COMP);
        AndDataByte(BT848_O_CONTROL, ~BT848_CONTROL_COMP);
        MuxSel = GetCardSetup(BtCardType)->MuxSelect[GetCardSetup(BtCardType)->SVideoInput & 7];
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
        MuxSel = GetCardSetup(BtCardType)->MuxSelect[nInput];
        break;
    }
    LOG(1, " Test %d", BT848_IFORM);
    
    MaskDataByte(BT848_IFORM, (BYTE) (((MuxSel) & 3) << 5), BT848_IFORM_MUXSEL);
    AndOrDataDword(BT848_GPIO_DATA, MuxSel >> 4, ~GetCardSetup(BtCardType)->GPIOMuxMask);
}

void CBT848Card::SetCardType(eTVCardId CardType)
{
    m_CardType = CardType;
}

eTVCardId CBT848Card::GetCardType()
{
    return m_CardType;
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


LPCSTR CBT848Card::GetChipType()
{
    switch(m_CardType)
    {
    case BT848:
        return "BT848";
        break;
    case BT849:
        return "BT849";
        break;
    case BT878:
        return "BT878";
        break;
    case BT878A:
        return "BT878A";
        break;
    default:
        return "Unknown";
        break;
    }
}

void CBT848Card::RestartRISCCode(DWORD RiscBasePhysical)
{
    BYTE CapCtl = ReadByte(BT848_CAP_CTL);
    MaskDataByte(BT848_CAP_CTL, 0, (BYTE) 0x0f);
    WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
    WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
    WriteByte(BT848_CAP_CTL, CapCtl);
}



void CBT848Card::SetEvenLumaDec(BOOL EvenLumaDec)
{
    if(EvenLumaDec)
    {
        MaskDataByte(BT848_E_CONTROL, BT848_CONTROL_LDEC, BT848_CONTROL_LDEC);
    }
    else
    {
        MaskDataByte(BT848_E_CONTROL, 0, BT848_CONTROL_LDEC);
    }
}

BOOL CBT848Card::GetEvenLumaDec()
{
    return (ReadByte(BT848_E_CONTROL) & BT848_CONTROL_LDEC) != 0;
}

void CBT848Card::SetOddLumaDec(BOOL OddLumaDec)
{
    if(OddLumaDec)
    {
        MaskDataByte(BT848_O_CONTROL, BT848_CONTROL_LDEC, BT848_CONTROL_LDEC);
    }
    else
    {
        MaskDataByte(BT848_O_CONTROL, 0, BT848_CONTROL_LDEC);
    }
}

BOOL CBT848Card::GetOddLumaDec()
{
    return ReadByte(BT848_O_CONTROL) & BT848_CONTROL_LDEC;
}

void CBT848Card::SetEvenChromaAGC(BOOL EvenChromaAGC)
{
    if(EvenChromaAGC)
    {
        MaskDataByte(BT848_E_SCLOOP, BT848_SCLOOP_CAGC, BT848_SCLOOP_CAGC);
    }
    else
    {
        MaskDataByte(BT848_E_SCLOOP, 0, BT848_SCLOOP_CAGC);
    }
}

BOOL CBT848Card::GetEvenChromaAGC()
{
    return (ReadByte(BT848_E_SCLOOP) & BT848_SCLOOP_CAGC) != 0;
}

void CBT848Card::SetOddChromaAGC(BOOL OddChromaAGC)
{
    if(OddChromaAGC)
    {
        MaskDataByte(BT848_O_SCLOOP, BT848_SCLOOP_CAGC, BT848_SCLOOP_CAGC);
    }
    else
    {
        MaskDataByte(BT848_O_SCLOOP, 0, BT848_SCLOOP_CAGC);
    }
}

BOOL CBT848Card::GetOddChromaAGC()
{
    return (ReadByte(BT848_O_SCLOOP) & BT848_SCLOOP_CAGC) != 0;
}

void CBT848Card::SetEvenLumaPeak(BOOL EvenLumaPeak)
{
    if(EvenLumaPeak)
    {
        MaskDataByte(BT848_E_SCLOOP, BT848_SCLOOP_LUMA_PEAK, BT848_SCLOOP_LUMA_PEAK);
    }
    else
    {
        MaskDataByte(BT848_E_SCLOOP, 0, BT848_SCLOOP_LUMA_PEAK);
    }
}

BOOL CBT848Card::GetEvenLumaPeak()
{
    return (ReadByte(BT848_E_SCLOOP) & BT848_SCLOOP_LUMA_PEAK) != 0;
}

void CBT848Card::SetOddLumaPeak(BOOL OddLumaPeak)
{
    if(OddLumaPeak)
    {
        MaskDataByte(BT848_O_SCLOOP, BT848_SCLOOP_LUMA_PEAK, BT848_SCLOOP_LUMA_PEAK);
    }
    else
    {
        MaskDataByte(BT848_O_SCLOOP, 0, BT848_SCLOOP_LUMA_PEAK);
    }
}

BOOL CBT848Card::GetOddLumaPeak()
{
    return (ReadByte(BT848_O_SCLOOP) & BT848_SCLOOP_LUMA_PEAK) != 0;
}

void CBT848Card::SetColorKill(BOOL ColorKill)
{
    if(ColorKill)
    {
        MaskDataByte(BT848_E_SCLOOP, BT848_SCLOOP_CKILL, BT848_SCLOOP_CKILL);
        MaskDataByte(BT848_O_SCLOOP, BT848_SCLOOP_CKILL, BT848_SCLOOP_CKILL);
    }
    else
    {
        MaskDataByte(BT848_E_SCLOOP, 0, BT848_SCLOOP_CKILL);
        MaskDataByte(BT848_O_SCLOOP, 0, BT848_SCLOOP_CKILL);
    }
}

BOOL CBT848Card::GetColorKill()
{
    return (ReadByte(BT848_E_SCLOOP) & BT848_SCLOOP_CKILL) != 0;
}

void CBT848Card::SetHorFilter(BOOL HorFilter)
{
    if(HorFilter)
    {
        MaskDataByte(BT848_O_SCLOOP, BT848_SCLOOP_HFILT_FULL, BT848_SCLOOP_HFILT_FULL);
        MaskDataByte(BT848_E_SCLOOP, BT848_SCLOOP_HFILT_FULL, BT848_SCLOOP_HFILT_FULL);
    }
    else
    {
        MaskDataByte(BT848_O_SCLOOP, 0, BT848_SCLOOP_HFILT_FULL);
        MaskDataByte(BT848_E_SCLOOP, 0, BT848_SCLOOP_HFILT_FULL);
    }
}

BOOL CBT848Card::GetHorFilter()
{
    return (ReadByte(BT848_O_SCLOOP) & BT848_SCLOOP_HFILT_FULL) != 0;
}

void CBT848Card::SetVertFilter(BOOL VertFilter)
{
    if(VertFilter)
    {
        MaskDataByte(BT848_E_VTC, BT848_VTC_VFILT_2TAPZ, BT848_VTC_VFILT_2TAPZ);
        MaskDataByte(BT848_E_VTC, BT848_VTC_VFILT_2TAPZ, BT848_VTC_VFILT_2TAPZ);
    }
    else
    {
        MaskDataByte(BT848_E_VTC, 0, BT848_VTC_VFILT_2TAPZ);
        MaskDataByte(BT848_E_VTC, 0, BT848_VTC_VFILT_2TAPZ);
    }
}

BOOL CBT848Card::GetVertFilter()
{
    return (ReadByte(BT848_E_VTC) & BT848_VTC_VFILT_2TAPZ) != 0;
}

void CBT848Card::SetFullLumaRange(BOOL FullLumaRange)
{
    if(FullLumaRange)
    {
        MaskDataByte(BT848_OFORM, BT848_OFORM_RANGE, BT848_OFORM_RANGE);
    }
    else
    {
        MaskDataByte(BT848_OFORM, 0, BT848_OFORM_RANGE);
    }
}

BOOL CBT848Card::GetFullLumaRange()
{
    return (ReadByte(BT848_OFORM) & BT848_OFORM_RANGE) != 0;
}

void CBT848Card::SetCoring(BOOL Coring)
{
    if(Coring)
    {
        MaskDataByte(BT848_OFORM, BT848_OFORM_CORE32, BT848_OFORM_CORE32);
    }
    else
    {
        MaskDataByte(BT848_OFORM, 0, BT848_OFORM_CORE32);
    }
}

BOOL CBT848Card::GetCoring()
{
    return (ReadByte(BT848_OFORM) & BT848_OFORM_CORE32) != 0;
}

   
void CBT848Card::SetEvenComb(BOOL EvenComb)
{
    if(EvenComb)
    {
        MaskDataByte(BT848_E_VSCALE_HI, BT848_VSCALE_COMB, BT848_VSCALE_COMB);
    }
    else
    {
        MaskDataByte(BT848_E_VSCALE_HI, 0, BT848_VSCALE_COMB);
    }
}

BOOL CBT848Card::GetEvenComb()
{
    return (ReadByte(BT848_E_VSCALE_HI) & BT848_VSCALE_COMB) != 0;
}

void CBT848Card::SetOddComb(BOOL OddComb)
{
    if(OddComb)
    {
        MaskDataByte(BT848_O_VSCALE_HI, BT848_VSCALE_COMB, BT848_VSCALE_COMB);
    }
    else
    {
        MaskDataByte(BT848_O_VSCALE_HI, 0, BT848_VSCALE_COMB);
    }
}

BOOL CBT848Card::GetOddComb()
{
    return (ReadByte(BT848_O_VSCALE_HI) & BT848_VSCALE_COMB) != 0;
}

void CBT848Card::SetAgcDisable(BOOL AgcDisable)
{
    if(AgcDisable)
    {
        MaskDataByte(BT848_ADC, BT848_ADC_AGC_EN, BT848_ADC_AGC_EN);
    }
    else
    {
        MaskDataByte(BT848_ADC, 0, BT848_ADC_AGC_EN);
    }
}

BOOL CBT848Card::GetAgcDisable()
{
    return (ReadByte(BT848_ADC) & BT848_ADC_AGC_EN) != 0;
}

void CBT848Card::SetCrush(BOOL Crush)
{
    if(Crush)
    {
        MaskDataByte(BT848_ADC, BT848_ADC_CRUSH, BT848_ADC_CRUSH);
    }
    else
    {
        MaskDataByte(BT848_ADC, 0, BT848_ADC_CRUSH);
    }
}

BOOL CBT848Card::GetCrush()
{
    return (ReadByte(BT848_ADC) & BT848_ADC_CRUSH) != 0;
}


void CBT848Card::SetColorBars(BOOL ColorBars)
{
    if(ColorBars)
    {
        MaskDataByte(BT848_COLOR_CTL, BT848_COLOR_CTL_COLOR_BARS, BT848_COLOR_CTL_COLOR_BARS);
    }
    else
    {
        MaskDataByte(BT848_COLOR_CTL, 0, BT848_COLOR_CTL_COLOR_BARS);
    }
}

BOOL CBT848Card::GetColorBars()
{
    return (ReadByte(BT848_COLOR_CTL) & BT848_COLOR_CTL_COLOR_BARS) != 0;
}

void CBT848Card::SetGammaCorrection(BOOL GammaCorrection)
{
    if(GammaCorrection)
    {
        MaskDataByte(BT848_COLOR_CTL, BT848_COLOR_CTL_GAMMA, BT848_COLOR_CTL_GAMMA);
    }
    else
    {
        MaskDataByte(BT848_COLOR_CTL, 0, BT848_COLOR_CTL_GAMMA);
    }
}

BOOL CBT848Card::GetGammaCorrection()
{
    return (ReadByte(BT848_COLOR_CTL) & BT848_COLOR_CTL_GAMMA) != 0;
}


//-------------------------------
void CBT848Card::SetGeoSize(eCardType BtCardType, eVideoSourceType nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelay, int HDelay)
{
    int vscale;
    int hscale;
    DWORD sr;
    int hdelay;
    int vdelay;
    int hactive;
    int vactive;
    BYTE crop;

    CurrentY = GetTVFormat(TVFormat)->wCropHeight;
    CurrentVBILines = GetTVFormat(TVFormat)->VBILines;

    if(IsCCIRSource(nInput))
    {
        int i;

        CurrentX = 720;

        // Disable TG Mode
        MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_ENABLE);

        // Reset the TG address
        MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_RESET);
        MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGMODE_RESET, BT848_TGCTRL_TGMODE_RESET);
        MaskDataByte(BT848_TGCTRL, 0, BT848_TGCTRL_TGMODE_RESET);

        // MAE 20Mar2001
        if (CurrentY == 576)
        {
            // Load up the TG table for CCIR656
            for (i=0;i<SRAMTable_PAL[0];++i)
            {
                WriteByte(BT848_TBLG, SRAMTable_PAL[i+1]);
            }
        }
        else
        {
            // Load up the TG table for CCIR656
            for (i=0;i<SRAMTable_NTSC[0];++i)
            {
                WriteByte(BT848_TBLG, SRAMTable_NTSC[i+1]);
            }
        }

        // Enable TG Mode
        MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGMODE_ENABLE, BT848_TGCTRL_TGMODE_ENABLE);

        // Enable the GPCLOCK
        MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_GPCLK, BT848_TGCTRL_TGCKI_GPCLK);

        // Set the PLL Mode
        WriteByte(BT848_PLL_XCI, 0x00);

        // Enable 656 Mode, bypass chroma filters
        WriteByte(BT848_DVSIF, BT848_DVSIF_VSIF_BCF | BT848_DVSIF_CCIR656);

        // MAE 20Mar2001
        if (CurrentY == 576)
        {
            // Enable PAL Mode (or SECAM)
            MaskDataByte(BT848_IFORM, (BT848_IFORM_PAL_BDGHI | BT848_IFORM_XTBOTH), (BT848_IFORM_NORM | BT848_IFORM_XTBOTH));
        }
        else
        {
            // Enable NTSC Mode (or PAL60)
            MaskDataByte(BT848_IFORM, (BT848_IFORM_NTSC | BT848_IFORM_XTBOTH), (BT848_IFORM_NORM | BT848_IFORM_XTBOTH));
        }

        // Disable full range luma
        WriteByte(BT848_OFORM, 0);

        // Enable the SC loop luma peaking filters
        WriteByte(BT848_E_SCLOOP, BT848_SCLOOP_LUMA_PEAK);
        WriteByte(BT848_O_SCLOOP, BT848_SCLOOP_LUMA_PEAK);

        // Standard NTSC 525 line Count
        WriteByte(BT848_VTOTAL_LO, 0x00);
        WriteByte(BT848_VTOTAL_HI, 0x00);

        // Setup parameters for overlay scale and crop calculation
        hactive = CurrentX;
        vactive = CurrentY;
        hscale = 0;
        vdelay = 16;
        hdelay = 0x80;
        vscale = 0;

        WriteByte(BT848_E_VTC, BT848_VTC_HSFMT_32);
        WriteByte(BT848_O_VTC, BT848_VTC_HSFMT_32);

    }
    else
    {
        // set the pll on the card if appropriate
        if(GetTVFormat(TVFormat)->NeedsPLL == TRUE && GetCardSetup(BtCardType)->pll != PLL_NONE)
        {
            SetPLL(GetCardSetup(BtCardType)->pll);
        }
        else
        {
            SetPLL(PLL_NONE);
        }

        WriteByte(BT848_ADELAY, GetTVFormat(TVFormat)->bDelayA);
        WriteByte(BT848_BDELAY, GetTVFormat(TVFormat)->bDelayB);

        WriteByte(BT848_VBI_PACK_SIZE, (BYTE)(GetTVFormat(TVFormat)->VBIPacketSize & 0xff));
        WriteByte(BT848_VBI_PACK_DEL, (BYTE)(GetTVFormat(TVFormat)->VBIPacketSize >> 8));
        MaskDataByte(BT848_IFORM, GetTVFormat(TVFormat)->bIForm, BT848_IFORM_NORM | BT848_IFORM_XTBOTH);
        hactive = CurrentX & ~2;
        if(CurrentX <= GetTVFormat(TVFormat)->wHActivex1)
        {
            hscale = ((GetTVFormat(TVFormat)->wHActivex1 - CurrentX) * 4096UL) / CurrentX;
        }
        else
        {
            CurrentX = GetTVFormat(TVFormat)->wHActivex1;
            hscale = 0;
        }
        if(VDelay == 0)
        {
            vdelay = GetTVFormat(TVFormat)->wVDelay;
        }
        else
        {
            vdelay = VDelay;
        }
        if(HDelay == 0)
        {
            hdelay = ((CurrentX * GetTVFormat(TVFormat)->wHDelayx1) / GetTVFormat(TVFormat)->wHActivex1) & 0x3fe;
        }
        else
        {
            hdelay = ((CurrentX * HDelay) / GetTVFormat(TVFormat)->wHActivex1) & 0x3fe;
        }

        if(TVFormat == FORMAT_PAL60)
        {
            WriteByte(BT848_VTOTAL_LO, (BYTE)(525 & 0xff));
            WriteByte(BT848_VTOTAL_HI, (BYTE)(525 >> 8));
        }
        else if(TVFormat == FORMAT_NTSC50)
        {
            WriteByte(BT848_VTOTAL_LO, (BYTE)(625 & 0xff));
            WriteByte(BT848_VTOTAL_HI, (BYTE)(625 >> 8));
        }

        sr = (GetTVFormat(TVFormat)->wCropHeight * 512) / CurrentY - 512;
        vscale = (WORD) (0x10000UL - sr) & 0x1fff;
        vactive = GetTVFormat(TVFormat)->wCropHeight;

    }

    // YUV 4:2:2 linear pixel format
    WriteByte(BT848_COLOR_FMT, (BYTE)((BT848_COLOR_FMT_YUY2 << 4) | BT848_COLOR_FMT_YUY2));
    crop = ((hactive >> 8) & 0x03) | ((hdelay >> 6) & 0x0c) | ((vactive >> 4) & 0x30) | ((vdelay >> 2) & 0xc0);
    SetGeometryEvenOdd(FALSE, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
    SetGeometryEvenOdd(TRUE, hscale, vscale, hactive, vactive, hdelay, vdelay, crop);
}


void CBT848Card::SetGeometryEvenOdd(BOOL bOdd, int wHScale, int wVScale, int wHActive, int wVActive, int wHDelay, int wVDelay, BYTE bCrop)
{
    int nOff = bOdd ? 0x80 : 0x00;

    WriteByte(BT848_E_HSCALE_HI + nOff, (BYTE) (wHScale >> 8));
    WriteByte(BT848_E_HSCALE_LO + nOff, (BYTE) (wHScale & 0xFF));
    MaskDataByte(BT848_E_VSCALE_HI + nOff, (BYTE) (wVScale >> 8), 0x1F);
    WriteByte(BT848_E_VSCALE_LO + nOff, (BYTE) (wVScale & 0xFF));
    WriteByte(BT848_E_HACTIVE_LO + nOff, (BYTE) (wHActive & 0xFF));
    WriteByte(BT848_E_HDELAY_LO + nOff, (BYTE) (wHDelay & 0xFF));
    WriteByte(BT848_E_VACTIVE_LO + nOff, (BYTE) (wVActive & 0xFF));
    WriteByte(BT848_E_VDELAY_LO + nOff, (BYTE) (wVDelay & 0xFF));
    WriteByte(BT848_E_CROP + nOff, bCrop);
}

void CBT848Card::SetPLL(ePLLFreq PLL)
{
    int i = 6;

    // reset the TGCKI bits
    MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_NOPLL, 0x18);

    switch(PLL)
    {
    case PLL_NONE:
        WriteByte(BT848_PLL_XCI, 0x00);
        return;
        break;
    case PLL_28:
        WriteByte(BT848_PLL_F_LO, 0xf9);
        WriteByte(BT848_PLL_F_HI, 0xdc);
        WriteByte(BT848_PLL_XCI, 0x8e);
        break;
    case PLL_35:
        WriteByte(BT848_PLL_F_LO, 0x39);
        WriteByte(BT848_PLL_F_HI, 0xB0);
        WriteByte(BT848_PLL_XCI, 0x89);
        break;
    }

    // wait for the PLL to lock
    while(i-- > 0 && ReadByte(BT848_DSTATUS) & BT848_DSTATUS_PLOCK)
    {
        WriteByte(BT848_DSTATUS, 0x00);
        Sleep(100);
    }

    // Set the TGCKI bits to use PLL rather than xtal
    MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_PLL, 0x18);

    WriteByte(BT848_VTOTAL_LO, 0x00);
    WriteByte(BT848_VTOTAL_HI, 0x00);
    WriteByte(BT848_DVSIF, 0x00);
}

BOOL CBT848Card::IsCCIRSource(eVideoSourceType nInput)
{
    switch (nInput)
    {
    case SOURCE_CCIR656_1:
    case SOURCE_CCIR656_2:
    case SOURCE_CCIR656_3:
    case SOURCE_CCIR656_4:
        return TRUE;
    }
    return FALSE;
}

void CBT848Card::SetAudioSource(eCardType BtCardType, eAudioMuxType nChannel)
{
    int i;
    DWORD MuxSelect;

    AndOrDataDword(BT848_GPIO_OUT_EN, GetCardSetup(BtCardType)->GPIOMask, ~GetCardSetup(BtCardType)->GPIOMask);

    switch(nChannel)
    {
    case AUDIOMUX_MSP_RADIO:
    case AUDIOMUX_MUTE:
        // just get on with it
        MuxSelect = GetCardSetup(BtCardType)->AudioMuxSelect[nChannel];
        break;
    default:
        // see if there is a video signal present
        i = 0;
        while ((i < 20) && (!(ReadByte(BT848_DSTATUS) & BT848_DSTATUS_PRES)))
        {
            i++;
            Sleep(50);
        }
        // if video not in H-lock, turn audio off 
        if (i == 20)
        {
            MuxSelect = GetCardSetup(BtCardType)->AudioMuxSelect[AUDIOMUX_MUTE];
        }
        else
        {
            MuxSelect = GetCardSetup(BtCardType)->AudioMuxSelect[nChannel];
        }
        break;
    }

    // select direct input 
    //BT848_WriteWord(BT848_GPIO_REG_INP, 0x00); // MAE 14 Dec 2000 disabled
    AndOrDataDword(BT848_GPIO_DATA, MuxSelect, ~GetCardSetup(BtCardType)->GPIOMask); 
}

BOOL CBT848Card::IsVideoPresent()
{
    return ((ReadByte(BT848_DSTATUS) & (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) == (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) ? TRUE : FALSE;
}


void CBT848Card::SetRISCStartAddress(DWORD RiscBasePhysical)
{
    WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
}

DWORD CBT848Card::GetRISCPos()
{
    return ReadDword(BT848_RISC_COUNT);
}

void CBT848Card::I2C_SetLine(BOOL bCtrl, BOOL bData)
{
    WriteDword(BT848_I2C, (bCtrl << 1) | bData);
    I2C_Wait(BT848_I2C_DELAY);
}

BOOL CBT848Card::I2C_GetLine()
{
    return ReadDword(BT848_I2C) & 1;
}

BYTE CBT848Card::I2C_Read(BYTE nAddr)
{
    DWORD i;
    volatile DWORD stat;

    WriteDword(BT848_INT_STAT, BT848_INT_I2CDONE);
    WriteDword(BT848_I2C, (nAddr << 24) | BT848_I2C_COMMAND);

    for (i = 0x7fffffff; i; i--)
    {
        stat = ReadDword(BT848_INT_STAT);
        if (stat & BT848_INT_I2CDONE)
            break;
    }

    if (!i)
        return (BYTE) - 1;
    if (!(stat & BT848_INT_RACK))
        return (BYTE) - 2;

    return (BYTE) ((ReadDword(BT848_I2C) >> 8) & 0xFF);
}

BOOL CBT848Card::I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth)
{
    DWORD i;
    DWORD data;
    DWORD stat;

    WriteDword(BT848_INT_STAT, BT848_INT_I2CDONE);

    data = (nAddr << 24) | (nData1 << 16) | BT848_I2C_COMMAND;
    if (bSendBoth)
        data |= (nData2 << 8) | BT848_I2C_W3B;
    WriteDword(BT848_I2C, data);

    for (i = 0x7fffffff; i; i--)
    {
        stat = ReadDword(BT848_INT_STAT);
        if (stat & BT848_INT_I2CDONE)
            break;
    }

    if (!i)
        return FALSE;
    if (!(stat & BT848_INT_RACK))
        return FALSE;

    return TRUE;
}

void CBT848Card::StopCapture()
{
    MaskDataByte(BT848_CAP_CTL, 0, 0x0f);
}

void CBT848Card::StartCapture(BOOL bCaptureVBI)
{
    int nFlags = BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
    if (bCaptureVBI == TRUE)
    {
        nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
    }
    MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);
}


const char* CBT848Card::GetSourceName(eVideoSourceType nVideoSource)
{
    switch (nVideoSource)
    {
    case SOURCE_TUNER:         return Channel_GetName(); break;
    case SOURCE_COMPOSITE:     return "Composite"; break;
    case SOURCE_SVIDEO:        return "S-Video"; break;
    case SOURCE_OTHER1:        return "Other 1"; break;
    case SOURCE_OTHER2:        return "Other 2"; break;
    case SOURCE_COMPVIASVIDEO: return "Composite via S-Video"; break;
    case SOURCE_CCIR656_1:     return "CCIR656 1"; break;
    case SOURCE_CCIR656_2:     return "CCIR656 2"; break;
    case SOURCE_CCIR656_3:     return "CCIR656 3"; break;
    case SOURCE_CCIR656_4:     return "CCIR656 4"; break;
    }
    return "Unknown";
}


BOOL APIENTRY CBT848Card::ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    CBT848Card* pThis = NULL;
    char szCardId[9] = "None    ";
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Card*)lParam; 
        SetDlgItemText(hDlg, IDC_TEXT1, pThis->GetChipType());
        SetDlgItemText(hDlg, IDC_TEXT13, pThis->m_TunerStatus);
        SetDlgItemText(hDlg, IDC_TEXT16, pThis->m_MSPVersion);
        dwCardId = pThis->GetSubSystemId();
        if(dwCardId != 0 && dwCardId != 0xffffffff)
        {
            sprintf(szCardId,"%8X", dwCardId);
        }
        SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);

        SetDlgItemText(hDlg, IDC_TEXT18, "YUV2");
        
        // TB 20010109 added Chip Type
        if (CpuFeatureFlags & FEATURE_SSE2)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE2");
        }
        else if (CpuFeatureFlags & FEATURE_SSE)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE");
        }
        else if (CpuFeatureFlags & FEATURE_MMXEXT)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMXEXT");
        }
        else if (CpuFeatureFlags & FEATURE_3DNOWEXT)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOWEXT");
        }
        else if (CpuFeatureFlags & FEATURE_3DNOW)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOW");
        }
        else
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMX");
        }


        break;

    case WM_COMMAND:

        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            EndDialog(hDlg, TRUE);
        }

        break;
    }

    return (FALSE);
}

