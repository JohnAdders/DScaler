/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.cpp,v 1.16 2002-03-17 21:55:10 tobbej Exp $
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
// Revision 1.15  2002/03/17 21:43:23  tobbej
// added input resolution submenu
//
// Revision 1.14  2002/03/15 23:03:51  tobbej
// reset m_bProcessingFirstFiled when starting
//
// Revision 1.13  2002/02/22 09:07:14  tobbej
// fixed small race condition when calling notifysizechange, workoutoverlaysize might have used the old size
//
// Revision 1.12  2002/02/19 16:03:37  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.11  2002/02/13 17:02:08  tobbej
// new filter properties menu
// fixed some problems in GetNextFiled
//
// Revision 1.10  2002/02/09 02:49:23  laurentg
// Overscan now stored in a setting of the source
//
// Revision 1.9  2002/02/07 22:08:23  tobbej
// changed for new file input
//
// Revision 1.8  2002/02/05 17:52:27  tobbej
// changed alignment calc
//
// Revision 1.7  2002/02/05 17:27:17  tobbej
// fixed alignment problems
// update dropped/drawn fields stats
//
// Revision 1.6  2002/02/03 20:05:58  tobbej
// made video format menu work
// fixed color controls
// enable/disable menu items
//
// Revision 1.5  2002/02/03 11:00:43  tobbej
// added support for picure controls
// fixed menu handling
// fixed GetNextField to work with dshow filter
//
// Revision 1.4  2001/12/18 13:12:12  adcockj
// Interim check-in for redesign of card specific settings
//
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
#include "FieldTiming.h"
#include "..\DScalerRes\resource.h"
#include "DSSource.h"
#include "DShowFileSource.h"
#include "CaptureDevice.h"
#include <dvdmedia.h>		//VIDEOINFOHEADER2
#include "AspectRatio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct videoStandardsType
{
	long format;
	LPSTR name;
};

///array of video standards
videoStandardsType videoStandards[] =
	{
		AnalogVideo_NTSC_M,"NTSC_M",
		AnalogVideo_NTSC_M_J,"NTSC_M",
		AnalogVideo_NTSC_433,"NTSC_433",
		AnalogVideo_PAL_B,"PAL_B",
		AnalogVideo_PAL_D,"PAL_D",
		AnalogVideo_PAL_G,"PAL_G",
		AnalogVideo_PAL_H,"PAL_H",
		AnalogVideo_PAL_I,"PAL_I",
		AnalogVideo_PAL_M,"PAL_M",
		AnalogVideo_PAL_N,"PAL_N",
		AnalogVideo_PAL_60,"PAL_60",
		AnalogVideo_SECAM_B,"SECAM_B",
		AnalogVideo_SECAM_D,"SECAM_D",
		AnalogVideo_SECAM_G,"SECAM_G",
		AnalogVideo_SECAM_H,"SECAM_H",
		AnalogVideo_SECAM_K,"SECAM_K",
		AnalogVideo_SECAM_K1,"SECAM_K1",
		AnalogVideo_SECAM_L,"SECAM_L",
		AnalogVideo_SECAM_L1,"SECAM_L1",
		AnalogVideo_PAL_N_COMBO,"PAL_N_COMBO",
		0,NULL
	};

struct resolutionType
{
	long x;
	long y;
};
//maybe this shoud be user configurable,
//but that is a bit hard when the settings classes dont allow string settings
resolutionType res[]=
{
	768,576,
	720,480,
	640,480,
	320,240,
	160,120,
	0,0
};

CDSSource::CDSSource(string device,string deviceName) :
	CSource(0,IDC_DSHOWSOURCEMENU),
	m_pDSGraph(NULL),
	m_device(device),
	m_deviceName(deviceName),
	m_currentX(0),
	m_currentY(0),
	m_cbFieldSize(0),
	m_bProcessingFirstField(true),
	m_pictureHistoryPos(0),
	m_lastNumDroppedFrames(-1),
	m_bIsFileSource(false)

{
	CreateSettings(device.c_str());

	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		m_pictureHistory[i].IsFirstInSeries=FALSE;
		m_pictureHistory[i].pData=NULL;
		m_pictureHistory[i].Flags= i%2 ? PICTURE_INTERLACED_ODD : PICTURE_INTERLACED_EVEN;
		m_unalignedBuffers[i]=NULL;
	}
}

