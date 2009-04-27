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

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsGeneric dialog

CTreeSettingsGeneric::CTreeSettingsGeneric(const string& name,SmartPtr<CSettingsHolder> SettingHolder)
    :CTreeSettingsPage(name, IDD_TREESETTINGS_GENERIC),
    m_CurrentSetting(0),
    m_Settings(SettingHolder),
    m_Combo(NULL),
    m_CheckBox(NULL),
    m_Spin(NULL),
    m_Slider(NULL),
    m_Edit(NULL),
    m_EditString(NULL),
    m_DefaultButton(NULL),
    m_ListBox(NULL),
    m_CheckGlobal(NULL),
    m_CheckGlobalBox(NULL),
    m_CheckSourceBox(NULL),
    m_CheckVideoInputBox(NULL),
    m_CheckAudioInputBox(NULL),
    m_CheckVideoFormatBox(NULL),
    m_CheckChannelBox(NULL),
    m_SavePerInfoBox(NULL),
    m_TopGroupBox(NULL)
{
    m_SettingsCount = m_Settings->GetNumSettings();
}

BOOL CTreeSettingsGeneric::ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    HANDLE_MSG(hDlg, WM_HSCROLL, OnHScroll);
    HANDLE_MSG(hDlg, WM_NOTIFY, OnNotify);
    HANDLE_MSG(hDlg, WM_SIZE, OnSize);
    default:
        return FALSE;
    }
}

