/////////////////////////////////////////////////////////////////////////////
// $Id: DSSource.cpp,v 1.30 2002-08-15 14:20:12 kooiman Exp $
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
// Revision 1.29  2002/08/14 22:03:23  kooiman
// Added TV tuner support for DirectShow capture devices
//
// Revision 1.28  2002/08/13 21:04:43  kooiman
// Add IDString() to Sources for identification purposes.
//
// Revision 1.27  2002/08/01 20:23:43  tobbej
// improved error messages when opening files.
// implemented AvgSyncOffset counter in dsrend
//
// Revision 1.26  2002/07/07 20:17:53  tobbej
// fixed deadlock when stoping input source
//
// Revision 1.25  2002/07/06 16:48:11  tobbej
// new field buffering
//
// Revision 1.24  2002/06/22 15:03:16  laurentg
// New vertical flip mode
//
// Revision 1.23  2002/05/24 15:18:32  tobbej
// changed filter properties dialog to include progpertypages from the pins
// fixed input source status
// fixed dot infront of start/pause/stop menu entries
// changed overscan settings a bit
// experimented a bit with measuring time for dscaler to process one field
//
// Revision 1.22  2002/05/02 19:50:39  tobbej
// changed dshow source filter submenu to use new tree based dialog
//
// Revision 1.21  2002/04/16 15:33:53  tobbej
// added overscan for capture devices
// added audio mute/unmute when starting and stopping source
// added waitForNextField
//
// Revision 1.20  2002/04/15 22:57:26  laurentg
// Automatic switch to "square pixels" AR mode when needed
//
// Revision 1.19  2002/04/07 14:52:13  tobbej
// fixed race when changing resolution
// improved error handling
//
// Revision 1.18  2002/04/03 19:52:30  tobbej
// added some more logging to help track the filters submenu problem
//
// Revision 1.17  2002/03/26 19:48:59  adcockj
// Improved error handling in DShow code
//
// Revision 1.16  2002/03/17 21:55:10  tobbej
// fixed resolution submenu so its properly disabled when using file input or there is no available resolutions
//
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
#include "DebugLog.h"
#include "AutoCriticalSection.h"
#include "Audio.h"
#include "TreeSettingsDlg.h"
#include "TreeSettingsOleProperties.h"
#include "SettingsPerChannel.h"
#include "ProgramList.h"

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

// For conversion from eVideoformat to DShow AnalogVideoStandard
const AnalogVideoStandard eVideoFormatTable[] =
{
    AnalogVideo_PAL_B,
		AnalogVideo_PAL_D,
		AnalogVideo_PAL_G,
		AnalogVideo_PAL_H,
		AnalogVideo_PAL_I,
		AnalogVideo_PAL_M,
		AnalogVideo_PAL_N,
		AnalogVideo_PAL_60,
    AnalogVideo_PAL_N_COMBO,

    AnalogVideo_SECAM_B,
		AnalogVideo_SECAM_D,
		AnalogVideo_SECAM_G,
		AnalogVideo_SECAM_H,
		AnalogVideo_SECAM_K,
		AnalogVideo_SECAM_K1,
		AnalogVideo_SECAM_L,
		AnalogVideo_SECAM_L1,

    AnalogVideo_NTSC_M,
		AnalogVideo_NTSC_M_J,
		AnalogVideo_NTSC_433,     // == VIDEOFORMAT_NTSC_50 ?
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
	88,72,
	0,0
};


static void DS_OnSetup(void *pThis, int Start)
{
   if (pThis != NULL)
   {
      ((CDSSource*)pThis)->SettingsPerChannelSetup(Start);
   }
}


CDSSource::CDSSource(string device,string deviceName) :
	CSource(0,IDC_DSHOWSOURCEMENU),
	m_pDSGraph(NULL),
	m_device(device),
	m_deviceName(deviceName),
	m_currentX(0),
	m_currentY(0),
	m_lastNumDroppedFrames(-1),
	m_bIsFileSource(false),
	m_dwRendStartTime(0)

{
	m_IDString = std::string("DS_") + deviceName;
  InitializeCriticalSection(&m_hOutThreadSync);
	CreateSettings(device.c_str());  

  SettingsPerChannel_RegisterOnSetup(this, DS_OnSetup);
}

CDSSource::CDSSource() :
	CSource(0,IDC_DSHOWSOURCEMENU),
	m_pDSGraph(NULL),
	m_currentX(0),
	m_currentY(0),
	m_lastNumDroppedFrames(-1),
	m_bIsFileSource(true),
	m_dwRendStartTime(0)

