/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsSettingConfig.cpp,v 1.5 2005-03-20 11:12:40 atnak Exp $
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
// Revision 1.4  2005/03/18 16:19:07  atnak
// Synchronizing work in progress.
//
// Revision 1.3  2004/08/15 03:04:00  atnak
// MAX_CLASS_NAME is not defined on all systems.
//
// Revision 1.2  2004/08/12 14:02:27  atnak
// minor changes
//
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

// Set a random ID for the adjust button.
#define IDC_TREESETTINGS_GENERIC_ADJUST (IDC_TREESETTINGS_GENERIC_LIST + 1000)


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig
//////////////////////////////////////////////////////////////////////////

CTreeSettingsSettingConfig::CTreeSettingsSettingConfig(CSettingConfigContainer* configs) :
	CTreeSettingsPage(configs->GetTitle().c_str(), IDD_TREESETTINGS_GENERIC),
	m_configs(configs),
	m_currentSetting(NULL),
	m_updatingSettingControls(FALSE),
	m_adjustButton(NULL),
	m_adjustButtonImagelist(NULL),
	m_adjustButtonMoveOnResize(FALSE)
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

	CreateAdjustButton();

	// Let the config know the interface is beginning so it can
	// do any internal initialization.
	m_configs->Begin();

	// Get the list box for all the setting names.
	CListBox* pList = (CListBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_LIST);

	// For some reason, the selection is highlighted even when the list
	// box isn't in focus.  The selection isn't meant to be visible when
	// there's no focus, unless LVS_SHOWSELALWAYS is set, in which case
	// it should appear grey.  Changing this style doesn't fix it.
	//pList->ModifyStyle(LVS_SHOWSELALWAYS, 0);

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


void CTreeSettingsSettingConfig::OnDestroy()
{
	DestroyAdjustButton();
}


void CTreeSettingsSettingConfig::InitCurrentSettingControls()
{
	ShowAdjustButton(FALSE);

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
		{
			CButton* pButton = (CButton*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHECK);
			pButton->SetWindowText(m_currentSetting->GetTitle().c_str());
			PositionAdjustButtonBeside(pButton, TRUE);
		}
		break;
	case SETTING_CONFIG_EDITBOX:
		ShowControl(IDC_TREESETTINGS_GENERIC_EDIT2, TRUE);
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT2);
			PositionAdjustButtonBeside(pEdit, TRUE);
		}
		break;
	case SETTING_CONFIG_LISTBOX:
		ShowControl(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, TRUE);
		{
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
			CSettingConfigListBox* config = (CSettingConfigListBox*)m_currentSetting;

			// This doesn't seem to disable sorting.
			//if (config->IsSorted())
			//{
			//	pComboBox->ModifyStyle(0, CBS_SORT);
			//}
			//else
			//{
			//	pComboBox->ModifyStyle(CBS_SORT, 0);
			//}

			// Fill the list box
			pComboBox->ResetContent();
			ULONG count = config->GetCount();
			for (ULONG i = 0; i < count; i++)
			{
				int index = pComboBox->AddString(config->GetElement(i));
				if (index != LB_ERR)
				{
					pComboBox->SetItemData(index, i);
					if (i == config->GetListBoxSelected())
					{
						pComboBox->SetCurSel(index);
					}
				}
			}
			PositionAdjustButtonBeside(pComboBox, TRUE);
		}
		break;
	case SETTING_CONFIG_SLIDER:
		ShowControl(IDC_TREESETTINGS_GENERIC_EDIT, TRUE);
		ShowControl(IDC_TREESETTINGS_GENERIC_SPIN, TRUE);
		ShowControl(IDC_TREESETTINGS_GENERIC_SLIDER, TRUE);
		{
			CSettingConfigSlider* config = (CSettingConfigSlider*)m_currentSetting;
			CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_TREESETTINGS_GENERIC_SLIDER);
			pSlider->SetRange(config->GetMin(), config->GetMax(), TRUE);
			pSlider->SetPageSize(config->GetStep());
			pSlider->SetLineSize(config->GetStep());
			pSlider->ClearTics();
			pSlider->SetTic(config->GetSliderDefaultValue());
			CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_TREESETTINGS_GENERIC_SPIN);
			pSpin->SetRange32(config->GetMin(), config->GetMax());
			PositionAdjustButtonBeside(pSpin, FALSE);
		}
		break;
	default:
		ShowControl(IDC_TREESETTINGS_GENERIC_DEFAULT, FALSE);
		break;
	}

	// Update the displayed values for each control.
	UpdateCurrentSettingControls(NULL);

	if (updateLocked)
	{
		UnlockWindowUpdate();
	}
}


