/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsDlg.cpp,v 1.5 2001-06-27 18:48:29 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 22 Jun 2001   Torbjörn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingsDlg.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

CSettingsDlg::CSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingsDlg::IDD, pParent),m_currentSetting(0)
{
	//{{AFX_DATA_INIT(CSettingsDlg)
	//}}AFX_DATA_INIT
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDlg)
	DDX_Control(pDX, IDC_CHOOSEFROMLIST, m_combo);
	DDX_Control(pDX, IDC_SETTINGS_CHECK, m_chk);
	DDX_Control(pDX, IDC_SETTINGS_SPIN, m_spin);
	DDX_Control(pDX, IDC_SETTINGS_SLIDER, m_slider);
	DDX_Control(pDX, IDC_SETTINGS_EDIT, m_edit);
	DDX_Control(pDX, IDC_SETTINGS_DEFAULT, m_btnDefault);
	DDX_Control(pDX, IDC_SETTINGS_LIST, m_lstbox);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CSettingsDlg)
	ON_LBN_SELCHANGE(IDC_SETTINGS_LIST, OnSelchangeList)
	ON_EN_CHANGE(IDC_SETTINGS_EDIT, OnChangeEdit)
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_SETTINGS_DEFAULT, OnSettingsDefault)
	ON_BN_CLICKED(IDC_SETTINGS_CHECK, OnCheckClick)
	ON_CBN_SELCHANGE(IDC_CHOOSEFROMLIST, OnSelchangeChoosefromlist)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SETTINGS_SPIN, OnDeltaposSettingsSpin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg message handlers

