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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file cx2388xsource.h  cx2388xsource Header file
 */

#ifdef WANT_CX2388X_SUPPORT

#ifndef __CX2388XSOURCE_H___
#define __CX2388XSOURCE_H___

#include "Source.h"
#include "CX2388xCard.h"
#include "HardwareMemory.h"
#include "Setting.h"
#include "CX2388x_Defines.h"
/// \todo get rid of dependencies below here
#include "IOutput.h"

/** The source controls a specific CCX2388xCard to provide interlaced video
*/
class CCX2388xSource : public CSource
{
    //DECLARE_CLASS_SETTINGS(CCX2388xSource);
public:
    CCX2388xSource(SmartPtr<CCX2388xCard> pCard, SmartPtr<CContigMemory> RiscDMAMem, SmartPtr<CUserMemory> DisplayDMAMem[5], SmartPtr<CUserMemory> VBIDMAMem[5], LPCTSTR IniSection);
    ~CCX2388xSource();
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, WPARAM wParam, LPARAM lParam);
    SmartPtr<CCX2388xCard> GetCard();
    tstring GetStatus();
    CSliderSetting* GetVolume() {return m_Volume;};
    CSliderSetting* GetBalance() {return m_Balance;};
    void Mute();
    void UnMute();
    CSliderSetting* GetBrightness();
    CSliderSetting* GetContrast();
    CSliderSetting* GetHue();
    CSliderSetting* GetSaturation();
    CSliderSetting* GetSaturationU();
    CSliderSetting* GetSaturationV();
    CYesNoSetting* GetAnalogueBlanking();
    CSliderSetting* GetTopOverscan();
    CSliderSetting* GetBottomOverscan();
    CSliderSetting* GetLeftOverscan();
    CSliderSetting* GetRightOverscan();
    CSliderSetting* GetHDelay();
    CSliderSetting* GetVDelay();
    eVideoFormat GetFormat();
    void SetFormat(eVideoFormat NewFormat);
    BOOL IsInTunerMode();
    int GetInitialWidth() {return GetWidth();};
    int GetInitialHeight() {return GetHeight() / 2;};
    int GetWidth();
    int GetHeight();
    void SetWidth(int w);
    void UpdateMenu() {return;};
    void SetMenu(HMENU hMenu);

    SmartPtr<CSettingsHolder> GetSettingsPage();

    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo);
    tstring GetMenuLabel();
    BOOL OpenMediaFile(const tstring& FileName, BOOL NewPlayList) {return FALSE;};
    BOOL IsAccessAllowed() {return TRUE;};
    void SetAspectRatioData();
    BOOL HasSquarePixels() {return FALSE;};
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff);
    tstring IDString() { return m_IDString; }

    int  NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    tstring GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    SmartPtr<ITuner> GetTuner();

    virtual void SetSourceAsCurrent();

    void Pause() {return;};
    void UnPause() {return;};

    BOOL HasMediaControl() {return FALSE;};

    BOOL IsAudioMixerAccessAllowed() {return TRUE;};

    BOOL IsInitialSetup() {return m_InitialSetup;};

private:
    virtual void CreateSettings(LPCTSTR IniSection);

    void CreateRiscCode(BOOL bCaptureVBI);

    static INT_PTR CALLBACK SelectCardProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);
    void GetNextFieldNormalProg(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurateProg(TDeinterlaceInfo* pInfo);
    void SmartSleep(TDeinterlaceInfo* pInfo, BOOL bRunningLate);

    void SetupCard();
    void SetupPictureStructures();
    void ChangeTVSettingsBasedOnTuner();
    void SetupSettings();

    CSliderSetting* GetCurrentAudioSetting();

    void InitializeUI();

    void StartUpdateAudioStatus();
    void StopUpdateAudioStatus();
    void UpdateAudioStatus();
    eSoundChannel AutoDetect_BGDK();
    eSoundChannel AutoDetect_I();
    BOOL StartStopConexantDriver(DWORD NewState);

private:
    SmartPtr<CCX2388xCard>  m_pCard;
    SmartPtr<CUserMemory> m_DisplayDMAMem[5];
    SmartPtr<CUserMemory> m_VBIDMAMem[5];
    BYTE*        m_pDisplay[5];
    BYTE*        m_pVBILines[5];
    DWORD        m_RiscBasePhysical;
    DWORD        m_RiscBasePhysicalVBI;
    DWORD*       m_RiscBaseLinear;
    long         m_BytesPerRISCField;
    TPicture     m_EvenFields[5];
    TPicture     m_OddFields[5];
    long         m_CurrentX;
    long         m_CurrentY;
    long         m_CurrentVBILines;
    BOOL         m_IsFieldOdd;
    BOOL         m_InSaturationUpdate;
    tstring  m_ChannelSubSection;
    int          m_NumFields;
    HINSTANCE    m_hCX2388xResourceInst;


    tstring  m_Section;
    tstring  m_IDString;

    CSliderSetting* m_CardType;
    CStringSetting* m_CardName;
    CYesNoSetting*  m_bSavePerInput;
    CYesNoSetting*  m_bSavePerFormat;
    CYesNoSetting*  m_bSavePerChannel;
    CSliderSetting* m_CustomPixelWidth;

    BOOL         m_InitialSetup;

    long         m_AutoDetectA2Counter;
    BOOL         m_bDriverStoped;

    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, SaturationU);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, SaturationV);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, AnalogueBlanking);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, TopOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, BottomOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, LeftOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, RightOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, TunerType);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, VideoFormat);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, IsVideoProgressive);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, FLIFilmDetect);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, VDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, EatLinesAtTop);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Sharpness);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, LumaAGC);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, ChromaAGC);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, FastSubcarrierLock);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, WhiteCrush);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, LowColorRemoval);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, CombFilter);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, FullLumaRange);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, Remodulation);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, Chroma2HComb);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, ForceRemodExcessChroma);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, IFXInterpolation);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, CombRange);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, SecondChromaDemod);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, ThirdChromaDemod);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, PixelWidth);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, WhiteCrushUp);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, WhiteCrushDown);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, WhiteCrushMajorityPoint);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, WhiteCrushPerFrame);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Volume);
    DEFINE_SLIDER_CALLBACK_SETTING(CCX2388xSource, Balance);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, AudioStandard);
    DEFINE_LIST_CALLBACK_SETTING(CCX2388xSource, StereoType);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, ConexantStopDriver);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, AutoMute);
    DEFINE_YESNO_CALLBACK_SETTING(CCX2388xSource, VerticalSyncDetection);

protected:
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue);
    void ChangeDefaultsForVideoInput(BOOL bDontSetValue);
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue) {};
};

#endif

#endif // WANT_CX2388X_SUPPORT
