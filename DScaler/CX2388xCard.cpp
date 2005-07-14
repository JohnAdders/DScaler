/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xCard.cpp,v 1.74 2005-07-14 05:05:21 dosx86 Exp $
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.73  2005/06/09 23:22:00  robmuller
// Fixed bug in GetTickCount().
//
// Revision 1.72  2005/03/24 17:57:57  adcockj
// Card access from one thread at a time
//
// Revision 1.71  2005/03/23 14:20:36  adcockj
// Test fix for threading issues
//
// Revision 1.70  2004/12/25 22:40:17  to_see
// Changed the card list to an ini file
//
// Revision 1.69  2004/12/20 18:55:33  to_see
// Moved tuner code to new file CX2388xCard_Tuner.cpp
//
// Revision 1.68  2004/11/29 18:02:57  to_see
// Changed TDA9887 detecting to Atsushi's new tda code.
//
// Revision 1.67  2004/11/13 21:45:56  to_see
// - Some minor fixes
// - Added "Vertical Sync Detection" in CX2388x Advanced Settings, enabled by default.
//   It reduces dead lock problems dramaticaly if no video signal is present. Faster videosignal detection.
//
// Revision 1.66  2004/09/29 20:36:02  to_see
// Added Card AverTV303, Thanks to Zbigniew Pluta
//
// Revision 1.65  2004/08/27 13:12:40  to_see
// Added audio support for Ati Tv Wonder Pro
//
// Revision 1.64  2004/07/10 11:57:17  adcockj
// improved cx2388x driver coverage when disabling drivers
//
// Revision 1.63  2004/06/23 20:15:22  to_see
// Created an new struct TControlSettings for better handling
// more cards and deleted class CTDA9887MsiMaster. Thanks to atnak.
//
// Revision 1.62  2004/06/02 18:43:54  to_see
// New TAudioRegList structure to hold audio register
// settings for better handling
//
// Revision 1.61  2004/05/21 18:35:58  to_see
// Bugfix: Moved StartStopConexantDriver code from CX2388xCard to CCX2388xSource that the driver is stoped before CCX2388xCard::InitTuner is called.
//
// More Loging when StartStopConexantDriver is not able to Stop the WDM-Driver when other SW uses the card Hardware.
//
// Revision 1.60  2004/05/16 19:45:08  to_see
// Added an new class for Msi Master Card
//
// Revision 1.59  2004/04/19 20:38:38  adcockj
// Fix for previous fix (must learn to program...)
//
// Revision 1.58  2004/04/19 15:13:20  adcockj
// Fix failing to find tda9887 at alternate addresses
//
// Revision 1.57  2004/04/18 12:01:03  adcockj
// Fixes for eeprom corruption
//
// Revision 1.56  2004/03/07 17:34:48  to_see
// moved CCX2388xCard::AutoDetectTuner from CX2388xCard.cpp to CX2388xCard_Types.cpp
// to can use correct sizeof(m_Tuners_Hauppauge_CX2388x_Card)
//
// Revision 1.55  2004/02/27 20:50:59  to_see
// -more logging in CCX2388xCard::StartStopConexxantDriver
// -handling for IDC_AUTODETECT in CX2388xSource_UI.cpp
// -renamed eAudioStandard to eCX2388xAudioStandard,
//  eStereoType to eCX2388xStereoType and moved from
//  cx2388xcard.h to cx2388x_defines.h
// -moved Audiodetecting from CX2388xCard_Audio.cpp
//  to CX2388xSource_Audio.cpp
// -CCX2388xCard::AutoDetectTuner read
//  at first from Registers
//
// Revision 1.54  2004/02/21 21:47:06  to_see
// Added AutodetectTuner for Hauppauge
//
// Revision 1.53  2004/02/21 14:11:29  to_see
// more A2-code
//
// Revision 1.52  2004/02/11 20:34:00  adcockj
// Support multiple locations of TDA9887 (thanks to Pityu)
//
// Revision 1.51  2004/02/08 13:08:50  adcockj
// SECAM changes suggested by MidiMaker
//
// Revision 1.50  2004/02/06 08:01:21  adcockj
// Fixed a couple of minor issues with Torsten's changes
//
// Revision 1.49  2004/02/05 21:47:53  to_see
// Starting/Stopping connexant-drivers while dscaler is running.
// To Enable/Disable it, go to Settings->Advanced Settings->
// CX2388X Advanced->Stopping Conexxant driver while Dscaler is running.
//
// This enables sound on my card without to go to windows control panel.
//
// Revision 1.48  2004/01/18 21:13:59  adcockj
// Fix for chip setup with no driver
//
// Revision 1.47  2004/01/16 09:35:12  adcockj
// CX2388x sound fixes suggested by MidiMaker
//
// Revision 1.46  2004/01/07 10:27:18  adcockj
// Patch from MidiMaker to fix issue with PAL comb filter
//
// Revision 1.45  2004/01/05 13:12:24  adcockj
// Added patch from Lavrenov Dmitrij (midimaker)
//
// Revision 1.44  2003/12/18 15:57:41  adcockj
// Added MT2050 tuner type support (untested)
//
// Revision 1.43  2003/10/27 10:39:51  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.42  2003/02/22 12:27:45  adcockj
// Solution to crashin problem with some cards
//
// Revision 1.41  2003/02/15 13:04:47  laurentg
// Setting Chroma 2H Comb : On is 0 and OFF is 1
//
// Revision 1.40  2003/02/03 17:25:20  adcockj
// Fixes for PlayHD
//
// Revision 1.39  2003/01/29 18:24:49  adcockj
// Test fix for crashing in other apps after running DScaler
//
// Revision 1.38  2003/01/27 22:04:07  laurentg
// First step to merge setup hardware and hardware info dialog boxes
// CPU flag information moved in the general hardware dialog box
// Hardware info dialog box available for CX2388x
//
// Revision 1.37  2003/01/26 12:33:26  adcockj
// Fixed problem with PAL60
//
// Revision 1.36  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.35  2003/01/15 15:54:22  adcockj
// Fixed some keyboard focus issues
//
// Revision 1.34  2003/01/13 17:46:44  adcockj
// HDelay and VDelay turned from absolute to adjustments
//
// Revision 1.33  2003/01/06 10:33:37  adcockj
// Set correct number of Dwords for VBI at 720 pixels
//
// Revision 1.32  2003/01/05 19:42:24  laurentg
// Correction to John's correction correcting Laurent's correction
//
// Revision 1.31  2003/01/05 19:01:13  adcockj
// Made some changes to Laurent's last set of VBI fixes
//
// Revision 1.30  2003/01/04 21:40:15  laurentg
// Update the default vertical delay when pixel width is different from 720 (32 replaced by 36)
//
// Revision 1.29  2003/01/04 21:00:33  laurentg
// Setting of register CX2388x_VBI_SIZE updated when pixel width is 720
//
// Revision 1.28  2002/12/15 13:54:13  adcockj
// Added code to reset card better
//
// Revision 1.27  2002/12/10 14:53:15  adcockj
// Sound fixes for cx2388x
//
// Revision 1.26  2002/12/04 17:43:49  adcockj
// Contrast and Brightness adjustments so that h3d card behaves in expected way
//
// Revision 1.25  2002/12/02 13:47:01  adcockj
// Allow fine control over white crush settings
//
// Revision 1.24  2002/11/28 18:07:37  adcockj
// Fixed overflowing registers
//
// Revision 1.23  2002/11/28 15:15:21  adcockj
// Fixed pll dump
//
// Revision 1.22  2002/11/27 17:41:57  adcockj
// Fixed setting PLL registers in DumpRegister
//
// Revision 1.21  2002/11/25 12:22:17  adcockj
// Added audio registers to debug file to help with audio questions
//
// Revision 1.20  2002/11/25 09:52:55  adcockj
// Commented out state code
//
// Revision 1.19  2002/11/12 19:32:21  adcockj
// Fixes for pixel width code
//
// Revision 1.18  2002/11/12 15:22:45  adcockj
// Made new flag settings have default setting
// Added pixel width for CX2388x cards
//
// Revision 1.17  2002/11/11 17:10:37  adcockj
// Added GPIO to dump and save/restore
//
// Revision 1.16  2002/11/09 20:53:46  laurentg
// New CX2388x settings
//
// Revision 1.15  2002/11/09 00:22:23  laurentg
// New settings for CX2388x chip
//
// Revision 1.14  2002/11/08 11:54:51  adcockj
// try looking for TDA9887 with MT2032 tuners
//
// Revision 1.13  2002/11/07 21:06:12  adcockj
// Fixes to prevent hanging with card that's not been initilaised
//
// Revision 1.12  2002/11/07 20:33:16  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.11  2002/11/07 20:06:07  adcockj
// Fixed problems with manage state function
//
// Revision 1.10  2002/11/07 13:37:43  adcockj
// Added State restoration code to PCICard
// Functionality disabled prior to testing and not done for SAA7134
//
// Revision 1.9  2002/11/06 20:15:27  adcockj
// Centered pixels to work with my equipment
//
// Revision 1.8  2002/11/06 11:11:23  adcockj
// Added new Settings and applied Laurent's filter setup suggestions
//
// Revision 1.7  2002/11/03 18:38:32  adcockj
// Fixes for cx2388x and PAL & NTSC
//
// Revision 1.6  2002/11/03 15:54:10  adcockj
// Added cx2388x register tweaker support
//
// Revision 1.5  2002/10/31 15:55:46  adcockj
// Moved audio code from Connexant dTV version
//
// Revision 1.4  2002/10/31 14:09:54  adcockj
// Move back to 720 pixel width now that we have analogue blanking mode
//
// Revision 1.3  2002/10/31 13:55:15  adcockj
// Made VBI similar to bt848
//
// Revision 1.2  2002/10/29 16:20:29  adcockj
// Added card setup for MSI TV@nywhere (no work done on sound)
//
// Revision 1.1  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// 
// CVS Log while file was called CT2388xCard.cpp
//
// Revision 1.20  2002/10/27 19:17:25  adcockj
// Fixes for cx2388x - PAL & NTSC tested
//
// Revision 1.19  2002/10/25 14:46:24  laurentg
// Notes for John about SECAM and the old mode
//
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
// Prelimainary support for CX2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xCard.cpp CCX2388xCard Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "..\DScalerResDbg\CX2388xRes\resource.h"
#include "resource.h"
#include "CX2388xCard.h"
#include "CX2388x_Defines.h"
#include "DebugLog.h"
#include "CPU.h"
#include "TVFormats.h"
#include "DScaler.h"

