/////////////////////////////////////////////////////////////////////////////
// $Id: TSOptionsDlg.cpp,v 1.3 2001-07-24 12:25:49 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2001   Eric Schmidt          Original Release.
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2001/07/24 12:24:25  adcockj
// Added Id to comment block
//
// Revision 1.1  2001/07/23 20:52:07  ericschmidt
// Added TimeShift class.  Original Release.  Got record and playback code working.
//
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DScaler.h"
#include "TSOptionsDlg.h"
#include "TimeShift.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTSOptionsDlg dialog


CTSOptionsDlg::CTSOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTSOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTSOptionsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTSOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTSOptionsDlg)
	DDX_Control(pDX, IDC_TSWAVEOUTCOMBO, m_WaveOutComboBox);
	DDX_Control(pDX, IDC_TSWAVEINCOMBO, m_WaveInComboBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTSOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CTSOptionsDlg)
	ON_BN_CLICKED(IDC_TSCOMPRESSIONBUTTON, OnButtonCompression)
	ON_BN_CLICKED(IDOK, OnButtonOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTSOptionsDlg message handlers

void CTSOptionsDlg::OnButtonCompression() 
{
    // Only popup the dialog if it's not up already, otherwise it'll crash.
    static bool options = false;
    if (!options)
    {
        options = true;
        TimeShift::OnCompressionOptions();
        options = false;
    }
}

void CTSOptionsDlg::OnButtonOK() 
{
    int index = m_WaveInComboBox.GetCurSel();
    TimeShift::OnSetWaveInDevice(index != CB_ERR ? index : 0);

    index = m_WaveOutComboBox.GetCurSel();
    TimeShift::OnSetWaveOutDevice(index != CB_ERR ? index : 0);

	CDialog::OnOK();
}

BOOL CTSOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    // Should've already created the timeshift object elsehere.
    ASSERT(TimeShift::m_pTimeShift != NULL);

    UINT numDevs = waveInGetNumDevs();
    for (UINT i = 0; i < numDevs; ++i)
    {        
        WAVEINCAPS caps;
        memset(&caps, 0, sizeof(caps));
        waveInGetDevCaps(i, &caps, sizeof(caps));

        // If getdevcaps fails, this'll just put a blank line in the control.
        m_WaveInComboBox.AddString(caps.szPname);
    }

    int index;
    if (!TimeShift::OnGetWaveInDevice(&index))
        index = 0;
    m_WaveInComboBox.SetCurSel(index);

    numDevs = waveOutGetNumDevs();
    for (i = 0; i < numDevs; ++i)
    {        
        WAVEOUTCAPS caps;
        memset(&caps, 0, sizeof(caps));
        waveOutGetDevCaps(i, &caps, sizeof(caps));

        // If getdevcaps fails, this'll just put a blank line in the control.
        m_WaveOutComboBox.AddString(caps.szPname);
    }

    index;
    if (!TimeShift::OnGetWaveOutDevice(&index))
        index = 0;
    m_WaveOutComboBox.SetCurSel(index);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
