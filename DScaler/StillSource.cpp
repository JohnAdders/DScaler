/////////////////////////////////////////////////////////////////////////////
// $Id: StillSource.cpp,v 1.6 2001-11-23 10:49:17 adcockj Exp $
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
// Revision 1.5  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.4  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.3  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.6  2001/08/23 16:04:57  adcockj
// Improvements to dynamic menus to remove requirement that they are not empty
//
// Revision 1.1.2.5  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.3  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "StillSource.h"
#include "DScaler.h"

CStillSource::CStillSource() :
    CSource(0, IDC_STILL)
{
    CreateSettings("StillSource");
}

CStillSource::~CStillSource()
{
}

void CStillSource::CreateSettings(LPCSTR IniSection)
{
}


void CStillSource::Start()
{
}

void CStillSource::Stop()
{
}

void CStillSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
    pInfo->LineLength = 720 * 2;
    pInfo->FrameWidth = 720;
    pInfo->FrameHeight = 480;
    pInfo->FieldHeight = 480 / 2;
}

BOOL CStillSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
    return FALSE;
}

LPCSTR CStillSource::GetStatus()
{
    return "Still Picture";
}

CSetting* CStillSource::GetVolume()
{
    return NULL;
}

CSetting* CStillSource::GetBalance()
{
    return NULL;
}

void CStillSource::Mute()
{
    return;
}

void CStillSource::UnMute()
{
    return;
}

CSetting* CStillSource::GetBrightness()
{
    return NULL;
}

CSetting* CStillSource::GetContrast()
{
    return NULL;
}

CSetting* CStillSource::GetHue()
{
    return NULL;
}

CSetting* CStillSource::GetSaturation()
{
    return NULL;
}

CSetting* CStillSource::GetSaturationU()
{
    return NULL;
}

CSetting* CStillSource::GetSaturationV()
{
    return NULL;
}

eVideoFormat CStillSource::GetFormat()
{
    return FORMAT_NTSC;
}

void CStillSource::Reset()
{
}

BOOL CStillSource::HasTuner()
{
    return FALSE;
}

void CStillSource::SetMenu(HMENU hMenu)
{
}

void CStillSource::HandleTimerMessages(int TimerId)
{
}

