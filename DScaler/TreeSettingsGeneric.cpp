/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file TreeSettingsGeneric.cpp Implementation for CTreeSettingsGeneric class
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "TreeSettingsGeneric.h"
#include "Settings.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsGeneric dialog

CTreeSettingsGeneric::CTreeSettingsGeneric(CString name,SmartPtr<CSettingsHolder> SettingHolder)
    :CTreeSettingsPage(name,CTreeSettingsGeneric::IDD),
    m_CurrentSetting(0),
    m_Settings(SettingHolder)
{
    //{{AFX_DATA_INIT(CTreeSettingsGeneric)
    //}}AFX_DATA_INIT
    m_SettingsCount = m_Settings->GetNumSettings();
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
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_EDIT2, m_EditString);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_DEFAULT, m_DefaultButton);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_LIST, m_ListBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_GLOBAL, m_CheckGlobalBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_SOURCE, m_CheckSourceBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOINPUT, m_CheckVideoInputBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_AUDIOINPUT, m_CheckAudioInputBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOFORMAT, m_CheckVideoFormatBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SAVEPER_CHANNEL, m_CheckChannelBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_SETTINGINFO, m_SavePerInfoBox);
    DDX_Control(pDX, IDC_TREESETTINGS_GENERIC_TOPBOX, m_TopGroupBox);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTreeSettingsGeneric, CTreeSettingsPage)
    //{{AFX_MSG_MAP(CTreeSettingsGeneric)
    ON_LBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_LIST, OnSelchangeList)
    ON_EN_CHANGE(IDC_TREESETTINGS_GENERIC_EDIT, OnChangeEdit)
    ON_EN_KILLFOCUS(IDC_TREESETTINGS_GENERIC_EDIT2, OnChangeEditString)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_DEFAULT, OnSettingsDefault)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_CHECK, OnCheckClick)
    ON_CBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, OnSelchangeChoosefromlist)
    ON_NOTIFY(UDN_DELTAPOS, IDC_TREESETTINGS_GENERIC_SPIN, OnDeltaposSettingsSpin)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_GLOBAL, OnCheckGlobalClick)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_SOURCE, OnCheckSourceClick)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOINPUT, OnCheckVideoInputClick)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_AUDIOINPUT, OnCheckAudioInputClick)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOFORMAT, OnCheckVideoFormatClick)
    ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_SAVEPER_CHANNEL, OnCheckChannelClick)
    ON_WM_SIZE()
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
        CSimpleSetting* pSetting = m_Settings->GetSetting(i);
        if ((pSetting != NULL) && !pSetting->GetDisplayName().empty() && (pSetting->GetType() != NOT_PRESENT))
        {
            int index=m_ListBox.AddString(pSetting->GetDisplayName().c_str());
            if(index!=LB_ERR)
            {
                m_ListBox.SetItemData(index,i);
            }
        }
    }

    if(m_SettingsCount>0)
    {
        m_ListBox.SetCurSel(0);
    }
    OnSelchangeList();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTreeSettingsGeneric::OnSelchangeList()
{
    long idx = m_ListBox.GetItemData(m_ListBox.GetCurSel());
    m_CurrentSetting = idx;

    SETTING_TYPE Type = NOT_PRESENT;

    if (m_CurrentSetting >= 0 && m_CurrentSetting < m_SettingsCount)
    {
        Type = m_Settings->GetSetting(m_CurrentSetting)->GetType();

        switch(Type)
        {
        case YESNO:
        case ONOFF:
            m_DefaultButton.ShowWindow(SW_SHOWNA);
            m_CheckBox.ShowWindow(SW_SHOWNA);
            m_Edit.ShowWindow(SW_HIDE);
            m_EditString.ShowWindow(SW_HIDE);
            m_Slider.ShowWindow(SW_HIDE);
            m_Spin.ShowWindow(SW_HIDE);
            m_Combo.ShowWindow(SW_HIDE);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_CheckBox);
            break;

        case SLIDER:
            m_DefaultButton.ShowWindow(SW_SHOWNA);
            m_Edit.ShowWindow(SW_SHOWNA);
            m_Spin.ShowWindow(SW_SHOWNA);
            m_Slider.ShowWindow(SW_SHOWNA);
            m_EditString.ShowWindow(SW_HIDE);
            m_CheckBox.ShowWindow(SW_HIDE);
            m_Combo.ShowWindow(SW_HIDE);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Edit);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Spin);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Slider);
            break;

        case ITEMFROMLIST:
            m_CheckBox.ShowWindow(SW_HIDE);
            m_DefaultButton.ShowWindow(SW_SHOWNA);
            m_Edit.ShowWindow(SW_HIDE);
            m_EditString.ShowWindow(SW_HIDE);
            m_Slider.ShowWindow(SW_HIDE);
            m_Spin.ShowWindow(SW_HIDE);
            m_Combo.ShowWindow(SW_SHOWNA);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Combo);
            break;

        case CHARSTRING:
            m_DefaultButton.ShowWindow(SW_SHOWNA);
            m_EditString.ShowWindow(SW_SHOWNA);
            m_CheckBox.ShowWindow(SW_HIDE);
            m_Edit.ShowWindow(SW_HIDE);
            m_Slider.ShowWindow(SW_HIDE);
            m_Spin.ShowWindow(SW_HIDE);
            m_Combo.ShowWindow(SW_HIDE);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_EditString);
            break;
        default:
            break;
        }

        string szName;
        szName = m_Settings->GetSetting(m_CurrentSetting)->GetDisplayName();
        if (szName.empty())
        {
            szName = m_Settings->GetSetting(m_CurrentSetting)->GetEntry();
        }

        m_TopGroupBox.SetWindowText("");
        m_SavePerInfoBox.SetWindowText(szName.c_str());
        m_SavePerInfoBox.ShowWindow(SW_HIDE);
        
        UpdateControls(NULL);
    }
    else
    {
        m_CheckBox.ShowWindow(SW_HIDE);
        m_DefaultButton.ShowWindow(SW_HIDE);
        m_Edit.ShowWindow(SW_HIDE);
        m_EditString.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Spin.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);
    }

    m_CheckGlobalBox.ShowWindow(SW_HIDE);
    m_CheckSourceBox.ShowWindow(SW_HIDE);
    m_CheckVideoInputBox.ShowWindow(SW_HIDE);
    m_CheckAudioInputBox.ShowWindow(SW_HIDE);
    m_CheckVideoFormatBox.ShowWindow(SW_HIDE);
    m_CheckChannelBox.ShowWindow(SW_HIDE);
}

