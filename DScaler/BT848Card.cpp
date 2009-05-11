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
 * @file BT848Card.cpp CBT848Card Implementation
 */

#include "stdafx.h"

#ifdef WANT_BT8X8_SUPPORT

#include "..\DScalerRes\resource.h"
#include "..\DScalerResDbg\BT8x8Res\resource.h"
#include "resource.h"
#include "BT848Card.h"
#include "BT848_Defines.h"
#include "Audio.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"
/// \todo remove need for this
#include "ProgramList.h"
/// \todo remove need for this
#include "OutThreads.h"
#include "DScaler.h"
#include "VBI.h"

using namespace std;

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



CBT848Card::CBT848Card(SmartPtr<CHardwareDriver> pDriver) :
    CPCICard(pDriver),
    m_CardType(TVCARD_UNKNOWN),
    m_CurrentInput(0),
    m_TunerType(_T("n/a")),
    m_AudioDecoderType(_T("n/a")),
    m_I2CInitialized(FALSE),
    m_I2CBus(new CI2CBusForLineInterface(this)),
    m_AudioControls(new CAudioControls()),
    m_AudioDecoder(new CAudioDecoder())
{
}

CBT848Card::~CBT848Card()
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

void CBT848Card::ManageMyState()
{
    // Drivers seem to reset most things when they run
    // so just return here especially now that we leave the chip
    // in a more shut down state
    return;

    // save and restore everything that might be used
    // by the real drivers
    ManageByte(BT848_IFORM);
    ManageByte(BT848_FCNTR);
    ManageByte(BT848_PLL_F_LO);
    ManageByte(BT848_PLL_F_HI);
    ManageByte(BT848_PLL_XCI);
    ManageByte(BT848_TGCTRL);
    ManageByte(BT848_TDEC);
    ManageByte(BT848_E_CROP);
    ManageByte(BT848_O_CROP);
    ManageByte(BT848_E_VDELAY_LO);
    ManageByte(BT848_O_VDELAY_LO);
    ManageByte(BT848_E_VACTIVE_LO);
    ManageByte(BT848_O_VACTIVE_LO);
    ManageByte(BT848_E_HDELAY_LO);
    ManageByte(BT848_O_HDELAY_LO);
    ManageByte(BT848_E_HACTIVE_LO);
    ManageByte(BT848_O_HACTIVE_LO);
    ManageByte(BT848_E_HSCALE_HI);
    ManageByte(BT848_O_HSCALE_HI);
    ManageByte(BT848_E_HSCALE_LO);
    ManageByte(BT848_O_HSCALE_LO);
    ManageByte(BT848_BRIGHT);
    ManageByte(BT848_E_CONTROL);
    ManageByte(BT848_O_CONTROL);
    ManageByte(BT848_CONTRAST_LO);
    ManageByte(BT848_SAT_U_LO);
    ManageByte(BT848_SAT_V_LO);
    ManageByte(BT848_HUE);
    ManageByte(BT848_E_SCLOOP);
    ManageByte(BT848_O_SCLOOP);
    ManageByte(BT848_WC_UP);
    ManageByte(BT848_WC_DOWN);
    ManageByte(BT848_VTOTAL_LO);
    ManageByte(BT848_VTOTAL_HI);
    ManageByte(BT848_DVSIF);
    ManageByte(BT848_OFORM);
    ManageByte(BT848_E_VSCALE_HI);
    ManageByte(BT848_O_VSCALE_HI);
    ManageByte(BT848_E_VSCALE_LO);
    ManageByte(BT848_O_VSCALE_LO);
    ManageByte(BT848_ADC);
    ManageByte(BT848_E_VTC);
    ManageByte(BT848_O_VTC);
    ManageByte(BT848_COLOR_FMT);
    ManageByte(BT848_COLOR_CTL);
    ManageByte(BT848_CAP_CTL);
    ManageByte(BT848_VBI_PACK_SIZE);
    ManageByte(BT848_VBI_PACK_DEL);
    ManageDword(BT848_INT_MASK);
    ManageDword(BT848_GPIO_OUT_EN);
    ManageDword(BT848_GPIO_OUT_EN_HIBYTE);
    ManageDword(BT848_GPIO_DATA);
    // do these ones last
    ManageDword(BT848_RISC_STRT_ADD);
    ManageWord(BT848_GPIO_DMA_CTL);
}


