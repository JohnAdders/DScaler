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

#if !defined(AFX_DSAUDIODEVICEPAGE_H__06F4CD66_73E2_49F4_8B9A_0A86543633D3__INCLUDED_)
#define AFX_DSAUDIODEVICEPAGE_H__06F4CD66_73E2_49F4_8B9A_0A86543633D3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TreeSettingsPage.h"
#include "..\DScalerRes\resource.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CDSAudioDevicePage dialog

class CDSAudioDevicePage : public CTreeSettingsPage
{
// Construction
public:
    CDSAudioDevicePage(CString name,std::string &AudioDevice);   // standard constructor
    CDSAudioDevicePage(CString name,std::string &AudioDevice,BOOL *bConnectAudio);
    ~CDSAudioDevicePage() {};

// Dialog Data
    //{{AFX_DATA(CDSAudioDevicePage)
    enum { IDD = IDD_DSHOW_AUDIODEVICE };
    CComboBox m_AudioDevice;
    CButton m_UseDefault;
    CButton m_ConnectAudio;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDSAudioDevicePage)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CDSAudioDevicePage)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelEndOkAudioDevice();
    afx_msg void OnClickedUseDefault();
    afx_msg void OnClickedConnectAudio();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void OnOK();
private:
    std::vector<std::string> m_DeviceList;
    std::string &m_AudioDeviceSetting;
    BOOL *m_bConnectAudio;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DSAUDIODEVICEPAGE_H__06F4CD66_73E2_49F4_8B9A_0A86543633D3__INCLUDED_)
