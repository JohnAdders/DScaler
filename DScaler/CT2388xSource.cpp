/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xSource.cpp,v 1.3 2002-09-16 19:34:19 adcockj Exp $
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
// Revision 1.2  2002/09/15 14:20:38  adcockj
// Fixed timing problems for cx2388x chips
//
// Revision 1.1  2002/09/11 18:19:37  adcockj
// Prelimainary support for CT2388x based cards
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CT2388xSource.h"
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
#include "CT2388x_Defines.h"
#include "FD_60Hz.h"
#include "FD_50Hz.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "SettingsPerChannel.h"

extern long EnableCancelButton;

void CT2388x_OnSetup(void *pThis, int Start)
{
   if (pThis != NULL)
   {
      ((CCT2388xSource*)pThis)->SavePerChannelSetup(Start);
   }
}


CCT2388xSource::CCT2388xSource(CCT2388xCard* pCard, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], LPCSTR IniSection) :
    CSource(WM_CT2388X_GETVALUE, IDC_CT2388X),
    m_pCard(pCard),
    m_CurrentX(720),
    m_CurrentY(480),
    m_Section(IniSection),
    m_IDString(IniSection),
    m_IsFieldOdd(FALSE),
    m_InSaturationUpdate(FALSE),
    m_CurrentChannel(-1),
    m_SettingsByChannelStarted(FALSE),
    m_NumFields(10)
{
    CreateSettings(IniSection);

    SettingsPerChannel_RegisterOnSetup(this, CT2388x_OnSetup);

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
    }

    SetupCard();
    Reset();

    NotifyInputChange(0, VIDEOINPUT, -1, m_VideoSource->GetValue());
}

CCT2388xSource::~CCT2388xSource()
{
    delete m_pCard;
}

void CCT2388xSource::SetupPictureStructures()
{
    if(m_IsVideoProgressive->GetValue())
    {
        // Set up 5 sets of pointers to the start of pictures
        for (int j(0); j < 5; j++)
        {
            m_EvenFields[j].pData = m_pDisplay[j];
            m_EvenFields[j].Flags = PICTURE_PROGRESSIVE;
            m_EvenFields[j].IsFirstInSeries = FALSE;
        }
    }
    else
    {
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
    }
}

void CCT2388xSource::CreateSettings(LPCSTR IniSection)
{
    m_Brightness = new CBrightnessSetting(this, "Brightness", 128, 0, 255, IniSection);
    m_Settings.push_back(m_Brightness);

    m_Contrast = new CContrastSetting(this, "Contrast", 128, 0, 255, IniSection);
    m_Settings.push_back(m_Contrast);

    m_Hue = new CHueSetting(this, "Hue", 128, 0, 255, IniSection);
    m_Settings.push_back(m_Hue);

    m_Saturation = new CSaturationSetting(this, "Saturation", 128, 0, 255, IniSection);
    m_Settings.push_back(m_Saturation);

    m_SaturationU = new CSaturationUSetting(this, "Blue Saturation", 128, 0, 255, IniSection);
    m_Settings.push_back(m_SaturationU);

    m_SaturationV = new CSaturationVSetting(this, "Red Saturation", 128, 0, 255, IniSection);
    m_Settings.push_back(m_SaturationV);

    m_Overscan = new COverscanSetting(this, "Overscan", DEFAULT_OVERSCAN_NTSC, 0, 150, IniSection);
    m_Settings.push_back(m_Overscan);

    m_VideoSource = new CVideoSourceSetting(this, "Video Source", 0, 0, 6, IniSection);
    m_Settings.push_back(m_VideoSource);

    m_VideoFormat = new CVideoFormatSetting(this, "Video Format", VIDEOFORMAT_NTSC_M, 0, VIDEOFORMAT_LASTONE - 1, IniSection);
    m_Settings.push_back(m_VideoFormat);

    m_CardType = new CSliderSetting("Card Type", CT2388xCARD_UNKNOWN, CT2388xCARD_UNKNOWN, CT2388xCARD_LASTONE - 1, IniSection, "CardType");
    m_Settings.push_back(m_CardType);

    m_TunerType = new CTunerTypeSetting(this, "Tuner Type", TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, IniSection);
    m_Settings.push_back(m_TunerType);

    m_bSavePerInput = new CYesNoSetting("Save Per Input", FALSE, IniSection, "SavePerInput");
    m_Settings.push_back(m_bSavePerInput);
    
    m_bSavePerFormat = new CYesNoSetting("Save Per Format", TRUE, IniSection, "SavePerFormat");
    m_Settings.push_back(m_bSavePerFormat);
    
    m_bSavePerChannel = new CYesNoSetting("Save Per Channel", FALSE, IniSection, "SavePerChannel");
    m_Settings.push_back(m_bSavePerChannel);

    m_IsVideoProgressive = new CIsVideoProgressiveSetting(this, "Is Video Progressive", TRUE, IniSection);
    m_Settings.push_back(m_IsVideoProgressive);

    ReadFromIni();
}


