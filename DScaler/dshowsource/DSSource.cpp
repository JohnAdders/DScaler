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
 * @file DSSource.cpp implementation of the CDSCaptureSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dscaler.h"
#include "..\DScalerRes\resource.h"
#include "DSSource.h"
#include "CaptureDevice.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "AutoCriticalSection.h"
#include "Audio.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"
#include "ProgramList.h"
#include "TreeSettingsDlg.h"
#include "DSVideoFormatPage.h"
#include "DSAudioDevicePage.h"
#include "OutThreads.h"

using namespace std;

struct videoStandardsType
{
    AnalogVideoStandard format;
    LPTSTR name;
};

///array of video standards
videoStandardsType videoStandards[] =
    {
        AnalogVideo_NTSC_M,_T("NTSC_M"),
        AnalogVideo_NTSC_M_J,_T("NTSC_M"),
        AnalogVideo_NTSC_433,_T("NTSC_433"),
        AnalogVideo_PAL_B,_T("PAL_B"),
        AnalogVideo_PAL_D,_T("PAL_D"),
        AnalogVideo_PAL_G,_T("PAL_G"),
        AnalogVideo_PAL_H,_T("PAL_H"),
        AnalogVideo_PAL_I,_T("PAL_I"),
        AnalogVideo_PAL_M,_T("PAL_M"),
        AnalogVideo_PAL_N,_T("PAL_N"),
        AnalogVideo_PAL_60,_T("PAL_60"),
        AnalogVideo_SECAM_B,_T("SECAM_B"),
        AnalogVideo_SECAM_D,_T("SECAM_D"),
        AnalogVideo_SECAM_G,_T("SECAM_G"),
        AnalogVideo_SECAM_H,_T("SECAM_H"),
        AnalogVideo_SECAM_K,_T("SECAM_K"),
        AnalogVideo_SECAM_K1,_T("SECAM_K1"),
        AnalogVideo_SECAM_L,_T("SECAM_L"),
        AnalogVideo_SECAM_L1,_T("SECAM_L1"),
        AnalogVideo_PAL_N_COMBO,_T("PAL_N_COMBO"),
        (AnalogVideoStandard)0,NULL
    };

CDSCaptureSource::CDSCaptureSource(tstring device,tstring deviceName) :
    CDSSourceBase(0,IDC_DSHOWSOURCEMENU),
    m_Device(device),
    m_DeviceName(deviceName),
    m_HaveInputList(FALSE),
    m_Tuner(new CDummyTuner)
{
    m_IDString = tstring(_T("DS_")) + device;
    CreateSettings(device.c_str());

    eEventType EventList[] = {EVENT_CHANNEL_CHANGE,EVENT_ENDOFLIST};
    EventCollector->Register(this, EventList);
}

CDSCaptureSource::~CDSCaptureSource()
{
    //save m_VideoFmt to ini file
    if(m_VideoFmt.size()>0)
    {
        tstring data;
        for(vector<CDShowGraph::CVideoFormat>::size_type i=0;i<m_VideoFmt.size();i++)
        {
            if(data.size()!=0)
            {
                data+=_T("&");
            }
            data+=m_VideoFmt[i];
        }
        WritePrivateProfileString(m_Device.c_str(),_T("ResolutionData"),data.c_str(),GetIniFileForSettings());
        WritePrivateProfileInt(m_Device.c_str(),_T("ResolutionSize"),data.size(),GetIniFileForSettings());
    }
    else
    {
        WritePrivateProfileString(m_Device.c_str(),_T("ResolutionData"),_T(""),GetIniFileForSettings());
        WritePrivateProfileInt(m_Device.c_str(),_T("ResolutionSize"),0,GetIniFileForSettings());
    }
}

BOOL CDSCaptureSource::IsAccessAllowed()
{
    return TRUE;
}

BOOL CDSCaptureSource::OpenMediaFile(const tstring& FileName, BOOL NewPlayList)
{
    return FALSE;
}

CSliderSetting* CDSCaptureSource::GetBrightness()
{
    if(m_pDSGraph==NULL)
    {
        return NULL;
    }

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc==NULL)
    {
        return NULL;
    }

    CDShowCaptureDevice *pCap=NULL;
    if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)pSrc;
        if(pCap->hasVideoProcAmp())
        {
            long min;
            long max;
            long def;

            try
            {
                pCap->getRange(VideoProcAmp_Brightness,&min,&max,NULL,&def);
                m_Brightness->SetMax(max);
                m_Brightness->SetMin(min);

                m_Brightness->ChangeDefault(def, TRUE);
                return m_Brightness.GetRawPointer();
            }
            catch(CDShowException& e)
            {
                LOG(1, "Exception in CDSCaptureSource::GetBrightness - %s", e.what());
            }
        }
    }
    return NULL;
}

void CDSCaptureSource::BrightnessOnChange(long Brightness, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }

    try
    {
        CDShowCaptureDevice *pCap=NULL;
        if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
        {
            pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
            LOG(3,_T("DSCaptureSource: Set brightness to %d"),Brightness);
            pCap->set(VideoProcAmp_Brightness,Brightness,VideoProcAmp_Flags_Manual);
        }
    }
    catch(CDShowException& e)
    {
        ErrorBox(e.what());
    }
}

CSliderSetting* CDSCaptureSource::GetContrast()
{
    if(m_pDSGraph==NULL)
    {
        return NULL;
    }

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc==NULL)
    {
        return NULL;
    }

    CDShowCaptureDevice *pCap=NULL;
    if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)pSrc;
        if(pCap->hasVideoProcAmp())
        {
            long min;
            long max;
            long def;

            try
            {
                pCap->getRange(VideoProcAmp_Contrast,&min,&max,NULL,&def);
                m_Contrast->SetMax(max);
                m_Contrast->SetMin(min);

                m_Contrast->ChangeDefault(def, TRUE);
                return m_Contrast.GetRawPointer();
            }
            catch(CDShowException& e)
            {
                LOG(1, "Exception in CDSCaptureSource::GetContrast - %s", e.what());
            }
        }
    }
    return NULL;
}

void CDSCaptureSource::ContrastOnChange(long Contrast, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }

    try
    {
        CDShowCaptureDevice *pCap=NULL;
        if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
        {
            pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
            pCap->set(VideoProcAmp_Contrast,Contrast,VideoProcAmp_Flags_Manual);
        }
    }
    catch(CDShowException& e)
    {
        ErrorBox(e.what());
    }
}