// atnak 2005/03/05:
// The control specified by 'pChangedControl' is excluded from
// the list of controls updated by this function.
void CTreeSettingsGeneric::UpdateControls(CWnd* pChangedControl)
{
    ASSERT(m_CurrentSetting>=0 && m_CurrentSetting<m_SettingsCount);

    static BOOL bInUpdate = FALSE;

    //if we dont do this check there will be a loop
    if(bInUpdate)
    {
        return;
    }
    bInUpdate = TRUE;

    if((m_Spin.GetStyle() & WS_VISIBLE) && (&m_Spin != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Spin);
    }

    if((m_Edit.GetStyle() & WS_VISIBLE) && (&m_Edit != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Edit);
    }

    if((m_EditString.GetStyle() & WS_VISIBLE) && (&m_EditString != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_EditString);
    }

    if((m_CheckBox.GetStyle() & WS_VISIBLE) && (&m_CheckBox != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_CheckBox);
    }

    if((m_Slider.GetStyle() & WS_VISIBLE) && (&m_Slider != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Slider);
    }

    if((m_Combo.GetStyle() & WS_VISIBLE) && (&m_Combo != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Combo);
    }

    bInUpdate = FALSE;
}

void CTreeSettingsGeneric::OnChangeEdit()
{
    if(m_Edit.m_hWnd==NULL)
        return;

    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_Edit);

    UpdateControls(&m_Edit);
}

void CTreeSettingsGeneric::OnChangeEditString()
{
    if(m_EditString.m_hWnd==NULL)
        return;

    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_EditString);

    UpdateControls(&m_EditString);
}

void CTreeSettingsGeneric::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    //slider has changed

    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_Slider.m_hWnd);

    UpdateControls(&m_Slider);
}

void CTreeSettingsGeneric::OnSettingsDefault()
{
    m_Settings->GetSetting(m_CurrentSetting)->ChangeValue(RESET_SILENT);

    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckClick()
{
    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_CheckBox);

    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckGlobalClick()
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckSourceClick()
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckVideoInputClick()
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckAudioInputClick()
{
    UpdateControls(NULL);
}


void CTreeSettingsGeneric::OnCheckVideoFormatClick()
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckChannelClick()
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnSelchangeChoosefromlist()
{
    if((m_Combo.GetCurSel()!=CB_ERR))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_Combo);
    }
    UpdateControls(&m_Combo);
}

