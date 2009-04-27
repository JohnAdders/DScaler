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

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CTSOptionsDlg dialog


CTSOptionsDlg::CTSOptionsDlg() :
    CDSDialog(MAKEINTRESOURCE(IDD_TSOPTIONS))
{
}

BOOL CTSOptionsDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    default:
        return FALSE;
    }
}

void CTSOptionsDlg::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_TS_SIZE_NO_LIMIT:
        if(BN_CLICKED == codeNotify)
        {
            OnTSSizeNoLimit(hDlg);
        }
        break;
    case IDC_TS_PATH_SELECT:
        if(BN_CLICKED == codeNotify)
        {
            OnTSPathSelect(hDlg);
        }
        break;
    case IDC_TSCOMPRESSIONBUTTON:
        if(BN_CLICKED == codeNotify)
        {
            OnButtonCompression(hDlg);
        }
        break;
    case IDC_TSCOMPRESSIONHELP:
        if(BN_CLICKED == codeNotify)
        {
            OnCompressionhelp(hDlg);
        }
        break;
    case IDC_TSWAVEHELP:
        if(BN_CLICKED == codeNotify)
        {
            OnWavehelp(hDlg);
        }
        break;
    case IDC_TSHEIGHTHELP:
        if(BN_CLICKED == codeNotify)
        {
            OnHeighthelp(hDlg);
        }
        break;
    case IDOK:
        if(BN_CLICKED == codeNotify)
        {
            OnOK(hDlg);
        }
        break;
    case IDCANCEL:
        if(BN_CLICKED == codeNotify)
        {
            EndDialog(hDlg, IDCANCEL);
        }
        break;
    case IDC_TSMIXERBUTTON:
        if(BN_CLICKED == codeNotify)
        {
            OnButtonMixer(hDlg);
        }
        break;
    case IDC_TS_SIZE:
        if(EN_UPDATE == codeNotify)
        {
            OnUpdateTSSize(hDlg);
        }
        if(EN_KILLFOCUS == codeNotify)
        {
            OnKillfocusTSSize(hDlg);
        }
        break;
    }
}

BOOL CTSOptionsDlg::UpdateData(HWND hDlg, BOOL bSaveAndValidate)
{
    if (!bSaveAndValidate)
    {
        /* Recording height radio buttons */
        switch (options.recHeight)
        {
        case TS_FULLHEIGHT:
            SetChecked(hDlg, IDC_TSFULLHEIGHTRADIO, TRUE);
            SetChecked(hDlg, IDC_TSHALFEVENRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFODDRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFAVERAGEDRADIO, FALSE);
            break;

        default:
        case TS_HALFHEIGHTEVEN:
            SetChecked(hDlg, IDC_TSHALFEVENRADIO, TRUE);
            SetChecked(hDlg, IDC_TSFULLHEIGHTRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFODDRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFAVERAGEDRADIO, FALSE);
            break;

        case TS_HALFHEIGHTODD:
            SetChecked(hDlg, IDC_TSHALFODDRADIO, TRUE);
            SetChecked(hDlg, IDC_TSFULLHEIGHTRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFEVENRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFAVERAGEDRADIO, FALSE);
            break;

        case TS_HALFHEIGHTAVG:
            SetChecked(hDlg, IDC_TSHALFAVERAGEDRADIO, TRUE);
            SetChecked(hDlg, IDC_TSFULLHEIGHTRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFEVENRADIO, FALSE);
            SetChecked(hDlg, IDC_TSHALFODDRADIO, FALSE);
            break;
        }

        /* Recording format radio buttons */
        SetChecked(hDlg, IDC_TSYUY2RADIO, options.format==FORMAT_YUY2 ? TRUE : FALSE);
        SetChecked(hDlg, IDC_TSRGBRADIO, options.format==FORMAT_YUY2 ? FALSE : TRUE);

        /* Path */
        Edit_SetText(GetDlgItem(hDlg, IDC_TS_PATH_DISPLAY), options.path);

        /* File size limit */
        SetDlgItemInt(hDlg, IDC_TS_SIZE, options.sizeLimit);
        OnUpdateTSSize(hDlg);
    }
    else
    {
        /* Recording height */
        if (IsChecked(hDlg, IDC_TSFULLHEIGHTRADIO))
        {
           options.recHeight = TS_FULLHEIGHT;
        }
        else if (IsChecked(hDlg, IDC_TSHALFEVENRADIO))
        {
           options.recHeight = TS_HALFHEIGHTEVEN;
        }
        else if (IsChecked(hDlg, IDC_TSHALFODDRADIO))
        {
            options.recHeight = TS_HALFHEIGHTODD;
        }
        else if (IsChecked(hDlg, IDC_TSHALFAVERAGEDRADIO))
        {
            options.recHeight = TS_HALFHEIGHTAVG;
        }

        /* Recording format */
        if (IsChecked(hDlg, IDC_TSYUY2RADIO))
        {
           options.format = FORMAT_YUY2;
        }
        else
        {
           options.format = FORMAT_RGB;
        }

        /* File size */
        options.sizeLimit = GetDlgItemInt(hDlg, IDC_TS_SIZE);
    }
    return TRUE;
}


