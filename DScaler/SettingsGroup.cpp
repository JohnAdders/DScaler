/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsGroup.cpp,v 1.2 2003-06-15 06:01:35 atnak Exp $
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

#include "stdafx.h"
#include "SettingsGroup.h"

//
//  Classes implemented in this file
//
//   class CSettingsGroup					- Main class
//   class CSettingsGroup::CSettingsList	- Internal class
//


//--------------------------------------------------------------------
// CSettingsGroup
//--------------------------------------------------------------------


CSettingsGroup::CSettingsGroup(CSettingsGroup* pParentGroup)
{
	m_pParentGroup		= pParentGroup;
	m_fFlags			= 0;

	m_nConditionBit		= (PERSETTING)-1;

	m_pConditionVector	= NULL;
	m_pNotifyFunc		= NULL;
}


//  The proper way to delete the class is to call the CascadeDeactivate()
//  method first.  Only thing the destructor does is to make sure it is
//  removed from the parent group's sub-groups list.  It does not correct
//  the condition vector or call deactivate events on the notification
//  callback.

CSettingsGroup::~CSettingsGroup()
{
	m_fFlags |= GRPF_DELETING;

	NotifyEvent(SGE_GROUP_DELETING, this);
	m_SubGroupsList.clear();

	if (m_pParentGroup != NULL)
	{
		m_pParentGroup->DetachSubGroup(this);
	}
}


void CSettingsGroup::SetCondition(PERSETTING nBit, long lValue)
{
	m_nConditionBit		= nBit;
	m_lConditionValue	= lValue;
}


void CSettingsGroup::SetConditionVector(CPerActiveBitsVector* pConditionVector)
{
	m_pConditionVector = pConditionVector;
}


void CSettingsGroup::SetNotifyFunc(TSGNotifyFunc* pNotifyFunc, void* pContextPtr)
{
	m_pNotifyFunc = pNotifyFunc;
	m_pNotifyContext = pContextPtr;
}


TSGNotifyFunc* CSettingsGroup::GetNotifyFunc(void** pContextPtr)
{
	if (m_pNotifyFunc != NULL)
	{
		if (pContextPtr != NULL)
		{
			*pContextPtr = m_pNotifyContext;
		}
		return m_pNotifyFunc;
	}

	if (m_pParentGroup != NULL)
	{
		return m_pParentGroup->GetNotifyFunc(pContextPtr);
	}

	return NULL;
}


void CSettingsGroup::AddSetting(CPerSetting* setting)
{
	if (setting->GetMasterBits() != 0)
	{
		PERBITS fMasterBits = setting->GetMasterBits();

		m_MastersList.push_back(setting);

		// Masters need to be sorted in the order of dependency
		for (CPerSetting* s = m_MastersList.first(); s != setting; )
		{
			if (s->GetPerBits() & fMasterBits)
			{
				fMasterBits |= s->GetMasterBits();
				m_MastersList.push_back(m_MastersList.release());
				s = m_MastersList.current();
			}
			else
			{
				s = m_MastersList.next();
			}
		}

#ifdef _DEBUG
		if (setting->GetPerBits() & fMasterBits)
		{
			// FATAL ERROR: Cyclic dependency
			//DBG LOG(0, "DEBUG: Setting Master: Setting is (indirectly)"
		//		" a master of its own per bit.");
			//\todo: do something fatal
			ASSERT(FALSE);
		}
#endif
	}
	else
	{
		CPerSetting* pSetting = m_SettingsList.last();

		// Settings should be sorted by perbits to minimize
		// ini section change.
		for ( ; pSetting != NULL; pSetting = m_SettingsList.prev())
		{
			if (pSetting->GetPerBits() < setting->GetPerBits())
			{
				m_SettingsList.next();
				m_SettingsList.insert(setting);
				return;
			}
		}

		m_SettingsList.push_front(setting);
	}
}


