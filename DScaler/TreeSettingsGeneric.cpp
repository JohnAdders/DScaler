/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsGeneric.cpp,v 1.1 2002-04-24 19:04:01 tobbej Exp $
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
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file TreeSettingsGeneric.cpp Implementation for CTreeSettingsGeneric class
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "TreeSettingsGeneric.h"
#include "Settings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsGeneric dialog

CTreeSettingsGeneric::CTreeSettingsGeneric(CString name,SETTING* settings,long count)
	:CTreeSettingsPage(name,CTreeSettingsGeneric::IDD),
	m_CurrentSetting(0),
	m_SettingsCount(count),
	m_Settings(settings)
{
    //{{AFX_DATA_INIT(CTreeSettingsGeneric)
    //}}AFX_DATA_INIT
}

void CTreeSettingsGeneric::DoDataExchange(CDataExchange* pDX)
{
    CTreeSettingsPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTreeSettingsGeneric)
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, m_Combo);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_CHECK, m_CheckBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SPIN, m_Spin);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SLIDER, m_Slider);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_EDIT, m_Edit);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_DEFAULT, m_DefaultButton);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_LIST, m_ListBox);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTreeSettingsGeneric, CTreeSettingsPage)
    //{{AFX_MSG_MAP(CTreeSettingsGeneric)
    ON_LBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_LIST, OnSelchangeList)
    ON_EN_CHANGE(IDC_TREESETTINGS_GENERIC_EDIT, OnChangeEdit)
    ON_WM_VSCROLL()
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_DEFAULT, OnSettingsDefault)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_CHECK, OnCheckClick)
    ON_CBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, OnSelchangeChoosefromlist)
    ON_NOTIFY(UDN_DELTAPOS, IDC_TREESETTINGS_GENERIC_SPIN, OnDeltaposSettingsSpin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsGeneric message handlers

BOOL CTreeSettingsGeneric::OnInitDialog()
{
    CTreeSettingsPage::OnInitDialog();

    //add relevant settings to listbox
    for(int i=0;i<m_SettingsCount;i++)
    {
        if(m_Settings[i].szDisplayName != NULL && m_Settings[i].Type != NOT_PRESENT)
        {
            int index=m_ListBox.AddString(m_Settings[i].szDisplayName);
            if(index!=LB_ERR)
            {
                m_ListBox.SetItemData(index,i);
            }
        }
    }

    if(m_SettingsCount>0)
    {
        m_ListBox.SetCurSel(0);
        OnSelchangeList();
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTreeSettingsGeneric::OnSelchangeList()
{
    long idx = m_ListBox.GetItemData(m_ListBox.GetCurSel());
    m_CurrentSetting = idx;

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
        if(m_Settings[idx].MaxValue <= UD_MAXVAL && m_Settings[idx].MinValue >= UD_MINVAL)
        {
            m_Spin.ShowWindow(SW_SHOWNA);
        }
        else
        {
            m_Spin.ShowWindow(SW_HIDE);
        }
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

void CTreeSettingsGeneric::UpdateControls()
{
    ASSERT(m_CurrentSetting>=0 && m_CurrentSetting<m_SettingsCount);

    static bool bInUpdate = false;

    //if we dont do this check there will be a loop
    if(bInUpdate)
    {
        return;
    }
    bInUpdate = true;


    if(m_Spin.GetStyle() & WS_VISIBLE)
    {
        m_Spin.SetRange32(m_Settings[m_CurrentSetting].MinValue,m_Settings[m_CurrentSetting].MaxValue);
        m_Spin.SetPos(*m_Settings[m_CurrentSetting].pValue);
    }

    if(m_Edit.GetStyle() & WS_VISIBLE)
    {
        CString newValue;
        newValue.Format("%d", *m_Settings[m_CurrentSetting].pValue);
        m_Edit.SetWindowText(newValue);
    }

    if(m_CheckBox.GetStyle() & WS_VISIBLE)
    {
        m_CheckBox.SetCheck(*m_Settings[m_CurrentSetting].pValue);
        m_CheckBox.SetWindowText(m_Settings[m_CurrentSetting].szDisplayName);
    }

    if(m_Slider.GetStyle() & WS_VISIBLE)
    {
        Setting_SetupSlider(&m_Settings[m_CurrentSetting], m_Slider.m_hWnd);
    }

    if(m_Combo.GetStyle() & WS_VISIBLE)
    {
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
    bInUpdate = false;
}

void CTreeSettingsGeneric::OnChangeEdit()
{
    if(m_Edit.m_hWnd==NULL)
        return;

    CString Value;
    m_Edit.GetWindowText(Value);
    Setting_SetValue(&m_Settings[m_CurrentSetting], atol(Value));
    UpdateControls();
}

void CTreeSettingsGeneric::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    //slider has changed
    Setting_SetFromControl(&m_Settings[m_CurrentSetting], m_Slider.m_hWnd);
    UpdateControls();
}

void CTreeSettingsGeneric::OnSettingsDefault()
{
    Setting_SetDefault(&m_Settings[m_CurrentSetting]);
    UpdateControls();
}

void CTreeSettingsGeneric::OnCheckClick()
{
    Setting_SetValue(&m_Settings[m_CurrentSetting], m_CheckBox.GetCheck());
    UpdateControls();
}

void CTreeSettingsGeneric::OnSelchangeChoosefromlist()
{
    if(m_Combo.GetCurSel()!=CB_ERR)
    {
        Setting_SetValue(&m_Settings[m_CurrentSetting], m_Combo.GetItemData(m_Combo.GetCurSel()));
    }
    UpdateControls();
}

void CTreeSettingsGeneric::OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult)
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

void CTreeSettingsGeneric::OnOK()
{
    WriteSettingsToIni(TRUE);
    CTreeSettingsPage::OnOK();
}