/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsSettingConfig.cpp,v 1.2 2004-08-12 14:02:27 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 Atsushi Nakagawa.  All rights reserved.
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
//  This file is part of the SettingRepository module.  See
//  SettingRepository.h for more information.
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2004/08/08 17:14:08  atnak
// TreeSettingsPage for configuring setting repository settings.
//
//////////////////////////////////////////////////////////////////////////////

/**
* @file TreeSettingsSettingConfig.cpp Implementation for CTreeSettingsSettingConfig class
*/

#include "stdafx.h"
#include "TreeSettingsSettingConfig.h"
#include "SettingRepository.h"
#include "..\DScalerRes\resource.h"


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig
//////////////////////////////////////////////////////////////////////////

CTreeSettingsSettingConfig::CTreeSettingsSettingConfig(CSettingConfigContainer* configs) :
	CTreeSettingsPage(configs->GetTitle().c_str(), IDD_TREESETTINGS_GENERIC),
	m_configs(configs),
	m_currentSetting(NULL)
{
	ASSERT(m_configs != NULL);
}


CTreeSettingsSettingConfig::~CTreeSettingsSettingConfig()
{
	if (m_configs != NULL && m_configs->IsPurgable())
	{
		delete m_configs;
	}
}


void CTreeSettingsSettingConfig::OnOK()
{
	// Apply any pending values.
	m_configs->ApplyValue();
	m_configs->End();
    CTreeSettingsPage::OnOK();
}


void CTreeSettingsSettingConfig::OnCancel()
{
	// Reset any changed values.
	m_configs->ResetValue();
	m_configs->End();
	CTreeSettingsPage::OnCancel();
}


BOOL CTreeSettingsSettingConfig::OnInitDialog()
{
	CTreeSettingsPage::OnInitDialog();

	// Hide controls that aren't used.
	ShowControl(IDC_TREESETTINGS_GENERIC_SAVEPER_GLOBAL, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SAVEPER_SOURCE, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOINPUT, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SAVEPER_AUDIOINPUT, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SAVEPER_VIDEOFORMAT, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SAVEPER_CHANNEL, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SETTINGINFO, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_TOPBOX, FALSE);

	// Let the config know the interface is beginning so it can
	// do any internal initialization.
	m_configs->Begin();

	// Get the list box for all the setting names.
	CListBox* pList = (CListBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_LIST);

	// Fill the list box with all the settings names.
	pList->ResetContent();
	ULONG count = m_configs->GetConfigCount();
	for (ULONG i = 0; i < count; i++)
	{
		PSETTINGCONFIG config = m_configs->GetConfig(i);
		int index = pList->AddString(config->GetTitle().c_str());
		if (index != LB_ERR)
		{
			pList->SetItemData(index, i);
		}
	}

	// Select the first item in the list if it exists
	if (pList->GetCount() > 0)
	{
		pList->SetCurSel(0);
		OnSelchangeSettingList();
	}
	else
	{
		m_currentSetting = NULL;
		InitCurrentSettingControls();
	}

	pList->SetFocus();
	return FALSE;
}


