/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsMaster2.cpp,v 1.1 2003-06-12 10:28:45 atnak Exp $
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
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingsMaster2.h"
#include "SettingsGroup.h"

using namespace smInternal;

static void GenericGroupCallback(int nEvent, void* pParam, void* pContextPtr);
static void ChannelGroupCallback(int nEvent, void* pParam, void* pContextPtr);
static void ExternalQueryCallback(CSettingWrapper* pSetting, void* pContextPtr);
static BOOL SettingChangeCallback(long, long, BOOL, void* pContextPtr);

static void DoChannelPrechange(void);


CSettingsMaster*	g_pSettingMaster = NULL;

static int			g_nNotifyReason = 0;


void InitializeSettingsMaster(void)
{
	g_pSettingMaster = new CSettingsMaster();
}


void CleanupSettingsMaster(void)
{
	// Delete the global group for graceful clean up
	g_pSettingMaster->DeleteGroup(0);

	delete g_pSettingMaster;
	g_pSettingMaster = NULL;
}


CSettingsMaster::CSettingsMaster()
{
	m_vGroups.reserve(SM_INITIAL_GROUPS_BUFFER);

	m_IniChanger = new CPerActiveBitsVector();

	m_MasterInfo.fUserPerMask = 0;
	m_MasterInfo.pIniChanger = m_IniChanger;

	m_ImplicitGroupID = 0;
	m_ImplicitPerBits = 0;
	m_ImplicitLockedBits = 0;

	// Create the global group (ID = 0)
	m_pMasterGroup = new CSettingsGroup();
	PushBackGroup(m_pMasterGroup, "", NULL);
}


CSettingsMaster::~CSettingsMaster()
{
	if (m_pMasterGroup != NULL)
	{
		delete m_pMasterGroup;
	}
	delete m_IniChanger;

	while (!m_vGroups.empty())
	{
#ifdef _DEBUG
		TGroupInfo* pGroupInfo = m_vGroups.back();

		if (pGroupInfo->pGroup != NULL)
		{
			// This SHOULD NOT happen
			ASSERT(FALSE);
			delete pGroupInfo->pGroup;
		}
#endif
		delete (TGroupInfo*)m_vGroups.back();
		m_vGroups.pop_back();
	}
}


GROUPID CSettingsMaster::CreateSettingsGroup(GROUPID nParentId, const char* szName)
{
	ASSERT(nParentId >= 0 && nParentId < n_vGroups.size());

	// Get the parent group
	CSettingsGroup* pParentGroup = m_vGroups[nParentId]->pGroup;

	ASSERT(pParentGroup != NULL);

	// Create a subgroup off the parent
	CSettingsGroup* pGroup = pParentGroup->CreateSubGroup();
	return PushBackGroup(pGroup, szName, m_vGroups[nParentId]);
}


GROUPID CSettingsMaster::CreateSettingsGroup(const char* szName)
{
	return CreateSettingsGroup(m_ImplicitGroupID, szName);
}


void CSettingsMaster::DeleteGroup(GROUPID nGroupId)
{
	ASSERT(nParentId >= 0 && nParentId < n_vGroups.size());

	CSettingsGroup* pGroup = GetGroupById(nGroupId);

	g_nNotifyReason = SMR_DELETE_GROUP;
	pGroup->CascadeDeactivate();
	delete pGroup;

	if (nGroupId == 0)
	{
		m_pMasterGroup = NULL;
	}
}


void CSettingsMaster::SetGroupCondition(GROUPID nGroupId, PERSETTING nBit, long lMatch)
{
	CSettingsGroup* pGroup = GetGroupById(nGroupId);
	pGroup->SetCondition(nBit, lMatch);
}


void CSettingsMaster::SetGroupCondition(PERSETTING nBit, long lMatch)
{
	SetGroupCondition(m_ImplicitGroupID, nBit, lMatch);
}


void CSettingsMaster::RemoveGroupCondition(GROUPID nGroupId)
{
	CSettingsGroup* pGroup = GetGroupById(nGroupId);
	pGroup->SetCondition((PERSETTING)SG_NOCONDITION);
}


