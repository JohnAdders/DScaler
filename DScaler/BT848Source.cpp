/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source.cpp,v 1.18 2001-12-22 13:18:04 adcockj Exp $
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
// Revision 1.17  2001/12/19 19:24:45  ittarnavsky
// prepended SOUNDCHANNEL_ to all members of the eSoundChannel enum
//
// Revision 1.16  2001/12/18 13:12:11  adcockj
// Interim check-in for redesign of card specific settings
//
// Revision 1.15  2001/12/16 10:14:16  laurentg
// Calculation of used fields restored
//
// Revision 1.14  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.13  2001/12/03 19:33:59  adcockj
// Bug fixes for settings and memory
//
// Revision 1.12  2001/12/03 17:27:56  adcockj
// SECAM NICAM patch from Quenotte
//
// Revision 1.11  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.10  2001/11/29 14:04:06  adcockj
// Added Javadoc comments
//
// Revision 1.9  2001/11/25 01:58:34  ittarnavsky
// initial checkin of the new I2C code
//
// Revision 1.8  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.7  2001/11/22 22:27:00  adcockj
// Bug Fixes
//
// Revision 1.6  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.5  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.4  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.11  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.10  2001/08/22 18:38:31  adcockj
// Fixed Recursive bug
//
// Revision 1.1.2.9  2001/08/22 11:12:48  adcockj
// Added VBI support
//
// Revision 1.1.2.8  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.1.2.7  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.6  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.5  2001/08/19 14:43:47  adcockj
// Fixed memory leaks
//
// Revision 1.1.2.4  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.3  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.2  2001/08/16 06:43:34  adcockj
// moved more stuff into the new file (deonsn't compile)
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "BT848Source.h"
#include "DScaler.h"
#include "VBI.h"
#include "VBI_VideoText.h"
#include "Audio.h"
#include "VideoSettings.h"
#include "OutThreads.h"
#include "OSD.h"
#include "Status.h"
#include "FieldTiming.h"
#include "ProgramList.h"
#include "BT848_Defines.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "DebugLog.h"



CBT848Source::CBT848Source(CBT848Card* pBT848Card, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection) :
    CSource(WM_BT848_GETVALUE, IDC_BT848),
    m_pBT848Card(pBT848Card),
    m_CurrentX(720),
    m_CurrentY(480),
    m_CurrentVBILines(19),
    m_Section(IniSection),
    m_IsFieldOdd(FALSE)
{
    CreateSettings(IniSection);

    ReadFromIni();
    ChangeSectionNamesForInput();
    ChangeDefaultsForInput();
    LoadInputSettings();

    m_RiscBaseLinear = (DWORD*)RiscDMAMem->GetUserPointer();
    m_RiscBasePhysical = RiscDMAMem->TranslateToPhysical(m_RiscBaseLinear, 83968, NULL);
    for(int i(0); i < 5; ++i)
    {
        m_pDisplay[i] = (BYTE*)DisplayDMAMem[i]->GetUserPointer();
        m_DisplayDMAMem[i] = DisplayDMAMem[i];
        m_pVBILines[i] = (BYTE*)VBIDMAMem[i]->GetUserPointer();
        m_VBIDMAMem[i] = VBIDMAMem[i];
    }

    // Set up 5 sets of pointers to the start of odd and even lines
    for (int j(0); j < 5; j++)
    {
        m_OddFields[j].pData = m_pDisplay[j] + 2048;
        m_OddFields[j].Flags = PICTURE_INTERLACED_ODD;
        m_OddFields[j].IsFirstInSeries = FALSE;
        m_EvenFields[j].pData = m_pDisplay[j];
        m_EvenFields[j].Flags = PICTURE_INTERLACED_EVEN;
        m_EvenFields[j].IsFirstInSeries = FALSE;
    }
    SetupCard();
    if(pBT848Card->HasMSP())
    {
        SetTimer(hWnd, TIMER_MSP, TIMER_MSP_MS, NULL);
    }
    Reset();
}

CBT848Source::~CBT848Source()
{
    KillTimer(hWnd, TIMER_MSP);
    delete m_pBT848Card;
}