CCX2388xCard::CCX2388xCard(CHardwareDriver* pDriver) :
    CPCICard(pDriver),
    m_CardType(CX2388xCARD_UNKNOWN),
    m_Tuner(NULL),
    m_SAA7118(NULL),
    m_RISCIsRunning(FALSE),
    m_CurrentInput(0),
	m_CurrentAudioStandard(AUDIO_STANDARD_AUTO),
	m_CurrentStereoType(STEREOTYPE_AUTO)
{
    strcpy(m_TunerType,"n/a");

    m_I2CInitialized = false;
    m_I2CBus = new CI2CBusForLineInterface(this);
    m_AudioControls = new CAudioControls();
    m_AudioDecoder = new CAudioDecoder();
}

CCX2388xCard::~CCX2388xCard()
{
    delete m_I2CBus;
    delete m_AudioDecoder;
    delete m_AudioControls;
    delete m_Tuner;
    delete m_SAA7118;

    ClosePCICard();
}

void CCX2388xCard::StartCapture(BOOL bCaptureVBI)
{
    DWORD value1;
    DWORD value2;
   
    // RISC Controller Enable
    WriteDword(CX2388X_DEV_CNTRL2, 1<<5 );

    // Clear Interrupt Status bits
    WriteDword(CX2388X_VID_INTSTAT, 0x0000000);
   
    value1 = ReadDword(CX2388X_VID_DMA_CNTRL) & 0xFFFFFF00;
    value2 = (ReadDword(CX2388X_CAPTURECONTROL) & 0xFFFFFF00);
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

    WriteDword(CX2388X_VID_DMA_CNTRL, value1);
    WriteDword(CX2388X_CAPTURECONTROL, value2);

    // Clear Interrupt Status bits
    WriteDword(CX2388X_VID_INTSTAT, 0xFFFFFFFF );

    m_RISCIsRunning = TRUE;
}

/*******************************************************************************
    Halt the Odd/Both Risc Program.
    both_fields == true - Halt both fields.
                == false - Halt odd field only.
*******************************************************************************/
void CCX2388xCard::StopCapture()
{
    // Firstly stop the card from doing anything
    // so stop the risc controller
    WriteDword(CX2388X_DEV_CNTRL2, 0x00000000);

    // then stop capturing video and VBI
    MaskDataDword(CX2388X_VID_DMA_CNTRL, 0x00, 0xFF);
    MaskDataDword(CX2388X_CAPTURECONTROL, 0x00,0xFF);

    // set the RISC addresses to be NULL
    // so that nobody tries to run our RISC code later
    SetRISCStartAddress(0x00000000);
    SetRISCStartAddressVBI(0x00000000);

    m_RISCIsRunning = FALSE;

/*
	// 2004/12/25 to_see: unused code

    // perform card specific Stop Capture
    if(m_CX2388xCards[m_CardType].pStopCaptureCardFunction != NULL)
    {
        // call correct function
        (*this.*m_CX2388xCards[m_CardType].pStopCaptureCardFunction)();
    }
*/
}

void CCX2388xCard::SetCardType(int CardType)
{
    if(m_CardType != CardType)
    {
        m_CardType = (eCX2388xCardId)CardType;

        // perform card specific init
        if(IsCurCardH3D())
        {
            InitH3D();
        }
    }
}

eCX2388xCardId CCX2388xCard::GetCardType()
{
    return m_CardType;
}

LPCSTR CCX2388xCard::GetCardName(eCX2388xCardId CardId)
{
    return m_CX2388xCards[CardId].szName;
}

void CCX2388xCard::SetContrastBrightness(BYTE Contrast, BYTE Brightness)
{
    if(IsCurCardH3D())
    {
        SetH3DContrastBrightness(Contrast, Brightness);
    }
    else
    {
        SetAnalogContrastBrightness(Contrast, Brightness);
    }
}

void CCX2388xCard::SetHue(BYTE Hue)
{
    if(IsCurCardH3D())
    {
        SetH3DHue(Hue);
    }
    else
    {
        SetAnalogHue(Hue);
    }
}

void CCX2388xCard::SetSaturationU(BYTE SaturationU)
{
    if(IsCurCardH3D())
    {
        SetH3DSaturationU(SaturationU);
    }
    else
    {
        SetAnalogSaturationU(SaturationU);
    }
}

void CCX2388xCard::SetSaturationV(BYTE SaturationV)
{
    if(IsCurCardH3D())
    {
        SetH3DSaturationV(SaturationV);
    }
    else
    {
        SetAnalogSaturationV(SaturationV);
    }
}

void CCX2388xCard::StandardSetFormat(int nInput, eVideoFormat Format, BOOL IsProgressive)
{
    // do nothing
}

void CCX2388xCard::SetAnalogContrastBrightness(BYTE Contrast, BYTE Brightness)
{
    DWORD dwval = ReadDword(CX2388X_BRIGHT_CONTRAST);
    dwval &= 0xffff0000;
    Brightness = (BYTE)(unsigned char)(Brightness - 0x80);
    dwval |= (Brightness & 0xff);
    dwval |= ((Contrast & 0xFF) << 8 );
    WriteDword(CX2388X_BRIGHT_CONTRAST,dwval);
}

void CCX2388xCard::SetAnalogHue(BYTE Hue)
{
    DWORD dwval = ReadDword(CX2388X_HUE);
    dwval &= 0xffffff00;
    Hue = (BYTE)(unsigned char)(Hue - 0x80);
    dwval |= (Hue & 0xff);
    WriteDword(CX2388X_HUE, dwval);
}

void CCX2388xCard::SetAnalogSaturationU(BYTE SaturationU)
{
    DWORD dwval = ReadDword(CX2388X_UVSATURATION);
    dwval &= 0xffffff00;
    dwval |= (SaturationU & 0xff);
    WriteDword(CX2388X_UVSATURATION,dwval);
}

void CCX2388xCard::SetAnalogSaturationV(BYTE SaturationV)
{
    DWORD dwval = ReadDword(CX2388X_UVSATURATION);
    dwval &= 0xffff00ff;
    dwval |= ((SaturationV & 0xFF) << 8 );
    WriteDword(CX2388X_UVSATURATION,dwval);
}


void CCX2388xCard::SetVIPBrightness(BYTE Brightness)
{
    DWORD dwval = ReadDword(CX2388X_VIP_CONTBRGT);
    dwval &= 0xffffff00;
    Brightness = (BYTE)(unsigned char)(Brightness - 0x80);
    dwval |= (Brightness & 0xFF);
    WriteDword(CX2388X_VIP_CONTBRGT, dwval);
}

void CCX2388xCard::SetVIPContrast(BYTE Contrast)
{
    DWORD dwval = ReadDword(CX2388X_VIP_CONTBRGT);
    dwval &= 0xffff00ff;
    dwval |= ((Contrast & 0xFF) << 8 );
    WriteDword(CX2388X_VIP_CONTBRGT, dwval);
}

void CCX2388xCard::SetVIPSaturation(BYTE Saturation)
{
    DWORD dwval = ReadDword(CX2388X_VIP_SATURATION);
    dwval &= 0xffffff00;
    dwval |= (Saturation & 0xFF);
    WriteDword(CX2388X_VIP_SATURATION, dwval);
}

void CCX2388xCard::SetLumaAGC(BOOL LumaAGC)
{
    if(LumaAGC)
    {
        OrDataDword(CX2388X_VIDEO_INPUT, (1 << 13) | (1 << 12));
    }
    else
    {
        AndDataDword(CX2388X_VIDEO_INPUT, ~((1 << 13) | (1 << 12)));
    }
}

void CCX2388xCard::SetChromaAGC(BOOL ChromaAGC)
{
    if(ChromaAGC)
    {
        OrDataDword(CX2388X_VIDEO_INPUT, (1 << 10));
    }
    else
    {
        AndDataDword(CX2388X_VIDEO_INPUT, ~(1 << 10));
    }
}

