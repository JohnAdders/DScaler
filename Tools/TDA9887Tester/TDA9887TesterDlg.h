/////////////////////////////////////////////////////////////////////////////
// $Id$
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
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TDA9887TESTERDLG_H__5D0B8F38_E6F7_4F29_9CB1_15DD05723296__INCLUDED_)
#define AFX_TDA9887TESTERDLG_H__5D0B8F38_E6F7_4F29_9CB1_15DD05723296__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "HardwareDriver.h"
#include "TDA9887.h"
#include "TDA9887Defines.h"

class CTDA9887TesterDlg : public CDialog
{

public:
	CTDA9887TesterDlg(CHardwareDriver* pDriver, CWnd* pParent = NULL);
	virtual ~CTDA9887TesterDlg();

// Dialogfelddaten
	//{{AFX_DATA(CTDA9887TesterDlg)
	enum { IDD = IDD_TDA9887TESTER_DIALOG };
	CComboBox	m_ctrlComboSources;
	CComboBox	m_ctrlComboAudioIF;
	CComboBox	m_ctrlComboVideoIF;
	CComboBox	m_ctrlComboTakeOverPoint;
	CComboBox	m_ctrlComboTVMod;
	CComboBox	m_ctrlComboStandards;
	BOOL	m_b_B7;
	BOOL	m_b_B6;
	BOOL	m_b_B5;
	BOOL	m_b_B2;
	BOOL	m_b_B1;
	BOOL	m_b_B0;
	BOOL	m_b_C7;
	BOOL	m_b_C6;
	BOOL	m_b_C5;
	BOOL	m_b_E7;
	BOOL	m_b_E6;
	BOOL	m_b_E5;
	CString	m_strControlBytes;
	CString	m_strI2CAdress;
	CString	m_strVideoDecoder;
	CString	m_strVendorID;
	CString	m_strDeviceID;
	CString	m_strSubID;
	//}}AFX_DATA

	// Vom Klassenassistenten generierte Überladungen virtueller Funktionen
	//{{AFX_VIRTUAL(CTDA9887TesterDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:
	HICON			m_hIcon;
	CToolTipCtrl	m_ToolTip;

	// Generierte Message-Map-Funktionen
	//{{AFX_MSG(CTDA9887TesterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeComboStandards();
	afx_msg void OnSelchangeComboSources();
	afx_msg void OnBtnDumpToFile();
	//}}AFX_MSG
	afx_msg void ParseAndWrite(UINT nID);
	DECLARE_MESSAGE_MAP()

private:
	enum eChipDriver
	{
		DRIVER_BT8X8,
		DRIVER_SAA713X,
		DRIVER_CX2388X,
	};

	typedef struct
	{
		char*       Name;
		WORD        VendorId;
		WORD        DeviceId;
		eChipDriver	ChipDriver;
	} TChip;

	typedef struct
	{
		TChip		Chip;
		int         DeviceIndex;
		DWORD       SubSystemId;
	} TSources;

    typedef struct
    {
        char*		szName;
        BYTE		byte;
    } TRegList;
    
	typedef struct
    {
        char* szName;
        BYTE  B;
        BYTE  C;
        BYTE  E;
    } TStandard;

private:
	void InitToolTips();
	void InitComboSources();
	void InitComboStandards();
	void InitComboRegList(const TRegList* pRegList, CComboBox* pComboBox);
	BOOL SelectComboBoxByData(const int iData, CComboBox* pComboBox);
	UINT GetComboSelItemData(const CComboBox* pComboBox);
	CString GetComboSelItemText(const CComboBox* pComboBox);
	int ScanSources();
	void EnableTdaDlgCtrls(BOOL bEnable);
	void DisableAllOtherControls();

private:
	BOOL SaveToFile();
	CHardwareDriver*		m_pDriver;
	CPCICard*				m_pPCICard;
	CTDA9887*				m_pTDA9887;

	CTypedPtrList<CPtrList, TSources*> m_pSources;

    static const TChip		m_ChipList[];

    static const TStandard	m_TStandards[];
    static const TRegList	m_TTVModulation[];
    static const TRegList	m_TTakeOverPoint[];
    static const TRegList	m_TVideoIF[];
    static const TRegList	m_TAudioIF[];
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // !defined(AFX_TDA9887TESTERDLG_H__5D0B8F38_E6F7_4F29_9CB1_15DD05723296__INCLUDED_)
