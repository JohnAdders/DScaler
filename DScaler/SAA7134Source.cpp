/////////////////////////////////////////////////////////////////////////////
// $id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// This software was based on BT848Source.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $log$
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Source.h"
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
#include "SAA7134_Defines.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"


void SAA7134_OnSetup(void *pThis, int Start)
{
   if (pThis != NULL)
   {
      ((CSAA7134Source*)pThis)->SavePerChannelSetup(Start);
   }
}


CSAA7134Source::CSAA7134Source(CSAA7134Card* pSAA7134Card, CContigMemory* PageTableDMAMem[4], CUserMemory* DisplayDMAMem[2], CUserMemory* VBIDMAMem[2], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex) :
    // \todo FIXME
    CSource(WM_BT848_GETVALUE, IDC_SAA7134),
    m_pSAA7134Card(pSAA7134Card),
    m_CurrentX(768),
    m_CurrentY(576),
    m_CurrentVBILines(19),
    m_Section(IniSection),
    m_IsFieldOdd(FALSE),
    m_InSaturationUpdate(FALSE),
    m_CurrentChannel(-1),
    m_SettingsByChannelStarted(FALSE),
    m_ChipName(ChipName),
    m_DeviceIndex(DeviceIndex),
    m_LastFieldIndex(0)
{
    m_IDString = IniSection;
    CreateSettings(IniSection);

    m_InitialACPIStatus = m_pSAA7134Card->GetACPIStatus();

    // if the card is powered down we need to power it up
    if(m_InitialACPIStatus != 0)
    {
        m_pSAA7134Card->SetACPIStatus(0);
    }

    SettingsPerChannel_RegisterOnSetup(this, SAA7134_OnSetup);

    ReadFromIni();
    ChangeSectionNamesForInput();
    ChangeDefaultsForInput();
    LoadInputSettings();

// Used for register guessing!
//  m_pSAA7134Card->DumpRegisters();
//  exit(1);

    for(int i(0); i < 2; ++i)
    {
        m_DisplayPageTableLinear[i] = (DWORD*)PageTableDMAMem[i]->GetUserPointer();
        m_DisplayPageTablePhysical[i] = PageTableDMAMem[i]->TranslateToPhysical(m_DisplayPageTableLinear[i], 4096, NULL);
        m_VBIPageTableLinear[i] = (DWORD*)PageTableDMAMem[i+2]->GetUserPointer();
        m_VBIPageTablePhysical[i] = PageTableDMAMem[i+2]->TranslateToPhysical(m_VBIPageTableLinear[i], 4096, NULL);
    }

    for(int j(0); j < 2; ++j)
    {
        m_pDisplay[j] = (BYTE*)DisplayDMAMem[j]->GetUserPointer();
        m_DisplayDMAMem[j] = DisplayDMAMem[j];
        m_pVBILines[j] = (BYTE*)VBIDMAMem[j]->GetUserPointer();
        m_VBIDMAMem[j] = VBIDMAMem[j];
    }

    // Set up 2 sets of pointers to the start of odd and even lines
    for (int k(0); k < 2; k++)
    {
        m_OddFields[k].pData = m_pDisplay[k] + 2048;
        m_OddFields[k].Flags = PICTURE_INTERLACED_ODD;
        m_OddFields[k].IsFirstInSeries = FALSE;
        m_EvenFields[k].pData = m_pDisplay[k];
        m_EvenFields[k].Flags = PICTURE_INTERLACED_EVEN;
        m_EvenFields[k].IsFirstInSeries = FALSE;
    }

    SetupCard();
    Reset();

    NotifyInputChange(0, VIDEOINPUT, -1, m_VideoSource->GetValue());
}

CSAA7134Source::~CSAA7134Source()
{
    SAA7134_OnSetup(this, 0);
    // if the card was not in D0 state we restore the original ACPI power state
    if(m_InitialACPIStatus != 0)
    {
        m_pSAA7134Card->SetACPIStatus(m_InitialACPIStatus);
    }

    delete m_pSAA7134Card;
}