void CCX2388xCard::SetFastSubcarrierLock(BOOL LockFast)
{
    if(LockFast)
    {
        OrDataDword(CX2388X_VIDEO_INPUT, (1 << 8));
    }
    else
    {
        AndDataDword(CX2388X_VIDEO_INPUT, ~(1 << 8));
    }
}

void CCX2388xCard::SetVerticalSyncDetection(BOOL SyncDetection)
{
    if(SyncDetection)
    {
        OrDataDword(CX2388X_VIDEO_INPUT, CX2388X_VIDEO_INPUT_VERTEN);
    }
    else
    {
        AndDataDword(CX2388X_VIDEO_INPUT, ~CX2388X_VIDEO_INPUT_VERTEN);
    }
}

void CCX2388xCard::SetWhiteCrushEnable(BOOL WhiteCrush)
{
    if(WhiteCrush)
    {
        OrDataDword(CX2388X_VIDEO_INPUT, (1 << 11));
    }
    else
    {
        AndDataDword(CX2388X_VIDEO_INPUT, ~(1 << 11));
    }
}

void CCX2388xCard::SetWhiteCrushUp(BYTE WhiteCrushUp)
{
    MaskDataDword(CX2388X_WHITE_CRUSH, WhiteCrushUp, 0x3F);
}

void CCX2388xCard::SetWhiteCrushDown(BYTE WhiteCrushDown)
{
    MaskDataDword(CX2388X_WHITE_CRUSH, WhiteCrushDown << 8, (0x3F << 8));
}

void CCX2388xCard::SetWhiteCrushMajorityPoint(eWhiteCrushMajSel WhiteCrushMajSel)
{
    MaskDataDword(CX2388X_WHITE_CRUSH, WhiteCrushMajSel << 6, 0x3 << 6);
}

void CCX2388xCard::SetWhiteCrushPerFrame(BOOL WhiteCrushPerFrame)
{
    MaskDataDword(CX2388X_WHITE_CRUSH, WhiteCrushPerFrame << 14, (1 << 14));
}

void CCX2388xCard::SetLowColorRemoval(BOOL LowColorRemoval)
{
    if(LowColorRemoval)
    {
        OrDataDword(CX2388X_VIDEO_INPUT, (1 << 9));
    }
    else
    {
        AndDataDword(CX2388X_VIDEO_INPUT, ~(1 << 9));
    }
}

void CCX2388xCard::SetCombFilter(eCombFilter CombFilter)
{
	switch(CombFilter)
	{
	case COMBFILTER_OFF:
        OrDataDword(CX2388X_FILTER_EVEN, (1 << 5) | (1 << 6));
        OrDataDword(CX2388X_FILTER_ODD, (1 << 5) | (1 << 6));
		break;
	case COMBFILTER_CHROMA_ONLY:
        OrDataDword(CX2388X_FILTER_EVEN, (1 << 5));
        AndDataDword(CX2388X_FILTER_EVEN, ~(1 << 6));
        OrDataDword(CX2388X_FILTER_ODD, (1 << 5));
        AndDataDword(CX2388X_FILTER_ODD, ~(1 << 6));
		break;
	case COMBFILTER_FULL:
        AndDataDword(CX2388X_FILTER_EVEN, ~((1 << 5) | (1 << 6)));
        AndDataDword(CX2388X_FILTER_ODD, ~((1 << 5) | (1 << 6)));
		break;
    case COMBFILTER_DEFAULT:
        if(m_CX2388xCards[m_CardType].Inputs[m_CurrentInput].InputType == INPUTTYPE_SVIDEO)
        {
            OrDataDword(CX2388X_FILTER_EVEN, (1 << 5));
            AndDataDword(CX2388X_FILTER_EVEN, ~(1 << 6));
            OrDataDword(CX2388X_FILTER_ODD, (1 << 5));
            AndDataDword(CX2388X_FILTER_ODD, ~(1 << 6));
        }
        else
        {
            AndDataDword(CX2388X_FILTER_EVEN, ~((1 << 5) | (1 << 6)));
            AndDataDword(CX2388X_FILTER_ODD, ~((1 << 5) | (1 << 6)));
        }
		break;
	default:
		break;
	}
}

void CCX2388xCard::SetFullLumaRange(BOOL FullLumaRange)
{
    if(FullLumaRange)
    {
        OrDataDword(CX2388X_FORMAT_2HCOMB, (1 << 3));
    }
    else
    {
        AndDataDword(CX2388X_FORMAT_2HCOMB, ~(1 << 3));
    }
}

void CCX2388xCard::SetRemodulation(eFlagWithDefault Remodulation)
{
    switch(Remodulation)
    {
    case FLAG_ON:
        AndDataDword(CX2388X_FORMAT_2HCOMB, ~(1 << 8));
        break;
    case FLAG_OFF:
        OrDataDword(CX2388X_FORMAT_2HCOMB, (1 << 8));
        break;
    default:
        MaskDataDword(CX2388X_FORMAT_2HCOMB, m_2HCombDefault, (1 << 8));
        break;
    }

}

void CCX2388xCard::SetChroma2HComb(eFlagWithDefault Chroma2HComb)
{
    switch(Chroma2HComb)
    {
    case FLAG_ON:
        AndDataDword(CX2388X_FORMAT_2HCOMB, ~(1 << 9));
        break;
    case FLAG_OFF:
        OrDataDword(CX2388X_FORMAT_2HCOMB, (1 << 9));
        break;
    default:
        MaskDataDword(CX2388X_FORMAT_2HCOMB, m_2HCombDefault, (1 << 9));
        break;
    }
}

void CCX2388xCard::SetForceRemodExcessChroma(eFlagWithDefault ForceRemodExcessChroma)
{
    switch(ForceRemodExcessChroma)
    {
    case FLAG_ON:
        OrDataDword(CX2388X_FORMAT_2HCOMB, (1 << 10));
        break;
    case FLAG_OFF:
        AndDataDword(CX2388X_FORMAT_2HCOMB, ~(1 << 10));
        break;
    default:
        MaskDataDword(CX2388X_FORMAT_2HCOMB, m_2HCombDefault, (1 << 10));
        break;
    }
}

void CCX2388xCard::SetIFXInterpolation(eFlagWithDefault IFXInterpolation)
{
    switch(IFXInterpolation)
    {
    case FLAG_ON:
        AndDataDword(CX2388X_FORMAT_2HCOMB, ~(1 << 15));
        break;
    case FLAG_OFF:
        OrDataDword(CX2388X_FORMAT_2HCOMB, (1 << 15));
        break;
    default:
        MaskDataDword(CX2388X_FORMAT_2HCOMB, m_2HCombDefault, (1 << 15));
        break;
    }
}

void CCX2388xCard::SetCombRange(int CombRange)
{
    DWORD dwval = ReadDword(CX2388X_FORMAT_2HCOMB);
    dwval &= 0xfc00ffff;
    dwval |= ((CombRange & 0x3FF) << 16 );
    WriteDword(CX2388X_FORMAT_2HCOMB, dwval);
}

void CCX2388xCard::SetSecondChromaDemod(eFlagWithDefault SecondChromaDemod)
{
    switch(SecondChromaDemod)
    {
    case FLAG_ON:
        OrDataDword(CX2388X_FILTER_EVEN, (1 << 16));
        OrDataDword(CX2388X_FILTER_ODD, (1 << 16));
        break;
    case FLAG_OFF:
        AndDataDword(CX2388X_FILTER_EVEN, ~(1 << 16));
        AndDataDword(CX2388X_FILTER_ODD, ~(1 << 16));
        break;
    default:
        MaskDataDword(CX2388X_FILTER_EVEN, m_FilterDefault, (1 << 16));
        MaskDataDword(CX2388X_FILTER_ODD, m_FilterDefault, (1 << 16));
        break;
    }
}

void CCX2388xCard::SetThirdChromaDemod(eFlagWithDefault ThirdChromaDemod)
{
    switch(ThirdChromaDemod)
    {
    case FLAG_ON:
        OrDataDword(CX2388X_FILTER_EVEN, (1 << 17));
        OrDataDword(CX2388X_FILTER_ODD, (1 << 17));
        break;
    case FLAG_OFF:
        AndDataDword(CX2388X_FILTER_EVEN, ~(1 << 17));
        AndDataDword(CX2388X_FILTER_ODD, ~(1 << 17));
        break;
    default:
        MaskDataDword(CX2388X_FILTER_EVEN, m_FilterDefault, (1 << 17));
        MaskDataDword(CX2388X_FILTER_ODD, m_FilterDefault, (1 << 17));
        break;
    }
}


void CCX2388xCard::SetHDelay(int nInput, eVideoFormat TVFormat, long CurrentX, int HDelayAdj)
{
    int HorzDelay;
    int CurrentY = GetTVFormat(TVFormat)->wCropHeight;

    if(IsCCIRSource(nInput))
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
    else if(CurrentX == 720)
    {
        if (CurrentY == 576)
        {
            HorzDelay = 0x82;
        }
        else
        {
            HorzDelay = 0x7E;
        }
    }
    else
    {
        if (CurrentY == 576)
        {
            HorzDelay = ((CurrentX * GetTVFormat(TVFormat)->wHDelayx1) / GetTVFormat(TVFormat)->wHActivex1) & 0x3fe;
        }
        else
        {
            HorzDelay = ((CurrentX * GetTVFormat(TVFormat)->wHDelayx1) / GetTVFormat(TVFormat)->wHActivex1) & 0x3fe;
        }
    }
    
    HorzDelay += HDelayAdj;

    WriteDword(CX2388X_HORZ_DELAY_EVEN, HorzDelay);
    WriteDword(CX2388X_HORZ_DELAY_ODD, HorzDelay);

    if(HorzDelay & 1)
    {
        // think we need to invert something here
        // but I've put step as 2 for time being
    }

}

