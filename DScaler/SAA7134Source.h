/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source.h,v 1.10 2002-10-06 09:49:19 atnak Exp $
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
// $Log: not supported by cvs2svn $
// Revision 1.9  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.8  2002/10/03 23:36:22  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.7  2002/09/28 13:33:04  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.6  2002/09/26 11:33:42  kooiman
// Use event collector
//
// Revision 1.5  2002/09/16 17:52:34  atnak
// Support for SAA7134Res.dll dialogs
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:24:03  atnak
// changed some UI stuff
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __SAA7134SOURCE_H___
#define __SAA7134SOURCE_H___

#include "Source.h"
#include "SAA7134Card.h"
#include "HardwareMemory.h"
#include "Setting.h"
#include "SAA7134_Defines.h"
/// \todo get rid of dependencies below here
#include "Other.h"

/** The source controls a specific CSAA7134Card to provide interlaced video
*/
class CSAA7134Source : public CSource,
                       public CSAA7134Common
{
    DECLARE_CLASS_SETTINGS(CSAA7134Source);
public:
    CSAA7134Source(CSAA7134Card* pSAA7134Card, CContigMemory* PagelistDMAMem[4], CUserMemory* DisplayDMAMem[2], CUserMemory* VBIDMAMem[2], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex);
    ~CSAA7134Source();
    void Start();
    void Stop();
    void Reset();
    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    CSAA7134Card* GetSAA7134Card();
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
    int  NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    const char* GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

private:
    virtual void CreateSettings(LPCSTR IniSection);

    void SetupVideoAudioSource();
    void SetupVideoAudioStandards();
    void SetupDMAMemory();

    DWORD CreatePageTable(CUserMemory* pDMAMemory, DWORD nPagesWanted, LPDWORD pPageTable);
    
    static BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY OtherEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void GetNextFieldNormal(TDeinterlaceInfo* pInfo);
    void GetNextFieldAccurate(TDeinterlaceInfo* pInfo);
    void SmartSleep(TDeinterlaceInfo* pInfo, BOOL bRunningLate);

    void UpdateAudioStatus();

    void SetupCard();
    void ChangeTVSettingsBasedOnCard();
    void ChangeTVSettingsBasedOnTuner();
    void ChangeSectionNamesForInput();
    void ChangeDefaultsForInput();
    void LoadInputSettings();
    void SaveInputSettings(BOOL bOptimizeFileAccess);

    void ChangeChannelSectionNames();

    ISetting* GetCurrentAudioSetting();
    ISetting* GetAudioSetting(int nVideoSource);

    void GiveNextField(TDeinterlaceInfo* pInfo, TPicture* picture);

    BOOL GetFinishedField(eRegionID& DoneRegionID, BOOL& bDoneIsFieldOdd);
    BOOL WaitForFinishedField(eRegionID& RegionID, BOOL& bIsFieldOdd, TDeinterlaceInfo* pInfo);

    int EnumulateField(eRegionID RegionID, BOOL bIsFieldOdd);
    void DenumulateField(int Index, eRegionID& RegionID, BOOL& bIsFieldOdd);

    void InitializeUI();
    void CleanupUI();

private:
    CSAA7134Card*   m_pSAA7134Card;
    BYTE*           m_pDisplay[2];
    BYTE*           m_pVBILines[2];
    CUserMemory*    m_VBIDMAMem[2];
    CUserMemory*    m_DisplayDMAMem[2];
    DWORD           m_DisplayPageTablePhysical[2];
    DWORD*          m_DisplayPageTableLinear[2];
    DWORD           m_VBIPageTablePhysical[2];
    DWORD*          m_VBIPageTableLinear[2];
    TPicture        m_EvenFields[2];
    TPicture        m_OddFields[2];
    long            m_CurrentX;
    long            m_CurrentY;
    long            m_CurrentVBILines;
    BOOL            m_InSaturationUpdate;
    int             m_CurrentChannel;    
    std::string     m_ChannelSubSection;
    BOOL            m_SettingsByChannelStarted;
    std::string     m_ChipName;
    int             m_DeviceIndex;

    std::string     m_Section;
    std::string     m_IDString;

    int             m_CurrentFrame;
    BOOL            m_IsFieldOdd;
    int             m_LastFieldIndex;

    // Used to time field to field tick delay
    ULONGLONG       m_LastPerformanceCount;
    // Used to store current field to field microsecond delay
    ULONGLONG       m_MinimumFieldDelay;

    // The field the card is currently processing
    eRegionID       m_ProcessingRegionID;
    BOOL            m_IsProcessingFieldOdd;

    BOOL            m_bSelectCardCancelButton;

    CSliderSetting* m_CustomPixelWidth;
    CYesNoSetting*  m_ReversePolarity;
    CSliderSetting* m_CardType;
    CYesNoSetting*  m_bSavePerInput;
    CYesNoSetting*  m_bSavePerFormat;
    CYesNoSetting*  m_bSavePerChannel;

    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, Overscan);
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
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioStandard);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource1);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource2);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource3);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource4);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource5);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioSource6);
    DEFINE_SLIDER_CALLBACK_SETTING(CSAA7134Source, AudioChannel);
    DEFINE_YESNO_CALLBACK_SETTING(CSAA7134Source, AutoStereoSelect);
protected:
    int         m_InitialACPIStatus;
    HINSTANCE   m_hSAA7134ResourceInst;
};




#endif