{
	m_IDString = std::string("DS_DShowFileInput");
  InitializeCriticalSection(&m_hOutThreadSync);
	CreateSettings("DShowFileInput");
}

CDSSource::~CDSSource()
{
	if(m_pDSGraph!=NULL)
	{
		delete m_pDSGraph;
		m_pDSGraph=NULL;
	}
	DeleteCriticalSection(&m_hOutThreadSync);
}

BOOL CDSSource::IsAccessAllowed()
{
	if(!m_bIsFileSource)
	{
		return TRUE;
	}

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

ISetting* CDSSource::GetBrightness()
{
	if(m_pDSGraph==NULL)
		return NULL;
	
	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
		return NULL;

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
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
			{
                LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
            }
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
		ErrorBox(e.getErrorText());
	}
}

ISetting* CDSSource::GetContrast()
{
	if(m_pDSGraph==NULL)
		return NULL;

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
		return NULL;

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
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
			{
                LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
            }
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
		ErrorBox(e.getErrorText());
	}
}

ISetting* CDSSource::GetHue()
{
	if(m_pDSGraph==NULL)
		return NULL;

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
		return NULL;

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
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
			{
                LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
            }
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
		ErrorBox(e.getErrorText());
	}
}

ISetting* CDSSource::GetSaturation()
{
	if(m_pDSGraph==NULL)
		return NULL;

	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
		return NULL;

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
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
			{
                LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
            }
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
		ErrorBox(e.getErrorText());
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

	m_Overscan = new COverscanSetting(this, "Overscan", 0, 0, 150, IniSection);
	m_Settings.push_back(m_Overscan);

  m_VideoInput = new CVideoInputSetting(this, "VideoInput", 0, 0, LONG_MAX, IniSection);
	m_Settings.push_back(m_VideoInput);

  m_LastTunerChannel = new CLastTunerChannelSetting(this, "LastTunerChannel", 0, 0, LONG_MAX, IniSection);
	m_Settings.push_back(m_LastTunerChannel);
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
		/*try
		{
			if(pCap!=NULL)
			{
				CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
				if(pCrossbar!=NULL)
				{
					//if changing a video input, set the related pin too
					bool isVideo=pCrossbar->GetInputType(LOWORD(wParam)-IDM_CROSSBAR_INPUT0)<4096;
          
          long CurrentInputIndex = pCrossbar->GetInputIndex();
          long CurrentInputType = pCrossbar->GetInputType(CurrentInputIndex);
          SettingsPerChannel_VideoInputChange(this, 1, CurrentInputIndex, CurrentInputType == PhysConn_Video_Tuner);          
          
					pCrossbar->SetInputIndex(LOWORD(wParam)-IDM_CROSSBAR_INPUT0,isVideo);          
          
          SettingsPerChannel_VideoInputChange(this, 0, LOWORD(wParam)-IDM_CROSSBAR_INPUT0, isVideo == PhysConn_Video_Tuner);
				}
			}
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Failed to change input\n\n")+e.getErrorText());
		} */
    m_VideoInput->SetValue(LOWORD(wParam)-IDM_CROSSBAR_INPUT0);

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
			ErrorBox(CString("Failed to change video format\n\n")+e.getErrorText());
		}
		return TRUE;
	}
	else if(LOWORD(wParam)>=IDM_DSHOW_RES_0 && LOWORD(wParam<=IDM_DSHOW_RES_MAX))
	{
		try
		{
			CAutoCriticalSection lock(m_hOutThreadSync);
			m_pDSGraph->changeRes(res[LOWORD(wParam)-IDM_DSHOW_RES_0].x,res[LOWORD(wParam)-IDM_DSHOW_RES_0].y);
		}
		catch(CDShowException &e)
		{
			ErrorBox(CString("Failed to change resolution\n\n")+e.getErrorText());
		}
	}
	
	switch(LOWORD(wParam))
	{
	case IDM_DSHOW_PLAY:
		try
		{
			m_pDSGraph->start();
      VideoInputOnChange(m_VideoInput->GetValue(), m_VideoInput->GetValue());
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

void CDSSource::Start()
{
	m_lastNumDroppedFrames=-1;
	m_currentX=0;
	m_currentY=0;
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
		Audio_Unmute();
		
		//get the stored settings
		ReadFromIni();
    //
    VideoInputOnChange(m_VideoInput->GetValue(), m_VideoInput->GetValue());
	}
	catch(CDShowException &e)
	{
		ErrorBox(e.getErrorText());
	}
    NotifySquarePixelsCheck();
}

void CDSSource::Stop()
{
	CAutoCriticalSection lock(m_hOutThreadSync);
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
			ErrorBox(e.getErrorText());
		}
	}
	Audio_Mute();
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
  LOG(2,"DSSource: IsInTunerMode start");
  if(m_pDSGraph==NULL)
	{
		return FALSE;
	}
	CDShowCaptureDevice *pCap=NULL;
	if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	}

	if(pCap!=NULL)
  {
		  CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
		  if(pCrossbar!=NULL)
      {
	        LOG(2,"DSSource: IsInTunerMode?");
          long CurrentInput = pCrossbar->GetInputIndex();
          if (pCrossbar->GetInputType(CurrentInput) == PhysConn_Video_Tuner)
          {
             LOG(2,"DSSource: IsInTunerMode: Yes");
             return TRUE;              
          }
          LOG(2,"DSSource: IsInTunerMode: No");
      }
  }
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
	if(m_pDSGraph==NULL)
		return FALSE;
	
	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
		return FALSE;

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
		if(pCap->getTVTuner() != NULL)
    {
        return TRUE;
    }
  }

  return FALSE;
}


