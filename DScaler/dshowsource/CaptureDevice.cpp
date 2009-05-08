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
 * @file CaptureDevice.cpp implementation of the CDShowCaptureDevice class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "CaptureDevice.h"
#include "DevEnum.h"
#include "PinEnum.h"
#include "debuglog.h"
#include "debug.h"
//#include "btwdmprop.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowCaptureDevice::CDShowCaptureDevice(IGraphBuilder *pGraph,tstring device,tstring deviceName,BOOL bConnectAudio)
:CDShowBaseSource(pGraph),m_bIsConnected(FALSE),m_pCrossbar(NULL),m_pTVTuner(NULL),m_pTVAudio(NULL),m_bConnectAudio(bConnectAudio)
{
    USES_CONVERSION;

    HRESULT hr=m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
    if(FAILED(hr))
    {
        throw CDShowCaptureDeviceException("Failed to create capture graph builder",hr);
    }
    hr=m_pBuilder->SetFiltergraph(m_pGraph);
    if(FAILED(hr))
    {
        throw CDShowCaptureDeviceException("SetFiltergraph failed on capture graph builder",hr);
    }

    CDShowDevEnum::createDevice(device,IID_IBaseFilter,&m_vidDev);
    hr=m_pGraph->AddFilter(m_vidDev,TStringToUnicode(deviceName).c_str());
    if(FAILED(hr))
    {
        throw CDShowCaptureDeviceException("Failed to add video capture device to graph",hr);
    }

    hr=m_vidDev.QueryInterface(&m_pAVideoDec);
    if(FAILED(hr))
    {
        m_pAVideoDec=NULL;
    }
    hr=m_vidDev.QueryInterface(&m_pVideoProcAmp);
    if(FAILED(hr))
    {
        m_pVideoProcAmp=NULL;
    }
    tstring tmp;
    DumpPreferredMediaTypes(m_vidDev,tmp);
    LOG(3,_T("Preferred media types for device %s\n%s"),deviceName.c_str(),tmp.c_str());
}

CDShowCaptureDevice::~CDShowCaptureDevice()
{
    if(m_pTVAudio!=NULL)
    {
        delete m_pTVAudio;
        m_pTVAudio=NULL;
    }
    if(m_pCrossbar!=NULL)
    {
        delete m_pCrossbar;
        m_pCrossbar=NULL;
    }

    if(m_pTVTuner!=NULL)
    {
        delete m_pTVTuner;
        m_pTVTuner=NULL;
    }
}