void CBT848Source::CreateSettings(LPCSTR IniSection)
{
    m_Brightness = new CBrightnessSetting(this, "Brightness", DEFAULT_BRIGHTNESS_NTSC, -128, 127, IniSection);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", DEFAULT_CONTRAST_NTSC, 0, 511, IniSection);
    m_Settings.push_back(m_Contrast);

    m_Hue = new CHueSetting(this, "Hue", DEFAULT_HUE_NTSC, -128, 127, IniSection);
    m_Settings.push_back(m_Hue);

    m_Saturation = new CSaturationSetting(this, "Saturation", (DEFAULT_SAT_V_NTSC + DEFAULT_SAT_U_NTSC) / 2, 0, 511, IniSection);
    m_Settings.push_back(m_Saturation);

    m_SaturationU = new CSaturationUSetting(this, "Blue Saturation", DEFAULT_SAT_U_NTSC, 0, 511, IniSection);
    m_Settings.push_back(m_SaturationU);

    m_SaturationV = new CSaturationVSetting(this, "Red Saturation", DEFAULT_SAT_V_NTSC, 0, 511, IniSection);
    m_Settings.push_back(m_SaturationV);

    m_BDelay = new CBDelaySetting(this, "Macrovision Timing", 0, 0, 255, IniSection);
    m_Settings.push_back(m_BDelay);

    m_BtAgcDisable = new CBtAgcDisableSetting(this, "AGC Disable", FALSE, IniSection);
    m_Settings.push_back(m_BtAgcDisable);

    m_BtCrush = new CBtCrushSetting(this, "Crush", TRUE, IniSection);
    m_Settings.push_back(m_BtCrush);

    m_BtEvenChromaAGC = new CBtEvenChromaAGCSetting(this, "Even Chroma AGC", TRUE, IniSection);
    m_Settings.push_back(m_BtEvenChromaAGC);

    m_BtOddChromaAGC = new CBtOddChromaAGCSetting(this, "Odd Chroma AGC", TRUE, IniSection);
    m_Settings.push_back(m_BtOddChromaAGC);

    m_BtEvenLumaPeak = new CBtEvenLumaPeakSetting(this, "Even Luma Peak", FALSE, IniSection);
    m_Settings.push_back(m_BtEvenLumaPeak);

    m_BtOddLumaPeak = new CBtOddLumaPeakSetting(this, "Odd Luma Peak", FALSE, IniSection);
    m_Settings.push_back(m_BtOddLumaPeak);

    m_BtFullLumaRange = new CBtFullLumaRangeSetting(this, "Full Luma Range", TRUE, IniSection);
    m_Settings.push_back(m_BtFullLumaRange);

    m_BtEvenLumaDec = new CBtEvenLumaDecSetting(this, "Even Luma Dec", FALSE, IniSection);
    m_Settings.push_back(m_BtEvenLumaDec);

    m_BtOddLumaDec = new CBtOddLumaDecSetting(this, "Odd Luma Dec", FALSE, IniSection);
    m_Settings.push_back(m_BtOddLumaDec);

    m_BtEvenComb = new CBtEvenCombSetting(this, "Even Comb", TRUE, IniSection);
    m_Settings.push_back(m_BtEvenComb);

    m_BtOddComb = new CBtOddCombSetting(this, "Odd Comb", TRUE, IniSection);
    m_Settings.push_back(m_BtOddComb);

    m_BtColorBars = new CBtColorBarsSetting(this, "Color Bars", FALSE, IniSection);
    m_Settings.push_back(m_BtColorBars);

    m_BtGammaCorrection = new CBtGammaCorrectionSetting(this, "Gamma Correction", FALSE, IniSection);
    m_Settings.push_back(m_BtGammaCorrection);

    m_BtCoring = new CBtCoringSetting(this, "Coring", FALSE, IniSection);
    m_Settings.push_back(m_BtCoring);

    m_BtHorFilter = new CBtHorFilterSetting(this, "Horizontal Filter", FALSE, IniSection);
    m_Settings.push_back(m_BtHorFilter);

    m_BtVertFilter = new CBtVertFilterSetting(this, "Vertical Filter", FALSE, IniSection);
    m_Settings.push_back(m_BtVertFilter);

    m_BtColorKill = new CBtColorKillSetting(this, "Color Kill", FALSE, IniSection);
    m_Settings.push_back(m_BtColorKill);

    m_BtWhiteCrushUp = new CBtWhiteCrushUpSetting(this, "White Crush Upper", 0xCF, 0, 255, IniSection);
    m_Settings.push_back(m_BtWhiteCrushUp);

    m_BtWhiteCrushDown = new CBtWhiteCrushDownSetting(this, "White Crush Lower", 0x7F, 0, 255, IniSection);
    m_Settings.push_back(m_BtWhiteCrushDown);

    m_PixelWidth = new CPixelWidthSetting(this, "Sharpness", 720, 120, DSCALER_MAX_WIDTH, IniSection);
    m_PixelWidth->SetStepValue(2);
    m_Settings.push_back(m_PixelWidth);

    m_CustomPixelWidth = new CSliderSetting("Custom Pixel Width", 750, 120, DSCALER_MAX_WIDTH, IniSection, "CustomPixelWidth");
    m_CustomPixelWidth->SetStepValue(2);
    m_Settings.push_back(m_CustomPixelWidth);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 6, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection);
    m_Settings.push_back(m_VideoFormat);

    m_HDelay = new CHDelaySetting(this, "Horzontal Delay", 0, 0, 255, IniSection);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay", 0, 0, 255, IniSection);
    m_VDelay->SetStepValue(2);
    m_Settings.push_back(m_VDelay);

    m_ReversePolarity = new CYesNoSetting("Reverse Polarity", FALSE, IniSection, "ReversePolarity");
    m_Settings.push_back(m_ReversePolarity);

    m_CardType = new CSliderSetting("Card Type", TVCARD_UNKNOWN, TVCARD_UNKNOWN, TVCARD_LASTONE - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_ProcessorSpeed = new CSliderSetting("Processor Speed", 1, 0, 2, IniSection, "ProcessorSpeed");
    m_Settings.push_back(m_ProcessorSpeed);

    m_TradeOff = new CSliderSetting("Quality Trade Off", 0, 1, 1, IniSection, "TradeOff");
    m_Settings.push_back(m_TradeOff);

    m_AudioSource = new CAudioSourceSetting(this, "Audio Source", AUDIOINPUT_MUTE, AUDIOINPUT_TUNER, AUDIOINPUT_STEREO, IniSection);
    m_Settings.push_back(m_AudioSource);

    m_AudioChannel = new CAudioChannelSetting(this, "Audio Channel", SOUNDCHANNEL_STEREO, SOUNDCHANNEL_MONO, SOUNDCHANNEL_LANGUAGE2, IniSection);
    m_Settings.push_back(m_AudioChannel);

    m_AutoStereoSelect = new CAutoStereoSelectSetting(this, "Auto Stereo Select", FALSE, IniSection);
    m_Settings.push_back(m_AutoStereoSelect);

    m_Volume = new CVolumeSetting(this, "Volume", 900, 0, 1000, IniSection);
    m_Volume->SetStepValue(20);
    m_Settings.push_back(m_Volume);

    m_Bass = new CBassSetting(this, "Bass", 0, -96, 127, IniSection);
    m_Settings.push_back(m_Bass);

    m_Treble = new CTrebleSetting(this, "Treble", 0, -96, 127, IniSection);
    m_Settings.push_back(m_Treble);

    m_Balance = new CBalanceSetting(this, "Balance", 0, -127, 127, IniSection);
    m_Settings.push_back(m_Balance);

    m_bSavePerInput = new CYesNoSetting("Save Per Input", FALSE, IniSection, "SavePerInput");
    m_Settings.push_back(m_bSavePerInput);
    
    m_bSavePerFormat = new CYesNoSetting("Save Per Format", FALSE, IniSection, "SavePerFormat");
    m_Settings.push_back(m_bSavePerFormat);
    
    m_bSavePerChannel = new CYesNoSetting("Save Per Channel", FALSE, IniSection, "SavePerChannel");
    m_Settings.push_back(m_bSavePerChannel);

    ReadFromIni();
}