void CCT2388xSource::Start()
{
    m_pCard->StopCapture();
    CreateRiscCode();
    m_pCard->StartCapture(m_RiscBasePhysical);
    Timing_Reset();
    NotifySizeChange();
    NotifySquarePixelsCheck();
}

void CCT2388xSource::Reset()
{
    m_pCard->ResetHardware();
    m_pCard->SetVideoSource(m_VideoSource->GetValue());

    m_pCard->SetBrightness(m_Brightness->GetValue());
    m_pCard->SetContrast(m_Contrast->GetValue());
    m_pCard->SetHue(m_Hue->GetValue());
    m_pCard->SetSaturationU(m_SaturationU->GetValue());
    m_pCard->SetSaturationV(m_SaturationV->GetValue());

    m_CurrentX = 720;
    m_pCard->SetGeoSize(
                                m_VideoSource->GetValue(), 
                                (eVideoFormat)m_VideoFormat->GetValue(), 
                                m_CurrentX, 
                                m_CurrentY, 
                                0, 
                                0,
                                m_IsVideoProgressive->GetValue()
                            );
    
    NotifySizeChange();
}

void CCT2388xSource::CreateRiscCode()
{
    DWORD *pRiscCode;    // For host memory version
    int nField;
    int nLine;
    LPBYTE pUser;
    DWORD pPhysical;
    DWORD GotBytesPerLine;
    DWORD BytesPerLine = 0;
    int NumLines;
    DWORD BytesToSkip;
    BOOL IsVideo480P = m_IsVideoProgressive->GetValue();

    SetupPictureStructures();

    pRiscCode = (DWORD*)m_RiscBaseLinear;

    // we create the RISC code for 10 fields
    // the first one (0) is even
    // last one (9) is odd
    if(IsVideo480P)
    {
        if(m_CurrentY == 576)
        {
            m_NumFields = 4;
        }
        else
        {
            m_NumFields = 5;
        }
    }
    else
    {
        m_NumFields = 10;
    }
    for (nField = 0; nField < m_NumFields; nField++)
    {
        DWORD Instruction(0);

        if(IsVideo480P)
        {
            Instruction = RISC_RESYNC;
        }
        else
        {
            // First we sync onto either the odd or even field (odd only for 480p)
            if (nField & 1)
            {
                Instruction = RISC_RESYNC_EVEN;
            }
            else
            {
                Instruction = RISC_RESYNC_ODD;
            }
        }

        // maintain counter that we use to tell us where we are
        // in the RISC code
        if(nField == 0)
        {
            Instruction |= RISC_CNT_RESET;
        }
        else
        {
            Instruction |= RISC_CNT_INC;
        }

        *(pRiscCode++) = Instruction;

        // work out the position of the first line
        // first line is line zero an even line
        if (IsVideo480P)
        {
            pUser = m_pDisplay[nField];
        }
        else
        {
            pUser = m_pDisplay[nField / 2];
            if(nField & 1)
            {
                pUser += 2048;
            }
        }
        BytesPerLine = m_CurrentX * 2;

        //
        // For 480p, we will do the full frame in sequential order. For 480i, we
        // do one (even or odd) field at a time and skip over the interlaced lines
        //
        NumLines = (IsVideo480P ? m_CurrentY : (m_CurrentY / 2));
        BytesToSkip = (IsVideo480P ? 2048 : 4096);

        for (nLine = 0; nLine < NumLines; nLine++)
        {
            if (IsVideo480P)
            {
                pPhysical = m_DisplayDMAMem[nField]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            }
            else
            {
                pPhysical = m_DisplayDMAMem[nField / 2]->TranslateToPhysical(pUser, BytesPerLine, &GotBytesPerLine);
            }
            if(pPhysical == 0 || BytesPerLine > GotBytesPerLine)
            {
                ErrorBox("GetPhysicalAddress failed during RISC build");
                return;
            }
            *(pRiscCode++) = RISC_WRITE | RISC_SOL | RISC_EOL | BytesPerLine;
            *(pRiscCode++) = pPhysical;
            // since we are doing all the lines of the same
            // polarity at the same time we skip two lines
            pUser += BytesToSkip;
        }
    }

    m_BytesPerRISCField = ((DWORD)pRiscCode - (DWORD)m_RiscBaseLinear) / m_NumFields;

    *(pRiscCode++) = RISC_JUMP;

    *(pRiscCode++) = m_RiscBasePhysical; 

    m_pCard->SetRISCStartAddress(m_RiscBasePhysical);
}


