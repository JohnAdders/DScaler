/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source.h,v 1.5 2001-11-22 13:32:03 adcockj Exp $
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

#ifndef __BT848SOURCE_H___
#define __BT848SOURCE_H___

#include "Source.h"
#include "BT848Card.h"
#include "HardwareMemory.h"
#include "Setting.h"
//TODO: get rid of dependencies below here
#include "Other.h"

class CBT848Source : public CSource
{
    DECLARE_CLASS_SETTINGS(CBT848Source);
public:
    CBT848Source(CBT848Card* pBT848Card, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection);
    ~CBT848Source();
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    CBT848Card* GetBT848Card();
    LPCSTR GetStatus();
    CSetting* GetVolume();
    CSetting* GetBalance();
    void Mute();
    void UnMute();
    CSetting* GetBrightness();
    CSetting* GetContrast();
    CSetting* GetHue();
    CSetting* GetSaturation();
    CSetting* GetSaturationU();
    CSetting* GetSaturationV();
    int GetRISCPosAsInt();
    eVideoFormat GetFormat();
    BOOL IsInTunerMode();
    int GetWidth();
    int GetHeight();
    void SetMenu(HMENU hMenu);
    BOOL HasTuner();
    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo);

private:
    virtual void CreateSettings(LPCSTR IniSection);
    void CreateRiscCode(BOOL bCaptureVBI);
    
    static BOOL APIENTRY AdvVideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY AudioSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY AudioSettingProc1(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);
    void SmartSleep(TDeinterlaceInfo* pInfo, BOOL bRunningLate);

    void SetupCard();
    void ChangeDefaultsBasedOnHardware();
    void ChangeTVSettingsBasedOnTuner();
    void ChangeSectionNamesForInput();
    void ChangeDefaultsForInput();
    void LoadInputSettings();
    void SaveInputSettings();

    CBT848Card*  m_pBT848Card;
    BYTE*        m_pDisplay[5];
    BYTE*        m_pVBILines[5];
    CUserMemory* m_VBIDMAMem[5];
    CUserMemory* m_DisplayDMAMem[5];
    DWORD        m_RiscBasePhysical; 
    DWORD*       m_RiscBaseLinear;
    long         m_BytesPerRISCField;
    TPicture     m_EvenFields[5];
    TPicture     m_OddFields[5];
    long         m_CurrentX;
    long         m_CurrentY;
    long         m_CurrentVBILines;
    BOOL         m_IsFieldOdd;

    std::string  m_Section;

    CSliderSetting* m_CustomPixelWidth;
    CYesNoSetting* m_ReversePolarity;
    CSliderSetting* m_CardType;
    CSliderSetting* m_TunerType;
    CSliderSetting* m_ProcessorSpeed;
    CSliderSetting* m_TradeOff;
    CYesNoSetting* m_bSavePerInput;
    CYesNoSetting* m_bSavePerFormat;
    CYesNoSetting* m_bSavePerChannel;

    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, SaturationU);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, SaturationV);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BtWhiteCrushUp);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BtWhiteCrushDown);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, PixelWidth);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VideoFormat);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtAgcDisable);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtCrush);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenChromaAGC);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddChromaAGC);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenLumaPeak);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddLumaPeak);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtFullLumaRange);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenLumaDec);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddLumaDec);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtEvenComb);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtOddComb);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtColorBars);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtGammaCorrection);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtVertFilter);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtHorFilter);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtCoring);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, BtColorKill);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Volume);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Spatial);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Loudness);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Bass);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Treble);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Balance);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Equalizer1);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Equalizer2);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Equalizer3);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Equalizer4);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Equalizer5);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, SuperBass);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, AutoStereoSelect);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, MSPMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, MSPStereo);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, MSPMajorMode);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, MSPMinorMode);
};




#endif