/////////////////////////////////////////////////////////////////////////////
// $Id: DSSourceBase.cpp,v 1.23 2003-08-12 19:29:26 laurentg Exp $
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
// Revision 1.22  2003/08/12 19:10:05  laurentg
// Move some methods from CDSFileSource to CDSSourceBase
//
// Revision 1.21  2003/08/12 19:02:27  laurentg
// Forward and backward actions added in menu
//
// Revision 1.20  2003/08/12 08:56:56  laurentg
// OSD texts attached to media player actions
//
// Revision 1.19  2003/08/10 11:55:01  tobbej
// implemented the base for seeking in files
//
// Revision 1.18  2003/07/22 22:30:20  laurentg
// Correct handling of pause (P key) for video file playing
//
// Revision 1.17  2003/07/05 10:58:17  laurentg
// New method SetWidth (not yet implemented)
//
// Revision 1.16  2003/04/07 09:17:16  adcockj
// Fixes for correct operation of IsFirstInSeries
//
// Revision 1.15  2003/02/22 16:49:02  tobbej
// added some comments about requierments for OpenFile (to avoid crashing)
//
// Revision 1.14  2003/01/10 17:38:47  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.13  2002/12/05 21:03:46  tobbej
// changed a comment (spelling)
//
// Revision 1.12  2002/10/07 20:35:17  kooiman
// Fixed cursor hide problem
//
// Revision 1.11  2002/09/29 10:14:15  adcockj
// Fixed problem with history in OutThreads
//
// Revision 1.10  2002/09/29 09:14:36  tobbej
// fixed mute/unmute
// added some checks to volume and balance controls
//
// Revision 1.9  2002/09/28 13:36:15  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.8  2002/09/27 14:23:15  kooiman
// Added volume change event
//
// Revision 1.7  2002/09/26 10:35:34  kooiman
// Use new event code.
//
// Revision 1.6  2002/09/24 17:15:36  tobbej
// support for volume, balance and mute/unmute
//
// Revision 1.5  2002/09/16 20:08:21  adcockj
// fixed format detect for cx2388x
//
// Revision 1.4  2002/09/14 17:05:49  tobbej
// implemented audio output device selection
//
// Revision 1.3  2002/09/04 17:07:16  tobbej
// renamed some variables
// fixed bug in Reset(), it called the wrong Start()
//
// Revision 1.2  2002/08/21 20:29:20  kooiman
// Fixed settings and added setting for resolution. Fixed videoformat==lastone in dstvtuner.
//
// Revision 1.1  2002/08/20 16:21:28  tobbej
// split CDSSource into 3 different classes
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSSourceBase.cpp implementation of the CDSSourceBase class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "dscaler.h"
#include "DSSourceBase.h"
#include "AutoCriticalSection.h"
#include "FieldTiming.h"
#include "DebugLog.h"
#include "TreeSettingsDlg.h"
#include "TreeSettingsOleProperties.h"
#include "OSD.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CDSSourceBase::CDSSourceBase(long SetMessage, long MenuId) :
	CSource(SetMessage,MenuId),
	m_pDSGraph(NULL),
	m_CurrentX(0),
	m_CurrentY(0),
	m_LastNumDroppedFrames(-1),
	m_dwRendStartTime(0)
{
	InitializeCriticalSection(&m_hOutThreadSync);
}

CDSSourceBase::~CDSSourceBase()
{
	{
		CAutoCriticalSection lock(m_hOutThreadSync);
		if(m_pDSGraph!=NULL)
		{
			delete m_pDSGraph;
			m_pDSGraph=NULL;
		}
	}
	DeleteCriticalSection(&m_hOutThreadSync);
	WritePrivateProfileString(m_IniSection,_T("AudioDevice"),m_AudioDevice.c_str(),GetIniFileForSettings());
}

