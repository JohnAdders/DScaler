/////////////////////////////////////////////////////////////////////////////
// $Id: SettingGroup.cpp,v 1.6 2004-08-20 09:16:19 atnak Exp $
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
// Revision 1.5  2004/08/20 07:30:31  atnak
// Added title value to groups plus other changes.
//
// Revision 1.4  2004/08/14 13:45:23  adcockj
// Fixes to get new settings code working under VS6
//
// Revision 1.3  2004/08/13 08:52:30  atnak
// Added a function for getting a setting's title.
//
// Revision 1.2  2004/08/12 14:03:42  atnak
// Added dependency mask setting and getting functions.
//
// Revision 1.1  2004/08/06 17:12:10  atnak
// Setting repository initial upload.
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingGroup.h"
#include "SettingRepository.h"
#include "SettingObject.h"
#include "SettingValue.h"
#include "SettingKey.h"
#include "SettingConfig.h"
#include <stdio.h>
#include <vector>
#include <list>


//////////////////////////////////////////////////////////////////////////
// CSettingGroup
//////////////////////////////////////////////////////////////////////////

CSettingGroup::CSettingGroup(IN std::string section, IN PSETTINGREPOSITORY repository) :
	m_repository(repository),
	m_mutexEnterCount(0),
	m_notifyProc(NULL)
{
	m_section = section;
	InitializeLocking();
}


CSettingGroup::~CSettingGroup()
{
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		if ((*it)->key != NULL)
		{
			(*it)->key->SetController(NULL, NULL);
		}
		delete (*it);
	}
	CleanupLocking();
}


HSETTING CSettingGroup::AddSetting(IN PSETTINGOBJECT object, IN PSETTINGKEY key)
{
	if (object == NULL)
	{
		return NULL;
	}

	PSETTINGINFO info = new CSettingInfo(key, object);
	if (info == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	EnterLock();
	AddSetting(info);
	LeaveLock();
	return (HSETTING)info;
}


HSETTING CSettingGroup::AddSetting(IN std::string title, IN PSETTINGOBJECT object, IN PSETTINGKEY key)
{
	if (object == NULL)
	{
		return NULL;
	}

	PSETTINGINFO info = new CSettingInfo(key, object, title);
	if (info == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	EnterLock();
	AddSetting(info);
	LeaveLock();
	return (HSETTING)info;
}


void CSettingGroup::SetNotifyProc(IN PSETTINGGROUP_NOTIFYPROC proc, IN PVOID context)
{
	EnterLock();
	m_notifyProc = proc;
	m_callbackContext = context;
	LeaveLock();
}


void CSettingGroup::LoadSettings()
{
	BULKCHANGELIST bulklist;

	EnterLock();

	// Let all settings in the group be CHANGING notified and loaded.
	LoadBulkSettings(&bulklist);

	// Have changed notification called on all that changed.
	CommonPostChangeRoutine(&bulklist);
	LeaveLock();
}


void CSettingGroup::SaveSettings()
{
	EnterLock();
	// Run through the settings list and save each one
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		(*it)->object->Save(m_repository, m_section.c_str());
	}
	LeaveLock();
}


void CSettingGroup::LoadSetting(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	CCINFO ccinfo;

	EnterLock();
	// Notification setup and generation is taken care of by Common* functions
	CommonPreChangeRoutine(info, &ccinfo);

	if (info->object->Load(m_repository, m_section.c_str(), CommonSettingCheckProc, (PVOID)&ccinfo))
	{
		CommonPostChangeRoutine(&ccinfo);
	}
	LeaveLock();
}


void CSettingGroup::Silence(IN BOOL silence)
{
	EnterLock();
	m_silent = silence;
	LeaveLock();
}


void CSettingGroup::ReviseSettings()
{
	BULKCHANGELIST bulklist;

	EnterLock();
	// Run through the settings and notify SETTING for each one
	ReviseBulkSettings(&bulklist);
	// Notify the SET on all that was SETTING notified.
	CommonPostChangeRoutine(&bulklist);
	LeaveLock();
}


PSETTINGKEY CSettingGroup::GetSettingKey(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	return info->key;
}


std::string CSettingGroup::GetSettingTitle(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	std::string title;

	EnterLock();
	if (info->title != "")
	{
		title = info->title;
	}
	else
	{
		title = info->object->GetTitle();
	}
	LeaveLock();
	return title;
}


BOOL CSettingGroup::IsChangedPending(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;

	EnterLock();
	BOOL pending = GetInfoFlag(info, FLAG_CHANGING);
	LeaveLock();
	return pending;
}


BOOL CSettingGroup::SilenceChangedPending(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;

	EnterLock();
	BOOL pending = GetInfoFlag(info, FLAG_CHANGING);
	if (pending)
	{
		SetInfoFlag(info, FLAG_CHANGING, FALSE);
	}
	LeaveLock();
	return pending;
}


void CSettingGroup::SaveSetting(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	EnterLock();
	info->object->Save(m_repository, m_section.c_str());
	LeaveLock();
}


void CSettingGroup::SetValue(IN HSETTING setting, IN RCSETTINGVALUE value)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	CCINFO ccinfo;

	EnterLock();
	// Notification setup and generation is taken care of by Common* functions
	CommonPreChangeRoutine(info, &ccinfo);

	if (info->object->SetValue(value, CommonSettingCheckProc, (PVOID)&ccinfo))
	{
		CommonPostChangeRoutine(&ccinfo);
	}
	LeaveLock();
}


CSettingValue CSettingGroup::GetValue(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	EnterLock();
	CSettingValue value = info->object->GetValue();
	LeaveLock();
	return value;
}


void CSettingGroup::UseDefault(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	CCINFO ccinfo;

	EnterLock();
	// Notification setup and generation is taken care of by Common* functions
	CommonPreChangeRoutine(info, &ccinfo);

	if (info->object->UseDefault(CommonSettingCheckProc, (PVOID)&ccinfo))
	{
		CommonPostChangeRoutine(&ccinfo);
	}
	LeaveLock();
}


void CSettingGroup::SetDefault(IN HSETTING setting, IN RCSETTINGVALUE value)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	CCINFO ccinfo;

	EnterLock();
	// Notification setup and generation is taken care of by Common* functions
	CommonPreChangeRoutine(info, &ccinfo);

	if (info->object->SetDefault(value, CommonSettingCheckProc, (PVOID)&ccinfo))
	{
		CommonPostChangeRoutine(&ccinfo);
	}
	LeaveLock();
}


