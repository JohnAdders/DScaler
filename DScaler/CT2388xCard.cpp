/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xCard.cpp,v 1.19 2002-10-25 14:46:24 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
// Revision 1.18  2002/10/24 17:17:17  adcockj
// Fixed silly bug in new buffer code
//
// Revision 1.17  2002/10/24 16:04:47  adcockj
// Another attempt to get VBI working
// Tidy up CMDS/Buffers code
//
// Revision 1.16  2002/10/23 20:26:52  adcockj
// Bug fixes for cx2388x
//
// Revision 1.15  2002/10/23 16:10:50  adcockj
// Fixed some filter setting bugs and added SECAM tests for Laurent
//
// Revision 1.14  2002/10/23 15:18:07  adcockj
// Added preliminary code for VBI
//
// Revision 1.13  2002/10/22 18:52:18  adcockj
// Added ASPI support
//
// Revision 1.12  2002/10/22 11:39:50  adcockj
// Changes to test 8xFsc mode
//
// Revision 1.11  2002/10/21 07:19:33  adcockj
// Preliminary Support for PixelView XCapture
//
// Revision 1.10  2002/10/18 16:12:31  adcockj
// Tidy up and fixes for Cx2388x analogue support
//
// Revision 1.9  2002/10/08 11:22:40  adcockj
// Changed some defines for consistency
//
// Revision 1.8  2002/09/29 16:16:21  adcockj
// Holo3d imrprovements
//
// Revision 1.7  2002/09/19 22:10:08  adcockj
// Holo3D Fixes for PAL
//
// Revision 1.6  2002/09/16 20:08:21  adcockj
// fixed format detect for cx2388x
//
// Revision 1.5  2002/09/16 19:34:18  adcockj
// Fix for auto format change
//
// Revision 1.4  2002/09/15 14:20:38  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.3  2002/09/12 21:45:35  ittarnavsky
// changed the IAudioControls to CAudioControls
//
// Revision 1.2  2002/09/11 19:33:06  adcockj
// a few tidy ups
//
// Revision 1.1  2002/09/11 18:19:36  adcockj
// Prelimainary support for CT2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CT2388xCard.h"
#include "CT2388x_Defines.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"
#include "NoTuner.h"
#include "MT2032.h"
#include "GenericTuner.h"


CCT2388xCard::CCT2388xCard(CHardwareDriver* pDriver) :
	CPCICard(pDriver),
    m_CardType(CT2388xCARD_CONEXANT_EVK),
    m_Tuner(NULL),
	m_SAA7118(NULL),
    m_RISCIsRunning(FALSE),
    m_CurrentInput(0)
{
    strcpy(m_TunerType,"n/a");

    m_I2CInitialized = false;
    m_I2CBus = new CI2CBusForLineInterface(this);
    m_AudioControls = new CAudioControls();
    m_AudioDecoder = new CAudioDecoder();
}

CCT2388xCard::~CCT2388xCard()
{
	delete m_I2CBus;
	delete m_AudioDecoder;
	delete m_AudioControls;
	delete m_Tuner;
	delete m_SAA7118;

    ClosePCICard();
}

// this functions returns 0 if the BT878 is in ACPI state D0 or on error/BT848
// returns 3 if in D3 state (full off)
int CCT2388xCard::GetACPIStatus()
{
    PCI_COMMON_CONFIG PCI_Config;

    if(GetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber))
    {
        DWORD ACPIStatus = PCI_Config.DeviceSpecific[0x10] & 3;

        LOG(1, "CX2388x ACPI status: D%d", ACPIStatus);
        return ACPIStatus;
    }

    return 0;
}

// Set ACPIStatus to 0 for D0/full on state. 3 for D3/full off
void CCT2388xCard::SetACPIStatus(int ACPIStatus)
{
    PCI_COMMON_CONFIG PCI_Config;

    if(!GetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber))
    {
        return;
    }
    PCI_Config.DeviceSpecific[0x10] &= ~3;
    PCI_Config.DeviceSpecific[0x10] |= ACPIStatus;

    LOG(1, "Attempting to set CX2388x ACPI status to D%d", ACPIStatus);

    SetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber);

    if(ACPIStatus == 0)
    {
        ::Sleep(500);
        // reset the chip
	    WriteDword( 0x310304, 0x1 );
    }
    LOG(1, "Set CX2388x ACPI status complete");
}


void CCT2388xCard::CloseCard()
{
    ClosePCICard();
}

void CCT2388xCard::StartCapture(BOOL bCaptureVBI)
{
    DWORD value1;
    DWORD value2;
   
    // Clear Interrupt Status bits
    WriteDword(CT2388X_VID_INTSTAT, 0x0000000);
    
    value1 = ReadDword(CT2388X_VID_DMA_CNTRL) & 0xFFFFFF00;
    value2 = (ReadDword(CT2388X_CAPTURECONTROL) & 0xFFFFFF00);
    if(bCaptureVBI == TRUE)
    {
        value1 |= 0x99;
        value2 |= 0x1E;
    }
    else
    {
        value1 |= 0x11;
        value2 |= 0x06;
    }

    WriteDword(CT2388X_VID_DMA_CNTRL, value1);
    WriteDword(CT2388X_CAPTURECONTROL, value2);

    // Clear Interrupt Status bits
    WriteDword(CT2388X_VID_INTSTAT, 0xFFFFFFFF );

    m_RISCIsRunning = TRUE;
}