void CCX2388xCard::SetVDelay(int nInput, eVideoFormat TVFormat, long CurrentX, int VDelayAdj)
{
    int VertDelay;
    int CurrentY = GetTVFormat(TVFormat)->wCropHeight;

    if(IsCCIRSource(nInput))
    {
        VertDelay = 0x0C;

        WriteDword(CX2388X_VDELAYCCIR_EVEN, VertDelay);
        WriteDword(CX2388X_VDELAYCCIR_ODD, VertDelay);
    }
    else if(CurrentX == 720)
    {
        if (CurrentY == 576)
        {
            VertDelay = 0x24;
        }
        else
        {
            VertDelay = 0x1C;
        }
    }
    else
    {
        if (CurrentY == 576)
        {
            VertDelay = 0x24;
        }
        else
        {
            VertDelay = 0x1C;
        }
    }
    VertDelay += VDelayAdj;
    WriteDword(CX2388X_VERT_DELAY_EVEN, VertDelay);
    WriteDword(CX2388X_VERT_DELAY_ODD, VertDelay);
}

// Sets up card to support size and format requested
void CCX2388xCard::SetGeoSize(int nInput, eVideoFormat TVFormat, long& CurrentX, long& CurrentY, long& CurrentVBILines, BOOL IsProgressive)
{
    int HorzScale;

    CurrentY = GetTVFormat(TVFormat)->wCropHeight;

    // start with default values 
    // the only bit switched on is CFILT
    m_FilterDefault = (1 << 19);
    m_2HCombDefault = 0x181f0008;

    if(IsCCIRSource(nInput))
    {
        CurrentX = 720;
        CurrentVBILines = 0;

        WriteByte(CX2388X_PINMUX_IO, 0x02);

        // Since we are digital here we don't really care which
        // format we choose as long as it has the right number of lines
        DWORD VideoInput = ReadDword(CX2388X_VIDEO_INPUT);
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

        WriteDword(CX2388X_VIDEO_INPUT, VideoInput);

        WriteDword(CX2388X_FORMAT_2HCOMB, 0x183f0008);
        WriteDword(CX2388X_VIP_CONFIG, 1);

        if (CurrentY == 576)
        {
            HorzScale = 0x0504;
        }
        else
        {
            HorzScale = 0x00F8;
        }
    }
    // if the user requests 720 pixels
    // use 27Mhz sampling rate
    else if(CurrentX == 720)
    {
        DWORD HTotal(0);

        CurrentVBILines = GetTVFormat(TVFormat)->VBILines;

        DWORD VBIPackets;

        if (CurrentY == 576)
        {
            VBIPackets = (DWORD)(GetTVFormat(TVFormat)->VBIPacketSize * 27.0 / (8 * GetTVFormat(VIDEOFORMAT_PAL_B)->Fsc));
        }
        else
        {
            VBIPackets = (DWORD)(GetTVFormat(TVFormat)->VBIPacketSize * 27.0 / (28.636363));
        }

            // set up VBI information
        WriteDword(CX2388X_VBI_SIZE, (VBIPackets & 0x1ff));

        double PLL = SetPLL(27.0);
        SetSampleRateConverter(PLL);

        // Setup correct format
        DWORD VideoInput = ReadDword(CX2388X_VIDEO_INPUT);
        VideoInput &= 0xfffffff0;

        if(m_CX2388xCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_SVIDEO)
        {
            // set up with
            // Previous line remodulation - off
            // 3-d Comb filter - off
            // Comb Range - 00
            // Full Luma Range - on
            // PAL Invert Phase - off
            // Coring - off
            m_2HCombDefault = 0x08;

            // switch off luma notch
            // Luma notch is 1 = off
            m_FilterDefault |= CX2388X_FILTER_LNOTCH;
            // turn on just the chroma Comb Filter
            m_FilterDefault |= 1 << 5;
            // Disable luma dec
            m_FilterDefault |= 1 << 12;
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
            m_2HCombDefault = 0x181f0008;
        }

        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
            VideoInput |= VideoFormatPALBDGHI;
            HTotal = HLNotchFilter135PAL | 864;
            break;
        case VIDEOFORMAT_PAL_N:
            VideoInput |= VideoFormatPALN;
            HTotal = HLNotchFilter135PAL | 864;
            m_2HCombDefault |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_M:
            VideoInput |= VideoFormatPALM;
            HTotal = HLNotchFilter135NTSC | 858;
            m_2HCombDefault |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_60:
            VideoInput |= VideoFormatPAL60;
            HTotal = HLNotchFilter135NTSC | 858;
            m_2HCombDefault |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            VideoInput |= VideoFormatPALNC;
            HTotal = HLNotchFilter135PAL | 864;
            m_2HCombDefault |= (1 << 26);
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
            // test for Laurent
            // other stuff that may be required
            // Comments from Laurent
            // Bits 12, 16, and 18 must be set to 1 for SECAM
            // It seems to work even for PAL with these bits
            // TODO : check that they must be set for all the video formats
            // turn of QCIF filter as suggested by MidiMaker
            // QCIF HFilter
            //m_FilterDefault |= (1<<11);
            // 29 Tap first chroma demod
            m_FilterDefault |= (1<<15);
            // Third Chroma Demod - on
            m_FilterDefault |= (1<<17);
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

        WriteDword(CX2388X_VIDEO_INPUT, VideoInput);
        WriteDword(CX2388X_PIXEL_CNT_NOTCH, HTotal);
        WriteDword(CX2388X_FORMAT_2HCOMB, m_2HCombDefault);
        WriteDword(CX2388X_FILTER_EVEN, m_FilterDefault);
        WriteDword(CX2388X_FILTER_ODD, m_FilterDefault);

        // set up subcarrier frequency
        DWORD RegValue = (DWORD)(((8.0 * GetTVFormat(TVFormat)->Fsc) / PLL) * (double)(1<<22));
        WriteDword( CX2388X_SUBCARRIERSTEP, RegValue & 0x7FFFFF );
        // Subcarrier frequency Dr, for SECAM only but lets
        // set it anyway
        RegValue = (DWORD)((8.0 * 4.406250 / PLL) * (double)(1<<22));
        WriteDword( CX2388X_SUBCARRIERSTEPDR, RegValue);

        if(IsSECAMVideoFormat(TVFormat))
        {
            // set up burst gate delay and AGC gate delay
            RegValue = (DWORD)(6.8 * PLL / 2.0 + 15.5);
            // value suggested by midimaker
            RegValue |= 0xA000;
        }
        else
        {
            // set up burst gate delay and AGC gate delay
            RegValue = (DWORD)(6.8 * PLL / 2.0 + 15.5);
            RegValue |= (DWORD)(6.5 * PLL / 2.0 + 21.5) << 8;
        }
        WriteDword(CX2388X_AGC_BURST_DELAY, RegValue);
        HorzScale = 0x00;
    }
    // if not using 720 or a CCIR source then 
    // use 8*Fsc capture
    else
    {
        DWORD HTotal(0);
        DWORD RegValue;

        CurrentVBILines = GetTVFormat(TVFormat)->VBILines;

        // set up VBI information
        WriteDword(CX2388X_VBI_SIZE, GetTVFormat(TVFormat)->VBIPacketSize);

        double PLL;

        if(GetTVFormat(TVFormat)->NeedsPLL == TRUE)
        {
            PLL = SetPLL(GetTVFormat(VIDEOFORMAT_PAL_B)->Fsc * 8);

            // set up subcarrier frequency
            // Comments from Laurent
            // The sub carriers frequencies are wrong for SECAM
            // => use the values given by AMCAP
            if(TVFormat == VIDEOFORMAT_SECAM_L)
            {
                RegValue = 0x003d5985;
            }
            else
            {
                RegValue = (DWORD)(((8.0 * GetTVFormat(TVFormat)->Fsc) / PLL) * (double)(1<<22));
            }
            WriteDword( CX2388X_SUBCARRIERSTEP, RegValue & 0x7FFFFF );
            // Subcarrier frequency Dr, for SECAM only but lets
            // set it anyway
            if(TVFormat == VIDEOFORMAT_SECAM_L)
                RegValue = 0x003f9aee;
            else
                RegValue = (DWORD)((8.0 * 4.406250 / PLL) * (double)(1<<22));
            WriteDword( CX2388X_SUBCARRIERSTEPDR, RegValue);

            WriteDword( CX2388X_SAMPLERATECONV, 0x19D5F);
        }
        else
        {
            PLL = SetPLL(28.636363);

            // set up subcarrier frequency
            RegValue = (DWORD)(((8.0 * GetTVFormat(TVFormat)->Fsc) / PLL) * (double)(1<<22));
            WriteDword( CX2388X_SUBCARRIERSTEP, RegValue & 0x7FFFFF );
            // Subcarrier frequency Dr, for SECAM only but lets
            // set it anyway
            RegValue = (DWORD)((8.0 * 4.406250 / PLL) * (double)(1<<22));
            WriteDword( CX2388X_SUBCARRIERSTEPDR, RegValue);

            WriteDword( CX2388X_SAMPLERATECONV, 0x20000);
       }

        if(IsSECAMVideoFormat(TVFormat))
        {
            // set up burst gate delay and AGC gate delay
            RegValue = (DWORD)(6.8 * PLL / 2.0 + 15.5);
            // value suggested by midimaker
            RegValue |= 0xA000;
        }
        else
        {
            // set up burst gate delay and AGC gate delay
            RegValue = (DWORD)(6.8 * PLL / 2.0 + 15.5);
            RegValue |= (DWORD)(6.5 * PLL / 2.0 + 21.5) << 8;
        }
        WriteDword(CX2388X_AGC_BURST_DELAY, RegValue);

        // set up HorzScaling factor
        if(CurrentX <= GetTVFormat(TVFormat)->wHActivex1)
        {
            HorzScale = ((GetTVFormat(TVFormat)->wHActivex1 - CurrentX) * 4096UL) / CurrentX;
        }
        else
        {
            CurrentX = GetTVFormat(TVFormat)->wHActivex1;
            HorzScale = 0;
        }

        // Setup correct format
        DWORD VideoInput = ReadDword(CX2388X_VIDEO_INPUT);
        VideoInput &= 0xfffffff0;

        if(m_CX2388xCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_SVIDEO)
        {
            // set up with
            // Previous line remodulation - off
            // 3-d Comb filter - off
            // Comb Range - 00
            // Full Luma Range - on
            // PAL Invert Phase - off
            // Coring - off
            m_2HCombDefault = 0x08;

            // switch off luma notch
            // Luma notch is 1 = off
            m_FilterDefault |= CX2388X_FILTER_LNOTCH;
            // turn on just the chroma Comb Filter
            m_FilterDefault |= 1 << 5;
            // Disable luma dec
            m_FilterDefault |= 1 << 12;
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
            m_2HCombDefault = 0x181f0008;
        }

        // set up total width
        // this info is shared with the bt848 chip setup
        HTotal = GetTVFormat(TVFormat)->wTotalWidth;
        HTotal |= HLNotchFilter4xFsc;

        switch(TVFormat)
        {
        case VIDEOFORMAT_PAL_B:
        case VIDEOFORMAT_PAL_D:
        case VIDEOFORMAT_PAL_G:
        case VIDEOFORMAT_PAL_H:
        case VIDEOFORMAT_PAL_I:
            VideoInput |= VideoFormatPALBDGHI;
            break;
        case VIDEOFORMAT_PAL_N:
            VideoInput |= VideoFormatPALN;
            m_2HCombDefault |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_M:
            VideoInput |= VideoFormatPALM;
            m_2HCombDefault |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_60:
            VideoInput |= VideoFormatPAL60;
            m_2HCombDefault |= (1 << 26);
            break;
        case VIDEOFORMAT_PAL_N_COMBO:
            VideoInput |= VideoFormatPALNC;
            m_2HCombDefault |= (1 << 26);
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
            // test for Laurent
            // other stuff that may be required
            // Comments from Laurent
            // Bits 12, 16, and 18 must be set to 1 for SECAM
            // It seems to work even for PAL with these bits
            // TODO : check that they must be set for all the video formats
            // QCIF HFilter
            // turn of QCIF filter as suggested by MidiMaker
            //m_FilterDefault |= (1<<11);
            // 29 Tap first chroma demod
            m_FilterDefault |= (1<<15);
            // Third Chroma Demod - on
            m_FilterDefault |= (1<<17);
            break;
        case VIDEOFORMAT_NTSC_M:
            VideoInput |= VideoFormatNTSC;
            break;
        case VIDEOFORMAT_NTSC_M_Japan:
            VideoInput |= VideoFormatNTSCJapan;
            break;
        case VIDEOFORMAT_NTSC_50:
            VideoInput |= VideoFormatNTSC443;
            break;
        default:
            VideoInput |= VideoFormatAuto;
            break;
        }

        WriteDword(CX2388X_VIDEO_INPUT, VideoInput);
        WriteDword(CX2388X_PIXEL_CNT_NOTCH, HTotal);
        WriteDword(CX2388X_FORMAT_2HCOMB, m_2HCombDefault);
        WriteDword(CX2388X_FILTER_EVEN, m_FilterDefault);
        WriteDword(CX2388X_FILTER_ODD, m_FilterDefault);
    }

    WriteDword(CX2388X_HACTIVE_EVEN, CurrentX);
    WriteDword(CX2388X_HACTIVE_ODD, CurrentX);

    WriteDword(CX2388X_VACTIVE_EVEN, CurrentY);
    WriteDword(CX2388X_VACTIVE_ODD, CurrentY);

    WriteDword(CX2388X_HSCALE_EVEN, HorzScale);
    WriteDword(CX2388X_HSCALE_ODD, HorzScale);

    WriteDword(CX2388X_VSCALE_EVEN, 0);
    WriteDword(CX2388X_VSCALE_ODD, 0);

    // call any card specific format setup
    if(IsCurCardH3D())
    {
        H3DSetFormat(nInput, TVFormat, IsProgressive);
    }
    else
    {
        StandardSetFormat(nInput, TVFormat, IsProgressive);
    }
}