CSliderSetting* CDSCaptureSource::GetHue()
{
    if(m_pDSGraph==NULL)
    {
        return NULL;
    }

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc==NULL)
    {
        return NULL;
    }

    CDShowCaptureDevice *pCap=NULL;
    if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)pSrc;
        if(pCap->hasVideoProcAmp())
        {
            long min;
            long max;
            long def;

            try
            {
                pCap->getRange(VideoProcAmp_Hue,&min,&max,NULL,&def);
                m_Hue->SetMax(max);
                m_Hue->SetMin(min);

                m_Hue->ChangeDefault(def, TRUE);
                return m_Hue.GetRawPointer();
            }
            catch(CDShowException& e)
            {
                LOG(1, "Exception in CDSCaptureSource::GetHue - %s", e.what());
            }
        }
    }
    return NULL;
}

void CDSCaptureSource::HueOnChange(long Hue, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }

    try
    {
        CDShowCaptureDevice *pCap=NULL;
        if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
        {
            pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
            pCap->set(VideoProcAmp_Hue,Hue,VideoProcAmp_Flags_Manual);
        }
    }
    catch(CDShowException& e)
    {
        ErrorBox(e.what());
    }
}

CSliderSetting* CDSCaptureSource::GetSaturation()
{
    if(m_pDSGraph==NULL)
    {
        return NULL;
    }

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc==NULL)
    {
        return NULL;
    }

    CDShowCaptureDevice *pCap=NULL;
    if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)pSrc;
        if(pCap->hasVideoProcAmp())
        {
            long min;
            long max;
            long def;

            try
            {
                pCap->getRange(VideoProcAmp_Saturation,&min,&max,NULL,&def);
                m_Saturation->SetMax(max);
                m_Saturation->SetMin(min);

                m_Saturation->ChangeDefault(def, TRUE);
                return m_Saturation.GetRawPointer();
            }
            catch(CDShowException& e)
            {
                LOG(1, "Exception in CDSCaptureSource::GetSaturation - %s", e.what());
            }
        }
    }
    return NULL;
}

void CDSCaptureSource::SaturationOnChange(long Saturation, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }

    try
    {
        CDShowCaptureDevice *pCap=NULL;
        if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
        {
            pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
            pCap->set(VideoProcAmp_Saturation,Saturation,VideoProcAmp_Flags_Manual);
        }
    }
    catch(CDShowException& e)
    {
        ErrorBox(e.what());
    }
}