CSettingValue CSettingGroup::GetDefault(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	EnterLock();
	CSettingValue value = info->object->GetDefault();
	LeaveLock();
	return value;
}


void CSettingGroup::CheckLimiter(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	CCINFO ccinfo;

	EnterLock();
	// Notification setup and generation is taken care of by Common* functions
	CommonPreChangeRoutine(info, &ccinfo);

	if (info->object->CheckLimiter(CommonSettingCheckProc, (PVOID)&ccinfo))
	{
		CommonPostChangeRoutine(&ccinfo);
	}
	LeaveLock();
}


void CSettingGroup::SetLimiter(IN HSETTING setting, IN PSETTINGLIMITER limiter)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	CCINFO ccinfo;

	EnterLock();
	// Notification setup and generation is taken care of by Common* functions
	CommonPreChangeRoutine(info, &ccinfo);

	if (info->object->SetLimiter(limiter, CommonSettingCheckProc, (PVOID)&ccinfo))
	{
		CommonPostChangeRoutine(&ccinfo);
	}
	LeaveLock();
}


PSETTINGLIMITER CSettingGroup::GetLimiter(IN HSETTING setting)
{
	PSETTINGINFO info = (PSETTINGINFO)setting;
	EnterLock();
	PSETTINGLIMITER limiter= info->object->GetLimiter();
	LeaveLock();
	return limiter;
}


void CSettingGroup::AddSetting(IN PSETTINGINFO info)
{
	m_settingList.push_back(info);
	if (info->key != NULL)
	{
		info->key->SetController(this, (PVOID)info);
	}
}


void CSettingGroup::LoadBulkSettings(PBULKCHANGELIST bulklist)
{
	CCINFO ccinfo;

	// Initialize the buffer that will be given to the callback
	ccinfo.group = this;

	// For bulk, first notify all pre-change and change the values.  Once all values
	// changes have been finalized and applied, CHANGED notification will be called
	// on all that changed.
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		PSETTINGINFO info = *it;

		// Initialize the buffer that will be given to the callback
		ccinfo.info = info;

		// Load a the setting from the repository
		if (info->object->Load(m_repository, m_section.c_str(), CommonSettingCheckProc, (PVOID)&ccinfo))
		{
			// Record the change
			bulklist->push_back(ccinfo);
		}
	}
}


void CSettingGroup::ReviseBulkSettings(IN PBULKCHANGELIST bulklist)
{
	CCINFO ccinfo;
	ccinfo.group = this;

	// Run through the settings list and notify each one
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		PSETTINGINFO info = *it;
		BOOL change;

		if (!info->object->IsSet())
		{
			continue;
		}

		// Get the current value.
		ccinfo.oldValue = info->object->GetValue();

		if (GetInfoFlag(ccinfo.info, FLAG_CHANGING))
		{
			// If CHANGING notification was already sent, send RECHANGING.
			change = NotifyRechanging(info, ccinfo.oldValue, ccinfo.oldValue);
		}
		else
		{
			// Set the initial flag so SETTING will be generated.
			SetInfoFlag(info, FLAG_INITIAL, TRUE);

			// Generate a SETTING notification.
			change = NotifyChanging(info, ccinfo.oldValue, ccinfo.oldValue);
			SetInfoFlag(info, FLAG_CHANGING, change);
		}

		if (change)
		{
			ccinfo.info = info;
			bulklist->push_back(ccinfo);
		}
	}
}


void CSettingGroup::InitializeLocking()
{
#ifdef SETTINGGROUP_THREAD_SAFE
	InitializeCriticalSection(&m_groupAccessMutex);
#endif
}


void CSettingGroup::CleanupLocking()
{
#ifdef SETTINGGROUP_THREAD_SAFE
	DeleteCriticalSection(&m_groupAccessMutex);
#endif
}


void CSettingGroup::EnterLock(IN UINT iterations)
{
#ifdef SETTINGGROUP_THREAD_SAFE
	for (UINT i = 0; i < iterations; i++)
	{
		EnterCriticalSection(&m_groupAccessMutex);
		m_mutexEnterCount++;
	}
#endif
}


UINT CSettingGroup::LeaveLock(IN BOOL iterate)
{
#ifdef SETTINGGROUP_THREAD_SAFE
	if (iterate)
	{
		UINT iterations = m_mutexEnterCount;
		m_mutexEnterCount = 0;

		for (UINT i = 0; i < iterations; i++)
		{
			LeaveCriticalSection(&m_groupAccessMutex);
		}
		return iterations;
	}

	m_mutexEnterCount--;
	LeaveCriticalSection(&m_groupAccessMutex);
	return 1;
#else
	return 0;
#endif
}


void CSettingGroup::SetInfoFlag(IN PSETTINGINFO info, IN BYTE flags, IN BOOL set)
{
	if (set)
	{
		info->flags |= flags;
	}
	else
	{
		info->flags &= ~flags;
	}
}


BOOL CSettingGroup::GetInfoFlag(IN PSETTINGINFO info, IN BYTE flags, IN BOOL strict)
{
	if (strict)
	{
		// If strict, make sure all the flags are set.
		return (info->flags & flags) == flags;
	}
	return (info->flags & flags) != 0;
}


void CSettingGroup::CommonPreChangeRoutine(IN PSETTINGINFO info, IN PCCINFO ccinfo)
{
	// Fill ccinfo buffer with information that
	// will be needed by CommonSettingCheckProc.
	ccinfo->group	= this;
	ccinfo->info	= info;
}


void CSettingGroup::CommonPostChangeRoutine(IN PCCINFO ccinfo)
{
	// Have changed notification called.
	CompletePostChange(ccinfo);
	// Notify that all notifications have been sent.
	NotifyAfterBulk();
}


void CSettingGroup::CommonPostChangeRoutine(IN PBULKCHANGELIST bulklist)
{
	// Call CHANGED or SET notification on all that changed
	BULKCHANGELIST::iterator it = bulklist->begin();
	BULKCHANGELIST::iterator ti = bulklist->end();
	for ( ; it != ti; it++)
	{
		// Have changed notification called.
		CompletePostChange(&*it);
	}
	// Notify that all notifications have been sent.
	NotifyAfterBulk();
}


void CSettingGroup::CompletePostChange(IN PCCINFO ccinfo)
{
	// Stop here if the changing flag is already removed.
	if (!GetInfoFlag(ccinfo->info, FLAG_CHANGING))
	{
		return;
	}

	// Remove the changing flag before notifying CHANGED.
	SetInfoFlag(ccinfo->info, FLAG_CHANGING, FALSE);

	// Send the post change notification
	ccinfo->group->NotifyChanged(ccinfo->info,
		ccinfo->info->object->GetValue(), ccinfo->oldValue);
}


