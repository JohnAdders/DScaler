/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsGroup.h,v 1.2 2003-10-27 10:39:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 12 Jun 2003   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2003/06/12 10:28:45  atnak
// Initial release
//
//
//////////////////////////////////////////////////////////////////////////////

/** 
 * @file settingsgroup.h settingsgroup Header file
 */
 
#ifndef __SETTINGSGROUP_H__
#define __SETTINGSGROUP_H__

#include "PerSettings.h"
#include "PtrList.h"


enum
{
	// Notify function events
	SGE_PRECHANGE			= 0,
	SGE_UPDATE_DEFAULTS		= 1,
	SGE_GROUP_ACTIVATED		= 2,
	SGE_GROUP_DEACTIVATED	= 3,
	SGE_GROUP_DELETING		= 4,

	// Special parameter values
	SG_NOCONDITION			= -1,
};

typedef void (TSGNotifyFunc)(int nEvent, void* pParam, void* contextPtr);


class CSettingsGroup
{
public:
	CSettingsGroup(CSettingsGroup* pParentGroup = NULL);
	virtual ~CSettingsGroup();

	void				SetCondition(PERSETTING nBit, long lValue = 0);
	void				SetConditionVector(CPerActiveBitsVector* conditionVector);

	void				AddSetting(CPerSetting* setting);
	BOOL				DetachSetting(CPerSetting* setting);

	CSettingsGroup*		CreateSubGroup();
	void				DetachSubGroup(CSettingsGroup* pGroup);

	BOOL				QueryChanges(PERBITS fChangingBits, void* pTidbit = NULL);
	BOOL				CommitChanges(void* pTidbit = NULL);

	BOOL				HatchQuery(CPerSetting* pSetting);

	PERBITS				GetConditionBits(long* pValue = NULL);

	void				SetNotifyFunc(TSGNotifyFunc* notifyFunc, void* contextPtr = NULL);
	TSGNotifyFunc*		GetNotifyFunc(void** pContextPtr = NULL);

	void				CascadeDeactivate();

	BOOL				IsActive() { return (m_fFlags & GRPF_ACTIVE) != 0; };
	BOOL				IsParentActive();

protected:
	BOOL				ConditionVectorCheck();
	void				ConditionVectorUpdate(CPerSetting* setting, BOOL bDeactivate = FALSE);

	void				CallPrechange(CPerSetting* setting);
	void				CallChangeDefaultsFor(CPerSetting* setting);

	void				CallGroupActivated();
	void				CallGroupDeactivated();

	void				NotifyEvent(int nEvent, void* pParam);

protected:
	// Bits for m_fFlags
	enum
	{
		GRPF_ACTIVE			= 1 << 0,
		GRPF_DELETING		= 1 << 1,
	};

	// CSettingsGroupsList is used to internally store subgroups
	class CSettingsGroupsList : public PtrList<CSettingsGroup*>
	{
	public:
		CSettingsGroupsList() { };
		virtual ~CSettingsGroupsList() { };
	};

	// CSettingsList is used to internally store settings
	class CSettingsList : public PtrList<CPerSetting*>
	{
	public:
		CSettingsList() : m_PerBitsSum(0), m_MasterBitsSum(0) { };
		virtual ~CSettingsList() { };

		void push_back(CPerSetting* setting);
		void push_front(CPerSetting* setting);
		void insert(CPerSetting* setting);

		PERBITS GetPerBitsSum() { return m_PerBitsSum; };
		PERBITS GetMasterBitsSum() { return m_MasterBitsSum; };

	protected:
		void AddBits(CPerSetting* setting);

	private:
		PERBITS		m_PerBitsSum;
		PERBITS		m_MasterBitsSum;
	};

private:
	BYTE					m_fFlags;
	CSettingsGroup*			m_pParentGroup;

	PERSETTING				m_nConditionBit;
	long					m_lConditionValue;

	CPerActiveBitsVector*	m_pConditionVector;

	CSettingsList			m_MastersList;
	CSettingsList			m_SettingsList;

	CSettingsGroupsList		m_SubGroupsList;

	TSGNotifyFunc*			m_pNotifyFunc;
	void*					m_pNotifyContext;
};


#endif