#define DEFAULT_SAA7134_BRIGHTNESS  128
#define DEFAULT_SAA7134_CONTRAST    68
#define DEFAULT_SAA7134_HUE         0
#define DEFAULT_SAA7134_SATURATION  64

void CSAA7134Source::CreateSettings(LPCSTR IniSection)
{
    m_Brightness = new CBrightnessSetting(this, "Brightness", DEFAULT_SAA7134_BRIGHTNESS, 0, 255, IniSection);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", DEFAULT_SAA7134_CONTRAST, 0, 255, IniSection);
    m_Settings.push_back(m_Contrast);

    m_Hue = new CHueSetting(this, "Hue", DEFAULT_SAA7134_HUE, 0, 255, IniSection);
    m_Settings.push_back(m_Hue);

    m_Saturation = new CSaturationSetting(this, "Saturation", DEFAULT_SAA7134_SATURATION, 0, 255, IniSection);
    m_Settings.push_back(m_Saturation);

    m_Overscan = new COverscanSetting(this, "Overscan", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection);
    m_Settings.push_back(m_Overscan);

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

    m_HDelay = new CHDelaySetting(this, "Horizontal Delay", 0, 0, 255, IniSection);
    m_Settings.push_back(m_HDelay);

    m_VDelay = new CVDelaySetting(this, "Vertical Delay", 0, 0, 255, IniSection);
    m_VDelay->SetStepValue(2);
    m_Settings.push_back(m_VDelay);

    m_ReversePolarity = new CYesNoSetting("Reverse Polarity", FALSE, IniSection, "ReversePolarity");
    m_Settings.push_back(m_ReversePolarity);

    m_CardType = new CSliderSetting("Card Type", TVCARD_UNKNOWN, TVCARD_UNKNOWN, m_pSAA7134Card->GetMaxCards() - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_AudioSource1 = new CAudioSource1Setting(this, "Audio Source 1", AUDIOINPUTLINE_LINE1, AUDIOINPUTLINE_TUNER, AUDIOINPUTLINE_LINE2, IniSection);
    m_Settings.push_back(m_AudioSource1);

    m_AudioStandard = new CAudioStandardSetting(this, "Audio Standard", AUDIOSTANDARD_BG_DUAL_FM, AUDIOSTANDARD_BG_DUAL_FM, AUDIOSTANDARD_LAST_ONE-1, IniSection);
    m_Settings.push_back(m_AudioSource1);

    m_AudioChannel = new CAudioChannelSetting(this, "Audio Channel", SOUNDCHANNEL_STEREO, SOUNDCHANNEL_MONO, SOUNDCHANNEL_LANGUAGE2, IniSection);
    m_Settings.push_back(m_AudioChannel);

    m_AutoStereoSelect = new CAutoStereoSelectSetting(this, "Auto Stereo Select", FALSE, IniSection);
    m_Settings.push_back(m_AutoStereoSelect);

    m_Volume = new CVolumeSetting(this, "Volume", 0, 0, 1000, IniSection);
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
    
    m_bSavePerFormat = new CYesNoSetting("Save Per Format", TRUE, IniSection, "SavePerFormat");
    m_Settings.push_back(m_bSavePerFormat);
    
    m_AudioSource2 = new CAudioSource2Setting(this, "Audio Source 2", AUDIOINPUTLINE_LINE1, AUDIOINPUTLINE_TUNER, AUDIOINPUTLINE_LINE2, IniSection);
    m_Settings.push_back(m_AudioSource2);

    m_AudioSource3 = new CAudioSource3Setting(this, "Audio Source 3", AUDIOINPUTLINE_LINE1, AUDIOINPUTLINE_TUNER, AUDIOINPUTLINE_LINE2, IniSection);
    m_Settings.push_back(m_AudioSource3);

    m_AudioSource4 = new CAudioSource4Setting(this, "Audio Source 4", AUDIOINPUTLINE_LINE1, AUDIOINPUTLINE_TUNER, AUDIOINPUTLINE_LINE2, IniSection);
    m_Settings.push_back(m_AudioSource4);

    m_AudioSource5 = new CAudioSource5Setting(this, "Audio Source 5", AUDIOINPUTLINE_LINE1, AUDIOINPUTLINE_TUNER, AUDIOINPUTLINE_LINE2, IniSection);
    m_Settings.push_back(m_AudioSource5);

    m_AudioSource6 = new CAudioSource6Setting(this, "Audio Source 6", AUDIOINPUTLINE_LINE1, AUDIOINPUTLINE_TUNER, AUDIOINPUTLINE_LINE2, IniSection);
    m_Settings.push_back(m_AudioSource6);

    ReadFromIni();
}


void CSAA7134Source::Start()
{
    m_pSAA7134Card->StopCapture();
    CreateDMAMemorySetup(bCaptureVBI);
    m_pSAA7134Card->StartCapture(bCaptureVBI);
    Audio_Unmute();
    Timing_Reset();
    NotifySizeChange();
    // m_pSAA7134Card->DumpRegisters();
    NotifySquarePixelsCheck();
    m_ProcessingRegionID = REGIONID_INVALID;
}

void CSAA7134Source::Reset()
{
    m_pSAA7134Card->ResetHardware();
    m_pSAA7134Card->ResetTask(TASKID_A);
    m_pSAA7134Card->ResetTask(TASKID_B);

    m_pSAA7134Card->SetVideoSource(m_VideoSource->GetValue());

    m_pSAA7134Card->SetBrightness(m_Brightness->GetValue());
    m_pSAA7134Card->SetContrast(m_Contrast->GetValue());
    m_pSAA7134Card->SetHue(m_Hue->GetValue());
    m_pSAA7134Card->SetSaturation(m_Saturation->GetValue());

    m_CurrentX = m_PixelWidth->GetValue();
    m_pSAA7134Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );

    NotifySizeChange();

    m_pSAA7134Card->SetAudioStandard((eVideoFormat)m_VideoFormat->GetValue());
    m_pSAA7134Card->SetAudioSource((eAudioInputLine)GetCurrentAudioSetting()->GetValue());
    m_pSAA7134Card->SetAudioChannel((eSoundChannel)m_AudioChannel->GetValue());
}




