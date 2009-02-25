/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Revision 1.14  2003/10/10 13:34:16  laurentg
// Movie file has no more a default square pixel ratio
//
// Revision 1.13  2003/08/12 19:10:05  laurentg
// Move some methods from CDSFileSource to CDSSourceBase
//
// Revision 1.12  2003/08/12 08:56:56  laurentg
// OSD texts attached to media player actions
//
// Revision 1.11  2003/08/11 22:49:27  laurentg
// Time slider in the media player toolbar
//
// Revision 1.10  2003/07/22 22:30:20  laurentg
// Correct handling of pause (P key) for video file playing
//
// Revision 1.9  2003/02/22 16:48:56  tobbej
// added some comments about requierments for OpenFile (to avoid crashing)
//
// Revision 1.8  2003/01/18 10:49:10  laurentg
// SetOverscan renamed SetAspectRatioData
//
// Revision 1.7  2003/01/08 20:49:49  laurentg
// New settings for analogue blanking by source
//
// Revision 1.6  2003/01/07 23:31:23  laurentg
// New overscan settings
//
// Revision 1.5  2002/10/22 04:09:49  flibuste2
// -- Modified CSource to include virtual ITuner* GetTuner();
// -- Modified HasTuner() and GetTunerId() when relevant
//
// Revision 1.4  2002/09/29 09:16:25  tobbej
// changed GetStatus to return filename only
//
// Revision 1.3  2002/09/14 17:03:11  tobbej
// implemented audio output device selection
//
// Revision 1.2  2002/08/20 16:22:42  tobbej
// fixed commit log entry (accidentaly used wrong log message)
//
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
#include "TreeSettingsDlg.h"
#include "DSAudioDevicePage.h"
#include "OSD.h"

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
	CDSSourceBase::CreateSettings(IniSection);
	
}

BOOL CDSFileSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	if(CDSSourceBase::HandleWindowsCommands(hWnd,wParam,lParam)==TRUE)
	{
		return TRUE;
	}

	if(LOWORD(wParam)==IDM_DSHOW_SETTINGS)
	{
		CTreeSettingsDlg dlg(CString("DirectShow Settings"));
		CDSAudioDevicePage AudioDevice(CString("Audio output"),m_AudioDevice);

		dlg.AddPage(&AudioDevice);
		dlg.DoModal();
		return TRUE;
	}
	return FALSE;
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
	//The output thread MUST be stoped when calling this function
	//or the output thread will probably crash.
	//There is two problems that causes this
	//1: the delete of m_pDSGraph while output thread is running
	//2: in Stop() all fields from DSRend is marked as not in use, this must
	//be done since dsrend must release all buffered IMediaSamples or stop
	//will deadlock. So that makes all buffers used in the output thread invalid.

	if(m_pDSGraph!=NULL)
	{
		Stop();
		delete m_pDSGraph;
		m_pDSGraph=NULL;
	}

	m_filename="";
	try
	{
		m_pDSGraph=new CDShowGraph(FileName,m_AudioDevice);
		m_filename=FileName;
		m_pDSGraph->start();
		return TRUE;
	}
	catch(CDShowUnsupportedFileException& e)
	{
		LOG(1, "CDShowUnsupportedFileException - %s", (LPCSTR)e.getErrorText());
		return FALSE;
	}
	catch(CDShowException& e)
	{
		AfxMessageBox((LPCSTR)e.getErrorText(),MB_OK|MB_ICONERROR);
        LOG(1, "Failed to open DShow file - %s", (LPCSTR)e.getErrorText());
		return FALSE;
	}
}

void CDSFileSource::SetAspectRatioData()
{
	AspectSettings.InitialTopOverscan = 0;
	AspectSettings.InitialBottomOverscan = 0;
	AspectSettings.InitialLeftOverscan = 0;
	AspectSettings.InitialRightOverscan = 0;
    AspectSettings.bAnalogueBlanking = FALSE;
}

LPCSTR CDSFileSource::GetStatus()
{
	if(m_filename.size()>0)
	{
		m_status=m_filename;
		std::string::size_type pos=m_status.rfind('\\');
		if(pos!=std::string::npos)
		{
			m_status=m_status.substr(pos+1);
		}
		return m_status.c_str();
	}
	return "No file loaded";
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
	return FALSE;
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
			m_pDSGraph=new CDShowGraph(m_filename,m_AudioDevice);
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

void CDSFileSource::Pause()
{
	if (m_pDSGraph != NULL && m_pDSGraph->getState() == State_Running)
	{
		try
		{
			m_pDSGraph->pause();
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Pause failed\n\n")+e.getErrorText());
		}
		OSD_ShowText("Pause", 0);
	}
}

void CDSFileSource::UnPause()
{
	if (m_pDSGraph != NULL && m_pDSGraph->getState() == State_Paused)
	{
		try
		{
			m_pDSGraph->start();
			//VideoInputOnChange(m_VideoInput->GetValue(), m_VideoInput->GetValue());
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Play failed\n\n")+e.getErrorText());
		}
		OSD_ShowText("Play", 0);
	}
}

#endif