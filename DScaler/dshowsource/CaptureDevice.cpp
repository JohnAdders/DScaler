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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.19  2003/02/05 19:12:38  tobbej
// added support for capture devices where audio can be rendered from directshow
// modified audio setings dialog so audio rendering can be turned off (usefull for devices with both internal and external audio)
//
// Revision 1.18  2003/01/17 17:37:49  tobbej
// fixed crasing bug (unhandled exception)
//
// Revision 1.17  2003/01/15 20:56:19  tobbej
// added audio channel selection menu
//
// Revision 1.16  2003/01/06 21:34:29  tobbej
// implemented GetFormat (not working yet)
// implemented fm radio support
//
// Revision 1.15  2002/10/29 19:32:21  tobbej
// new tuner class for direct tuning to a frequency
// implemented IsVideoPresent, channel scaning shoud work now
//
// Revision 1.14  2002/10/27 12:18:49  tobbej
// renamed GetTVTuner
// changed FindInterface
//
// Revision 1.13  2002/09/24 17:22:19  tobbej
// a few renamed function
//
// Revision 1.12  2002/09/14 17:03:11  tobbej
// implemented audio output device selection
//
// Revision 1.11  2002/08/14 22:03:23  kooiman
// Added TV tuner support for DirectShow capture devices
//
// Revision 1.10  2002/08/10 16:52:00  tobbej
// more videoport changes, hopfully this code works on both xp and non winxp os
//
// Revision 1.9  2002/07/29 17:41:44  tobbej
// commented nonworking videoport code
//
// Revision 1.8  2002/07/17 19:18:08  tobbej
// try to connect the videoport pin first if there is one.
// commented out unused ir code
//
// Revision 1.7  2002/04/16 15:30:53  tobbej
// fixed dropped frames counter, previously it didnt find the IAMDroppedFrames when it was on one of the output pins
//
// Revision 1.6  2002/04/03 19:53:19  tobbej
// try to render interleaved stream first, this might help dv sources (untested)
//
// Revision 1.5  2002/03/15 23:08:59  tobbej
// changed dropped frames counter to include dropped frames in source filter.
// experimented a bit with btwincap:s ir support
//
// Revision 1.4  2002/02/07 22:05:43  tobbej
// new classes for file input
// rearanged class inheritance a bit
//
// Revision 1.3  2002/02/05 17:24:12  tobbej
// changed javadoc coments
//
// Revision 1.2  2002/02/03 11:04:01  tobbej
// pal/ntsc selection support
// picture controls support
//
// Revision 1.1  2001/12/17 19:28:26  tobbej
// class for capture devices
//
//
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowCaptureDevice::CDShowCaptureDevice(IGraphBuilder *pGraph,string device,string deviceName,bool bConnectAudio)
:CDShowBaseSource(pGraph),m_bIsConnected(false),m_pCrossbar(NULL),m_pTVTuner(NULL),m_pTVAudio(NULL),m_bConnectAudio(bConnectAudio)
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
	hr=m_pGraph->AddFilter(m_vidDev,A2CW(deviceName.c_str()));
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
	std::string tmp;
	DumpPreferredMediaTypes(m_vidDev,tmp);
	LOG(3,"Preferred media types for device %s\n%s",deviceName.c_str(),tmp.c_str());
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
		LOG(2,"Capture device has a VideoPort pin, trying to connect it...",hr);
		//render the vp pin
		CString tmpstr;
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
					DWORD len=AMGetErrorText(hr,tmpstr.GetBufferSetLength(MAX_ERROR_TEXT_LEN),MAX_ERROR_TEXT_LEN);
					tmpstr.ReleaseBuffer(len);
					LOG(2,"IVideoWindow::put_AutoShow failed. ErrorCode: 0x%x ErrorText: '%s'",hr,(LPCTSTR)tmpstr);
				}
				hr=pVWnd->put_Visible(OAFALSE);
				if(FAILED(hr))
				{
					DWORD len=AMGetErrorText(hr,tmpstr.GetBufferSetLength(MAX_ERROR_TEXT_LEN),MAX_ERROR_TEXT_LEN);
					tmpstr.ReleaseBuffer(len);
					LOG(2,"IVideoWindow::put_Visible failed. ErrorCode: 0x%x ErrorText: '%s'",hr,(LPCTSTR)tmpstr);
				}
			}
		}
		else
		{
			DWORD len=AMGetErrorText(hr,tmpstr.GetBufferSetLength(MAX_ERROR_TEXT_LEN),MAX_ERROR_TEXT_LEN);
			tmpstr.ReleaseBuffer(len);
			LOG(1,"Failed to connect VideoPort pin. ErrorCode: 0x%x ErrorText: '%s'",hr,(LPCTSTR)tmpstr);
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
	m_bIsConnected=true;
	if(m_pDroppedFrames==NULL)
	{
		findIAMDroppedFrames(VideoFilter);
	}

	if(m_bConnectAudio)
	{
		//try to render audio, if this fails then this device probably don't have any audio
		bool bAudioRendered=false;
		int AudioStreamCount=0;
		/*
		This will always add one extra unconnected audio renderer when using
		a user specified audio renderer
		*/
		while(hr=m_pBuilder->RenderStream(NULL,&MEDIATYPE_Audio,m_vidDev,NULL,GetNewAudioRenderer()),SUCCEEDED(hr))
		{
			bAudioRendered=true;
			AudioStreamCount++;
		}
		if(bAudioRendered)
		{
			LOG(2,"DShowCaptureDevice: %d Audio streams rendered",AudioStreamCount);
		}
		else
		{
			LOG(2,"DShowCaptureDevice: Unsupported audio or no audio found, error code: 0x%x",hr);
		}

	}

	/*if(driverSupportsIR())
	{
		TRACE("Yes! driver supports ir\n");
		if(isRemotePresent())
		{
			TRACE("Remote is present\n");
			ULONG code=getRemoteCode();
			if(code&0x10000)
			{
				code=code & ~0x10000;
				TRACE("Remote code=0x%x\n",code);
			}
		}
		else
		{
			TRACE("No remote\n");
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
bool CDShowCaptureDevice::driverSupportsIR()
{
	CComPtr<IKsPropertySet> pPropSet;
	if(SUCCEEDED(m_vidDev.QueryInterface(&pPropSet)))
	{
		ULONG supported;
		HRESULT hr=pPropSet->QuerySupported(PROPSETID_IR,KSPROPERTY_IR_CAPS,&supported);
		if(SUCCEEDED(hr) && (supported & KSPROPERTY_SUPPORT_GET))
		{
			return true;
		}

	}
	return false;
}

bool CDShowCaptureDevice::isRemotePresent()
{
	if(!driverSupportsIR())
		return false;

	CComPtr<IKsPropertySet> pPropSet;
	if(SUCCEEDED(m_vidDev.QueryInterface(&pPropSet)))
	{
		KSPROPERTY_IR_CAPS_S caps;
		ULONG bytes=sizeof(KSPROPERTY_IR_CAPS_S);
		HRESULT hr=pPropSet->Get(PROPSETID_IR,KSPROPERTY_IR_CAPS,&caps,sizeof(KSPROPERTY_IR_CAPS_S),&caps,sizeof(KSPROPERTY_IR_CAPS_S),&bytes);
		if(SUCCEEDED(hr) && (caps.Caps & KSPROPERTY_IR_CAPS_AVAILABLE))
		{
			return true;
		}
	}
	return false;
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
			//TRACE("Got ircode: %lu\n",(tmp.Code & ~0x10000));
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
	LOG(2,"DShowCaptureDevice: GetTuner");
	if(m_pTVTuner==NULL)
	{
		LOG(2,"DShowCaptureDevice: find Tuner");

		CComPtr<IAMTVTuner> pTVTuner;
		HRESULT hr=m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY,NULL,m_vidDev,IID_IAMTVTuner,(void**)&pTVTuner);
		if(SUCCEEDED(hr))
		{
			m_pTVTuner=new CDShowDirectTuner(pTVTuner, m_pGraph);
			LOG(2,"DShowCaptureDevice: GetTuner found");
		}
		else
		{
			LOG(2,"DShowCaptureDevice: GetTuner not found");
		}
	}
	LOG(2,"DShowCaptureDevice: GetTuner (%s)",(m_pTVTuner==NULL)?"Failed":"Ok");
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

bool CDShowCaptureDevice::IsHorizontalLocked()
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