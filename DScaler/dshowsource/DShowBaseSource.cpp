/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
 * @file DShowBaseSource.cpp implementation of the CDShowBaseSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DShowBaseSource.h"
#include "DevEnum.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDShowBaseSource::CDShowBaseSource(IGraphBuilder *pGraph)
:CDShowObject(pGraph)
{

}

CDShowBaseSource::~CDShowBaseSource()
{

}

void CDShowBaseSource::SetAudioDevice(std::string device)
{
    m_AudioDevice=device;
}

CComPtr<IBaseFilter> CDShowBaseSource::GetNewAudioRenderer()
{
    if(m_AudioDevice.size()==0)
    {
        return NULL;
    }
    else
    {
        CComPtr<IBaseFilter> pAudioDevice;
        try
        {
            CDShowDevEnum::createDevice(m_AudioDevice,IID_IBaseFilter,&pAudioDevice);
        }
        catch(CDShowDevEnumException& e)
        {
            //the audio device coud not be created, change the error message
            //to something less cryptic
            throw CDShowDevEnumException("The selected audio device coud not be created, please change audio device in settings",e.getErrNo());
        }

        HRESULT hr=m_pGraph->AddFilter(pAudioDevice,L"Audio Renderer");
        if(FAILED(hr))
        {
            throw CDShowException("Failed to add audio renderer to filter graph",hr);
        }
        return pAudioDevice;
    }
}

#endif