/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsDlg.cpp,v 1.10 2001-07-14 16:06:57 tobbej Exp $
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
// 22 Jun 2001   Torbjörn Jansson      Initial release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.9  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.8  2001/07/12 19:24:35  adcockj
// Fixes for vertical sliders
//
// Revision 1.7  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

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

CSettingsDlg::CSettingsDlg(CWnd* pParent)
    : CDialog(CSettingsDlg::IDD, pParent),m_CurrentSetting(0)
{
    //{{AFX_DATA_INIT(CSettingsDlg)
    //}}AFX_DATA_INIT
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSettingsDlg)
    DDX_Control(pDX, IDC_CHOOSEFROMLIST, m_Combo);
    DDX_Control(pDX, IDC_SETTINGS_CHECK, m_CheckBox);
    DDX_Control(pDX, IDC_SETTINGS_SPIN, m_Spin);
    DDX_Control(pDX, IDC_SETTINGS_SLIDER, m_Slider);
    DDX_Control(pDX, IDC_SETTINGS_EDIT, m_Edit);
    DDX_Control(pDX, IDC_SETTINGS_DEFAULT, m_DefaultButton);
    DDX_Control(pDX, IDC_SETTINGS_LIST, m_ListBox);
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
    //shoud probably filter out settings with NOT_PRESENT Type
    for(int i=0;i<m_SettingsCount;i++)
    {
        int index=m_ListBox.AddString(m_Settings[i].szDisplayName);
        if(index!=LB_ERR)
        {
            m_ListBox.SetItemData(index,i);
        }
    }
    SetWindowText(m_Caption);
    
    if(m_SettingsCount>0)
    {
        m_ListBox.SetCurSel(0);
        OnSelchangeList();
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSettingsDlg::ShowSettingsDlg(CString caption,SETTING *settings,long Count,CWnd *pParent)
{
    if(settings==NULL || Count==0)
    {
        AfxMessageBox("No settings to display",MB_OK|MB_ICONINFORMATION);
        return;
    }

    CSettingsDlg dlg(pParent);
    dlg.m_SettingsCount=Count;
    dlg.m_Settings=settings;
    dlg.m_Caption=caption;

    dlg.DoModal();
}

void CSettingsDlg::OnSelchangeList() 
{
    long idx=m_ListBox.GetItemData(m_ListBox.GetCurSel());
    m_CurrentSetting=idx;
    
    switch(m_Settings[idx].Type)
    {
    case YESNO:
    case ONOFF:
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_CheckBox.ShowWindow(SW_SHOWNA);
        m_Edit.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);     
        m_Combo.ShowWindow(SW_HIDE);
        break;

    case SLIDER:
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_Edit.ShowWindow(SW_SHOWNA);
        m_Spin.ShowWindow(SW_SHOWNA);
        m_Slider.ShowWindow(SW_SHOWNA);
        m_CheckBox.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
        break;

    case ITEMFROMLIST:
        m_CheckBox.ShowWindow(SW_HIDE);
        m_DefaultButton.ShowWindow(SW_SHOWNA);
        m_Edit.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_SHOWNA);
        break;

    default:
        m_CheckBox.ShowWindow(SW_HIDE);
        m_DefaultButton.ShowWindow(SW_HIDE);
        m_Edit.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
    }
    
    UpdateControls();
}

void CSettingsDlg::UpdateControls()
{
    ASSERT(m_CurrentSetting>=0 && m_CurrentSetting<m_SettingsCount);

    CString newValue;
    CString oldValue;
    newValue.Format("%ld",*m_Settings[m_CurrentSetting].pValue);
    m_Edit.GetWindowText(oldValue);
    
    //if we dont do this check there will be a loop
    if(oldValue!=newValue)
    {
        m_Edit.SetWindowText(newValue);
        m_Spin.SetRange32(m_Settings[m_CurrentSetting].MinValue,m_Settings[m_CurrentSetting].MaxValue);
        m_Spin.SetPos(*m_Settings[m_CurrentSetting].pValue);
    }
    
    m_CheckBox.SetCheck(*m_Settings[m_CurrentSetting].pValue);
    m_CheckBox.SetWindowText(m_Settings[m_CurrentSetting].szDisplayName);

    Setting_SetupSlider(&m_Settings[m_CurrentSetting], m_Slider.m_hWnd);
    
    m_Combo.ResetContent();
    if(m_Settings[m_CurrentSetting].pszList != NULL)
    {
        bool bFoundSetting=false;
        for(int i(m_Settings[m_CurrentSetting].MinValue); i <= m_Settings[m_CurrentSetting].MaxValue; ++i)
        {
            //is there any text for this item?
            if(strlen(m_Settings[m_CurrentSetting].pszList[i])>0)
            {
                int pos=m_Combo.AddString(m_Settings[m_CurrentSetting].pszList[i]);
                
                //store Value in itemdata
                m_Combo.SetItemData(pos,i);
                
                //is this item the current Value?
                if(Setting_GetValue(&m_Settings[m_CurrentSetting]) == i)
                {
                    m_Combo.SetCurSel(pos);
                    bFoundSetting=true;
                }
            }
        }
        if(bFoundSetting==false)
        {
            //clear selection since we didnt find pValue
            m_Combo.SetCurSel(-1);
        }
    }
}

void CSettingsDlg::OnChangeEdit() 
{
    if(m_Edit.m_hWnd==NULL)
        return;

    CString Value;
    m_Edit.GetWindowText(Value);
    Setting_SetValue(&m_Settings[m_CurrentSetting], atol(Value));
    UpdateControls();
}

void CSettingsDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    //slider has changed
    Setting_SetFromControl(&m_Settings[m_CurrentSetting], m_Slider.m_hWnd);
    UpdateControls();
}

void CSettingsDlg::OnSettingsDefault() 
{
    Setting_SetDefault(&m_Settings[m_CurrentSetting]);
    UpdateControls();
}

void CSettingsDlg::OnCheckClick() 
{
    Setting_SetValue(&m_Settings[m_CurrentSetting], m_CheckBox.GetCheck());
    UpdateControls();   
}

void CSettingsDlg::OnSelchangeChoosefromlist()
{
    if(m_Combo.GetCurSel()!=CB_ERR)
    {
        Setting_SetValue(&m_Settings[m_CurrentSetting], m_Combo.GetItemData(m_Combo.GetCurSel()));
    }
    UpdateControls();
}

void CSettingsDlg::OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    if(pNMUpDown->iDelta > 0)
    {
        Setting_Up(&m_Settings[m_CurrentSetting]);
    }
    else
    {
        Setting_Down(&m_Settings[m_CurrentSetting]);
    }
        
    *pResult = *m_Settings[m_CurrentSetting].pValue;

    UpdateControls();
}