BOOL CDSSource::SetTunerFrequency(long FrequencyId, eVideoFormat VideoFormat)
{
	if(m_pDSGraph==NULL)
		return FALSE;
	
	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc==NULL)
		return FALSE;

	CDShowCaptureDevice *pCap=NULL;
	if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	{
		pCap=(CDShowCaptureDevice*)pSrc;
    CDShowTVTuner *pTvTuner = pCap->getTVTuner();
		if(pTvTuner == NULL) 
      return FALSE;
    
    LOG(2,"DSSource: SetTunerFrequency: Found TVTuner");

      //Choose a country with a unicable frequency table
      int CountryCode = 31; 
        
      long lCurrentCountryCode = pTvTuner->getCountryCode();
      if (CountryCode != lCurrentCountryCode)
      {
          LOG(2,"DSSource: SetTunerFrequency: Set country code");
          pTvTuner->putCountryCode(CountryCode);
      }

      
      TunerInputType pInputType = pTvTuner->getInputType();
      TunerInputType pNewInputType = TunerInputCable; //unicable frequency table
      
      if (pInputType != pNewInputType)
      {
          LOG(2,"DSSource: SetTunerFrequency: Set input type");
          pTvTuner->setInputType(pNewInputType); 
      }
      
      // set video format
      long lAnalogVideoStandard = pCap->getTVFormat();
      long lNewAnalogVideoStandard;
      if ((int)VideoFormat < 0)
      {          
          lNewAnalogVideoStandard = lAnalogVideoStandard;
      }
      else
      {
          lNewAnalogVideoStandard = eVideoFormatTable[VideoFormat];
      }      

      if (lAnalogVideoStandard != lNewAnalogVideoStandard)
      {
          LOG(2,"DSSource: SetTunerFrequency: pCap: Set TV format");
          pCap->putTVFormat(lNewAnalogVideoStandard);
      }
      LOG(2,"DSSource: SetTunerFrequency: Set Frequency %d",FrequencyId);
      return pTvTuner->setTunerFrequency(FrequencyId);
  }
  return FALSE;
}

