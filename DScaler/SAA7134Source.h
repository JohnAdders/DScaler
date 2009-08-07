/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file saa7134source.h saa7134source Header file
 */

#ifdef WANT_SAA713X_SUPPORT

#ifndef __SAA7134SOURCE_H___
#define __SAA7134SOURCE_H___

#include "Source.h"
#include "SAA7134Card.h"
#include "SAA7134Common.h"
#include "HardwareMemory.h"
#include "Setting.h"
/// \todo get rid of dependencies below here
#include "IOutput.h"

/** The source controls a specific CSAA7134Card to provide interlaced video
*/
class CSAA7134Source : public CSource,
                       public CSAA7134Common
{
    //DECLARE_CLASS_SETTINGS(CSAA7134Source);
public:
    CSAA7134Source(SmartPtr<CSAA7134Card> pSAA7134Card, SmartPtr<CContigMemory> PagelistDMAMem[kMAX_PAGETABLES], SmartPtr<CUserMemory> DisplayDMAMem[kMAX_FRAMEBUFFERS], SmartPtr<CUserMemory> VBIDMAMem[kMAX_FRAMEBUFFERS], LPCTSTR IniSection, LPCTSTR ChipName, int DeviceIndex);
    ~CSAA7134Source();

    void Start();
    void Stop();
    void Reset();

    void Mute();
    void UnMute();

    SmartPtr<CSAA7134Card> GetCard();

    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    void DecodeVBI(TDeinterlaceInfo* pInfo);

    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);

    tstring GetStatus();
    BOOL IsInTunerMode();
    BOOL IsVideoPresent();

    SmartPtr<ITuner> GetTuner();

    void SetFormat(eVideoFormat NewFormat);
    eVideoFormat GetFormat();

    void SetAspectRatioData();

    int GetInitialWidth() {return GetWidth();};
    int GetInitialHeight() {return GetHeight() / 2;};

    int GetWidth();
    int GetHeight();
    void SetWidth(int w);

    int GetDeviceIndex();
    tstring GetChipName();
    tstring IDString() { return m_IDString; }

    int NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    tstring GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    CSliderSetting* GetVolume();
    CSliderSetting* GetBalance();

    CSliderSetting* GetBrightness();
    CSliderSetting* GetContrast();
    CSliderSetting* GetHue();
    CSliderSetting* GetSaturation();
    CSliderSetting* GetSaturationU();
    CSliderSetting* GetSaturationV();
    CYesNoSetting* GetAnalogueBlanking() {return NULL;};
    CSliderSetting* GetTopOverscan();
    CSliderSetting* GetBottomOverscan();
    CSliderSetting* GetLeftOverscan();
    CSliderSetting* GetRightOverscan();
    CSliderSetting* GetHDelay();
    CSliderSetting* GetVDelay();

    tstring GetMenuLabel();
    void SetMenu(HMENU hMenu);
    void UpdateMenu() {return;};

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    BOOL HandleWindowsCommands(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void HandleTimerMessages(int TimerId);

    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff);

    BOOL OpenMediaFile(const tstring& FileName, BOOL NewPlayList) {return FALSE;};
    BOOL IsAccessAllowed() {return TRUE;};
    BOOL HasSquarePixels() {return FALSE;};

    SmartPtr<CSettingsHolder> GetSettingsPage();

    void SetSourceAsCurrent();

    void Pause() {return;};
    void UnPause() {return;};

    BOOL HasMediaControl() {return FALSE;};

    BOOL IsAudioMixerAccessAllowed() {return TRUE;};

    BOOL IsInitialSetup() {return m_InitialSetup;};

