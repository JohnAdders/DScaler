/////////////////////////////////////////////////////////////////////////////
// $Id: TreeSettingsAssociations.cpp,v 1.1 2004-08-12 14:08:55 atnak Exp $
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
//////////////////////////////////////////////////////////////////////////////

/**
* @file TreeSettingsAssociations.cpp Implementation for CTreeSettingsAssociations class
*/

#include "stdafx.h"
#include "TreeSettingsAssociations.h"
#include "SettingRepository.h"
#include "..\DScalerRes\resource.h"
#include "SubItemCheckboxListCtrl.h"
#include <sstream>
#include <strsafe.h>

// Define resource IDs that will be used by the dynamic controls.  Since
// IDD_TREESETTINGS_EMPTY shouldn't have any prefab controls, there should
// be no conflict by selecting any ID.
static const int IDD_SETTINGS_LIST = IDD_TREESETTINGS_EMPTY + 1;
static const int IDD_DEPENDEE_BUTTONS_FIRST = IDD_TREESETTINGS_EMPTY + 2;

// These should probably go into a resource for translatability.  There're
// hardcoded texts in UpdateInformationText() as well.
static LPCTSTR kDependeeHeading = _T("Save setting values associated with these master settings:");
static LPCTSTR kCustomizeHeading = _T("Customize dependencies for individual setting:");
static LPCTSTR kSettingColumnText = _T("Setting");


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsAssociations
//////////////////////////////////////////////////////////////////////////

CTreeSettingsAssociations::CTreeSettingsAssociations(PSETTINGCONFIG configs) :
	CTreeSettingsPage(configs->GetTitle().c_str(), IDD_TREESETTINGS_EMPTY),
	m_dependeeCount(0),
	m_dependeeHeading(NULL),
	m_dependeeButtons(NULL),
	m_dependantHeading(NULL),
	m_pListCtrl(NULL),
	m_informationIndex(-1)
{
	ASSERT(configs != NULL && configs->GetType() == SETTING_CONFIG_ASSOCIATION);
	m_configs = reinterpret_cast<CSettingConfigAssociation*>(configs);

	m_dependeeHeading = new CStatic();
	m_dependantHeading = new CStatic();
	m_informationText = new CStatic();
	m_pListCtrl = new CSubItemCheckboxListCtrl(SCBQueryProc, this);
}


CTreeSettingsAssociations::~CTreeSettingsAssociations()
{
	if (m_configs != NULL && m_configs->IsPurgable())
	{
		delete m_configs;
	}

	// Delete the dynamically created controls.
	if (m_dependeeHeading != NULL)
	{
		delete m_dependeeHeading;
	}
	if (m_dependeeButtons != NULL)
	{
		delete [] m_dependeeButtons;
	}
	if (m_dependantHeading != NULL)
	{
		delete m_dependantHeading;
	}
	if (m_pListCtrl != NULL)
	{
		delete m_pListCtrl;
	}
}


void CTreeSettingsAssociations::OnOK()
{
	// Apply any pending values.
	m_configs->ApplyValue();
	m_configs->End();
	CTreeSettingsPage::OnOK();
}


void CTreeSettingsAssociations::OnCancel()
{
	// Reset any changed values.
	m_configs->ResetValue();
	m_configs->End();
	CTreeSettingsPage::OnCancel();
}


BOOL CTreeSettingsAssociations::OnInitDialog()
{
	CTreeSettingsPage::OnInitDialog();

	ModifyStyle(0, WS_CLIPCHILDREN);

	// Set all sub-control fonts to same as this.
	m_dependeeHeading->SetFont(GetFont());
	m_dependantHeading->SetFont(GetFont());
	m_informationText->SetFont(GetFont());
	m_pListCtrl->SetFont(GetFont());

	for (int i = 0; i < m_dependeeCount; i++)
	{
		m_dependeeButtons[i].SetFont(GetFont());
		m_dependeeButtons[i].SetCheck(m_configs->IsDependencyEnabled(i));
	}

	m_pListCtrl->SetFocus();
	return FALSE;
}


void CTreeSettingsAssociations::UpdateInformationText(ULONG index)
{
	CSettingConfigDependant* config = (CSettingConfigDependant*)m_configs->GetConfig(index);

	std::ostringstream oss;
	std::string last;
	oss << config->GetTitle().c_str();

	BOOL count = 0;
	for (int i = 0; i < m_dependeeCount; i++)
	{
		if (config->IsDependantLocked(i) ||
			config->IsDependant(i) && m_configs->IsDependencyEnabled(i))
		{
			if (count > 0)
			{
				oss << (count == 1 ? " depends on " : ", ") << last;
			}
			last = m_configs->GetDependencyTitle(i);
			count++;
		}
	}

	if (count == 0)
	{
		// Improper grammar?  "depends on nothing" is easier to read at
		// a glance than "does not depend on anything."
		oss << " depends on nothing.";
	}
	else
	{
		oss << (count == 1 ? " depends on " : " and ") << last << ".";
	}

	m_informationText->SetWindowText(CString(oss.str().c_str()));
	m_informationIndex = index;

	ShowInformationText(TRUE);
}


