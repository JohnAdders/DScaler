/////////////////////////////////////////////////////////////////////////////
// $Id: CT2388xSource.h,v 1.9 2002-10-22 04:08:50 flibuste2 Exp $
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

#ifndef __CT2388XSOURCE_H___
#define __CT2388XSOURCE_H___

#include "Source.h"
#include "CT2388xCard.h"
#include "HardwareMemory.h"
#include "Setting.h"
#include "CT2388x_Defines.h"
/// \todo get rid of dependencies below here
#include "Other.h"

/** The source controls a specific CCT2388xCard to provide interlaced video
*/
class CCT2388xSource : public CSource
{
    DECLARE_CLASS_SETTINGS(CCT2388xSource);
public:
    CCT2388xSource(CCT2388xCard* pCard, CContigMemory* RiscDMAMem, CUserMemory* DisplayDMAMem[5], LPCSTR IniSection);
    ~CCT2388xSource();
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    CCT2388xCard* GetCard();
    LPCSTR GetStatus();
    ISetting* GetVolume() {return NULL;};;
    ISetting* GetBalance() {return NULL;};;
    void Mute() {};
    void UnMute() {};
    ISetting* GetBrightness();
    ISetting* GetContrast();
    ISetting* GetHue();
    ISetting* GetSaturation();
    ISetting* GetSaturationU();
    ISetting* GetSaturationV();
    ISetting* GetOverscan();
    eVideoFormat GetFormat();
    void SetFormat(eVideoFormat NewFormat);
    BOOL IsInTunerMode();
    int GetWidth();
    int GetHeight();
    void UpdateMenu() {return;};
    void SetMenu(HMENU hMenu);
  
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
    void SavePerChannelSetup(int Start);
    LPCSTR IDString() { return m_IDString.c_str(); }

    int  NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    const char* GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    ITuner* GetTuner();

private:
    virtual void CreateSettings(LPCSTR IniSection);

    void CreateRiscCode();
    
    static BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);
    void GetNextFieldNormalProg(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurateProg(TDeinterlaceInfo* pInfo);
    void SmartSleep(TDeinterlaceInfo* pInfo, BOOL bRunningLate);

    void SetupCard();
    void ChangeTVSettingsBasedOnTuner();
    void ChangeSectionNamesForInput();
    void ChangeDefaultsForInput();
    void LoadInputSettings();
    void SaveInputSettings(BOOL bOptimizeFileAccess);
    void SetupPictureStructures();

    void ChangeChannelSectionNames();
    void ChangeDefaultsForCard();

    ISetting* GetCurrentAudioSetting();

private:
    CCT2388xCard*  m_pCard;
    BYTE*        m_pDisplay[5];
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
    int          m_NumFields;

    std::string  m_Section;
    std::string  m_IDString;

    CSliderSetting* m_CardType;
    CYesNoSetting* m_bSavePerInput;
    CYesNoSetting* m_bSavePerFormat;
    CYesNoSetting* m_bSavePerChannel;

    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, SaturationU);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, SaturationV);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, Overscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, TunerType);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, VideoSource);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, VideoFormat);
    DEFINE_YESNO_CALLBACK_SETTING(CCT2388xSource, IsVideoProgressive);
	DEFINE_YESNO_CALLBACK_SETTING(CCT2388xSource, FLIFilmDetect);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, HDelay);
    DEFINE_SLIDER_CALLBACK_SETTING(CCT2388xSource, VDelay);
protected:
};




#endif