void CTreeSettingsSettingConfig::InitCurrentSettingControls()
{
	BOOL updateLocked = LockWindowUpdate();

	// First set all the visibility to the most common state.
	ShowControl(IDC_TREESETTINGS_GENERIC_EDIT, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SPIN, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_EDIT2, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_CHECK, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, FALSE);
	ShowControl(IDC_TREESETTINGS_GENERIC_SLIDER, FALSE);

	// Stop if m_currentSetting is NULL.
	if (m_currentSetting == NULL)
	{
		ShowControl(IDC_TREESETTINGS_GENERIC_DEFAULT, FALSE);

		if (updateLocked)
		{
			UnlockWindowUpdate();
		}
		return;
	}

	ShowControl(IDC_TREESETTINGS_GENERIC_DEFAULT, TRUE);

	// Adjust visibility of various controls to current setting type
	switch (m_currentSetting->GetType())
	{
	case SETTING_CONFIG_CHECKBOX:
		ShowControl(IDC_TREESETTINGS_GENERIC_CHECK, TRUE);
		break;
	case SETTING_CONFIG_EDITBOX:
		ShowControl(IDC_TREESETTINGS_GENERIC_EDIT2, TRUE);
		break;
	case SETTING_CONFIG_LISTBOX:
		ShowControl(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, TRUE);
		{
			CListBox* pListBox = (CListBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
			CSettingConfigListBox* config = (CSettingConfigListBox*)m_currentSetting;

			// Fill the list box
			pListBox->ResetContent();
			ULONG count = config->GetCount();
			for (ULONG i = 0; i < count; i++)
			{
				int index = pListBox->AddString(config->GetElement(i));
				if (index != LB_ERR)
				{
					pListBox->SetItemData(index, i);
				}
			}
		}
		break;
	case SETTING_CONFIG_SLIDER:
		ShowControl(IDC_TREESETTINGS_GENERIC_EDIT, TRUE);
		ShowControl(IDC_TREESETTINGS_GENERIC_SPIN, TRUE);
		ShowControl(IDC_TREESETTINGS_GENERIC_SLIDER, TRUE);
		break;
	default:
		ShowControl(IDC_TREESETTINGS_GENERIC_DEFAULT, FALSE);
		break;
	}

	// Update the displayed values for each control.
	UpdateCurrentSettingControls();

	if (updateLocked)
	{
		UnlockWindowUpdate();
	}
}


void CTreeSettingsSettingConfig::UpdateCurrentSettingControls()
{
	if (m_currentSetting == NULL)
	{
		return;
	}

	switch (m_currentSetting->GetType())
	{
	case SETTING_CONFIG_CHECKBOX:
		{
			CSettingConfigCheckbox* config = (CSettingConfigCheckbox*)m_currentSetting;
			CButton* pButton = (CButton*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHECK);
			pButton->SetCheck(config->GetCheckboxState());
		}
		break;
	case SETTING_CONFIG_EDITBOX:
		{
			CSettingConfigEditBox* config = (CSettingConfigEditBox*)m_currentSetting;
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT2);
			pEdit->SetWindowText(config->GetEditBoxText().c_str());
		}
		break;
	case SETTING_CONFIG_LISTBOX:
		{
			CSettingConfigListBox* config = (CSettingConfigListBox*)m_currentSetting;
			CListBox* pListBox = (CListBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
			int i = pListBox->FindStringExact(-1, config->GetElement(config->GetListBoxSelected()));
			if (i != LB_ERR)
			{
				pListBox->SetCurSel(i);
			}
		}
		break;
	case SETTING_CONFIG_SLIDER:
		{
			CSettingConfigSlider* config = (CSettingConfigSlider*)m_currentSetting;
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT);
			CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_TREESETTINGS_GENERIC_SLIDER);
			pEdit->SetWindowText(config->GetValue().ToString().c_str());
			pSlider->SetPos(config->GetSliderValue());
		}
		break;
	}
}


void CTreeSettingsSettingConfig::ShowControl(int nID, BOOL bShow)
{
	CWnd* pWnd = GetDlgItem(nID);
	if (pWnd != NULL)
	{
		pWnd->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
}


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig AFX Message Maps
//////////////////////////////////////////////////////////////////////////

// Declare the AFX code necessary to make our maps work.
BEGIN_MESSAGE_MAP(CTreeSettingsSettingConfig, CTreeSettingsPage)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_LBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_LIST, OnSelchangeSettingList)
	ON_EN_KILLFOCUS(IDC_TREESETTINGS_GENERIC_EDIT, OnChangeEditValueNumber)
	ON_NOTIFY(UDN_DELTAPOS, IDC_TREESETTINGS_GENERIC_SPIN, OnDeltaposValueNumberSpin)
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_TREESETTINGS_GENERIC_EDIT2, OnChangeEditValueString)
	ON_CBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, OnSelchangeValueList)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_CHECK, OnClickValueCheckbox)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_DEFAULT, OnClickValueDefault)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig AFX Message Handlers
//////////////////////////////////////////////////////////////////////////