BOOL CSettingsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	//add all settings to listbox
	//shoud probably filter out settings with NOT_PRESENT type
	for(int i=0;i<m_settingsCount;i++)
	{
		int index=m_lstbox.AddString(m_settings[i].szDisplayName);
		if(index!=LB_ERR)
		{
			m_lstbox.SetItemData(index,i);
		}
	}
	SetWindowText(m_caption);
	
	if(m_settingsCount>0)
	{
		m_lstbox.SetCurSel(0);
		OnSelchangeList();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSettingsDlg::ShowSettingsDlg(CString caption,SETTING *settings,long count,CWnd *pParent)
{
	if(settings==NULL || count==0)
	{
		AfxMessageBox("No settings to display",MB_OK|MB_ICONINFORMATION);
		return;
	}

	CSettingsDlg dlg(pParent);
	dlg.m_settingsCount=count;
	dlg.m_settings=settings;
	dlg.m_caption=caption;

	dlg.DoModal();
}

void CSettingsDlg::OnSelchangeList() 
{
	long idx=m_lstbox.GetItemData(m_lstbox.GetCurSel());
	m_currentSetting=idx;
	
	switch(m_settings[idx].Type)
	{
	case YESNO:
	case ONOFF:
		m_btnDefault.ShowWindow(SW_SHOWNA);
		m_chk.ShowWindow(SW_SHOWNA);
		m_edit.ShowWindow(SW_HIDE);
		m_slider.ShowWindow(SW_HIDE);
		m_spin.ShowWindow(SW_HIDE);		
		m_combo.ShowWindow(SW_HIDE);
		break;

	case SLIDER:
		m_btnDefault.ShowWindow(SW_SHOWNA);
		m_edit.ShowWindow(SW_SHOWNA);
		m_spin.ShowWindow(SW_SHOWNA);
		m_slider.ShowWindow(SW_SHOWNA);
		m_chk.ShowWindow(SW_HIDE);
		m_combo.ShowWindow(SW_HIDE);
		break;

	case ITEMFROMLIST:
		m_chk.ShowWindow(SW_HIDE);
		m_btnDefault.ShowWindow(SW_SHOWNA);
		m_edit.ShowWindow(SW_HIDE);
		m_slider.ShowWindow(SW_HIDE);
		m_spin.ShowWindow(SW_HIDE);
		m_combo.ShowWindow(SW_SHOWNA);
		break;

	default:
		m_chk.ShowWindow(SW_HIDE);
		m_btnDefault.ShowWindow(SW_HIDE);
		m_edit.ShowWindow(SW_HIDE);
		m_slider.ShowWindow(SW_HIDE);
		m_spin.ShowWindow(SW_HIDE);
		m_combo.ShowWindow(SW_HIDE);
	}
	
	UpdateControls();
}

void CSettingsDlg::UpdateControls()
{
	ASSERT(m_currentSetting>=0 && m_currentSetting<m_settingsCount);

	CString newValue;
	CString oldValue;
	newValue.Format("%ld",*m_settings[m_currentSetting].pValue);
	m_edit.GetWindowText(oldValue);
	
	//if we dont do this check there will be a loop
	if(oldValue!=newValue)
	{
		m_edit.SetWindowText(newValue);
		m_spin.SetRange32(m_settings[m_currentSetting].MinValue,m_settings[m_currentSetting].MaxValue);
		m_spin.SetPos(*m_settings[m_currentSetting].pValue);
	}
	
	m_chk.SetCheck(*m_settings[m_currentSetting].pValue);
	m_chk.SetWindowText(m_settings[m_currentSetting].szDisplayName);
	
	m_slider.SetRange(m_settings[m_currentSetting].MinValue,m_settings[m_currentSetting].MaxValue);
	m_slider.ClearTics(TRUE);
	m_slider.SetTic(m_settings[m_currentSetting].Default);
	m_slider.SetPos(*m_settings[m_currentSetting].pValue);
	m_slider.SetPageSize(m_settings[m_currentSetting].StepValue);
	
	m_combo.ResetContent();
	if(m_settings[m_currentSetting].pszList != NULL)
	{
		bool bFoundSetting=false;
		for(int i(m_settings[m_currentSetting].MinValue); i <= m_settings[m_currentSetting].MaxValue; ++i)
		{
			//is there any text for this item?
			if(strlen(m_settings[m_currentSetting].pszList[i])>0)
			{
				int pos=m_combo.AddString(m_settings[m_currentSetting].pszList[i]);
				
				//store value in itemdata
				m_combo.SetItemData(pos,i);
				
				//is this item the current value?
				if(Setting_GetValue(&m_settings[m_currentSetting]) == i)
				{
					m_combo.SetCurSel(pos);
					bFoundSetting=true;
				}
			}
		}
		if(bFoundSetting==false)
		{
			//clear selection since we didnt find pValue
			m_combo.SetCurSel(-1);
		}
	}
}

void CSettingsDlg::OnChangeEdit() 
{
	if(m_edit.m_hWnd==NULL)
		return;

	CString value;
	m_edit.GetWindowText(value);
	Setting_SetValue(&m_settings[m_currentSetting], atol(value));
	UpdateControls();
}

void CSettingsDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//slider has changed
	long value=m_slider.GetPos();
	
	Setting_SetValue(&m_settings[m_currentSetting], value);
	UpdateControls();
}

void CSettingsDlg::OnSettingsDefault() 
{
	Setting_SetDefault(&m_settings[m_currentSetting]);
	UpdateControls();
}

void CSettingsDlg::OnCheckClick() 
{
	Setting_SetValue(&m_settings[m_currentSetting], m_chk.GetCheck());
	UpdateControls();	
}

void CSettingsDlg::OnSelchangeChoosefromlist()
{
	if(m_combo.GetCurSel()!=CB_ERR)
	{
		Setting_SetValue(&m_settings[m_currentSetting], m_combo.GetItemData(m_combo.GetCurSel()));
	}
	UpdateControls();
}

void CSettingsDlg::OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

	if(pNMUpDown->iDelta > 0)
	{
		Setting_Up(&m_settings[m_currentSetting]);
	}
	else
	{
		Setting_Down(&m_settings[m_currentSetting]);
	}
		
	*pResult = *m_settings[m_currentSetting].pValue;

	UpdateControls();
}