CDSSource::CDSSource() :
	CSource(0,IDC_DSHOWSOURCEMENU),
	m_pDSGraph(NULL),
	m_currentX(0),
	m_currentY(0),
	m_cbFieldSize(0),
	m_bProcessingFirstField(true),
	m_pictureHistoryPos(0),
	m_lastNumDroppedFrames(-1),
	m_bIsFileSource(true)

{
	CreateSettings("DShowFileInput");
	for(int i=0;i<MAX_PICTURE_HISTORY;i++)
	{
		m_pictureHistory[i].IsFirstInSeries=FALSE;
		m_pictureHistory[i].pData=NULL;
		m_pictureHistory[i].Flags= i%2 ? PICTURE_INTERLACED_ODD : PICTURE_INTERLACED_EVEN;
		m_unalignedBuffers[i]=NULL;
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
		if(m_unalignedBuffers[i]!=NULL)
		{
			delete m_unalignedBuffers[i];
			m_unalignedBuffers[i]=NULL;
		}
		m_pictureHistory[i].pData=NULL;
	}
}

BOOL CDSSource::IsAccessAllowed()
{
	if(!m_bIsFileSource)
	{
		return TRUE;
	}

	/*
	if(m_pDSGraph!=NULL)
	{
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_FILE)
		{
			CDShowBaseSource *pFile=(CDShowBaseSource*)m_pDSGraph->getSourceDevice();
			if(pFile->isConnected())
				return TRUE;
		}
	}
	*/
	if(m_filename.size()>0)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CDSSource::OpenMediaFile(LPCSTR FileName, BOOL NewPlayList)
{
	if(!m_bIsFileSource)
		return FALSE;
	
	if(m_pDSGraph!=NULL)
		Stop();
	
	m_filename="";
	try
	{
		m_pDSGraph=new CDShowGraph(FileName);
		m_filename=FileName;
		m_pDSGraph->start();
		return TRUE;
	}
	catch(CDShowException e)
	{
		return FALSE;
	}
}

ISetting* CDSSource::GetBrightness()
{
	if(m_pDSGraph==NULL)
		return NULL;
	
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;
			long value;

			try
			{
				pCap->getRange(VideoProcAmp_Brightness,&min,&max,NULL,&def);
				m_Brightness->SetMax(max);
				m_Brightness->SetMin(min);

				//why does ChangeDefault() change the value itself too?
				value=m_Brightness->GetValue();
				m_Brightness->ChangeDefault(def);
				m_Brightness->SetValue(value);
				return m_Brightness;
			}
			catch(CDShowException e)
			{}
		}
	}
	return NULL;
}

