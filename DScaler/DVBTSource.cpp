/////////////////////////////////////////////////////////////////////////////
// $Id: DVBTSource.cpp,v 1.7 2002-04-15 22:50:09 laurentg Exp $
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
// Revision 1.6  2002/02/09 02:44:56  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.5  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.4  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.3  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.2  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.1  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
//////////////////////////////////////////////////////////////////////////////

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

ISetting* CDVBTSource::GetOverscan()
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

void CDVBTSource::SetOverscan()
{
    AspectSettings.InitialOverscan = 0;
}