//
// Changing operations (SetValue(), LoadSetting(), etc.) called from inside a
// notify callback is treated just like a normal call.  Their processing is done
// then and there and they generate CHANGING and CHANGED notifications as normal.
//
// However, an exception to this is when the changing operation modifies a
// setting with an outstanding CHANGED notification.  That is, a CHANGING
// notification was sent for the setting and approved by the callback but the
// CHANGED notification is yet to be sent.  In this case, a RECHANGING
// notification is sent instead of the usual CHANGING and CHANGED pair.  If the
// RECHANGING notification is approved, a CHANGED notification will be sent.  If
// it is not approved, a ChANGED notification will still be sent after the notify
// callback returns, to match the original CHANGING notification.  The full
// sequence of notifications that include a RECHANGING will always look like
// CHANGING -> RECHANGING -> CHANGED.
//
// After the notify callback returns, the processs that was occuring before the
// notification is continued as normal.  (However, any RECHANGING notifications
// that was proceeded with a CHANGED notification will remove the old CHANGED
// notifications that were pending.)
//
// Example 1: (LoadSettings() a group of three settings)
//
// P1 ->
//    <- T
// P2 ->
//    <- T
// P3 ->
//    <- T
// C1, C2, C3
//
// Pn	= CHANGING notification.
// Cn	= CHANGED notification.
// <- T	= Notify returns with approval.
//
// Example 2: (Same as 1 but the P2 callback calls LoadSettings() again.
//             --The first time only so no infinite recursion.)
//
// P1 ->
//    <- T
// P2 -> R1 ->
//          <- T
//       P2 ->
//          <- T
//       P3 ->
//          <- T
//       C1, C2, C3
//    <- T
// P3 ->
//    <- T
// C2, C3
//
// Rn	= RECHANGING notification.
//
// Notice that the original LoadSettings() has the opportunity to
// override any changes made by the nested LoadSettings() for the
// settings 2 and 3.  Also notice that since P2 had not returned
// at the stage where the nested LoadSettings() is called, it is
// a P2 followed by C2 instead of an R2 followed by C2.
// 

BOOL CSettingGroup::CommonSettingCheckProc(RCSETTINGVALUE newValue,
										   RCSETTINGVALUE oldValue,
										   PCSETTINGOBJECT object,
										   PVOID context)
{
	// Don't let the setting change if the values don't change
	if (object->IsSet() && newValue.IsEqual(oldValue))
	{
		return FALSE;
	}

	PCCINFO ccinfo = (PCCINFO)context;
	BOOL change;

	if (ccinfo->group->GetInfoFlag(ccinfo->info, FLAG_CHANGING))
	{
		// If CHANGING notification was already sent, send RECHANGING.
		change = ccinfo->group->NotifyRechanging(ccinfo->info, newValue, oldValue);
	}
	else
	{
		// Set whether this is the initial value.
		ccinfo->group->SetInfoFlag(ccinfo->info, FLAG_INITIAL, !object->IsSet());

		// Generate a CHANGING notification.
		change = ccinfo->group->NotifyChanging(ccinfo->info, newValue, oldValue);
		ccinfo->group->SetInfoFlag(ccinfo->info, FLAG_CHANGING, change);
	}

	if (change)
	{
		// Save the old value.
		ccinfo->oldValue = oldValue;
	}

	return change;
}


BOOL CSettingGroup::NotifyChanging(IN PSETTINGINFO info,
								   IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	INT message = GetInfoFlag(info, FLAG_INITIAL) ? NOTIFY_VALUE_SETTING : NOTIFY_VALUE_CHANGING;
	return Notify(info, message, newValue, oldValue);
}


BOOL CSettingGroup::NotifyRechanging(IN PSETTINGINFO info,
								   IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	INT message = GetInfoFlag(info, FLAG_INITIAL) ? NOTIFY_VALUE_RESETTING : NOTIFY_VALUE_RECHANGING;
	return Notify(info, message, newValue, oldValue);
}


BOOL CSettingGroup::NotifyChanged(IN PSETTINGINFO info,
								  IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	INT message = GetInfoFlag(info, FLAG_INITIAL) ? NOTIFY_VALUE_SET : NOTIFY_VALUE_CHANGED;
	return Notify(info, message, newValue, oldValue);
}


BOOL CSettingGroup::NotifyAfterBulk()
{
	return Notify(NULL, NOTIFY_AFTER_BULK, CSettingValue(), CSettingValue());
}


BOOL CSettingGroup::Notify(IN PSETTINGINFO info, IN INT message,
						   IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	if (m_silent)
	{
		return TRUE;
	}

	UINT lockLeaveCount = 0;
	BOOL response = TRUE;

	// First call the global notification callback if one exists
	if (m_notifyProc != NULL)
	{
		// All iterations of the lock MUST be removed before entering the
		// notification callback.  It's otherwise too easy to create
		// deadlocking conditions in DScaler's current code.  No protection
		// is necessary during the callback time because there is another
		// synchronization method (RECHANGING) is used for changes that
		// occur during this time.  Hopefully, m_notifyProc, info and
		// newValue will not change during the unsafe window created here.
		lockLeaveCount = LeaveLock(TRUE);

		response = (m_notifyProc)(message, newValue, oldValue,
			(HSETTING)info, m_callbackContext);

		if (response == NOTIFY_REPLY_DONT_NOTIFY)
		{
			EnterLock(lockLeaveCount);
			return TRUE;
		}
		if (response == NOTIFY_REPLY_DONT_CHANGE)
		{
			EnterLock(lockLeaveCount);
			return FALSE;
		}
	}

	if (info != NULL)
	{
		if (lockLeaveCount == 0)
		{
			lockLeaveCount = LeaveLock(TRUE);
		}

		// Call the notification callback of this setting object
		if (!info->object->Notify(message, newValue, oldValue))
		{
			// EnterLock();
			return FALSE;
		}

		// Call the notification callback of this setting key
		if (info->key != NULL)
		{
			response = info->key->Notify(message, newValue, oldValue);
		}
	}

	// Reenter all iterations of the lock that was left above.
	EnterLock(lockLeaveCount);
	return response;
}


//////////////////////////////////////////////////////////////////////////
// CSettingGroup::CSettingInfo
//////////////////////////////////////////////////////////////////////////

