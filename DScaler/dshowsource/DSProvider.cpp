/////////////////////////////////////////////////////////////////////////////
// $Id: DSProvider.cpp,v 1.1 2001-12-09 22:01:48 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSProvider.cpp implementation of the CDSProvider class.
 */

#include "stdafx.h"
//#include "dscaler.h"
#include "DSProvider.h"
#include "devenum.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CDSProvider::CDSProvider()
{
	CDevEnum devenum(CLSID_VideoInputDeviceCategory);
	
	while(devenum.getNext()==true)
	{
		string deviceName=devenum.getProperty("FriendlyName");
		CDSSource *tmpsrc=new CDSSource(devenum.getDisplayName(),deviceName);
		m_DSSources.push_back(tmpsrc);
		m_SourceNames[m_DSSources.size()-1]=deviceName;
	}
}

CDSProvider::~CDSProvider()
{
	//remove all sources
	for(vector<CDSSource*>::iterator it=m_DSSources.begin();it!=m_DSSources.end();it++)
	{
		delete *it;
	}
}

string CDSProvider::getSourceName(int SourceIndex)
{
	ASSERT(SourceIndex>=0 && SourceIndex<m_DSSources.size());
	return m_SourceNames[SourceIndex];
}

int CDSProvider::GetNumberOfSources()
{
	return m_DSSources.size();
}

CSource* CDSProvider::GetSource(int SourceIndex)
{
	ASSERT(SourceIndex>=0 && SourceIndex<m_DSSources.size());
	return m_DSSources[SourceIndex];
}