DWORD CSAA7134Source::CreatePageTable(CUserMemory* pDMAMemory, DWORD nPagesWanted, LPDWORD pPageTable)
{
    const WORD PAGE_SIZE = 4096;
    const WORD PAGE_MASK = (~(PAGE_SIZE-1));
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytes;
    DWORD nPages;

    if (pDMAMemory == NULL || nPagesWanted == 0 || pPageTable == NULL)
    {
        return 0;
    }

    pUser = (LPBYTE) pDMAMemory->GetUserPointer();

    // This routine should get nPagesWanted pages in their page boundries
    for (nPages = 0; nPages < nPagesWanted; nPages++)
    {
        pPhysical = pDMAMemory->TranslateToPhysical(pUser, PAGE_SIZE, &GotBytes);
        if(pPhysical == 0 || pPhysical & ~PAGE_MASK > 0 || GotBytes < PAGE_SIZE)
        {
            break;
        }

        *(pPageTable++) = pPhysical;
        pUser += PAGE_SIZE;
    }

    return nPages;
}

void CSAA7134Source::CreateDMAMemorySetup(BOOL bCaptureVBI)
{
    WORD nFrame;
    DWORD nPages;
    eRegionID RegionID;

    for (nFrame = 0; nFrame < 2; nFrame++)
    {
        nPages = CreatePageTable(m_DisplayDMAMem[nFrame],
                                    m_CurrentY/2,
                                    m_DisplayPageTableLinear[nFrame]
                                );

        RegionID = (nFrame == 0) ? REGIONID_VIDEO_A : REGIONID_VIDEO_B;

        m_pSAA7134Card->SetDMA(RegionID, FALSE);

        m_pSAA7134Card->SetPageTable(RegionID, m_DisplayPageTablePhysical[nFrame], nPages);
        m_pSAA7134Card->SetBaseOffsets(RegionID, 0, 2048, 4096);
        m_pSAA7134Card->SetBSwapAndWSwap(RegionID, TRUE, FALSE);

        // It should now be safe to enable the channel
        m_pSAA7134Card->SetDMA(RegionID, TRUE);

        if (bCaptureVBI)
        {
            nPages = CreatePageTable(m_VBIDMAMem[nFrame],
                                        m_CurrentVBILines,
                                        m_VBIPageTableLinear[nFrame]
                                    );

            RegionID = (nFrame == 0) ? REGIONID_VBI_A : REGIONID_VBI_B;

            m_pSAA7134Card->SetDMA(RegionID, FALSE);

            m_pSAA7134Card->SetPageTable(RegionID, m_VBIPageTablePhysical[nFrame], nPages);
            m_pSAA7134Card->SetBaseOffsets(RegionID, 0, 0 + m_CurrentVBILines * 2048, 2048);
            m_pSAA7134Card->SetBSwapAndWSwap(RegionID, FALSE, FALSE);

            // It should now be safe to enable the channel
            m_pSAA7134Card->SetDMA(RegionID, TRUE);
        }
    }
}