BOOL CSettingsGroup::DetachSetting(CPerSetting* setting)
{
	if (setting->GetMasterBits() != 0)
	{
		return m_MastersList.release(setting) != NULL;
	}

	return m_SettingsList.release(setting) != NULL;
}


// fChangingBits == PER_QUERYALL means reload all
BOOL CSettingsGroup::QueryChanges(PERBITS fChangingBits, void* pParam)
{
	CPerSetting* setting;
	CSettingsGroup* group;

	// See if the group condition changes
	if (fChangingBits == PER_QUERYALL || GetConditionBits() & fChangingBits)
	{
		if (!ConditionVectorCheck() != !(m_fFlags & GRPF_ACTIVE))
		{
			if (m_fFlags & GRPF_ACTIVE)
			{
				// The condition no longer matches
				CascadeDeactivate();
			}
			else
			{
				// The condition now matches
				m_fFlags |= GRPF_ACTIVE;
				CallGroupActivated();

				// Reload all proceeding settings
				fChangingBits = PER_QUERYALL;
			}
		}
	}

	// Make sure this group is active
	if (!(m_fFlags & GRPF_ACTIVE))
	{
		return FALSE;
	}

	// Process all master settings first
	if (fChangingBits == PER_QUERYALL || m_MastersList.GetPerBitsSum() & fChangingBits)
	{
		for (setting = m_MastersList.first(); setting != NULL;
			setting = m_MastersList.next())
		{
			// See if this setting value changes
			if (setting->QueryChange(fChangingBits, pParam))
			{
				// Let the caller know of the change
				HatchQuery(setting);

				if (fChangingBits != PER_QUERYALL)
				{
					fChangingBits |= setting->GetMasterBits();
				}
			}
		}
	}

	// Process all the settings
	if (fChangingBits == PER_QUERYALL || m_SettingsList.GetPerBitsSum() & fChangingBits)
	{
		for (setting = m_SettingsList.first(); setting != NULL;
			setting = m_SettingsList.next())
		{
			// See if this setting value changes
			if (setting->QueryChange(fChangingBits, pParam))
			{
				// Let the caller know of the change
				HatchQuery(setting);
			}
		}
	}

	// Process all the subgroups
	for (group = m_SubGroupsList.first(); group != NULL;
		group = m_SubGroupsList.next())
	{
		group->QueryChanges(fChangingBits, pParam);
	}

	return TRUE;
}


BOOL CSettingsGroup::HatchQuery(CPerSetting* pSetting)
{
	// Send prechange event
	CallPrechange(pSetting);

	if (pSetting->GetMasterBits() != 0)
	{
		// Send change defaults event
		CallChangeDefaultsFor(pSetting);

		// Update condition vector
		ConditionVectorUpdate(pSetting);
	}

	return 0;  // Unused
}


BOOL CSettingsGroup::CommitChanges(void* pParam)
{
	// Make sure this group is active
	if (!(m_fFlags & GRPF_ACTIVE))
	{
		return FALSE;
	}

	CPerSetting* setting;
	CSettingsGroup* group;

	// Commit all master setting changes
	for (setting = m_MastersList.first(); setting != NULL;
		setting = m_MastersList.next())
	{
		setting->CommitChange(pParam);
	}

	// Commit all the setting changes
	for (setting = m_SettingsList.first(); setting != NULL;
		setting = m_SettingsList.next())
	{
		setting->CommitChange(pParam);
	}

	// Commit all the subgroup changes
	for (group = m_SubGroupsList.first(); group != NULL;
		group = m_SubGroupsList.next())
	{
		group->CommitChanges(pParam);
	}

	return TRUE;
}


void CSettingsGroup::CascadeDeactivate()
{
	if (!(m_fFlags & GRPF_ACTIVE))
	{
		return;
	}

	m_fFlags &= ~GRPF_ACTIVE;

	// Let the caller know of the deactivate
	CallGroupDeactivated();

	CPerSetting* setting;
	CSettingsGroup* group;

	// Remove all masters from the condition vector
	for (setting = m_MastersList.first(); setting != NULL;
		setting = m_MastersList.next())
	{
		ConditionVectorUpdate(setting, TRUE);
	}

	// Deactivate all subgroups
	for (group = m_SubGroupsList.first(); group != NULL;
		group = m_SubGroupsList.next())
	{
		group->CascadeDeactivate();
	}
}


