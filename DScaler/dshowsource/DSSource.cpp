/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.cpp,v 1.4 2001-12-18 13:12:12 adcockj Exp $
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
// Revision 1.3  2001/12/17 19:39:38  tobbej
// implemented the picture history and field management
// crossbar support.
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
 * @file DSSource.cpp implementation of the CDSSource class.
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT
#include "dscaler.h"
#include "..\DScalerRes\resource.h"
#include "DSSource.h"
#include <dvdmedia.h>		//VIDEOINFOHEADER2

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/// @todo better error handling
CDSSource::CDSSource(string device,string deviceName) :
	CSource(0,IDC_DSHOWSOURCEMENU),
	m_pDSGraph(NULL),
	m_device(device),
	m_deviceName(deviceName),
	m_currentX(0),
	m_currentY(0),
	m_cbFieldSize(0),
	m_bProcessingFirstField(true),
	m_pictureHistoryPos(0)

{
	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		m_pictureHistory[i].IsFirstInSeries=FALSE;
		m_pictureHistory[i].pData=NULL;
		m_pictureHistory[i].Flags= i%2 ? PICTURE_INTERLACED_ODD : PICTURE_INTERLACED_EVEN;
	}
}

CDSSource::~CDSSource()
{
	if(m_pDSGraph!=NULL)
	{
		delete m_pDSGraph;
		m_pDSGraph=NULL;
	}

	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		if(m_pictureHistory[i].pData!=NULL)
		{
			delete m_pictureHistory[i].pData;
		}
	}
}

void CDSSource::CreateSettings(LPCSTR IniSection)
{

}

BOOL CDSSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	if(LOWORD(wParam)>=IDM_CROSSBAR_INPUT0 && LOWORD(wParam)<=IDM_CROSSBAR_INPUT_MAX)
	{
		try
		{
			if(m_pDSGraph!=NULL)
			{
				CDShowBaseCrossbar *pCrossbar=m_pDSGraph->getCaptureDevice()->getCrossbar();
				if(pCrossbar!=NULL)
				{
					pCrossbar->SetInputIndex(LOWORD(wParam)-IDM_CROSSBAR_INPUT0,false);
				}
			}
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Failed to change input\n\n")+e.getErrorText(),MB_OK|MB_ICONERROR);
		}

		return TRUE;
	}
	switch(LOWORD(wParam))
	{
	case IDM_DSHOW_RENDERERPROPERTIES:
		if(m_pDSGraph!=NULL)
		{
			m_pDSGraph->showRendererProperies(hWnd);
		}
		return TRUE;
		break;
	}
	
	return FALSE;
}

void CDSSource::Start()
{
	m_pictureHistoryPos=0;
	try
	{
		if(m_pDSGraph==NULL)
		{
			m_pDSGraph=new CDShowGraph(m_device,m_deviceName);
		}
		m_pDSGraph->start();
	}
	catch(CDShowException &e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONERROR);
	}
}

void CDSSource::Stop()
{
	if(m_pDSGraph!=NULL)
	{
		try
		{
			m_pDSGraph->stop();
			delete m_pDSGraph;
			m_pDSGraph=NULL;
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONERROR);
		}
	}
	//shoud probably free the memory allocated by the picture history array
}

void CDSSource::Reset()
{
	Stop();
	Start();
}

eVideoFormat CDSSource::GetFormat()
{
	return VIDEOFORMAT_PAL_B;
}

BOOL CDSSource::IsInTunerMode()
{
	return FALSE;
}

int CDSSource::GetWidth()
{
	return m_currentX;
}

int CDSSource::GetHeight()
{
	return m_currentY;
}

BOOL CDSSource::HasTuner()
{
	return FALSE;
}

BOOL CDSSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
	return FALSE;
}

BOOL CDSSource::IsVideoPresent()
{
	return FALSE;
}