void CDSSource::BrightnessOnChange(long Brightness, long OldValue)
{
	if(Brightness==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	
	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			pCap->set(VideoProcAmp_Brightness,Brightness,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

ISetting* CDSSource::GetContrast()
{
	if(m_pDSGraph==NULL)
		return NULL;

	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;
			long value;

			try
			{
				pCap->getRange(VideoProcAmp_Contrast,&min,&max,NULL,&def);
				m_Contrast->SetMax(max);
				m_Contrast->SetMin(min);
				value=m_Contrast->GetValue();
				m_Contrast->ChangeDefault(def);
				m_Contrast->SetValue(value);
				return m_Contrast;
			}
			catch(CDShowException e)
			{}
		}
	}
	return NULL;
}

void CDSSource::ContrastOnChange(long Contrast, long OldValue)
{
	if(Contrast==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			pCap->set(VideoProcAmp_Contrast,Contrast,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

ISetting* CDSSource::GetHue()
{
	if(m_pDSGraph==NULL)
		return NULL;

	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;
			long value;

			try
			{
				pCap->getRange(VideoProcAmp_Hue,&min,&max,NULL,&def);
				m_Hue->SetMax(max);
				m_Hue->SetMin(min);
				value=m_Hue->GetValue();
				m_Hue->ChangeDefault(def);
				m_Hue->SetValue(value);
				return m_Hue;
			}
			catch(CDShowException e)
			{}
		}
	}
	return NULL;
}

void CDSSource::HueOnChange(long Hue, long OldValue)
{
	if(Hue==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			pCap->set(VideoProcAmp_Hue,Hue,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

ISetting* CDSSource::GetSaturation()
{
	if(m_pDSGraph==NULL)
		return NULL;

	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
		if(pCap->hasVideoProcAmp())
		{
			long min;
			long max;
			long def;
			long value;

			try
			{
				pCap->getRange(VideoProcAmp_Saturation,&min,&max,NULL,&def);
				m_Saturation->SetMax(max);
				m_Saturation->SetMin(min);
				value=m_Saturation->GetValue();
				m_Saturation->ChangeDefault(def);
				m_Saturation->SetValue(value);
				return m_Saturation;
			}
			catch(CDShowException e)
			{}
		}
	}
	return NULL;
}

void CDSSource::SaturationOnChange(long Saturation, long OldValue)
{
	if(Saturation==OldValue)
		return;
	if(m_pDSGraph==NULL)
		return;
	try
	{
		CDShowCaptureDevice *pCap=NULL;
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
			pCap->set(VideoProcAmp_Saturation,Saturation,VideoProcAmp_Flags_Manual);
		}
	}
	catch(CDShowException e)
	{
		AfxMessageBox(e.getErrorText(),MB_OK|MB_ICONEXCLAMATION);
	}
}

void CDSSource::CreateSettings(LPCSTR IniSection)
{
	//at this time we dont know what the min and max will be
	m_Brightness = new CBrightnessSetting(this, "Brightness", 0, LONG_MIN, LONG_MAX, IniSection);
	m_Settings.push_back(m_Brightness);

	m_Contrast = new CContrastSetting(this, "Contrast", 0, LONG_MIN, LONG_MAX, IniSection);
	m_Settings.push_back(m_Contrast);

	m_Hue = new CHueSetting(this, "Hue", 0, LONG_MIN, LONG_MAX, IniSection);
	m_Settings.push_back(m_Hue);

	m_Saturation = new CSaturationSetting(this, "Saturation", 0, LONG_MIN, LONG_MAX, IniSection);
	m_Settings.push_back(m_Saturation);

}

BOOL CDSSource::HandleWindowsCommands(HWND hWnd, UINT wParam, LONG lParam)
{
	if(m_pDSGraph==NULL)
	{
		return FALSE;
	}
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}

	if(LOWORD(wParam)>=IDM_CROSSBAR_INPUT0 && LOWORD(wParam)<=IDM_CROSSBAR_INPUT_MAX)
	{
		try
		{
			if(pCap!=NULL)
			{
				CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
				if(pCrossbar!=NULL)
				{
					//if changing a video input, set the related pin too
					bool isVideo=pCrossbar->GetInputType(LOWORD(wParam)-IDM_CROSSBAR_INPUT0)<4096;
					pCrossbar->SetInputIndex(LOWORD(wParam)-IDM_CROSSBAR_INPUT0,isVideo);
				}
			}
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Failed to change input\n\n")+e.getErrorText(),MB_OK|MB_ICONERROR);
		}

		return TRUE;
	}
	else if(LOWORD(wParam)>=IDM_DSVIDEO_STANDARD_0 && LOWORD(wParam)<=IDM_DSVIDEO_STANDARD_MAX)
	{
		try
		{
			if(pCap!=NULL)
			{
				pCap->putTVFormat(videoStandards[LOWORD(wParam)-IDM_DSVIDEO_STANDARD_0].format);
			}
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Failed to change video format\n\n")+e.getErrorText(),MB_OK|MB_ICONERROR);
		}
		return TRUE;
	}
	else if(LOWORD(wParam)>=IDM_DSHOW_FILTER_0 && LOWORD(wParam<=IDM_DSHOW_FILTER_MAX))
	{
		try
		{
			m_pDSGraph->showPropertyPage(hWnd,LOWORD(wParam)-IDM_DSHOW_FILTER_0);
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Failed to show property page\n\n")+e.getErrorText(),MB_OK|MB_ICONERROR);
		}

	}
	else if(LOWORD(wParam)>=IDM_DSHOW_RES_0 && LOWORD(wParam<=IDM_DSHOW_RES_MAX))
	{
		try
		{
			m_pDSGraph->changeRes(res[LOWORD(wParam)-IDM_DSHOW_RES_0].x,res[LOWORD(wParam)-IDM_DSHOW_RES_0].y);
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Failed to change resolution\n\n")+e.getErrorText(),MB_OK|MB_ICONERROR);
		}
	}
	
	switch(LOWORD(wParam))
	{
	case IDM_DSHOW_PLAY:
		try
		{
			m_pDSGraph->start();
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Play failed\n\n")+e.getErrorText(),MB_OK|MB_ICONINFORMATION);
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
			AfxMessageBox(CString("Pause failed\n\n")+e.getErrorText(),MB_OK|MB_ICONINFORMATION);
		}
		return TRUE;
		break;

	case IDM_DSHOW_STOP:
		try
		{
			m_pDSGraph->stop();
		}
		catch(CDShowException &e)
		{
			AfxMessageBox(CString("Stop failed\n\n")+e.getErrorText(),MB_OK|MB_ICONINFORMATION);
		}
		return TRUE;
		break;
	}
	return FALSE;
}

void CDSSource::Start()
{
	m_pictureHistoryPos=0;
	m_lastNumDroppedFrames=-1;
	m_currentX=0;
	m_currentY=0;
	m_bProcessingFirstField=true;
	try
	{
		CWaitCursor wait;
		if(m_pDSGraph==NULL)
		{
			if(m_bIsFileSource)
			{
				m_pDSGraph=new CDShowGraph(m_filename);
			}
			else
			{
				m_pDSGraph=new CDShowGraph(m_device,m_deviceName);
			}
		}

		m_pDSGraph->start();
		
		//get the stored settings
		ReadFromIni();
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
			WriteToIni(TRUE);
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

void CDSSource::SetMenu(HMENU hMenu)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	CMenu topMenu;
	topMenu.Attach(m_hMenu);
	CMenu *menu=topMenu.GetSubMenu(0);
	
	CDShowCaptureDevice *pCap=NULL;
	CDShowBaseCrossbar *pCrossbar=NULL;

	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
		pCrossbar=pCap->getCrossbar();
	}

	//setup input selection menus
	CString menuText;
	if(pCrossbar!=NULL)
	{
		CMenu vidSubMenu;
		CMenu audSubMenu;
		
		//create video input submenu and insert it
		vidSubMenu.CreateMenu();
		menu->GetMenuString(0,menuText,MF_BYPOSITION);
		menu->ModifyMenu(0,MF_POPUP|MF_BYPOSITION,(UINT) vidSubMenu.GetSafeHmenu(),menuText);
		menu->EnableMenuItem(0,MF_BYPOSITION|MF_ENABLED);

		//same for audio input menu
		audSubMenu.CreateMenu();
		menu->GetMenuString(3,menuText,MF_BYPOSITION);
		menu->ModifyMenu(3,MF_POPUP|MF_BYPOSITION,(UINT) audSubMenu.GetSafeHmenu(),menuText);
		menu->EnableMenuItem(3,MF_BYPOSITION|MF_ENABLED);

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
				audSubMenu.CheckMenuItem(IDM_CROSSBAR_INPUT0+i,bSelected ? MF_CHECKED:MF_UNCHECKED);
			}
		}
	}
	else
	{
		menu->EnableMenuItem(0,MF_BYPOSITION|MF_GRAYED);
		menu->EnableMenuItem(3,MF_BYPOSITION|MF_GRAYED);
	}

	//video format menu
	if(pCap!=NULL && pCap->hasVideoDec())
	{
		CMenu formatMenu;
		
		long formats=0;
		long selectedFormat=0;
		try
		{
			formats=pCap->getSupportedTVFormats();
			selectedFormat=pCap->getTVFormat();
		}
		catch(CDShowException e)
		{}
		
		//make sure there is atleast one format to be selected
		if(formats!=0)
		{
			formatMenu.CreateMenu();
			menu->GetMenuString(1,menuText,MF_BYPOSITION);
			menu->ModifyMenu(1,MF_POPUP|MF_BYPOSITION,(UINT)formatMenu.GetSafeHmenu(),menuText);
			menu->EnableMenuItem(1,MF_BYPOSITION|MF_ENABLED);
			
			int i=0;
			while(videoStandards[i].format!=0)
			{
				ASSERT(i <= IDM_DSVIDEO_STANDARD_MAX-IDM_DSVIDEO_STANDARD_0);

				if(formats & videoStandards[i].format)
				{
					UINT check=selectedFormat & videoStandards[i].format ? MF_CHECKED:MF_UNCHECKED;
					formatMenu.AppendMenu(MF_STRING,IDM_DSVIDEO_STANDARD_0+i,videoStandards[i].name);
					formatMenu.CheckMenuItem(IDM_DSVIDEO_STANDARD_0+i,check);
				}
				i++;
			}
		}
		else
		{
			menu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);
		}
	}
	else
	{
		menu->EnableMenuItem(1,MF_BYPOSITION|MF_GRAYED);
	}

	//resolution submenu
	int i=0;
	bool resAdded=false;
	CMenu resSubMenu;
	resSubMenu.CreateMenu();
	while(res[i].x!=0 || res[i].y!=0)
	{
		if(m_pDSGraph->isValidRes(res[i].x,res[i].y))
		{
			CString str;
			str.Format("%ldx%ld",res[i].x,res[i].y);
			ASSERT(IDM_DSHOW_RES_0+i<=IDM_DSHOW_RES_MAX);
			resSubMenu.AppendMenu(MF_STRING,IDM_DSHOW_RES_0+i,str);
			resAdded=true;
		}
		i++;
	}
	if(resAdded)
	{
		menu->GetMenuString(2,menuText,MF_BYPOSITION);
		menu->ModifyMenu(2,MF_POPUP|MF_BYPOSITION,(UINT) resSubMenu.GetSafeHmenu(),menuText);
		menu->EnableMenuItem(2,MF_BYPOSITION|MF_ENABLED);
	}
	else
	{
		//no resolution was added, gray the menu item
		menu->EnableMenuItem(2,MF_BYPOSITION|MF_GRAYED);
		resSubMenu.DestroyMenu();
	}
	
	//filter properties submenu
	CMenu filtersMenu;
	filtersMenu.CreateMenu();
	menu->GetMenuString(11,menuText,MF_BYPOSITION);
	menu->ModifyMenu(11,MF_POPUP|MF_BYPOSITION,(UINT) filtersMenu.GetSafeHmenu(),menuText);
	string name;
	int index=0;
	bool hasPropertyPage;
	while(m_pDSGraph->getFilterName(index,name,hasPropertyPage))
	{
		ASSERT(IDM_DSHOW_FILTER_0+index<=IDM_DSHOW_FILTER_MAX);
		filtersMenu.AppendMenu(MF_STRING| (hasPropertyPage ? MF_ENABLED : MF_GRAYED),IDM_DSHOW_FILTER_0+index,name.c_str());
		index++;
	}
	menu->EnableMenuItem(11,MF_BYPOSITION|(index==0 ? MF_GRAYED : MF_ENABLED));

	//set a radio checkmark infront of the current play/pause/stop menu entry
	FILTER_STATE state=m_pDSGraph->getState();
	UINT pos=8-state;
	menu->CheckMenuRadioItem(5,7,pos,MF_BYPOSITION);

	topMenu.Detach();
}