BOOL CTSOptionsDlg::IsChecked(HWND hDlg, int id)
{
    HWND hwnd = GetDlgItem(hDlg, id);
    if (hwnd)
    {
        return BST_CHECKED & (int)SendMessage(hwnd, BM_GETCHECK, 0, 0);
    }

    return FALSE;
}

void CTSOptionsDlg::SetChecked(HWND hDlg, int id, BOOL checked)
{
    HWND hwnd = GetDlgItem(hDlg, id);
    if (hwnd)
    {
        SendMessage(hwnd,
                      BM_SETCHECK,
                      checked ? BST_CHECKED : BST_UNCHECKED,
                      0);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CTSOptionsDlg message handlers

void CTSOptionsDlg::OnButtonCompression(HWND hDlg)
{
    if(UpdateData(hDlg, TRUE))
    {
        CTSCompressionDlg dlg(&options);

        dlg.DoModal(hDlg);
    }
}

void CTSOptionsDlg::OnButtonMixer(HWND hDlg)
{
    // Bring up the audio mixer setup dialog.
    Mixer_SetupDlg(hDlg);
}

int GetComboBoxString(HWND hDlg, int ComboId, string& ComboString)
{
    HWND hComboBox = GetDlgItem(hDlg, ComboId);
    int CurSel(ComboBox_GetCurSel(hComboBox));
    int TextLen(ComboBox_GetLBTextLen(hComboBox, CurSel));
    vector<char> Buffer(TextLen + 1);
    int Result = ComboBox_GetLBText(hComboBox, CurSel, &Buffer[0]);
    ComboString = &Buffer[0];
    return Result;
}

void CTSOptionsDlg::OnOK(HWND hDlg)
{

    /* Update the options that the user has selected in the dialog */
    if (UpdateData(hDlg, TRUE))
    {
        /* Wave devices */
        string name;
        if (GetComboBoxString(hDlg, IDC_TSWAVEINCOMBO, name) != CB_ERR)
        {
            TimeShiftSetWaveInDevice(name.c_str());
        }

        if (GetComboBoxString(hDlg, IDC_TSWAVEOUTCOMBO, name) != CB_ERR)
        {
            TimeShiftSetWaveOutDevice(name.c_str());
        }

        TimeShiftSetRecHeight(options.recHeight);
        TimeShiftSetRecFormat(options.format);
        TimeShiftSetSavingPath(options.path);
        TimeShiftSetFileSizeLimit(options.sizeLimit);
        TimeShiftSetFourCC(options.fcc);
        EndDialog(hDlg, IDOK);
    }
}

BOOL CTSOptionsDlg::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    char       *waveDevice;
    UINT       numDevs;
    UINT       i;
    int        index;
    const char *tsPath = TimeShiftGetSavingPath();

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
        {
            index = i;
        }

        // If getdevcaps fails, this'll just put a blank line in the control.
        ComboBox_AddItemData(GetDlgItem(hDlg, IDC_TSWAVEINCOMBO), caps.szPname);
    }

    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TSWAVEINCOMBO), index);

    if (!TimeShiftGetWaveOutDevice(&waveDevice))
    {
        waveDevice = NULL;
    }

    numDevs = waveOutGetNumDevs();
    index   = 0;
    for (i = 0; i < numDevs; i++)
    {
        WAVEOUTCAPS caps;

        memset(&caps, 0, sizeof(caps));
        waveOutGetDevCaps(i, &caps, sizeof(caps));

        if (waveDevice && !lstrcmp(caps.szPname, waveDevice))
        {
            index = i;
        }

        // If getdevcaps fails, this'll just put a blank line in the control.
        ComboBox_AddItemData(GetDlgItem(hDlg, IDC_TSWAVEOUTCOMBO), caps.szPname);
    }

    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_TSWAVEOUTCOMBO), index);

    /**** Recording format related ****/
    if (TimeShiftGetRecHeight(&index))
    {
        options.recHeight = index;
    }

    TimeShiftGetRecFormat(&options.format);
    TimeShiftGetFourCC(&options.fcc);

    /**** File path and limit ****/
    strncpy(options.path, tsPath ? tsPath : TS_DEFAULT_PATH,
            sizeof(options.path));

    options.sizeLimit = TimeShiftGetFileSizeLimit();
    m_lastSize        = options.sizeLimit;

    /* Limit the edit control to only accept 8 numbers */
    Edit_LimitText(GetDlgItem(hDlg, IDC_TS_SIZE), 8);

    // Refresh the controls on the dialog with the current data.
    UpdateData(hDlg, FALSE);

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
void CTSOptionsDlg::OnTSPathSelect(HWND hDlg)
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
    {
        memset(path, 0, sizeof(path));
    }

    memset(&info, 0, sizeof(info));
    info.hwndOwner = hDlg;
    info.lpfn      = browseCallback;
    info.ulFlags   = BIF_NEWDIALOGSTYLE;
    info.lParam    = (LPARAM)path;

    list = SHBrowseForFolder(&info);
    if (list)
    {
        if (!SHGetPathFromIDList(list, path) || !TimeShiftIsPathValid(path))
        {
            MessageBox(hDlg, "Invalid path selected", "Error", MB_OK | MB_ICONERROR);
        }
        else
        {
            strncpy(options.path, path, sizeof(options.path));

            maxSize = GetMaximumVolumeFileSize(options.path);
            if (maxSize)
            {
                string text(MakeString() << "The path you have selected is on a volume "
                               "that can't store files that are larger than "
                               "about " << fixed << setprecision(2) << (float)maxSize / (float)(1 << 10) << " GiB"
                               ". Your recorded files will be "
                               "limited to this size even if the maximum size "
                               "is set higher.");
                MessageBox(hDlg, text.c_str(), "Limited Volume", MB_ICONWARNING | MB_OK);
            }
        }

        if (SHGetMalloc(&pMalloc) != E_FAIL)
        {
            pMalloc->Free(list);
            pMalloc->Release();
        }
        else
        {
            MessageBox(hDlg, "Memory leak", "Error", MB_OK | MB_ICONERROR);
        }

        SetWindowText(GetDlgItem(hDlg, IDC_TS_PATH_DISPLAY), options.path);
    }
}

