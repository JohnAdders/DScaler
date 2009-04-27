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

/**
 * @file DSAudioDevicePage.cpp implementation file
 */

#include "stdafx.h"

#ifdef WANT_DSHOW_SUPPORT

#include "..\dscaler.h"
#include "DSAudioDevicePage.h"
#include "DevEnum.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CDSAudioDevicePage dialog


CDSAudioDevicePage::CDSAudioDevicePage(const string& name, SmartPtr<CStringSetting> AudioDeviceSetting) :
    CTreeSettingsPage(name, IDD_DSHOW_AUDIODEVICE),
    m_AudioDeviceSetting(AudioDeviceSetting),
    m_bConnectAudio(NULL)
{
}

CDSAudioDevicePage::CDSAudioDevicePage(const string& name, SmartPtr<CStringSetting> AudioDeviceSetting, BOOL *bConnectAudio) :
    CTreeSettingsPage(name,IDD_DSHOW_AUDIODEVICE),
    m_AudioDeviceSetting(AudioDeviceSetting),
    m_bConnectAudio(bConnectAudio)
{
}

BOOL CDSAudioDevicePage::ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    default:
        return FALSE;
    }
}

void CDSAudioDevicePage::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_DSHOW_AUDIODEVICE_USEDEFAULT:
        if(BN_CLICKED == codeNotify)
        {
            OnClickedUseDefault(hDlg);
        }
        break;
    case IDC_DSHOW_AUDIODEVICE_RENDER:
        if(BN_CLICKED == codeNotify)
        {
            OnClickedConnectAudio(hDlg);
        }
        break;
    case IDC_DSHOW_AUDIODEVICE_DEVICE:
        if(CBN_SELENDOK == codeNotify)
        {
            OnSelEndOkAudioDevice(hDlg);
        }
        break;
    }
}

BOOL CDSAudioDevicePage::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    try
    {
        CDShowDevEnum devenum(CLSID_AudioRendererCategory);
        while(devenum.getNext()==TRUE)
        {
            string deviceName=devenum.getProperty("FriendlyName");
            int pos = ComboBox_AddString(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_DEVICE), deviceName.c_str());
            if(pos!=CB_ERR)
            {
                m_DeviceList.push_back(devenum.getDisplayName());
                ComboBox_SetItemData(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_DEVICE), pos, m_DeviceList.size()-1);
                if(m_AudioDeviceSetting->GetValue() == devenum.getDisplayName())
                {
                    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_DEVICE), pos);
                }
            }
        }
    }
    catch(CDShowDevEnumException&)
    {
    }

    if(m_AudioDeviceSetting->GetValue() == NULL)
    {
        Button_SetCheck(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_USEDEFAULT), BST_CHECKED);
        OnClickedUseDefault(hDlg);
    }

    if(ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_DEVICE))==CB_ERR && m_DeviceList.size()>0)
    {
        ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_DEVICE), 0);
    }

    if(m_bConnectAudio!=NULL)
    {
        Button_SetCheck(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_RENDER), *m_bConnectAudio?BST_UNCHECKED:BST_CHECKED);
        OnClickedConnectAudio(hDlg);
    }
    else
    {
        ShowWindow(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_RENDER), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_TEXT2), SW_HIDE);
    }

    return TRUE;
}

void CDSAudioDevicePage::OnOK(HWND hDlg)
{
    int pos = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_DEVICE));
    if(pos!=CB_ERR && Button_GetCheck(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_USEDEFAULT))==BST_UNCHECKED)
    {
        int index = ComboBox_GetItemData(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_DEVICE), pos);
        m_AudioDeviceSetting->SetValue(m_DeviceList[index].c_str());
    }
    else
    {
        m_AudioDeviceSetting->SetValue("");
    }
    if(m_bConnectAudio != NULL)
    {
        *m_bConnectAudio = (Button_GetCheck(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_RENDER)) == BST_UNCHECKED);
    }

    EndDialog(hDlg, IDOK);
}

void CDSAudioDevicePage::OnSelEndOkAudioDevice(HWND hDlg)
{

}

void CDSAudioDevicePage::OnClickedUseDefault(HWND hDlg)
{
    if(Button_GetCheck(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_USEDEFAULT))==BST_CHECKED)
    {
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE),FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE),TRUE);
    }
}

void CDSAudioDevicePage::OnClickedConnectAudio(HWND hDlg)
{
    if(Button_GetCheck(GetDlgItem(hDlg, IDC_DSHOW_AUDIODEVICE_RENDER))==BST_UNCHECKED)
    {
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE_TEXT),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_GRPBOX),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_USEDEFAULT),TRUE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE_LABEL),TRUE);
        OnClickedUseDefault(hDlg);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE_TEXT),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_GRPBOX),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_USEDEFAULT),FALSE);
        EnableWindow(GetDlgItem(hDlg,IDC_DSHOW_AUDIODEVICE_DEVICE_LABEL),FALSE);
    }
}

#endif
