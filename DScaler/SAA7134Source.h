/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Source.h,v 1.47 2005-05-18 12:18:32 robmuller Exp $
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
// Revision 1.46  2004/11/20 14:23:55  atnak
// Added SAA7134 card name setting for storing the card selection as text.
//
// Revision 1.45  2004/04/06 12:20:48  adcockj
// Added .NET 2003 project files and some fixes to support this
//
// Revision 1.44  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.43  2003/10/10 11:15:20  laurentg
// Bug fixed : access to the audio mixer
//
// Revision 1.42  2003/08/15 18:22:57  laurentg
// Save in the source if it is the first setup
//
// Revision 1.41  2003/08/11 20:45:55  laurentg
// Method to know if the source can be controled through a media player
//
// Revision 1.40  2003/07/22 22:31:36  laurentg
// Correct handling of pause (P key) for video file playing
//
// Revision 1.39  2003/07/05 10:55:57  laurentg
// New method SetWidth
//
// Revision 1.38  2003/01/27 22:04:15  laurentg
// First step to merge setup hardware and hardware info dialog boxes
// CPU flag information moved in the general hardware dialog box
// Hardware info dialog box available for CX2388x
//
// Revision 1.37  2003/01/19 19:46:27  laurentg
// GetInitialHeight must return the height of the input picture
//
// Revision 1.36  2003/01/19 11:09:11  laurentg
// New methods GetInitialWidth and GetInitialHeight to store the initial size before resizing in DScaler (for stills)
//
// Revision 1.35  2003/01/18 13:55:43  laurentg
// New methods GetHDelay and GetVDelay
//
// Revision 1.34  2003/01/18 10:52:11  laurentg
// SetOverscan renamed SetAspectRatioData
// Unnecessary call to SetOverscan deleted
// Overscan setting specific to calibration deleted
//
// Revision 1.33  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.32  2003/01/10 17:38:17  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.31  2003/01/08 19:59:38  laurentg
// Analogue Blanking setting by source
//
// Revision 1.30  2003/01/08 00:22:41  atnak
// Put back VBI upscale divisor
//
// Revision 1.29  2003/01/07 23:27:04  laurentg
// New overscan settings
//
// Revision 1.28  2003/01/07 22:59:58  atnak
// Removed variable upscale devisor and locked in at 0x200 scaling
// for 27Mhz VBI stepping
//
// Revision 1.27  2002/11/10 09:30:57  atnak
// Added Chroma only comb filter mode for SECAM
//
// Revision 1.26  2002/11/10 05:11:23  atnak
// Added adjustable audio input level
//
// Revision 1.25  2002/11/07 20:33:17  adcockj
// Promoted ACPI functions so that state management works properly
//
// Revision 1.24  2002/11/07 18:54:20  atnak
// Redid getting next field -- fixes some issues
//
// Revision 1.23  2002/10/31 05:39:02  atnak
// Added SoundChannel change event for toolbar
//
// Revision 1.22  2002/10/31 05:02:55  atnak
// Settings cleanup and audio tweaks
//
// Revision 1.21  2002/10/31 03:10:55  atnak
// Changed CSource::GetTreeSettingsPage to return CTreeSettingsPage*
//
// Revision 1.20  2002/10/29 03:07:18  atnak
// Added SAA713x TreeSettings Page
//
// Revision 1.19  2002/10/28 11:10:11  atnak
// Various changes and revamp to settings
//
// Revision 1.18  2002/10/26 04:42:50  atnak
// Added AGC config and automatic volume leveling control
//
// Revision 1.17  2002/10/23 17:05:18  atnak
// Added variable VBI sample rate scaling
//
// Revision 1.16  2002/10/22 04:08:50  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.15  2002/10/20 07:41:04  atnak
// custom audio standard setup + etc
//
// Revision 1.14  2002/10/12 20:03:12  atnak
// added half second wait for DecodeVBI() after channel change
//
// Revision 1.13  2002/10/12 01:37:28  atnak
// fixes negative dropped frames bug
//
// Revision 1.12  2002/10/08 20:35:39  atnak
// whitepeak, colorpeak, comb filter UI options
//
// Revision 1.11  2002/10/08 12:30:38  atnak
// tweaks and fixes
//
// Revision 1.10  2002/10/06 09:49:19  atnak
// Smarter GetNextField sleeping
//
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

/** 
 * @file saa7134source.h saa7134source Header file
 */
 
#ifndef __SAA7134SOURCE_H___
#define __SAA7134SOURCE_H___