void CDSSourceBase::CreateSettings(LPCSTR IniSection)
{
	m_Volume = new CVolumeSetting(this, "Volume", 0, LONG_MIN, LONG_MAX, IniSection);
	m_Settings.push_back(m_Volume);

	m_Balance = new CBalanceSetting(this, "Balance", 0, LONG_MIN, LONG_MAX, IniSection);
	m_Settings.push_back(m_Balance);

	CString AudioDevice;
	m_IniSection=IniSection;
	GetPrivateProfileString(IniSection,_T("AudioDevice"),_T(""),AudioDevice.GetBufferSetLength(MAX_PATH),MAX_PATH,GetIniFileForSettings());
	AudioDevice.ReleaseBuffer();
	m_AudioDevice=AudioDevice;
}

int CDSSourceBase::GetWidth()
{
	return m_CurrentX;
}

int CDSSourceBase::GetHeight()
{
	return m_CurrentY;
}

void CDSSourceBase::SetWidth(int w)
{
	// todo: set the width
}

void CDSSourceBase::Start()
{
	m_LastNumDroppedFrames=-1;
	m_CurrentX=0;
	m_CurrentY=0;
	try
	{
		//derived class must create the graph first
		ASSERT(m_pDSGraph!=NULL);

		m_pDSGraph->start();
	}
	catch(CDShowException &e)
	{
		ErrorBox(e.getErrorText());
	}
	NotifySquarePixelsCheck();
}

void CDSSourceBase::Stop()
{
	CAutoCriticalSection lock(m_hOutThreadSync);
	if(m_pDSGraph!=NULL)
	{
		try
		{
			m_pDSGraph->stop();
		}
		catch(CDShowException &e)
		{
			ErrorBox(e.getErrorText());
		}
	}
}

void CDSSourceBase::Reset()
{
	Stop();
	Start();
}

void CDSSourceBase::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
	CAutoCriticalSection lock(m_hOutThreadSync);
	if(m_pDSGraph==NULL)
	{
		return;
	}
	DWORD dwLastDelay=0;
	if(m_dwRendStartTime!=0)
	{
		dwLastDelay=timeGetTime()-m_dwRendStartTime;
		//TRACE("Processing delay: %ld(ms)\n",dwLastDelay);
		m_dwRendStartTime=0;
	}
	//info to return if we fail
	pInfo->bRunningLate=TRUE;
	pInfo->bMissedFrame=TRUE;
	pInfo->FrameWidth=0;
	pInfo->FrameHeight=0;

	//clear the picture history
	memset(pInfo->PictureHistory, 0, MAX_PICTURE_HISTORY * sizeof(TPicture*));
	//ClearPictureHistory(pInfo);
	
	//is the graph running? there is no point in continuing if it isnt
	/*if(m_pDSGraph->getState()!=State_Running)
	{
		return;
	}*/
	
	long size=MAX_PICTURE_HISTORY;
	FieldBuffer fields[MAX_PICTURE_HISTORY];
	BufferInfo binfo;
	if(!m_pDSGraph->GetFields(&size,fields,binfo,dwLastDelay))
	{
		m_dwRendStartTime=0;
		UpdateDroppedFields();
		return;
	}

	//width must be 16 byte aligned or optimized memcpy will not work
	//this assert will never be triggered (checked in dsrend filter)
	ASSERT((binfo.Width&0xf)==0);

	//check if size has changed
	if(m_CurrentX!=binfo.Width || m_CurrentY!=binfo.Height*2)
	{
		m_CurrentX=binfo.Width;
		m_CurrentY=binfo.Height*2;
		NotifySizeChange();
	}
	
	pInfo->FrameWidth=binfo.Width;
	pInfo->FrameHeight=binfo.Height*2;
	pInfo->LineLength=binfo.Width*2;
	pInfo->FieldHeight=binfo.Height;
	pInfo->InputPitch=pInfo->LineLength;
	pInfo->bMissedFrame=FALSE;
	pInfo->bRunningLate=FALSE;
	pInfo->CurrentFrame=binfo.CurrentFrame;

	UpdateDroppedFields();
	
	static bool FieldFlag=true;
	for(int i=0;i<size;i++)
	{
		m_PictureHistory[i].pData=fields[i].pBuffer;
		m_PictureHistory[i].IsFirstInSeries=FALSE;
		if(fields[i].flags!=BUFFER_FLAGS_FIELD_UNKNOWN)
		{
			m_PictureHistory[i].Flags=fields[i].flags;
		}
		else
		{
			m_PictureHistory[i].Flags= FieldFlag==true ? PICTURE_INTERLACED_EVEN : PICTURE_INTERLACED_ODD;
			FieldFlag=!FieldFlag;
		}
			
		pInfo->PictureHistory[i] = &m_PictureHistory[i];
	}
	Timing_IncrementUsedFields();
	m_dwRendStartTime=timeGetTime();
}

