/////////////////////////////////////////////////////////////////////////////
// $Id: TDA9887TesterDlg.cpp,v 1.2 2004-11-28 21:33:29 to_see Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Torsten Seeboth. All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
// $Log: not supported by cvs2svn $
// Revision 1.1  2004/10/30 19:30:22  to_see
// initial checkin
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TDA9887Tester.h"
#include "TDA9887TesterDlg.h"

#include "BT848Card.h"
#include "CX2388xCard.h"
#include "SAA7134Card.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const CTDA9887TesterDlg::TChip CTDA9887TesterDlg::m_ChipList[] =
{
    {"BT848",	0x109e, 0x0350, DRIVER_BT8X8,},
    {"BT849",	0x109e, 0x0351, DRIVER_BT8X8,},
    {"BT878",	0x109e, 0x036e, DRIVER_BT8X8,},
    {"BT878A",	0x109e, 0x036f, DRIVER_BT8X8,},
    {"CX2388x",	0x14F1, 0x8800, DRIVER_CX2388X,},
    {"SAA7134",	0x1131, 0x7134, DRIVER_SAA713X,},
    {"SAA7133",	0x1131, 0x7133, DRIVER_SAA713X,},
    {"SAA7130",	0x1131, 0x7130, DRIVER_SAA713X,}, // not sure
};

const CTDA9887TesterDlg::TRegList CTDA9887TesterDlg::m_TTVModulation[]=
{
	{"Positive AM TV",		TDA9887_PositiveAmTV},
	{"FM Radio (1)",		TDA9887_FmRadio1},
	{"Negative FM TV",		TDA9887_NegativeFmTV},
	{"FM Radio (2)",		TDA9887_FmRadio2},
	{NULL, NULL},
};

const CTDA9887TesterDlg::TRegList CTDA9887TesterDlg::m_TTakeOverPoint[]=
{
	{"+15",	0x1f},	{"+14",	0x1e},	{"+13",	0x1d},	{"+12",	0x1c},
	{"+11",	0x1b},	{"+10",	0x1a},	{"+9",	0x19},	{"+8",	0x18},
	{"+7",	0x17},	{"+6",	0x16},	{"+5",	0x15},	{"+4",	0x14},
	{"+3",	0x13},	{"+2",	0x12},	{"+1",	0x11},	{"0",	0x10},
	{"-1",	0x0f},	{"-2",	0x0e},	{"-3",	0x0d},	{"-4",	0x0c},
	{"-5",	0x0b},	{"-6",	0x0a},	{"-7",	0x09},	{"-8",	0x08},
	{"-9",	0x07},	{"-10",	0x06},	{"-11",	0x05},	{"-12",	0x04},
	{"-13",	0x03},	{"-14",	0x02},	{"-15",	0x01},	{"-16",	0x00},
	{NULL, NULL},
};

const CTDA9887TesterDlg::TRegList CTDA9887TesterDlg::m_TVideoIF[]=
{
	{"58.75 MHz",			TDA9887_VideoIF_58_75},
	{"45.75 MHz",			TDA9887_VideoIF_45_75},
	{"38.9 MHz",			TDA9887_VideoIF_38_90},
	{"38.0 MHz",			TDA9887_VideoIF_38_00},
	{"33.9 MHz",			TDA9887_VideoIF_33_90},
	{"33.4 MHz",			TDA9887_VideoIF_33_40},
	{"45.75 MHz + pin13",	TDA9887_RadioIF_45_75},
	{"38.9 MHz + pin13",	TDA9887_RadioIF_38_90},
	{NULL, NULL},
};

const CTDA9887TesterDlg::TRegList CTDA9887TesterDlg::m_TAudioIF[]=
{
	{"4.5 MHz",				TDA9887_AudioIF_4_5},
	{"5.5 MHz",				TDA9887_AudioIF_5_5},
	{"6.0 MHz",				TDA9887_AudioIF_6_0},
	{"6.5 MHz / AM",		TDA9887_AudioIF_6_5},
	{NULL, NULL},
};