BOOL CCX2388xCard::IsCCIRSource(int nInput)
{
    if(nInput < m_CX2388xCards[m_CardType].NumInputs && nInput >= 0)
    {
        return (m_CX2388xCards[m_CardType].Inputs[nInput].InputType == INPUTTYPE_CCIR);
    }
    else
    {
        return FALSE;
    }
}

BOOL CCX2388xCard::IsVideoPresent()
{
    DWORD dwval			= ReadDword(CX2388X_DEVICE_STATUS);
	DWORD dwUseVSync	= ReadDword(CX2388X_VIDEO_INPUT);

	// "Vertical Sync Detection" in CX2388x Advanced Settings enabled?
	if((dwUseVSync & CX2388X_VIDEO_INPUT_VERTEN) == CX2388X_VIDEO_INPUT_VERTEN)
	{
		// detection is much faster
		return ((dwval & CX2388X_DEVICE_STATUS_VPRES) == CX2388X_DEVICE_STATUS_VPRES);
	}

	else
	{
		// use the old way
		return ((dwval & CX2388X_DEVICE_STATUS_HLOCK) == CX2388X_DEVICE_STATUS_HLOCK);
	}
}

DWORD CCX2388xCard::GetRISCPos()
{
    return ReadDword(CX2388X_VIDY_GP_CNT);
}

// question to other developers:
// the following code is not realy working.
// when stop WDM-driver on DS's start
// an restart on DS's exit bundled soft works correct.
// we should delete them.
void CCX2388xCard::ManageMyState()
{
    // This function doesn't seem to work properly for some
    // reason.  It causes crashes so don't bother trying.
    return;

    // save and restore everything that might be used
    // by the real drivers

    // save and restore the whole of the SRAM
    // as the drivers might well have stored code and fifo
    // buffers in there that we will overwrite
    for(DWORD i(0x180040); i < 0x187FFF; i += 4)
    {
        ManageDword(i);
    }

    // save and restore the registers than we overwrite
    ManageDword(MO_DMA21_PTR2);
    ManageDword(MO_DMA21_CNT1);
    ManageDword(MO_DMA21_CNT2);

    ManageDword(MO_DMA24_PTR2);
    ManageDword(MO_DMA24_CNT1);
    ManageDword(MO_DMA24_CNT2);
    
    ManageDword(MO_DMA25_PTR2);
    ManageDword(MO_DMA25_CNT1);
    ManageDword(MO_DMA25_CNT2);

    ManageDword(MO_DMA26_PTR2);
    ManageDword(MO_DMA26_CNT1);
    ManageDword(MO_DMA26_CNT2);

    // Drivers seem to reset most things when they run
    // so just return here especially now that we leave the chip
    // in a more shut down state
    return;


    // do these last just in case the chip was
    // left in a running mode
    ManageDword(CX2388X_DEV_CNTRL2); 
    ManageDword(CX2388X_VID_DMA_CNTRL);
    ManageDword(CX2388X_CAPTURECONTROL);
    
    ManageDword(CX2388X_VID_INTMSK);

    ManageDword(CX2388X_VIDEO_INPUT);
    ManageDword(CX2388X_TEMPORAL_DEC);
    ManageDword(CX2388X_AGC_BURST_DELAY);
    ManageDword(CX2388X_BRIGHT_CONTRAST); 
    ManageDword(CX2388X_UVSATURATION);    
    ManageDword(CX2388X_HUE);             
    ManageDword(CX2388X_WHITE_CRUSH);
    ManageDword(CX2388X_PIXEL_CNT_NOTCH);
    ManageDword(CX2388X_HORZ_DELAY_EVEN);
    ManageDword(CX2388X_HORZ_DELAY_ODD);
    ManageDword(CX2388X_VERT_DELAY_EVEN);
    ManageDword(CX2388X_VERT_DELAY_ODD);
    ManageDword(CX2388X_VDELAYCCIR_EVEN);
    ManageDword(CX2388X_VDELAYCCIR_ODD);
    ManageDword(CX2388X_HACTIVE_EVEN);
    ManageDword(CX2388X_HACTIVE_ODD);
    ManageDword(CX2388X_VACTIVE_EVEN);    
    ManageDword(CX2388X_VACTIVE_ODD);     
    ManageDword(CX2388X_HSCALE_EVEN);     
    ManageDword(CX2388X_HSCALE_ODD);      
    ManageDword(CX2388X_VSCALE_EVEN);     
    ManageDword(CX2388X_VSCALE_ODD);      
    ManageDword(CX2388X_FILTER_EVEN);     
    ManageDword(CX2388X_FILTER_ODD);      
    ManageDword(CX2388X_FORMAT_2HCOMB);
    //ManageDword(CX2388X_PLL);
    //ManageDword(CX2388X_PLL_ADJUST);
    ManageDword(CX2388X_VBI_SIZE);
    ManageDword(CX2388X_FIELD_CAP_CNT);
    ManageDword(CX2388X_VIP_CONFIG);
    ManageDword(CX2388X_VIP_CONTBRGT);
    ManageDword(CX2388X_VIP_HSCALE);
    ManageDword(CX2388X_VIP_VSCALE);
    
    ManageDword(CX2388X_VIDEO_COLOR_FORMAT);

    ManageDword(MO_PDMA_STHRSH);
    ManageDword(MO_PDMA_DTHRSH);

    ManageDword(CX2388X_AGC_SYNC_TIP1); 
    ManageDword(CX2388X_AGC_BACK_VBI); 

    ManageDword(CX2388X_SAMPLERATEFIFO);  
    ManageDword(CX2388X_SAMPLERATECONV);  
    ManageDword(CX2388X_SUBCARRIERSTEP);  
    ManageDword(CX2388X_SUBCARRIERSTEPDR);

    ManageDword(MO_GP0_IO);
    ManageDword(MO_GP1_IO);   
    ManageDword(MO_GP2_IO);
    ManageDword(MO_GP3_IO);
    ManageDword(MO_GPIO);
    ManageDword(MO_GPOE);
    ManageDword(MO_GP_ISM);
}