void CDSSourceBase::UpdateDroppedFields()
{
	if(m_pDSGraph==NULL)
		return;
	
	int dropped;
	try
	{
		dropped=m_pDSGraph->getDroppedFrames();
	}
	catch(CDShowException e)
	{
		LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
		return;
	}
	
	//is the m_lastNumDroppedFrames count valid?
	if(m_LastNumDroppedFrames!=-1)
	{
		if(dropped-m_LastNumDroppedFrames >0)
		{
			Timing_AddDroppedFields((dropped-m_LastNumDroppedFrames)*2);
		}
	}
	m_LastNumDroppedFrames=dropped;
}

BOOL CDSSourceBase::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	FILTER_STATE OldState;

	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}

	switch(LOWORD(wParam))
	{
	case IDM_DSHOW_PLAY:
		if (m_pDSGraph->getState() == State_Paused)
		{
			SendMessage(hWnd, WM_COMMAND, IDM_CAPTURE_PAUSE, 0);
		}
		else if (m_pDSGraph->getState() == State_Stopped)
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
		return TRUE;
		break;

	case IDM_DSHOW_PAUSE:
		if (m_pDSGraph->getState() == State_Running)
		{
			SendMessage(hWnd, WM_COMMAND, IDM_CAPTURE_PAUSE, 0);
		}
		return TRUE;
		break;

	case IDM_DSHOW_STOP:
		OldState = m_pDSGraph->getState();
		try
		{
			//we must ensure that the output thread don't try to get any
			//more fields. if it does a deadlock can occure.
			//the deadlock will only occure if the output thread is 
			//blocked when it accesses the main threads gui, 
			//since this prevents IDSRendFilter::FreeFields from being called
			CAutoCriticalSection lock(m_hOutThreadSync);
			m_pDSGraph->stop();

			//i don't know if this is the right place to put this
			//but it looks like its working
			CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
			if(pSeeking!=NULL)
			{
				//here is a sample on how to get the positions
				//sometimes AM_SEEKING_CanGetCurrentPos is not set but 
				//IMediaSeeking::GetCurrentPosition still works.
				if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
				{
					LONGLONG pos=pSeeking->GetCurrentPos();
					LONGLONG duration=pSeeking->GetDuration();
					double pos1=pos/(double)10000000;
					double duration1=duration/(double)10000000;
					LOGD("Current media position in seconds: %f / %f\n",pos1,duration1);
				}

				//atleast one of those flags must be set to be able to seek to the begining
				if(pSeeking->GetCaps()&(AM_SEEKING_CanSeekBackwards|AM_SEEKING_CanSeekAbsolute))
				{
					//seek to the begining
					pSeeking->SeekTo(0);
				}
			}
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Stop failed\n\n")+e.getErrorText());
		}
		if (OldState == State_Paused)
		{
			SendMessage(hWnd, WM_COMMAND, IDM_CAPTURE_PAUSE, 0);
		}
		OSD_ShowText("Stop", 0);
		return TRUE;
		break;
	case IDM_DSHOW_BACKWARD_5S:
		ChangePos(-5);
		return TRUE;
		break;
	case IDM_DSHOW_BACKWARD_10S:
		ChangePos(-10);
		return TRUE;
		break;
	case IDM_DSHOW_BACKWARD_30S:
		ChangePos(-30);
		return TRUE;
		break;
	case IDM_DSHOW_BACKWARD_300S:
		ChangePos(-300);
		return TRUE;
		break;
	case IDM_DSHOW_BACKWARD_600S:
		ChangePos(-600);
		return TRUE;
		break;
	case IDM_DSHOW_BACKWARD_1800S:
		ChangePos(-1800);
		return TRUE;
		break;
	case IDM_DSHOW_FORWARD_5S:
		ChangePos(5);
		return TRUE;
		break;
	case IDM_DSHOW_FORWARD_10S:
		ChangePos(10);
		return TRUE;
		break;
	case IDM_DSHOW_FORWARD_30S:
		ChangePos(30);
		return TRUE;
		break;
	case IDM_DSHOW_FORWARD_300S:
		ChangePos(300);
		return TRUE;
		break;
	case IDM_DSHOW_FORWARD_600S:
		ChangePos(600);
		return TRUE;
		break;
	case IDM_DSHOW_FORWARD_1800S:
		ChangePos(1800);
		return TRUE;
		break;
	case IDM_DSHOW_FILTERS:
		{
			CTreeSettingsDlg dlg("Filter properties");
			CTreeSettingsPage rootPage("Filters",IDD_TREESETTINGS_EMPTY);
			int root=dlg.AddPage(&rootPage);

			int filterIndex=0;
			vector<CTreeSettingsPage*> pages;
			CTreeSettingsPage *pPage=NULL;
			
			bool bHasSubPage=false;
			while(m_pDSGraph->getFilterPropertyPage(filterIndex,&pPage,bHasSubPage))
			{
				pages.push_back(pPage);
				int filterRoot=dlg.AddPage(pPage,root);
				if(bHasSubPage)
				{
					int subIndex=0;
					while(m_pDSGraph->getFilterSubPage(filterIndex,subIndex,&pPage))
					{
						pages.push_back(pPage);
						dlg.AddPage(pPage,filterRoot);
						subIndex++;
					}
				}
				
				filterIndex++;
			}
			if(filterIndex!=0)
			{
				//show the dialog
				dlg.DoModal();
			}
			else
			{
				AfxMessageBox(_T("There is no filters to show properties for"),MB_OK|MB_ICONINFORMATION);
			}
			
			for(vector<CTreeSettingsPage*>::iterator it=pages.begin();it!=pages.end();it++)
			{
				delete *it;
			}
			return TRUE;
			break;
		}
	}
	return FALSE;
}