void CBT848Card::ResetHardware(DWORD RiscBasePhysical)
{
    SetDMA(FALSE);
    WriteByte(BT848_CAP_CTL, 0x00);

    WriteByte(BT848_SRESET, 0);
    ::Sleep(100);

    WriteByte(BT848_CAP_CTL, 0x00);
    WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
    WriteByte(BT848_VBI_PACK_SIZE, (VBI_SPL / 4) & 0xff);
    WriteByte(BT848_VBI_PACK_DEL, (VBI_SPL / 4) >> 8);
    WriteWord(BT848_GPIO_DMA_CTL, 0xfc);
    WriteByte(BT848_IFORM, BT848_IFORM_MUX1 | BT848_IFORM_XTAUTO | BT848_IFORM_PAL_BDGHI);

    WriteByte(BT848_TDEC, 0x00);

    WriteDword(BT848_INT_STAT, (DWORD) 0x0fffffff);
    WriteDword(BT848_INT_MASK, 0);

    WriteByte(BT848_ADC, BT848_ADC_RESERVED);

    SetPLL(PLL_NONE);
}

void CBT848Card::SetCardType(int CardType)
{
    if(m_CardType != CardType)
    {
        m_CardType = (eTVCardId)CardType;

        // perform card specific init
        if(m_TVCards[m_CardType].pInitCardFunction != NULL)
        {
            // call correct function
            // this funny syntax is the only one that works
            // if you want help understanding what is going on
            // I suggest you read http://www.newty.de/
            (*this.*m_TVCards[m_CardType].pInitCardFunction)();
        }
    }
}

eTVCardId CBT848Card::GetCardType()
{
    return m_CardType;
}

tstring CBT848Card::GetCardName(eTVCardId CardId)
{
    return m_TVCards[CardId].szName;
}


void CBT848Card::SetAnalogContrastBrightness(WORD Contrast, WORD Brightness)
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
    WriteByte(BT848_BRIGHT, (BYTE)Brightness);
}

void CBT848Card::SetWhiteCrushUp(BYTE WhiteCrushUp)
{
    WriteByte(BT848_WC_UP, WhiteCrushUp);
}

void CBT848Card::SetWhiteCrushDown(BYTE WhiteCrushDown)
{
    WriteByte(BT848_WC_DOWN, WhiteCrushDown);
}

void CBT848Card::SetAnalogHue(BYTE Hue)
{
    WriteByte(BT848_HUE, Hue);
}

void CBT848Card::SetAnalogSaturationU(WORD SaturationU)
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


void CBT848Card::SetAnalogSaturationV(WORD SaturationV)
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


void CBT848Card::SetBDelay(BYTE BDelay)
{
    WriteByte(BT848_BDELAY, BDelay);
}

tstring CBT848Card::GetChipType()
{
    switch (m_DeviceId)
    {
    case 0x0350:
        return _T("Bt848");
    case 0x0351:
        return _T("Bt849");
    case 0x036E:
        return _T("Bt878");
    case 0x036F:
        return _T("Bt878a");
    }
    return _T("n/a");
}