void CSettingsMaster::RemoveGroupCondition()
{
	RemoveGroupCondition(m_ImplicitGroupID);
}


GROUPID CSettingsMaster::SetImplicitGroup(GROUPID nGroupID)
{
	GROUPID nOldID = m_ImplicitGroupID;
	m_ImplicitGroupID = nGroupID;
	return nOldID;
}


PERBITS CSettingsMaster::SetImplicitPer(PERBITS fBits)
{
	PERBITS fOldBits = m_ImplicitPerBits;
	m_ImplicitPerBits = fBits;
	return fOldBits;
}


PERBITS CSettingsMaster::SetImplicitLockedPer(PERBITS fBits)
{
	PERBITS fOldBits = m_ImplicitLockedBits;
	m_ImplicitLockedBits = fBits;
	return fOldBits;
}


GROUPID CSettingsMaster::ResetImplicitGroup()
{
    GROUPID nOldID = m_ImplicitGroupID;
    m_ImplicitGroupID = 0;
    return nOldID;
}


PERBITS CSettingsMaster::ResetImplicitPer()
{
    PERBITS fOldBits = m_ImplicitPerBits;
    m_ImplicitPerBits = 0;
    return fOldBits;
}


PERBITS CSettingsMaster::ResetImplicitLockedPer()
{
    PERBITS fOldBits = m_ImplicitLockedBits;
    m_ImplicitLockedBits = 0;
    return fOldBits;
}


void CSettingsMaster::AddSetting(GROUPID nGroupId, LPSETTING pSetting, PERBITS fPerBits,
								 PERBITS fLockedBits, PERBITS fMasterBits)
{
	CSettingsGroup* pGroup = GetGroupById(nGroupId);

	CSettingWrapper* setting = new CSettingWrapper(pSetting, (fPerBits | m_ImplicitPerBits),
		(fLockedBits | m_ImplicitLockedBits), fMasterBits);

	// Make sure we're notified of external changes
	setting->HookExternalChange(ExternalQueryCallback, m_vGroups[nGroupId]);

	pGroup->AddSetting(setting);
}


void CSettingsMaster::AddSetting(LPSETTING pSetting, PERBITS fPerBits, PERBITS fLockedBits,
								 PERBITS fMasterBits)
{
	AddSetting(m_ImplicitGroupID, pSetting, fPerBits, fLockedBits, fMasterBits);
}


void CSettingsMaster::AddMasterSetting(GROUPID nGroupId, LPSETTING pSetting,
									   PERBITS fMasterBits, PERBITS fPerBits,
									   PERBITS fLockedBits)
{
	AddSetting(nGroupId, pSetting, fPerBits, fLockedBits, fMasterBits);
}


void CSettingsMaster::AddMasterSetting(LPSETTING pSetting, PERBITS fMasterBits,
									   PERBITS fPerBits, PERBITS fLockedBits)
{
	AddSetting(m_ImplicitGroupID, pSetting, fPerBits, fLockedBits, fMasterBits);
}


void CSettingsMaster::AddChannelCondition(GROUPID nGroupId, PERSETTING nBit, long lMatch)
{
	CSettingsGroup* pParentGroup = GetGroupById(nGroupId);

	// Create a sub group that will take care of the channel condition
	CSettingsGroup* pGroup = pParentGroup->CreateSubGroup();

	pGroup->SetConditionVector(m_IniChanger);
	pGroup->SetCondition(nBit, lMatch);

	pGroup->SetNotifyFunc(ChannelGroupCallback, this);

	CSettingWrapper* pSetting =
		new CSettingWrapper(m_ChannelSetting, 0, 1 << nBit, PER_CHANNEL);

	pSetting->m_fFlags |= CSettingWrapper::SETF_CHANNEL_SETTING;

	// Make sure we're notified of external changes
	pSetting->HookExternalChange(ExternalQueryCallback, m_vGroups[nGroupId]);

	// Add a setting to the parent group to be the channel master
	pParentGroup->AddSetting(pSetting);
}