const CTDA9887TesterDlg::TStandard CTDA9887TesterDlg::m_TStandards[]=
{
	{	
		"PAL-BG",
		TDA9887_NegativeFmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_5_5  | TDA9887_VideoIF_38_90,
	},
	{	
		"PAL-I",
		TDA9887_NegativeFmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_0  | TDA9887_VideoIF_38_90,
	},
	{	
		"PAL-DK",
		TDA9887_NegativeFmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_5  | TDA9887_VideoIF_38_00,
	},
	{	
		"PAL-M/N",
		TDA9887_NegativeFmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis75 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_4_5  | TDA9887_VideoIF_45_75,
	},
	{	
		"SECAM-L",
		TDA9887_PositiveAmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_5  | TDA9887_VideoIF_38_90,
	},
	{	
		"SECAM-DK",
		TDA9887_NegativeFmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_6_5  | TDA9887_VideoIF_38_00,
	},
	{	
		"NTSC-M",
		TDA9887_NegativeFmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50  | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_4_5  | TDA9887_VideoIF_45_75 | TDA9887_Gating_36,
	},
	{	
		"NTSC-JP",
		TDA9887_NegativeFmTV | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis50  | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_4_5  | TDA9887_VideoIF_58_75 | TDA9887_Gating_36,
	},
	{	
		"Radio",
		TDA9887_FmRadio1	 | TDA9887_QSS | TDA9887_OutputPort1Inactive | TDA9887_OutputPort2Inactive,
		TDA9887_DeemphasisON | TDA9887_Deemphasis75 | TDA9887_TakeOverPointDefault,
		TDA9887_AudioIF_5_5  | TDA9887_RadioIF_38_90,
	},
};

CTDA9887TesterDlg::CTDA9887TesterDlg(CHardwareDriver* pDriver, CWnd* pParent /*=NULL*/) :
	CDialog(CTDA9887TesterDlg::IDD, pParent),
	m_pDriver(pDriver),
	m_pPCICard(NULL),
	m_pTDA9887(NULL)
{
	//{{AFX_DATA_INIT(CTDA9887TesterDlg)
	m_b_B7 = FALSE;
	m_b_B6 = FALSE;
	m_b_B5 = FALSE;
	m_b_B2 = FALSE;
	m_b_B1 = FALSE;
	m_b_B0 = FALSE;
	m_b_C7 = FALSE;
	m_b_C6 = FALSE;
	m_b_C5 = FALSE;
	m_b_E7 = FALSE;
	m_b_E6 = FALSE;
	m_b_E5 = FALSE;
	m_strVideoDecoder	= _T("n/a");
	m_strVendorID		= _T("n/a");
	m_strDeviceID		= _T("n/a");
	m_strSubID			= _T("n/a");
	m_strI2CAdress		= _T("n/a");
	m_strControlBytes	= _T("n/a");
	//}}AFX_DATA_INIT

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CTDA9887TesterDlg::~CTDA9887TesterDlg()
{
	delete m_pTDA9887;
	delete m_pPCICard;

	while(! m_pSources.IsEmpty() )
	{
		TSources* pSource = m_pSources.RemoveHead();
		delete pSource;
	}
}

void CTDA9887TesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTDA9887TesterDlg)
	DDX_Control(pDX, IDC_COMBO_SOURCES,			m_ctrlComboSources);
	DDX_Control(pDX, IDC_COMBO_AUDIO_IF,		m_ctrlComboAudioIF);
	DDX_Control(pDX, IDC_COMBO_VIDEO_IF,		m_ctrlComboVideoIF);
	DDX_Control(pDX, IDC_COMBO_TAKEOVER_POINT,	m_ctrlComboTakeOverPoint);
	DDX_Control(pDX, IDC_COMBO_TV_MOD,			m_ctrlComboTVMod);
	DDX_Control(pDX, IDC_COMBO_STANDARDS,		m_ctrlComboStandards);
	DDX_Check(pDX, IDC_CHECK1, m_b_B7);
	DDX_Check(pDX, IDC_CHECK2, m_b_B6);
	DDX_Check(pDX, IDC_CHECK3, m_b_B5);
	DDX_Check(pDX, IDC_CHECK4, m_b_B2);
	DDX_Check(pDX, IDC_CHECK5, m_b_B1);
	DDX_Check(pDX, IDC_CHECK6, m_b_B0);
	DDX_Check(pDX, IDC_CHECK7, m_b_C7);
	DDX_Check(pDX, IDC_CHECK8, m_b_C6);
	DDX_Check(pDX, IDC_CHECK9, m_b_C5);
	DDX_Check(pDX, IDC_CHECK10, m_b_E7);
	DDX_Check(pDX, IDC_CHECK11, m_b_E6);
	DDX_Check(pDX, IDC_CHECK12, m_b_E5);
	DDX_Text(pDX, IDC_CONTROL_BYTES,			m_strControlBytes);
	DDX_Text(pDX, IDC_TDA_I2C_ADRESS,			m_strI2CAdress);
	DDX_Text(pDX, IDC_VIDEO_DECODER,			m_strVideoDecoder);
	DDX_Text(pDX, IDC_VENDOR_ID,				m_strVendorID);
	DDX_Text(pDX, IDC_DEVICE_ID,				m_strDeviceID);
	DDX_Text(pDX, IDC_SUBSYSTEM_ID,				m_strSubID);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTDA9887TesterDlg, CDialog)
	//{{AFX_MSG_MAP(CTDA9887TesterDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO_STANDARDS, OnSelchangeComboStandards)
	ON_CBN_SELCHANGE(IDC_COMBO_SOURCES, OnSelchangeComboSources)
	ON_BN_CLICKED(IDC_BTN_DUMP_TO_FILE, OnBtnDumpToFile)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(BN_CLICKED, IDC_CHECK1, IDC_CHECK12, ParseAndWrite)
	ON_CONTROL_RANGE(CBN_SELCHANGE, IDC_COMBO_TV_MOD, IDC_COMBO_AUDIO_IF, ParseAndWrite)
