/* $Id: TSCompressionDlg.cpp,v 1.2 2005-09-24 18:47:46 dosx86 Exp $ */

/** \file
 * Time Shift compression options dialog
 * \author Nick Kochakian
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "TSCompressionDlg.h"
#include "TimeShift.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTSCompressionDlg dialog


CTSCompressionDlg::CTSCompressionDlg(CWnd *pParent /*=NULL*/,
                                     TS_OPTIONS *options)
	: CDialog(CTSCompressionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTSCompressionDlg)
	m_AudioFormat = _T("");
	m_VideoFormat = _T("");
	//}}AFX_DATA_INIT

    this->options = options;
    m_fcc         = options->fcc;
}


void CTSCompressionDlg::DoDataExchange(CDataExchange* pDX)
{
    char buffer[256];

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTSCompressionDlg)
	DDX_Text(pDX, IDC_TSAUDIOFORMAT, m_AudioFormat);
	DDX_Text(pDX, IDC_TSVIDEOFORMAT, m_VideoFormat);
	//}}AFX_DATA_MAP

    if (!pDX->m_bSaveAndValidate)
    {
        if (TimeShiftGetVideoCompressionDesc(buffer,
                                             sizeof(buffer),
                                             options->recHeight,
                                             options->format))
           m_VideoFormat = buffer;

        if (TimeShiftGetAudioCompressionDesc(buffer, sizeof(buffer)))
           m_AudioFormat = buffer;
    }
}

BEGIN_MESSAGE_MAP(CTSCompressionDlg, CDialog)
	//{{AFX_MSG_MAP(CTSCompressionDlg)
	ON_BN_CLICKED(IDC_TSCONFIGVIDEO, OnConfigVideo)
	ON_BN_CLICKED(IDC_TSCONFIGAUDIO, OnConfigAudio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTSCompressionDlg message handlers

BOOL CTSCompressionDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CTSCompressionDlg::OnConfigVideo() 
{
    TimeShiftVideoCompressionOptions(m_hWnd, options->recHeight,
                                     options->format, &m_fcc);
}

void CTSCompressionDlg::OnConfigAudio() 
{
}

void CTSCompressionDlg::OnOK() 
{
    /* Update the options */
    options->fcc = m_fcc;

    CDialog::OnOK();
}