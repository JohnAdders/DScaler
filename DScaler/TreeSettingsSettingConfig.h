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
// Revision 1.3  2005/03/18 16:19:07  atnak
// Synchronizing work in progress.
//
// Revision 1.2  2004/08/12 14:02:27  atnak
// minor changes
//
// Revision 1.1  2004/08/08 17:14:08  atnak
// TreeSettingsPage for configuring setting repository settings.
//
//////////////////////////////////////////////////////////////////////////////

/**
* @file TreeSettingsSettingConfig.h Header file for CTreeSettingsSettingConfig class
*/

#ifndef __TREESETTINGSSETTINGCONFIG_H__
#define __TREESETTINGSSETTINGCONFIG_H__

#include "TreeSettingsPage.h"
#include "SettingRepository.h"
#include "..\DScalerRes\resource.h"

// This class derives CTreeSettingsPage and uses CTreeSettingsGeneric's
// setting manipulation form to provide a similar interface for settings
// using SettingRepository's CSettingConfig user-interface descriptors.


//////////////////////////////////////////////////////////////////////////
// CTreeSettingsSettingConfig
//////////////////////////////////////////////////////////////////////////
class CTreeSettingsSettingConfig : public CTreeSettingsPage
{
public:
	CTreeSettingsSettingConfig(CSettingConfigContainer* configs); 
    virtual ~CTreeSettingsSettingConfig();

protected:
	virtual void OnOK();
	virtual void OnCancel();

	virtual BOOL OnInitDialog();
	virtual void OnDestroy();

	//////////////////////////////////////////////////////////////////////////
	// AFX Message Maps
	//////////////////////////////////////////////////////////////////////////	
	// This AFX map thing is actually quite handy.  It appears it's possible
	// to get their benefits without using the MFC code generator.  The
	// necessary maps were copied from TreeSettingsGeneric.h.

	// Called when the panel is resizing.
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// Called when the panel is resizing.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	// The big list at the top for all the settings.
	afx_msg void OnSelchangeSettingList();
	// The edit box for inputting numerals.
	afx_msg void OnChangeEditValueNumber();
	// The small up and down buttons for stepping the numerals field.
	afx_msg void OnDeltaposValueNumberSpin(NMHDR* pNMHDR, LRESULT* pResult);
	// Used to receive slider messages for a number value.
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	// The edit box for inputting text for a string value.
	afx_msg void OnChangeEditValueString();
	// The list-box for selecting values from a list.
	afx_msg void OnSelchangeValueList();
	// The check-box for setting binary values.
	afx_msg void OnClickValueCheckbox();
	// The button for setting the default value.
	afx_msg void OnClickValueDefault();
	// The button for updating controls.
	afx_msg void OnClickAdjust();

	// Define some AFX stuff that are necessary for maps.
	DECLARE_MESSAGE_MAP()

protected:
	// Sets up all the controls for the setting in m_currentSetting.
	virtual void InitCurrentSettingControls();
	// Adjusts the values displayed by the controls for m_currentSetting.
	virtual void UpdateCurrentSettingControls(CWnd* pSkipControl);

	// Create the adjust button.
	void CreateAdjustButton();
	// Destroy the adjust button.
	void DestroyAdjustButton();
	// Shows or hides the adjust button.
	void ShowAdjustButton(BOOL show);
	// Positions the adjust button to the right of the given control.
	void PositionAdjustButtonBeside(CWnd* wnd, BOOL moveOnResize);

	// Displays or hides a controls.
	virtual inline void ShowControl(int nID, BOOL bShow);

protected:
	CSettingConfigContainer*	m_configs;
	PSETTINGCONFIG				m_currentSetting;
	BOOL						m_updatingSettingControls;
	CButton*					m_adjustButton;
	HICON						m_adjustButtonIcon;
	BYTE						m_adjustButtonMoveOnResize;
};


#endif