void CDSSourceBase::Mute()
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
	if(pControls==NULL)
	{
		LOG(3,"Failed to get audio controlls, mute will not work");
		return;
	}
	
	if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_VOLUME)
	{
		long min=0;
		long max=0;
		pControls->GetVolumeMinMax(min,max);
		
		try
		{
			pControls->SetVolume(min);
			EventCollector->RaiseEvent(this,EVENT_MUTE,0,1);
		}
		catch(CDShowException e)
		{
			LOG(3,"Exception in Mute, Error: %s",(LPCSTR)e.getErrorText());
		}
	}
}

void CDSSourceBase::UnMute()
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
	if(pControls==NULL)
	{
		LOG(3,"Failed to get audio controlls, mute will not work");
		return;
	}
	
	if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_VOLUME)
	{
		long volume=m_Volume->GetValue();
		try
		{
			pControls->SetVolume(volume);
			EventCollector->RaiseEvent(this,EVENT_MUTE,1,0);
		}
		catch(CDShowException e)
		{
			LOG(3,"Exception in UnMute, Error: %s",(LPCSTR)e.getErrorText());
		}
	}
}

ISetting* CDSSourceBase::GetVolume()
{
	if(m_pDSGraph==NULL)
	{
		return NULL;
	}
	CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
	if(pControls==NULL)
	{
		LOG(3,"Failed to get audio controlls, volume will not work");
		return NULL;
	}

	if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_VOLUME)
	{	
		long min=0;
		long max=0;
		pControls->GetVolumeMinMax(min,max);
		m_Volume->SetMax(max);
		m_Volume->SetMin(min);
		return m_Volume;
	}
	else
	{
		LOG(3,"Audio controls don't support volume");
		return NULL;
	}
}

ISetting* CDSSourceBase::GetBalance()
{
	if(m_pDSGraph==NULL)
	{
		return NULL;
	}
	CDShowAudioControls *pControls=m_pDSGraph->GetAudioControls();
	if(pControls==NULL)
	{
		LOG(3,"Failed to get audio controlls, balance will not work");
		return NULL;
	}
	
	if(pControls->GetAudioCaps()|DSHOW_AUDIOCAPS_HAS_BALANCE)
	{
		long min=0;
		long max=0;
		pControls->GetBalanceMinMax(min,max);
		m_Balance->SetMax(max);
		m_Balance->SetMin(min);
		return m_Balance;
	}
	else
	{
		LOG(3,"Audio controls don't support balance");
		return NULL;
	}
}

