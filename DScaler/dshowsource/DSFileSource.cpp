/////////////////////////////////////////////////////////////////////////////
// $Id: DSFileSource.cpp,v 1.2 2002-08-20 16:22:42 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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
// Revision 1.1  2002/08/20 16:19:42  tobbej
// split CDSSource into 3 different classes
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSFileSource.cpp implementation of the CDSFileSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dscaler.h"
#include "DSFileSource.h"
#include "DShowFileSource.h"
#include "DebugLog.h"
#include "AspectRatio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CDSFileSource::CDSFileSource()
:CDSSourceBase(0,IDC_DSHOW_FILESOURCE_MENU)
{
	m_IDString = std::string("DS_DShowFileInput");
	CreateSettings("DShowFileInput");
}

CDSFileSource::~CDSFileSource()
{

}

void CDSFileSource::CreateSettings(LPCSTR IniSection)
{

}

BOOL CDSFileSource::IsAccessAllowed()
{
	if(m_filename.size()>0)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CDSFileSource::OpenMediaFile(LPCSTR FileName, BOOL NewPlayList)
{
	if(m_pDSGraph!=NULL)
	{
		Stop();
		delete m_pDSGraph;
		m_pDSGraph=NULL;
	}

	m_filename="";
	try
	{
		m_pDSGraph=new CDShowGraph(FileName);
		m_filename=FileName;
		m_pDSGraph->start();
		return TRUE;
	}
	catch(CDShowUnsupportedFileException e)
	{
		LOG(1, "CDShowUnsupportedFileException - %s", (LPCSTR)e.getErrorText());
		return FALSE;
	}
	catch(CDShowException e)
	{
		AfxMessageBox((LPCSTR)e.getErrorText(),MB_OK|MB_ICONERROR);
        LOG(1, "Failed to open DShow file - %s", (LPCSTR)e.getErrorText());
		return FALSE;
	}
}

void CDSFileSource::SetOverscan()
{
	AspectSettings.InitialOverscan = 0;
}

LPCSTR CDSFileSource::GetStatus()
{
	if(m_filename.size()>0)
	{
		return m_filename.c_str();
	}
	return "No file loaded";
}

eTunerId CDSFileSource::GetTunerId()
{
	return TUNER_ABSENT;
}

BOOL CDSFileSource::HasTuner()
{
	return FALSE;
}

BOOL CDSFileSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
	return FALSE;
}

BOOL CDSFileSource::IsInTunerMode()
{
	return FALSE;
}

eVideoFormat CDSFileSource::GetFormat()
{
	//there is no videoformat that fits
	return VIDEOFORMAT_PAL_B;
}

BOOL CDSFileSource::IsVideoPresent()
{
	return TRUE;
}

BOOL CDSFileSource::HasSquarePixels()
{
	return TRUE;
}

LPCSTR CDSFileSource::GetMenuLabel()
{
	if(m_filename.size()>0)
	{
		///@todo remove path from filename
		return m_filename.c_str();
	}
	return NULL;
}

void CDSFileSource::Start()
{
	try
	{
		if(m_pDSGraph==NULL)
		{
			m_pDSGraph=new CDShowGraph(m_filename);
		}
		CDSSourceBase::Start();
	}
	catch(CDShowException &e)
	{
		ErrorBox(e.getErrorText());
	}
}

void CDSFileSource::HandleTimerMessages(int TimerId)
{

}

void CDSFileSource::UpdateMenu()
{

}

void CDSFileSource::SetMenu(HMENU hMenu)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	CMenu topMenu;
	topMenu.Attach(m_hMenu);
	CMenu *menu=topMenu.GetSubMenu(0);

	//set a radio checkmark in front of the current play/pause/stop menu entry
	FILTER_STATE state=m_pDSGraph->getState();
	UINT pos=2-state;
	menu->CheckMenuRadioItem(0,2,pos,MF_BYPOSITION);

	topMenu.Detach();
}

#endif