CSettingGroup::CSettingInfo::CSettingInfo(IN PSETTINGKEY key,
										  IN PSETTINGOBJECT object, IN std::string title) :
	key(key),
	object(object),
	title(title),
	flags(0)
{
}


CSettingGroup::CSettingInfo::~CSettingInfo()
{
	delete object;
}


//////////////////////////////////////////////////////////////////////////
// CSettingGroupEx
//////////////////////////////////////////////////////////////////////////

CSettingGroupEx::CSettingGroupEx(IN std::string section, IN PSETTINGREPOSITORY repository,
								 IN PSETTINGGROUPEX parent) :
	CSettingGroup(section, repository),
	m_suspended(FALSE),
	m_haveDependeeBits(0),
	m_parentGroup(parent)
{
	if (parent != NULL)
	{
		// Take the parent's dependency gestalt object
		m_dependencyGestalt = parent->m_dependencyGestalt;
	}
	else
	{
		// Create a dependency gestalt object
		m_dependencyGestalt = new CDependencyGestalt();
		if (m_dependencyGestalt == NULL)
		{
			OUT_OF_MEMORY_ERROR;
		}
	}
}


CSettingGroupEx::~CSettingGroupEx()
{
	// Delete all subgroups
	SUBGROUPEXLIST::iterator it = m_subgroupList.begin();
	SUBGROUPEXLIST::iterator ti = m_subgroupList.end();
	for ( ; it != ti; it++)
	{
		delete (*it);
	}
	m_subgroupList.clear();

	if (m_parentGroup == NULL)
	{
		delete m_dependencyGestalt;
	}
}


DBIT CSettingGroupEx::CreateDependency(IN std::string title, IN std::string section)
{
	EnterLock();
	DBIT dependeeBit = m_dependencyGestalt->CreateDependee(title, section);
	LeaveLock();
	return dependeeBit;
}


HSETTING CSettingGroupEx::AddSetting(IN PSETTINGOBJECT object, IN PSETTINGKEY key,
									 IN BYTE dependantOptionalBits, IN BYTE dependantAbsoluteBits)
{
	if (object == NULL)
	{
		return NULL;
	}

	PSETTINGINFOEX info = new CSettingInfoEx(key, object);
	if (info == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}

	// Assert that there're no stray bits.
	ASSERT(!(dependantOptionalBits & ~m_dependencyGestalt->GetValidDependeeBits()));
	ASSERT(!(dependantAbsoluteBits & ~m_dependencyGestalt->GetValidDependeeBits()));

	info->dependantOptionalBits = dependantOptionalBits;
	info->dependantAbsoluteBits = dependantAbsoluteBits;
	EnterLock();
	CSettingGroup::AddSetting(info);
	LeaveLock();
	return (HSETTING)info;
}


HSETTING CSettingGroupEx::AddSetting(IN std::string title, IN PSETTINGOBJECT object, IN PSETTINGKEY key,
									 IN BYTE dependantOptionalBits, IN BYTE dependantAbsoluteBits)
{
	EnterLock();
	PSETTINGINFOEX info = (PSETTINGINFOEX)AddSetting(object, key,
		dependantOptionalBits, dependantAbsoluteBits);

	if (info != NULL)
	{
		info->title = title;
	}
	LeaveLock();
	return (HSETTING)info;
}


HSETTING CSettingGroupEx::AddMaster(IN PSETTINGOBJECT object, IN PSETTINGKEY key, IN BYTE dependeeBits,
									IN BYTE dependantOptionalBits, IN BYTE dependantAbsoluteBits)
{
	if (object == NULL)
	{
		return NULL;
	}

	PSETTINGINFOEX info = new CSettingInfoEx(key, object);
	if (info == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}

	EnterLock();

	// Assert that there're no stray bits.
	ASSERT(!(dependeeBits & ~m_dependencyGestalt->GetValidDependeeBits()));
	ASSERT(!(dependantOptionalBits & ~m_dependencyGestalt->GetValidDependeeBits()));
	ASSERT(!(dependantAbsoluteBits & ~m_dependencyGestalt->GetValidDependeeBits()));

	m_haveDependeeBits |= dependeeBits;

	info->dependeeBit = dependeeBits;
	info->dependantOptionalBits = dependantOptionalBits;
	info->dependantAbsoluteBits = dependantAbsoluteBits;
	CSettingGroup::AddSetting(info);
	LeaveLock();
	return (HSETTING)info;
}


HSETTING CSettingGroupEx::AddMaster(IN std::string title, IN PSETTINGOBJECT object, IN PSETTINGKEY key,
									IN BYTE dependeeBits, IN BYTE dependantOptionalBits,
									IN BYTE dependantAbsoluteBits)
{
	EnterLock();
	PSETTINGINFOEX info = (PSETTINGINFOEX)AddMaster(object, key, 
		dependeeBits, dependantOptionalBits, dependantAbsoluteBits);

	if (info != NULL)
	{
		info->title = title;
	}
	LeaveLock();
	return (HSETTING)info;
}



void CSettingGroupEx::SaveSettings()
{
	std::string cacheString;
	DBIT cacheBits = 0;

	EnterLock();
	// Run through the settings list and save each one
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		PSETTINGINFOEX info = dynamic_cast<PSETTINGINFOEX>(*it);
		info->object->Save(m_repository, GetSection(info->loadedDependantBits,
			TRUE, &cacheString, &cacheBits));
	}

	// Run through all subgroups and have them save all settings too.
	SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
	SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
	for ( ; sgit != sgti; sgit++)
	{
		// Entering the lock multiple times should not be a problem.
		(*sgit)->SaveSettings();
	}
	LeaveLock();
}


void CSettingGroupEx::SaveSetting(IN HSETTING setting)
{
	PSETTINGINFOEX info = (PSETTINGINFOEX)setting;

	std::string cacheString;
	DBIT cacheBits = 0;

	EnterLock();
	info->object->Save(m_repository, GetSection(info->loadedDependantBits,
		TRUE, &cacheString, &cacheBits));
	LeaveLock();
}


void CSettingGroupEx::ReviseSettings(IN BOOL includeSubgroups)
{
	BULKCHANGELIST bulklist;

	EnterLock();
	// Run through the settings and notify SETTING on each one
	ReviseBulkSettings(includeSubgroups, &bulklist);
	// Notify the SET on all that was SETTING notified.
	CSettingGroup::CommonPostChangeRoutine(&bulklist);
	LeaveLock();
}