void CDSCaptureSource::CreateSettings(LPCTSTR IniSection)
{
    CDSSourceBase::CreateSettings(IniSection);

    CSettingGroup *pVideoGroup = SettingsMaster->GetGroup(_T("DS - Video"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, TRUE);
    CSettingGroup *pOverscanGroup = SettingsMaster->GetGroup(_T("DS - Overscan"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    //at this time we dont know what the min and max will be
    m_Brightness = new CBrightnessSetting(this, _T("Brightness"), 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
    m_Settings.push_back(m_Brightness.DynamicCast<CSimpleSetting>());

    m_Contrast = new CContrastSetting(this, _T("Contrast"), 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
    m_Settings.push_back(m_Contrast.DynamicCast<CSimpleSetting>());

    m_Hue = new CHueSetting(this, _T("Hue"), 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
    m_Settings.push_back(m_Hue.DynamicCast<CSimpleSetting>());

    m_Saturation = new CSaturationSetting(this, _T("Saturation"), 0, LONG_MIN, LONG_MAX, IniSection, pVideoGroup);
    m_Settings.push_back(m_Saturation.DynamicCast<CSimpleSetting>());

    m_TopOverscan = new CTopOverscanSetting(this, _T("Overscan at Top"), 0, 0, 150, IniSection, pOverscanGroup);
    m_Settings.push_back(m_TopOverscan.DynamicCast<CSimpleSetting>());

    m_BottomOverscan = new CBottomOverscanSetting(this, _T("Overscan at Bottom"), 0, 0, 150, IniSection, pOverscanGroup);
    m_Settings.push_back(m_BottomOverscan.DynamicCast<CSimpleSetting>());

    m_LeftOverscan = new CLeftOverscanSetting(this, _T("Overscan at Left"), 0, 0, 150, IniSection, pOverscanGroup);
    m_Settings.push_back(m_LeftOverscan.DynamicCast<CSimpleSetting>());

    m_RightOverscan = new CRightOverscanSetting(this, _T("Overscan at Right"), 0, 0, 150, IniSection, pOverscanGroup);
    m_Settings.push_back(m_RightOverscan.DynamicCast<CSimpleSetting>());

    m_VideoInput = new CVideoInputSetting(this, _T("VideoInput"), 0, 0, LONG_MAX, IniSection);
    m_Settings.push_back(m_VideoInput.DynamicCast<CSimpleSetting>());

    m_AudioInput = new CAudioInputSetting(this, _T("AudioInput"), 0, 0, LONG_MAX, IniSection);
    m_Settings.push_back(m_AudioInput.DynamicCast<CSimpleSetting>());

    m_Resolution = new CResolutionSetting(this, _T("Resolution"), -1, -1, LONG_MAX, IniSection, pVideoGroup);
    m_Settings.push_back(m_Resolution.DynamicCast<CSimpleSetting>());

    m_ConnectAudio = new CConnectAudioSetting(this,_T("ConnectAudio"),TRUE,IniSection);
    m_Settings.push_back(m_ConnectAudio.DynamicCast<CSimpleSetting>());

    //restore m_VideoFmt from ini file
    int ResolutionDataIniSize=GetPrivateProfileInt(IniSection,_T("ResolutionSize"),-1,GetIniFileForSettings());
    if(ResolutionDataIniSize>0)
    {
        TCHAR* pcData=new TCHAR[ResolutionDataIniSize+1];
        DWORD result=GetPrivateProfileString(IniSection,_T("ResolutionData"),_T(""),pcData,ResolutionDataIniSize+1,GetIniFileForSettings());
        if(result<ResolutionDataIniSize)
        {
            LOG(2,_T("DSCaptureSource: Reading too litle data, problem with ResolutionSize or ResolutionData in ini file"));
        }

        tstring str=pcData;
        vector<tstring> strlist;
        tstring::size_type LastPos=0;
        tstring::size_type pos;
        while(pos=str.find(_T("&"),LastPos),pos!=tstring::npos)
        {
            strlist.push_back(str.substr(LastPos,pos-LastPos));
            LastPos=pos+1;
        }
        if(LastPos<str.size())
        {
            strlist.push_back(str.substr(LastPos));
        }
        for(vector<tstring>::size_type i=0;i<strlist.size();i++)
        {
            CDShowGraph::CVideoFormat fmt;
            fmt=strlist[i];
            m_VideoFmt.push_back(fmt);
        }

        delete pcData;
    }

    ReadFromIni();
    LOG(2,_T("DSCaptureSource: setting read from .ini"));
}

int CDSCaptureSource::ChangeRes(int nResIndex)
{
    int resu = 1;
    try
    {
        CAutoCriticalSection lock(m_hOutThreadSync);

        _ASSERTE(nResIndex>=0 && nResIndex<m_VideoFmt.size());
        CDShowGraph::eChangeRes_Error err=m_pDSGraph->ChangeRes(m_VideoFmt[nResIndex]);
        switch(err)
        {
        case CDShowGraph::ERROR_NO_GRAPH:
            ErrorBox(_T("Can't change resolution because there is no filter graph (bug)"));
            break;
        case CDShowGraph::ERROR_CHANGED_BACK:
            ErrorBox(_T("The selected resolution is not valid or coud not be used"));
            break;
        case CDShowGraph::ERROR_FAILED_TO_CHANGE_BACK:
            ErrorBox(_T("Failed to change resolution and faild to change back to previous resolution"));
            //shod probably call Stop() or Reset() here since the
            //filter graph is most likely broken now
            break;
        case CDShowGraph::SUCCESS:
            m_Resolution->SetValue(nResIndex);
            resu = 0;
            break;
        }
        NotifySquarePixelsCheck();
    }
    catch(CDShowException &e)
    {
        ErrorBox(MakeString() << _T("Error when changeing resolution\n\n") << e.what());
    }
    catch(exception &e2)
    {
        ErrorBox(MakeString() << _T("Stl exception:\n\n") << e2.what());
    }
    return resu;
}

BOOL CDSCaptureSource::HandleWindowsCommands(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    if(CDSSourceBase::HandleWindowsCommands(hWnd,wParam,lParam)==TRUE)
    {
        return TRUE;
    }

    if(LOWORD(wParam)==IDM_DSHOW_SETTINGS)
    {
        CTreeSettingsDlg dlg(_T("DirectShow Settings"));

        BOOL bConnectAudio=(m_ConnectAudio->GetValue()!=0);
        SmartPtr<CTreeSettingsPage> AudioDevice(new CDSAudioDevicePage(_T("Audio output"), m_AudioDevice, &bConnectAudio));
        SmartPtr<CTreeSettingsPage> VidemFmt(new CDSVideoFormatPage(_T("Resolution"), m_VideoFmt, m_Resolution.GetRawPointer()));

        dlg.AddPage(AudioDevice);
        dlg.AddPage(VidemFmt);
        dlg.DoModal(hWnd);

        m_ConnectAudio->SetValue(bConnectAudio);

        return TRUE;
    }

    if(m_pDSGraph==NULL)
    {
        return FALSE;
    }

    CDShowCaptureDevice *pCap=NULL;
    CDShowTVAudio *pTVAudio=NULL;
    if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
        pTVAudio=pCap->GetTVAudio();
    }

    if(LOWORD(wParam)>=IDM_CROSSBAR_INPUT0 && LOWORD(wParam)<=IDM_CROSSBAR_INPUT_MAX)
    {
        try
        {
            if(pCap!=NULL)
            {
                CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
                if(pCrossbar!=NULL)
                {
                    PhysicalConnectorType type=pCrossbar->GetInputType(LOWORD(wParam)-IDM_CROSSBAR_INPUT0);
                    if(type<0x1000)
                    {
                        m_VideoInput->SetValue(LOWORD(wParam)-IDM_CROSSBAR_INPUT0);
                    }
                    else
                    {
                        m_AudioInput->SetValue(LOWORD(wParam)-IDM_CROSSBAR_INPUT0);
                    }
                }
            }
        }
        catch(CDShowException &e)
        {
            ErrorBox(MakeString() << _T("Failed to change input\n\n") << e.what());
        }

        return TRUE;
    }
    else if(LOWORD(wParam)>=IDM_DSVIDEO_STANDARD_0 && LOWORD(wParam)<=IDM_DSVIDEO_STANDARD_MAX)
    {
        try
        {
            if(pCap!=NULL)
            {
                pCap->PutTVFormat(videoStandards[LOWORD(wParam)-IDM_DSVIDEO_STANDARD_0].format);
            }
        }
        catch(CDShowException &e)
        {
            ErrorBox(MakeString() << _T("Failed to change video format\n\n") << e.what());
        }
        return TRUE;
    }
    else if(LOWORD(wParam)>=IDM_DSHOW_RES_0 && LOWORD(wParam<=IDM_DSHOW_RES_MAX))
    {
        TGUIRequest req;
        req.type = REQ_DSHOW_CHANGERES;
        req.param1 = LOWORD(wParam)-IDM_DSHOW_RES_0;
        _ASSERTE(req.param1>=0 && req.param1<m_VideoFmt.size());
        PutRequest(&req);
    }
    else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_MONO)
    {
        if(pTVAudio!=NULL)
        {
            try
            {
                pTVAudio->SetMode(AMTVAUDIO_MODE_MONO);
            }
            catch(CDShowException &e)
            {
                ErrorBox(MakeString() << _T("Error changing audio channel\n\n") << e.what());
            }
        }
    }
    else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_STEREO)
    {
        if(pTVAudio!=NULL)
        {
            try
            {
                pTVAudio->SetMode(AMTVAUDIO_MODE_STEREO);
            }
            catch(CDShowException &e)
            {
                ErrorBox(MakeString() << _T("Error changing audio channel\n\n") << e.what());
            }
        }
    }
    else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_LANGUAGEA)
    {
        if(pTVAudio!=NULL)
        {
            try
            {
                pTVAudio->SetMode(AMTVAUDIO_MODE_LANG_A);
            }
            catch(CDShowException &e)
            {
                ErrorBox(MakeString() << _T("Error changing audio channel\n\n") << e.what());
            }
        }
    }
    else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_LANGUAGEB)
    {
        if(pTVAudio!=NULL)
        {
            try
            {
                pTVAudio->SetMode(AMTVAUDIO_MODE_LANG_B);
            }
            catch(CDShowException &e)
            {
                ErrorBox(MakeString() << _T("Error changing audio channel\n\n") << e.what());
            }
        }
    }
    else if(LOWORD(wParam)==ID_DSHOW_AUDIOCHANNEL_LANGUAGEB)
    {
        if(pTVAudio!=NULL)
        {
            try
            {
                pTVAudio->SetMode(AMTVAUDIO_MODE_LANG_C);
            }
            catch(CDShowException &e)
            {
                ErrorBox(MakeString() << _T("Error changing audio channel\n\n") << e.what());
            }
        }
    }

    return FALSE;
}

eVideoFormat CDSCaptureSource::GetFormat()
{
    if(m_pDSGraph==NULL)
    {
        LOG(1,_T("CDSCaptureSource::GetFormat called when there is no filter graph"));
        return VIDEOFORMAT_PAL_B;
    }

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc==NULL)
    {
        return VIDEOFORMAT_PAL_B;
    }

    CDShowCaptureDevice *pCap=NULL;
    if(pSrc->getObjectType()!=DSHOW_TYPE_SOURCE_CAPTURE)
    {
        return VIDEOFORMAT_PAL_B;
    }
    pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();

    AnalogVideoStandard VideoStd=AnalogVideo_None;
    try
    {
        VideoStd=pCap->GetTVFormat();
    }
    catch(CDShowException &e)
    {
        LOG(1,"Exception in CDSCaptureSource::GetFormat - %s",e.what());
    }

    return ConvertVideoStd(VideoStd);
}

