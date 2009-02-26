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

#include "stdafx.h"
#include "DVBTSource.h"
#include "AspectRatio.h"

CDVBTSource::CDVBTSource(CDVBTCard* pDVBTCard, LPCSTR IniSection) :
    CSource(WM_DVBT_GETVALUE, IDC_DVBT),
    m_pDVBTCard(pDVBTCard),
    m_Section(IniSection)
{
    CreateSettings(IniSection);

    ReadFromIni();
}

CDVBTSource::~CDVBTSource()
{
    delete m_pDVBTCard;
}


void CDVBTSource::CreateSettings(LPCSTR IniSection)
{
}


void CDVBTSource::Start()
{
    // stop capture
    m_pDVBTCard->StartCapture();
    NotifySquarePixelsCheck();
}

void CDVBTSource::Reset()
{
}


void CDVBTSource::Stop()
{
    // stop capture
    m_pDVBTCard->StopCapture();
}

void CDVBTSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
}

eVideoFormat CDVBTSource::GetFormat()
{
    return FORMAT_PAL_BDGHI;
}

ISetting* CDVBTSource::GetBrightness()
{
    return NULL;
}

ISetting* CDVBTSource::GetContrast()
{
    return NULL;
}

ISetting* CDVBTSource::GetHue()
{
    return NULL;
}

ISetting* CDVBTSource::GetSaturation()
{
    return NULL;
}

ISetting* CDVBTSource::GetSaturationU()
{
    return NULL;
}

ISetting* CDVBTSource::GetSaturationV()
{
    return NULL;
}

ISetting* CDVBTSource::GetTopOverscan()
{
    return NULL;
}

ISetting* CDVBTSource::GetBottomOverscan()
{
    return NULL;
}

ISetting* CDVBTSource::GetLeftOverscan()
{
    return NULL;
}

ISetting* CDVBTSource::GetRightOverscan()
{
    return NULL;
}

BOOL CDVBTSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
    return FALSE;
}

BOOL CDVBTSource::IsVideoPresent()
{
    return m_pDVBTCard->IsVideoPresent();
}

void CDVBTSource::SetAspectRatioData()
{
    AspectSettings.InitialTopOverscan = 0;
    AspectSettings.InitialBottomOverscan = 0;
    AspectSettings.InitialLeftOverscan = 0;
    AspectSettings.InitialRightOverscan = 0;
    AspectSettings.bAnalogueBlanking = FALSE;
}
