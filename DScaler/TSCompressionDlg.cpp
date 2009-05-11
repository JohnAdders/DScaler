/* $Id$ */

/** \file
 * Time Shift compression options dialog
 * \author Nick Kochakian
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "TSCompressionDlg.h"
#include "TimeShift.h"

/////////////////////////////////////////////////////////////////////////////
// CTSCompressionDlg dialog


CTSCompressionDlg::CTSCompressionDlg(TS_OPTIONS* Options) :
    CDSDialog(MAKEINTRESOURCE(IDD_TSCOMPRESSION)),
    m_Options(Options),
    m_fcc(Options->fcc)
{
}

INT_PTR CTSCompressionDlg::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    default:
        return FALSE;
    }
}

void CTSCompressionDlg::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_TSCONFIGVIDEO:
        if(BN_CLICKED == codeNotify)
        {
            OnConfigVideo(hDlg);
        }
        break;
    case IDC_TSCONFIGAUDIO:
        if(BN_CLICKED == codeNotify)
        {
            OnConfigAudio(hDlg);
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
    }
}



/////////////////////////////////////////////////////////////////////////////
// CTSCompressionDlg message handlers

BOOL CTSCompressionDlg::OnInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam)
{
    TCHAR buffer[256];

    if (TimeShiftGetVideoCompressionDesc(buffer,
                                     sizeof(buffer),
                                     m_Options->recHeight,
                                     m_Options->format))
    {
        SetWindowText(GetDlgItem(hDlg, IDC_TSVIDEOFORMAT), buffer);
    }

    if (TimeShiftGetAudioCompressionDesc(buffer, sizeof(buffer)))
    {
        SetWindowText(GetDlgItem(hDlg, IDC_TSAUDIOFORMAT), buffer);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CTSCompressionDlg::OnConfigVideo(HWND hDlg)
{
    TimeShiftVideoCompressionOptions(hDlg, m_Options->recHeight, m_Options->format, &m_fcc);
}

void CTSCompressionDlg::OnConfigAudio(HWND hDlg)
{
}

void CTSCompressionDlg::OnOK(HWND hDlg)
{
    /* Update the options */
    m_Options->fcc = m_fcc;
    m_AudioFormat = GetDlgItemString(hDlg, IDC_TSAUDIOFORMAT);
    m_VideoFormat = GetDlgItemString(hDlg, IDC_TSVIDEOFORMAT);

    EndDialog(hDlg, IDOK);
}