void CTreeSettingsAssociations::ShowInformationText(BOOL show)
{
	if (((m_informationText->GetStyle() & WS_VISIBLE) != 0) == show)
	{
		return;
	}

	CRect listRect;
	m_pListCtrl->GetWindowRect(listRect);
	CRect textRect;
	m_informationText->GetWindowRect(textRect);

	ScreenToClient(listRect);
	ScreenToClient(textRect);

	if (show)
	{
		listRect.bottom = textRect.top - 5;
		m_informationText->ShowWindow(SW_SHOW);
	}
	else
	{
		listRect.bottom = textRect.bottom;
		m_informationText->ShowWindow(SW_HIDE);
	}

	m_pListCtrl->MoveWindow(listRect);
}


UINT CTreeSettingsAssociations::SCBQueryProc(UINT query, int iItem, int iSubItem,
											 PSCBLISTCTRL pList, PVOID context)
{
	CTreeSettingsAssociations* pWnd = (CTreeSettingsAssociations*)context;

	if (iSubItem == 0 || --iSubItem >= pWnd->m_dependeeCount)
	{
		return FALSE;
	}

	ULONG index = (ULONG)pList->GetItemData(iItem);

	CSettingConfig* scfg = pWnd->m_configs->GetConfig(index);
	if (scfg->GetType() != SETTING_CONFIG_DEPENDANT)
	{
		return FALSE;
	}

	CSettingConfigDependant* config = (CSettingConfigDependant*)scfg;
	UINT checkbox = SCBR_CHECK_SHOW|SCBR_CHECK_FLAT;

	switch (query)
	{
	case SCBQ_GET_CHECKBOX:
		if (config->IsDependee(iSubItem))
		{
			return checkbox|SCBR_CHECK_INACTIVE;
		}
		if (config->IsDependantLocked(iSubItem))
		{
			return checkbox|SCBR_CHECK_CHECKED|SCBR_CHECK_INACTIVE;
		}
		if (config->IsDependant(iSubItem))
		{
			return checkbox|SCBR_CHECK_CHECKED;
		}
		if (pWnd->m_configs->IsDependantBlocked(index, iSubItem))
		{
			return checkbox|SCBR_CHECK_INACTIVE;
		}
		return checkbox;
	case SCBQ_TOGGLE_CHECKBOX:
		config->SetDependant(iSubItem, !config->IsDependant(iSubItem));
		if (config->IsDependee())
		{
			pWnd->m_configs->RecalculateBlocked();
			pList->Invalidate(FALSE);
		}
		pWnd->UpdateInformationText(pList->GetItemData(iItem));
		break;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig AFX Message Maps
//////////////////////////////////////////////////////////////////////////

// Declare the AFX code necessary to make our maps work.
BEGIN_MESSAGE_MAP(CTreeSettingsAssociations, CTreeSettingsPage)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_CONTROL_RANGE(BN_CLICKED, IDD_DEPENDEE_BUTTONS_FIRST, IDD_DEPENDEE_BUTTONS_FIRST+32, OnClickRange)
	ON_NOTIFY(LVN_ITEMCHANGED, IDD_SETTINGS_LIST, OnItemChangeList)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig AFX Message Handlers
//////////////////////////////////////////////////////////////////////////

int CTreeSettingsAssociations::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeSettingsPage::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// Let config know the interface is beginning so it can
	// do any internal initialization.
	m_configs->Begin();

	// Get the number of dependees there are.
	m_dependeeCount = m_configs->GetDependencyCount();

	// Adjust the number of buttons to the number of dependees.
	if (m_dependeeButtons != NULL)
	{
		delete [] m_dependeeButtons;
	}
	m_dependeeButtons = new CButton[m_dependeeCount];

	CRect rect(0, 0, 0, 0);
	ULONG i;

	// Create the dependee heading text.
	m_dependeeHeading->Create(kDependeeHeading,
		WS_CHILD|WS_VISIBLE|SS_LEFT, rect, this);

	std::ostringstream oss;

	// Create all the dependee checkboxes.
	for (i = 0; i < m_dependeeCount; i++)
	{
		oss.str("");
		oss << i+1 << ". " << m_configs->GetDependencyTitle((BYTE)i);
		m_dependeeButtons[i].Create(CString(oss.str().c_str()),
			WS_CHILD|WS_VISIBLE|WS_TABSTOP|BS_AUTOCHECKBOX|BS_LEFT|BS_VCENTER, rect,
			this, IDD_DEPENDEE_BUTTONS_FIRST + i);
	}

	// Create the individual settings list heading text.
	m_dependantHeading->Create(kCustomizeHeading,
		WS_CHILD|WS_VISIBLE|SS_LEFT, rect, this);

	// Create the individual settings list.
	m_pListCtrl->Create(WS_CHILD|WS_VISIBLE|WS_BORDER|WS_TABSTOP|LVS_REPORT|LVS_NOSORTHEADER|LVS_SINGLESEL,
		rect, this, IDD_SETTINGS_LIST);
	DWORD dwStyle = m_pListCtrl->GetExtendedStyle();
	m_pListCtrl->SetExtendedStyle(dwStyle|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);

	// Insert the first column in the settings list
	m_pListCtrl->InsertColumn(0, kSettingColumnText, LVCFMT_LEFT, 20);

	// Insert all the sub-columns in the settings list.
	for (i = 0; i < m_dependeeCount; i++)
	{
		oss.str("");
		oss << i+1;
		m_pListCtrl->InsertColumn(i+1, CString(oss.str().c_str()), LVCFMT_LEFT, 20);
	}

	// Insert all setting names into the settings list.
	ULONG configCount = m_configs->GetConfigCount();
	for (i = 0; i < configCount; i++)
	{
		PSETTINGCONFIG config = m_configs->GetConfig(i);
		int index = m_pListCtrl->InsertItem(i, CString(config->GetTitle().c_str()));
		if (index != -1)
		{
			m_pListCtrl->SetItemData(index, i);
		}
	}

	m_informationText->Create(_T(""),
		WS_CHILD|SS_LEFT|SS_ENDELLIPSIS, rect, this);

	return 0;
}


void CTreeSettingsAssociations::OnSize(UINT nType, int cx, int cy)
{
	CTreeSettingsPage::OnSize(nType, cx, cy);

	if (nType != SIZE_RESTORED && nType != SIZE_MAXIMIZED)
	{
		return;
	}

	CRect clientRect;
	GetClientRect(clientRect);

	if (clientRect.Width() < 200)
	{
		clientRect.right = clientRect.left + 200;
	}
	if (clientRect.Height() < 200)
	{
		clientRect.bottom = clientRect.top + 200;
	}

	int buttonWidth = (int)(clientRect.Width()/3);
	int buttonHeight = GetSystemMetrics(SM_CYMENUSIZE);

	CRect rect;
	int i;

	// Move the dependee heading text to the correct position.
	rect.SetRect(0, 0, clientRect.Width(), buttonHeight);
	m_dependeeHeading->MoveWindow(rect);
	// Adjust for the next control.
	rect.MoveToY(rect.bottom + 1);

	// Move all the buttons into correct position.
	rect.right = buttonWidth;
	for (i = 0; i < m_dependeeCount; i++)
	{
		m_dependeeButtons[i].MoveWindow(rect);
		if (i % 3 == 2)
		{
			rect.MoveToXY(0, rect.bottom);
		}
		else
		{
			rect.OffsetRect(buttonWidth, 0);
		}
	}

	// Adjust for the next control.
	if (i % 3 != 0)
	{
		rect.MoveToXY(0, rect.bottom);
	}
	rect.OffsetRect(0, 7);

	// Move the settings list heading text into position.
	rect.right = clientRect.Width();
	m_dependantHeading->MoveWindow(rect);
	// Adjust for the next control.
	rect.MoveToY(rect.bottom + 1);

	// Move the individual settings list into position.
	if (m_informationText->GetStyle() & WS_VISIBLE)
	{
		rect.bottom = clientRect.bottom - buttonHeight - 5;
	}
	else
	{
		rect.bottom = clientRect.bottom;
	}
	m_pListCtrl->MoveWindow(rect);
	m_pListCtrl->Invalidate(FALSE);

	// Resize the first column to fill the available space.
	LVCOLUMN lvcolumn;
	lvcolumn.mask = LVCF_WIDTH;
	lvcolumn.cx = clientRect.Width() - 20 * m_dependeeCount - GetSystemMetrics(SM_CXVSCROLL) - 5;
	m_pListCtrl->SetColumn(0, &lvcolumn);

	rect.MoveToY(clientRect.bottom - buttonHeight);
	rect.bottom = rect.top + buttonHeight;
	m_informationText->MoveWindow(rect);
	m_informationText->Invalidate(FALSE);
}


void CTreeSettingsAssociations::OnClickRange(UINT nId)
{
	CButton* pButton = (CButton*)GetDlgItem(nId);
	m_configs->EnableDependency(nId - IDD_DEPENDEE_BUTTONS_FIRST, pButton->GetCheck());
	if (m_informationIndex != -1)
	{
		UpdateInformationText(m_informationIndex);
	}
}


void CTreeSettingsAssociations::OnItemChangeList(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pnmv = (LPNMLISTVIEW)pNMHDR; 

	if (pnmv->uNewState & LVIS_SELECTED)
	{
		UpdateInformationText(m_pListCtrl->GetItemData(pnmv->iItem));
	}
}