void CSAA7134Source::Stop()
{
    // stop capture
    m_pSAA7134Card->StopCapture();
    Audio_Mute();
}

void CSAA7134Source::GiveNextField(TDeinterlaceInfo* pInfo, TPicture* picture)
{
    if (pInfo->bMissedFrame)
    {
        picture->IsFirstInSeries = TRUE;
    }
    else
    {
        picture->IsFirstInSeries = FALSE;
    }

    if ((picture->Flags & PICTURE_INTERLACED_EVEN) > 0 ||
        pInfo->bMissedFrame)
    {
        pInfo->CurrentFrame = (pInfo->CurrentFrame + 1) % 5;
    }

    Shift_Picture_History(pInfo);
    Replace_Picture_In_History(pInfo, 0, picture);
}

void CSAA7134Source::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
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

    if(m_IsFieldOdd)
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            /* // DEBUG VBI testing
            for (int i = 0; i < m_CurrentVBILines; i++)
            {
                for (int j = 0; j < m_CurrentX * 2; j++)
                {

                    m_pDisplay[m_CurrentFrame][2048 + i * 4096 + j] = m_pVBILines[m_CurrentFrame][m_CurrentVBILines * 2048 + 2048 * i + (int) (j/2)];
					// m_pDisplay[m_CurrentFrame][2048 + i * 4096 + j] = m_pVBILines[m_CurrentFrame][m_CurrentVBILines * 2048 + 2048 * i + j];

                }
            }
            //*/

            GiveNextField(pInfo, &m_OddFields[m_CurrentFrame]);
        }
        else
        {
            GiveNextField(pInfo, &m_EvenFields[m_CurrentFrame]);
        }
    }
    else
    {
        if(m_ReversePolarity->GetValue() == FALSE)
        {
            /* // DEBUG VBI testing
            for (int i = 0; i < m_CurrentVBILines; i++)
            {
                for (int j = 0; j < m_CurrentX * 2; j++)
                {
                    m_pDisplay[m_CurrentFrame][i * 4096 + j] = m_pVBILines[m_CurrentFrame][2048 * i + (int) (j/2)];
					// m_pDisplay[m_CurrentFrame][i * 4096 + j] = m_pVBILines[m_CurrentFrame][2048 * i + j];
                }
            }
            //*/
            
            GiveNextField(pInfo, &m_EvenFields[m_CurrentFrame]);
        }
        else
        {
            GiveNextField(pInfo, &m_OddFields[m_CurrentFrame]);
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

void CSAA7134Source::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    eRegionID RegionID;
    BOOL bIsFieldOdd;
    BOOL bSlept = FALSE;
    int FieldIndex;
    int SkippedFields;

    // This function blocks until field is ready
    WaitForFinishedField(RegionID, bIsFieldOdd, pInfo);

    FieldIndex = EnumulateField(RegionID, bIsFieldOdd);
    SkippedFields = (4 + FieldIndex - m_LastFieldIndex - 1) % 4;

    if(SkippedFields != 0)
    {
        // delete all history
        Free_Picture_History(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(SkippedFields);
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

    m_LastFieldIndex = FieldIndex;

    if (RegionID2TaskID(RegionID) == TASKID_A)
        m_CurrentFrame = 0;
    else
        m_CurrentFrame = 1;

    m_IsFieldOdd = bIsFieldOdd;
}

void CSAA7134Source::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    eRegionID RegionID;
    BOOL bIsFieldOdd;
    BOOL bSlept = FALSE;
    int FieldIndex;
    int SkippedFields;

    if (m_ProcessingRegionID == REGIONID_INVALID)
    {
        m_pSAA7134Card->GetProcessingRegion(m_ProcessingRegionID,
                        m_IsProcessingFieldOdd);
    }

    while (!GetFinishedField(RegionID, bIsFieldOdd))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    FieldIndex = EnumulateField(RegionID, bIsFieldOdd);
    SkippedFields = (4 + FieldIndex - m_LastFieldIndex - 1) % 4;

    if(SkippedFields == 0)
    {
    }
    else if(SkippedFields == 1) 
    {
        m_ProcessingRegionID = RegionID;
        m_IsProcessingFieldOdd = bIsFieldOdd;
        FieldIndex = (m_LastFieldIndex + 1) % 4;
        DenumulateField(FieldIndex, RegionID, bIsFieldOdd);
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Slightly late");
    }
    // This might not be possible with only a 4 field cycle
    else if(SkippedFields == 2) 
    {
        m_ProcessingRegionID = RegionID;
        m_IsProcessingFieldOdd = bIsFieldOdd;
        FieldIndex = (m_LastFieldIndex + 1) % 4;
        DenumulateField(FieldIndex, RegionID, bIsFieldOdd);
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Very late");
    }
    else
    {
        // delete all history
        Free_Picture_History(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(SkippedFields);
        LOG(2, " Dropped Frame");
        Timing_Reset();
    }

    m_LastFieldIndex = FieldIndex;

    if (RegionID2TaskID(RegionID) == TASKID_A)
        m_CurrentFrame = 0;
    else
        m_CurrentFrame = 1;

    m_IsFieldOdd = bIsFieldOdd;

    // we've just got a new field
    // we are going to time the odd to odd
    // input frequency
    if(m_IsFieldOdd)
    {
        Timing_UpdateRunningAverage(pInfo);
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}

int CSAA7134Source::GetWidth()
{
    return m_CurrentX;
}

int CSAA7134Source::GetHeight()
{
    return m_CurrentY;
}


CSAA7134Card* CSAA7134Source::GetSAA7134Card()
{
    return m_pSAA7134Card;
}

LPCSTR CSAA7134Source::GetStatus()
{
    static LPCSTR pRetVal = "";
    if (IsInTunerMode())
    {
        if (*VT_GetStation() != 0x00)
        {
            pRetVal = VT_GetStation();
        }
        else if (VPSLastName[0] != 0x00)
        {
            pRetVal = VPSLastName;
        }
        else
        {
            pRetVal = Channel_GetName();
        }
    }
    else
    {
        pRetVal = m_pSAA7134Card->GetInputName(m_VideoSource->GetValue());
    }
    return pRetVal;
}


int CSAA7134Source::EnumulateField(eRegionID RegionID, BOOL bIsFieldOdd)
{
    if (RegionID == REGIONID_VIDEO_A)
    {
        return bIsFieldOdd ? 1 : 0;
    }
    else if (RegionID == REGIONID_VIDEO_B)
    {
        return bIsFieldOdd ? 3 : 2;
    }

    return -1;
}


void CSAA7134Source::DenumulateField(int Index, eRegionID& RegionID, BOOL& bIsFieldOdd)
{
    switch (Index)
    {
    case 0: RegionID = REGIONID_VIDEO_A; bIsFieldOdd = FALSE; break;
    case 1: RegionID = REGIONID_VIDEO_A; bIsFieldOdd = TRUE; break;
    case 2: RegionID = REGIONID_VIDEO_B; bIsFieldOdd = FALSE; break;
    case 3: RegionID = REGIONID_VIDEO_B; bIsFieldOdd = TRUE; break;
    }
}


BOOL CSAA7134Source::WaitForFinishedField(eRegionID& RegionID, BOOL& bIsFieldOdd,
                                          TDeinterlaceInfo* pInfo)
{
    BOOL    bUsingInterrupts = FALSE;

    if (bUsingInterrupts)
    {
        // Need to implement interrupts
    }
    else
    {
        BOOL bSlept = FALSE;

        if (m_ProcessingRegionID == REGIONID_INVALID)
        {
            m_pSAA7134Card->GetProcessingRegion(
                            m_ProcessingRegionID,
                            m_IsProcessingFieldOdd
                            );
        }

        while (!GetFinishedField(RegionID, bIsFieldOdd))
        {
            // need to sleep more often
            // so that we don't take total control of machine
            // in normal operation
            Timing_SmartSleep(pInfo, FALSE, bSlept);
            pInfo->bRunningLate = FALSE;            // if we waited then we are not late
        }
    }

    return TRUE;
}


BOOL CSAA7134Source::GetFinishedField(eRegionID& DoneRegionID, BOOL& bDoneIsFieldOdd)
{
    eRegionID RegionID;
    BOOL bIsFieldOdd;


    if (!m_pSAA7134Card->GetProcessingRegion(RegionID, bIsFieldOdd))
    {
        return FALSE;
    }

    if (RegionID == m_ProcessingRegionID && bIsFieldOdd == m_IsProcessingFieldOdd)
    {
        return FALSE;
    }

    DoneRegionID = m_ProcessingRegionID;
    bDoneIsFieldOdd = m_IsProcessingFieldOdd;

    m_ProcessingRegionID = RegionID;
    m_IsProcessingFieldOdd = bIsFieldOdd;

    return TRUE;
}



eVideoFormat CSAA7134Source::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}

void CSAA7134Source::SetFormat(eVideoFormat NewFormat)
{
    m_VideoFormat->SetValue(NewFormat);
}


ISetting* CSAA7134Source::GetBrightness()
{
    return m_Brightness;
}

ISetting* CSAA7134Source::GetContrast()
{
    return m_Contrast;
}

ISetting* CSAA7134Source::GetHue()
{
    return m_Hue;
}

ISetting* CSAA7134Source::GetSaturation()
{
    return m_Saturation;
}

ISetting* CSAA7134Source::GetSaturationU()
{
    return NULL;
}

ISetting* CSAA7134Source::GetSaturationV()
{
    return NULL;
}

ISetting* CSAA7134Source::GetOverscan()
{
    return m_Overscan;
}


////////////////////////////////////////////////////////////////////////
void CSAA7134Source::VideoSourceOnChange(long NewValue, long OldValue)
{
    NotifyInputChange(1, VIDEOINPUT, OldValue, NewValue);

    Stop_Capture();
    Audio_Mute();
    SaveInputSettings(TRUE);
    LoadInputSettings();
    Reset();

    NotifyInputChange(0, VIDEOINPUT, OldValue, NewValue);

    // set up sound
    if(m_pSAA7134Card->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
    }
    Audio_Unmute();
    Start_Capture();
}

void CSAA7134Source::VideoFormatOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    SaveInputSettings(TRUE);
    LoadInputSettings();
    Reset();
    Start_Capture();
}

void CSAA7134Source::PixelWidthOnChange(long NewValue, long OldValue)
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
    m_pSAA7134Card->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                m_CurrentVBILines,
                                m_VDelay->GetValue(), 
                                m_HDelay->GetValue()
                            );

    NotifySizeChange();

    Start_Capture();
}