BOOL CDSCaptureSource::IsInTunerMode()
{
    if(m_pDSGraph==NULL)
    {
        return FALSE;
    }
    CDShowCaptureDevice *pCap=NULL;
    if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
    }

    if(pCap!=NULL)
    {
        CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
        if(pCrossbar!=NULL)
        {
            try
            {
                long cIn,cOut;
                pCrossbar->GetPinCounts(cIn,cOut);
                for(long i=0;i<cIn;i++)
                {
                    if (pCrossbar->IsInputSelected(i))
                    {
                        if (pCrossbar->GetInputType(i) == PhysConn_Video_Tuner)
                        {
                            LOG(3,_T("DSSource: IsInTunerMode."));
                            return TRUE;
                        }
                    }
                }
            }
            catch(CDShowException &e)
            {
                LOG(1,"Exception in CDSCaptureSource::IsInTunerMode - %s",e.what());
            }
            LOG(3,_T("DSSource: IsInTunerMode: No"));
        }
    }
    return FALSE;
}

SmartPtr<ITuner> CDSCaptureSource::GetTuner()
{
    return m_Tuner;
}

AnalogVideoStandard CDSCaptureSource::ConvertVideoStd(eVideoFormat fmt)
{
    switch(fmt)
    {
    case VIDEOFORMAT_PAL_B:
        return AnalogVideo_PAL_B;
    case VIDEOFORMAT_PAL_D:
        return AnalogVideo_PAL_D;
    case VIDEOFORMAT_PAL_G:
        return AnalogVideo_PAL_G;
    case VIDEOFORMAT_PAL_H:
        return AnalogVideo_PAL_H;
    case VIDEOFORMAT_PAL_I:
        return AnalogVideo_PAL_I;
    case VIDEOFORMAT_PAL_M:
        return AnalogVideo_PAL_M;
    case VIDEOFORMAT_PAL_N:
        return AnalogVideo_PAL_N;
    case VIDEOFORMAT_PAL_60:
        return AnalogVideo_PAL_60;
    case VIDEOFORMAT_PAL_N_COMBO:
        return AnalogVideo_PAL_N_COMBO;
    case VIDEOFORMAT_SECAM_B:
        return AnalogVideo_SECAM_B;
    case VIDEOFORMAT_SECAM_D:
        return AnalogVideo_SECAM_D;
    case VIDEOFORMAT_SECAM_G:
        return AnalogVideo_SECAM_G;
    case VIDEOFORMAT_SECAM_H:
        return AnalogVideo_SECAM_H;
    case VIDEOFORMAT_SECAM_K:
        return AnalogVideo_SECAM_K;
    case VIDEOFORMAT_SECAM_K1:
        return AnalogVideo_SECAM_K1;
    case VIDEOFORMAT_SECAM_L:
        return AnalogVideo_SECAM_L;
    case VIDEOFORMAT_SECAM_L1:
        return AnalogVideo_SECAM_L1;
    case VIDEOFORMAT_NTSC_M:
        return AnalogVideo_NTSC_M;
    case VIDEOFORMAT_NTSC_M_Japan:
        return AnalogVideo_NTSC_M_J;
    case VIDEOFORMAT_NTSC_50:
        return AnalogVideo_NTSC_433;
    default:
        LOG(1,_T("CDSCaptureSource::ConvertVideoFmt: Unknown videoformat!"));
        return AnalogVideo_None;
    }
}

eVideoFormat CDSCaptureSource::ConvertVideoStd(AnalogVideoStandard fmt)
{
    switch(fmt)
    {
    case AnalogVideo_PAL_B:
        return VIDEOFORMAT_PAL_B;
    case AnalogVideo_PAL_D:
        return VIDEOFORMAT_PAL_D;
    case AnalogVideo_PAL_G:
        return VIDEOFORMAT_PAL_G;
    case AnalogVideo_PAL_H:
        return VIDEOFORMAT_PAL_H;
    case AnalogVideo_PAL_I:
        return VIDEOFORMAT_PAL_I;
    case AnalogVideo_PAL_M:
        return VIDEOFORMAT_PAL_M;
    case AnalogVideo_PAL_N:
        return VIDEOFORMAT_PAL_N;
    case AnalogVideo_PAL_60:
        return VIDEOFORMAT_PAL_60;
    case AnalogVideo_PAL_N_COMBO:
        return VIDEOFORMAT_PAL_N_COMBO;
    case AnalogVideo_SECAM_B:
        return VIDEOFORMAT_SECAM_B;
    case AnalogVideo_SECAM_D:
        return VIDEOFORMAT_SECAM_D;
    case AnalogVideo_SECAM_G:
        return VIDEOFORMAT_SECAM_G;
    case AnalogVideo_SECAM_H:
        return VIDEOFORMAT_SECAM_H;
    case AnalogVideo_SECAM_K:
        return VIDEOFORMAT_SECAM_K;
    case AnalogVideo_SECAM_K1:
        return VIDEOFORMAT_SECAM_K1;
    case AnalogVideo_SECAM_L:
        return VIDEOFORMAT_SECAM_L;
    case AnalogVideo_SECAM_L1:
        return VIDEOFORMAT_SECAM_L1;
    case AnalogVideo_NTSC_M:
        return VIDEOFORMAT_NTSC_M;
    case AnalogVideo_NTSC_M_J:
        return VIDEOFORMAT_NTSC_M_Japan;
    case AnalogVideo_NTSC_433:
        return VIDEOFORMAT_NTSC_50;
    default:
        LOG(1,_T("CDSCaptureSource::ConvertVideoFmt: Unknown videoformat!"));
        return VIDEOFORMAT_PAL_B;
    }
}