tstring CBT848Card::GetTunerType()
{
    return m_TunerType;
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

void CBT848Card::SetContrastBrightness(WORD Contrast, WORD Brightness)
{
    (*this.*m_TVCards[m_CardType].pSetContrastBrightness)(Contrast, Brightness);
}

void CBT848Card::SetHue(BYTE Hue)
{
    (*this.*m_TVCards[m_CardType].pSetHue)(Hue);
}

void CBT848Card::SetSaturationU(WORD SaturationU)
{
    (*this.*m_TVCards[m_CardType].pSetSaturationU)(SaturationU);
}

void CBT848Card::SetSaturationV(WORD SaturationV)
{
    (*this.*m_TVCards[m_CardType].pSetSaturationV)(SaturationV);
}

void CBT848Card::StandardSetFormat(int nInput, eVideoFormat Format)
{
    // do nothing
}


//-------------------------------
void CBT848Card::SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelayAdj, int HDelayAdj)
{
    int VertScale;
    int HorzScale;
    int HorzDelay;
    int VertDelay;
    int HorzActive;
    int VertActive;
    BYTE Crop;

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

        // Load up the right table which is depandant on the number of lines
        if (GetTVFormat(TVFormat)->wCropHeight == 576)
        {
            CurrentY = 576;
            // Load up the TG table for CCIR656
            for (i=0;i<SRAMTable_PAL[0];++i)
            {
                WriteByte(BT848_TBLG, SRAMTable_PAL[i+1]);
            }
        }
        else
        {
            CurrentY = 480;
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

        // Since we are digital here we don't really care which
        // format we choose as long as it has the right number of lines
        if (GetTVFormat(TVFormat)->wCropHeight == 576)
        {
            // Enable PAL Mode (or SECAM)
            MaskDataByte(BT848_IFORM, (BT848_IFORM_PAL_BDGHI | BT848_IFORM_XTBOTH), (BT848_IFORM_NORM | BT848_IFORM_XTBOTH));
        }
        else
        {
            // Enable NTSC Mode (or PAL60)
            MaskDataByte(BT848_IFORM, (BT848_IFORM_NTSC | BT848_IFORM_XTBOTH), (BT848_IFORM_NORM | BT848_IFORM_XTBOTH));
        }

        // Standard line Count
        WriteByte(BT848_VTOTAL_LO, 0x00);
        WriteByte(BT848_VTOTAL_HI, 0x00);

        // Setup parameters for overlay scale and Crop calculation
        HorzActive = CurrentX;
        VertActive = CurrentY;
        HorzScale = 0;
        HorzDelay = 0x80;
        VertScale = 0;

        VertDelay = GetTVFormat(TVFormat)->wVDelay + VDelayAdj;

        HorzDelay = 0x80 + HDelayAdj;

        WriteByte(BT848_E_VTC, BT848_VTC_HSFMT_32);
        WriteByte(BT848_O_VTC, BT848_VTC_HSFMT_32);
    }
    else
    {
        // set the pll on the card if appropriate
        if(GetTVFormat(TVFormat)->NeedsPLL == TRUE && GetCardSetup()->PLLFreq != PLL_NONE)
        {
            SetPLL(GetCardSetup()->PLLFreq);
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
        HorzActive = CurrentX & ~2;
        if(CurrentX <= GetTVFormat(TVFormat)->wHActivex1)
        {
            HorzScale = ((GetTVFormat(TVFormat)->wHActivex1 - CurrentX) * 4096UL) / CurrentX;
        }
        else
        {
            CurrentX = GetTVFormat(TVFormat)->wHActivex1;
            HorzScale = 0;
        }

        VertDelay = GetTVFormat(TVFormat)->wVDelay + VDelayAdj;

        HorzDelay = (((CurrentX * GetTVFormat(TVFormat)->wHDelayx1) / GetTVFormat(TVFormat)->wHActivex1) & 0x3fe) + HDelayAdj;

        if(TVFormat == VIDEOFORMAT_PAL_60)
        {
            WriteByte(BT848_VTOTAL_LO, (BYTE)(525 & 0xff));
            WriteByte(BT848_VTOTAL_HI, (BYTE)(525 >> 8));
        }
        else if(TVFormat == VIDEOFORMAT_NTSC_50)
        {
            WriteByte(BT848_VTOTAL_LO, (BYTE)(625 & 0xff));
            WriteByte(BT848_VTOTAL_HI, (BYTE)(625 >> 8));
        }

        DWORD sr = (GetTVFormat(TVFormat)->wCropHeight * 512) / CurrentY - 512;
        VertScale = (WORD) (0x10000UL - sr) & 0x1fff;
        VertActive = GetTVFormat(TVFormat)->wCropHeight;
    }

    // YUV 4:2:2 linear pixel format
    WriteByte(BT848_COLOR_FMT, BT848_COLOR_FMT_YUY2);

    Crop = ((HorzActive >> 8) & 0x03) | ((HorzDelay >> 6) & 0x0c) | ((VertActive >> 4) & 0x30) | ((VertDelay >> 2) & 0xc0);
    SetGeometryEvenOdd(FALSE, HorzScale, VertScale, HorzActive, VertActive, HorzDelay, VertDelay, Crop);
    SetGeometryEvenOdd(TRUE, HorzScale, VertScale, HorzActive, VertActive, HorzDelay, VertDelay, Crop);

    // call any card specific format setup
    (*this.*m_TVCards[m_CardType].pSetFormat)(nInput, TVFormat);
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
        ::Sleep(100);
    }

    // Set the TGCKI bits to use PLL rather than xtal
    MaskDataByte(BT848_TGCTRL, BT848_TGCTRL_TGCKI_PLL, 0x18);

    WriteByte(BT848_VTOTAL_LO, 0x00);
    WriteByte(BT848_VTOTAL_HI, 0x00);
    WriteByte(BT848_DVSIF, 0x00);
}

