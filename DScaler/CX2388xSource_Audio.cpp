/////////////////////////////////////////////////////////////////////////////
// $Id: CX2388xSource_Audio.cpp,v 1.3 2003-10-27 10:39:51 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
//
// This code is based on a version of dTV modified by Michael Eskin and
// others at Connexant.  Those parts are probably (c) Connexant 2002
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/12/10 14:53:16  adcockj
// Sound fixes for cx2388x
//
// Revision 1.1  2002/10/31 15:55:50  adcockj
// Moved audio code from Connexant dTV version
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file CX2388xSource.cpp CCX2388xSource Implementation (Audio)
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "CX2388xSource.h"
#include "CX2388x_Defines.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "SettingsPerChannel.h"

void CCX2388xSource::Mute()
{
    m_pCard->SetAudioMute();
}

void CCX2388xSource::UnMute()
{
    m_pCard->SetAudioUnMute(m_Volume->GetValue());
}

void CCX2388xSource::VolumeOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioVolume(NewValue);    
	EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
}

void CCX2388xSource::BalanceOnChange(long NewValue, long OldValue)
{
    m_pCard->SetAudioBalance(NewValue);
}

void CCX2388xSource::AudioStandardOnChange(long NewValue, long OldValue)
{
    m_pCard->AudioInit(
                        m_VideoSource->GetValue(), 
                        (eVideoFormat)m_VideoFormat->GetValue(), 
                        (CCX2388xCard::eAudioStandard)NewValue,
                        (CCX2388xCard::eStereoType)m_StereoType->GetValue()
                      );
}

void CCX2388xSource::StereoTypeOnChange(long NewValue, long OldValue)
{
    m_pCard->AudioInit(
                        m_VideoSource->GetValue(), 
                        (eVideoFormat)m_VideoFormat->GetValue(), 
                        (CCX2388xCard::eAudioStandard)m_AudioStandard->GetValue(),
                        (CCX2388xCard::eStereoType)NewValue
                      );
}