void CBT848Source::Start()
{
    m_pBT848Card->StopCapture();
    CreateRiscCode(bCaptureVBI);
    m_pBT848Card->StartCapture(bCaptureVBI);
    m_pBT848Card->SetDMA(TRUE);
    Audio_Unmute();
    Timing_Reset();
}

void CBT848Source::Reset()
{
    m_pBT848Card->ResetHardware(m_RiscBasePhysical);
    m_pBT848Card->SetVideoSource(m_VideoSource->GetValue());
    if (m_BDelay->GetValue() != 0)
    {
        // BDELAY override from .ini file
        m_pBT848Card->SetBDelay(m_BDelay->GetValue());
    }

    m_pBT848Card->SetBrightness(m_Brightness->GetValue());
    m_pBT848Card->SetContrast(m_Contrast->GetValue());
    m_pBT848Card->SetHue(m_Hue->GetValue());
    m_pBT848Card->SetSaturationU(m_SaturationU->GetValue());
    m_pBT848Card->SetSaturationV(m_SaturationV->GetValue());
    m_pBT848Card->SetEvenLumaDec(m_BtEvenLumaDec->GetValue());
    m_pBT848Card->SetOddLumaDec(m_BtOddLumaDec->GetValue());
    m_pBT848Card->SetEvenChromaAGC(m_BtEvenChromaAGC->GetValue());
    m_pBT848Card->SetOddChromaAGC(m_BtOddChromaAGC->GetValue());
    m_pBT848Card->SetEvenLumaPeak(m_BtEvenLumaPeak->GetValue());
    m_pBT848Card->SetOddLumaPeak(m_BtOddLumaPeak->GetValue());
    m_pBT848Card->SetColorKill(m_BtColorKill->GetValue());
    m_pBT848Card->SetHorFilter(m_BtHorFilter->GetValue());
    m_pBT848Card->SetVertFilter(m_BtVertFilter->GetValue());
    m_pBT848Card->SetFullLumaRange(m_BtFullLumaRange->GetValue());
    m_pBT848Card->SetCoring(m_BtCoring->GetValue());
    m_pBT848Card->SetEvenComb(m_BtEvenComb->GetValue());
    m_pBT848Card->SetOddComb(m_BtOddComb->GetValue());
    m_pBT848Card->SetAgcDisable(m_BtAgcDisable->GetValue());
    m_pBT848Card->SetCrush(m_BtCrush->GetValue());
    m_pBT848Card->SetColorBars(m_BtColorBars->GetValue());
    m_pBT848Card->SetGammaCorrection(m_BtGammaCorrection->GetValue());
    m_pBT848Card->SetWhiteCrushUp(m_BtWhiteCrushUp->GetValue());
    m_pBT848Card->SetWhiteCrushDown(m_BtWhiteCrushDown->GetValue());

    m_CurrentX = m_PixelWidth->GetValue();
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );

    /// \todo FIXME anything else to initialize here?
    m_pBT848Card->SetAudioStandard((eVideoFormat)m_VideoFormat->GetValue());
    m_pBT848Card->SetAudioSource((eAudioInput)m_AudioSource->GetValue());
    m_pBT848Card->SetAudioChannel((eSoundChannel)m_AudioChannel->GetValue());
}