BOOL CBT848Card::IsCCIRSource(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
}

BOOL CBT848Card::IsSPISource(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_SPI);
}

BOOL CBT848Card::IsVideoPresent()
{
    if (IsSPISource(m_CurrentInput))
    {
        DWORD _tstat = ReadDword(BT848_INT_STAT);
        WriteDword(BT848_INT_STAT, BT848_INT_HSYNC);
        return (_tstat & BT848_INT_HSYNC);
    }
    else
    {
        return ((ReadByte(BT848_DSTATUS) & (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) == (BT848_DSTATUS_PRES | BT848_DSTATUS_HLOC)) ? TRUE : FALSE;
    }
}


void CBT848Card::SetRISCStartAddress(DWORD RiscBasePhysical)
{
    WriteDword(BT848_RISC_STRT_ADD, RiscBasePhysical);
}

DWORD CBT848Card::GetRISCPos()
{
    return ReadDword(BT848_RISC_COUNT);
}


void CBT848Card::StopCapture()
{
    // stop the chip running DMA
    SetDMA(FALSE);

    // Turn off capture
    MaskDataByte(BT848_CAP_CTL, 0, 0x0f);

    // set the RISC addresses to be NULL
    // so that nobody tries to run our RISC code later
    SetRISCStartAddress(0x00000000);
}

void CBT848Card::StartCapture(BOOL bCaptureVBI)
{
    int nFlags = BT848_CAP_CTL_CAPTURE_EVEN | BT848_CAP_CTL_CAPTURE_ODD;
    if (IsSPISource(m_CurrentInput))
    {
        nFlags = BT848_CAP_CTL_CAPTURE_ODD;
    }
    else if (bCaptureVBI == TRUE)
    {
        nFlags |= BT848_CAP_CTL_CAPTURE_VBI_EVEN | BT848_CAP_CTL_CAPTURE_VBI_ODD;
    }
    MaskDataByte(BT848_CAP_CTL, (BYTE) nFlags, (BYTE) 0x0f);
}


tstring CBT848Card::GetInputName(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_TVCards[m_CardType].Inputs[nInput].szName;
    }
    return _T("Error");
}