private:
    virtual void CreateSettings(LPCTSTR IniSection);

    void SetupCard();

    void SetupVideoSource();
    void SetupAudioSource();
    void SetupVideoStandard();
    void SetupAudioStandard();
    void UpdateAudioStatus();

    void SetupDMAMemory();
    DWORD CreatePageTable(CUserMemory* pDMAMemory, DWORD nPagesWanted, LPDWORD pPageTable);

    TPicture* GetFieldBuffer(TFieldID FieldID);
    void GetFrameIndex(TFieldID FieldID, BYTE* pFrameIndex, BOOL* pIsFieldOdd);

    void GiveNextField(TDeinterlaceInfo* pInfo, TPicture* picture);
    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);

    BOOL PollForNextField(TFieldID* pNextFieldID, int* pFieldDistance, BOOL bSmartSleep);

    void InitializeSmartSleep();
    void PerformSmartSleep(ULONGLONG* pPerformanceTick, ULONGLONG* pTimePassed);
    void UpdateSmartSleep(BOOL bRecalculate, ULONGLONG LastTick, ULONGLONG TimePassed);

    void InitializeUI();

    static INT_PTR CALLBACK SelectCardProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK RegisterEditProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK AudioStandardProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK OtherEditProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    void ChangeDefaultsForVideoInput(BOOL bDontSetValue);
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue);
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue);
    void ChangeTVSettingsBasedOnTuner();

protected:
    HINSTANCE       m_hSAA7134ResourceInst;

private:
    SmartPtr<CSAA7134Card> m_pSAA7134Card;
    SmartPtr<CUserMemory> m_VBIDMAMem[kMAX_FRAMEBUFFERS];
    SmartPtr<CUserMemory> m_DisplayDMAMem[kMAX_FRAMEBUFFERS];
    BYTE*           m_pDisplay[kMAX_FRAMEBUFFERS];
    BYTE*           m_pVBILines[kMAX_FRAMEBUFFERS];
    DWORD           m_DisplayPageTablePhysical[kMAX_VID_PAGETABLES];
    DWORD*          m_DisplayPageTableLinear[kMAX_VID_PAGETABLES];
    DWORD           m_VBIPageTablePhysical[kMAX_VBI_PAGETABLES];
    DWORD*          m_VBIPageTableLinear[kMAX_VBI_PAGETABLES];
    TPicture        m_EvenFields[kMAX_FRAMEBUFFERS];
    TPicture        m_OddFields[kMAX_FRAMEBUFFERS];
    long            m_CurrentX;
    long            m_CurrentY;
    long            m_CurrentVBILines;
    BOOL            m_InSaturationUpdate;
    tstring         m_ChannelSubSection;
    tstring         m_ChipName;
    int             m_DeviceIndex;

    tstring         m_Section;
    tstring         m_IDString;

    eAudioChannel   m_DetectedAudioChannel;

    DWORD           m_ChannelChangeTick;

    ULONGLONG       m_PerformanceFrequency;
    // Used to time field to field tick delay
    ULONGLONG       m_LastFieldPerformanceCount;
    // Used to store current field to field microsecond delay
    ULONGLONG       m_MinimumFieldDelay;

    // The field the card is currently processing
    TFieldID        m_ProcessingFieldID;
    // The field that is currently in use
    TFieldID        m_CurrentFieldID;

    BOOL            m_bSelectCardCancelButton;

    SmartPtr<CSliderSetting> m_CustomPixelWidth;
    SmartPtr<CYesNoSetting>  m_ReversePolarity;
    SmartPtr<CYesNoSetting>  m_VBIDebugOverlay;
    SmartPtr<CSliderSetting> m_CardType;
    SmartPtr<CStringSetting> m_CardName;

    BOOL            m_InitialSetup;

    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, TopOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, BottomOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, LeftOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, RightOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, PixelWidth);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VideoFormat);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, TunerType);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, HPLLMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Volume);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Bass);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Treble);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Balance);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioStandard);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioChannel);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSampleRate);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, VBIUpscaleDivisor);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, GainControlLevel);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, GammaLevel);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  AutomaticGainControl);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  VideoMirror);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  AutoStereoSelect);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  WhitePeak);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  ColorPeak);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  CustomAudioStandard);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source,  GammaControl);
    DEFINE_LIST_CALLBACK_SETTING(CSAA7134Source,   AdaptiveCombFilter);
    DEFINE_LIST_CALLBACK_SETTING(CSAA7134Source,   AutomaticVolumeLevel);
    DEFINE_LIST_CALLBACK_SETTING(CSAA7134Source,   AudioLine1Voltage);
    DEFINE_LIST_CALLBACK_SETTING(CSAA7134Source,   AudioLine2Voltage);

    // These settings are only effective when CustomAudioStandard is set
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMajorCarrier);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMinorCarrier);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMajorCarrierMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioMinorCarrierMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioCh1FMDeemph);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioCh2FMDeemph);
};

#endif

#endif//xxx

