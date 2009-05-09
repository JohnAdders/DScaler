/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
 * @file Source.cpp CSource Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Events.h"
#include "Source.h"
#include "DScaler.h"
#include "Providers.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"

CSource::CSource(long SetMessage, long MenuId) :
    CSettingsHolder(SetMessage),
    m_FieldFrequency(0),
    m_Comments(_T(""))
{
    m_hMenu = LoadMenu(hResourceInst, MAKEINTRESOURCE(MenuId));
}

CSource::~CSource()
{
    DestroyMenu(m_hMenu);
}

double CSource::GetFieldFrequency()
{
    return m_FieldFrequency;
}

HMENU CSource::GetSourceMenu()
{
    return m_hMenu;
}

const TCHAR* CSource::GetComments()
{
    return m_Comments.c_str();
}

void CSource::NotifySizeChange()
{
    if (Providers_GetCurrentSource() == this)
    {
        //tell dscaler that size has changed, the real work will be done in the main message loop
        PostMessageToMainWindow(UWM_INPUTSIZE_CHANGE,0,0);
    }
}

void CSource::NotifySquarePixelsCheck()
{
    if (Providers_GetCurrentSource() == this)
    {
        // Tell dscaler to check whether _T("square pixels") AR mode must be on or off
        // The real work will be done in the main message loop
        PostMessageToMainWindow(UWM_SQUAREPIXELS_CHECK,0,0);
    }
}

BOOL CSource::HasTuner()
{
    if (!GetTuner().IsValid())
    {
        return FALSE;
    }
    //this may differ from base behaviour..
    if (GetTunerId() == TUNER_ABSENT)
    {
        return FALSE;
    }
    return (GetTuner()->GetTunerId() != TUNER_ABSENT);
}


void CSource::ChangeDefaultsForSetup(WORD Setup, BOOL bDontSetValue)
{
    if (Setup & SETUP_CHANGE_VIDEOINPUT)
    {
        ChangeDefaultsForVideoInput(bDontSetValue);
    }
    if (Setup & SETUP_CHANGE_VIDEOFORMAT)
    {
        ChangeDefaultsForVideoFormat(bDontSetValue);
    }
    if (Setup & SETUP_CHANGE_AUDIOINPUT)
    {
        ChangeDefaultsForAudioInput(bDontSetValue);
    }
}

void CSource::SetSourceAsCurrent()
{
    // may need to register settings in here
    // not sure yet

    if (GetVolume() == NULL)
    {
        EventCollector->RaiseEvent(this, EVENT_NO_VOLUME, 0, 1);
    }
}

void CSource::UnsetSourceAsCurrent()
{
    SettingsMaster->SaveGroupedSettings();
}