void CSAA7134Source::HDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pSAA7134Card->SetGeoSize(
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

void CSAA7134Source::VDelayOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    m_pSAA7134Card->SetGeoSize(
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

void CSAA7134Source::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pSAA7134Card->SetBrightness(Brightness);
}

void CSAA7134Source::HueOnChange(long Hue, long OldValue)
{
    m_pSAA7134Card->SetHue(Hue);
}

void CSAA7134Source::ContrastOnChange(long Contrast, long OldValue)
{
    m_pSAA7134Card->SetContrast(Contrast);
}

void CSAA7134Source::SaturationOnChange(long Sat, long OldValue)
{
    m_pSAA7134Card->SetSaturation(Sat);
}

void CSAA7134Source::OverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CSAA7134Source::TunerTypeOnChange(long TunerId, long OldValue)
{
    m_pSAA7134Card->InitTuner((eTunerId)TunerId);
}


BOOL CSAA7134Source::IsInTunerMode()
{
    return m_pSAA7134Card->IsInputATuner(m_VideoSource->GetValue());
}


void CSAA7134Source::SetupCard()
{
    if(m_CardType->GetValue() == TVCARD_UNKNOWN)
    {
        /* // try to detect the card - \todo check if feasible
        m_CardType->SetValue(m_pSAA7134Card->AutoDetectCardType());
        m_TunerType->SetValue(m_pSAA7134Card->AutoDetectTuner((eTVCardId)m_CardType->GetValue()));
        */

        // then display the hardware setup dialog
        // \todo FIXME
        // EnableCancelButton = 0;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
        // EnableCancelButton = 1;
    }
    m_pSAA7134Card->SetCardType(m_CardType->GetValue());
    m_pSAA7134Card->InitTuner((eTunerId)m_TunerType->GetValue());
    m_pSAA7134Card->InitAudio();
    ChangeTVSettingsBasedOnCard();
}

void CSAA7134Source::ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
    // now do defaults based on the processor speed selected
    if(ProcessorSpeed == 0)
    {
        // User has selected below 300 MHz
        m_PixelWidth->ChangeDefault(640);
    }
    else if(ProcessorSpeed == 1)
    {
        // User has selected 300 MHz - 500 MHz
        m_PixelWidth->ChangeDefault(720);
    }
    else if(ProcessorSpeed == 2)
    {
        // User has selected 500 MHz - 1 GHz
        m_PixelWidth->ChangeDefault(720);
    }
    else
    {
        // user has fast processor use best defaults
        m_PixelWidth->ChangeDefault(720);
    }
}