void CSettingGroupEx::SaveOptionalDependencies()
{
	EnterLock();
	// Create a section string that is m_section appended with DEPENDANTSECTIONPOSTFIX.
	std::string section(m_section);
	section += DEPENDANTSECTIONPOSTFIX;

	const DWORD kBufferSize = 128;
	CHAR buffer[kBufferSize];

	// First save the optional dependants mask.
	sprintf(buffer, "%x", (INT)m_dependencyGestalt->GetDependantMask());
	m_repository->SaveSettingString(section.c_str(), DEPENDANTMASKSAVEKEY, buffer);

	// Save the optional dependant bits for every setting in the group and subgroups.
	_SaveOptionalDependencies(section.c_str(), buffer, kBufferSize);
	LeaveLock();
}


void CSettingGroupEx::_SaveOptionalDependencies(IN LPCSTR section, IN LPSTR buffer,
												IN DWORD bufferSize)
{
	LPCSTR key;
	ASSERT(bufferSize >= sizeof(DBIT)*2+1);

	// The buffer and bufferSize is passed into each call so that it does not
	// need to be taken off the stack for each recursion.

	// Run through the settings list and load the optional dependant bits.
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		PSETTINGINFOEX info = dynamic_cast<PSETTINGINFOEX>(*it);

		if (!GetInfoFlag(info, FLAG_OPTDEPCHANGED) || (key = info->object->GetKey()) == NULL)
		{
			continue;
		}

		sprintf(buffer, "%x", (INT)info->dependantOptionalBits);
		m_repository->SaveSettingString(section, key, buffer);
		SetInfoFlag(info, FLAG_OPTDEPCHANGED, FALSE);
	}

	// Run through all subgroups and have them save dependant bits too.
	SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
	SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
	for ( ; sgit != sgti; sgit++)
	{
		(*sgit)->_SaveOptionalDependencies(section, buffer, bufferSize);
	}
}


void CSettingGroupEx::LoadOptionalDependencies(IN BOOL suspended)
{
	EnterLock();
	// Create a section string that is m_section appended with DEPENDANTSECTIONPOSTFIX.
	std::string section(m_section);
	section += DEPENDANTSECTIONPOSTFIX;

	const DWORD kBufferSize = 128;
	CHAR buffer[kBufferSize];

	DBIT mask = m_dependencyGestalt->GetValidDependeeBits();

	// First load the optional dependants mask.
	if (m_repository->LoadSettingString(section.c_str(), DEPENDANTMASKSAVEKEY, buffer, kBufferSize))
	{
		m_dependencyGestalt->SetDependantMask((DBIT)(strtoul(buffer, NULL, 16) & mask));
	}

	// Load the optional dependant bits for every setting in the group and subgroups.
	_LoadOptionalDependencies(section.c_str(), mask, buffer, kBufferSize);

	if (!suspended && !m_suspended)
	{
		Activate(FALSE);
	}
	LeaveLock();
}


void CSettingGroupEx::_LoadOptionalDependencies(IN LPCSTR section, IN DBIT validMask,
												IN LPSTR buffer, IN DWORD bufferSize)
{
	LPCSTR key;

	// The buffer and bufferSize is passed into each call so that it does not
	// need to be taken off the stack for each recursion.

	// Run through the settings list and load the optional dependant bits.
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		PSETTINGINFOEX info = dynamic_cast<PSETTINGINFOEX>(*it);

		if ((key = info->object->GetKey()) == NULL)
		{
			continue;
		}
		if (m_repository->LoadSettingString(section, key, buffer, bufferSize))
		{
			info->dependantOptionalBits = (DBIT)(strtoul(buffer, NULL, 16) & validMask);
			SetInfoFlag(info, FLAG_OPTDEPCHANGED, FALSE);
		}
	}

	// Run through all subgroups and have them load dependant bits too.
	SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
	SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
	for ( ; sgit != sgti; sgit++)
	{
		(*sgit)->_LoadOptionalDependencies(section, validMask, buffer, bufferSize);
	}
}


