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
 * @file DSAudioDevicePage.h header file
 */

#if !defined(_DSAUDIODEVICEPAGE_H__)
#define _DSAUDIODEVICEPAGE_H__

#include "TreeSettingsPage.h"
#include "..\DScalerRes\resource.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CDSAudioDevicePage dialog

class CDSAudioDevicePage : public CTreeSettingsPage
{
// Construction
public:
    CDSAudioDevicePage(const std::string& name, SmartPtr<CStringSetting> AudioDeviceSetting);   // standard constructor
    CDSAudioDevicePage(const std::string& name, SmartPtr<CStringSetting> AudioDeviceSetting, BOOL *bConnectAudio);
    ~CDSAudioDevicePage() {};

// Implementation
public:
    virtual void OnOK(HWND hDlg);

private:
    virtual BOOL ChildDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
    void OnSelEndOkAudioDevice(HWND hDlg);
    void OnClickedUseDefault(HWND hDlg);
    void OnClickedConnectAudio(HWND hDlg);

    std::vector<std::string> m_DeviceList;
    SmartPtr<CStringSetting> m_AudioDeviceSetting;
    BOOL *m_bConnectAudio;
};


#endif