void CDShowCaptureDevice::Connect(CComPtr<IBaseFilter> VideoFilter)
{
    //this will connect the capture device and add all needed filters upstream like tuners and crossbars

    //the following code tries to connect the video port pin of the
    //caputure filter if it has one.
    //it looks like this code needs some tweaking since it doesnt work yet.
    //winxp has a special filter that other os:es dont have.
    CComPtr<IPin> pVPPin;
    HRESULT hr=m_pBuilder->FindPin(m_vidDev,PINDIR_OUTPUT,&PIN_CATEGORY_VIDEOPORT,NULL,TRUE,0,&pVPPin);
    if(SUCCEEDED(hr))
    {
        LOG(2,_T("Capture device has a VideoPort pin, trying to connect it..."),hr);
        //render the vp pin
        hr=m_pGraph->Render(pVPPin);
        if(SUCCEEDED(hr))
        {
            //the render call above most likely inserted a video renderer filter,
            //disable it so we dont get an extra window.
            CComPtr<IVideoWindow> pVWnd;
            hr=m_pBuilder->FindInterface(&LOOK_DOWNSTREAM_ONLY,NULL,m_vidDev,IID_IVideoWindow,(void**)&pVWnd);
            if(SUCCEEDED(hr))
            {
                hr=pVWnd->put_AutoShow(OAFALSE);
                if(FAILED(hr))
                {
                    vector<TCHAR> tmpstr(MAX_ERROR_TEXT_LEN);
                    DWORD len=AMGetErrorText(hr,&tmpstr[0],MAX_ERROR_TEXT_LEN);
                    LOG(2,_T("IVideoWindow::put_AutoShow failed. ErrorCode: 0x%x ErrorText: '%s'"),hr,&tmpstr[0]);
                }
                hr=pVWnd->put_Visible(OAFALSE);
                if(FAILED(hr))
                {
                    vector<TCHAR> tmpstr(MAX_ERROR_TEXT_LEN);
                    DWORD len=AMGetErrorText(hr,&tmpstr[0],MAX_ERROR_TEXT_LEN);
                    LOG(2,_T("IVideoWindow::put_Visible failed. ErrorCode: 0x%x ErrorText: '%s'"),hr,&tmpstr[0]);
                }
            }
        }
        else
        {
            vector<TCHAR> tmpstr(MAX_ERROR_TEXT_LEN);
            DWORD len=AMGetErrorText(hr,&tmpstr[0],MAX_ERROR_TEXT_LEN);
            LOG(1,_T("Failed to connect VideoPort pin. ErrorCode: 0x%x ErrorText: '%s'"),hr,&tmpstr[0]);
        }
    }

    //first try to render interleaved (dv source), if it fails try normal render
    hr=m_pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Interleaved,m_vidDev,NULL,VideoFilter);
    if(FAILED(hr))
    {
        hr=m_pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video,m_vidDev,NULL,VideoFilter);
        if(FAILED(hr))
        {
            throw CDShowException("Failed to connect video capture device to renderer",hr);
        }
    }
    m_bIsConnected=TRUE;
    if(m_pDroppedFrames==NULL)
    {
        findIAMDroppedFrames(VideoFilter);
    }

    if(m_bConnectAudio)
    {
        //try to render audio, if this fails then this device probably don't have any audio
        BOOL bAudioRendered=FALSE;
        int AudioStreamCount=0;
        /*
        This will always add one extra unconnected audio renderer when using
        a user specified audio renderer
        */
        while(hr=m_pBuilder->RenderStream(NULL,&MEDIATYPE_Audio,m_vidDev,NULL,GetNewAudioRenderer()),SUCCEEDED(hr))
        {
            bAudioRendered=TRUE;
            AudioStreamCount++;
        }
        if(bAudioRendered)
        {
            LOG(2,_T("DShowCaptureDevice: %d Audio streams rendered"),AudioStreamCount);
        }
        else
        {
            LOG(2,_T("DShowCaptureDevice: Unsupported audio or no audio found, error code: 0x%x"),hr);
        }

    }

    /*if(driverSupportsIR())
    {
        LOGD(_T("Yes! driver supports ir\n"));
        if(isRemotePresent())
        {
            LOGD(_T("Remote is present\n"));
            ULONG code=getRemoteCode();
            if(code&0x10000)
            {
                code=code & ~0x10000;
                LOGD(_T("Remote code=0x%x\n"),code);
            }
        }
        else
        {
            LOGD(_T("No remote\n"));
        }
    }*/
}

void CDShowCaptureDevice::findIAMDroppedFrames(CComPtr<IBaseFilter> filter)
{
    CComPtr<IAMDroppedFrames> df;
    //try to find IAMDroppedFrames on the filter itself first
    HRESULT hr=m_vidDev.QueryInterface(&df);
    if(SUCCEEDED(hr))
    {
        m_pDroppedFrames=df;
    }
    else
    {
        //try to find the interface on one of the pins
        hr=m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,&MEDIATYPE_Video,filter,IID_IAMDroppedFrames,(void**)&df);
        if(SUCCEEDED(hr))
        {
            m_pDroppedFrames=df;
        }
    }
}