void CSAA7134Source::ChangeTVSettingsBasedOnCard()
{
    int MaxInputs = m_pSAA7134Card->GetNumInputs();

    for (int Input = 0; Input < MaxInputs; Input++)
    {
        GetAudioSetting(Input)->SetValue(m_pSAA7134Card->GetInputAudioLine(Input));
    }
}

void CSAA7134Source::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        eVideoFormat videoFormat = m_pSAA7134Card->GetTuner()->GetDefaultVideoFormat();
        m_VideoFormat->ChangeDefault(videoFormat);
    }
}

BOOL CSAA7134Source::HasTuner()
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


BOOL CSAA7134Source::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    if(VideoFormat == VIDEOFORMAT_LASTONE)
    {
        VideoFormat = m_pSAA7134Card->GetTuner()->GetDefaultVideoFormat();
    }
    if(VideoFormat != m_VideoFormat->GetValue())
    {
        m_VideoFormat->SetValue(VideoFormat);
        m_pSAA7134Card->SetAudioStandard(VideoFormat);
    }
    return m_pSAA7134Card->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
}

BOOL CSAA7134Source::IsVideoPresent()
{
    return m_pSAA7134Card->IsVideoPresent();
}

int round(double i)
{
	if ((int) (i * 10) % 10 >= 5)
	{
		return (int) i + 1;
	}

	return (int) i;
}