int CDSSource::FindMenuID(CMenu *menu,UINT menuID)
{
	ASSERT(menu);
	ASSERT(::IsMenu(menu->GetSafeHmenu()));

	int count=menu->GetMenuItemCount();
	for(int i=0;i<count;i++)
	{
		if(menu->GetMenuItemID(i)==menuID)
			return i;
	}

	return -1;
}

void CDSSource::SetMenu(HMENU hMenu)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}

	CDShowBaseCrossbar *pCrossbar=m_pDSGraph->getCaptureDevice()->getCrossbar();
	if(pCrossbar==NULL)
	{
		return;
	}
	CMenu topMenu;
	topMenu.Attach(m_hMenu);
	CMenu *menu=topMenu.GetSubMenu(0);

	int vidPos=FindMenuID(menu,IDM_DSHOW_VIDEOINPUT);
	int audPos=FindMenuID(menu,IDM_DSHOW_AUDIOINPUT);
	CMenu vidSubMenu;
	CMenu audSubMenu;

	//check if there is already a submenu
	if(menu->GetSubMenu(vidPos)!=NULL)
	{
		vidSubMenu.Attach(menu->GetSubMenu(vidPos)->GetSafeHmenu());
		if(vidSubMenu.GetMenuItemCount()>0)
		{
			while(vidSubMenu.GetMenuItemCount()>0)
			{
				vidSubMenu.DeleteMenu(0, MF_BYPOSITION);
			}
		}
		TRACE("GetMenuItemCount() after delete %d",vidSubMenu.GetMenuItemCount());
	}
	else
	{
		//create a new submenu and insert it
		CString str;
		vidSubMenu.CreateMenu();
		menu->GetMenuString(IDM_DSHOW_VIDEOINPUT,str,MF_BYCOMMAND);

		menu->ModifyMenu(IDM_DSHOW_VIDEOINPUT,MF_POPUP|MF_BYCOMMAND,(UINT) vidSubMenu.GetSafeHmenu(),str);
	}

	//same thing for the audio submenu
	if(menu->GetSubMenu(audPos)!=NULL)
	{
		audSubMenu.Attach(menu->GetSubMenu(audPos)->GetSafeHmenu());
		if(audSubMenu.GetMenuItemCount()>0)
		{
			while(audSubMenu.GetMenuItemCount()>0)
			{
				audSubMenu.DeleteMenu(0, MF_BYPOSITION);
			}
		}
		TRACE("GetMenuItemCount() after delete %d",audSubMenu.GetMenuItemCount());
	}
	else
	{
		//create a new submenu and insert it
		CString str;
		audSubMenu.CreateMenu();
		menu->GetMenuString(IDM_DSHOW_AUDIOINPUT,str,MF_BYCOMMAND);

		menu->ModifyMenu(IDM_DSHOW_AUDIOINPUT,MF_POPUP|MF_BYCOMMAND,(UINT) audSubMenu.GetSafeHmenu(),str);
	}

	for(int i=0;i<pCrossbar->GetInputCount();i++)
	{
		ASSERT((IDM_CROSSBAR_INPUT0+i)<=IDM_CROSSBAR_INPUT_MAX);
		bool bSelected=pCrossbar->isInputSelected(i);

		//is it an audio or video input?
		if(pCrossbar->GetInputType(i)<4096)
		{
			vidSubMenu.AppendMenu(MF_STRING,IDM_CROSSBAR_INPUT0+i,pCrossbar->GetInputName(i));
			vidSubMenu.CheckMenuItem(IDM_CROSSBAR_INPUT0+i,bSelected ? MF_CHECKED:MF_UNCHECKED);
		}
		else
		{
			audSubMenu.AppendMenu(MF_STRING,IDM_CROSSBAR_INPUT0+i,pCrossbar->GetInputName(i));
			vidSubMenu.CheckMenuItem(IDM_CROSSBAR_INPUT0+i,bSelected ? MF_CHECKED:MF_UNCHECKED);
		}
	}
	topMenu.Detach();
	//vidSubMenu.Detach();
	//audSubMenu.Detach();

}

void CDSSource::HandleTimerMessages(int TimerId)
{

}

LPCSTR CDSSource::GetMenuLabel()
{
	return NULL;
}