void CCX2388xCard::ResetChip()
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
            LOG(1, " CX2388x PCI Command was %d", Command);
            Command |= 0x06;
            SetPCIConfigOffset(&Command, 0x04, m_BusNumber, m_SlotNumber);
            ::Sleep(500);
        }
    }

    if(GetPCIConfigOffset(&Command, 0x40, m_BusNumber, m_SlotNumber))
    {
        // switch on allow master and respond to memory requests
        if((Command & 0x01) == 0x01)
        {
            LOG(1, " CX2388x eeprom write were enabled 0x40 Register was %d", Command);
            Command &= ~0x01;
            SetPCIConfigOffset(&Command, 0x40, m_BusNumber, m_SlotNumber);
        }
    }

    // \todo log what's there already for key settings

    // Firstly stop the card from doing anything
    // so stop the risc controller
    WriteDword(CX2388X_DEV_CNTRL2, 0x00000000);
    // then stop any DMA transfers
    WriteDword(MO_VID_DMACNTRL, 0x00000000);
    WriteDword(MO_AUD_DMACNTRL, 0x00000000);
    WriteDword(MO_TS_DMACNTRL, 0x00000000);
    WriteDword(MO_VIP_DMACNTRL, 0x00000000);
    WriteDword(MO_GPHST_DMACNTRL, 0x00000000);

    // secondly stop any interupts from happening
    // if we change something and let an 
    // interupt happen than the driver might try and
    // do something bad
    WriteDword( CX2388X_PCI_INTMSK, 0x00000000 );
    WriteDword( CX2388X_VID_INTMSK, 0x00000000 );
    WriteDword( CX2388X_AUD_INTMSK, 0x00000000 );
    WriteDword( CX2388X_TS_INTMSK, 0x00000000 );
    WriteDword( CX2388X_VIP_INTMSK, 0x00000000 );
    WriteDword( CX2388X_GPHST_INTMSK, 0x00000000 );

    WriteDword( CX2388X_VID_INTSTAT, 0xFFFFFFFF ); // Clear PIV int
    WriteDword( CX2388X_PCI_INTSTAT, 0xFFFFFFFF ); // Clear PCI int
    WriteDword( MO_INT1_STAT, 0xFFFFFFFF );        // Clear RISC int

    // wait a bit so that everything has cleared through
    ::Sleep(500);
}

void CCX2388xCard::ResetHardware()
{
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

    // Reset the chip (note sure about this)
    //WriteDword( 0x310304, 0x1 );
    //::Sleep(500);

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
    // Setup for Audio Input
    // Note that the cluster table and buffers are shared
    // between the input and output.
    // This setup will work where the sound is being decoded and
    // sent to the internal DAC for routing via an analog cable
    // If capture of audio is required I'd guess that this
    // will need to be changed and some RISC code will be required.
    /////////////////////////////////////////////////////////////////
        
    // Cluster table base 
    WriteDword(SRAM_CMDS_25 + 0x04, SRAM_CLUSTER_TABLE_AUDIO); 

    // Cluster table size is in QWORDS
    WriteDword(SRAM_CMDS_25 + 0x08, (SRAM_CLUSTER_TABLE_AUDIO_SIZE / 8));

    // Fill in cluster buffer entries
    for(i = 0; i < SRAM_AUDIO_BUFFERS; ++i)
    {
        WriteDword(
                    SRAM_CLUSTER_TABLE_AUDIO + (i * 0x10), 
                    SRAM_FIFO_AUDIO_BUFFERS + (i * SRAM_FIFO_AUDIO_BUFFER_SIZE)
                  );
    }
    
    // Copy the cluster buffer info to the DMAC 
    
    // Set the DMA Cluster Table Address
    WriteDword( MO_DMA25_PTR2, SRAM_CLUSTER_TABLE_AUDIO);
    
    // Set the DMA buffer limit size in qwords
    WriteDword( MO_DMA25_CNT1, SRAM_FIFO_AUDIO_BUFFER_SIZE / 8);
    
    // Set the DMA Cluster Table Size in qwords
    WriteDword( MO_DMA25_CNT2, (SRAM_CLUSTER_TABLE_AUDIO_SIZE / 8));

    /////////////////////////////////////////////////////////////////
    // Setup for Audio Output
    /////////////////////////////////////////////////////////////////
        
    // Cluster table base 
    WriteDword(SRAM_CMDS_26 + 0x04, SRAM_CLUSTER_TABLE_AUDIO); 

    // Cluster table size is in QWORDS
    WriteDword(SRAM_CMDS_26 + 0x08, (SRAM_CLUSTER_TABLE_AUDIO_SIZE / 8));

    // cluster buffer entries already filled as shared with audio input

    // Copy the cluster buffer info to the DMAC 
    
    // Set the DMA Cluster Table Address
    WriteDword( MO_DMA26_PTR2, SRAM_CLUSTER_TABLE_AUDIO);
    
    // Set the DMA buffer limit size in qwords
    WriteDword( MO_DMA26_CNT1, SRAM_FIFO_AUDIO_BUFFER_SIZE / 8);
    
    // Set the DMA Cluster Table Size in qwords
    WriteDword( MO_DMA26_CNT2, (SRAM_CLUSTER_TABLE_AUDIO_SIZE / 8));

    /////////////////////////////////////////////////////////////////
    // Other one off settings for the chip
    /////////////////////////////////////////////////////////////////

    // set format to YUY2
    MaskDataDword(CX2388X_VIDEO_COLOR_FORMAT, 0x00000044, 0x000000FF);

    // Test from Mike Asbury's regtool init code
    WriteDword( MO_PDMA_STHRSH, 0x0807 ); // Fifo source Threshhold
    WriteDword( MO_PDMA_DTHRSH, 0x0807 ); // Fifo Threshhold

    WriteDword( CX2388X_VID_INTSTAT, 0xFFFFFFFF ); // Clear PIV int
    WriteDword( CX2388X_PCI_INTSTAT, 0xFFFFFFFF ); // Clear PCI int
    WriteDword( MO_INT1_STAT, 0xFFFFFFFF );   // Clear RISC int

    //
    // Fixes for flashing suggested by Ben Felts
    //
    // 1.  Set bits 16:9 of register 0xE4310208 to 0x00.  
    //     The default value is 0x03803C0F, which becomes 0x0380000F with this change.
    WriteDword( CX2388X_AGC_SYNC_TIP1, 0x0380000F ); 

    //2.  Set bits 27:26 of register 0xE4310200 to 0x0.  The default value is
    //    0x0CE00555, which becomes 0x00E00555 with this change.
    WriteDword( CX2388X_AGC_BACK_VBI, 0x00E00555 ); 
}    

