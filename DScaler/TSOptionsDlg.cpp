/////////////////////////////////////////////////////////////////////////////
// $Id: TSOptionsDlg.cpp,v 1.13 2003-10-27 10:39:54 adcockj Exp $
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
// Revision 1.12  2003/09/13 13:53:25  laurentg
// half height mode removed from the options dialog box
//
// Revision 1.11  2003/08/15 17:18:36  laurentg
// Factorize treatments
//
// Revision 1.10  2003/08/15 16:51:11  laurentg
// New event type EVENT_NO_VOLUME
// Update the volume toolbar when exiting from the audio mixer setup dialog box
//
// Revision 1.9  2003/07/08 21:04:59  laurentg
// New timeshift mode (full height) - experimental
//
// Revision 1.8  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.7  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.6  2001/11/20 11:43:00  temperton
// Store wave-device names instead of indexes in ini
//
// Revision 1.5  2001/08/06 03:00:17  ericschmidt
// solidified auto-pixel-width detection
// preliminary pausing-of-live-tv work
//
// Revision 1.4  2001/07/26 15:28:14  ericschmidt
// Added AVI height control, i.e. even/odd/averaged lines.
// Used existing cpu/mmx detection in CTimeShift:: code.
//
// Revision 1.3  2001/07/24 12:25:49  adcockj
// Added copyright notice as per standards
//
// Revision 1.2  2001/07/24 12:24:25  adcockj
// Added Id to comment block
//
// Revision 1.1  2001/07/23 20:52:07  ericschmidt
// Added CTimeShift:: class.  Original Release.  Got record and playback code working.
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file TSOptionsDlg.cpp CTSOptionsDlg Implementation
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScaler.h"
#include "TSOptionsDlg.h"
#include "TimeShift.h"
#include "MixerDev.h"

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
    m_RecHeight = TS_HALFHEIGHTEVEN;
}


void CTSOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTSOptionsDlg)
	DDX_Control(pDX, IDC_TSWAVEOUTCOMBO, m_WaveOutComboBox);
	DDX_Control(pDX, IDC_TSWAVEINCOMBO, m_WaveInComboBox);
	//}}AFX_DATA_MAP
    if (pDX->m_bSaveAndValidate)
    {
        if (IsChecked(IDC_TSFULLHEIGHTRADIO))
            m_RecHeight = TS_FULLHEIGHT;
        else if (IsChecked(IDC_TSHALFEVENRADIO))
            m_RecHeight = TS_HALFHEIGHTEVEN;
        else if (IsChecked(IDC_TSHALFODDRADIO))
            m_RecHeight = TS_HALFHEIGHTODD;
        else if (IsChecked(IDC_TSHALFAVERAGEDRADIO))
            m_RecHeight = TS_HALFHEIGHTAVG;
    }
    else
    {
        switch (m_RecHeight)
        {
        case TS_FULLHEIGHT:
            SetChecked(IDC_TSFULLHEIGHTRADIO, TRUE);
            SetChecked(IDC_TSHALFEVENRADIO, FALSE);
            SetChecked(IDC_TSHALFODDRADIO, FALSE);
            SetChecked(IDC_TSHALFAVERAGEDRADIO, FALSE);
            break;

        default:
        case TS_HALFHEIGHTEVEN:
            SetChecked(IDC_TSHALFEVENRADIO, TRUE);
            SetChecked(IDC_TSFULLHEIGHTRADIO, FALSE);
            SetChecked(IDC_TSHALFODDRADIO, FALSE);
            SetChecked(IDC_TSHALFAVERAGEDRADIO, FALSE);
            break;

        case TS_HALFHEIGHTODD:
            SetChecked(IDC_TSHALFODDRADIO, TRUE);
            SetChecked(IDC_TSFULLHEIGHTRADIO, FALSE);
            SetChecked(IDC_TSHALFEVENRADIO, FALSE);
            SetChecked(IDC_TSHALFAVERAGEDRADIO, FALSE);
            break;

        case TS_HALFHEIGHTAVG:
            SetChecked(IDC_TSHALFAVERAGEDRADIO, TRUE);
            SetChecked(IDC_TSFULLHEIGHTRADIO, FALSE);
            SetChecked(IDC_TSHALFEVENRADIO, FALSE);
            SetChecked(IDC_TSHALFODDRADIO, FALSE);
            break;
        }
    }
}