PSETTINGGROUPEX CSettingGroupEx::CreateSubgroup()
{
	EnterLock();
	PSETTINGGROUPEX subgroup = new CSettingGroupEx(m_section, m_repository, this);
	if (subgroup == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	m_subgroupList.push_back(subgroup);
	LeaveLock();
	return subgroup;
}


void CSettingGroupEx::EnableOptionalDependencies(IN DBIT dependeeBit, IN BOOL set, IN BOOL suspended)
{
	EnterLock();
	if (m_parentGroup != NULL)
	{
		LeaveLock();
		// Disallow the changing of the dependant mask from a subgroup.
		return;
	}

	DBIT mask = m_dependencyGestalt->GetDependantMask();
	if (set)
	{
		mask |= dependeeBit;
	}
	else
	{
		mask &= ~dependeeBit;
	}

	BOOL changed = m_dependencyGestalt->SetDependantMask(mask);
	if (changed && !m_suspended && !suspended)
	{
		// Calling Activate() takes care of synchronizing all changes
		// that occur as a result of the dependant mask change.
		Activate(FALSE);
	}
	LeaveLock();
}


void CSettingGroupEx::SetEnabledOptionalDependencies(IN DBIT mask, IN BOOL suspended)
{
	EnterLock();
	if (m_parentGroup != NULL)
	{
		LeaveLock();
		// Disallow the changing of the dependant mask from a subgroup.
		return;
	}

	BOOL changed = m_dependencyGestalt->SetDependantMask(mask);
	if (changed && !m_suspended && !suspended)
	{
		// Calling Activate() takes care of synchronizing all changes
		// that occur as a result of the dependant mask change.
		Activate(FALSE);
	}
	LeaveLock();
}


DBIT CSettingGroupEx::GetEnabledOptionalDependencies()
{
	EnterLock();
	DBIT mask = m_dependencyGestalt->GetDependantMask();
	LeaveLock();
	return mask;
}


void CSettingGroupEx::Suspend(IN BOOL suspend)
{
	EnterLock();
	m_suspended = suspend;
	LeaveLock();
}


BOOL CSettingGroupEx::IsSuspended()
{
	EnterLock();
	BOOL suspended = m_suspended;
	LeaveLock();
	return suspended;
}


void CSettingGroupEx::Activate(IN BOOL unsuspend)
{
	EnterLock();
	if (unsuspend)
	{
		m_suspended = FALSE;
	}

	BULKCHANGELIST bulklist;
	// Synchronize changes that have occurred.
	ProcessVectorChanges(&bulklist, MODE_DEPENDANT_CHANGE);
	// Call changed notification on all that changed
	CSettingGroup::CommonPostChangeRoutine(&bulklist);
	LeaveLock();
}


void CSettingGroupEx::JostleBit(IN DBIT dependeeBit, IN RCSETTINGVALUE dependeeValue,
								IN BOOL suspended)
{
	EnterLock();
	if (!m_dependencyGestalt->RegisterDependeeChange(&m_dependedValues,
		dependeeBit, dependeeValue))
	{
		LeaveLock();
		// Unless a master changes the dependee value vector, there
		// is no need to check other settings.
		return;
	}

	// Don't do anything if the group is suspended.
	if (m_suspended || suspended)
	{
		LeaveLock();
		return;
	}

	BULKCHANGELIST bulklist;

	// Check and process all settings for changes.  Efficiency can be
	// gained by setting the mode to MODE_DEPENDEE_CHECK when it's
	// certain only values have changed.
	JostleEverySetting(&bulklist, MODE_DEPENDEE_CHECK, dependeeBit);

	// Call changed notification on all that changed
	CSettingGroup::CommonPostChangeRoutine(&bulklist);
	LeaveLock();
}



DBIT CSettingGroupEx::GetDependeeBits(IN HSETTING setting)
{
	PSETTINGINFOEX info = (PSETTINGINFOEX)setting;
	EnterLock();
	DBIT bits = info->dependeeBit;
	LeaveLock();
	return bits;
}

DBIT CSettingGroupEx::GetOptionalDependantBits(IN HSETTING setting)
{
	PSETTINGINFOEX info = (PSETTINGINFOEX)setting;
	EnterLock();
	DBIT bits = info->dependantOptionalBits;
	LeaveLock();
	return bits;
}


DBIT CSettingGroupEx::GetAbsoluteDependantBits(IN HSETTING setting)
{
	PSETTINGINFOEX info = (PSETTINGINFOEX)setting;
	EnterLock();
	DBIT bits = info->dependantAbsoluteBits;
	LeaveLock();
	return bits;
}

void CSettingGroupEx::SetOptionalDependantBits(IN HSETTING setting,
											   IN DBIT dependantOptionalBits)
{
	PSETTINGINFOEX info = (PSETTINGINFOEX)setting;
	EnterLock();
	if (info->dependantOptionalBits != dependantOptionalBits)
	{
		SetInfoFlag(info, FLAG_OPTDEPCHANGED, TRUE);
		info->dependantOptionalBits = dependantOptionalBits;
		// Save to the previous section and load from the new section.
		// Any necessary dependency operations will be performed by
		// LoadSettings(). Although it may not be necessary to save to the
		// old section, saving will ensure the setting's value is not lost
		// if the new optional dependency bits value did not actually cause
		// a section change.  (This can happen because of the absolute
		// dependency bits and the optional dependency mask.)
		SaveSetting(setting);
		LoadSetting(setting);
	}
	LeaveLock();
}


PSETTINGCONFIG CSettingGroupEx::CreateAssociationConfig(std::string title, BOOL addSettings)
{
	EnterLock();
	BYTE dependeeCount = m_dependencyGestalt->GetDependeeCount();
	CSettingConfigAssociation* config = new CSettingConfigAssociation(title,
		this, dependeeCount);

	// Add all the dependee titles to the configuration object.
	for (BYTE i = 0; i < dependeeCount; i++)
	{
		config->AddDependency(m_dependencyGestalt->GetDependeeTitle(i));
	}

	// Add all settings of the group and subgroups to the configuration object.
	if (addSettings)
	{
		_CreateAssociationConfig(config);
	}
	LeaveLock();
	return config;
}


void CSettingGroupEx::_CreateAssociationConfig(PSETTINGCONFIG association)
{
	CSettingConfigAssociation* config = (CSettingConfigAssociation*)association;

	// Add all settings with a title to the configuration object.
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		std::string title = GetSettingTitle((HSETTING)(*it));
		if (title != "")
		{
			config->AddConfig(new CSettingConfigDependant(this, (HSETTING)(*it)));
		}
	}

	// Have all subgroup settings added too.
	SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
	SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
	for ( ; sgit != sgti; sgit++)
	{
		(*sgit)->_CreateAssociationConfig(association);
	}
}


void CSettingGroupEx::EnterLock(IN UINT iterations)
{
#ifdef SETTINGGROUP_THREAD_SAFE
	// Lock this group and all subgroups at the same time.
	m_dependencyGestalt->EnterMutexSection(iterations);
#endif
}


UINT CSettingGroupEx::LeaveLock(IN BOOL iterate)
{
#ifdef SETTINGGROUP_THREAD_SAFE
	// Unlock this group and all subgroups at the same time.
	return m_dependencyGestalt->LeaveMutexSection(iterate);
#else
	return 0;
#endif
}


void CSettingGroupEx::CommonPostChangeRoutine(IN PCCINFO ccinfo)
{
	PSETTINGINFOEX info = dynamic_cast<PSETTINGINFOEX>(ccinfo->info);

	// If a master setting changes, a lot more may need to be
	// done before the final CHANGED notification can be sent.
	if (info->dependeeBit != 0)
	{
		// Set the master's dependee bit as changed.
		if (m_dependencyGestalt->RegisterDependeeChange(&m_dependedValues,
			info->dependeeBit, info->object->GetValue()) && !m_suspended)
		{
			BULKCHANGELIST bulklist;

			// Push the setting on as the first setting to be notified.
			bulklist.push_back(*ccinfo);

			// Check and process all settings for changes. 
			JostleEverySetting(&bulklist,
				MODE_DEPENDEE_CHECK, info->dependeeBit, GetDependantBits(info));

			// Call changed notification on all that changed.  All settings this
			// master depends is set checked because those should not change in
			// the course of processing every setting since there should be no
			//cyclic dependencies.
			CSettingGroup::CommonPostChangeRoutine(&bulklist);
			// Return here so the single post change routine isn't done.
			return;
		}
	}

	// Let the old method send the CHANGED notification.
	CSettingGroup::CommonPostChangeRoutine(ccinfo);
}


void CSettingGroupEx::LoadBulkSettings(IN OUT PBULKCHANGELIST bulklist)
{
	// The MODE_LOADING is a special mode used specifically
	// for the purpose loading every setting.
	JostleEverySetting(bulklist, MODE_LOADING);
}


void CSettingGroupEx::ReviseBulkSettings(IN BOOL includeSubgroups, IN PBULKCHANGELIST bulklist)
{
	CSettingGroup::ReviseBulkSettings(bulklist);

	if (includeSubgroups)
	{
		// Run through all subgroups and have them save dependant bits too.
		SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
		SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
		for ( ; sgit != sgti; sgit++)
		{
			(*sgit)->ReviseBulkSettings(includeSubgroups, bulklist);
		}
	}
}