/*******************************************************************************
    Halt the Odd/Both Risc Program.
    both_fields == true - Halt both fields.
                == false - Halt odd field only.
*******************************************************************************/
void CCT2388xCard::StopCapture()
{
    DWORD value1;
    DWORD value2;
    
    value1 = ReadDword(CT2388X_VID_DMA_CNTRL) & 0xFFFFFF00;
    value2 = ReadDword(CT2388X_CAPTURECONTROL) & 0xFFFFFF00;
   
    ::Sleep(100);

    // Original code before restart workaround
    WriteDword(CT2388X_VID_DMA_CNTRL, value1);
    WriteDword(CT2388X_CAPTURECONTROL, value2);

    m_RISCIsRunning = FALSE;
}

void CCT2388xCard::SetCardType(int CardType)
{
    if(m_CardType != CardType)
    {
        m_CardType = (eCT2388xCardId)CardType;

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

eCT2388xCardId CCT2388xCard::GetCardType()
{
    return m_CardType;
}

LPCSTR CCT2388xCard::GetCardName(eCT2388xCardId CardId)
{
    return m_TVCards[CardId].szName;
}

void CCT2388xCard::SetBrightness(BYTE Brightness)
{
    (*this.*m_TVCards[m_CardType].pSetBrightness)(Brightness);
}

void CCT2388xCard::SetHue(BYTE Hue)
{
    (*this.*m_TVCards[m_CardType].pSetHue)(Hue);
}

void CCT2388xCard::SetContrast(BYTE Contrast)
{
    (*this.*m_TVCards[m_CardType].pSetContrast)(Contrast);
}

void CCT2388xCard::SetSaturationU(BYTE SaturationU)
{
    (*this.*m_TVCards[m_CardType].pSetSaturationU)(SaturationU);
}

void CCT2388xCard::SetSaturationV(BYTE SaturationV)
{
    (*this.*m_TVCards[m_CardType].pSetSaturationV)(SaturationV);
}

void CCT2388xCard::StandardSetFormat(int nInput, eVideoFormat Format, BOOL IsProgressive)
{
    // do nothing
}

void CCT2388xCard::SetAnalogBrightness(BYTE Brightness)
{
    DWORD dwval = ReadDword(CT2388X_BRIGHT_CONTRAST);
    dwval &= 0xffffff00;
    Brightness = (BYTE)(unsigned char)(Brightness - 0x80);
    dwval |= (Brightness & 0xff);
    WriteDword(CT2388X_BRIGHT_CONTRAST,dwval);
}

void CCT2388xCard::SetAnalogHue(BYTE Hue)
{
    DWORD dwval = ReadDword(CT2388X_HUE);
    dwval &= 0xffffff00;
    Hue = (BYTE)(unsigned char)(Hue - 0x80);
    dwval |= (Hue & 0xff);
    WriteDword(CT2388X_HUE, dwval);
}

void CCT2388xCard::SetAnalogContrast(BYTE Contrast)
{
    DWORD dwval = ReadDword(CT2388X_BRIGHT_CONTRAST); // Brightness/contrast register
    dwval &= 0xffff00ff;
    dwval |= ((Contrast & 0xFF) << 8 );
    WriteDword(CT2388X_BRIGHT_CONTRAST,dwval);
}

void CCT2388xCard::SetAnalogSaturationU(BYTE SaturationU)
{
    DWORD dwval = ReadDword(CT2388X_UVSATURATION);
    dwval &= 0xffffff00;
    dwval |= (SaturationU & 0xff);
    WriteDword(CT2388X_UVSATURATION,dwval);
}

void CCT2388xCard::SetAnalogSaturationV(BYTE SaturationV)
{
    DWORD dwval = ReadDword(CT2388X_UVSATURATION);
    dwval &= 0xffff00ff;
    dwval |= ((SaturationV & 0xFF) << 8 );
    WriteDword(CT2388X_UVSATURATION,dwval);
}


void CCT2388xCard::SetVIPBrightness(BYTE Brightness)
{
    DWORD dwval = ReadDword(CT2388X_VIP_CONTBRGT);
    dwval &= 0xffffff00;
    Brightness = (BYTE)(unsigned char)(Brightness - 0x80);
    dwval |= (Brightness & 0xFF);
    WriteDword(CT2388X_VIP_CONTBRGT, dwval);
}

void CCT2388xCard::SetVIPContrast(BYTE Contrast)
{
    DWORD dwval = ReadDword(CT2388X_VIP_CONTBRGT);
    dwval &= 0xffff00ff;
    dwval |= ((Contrast & 0xFF) << 8 );
    WriteDword(CT2388X_VIP_CONTBRGT, dwval);
}

void CCT2388xCard::SetVIPSaturation(BYTE Saturation)
{
    DWORD dwval = ReadDword(CT2388X_VIP_SATURATION);
    dwval &= 0xffffff00;
    dwval |= (Saturation & 0xFF);
    WriteDword(CT2388X_VIP_SATURATION, dwval);
}


LPCSTR CCT2388xCard::GetTunerType()
{
    return m_TunerType;
}

// Sets up card to support size and format requested
// at the moment we insist on 720 pixel width.
void CCT2388xCard::SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, int VDelayOverride, int HDelayOverride, BOOL IsProgressive)
{
    int HorzDelay;
    int VertDelay;
    int HorzScale;

    CurrentY = GetTVFormat(TVFormat)->wCropHeight;

    if(IsCCIRSource(nInput))
    {
        CurrentX = 720;
        CurrentVBILines = 0;

		WriteByte(CT2388X_PINMUX_IO, 0x02);

        // Since we are digital here we don't really care which
        // format we choose as long as it has the right number of lines
		DWORD VideoInput = ReadDword(CT2388X_VIDEO_INPUT);
		VideoInput &= 0xfffffff0;

        if (CurrentY == 576)
        {
            // Enable PAL Mode (or SECAM)
            VideoInput |= VideoFormatPALBDGHI;
        }
        else
        {
            // Enable NTSC Mode (or PAL60)
            VideoInput |= VideoFormatNTSC;
        }

        WriteDword(CT2388X_VIDEO_INPUT, VideoInput);

        if(VDelayOverride != 0)
        {
            VertDelay = VDelayOverride;
        }
        else
        {
            VertDelay = 0x0C;
        }

        WriteDword(CT2388X_FORMAT_2HCOMB, 0x183f0008);
        WriteDword(CT2388X_VIP_CONFIG, 1);
        WriteDword(CT2388X_VDELAYCCIR_EVEN, VertDelay);
        WriteDword(CT2388X_VDELAYCCIR_ODD, VertDelay);
        WriteDword(CT2388X_VERT_DELAY_EVEN, VertDelay);
        WriteDword(CT2388X_VERT_DELAY_ODD, VertDelay);

        if(HDelayOverride != 0)
        {
            HorzDelay = HDelayOverride;
        }
        else
        {
            if (CurrentY == 576)
            {
                HorzDelay = 0x94;
            }
            else
            {
                HorzDelay = 0x7E;
            }
        }

        if (CurrentY == 576)
        {
            HorzScale = 0x0504;
        }
        else
        {
            HorzScale = 0x00F8;
        }
    }
    // Test to see how to get best analogue picture
    // try using the sample rate convertor
    else if(true)
    {
        DWORD HTotal(0);
        DWORD Format2HComb;

		if(m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_SVIDEO)
		{
			// set up with
			// Previous line remodulation - off
			// 3-d Comb filter - off
			// Comb Range - 00
			// Full Luma Range - on
			// PAL Invert Phase - off
			// Coring - off
			Format2HComb = 0x08;
		}
		else
		{
			// set up with
			// Previous line remodulation - on
			// 3-d Comb filter - on
			// Comb Range - 1f
			// Full Luma Range - on
			// PAL Invert Phase - off
			// Coring - off
			Format2HComb = 0x181f0008;
		}

        CurrentVBILines = GetTVFormat(TVFormat)->VBILines;

        // set up VBI information
        WriteDword(CT2388X_VBI_SIZE, (GetTVFormat(TVFormat)->VBIPacketSize) | (2 << 11));

        if (CurrentY == 576)
        {
	        CurrentX = 702;
        }
        else
        {
	        CurrentX = 714;
        }

        SetPLL(27.0);
        SetSampleRateConverter(27.0);

        // Setup correct format
		DWORD VideoInput = ReadDword(CT2388X_VIDEO_INPUT);
		VideoInput &= 0xfffffff0;

        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
            VideoInput |= VideoFormatPALBDGHI;
            HTotal = HLNotchFilter135PAL | 864;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_N:
            VideoInput |= VideoFormatPALN;
            HTotal = HLNotchFilter135PAL | 864;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_M:
            VideoInput |= VideoFormatPALM;
            HTotal = HLNotchFilter135NTSC | 858;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_60:
            VideoInput |= VideoFormatPAL60;
            HTotal = HLNotchFilter135NTSC | 858;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            VideoInput |= VideoFormatPALNC;
            HTotal = HLNotchFilter135PAL | 864;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_SECAM_B:
        case VIDEOFORMAT_SECAM_D:
        case VIDEOFORMAT_SECAM_G:
        case VIDEOFORMAT_SECAM_H:
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
        case VIDEOFORMAT_SECAM_L:
        case VIDEOFORMAT_SECAM_L1:
            VideoInput |= VideoFormatSECAM;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        case VIDEOFORMAT_NTSC_M:
            VideoInput |= VideoFormatNTSC;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            VideoInput |= VideoFormatNTSCJapan;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        case VIDEOFORMAT_NTSC_50:
            VideoInput |= VideoFormatNTSC443;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        default:
            VideoInput |= VideoFormatAuto;
            HTotal = HLNotchFilter135NTSC | 858;
            break;
        }

        WriteDword(CT2388X_VIDEO_INPUT, VideoInput);
        WriteDword(CT2388X_PIXEL_CNT_NOTCH, HTotal);
        WriteDword(CT2388X_FORMAT_2HCOMB, Format2HComb);

        // set up subcarrier frequency
        DWORD RegValue = (DWORD)(((8.0 * GetTVFormat(TVFormat)->Fsc) / 27.0) * (double)(1<<22));
        WriteDword( CT2388X_SUBCARRIERSTEP, RegValue & 0x7FFFFF );
        // Subcarrier frequency Dr, for SECAM only but lets
        // set it anyway
        RegValue = (DWORD)((8.0 * 4.406250 / 27.0) * (double)(1<<22));
        WriteDword( CT2388X_SUBCARRIERSTEPDR, RegValue);

        if(VDelayOverride != 0)
        {
            VertDelay = VDelayOverride;
        }
        else
        {
            if (CurrentY == 576)
            {
                VertDelay = 0x20;
            }
            else
            {
                VertDelay = 0x1C;
            }
        }

        WriteDword(CT2388X_VERT_DELAY_EVEN, VertDelay);
        WriteDword(CT2388X_VERT_DELAY_ODD, VertDelay);

        if(HDelayOverride != 0)
        {
            HorzDelay = HDelayOverride;
        }
        else
        {
            if (CurrentY == 576)
            {
                HorzDelay = 0x8A;
            }
            else
            {
                HorzDelay = 0x7E;
            }
        }

        HorzScale = 0x00;
    }
    // compare to method using old style 8*Fsc capture
    else
    {
        DWORD HTotal(0);
        DWORD Format2HComb(0x183f0008);

        CurrentVBILines = GetTVFormat(TVFormat)->VBILines;

        // set up VBI information
        // need packet size and delay
        // hopefully this is the same info that the bt8x8 chips needed
        WriteDword(CT2388X_VBI_SIZE, (GetTVFormat(TVFormat)->VBIPacketSize & 0xff) | ((GetTVFormat(TVFormat)->VBIPacketSize >> 8) << 11));

        CurrentX = 720;

        if (CurrentY == 576)
        {
			DWORD RegValue;
            HorzScale = 0x0504;
            double PALFsc8(GetTVFormat(VIDEOFORMAT_PAL_B)->Fsc * 8);

            SetPLL(PALFsc8);

			// set up subcarrier frequency
			// Comments from Laurent
			// The sub carriers frequencies are wrong for SECAM
			// => use the values given by AMCAP
			if(TVFormat == VIDEOFORMAT_SECAM_L)
				RegValue = 0x003d5985;
			else
				RegValue = (DWORD)(((8.0 * GetTVFormat(TVFormat)->Fsc) / PALFsc8) * (double)(1<<22));
			WriteDword( CT2388X_SUBCARRIERSTEP, RegValue & 0x7FFFFF );
			// Subcarrier frequency Dr, for SECAM only but lets
			// set it anyway
			if(TVFormat == VIDEOFORMAT_SECAM_L)
				RegValue = 0x003f9aee;
			else
				RegValue = (DWORD)((8.0 * 4.406250 / PALFsc8) * (double)(1<<22));
			WriteDword( CT2388X_SUBCARRIERSTEPDR, RegValue);

            WriteDword( CT2388X_SAMPLERATECONV, 0x19D5F);
        }
        else
        {
            SetPLL(28.636363);
            HorzScale = 0x00F8;

			// set up subcarrier frequency
			DWORD RegValue = (DWORD)(((8.0 * GetTVFormat(TVFormat)->Fsc) / 28.636363) * (double)(1<<22));
			WriteDword( CT2388X_SUBCARRIERSTEP, RegValue & 0x7FFFFF );
			// Subcarrier frequency Dr, for SECAM only but lets
			// set it anyway
			RegValue = (DWORD)((8.0 * 4.406250 / 28.636363) * (double)(1<<22));
			WriteDword( CT2388X_SUBCARRIERSTEPDR, RegValue);

            WriteDword( CT2388X_SAMPLERATECONV, 0x20000);
        }



        // Setup correct format
		DWORD VideoInput = ReadDword(CT2388X_VIDEO_INPUT);
		VideoInput &= 0xfffffff0;

        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
            VideoInput |= VideoFormatPALBDGHI;
            HTotal = HLNotchFilter4xFsc | 864;
			// Comments from Laurent
			// HTotal seems to be wrong for PAL
			// => use the same value as for SECAM because it seems to work
			// TODO: use the correct value for each video format
			HTotal = 0x0000046f;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_N:
            VideoInput |= VideoFormatPALN;
            HTotal = HLNotchFilter4xFsc | 864;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_M:
            VideoInput |= VideoFormatPALM;
            HTotal = HLNotchFilter4xFsc | 858;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_60:
            VideoInput |= VideoFormatPAL60;
            HTotal = HLNotchFilter4xFsc | 858;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            VideoInput |= VideoFormatPALNC;
            HTotal = HLNotchFilter4xFsc | 864;
            Format2HComb |= (1 << 26);
            break;
        case VIDEOFORMAT_SECAM_B:
        case VIDEOFORMAT_SECAM_D:
        case VIDEOFORMAT_SECAM_G:
        case VIDEOFORMAT_SECAM_H:
        case VIDEOFORMAT_SECAM_K:
        case VIDEOFORMAT_SECAM_K1:
        case VIDEOFORMAT_SECAM_L:
        case VIDEOFORMAT_SECAM_L1:
            VideoInput |= VideoFormatSECAM;
            HTotal = HLNotchFilter4xFsc | 864;
			// Comments from Laurent
			// HTotal is wrong for SECAM
			// => use the value given by AMCAP
			HTotal = 0x0000046f;
            break;
        case VIDEOFORMAT_NTSC_M:
            VideoInput |= VideoFormatNTSC;
            HTotal = HLNotchFilter4xFsc | 858;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            VideoInput |= VideoFormatNTSCJapan;
            HTotal = HLNotchFilter4xFsc | 858;
            break;
        case VIDEOFORMAT_NTSC_50:
            VideoInput |= VideoFormatNTSC443;
            HTotal = HLNotchFilter4xFsc | 864;
            break;
        default:
            VideoInput |= VideoFormatAuto;
            HTotal = HLNotchFilter4xFsc | 858;
            break;
        }

        WriteDword(CT2388X_VIDEO_INPUT, VideoInput);
        WriteDword(CT2388X_PIXEL_CNT_NOTCH, HTotal);
        WriteDword(CT2388X_FORMAT_2HCOMB, Format2HComb);

        if(VDelayOverride != 0)
        {
            VertDelay = VDelayOverride;
        }
        else
        {
            if (CurrentY == 576)
            {
                VertDelay = 0x20;
            }
            else
            {
                VertDelay = 0x1C;
            }
        }

        WriteDword(CT2388X_VERT_DELAY_EVEN, VertDelay);
        WriteDword(CT2388X_VERT_DELAY_ODD, VertDelay);

        if(HDelayOverride != 0)
        {
            HorzDelay = HDelayOverride;
        }
        else
        {
            if (CurrentY == 576)
            {
                HorzDelay = 0x8A;
            }
            else
            {
                HorzDelay = 0x7E;
            }
        }
    }

    WriteDword(CT2388X_HACTIVE_EVEN, CurrentX);
    WriteDword(CT2388X_HACTIVE_ODD, CurrentX);

    WriteDword(CT2388X_VACTIVE_EVEN, CurrentY);
    WriteDword(CT2388X_VACTIVE_ODD, CurrentY);

    WriteDword(CT2388X_HORZ_DELAY_EVEN, HorzDelay);
    WriteDword(CT2388X_HORZ_DELAY_ODD, HorzDelay);

    WriteDword(CT2388X_HSCALE_EVEN, HorzScale);
    WriteDword(CT2388X_HSCALE_ODD, HorzScale);

    WriteDword(CT2388X_VSCALE_EVEN, 0);
    WriteDword(CT2388X_VSCALE_ODD, 0);

    // call any card specific format setup
    (*this.*m_TVCards[m_CardType].pSetFormat)(nInput, TVFormat, IsProgressive);
}


