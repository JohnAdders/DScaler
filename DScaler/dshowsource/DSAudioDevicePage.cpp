/////////////////////////////////////////////////////////////////////////////
// $Id: DSAudioDevicePage.cpp,v 1.1 2002-09-15 12:20:07 tobbej Exp $
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
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSAudioDevicePage.cpp implementation file
 */

#include "stdafx.h"
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
	: CTreeSettingsPage(name,CDSAudioDevicePage::IDD),m_AudioDeviceSetting(AudioDevice)
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
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDSAudioDevicePage, CTreeSettingsPage)
	//{{AFX_MSG_MAP(CDSAudioDevicePage)
	ON_CBN_SELENDOK(IDC_DSHOW_AUDIODEVICE_DEVICE,OnSelEndOkAudioDevice)
	ON_BN_CLICKED(IDC_DSHOW_AUDIODEVICE_USEDEFAULT,OnClickedUseDefault)
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
	catch(CDShowDevEnumException e)
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