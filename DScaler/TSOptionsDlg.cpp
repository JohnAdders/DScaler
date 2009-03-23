/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Eric Schmidt.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
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
#include "TSCompressionDlg.h"
#include "TimeShift.h"
#include "MixerDev.h"
#include <shlobj.h>

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
    //}}AFX_DATA_INIT
}

int CTSOptionsDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CDialog::OnCreate(lpCreateStruct)==-1)
       return -1;

    if (FAILED(CoInitialize(NULL)))
       return -1;

    return 0;
}

void CTSOptionsDlg::OnDestroy() 
{
    CDialog::OnDestroy();
    CoUninitialize();
}

void CTSOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    //{{AFX_DATA_MAP(CTSOptionsDlg)
    DDX_Control(pDX, IDC_TS_PATH_DISPLAY, m_PathDisplay);
    DDX_Control(pDX, IDC_TS_SIZE_GIB, m_SizeGiB);
    DDX_Control(pDX, IDC_TS_SIZE_NO_LIMIT, m_SizeCheckBox);
    DDX_Control(pDX, IDC_TS_SIZE, m_SizeEdit);
    DDX_Control(pDX, IDC_TSWAVEOUTCOMBO, m_WaveOutComboBox);
    DDX_Control(pDX, IDC_TSWAVEINCOMBO, m_WaveInComboBox);
    //}}AFX_DATA_MAP

    if (!pDX->m_bSaveAndValidate)
    {
        /* Recording height radio buttons */
        switch (options.recHeight)
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

        /* Recording format radio buttons */
        SetChecked(IDC_TSYUY2RADIO, options.format==FORMAT_YUY2 ? TRUE : FALSE);
        SetChecked(IDC_TSRGBRADIO, options.format==FORMAT_YUY2 ? FALSE : TRUE);

        /* Path */
        m_PathDisplay.SetWindowText(options.path);

        /* File size limit */
        SetDlgItemInt(IDC_TS_SIZE, options.sizeLimit, FALSE);
        OnUpdateTSSize();
    } else
    {
        /* Recording height */
        if (IsChecked(IDC_TSFULLHEIGHTRADIO))
           options.recHeight = TS_FULLHEIGHT;
           else if (IsChecked(IDC_TSHALFEVENRADIO))
                   options.recHeight = TS_HALFHEIGHTEVEN;
           else if (IsChecked(IDC_TSHALFODDRADIO))
                   options.recHeight = TS_HALFHEIGHTODD;
           else if (IsChecked(IDC_TSHALFAVERAGEDRADIO))
                   options.recHeight = TS_HALFHEIGHTAVG;

        /* Recording format */
        if (IsChecked(IDC_TSYUY2RADIO))
           options.format = FORMAT_YUY2;
           else
           options.format = FORMAT_RGB;

        /* File size */
        options.sizeLimit = GetDlgItemInt(IDC_TS_SIZE, NULL, FALSE);
    }
}