void CTSOptionsDlg::OnCompressionhelp(HWND hDlg)
{
    MessageBox(hDlg, "Configure the audio and video compression settings. "
               "The default is uncompressed frames, which should be "
               "changed if the recording format is RGB. Try using a "
               "fast codec for best results. Huffyuv is recommended.",
               "Compression Tips",
               MB_OK);
}

void CTSOptionsDlg::OnWavehelp(HWND hDlg)
{
    MessageBox(hDlg, "WaveIn: Choose the device to which your tuner card is "
               "directly attached.",
               "Wave Device Help",
               MB_OK);
}

void CTSOptionsDlg::OnHeighthelp(HWND hDlg)
{
    MessageBox(hDlg, "Using full height will record every interlaced frame assuming "
               "none are lost. It's recommended that you use a fast, "
               "lightweight lossless codec like Huffyuv in this mode. "
               "Recording at half the height will reduce the vertical "
               "resolution, but it's faster than recording at full height and "
               "requires much less disk space.",
               "Recording Height Help",
               MB_OK);
}

void CTSOptionsDlg::OnUpdateTSSize(HWND hDlg)
{
    DWORD   value;

    /* The file size edit control has been changed */
    value = GetDlgItemInt(hDlg, IDC_TS_SIZE);

    /* Enable or disable the "no limit" check box */
    if (value)
    {
        if (IsChecked(hDlg, IDC_TS_SIZE_NO_LIMIT))
        {
            SetChecked(hDlg, IDC_TS_SIZE_NO_LIMIT, FALSE);
        }
    } 
    else
    {
        if (!IsChecked(hDlg, IDC_TS_SIZE_NO_LIMIT))
        {
            SetChecked(hDlg, IDC_TS_SIZE_NO_LIMIT, TRUE);
        }
    }

    /* Update the size in GiB */
    string SizeText(MakeString() << "(" << fixed << setprecision(2) << (float)value / (float)(1 << 10) << " GiB)");
    SetWindowText(GetDlgItem(hDlg, IDC_TS_SIZE_GIB), SizeText.c_str());
}

void CTSOptionsDlg::OnKillfocusTSSize(HWND hDlg)
{
    DWORD value;

    /* Make sure something's being displayed in the edit control to reduce
       confusion */
    if (GetDlgItemString(hDlg,IDC_TS_SIZE).empty())
    {
        SetDlgItemInt(hDlg, IDC_TS_SIZE, 0);
    }
    else
    {
        /* Validate the data. Make sure the value is in the range of valid
          values. */
        value = GetDlgItemInt(hDlg, IDC_TS_SIZE);
        if (value > MAX_FILE_SIZE)
        {
            value = MAX_FILE_SIZE;
        }

        /* This will set the value back into the edit box. Weird formatting
          like leading zeroes will also be removed after the value is
          set. */
        SetDlgItemInt(hDlg, IDC_TS_SIZE, value);

        /* Save the value as the last size if it's not zero */
        if (value)
        {
            m_lastSize = value;
        }
   }
}

void CTSOptionsDlg::OnTSSizeNoLimit(HWND hDlg)
{
    if(IsChecked(hDlg, IDC_TS_SIZE_NO_LIMIT))
    {
        SetDlgItemInt(hDlg, IDC_TS_SIZE, 0);
    }
    else
    {
        SetDlgItemInt(hDlg, IDC_TS_SIZE, m_lastSize ? m_lastSize : (MAX_FILE_SIZE >> 1));
    }
}