BOOL APIENTRY CCX2388xCard::ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    CCX2388xCard* pThis = NULL;
    char szCardId[9] = "n/a     ";
    char szVendorId[9] = "n/a ";
    char szDeviceId[9] = "n/a ";
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CCX2388xCard*)lParam; 
        SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, "CX2388x");
        sprintf(szVendorId,"%04X", pThis->GetVendorId());
        SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);
        sprintf(szDeviceId,"%04X", pThis->GetDeviceId());
        SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);
        SetDlgItemText(hDlg, IDC_TUNER_TYPE, pThis->GetTunerType());
        SetDlgItemText(hDlg, IDC_AUDIO_DECODER_TYPE, "");
        dwCardId = pThis->GetSubSystemId();
        if(dwCardId != 0 && dwCardId != 0xffffffff)
        {
            sprintf(szCardId,"%8X", dwCardId);
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

ULONG CCX2388xCard::GetTickCount()
// an overflow happens after 21 days uptime on a 10GHz machine
{
    ULONGLONG ticks;
    ULONGLONG frequency;

    QueryPerformanceFrequency((PLARGE_INTEGER)&frequency);
    QueryPerformanceCounter((PLARGE_INTEGER)&ticks);

	ticks = ticks * 1000 / frequency;
    return (ULONG)ticks;
}

void CCX2388xCard::InitializeI2C()
{
    WriteDword(CX2388X_I2C, 1);
    m_I2CRegister = ReadDword(CX2388X_I2C);

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

void CCX2388xCard::Sleep()
{
    for (volatile DWORD i = m_I2CSleepCycle; i > 0; i--);
}

void CCX2388xCard::SetSDA(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        m_I2CRegister |= CX2388X_I2C_SDA;
    }
    else
    {
        m_I2CRegister &= ~CX2388X_I2C_SDA;
    }
    WriteDword(CX2388X_I2C, m_I2CRegister);
}

void CCX2388xCard::SetSCL(bool value)
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    if (value)
    {
        m_I2CRegister |= CX2388X_I2C_SCL;
    }
    else
    {
        m_I2CRegister &= ~CX2388X_I2C_SCL;
    }
    WriteDword(CX2388X_I2C, m_I2CRegister);
}

bool CCX2388xCard::GetSDA()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    bool state = ReadDword(CX2388X_I2C) & CX2388X_I2C_SDA ? true : false;
    return state;
}

bool CCX2388xCard::GetSCL()
{
    if (!m_I2CInitialized)
    {
        InitializeI2C();
    }
    bool state = ReadDword(CX2388X_I2C) & CX2388X_I2C_SCL ? true : false;
    return state;
}

void CCX2388xCard::I2CLock()
{
    LockCard();
}

void CCX2388xCard::I2CUnlock()
{
    UnlockCard();
}


/*******************************************************************************
    Program the PLL to a specific output frequency.
    Assume that we have a PLL pre dividor of 2
*******************************************************************************/
double CCX2388xCard::SetPLL(double PLLFreq)
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
        return 0.0;
    }
    
    // Set register int and fraction values
    RegValue |= PLLInt << 20;
    RegValue |= PLLFraction & 0xFFFFF;
    
    WriteDword(CX2388X_PLL , RegValue );

    return (28.63636 / (8.0 * (double)Prescaler)) * ((double)PLLInt + (double)PLLFraction / (double)(1 << 20));
}


void CCX2388xCard::SetSampleRateConverter(double PLLFreq)
{
    DWORD RegValue = (DWORD)((28.63636 / PLLFreq) * (double)(1<<17));
    WriteDword( CX2388X_SAMPLERATECONV, RegValue & 0x7FFFF );
}

void CCX2388xCard::SetRISCStartAddress(DWORD RiscBasePhysical)
{
    WriteDword( SRAM_CMDS_21, RiscBasePhysical); // RISC STARTING ADDRESS

    // Set as PCI address
    AndDataDword( SRAM_CMDS_21 + 0x10, 0x7fffffff); 
}

void CCX2388xCard::SetRISCStartAddressVBI(DWORD RiscBasePhysical)
{
    WriteDword( SRAM_CMDS_24, RiscBasePhysical); // RISC STARTING ADDRESS

    // Set as PCI address
    AndDataDword( SRAM_CMDS_24 + 0x10, 0x7fffffff); 
}

#define DumpRegister(Reg) fprintf(hFile, #Reg "\t%08x\n", ReadDword(Reg))
#define DumpBRegister(Reg) fprintf(hFile, #Reg "\t%02x\n", ReadByte(Reg))

void CCX2388xCard::DumpChipStatus(const char* CardName)
{
    FILE* hFile;
    char Filename[256];

    sprintf(Filename, "%s.txt", CardName);

    hFile = fopen(Filename, "w");
    if(!hFile)
    {
        return;
    }

    fprintf(hFile, "SubSystemId\t%08x\n", m_SubSystemId);

    DumpRegister(CX2388X_DEVICE_STATUS);
    DumpRegister(CX2388X_VIDEO_INPUT);
    DumpRegister(CX2388X_TEMPORAL_DEC);
    DumpRegister(CX2388X_AGC_BURST_DELAY);
    DumpRegister(CX2388X_BRIGHT_CONTRAST); 
    DumpRegister(CX2388X_UVSATURATION);    
    DumpRegister(CX2388X_HUE);             
    DumpRegister(CX2388X_WHITE_CRUSH);
    DumpRegister(CX2388X_PIXEL_CNT_NOTCH);
    DumpRegister(CX2388X_HORZ_DELAY_EVEN);
    DumpRegister(CX2388X_HORZ_DELAY_ODD);
    DumpRegister(CX2388X_VERT_DELAY_EVEN);
    DumpRegister(CX2388X_VERT_DELAY_ODD);
    DumpRegister(CX2388X_VDELAYCCIR_EVEN);
    DumpRegister(CX2388X_VDELAYCCIR_ODD);
    DumpRegister(CX2388X_HACTIVE_EVEN);
    DumpRegister(CX2388X_HACTIVE_ODD);
    DumpRegister(CX2388X_VACTIVE_EVEN);    
    DumpRegister(CX2388X_VACTIVE_ODD);     
    DumpRegister(CX2388X_HSCALE_EVEN);     
    DumpRegister(CX2388X_HSCALE_ODD);      
    DumpRegister(CX2388X_VSCALE_EVEN);     
    DumpRegister(CX2388X_VSCALE_ODD);      
    DumpRegister(CX2388X_FILTER_EVEN);     
    DumpRegister(CX2388X_FILTER_ODD);      
    DumpRegister(CX2388X_FORMAT_2HCOMB);
    DumpRegister(CX2388X_PLL);
    DumpRegister(CX2388X_PLL_ADJUST);
    DumpRegister(CX2388X_SAMPLERATECONV);  
    DumpRegister(CX2388X_SAMPLERATEFIFO);  
    DumpRegister(CX2388X_SUBCARRIERSTEP);  
    DumpRegister(CX2388X_SUBCARRIERSTEPDR);
    DumpRegister(CX2388X_CAPTURECONTROL);  
    DumpRegister(CX2388X_VIDEO_COLOR_FORMAT);
    DumpRegister(CX2388X_VBI_SIZE);
    DumpRegister(CX2388X_FIELD_CAP_CNT);
    DumpRegister(CX2388X_VIP_CONFIG);
    DumpRegister(CX2388X_VIP_CONTBRGT);
    DumpRegister(CX2388X_VIP_HSCALE);
    DumpRegister(CX2388X_VIP_VSCALE);
    DumpRegister(CX2388X_VBOS);

    DumpRegister(MO_GP0_IO);
    DumpRegister(MO_GP1_IO);   
    DumpRegister(MO_GP2_IO);
    DumpRegister(MO_GP3_IO);
    DumpRegister(MO_GPIO);
    DumpRegister(MO_GPOE);

    DumpBRegister(0x320d01);
    DumpBRegister(0x320d02);
    DumpBRegister(0x320d03);
    DumpBRegister(0x320d04);
    DumpBRegister(0x320d2a);
    DumpBRegister(0x320d2b);

    DumpRegister(AUD_DN0_FREQ);
    DumpRegister(AUD_POLY0_DDS_CONSTANT);
    DumpRegister(AUD_IIR1_0_SEL);
    DumpRegister(AUD_IIR1_1_SEL);
    DumpRegister(AUD_IIR1_2_SEL);
    DumpRegister(AUD_IIR1_3_SEL);
    DumpRegister(AUD_IIR1_4_SEL);
    DumpRegister(AUD_IIR1_5_SEL);
    DumpRegister(AUD_IIR1_0_SHIFT);
    DumpRegister(AUD_IIR1_1_SHIFT);
    DumpRegister(AUD_IIR1_2_SHIFT);
    DumpRegister(AUD_IIR1_3_SHIFT);
    DumpRegister(AUD_IIR1_4_SHIFT);
    DumpRegister(AUD_IIR1_5_SHIFT);
    DumpRegister(AUD_IIR2_0_SEL);
    DumpRegister(AUD_IIR2_1_SEL);
    DumpRegister(AUD_IIR2_2_SEL);
    DumpRegister(AUD_IIR2_3_SEL);
    DumpRegister(AUD_IIR3_0_SEL);
    DumpRegister(AUD_IIR3_1_SEL);
    DumpRegister(AUD_IIR3_2_SEL);
    DumpRegister(AUD_IIR3_0_SHIFT);
    DumpRegister(AUD_IIR3_1_SHIFT);
    DumpRegister(AUD_IIR3_2_SHIFT);
    DumpRegister(AUD_IIR4_0_SEL);
    DumpRegister(AUD_IIR4_1_SEL);
    DumpRegister(AUD_IIR4_2_SEL);
    DumpRegister(AUD_IIR4_0_SHIFT);
    DumpRegister(AUD_IIR4_1_SHIFT);
    DumpRegister(AUD_IIR4_2_SHIFT);
    DumpRegister(AUD_IIR4_0_CA0);
    DumpRegister(AUD_IIR4_0_CA1);
    DumpRegister(AUD_IIR4_0_CA2);
    DumpRegister(AUD_IIR4_0_CB0);
    DumpRegister(AUD_IIR4_0_CB1);
    DumpRegister(AUD_IIR4_1_CA0);
    DumpRegister(AUD_IIR4_1_CA1);
    DumpRegister(AUD_IIR4_1_CA2);
    DumpRegister(AUD_IIR4_1_CB0);
    DumpRegister(AUD_IIR4_1_CB1);
    DumpRegister(AUD_IIR4_2_CA0);
    DumpRegister(AUD_IIR4_2_CA1);
    DumpRegister(AUD_IIR4_2_CA2);
    DumpRegister(AUD_IIR4_2_CB0);
    DumpRegister(AUD_IIR4_2_CB1);
    DumpRegister(AUD_HP_MD_IIR4_1);
    DumpRegister(AUD_HP_PROG_IIR4_1);
    DumpRegister(AUD_DN1_FREQ);
    DumpRegister(AUD_DN1_SRC_SEL);
    DumpRegister(AUD_DN1_SHFT);
    DumpRegister(AUD_DN1_AFC);
    DumpRegister(AUD_DN1_FREQ_SHIFT);
    DumpRegister(AUD_DN2_SRC_SEL);
    DumpRegister(AUD_DN2_SHFT);
    DumpRegister(AUD_DN2_AFC);
    DumpRegister(AUD_DN2_FREQ);
    DumpRegister(AUD_DN2_FREQ_SHIFT);
    DumpRegister(AUD_PDET_SRC);
    DumpRegister(AUD_PDET_SHIFT);
    DumpRegister(AUD_DEEMPH0_SRC_SEL);
    DumpRegister(AUD_DEEMPH1_SRC_SEL);
    DumpRegister(AUD_DEEMPH0_SHIFT);
    DumpRegister(AUD_DEEMPH1_SHIFT);
    DumpRegister(AUD_DEEMPH0_G0);
    DumpRegister(AUD_DEEMPH0_A0);
    DumpRegister(AUD_DEEMPH0_B0);
    DumpRegister(AUD_DEEMPH0_A1);
    DumpRegister(AUD_DEEMPH0_B1);
    DumpRegister(AUD_DEEMPH1_G0);
    DumpRegister(AUD_DEEMPH1_A0);
    DumpRegister(AUD_DEEMPH1_B0);
    DumpRegister(AUD_DEEMPH1_A1);
    DumpRegister(AUD_DEEMPH1_B1);
    DumpRegister(AUD_PLL_EN);
    DumpRegister(AUD_DMD_RA_DDS);
    DumpRegister(AUD_RATE_ADJ1);
    DumpRegister(AUD_RATE_ADJ2);
    DumpRegister(AUD_RATE_ADJ3);
    DumpRegister(AUD_RATE_ADJ4);
    DumpRegister(AUD_RATE_ADJ5);
    DumpRegister(AUD_C2_UP_THR);
    DumpRegister(AUD_C2_LO_THR);
    DumpRegister(AUD_C1_UP_THR);
    DumpRegister(AUD_C1_LO_THR);
    DumpRegister(AUD_CTL);
    DumpRegister(AUD_DCOC_0_SRC);
    DumpRegister(AUD_DCOC_1_SRC);
    DumpRegister(AUD_DCOC1_SHIFT);
    DumpRegister(AUD_DCOC_1_SHIFT_IN0);
    DumpRegister(AUD_DCOC_1_SHIFT_IN1);
    DumpRegister(AUD_DCOC_PASS_IN);
    DumpRegister(AUD_IIR4_0_SEL);
    DumpRegister(AUD_DN1_AFC);
    DumpRegister(AUD_DCOC_2_SRC);
    DumpRegister(AUD_IIR4_1_SEL);
    DumpRegister(AUD_CTL);
    DumpRegister(AUD_DN2_SRC_SEL);
    DumpRegister(AUD_DN2_FREQ);
    DumpRegister(AUD_POLY0_DDS_CONSTANT);
    DumpRegister(AUD_PHASE_FIX_CTL);
    DumpRegister(AUD_CORDIC_SHIFT_1);

    // setup Audio PLL
    DumpRegister(AUD_PLL_PRESCALE);
    DumpRegister(AUD_PLL_INT);


    fclose(hFile);
}

