/* $Id: TSCompressionDlg.cpp,v 1.1 2005-07-17 20:45:44 dosx86 Exp $ */

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


CTSCompressionDlg::CTSCompressionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTSCompressionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTSCompressionDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTSCompressionDlg::DoDataExchange(CDataExchange* pDX)
{
    char buffer[256];

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTSCompressionDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

    if (!pDX->m_bSaveAndValidate)
    {
        if (TimeShiftGetCompressionDesc(buffer, sizeof(buffer), true))
           SetText(IDC_TSVIDEOFORMAT, buffer);

        if (TimeShiftGetCompressionDesc(buffer, sizeof(buffer), false))
           SetText(IDC_TSAUDIOFORMAT, buffer);
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

void CTSCompressionDlg::OnConfigVideo() 
{
    TimeShiftVideoCompressionOptions(m_hWnd);
}

void CTSCompressionDlg::OnConfigAudio() 
{
}

void CTSCompressionDlg::SetText(int id, LPSTR text)
{
    HWND hWnd = NULL;

    GetDlgItem(id, &hWnd);
    if (hWnd)
       ::SetWindowText(hWnd, text);
}