BOOL CDSCaptureSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    if(m_pDSGraph==NULL)
    {
        return FALSE;
    }

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc==NULL)
    {
        return FALSE;
    }

    CDShowCaptureDevice *pCap=NULL;
    if(pSrc->getObjectType()!=DSHOW_TYPE_SOURCE_CAPTURE)
    {
        return FALSE;
    }

    pCap=(CDShowCaptureDevice*)pSrc;
    CDShowDirectTuner *pTuner = pCap->GetTuner();
    if(pTuner==NULL)
    {
        return FALSE;
    }
    try
    {
        //fm radio
        if(VideoFormat==VIDEOFORMAT_LASTONE+1)
        {
            if(pTuner->GetAvailableModes()&AMTUNER_MODE_FM_RADIO)
            {
                pTuner->SetFrequency(FrequencyId,AMTUNER_MODE_FM_RADIO,AnalogVideo_None);
                return TRUE;
            }
            else
            {
                LOG(1,_T("CDSCaptureSource::SetTunerFrequency: Tuner does not support fm radio"));
                return FALSE;
            }
        }
        else
        {
            if(pTuner->GetAvailableModes()&AMTUNER_MODE_TV)
            {
                AnalogVideoStandard format=ConvertVideoStd(VideoFormat);
                if(format&pCap->GetSupportedTVFormats())
                {
                    pCap->PutTVFormat(format);
                }
                else
                {
                    LOG(1,_T("CDSCaptureSource::SetTunerFrequency: Specified video format is not supported!!!"));
                }
                pTuner->SetFrequency(FrequencyId,AMTUNER_MODE_TV,format);

                return TRUE;
            }
            else
            {
                LOG(1,_T("CDSCaptureSource::SetTunerFrequency: Tuner is not a tvtuner"));
                return FALSE;
            }
        }
    }
    catch(CDShowException& e)
    {
        LOG(1, "CDSCaptureSource::SetTunerFrequency: DShow Exception - %s", e.what());
        return FALSE;
    }
}

BOOL CDSCaptureSource::IsVideoPresent()
{
    if(m_pDSGraph==NULL)
    {
        return FALSE;
    }

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc==NULL)
    {
        return FALSE;
    }

    CDShowCaptureDevice *pCap=NULL;
    if(pSrc->getObjectType()!=DSHOW_TYPE_SOURCE_CAPTURE)
    {
        return FALSE;
    }

    pCap=(CDShowCaptureDevice*)pSrc;
    /*CDShowDirectTuner *pTuner = pCap->GetTuner();
    if(pTuner==NULL)
    {
        return FALSE;
    }*/
    try
    {
        //this is comented out for now since it looks like it doesn't work
        //hopefully IsHorizontalLocked will work properly most of the time
        /*CDShowDirectTuner::eSignalType type;

        long signal=pTuner->GetSignalStrength(type);
        switch(type)
        {
        case CDShowDirectTuner::eSignalType::SIGNALTYPE_SIGNALSTRENGTH:
            LOG(2,_T("CDSCaptureSource::IsVideoPresent: Using SignalStrength to determine signal present"));
            if(signal>0)
            {
                return TRUE;
            }
            break;
        case CDShowDirectTuner::eSignalType::SIGNALTYPE_PLL:
            LOG(2,_T("CDSCaptureSource::IsVideoPresent: Using PLL to determine signal present"));
            if(signal==0)
            {
                return TRUE;
            }
            break;
        case CDShowDirectTuner::eSignalType::SIGNALTYPE_NONE:
            LOG(2,_T("CDSCaptureSource::IsVideoPresent: Using IsHorizontalLocked() to determine signal present"));
            return pCap->IsHorizontalLocked();
            break;
        }*/
        return pCap->IsHorizontalLocked();
    }
    catch(CDShowException& e)
    {
        LOG(1, "CDSCaptureSource::IsVideoPresent: DShow Exception - %s", e.what());
    }

    return FALSE;
}
eTunerId CDSCaptureSource::CDummyTuner::GetTunerId()
{
    //there is no value that fits dshow
    //any value besides TUNER_ABSENT will work
    return TUNER_USER_SETUP;
}
eVideoFormat CDSCaptureSource::CDummyTuner::GetDefaultVideoFormat()
{
    return VIDEOFORMAT_PAL_B;
}
BOOL CDSCaptureSource::CDummyTuner::HasRadio() const
{
    return FALSE;
}
BOOL CDSCaptureSource::CDummyTuner::SetRadioFrequency(long nFrequency)
{
    return FALSE;
}
BOOL CDSCaptureSource::CDummyTuner::SetTVFrequency(long nFrequency, eVideoFormat videoFormat)
{
    return FALSE;
}
long CDSCaptureSource::CDummyTuner::GetFrequency()
{
    return 0;
}
eTunerLocked CDSCaptureSource::CDummyTuner::IsLocked()
{
    return TUNER_LOCK_NOTSUPPORTED;
}
eTunerAFCStatus CDSCaptureSource::CDummyTuner::GetAFCStatus(long &nFreqDeviation)
{
    return TUNER_AFC_NOTSUPPORTED;
}