void CTreeSettingsSettingConfig::OnSize(UINT nType, int cx, int cy)
{
	CTreeSettingsPage::OnSize(nType, cx, cy);

	if (nType != SIZE_RESTORED && nType != SIZE_MAXIMIZED)
	{
		return;
	}

	CWnd *pWnd;
	RECT rect;
	LONG iDeltaX = 0;
	LONG iDeltaY = 0;

	// Size and move the bottom value group box into place.
	if ((pWnd = GetDlgItem(IDC_TREESETTINGS_GENERIC_VALUEBOX)) != NULL)
	{
		pWnd->GetWindowRect(&rect);
		this->ScreenToClient(&rect);
		// Calculate out how much the window changed.
		iDeltaX = cx - rect.right + rect.left;
		iDeltaY = cy - rect.bottom;
		// Adjust the control position and size.
		OffsetRect(&rect, 0, iDeltaY);
		rect.right = cx;
		pWnd->MoveWindow(&rect);
	}

	if (iDeltaX == 0 && iDeltaY == 0)
	{
		return;
	}

	// Define the rest of the controls that need to rescale here.
	struct
	{
		int nID;
		int moveGrowDown;
		int moveGrowRight;
	} scaleControls[] =
	{
		{ IDC_TREESETTINGS_GENERIC_LIST, 2, 2 },
		{ IDC_TREESETTINGS_GENERIC_EDIT, 1, 2 },
		{ IDC_TREESETTINGS_GENERIC_SPIN, 1, 0 },
		{ IDC_TREESETTINGS_GENERIC_SLIDER, 1, 2 },
		{ IDC_TREESETTINGS_GENERIC_EDIT2, 1, 2 },
		{ IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, 1, 2 },
		{ IDC_TREESETTINGS_GENERIC_CHECK, 1, 0 },
		{ IDC_TREESETTINGS_GENERIC_DEFAULT, 1, 1 },
	};

	// Use a generic loop for adjusting the rest of the controls
	// based on the horizontal and vertical deltas that were
	// calculated.
	int size = sizeof(scaleControls)/sizeof(scaleControls[0]);
	for (int i = 0; i < size; i++)
	{
		if ((pWnd = GetDlgItem(scaleControls[i].nID)) != NULL)
		{
			// Get the control's rect.
			pWnd->GetWindowRect(&rect);
			// Convert the rect to local coordinates.
			ScreenToClient(&rect);

			// Move or grow the rect down.
			if (scaleControls[i].moveGrowDown)
			{
				if (scaleControls[i].moveGrowDown == 1)
				{
					rect.top += iDeltaY;

				}
				rect.bottom += iDeltaY;
			}
			// Move or grow rect right.
			if (scaleControls[i].moveGrowRight)
			{
				if (scaleControls[i].moveGrowRight == 1)
				{
					rect.left += iDeltaX;

				}
				rect.right += iDeltaX;
			}
			// Adjust the control's size and position.
			pWnd->MoveWindow(&rect);
		}
	}
}


static BOOL CALLBACK ClipChildRgnProc(HWND hWnd, LPARAM lParam)
{
	// Ignore regions for hidden controls.
	if (!(GetWindowStyle(hWnd) & WS_VISIBLE))
	{
		return TRUE;
	}

	// Get the class name for the control.
	TCHAR className[MAX_CLASS_NAME];
	GetClassName(hWnd, className, MAX_CLASS_NAME);

	// Ignore regions for group boxes.
	if (_tcscmp(className, _T("Button")) == 0)
	{
		if (GetWindowStyle(hWnd) & BS_GROUPBOX)
		{
			return TRUE;
		}
	}

	// Get variables back out of lParam
	CWnd* pWnd = (CWnd*)	((VOID**)lParam)[0];
	CDC* pDC = (CDC*)		((VOID**)lParam)[1];

	RECT rect;
	CRgn rgn;

	// Get the child window's rect
	GetWindowRect(hWnd, &rect);
	// Convert the rect to local coordinates
	pWnd->ScreenToClient(&rect);

	// Create a region object with the rect
	rgn.CreateRectRgnIndirect(&rect);
	// Clip out the child window's area
	pDC->SelectClipRgn(&rgn, RGN_DIFF);
	return TRUE;
}


BOOL CTreeSettingsSettingConfig::OnEraseBkgnd(CDC* pDC)
{
	RECT rect;
	// Erase the background manually in a similar fashion to having
	// the WS_CLIPCHILDREN style enabled, to prevent flickering when
	// resizing.

	// Save the DC before setting up clipping
	pDC->SaveDC();

	// Clip out the regions occupied by all controls except group
	// box controls.  Doing this is very similar to enabling the
	// WS_CLIPCHILDREN style but WS_CLIPCHILDREN will also clip the
	// region occupied by group boxes.  Since group boxes do not
	// paint their own backgrounds, we must ensure their region
	// is not clipped so that it can be painted by us.
	VOID* lParam[2] = { this, pDC };
	EnumChildWindows(this->m_hWnd, ClipChildRgnProc, (LPARAM)lParam);

	// Get the client rect.
	GetClientRect(&rect);
	// Fill the client rect with the system background colour.
	pDC->FillSolidRect(&rect, GetSysColor(COLOR_BTNFACE));

	// Restore the DC to discard clipping.
	pDC->RestoreDC(-1);
	// Return TRUE to indicate we erased background.
	return TRUE;
}