BOOL CCT2388xCard::IsCCIRSource(int nInput)
{
    return (m_TVCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
}

BOOL CCT2388xCard::IsVideoPresent()
{
    DWORD dwval = ReadDword(CT2388X_DEVICE_STATUS);
    return ((dwval & CT2388X_DEVICE_STATUS_HLOCK) == CT2388X_DEVICE_STATUS_HLOCK);
}

DWORD CCT2388xCard::GetRISCPos()
{
    return ReadDword(CT2388X_VIDY_GP_CNT);
}

void CCT2388xCard::ResetHardware()
{
    PCI_COMMON_CONFIG PCI_Config;

    // try and switch on the card using the PCI Command value
    // this is to try and solve problems when a driver hasn't been
    // loaded for the card, which may be necessary when you have 
    // multiple cx2388x cards
    if(GetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber))
    {
        // switch on allow master and respond to memory requests
        if((PCI_Config.Command & 0x06) != 0x06)
        {
            LOG(1, " CX2388x PCI Command was %d", PCI_Config.Command);
            PCI_Config.Command |= 0x06;
            SetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber);
        }
    }

    // \todo log what's there already for key settings

    // Clear out the SRAM Channel Management data structures
    // for all 12 devices
    for (int i(1); i<=12; ++i)
    {   
        DWORD dwaddr = 0x180000+i*0x40;
        for (int j(0); j<5; ++j)
        {
            WriteDword(dwaddr+(j*4),0);
        }
    }

    // Reset the chip
    WriteDword( 0x310304, 0x1 );

    ::Sleep(500);

    /* RISC Controller Enable */
    WriteDword(CT2338X_DEV_CNTRL2, 1<<5 );

    ::Sleep(500);

    /////////////////////////////////////////////////////////////////
    // Setup SRAM tables
    /////////////////////////////////////////////////////////////////

    // first check that everything we want to fit in SRAM
    // actually does fit, I'd hope this gets picked up in debug
    if(SRAM_NEXT > SRAM_MAX)
    {
        ErrorBox("Too much to fit in SRAM")
    }

    /////////////////////////////////////////////////////////////////
    // Setup for video channel 21
    /////////////////////////////////////////////////////////////////
        
    // Instruction Queue Base
    WriteDword(SRAM_CMDS_21 + 0x0c, SRAM_INSTRUCTION_QUEUE_VIDEO);
    
    // Instruction Queue Size is in DWORDs
    WriteDword(SRAM_CMDS_21 + 0x10, (SRAM_INSTRUCTION_QUEUE_SIZE / 4));
    
    // Cluster table base 
    WriteDword(SRAM_CMDS_21 + 0x04, SRAM_CLUSTER_TABLE_VIDEO); 

    // Cluster table size is in QWORDS
    WriteDword(SRAM_CMDS_21 + 0x08, SRAM_CLUSTER_TABLE_VIDEO_SIZE / 8);

    // Fill in cluster buffer entries
    for(i = 0; i < SRAM_VIDEO_BUFFERS; ++i)
    {
        WriteDword(
                    SRAM_CLUSTER_TABLE_VIDEO + (i * 0x10), 
                    SRAM_FIFO_VIDEO_BUFFERS + (i * SRAM_FIFO_VIDEO_BUFFER_SIZE)
                  );
    }
    
    // Copy the cluster buffer info to the DMAC 
    
    // Set the DMA Cluster Table Address
    WriteDword( MO_DMA21_PTR2, SRAM_CLUSTER_TABLE_VIDEO);
    
    // Set the DMA buffer limit size in qwords
    WriteDword( MO_DMA21_CNT1, SRAM_FIFO_VIDEO_BUFFER_SIZE / 8);
    
    // Set the DMA Cluster Table Size in qwords
    WriteDword( MO_DMA21_CNT2, SRAM_CLUSTER_TABLE_VIDEO_SIZE / 8);

    /////////////////////////////////////////////////////////////////
    // Setup for VBI channel 24
    /////////////////////////////////////////////////////////////////
        
    // Instruction Queue Base
    WriteDword(SRAM_CMDS_24 + 0x0c, SRAM_INSTRUCTION_QUEUE_VBI);
    
    // Instruction Queue Size is in DWORDs
    WriteDword(SRAM_CMDS_24 + 0x10, (SRAM_INSTRUCTION_QUEUE_SIZE / 4));
    
    // Cluster table base 
    WriteDword(SRAM_CMDS_24 + 0x04, SRAM_CLUSTER_TABLE_VBI); 

    // Cluster table size is in QWORDS
    WriteDword(SRAM_CMDS_24 + 0x08, (SRAM_CLUSTER_TABLE_VBI_SIZE / 8));

    // Fill in cluster buffer entries
    for(i = 0; i < SRAM_VBI_BUFFERS; ++i)
    {
        WriteDword(
                    SRAM_CLUSTER_TABLE_VBI + (i * 0x10), 
                    SRAM_FIFO_VBI_BUFFERS + (i * SRAM_FIFO_VBI_BUFFER_SIZE)
                  );
    }
    
    // Copy the cluster buffer info to the DMAC 
    
    // Set the DMA Cluster Table Address
    WriteDword( MO_DMA24_PTR2, SRAM_CLUSTER_TABLE_VBI);
    
    // Set the DMA buffer limit size in qwords
    WriteDword( MO_DMA24_CNT1, SRAM_FIFO_VBI_BUFFER_SIZE / 8);
    
    // Set the DMA Cluster Table Size in qwords
    WriteDword( MO_DMA24_CNT2, (SRAM_CLUSTER_TABLE_VBI_SIZE / 8));

    /////////////////////////////////////////////////////////////////
    // Other one off settings for the chip
    /////////////////////////////////////////////////////////////////

    // set format to YUY2
    MaskDataDword(CT2388X_VIDEO_COLOR_FORMAT, 0x00000044, 0x000000FF);

    // Test from Mike Asbury's regtool init code
    WriteDword( MO_PDMA_STHRSH, 0x0807 ); // Fifo source Threshhold
    WriteDword( MO_PDMA_DTHRSH, 0x0807 ); // Fifo Threshhold

    WriteDword( CT2388X_VID_INTSTAT, 0xFFFFFFFF ); // Clear PIV int
    WriteDword( CT2388X_PCI_INTSTAT, 0xFFFFFFFF ); // Clear PCI int
    WriteDword( MO_INT1_STAT, 0xFFFFFFFF );   // Clear RISC int

    //
    // Fixes for flashing suggested by Ben Felts
    //
    // 1.  Set bits 16:9 of register 0xE4310208 to 0x00.  
    //     The default value is 0x03803C0F, which becomes 0x0380000F with this change.
    WriteDword( 0x00310208, 0x0380000F ); 

    //2.  Set bits 27:26 of register 0xE4310200 to 0x0.  The default value is
    //    0x0CE00555, which becomes 0x00E00555 with this change.
    WriteDword( 0x00310200, 0x00E00555 ); 

    // Disable all of the interrupts
    WriteDword( CT2388X_VID_INTMSK, 0x00000000 );
}    