long CDShowCaptureDevice::GetNumDroppedFrames()
{
    if(m_pDroppedFrames==NULL)
    {
        return 0;
    }

    long dropped=0;
    HRESULT hr=m_pDroppedFrames->GetNumDropped(&dropped);
    if(FAILED(hr))
    {
        return 0;
    }
    return dropped;
}
/*
BOOL CDShowCaptureDevice::driverSupportsIR()
{
    CComPtr<IKsPropertySet> pPropSet;
    if(SUCCEEDED(m_vidDev.QueryInterface(&pPropSet)))
    {
        ULONG supported;
        HRESULT hr=pPropSet->QuerySupported(PROPSETID_IR,KSPROPERTY_IR_CAPS,&supported);
        if(SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_GET))
        {
            return TRUE;
        }

    }
    return FALSE;
}

BOOL CDShowCaptureDevice::isRemotePresent()
{
    if(!driverSupportsIR())
        return FALSE;

    CComPtr<IKsPropertySet> pPropSet;
    if(SUCCEEDED(m_vidDev.QueryInterface(&pPropSet)))
    {
        KSPROPERTY_IR_CAPS_S caps;
        ULONG bytes=sizeof(KSPROPERTY_IR_CAPS_S);
        HRESULT hr=pPropSet->Get(PROPSETID_IR,KSPROPERTY_IR_CAPS,&caps,sizeof(KSPROPERTY_IR_CAPS_S),&caps,sizeof(KSPROPERTY_IR_CAPS_S),&bytes);
        if(SUCCEEDED(hr) && (caps.Caps & KSPROPERTY_IR_CAPS_AVAILABLE))
        {
            return TRUE;
        }
    }
    return FALSE;
}

ULONG CDShowCaptureDevice::getRemoteCode()
{
    CComPtr<IKsPropertySet> pPropSet;
    if(SUCCEEDED(m_vidDev.QueryInterface(&pPropSet)))
    {
        //get the ir code
        KSPROPERTY_IR_GETCODE_S tmp;
        ULONG bytes=sizeof(KSPROPERTY_IR_GETCODE_S);
        HRESULT hr=pPropSet->Get(PROPSETID_IR,KSPROPERTY_IR_GETCODE,&tmp,sizeof(KSPROPERTY_IR_GETCODE_S),&tmp,sizeof(KSPROPERTY_IR_GETCODE_S),&bytes);
        if(SUCCEEDED(hr))
        {
            //LOGD(_T("Got ircode: %lu\n"),(tmp.Code & ~0x10000));
            return tmp.Code;
        }
    }
    return 0;
}*/

CDShowBaseCrossbar* CDShowCaptureDevice::getCrossbar()
{
    if(m_pCrossbar==NULL)
    {
        CComPtr<IAMCrossbar> pCrossbar;

        //FindInterface adds any nessesary filters upstream of the videocapture device
        //like tvtunners and crossbars
        HRESULT hr=m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,NULL,m_vidDev,IID_IAMCrossbar,(void**)&pCrossbar);
        if(SUCCEEDED(hr))
        {
            m_pCrossbar=new CDShowSingleCrossbar(pCrossbar,m_pGraph);
        }
    }
    return m_pCrossbar;
}

CDShowDirectTuner* CDShowCaptureDevice::GetTuner()
{
    LOG(2,_T("DShowCaptureDevice: GetTuner"));
    if(m_pTVTuner==NULL)
    {
        LOG(2,_T("DShowCaptureDevice: find Tuner"));

        CComPtr<IAMTVTuner> pTVTuner;
        HRESULT hr=m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,NULL,m_vidDev,IID_IAMTVTuner,(void**)&pTVTuner);
        if(SUCCEEDED(hr))
        {
            m_pTVTuner=new CDShowDirectTuner(pTVTuner, m_pGraph);
            LOG(2,_T("DShowCaptureDevice: GetTuner found"));
        }
        else
        {
            LOG(2,_T("DShowCaptureDevice: GetTuner not found"));
        }
    }
    LOG(2,_T("DShowCaptureDevice: GetTuner (%s)"),(m_pTVTuner==NULL)?_T("Failed"):_T("Ok"));
    return m_pTVTuner;
}

