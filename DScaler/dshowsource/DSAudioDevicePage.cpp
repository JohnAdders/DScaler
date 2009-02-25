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
// Revision 1.3  2003/02/05 19:12:38  tobbej
// added support for capture devices where audio can be rendered from directshow
// modified audio setings dialog so audio rendering can be turned off (usefull for devices with both internal and external audio)
//
// Revision 1.2  2002/09/17 19:29:56  adcockj
// Fixed compile problems with dshow off
//
// Revision 1.1  2002/09/15 12:20:07  tobbej
// implemented audio output device selection
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSAudioDevicePage.cpp implementation file
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "..\dscaler.h"
#include "DSAudioDevicePage.h"
#include "DevEnum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDSAudioDevicePage dialog


CDSAudioDevicePage::CDSAudioDevicePage(CString name,std::string &AudioDevice)
	: CTreeSettingsPage(name,CDSAudioDevicePage::IDD),m_AudioDeviceSetting(AudioDevice),m_bConnectAudio(NULL)
{
	//{{AFX_DATA_INIT(CDSAudioDevicePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CDSAudioDevicePage::CDSAudioDevicePage(CString name,std::string &AudioDevice,bool *bConnectAudio)
	: CTreeSettingsPage(name,CDSAudioDevicePage::IDD),m_AudioDeviceSetting(AudioDevice),m_bConnectAudio(bConnectAudio)
{
	//{{AFX_DATA_INIT(CDSAudioDevicePage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDSAudioDevicePage::DoDataExchange(CDataExchange* pDX)
{
	CTreeSettingsPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDSAudioDevicePage)
	DDX_Control(pDX, IDC_DSHOW_AUDIODEVICE_DEVICE, m_AudioDevice);
	DDX_Control(pDX, IDC_DSHOW_AUDIODEVICE_USEDEFAULT, m_UseDefault);
	DDX_Control(pDX, IDC_DSHOW_AUDIODEVICE_RENDER, m_ConnectAudio);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDSAudioDevicePage, CTreeSettingsPage)
	//{{AFX_MSG_MAP(CDSAudioDevicePage)
	ON_CBN_SELENDOK(IDC_DSHOW_AUDIODEVICE_DEVICE,OnSelEndOkAudioDevice)
	ON_BN_CLICKED(IDC_DSHOW_AUDIODEVICE_USEDEFAULT,OnClickedUseDefault)
	ON_BN_CLICKED(IDC_DSHOW_AUDIODEVICE_RENDER,OnClickedConnectAudio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CDSAudioDevicePage::OnInitDialog() 
{
	CTreeSettingsPage::OnInitDialog();

	try
	{
		CDShowDevEnum devenum(CLSID_AudioRendererCategory);
		while(devenum.getNext()==true)
		{
			string deviceName=devenum.getProperty("FriendlyName");
			int pos=m_AudioDevice.AddString(deviceName.c_str());
			if(pos!=CB_ERR)
			{
				m_DeviceList.push_back(devenum.getDisplayName());
				m_AudioDevice.SetItemData(pos,m_DeviceList.size()-1);
				if(m_AudioDeviceSetting==devenum.getDisplayName())
				{
					m_AudioDevice.SetCurSel(pos);
				}
			}
		}
	}
	catch(CDShowDevEnumException&)
	{
	}
	
	if(m_AudioDeviceSetting.size()==0)
	{
		m_UseDefault.SetCheck(BST_CHECKED);
		OnClickedUseDefault();
	}
	
	if(m_AudioDevice.GetCurSel()==CB_ERR && m_DeviceList.size()>0)
	{
		m_AudioDevice.SetCurSel(0);
	}
	
	if(m_bConnectAudio!=NULL)
	{
		m_ConnectAudio.SetCheck(*m_bConnectAudio?BST_UNCHECKED:BST_CHECKED);
		OnClickedConnectAudio();
	}
	else
	{
		m_ConnectAudio.ShowWindow(SW_HIDE);
		::ShowWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_TEXT2),SW_HIDE);
	}
	
	return TRUE;
}

void CDSAudioDevicePage::OnOK()
{
	int pos=m_AudioDevice.GetCurSel();
	if(pos!=CB_ERR && m_UseDefault.GetCheck()==BST_UNCHECKED)
	{
		int index=m_AudioDevice.GetItemData(pos);
		m_AudioDeviceSetting=m_DeviceList[index];
	}
	else
	{
		m_AudioDeviceSetting="";
	}
	if(m_bConnectAudio!=NULL)
	{
		*m_bConnectAudio=(m_ConnectAudio.GetCheck()==BST_UNCHECKED);
	}
}

void CDSAudioDevicePage::OnSelEndOkAudioDevice()
{
	
}

void CDSAudioDevicePage::OnClickedUseDefault()
{
	if(m_UseDefault.GetCheck()==BST_CHECKED)
	{
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE),FALSE);
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE),TRUE);
	}
}

void CDSAudioDevicePage::OnClickedConnectAudio()
{
	if(m_ConnectAudio.GetCheck()==BST_UNCHECKED)
	{
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE_TEXT),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_GRPBOX),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_USEDEFAULT),TRUE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE_LABEL),TRUE);
		OnClickedUseDefault();
	}
	else
	{
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE_TEXT),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_GRPBOX),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_USEDEFAULT),FALSE);
		::EnableWindow(::GetDlgItem(m_hWnd,IDC_DSHOW_AUDIODEVICE_DEVICE_LABEL),FALSE);
	}
}

#endif