void CCT2388xSource::Stop()
{
    // stop capture
    m_pCard->StopCapture();
}

void CCT2388xSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    static long RepeatCount = 0;

    if(m_IsVideoProgressive->GetValue())
    {
        if(AccurateTiming)
        {
            GetNextFieldAccurateProg(pInfo);
        }
        else
        {
            GetNextFieldNormalProg(pInfo);
        }
    }
    else
    {
        if(AccurateTiming)
        {
            GetNextFieldAccurate(pInfo);
        }
        else
        {
            GetNextFieldNormal(pInfo);
        }
    }

    if (!pInfo->bRunningLate)
    {
    }

    Shift_Picture_History(pInfo);

    if(m_IsVideoProgressive->GetValue())
    {
        Replace_Picture_In_History(pInfo, 0, &m_EvenFields[pInfo->CurrentFrame]);

        pInfo->LineLength = m_CurrentX * 2;
        pInfo->FrameWidth = m_CurrentX;
        pInfo->FrameHeight = m_CurrentY;
        pInfo->FieldHeight = m_CurrentY;
        pInfo->InputPitch = 2048;
    }
    else
    {
        if(m_IsFieldOdd)
        {
            Replace_Picture_In_History(pInfo, 0, &m_OddFields[pInfo->CurrentFrame]);
        }
        else
        {
            Replace_Picture_In_History(pInfo, 0, &m_EvenFields[pInfo->CurrentFrame]);
        }

        pInfo->LineLength = m_CurrentX * 2;
        pInfo->FrameWidth = m_CurrentX;
        pInfo->FrameHeight = m_CurrentY;
        pInfo->FieldHeight = m_CurrentY / 2;
        pInfo->InputPitch = 4096;
    }

    Timing_IncrementUsedFields();

    // auto input detect
    Timimg_AutoFormatDetect(pInfo);

}

int CCT2388xSource::GetWidth()
{
    return m_CurrentX;
}

int CCT2388xSource::GetHeight()
{
    return m_CurrentY;
}


CCT2388xCard* CCT2388xSource::GetCard()
{
    return m_pCard;
}

LPCSTR CCT2388xSource::GetStatus()
{
    static LPCSTR pRetVal = "";
    pRetVal = m_pCard->GetInputName(m_VideoSource->GetValue());
    return pRetVal;
}

eVideoFormat CCT2388xSource::GetFormat()
{
    return (eVideoFormat)m_VideoFormat->GetValue();
}

void CCT2388xSource::SetFormat(eVideoFormat NewFormat)
{
    PostMessage(hWnd, WM_CT2388X_SETVALUE, CT2388XTVFORMAT, NewFormat);
}


ISetting* CCT2388xSource::GetBrightness()
{
    return m_Brightness;
}

ISetting* CCT2388xSource::GetContrast()
{
    return m_Contrast;
}

ISetting* CCT2388xSource::GetHue()
{
    return m_Hue;
}

ISetting* CCT2388xSource::GetSaturation()
{
    return m_Saturation;
}

ISetting* CCT2388xSource::GetSaturationU()
{
    if(m_CardType->GetValue() != CT2388xCARD_HOLO3D)
    {
        return m_SaturationU;
    }
    else
    {
        return NULL;
    }
}

ISetting* CCT2388xSource::GetSaturationV()
{
    if(m_CardType->GetValue() != CT2388xCARD_HOLO3D)
    {
        return m_SaturationV;
    }
    else
    {
        return NULL;
    }
}