BEGIN_MESSAGE_MAP(CTSOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CTSOptionsDlg)
	ON_BN_CLICKED(IDC_TSCOMPRESSIONBUTTON, OnButtonCompression)
	ON_BN_CLICKED(IDOK, OnButtonOK)
	ON_BN_CLICKED(IDC_TSCOMPRESSIONHELP, OnCompressionhelp)
	ON_BN_CLICKED(IDC_TSWAVEHELP, OnWavehelp)
	ON_BN_CLICKED(IDC_TSHEIGHTHELP, OnHeighthelp)
	ON_BN_CLICKED(IDC_TSMIXERHELP, OnMixerhelp)
	ON_BN_CLICKED(IDC_TSMIXERBUTTON, OnButtonMixer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CTSOptionsDlg::IsChecked(int id)
{
    HWND hwnd = NULL;
    GetDlgItem(id, &hwnd);
    if (hwnd)
        return BST_CHECKED & (int)::SendMessage(hwnd, BM_GETCHECK, 0, 0);

    return false;
}

void CTSOptionsDlg::SetChecked(int id, BOOL checked)
{
    HWND hwnd = NULL;
    GetDlgItem(id, &hwnd);
    if (hwnd)
        ::SendMessage(hwnd,
                      BM_SETCHECK,
                      checked ? BST_CHECKED : BST_UNCHECKED,
                      0);
}

void CTSOptionsDlg::EnableCtrl(int id, BOOL enable)
{
    HWND hwnd = NULL;
    GetDlgItem(id, &hwnd);
    if (hwnd)
        ::EnableWindow(hwnd, enable);
}


/////////////////////////////////////////////////////////////////////////////
// CTSOptionsDlg message handlers

void CTSOptionsDlg::OnButtonCompression() 
{
    // Only popup the dialog if it's not up already, otherwise it'll crash.
    static bool options = false;
    if (!options)
    {
        options = true;
        CTimeShift::OnCompressionOptions();
        options = false;
    }
}

void CTSOptionsDlg::OnButtonMixer() 
{
    // Bring up the audio mixer setup dialog.
    Mixer_SetupDlg(m_hWnd);
}

void CTSOptionsDlg::OnButtonOK() 
{
    if (UpdateData(TRUE))
    {
        char name[MAXPNAMELEN];
        if(m_WaveInComboBox.GetLBText(m_WaveInComboBox.GetCurSel(), (char*)&name)!=CB_ERR)
        {
            CTimeShift::OnSetWaveInDevice((char*) &name);
        }

        if(m_WaveOutComboBox.GetLBText(m_WaveOutComboBox.GetCurSel(), (char*)&name)!=CB_ERR)
        {
            CTimeShift::OnSetWaveOutDevice((char*) &name);
        }

        CTimeShift::OnSetRecHeight(m_RecHeight);

        CDialog::OnOK();
    }
}

BOOL CTSOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    // Should've already created the timeshift object elsehere.
    ASSERT(CTimeShift::m_pTimeShift != NULL);

    int index = 0;
    char* waveInDevice;
    if(!CTimeShift::OnGetWaveInDevice(&waveInDevice))
        waveInDevice = NULL;

    UINT numDevs = waveInGetNumDevs();
    for (UINT i = 0; i < numDevs; ++i)
    {        
        WAVEINCAPS caps;
        memset(&caps, 0, sizeof(caps));
        waveInGetDevCaps(i, &caps, sizeof(caps));

        if(waveInDevice && !lstrcmp(caps.szPname, waveInDevice))
        {
            index = i;
        }

        // If getdevcaps fails, this'll just put a blank line in the control.
        m_WaveInComboBox.AddString(caps.szPname);
    }

    m_WaveInComboBox.SetCurSel(index);

    index = 0;
    char* waveOutDevice;
    if(!CTimeShift::OnGetWaveOutDevice(&waveOutDevice))
        waveOutDevice = NULL;

    numDevs = waveOutGetNumDevs();
    for (i = 0; i < numDevs; ++i)
    {        
        WAVEOUTCAPS caps;
        memset(&caps, 0, sizeof(caps));
        waveOutGetDevCaps(i, &caps, sizeof(caps));

        if(waveOutDevice && !lstrcmp(caps.szPname, waveOutDevice))
        {
            index = i;
        }

        // If getdevcaps fails, this'll just put a blank line in the control.
        m_WaveOutComboBox.AddString(caps.szPname);
    }

    m_WaveOutComboBox.SetCurSel(index);

    index;
    if (CTimeShift::OnGetRecHeight(&index)) // Leave as default if fails.
        m_RecHeight = index;

    // Refresh the controls on the dialog with the current data.
    UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTSOptionsDlg::OnCompressionhelp() 
{
    MessageBox("Choose a codec that's good at compressing 24-bit RGB images.  "
               "The default is full frames and you'll quickly run out of room "
               "if you stick with that.  Leave the audio at the default for "
               "best results.",
               "Compression Help",
               MB_OK);
}

void CTSOptionsDlg::OnWavehelp() 
{
    MessageBox("Choose the device to which your tuner card is directly "
               "attached.  (i.e. via an internal stereo patch cable.)  But "
               "feel free to try all your devices if you don't get audio "
               "recording/playback from the AVI.",
               "Wave Device Help",
               MB_OK);
}

void CTSOptionsDlg::OnHeighthelp() 
{
    MessageBox("Using Full-height will record 100% of the image data but it's "
               "the slowest.  Using 1/2-height Even or Odd will throw out 1/2 "
               "the available image data, but these are the fastest.  "
               "Averaging will help to remove the pixelated look of 1/2 height "
               "recording, and it's only slightly slower than using only "
               "the Even or Odd lines.\n\n"
               "NOTE: This only applys to straight recording.  During time "
               "shifting operations (i.e. pausing of live TV), 1/2-height Even "
               "will be used.",
               "Recording Height Help",
               MB_OK);
}

void CTSOptionsDlg::OnMixerhelp() 
{
    MessageBox("The audio mixer is used during playback to mute the input from "
               "your tuner card, yet still receive audio from it for recording "
               "during time shifting operations (i.e. pausing of live TV).  "
               "You do not need to have the 'Use Mixer' checkbox checked, but "
               "you do need to have all of the combo boxes below it set to "
               "the correct input in order for playback of audio to function "
               "correctly.",
               "Audio-Mixer Setup Help",
               MB_OK);
}
