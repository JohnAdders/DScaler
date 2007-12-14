/////////////////////////////////////////////////////////////////////////////
// $Id: DSProvider.cpp,v 1.11 2007-12-14 19:31:47 adcockj Exp $
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
// Revision 1.10  2003/02/05 19:39:46  tobbej
// renamed some functions
//
// Revision 1.9  2002/12/03 22:02:18  tobbej
// added some checks before enabling dshow support,
// currently it checks that dsrend is properly installed
//
// Revision 1.8  2002/08/20 16:22:59  tobbej
// split CDSSource into 3 different classes
//
// Revision 1.7  2002/05/01 20:36:49  tobbej
// renamed file input source
//
// Revision 1.6  2002/04/04 16:04:45  tobbej
// fixed error message
// fixed problem with file open not accepting dshow files (when no wdm driver is present)
//
// Revision 1.5  2002/03/26 19:48:59  adcockj
// Improved error handling in DShow code
//
// Revision 1.4  2002/02/07 22:06:26  tobbej
// new classes for file input
//
// Revision 1.3  2001/12/17 19:36:45  tobbej
// renamed a few classes
//
// Revision 1.2  2001/12/14 14:11:13  adcockj
// Added #ifdef to allow compilation without SDK
//
// Revision 1.1  2001/12/09 22:01:48  tobbej
// experimental dshow support, doesnt work yet
// define WANT_DSHOW_SUPPORT if you want to try it
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSProvider.cpp implementation of the CDSProvider class.
 */


#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
//#include "dscaler.h"
#include "DSProvider.h"
#include "devenum.h"
#include "CaptureDevice.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CDSProvider::CDSProvider()
{
	std::string ErrMsg;
	if(!CanUseDShow(ErrMsg))
	{
		//the only reason for the extra \n is to avoid text on the osd being drawn outside the window
		//(== some text will not be visible)
		std::string msg="Can't use DirectShow support because\n"+ErrMsg+"\nThe DirectShow input sources will be disabled";
		ErrorBox(msg.c_str());
		return;
	}
	
	try
	{
		try
		{
			CDShowDevEnum devenum(CLSID_VideoInputDeviceCategory);
			//get all video capture devices
			while(devenum.getNext()==true)
			{
				string deviceName=devenum.getProperty("FriendlyName");
				CDSCaptureSource *tmpsrc=new CDSCaptureSource(devenum.getDisplayName(),deviceName);
				m_DSSources.push_back(tmpsrc);
				m_SourceNames[m_DSSources.size()-1]=deviceName;
			}
		}
		catch(CDShowDevEnumException&)
		{
			//creation of device enumerator failed, this probably means that CLSID_VideoInputDeviceCategory is empty/non existant
		}

		//add one file source
		CDSFileSource *src=new CDSFileSource();
		m_DSSources.push_back(src);
		m_SourceNames[m_DSSources.size()-1]="Media file";
    }
    catch(std::runtime_error& e)
    {
        ErrorBox(e.what());
    }
    catch(CDShowException& e)
    {
        ErrorBox(e.getErrorText());
    }
    catch(...)
    {
        ErrorBox("Unexpected Error");
    }
}

CDSProvider::~CDSProvider()
{
	//remove all sources
	for(vector<CDSSourceBase*>::iterator it=m_DSSources.begin();it!=m_DSSources.end();it++)
	{
		delete *it;
	}
}

string CDSProvider::GetSourceName(int SourceIndex)
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

	if(SourceIndex>=0 && SourceIndex<m_DSSources.size())
	{
		return m_DSSources[SourceIndex];
	}
	else
	{
		return NULL;
	}
}

bool CDSProvider::CanUseDShow(std::string &FailMsg)
{
	///@todo add a check for directx version

	//check dsrend.dll filter
	CComPtr<IBaseFilter> filter;
	HRESULT hr=filter.CoCreateInstance(CLSID_DSRendFilter);
	if(FAILED(hr))
	{
		///@todo maybe try to register dsrend.dll and only return false if it failed
		FailMsg="the 'DScaler renderer filter' is not properly installed";
		return false;
	}
	CComPtr<IDSRendFilter> DSRendIf;
	hr=filter.QueryInterface(&DSRendIf);
	if(FAILED(hr))
	{
		FailMsg="the 'DScaler renderer filter' does not support the necessary interface\n(maybe the filter is of the wrong version)";
		return false;
	}
	return true;
}

#endif