void CTreeSettingsGeneric::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_TREESETTINGS_GENERIC_LIST:
        if(LBN_SELCHANGE == codeNotify)
        {
            OnSelchangeList(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_EDIT:
        if(EN_CHANGE == codeNotify)
        {
            OnChangeEdit(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_EDIT2:
        if(EN_KILLFOCUS == codeNotify)
        {
            OnChangeEditString(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_DEFAULT:
        if(BN_CLICKED == codeNotify)
        {
            OnSettingsDefault(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_CHECK:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckClick(hDlg);
        }
    break;
        case IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST:
        if(CBN_SELCHANGE == codeNotify)
        {
            OnSelchangeChoosefromlist(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_SAVEPER_GLOBAL:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckGlobalClick(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_SAVEPER_SOURCE:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckSourceClick(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOINPUT:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckVideoInputClick(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_SAVEPER_AUDIOINPUT:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckAudioInputClick(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOFORMAT:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckVideoFormatClick(hDlg);
        }
        break;
    case IDC_TREESETTINGS_GENERIC_SAVEPER_CHANNEL:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckChannelClick(hDlg);
        }
        break;
    }
}

LRESULT CTreeSettingsGeneric::OnNotify(HWND hwnd, int id, LPNMHDR nmhdr)
{
    switch(id)
    {
    case IDC_TREESETTINGS_GENERIC_SPIN:
        if(nmhdr->code == UDN_DELTAPOS)
        {
            return OnDeltaposSettingsSpin(hwnd, nmhdr);
        }
        break;
    default:
        break;
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CTreeSettingsGeneric message handlers

BOOL CTreeSettingsGeneric::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    // get the window handles of all the controls we care about
    m_Combo = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
    m_CheckBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_CHECK);
    m_Spin = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SPIN);
    m_Slider = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SLIDER);
    m_Edit = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_EDIT);
    m_EditString = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_EDIT2);
    m_DefaultButton = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_DEFAULT);
    m_ListBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_LIST);
    m_CheckGlobalBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SAVEPER_GLOBAL);
    m_CheckSourceBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SAVEPER_SOURCE);
    m_CheckVideoInputBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOINPUT);
    m_CheckAudioInputBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SAVEPER_AUDIOINPUT);
    m_CheckVideoFormatBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOFORMAT);
    m_CheckChannelBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SAVEPER_CHANNEL);
    m_SavePerInfoBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SETTINGINFO);
    m_TopGroupBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_TOPBOX);

    //add relevant settings to listbox
    for(int i=0;i<m_SettingsCount;i++)
    {
        CSimpleSetting* pSetting = m_Settings->GetSetting(i);
        if ((pSetting != NULL) && !pSetting->GetDisplayName().empty() && (pSetting->GetType() != NOT_PRESENT))
        {
            int index=ListBox_AddString(m_ListBox, pSetting->GetDisplayName().c_str());
            if(index!=LB_ERR)
            {
                ListBox_SetItemData(m_ListBox, index, i);
            }
        }
    }

    if(m_SettingsCount>0)
    {
        ListBox_SetCurSel(m_ListBox, 0);
    }
    OnSelchangeList(hDlg);
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTreeSettingsGeneric::OnSelchangeList(HWND hDlg)
{
    long idx = ListBox_GetItemData(m_ListBox, ListBox_GetCurSel(m_ListBox));
    m_CurrentSetting = idx;

    SETTING_TYPE Type = NOT_PRESENT;

    if (m_CurrentSetting >= 0 && m_CurrentSetting < m_SettingsCount)
    {
        Type = m_Settings->GetSetting(m_CurrentSetting)->GetType();

        switch(Type)
        {
        case YESNO:
        case ONOFF:
            ShowWindow(m_DefaultButton, SW_SHOWNA);
            ShowWindow(m_CheckBox, SW_SHOWNA);
            ShowWindow(m_Edit, SW_HIDE);
            ShowWindow(m_EditString, SW_HIDE);
            ShowWindow(m_Slider, SW_HIDE);
            ShowWindow(m_Spin, SW_HIDE);
            ShowWindow(m_Combo, SW_HIDE);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_CheckBox);
            break;

        case SLIDER:
            ShowWindow(m_DefaultButton, SW_SHOWNA);
            ShowWindow(m_Edit, SW_SHOWNA);
            ShowWindow(m_Spin, SW_SHOWNA);
            ShowWindow(m_Slider, SW_SHOWNA);
            ShowWindow(m_EditString, SW_HIDE);
            ShowWindow(m_CheckBox, SW_HIDE);
            ShowWindow(m_Combo, SW_HIDE);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Edit);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Spin);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Slider);
            break;

        case ITEMFROMLIST:
            ShowWindow(m_CheckBox, SW_HIDE);
            ShowWindow(m_DefaultButton, SW_SHOWNA);
            ShowWindow(m_Edit, SW_HIDE);
            ShowWindow(m_EditString, SW_HIDE);
            ShowWindow(m_Slider, SW_HIDE);
            ShowWindow(m_Spin, SW_HIDE);
            ShowWindow(m_Combo, SW_SHOWNA);
            m_Settings->GetSetting(m_CurrentSetting)->SetupControl(m_Combo);
            break;

        case CHARSTRING:
            ShowWindow(m_DefaultButton, SW_SHOWNA);
            ShowWindow(m_EditString, SW_SHOWNA);
            ShowWindow(m_CheckBox, SW_HIDE);
            ShowWindow(m_Edit, SW_HIDE);
            ShowWindow(m_Slider, SW_HIDE);
            ShowWindow(m_Spin, SW_HIDE);
            ShowWindow(m_Combo, SW_HIDE);
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

        SetWindowText(m_TopGroupBox, "");
        SetWindowText(m_SavePerInfoBox, szName.c_str());
        ShowWindow(m_SavePerInfoBox, SW_HIDE);

        UpdateControls(NULL);
    }
    else
    {
        ShowWindow(m_CheckBox, SW_HIDE);
        ShowWindow(m_DefaultButton, SW_HIDE);
        ShowWindow(m_Edit, SW_HIDE);
        ShowWindow(m_EditString, SW_HIDE);
        ShowWindow(m_Slider, SW_HIDE);
        ShowWindow(m_Spin, SW_HIDE);
        ShowWindow(m_Combo, SW_HIDE);
    }

    ShowWindow(m_CheckGlobalBox, SW_HIDE);
    ShowWindow(m_CheckSourceBox, SW_HIDE);
    ShowWindow(m_CheckVideoInputBox, SW_HIDE);
    ShowWindow(m_CheckAudioInputBox, SW_HIDE);
    ShowWindow(m_CheckVideoFormatBox, SW_HIDE);
    ShowWindow(m_CheckChannelBox, SW_HIDE);
}