LPCSTR CCT2388xCard::GetInputName(int nInput)
{
    if(nInput < m_TVCards[m_CardType].NumInputs && nInput >= 0)
    {
        return m_TVCards[m_CardType].Inputs[nInput].szName;
    }
    return "Error";
}

ULONG CCT2388xCard::GetTickCount()
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);
    ticks = (ticks & 0xFFFFFFFF00000000) / frequency * 10000000 +
            (ticks & 0xFFFFFFFF) * 10000000 / frequency;
    return (ULONG)(ticks / 10000);
}

void CCT2388xCard::InitializeI2C()
{
    WriteDword(CT2338X_I2C, 1);
    m_I2CRegister = ReadDword(CT2338X_I2C);

    m_I2CSleepCycle = 10000L;
    DWORD elapsed = 0L;
    // get a stable reading
    while (elapsed < 5)
    {
        m_I2CSleepCycle *= 10;
        DWORD start = GetTickCount();
        for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
        elapsed = GetTickCount() - start;
    }
    // calculate how many cycles a 50kHZ is (half I2C bus cycle)
    m_I2CSleepCycle = m_I2CSleepCycle / elapsed * 1000L / 50000L;
    
    m_I2CInitialized = true;
}

void CCT2388xCard::Sleep()
{
    for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
}