void CSettingsGroup::CallPrechange(CPerSetting* setting)
{
	NotifyEvent(SGE_PRECHANGE, setting);
}


void CSettingsGroup::CallChangeDefaultsFor(CPerSetting* setting)
{
	NotifyEvent(SGE_UPDATE_DEFAULTS, setting);
}


void CSettingsGroup::CallGroupActivated()
{
	NotifyEvent(SGE_GROUP_ACTIVATED, this);
}


void CSettingsGroup::CallGroupDeactivated()
{
	NotifyEvent(SGE_GROUP_DEACTIVATED, this);
}


void CSettingsGroup::NotifyEvent(int nEvent, void* pParam)
{
	TSGNotifyFunc*	pNotifyFunc;
	void*			pContextPtr;

	pNotifyFunc = GetNotifyFunc(&pContextPtr);

	if (pNotifyFunc != NULL)
	{
		(pNotifyFunc)(nEvent, pParam, pContextPtr);
	}
}


PERBITS CSettingsGroup::GetConditionBits(long* pValue)
{
	if (m_nConditionBit == -1)
	{
		return 0;
	}

	if (pValue != NULL)
	{
		*pValue = m_lConditionValue;
	}

	return (1 << m_nConditionBit);
}


BOOL CSettingsGroup::ConditionVectorCheck()
{
	if (m_nConditionBit == -1)
	{
		return TRUE;
	}

	if (m_pConditionVector != NULL)
	{
		return m_pConditionVector->Check(m_nConditionBit, m_lConditionValue);
	}

	return FALSE;
}


void CSettingsGroup::ConditionVectorUpdate(CPerSetting* setting, BOOL bDeactivate)
{
	if (m_pConditionVector == NULL)
	{
		return;
	}

	PERBITS fMasterBits = setting->GetMasterBits();

	// Update the condition vector for all bits to which
	// the setting is a master of
	for (int i = 0; i < SETTING_PER_LASTONE; i++)
	{
		if (fMasterBits & (1 << i))
		{
			if (bDeactivate)
			{
				m_pConditionVector->Deactivate((PERSETTING)i);
			}
			else
			{
				m_pConditionVector->Set((PERSETTING)i, setting->GetQueriedValue());
			}
		}
	}
}


CSettingsGroup* CSettingsGroup::CreateSubGroup()
{
	CSettingsGroup* group;

	group = new CSettingsGroup(this);
	m_SubGroupsList.push_back(group);

	return group;
}


void CSettingsGroup::DetachSubGroup(CSettingsGroup* group)
{
	if (m_fFlags & GRPF_DELETING)
	{
		return;
	}

	m_SubGroupsList.release(group);
}


BOOL CSettingsGroup::IsParentActive()
{
	if (m_pParentGroup == NULL)
	{
		return TRUE;
	}

	return m_pParentGroup->IsActive();
}


//--------------------------------------------------------------------
// CSettingsGroup::CSettingsList
//--------------------------------------------------------------------

void CSettingsGroup::CSettingsList::push_back(CPerSetting* setting)
{
	AddBits(setting);
	PtrList<CPerSetting*>::push_back(setting);
}


void CSettingsGroup::CSettingsList::push_front(CPerSetting* setting)
{
	AddBits(setting);
	PtrList<CPerSetting*>::push_front(setting);
}


void CSettingsGroup::CSettingsList::insert(CPerSetting* setting)
{
	AddBits(setting);
	PtrList<CPerSetting*>::insert(setting);
}


void CSettingsGroup::CSettingsList::AddBits(CPerSetting* setting)
{
	m_MasterBitsSum |= setting->GetMasterBits();
	m_PerBitsSum |= setting->GetPerBits();
}

