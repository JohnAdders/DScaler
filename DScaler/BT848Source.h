/////////////////////////////////////////////////////////////////////////////
// $Id: BT848Source.h,v 1.32 2002-08-26 18:25:10 adcockj Exp $
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
#include "Bt848_Defines.h"
/// \todo get rid of dependencies below here
#include "Other.h"

/** The source controls a specific CBT848Card to provide interlaced video
*/
class CBT848Source : public CSource
{
    DECLARE_CLASS_SETTINGS(CBT848Source);
public:
    CBT848Source(CBT848Card* pBT848Card, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], CUserMemory* VBIDMAMem[5], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex);
    ~CBT848Source();
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    CBT848Card* GetBT848Card();
    LPCSTR GetStatus();
    ISetting* GetVolume();
    ISetting* GetBalance();
    void Mute();
    void UnMute();
    ISetting* GetBrightness();
    ISetting* GetContrast();
    ISetting* GetHue();
    ISetting* GetSaturation();
    ISetting* GetSaturationU();
    ISetting* GetSaturationV();
    ISetting* GetOverscan();
    /// Gets the current field being processed by the card
    int GetRISCPosAsInt();
	eTunerId GetTunerId();
    eVideoFormat GetFormat();
    void SetFormat(eVideoFormat NewFormat);
    BOOL IsInTunerMode();
    int GetWidth();
    int GetHeight();
    void UpdateMenu() {return;};
    void SetMenu(HMENU hMenu);
    BOOL HasTuner();
    void HandleTimerMessages(int TimerId);
    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsVideoPresent();
    void DecodeVBI(TDeinterlaceInfo* pInfo);
    LPCSTR GetMenuLabel();
    BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList) {return FALSE;};
    BOOL IsAccessAllowed() {return TRUE;};
    void SetOverscan();
    BOOL HasSquarePixels() {return FALSE;};
    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff);
    void ChannelChange(int PreChange,int OldChannel,int NewChannel);
    void SavePerChannelSetup(int Start);
    int GetDeviceIndex();
    const char* GetChipName();
    LPCSTR IDString() { return m_IDString.c_str(); }

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
    void ChangeTVSettingsBasedOnTuner();
    void ChangeSectionNamesForInput();
    void ChangeDefaultsForInput();
    void LoadInputSettings();
    void SaveInputSettings(BOOL bOptimizeFileAccess);

    void ChangeChannelSectionNames();

    ISetting* GetCurrentAudioSetting();

private:
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
    BOOL         m_InSaturationUpdate;
    int          m_CurrentChannel;    
    std::string  m_ChannelSubSection;
    BOOL         m_SettingsByChannelStarted;
    std::string  m_ChipName;
    int          m_DeviceIndex;

    std::string  m_Section;
    std::string  m_IDString;

    CSliderSetting* m_CustomPixelWidth;
    CYesNoSetting* m_ReversePolarity;
    CSliderSetting* m_CardType;
    CYesNoSetting* m_bSavePerInput;
    CYesNoSetting* m_bSavePerFormat;
    CYesNoSetting* m_bSavePerChannel;

    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, SaturationU);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, SaturationV);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Overscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BtWhiteCrushUp);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, BtWhiteCrushDown);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, PixelWidth);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, VideoFormat);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, TunerType);
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
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Bass);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Treble);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, Balance);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, AutoStereoSelect);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource1);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource2);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource3);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource4);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource5);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioSource6);
    DEFINE_SLIDER_CALLBACK_SETTING(CBT848Source, AudioChannel);
    DEFINE_YESNO_CALLBACK_SETTING(CBT848Source, UseInputPin1);
protected:
	int m_InitialACPIStatus;
};




#endif