// atnak 2005/03/05:
// The control specified by 'pChangedControl' is excluded from
// the list of controls updated by this function.
void CTreeSettingsGeneric::UpdateControls(HWND pChangedControl)
{
    _ASSERTE(m_CurrentSetting>=0 && m_CurrentSetting<m_SettingsCount);

    static BOOL bInUpdate = FALSE;

    //if we dont do this check there will be a loop
    if(bInUpdate)
    {
        return;
    }
    bInUpdate = TRUE;

    if((GetWindowLong(m_Spin, GWL_STYLE) & WS_VISIBLE) && (m_Spin != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Spin);
    }

    if((GetWindowLong(m_Edit, GWL_STYLE) & WS_VISIBLE) && (m_Edit != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Edit);
    }

    if((GetWindowLong(m_EditString, GWL_STYLE) & WS_VISIBLE) && (m_EditString != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_EditString);
    }

    if((GetWindowLong(m_CheckBox, GWL_STYLE) & WS_VISIBLE) && (m_CheckBox != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_CheckBox);
    }

    if((GetWindowLong(m_Slider, GWL_STYLE) & WS_VISIBLE) && (m_Slider != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Slider);
    }

    if((GetWindowLong(m_Combo, GWL_STYLE) & WS_VISIBLE) && (m_Combo != pChangedControl))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetControlValue(m_Combo);
    }
    bInUpdate = FALSE;
}

void CTreeSettingsGeneric::OnChangeEdit(HWND hDlg)
{
    if(m_Edit == NULL)
        return;

    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_Edit);

    UpdateControls(m_Edit);
}

void CTreeSettingsGeneric::OnChangeEditString(HWND hDlg)
{
    if(m_EditString==NULL)
        return;

    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_EditString);

    UpdateControls(m_EditString);
}

void CTreeSettingsGeneric::OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
    //slider has changed

    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_Slider);

    UpdateControls(m_Slider);
}

void CTreeSettingsGeneric::OnSettingsDefault(HWND hDlg)
{
    m_Settings->GetSetting(m_CurrentSetting)->ChangeValue(RESET_SILENT);

    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckClick(HWND hDlg)
{
    m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_CheckBox);

    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckGlobalClick(HWND hDlg)
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckSourceClick(HWND hDlg)
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckVideoInputClick(HWND hDlg)
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckAudioInputClick(HWND hDlg)
{
    UpdateControls(NULL);
}


void CTreeSettingsGeneric::OnCheckVideoFormatClick(HWND hDlg)
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnCheckChannelClick(HWND hDlg)
{
    UpdateControls(NULL);
}

void CTreeSettingsGeneric::OnSelchangeChoosefromlist(HWND hDlg)
{
    if((ComboBox_GetCurSel(m_Combo)!=CB_ERR))
    {
        m_Settings->GetSetting(m_CurrentSetting)->SetFromControl(m_Combo);
    }
    UpdateControls(m_Combo);
}

LRESULT CTreeSettingsGeneric::OnDeltaposSettingsSpin(HWND hDlg, NMHDR* pNMHDR)
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

    UpdateControls(NULL);
    return 0;
}

void CTreeSettingsGeneric::OnOK(HWND hDlg)
{
    //Write settings
    m_Settings->WriteToIni(TRUE);
    EndDialog(hDlg, IDOK);
}