void CTreeSettingsGeneric::OnDeltaposSettingsSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

    CSimpleSetting* Slider = m_Settings->GetSetting(m_CurrentSetting);

    if(pNMUpDown->iDelta > 0)
    {
        Slider->ChangeValue(ADJUSTDOWN_SILENT);
    }
    else
    {
        Slider->ChangeValue(ADJUSTUP_SILENT);
    }

    *pResult = 0;

    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnOK()
{
    //Write settings
    m_Settings->WriteToIni(TRUE);
    CTreeSettingsPage::OnOK();
}

void CTreeSettingsGeneric::OnSize(UINT nType, int cx, int cy)
{
    CTreeSettingsPage::OnSize(nType,cx,cy);

    RECT rect;
    int TopBoxBottom = -1;
    int ValueBoxTop = -1;

    CWnd *pTopBox = GetDlgItem(IDC_TREESETTINGS_GENERIC_TOPBOX);
    if (pTopBox != NULL)
    {
        pTopBox->GetWindowRect(&rect);
        ScreenToClient(&rect);
        rect.right = cx;
        pTopBox->MoveWindow(&rect);
        TopBoxBottom = rect.bottom;
    }


    CWnd *pValueBox = GetDlgItem(IDC_TREESETTINGS_GENERIC_VALUEBOX);
    if (pValueBox != NULL)
    {
        int DefaultBtnLeft = 0;
        RECT rcBox;
        RECT rcBoxOrg;

        pValueBox->GetWindowRect(&rect);
        ScreenToClient(&rect);

        rcBoxOrg = rect;

        rect.right = cx;
        rect.top = cy - (rect.bottom-rect.top);
        rect.bottom = cy;
        pValueBox->MoveWindow(&rect);
        ValueBoxTop = rect.top;

        // Items in box
        rcBox = rect;

        // Default button
        CWnd *pDefaultBtn = GetDlgItem(IDC_TREESETTINGS_GENERIC_DEFAULT);
        if (pDefaultBtn != NULL)
        {
            pDefaultBtn->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Width = (rect.right-rect.left);
            int Height= (rect.bottom-rect.top);

            rect.right = rcBox.right - 5;
            rect.bottom = rcBox.bottom - 5;

            rect.left = rect.right - Width;
            rect.top = rect.bottom - Height;
            pDefaultBtn->MoveWindow(&rect);

            DefaultBtnLeft = rect.left;
        }

        // Check box
        CWnd *pCheckBtn = GetDlgItem(IDC_TREESETTINGS_GENERIC_CHECK);
        if (pCheckBtn != NULL)
        {
            pCheckBtn->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            rect.right = rcBox.right - 5;
            pCheckBtn->MoveWindow(&rect);
        }

        // Edit box
        CWnd *pEditBox = GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT);
        if (pEditBox != NULL)
        {
            pEditBox->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pEditBox->MoveWindow(&rect);
        }

        // Edit box2
        CWnd *pEditBox2 = GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT2);
        if (pEditBox2 != NULL)
        {
            pEditBox2->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pEditBox2->MoveWindow(&rect);
        }

        // Spin
        CWnd *pSpinControl = GetDlgItem(IDC_TREESETTINGS_GENERIC_SPIN);
        if (pSpinControl != NULL)
        {
            pSpinControl->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pSpinControl->MoveWindow(&rect);
        }

        //Slider
        CWnd *pSlider = GetDlgItem(IDC_TREESETTINGS_GENERIC_SLIDER);
        if (pSlider != NULL)
        {
            pSlider->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pSlider->MoveWindow(&rect);
        }

        //Combo box
        CWnd *pChooseFromList = GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
        if (pChooseFromList != NULL)
        {
            pChooseFromList->GetWindowRect(&rect);
            ScreenToClient(&rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            pChooseFromList->MoveWindow(&rect);
        }
    }


    CWnd *pList = GetDlgItem(IDC_TREESETTINGS_GENERIC_LIST);
    if (pList != NULL)
    {
        pList->GetWindowRect(&rect);
        ScreenToClient(&rect);
        rect.right = cx;
        if (ValueBoxTop > rect.top)
        {
           rect.bottom = ValueBoxTop;
        }
        pList->MoveWindow(&rect);
    }
    Invalidate();

}