void CBT848Source::CreateRiscCode(BOOL bCaptureVBI)
{
    DWORD* pRiscCode;
    WORD nField;
    WORD nLine;
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytesPerLine;
    DWORD BytesPerLine = 0;

    pRiscCode = (DWORD*)m_RiscBaseLinear;
    // we create the RISC code for 10 fields
    // the first one (0) is even
    // last one (9) is odd
    for(nField = 0; nField < 10; nField++)
    {
        // First we sync onto either the odd or even field
        if(nField & 1)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRO);
        }
        else
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_RISC_RESYNC | BT848_FIFO_STATUS_VRE);
        }
        *(pRiscCode++) = 0;

        // Create VBI code of required
        if (bCaptureVBI)
        {
            *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
            *(pRiscCode++) = 0;

            pUser = m_pVBILines[nField / 2];
            if((nField & 1) == 1)
            {
                pUser += m_CurrentVBILines * 2048;
            }
            for (nLine = 0; nLine < m_CurrentVBILines; nLine++)
            {
                pPhysical = m_VBIDMAMem[nField / 2]->TranslateToPhysical(pUser, VBI_SPL, &GotBytesPerLine);
                if(pPhysical == 0 || VBI_SPL > GotBytesPerLine)
                {
                    return;
                }
                *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | VBI_SPL;
                *(pRiscCode++) = pPhysical;
                pUser += 2048;
            }
        }

        *(pRiscCode++) = (DWORD) (BT848_RISC_SYNC | BT848_FIFO_STATUS_FM1);
        *(pRiscCode++) = 0;


        // work out the position of the first line
        // first line is line zero an even line
        pUser = m_pDisplay[nField / 2];
        if(nField & 1)
        {
            pUser += 2048;
        }
        BytesPerLine = m_CurrentX * 2;
        for (nLine = 0; nLine < m_CurrentY / 2; nLine++)
        {

            pPhysical = m_DisplayDMAMem[nField / 2]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
            {
                return;
            }
            *(pRiscCode++) = BT848_RISC_WRITE | BT848_RISC_SOL | BT848_RISC_EOL | BytesPerLine;
            *(pRiscCode++) = pPhysical;
            // since we are doing all the lines of the same
            // polarity at the same time we skip two lines
            pUser += 4096;
        }
    }

    m_BytesPerRISCField = ((long)pRiscCode - (long)m_RiscBaseLinear) / 10;
    *(pRiscCode++) = BT848_RISC_JUMP;
    *(pRiscCode++) = m_RiscBasePhysical;

    m_pBT848Card->SetRISCStartAddress(m_RiscBasePhysical);
}