void CDSSource::HandleTimerMessages(int TimerId)
{

}

LPCSTR CDSSource::GetMenuLabel()
{
	if(m_pDSGraph!=NULL)
	{
		if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_FILE)
		{
			CDShowFileSource *pFile=(CDShowFileSource*)m_pDSGraph->getSourceDevice();
			return pFile->getFileName().c_str();
		}
	}
	return NULL;
}

void CDSSource::SetOverscan()
{
    AspectSettings.InitialOverscan = 0;
}

LPCSTR CDSSource::GetStatus()
{
	if(m_bIsFileSource)
	{
		if(m_pDSGraph!=NULL)
		{
			if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_FILE)
			{
				CDShowFileSource *pFile=(CDShowFileSource*)m_pDSGraph->getSourceDevice();
				return pFile->getFileName().c_str();
			}
		}
		return "Unknown file";
	}
	else
	{
		return m_deviceName.c_str();
	}
}

void CDSSource::GetNextField(TDeinterlaceInfo* pInfo, BOOL AccurateTiming)
{
	if(m_pDSGraph==NULL)
	{
		return;
	}
	
	//which field in the history is the first one to be returned
	int historyStart=0;

	//get a new frame or not
	if(m_bProcessingFirstField)
	{
		//info to return if we fail
		pInfo->bRunningLate=TRUE;
		pInfo->bMissedFrame=TRUE;
		pInfo->FrameWidth=0;
		pInfo->FrameHeight=0;

		//clear the picture history
		memset(pInfo->PictureHistory, 0, MAX_PICTURE_HISTORY * sizeof(TPicture*));
		
		//is the graph running? there is no point in continuing if it is stopped
		if(m_pDSGraph->getState()==State_Stopped)
		{
			return;
		}

		//get the current media type
		AM_MEDIA_TYPE mediaType;
		memset(&mediaType,0,sizeof(mediaType));
		try
		{
			m_pDSGraph->getConnectionMediatype(&mediaType);
		}
		catch(CDShowException e)
		{
			return;
		}
		
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
		
		CComPtr<IMediaSample> pSample;
		if(!m_pDSGraph->getNextSample(pSample))
		{
			return;
		}

		LONG size=pSample->GetActualDataLength();
		BYTE *pData;
		HRESULT hr=pSample->GetPointer(&pData);
		if(FAILED(hr))
		{
			//oops, coudn't get any pointer
			return;
		}

		//is the buffers large enough?
		LONG fieldSize=bmi->biSizeImage/2;
		if(m_cbFieldSize<fieldSize)
		{
			//no, fix it
			for(int i=0;i<MAX_PICTURE_HISTORY;i++)
			{
				if(m_unalignedBuffers[i]!=NULL)
				{
					delete m_unalignedBuffers[i];
					m_unalignedBuffers[i]=NULL;
				}
				m_unalignedBuffers[i]=new BYTE[fieldSize+16];

				m_pictureHistory[i].pData=m_unalignedBuffers[i]+(0x10-LOBYTE(m_unalignedBuffers[i])&0xf);
			}
			m_cbFieldSize=fieldSize;
			m_pictureHistory[0].IsFirstInSeries=TRUE;
		}
		else
		{
			m_pictureHistory[0].IsFirstInSeries=FALSE;
		}
		//ASSERT(m_cbFieldSize>=size/2);
		
		
		//split the media sample into even and odd field
		//what if the input picture upside down?
		long lineSize=bmi->biWidth*bmi->biBitCount/8;
		for(int i=0;i<(bmi->biHeight/2);i++)
		{
			pInfo->pMemcpy(m_pictureHistory[m_pictureHistoryPos].pData+i*lineSize,pData+(2*i)*lineSize,lineSize);
			pInfo->pMemcpy(m_pictureHistory[m_pictureHistoryPos+1].pData+i*lineSize,pData+(2*i+1)*lineSize,lineSize);
		}

		m_pictureHistory[m_pictureHistoryPos].Flags=PICTURE_INTERLACED_EVEN;
		m_pictureHistory[m_pictureHistoryPos+1].Flags=PICTURE_INTERLACED_ODD;
		
		//check if size has changed
		if(m_currentX!=bmi->biWidth || m_currentY!=bmi->biHeight)
		{
			m_currentX=bmi->biWidth;
			m_currentY=bmi->biHeight;
			NotifySizeChange();
		}
		
		m_bytePerPixel=bmi->biBitCount/8;
		pInfo->FrameWidth=bmi->biWidth;
		pInfo->FrameHeight=bmi->biHeight;
		pInfo->LineLength=bmi->biWidth * m_bytePerPixel;
		pInfo->FieldHeight=bmi->biHeight / 2;
		pInfo->InputPitch=pInfo->LineLength;
		pInfo->bMissedFrame=FALSE;
		pInfo->bRunningLate=FALSE;
		
		historyStart=m_pictureHistoryPos;
		
		//update m_pictureHistoryPos
		m_pictureHistoryPos+=2;
		if(m_pictureHistoryPos>=MAX_PICTURE_HISTORY)
		{
			m_pictureHistoryPos=0;
		}
		
		updateDroppedFields();

		//free format block if any
		if(mediaType.cbFormat!=0)
		{
			CoTaskMemFree((PVOID)mediaType.pbFormat);
			mediaType.pbFormat=NULL;
			mediaType.cbFormat=0;
		}
		if(mediaType.pUnk!=NULL)
		{
			mediaType.pUnk->Release();
			mediaType.pUnk=NULL;
		}
		
		m_bProcessingFirstField=false;
	}
	else
	{
		//FIXME: need to wait one field here
		//Sleep(20);

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
	Timing_IncrementUsedFields();
}

void CDSSource::updateDroppedFields()
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
		return;
	}
	
	//is the m_lastNumDroppedFrames count valid?
	if(m_lastNumDroppedFrames!=-1)
	{
		if(dropped-m_lastNumDroppedFrames >0)
		{
			Timing_AddDroppedFields((dropped-m_lastNumDroppedFrames)*2);
		}
	}
	m_lastNumDroppedFrames=dropped;
}

#endif