void CCT2388xCard::SetSDA(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        m_I2CRegister |= CT2338X_I2C_SDA;
    }
    else
    {
        m_I2CRegister &= ~CT2338X_I2C_SDA;
    }
    WriteDword(CT2338X_I2C, m_I2CRegister);
}

void CCT2388xCard::SetSCL(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        m_I2CRegister |= CT2338X_I2C_SCL;
    }
    else
    {
        m_I2CRegister &= ~CT2338X_I2C_SCL;
    }
    WriteDword(CT2338X_I2C, m_I2CRegister);
}

bool CCT2388xCard::GetSDA()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    bool state = ReadDword(CT2338X_I2C) & CT2338X_I2C_SDA ? true : false;
    return state;
}

bool CCT2388xCard::GetSCL()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    bool state = ReadDword(CT2338X_I2C) & CT2338X_I2C_SCL ? true : false;
    return state;
}



/*******************************************************************************
    Program the PLL to a specific output frequency.
    Assume that we have a PLL pre dividor of 2
*******************************************************************************/
void CCT2388xCard::SetPLL(double PLLFreq)
{
    DWORD RegValue = 0;
    int Prescaler = 2;
    double PLLValue;
    int PLLInt = 0;
    int PLLFraction;

    PLLValue = PLLFreq * 8.0 * (double)Prescaler / 28.63636;

    while(PLLValue < 14.0 && Prescaler < 5)
    {
        ++Prescaler;
        PLLValue = PLLFreq * 8.0 * (double)Prescaler / 28.63636;
    }

    switch(Prescaler)
    {
    case 2:
        RegValue = 0 << 26;
        break;
    case 3:
        RegValue = 3 << 26;
        break;
    case 4:
        RegValue = 2 << 26;
        break;
    case 5:
        RegValue = 1 << 26;
        break;
    default:
        LOG(0, "Invalid PLL Pre Scaler value %d", Prescaler);
        break;
    }

    PLLInt = (int)PLLValue;
    PLLFraction = (int)((PLLValue - (double)PLLInt) * (double)(1<<20) + 0.5);

    // Check for illegal PLL values
    if( PLLInt < 14 || PLLInt > 63)
    {
        LOG(0, "Invalid PLL value %f MHz", PLLFreq);
        return;
    }
    
    // Set register int and fraction values
    RegValue |= PLLInt << 20;
    RegValue |= PLLFraction & 0xFFFFF;
    
    WriteDword(CT2388X_PLL , RegValue );
}