void CSAA7134Source::DecodeVBI(TDeinterlaceInfo* pInfo)
{
    int nLineTarget;
    // VBI should have been DMA'd before the video
    BYTE* pVBI = (LPBYTE) m_pVBILines[m_CurrentFrame];

	BYTE ConvertBuffer[2048];

    if (m_IsFieldOdd)
    {
        pVBI += m_CurrentVBILines * 2048;
    }

	// Convert SAA7134's VBI buffer to the way DScaler wants it
	// 1. Shift the data 100 bytes to to the left
	// 2. Horizontal scale 262.54%  Half of this is already done.
	//    We get the card to do 131.28% scaling for us in SAA7134Card
	//    so we only need to double the bytes. ala. SAA7134Card::SetupVBI()
	//
	for (int i(0); i < 100; i++)
	{
		ConvertBuffer[i] = 0x00;
	}

    for (nLineTarget = 0; nLineTarget < m_CurrentVBILines ; nLineTarget++)
    {
		for (int i(100), j(0); i < 2044; i++, j++)
		{
			ConvertBuffer[i] = pVBI[nLineTarget * 2048 + (int) (j / 2)];
		}
        VBI_DecodeLine(ConvertBuffer, nLineTarget, m_IsFieldOdd);
		// VBI_DecodeLine(pVBI + nLineTarget * 2048, nLineTarget, m_IsFieldOdd);
    }
}