END_MESSAGE_MAP()

BOOL CTDA9887TesterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	InitToolTips();

	if(!ScanSources())
	{
		DisableAllOtherControls();
		return FALSE;
	}

	InitComboSources();
	InitComboStandards();

	InitComboRegList(m_TTVModulation,	&m_ctrlComboTVMod);
	InitComboRegList(m_TTakeOverPoint,	&m_ctrlComboTakeOverPoint);
	InitComboRegList(m_TVideoIF,		&m_ctrlComboVideoIF);
	InitComboRegList(m_TAudioIF,		&m_ctrlComboAudioIF);

	EnableTdaDlgCtrls(FALSE);
	
	// Start with 1st card
	m_ctrlComboSources.SetCurSel(0);

	// Needed. CComboBox does not send CBN_SELCHANGE when call SetCurSel()
	OnSelchangeComboSources();

	return TRUE;
}

void CTDA9887TesterDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}

	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CTDA9887TesterDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CTDA9887TesterDlg::PreTranslateMessage(MSG* pMsg) 
{
	m_ToolTip.RelayEvent(pMsg);	
	return CDialog::PreTranslateMessage(pMsg);
}

void CTDA9887TesterDlg::InitComboRegList(const TRegList* pRegList, CComboBox* pComboBox)
{
	for(int i = 0; pRegList[i].szName; i++)
	{
		int j = pComboBox->AddString(pRegList[i].szName);
		pComboBox->SetItemData(j, pRegList[i].byte);
	}
}

void CTDA9887TesterDlg::InitComboStandards()
{
	int iSize = sizeof(m_TStandards) / sizeof(m_TStandards[0]);
	for(int i = 0; i < iSize; i++)
	{
		int j = m_ctrlComboStandards.AddString(m_TStandards[i].szName);
		m_ctrlComboStandards.SetItemData(j, i);
	}
}

