/////////////////////////////////////////////////////////////////////////////
// $Id: SettingsMaster2.h,v 1.3 2003-10-27 10:39:54 adcockj Exp $
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
// Revision 1.2  2003/06/15 06:01:35  atnak
// Minor updates
//
// Revision 1.1  2003/06/12 10:28:45  atnak
// Initial release
//
//
//////////////////////////////////////////////////////////////////////////////


/** 
 * @file settingsmaster2.h settingsmaster2 Header file
 */
 
#ifndef __SETTINGSMASTER2_H__
#define __SETTINGSMASTER2_H__

#include "PerSettings.h"
#include "SettingsGroup.h"
#include "Settings.h"

// Temporally define
#define CSettingsMaster CSettingsMaster2

#define SM_INITIAL_GROUPS_BUFFER		10

#define MAX_INI_SUBSECTION_LENGTH		256	

typedef WORD				GROUPID;
typedef CSimpleSetting*		LPSETTING;

enum
{
	// SettingsMaster notify events
	SME_PRECHANGE			= 0,
	SME_UPDATE_DEFAULTS,
	SME_GROUP_ACTIVATED,
	SME_GROUP_DEACTIVATED,

	// SettingsMaster notify reasons
	SMR_PER_TRIGGER			= 0,
	SMR_SET_VALUE,
	SMR_READY_GROUP,
	SMR_DELETE_GROUP,
};


typedef class CSettingsMaster CSettingsMaster;

typedef BOOL (TSMNotifyFunc)(int nEventId, int nReason, DWORD dwParam,
							 long newValue, long oldValue);

namespace smInternal
{
	//  Internal datastructures used by SettingsMaster

	typedef class CSettingWrapper CSettingWrapper;
	typedef void (ExternalQueryFunc)(CSettingWrapper*, void* pContextPtr);

	class CSettingWrapper : public CPerSetting
	{
	public:
		CSettingWrapper(LPSETTING, PERBITS fPer, PERBITS fLocked, PERBITS fMaster);
		virtual ~CSettingWrapper();

		BOOL QueryChange(PERBITS fChangingBits, void* pTidbit);
		BOOL CommitChange(void* pTidbit);

		long GetQueriedValue();
		long GetCommittedValue();

		void HookExternalChange(ExternalQueryFunc* pFunc, void* pContext);

	public:
		// Bits for m_fFlags
		enum
		{
			SETF_VALUELOADED,
			SETF_QUERIED,
			SETF_NO_ONCHANGE,
			SETF_CHANNEL_SETTING,
		};

		UCHAR		m_fFlags;
		PERBITS		m_fLockedBits;

		long		m_lOldValue;
		LPSETTING	m_pActualSetting;

		ExternalQueryFunc*	m_pHookFunc;
		void*				m_pHookContext;
	};


	typedef struct tagGroupInfo
	{
		CSettingsGroup*			pGroup;
		GROUPID					nGroupId;
		std::string				GroupName;
		TSMNotifyFunc*			pNotifyFunc;
		tagGroupInfo*			pParentGroupInfo;
		CSettingsMaster*		pMaster;
	} TGroupInfo;


	typedef struct tagMasterInfo
	{
		PERBITS					fUserPerMask;
		CPerActiveBitsVector*	pIniChanger;
	} TMasterInfo;
};


class CSettingsMaster
{
public:
	CSettingsMaster();
	virtual ~CSettingsMaster();

	GROUPID CreateSettingsGroup(GROUPID nParentID, const char* name = "");
	GROUPID CreateSettingsGroup(const char* name = "");

	void DeleteGroup(GROUPID nID);

	void SetGroupCondition(GROUPID nID, PERSETTING nBit, long nMatch);
	void SetGroupCondition(PERSETTING nBit, long nMatch);

	void RemoveGroupCondition(GROUPID nID);
	void RemoveGroupCondition();

	GROUPID SetImplicitGroup(GROUPID nID);
	PERBITS SetImplicitPer(PERBITS fBits);
	PERBITS SetImplicitLockedPer(PERBITS fBits);

