/////////////////////////////////////////////////////////////////////////////
// $Id: DVBTSource.cpp,v 1.1 2001-11-02 16:30:07 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DVBTSource.h"
#include "resource.h"

CDVBTSource::CDVBTSource(CDVBTCard* pDVBTCard, LPCSTR IniSection) :
    CInterlacedSource(WM_DVBT_GETVALUE, IDC_DVBT),
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
}

void CDVBTSource::Reset()
{
}


void CDVBTSource::Stop()
{
    // stop capture
    m_pDVBTCard->StopCapture();
}

void CDVBTSource::GetNextField(DEINTERLACE_INFO* pInfo, BOOL AccurateTiming)
{
}

eVideoFormat CDVBTSource::GetFormat()
{
    return FORMAT_PAL_BDGHI;
}

CSetting* CDVBTSource::GetBrightness()
{
    return NULL;
}

CSetting* CDVBTSource::GetContrast()
{
    return NULL;
}

CSetting* CDVBTSource::GetHue()
{
    return NULL;
}

CSetting* CDVBTSource::GetSaturation()
{
    return NULL;
}

CSetting* CDVBTSource::GetSaturationU()
{
    return NULL;
}

CSetting* CDVBTSource::GetSaturationV()
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