void CSettingsMaster::AddChannelCondition(PERSETTING nBit, long lMatch)
{
	AddChannelCondition(m_ImplicitGroupID, nBit, lMatch);
}


void CSettingsMaster::SetNotifyFunction(GROUPID nGroupId, TSMNotifyFunc pFunc)
{
	ASSERT(nGroupId >= 0 && nGroupId < m_vGroups.size());

	TGroupInfo* pGroupInfo = m_vGroups[nGroupId];
	pGroupInfo->pNotifyFunc = pFunc;
}


void CSettingsMaster::SetNotifyFunction(TSMNotifyFunc pFunc)
{
	SetNotifyFunction(m_ImplicitGroupID, pFunc);
}


void CSettingsMaster::ReadyGroup(GROUPID nGroupId)
{
	CSettingsGroup* pGroup = GetGroupById(nGroupId);

	if (pGroup->IsParentActive())
	{
		g_nNotifyReason = SMR_READY_GROUP;
		pGroup->QueryChanges(PER_QUERYALL, &m_MasterInfo);
		pGroup->CommitChanges(&m_MasterInfo);
	}
}


void CSettingsMaster::ReadyGroup()
{
	ReadyGroup(m_ImplicitGroupID);
}


void CSettingsMaster::EnableUserPerBits(PERBITS fUserBits)
{
	m_MasterInfo.fUserPerMask |= fUserBits;
}


void CSettingsMaster::DisableUserPerBits(PERBITS fUserBits)
{
	m_MasterInfo.fUserPerMask &= ~fUserBits;
}


void CSettingsMaster::ChangeChannel(long lValue)
{
	m_ChannelSetting->SetValue(lValue);
}


void CSettingsMaster::OnExternalQuery(CSettingWrapper* pChangedSetting,
									  CSettingsGroup* pParentGroup)
{
	if (pParentGroup == NULL)
	{
		pParentGroup = m_pMasterGroup;
	}

	// Have the event callbacks properly called
	g_nNotifyReason = SMR_SET_VALUE;
	pParentGroup->HatchQuery(pChangedSetting);

	if (pChangedSetting->GetMasterBits() == 0)
	{
		// Just commit and OnChange the change
		pChangedSetting->CommitChange(&m_MasterInfo);
		return;
	}

	g_nNotifyReason = SMR_PER_TRIGGER;
	pParentGroup->QueryChanges(pChangedSetting->GetMasterBits(), &m_MasterInfo);
	pParentGroup->CommitChanges(&m_MasterInfo);
}


GROUPID CSettingsMaster::PushBackGroup(CSettingsGroup* pGroup, const char* szGroupName,
									   TGroupInfo* pParentGroupInfo)
{
	TGroupInfo* pGroupInfo = new TGroupInfo;

	pGroupInfo->pGroup				= pGroup;
	pGroupInfo->nGroupId			= m_vGroups.size();
	pGroupInfo->GroupName			= szGroupName;
	pGroupInfo->pNotifyFunc			= NULL;
	pGroupInfo->pParentGroupInfo	= pParentGroupInfo;
	pGroupInfo->pMaster				= this;

	m_vGroups.push_back(pGroupInfo);

	pGroup->SetConditionVector(m_IniChanger);
	pGroup->SetNotifyFunc(GenericGroupCallback, pGroupInfo);

	return pGroupInfo->nGroupId;
}


CSettingsGroup* CSettingsMaster::GetGroupById(GROUPID nGroupId)
{
	ASSERT(nGroupId >= 0 && nGroupId < m_vGroups.size());

	// Should not be NULL unless the group is deleted
	ASSERT(m_vGroups[nGroupId]->pGroup != NULL);

	return m_vGroups[nGroupId]->pGroup;
}


static TSMNotifyFunc* GetNotifyFunction(TGroupInfo* pGroupInfo)
{
	if (pGroupInfo->pNotifyFunc != NULL)
	{
		return pGroupInfo->pNotifyFunc;
	}

	if (pGroupInfo->pParentGroupInfo != NULL)
	{
		GetNotifyFunction(pGroupInfo->pParentGroupInfo);
	}

	return NULL;
}