	GROUPID ResetImplicitGroup();
	PERBITS ResetImplicitPer();
	PERBITS ResetImplicitLockedPer();

	void AddSetting(GROUPID, LPSETTING, PERBITS fPerBits = 0, PERBITS fLockedBits = 0, PERBITS fMasterBits = 0);
	void AddSetting(LPSETTING, PERBITS fPerBits = 0, PERBITS fLockedBits = 0, PERBITS fMasterBits = 0);

	void AddMasterSetting(GROUPID, LPSETTING, PERBITS fMasterBits, PERBITS fPerBits = 0, PERBITS fLockedBits = 0);
	void AddMasterSetting(LPSETTING, PERBITS fMasterBits, PERBITS fPerBits = 0, PERBITS fLockedBits = 0);

	// This needs to be properly called at least once per source
	// for channels to work properly on that source.
	void AddChannelCondition(GROUPID, PERSETTING nBit, long lMatch);
	void AddChannelCondition(PERSETTING nBit, long lMatch);

	void SetNotifyFunction(GROUPID, TSMNotifyFunc*);
	void SetNotifyFunction(TSMNotifyFunc*);

	// Call this after setting up a group.  Only needs to be called
	// once on the parent group.  Subgroups are considered.
	void ReadyGroup(GROUPID nID);
	void ReadyGroup();

	// Used to enable or disable unlocked per bits
	void EnableUserPerBits(PERBITS fUserBits);
	void DisableUserPerBits(PERBITS fUserBits);

	void ChangeChannel(long lValue);

public:
	void OnExternalQuery(smInternal::CSettingWrapper* pChangedSetting,
		CSettingsGroup* pParentGroup);

protected:
	typedef smInternal::TGroupInfo			TGroupInfo;
	typedef smInternal::TMasterInfo			TMasterInfo;

protected:
	GROUPID PushBackGroup(CSettingsGroup*, const char*, TGroupInfo* pParentGroupInfo);
	CSettingsGroup* GetGroupById(GROUPID groupID);

private:
	GROUPID					m_ImplicitGroupID;
	PERBITS					m_ImplicitPerBits;
	PERBITS					m_ImplicitLockedBits;

	CSettingsGroup*			m_pMasterGroup;

	vector<TGroupInfo*>		m_vGroups;
	TMasterInfo				m_MasterInfo;

	CPerActiveBitsVector*	m_IniChanger;

	DEFINE_SLIDER_CALLBACK_SETTING(CSettingsMaster, ChannelSetting);
};


void	InitializeSettingsMaster		(void);
void	CleanupSettingsMaster			(void);

extern CSettingsMaster* g_pSettingMaster;

#define	SMCreateSettingsGroup		g_pSettingMaster->CreateSettingsGroup
#define SMSetGroupCondition			g_pSettingMaster->SetGroupCondition
#define SMRemoveGroupCondition		g_pSettingMaster->RemoveGroupCondition

#define	SMSetImplicitGroup			g_pSettingMaster->SetImplicitGroup
#define SMSetImplicitPer			g_pSettingMaster->SetImplicitPer
#define SMSetImplicitLockedPer		g_pSettingMaster->SetImplicitLockedPer

#define	SMResetImplicitGroup		g_pSettingMaster->ResetImplicitGroup
#define SMResetImplicitPer			g_pSettingMaster->ResetImplicitPer
#define SMResetImplicitLockedPer	g_pSettingMaster->ResetImplicitLockedPer

#define	SMAddSetting				g_pSettingMaster->AddSetting
#define SMAddMasterSetting			g_pSettingMaster->AddMasterSetting
#define SMAddChannelCondition		g_pSettingMaster->AddChannelCondition

#define SMSetNotifyFunction			g_pSettingMaster->SetNotifyFunction

#define SMReadyGroup				g_pSettingMaster->ReadyGroup

#define SMEnableUserPerBits			g_pSettingMaster->EnableUserPerBits
#define SMDisableUserPerBits		g_pSettingMaster->DisableUserPerBits


#endif