BOOL APIENTRY CBT848Card::ChipSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CBT848Card* pThis = NULL;
    TCHAR szCardId[9] = _T("n/a     ");
    TCHAR szVendorId[9] = _T("n/a ");
    TCHAR szDeviceId[9] = _T("n/a ");
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Card*)lParam;
        SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, pThis->GetChipType().c_str());
        _sntprintf(szVendorId, 9, _T("%04X"), pThis->GetVendorId());
        SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);
        _sntprintf(szDeviceId, 9, _T("%04X"), pThis->GetDeviceId());
        SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);
        SetDlgItemText(hDlg, IDC_TUNER_TYPE, pThis->GetTunerType().c_str());
        SetDlgItemText(hDlg, IDC_AUDIO_DECODER_TYPE, _T("")); // FIXME pThis->GetAudioDecoderType());
        dwCardId = pThis->GetSubSystemId();
        if(dwCardId != 0 && dwCardId != 0xffffffff)
        {
            _sntprintf(szCardId, 9, _T("%8X"), dwCardId);
        }
        SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);
        return TRUE;
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

ULONG CBT848Card::GetTickCount()
// an overflow happens after 21 days uptime on a 10GHz machine
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);

    ticks = ticks * 1000 / frequency;
    return (ULONG)ticks;
}

void CBT848Card::InitializeI2C()
{
    WriteDword(BT848_I2C, 1);
    m_I2CRegister = ReadDword(BT848_I2C);

    ULONGLONG frequency;
    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);

    m_I2CSleepCycle = (unsigned long)(frequency / 50000);

    m_I2CInitialized = TRUE;
}

void CBT848Card::Sleep()
{
    ULONGLONG ticks = 0;
    ULONGLONG start;

    QueryPerformanceCounter((PLARGE_INTEGER)&start);

    while(start + m_I2CSleepCycle > ticks)
    {
        QueryPerformanceCounter((PLARGE_INTEGER)&ticks);
    }
}

void CBT848Card::SetSDA(BOOL value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        LOG(3, (m_I2CRegister & BT848_I2C_SDA) ? _T("BT848 SetSDA - d^") : _T("BT848 SetSDA - d/"));
        m_I2CRegister |= BT848_I2C_SDA;
    }
    else
    {
        LOG(3, (m_I2CRegister & BT848_I2C_SDA) ? _T("BT848 SetSDA - d\\") : _T("BT848 SetSDA - d_"));
        m_I2CRegister &= ~BT848_I2C_SDA;
    }
    WriteDword(BT848_I2C, m_I2CRegister);
}

void CBT848Card::SetSCL(BOOL value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        LOG(3, (m_I2CRegister & BT848_I2C_SCL) ? _T("BT848 SetSCL - c^") : _T("BT848 SetSCL - c/"));
        m_I2CRegister |= BT848_I2C_SCL;
    }
    else
    {
        LOG(3, (m_I2CRegister & BT848_I2C_SCL) ? _T("BT848 SetSCL - c\\") : _T("BT848 SetSCL - c_"));
        m_I2CRegister &= ~BT848_I2C_SCL;
    }
    WriteDword(BT848_I2C, m_I2CRegister);
}

BOOL CBT848Card::GetSDA()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    BOOL state = ReadDword(BT848_I2C) & BT848_I2C_SDA ? TRUE : FALSE;
    LOG(3, state ? _T("BT848 GetSDA - d^") : _T("BT848 GetSDA - d_"));
    return state;
}

BOOL CBT848Card::GetSCL()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    BOOL state = ReadDword(BT848_I2C) & BT848_I2C_SCL ? TRUE : FALSE;
    LOG(3, state ? _T("BT848 GetSCL - c^") : _T("BT848 GetSCL - c_"));
    return state;
}

void CBT848Card::I2CLock()
{
    LockCard();
}

void CBT848Card::I2CUnlock()
{
    UnlockCard();
}


BOOL CBT848Card::Is878Family()
{
    // returns TRUE if BT878 or BT878a is present
    return (m_DeviceId == 0x036E || m_DeviceId == 0x036F);
}

void CBT848Card::SetGPOE(ULONG val)
{
    WriteDword(BT848_GPIO_OUT_EN, val);
}