#include "Source.h"
#include "SAA7134Card.h"
#include "SAA7134Common.h"
#include "HardwareMemory.h"
#include "Setting.h"
/// \todo get rid of dependencies below here
#include "Other.h"

/** The source controls a specific CSAA7134Card to provide interlaced video
*/
class CSAA7134Source : public CSource,
                       public CSAA7134Common
{
    //DECLARE_CLASS_SETTINGS(CSAA7134Source);
public:
    CSAA7134Source(CSAA7134Card* pSAA7134Card, CContigMemory* PagelistDMAMem[kMAX_PAGETABLES], CUserMemory* DisplayDMAMem[kMAX_FRAMEBUFFERS], CUserMemory* VBIDMAMem[kMAX_FRAMEBUFFERS], LPCSTR IniSection, LPCSTR ChipName, int DeviceIndex);
    ~CSAA7134Source();

    void Start();
    void Stop();
    void Reset();

    void Mute();
    void UnMute();

    CSAA7134Card* GetCard();

    void GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming);
    void DecodeVBI(TDeinterlaceInfo* pInfo);

    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);

    LPCSTR GetStatus();
    BOOL IsInTunerMode();
    BOOL IsVideoPresent();

    ITuner* GetTuner();

    void SetFormat(eVideoFormat NewFormat);
    eVideoFormat GetFormat();

    void SetAspectRatioData();

    int GetInitialWidth() {return GetWidth();};
    int GetInitialHeight() {return GetHeight() / 2;};

    int GetWidth();
    int GetHeight();
	void SetWidth(int w);

    int GetDeviceIndex();
    const char* GetChipName();
    LPCSTR IDString() { return m_IDString.c_str(); }

    int NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    const char* GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);

    ISetting* GetVolume();
    ISetting* GetBalance();

    ISetting* GetBrightness();
    ISetting* GetContrast();
    ISetting* GetHue();
    ISetting* GetSaturation();
    ISetting* GetSaturationU();
    ISetting* GetSaturationV();
    ISetting* GetAnalogueBlanking() {return NULL;};
    ISetting* GetTopOverscan();
    ISetting* GetBottomOverscan();
    ISetting* GetLeftOverscan();
    ISetting* GetRightOverscan();
	ISetting* GetHDelay();
	ISetting* GetVDelay();

    LPCSTR GetMenuLabel();
    void SetMenu(HMENU hMenu);
    void UpdateMenu() {return;};

    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    void HandleTimerMessages(int TimerId);

    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff);

    BOOL OpenMediaFile(LPCSTR FileName, BOOL NewPlayList) {return FALSE;};
    BOOL IsAccessAllowed() {return TRUE;};
    BOOL HasSquarePixels() {return FALSE;};

    CTreeSettingsPage* CSAA7134Source::GetTreeSettingsPage();
    
    void SetSourceAsCurrent();
   
    void Pause() {return;};
    void UnPause() {return;};

	BOOL HasMediaControl() {return FALSE;};

    BOOL IsAudioMixerAccessAllowed() {return TRUE;};

	BOOL IsInitialSetup() {return m_InitialSetup;};

private:
    virtual void CreateSettings(LPCSTR IniSection);

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
    void CleanupUI();

    static BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY RegisterEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY AudioStandardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
    static BOOL APIENTRY OtherEditProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

    void ChangeDefaultsForVideoInput(BOOL bDontSetValue);
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue);
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue);
    void ChangeTVSettingsBasedOnTuner();

protected:
    HINSTANCE       m_hSAA7134ResourceInst;

private:
    CSAA7134Card*   m_pSAA7134Card;
    BYTE*           m_pDisplay[kMAX_FRAMEBUFFERS];
    BYTE*           m_pVBILines[kMAX_FRAMEBUFFERS];
    CUserMemory*    m_VBIDMAMem[kMAX_FRAMEBUFFERS];
    CUserMemory*    m_DisplayDMAMem[kMAX_FRAMEBUFFERS];
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
    std::string     m_ChannelSubSection;
    std::string     m_ChipName;
    int             m_DeviceIndex;

    std::string     m_Section;
    std::string     m_IDString;

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

    CSliderSetting* m_CustomPixelWidth;
    CYesNoSetting*  m_ReversePolarity;
    CYesNoSetting*  m_VBIDebugOverlay;
    CSliderSetting* m_CardType;
    CStringSetting* m_CardName;

	BOOL			m_InitialSetup;

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

