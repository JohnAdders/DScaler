/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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

/**
 * @file DSSource.h interface for the CDSCaptureSource class.
 */

#if !defined(AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_)
#define AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSSourceBase.h"
#include <map>

/**
 * This source controls a DirectShow capture graph.
 * @see CDSProvider
 * @see CDShowGraph
 * @see CDSSourceBase
 */
class CDSCaptureSource : public CDSSourceBase
{
public:
    CDSCaptureSource(std::string device, std::string deviceName);
    virtual ~CDSCaptureSource();

    //from CSettingsHolder
    void CreateSettings(LPCSTR IniSection);

    BOOL HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam);
    void HandleTimerMessages(int TimerId);

    std::string GetStatus();

    CSliderSetting* GetBrightness();
    CSliderSetting* GetContrast();
    CSliderSetting* GetHue();
    CSliderSetting* GetSaturation();
    CSliderSetting* GetSaturationU() {return NULL;};
    CSliderSetting* GetSaturationV() {return NULL;};

    CYesNoSetting* GetAnalogueBlanking() {return NULL;};
    CSliderSetting* GetTopOverscan();
    CSliderSetting* GetBottomOverscan();
    CSliderSetting* GetLeftOverscan();
    CSliderSetting* GetRightOverscan();

    CSliderSetting* GetHDelay() {return NULL;};
    CSliderSetting* GetVDelay() {return NULL;};

    eVideoFormat GetFormat();
    BOOL IsInTunerMode();

    BOOL SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat);
    BOOL IsVideoPresent();

    int  NumInputs(eSourceInputType InputType);
    BOOL SetInput(eSourceInputType InputType, int Nr);
    int GetInput(eSourceInputType InputType);
    std::string GetInputName(eSourceInputType InputType, int Nr);
    BOOL InputHasTuner(eSourceInputType InputType, int Nr);


    void UpdateMenu() {return;};
    void SetMenu(HMENU hMenu);
    BOOL IsAccessAllowed();
    std::string GetMenuLabel();

    void SetAspectRatioData();

    BOOL OpenMediaFile(const std::string& FileName, BOOL NewPlayList);
    void DecodeVBI(TDeinterlaceInfo* pInfo){};

    ///@todo this probably have to be changed
    BOOL HasSquarePixels() {return FALSE;};

    void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff) {;};

    static void ChannelChange(void *pThis,int PreChange,int OldChannel,int NewChannel);

    void Start();
    void Stop();

    SmartPtr<ITuner> GetTuner();

    BOOL IsAudioMixerAccessAllowed() {return TRUE;};

    int ChangeRes(int nResIndex);

private:

    /**
     * Dummy ITuner.
     * The purpose of this implementation of ITuner is only to get the
     * channels menu active.
     */
    class CDummyTuner : public ITuner
    {
    public:
        eTunerId GetTunerId();
        eVideoFormat GetDefaultVideoFormat();
        BOOL HasRadio() const;
        BOOL SetRadioFrequency(long nFrequency);
        BOOL SetTVFrequency(long nFrequency, eVideoFormat videoFormat);
        long GetFrequency();
        eTunerLocked IsLocked();
        eTunerAFCStatus GetAFCStatus(long &nFreqDeviation);
    };

    ///coverts from dscaler eVideoFormat to directshow AnalogVideoStandard
    AnalogVideoStandard ConvertVideoStd(eVideoFormat fmt);

    ///coverts from directshow AnalogVideoStandard to dscaler eVideoFormat
    eVideoFormat ConvertVideoStd(AnalogVideoStandard fmt);

    ///resets m_VideoFmt to default
    void CreateDefaultVideoFmt();

    std::string m_Device;
    std::string m_DeviceName;

    std::vector<CDShowGraph::CVideoFormat> m_VideoFmt;

    BOOL m_HaveInputList;
    std::vector<int> m_VideoInputList;
    std::vector<int> m_AudioInputList;
    SmartPtr<ITuner> m_Tuner;

    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Brightness);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Contrast);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Hue);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Saturation);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, TopOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, BottomOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, LeftOverscan);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, RightOverscan);

    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, VideoInput);
    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, AudioInput);

    DEFINE_SLIDER_CALLBACK_SETTING(CDSCaptureSource, Resolution);
    DEFINE_YESNO_CALLBACK_SETTING(CDSCaptureSource, ConnectAudio);

protected:
    void ChangeDefaultsForVideoFormat(BOOL bDontSetValue) {};
    void ChangeDefaultsForVideoInput(BOOL bDontSetValue) {};
    void ChangeDefaultsForAudioInput(BOOL bDontSetValue) {};
};

#endif // !defined(AFX_DSSOURCE_H__C552BD3D_0240_4408_805B_0783992D937E__INCLUDED_)
