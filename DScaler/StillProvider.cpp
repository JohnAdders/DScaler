/////////////////////////////////////////////////////////////////////////////
// $Id: StillProvider.cpp,v 1.7 2001-11-26 13:02:27 adcockj Exp $
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
#include "TiffSource.h"


CStillProvider::CStillProvider()
{
    CStillSource* pNewSource = new CTiffSource("testd.tif");
    m_StillSources.push_back(pNewSource);
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

BOOL CStillProvider::AddStillSource(CStillSource* pStillSource)
{
    m_StillSources.push_back(pStillSource);
    return TRUE;
}

BOOL CStillProvider::RemoveStillSource(CStillSource* pStillSource)
{
    for(vector<CStillSource*>::iterator it = m_StillSources.begin();
        it != m_StillSources.end();
        ++it)
    {
        if (*it == pStillSource)
        {
            m_StillSources.erase(it);
            return TRUE;
        }
    }
    return FALSE;
}