void CSettingGroupEx::JostleEverySetting(IN OUT PBULKCHANGELIST bulklist, IN BYTE jostleMode,
										 IN DBIT changedBits, IN DBIT checkedBits)
{
	JOSTLESTRUCT jostleStruct;

	// Initialize section string caches
	jostleStruct.saveSectionCacheBits = 0;
	jostleStruct.loadSectionCacheBits = 0;

	// Set the changes bits as changed and checked.
	jostleStruct.changedBits = changedBits;
	jostleStruct.checkedBits = changedBits | checkedBits;
	// Set the jostle mode
	jostleStruct.jostleMode = jostleMode;

	// Set the bulk change list to use
	jostleStruct.bulklist = bulklist;

	// Process every setting in this group
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		JostleSetting(dynamic_cast<PSETTINGINFOEX>(*it), &jostleStruct);
	}

	// The delta is no longer necessary for this group so copy the
	// new values across into m_dependedValues.
	m_dependencyGestalt->ApplyVectorChanges(&m_dependedValues, jostleStruct.changedBits);

	// All that is left for this group is to have CHANGED notification
	// called on all the setting that changed.  However, before that
	// happens, do the same as what we did on this group on all
	// subgroups.
	SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
	SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
	for ( ; sgit != sgti; sgit++)
	{
		// Don't update the subgroup if sgit is suspended, even if it
		// is loading mode.
		if (!(*sgit)->IsSuspended())
		{
			(*sgit)->ProcessVectorChanges(bulklist, jostleMode);
		}
	}
}


void CSettingGroupEx::JostleSetting(IN PSETTINGINFOEX info, IN PJOSTLESTRUCT jostleStruct)
{
	// Stop here if it is unnecessary to check this setting.
	if (jostleStruct->checkedBits & info->dependeeBit)
	{
		return;
	}

	// Flag the dependee bit this setting owns, if any, as checked.
	jostleStruct->checkedBits |= info->dependeeBit;

	// Get all the bits this setting depends on
	DBIT dependantBits = GetDependantBits(info);

	// Always reload the setting if in CHECK_LOADING is the mode.  The second
	// expression checks to see if the setting is actually loaded.  The third
	// checks if dependants have changed since the last time the setting was
	// loaded.  The fourth checks if the setting's dependee's values have changed.
	if (jostleStruct->jostleMode == MODE_LOADING || !info->object->IsSet() ||
		dependantBits != info->loadedDependantBits || dependantBits & jostleStruct->changedBits)
	{
		// If there are uncheck dependant bits, check those first
		if (dependantBits & ~jostleStruct->checkedBits)
		{
			JostleMasters(dependantBits & ~jostleStruct->checkedBits, jostleStruct);
		}

		// Save the setting to the old section if it is set.  If CHECK_LOADING is
		// the mode, discard any old values and just reload the setting.
		if (jostleStruct->jostleMode!= MODE_LOADING && info->object->IsSet())
		{
			// Get the appropriate section for saving the setting to.
			LPCSTR saveSection = GetSection(info->loadedDependantBits, TRUE,
				&jostleStruct->saveSectionCacheString, &jostleStruct->saveSectionCacheBits);

			// Save the setting value to the old section
			info->object->Save(m_repository, saveSection);
		}

		// Get the appropriate section for loading the setting from.
		LPCSTR loadSection = GetSection(dependantBits, FALSE,
			&jostleStruct->loadSectionCacheString, &jostleStruct->loadSectionCacheBits);

		CCINFO ccinfo;

		// Get the ccinfo structure filled with data necessary by CommonSettingCheckProc.
		CommonPreChangeRoutine(info, &ccinfo);

		// Load the setting value from the new section
		if (info->object->Load(m_repository, loadSection, CommonSettingCheckProc, &ccinfo))
		{
			// Record this change so that CHANGED notify can be called later.
			jostleStruct->bulklist->push_back(ccinfo);

			// If this setting is a master
			if (info->dependeeBit)
			{
				// Register the master setting's value change.
				if (m_dependencyGestalt->RegisterDependeeChange(&m_dependedValues,
					info->dependeeBit, info->object->GetValue()))
				{
					jostleStruct->changedBits |= info->dependeeBit;
				}
			}
		}

		// Update the loaded section record
		info->loadedDependantBits = dependantBits;
	}
}


void CSettingGroupEx::JostleMasters(IN DBIT dependeeBits, IN PJOSTLESTRUCT jostleStruct)
{
	if (dependeeBits & m_haveDependeeBits)
	{
		// Find all the relevant masters in this group
		SETTINGINFOLIST::iterator it = m_settingList.begin();
		SETTINGINFOLIST::iterator ti = m_settingList.end();
		for ( ; it != ti; it++)
		{
			PSETTINGINFOEX info = dynamic_cast<PSETTINGINFOEX>(*it);

			// Check to see if this is the master we're looking for
			if (info->dependeeBit & dependeeBits)
			{
				// Process the master changes
				JostleSetting(info, jostleStruct);

				// Remove the bit to keep track of those that are done
				dependeeBits &= ~info->dependeeBit;
				// Stop if there are no relevant masters left
				if (!(dependeeBits & m_haveDependeeBits))
				{
					break;
				}
			}
		}
	}

	// Set any left over bits for whom masters could not be found as
	// checked so they aren't searched for again.
	jostleStruct->checkedBits |= dependeeBits;
}


void CSettingGroupEx::ProcessVectorChanges(IN BULKCHANGELIST* bulklist, IN BYTE jostleMode)
{
	// Compare every bit for changes between the dependee values in
	// m_dependencyGestalt and our recorded dependee values.
	DBIT changedBits = m_dependencyGestalt->CompareVectorChanges(&m_dependedValues);

	if (jostleMode == MODE_DEPENDEE_CHECK && !changedBits)
	{
		// There is nothing to do if no dependees change.
		return;
	}

	// Process all settings and subgroup settings for changes.
	JostleEverySetting(bulklist, jostleMode, changedBits);
}


DBIT CSettingGroupEx::GetDependantBits(PSETTINGINFOEX info)
{
	return (info->dependantOptionalBits &
		m_dependencyGestalt->GetDependantMask()) | info->dependantAbsoluteBits;
}


