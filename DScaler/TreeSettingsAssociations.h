/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Revision 1.1  2004/08/12 14:08:55  atnak
// TreeSettingsPage for configuring setting repository's setting associations.
//
//////////////////////////////////////////////////////////////////////////////

/**
* @file TreeSettingsAssociations.h Header file for CTreeSettingsAssociations class
*/

#ifndef __TREESETTINGSASSOCIATIONS_H__
#define __TREESETTINGSASSOCIATIONS_H__

#include "TreeSettingsPage.h"
#include "SettingRepository.h"
#include "..\DScalerRes\resource.h"
#include "SubItemCheckboxListCtrl.h"

// This class derives CTreeSettingsPage and uses CTreeSettingsGeneric's
// setting manipulation form to provide a similar interface for settings
// using SettingRepository's CSettingConfig user-interface descriptors.


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig
//////////////////////////////////////////////////////////////////////////
class CTreeSettingsAssociations : public CTreeSettingsPage
{
public:
	CTreeSettingsAssociations(PSETTINGCONFIG configs);
    virtual ~CTreeSettingsAssociations();

protected:
	virtual void OnOK();
	virtual void OnCancel();

	virtual BOOL OnInitDialog();

	virtual void UpdateInformationText(ULONG index);
	virtual void ShowInformationText(BOOL show);

	// Used in for configuring CSubItemCheckboxListctrl that is needed
	// by the individual setting list.
	static UINT SCBQueryProc(UINT query, int iItem, int iSubItem,
							 PSCBLISTCTRL pList, PVOID context);

	//////////////////////////////////////////////////////////////////////////
	// AFX Message Maps
	//////////////////////////////////////////////////////////////////////////
	// Called when the control is to be created.
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// Called when the panel is resizing.
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// Used for receiving dependee checkbox pressing.
	afx_msg void OnClickRange(UINT nId);
	// Called when the individual settings list selection changes.
	afx_msg void OnItemChangeList(NMHDR* pNMHDR, LRESULT* pResult);

	// Define some AFX stuff that are necessary for maps.
	DECLARE_MESSAGE_MAP()


protected:
	BYTE						m_dependeeCount;
	CStatic*					m_dependeeHeading;
	CButton*					m_dependeeButtons;
	CStatic*					m_dependentHeading;
	CListCtrl*					m_pListCtrl;
	CStatic*					m_informationText;

	CSettingConfigAssociation*	m_configs;
	int							m_informationIndex;
};


#endif