long CDShowCaptureDevice::GetSupportedTVFormats()
{
    if(m_pAVideoDec==NULL)
    {
        throw CDShowException("No IAMAnalogVideoDecoder interface");
    }
    long formats;
    HRESULT hr=m_pAVideoDec->get_AvailableTVFormats(&formats);
    if(FAILED(hr))
    {
        throw CDShowException("Failed to get supported tvformats",hr);
    }
    return formats;
}

AnalogVideoStandard CDShowCaptureDevice::GetTVFormat()
{
    if(m_pAVideoDec==NULL)
    {
        throw CDShowException("No IAMAnalogVideoDecoder interface");
    }
    long format;
    HRESULT hr=m_pAVideoDec->get_TVFormat(&format);
    if(FAILED(hr))
    {
        throw CDShowException("Failed to get tvformat",hr);
    }
    return (AnalogVideoStandard)format;
}

void CDShowCaptureDevice::PutTVFormat(AnalogVideoStandard format)
{
    if(m_pAVideoDec==NULL)
    {
        throw CDShowException("No IAMAnalogVideoDecoder interface");
    }
    HRESULT hr=m_pAVideoDec->put_TVFormat(format);
    if(FAILED(hr))
    {
        throw CDShowException("Failed to set tvformat",hr);
    }
}

BOOL CDShowCaptureDevice::IsHorizontalLocked()
{
    if(m_pAVideoDec==NULL)
    {
        throw CDShowException("No IAMAnalogVideoDecoder interface");
    }
    long hlock=0;
    HRESULT hr=m_pAVideoDec->get_HorizontalLocked(&hlock);
    if(FAILED(hr))
    {
        throw CDShowException("get_HorizontalLocked Failed",hr);
    }
    return hlock!=0;
}

void CDShowCaptureDevice::set(long prop,long value,long flags)
{
    if(m_pVideoProcAmp==NULL)
    {
        throw CDShowException("No IAMVideoProcAmp interface");
    }
    HRESULT hr=m_pVideoProcAmp->Set(prop,value,flags);
    if(FAILED(hr))
    {
        throw CDShowException("IAMVideoProcAmp::Set failed",hr);
    }
}

void CDShowCaptureDevice::get(long prop,long *pValue,long *pFlags)
{
    if(m_pVideoProcAmp==NULL)
    {
        throw CDShowException("No IAMVideoProcAmp interface");
    }
    long value;
    long flags;

    HRESULT hr=m_pVideoProcAmp->Get(prop,&value,&flags);
    if(FAILED(hr))
    {
        throw CDShowException("IAMVideoProcAmp::Get failed",hr);
    }
    if(pValue!=NULL)
        *pValue=value;
    if(pFlags!=NULL)
        *pFlags=flags;
}

void CDShowCaptureDevice::getRange(long prop,long *pMin,long *pMax, long *pStepSize,long *pDefault,long *pFlags)
{
    if(m_pVideoProcAmp==NULL)
    {
        throw CDShowException("No IAMVideoProcAmp interface");
    }
    long min;
    long max;
    long stepSize;
    long def;
    long flags;

    HRESULT hr=m_pVideoProcAmp->GetRange(prop,&min,&max,&stepSize,&def,&flags);
    if(FAILED(hr))
    {
        throw CDShowException("IAMVideoProcAmp::GetRange failed",hr);
    }

    if(pMin!=NULL)
        *pMin=min;
    if(pMax!=NULL)
        *pMax=max;
    if(pStepSize!=NULL)
        *pStepSize=stepSize;
    if(pDefault!=NULL)
        *pDefault=def;
    if(pFlags!=NULL)
        *pFlags=flags;
}

CDShowTVAudio* CDShowCaptureDevice::GetTVAudio()
{
    if(m_pTVAudio==NULL)
    {
        CComPtr<IAMTVAudio> pTVAudio;
        HRESULT hr=m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,NULL,m_vidDev,IID_IAMTVAudio,(void**)&pTVAudio);
        if(SUCCEEDED(hr))
        {
            m_pTVAudio=new CDShowTVAudio(pTVAudio);
        }
    }
    return m_pTVAudio;
}
#endif