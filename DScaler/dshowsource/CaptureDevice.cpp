/////////////////////////////////////////////////////////////////////////////
// $Id: CaptureDevice.cpp,v 1.4 2002-02-07 22:05:43 tobbej Exp $
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

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowCaptureDevice::CDShowCaptureDevice(IGraphBuilder *pGraph,string device,string deviceName)
:CDShowBaseSource(pGraph),m_bIsConnected(false),m_pCrossbar(NULL)
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
}

CDShowCaptureDevice::~CDShowCaptureDevice()
{
	if(m_pCrossbar!=NULL)
	{
		delete m_pCrossbar;
		m_pCrossbar=NULL;
	}
}

void CDShowCaptureDevice::connect(CComPtr<IBaseFilter> filter)
{
	//this will connect the capture device and add all needed filters upstream like tuners and crossbars
	HRESULT hr=m_pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video,m_vidDev,NULL,filter);
	if(FAILED(hr))
	{
		throw CDShowException("Failed to connect video capture device to renderer",hr);
	}
	m_bIsConnected=true;
}

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

long CDShowCaptureDevice::getSupportedTVFormats()
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

long CDShowCaptureDevice::getTVFormat()
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
	return format;
}

void CDShowCaptureDevice::putTVFormat(long format)
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
		throw("IAMVideoProcAmp::GetRange failed",hr);
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

#endif