void CBT848Source::Stop()
{
    // stop capture
    m_pBT848Card->StopCapture();
}

void CBT848Source::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    static long RepeatCount = 0;

    if(AccurateTiming)
    {
        GetNextFieldAccurate(pInfo);
    }
    else
    {
        GetNextFieldNormal(pInfo);
    }

    if (!pInfo->bRunningLate)
    {
    }

    memmove(&pInfo->PictureHistory[1], &pInfo->PictureHistory[0], sizeof(pInfo->PictureHistory) - sizeof(pInfo->PictureHistory[0]));
    if(m_IsFieldOdd)
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            pInfo->PictureHistory[0] = &m_OddFields[pInfo->CurrentFrame];
        }
        else
        {
            pInfo->PictureHistory[0] = &m_EvenFields[pInfo->CurrentFrame];
        }
    }
    else
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            pInfo->PictureHistory[0] = &m_EvenFields[pInfo->CurrentFrame];
        }
        else
        {
            pInfo->PictureHistory[0] = &m_OddFields[(pInfo->CurrentFrame + 4) % 5];
        }
    }

    pInfo->LineLength = m_CurrentX * 2;
    pInfo->FrameWidth = m_CurrentX;
    pInfo->FrameHeight = m_CurrentY;
    pInfo->FieldHeight = m_CurrentY / 2;
    pInfo->InputPitch = 4096;

    Timing_IncrementUsedFields();

    // auto input detect
    Timimg_AutoFormatDetect(pInfo);
}

int CBT848Source::GetWidth()
{
    return m_CurrentX;
}

int CBT848Source::GetHeight()
{
    return m_CurrentY;
}


CBT848Card* CBT848Source::GetBT848Card()
{
    return m_pBT848Card;
}

LPCSTR CBT848Source::GetStatus()
{
    static LPCSTR pRetVal = "";
    if (!m_pBT848Card->IsVideoPresent())
    {
        pRetVal = "No Video Signal Found";
    }
    else
    {
        if (*VT_GetStation() != 0x00)
        {
            pRetVal = VT_GetStation();
        }
        else if (VPSLastName[0] != 0x00)
        {
            pRetVal = VPSLastName;
        }
    }
    return pRetVal;
}