void CCT2388xCard::SetSampleRateConverter(double PLLFreq)
{
    DWORD RegValue = (DWORD)((28.636363 / PLLFreq) * (double)(1<<17));
    WriteDword( CT2388X_SAMPLERATECONV, RegValue & 0x7FFFF );
}

eTunerId CCT2388xCard::AutoDetectTuner(eCT2388xCardId CardId)
{
    return TUNER_ABSENT;
}

BOOL CCT2388xCard::InitTuner(eTunerId tunerId)
{
    // clean up if we get called twice
    if(m_Tuner != NULL)
    {
        delete m_Tuner; 
        m_Tuner = NULL;
    }

    switch (tunerId)
    {
    case TUNER_MT2032:
        // \todo fix this
        //m_Tuner = new CMT2032();
        //strcpy(m_TunerType, "MT2032 ");
        break;
    case TUNER_AUTODETECT:
    case TUNER_USER_SETUP:
    case TUNER_ABSENT:
        m_Tuner = new CNoTuner();
        strcpy(m_TunerType, "None ");
        break;
    default:
        m_Tuner = new CGenericTuner(tunerId);
        strcpy(m_TunerType, "Generic ");
        break;
    }
    if (tunerId != TUNER_ABSENT) 
    {
        int kk = strlen(m_TunerType);
        for (BYTE test = 0xC0; test < 0xCF; test +=2)
        {
            if (m_I2CBus->Write(&test, sizeof(test)))
            {
                m_Tuner->Attach(m_I2CBus, test>>1);
                sprintf(m_TunerType + kk, " @I2C@0x%02x", test);
                break;
            }
        }
    }
    return TRUE;
}