void CCX2388xCard::ShowRegisterSettingsDialog(HINSTANCE hCX2388xResourceInst)
{
    DialogBoxParam(hCX2388xResourceInst, "REGISTEREDIT", GetMainWnd(), RegisterEditProc, (LPARAM)this);
}

#define AddRegister(Reg) {long Index = ComboBox_AddString(GetDlgItem(hDlg, IDC_REGISTERSELECT), #Reg); ComboBox_SetItemData(GetDlgItem(hDlg, IDC_REGISTERSELECT), Index, Reg);}

BOOL APIENTRY CCX2388xCard::RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static CCX2388xCard* pThis;
    static DWORD dwAddress;
    static DWORD dwRegValue;

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CCX2388xCard*)lParam;
        SendMessage(GetDlgItem(hDlg, IDC_REGISTERSELECT), CB_RESETCONTENT, 0, 0);
        
        AddRegister(CX2388X_DEVICE_STATUS);
        AddRegister(CX2388X_VIDEO_INPUT);
        AddRegister(CX2388X_TEMPORAL_DEC);
        AddRegister(CX2388X_AGC_BURST_DELAY);
        AddRegister(CX2388X_BRIGHT_CONTRAST); 
        AddRegister(CX2388X_UVSATURATION);    
        AddRegister(CX2388X_HUE);             
        AddRegister(CX2388X_WHITE_CRUSH);
        AddRegister(CX2388X_PIXEL_CNT_NOTCH);
        AddRegister(CX2388X_HORZ_DELAY_EVEN);
        AddRegister(CX2388X_HORZ_DELAY_ODD);
        AddRegister(CX2388X_VERT_DELAY_EVEN);
        AddRegister(CX2388X_VERT_DELAY_ODD);
        AddRegister(CX2388X_VDELAYCCIR_EVEN);
        AddRegister(CX2388X_VDELAYCCIR_ODD);
        AddRegister(CX2388X_HACTIVE_EVEN);
        AddRegister(CX2388X_HACTIVE_ODD);
        AddRegister(CX2388X_VACTIVE_EVEN);    
        AddRegister(CX2388X_VACTIVE_ODD);     
        AddRegister(CX2388X_HSCALE_EVEN);     
        AddRegister(CX2388X_HSCALE_ODD);      
        AddRegister(CX2388X_VSCALE_EVEN);     
        AddRegister(CX2388X_VSCALE_ODD);      
        AddRegister(CX2388X_FILTER_EVEN);     
        AddRegister(CX2388X_FILTER_ODD);      
        AddRegister(CX2388X_FORMAT_2HCOMB);
        AddRegister(CX2388X_PLL);
        AddRegister(CX2388X_PLL_ADJUST);
        AddRegister(CX2388X_SAMPLERATECONV);  
        AddRegister(CX2388X_SAMPLERATEFIFO);  
        AddRegister(CX2388X_SUBCARRIERSTEP);  
        AddRegister(CX2388X_SUBCARRIERSTEPDR);
        AddRegister(CX2388X_CAPTURECONTROL);  
        AddRegister(CX2388X_VIDEO_COLOR_FORMAT);
        AddRegister(CX2388X_VBI_SIZE);
        AddRegister(CX2388X_FIELD_CAP_CNT);
        AddRegister(CX2388X_VIP_CONFIG);
        AddRegister(CX2388X_VIP_CONTBRGT);
        AddRegister(CX2388X_VIP_HSCALE);
        AddRegister(CX2388X_VIP_VSCALE);
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
        case IDC_BIT8:
        case IDC_BIT9:
        case IDC_BIT10:
        case IDC_BIT11:
        case IDC_BIT12:
        case IDC_BIT13:
        case IDC_BIT14:
        case IDC_BIT15:
        case IDC_BIT16:
        case IDC_BIT17:
        case IDC_BIT18:
        case IDC_BIT19:
        case IDC_BIT20:
        case IDC_BIT21:
        case IDC_BIT22:
        case IDC_BIT23:
        case IDC_BIT24:
        case IDC_BIT25:
        case IDC_BIT26:
        case IDC_BIT27:
        case IDC_BIT28:
        case IDC_BIT29:
        case IDC_BIT30:
        case IDC_BIT31:
            if(Button_GetCheck(GetDlgItem(hDlg, LOWORD(wParam))) == BST_CHECKED)
            {
                dwRegValue |= 1 << (LOWORD(wParam) - IDC_BIT0);
            }
            else
            {
                dwRegValue &= ~(1 << (LOWORD(wParam) - IDC_BIT0));
            }
            pThis->WriteDword(dwAddress, dwRegValue);
            break;

        case IDC_REGISTERSELECT:
            if(HIWORD(wParam) == LBN_SELCHANGE)
            {
                long Index = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_REGISTERSELECT));
                if(Index != -1)
                {
                    dwAddress = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_REGISTERSELECT), Index);
                    dwRegValue = pThis->ReadDword(dwAddress);
                    DWORD TempRegValue(dwRegValue);
                    for(int i(0); i < 32; ++i)
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