void CTDA9887TesterDlg::InitComboSources()
{
	m_ctrlComboSources.ResetContent();
	POSITION pos = m_pSources.GetHeadPosition();
	while(pos)
	{
		TSources* pSource = m_pSources.GetAt(pos);

		CString strTemp;
		strTemp.Format("%s Card [%d]", pSource->Chip.Name, pSource->DeviceIndex);

		int iItem = m_ctrlComboSources.AddString(strTemp);
		m_ctrlComboSources.SetItemData(iItem, (DWORD)pos);

		m_pSources.GetNext(pos);
	}
}

BOOL CTDA9887TesterDlg::SelectComboBoxByData(const int iData, CComboBox* pComboBox)
{
	BOOL bOK = FALSE;
	
	for(int i = 0; i < pComboBox->GetCount(); i++)
	{
		if(iData == (int)pComboBox->GetItemData(i))
		{
			pComboBox->SetCurSel(i);
			bOK = TRUE;
			break;
		}
	}

	return bOK;
}

UINT CTDA9887TesterDlg::GetComboSelItemData(const CComboBox* pComboBox)
{
	UINT uReturn = 0;
	int iSelItem = pComboBox->GetCurSel();
	
	if(iSelItem != CB_ERR)
	{
		UINT uData = pComboBox->GetItemData(iSelItem);
		if(uData != CB_ERR)
		{
			uReturn = uData;
		}
	}

	return uReturn;
}

CString CTDA9887TesterDlg::GetComboSelItemText(const CComboBox *pComboBox)
{
	CString strReturn;
	strReturn.Empty();

	int iSelItem = pComboBox->GetCurSel();
	
	if(iSelItem != CB_ERR)
	{
		pComboBox->GetLBText(iSelItem, strReturn);
	}

	return strReturn;
}

void CTDA9887TesterDlg::OnSelchangeComboSources() 
{
	POSITION pos = (POSITION)GetComboSelItemData(&m_ctrlComboSources);
	if(!pos)
	{
		return;
	}

	TSources* pSource = m_pSources.GetAt(pos);

	m_strVideoDecoder = pSource->Chip.Name;
	m_strVendorID.Format("0x%04X", pSource->Chip.VendorId);
	m_strDeviceID.Format("0x%04X", pSource->Chip.DeviceId);
	m_strSubID.Format("0x%08X", pSource->SubSystemId);

	UpdateData(FALSE);

	if(m_pTDA9887)
	{
		delete m_pTDA9887;
		m_pTDA9887 = NULL;
	}

	if(m_pPCICard)
	{
		delete m_pPCICard;
		m_pPCICard = NULL;
	}

	switch(pSource->Chip.ChipDriver)
	{
	case DRIVER_BT8X8:
		m_pPCICard = new CBT848Card(m_pDriver);
		break;

	case DRIVER_SAA713X:
		m_pPCICard = new CSAA7134Card(m_pDriver);
		break;

	case DRIVER_CX2388X:
		m_pPCICard = new CCX2388xCard(m_pDriver);
		break;

	// never get here
	default:
        AfxMessageBox("Error in chip list - no driver found.\nPlease contact developer.", MB_OK);
        return;
	}

	if (m_pPCICard->OpenPCICard(pSource->Chip.VendorId, pSource->Chip.DeviceId, pSource->DeviceIndex) == FALSE)
	{
        delete m_pPCICard;
		m_pPCICard = NULL;
        AfxMessageBox("Error at open PCI-Card", MB_OK);
        return;
	}

	m_pTDA9887 = new CTDA9887(m_pPCICard);

	if(m_pTDA9887->Detect())
	{
		EnableTdaDlgCtrls(TRUE);
		m_strI2CAdress.Format("0x%02X", *m_pTDA9887->GetI2CAdress());

		// Default settings Pal(BG)
		m_ctrlComboStandards.SetCurSel(0);

		// Needed. CComboBox does not send CBN_SELCHANGE when call SetCurSel()
		OnSelchangeComboStandards();
	}

	else
	{
		delete m_pTDA9887;
		m_pTDA9887 = NULL;
		EnableTdaDlgCtrls(FALSE);
		m_strI2CAdress = ("n/a");
	}

	UpdateData(FALSE);
}