BEGIN_MESSAGE_MAP(CTSOptionsDlg, CDialog)
    //{{AFX_MSG_MAP(CTSOptionsDlg)
    ON_BN_CLICKED(IDC_TSCOMPRESSIONBUTTON, OnButtonCompression)
    ON_BN_CLICKED(IDC_TSCOMPRESSIONHELP, OnCompressionhelp)
    ON_BN_CLICKED(IDC_TSWAVEHELP, OnWavehelp)
    ON_BN_CLICKED(IDC_TSHEIGHTHELP, OnHeighthelp)
    ON_BN_CLICKED(IDC_TSMIXERBUTTON, OnButtonMixer)
    ON_BN_CLICKED(IDC_TS_PATH_SELECT, OnTSPathSelect)
    ON_WM_CREATE()
    ON_WM_DESTROY()
    ON_EN_UPDATE(IDC_TS_SIZE, OnUpdateTSSize)
    ON_EN_KILLFOCUS(IDC_TS_SIZE, OnKillfocusTSSize)
    ON_BN_CLICKED(IDC_TS_SIZE_NO_LIMIT, OnTSSizeNoLimit)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CTSOptionsDlg::IsChecked(int id)
{
    HWND hwnd = NULL;
    GetDlgItem(id, &hwnd);
    if (hwnd)
        return BST_CHECKED & (int)::SendMessage(hwnd, BM_GETCHECK, 0, 0);

    return FALSE;
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

/////////////////////////////////////////////////////////////////////////////
// CTSOptionsDlg message handlers

void CTSOptionsDlg::OnButtonCompression() 
{
    if (UpdateData(TRUE))
    {
        CTSCompressionDlg dlg(CWnd::FromHandle(m_hWnd), &options);

        dlg.DoModal();
    }
}

void CTSOptionsDlg::OnButtonMixer()
{
    // Bring up the audio mixer setup dialog.
    Mixer_SetupDlg(m_hWnd);
}

void CTSOptionsDlg::OnOK()
{
    char name[MAXPNAMELEN];

    /* Update the options that the user has selected in the dialog */
    if (UpdateData(TRUE))
    {
        /* Wave devices */
        if (m_WaveInComboBox.GetLBText(m_WaveInComboBox.GetCurSel(), name) != CB_ERR)
           TimeShiftSetWaveInDevice(name);

        if (m_WaveOutComboBox.GetLBText(m_WaveOutComboBox.GetCurSel(), name)!=CB_ERR)
           TimeShiftSetWaveOutDevice(name);

        TimeShiftSetRecHeight(options.recHeight);
        TimeShiftSetRecFormat(options.format);
        TimeShiftSetSavingPath(options.path);
        TimeShiftSetFileSizeLimit(options.sizeLimit);
        TimeShiftSetFourCC(options.fcc);

        CDialog::OnOK();
    }
}

BOOL CTSOptionsDlg::OnInitDialog() 
{
    char       *waveDevice;
    UINT       numDevs;
    UINT       i;
    int        index;
    const char *tsPath = TimeShiftGetSavingPath();

    CDialog::OnInitDialog();

    /* Initialize the options */
    memset(&options, 0, sizeof(options));

    options.recHeight = TS_HALFHEIGHTEVEN;
    options.format    = FORMAT_YUY2;

    /**** Wave devices ****/
    if (!TimeShiftGetWaveInDevice(&waveDevice))
       waveDevice = NULL;

    numDevs = waveInGetNumDevs();
    index   = 0;
    for (i = 0; i < numDevs; i++)
    {        
        WAVEINCAPS caps;

        memset(&caps, 0, sizeof(caps));
        waveInGetDevCaps(i, &caps, sizeof(caps));

        if (waveDevice && !lstrcmp(caps.szPname, waveDevice))
           index = i;

        // If getdevcaps fails, this'll just put a blank line in the control.
        m_WaveInComboBox.AddString(caps.szPname);
    }

    m_WaveInComboBox.SetCurSel(index);

    if (!TimeShiftGetWaveOutDevice(&waveDevice))
       waveDevice = NULL;

    numDevs = waveOutGetNumDevs();
    index   = 0;
    for (i = 0; i < numDevs; i++)
    {        
        WAVEOUTCAPS caps;

        memset(&caps, 0, sizeof(caps));
        waveOutGetDevCaps(i, &caps, sizeof(caps));

        if (waveDevice && !lstrcmp(caps.szPname, waveDevice))
           index = i;

        // If getdevcaps fails, this'll just put a blank line in the control.
        m_WaveOutComboBox.AddString(caps.szPname);
    }

    m_WaveOutComboBox.SetCurSel(index);

    /**** Recording format related ****/
    if (TimeShiftGetRecHeight(&index))
       options.recHeight = index;

    TimeShiftGetRecFormat(&options.format);
    TimeShiftGetFourCC(&options.fcc);

    /**** File path and limit ****/
    strncpy(options.path, tsPath ? tsPath : TS_DEFAULT_PATH,
            sizeof(options.path));

    options.sizeLimit = TimeShiftGetFileSizeLimit();
    m_lastSize        = options.sizeLimit;

    /* Limit the edit control to only accept 8 numbers */
    m_SizeEdit.LimitText(8);

    // Refresh the controls on the dialog with the current data.
    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

int CALLBACK browseCallback(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    LPITEMIDLIST list;
    char         path[MAX_PATH + 1];

    switch (uMsg)
    {
        case BFFM_INITIALIZED:
            if (lpData && strlen((const char *)lpData) > 0)
               ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE,
                             (LPARAM)(const char *)lpData);
        break;

        case BFFM_SELCHANGED:
            /* Either enable or disable the OK button depending if the
               currently selected path is valid or not */
            list = (LPITEMIDLIST)lParam;

            if (SHGetPathFromIDList(list, path))
               ::SendMessage(hWnd, BFFM_ENABLEOK, 0,
                             TimeShiftIsPathValid(path) ? 1 : 0);
               else
               ::SendMessage(hWnd, BFFM_ENABLEOK, 0, 0);
        break;
    }

    return 0;
}

/** Path select button pressed */
void CTSOptionsDlg::OnTSPathSelect() 
{
    BROWSEINFO   info;
    LPITEMIDLIST list;
    char         path[MAX_PATH + 1];
    LPMALLOC     pMalloc;
    DWORD        maxSize;

    /* Get the path that's currently being used and pass it to the callback
       function so it can select it after the dialog initializes */
    strncpy(path, TimeShiftGetSavingPath(), sizeof(path));
    if (!TimeShiftIsPathValid(path))
       memset(path, 0, sizeof(path));

    memset(&info, 0, sizeof(info));
    info.hwndOwner = m_hWnd;
    info.lpfn      = browseCallback;
    info.ulFlags   = BIF_NEWDIALOGSTYLE;
    info.lParam    = (LPARAM)path;

    list = SHBrowseForFolder(&info);
    if (list)
    {
        if (!SHGetPathFromIDList(list, path) || !TimeShiftIsPathValid(path))
           MessageBox("Invalid path selected", "Error", MB_OK | MB_ICONERROR);
           else
           {
               strncpy(options.path, path, sizeof(options.path));

               maxSize = GetMaximumVolumeFileSize(options.path);
               if (maxSize)
               {
                   CString text;

                   text.Format("The path you have selected is on a volume "
                               "that can't store files that are larger than "
                               "about %0.2f GiB. Your recorded files will be "
                               "limited to this size even if the maximum size "
                               "is set higher.",
                               (float)maxSize / (float)(1 << 10));
                   MessageBox(text, "Limited Volume", MB_ICONWARNING | MB_OK);
               }
           }

        if (SHGetMalloc(&pMalloc) != E_FAIL)
        {
            pMalloc->Free(list);
            pMalloc->Release();
        } else
          MessageBox("Memory leak", "Error", MB_OK | MB_ICONERROR);

        m_PathDisplay.SetWindowText(options.path);
    }
}

void CTSOptionsDlg::OnCompressionhelp() 
{
    MessageBox("Configure the audio and video compression settings. "
               "The default is uncompressed frames, which should be "
               "changed if the recording format is RGB. Try using a "
               "fast codec for best results. Huffyuv is recommended.",
               "Compression Tips",
               MB_OK);
}

void CTSOptionsDlg::OnWavehelp() 
{
    MessageBox("WaveIn: Choose the device to which your tuner card is "
               "directly attached.",
               "Wave Device Help",
               MB_OK);
}

void CTSOptionsDlg::OnHeighthelp() 
{
    MessageBox("Using full height will record every interlaced frame assuming "
               "none are lost. It's recommended that you use a fast, "
               "lightweight lossless codec like Huffyuv in this mode. "
               "Recording at half the height will reduce the vertical "
               "resolution, but it's faster than recording at full height and "
               "requires much less disk space.",
               "Recording Height Help",
               MB_OK);
}

void CTSOptionsDlg::OnUpdateTSSize() 
{
    DWORD   value;
    CString text;

    /* The file size edit control has been changed */
    value = GetDlgItemInt(IDC_TS_SIZE, NULL, FALSE);

    /* Enable or disable the "no limit" check box */
    if (!value)
    {
        if (m_SizeCheckBox.GetCheck() != BST_CHECKED)
           m_SizeCheckBox.SetCheck(BST_CHECKED);
    } else
    {
        if (m_SizeCheckBox.GetCheck() != BST_UNCHECKED)
           m_SizeCheckBox.SetCheck(BST_UNCHECKED);
    }

    /* Update the size in GiB */
    text.Format("(%0.2f GiB)", (float)value / (float)(1 << 10));
    m_SizeGiB.SetWindowText(text);
}

void CTSOptionsDlg::OnKillfocusTSSize() 
{
    DWORD value;

    /* Make sure something's being displayed in the edit control to reduce
       confusion */
    if (m_SizeEdit.GetWindowTextLength() <= 0)
       SetDlgItemInt(IDC_TS_SIZE, 0, FALSE);
       else
       {
           /* Validate the data. Make sure the value is in the range of valid
              values. */
           value = GetDlgItemInt(IDC_TS_SIZE, NULL, FALSE);
           if (value > MAX_FILE_SIZE)
              value = MAX_FILE_SIZE;

           /* This will set the value back into the edit box. Weird formatting
              like leading zeroes will also be removed after the value is
              set. */
           SetDlgItemInt(IDC_TS_SIZE, value, FALSE);

           /* Save the value as the last size if it's not zero */
           if (value)
              m_lastSize = value;
       }
}

void CTSOptionsDlg::OnTSSizeNoLimit() 
{
    if (m_SizeCheckBox.GetCheck()==BST_CHECKED)
       SetDlgItemInt(IDC_TS_SIZE, 0, FALSE);
       else
       SetDlgItemInt(IDC_TS_SIZE, m_lastSize ? m_lastSize :
                                               (MAX_FILE_SIZE >> 1), FALSE);
}