ULONG CBT848Card::GetGPOE()
{
    return ReadDword(BT848_GPIO_OUT_EN);
}

void CBT848Card::SetGPDATA(ULONG val)
{
    WriteDword(BT848_GPIO_DATA, val);
}

ULONG CBT848Card::GetGPDATA()
{
    return ReadDword(BT848_GPIO_DATA);

}

void CBT848Card::ResetChip()
{
    BYTE Command = 0;

    // try and switch on the card using the PCI Command value
    // this is to try and solve problems when a driver hasn't been
    // loaded for the card, which may be necessary when you have
    // multiple cards
    if(GetPCIConfigOffset(&Command, 0x04, m_BusNumber, m_SlotNumber))
    {
        // switch on allow master and respond to memory requests
        if((Command & 0x06) != 0x06)
        {
            LOG(1, _T(" CX2388x PCI Command was %d"), Command);
            Command |= 0x06;
            SetPCIConfigOffset(&Command, 0x04, m_BusNumber, m_SlotNumber);
            ::Sleep(500);
        }
    }

    WriteByte(BT848_SRESET, 0);
}

void CBT848Card::ShowRegisterSettingsDialog(HINSTANCE hBT8x8ResourceInst)
{
    DialogBoxParam(hBT8x8ResourceInst, _T("REGISTEREDIT"), GetMainWnd(), RegisterEditProc, (LPARAM)this);
}

#define AddRegister(Reg) {long Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_REGISTERSELECT), #Reg); ComboBox_SetItemData(GetDlgItem(hDlg, IDC_REGISTERSELECT), Index, Reg);}