static void GenericGroupCallback(int nEvent, void* pParam, void* pContextPtr)
{
	TGroupInfo* pGroupInfo = (TGroupInfo*)pContextPtr;

	if (nEvent == SGE_GROUP_DELETING)
	{
		ASSERT(pGroupInfo->pGroup == pParam);
		pGroupInfo->pGroup = NULL;
		return;
	}

	TSMNotifyFunc* pNotifyFunc = GetNotifyFunction(pGroupInfo);

	if (nEvent == SGE_PRECHANGE || nEvent == SGE_UPDATE_DEFAULTS)
	{
		CSettingWrapper* pSetting = (CSettingWrapper*)pParam;

		if (pSetting->m_fFlags & CSettingWrapper::SETF_CHANNEL_SETTING)
		{
			if (nEvent == SGE_PRECHANGE)
			{
				DoChannelPrechange();
			}
			return;
		}

		if (pNotifyFunc == NULL)
		{
			return;
		}

		if (nEvent == SGE_PRECHANGE)
		{
			if (!(pNotifyFunc)(SME_PRECHANGE, g_nNotifyReason, (DWORD)pSetting->m_pActualSetting,
				pSetting->GetQueriedValue(), pSetting->GetCommittedValue()))
			{
				pSetting->m_fFlags |= CSettingWrapper::SETF_NO_ONCHANGE;
			}
		}
		else
		{
			(pNotifyFunc)(SME_UPDATE_DEFAULTS, g_nNotifyReason, (DWORD)pSetting->m_pActualSetting,
				pSetting->GetQueriedValue(), pSetting->GetCommittedValue());
		}
	}
	else if (nEvent == SGE_GROUP_ACTIVATED || nEvent == SGE_GROUP_DEACTIVATED)
	{
		if (pNotifyFunc == NULL)
		{
			return;
		}

		CSettingsGroup*	pGroup = (CSettingsGroup*)pParam;
		long lConditionValue = 0;

		pGroup->GetConditionBits(&lConditionValue);

		if (nEvent == SGE_GROUP_ACTIVATED)
		{
			(pNotifyFunc)(SME_GROUP_ACTIVATED, g_nNotifyReason, pGroupInfo->nGroupId,
				lConditionValue, 0);
		}
		else
		{
			(pNotifyFunc)(SME_GROUP_DEACTIVATED, g_nNotifyReason, pGroupInfo->nGroupId,
				0, lConditionValue);
		}
	}
	else
	{
		// NEVER_GET_HERE;
		ASSERT(FALSE);
	}
}


// (Many to one bottleneck)
static void ChannelGroupCallback(int nEvent, void* pParam, void* pContextPtr)
{
	if (nEvent == SGE_GROUP_ACTIVATED)
	{

	}
	else if (nEvent == SGE_GROUP_DEACTIVATED)
	{

	}
}


// (Many to one bottleneck)
static void DoChannelPrechange()
{
	// Unused
}


void CSettingsMaster::ChannelSettingOnChange(long NewValue, long OldValue)
{
	// Do stuff here
}


static void ExternalQueryCallback(CSettingWrapper* pSetting, void* pContextPtr)
{
	TGroupInfo* pGroupInfo = (TGroupInfo*)pContextPtr;
	CSettingsMaster* pMaster = pGroupInfo->pMaster;

	pMaster->OnExternalQuery(pSetting, pGroupInfo->pGroup);
}


//--------------------------------------------------------------------
// CSettingWrapper
//--------------------------------------------------------------------

CSettingWrapper::CSettingWrapper(LPSETTING pSetting, PERBITS fPerBits,
								 PERBITS fLockedBits, PERBITS fMasterBits)
{
	m_fPerBits = fPerBits;
	m_fMasterBits = fMasterBits;
	m_fLockedBits = fLockedBits;

	m_fFlags = 0;

	m_lOldValue = 0;
	m_pActualSetting = pSetting;

	m_pHookFunc = NULL;
}