void CTreeSettingsSettingConfig::OnSelchangeSettingList()
{
	CListBox* pList = (CListBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_LIST);
	int index = pList->GetCurSel();
	if (index != LB_ERR)
	{
		ULONG i = pList->GetItemData(index);
		// Updated the selected setting pointer.
		m_currentSetting = m_configs->GetConfig(i);
	}
	else
	{
		m_currentSetting = NULL;
	}

	// Fix up the controls for the different setting.
	InitCurrentSettingControls();
}


void CTreeSettingsSettingConfig::OnChangeEditValueNumber()
{
	if (m_currentSetting == NULL ||
		m_currentSetting->GetType() != SETTING_CONFIG_SLIDER)
	{
		return;
	}

	// Perform the necessary variable extraction.
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT);
	CSettingConfigSlider* config = (CSettingConfigSlider*)m_currentSetting;

	// Get the text in the edit box.
	CString text;
	pEdit->GetWindowText(text);

	// Set the new text
	config->SetSliderValue(_ttoi(text));

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls();
}


void CTreeSettingsSettingConfig::OnDeltaposValueNumberSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_currentSetting == NULL ||
		m_currentSetting->GetType() != SETTING_CONFIG_SLIDER)
	{
		return;
	}

	// Perform the necessary variable extraction.
	CSettingConfigSlider* config = (CSettingConfigSlider*)m_currentSetting;

	// Increment or decrement the value by the step amount.
	INT step = config->GetStep();
	if (((NM_UPDOWN*)pNMHDR)->iDelta < 0)
	{
		step *= -1;
	}
	config->SetSliderValue(config->GetSliderValue() + step);

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls();
}


void CTreeSettingsSettingConfig::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_currentSetting == NULL ||
		m_currentSetting->GetType() != SETTING_CONFIG_SLIDER)
	{
		return;
	}

	// Perform the necessary variable extraction.
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_TREESETTINGS_GENERIC_SLIDER);
	CSettingConfigSlider* config = (CSettingConfigSlider*)m_currentSetting;

	// Set the new position
	config->SetSliderValue(pSlider->GetPos());

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls();
}


void CTreeSettingsSettingConfig::OnChangeEditValueString()
{
	if (m_currentSetting == NULL ||
		m_currentSetting->GetType() != SETTING_CONFIG_EDITBOX)
	{
		return;
	}

	// Perform the necessary variable extraction.
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT2);
	CSettingConfigEditBox* config = (CSettingConfigEditBox*)m_currentSetting;

	CString text;
	pEdit->GetWindowText(text);

	// Set the new text
	config->SetEditBoxText((LPCSTR)text);

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls();
}


void CTreeSettingsSettingConfig::OnSelchangeValueList()
{
	if (m_currentSetting == NULL ||
		m_currentSetting->GetType() != SETTING_CONFIG_EDITBOX)
	{
		return;
	}

	// Perform the necessary variable extraction.
	CListBox* pListBox = (CListBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
	CSettingConfigListBox* config = (CSettingConfigListBox*)m_currentSetting;

	int index = pListBox->GetCurSel();
	if (index == LB_ERR)
	{
		return;
	}

	// Set the new selection
	config->SetListBoxSelected(pListBox->GetItemData(index));

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls();
}


void CTreeSettingsSettingConfig::OnClickValueCheckbox()
{
	if (m_currentSetting == NULL ||
		m_currentSetting->GetType() != SETTING_CONFIG_CHECKBOX)
	{
		return;
	}

	CButton* pButton = (CButton*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHECK);
	CSettingConfigCheckbox* config = (CSettingConfigCheckbox*)m_currentSetting;

	config->SetCheckboxState(pButton->GetCheck());

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls();
}


void CTreeSettingsSettingConfig::OnClickValueDefault()
{
	if (m_currentSetting == NULL)
	{
		return;
	}

	switch (m_currentSetting->GetType())
	{
	case SETTING_CONFIG_CHECKBOX:
	case SETTING_CONFIG_EDITBOX:
	case SETTING_CONFIG_LISTBOX:
	case SETTING_CONFIG_SLIDER:
		{
			CSettingConfigSetting* config = (CSettingConfigSetting*)m_currentSetting;
			config->UseDefault();
		}
		break;
	default:
		return;
	}

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls();
}