void CTreeSettingsSettingConfig::UpdateCurrentSettingControls(CWnd* pSkipControl)
{
	if (m_updatingSettingControls || m_currentSetting == NULL)
	{
		return;
	}

	// Set this to prevent re-enter.
	m_updatingSettingControls = TRUE;

	switch (m_currentSetting->GetType())
	{
	case SETTING_CONFIG_CHECKBOX:
		{
			CButton* pButton = (CButton*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHECK);
			if (pButton != pSkipControl)
			{
				CSettingConfigCheckbox* config = (CSettingConfigCheckbox*)m_currentSetting;
				pButton->SetCheck(config->GetCheckboxState());
				ShowAdjustButton(FALSE);
			}
		}
		break;
	case SETTING_CONFIG_EDITBOX:
		{
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT2);
			if (pEdit != pSkipControl)
			{
				CSettingConfigEditBox* config = (CSettingConfigEditBox*)m_currentSetting;
				pEdit->SetWindowText(config->GetEditBoxText().c_str());
				ShowAdjustButton(FALSE);
			}
		}
		break;
	case SETTING_CONFIG_LISTBOX:
		{
			CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
			if (pComboBox != pSkipControl)
			{
				CSettingConfigListBox* config = (CSettingConfigListBox*)m_currentSetting;
				int count = pComboBox->GetCount();
				ULONG selected = config->GetListBoxSelected();
				for (int index = 0; index < count; index++)
				{
					if (pComboBox->GetItemData(index) == selected)
					{
						pComboBox->SetCurSel(index);
						ShowAdjustButton(FALSE);
						break;
					}
				}
			}
		}
		break;
	case SETTING_CONFIG_SLIDER:
		{
			CSettingConfigSlider* config = (CSettingConfigSlider*)m_currentSetting;
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_TREESETTINGS_GENERIC_EDIT);
			if (pEdit != pSkipControl)
			{
				pEdit->SetWindowText(config->GetValue().ToString().c_str());
				ShowAdjustButton(FALSE);
			}
			CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_TREESETTINGS_GENERIC_SLIDER);
			if (pSlider != pSkipControl)
			{
				pSlider->SetPos(config->GetSliderValue());
			}
			CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_TREESETTINGS_GENERIC_SPIN);
			if (pSpin != pSkipControl)
			{
				pSpin->SetPos(config->GetSliderValue());
			}
		}
		break;
	}

	m_updatingSettingControls = FALSE;
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
	ON_EN_CHANGE(IDC_TREESETTINGS_GENERIC_EDIT, OnChangeEditValueNumber)
	ON_NOTIFY(UDN_DELTAPOS, IDC_TREESETTINGS_GENERIC_SPIN, OnDeltaposValueNumberSpin)
	ON_WM_HSCROLL()
	ON_EN_KILLFOCUS(IDC_TREESETTINGS_GENERIC_EDIT2, OnChangeEditValueString)
	ON_CBN_SELCHANGE(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST, OnSelchangeValueList)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_CHECK, OnClickValueCheckbox)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_DEFAULT, OnClickValueDefault)
	ON_BN_CLICKED(IDC_TREESETTINGS_GENERIC_ADJUST, OnClickAdjust)
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

	enum eResizeAdjust
	{
		ADJUST_NONE		= 0,
		ADJUST_MOVE		= 1,
		ADJUST_GROW		= 2,
	};

	// How the adjust button scales depends on which control it is placed beside.
	eResizeAdjust adjustButton = m_adjustButtonMoveOnResize ? ADJUST_MOVE : ADJUST_NONE;

	// Define the rest of the controls that need to rescale here.
	struct
	{
		int nID;
		eResizeAdjust rightAdjust;
		eResizeAdjust downAdjust;
	} scaleControls[] =
	{
		{ IDC_TREESETTINGS_GENERIC_LIST,			ADJUST_GROW, ADJUST_GROW },
		{ IDC_TREESETTINGS_GENERIC_EDIT,			ADJUST_NONE, ADJUST_MOVE },
		{ IDC_TREESETTINGS_GENERIC_SPIN,			ADJUST_NONE, ADJUST_MOVE },
		{ IDC_TREESETTINGS_GENERIC_SLIDER,			ADJUST_GROW, ADJUST_MOVE },
		{ IDC_TREESETTINGS_GENERIC_EDIT2,			ADJUST_GROW, ADJUST_MOVE },
		{ IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST,	ADJUST_GROW, ADJUST_MOVE },
		{ IDC_TREESETTINGS_GENERIC_CHECK,			ADJUST_NONE, ADJUST_MOVE },
		{ IDC_TREESETTINGS_GENERIC_DEFAULT,			ADJUST_MOVE, ADJUST_MOVE },
		{ IDC_TREESETTINGS_GENERIC_ADJUST,			adjustButton, ADJUST_MOVE },
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
			if (scaleControls[i].downAdjust)
			{
				if (scaleControls[i].downAdjust == ADJUST_MOVE)
				{
					rect.top += iDeltaY;
				}
				rect.bottom += iDeltaY;
			}
			// Move or grow rect right.
			if (scaleControls[i].rightAdjust)
			{
				if (scaleControls[i].rightAdjust == ADJUST_MOVE)
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
	TCHAR className[1024];
	GetClassName(hWnd, className, sizeof(className)/sizeof(TCHAR));

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
	if (m_currentSetting == NULL || m_updatingSettingControls ||
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

	// Set the new value.
	INT value = _ttoi(text);
	ShowAdjustButton(config->SetSliderValue(value) != value);

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls(pEdit);
}


void CTreeSettingsSettingConfig::OnDeltaposValueNumberSpin(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_currentSetting == NULL || m_updatingSettingControls ||
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
	UpdateCurrentSettingControls(NULL);
}


void CTreeSettingsSettingConfig::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_currentSetting == NULL || m_updatingSettingControls ||
		m_currentSetting->GetType() != SETTING_CONFIG_SLIDER)
	{
		return;
	}

	// Perform the necessary variable extraction.
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_TREESETTINGS_GENERIC_SLIDER);
	CSettingConfigSlider* config = (CSettingConfigSlider*)m_currentSetting;

	// Set the new position
	int value = pSlider->GetPos();
	if (config->GetStep() > 1)
	{
		value -= value % config->GetStep();
	}
	config->SetSliderValue(value);

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls(pSlider);
}