void CTDA9887TesterDlg::OnSelchangeComboStandards() 
{
	int iSel = m_ctrlComboStandards.GetCurSel();
	if(iSel == CB_ERR)
	{
		return;
	}

	m_TStandards[iSel].B & TDA9887_OutputPort2Inactive	?	m_b_B7 = FALSE	: m_b_B7 = TRUE;			// bit b7
	m_TStandards[iSel].B & TDA9887_OutputPort1Inactive	?	m_b_B6 = FALSE	: m_b_B6 = TRUE;			// bit b6
	m_TStandards[iSel].B & TDA9887_ForcedMuteAudioON	?	m_b_B5 = TRUE	: m_b_B5 = FALSE;			// bit b5
	SelectComboBoxByData(m_TStandards[iSel].B & TDA9887_FmRadio2, &m_ctrlComboTVMod);					// bit b3:4
	m_TStandards[iSel].B & TDA9887_QSS					?	m_b_B2 = TRUE : m_b_B2 = FALSE;				// bit b2
	m_TStandards[iSel].B & TDA9887_AutoMuteFmActive		?	m_b_B1 = TRUE : m_b_B1 = FALSE;				// bit b1
	m_TStandards[iSel].B & TDA9887_VideoTrapBypassON	?	m_b_B0 = TRUE : m_b_B0 = FALSE;				// bit b0

	m_TStandards[iSel].C & TDA9887_AudioGain6			?	m_b_C7 = TRUE : m_b_C7 = FALSE;				// bit c7
	m_TStandards[iSel].C & TDA9887_Deemphasis50			?	m_b_C6 = TRUE : m_b_C6 = FALSE;				// bit c6
	m_TStandards[iSel].C & TDA9887_DeemphasisON			?	m_b_C5 = TRUE : m_b_C5 = FALSE;				// bit c5
	SelectComboBoxByData(m_TStandards[iSel].C & TDA9887_TakeOverPointMax, &m_ctrlComboTakeOverPoint);	// bit c0:4

	m_TStandards[iSel].E & TDA9887_AgcOutON				?	m_b_E7 = TRUE : m_b_E7 = FALSE;				// bit e7
	m_TStandards[iSel].E & TDA9887_Gating_36			?	m_b_E6 = TRUE : m_b_E6 = FALSE;				// bit e6
	m_TStandards[iSel].E & TDA9887_TunerGainLow			?	m_b_E5 = TRUE : m_b_E5 = FALSE;				// bit e5
	SelectComboBoxByData(m_TStandards[iSel].E & TDA9887_RadioIF_38_90,	&m_ctrlComboVideoIF);			// bit e2:4
	SelectComboBoxByData(m_TStandards[iSel].E & TDA9887_AudioIF_6_5,	&m_ctrlComboAudioIF);			// bit e0:1

	UpdateData(FALSE);

	ParseAndWrite(IDC_CHECK1); // IDC_CHECK1 = dummy
}

void CTDA9887TesterDlg::ParseAndWrite(UINT nID) 
{
	if(!m_pTDA9887)
	{
		return;
	}

	UpdateData();

    BYTE bData[3];
	ZeroMemory(bData, sizeof(bData));

/*
	Content of bData[]:

	bData[0] = Byte B
	bData[1] = Byte C
	bData[2] = Byte E
*/

	if(m_b_B7	== FALSE) bData[0] |= TDA9887_OutputPort2Inactive;	// bit b7
	if(m_b_B6	== FALSE) bData[0] |= TDA9887_OutputPort1Inactive;	// bit b6
	if(m_b_B5	== TRUE ) bData[0] |= TDA9887_ForcedMuteAudioON;	// bit b5
	bData[0] |= GetComboSelItemData(&m_ctrlComboTVMod);				// bit b3:4
	if(m_b_B2	== TRUE ) bData[0] |= TDA9887_QSS;					// bit b2
	if(m_b_B1	== TRUE ) bData[0] |= TDA9887_AutoMuteFmActive;		// bit b1
	if(m_b_B0	== TRUE ) bData[0] |= TDA9887_VideoTrapBypassON;	// bit b0

	if(m_b_C7	== TRUE ) bData[1] |= TDA9887_AudioGain6;			// bit c7
	if(m_b_C6	== TRUE ) bData[1] |= TDA9887_Deemphasis50;			// bit c6
	if(m_b_C5	== TRUE ) bData[1] |= TDA9887_DeemphasisON;			// bit c5
	bData[1] |= GetComboSelItemData(&m_ctrlComboTakeOverPoint);		// bit c0:4

	if(m_b_E7	== TRUE ) bData[2] |= TDA9887_AgcOutOFF;			// bit e7
	if(m_b_E6	== TRUE ) bData[2] |= TDA9887_Gating_36;			// bit e6
	if(m_b_E5	== TRUE ) bData[2] |= TDA9887_TunerGainLow;			// bit e5
	bData[2] |= GetComboSelItemData(&m_ctrlComboVideoIF);			// bit e2:4
	bData[2] |= GetComboSelItemData(&m_ctrlComboAudioIF);			// bit e0:1

	m_strControlBytes.Format("0x%02x 0x%02x 0x%02x", bData[0], bData[1], bData[2]);
	UpdateData(FALSE);

	m_pTDA9887->WriteControlBytes(bData);
}