void CDSCaptureSource::SetMenu(HMENU hMenu)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }

    HMENU menu = GetSubMenu(m_hMenu, 0);

    CDShowCaptureDevice *pCap=NULL;
    CDShowBaseCrossbar *pCrossbar=NULL;

    CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
    if(pSrc!=NULL)
    {
        if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
        {
            pCap=(CDShowCaptureDevice*)pSrc;
            pCrossbar=pCap->getCrossbar();
        }
    }

    //setup input selection menus
    vector<TCHAR> menuText(MAX_PATH+1);
    if(pCrossbar!=NULL)
    {
        //create video input submenu and insert it
        HMENU vidSubMenu = CreateMenu();

        GetMenuString(menu,0, &menuText[0], MAX_PATH, MF_BYPOSITION);
        ModifyMenu(menu, 0,MF_POPUP|MF_BYPOSITION,(UINT)vidSubMenu, &menuText[0]);
        EnableMenuItem(menu, 0,MF_BYPOSITION|MF_ENABLED);

        //same for audio input menu
        HMENU audSubMenu = CreateMenu();
        GetMenuString(menu, 3, &menuText[0], MAX_PATH, MF_BYPOSITION);
        ModifyMenu(menu, 3, MF_POPUP|MF_BYPOSITION, (UINT) audSubMenu, &menuText[0]);
        EnableMenuItem(menu, 3, MF_BYPOSITION|MF_ENABLED);

        long cIn,cOut;
        pCrossbar->GetPinCounts(cIn,cOut);
        for(int i=0;i<cIn;i++)
        {
            _ASSERTE((IDM_CROSSBAR_INPUT0+i)<=IDM_CROSSBAR_INPUT_MAX);
            BOOL bSelected=pCrossbar->IsInputSelected(i);

            //is it an audio or video input?
            if(pCrossbar->GetInputType(i)<4096)
            {
                AppendMenu(vidSubMenu, MF_STRING,IDM_CROSSBAR_INPUT0+i,pCrossbar->GetInputName(i).c_str());
                CheckMenuItem(vidSubMenu, IDM_CROSSBAR_INPUT0+i,bSelected ? MF_CHECKED:MF_UNCHECKED);
            }
            else
            {
                AppendMenu(audSubMenu, MF_STRING,IDM_CROSSBAR_INPUT0+i,pCrossbar->GetInputName(i).c_str());
                CheckMenuItem(audSubMenu, IDM_CROSSBAR_INPUT0+i,bSelected ? MF_CHECKED:MF_UNCHECKED);
            }
        }
    }
    else
    {
        EnableMenuItem(menu, 0,MF_BYPOSITION|MF_GRAYED);
        EnableMenuItem(menu, 3,MF_BYPOSITION|MF_GRAYED);
    }

    //video standards menu
    if(IsInTunerMode()==FALSE && pCap!=NULL && pCap->hasVideoDec())
    {

        long formats=0;
        long selectedFormat=0;
        try
        {
            formats=pCap->GetSupportedTVFormats();
            selectedFormat=pCap->GetTVFormat();
        }
        catch(CDShowException& e)
        {
            LOG(1, "Exception in CDSCaptureSource::SetMenu - %s", e.what());
        }

        //make sure there is at least one format to be selected
        if(formats!=0)
        {
            HMENU formatMenu = CreateMenu();
            GetMenuString(menu, 1, &menuText[0], MAX_PATH, MF_BYPOSITION);
            ModifyMenu(menu, 1, MF_POPUP|MF_BYPOSITION, (UINT)formatMenu, &menuText[0]);
            EnableMenuItem(menu, 1,MF_BYPOSITION|MF_ENABLED);

            int i=0;
            while(videoStandards[i].format!=0)
            {
                _ASSERTE(i <= IDM_DSVIDEO_STANDARD_MAX-IDM_DSVIDEO_STANDARD_0);

                if(formats & videoStandards[i].format)
                {
                    UINT check=selectedFormat & videoStandards[i].format ? MF_CHECKED:MF_UNCHECKED;
                    AppendMenu(formatMenu, MF_STRING,IDM_DSVIDEO_STANDARD_0+i,videoStandards[i].name);
                    CheckMenuItem(formatMenu, IDM_DSVIDEO_STANDARD_0+i,check);
                }
                i++;
            }
        }
        else
        {
            EnableMenuItem(menu, 1,MF_BYPOSITION|MF_GRAYED);
        }
    }
    else
    {
        EnableMenuItem(menu, 1,MF_BYPOSITION|MF_GRAYED);
    }

    //resolution submenu
    BOOL ResAdded=FALSE;
    HMENU ResSubMenu = CreateMenu();

    for(vector<CDShowGraph::CVideoFormat>::size_type index=0;index<m_VideoFmt.size();index++)
    {
        _ASSERTE(IDM_DSHOW_RES_0+index<=IDM_DSHOW_RES_MAX);
        AppendMenu(ResSubMenu, MF_STRING|(m_Resolution->GetValue()==index?MF_CHECKED:MF_UNCHECKED),IDM_DSHOW_RES_0+index,m_VideoFmt[index].m_Name.c_str());
        ResAdded=TRUE;
    }

    if(ResAdded)
    {
        GetMenuString(menu, 2, &menuText[0], MAX_PATH, MF_BYPOSITION);
        ModifyMenu(menu, 2,MF_POPUP|MF_BYPOSITION, (UINT) ResSubMenu, &menuText[0]);
        EnableMenuItem(menu, 2, MF_BYPOSITION|MF_ENABLED);
    }
    else
    {
        //no resolution was added, gray the menu item
        EnableMenuItem(menu, 2,MF_BYPOSITION|MF_GRAYED);
        DestroyMenu(ResSubMenu);
    }

    //set a radio checkmark infront of the current play/pause/stop menu entry
    FILTER_STATE state=m_pDSGraph->getState();
    UINT pos=8-state;
    CheckMenuRadioItem(menu, 6, 8, pos, MF_BYPOSITION);

    //audio channel submenu
    CDShowTVAudio *pTVAudio=pCap->GetTVAudio();
    if(pTVAudio!=NULL && pTVAudio->GetAvailableModes()!=0)
    {
        EnableMenuItem(menu, 4, MF_BYPOSITION);
        long modes=pTVAudio->GetAvailableModes();
        TVAudioMode mode=pTVAudio->GetMode();
        if(modes&AMTVAUDIO_MODE_MONO)
        {
            EnableMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_MONO, MF_BYCOMMAND);
            CheckMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_MONO, MF_BYCOMMAND | (mode&AMTVAUDIO_MODE_MONO ? MF_CHECKED : MF_UNCHECKED));

        }
        if(modes&AMTVAUDIO_MODE_STEREO)
        {
            EnableMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_STEREO,MF_BYCOMMAND);
            CheckMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_STEREO,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_STEREO ? MF_CHECKED : MF_UNCHECKED));
        }
        if(modes&AMTVAUDIO_MODE_LANG_A)
        {
            EnableMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_LANGUAGEA,MF_BYCOMMAND);
            CheckMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_LANGUAGEA,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_LANG_A ? MF_CHECKED : MF_UNCHECKED));
        }
        if(modes&AMTVAUDIO_MODE_LANG_B)
        {
            EnableMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_LANGUAGEB,MF_BYCOMMAND);
            CheckMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_LANGUAGEB,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_LANG_B ? MF_CHECKED : MF_UNCHECKED));
        }
        if(modes&AMTVAUDIO_MODE_LANG_C)
        {
            EnableMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_LANGUAGEC,MF_BYCOMMAND);
            CheckMenuItem(menu, ID_DSHOW_AUDIOCHANNEL_LANGUAGEC,MF_BYCOMMAND| (mode&AMTVAUDIO_MODE_LANG_C ? MF_CHECKED : MF_UNCHECKED));
        }
    }
    else
    {
        EnableMenuItem(menu, 4,MF_BYPOSITION|MF_GRAYED);
    }
}