void CCT2388xCard::SetRISCStartAddress(DWORD RiscBasePhysical)
{
    WriteDword( SRAM_CMDS_21, RiscBasePhysical); // RISC STARTING ADDRESS

    // Set as PCI address
    AndDataDword( SRAM_CMDS_21 + 0x10, 0x7fffffff); 
}

void CCT2388xCard::SetRISCStartAddressVBI(DWORD RiscBasePhysical)
{
    WriteDword( SRAM_CMDS_24, RiscBasePhysical); // RISC STARTING ADDRESS

    // Set as PCI address
    AndDataDword( SRAM_CMDS_24 + 0x10, 0x7fffffff); 
}

ITuner* CCT2388xCard::GetTuner() const
{
    return m_Tuner;
}

#define DumpRegister(Reg) fprintf(hFile, #Reg "\t%08x\n", ReadDword(Reg))

void CCT2388xCard::DumpChipStatus()
{
    FILE* hFile;

    hFile = fopen("CT2388x.txt", "w");
    if(!hFile)
    {
        return;
    }

    fprintf(hFile, "SubSystemId\t%08x\n", m_SubSystemId);

    DumpRegister(CT2388X_DEVICE_STATUS);
    DumpRegister(CT2388X_VIDEO_INPUT);
    DumpRegister(CT2388X_TEMPORAL_DEC);
    DumpRegister(CT2388X_AGC_BURST_DELAY);
    DumpRegister(CT2388X_BRIGHT_CONTRAST); 
    DumpRegister(CT2388X_UVSATURATION);    
    DumpRegister(CT2388X_HUE);             
    DumpRegister(CT2388X_WHITE_CRUSH);
    DumpRegister(CT2388X_PIXEL_CNT_NOTCH);
    DumpRegister(CT2388X_HORZ_DELAY_EVEN);
    DumpRegister(CT2388X_HORZ_DELAY_ODD);
    DumpRegister(CT2388X_VERT_DELAY_EVEN);
    DumpRegister(CT2388X_VERT_DELAY_ODD);
    DumpRegister(CT2388X_VDELAYCCIR_EVEN);
    DumpRegister(CT2388X_VDELAYCCIR_ODD);
    DumpRegister(CT2388X_HACTIVE_EVEN);
    DumpRegister(CT2388X_HACTIVE_ODD);
    DumpRegister(CT2388X_VACTIVE_EVEN);    
    DumpRegister(CT2388X_VACTIVE_ODD);     
    DumpRegister(CT2388X_HSCALE_EVEN);     
    DumpRegister(CT2388X_HSCALE_ODD);      
    DumpRegister(CT2388X_VSCALE_EVEN);     
    DumpRegister(CT2388X_VSCALE_ODD);      
    DumpRegister(CT2388X_FILTER_EVEN);     
    DumpRegister(CT2388X_FILTER_ODD);      
    DumpRegister(CT2388X_FORMAT_2HCOMB);
    DumpRegister(CT2388X_PLL);
    DumpRegister(CT2388X_PLL_ADJUST);
    DumpRegister(CT2388X_SAMPLERATECONV);  
    DumpRegister(CT2388X_SAMPLERATEFIFO);  
    DumpRegister(CT2388X_SUBCARRIERSTEP);  
    DumpRegister(CT2388X_SUBCARRIERSTEPDR);
    DumpRegister(CT2388X_CAPTURECONTROL);  
    DumpRegister(CT2388X_VIDEO_COLOR_FORMAT);
    DumpRegister(CT2388X_VBI_SIZE);
    DumpRegister(CT2388X_FIELD_CAP_CNT);
    DumpRegister(CT2388X_VIP_CONFIG);
    DumpRegister(CT2388X_VIP_CONTBRGT);
    DumpRegister(CT2388X_VIP_HSCALE);
    DumpRegister(CT2388X_VIP_VSCALE);
    DumpRegister(CT2388X_VBOS);

    fclose(hFile);
}