LPCSTR CDSSource::GetStatus()
{
	return "This source dont work yet";
}

void CDSSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	
	//which filed in the history is the first one to be returned
	int historyStart=0;

	//get a new frame or not
	if(m_bProcessingFirstField)
	{
		//get the current media type
		AM_MEDIA_TYPE mediaType;
		m_pDSGraph->getCurrentMediatype(&mediaType);
		BITMAPINFOHEADER *bmi=NULL;
		if(mediaType.formattype==FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER *videoInfo=(VIDEOINFOHEADER*)mediaType.pbFormat;
			bmi=&(videoInfo->bmiHeader);
		}
		else if(mediaType.formattype==FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2 *videoInfo2=(VIDEOINFOHEADER2*)mediaType.pbFormat;
			bmi=&(videoInfo2->bmiHeader);
		}

		//is the buffers large enough?
		LONG fieldSize=bmi->biSizeImage/2;
		if(m_cbFieldSize<fieldSize)
		{
			//no, fix it
			for(int i=0;i<MAX_PICTURE_HISTORY;i++)
			{
				if(m_pictureHistory[i].pData!=NULL)
				{
					delete m_pictureHistory[i].pData;
					m_pictureHistory[i].pData=NULL;
				}
				m_pictureHistory[i].pData=new BYTE[fieldSize];
			}
			m_cbFieldSize=fieldSize;
			m_pictureHistory[0].IsFirstInSeries=TRUE;
		}
		else
		{
			m_pictureHistory[0].IsFirstInSeries=FALSE;
		}
		
		//get the fields
		if(!m_pDSGraph->getNextSample(m_pictureHistory[m_pictureHistoryPos].pData,m_pictureHistory[m_pictureHistoryPos+1].pData,fieldSize))
		{
			pInfo->bRunningLate=TRUE;
			pInfo->bMissedFrame=TRUE;
			pInfo->FrameWidth=10;
			pInfo->FrameHeight=10;
			return;
		}

		m_pictureHistory[m_pictureHistoryPos].Flags=PICTURE_INTERLACED_EVEN;
		m_pictureHistory[m_pictureHistoryPos+1].Flags=PICTURE_INTERLACED_ODD;

		m_currentX=bmi->biWidth;
		m_currentY=bmi->biHeight;
		m_bytePerPixel=bmi->biBitCount/8;
		pInfo->FrameWidth=bmi->biWidth;
		pInfo->FrameHeight=bmi->biHeight;
		pInfo->LineLength=bmi->biWidth * m_bytePerPixel;
		pInfo->FieldHeight=bmi->biHeight / 2;
		pInfo->InputPitch=pInfo->LineLength;
		pInfo->bMissedFrame=FALSE;
		
		historyStart=m_pictureHistoryPos;
		
		//update m_pictureHistoryPos
		m_pictureHistoryPos+=2;
		if(m_pictureHistoryPos>=MAX_PICTURE_HISTORY)
		{
			m_pictureHistoryPos=0;
		}

		//free format block if any
		if(mediaType.cbFormat!=0)
		{
			CoTaskMemFree((PVOID)mediaType.pbFormat);
		}
		
		m_bProcessingFirstField=false;
	}
	else
	{
		pInfo->FrameHeight=m_currentY;
		pInfo->FrameWidth=m_currentX;
		pInfo->LineLength=m_currentX * m_bytePerPixel;
		pInfo->FieldHeight=pInfo->FrameHeight/2;
		pInfo->InputPitch=pInfo->LineLength;
		pInfo->bMissedFrame=FALSE;

		historyStart=m_pictureHistoryPos-1;
		m_bProcessingFirstField=true;
	}
	
	int pos=historyStart;
	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		pos--;
		if(pos<0)
		{
			pos=MAX_PICTURE_HISTORY-1;
		}

		ASSERT(pos>=0 && pos<MAX_PICTURE_HISTORY);
		pInfo->PictureHistory[i]=&m_pictureHistory[pos];
	}
}

#endif