void CDSCaptureSource::HandleTimerMessages(int TimerId)
{

}

tstring CDSCaptureSource::GetMenuLabel()
{
    return _T("");
}

CSliderSetting* CDSCaptureSource::GetTopOverscan()
{
    return m_TopOverscan.GetRawPointer();
}

CSliderSetting* CDSCaptureSource::GetBottomOverscan()
{
    return m_BottomOverscan.GetRawPointer();
}

CSliderSetting* CDSCaptureSource::GetLeftOverscan()
{
    return m_LeftOverscan.GetRawPointer();
}

CSliderSetting* CDSCaptureSource::GetRightOverscan()
{
    return m_RightOverscan.GetRawPointer();
}

void CDSCaptureSource::SetAspectRatioData()
{
    AspectSettings.InitialTopOverscan = m_TopOverscan->GetValue();
    AspectSettings.InitialBottomOverscan = m_BottomOverscan->GetValue();
    AspectSettings.InitialLeftOverscan = m_LeftOverscan->GetValue();
    AspectSettings.InitialRightOverscan = m_RightOverscan->GetValue();
    AspectSettings.bAnalogueBlanking = FALSE;
}

void CDSCaptureSource::TopOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialTopOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CDSCaptureSource::BottomOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialBottomOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CDSCaptureSource::LeftOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialLeftOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

void CDSCaptureSource::RightOverscanOnChange(long Overscan, long OldValue)
{
    AspectSettings.InitialRightOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
}

tstring CDSCaptureSource::GetStatus()
{
    return m_DeviceName;
}

void CDSCaptureSource::VideoInputOnChange(long NewValue, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }

    //this shoud always succeed
    CDShowCaptureDevice *pCap=NULL;
    if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
    }
    _ASSERTE(pCap!=NULL);

    try
    {
        if(pCap!=NULL)
        {
            CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
            if(pCrossbar!=NULL)
            {
                PhysicalConnectorType OldInputType = pCrossbar->GetInputType(OldValue);
                EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_PRECHANGE, OldValue, NewValue);

                LOG(2,_T("DSCaptureSource: Set video input to %d"), NewValue);

                //set the related pin too since this is a video pin,maybe this shoud be user configurable?
                pCrossbar->SetInputIndex(NewValue,TRUE);

                PhysicalConnectorType NewInputType = pCrossbar->GetInputType(NewValue);

                /**
                 * @todo we also must figure out what the related pin is and then call
                 * AudioInputOnChange if it is an audio pin.
                 */

                EventCollector->RaiseEvent(this, EVENT_VIDEOINPUT_CHANGE, OldValue, NewValue);

                if(NewInputType == PhysConn_Video_Tuner)
                {
                    // set up channel
                    // this must happen after the VideoInput change is sent
                    if(pCap->GetTuner()!=NULL)
                    {
                        Channel_SetCurrent();
                    }
                }
            }
        }
    }
    catch(CDShowException &e)
    {
        ErrorBox(MakeString() << _T("Failed to change video input\n\n") << e.what());
    }
}

void CDSCaptureSource::AudioInputOnChange(long NewValue, long OldValue)
{
    if(m_pDSGraph==NULL)
    {
        return;
    }

    //this shoud always succeed
    CDShowCaptureDevice *pCap=NULL;
    if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
    }
    _ASSERTE(pCap!=NULL);

    try
    {
        if(pCap!=NULL)
        {
            CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
            if(pCrossbar!=NULL)
            {
                LOG(2,_T("DSCaptureSource: Set audio input to %d"),NewValue);

                //prevent changing to a pin that is not an audio pin
                PhysicalConnectorType type=pCrossbar->GetInputType(NewValue);
                if(type>=0x1000)
                {
                    pCrossbar->SetInputIndex(NewValue,FALSE);
                }
                else
                {
                    LOG(2,_T("DSCaptureSource: New audio input pin is not a audio pin, will not change audio input"));
                }
            }
        }
    }
    catch(CDShowException &e)
    {
        ErrorBox(MakeString() << _T("Failed to change audio input\n\n") << e.what());
    }
}

void CDSCaptureSource::Start()
{
    try
    {
        if(m_pDSGraph==NULL)
        {
            m_pDSGraph=new CDShowGraph(m_Device,m_DeviceName,m_AudioDevice->GetValue(),m_ConnectAudio->GetValue()!=0);
        }

        m_pDSGraph->ConnectGraph();

        if(m_VideoFmt.size()==0)
        {
            CreateDefaultVideoFmt();
        }

        if(m_Resolution->GetValue()>=0 && m_Resolution->GetValue()<m_VideoFmt.size())
        {
            CDShowGraph::eChangeRes_Error err=m_pDSGraph->ChangeRes(m_VideoFmt[m_Resolution->GetValue()]);
            switch(err)
            {
                case CDShowGraph::ERROR_NO_GRAPH:
                    LOG(2,_T("Error restoring resolution, no filter graph (bug)"));
                    break;
                case CDShowGraph::ERROR_CHANGED_BACK:
                    LOG(2,_T("Error restoring resolution, failed to change to new resolution or format not supported"));
                    break;
                case CDShowGraph::ERROR_FAILED_TO_CHANGE_BACK:
                    //oops we broke the filter graph, reset resolution
                    //setting so we don't break it the next time too.
                    m_Resolution->SetValue(-1);
                    LOG(2,_T("Error restoring resolution, broke filter graph"));
                    break;
                case CDShowGraph::SUCCESS:
                    break;
            }
        }

        // Sets min/max/default values
        GetHue();
        GetBrightness();
        GetContrast();
        GetSaturation();
        GetTopOverscan();
        GetBottomOverscan();
        GetLeftOverscan();
        GetRightOverscan();

        VideoInputOnChange(m_VideoInput->GetValue(), m_VideoInput->GetValue());
        AudioInputOnChange(m_AudioInput->GetValue(), m_AudioInput->GetValue());

        CDSSourceBase::Start();
    }
    catch(CDShowException &e)
    {
        ErrorBox(e.what());
    }
}