void CTDA9887TesterDlg::EnableTdaDlgCtrls(BOOL bEnable)
{
	for(int i = IDC_STATIC_LOAD_STANDARD_FRAME; i < IDC_BTN_DUMP_TO_FILE +1; i++)
	{
		if(GetDlgItem(i)->GetSafeHwnd())
		{
			GetDlgItem(i)->EnableWindow(bEnable);
		}
	}
}

void CTDA9887TesterDlg::DisableAllOtherControls()
{
	for(int i = IDC_STATIC_SOURCE_CARD_FRAME; i < IDC_BTN_DUMP_TO_FILE +1; i++)
	{
		if(GetDlgItem(i)->GetSafeHwnd())
		{
			GetDlgItem(i)->EnableWindow(FALSE);
		}
	}
}

void CTDA9887TesterDlg::InitToolTips()
{
	m_ToolTip.Create(this);
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK1),				"Control-Bit B7");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK2),				"Control-Bit B6");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK3),				"Control-Bit B5");
	m_ToolTip.AddTool(GetDlgItem(IDC_COMBO_TV_MOD),			"Control-Bit's B3:B4");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK4),				"Control-Bit B2");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK5),				"Control-Bit B1");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK6),				"Control-Bit B0");

	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK7),				"Control-Bit C7");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK8),				"Control-Bit C6");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK9),				"Control-Bit C5");
	m_ToolTip.AddTool(GetDlgItem(IDC_COMBO_TAKEOVER_POINT),	"Control-Bit's C0:C4");

	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK10),				"Control-Bit E7");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK11),				"Control-Bit E6");
	m_ToolTip.AddTool(GetDlgItem(IDC_CHECK12),				"Control-Bit E5");
	m_ToolTip.AddTool(GetDlgItem(IDC_COMBO_VIDEO_IF),		"Control-Bit's E2:E4");
	m_ToolTip.AddTool(GetDlgItem(IDC_COMBO_AUDIO_IF),		"Control-Bit's E0:E1");
}

int CTDA9887TesterDlg::ScanSources()
{
	int iSources = 0;
	for(int i=0; i < sizeof(m_ChipList) / sizeof(TChip); i++)
	{
		DWORD	dwSubSystemId	= 0;
		int		iDeviceIndex	= 0;

		while(m_pDriver->DoesThisPCICardExist(m_ChipList[i].VendorId, m_ChipList[i].DeviceId, iDeviceIndex, dwSubSystemId) == TRUE)
		{
			TSources* pSource		= new TSources;
			pSource->DeviceIndex	= iDeviceIndex;
			pSource->SubSystemId	= dwSubSystemId;
			pSource->Chip			= m_ChipList[i];
			m_pSources.AddTail(pSource);

			iDeviceIndex++;
			iSources++;
		}
	}

	return iSources;
}