int CBT848Source::GetRISCPosAsInt()
{
    int CurrentPos = 10;
    while(CurrentPos > 9)
    {
        DWORD CurrentRiscPos = m_pBT848Card->GetRISCPos();
        CurrentPos = (CurrentRiscPos - m_RiscBasePhysical) / m_BytesPerRISCField;
    }

    return CurrentPos;
}

eVideoFormat CBT848Source::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}

ISetting* CBT848Source::GetBrightness()
{
    return m_Brightness;
}

ISetting* CBT848Source::GetContrast()
{
    return m_Contrast;
}

ISetting* CBT848Source::GetHue()
{
    return m_Hue;
}

ISetting* CBT848Source::GetSaturation()
{
    return m_Saturation;
}

ISetting* CBT848Source::GetSaturationU()
{
    return m_SaturationU;
}

ISetting* CBT848Source::GetSaturationV()
{
    return m_SaturationV;
}

void CBT848Source::BtAgcDisableOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetAgcDisable(NewValue);
}

void CBT848Source::BtCrushOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetCrush(NewValue);
}

void CBT848Source::BtEvenChromaAGCOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenChromaAGC(NewValue);
}

void CBT848Source::BtOddChromaAGCOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddChromaAGC(NewValue);
}

void CBT848Source::BtEvenLumaPeakOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenLumaPeak(NewValue);
}

void CBT848Source::BtOddLumaPeakOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddLumaPeak(NewValue);
}

void CBT848Source::BtFullLumaRangeOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetFullLumaRange(NewValue);
}

void CBT848Source::BtEvenLumaDecOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenLumaDec(NewValue);
}

void CBT848Source::BtOddLumaDecOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddLumaDec(NewValue);
}

void CBT848Source::BtEvenCombOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetEvenComb(NewValue);
}

void CBT848Source::BtOddCombOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetOddComb(NewValue);
}

void CBT848Source::BtColorBarsOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetColorBars(NewValue);
}

void CBT848Source::BtGammaCorrectionOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetGammaCorrection(NewValue);
}

void CBT848Source::BtVertFilterOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetVertFilter(NewValue);
}

void CBT848Source::BtHorFilterOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetHorFilter(NewValue);
}

