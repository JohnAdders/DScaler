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
 * @file StillProvider.cpp CStillProvider Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "StillProvider.h"
#include "Providers.h"
#include "PathHelpers.h"

using namespace std;

#define MAX_SNAPSHOT_FILES 10000


CStillProvider::CStillProvider()
{
    SmartPtr<CStillSource> pStillSource;

    pStillSource = new CStillSource(_T("Still"));
    m_StillSources.push_back(pStillSource);

    pStillSource = new CStillSource(_T("Patterns"));
    m_StillSources.push_back(pStillSource);
    tstring PlayList(GetInstallationPath());
    PlayList += _T("\\patterns\\pj_calibr.d3u");
    pStillSource->LoadPlayList(PlayList);

    pStillSource = new CStillSource(_T("Snapshots"));
    m_StillSources.push_back(pStillSource);

    pStillSource = new CStillSource(_T("DScaler intro"));
    m_StillSources.push_back(pStillSource);
    PlayList = GetInstallationPath();
    PlayList += _T("\\DScaler.d3u");
    pStillSource->OpenMediaFile(PlayList, TRUE);
    pStillSource->SetNavigOnly(TRUE);
}

CStillProvider::~CStillProvider()
{
}

int CStillProvider::GetNumberOfSources()
{
    return m_StillSources.size();
}


SmartPtr<CSource> CStillProvider::GetSource(int SourceIndex)
{
    if(SourceIndex >= 0 && SourceIndex < m_StillSources.size())
    {
        return DynamicPtrCast<CSource>(m_StillSources[SourceIndex]);
    }
    else
    {
        return SmartPtr<CSource>();
    }
}