ISetting* CCT2388xSource::GetOverscan()
{
    return m_Overscan;
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
void CCT2388xSource::GetNextFieldNormal(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;

    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
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
        Free_Picture_History(pInfo);
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
void CCT2388xSource::GetNextFieldNormalProg(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;

    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        // need to sleep more often
        // so that we don't take total control of machine
        // in normal operation
        Timing_SmartSleep(pInfo, FALSE, bSlept);
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }

    Diff = (m_NumFields + NewPos - OldPos) % m_NumFields;
    if(Diff > 1)
    {
        // delete all history
        Free_Picture_History(pInfo);
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

    pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
}

void CCT2388xSource::GetNextFieldAccurate(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame * 2 + m_IsFieldOdd + 1) % 10;
    static int FieldCount(0);
    
    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
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
        Free_Picture_History(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
        Timing_Reset();
        FieldCount = 0;
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
    
    FieldCount += Diff;
    // do input frequency on cleanish field changes only
    if(Diff == 1 && FieldCount > 1)
    {
        Timing_UpdateRunningAverage(pInfo, FieldCount);
        FieldCount = 0;
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}


void CCT2388xSource::GetNextFieldAccurateProg(TDeinterlaceInfo* pInfo)
{
    BOOL bSlept = FALSE;
    int NewPos;
    int Diff;
    int OldPos = (pInfo->CurrentFrame + 1) % m_NumFields;
	static int FieldCount(0);
    
    while(OldPos == (NewPos = m_pCard->GetRISCPos()))
    {
        pInfo->bRunningLate = FALSE;            // if we waited then we are not late
    }


    Diff = (m_NumFields + NewPos - OldPos) % m_NumFields;
    if(Diff == 1)
    {
    }
    else if(Diff == 2) 
    {
        NewPos = (OldPos + 1) % m_NumFields;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Slightly late");
    }
    else if(Diff == 3) 
    {
        NewPos = (OldPos + 1) % m_NumFields;
        Timing_SetFlipAdjustFlag(TRUE);
        LOG(2, " Very late");
    }
    else
    {
        // delete all history
        Free_Picture_History(pInfo);
        pInfo->bMissedFrame = TRUE;
        Timing_AddDroppedFields(Diff - 1);
        LOG(2, " Dropped Frame");
        Timing_Reset();
        FieldCount = 0;
    }

	pInfo->CurrentFrame = (NewPos + m_NumFields - 1) % m_NumFields;
    
    FieldCount += Diff;
    // do input frequency on cleanish field changes only
    if(Diff == 1 && FieldCount > 1)
    {
        Timing_UpdateRunningAverage(pInfo, FieldCount);
        FieldCount = 0;
    }

    Timing_SmartSleep(pInfo, pInfo->bRunningLate, bSlept);
}

void CCT2388xSource::VideoSourceOnChange(long NewValue, long OldValue)
{
    NotifyInputChange(1, VIDEOINPUT, OldValue, NewValue);

    Stop_Capture();
    SaveInputSettings(TRUE);
    LoadInputSettings();
    Reset();

    NotifyInputChange(0, VIDEOINPUT, OldValue, NewValue);
    // set up sound
    if(m_pCard->IsInputATuner(NewValue))
    {
        Channel_SetCurrent();
    }
    Start_Capture();
}

void CCT2388xSource::VideoFormatOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    SaveInputSettings(TRUE);
    LoadInputSettings();
    Reset();
    Start_Capture();
}

void CCT2388xSource::BrightnessOnChange(long Brightness, long OldValue)
{
    m_pCard->SetBrightness(Brightness);
}

void CCT2388xSource::HueOnChange(long Hue, long OldValue)
{
    m_pCard->SetHue(Hue);
}

void CCT2388xSource::ContrastOnChange(long Contrast, long OldValue)
{
    m_pCard->SetContrast(Contrast);
}

void CCT2388xSource::SaturationUOnChange(long SatU, long OldValue)
{
    m_pCard->SetSaturationU(SatU);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatU + m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatU - m_SaturationV->GetValue()) / 2);
        m_Saturation->SetMax(255 - abs(SatU - m_SaturationV->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CCT2388xSource::SaturationVOnChange(long SatV, long OldValue)
{
    m_pCard->SetSaturationV(SatV);
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        m_Saturation->SetValue((SatV + m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMin(abs(SatV - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(255 - abs(SatV - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}


void CCT2388xSource::SaturationOnChange(long Sat, long OldValue)
{
    if(m_InSaturationUpdate == FALSE)
    {
        m_InSaturationUpdate = TRUE;
        long NewSaturationU = m_SaturationU->GetValue() + (Sat - OldValue);
        long NewSaturationV = m_SaturationV->GetValue() + (Sat - OldValue);
        m_SaturationU->SetValue(NewSaturationU);
        m_SaturationV->SetValue(NewSaturationV);
        m_Saturation->SetMin(abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_Saturation->SetMax(255 - abs(m_SaturationV->GetValue() - m_SaturationU->GetValue()) / 2);
        m_InSaturationUpdate = FALSE;
    }
}

void CCT2388xSource::OverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CCT2388xSource::TunerTypeOnChange(long TunerId, long OldValue)
{
    m_pCard->InitTuner((eTunerId)TunerId);
}


BOOL CCT2388xSource::IsInTunerMode()
{
    return m_pCard->IsInputATuner(m_VideoSource->GetValue());
}


void CCT2388xSource::SetupCard()
{
    if(m_CardType->GetValue() == CT2388xCARD_UNKNOWN)
    {
        // try to detect the card
        m_CardType->SetValue(m_pCard->AutoDetectCardType());
        m_TunerType->SetValue(m_pCard->AutoDetectTuner((eCT2388xCardId)m_CardType->GetValue()));

        // then display the hardware setup dialog
        EnableCancelButton = 0;
        DialogBoxParam(hResourceInst, MAKEINTRESOURCE(IDD_SELECTCARD), hWnd, (DLGPROC) SelectCardProc, (LPARAM)this);
        EnableCancelButton = 1;

    }
    m_pCard->SetCardType(m_CardType->GetValue());
    m_pCard->InitTuner((eTunerId)m_TunerType->GetValue());
}

void CCT2388xSource::ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff)
{
}

void CCT2388xSource::ChangeTVSettingsBasedOnTuner()
{
    // default the TVTYPE dependant on the Tuner selected
    // should be OK most of the time
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        eVideoFormat videoFormat = m_pCard->GetTuner()->GetDefaultVideoFormat();
        m_VideoFormat->ChangeDefault(videoFormat);
    }
}

BOOL CCT2388xSource::HasTuner()
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


BOOL CCT2388xSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    if(VideoFormat == VIDEOFORMAT_LASTONE)
    {
        VideoFormat = m_pCard->GetTuner()->GetDefaultVideoFormat();
    }
    if(VideoFormat != m_VideoFormat->GetValue())
    {
        m_VideoFormat->SetValue(VideoFormat);
    }
    return m_pCard->GetTuner()->SetTVFrequency(FrequencyId, VideoFormat);
}

BOOL CCT2388xSource::IsVideoPresent()
{
    return m_pCard->IsVideoPresent();
}


void CCT2388xSource::DecodeVBI(TDeinterlaceInfo* pInfo)
{
}

eTunerId CCT2388xSource::GetTunerId()
{
    return m_pCard->GetTuner()->GetTunerId();
}

LPCSTR CCT2388xSource::GetMenuLabel()
{
    return m_pCard->GetCardName(m_pCard->GetCardType());
}

void CCT2388xSource::SetOverscan()
{
    AspectSettings.InitialOverscan = m_Overscan->GetValue();
}

void CCT2388xSource::SavePerChannelSetup(int Start)
{
    if (Start)
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

void CCT2388xSource::HandleTimerMessages(int TimerId)
{
}

void CCT2388xSource::IsVideoProgressiveOnChange(long NewValue, long OldValue)
{
    Stop_Capture();
    Reset();
    Start_Capture();
}


int  CCT2388xSource::NumInputs(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_pCard->GetNumInputs();      
  }
  return 0;
}

BOOL CCT2388xSource::SetInput(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      m_VideoSource->SetValue(Nr);
      return TRUE;
  }
  return FALSE;
}

int CCT2388xSource::GetInput(eSourceInputType InputType)
{
  if (InputType == VIDEOINPUT)
  {
      return m_VideoSource->GetValue();
  }
  return -1;
}

const char* CCT2388xSource::GetInputName(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
      if ((Nr>=0) && (Nr < m_pCard->GetNumInputs()) )
      {
          return m_pCard->GetInputName(Nr);
      }
  } 
  return NULL;
}

BOOL CCT2388xSource::InputHasTuner(eSourceInputType InputType, int Nr)
{
  if (InputType == VIDEOINPUT)
  {
    if(m_TunerType->GetValue() != TUNER_ABSENT)
    {
        return m_pCard->IsInputATuner(Nr);
    }
    else
    {
        return FALSE;
    }
  }
  return FALSE;
}
