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
 * @file TreeSettingsGeneric.h Header file for CTreeSettingsGeneric class
 */

#if !defined(_TREESETTINGSGENERIC_H__)
#define _TREESETTINGSGENERIC_H__

#include "Setting.h"
#include "DS_ApiCommon.h"
#include "TreeSettingsPage.h"
#include "Settings.h"
#include "..\DScalerRes\resource.h"

class CSimpleSetting;
/**
 * Generic settings page for tree based settings dialog.
 * This dialog is almost the same as CSettingsDlg but is modified to work as
 * a page is CTreeSettingsDlg
 *
 * @see CHSListBox
 * @see CTreeSettingsDlg
 */
class CTreeSettingsGeneric : public CTreeSettingsPage
{
// Construction
public:

    /**
     * @param name name used in the tree
     * @param SettingHolder Settings to display in Dialog
     */
    CTreeSettingsGeneric(const tstring& name,SmartPtr<CSettingsHolder> SettingHolder);
    ~CTreeSettingsGeneric() {};

public:
    virtual void OnOK(HWND hDlg);

private:
    virtual BOOL CTreeSettingsGeneric::ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    LRESULT OnNotify(HWND hwnd, int id, LPNMHDR nmhdr);
    void OnSelchangeList(HWND hDlg);
    void OnChangeEdit(HWND hDlg);
    void OnChangeEditString(HWND hDlg);
    void OnHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
    void OnSettingsDefault(HWND hDlg);
    void OnCheckClick(HWND hDlg);
    void OnSelchangeChoosefromlist(HWND hDlg);
    LRESULT OnDeltaposSettingsSpin(HWND hDlg, NMHDR* pNMHDR);
    void OnCheckGlobalClick(HWND hDlg);
    void OnCheckSourceClick(HWND hDlg);
    void OnCheckVideoInputClick(HWND hDlg);
    void OnCheckAudioInputClick(HWND hDlg);
    void OnCheckVideoFormatClick(HWND hDlg);
    void OnCheckChannelClick(HWND hDlg);
    void OnSize(HWND hDlg, UINT nType, int cx, int cy);
    void UpdateControls(HWND pChangedControl);
    long m_SettingsCount;
    SmartPtr<CSettingsHolder> m_Settings;
    long m_CurrentSetting;

    // control window handles
    HWND m_Combo;
    HWND m_CheckBox;
    HWND m_Spin;
    HWND m_Slider;
    HWND m_Edit;
    HWND m_EditString;
    HWND m_DefaultButton;
    HWND m_ListBox;
    HWND m_CheckGlobal;
    HWND m_CheckGlobalBox;
    HWND m_CheckSourceBox;
    HWND m_CheckVideoInputBox;
    HWND m_CheckAudioInputBox;
    HWND m_CheckVideoFormatBox;
    HWND m_CheckChannelBox;
    HWND m_SavePerInfoBox;
    HWND m_TopGroupBox;
};

#endif