eTunerId CSAA7134Source::GetTunerId()
{
    return m_pSAA7134Card->GetTuner()->GetTunerId();
}

LPCSTR CSAA7134Source::GetMenuLabel()
{
    return m_pSAA7134Card->GetCardName(m_pSAA7134Card->GetCardType());
}

void CSAA7134Source::SetOverscan()
{
    AspectSettings.InitialOverscan = m_Overscan->GetValue();
}

void CSAA7134Source::SavePerChannelSetup(int Start)
{
    if (Start&1)
    {
        m_SettingsByChannelStarted = TRUE;
        ChangeChannelSectionNames();
    }    
    else
    {
        m_SettingsByChannelStarted = FALSE;
        if (m_ChannelSubSection.size() > 0)
        {
            SettingsPerChannel_UnregisterSection(m_ChannelSubSection.c_str());
        }
    }
}

int CSAA7134Source::GetDeviceIndex()
{
    return m_DeviceIndex;
}

const char* CSAA7134Source::GetChipName()
{
    return m_ChipName.c_str();
}

int  CSAA7134Source::NumInputs(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_pSAA7134Card->GetNumInputs();      
  }
/*  else if (InputType == AUDIOINPUT)
  {
      return m_pSAA7134Card->GetNumAudioInputs();      
  }*/
  return 0;
}

BOOL CSAA7134Source::SetInput(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      m_VideoSource->SetValue(Nr);
      return TRUE;
  }
/* else if (InputType == AUDIOINPUT)
  {      
      m_pSAA7134Card->SetAudioSource((eAudioInput)Nr);          
      return TRUE;      
  }*/
  return FALSE;
}

int CSAA7134Source::GetInput(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_VideoSource->GetValue();
  }
/*  else if (InputType == AUDIOINPUT)
  {
      return m_pSAA7134Card->GetAudioInput();    
  }*/
  return -1;
}

const char* CSAA7134Source::GetInputName(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      if ((Nr>=0) && (Nr < m_pSAA7134Card->GetNumInputs()) )
      {
          return m_pSAA7134Card->GetInputName(Nr);
      }
  } 
/*  else if (InputType == AUDIOINPUT)
  {      
      return m_pSAA7134Card->GetAudioInputName((eAudioInput)Nr);
  }*/
  return NULL;
}

BOOL CSAA7134Source::InputHasTuner(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        return m_pSAA7134Card->IsInputATuner(Nr);
    }
    else
    {
        return FALSE;
    }
  }
  return FALSE;
}
