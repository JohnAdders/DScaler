/////////////////////////////////////////////////////////////////////////////
// $Id: CaptureDevice.cpp,v 1.1 2001-12-17 19:28:26 tobbej Exp $
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
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DShowCaptureDevice.cpp implementation of the CDShowCaptureDevice class.
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
:CDShowObject(pGraph),m_bIsConnected(false),m_pCrossbar(NULL)
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
	HRESULT hr=m_pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,&MEDIATYPE_Video,m_vidDev,NULL,filter);
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
#endif