BOOL APIENTRY CBT848Card::RegisterEditProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CBT848Card* pThis;
    static DWORD dwAddress;
    static BYTE RegValue;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CBT848Card*)lParam;
        SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_RESETCONTENT, 0, 0);
        AddRegister(BT848_DSTATUS);
        AddRegister(BT848_IFORM);
        AddRegister(BT848_FCNTR);
        AddRegister(BT848_PLL_F_LO);
        AddRegister(BT848_PLL_F_HI);
        AddRegister(BT848_PLL_XCI);
        AddRegister(BT848_TGCTRL);
        AddRegister(BT848_TDEC);
        AddRegister(BT848_E_CROP);
        AddRegister(BT848_O_CROP);
        AddRegister(BT848_E_VDELAY_LO);
        AddRegister(BT848_O_VDELAY_LO);
        AddRegister(BT848_E_VACTIVE_LO);
        AddRegister(BT848_O_VACTIVE_LO);
        AddRegister(BT848_E_HDELAY_LO);
        AddRegister(BT848_O_HDELAY_LO);
        AddRegister(BT848_E_HACTIVE_LO);
        AddRegister(BT848_O_HACTIVE_LO);
        AddRegister(BT848_E_HSCALE_HI);
        AddRegister(BT848_O_HSCALE_HI);
        AddRegister(BT848_E_HSCALE_LO);
        AddRegister(BT848_O_HSCALE_LO);
        AddRegister(BT848_BRIGHT);
        AddRegister(BT848_E_CONTROL);
        AddRegister(BT848_O_CONTROL);
        AddRegister(BT848_CONTRAST_LO);
        AddRegister(BT848_SAT_U_LO);
        AddRegister(BT848_SAT_V_LO);
        AddRegister(BT848_HUE);
        AddRegister(BT848_E_SCLOOP);
        AddRegister(BT848_O_SCLOOP);
        AddRegister(BT848_WC_UP);
        AddRegister(BT848_WC_DOWN);
        AddRegister(BT848_VTOTAL_LO);
        AddRegister(BT848_VTOTAL_HI);
        AddRegister(BT848_DVSIF);
        AddRegister(BT848_OFORM);
        AddRegister(BT848_E_VSCALE_HI);
        AddRegister(BT848_O_VSCALE_HI);
        AddRegister(BT848_E_VSCALE_LO);
        AddRegister(BT848_O_VSCALE_LO);
        AddRegister(BT848_TEST);
        AddRegister(BT848_ADELAY);
        AddRegister(BT848_BDELAY);
        AddRegister(BT848_ADC);
        AddRegister(BT848_E_VTC);
        AddRegister(BT848_O_VTC);
        AddRegister(BT848_COLOR_FMT);
        AddRegister(BT848_COLOR_CTL);
        AddRegister(BT848_CAP_CTL);
        AddRegister(BT848_VBI_PACK_SIZE);
        AddRegister(BT848_VBI_PACK_DEL);
        AddRegister(BT848_INT_STAT);
        AddRegister(BT848_INT_MASK);
        AddRegister(BT848_GPIO_DMA_CTL);
        AddRegister(BT848_I2C);
        AddRegister(BT848_RISC_STRT_ADD);
        AddRegister(BT848_GPIO_OUT_EN);
        AddRegister(BT848_GPIO_OUT_EN_HIBYTE);
        AddRegister(BT848_GPIO_REG_INP);
        AddRegister(BT848_GPIO_REG_INP_HIBYTE);
        AddRegister(BT848_RISC_COUNT);
        AddRegister(BT848_GPIO_DATA);
        AddRegister(BT848_GPIO_DATA_HIBYTE);
        AddRegister(BT848_TBLG);
        if(pThis->m_SAA7118.IsValid())
        {
            for(int i(0); i < 0xFF; ++i)
            {
                TCHAR RegName[20];
                _stprintf(RegName, _T("SAA7118_%02x"),i);
                long Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_REGISTERSELECT), RegName);
                ComboBox_SetItemData(GetDlgItem(hDlg, IDC_REGISTERSELECT), Index, i + 0x1000);
            }
        }
        return TRUE;
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, TRUE);
            break;

        case IDC_BIT0:
        case IDC_BIT1:
        case IDC_BIT2:
        case IDC_BIT3:
        case IDC_BIT4:
        case IDC_BIT5:
        case IDC_BIT6:
        case IDC_BIT7:
            if(Button_GetCheck(GetDlgItem(hDlg, LOWORD(wParam))) == BST_CHECKED)
            {
                RegValue |= 1 << (LOWORD(wParam) - IDC_BIT0);
            }
            else
            {
                RegValue &= ~(1 << (LOWORD(wParam) - IDC_BIT0));
            }
            if(dwAddress < 0x1000)
            {
                pThis->WriteDword(dwAddress, RegValue);
            }
            else
            {
                if(pThis->m_SAA7118.IsValid())
                {
                    pThis->m_SAA7118->SetRegister((BYTE)(dwAddress - 0x1000), RegValue);
                }
            }
            break;

        case IDC_REGISTERSELECT:
            if(HIWORD(wParam) == LBN_SELCHANGE)
            {
                long Index = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_REGISTERSELECT));
                if(Index != -1)
                {
                    dwAddress = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_REGISTERSELECT), Index);
                    if(dwAddress < 0x1000)
                    {
                        RegValue = pThis->ReadByte(dwAddress);
                    }
                    else
                    {
                        if(pThis->m_SAA7118.IsValid())
                        {
                            RegValue = pThis->m_SAA7118->GetRegister((BYTE)(dwAddress - 0x1000));
                        }
                    }
                    BYTE TempRegValue(RegValue);
                    for(int i(0); i < 8; ++i)
                    {
                        if(TempRegValue & 1)
                        {
                            Button_SetCheck(GetDlgItem(hDlg, IDC_BIT0 + i), BST_CHECKED);
                        }
                        else
                        {
                            Button_SetCheck(GetDlgItem(hDlg, IDC_BIT0 + i), BST_UNCHECKED);
                        }
                        TempRegValue = TempRegValue >> 1;
                    }
                }
            }
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
    return (FALSE);
}

#endif // WANT_BT8X8_SUPPORT