/////////////////////////////////////////////////////////////////////////////
// $Id: StillProvider.cpp,v 1.23 2002-10-27 11:29:29 laurentg Exp $
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
// Revision 1.22  2002/10/26 21:37:13  laurentg
// Take consecutive stills
//
// Revision 1.21  2002/10/26 17:56:19  laurentg
// Possibility to take stills in memory added
//
// Revision 1.20  2002/07/25 20:43:56  laurentg
// Setting added to take still always in the same file
//
// Revision 1.19  2002/06/30 18:52:31  laurentg
// Text displayed (OSD) when taking a still
//
// Revision 1.18  2002/05/25 17:57:28  laurentg
// no message
//
// Revision 1.17  2002/05/23 21:25:33  robmuller
// Applied patch #558348 by PietOO.
// New naming of snapshots.
//
// Revision 1.16  2002/05/02 20:16:27  laurentg
// JPEG format added to take still
//
// Revision 1.15  2002/04/27 00:38:33  laurentg
// New default source (still) used at DScaler startup or when there is no more source accessible
//
// Revision 1.14  2002/03/30 13:18:31  laurentg
// New ini setting to choose the directory where to save snapshots
//
// Revision 1.13  2002/03/29 09:14:00  robmuller
// Fixed lockup when there are too many captures.
//
// Revision 1.12  2002/02/18 23:25:01  laurentg
// At startup, go to the first source having a content
// Order of still sources changed (Patterns before Snapshots)
//
// Revision 1.11  2002/02/11 21:33:13  laurentg
// Patterns as a new source from the Still provider
//
// Revision 1.10  2001/12/08 13:48:40  laurentg
// New StillSource for snapshots done during the DScaler session
//
// Revision 1.9  2001/12/08 12:04:07  laurentg
// New setting m_StillFormat
//
// Revision 1.8  2001/11/28 16:04:50  adcockj
// Major reorganization of STill support
//
// Revision 1.7  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.6  2001/11/25 21:21:56  laurentg
// Destructor modified to delete sources
//
// Revision 1.5  2001/11/24 17:58:06  laurentg
// Still source
//
// Revision 1.4  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.3  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.2  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.1  2001/08/15 14:44:05  adcockj
// Starting to put some flesh onto the new structure
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "StillProvider.h"
#include "Providers.h"


#define MAX_SNAPSHOT_FILES 10000


CStillProvider::CStillProvider()
{
    char PlayList[MAX_PATH];
    CStillSource* pStillSource;

    pStillSource = new CStillSource("Still");
    m_StillSources.push_back(pStillSource);

    pStillSource = new CStillSource("Patterns");
    m_StillSources.push_back(pStillSource);
    GetModuleFileName (NULL, PlayList, sizeof(PlayList));
    strcpy(strrchr(PlayList, '\\'), "\\patterns\\pj_calibr.d3u");
    pStillSource->LoadPlayList(PlayList);

    pStillSource = new CStillSource("Snapshots");
    m_StillSources.push_back(pStillSource);

    pStillSource = new CStillSource("DScaler intro");
    m_StillSources.push_back(pStillSource);
    GetModuleFileName (NULL, PlayList, sizeof(PlayList));
    strcpy(strrchr(PlayList, '\\'), "\\DScaler.d3u");
    pStillSource->OpenMediaFile(PlayList, TRUE);
    pStillSource->SetNavigOnly(TRUE);
}

CStillProvider::~CStillProvider()
{
    for(vector<CStillSource*>::iterator it = m_StillSources.begin();
        it != m_StillSources.end();
        ++it)
    {
        delete *it;
    }
}

int CStillProvider::GetNumberOfSources()
{
    return m_StillSources.size();
}


CSource* CStillProvider::GetSource(int SourceIndex)
{
    if(SourceIndex >= 0 && SourceIndex < m_StillSources.size())
    {
        return m_StillSources[SourceIndex];
    }
    else
    {
        return NULL;
    }
}

void StillProvider_SaveSnapshot(TDeinterlaceInfo* pInfo, BYTE* pAllocBuffer, BOOL InMemory)
{
	// The overlay (back buffer) is already locked
	// and pInfo->Overlay is a pointer to this overlay

    CStillSource* pStillSource = (CStillSource*) Providers_GetSnapshotsSource();

    if(pStillSource != NULL)
    {
		if (InMemory)
		{
			pStillSource->SaveSnapshotInMemory(pInfo->FrameHeight, pInfo->FrameWidth, pInfo->Overlay, pInfo->OverlayPitch, pAllocBuffer);
		}
		else
		{
			pStillSource->SaveSnapshotInFile(pInfo->FrameHeight, pInfo->FrameWidth, pInfo->Overlay, pInfo->OverlayPitch);
		}
    }
	else
	{
		if (pAllocBuffer != NULL)
		{
			free(pAllocBuffer);
		}
	}
}