CSettingWrapper::~CSettingWrapper()
{
	if (m_pHookFunc != NULL)
	{
		m_pActualSetting->HookOnChange(NULL, NULL);
	}
}


BOOL CSettingWrapper::QueryChange(PERBITS fChangingBits, void* pTidbit)
{
	TMasterInfo* pMasterInfo = (TMasterInfo*)pTidbit;

	PERBITS fAbsoluteBits = m_fPerBits & pMasterInfo->fUserPerMask | m_fLockedBits;

	if (fChangingBits != PER_QUERYALL && !(fAbsoluteBits & fChangingBits))
	{
		// The change mask doesn't affect this setting
		return FALSE;
	}

	CPerActiveBitsVector* pIniChanger = pMasterInfo->pIniChanger;

	if (m_fFlags & SETF_VALUELOADED)
	{
		if (m_pActualSetting->GetValue() != m_pActualSetting->GetDefault())
		{
			m_pActualSetting->WriteToIni(TRUE);
		}

		m_lOldValue = m_pActualSetting->GetValue();
	}

	char szSection[MAX_INI_SUBSECTION_LENGTH];

	pIniChanger->Describe(fAbsoluteBits, szSection, MAX_INI_SUBSECTION_LENGTH);

	m_pActualSetting->SetSection(szSection);
	m_pActualSetting->ReadFromIni();

	if (m_fFlags & SETF_VALUELOADED)
	{
		if (m_pActualSetting->GetValue() == m_lOldValue)
		{
			return FALSE;
		}
	}

	m_fFlags |= SETF_QUERIED;
	m_fFlags &= ~SETF_NO_ONCHANGE;

	return TRUE;
}


BOOL CSettingWrapper::CommitChange(void* pTidbit)
{
	if (!(m_fFlags & SETF_QUERIED))
	{
		return FALSE;
	}

	if (!(m_fFlags & SETF_NO_ONCHANGE))
	{
		m_pActualSetting->OnChange(m_pActualSetting->GetValue(), m_lOldValue);
	}

	m_fFlags |= SETF_VALUELOADED;
	m_fFlags &= ~SETF_QUERIED;

	return TRUE;
}


long CSettingWrapper::GetQueriedValue()
{
	return m_pActualSetting->GetValue();
}


long CSettingWrapper::GetCommittedValue()
{
	if (m_fFlags & SETF_QUERIED)
	{
		return m_lOldValue;
	}
	return m_pActualSetting->GetValue();
}


void CSettingWrapper::HookExternalChange(ExternalQueryFunc* pFunc, void* pContext)
{
	m_pHookFunc = pFunc;
	m_pHookContext = pContext;

	if (m_pHookFunc != NULL)
	{
		m_pActualSetting->HookOnChange(SettingChangeCallback, this);
	}
	else
	{
		m_pActualSetting->HookOnChange(NULL, NULL);
	}
}


static BOOL SettingChangeCallback(long newValue, long oldValue,
								  BOOL bSuppressOnChange, void* contextPtr)
{
	CSettingWrapper* pChangedSetting = (CSettingWrapper*)contextPtr;

	if (pChangedSetting->m_pHookFunc == NULL)
	{
		return !bSuppressOnChange;
	}

	if (newValue == oldValue)
	{
		return FALSE;
	}

	// Ignore any suppression of master settings
	if (bSuppressOnChange && pChangedSetting->GetMasterBits() == 0)
	{
		// There is no need to do any more because the setting
		// is already set in CSimpleSetting before the hook
		return FALSE;
	}

	// Continue the full query-and-commit path to get
	// PRECHANGE and master related events properly called

	pChangedSetting->m_lOldValue = oldValue;
	pChangedSetting->m_fFlags |= CSettingWrapper::SETF_QUERIED;
	pChangedSetting->m_fFlags &= ~CSettingWrapper::SETF_NO_ONCHANGE;

	(pChangedSetting->m_pHookFunc)
		(pChangedSetting, pChangedSetting->m_pHookContext);

	return FALSE;
}

