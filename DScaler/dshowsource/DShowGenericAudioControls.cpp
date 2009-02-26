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
 * DShowGenericAudioControls.cpp: implementation of the CDShowGenericAudioControls class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DShowGenericAudioControls.h"
#include "exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CDShowGenericAudioControls::CDShowGenericAudioControls(CComPtr<IBasicAudio> pAudio)
:m_pAudio(pAudio)
{
    ASSERT(pAudio!=NULL);
}

CDShowGenericAudioControls::~CDShowGenericAudioControls()
{

}

int CDShowGenericAudioControls::GetAudioCaps()
{
    if(m_pAudio!=NULL)
    {
        return DSHOW_AUDIOCAPS_HAS_VOLUME|DSHOW_AUDIOCAPS_HAS_BALANCE;
    }
    else
    {
        return 0;
    }
}

long CDShowGenericAudioControls::GetVolume()
{
    if(m_pAudio==NULL)
    {
        return 0;
    }
    long volume=0;
    HRESULT hr=m_pAudio->get_Volume(&volume);
    if(FAILED(hr))
    {
        throw CDShowException("get_Volume failed",hr);
    }
    return volume/100;
}

void CDShowGenericAudioControls::SetVolume(long volume)
{
    ASSERT((volume/100)<=0 && (volume/100)>=-10000);
    if(m_pAudio==NULL)
    {
        return;
    }
    HRESULT hr=m_pAudio->put_Volume(volume*100);
    if(FAILED(hr))
    {
        throw CDShowException("put_Volume failed",hr);
    }
}

void CDShowGenericAudioControls::GetVolumeMinMax(long &min,long &max)
{
    max=0;
    min=-10000/100;
}

long CDShowGenericAudioControls::GetBalance()
{
    if(m_pAudio==NULL)
    {
        return 0;
    }
    long balance=0;
    HRESULT hr=m_pAudio->get_Balance(&balance);
    if(FAILED(hr))
    {
        throw CDShowException("get_Balance failed",hr);
    }
    return balance/100;
}

void CDShowGenericAudioControls::SetBalance(long balance)
{
    ASSERT((balance/100)<=10000 && (balance/100)>=-10000);
    if(m_pAudio==NULL)
    {
        return;
    }
    HRESULT hr=m_pAudio->put_Balance(balance*100);
    if(FAILED(hr))
    {
        throw CDShowException("put_Balance failed",hr);
    }
}

void CDShowGenericAudioControls::GetBalanceMinMax(long &min,long &max)
{
    min=-10000/100;
    max=10000/100;
}
#endif