void CTreeSettingsGeneric::OnSize(HWND hDlg, UINT nType, int cx, int cy)
{
    RECT rect;
    int TopBoxBottom = -1;
    int ValueBoxTop = -1;

    HWND pTopBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_TOPBOX);
    if (pTopBox != NULL)
    {
        GetWindowRect(pTopBox, &rect);
        ScreenToClient(hDlg, rect);
        rect.right = cx;
        MoveWindow(pTopBox, rect);
        TopBoxBottom = rect.bottom;
    }


    HWND pValueBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_VALUEBOX);
    if (pValueBox != NULL)
    {
        int DefaultBtnLeft = 0;
        RECT rcBox;
        RECT rcBoxOrg;

        GetWindowRect(pValueBox, &rect);
        ScreenToClient(hDlg, rect);

        rcBoxOrg = rect;

        rect.right = cx;
        rect.top = cy - (rect.bottom-rect.top);
        rect.bottom = cy;
        MoveWindow(pValueBox, rect);
        ValueBoxTop = rect.top;

        // Items in box
        rcBox = rect;

        // Default button
        HWND pDefaultBtn = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_DEFAULT);
        if (pDefaultBtn != NULL)
        {
            GetWindowRect(pDefaultBtn, &rect);
            ScreenToClient(hDlg, rect);
            int Width = (rect.right-rect.left);
            int Height= (rect.bottom-rect.top);

            rect.right = rcBox.right - 5;
            rect.bottom = rcBox.bottom - 5;

            rect.left = rect.right - Width;
            rect.top = rect.bottom - Height;
            MoveWindow(pDefaultBtn, rect);

            DefaultBtnLeft = rect.left;
        }

        // Check box
        HWND pCheckBtn = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_CHECK);
        if (pCheckBtn != NULL)
        {
            GetWindowRect(pCheckBtn, &rect);
            ScreenToClient(hDlg, rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            rect.right = rcBox.right - 5;
            MoveWindow(pCheckBtn, rect);
        }

        // Edit box
        HWND pEditBox = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_EDIT);
        if (pEditBox != NULL)
        {
            GetWindowRect(pEditBox, &rect);
            ScreenToClient(hDlg, rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            MoveWindow(pEditBox, rect);
        }

        // Edit box2
        HWND pEditBox2 = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_EDIT2);
        if (pEditBox2 != NULL)
        {
            GetWindowRect(pEditBox2, &rect);
            ScreenToClient(hDlg, rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            MoveWindow(pEditBox2, rect);
        }

        // Spin
        HWND pSpinControl = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SPIN);
        if (pSpinControl != NULL)
        {
            GetWindowRect(pSpinControl, &rect);
            ScreenToClient(hDlg, rect);
            int Height= (rect.bottom-rect.top);
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            MoveWindow(pSpinControl, rect);
        }

        //Slider
        HWND pSlider = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_SLIDER);
        if (pSlider != NULL)
        {
            GetWindowRect(pSlider, &rect);
            ScreenToClient(hDlg, rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            MoveWindow(pSlider, rect);
        }

        //Combo box
        HWND pChooseFromList = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
        if (pChooseFromList != NULL)
        {
            GetWindowRect(pChooseFromList, &rect);
            ScreenToClient(hDlg, rect);
            int Height= (rect.bottom-rect.top);
            if (DefaultBtnLeft > 10)
            {
                rect.right = DefaultBtnLeft - 10;
            }
            rect.top = (rect.top - rcBoxOrg.top) + rcBox.top;
            rect.bottom = rect.top + Height;
            MoveWindow(pChooseFromList, rect);
        }
    }


    HWND pList = GetDlgItem(hDlg, IDC_TREESETTINGS_GENERIC_LIST);
    if (pList != NULL)
    {
        GetWindowRect(pList, &rect);
        ScreenToClient(hDlg, rect);
        rect.right = cx;
        if (ValueBoxTop > rect.top)
        {
           rect.bottom = ValueBoxTop;
        }
        MoveWindow(pList, rect);
    }
    InvalidateRect(hDlg, NULL, TRUE);

}