void CDSSourceBase::VolumeOnChange(long NewValue, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		LOG(3,"Can't change volume because there is no filter graph");
		return;
	}
	CDShowAudioControls *pControlls=m_pDSGraph->GetAudioControls();
	if(pControlls==NULL)
	{
		LOG(3,"Failed to get audio controlls");
		return;
	}

	try
	{
		pControlls->SetVolume(NewValue);
		EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, NewValue);
	}
	catch(CDShowException e)
	{
		LOG(3,"Exception in VolumeOnChange, Error: %s",(LPCSTR)e.getErrorText());
		m_Volume->SetValue(OldValue);
		EventCollector->RaiseEvent(this, EVENT_VOLUME, OldValue, OldValue);
	}
}

void CDSSourceBase::BalanceOnChange(long NewValue, long OldValue)
{
	if(m_pDSGraph==NULL)
	{
		LOG(3,"Can't change balance because there is no filter graph");
		return;
	}
	CDShowAudioControls *pControlls=m_pDSGraph->GetAudioControls();
	if(pControlls==NULL)
	{
		LOG(3,"Failed to get audio controlls");
		return;
	}

	try
	{
		pControlls->SetBalance(NewValue);
	}
	catch(CDShowException e)
	{
		LOG(3,"Exception in BalanceOnChange, Error: %s",(LPCSTR)e.getErrorText());
		m_Balance->SetValue(OldValue);
	}
}

LPCSTR CDSSourceBase::IDString()
{
	return m_IDString.c_str();
}

int CDSSourceBase::GetCurrentPos()
{
	CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
	if(pSeeking!=NULL)
	{
		if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
		{
			return pSeeking->GetCurrentPos() / 1000000;
		}
	}
	return -1;
}

void CDSSourceBase::SetPos(int pos)
{
	CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
	if(pSeeking!=NULL)
	{
		if(pSeeking->GetCaps()&AM_SEEKING_CanSeekAbsolute)
		{
			LONGLONG RealPos = (LONGLONG)pos * 1000000;
			pSeeking->SeekTo(RealPos);

			int pos1=RealPos/10000000;
			char text[32];
			sprintf(text, "Jump to time %d:%2.2d", pos1 / 60, pos1 % 60);
			OSD_ShowText(text, 0);
		}
	}
}

int CDSSourceBase::GetDuration()
{
	CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
	if(pSeeking!=NULL)
	{
		if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
		{
			return pSeeking->GetDuration() / 1000000;
		}
	}
	return -1;
}

void CDSSourceBase::ChangePos(int delta_sec)
{
	CDShowSeeking *pSeeking=m_pDSGraph->GetSeeking();
	if(pSeeking!=NULL)
	{
		if(pSeeking->GetCaps()&AM_SEEKING_CanGetDuration)
		{
			LONGLONG pos=pSeeking->GetCurrentPos();
			LONGLONG duration=pSeeking->GetDuration();
			LONGLONG newpos = pos + 10000000 * (LONGLONG)delta_sec;
			if (newpos < 0)
			{
				newpos = 0;
			}
			else if (newpos > duration)
			{
				newpos = duration;
			}
			if ( ( (delta_sec < 0) && (pSeeking->GetCaps()&(AM_SEEKING_CanSeekBackwards|AM_SEEKING_CanSeekAbsolute)) )
			  || ( (delta_sec > 0) && (pSeeking->GetCaps()&(AM_SEEKING_CanSeekForwards|AM_SEEKING_CanSeekAbsolute)) ) )
			{
				pSeeking->SeekTo(newpos);

				int pos1=newpos/10000000;
				char text[32];
				sprintf(text, "Jump to time %d:%2.2d", pos1 / 60, pos1 % 60);
				OSD_ShowText(text, 0);
			}
		}
	}
}

#endif