void CDSCaptureSource::Stop()
{
    CAutoCriticalSection lock(m_hOutThreadSync);

    CDSSourceBase::Stop();

    //need to remove the graph since we dont want to risk having both
    //dscalers own drivers and directshow drivers active at the same time
    if(m_pDSGraph!=NULL)
    {
        delete m_pDSGraph;
        m_pDSGraph=NULL;
    }
}

void CDSCaptureSource::CreateDefaultVideoFmt()
{
    struct ResolutionType
    {
        long x;
        long y;
    }res[]=
    {
        768,576,
        720,576,
        720,480,
        720,288,
        720,240,
        704,576,
        640,480,
        640,360,
        640,288,
        640,240,
        480,360,
        352,288,
        352,240,
        320,240,
        240,180,
        240,176,
        176,144,
        160,120,
        120,96,
        88,72
    };

    m_VideoFmt.erase(m_VideoFmt.begin(),m_VideoFmt.end());
    for(int i=0;i<sizeof(res)/sizeof(ResolutionType);i++)
    {
        CDShowGraph::CVideoFormat fmt;
        fmt.m_Width=res[i].x;
        fmt.m_Height=res[i].y;
        fmt.m_Name=MakeString() << res[i].x << _T("x") << res[i].y;

        if(m_pDSGraph->IsValidRes(fmt))
        {
            m_VideoFmt.push_back(fmt);
        }
    }
}

void CDSCaptureSource::ResolutionOnChange(long NewValue, long OldValue)
{
    //
}

void CDSCaptureSource::ConnectAudioOnChange(BOOL NewValue, BOOL OldValue)
{
    //
}

int CDSCaptureSource::NumInputs(eSourceInputType InputType)
{
    if(m_pDSGraph==NULL)
    {
        return 0;
    }
    //this shoud always succeed
    CDShowCaptureDevice *pCap=NULL;
    if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
    }
    if(pCap == NULL) { return 0; }

    if(!m_HaveInputList)
    {
        m_VideoInputList.clear();
        m_AudioInputList.clear();
        try
        {
            CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
            if(pCrossbar==NULL)
            {
                return 0;
            }

            long cIn,cOut;
            pCrossbar->GetPinCounts(cIn,cOut);
            for(int i=0;i<cIn;i++)
            {
                //is it an audio or video input?
                if(pCrossbar->GetInputType(i)<4096)
                {
                    m_VideoInputList.push_back(i);
                }
                else
                {
                    m_AudioInputList.push_back(i);
                }
            }
            m_HaveInputList = TRUE;
        }
        catch(CDShowException &e)
        {
            LOG(1,"DSCaptureSource: NumInputs: Error: %s",e.what());
            return 0;
        }
    }

    if(InputType == VIDEOINPUT)
    {
        return m_VideoInputList.size();
    }
    else
    {
        return m_AudioInputList.size();
    }
    return 0;
}

BOOL CDSCaptureSource::SetInput(eSourceInputType InputType, int Nr)
{
    if(!m_HaveInputList)
    {
        NumInputs(InputType);   // Make input list
    }
    if(InputType == VIDEOINPUT)
    {
        if((Nr>=0) && (Nr < m_VideoInputList.size()))
        {
            m_VideoInput->SetValue(m_VideoInputList[Nr]);
            /// \todo Should check if this failed
            return TRUE;
        }
        return FALSE;
    }
    else if(InputType == AUDIOINPUT)
    {
        if((Nr>=0) && (Nr < m_AudioInputList.size()))
        {
            m_AudioInput->SetValue(m_AudioInputList[Nr]);
            /// \todo Should check if this failed
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

int CDSCaptureSource::GetInput(eSourceInputType InputType)
{
    if(!m_HaveInputList)
    {
        NumInputs(InputType);   // Make input list
    }

    if(InputType == VIDEOINPUT)
    {
        int i;
        for(i = 0; i < m_VideoInputList.size(); i++)
        {
            if(m_VideoInputList[i] == m_VideoInput->GetValue())
            {
                return i;
            }
        }
    }
    else if(InputType == AUDIOINPUT)
    {
        int i;
        for(i = 0; i < m_AudioInputList.size(); i++)
        {
            if(m_AudioInputList[i] == m_AudioInput->GetValue())
            {
                return i;
            }
        }
    }
    return -1;
}

tstring CDSCaptureSource::GetInputName(eSourceInputType InputType, int Nr)
{
    if(m_pDSGraph==NULL)
    {
        return _T("");
    }
    //this shoud always succeed
    CDShowCaptureDevice *pCap=NULL;
    if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
    }
    if (pCap == NULL)
    {
        return _T("");
    }

    if(!m_HaveInputList)
    {
        // Make input list
        NumInputs(InputType);
    }
    if((InputType == VIDEOINPUT) || (InputType == AUDIOINPUT))
    {
        if((Nr>=0) && (((InputType == VIDEOINPUT) && (Nr < m_VideoInputList.size()))
            || ((InputType == AUDIOINPUT) && (Nr < m_AudioInputList.size())))
            )
        {
            try
            {
                CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
                if(pCrossbar==NULL)
                {
                    return _T("");
                }

                int nInputNumber;
                if (InputType == VIDEOINPUT)
                {
                    nInputNumber = m_VideoInputList[ Nr ];
                }
                else
                {
                    nInputNumber = m_AudioInputList[ Nr ];
                }
                return  pCrossbar->GetInputName(nInputNumber);
            }
            catch(CDShowException &e)
            {
                LOG(1,"DSCaptureSource: GetInputName: Error: %s",e.what());
            }
        }
    }
    return _T("");
}

BOOL CDSCaptureSource::InputHasTuner(eSourceInputType InputType, int Nr)
{
    if(m_pDSGraph==NULL)
    {
        return FALSE;
    }
    //this shoud always succeed
    CDShowCaptureDevice *pCap=NULL;
    if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
    {
        pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
    }
    if(pCap == NULL)
    {
        return FALSE;
    }


    if(InputType == VIDEOINPUT)
    {
        if(!m_HaveInputList)
        {
            NumInputs(InputType);
        }

        if((Nr>=0) && (Nr < m_VideoInputList.size()))
        {
            try
            {
                CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
                if(pCrossbar==NULL)
                {
                    return FALSE;
                }

                if(pCrossbar->GetInputType(m_VideoInputList[Nr]) == PhysConn_Video_Tuner)
                {
                    return TRUE;
                }
            }
            catch(CDShowException &e)
            {
                LOG(1,"DSCaptureSource: InputHasTuner: Error: %s",e.what());
            }
        }
    }

    return FALSE;
}


#endif