////////////////////////////////////////////////////////////////////////////////////
// The following function will continually check the position in the RISC code
// until it is  is different from what we already have.
// We know were we are so we set the current field to be the last one
// that has definitely finished.
//
// Added code here to use a user specified parameter for how long to sleep.  Note that
// windows timer tick resolution is really MUCH worse than 1 millesecond.  Who knows 
// what logic W98 really uses?
//
// Note also that sleep(0) just tells the operating system to dispatch some other
// task now if one is ready, not to sleep for zero seconds.  Since I've taken most
// of the unneeded waits out of other processing here Windows will eventually take 
// control away from us anyway, We might as well choose the best time to do it, without
// waiting more than needed. 
//
// Also added code to HurryWhenLate.  This checks if the new field is already here by
// the time we arrive.  If so, assume we are not keeping up with the BT chip and skip
// some later processing.  Skip displaying this field and use the CPU time gained to 
// get back here faster for the next one.  This should help us degrade gracefully on
// slower or heavily loaded systems but use all available time for processing a good
// picture when nothing else is running.  TRB 10/28/00
//
void CBT848Source::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;

    while(OldPos == (NewPos = GetRISCPosAsInt()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    Diff = (10 + NewPos - OldPos) % 10;
    if(Diff > 1)
    {
        // delete all history
        memset(pInfo->PictureHistory, 0, MAX_PICTURE_HISTORY * sizeof(TPicture*));
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
    }
    else
    {
        pInfo->bMissedFrame = FALSE;
        if (pInfo->bRunningLate)
        {
            Timing_AddDroppedFields(1);
            LOG(2, "Running Late");
        }
    }

    switch(NewPos)
    {
    case 0: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 4; break;
    case 1: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 0; break;
    case 2: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 0; break;
    case 3: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 1; break;
    case 4: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 1; break;
    case 5: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 2; break;
    case 6: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 2; break;
    case 7: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 3; break;
    case 8: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 3; break;
    case 9: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 4; break;
    }
}

void CBT848Source::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    
    while(OldPos == (NewPos = GetRISCPosAsInt()))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    Diff = (10 + NewPos - OldPos) % 10;
    if(Diff == 1)
    {
    }
    else if(Diff == 2) 
    {
        NewPos = (OldPos + 1) % 10;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Slightly late");
    }
    else if(Diff == 3) 
    {
        NewPos = (OldPos + 1) % 10;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Very late");
    }
    else
    {
        // delete all history
        memset(pInfo->PictureHistory, 0, MAX_PICTURE_HISTORY * sizeof(TPicture*));
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
        Timing_Reset();
    }

    switch(NewPos)
    {
    case 0: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 4; break;
    case 1: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 0; break;
    case 2: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 0; break;
    case 3: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 1; break;
    case 4: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 1; break;
    case 5: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 2; break;
    case 6: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 2; break;
    case 7: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 3; break;
    case 8: m_IsFieldOdd = TRUE;  pInfo->CurrentFrame = 3; break;
    case 9: m_IsFieldOdd = FALSE; pInfo->CurrentFrame = 4; break;
    }
    
    // we've just got a new field
    // we are going to time the odd to odd
    // input frequency
    if(m_IsFieldOdd)
    {
        Timing_UpdateRunningAverage(pInfo);
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}

void CBT848Source::VideoSourceOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    Audio_Mute();
    SaveInputSettings();
    LoadInputSettings();
    Reset();

    // set up sound
    if(m_pBT848Card->IsInputATuner(NewValue))
    {
        if(m_pBT848Card->HasMSP())
        {
            m_AudioSource->SetValue(AUDIOINPUT_RADIO);
        }
        else
        {
            m_AudioSource->SetValue(AUDIOINPUT_TUNER);
        }
        Channel_SetCurrent();
    }
    else
    {
        m_AudioSource->SetValue(AUDIOINPUT_EXTERNAL);
    }
    Audio_Unmute();
    Start_Capture();
}

void CBT848Source::VideoFormatOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    SaveInputSettings();
    LoadInputSettings();
    Reset();
    Start_Capture();
}

void CBT848Source::PixelWidthOnChange(long NewValue, long OldValue)
{
    if(NewValue != 768 &&
        NewValue != 754 &&
        NewValue != 720 &&
        NewValue != 640 &&
        NewValue != 384 &&
        NewValue != 320)
    {
        m_CustomPixelWidth->SetValue(NewValue);
    }
    Stop_Capture();
    m_CurrentX = NewValue;
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    Start_Capture();
}

void CBT848Source::HDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    Start_Capture();
}

void CBT848Source::VDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pBT848Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );
    Start_Capture();
}

void CBT848Source::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pBT848Card->SetBrightness(Brightness);
}

void CBT848Source::BtWhiteCrushUpOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushUp(NewValue);
}

void CBT848Source::BtWhiteCrushDownOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetWhiteCrushDown(NewValue);
}

void CBT848Source::BtCoringOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetCoring(NewValue);
}

void CBT848Source::BtColorKillOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetColorKill(NewValue);
}

void CBT848Source::BDelayOnChange(long NewValue, long OldValue)
{
    m_pBT848Card->SetBDelay(NewValue);
}

void CBT848Source::HueOnChange(long Hue, long OldValue)
{
    m_pBT848Card->SetHue(Hue);
}

void CBT848Source::ContrastOnChange(long Contrast, long OldValue)
{
    m_pBT848Card->SetContrast(Contrast);
}

