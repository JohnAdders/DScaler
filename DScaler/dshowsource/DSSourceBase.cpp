/////////////////////////////////////////////////////////////////////////////
// $Id: DSSourceBase.cpp,v 1.5 2002-09-16 20:08:21 adcockj Exp $
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
	if(m_pDSGraph!=NULL)
	{
		delete m_pDSGraph;
		m_pDSGraph=NULL;
	}
	DeleteCriticalSection(&m_hOutThreadSync);
	WritePrivateProfileString(m_IniSection,_T("AudioDevice"),m_AudioDevice.c_str(),GetIniFileForSettings());
}

void CDSSourceBase::CreateSettings(LPCSTR IniSection)
{
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
	//Free_Picture_History(pInfo);
	
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
		m_PictureHistory[i].IsFirstInSeries=false;
		if(fields[i].flags!=BUFFER_FLAGS_FIELD_UNKNOWN)
		{
			m_PictureHistory[i].Flags=fields[i].flags;
		}
		else
		{
			m_PictureHistory[i].Flags= FieldFlag==true ? PICTURE_INTERLACED_EVEN : PICTURE_INTERLACED_ODD;
			FieldFlag=!FieldFlag;
		}
			
		pInfo->PictureHistory[i]=&m_PictureHistory[i];
		//Replace_Picture_In_History(pInfo, i, &m_PictureHistory[i]);
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
	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}

	switch(LOWORD(wParam))
	{
	case IDM_DSHOW_PLAY:
		try
		{
			m_pDSGraph->start();
			//VideoInputOnChange(m_VideoInput->GetValue(), m_VideoInput->GetValue());
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Play failed\n\n")+e.getErrorText());
		}
		return TRUE;
		break;

	case IDM_DSHOW_PAUSE:
		try
		{
			m_pDSGraph->pause();
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Pause failed\n\n")+e.getErrorText());
		}
		return TRUE;
		break;

	case IDM_DSHOW_STOP:
		try
		{
			//we must ensure that the output thread dont tries to get any
			//more fields. if it does a deadlock can occure.
			//the deadlock will only occure if the output thread is 
			//blocked when it accesses the main threads gui, 
			//since this prevents IDSRendFilter::FreeFields from being called
			CAutoCriticalSection lock(m_hOutThreadSync);
			m_pDSGraph->stop();
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Stop failed\n\n")+e.getErrorText());
		}
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

}

void CDSSourceBase::UnMute()
{

}

ISetting* CDSSourceBase::GetVolume()
{
	return NULL;
}

ISetting* CDSSourceBase::GetBalance()
{
	return NULL;
}

LPCSTR CDSSourceBase::IDString()
{
	return m_IDString.c_str();
}
#endif