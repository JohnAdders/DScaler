/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.cpp,v 1.1 2001-12-09 22:01:48 tobbej Exp $
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
 * @file DSSource.cpp implementation of the CDSSource class.
 */

#include "stdafx.h"
#include "dscaler.h"
#include "..\DScalerRes\resource.h"
#include "DSSource.h"
#include "DevEnum.h"
#include "debug.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/// @todo better error handling
CDSSource::CDSSource(string device,string deviceName)
:CSource(0,IDC_WDMSOURCE)
{
	USES_CONVERSION;
	HRESULT hr=m_pBuilder.CoCreateInstance(CLSID_CaptureGraphBuilder2);
	HRESULT hr1=m_pGraph.CoCreateInstance(CLSID_FilterGraph);

	if(FAILED(hr) || FAILED(hr1))
	{
		//FIXME: handle error
	}
	
	//connect capture graph builder with the graph
	m_pBuilder->SetFiltergraph(m_pGraph);

	//this makes it posibel to connect graphedit to the graph
#ifdef _DEBUG
	AddToRot(m_pGraph,&m_hROT);
#endif

	//get mediacontrol so it is posibel to start and stop the filter graph
	hr=m_pGraph.QueryInterface(&m_pControl);
	if(FAILED(hr))
	{
		//FIXME:handle error
	}

	try
	{
		CDevEnum::createDevice(device,IID_IBaseFilter,&m_vidDev);
		HRESULT hr=m_pGraph->AddFilter(m_vidDev,A2CW(deviceName.c_str()));
		if(FAILED(hr))
		{
		
		}
	}
	catch(CDevEnumException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONERROR);
	}
}

CDSSource::~CDSSource()
{
#ifdef _DEBUG
	RemoveFromRot(m_hROT);
#endif
}

void CDSSource::CreateSettings(LPCSTR IniSection)
{

}

BOOL CDSSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	return FALSE;
}

void CDSSource::Start()
{

}

void CDSSource::Stop()
{

}

void CDSSource::Reset()
{

}

eVideoFormat CDSSource::GetFormat()
{
	return VIDEOFORMAT_PAL_B;
}

BOOL CDSSource::IsInTunerMode()
{
	return FALSE;
}

int CDSSource::GetWidth()
{
	return 0;
}

int CDSSource::GetHeight()
{
	return 0;
}

BOOL CDSSource::HasTuner()
{
	return FALSE;
}

BOOL CDSSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
	return FALSE;
}

BOOL CDSSource::IsVideoPresent()
{
	return FALSE;
}

void CDSSource::SetMenu(HMENU hMenu)
{

}

void CDSSource::HandleTimerMessages(int TimerId)
{

}

LPCSTR CDSSource::GetMenuLabel()
{
	return "FIXME: not working";
}

LPCSTR CDSSource::GetStatus()
{
	return "This source dont work yet";
}

void CDSSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{

}