void CBT848Source::SaturationUOnChange(long SatU, long OldValue)
{
    m_pBT848Card->SetSaturationU(SatU);
    if(SatU < OldValue - 2 || SatU > OldValue + 2)
    {
        m_Saturation->SetValue((SatU + m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatU - m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(SatU - m_SaturationV->GetValue()) / 2);
    }
}

void CBT848Source::SaturationVOnChange(long SatV, long OldValue)
{
    m_pBT848Card->SetSaturationV(SatV);
    if(SatV < OldValue - 2 || SatV > OldValue + 2)
    {
        m_Saturation->SetValue((SatV + m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatV - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(SatV - m_SaturationU->GetValue()) / 2);
    }
}


void CBT848Source::SaturationOnChange(long Sat, long OldValue)
{
    if(Sat != OldValue)
    {
        long NewSaturationU = m_SaturationU->GetValue() + (Sat - OldValue);
        long NewSaturationV = m_SaturationV->GetValue() + (Sat - OldValue);
        m_SaturationU->SetValue(NewSaturationU);
        m_SaturationV->SetValue(NewSaturationV);
        m_Saturation->SetMin(abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(511 - abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
    }
}

void CBT848Source::TunerTypeOnChange(long TunerId, long OldValue)
{
	m_pBT848Card->InitTuner((eTunerId)TunerId);
}


BOOL CBT848Source::IsInTunerMode()
{
    return m_pBT848Card->IsInputATuner(m_VideoSource->GetValue());
}


void CBT848Source::SetupCard()
{
    if(m_CardType->GetValue() == TVCARD_UNKNOWN)
    {
        // try to detect the card
        m_CardType->SetValue(m_pBT848Card->AutoDetectCardType());
        m_TunerType->SetValue(m_pBT848Card->AutoDetectTuner((eTVCardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
    }
    m_pBT848Card->SetCardType(m_CardType->GetValue());
    m_pBT848Card->InitTuner((eTunerId)m_TunerType->GetValue());
    m_pBT848Card->InitAudio();
}

void CBT848Source::ChangeDefaultsBasedOnHardware()
{
    // now do defaults based on the processor speed selected
    if(m_ProcessorSpeed->GetValue() == 1 && m_TradeOff->GetValue() == 0)
    {
        // User has selected 300-500 MHz and low judder
        Setting_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        m_PixelWidth->ChangeDefault(720);
    }
    else if(m_ProcessorSpeed->GetValue() == 1 && m_TradeOff->GetValue() == 1)
    {
        // User has selected 300-500 MHz and best picture
        Setting_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        m_PixelWidth->ChangeDefault(720);
    }
    else if(m_ProcessorSpeed->GetValue() == 2 && m_TradeOff->GetValue() == 0)
    {
        // User has selected below 300 MHz and low judder
        Setting_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        m_PixelWidth->ChangeDefault(640);
    }
    else if(m_ProcessorSpeed->GetValue() == 2 && m_TradeOff->GetValue() == 1)
    {
        // User has selected below 300 MHz and best picture
        Setting_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        m_PixelWidth->ChangeDefault(640);
    }
    else
    {
        // user has fast processor use best defaults
        Setting_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_ADAPTIVE);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY2FRAME);
        m_PixelWidth->ChangeDefault(720);
    }

}

void CBT848Source::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        eVideoFormat videoFormat = m_pBT848Card->GetTuner()->GetDefaultVideoFormat();
        m_VideoFormat->ChangeDefault(videoFormat);
    }
}

BOOL CBT848Source::HasTuner()
{
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL CBT848Source::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    if(VideoFormat == VIDEOFORMAT_LASTONE)
    {
        VideoFormat = m_pBT848Card->GetTuner()->GetDefaultVideoFormat();
    }
    if(VideoFormat != m_VideoFormat->GetValue())
    {
        m_VideoFormat->SetValue(VideoFormat);
	    m_pBT848Card->SetAudioStandard(VideoFormat);
    }
    return m_pBT848Card->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
}

BOOL CBT848Source::IsVideoPresent()
{
    return m_pBT848Card->IsVideoPresent();
}


void CBT848Source::DecodeVBI(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    BYTE* pVBI = (LPBYTE) m_pVBILines[(pInfo->CurrentFrame + 4) % 5];
    if (m_IsFieldOdd)
    {
        pVBI += m_CurrentVBILines * 2048;
    }
    for (nLineTarget = 0; nLineTarget < m_CurrentVBILines ; nLineTarget++)
    {
       VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget, m_IsFieldOdd);
    }
}