BOOL CDSSource::IsVideoPresent()
{
	return TRUE;
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
	
	CDShowBaseSource *pSrc=m_pDSGraph->getSourceDevice();
	if(pSrc!=NULL)
	{
		if(pSrc->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
		{
			pCap=(CDShowCaptureDevice*)pSrc;
			pCrossbar=pCap->getCrossbar();
		}
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
		{
            LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
        }
		
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

	//set a radio checkmark infront of the current play/pause/stop menu entry
	FILTER_STATE state=m_pDSGraph->getState();
	UINT pos=8-state;
	menu->CheckMenuRadioItem(6,8,pos,MF_BYPOSITION);

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

ISetting* CDSSource::GetOverscan()
{
	if(!m_bIsFileSource)
	{
		return m_Overscan;
	}
	else
	{
		return NULL;
	}
}

void CDSSource::SetOverscan()
{
	if(!m_bIsFileSource)
	{
		AspectSettings.InitialOverscan = m_Overscan->GetValue();
	}
	else
	{
		AspectSettings.InitialOverscan = 0;
	}
}

void CDSSource::OverscanOnChange(long Overscan, long OldValue)
{
	AspectSettings.InitialOverscan = Overscan;
    WorkoutOverlaySize(TRUE);
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
		updateDroppedFields();
		return;
	}

	//width must be 16 byte aligned or optimized memcpy will not work
	//this assert will never be triggered (checked in dsrend filter)
	ASSERT((binfo.Width&0xf)==0);

	//check if size has changed
	if(m_currentX!=binfo.Width || m_currentY!=binfo.Height*2)
	{
		m_currentX=binfo.Width;
		m_currentY=binfo.Height*2;
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

	updateDroppedFields();
	
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
	}
	Timing_IncrementUsedFields();
	Timimg_AutoFormatDetect(pInfo);
	m_dwRendStartTime=timeGetTime();
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
        LOG(1, "DShow Exception - %s", (LPCSTR)e.getErrorText());
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

void DSSource_ChannelChange(void *pThis,int PreChange,int OldChannel,int NewChannel)
{
    if (pThis != NULL)
    {
        
        ((CDSSource*)pThis)->TunerChannelChange(PreChange,OldChannel,NewChannel);        
    }
}

void CDSSource::SettingsPerChannelSetup(int Start)
{
    if (Start&1)
    {
        SettingsPerChannel_RegisterSetSection(m_IDString.c_str());
        SettingsPerChannel_RegisterSetting("DSBrightness","DShow - Brightness",TRUE, m_Brightness);
        SettingsPerChannel_RegisterSetting("DSHue","DShow - Hue",TRUE, m_Hue);            
        SettingsPerChannel_RegisterSetting("DSContrast","DShow - Contrast",TRUE, m_Contrast);        
        SettingsPerChannel_RegisterSetting("DSSaturation","DShow - Saturation",TRUE, m_Saturation);

        SettingsPerChannel_RegisterSetting("DSOverscan","DShow - Overscan",TRUE, m_Overscan);

        Channel_Register_Change_Notification(this, DSSource_ChannelChange);

    }    
    else
    {
        SettingsPerChannel_UnregisterSection(m_IDString.c_str());        
        Channel_UnRegister_Change_Notification(this, DSSource_ChannelChange);
    }
}

    

void CDSSource::VideoInputOnChange(long input, long OldValue)
{
    if(m_pDSGraph==NULL)
	  {
		    return;
	  }
	  CDShowCaptureDevice *pCap=NULL;
	  if(m_pDSGraph->getSourceDevice()->getObjectType()==DSHOW_TYPE_SOURCE_CAPTURE)
	  {
		    pCap=(CDShowCaptureDevice*)m_pDSGraph->getSourceDevice();
	  }
    try
		{
	      if(pCap!=NULL)
		    {
				    CDShowBaseCrossbar *pCrossbar=pCap->getCrossbar();
				    if(pCrossbar!=NULL)
				    {
					      //if changing a video input, set the related pin too
					      long CurrentInputIndex = pCrossbar->GetInputIndex();
                long CurrentInputType = pCrossbar->GetInputType(CurrentInputIndex);
                SettingsPerChannel_VideoInputChange(this, 1, CurrentInputIndex, CurrentInputType == PhysConn_Video_Tuner);          
          
                long NewInputType = pCrossbar->GetInputType(input);
					      pCrossbar->SetInputIndex(input,(NewInputType<4096));          

                if (NewInputType == PhysConn_Video_Tuner)
                {
                    if (pCap->getTVTuner()!=NULL)
                    {
                        pCap->getTVTuner()->setInputPin(input);
                        LastTunerChannelOnChange(m_LastTunerChannel->GetValue(), m_LastTunerChannel->GetValue());
                    }
                }
          
                SettingsPerChannel_VideoInputChange(this, 0, input, NewInputType == PhysConn_Video_Tuner);
				      }
         }
	  }
    catch(CDShowException &e)
		{
			ErrorBox(CString("Failed to change input\n\n")+e.getErrorText());
		}

}

void CDSSource::TunerChannelChange(int PreChange, int OldChannel, int NewChannel)
{
    if (!PreChange)
    {
        m_LastTunerChannel->SetValue(NewChannel);
    }
}

void CDSSource::LastTunerChannelOnChange(long Channel, long OldValue)
{
   static int RecurseLevel = 1;
   if (RecurseLevel > 1)
   {
      return;
   }   
   RecurseLevel++;
   if (IsInTunerMode())
   {      
      Channel_Change(Channel, 0);
   }
   RecurseLevel--;
}



#endif