LPCSTR CSettingGroupEx::GetSection(IN DBIT dependantBits, IN BOOL useOldValues,
								   IN OUT std::string* cacheString, IN OUT DBIT* cacheBits)
{
	// If there is no dependency, just use the base section string.
	if (dependantBits == 0)
	{
		return m_section.c_str();
	}

	// If the dependency is different from cached, recreate the string
	if (dependantBits != *cacheBits)
	{
		*cacheString = m_dependencyGestalt->CreateSection(dependantBits,
			m_section.c_str(), SECTIONDELIM, useOldValues ? &m_dependedValues : NULL);
		*cacheBits = dependantBits;
	}
	return cacheString->c_str();
}


//////////////////////////////////////////////////////////////////////////
// CSettingGroupEx::CDependencyGestalt
//////////////////////////////////////////////////////////////////////////

CSettingGroupEx::CDependencyGestalt::CDependencyGestalt() :
	m_validDependeeBits(0),
	m_dependantMask(0),
	m_sectionMutexEnterCount(0)
{
	InitializeCriticalSection(&m_sectionMutex);
}


CSettingGroupEx::CDependencyGestalt::~CDependencyGestalt()
{
	m_dependeeList.clear();
	m_dependeeValueVector.clear();
	DeleteCriticalSection(&m_sectionMutex);
}


DBIT CSettingGroupEx::CDependencyGestalt::CreateDependee(IN std::string title, IN std::string section)
{
	ASSERT(m_dependeeList.size() < sizeof(DBIT) * 8);

	DBIT bit = (DBIT)(1 << m_dependeeList.size());

	// Push the two strings into the vector.
	m_dependeeList.push_back(CDependeeInfo(title, section));
	// Synchronize the value vector size.
	m_dependeeValueVector.resize(m_dependeeList.size());
	// Update the valid bits mask
	m_validDependeeBits |= bit;

	// return the bit
	return bit;
}

BYTE CSettingGroupEx::CDependencyGestalt::GetDependeeCount()
{
	return (BYTE)m_dependeeList.size();
}


std::string CSettingGroupEx::CDependencyGestalt::GetDependeeTitle(BYTE index)
{
	ASSERT(index < m_dependeeList.size());
	return m_dependeeList.at(index).title;
}


DBIT CSettingGroupEx::CDependencyGestalt::GetValidDependeeBits()
{
	return m_validDependeeBits;
}


std::string CSettingGroupEx::CDependencyGestalt::CreateSection(IN DBIT dependantBits, IN LPCSTR baseSection,
															   IN LPCSTR sectionDelimiter,
															   IN PCDEPENDVALUEVECTOR dependeeValueVector)
{
	std::string section(baseSection);

	for (BYTE i = 0; i < m_dependeeList.size(); i++)
	{
		if (dependantBits & (1 << i))
		{
			// Stick on the delimiter string
			section += sectionDelimiter;
			// Stick on the section string
			section += m_dependeeList.at(i).section;
			// Stick on the recorded value string
			if (dependeeValueVector != NULL)
			{
				if (i >= dependeeValueVector->size())
				{
					section += CSettingValue().ToString();

				}
				else
				{
					section += dependeeValueVector->at(i).ToString();
				}
			}
			else
			{
				section += m_dependeeValueVector.at(i).ToString();
			}
		}
	}
	return section;
}


BOOL CSettingGroupEx::CDependencyGestalt::RegisterDependeeChange(IN PCDEPENDVALUEVECTOR dependeeValueVector,
																 IN DBIT dependeeBit, IN RCSETTINGVALUE value)
{
	BYTE index = DependeeBitToIndex(dependeeBit);
	ASSERT(index < m_dependeeList.size());

	m_dependeeValueVector[index] = value;

	// Return whether or not the vector will change because of this.
	if (index >= dependeeValueVector->size())
	{
		return TRUE;
	}
	return !(*dependeeValueVector)[index].IsEqual(m_dependeeValueVector[index]);
}


DBIT CSettingGroupEx::CDependencyGestalt::CompareVectorChanges(IN PCDEPENDVALUEVECTOR dependeeValueVector, IN DBIT mask)
{
	DBIT changes = 0;

	for (BYTE i = 0; i < m_dependeeList.size(); i++)
	{
		if (mask & (1 << i) && (i >= dependeeValueVector->size() ||
			!(*dependeeValueVector)[i].IsEqual(m_dependeeValueVector[i])))
		{
			changes |= 1 << i;
		}
	}
	return changes;
}


void CSettingGroupEx::CDependencyGestalt::ApplyVectorChanges(IN OUT PDEPENDVALUEVECTOR dependeeValueVector, IN DBIT mask)
{
	if (dependeeValueVector->size() < m_dependeeList.size())
	{
		dependeeValueVector->resize(m_dependeeList.size());
	}

	for (BYTE i = 0; i < m_dependeeList.size(); i++)
	{
		if (mask & (1 << i))
		{
			(*dependeeValueVector)[i] = m_dependeeValueVector[i];
		}
	}
}


BOOL CSettingGroupEx::CDependencyGestalt::SetDependantMask(IN DBIT dependantMask)
{
	dependantMask &= m_validDependeeBits;
	BOOL changed = dependantMask != m_dependantMask;
	m_dependantMask = dependantMask;
	return changed;
}


DBIT CSettingGroupEx::CDependencyGestalt::GetDependantMask()
{
	return m_dependantMask;
}


void CSettingGroupEx::CDependencyGestalt::EnterMutexSection(IN UINT iterations)
{
	for (UINT i = 0; i < iterations; i++)
	{
		EnterCriticalSection(&m_sectionMutex);
		m_sectionMutexEnterCount++;
	}
}


UINT CSettingGroupEx::CDependencyGestalt::LeaveMutexSection(IN BOOL iterate)
{
	if (iterate)
	{
		UINT iterations = m_sectionMutexEnterCount;
		m_sectionMutexEnterCount = 0;

		for (UINT i = 0; i < iterations; i++)
		{
			LeaveCriticalSection(&m_sectionMutex);
		}
		return iterations;
	}

	m_sectionMutexEnterCount--;
	LeaveCriticalSection(&m_sectionMutex);
	return 1;
}


BYTE CSettingGroupEx::CDependencyGestalt::DependeeBitToIndex(IN DBIT dependeeBit)
{
	BYTE index = 0;
	BYTE half = (BYTE)m_dependeeList.size();

	for ( ; half != 0; half = half / 2 + half % 2)
	{
		if (dependeeBit & (~((DBIT)0) << half))
		{
			index += half;
			dependeeBit >>= half;
		}
		if (half == 1)
		{
			break;
		}
	}
	return index;
}