void CTDA9887TesterDlg::OnBtnDumpToFile() 
{
	if(!SaveToFile())
	{
		AfxMessageBox("Error at writing dump file.");
	}
}

BOOL CTDA9887TesterDlg::SaveToFile()
{
	CFileDialog FileSaveDlg(	FALSE,
								"txt",
								"Tda9887",
 								OFN_EXPLORER|OFN_NOCHANGEDIR|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_ENABLESIZING,
 								"Text Documents (*.txt)|*.txt|All Files (*.*)|*.*||",
								this );
 	
 	FileSaveDlg.m_ofn.lpstrTitle = "Save TDA9887Tester's settings";
 
 	if(FileSaveDlg.DoModal() == IDOK)
	{
		FILE* hFile = fopen(FileSaveDlg.GetPathName(), "w");
		if(hFile == NULL)
		{
			return FALSE;
		}

		fprintf(hFile, "%-25s %s\n",	"Video Decoder:",			m_strVideoDecoder);
		fprintf(hFile, "%-25s %s\n",	"Vendor ID:",				m_strVendorID);
		fprintf(hFile, "%-25s %s\n",	"Device ID:",				m_strDeviceID);
		fprintf(hFile, "%-25s %s\n\n",	"Subsystem ID:",			m_strSubID);

		fprintf(hFile, "%-25s %s\n",	"TDA9887 I2C-Adress:",		m_strI2CAdress);
		fprintf(hFile, "%-25s %s\n",	"Byte B/C/E:",				m_strControlBytes);
		fprintf(hFile, "%-25s %s\n\n",	"TV Standard:",				GetComboSelItemText(&m_ctrlComboStandards));
		
		fprintf(hFile, "%-25s %s\n",	"B7   Out Port2 Active:",	m_b_B7 ? "Yes"			: "No");
		fprintf(hFile, "%-25s %s\n",	"B6   Out Port1 Active:",	m_b_B6 ? "Yes"			: "No");
		fprintf(hFile, "%-25s %s\n",	"B5   Force Audio Mute:",	m_b_B5 ? "Yes"			: "No");
		fprintf(hFile, "%-25s %s\n",	"B3:4 TV Modulation:",		GetComboSelItemText(&m_ctrlComboTVMod));
		fprintf(hFile, "%-25s %s\n",	"B2   Carrier Mode:",		m_b_B2 ? "QSS"			: "Intercarrier");
		fprintf(hFile, "%-25s %s\n",	"B1   Auto Mute FM:",		m_b_B1 ? "Yes"			: "No");
		fprintf(hFile, "%-25s %s\n\n",	"B0   Sound Trap Bypass:",	m_b_B0 ? "Yes(Video)"	: "No(Audio)");
		
		fprintf(hFile, "%-25s %s\n",	"C7   Audio Gain:",			m_b_C7 ? "-6dB"			: "0dB");
		fprintf(hFile, "%-25s %s\n",	"C6   Deemphasis:",			m_b_C6 ? "50"			: "75");
		fprintf(hFile, "%-25s %s\n",	"C5   Deemphasis On:",		m_b_C5 ? "Yes"			: "No");
		fprintf(hFile, "%-25s %s\n\n",	"C0:4 Takeover Point:",		GetComboSelItemText(&m_ctrlComboTakeOverPoint));

		fprintf(hFile, "%-25s %s\n",	"E7   AGC Feature:",		m_b_E7 ? "Yes"			: "No");
		fprintf(hFile, "%-25s %s\n",	"E6   Secam(L) Gating:",	m_b_E6 ? "36%"			: "0%");
		fprintf(hFile, "%-25s %s\n",	"E5   Min. Gain:",			m_b_E5 ? "Yes"			: "No");
		fprintf(hFile, "%-25s %s\n",	"E2:4 Video IF:",			GetComboSelItemText(&m_ctrlComboVideoIF));
		fprintf(hFile, "%-25s %s\n\n",	"E0:1 Audio IF:",			GetComboSelItemText(&m_ctrlComboAudioIF));

		fclose(hFile);

		return TRUE;
	}

	else
	{
		return FALSE;
	}
}

