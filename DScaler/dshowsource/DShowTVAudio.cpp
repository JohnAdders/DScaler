/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Torbjörn Jansson.  All rights reserved.
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
 * @file DShowTVAudio.cpp implementation of the CDShowTVAudio class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "DShowTVAudio.h"
#include "exception.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDShowTVAudio::CDShowTVAudio(CComPtr<IAMTVAudio> pTVAudio)
:m_pTVAudio(pTVAudio)
{

}

CDShowTVAudio::~CDShowTVAudio()
{

}

long CDShowTVAudio::GetAvailableModes()
{
    HRESULT hr;
    long modes;
    hr=m_pTVAudio->GetAvailableTVAudioModes(&modes);
    if(FAILED(hr))
    {
        throw CDShowException("CDShowTVAudio::GetAvailableModes failed",hr);
    }
    return modes;
}

void CDShowTVAudio::SetMode(TVAudioMode mode)
{
    HRESULT hr;
    hr=m_pTVAudio->put_TVAudioMode(mode);
    if(FAILED(hr))
    {
        throw CDShowException("CDShowTVAudio::SetMode failed",hr);
    }
}

TVAudioMode CDShowTVAudio::GetMode()
{
    HRESULT hr;
    TVAudioMode mode;
    hr=m_pTVAudio->get_TVAudioMode((long*)&mode);
    if(FAILED(hr))
    {
        throw CDShowException("CDShowTVAudio::GetMode failed",hr);
    }
    return mode;
}

#endif