void CTreeSettingsSettingConfig::OnChangeEditValueString()
{
	if (m_currentSetting == NULL || m_updatingSettingControls ||
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
	std::string result(config->SetEditBoxText((LPCSTR)text));
	ShowAdjustButton(text.Compare(result.c_str()) == 0);

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls(pEdit);
}


void CTreeSettingsSettingConfig::OnSelchangeValueList()
{
	if (m_currentSetting == NULL || m_updatingSettingControls ||
		m_currentSetting->GetType() != SETTING_CONFIG_LISTBOX)
	{
		return;
	}

	// Perform the necessary variable extraction.
	CComboBox* pComboBox = (CComboBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHOOSEFROMLIST);
	CSettingConfigListBox* config = (CSettingConfigListBox*)m_currentSetting;

	int index = pComboBox->GetCurSel();
	if (index == LB_ERR)
	{
		return;
	}

	// Set the new selection
	ULONG value = pComboBox->GetItemData(index);
	ShowAdjustButton(config->SetListBoxSelected(value) != value);

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls(pComboBox);
}


void CTreeSettingsSettingConfig::OnClickValueCheckbox()
{
	if (m_currentSetting == NULL || m_updatingSettingControls ||
		m_currentSetting->GetType() != SETTING_CONFIG_CHECKBOX)
	{
		return;
	}

	CButton* pButton = (CButton*)GetDlgItem(IDC_TREESETTINGS_GENERIC_CHECK);
	CSettingConfigCheckbox* config = (CSettingConfigCheckbox*)m_currentSetting;

	BOOL value = pButton->GetCheck() != 0;
	ShowAdjustButton(config->SetCheckboxState(value) != value);

	// Let any necessary updates be performed.
	UpdateCurrentSettingControls(pButton);
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
	UpdateCurrentSettingControls(NULL);
}


void CTreeSettingsSettingConfig::OnClickAdjust()
{
	UpdateCurrentSettingControls(NULL);
	m_adjustButton->ShowWindow(SW_HIDE);
}


void CTreeSettingsSettingConfig::CreateAdjustButton()
{
	CRect rect(0, 0, GetSystemMetrics(SM_CXSMICON)+6, GetSystemMetrics(SM_CYSMICON)+6);

	// Create the button.
	m_adjustButton = new CButton();
	m_adjustButton->Create("", BS_FLAT, rect, this, IDC_TREESETTINGS_GENERIC_ADJUST);
/*
	// Create the image list and icon picture.
	m_adjustButtonImagelist = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON), ILC_COLOR32, 1, 1);
	ImageList_AddIcon(m_adjustButtonImagelist, LoadIcon(NULL, IDI_EXCLAMATION));

	// Set the image list.
	BUTTON_IMAGELIST buttonImagelist;
	buttonImagelist.himl = m_adjustButtonImagelist;
	SetRect(&buttonImagelist.margin, 0, 0, 0, 0);
	buttonImagelist.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
	m_adjustButton->SetImageList(&buttonImagelist);*/
}


void CTreeSettingsSettingConfig::DestroyAdjustButton()
{
	if (m_adjustButton != NULL)
	{
		m_adjustButton->DestroyWindow();
		m_adjustButton = NULL;
	}
	if (m_adjustButtonImagelist != NULL)
	{
		ImageList_Destroy(m_adjustButtonImagelist);
		m_adjustButtonImagelist = NULL;
	}
}


void CTreeSettingsSettingConfig::ShowAdjustButton(BOOL show)
{
	if (GetFocus() == m_adjustButton)
	{
		CListBox* pList = (CListBox*)GetDlgItem(IDC_TREESETTINGS_GENERIC_LIST);
		pList->SetFocus();
	}
	ShowControl(IDC_TREESETTINGS_GENERIC_ADJUST, show);
}


void CTreeSettingsSettingConfig::PositionAdjustButtonBeside(CWnd* pWnd, BOOL moveOnResize)
{
	CRect rectWnd;
	CRect rectButton;

	pWnd->GetWindowRect(rectWnd);
	this->ScreenToClient(rectWnd);

	m_adjustButton->GetWindowRect(rectButton);

	rectButton.OffsetRect(rectWnd.right + rectButton.Width() / 2,
		rectWnd.top + (rectWnd.Height() - rectButton.Height()) / 2);
	m_adjustButton->MoveWindow(rectButton);

	m_adjustButtonMoveOnResize = moveOnResize;
}

