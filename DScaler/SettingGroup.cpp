/////////////////////////////////////////////////////////////////////////////
// $Id: SettingGroup.cpp,v 1.9 2005-03-17 03:55:18 atnak Exp $
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
// Revision 1.8  2005/03/05 12:15:20  atnak
// Syncing files.
//
// Revision 1.7  2004/09/08 07:19:01  atnak
// Major changes in the way operations are handled.  For better handling of
// operations called from inside callbacks.  Plus other changes.
//
// Revision 1.6  2004/08/20 09:16:19  atnak
// Fixed the anti-deadlock no-lock sanctuary for the notication callbacks.
//
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
#include <vector>
#include <list>


//////////////////////////////////////////////////////////////////////////
// CSettingGroup_
//////////////////////////////////////////////////////////////////////////

CSettingGroup_::CSettingGroup_(IN std::string section, IN PSETTINGREPOSITORY repository) :
	m_repository(repository),
	m_notifyProc(NULL),
	m_deferredOperationList(NULL),
	m_silent(0)
{
	m_section = section;
	InitializeLocking();
}


CSettingGroup_::~CSettingGroup_()
{
	// If m_deferredOperationList is not NULL when the group is being
	// deleted, this buffer is the least of the worries.  The group is
	// being deleted while it is still in use.  Besides, it's not a
	// simple matter to delete the buffer because the OPERATIONINFO.ptr
	// is an operation dependent buffer.
	ASSERT(m_deferredOperationList == NULL);

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


//////////////////////////////////////////////////////////////////////////
// Add setting functions
//////////////////////////////////////////////////////////////////////////

HSETTING CSettingGroup_::AddSetting(IN PSETTINGOBJECT object, IN PSETTINGKEY key)
{
	return AddSetting("", object, key);
}


HSETTING CSettingGroup_::AddSetting(IN std::string title, IN PSETTINGOBJECT object, IN PSETTINGKEY key)
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

	RegisterSetting(info);
	return reinterpret_cast<HSETTING>(info);
}


void CSettingGroup_::RegisterSetting(IN PSETTINGINFO info)
{
	if (info->key != NULL)
	{
		info->key->SetController(this, reinterpret_cast<HSETTING>(info));
	}

	m_settingList.push_back(info);
}


//////////////////////////////////////////////////////////////////////////
// Group setting functions.
//////////////////////////////////////////////////////////////////////////

void CSettingGroup_::SetNotifyProc(IN PSETTINGGROUP_NOTIFYPROC proc, IN PVOID context)
{
	m_notifyProc = proc;
	m_callbackContext = context;
}


void CSettingGroup_::EnableDeferring(IN BOOL enable)
{
	PNOTIFICATIONSTATE notifyState = GetActiveNotification();

	// This should only be called from inside a notification.
	if (notifyState == NULL)
	{
		return;
	}

	if (((notifyState->flags & FLAG_DEFER) != 0) != (enable != FALSE))
	{
		notifyState->flags ^= FLAG_DEFER;
	}
}


BOOL CSettingGroup_::IsSilent() const
{
	return m_silent > 0;
}


BOOL CSettingGroup_::IsSilent(IN BYTE options) const
{
	return (options & SGOF_SILENT) || IsSilent();
}


//////////////////////////////////////////////////////////////////////////
// Saving functions
//////////////////////////////////////////////////////////////////////////

void CSettingGroup_::SaveSettings()
{
	EnterObjectLock();

	// Run through the settings list and save each one
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		(*it)->object->Save(m_repository, m_section.c_str());
	}

	LeaveObjectLock();
}


void CSettingGroup_::SaveSetting(IN HSETTING setting)
{
	PSETTINGINFO info = reinterpret_cast<PSETTINGINFO>(setting);
	EnterObjectLock();
	info->object->Save(m_repository, m_section.c_str());
	LeaveObjectLock();
}


//////////////////////////////////////////////////////////////////////////
// Deferrable state changing functions
//////////////////////////////////////////////////////////////////////////

void CSettingGroup_::Silence(IN BOOL silence)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_SILENCE|OP_TYPE_STATE;
	opinfo.lParam	= (LPARAM)silence;
	opinfo.options	= 0;

	ProcessOperation(&opinfo);
}


//////////////////////////////////////////////////////////////////////////
// Value changing/notifying functions
//////////////////////////////////////////////////////////////////////////

void CSettingGroup_::LoadSettings(IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_LOAD_SETTINGS|OP_TYPE_CONTAINED;
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroup_::InsertMarker(IN INT markerID)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_INSERT_MARKER|OP_TYPE_CONTAINED;
	opinfo.lParam	= (LPARAM)markerID;
	opinfo.options	= 0;

	ProcessOperation(&opinfo);
}


void CSettingGroup_::LoadSetting(IN HSETTING setting, IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_LOAD_SETTING|OP_TYPE_CONTAINED;
	opinfo.info		= reinterpret_cast<PSETTINGINFO>(setting);
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroup_::SetValue(IN HSETTING setting, IN RCSETTINGVALUE value, IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_SET_VALUE|OP_TYPE_OBJECT;
	opinfo.info		= reinterpret_cast<PSETTINGINFO>(setting);
	opinfo.value	= new CSettingValue(value);
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroup_::UseDefault(IN HSETTING setting, IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_USE_DEFAULT|OP_TYPE_OBJECT;
	opinfo.info		= reinterpret_cast<PSETTINGINFO>(setting);
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroup_::SetDefault(IN HSETTING setting, IN RCSETTINGVALUE value, IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_SET_DEFAULT|OP_TYPE_OBJECT;
	opinfo.info		= reinterpret_cast<PSETTINGINFO>(setting);
	opinfo.value	= new CSettingValue(value);;
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroup_::CheckLimiter(IN HSETTING setting, IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_CHECK_LIMITER|OP_TYPE_OBJECT;
	opinfo.info		= reinterpret_cast<PSETTINGINFO>(setting);
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroup_::SetLimiter(IN HSETTING setting, IN PSETTINGLIMITER limiter, IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_SET_LIMITER|OP_TYPE_OBJECT;
	opinfo.info		= reinterpret_cast<PSETTINGINFO>(setting);
	opinfo.limiter	= limiter;
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


//////////////////////////////////////////////////////////////////////////
// Single value getting functions
//////////////////////////////////////////////////////////////////////////

PSETTINGKEY CSettingGroup_::GetSettingKey(IN HSETTING setting)
{
	PSETTINGINFO info = reinterpret_cast<PSETTINGINFO>(setting);
	return info->key;
}


std::string CSettingGroup_::GetSettingTitle(IN HSETTING setting)
{
	PSETTINGINFO info = reinterpret_cast<PSETTINGINFO>(setting);
	EnterObjectLock();
	std::string title = (info->title != "") ? info->title : info->object->GetTitle();
	LeaveObjectLock();
	return title;
}


CSettingValue CSettingGroup_::GetValue(IN HSETTING setting)
{
	PSETTINGINFO info = reinterpret_cast<PSETTINGINFO>(setting);
	EnterObjectLock();
	CSettingValue value = info->object->GetValue();
	LeaveObjectLock();
	return value;
}


CSettingValue CSettingGroup_::GetDefault(IN HSETTING setting)
{
	PSETTINGINFO info = reinterpret_cast<PSETTINGINFO>(setting);
	EnterObjectLock();
	CSettingValue value = info->object->GetDefault();
	LeaveObjectLock();
	return value;
}


PSETTINGLIMITER CSettingGroup_::GetLimiter(IN HSETTING setting)
{
	PSETTINGINFO info = reinterpret_cast<PSETTINGINFO>(setting);
	EnterObjectLock();
	PSETTINGLIMITER limiter= info->object->GetLimiter();
	LeaveObjectLock();
	return limiter;
}


//////////////////////////////////////////////////////////////////////////
// Single setting misc functions.
//////////////////////////////////////////////////////////////////////////

BOOL CSettingGroup_::IsPendingChanged(IN HSETTING setting)
{
	return GetInfoFlag(reinterpret_cast<PSETTINGINFO>(setting), FLAG_CHANGING);
}


BOOL CSettingGroup_::SilencePendingChanged(IN HSETTING setting)
{
	PSETTINGINFO info = reinterpret_cast<PSETTINGINFO>(setting);

	BOOL pending = GetInfoFlag(info, FLAG_CHANGING);
	if (pending)
	{
		UpdateChangedNotification(info, TRUE);
		SetInfoFlag(info, FLAG_CHANGING, FALSE);
	}
	return pending;
}


//////////////////////////////////////////////////////////////////////////
// Locking functions
//////////////////////////////////////////////////////////////////////////

void CSettingGroup_::InitializeLocking()
{
#ifdef SETTINGGROUP_THREAD_SAFE
	InitializeCriticalSection(&m_objectLockCriticalSection);
#endif
}


void CSettingGroup_::CleanupLocking()
{
#ifdef SETTINGGROUP_THREAD_SAFE
	DeleteCriticalSection(&m_objectLockCriticalSection);
#endif
}


void CSettingGroup_::EnterObjectLock()
{
#ifdef SETTINGGROUP_THREAD_SAFE
	EnterCriticalSection(&m_objectLockCriticalSection);
#endif
}


void CSettingGroup_::LeaveObjectLock()
{
#ifdef SETTINGGROUP_THREAD_SAFE
	LeaveCriticalSection(&m_objectLockCriticalSection);
#endif
}


//////////////////////////////////////////////////////////////////////////
// Info flag functions
//////////////////////////////////////////////////////////////////////////

void CSettingGroup_::SetInfoFlag(IN PSETTINGINFO info, IN BYTE flags, IN BOOL set)
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


BOOL CSettingGroup_::GetInfoFlag(IN PSETTINGINFO info, IN BYTE flags, IN BOOL strict)
{
	if (strict)
	{
		// If strict, make sure all the flags are set.
		return (info->flags & flags) == flags;
	}
	return (info->flags & flags) != 0;
}


//////////////////////////////////////////////////////////////////////////
// Operation processing functions.
//////////////////////////////////////////////////////////////////////////

void CSettingGroup_::ProcessOperation(IN POPERATIONINFO opinfo)
{
	// Get necessary information related to the current notification
	// if this operation was called from inside one.
	PNOTIFICATIONSTATE notifyState = GetActiveNotification();

	// If this is inside a notification and deferring is enabled.
	if (notifyState != NULL && notifyState->flags & FLAG_DEFER)
	{
		// Defer the operation until the notification has returned.
		EnqueueDeferredOperation(opinfo);
	}
	else if ((opinfo->op & OP_TYPE_MASK) == OP_TYPE_STATE)
	{
		opinfo->group->PerformStateOperation(opinfo);
	}
	else
	{
		CHANGEDNOTIFYLIST changedList;

		// Set our list as active so it will be used to enqueue pending
		// notifications.
		SetActiveChangedNotifyList(&changedList);

		// Perform the operation now.
		opinfo->group->PerformOperation(opinfo);
		// Send all the queued CHANGED/SET notifications.
		SendChangedNotifications(&changedList);

		// Notify only if this isn't inside another notification.
		if (notifyState == NULL)
		{
			NotifyAfterChanges();
			ProcessDeferredOperations();
		}

		SendChangedNotifications(&changedList);
		RevertActiveChangedNotifyList();
	}
}


void CSettingGroup_::PerformOperation(IN POPERATIONINFO opinfo)
{
	switch (opinfo->op & OP_TYPE_MASK)
	{
	case OP_TYPE_STATE:
		PerformStateOperation(opinfo);
		break;
	case OP_TYPE_CONTAINED:
		PerformContainedOperation(opinfo);
		break;
	case OP_TYPE_OBJECT:
		{
			CHANGEVALUES values;
			EnterObjectLock();
			PerformObjectOperation(opinfo, &values);
			LeaveObjectLock();

			// Process the changed value if it's set.
			if (values.isSet)
			{
				ProcessValueChange(opinfo->info, &values, opinfo->options);
			}
		}
		break;
	}
}


void CSettingGroup_::PerformStateOperation(IN POPERATIONINFO opinfo)
{
	switch (opinfo->op & OP_MASK)
	{
	case OP_SILENCE:
		if ((BOOL)opinfo->lParam)
		{
			m_silent++;
		}
		else
		{
			m_silent--;
		}
		break;
	}
}


void CSettingGroup_::PerformContainedOperation(IN POPERATIONINFO opinfo)
{
	switch (opinfo->op & OP_MASK)
	{
	case OP_LOAD_SETTINGS:
		// Let all settings in the group be CHANGING notified and loaded.
		LoadSettings(opinfo);
		break;
	case OP_LOAD_SETTING:
		LoadSetting(opinfo);
		break;
	case OP_INSERT_MARKER:
		InsertMarker(opinfo);
		break;
	}
}


void CSettingGroup_::PerformObjectOperation(IN POPERATIONINFO opinfo, IN PCHANGEVALUES values)
{
	switch (opinfo->op & OP_MASK)
	{
	case OP_SET_VALUE:
		opinfo->info->object->SetValue(*opinfo->value, GetValueChangeProc, values);
		delete opinfo->value;
		break;
	case OP_USE_DEFAULT:
		opinfo->info->object->UseDefault(GetValueChangeProc, values);
		break;
	case OP_SET_DEFAULT:
		opinfo->info->object->SetDefault(*opinfo->value, GetValueChangeProc, values);
		delete opinfo->value;
		break;
	case OP_CHECK_LIMITER:
		opinfo->info->object->CheckLimiter(GetValueChangeProc, values);
		break;
	case OP_SET_LIMITER:
		opinfo->info->object->SetLimiter(opinfo->limiter, GetValueChangeProc, values);
		break;
	}
}


BOOL CSettingGroup_::ProcessValueChange(IN PSETTINGINFO info, IN PCHANGEVALUES values,
										IN BYTE options)
{
	ASSERT(values->isSet);

	BOOL isEqual = values->newValue.IsEqual(values->oldValue);
	BOOL isRevise = (options & SGOF_REVISE) != 0;

	// Don't do anything if values don't change unless it's is revise mode.
	if (isEqual && !isRevise)
	{
		return FALSE;
	}

	BOOL changeable = TRUE;

	// There's no need to do any of this if notifications are disabled.
	if (!IsSilent(options))
	{
		if (GetInfoFlag(info, FLAG_CHANGING))
		{
			// Since CHANGING notification was already sent, send RECHANGING.
			changeable = NotifyRechanging(info, values->newValue, values->oldValue);

			if (changeable)
			{
				// Move the previous queuing of this change back to the end.
				UpdateChangedNotification(info, FALSE);
			}
		}
		else
		{
			// Set whether this is the initial value.
			SetInfoFlag(info, FLAG_INITIAL, isRevise ? TRUE : info->object->IsSet());

			changeable = NotifyChanging(info, values->newValue, values->oldValue);

			// Generate a CHANGING/SETTING notification.
			if (changeable)
			{
				// Flag the setting info as changing
				SetInfoFlag(info, FLAG_CHANGING, changeable);

				CHANGEDNOTIFY notification;
				// Save the information for the notification.
				notification.group = this;
				notification.info = info;
				notification.oldValue = values->oldValue;

				// Enqueue the pending notification
				EnqueueChangedNotification(&notification);
			}
		}
	}

	if (isEqual)
	{
		changeable = FALSE;
	}
	if (changeable)
	{
		// Set the new value.
		EnterObjectLock();
		info->object->SetValue(values->newValue, NULL, NULL);
		LeaveObjectLock();
	}

	// Process deferred operations queued during the calling of
	// the notification above.
	ProcessDeferredOperations();
	// Return whether or not the values actually change.
	return changeable;
}


void CSettingGroup_::LoadSettings(POPERATIONINFO opinfo)
{
	CHANGEVALUES values;

	// Load every setting not already loaded.
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		EnterObjectLock();
		if ((opinfo->options & SGOF_RELOAD) || !(*it)->object->IsSet())
		{
			// Load a the setting from the repository
			(*it)->object->Load(m_repository, m_section.c_str(), GetValueChangeProc, &values);
		}
		else if ((opinfo->options & SGOF_REVISE) && !IsSilent(opinfo->options))
		{
			// Revise will notify the current value as is.
			values.Set((*it)->object->GetValue(), (*it)->object->GetValue());
		}
		LeaveObjectLock();

		if (values.isSet)
		{
			// Perform the necessary checks for the new value.
			ProcessValueChange(*it, &values, opinfo->options);
		}
	}
}


void CSettingGroup_::LoadSetting(POPERATIONINFO opinfo)
{
	CHANGEVALUES values;

	EnterObjectLock();
	if ((opinfo->options & SGOF_RELOAD) || !opinfo->info->object->IsSet())
	{
		// Load a the setting from the repository
		opinfo->info->object->Load(m_repository, m_section.c_str(), GetValueChangeProc, &values);
	}
	else if ((opinfo->options & SGOF_REVISE) && !IsSilent(opinfo->options))
	{
		// Revise will notify the current value as is.
		values.Set(opinfo->info->object->GetValue(), opinfo->info->object->GetValue());
	}
	LeaveObjectLock();

	if (values.isSet)
	{
		ProcessValueChange(opinfo->info, &values, opinfo->options);
	}
}


void CSettingGroup_::InsertMarker(POPERATIONINFO opinfo)
{
	if (IsSilent())
	{
		// There's no point in doing this if silent.
		return;
	}

	CSettingValue value;
	value.SetInt((INT)opinfo->lParam);

	if (NotifyMarkerQueue(value))
	{
		CHANGEDNOTIFY notification;
		// Save the information for the notification.
		notification.group = this;
		notification.info = NULL;
		notification.oldValue = value;

		// Enqueue the pending notification
		EnqueueChangedNotification(&notification);
	}

	// Process deferred operations queued during the calling of
	// the marker notification.
	ProcessDeferredOperations();
}


//////////////////////////////////////////////////////////////////////////
// Changed notify list functions
//////////////////////////////////////////////////////////////////////////

CSettingGroup_::PCHANGEDNOTIFYSTACK CSettingGroup_::GetChangedNotifyStack()
{
	return &m_changedNotifyStack;
}


CSettingGroup_::PCHANGEDNOTIFYLIST CSettingGroup_::GetActiveChangedNotifyList()
{
	PCHANGEDNOTIFYSTACK stack = GetChangedNotifyStack();
	return stack->empty() ? NULL : stack->back();
}


void CSettingGroup_::SetActiveChangedNotifyList(IN PCHANGEDNOTIFYLIST changedList)
{
	// Push the changed list onto the stack.
	GetChangedNotifyStack()->push_back(changedList);
}


void CSettingGroup_::RevertActiveChangedNotifyList()
{
	// Pop the top of the stack.
	GetChangedNotifyStack()->pop_back();
}


void CSettingGroup_::EnqueueChangedNotification(IN PCHANGEDNOTIFY notification)
{
	PCHANGEDNOTIFYLIST changedList = GetActiveChangedNotifyList();
	// Enqueue the pending changed notification to the end of the list.
	changedList->push_back(*notification);
}


void CSettingGroup_::SendChangedNotifications(IN PCHANGEDNOTIFYLIST changedList)
{
	CHANGEDNOTIFYLIST::iterator it = changedList->begin();
	// Note: don't cache end() because it can change in the loop.
	for ( ; it != changedList->end(); it++)
	{
		// The NULL group indicates deleted.
		if (it->group == NULL)
		{
			continue;
		}

		// The NULL info indicates a marker.
		if (it->info == NULL)
		{
			it->group->NotifyMarkerQueued(it->oldValue);
		}
		else
		{
			EnterObjectLock();
			CSettingValue newValue = it->info->object->GetValue();
			LeaveObjectLock();

			// Remove the changing flag before notifying CHANGED.
			SetInfoFlag(it->info, FLAG_CHANGING, FALSE);

			// Send the post change notification
			it->group->NotifyChanged(it->info, newValue, it->oldValue);
		}
		// Perform any deferred operations that have accumulated.
		ProcessDeferredOperations();
	}

	changedList->clear();
}


void CSettingGroup_::UpdateChangedNotification(IN PSETTINGINFO info, IN BOOL removeOnly)
{
	PCHANGEDNOTIFYSTACK stack = GetChangedNotifyStack();

	// Removes from the pending notify queue the first instance where info
	// matches the value passed.  If the instance is not found in the
	// active bulk change info, those higher up the stack are searched.
	CHANGEDNOTIFYSTACK::reverse_iterator rit = stack->rbegin();
	CHANGEDNOTIFYSTACK::reverse_iterator rti = stack->rend();
	for ( ; rit != rti; rit++)
	{
		PCHANGEDNOTIFYLIST changedList = *rit;

		// Look through the notify list to find the first where info match.
		CHANGEDNOTIFYLIST::iterator it = changedList->begin();
		CHANGEDNOTIFYLIST::iterator ti = changedList->end();
		for ( ; it != ti; it++)
		{
			if (it->info == info)
			{
				if (!removeOnly)
				{
					// Enqueue the item again to the last position.
					EnqueueChangedNotification(&*it);
				}

				// Invalidate the item in the old position but don't erase.
				it->info = NULL;
				it->group = NULL;
				return;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Active notification and deferring functions
//////////////////////////////////////////////////////////////////////////

CSettingGroup_::PNOTIFICATIONSTACK CSettingGroup_::GetNotificationStack()
{
	return &m_notificationStack;
}


CSettingGroup_::PNOTIFICATIONSTATE CSettingGroup_::GetActiveNotification()
{
	PNOTIFICATIONSTACK stack = GetNotificationStack();
	return stack->empty() ? NULL : stack->back();
}


void CSettingGroup_::EnqueueDeferredOperation(IN POPERATIONINFO opinfo)
{
	PNOTIFICATIONSTATE notifyState = GetActiveNotification();

	if (notifyState->deferList == NULL)
	{
		notifyState->deferList = new OPERATIONLIST();
	}

	// Defer the operation until the notification has returned.
	notifyState->deferList->push_back(*opinfo);
}


void CSettingGroup_::StackDeferredOperations(IN POPERATIONLIST oplist)
{
	if (m_deferredOperationList == NULL)
	{
		m_deferredOperationList = oplist;
	}
	else
	{
		m_deferredOperationList->insert(m_deferredOperationList->end(),
			oplist->begin(), oplist->end());
		delete oplist;
	}
}


void CSettingGroup_::ProcessDeferredOperations()
{
	if (m_deferredOperationList != NULL)
	{
		POPERATIONLIST operationlist = m_deferredOperationList;
		m_deferredOperationList = NULL;

		// Perform all deferred operations now.
		OPERATIONLIST::iterator it = operationlist->begin();
		OPERATIONLIST::iterator ti = operationlist->end();
		for ( ; it != ti; it++)
		{
			(*it).group->PerformOperation(&*it);
		}

		delete operationlist;
	}
}


BOOL CSettingGroup_::GetValueChangeProc(RCSETTINGVALUE newValue,
									   RCSETTINGVALUE oldValue,
									   PCSETTINGOBJECT object,
									   PVOID context)
{
	PCHANGEVALUES values = (PCHANGEVALUES)context;
	values->Set(values->newValue, values->oldValue);
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// Notifying functions.
//////////////////////////////////////////////////////////////////////////

BOOL CSettingGroup_::NotifyChanging(IN PSETTINGINFO info,
								   IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	INT message = GetInfoFlag(info, FLAG_INITIAL) ? NOTIFY_VALUE_SETTING : NOTIFY_VALUE_CHANGING;
	return Notify(info, message, newValue, oldValue);
}


BOOL CSettingGroup_::NotifyRechanging(IN PSETTINGINFO info,
								   IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	INT message = GetInfoFlag(info, FLAG_INITIAL) ? NOTIFY_VALUE_RESETTING : NOTIFY_VALUE_RECHANGING;
	return Notify(info, message, newValue, oldValue);
}


BOOL CSettingGroup_::NotifyChanged(IN PSETTINGINFO info,
								  IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	INT message = GetInfoFlag(info, FLAG_INITIAL) ? NOTIFY_VALUE_SET : NOTIFY_VALUE_CHANGED;
	return Notify(info, message, newValue, oldValue);
}


BOOL CSettingGroup_::NotifyMarkerQueue(IN RCSETTINGVALUE markerValue)
{
	return Notify(NULL, NOTIFY_MARKER_QUEUE, markerValue, CSettingValue());
}


BOOL CSettingGroup_::NotifyMarkerQueued(IN RCSETTINGVALUE markerValue)
{
	return Notify(NULL, NOTIFY_MARKER_QUEUED, markerValue, CSettingValue());
}


BOOL CSettingGroup_::NotifyAfterChanges()
{
	return Notify(NULL, NOTIFY_AFTER_CHANGES, CSettingValue(), CSettingValue());
}


BOOL CSettingGroup_::Notify(IN PSETTINGINFO info, IN INT message,
						   IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
{
	if (IsSilent())
	{
		return TRUE;
	}

	BOOL response = -1;

	// Get the notification stack.
	PNOTIFICATIONSTACK stack = GetNotificationStack();
	// Check to make sure this info isn't already in a notification.
	NOTIFICATIONSTACK::iterator it = stack->begin();
	NOTIFICATIONSTACK::iterator ti = stack->end();
	for ( ; it != ti; it++)
	{
		if (((PNOTIFICATIONSTATE)(*it))->identity == info)
		{
			TRACE("WARNING: Re-enter of notification for \"%s\" blocked!!\n", info->title.c_str());
			return TRUE;
		}
	}

	NOTIFICATIONSTATE notifyState = { info, 0, NULL };

	if (message == NOTIFY_VALUE_CHANGING || message == NOTIFY_VALUE_RECHANGING ||
		message == NOTIFY_VALUE_SETTING || message == NOTIFY_VALUE_RESETTING)
	{
		notifyState.flags |= FLAG_DEFER;
	}

	// Push our buffer onto the notification stack.
	stack->push_back(&notifyState);

	// First call the global notification callback if one exists
	if (m_notifyProc != NULL)
	{
		response = (m_notifyProc)(message, newValue, oldValue,
			reinterpret_cast<HSETTING>(info), m_callbackContext);

		switch (response)
		{
		case NOTIFY_REPLY_DONT_NOTIFY: response = TRUE; break;
		case NOTIFY_REPLY_DONT_CHANGE: response = FALSE; break;
		default: response = -1; break;
		}
	}

	if (response == -1 && info != NULL)
	{
		// Reset notification flags to default before the notification
		notifyState.flags = 0;
		// Call the notification callback of this setting object
		if (!info->object->Notify(message, newValue, oldValue))
		{
			// EnterLock();
			response = FALSE;
		}
		else if (info->key != NULL)
		{
			// Reset notification flags to default before the notification
			notifyState.flags = 0;
			// Call the notification callback of this setting key
			response = info->key->Notify(message, newValue, oldValue);
		}
	}

	// Revert the notification stack
	stack->pop_back();

	if (notifyState.deferList != NULL)
	{
		// Update the deferred operations list.
		StackDeferredOperations(notifyState.deferList);
	}

	return response == -1 ? TRUE : response;
}


//////////////////////////////////////////////////////////////////////////
// CSettingGroup_::CSettingInfo
//////////////////////////////////////////////////////////////////////////

CSettingGroup_::CSettingInfo::CSettingInfo(IN PSETTINGKEY key, IN PSETTINGOBJECT object,
										  IN std::string title) :
	key(key),
	object(object),
	title(title),
	flags(0)
{
}


CSettingGroup_::CSettingInfo::~CSettingInfo()
{
	delete object;
}

// The string that is used to delimit sub-section when saving and
// loading from the repository.  Changing will lose previously
// saved sections
const LPCSTR CSettingGroupEx::SECTIONDELIM = ",";
// This string is attached to the section string for the section
// used to store optional dependant bits for every setting.
const LPCSTR CSettingGroupEx::DEPENDANTSECTIONPOSTFIX = "#dep";
const LPCSTR CSettingGroupEx::DEPENDANTMASKSAVEKEY = "_depmask";


//////////////////////////////////////////////////////////////////////////
// CSettingGroupEx::CSettingInfoEx
//////////////////////////////////////////////////////////////////////////

CSettingGroupEx::CSettingInfoEx::CSettingInfoEx(IN PSETTINGKEY key, IN PSETTINGOBJECT object,
												IN std::string title, IN DBIT dependeeBit,
												IN DBIT optionalBits, IN DBIT absoluteBits) :
	CSettingInfo(key, object, title),
	dependeeBit(dependeeBit),
	dependantOptionalBits(optionalBits),
	dependantAbsoluteBits(absoluteBits),
	loadedDependantBits(0)
{
}


CSettingGroupEx::CSettingInfoEx::~CSettingInfoEx()
{
}


//////////////////////////////////////////////////////////////////////////
// CSettingGroupEx
//////////////////////////////////////////////////////////////////////////

CSettingGroupEx::CSettingGroupEx(IN std::string section, IN PSETTINGREPOSITORY repository,
								 IN PSETTINGGROUPEX parent) :
	CSettingGroup_(section, repository),
	m_suspended(0),
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


//////////////////////////////////////////////////////////////////////////
// Extra creation functions
//////////////////////////////////////////////////////////////////////////

PSETTINGGROUPEX CSettingGroupEx::CreateSubgroup()
{
	PSETTINGGROUPEX subgroup = new CSettingGroupEx(m_section, m_repository, this);
	if (subgroup == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}

	m_subgroupList.push_back(subgroup);
	return subgroup;
}


DBIT CSettingGroupEx::CreateDependency(IN std::string title, IN std::string section)
{
	return m_dependencyGestalt->CreateDependee(title, section);
}


//////////////////////////////////////////////////////////////////////////
// Setting adding functions
//////////////////////////////////////////////////////////////////////////

HSETTING CSettingGroupEx::AddSetting(IN PSETTINGOBJECT object, IN PSETTINGKEY key,
									 IN DBIT optionalBits, IN DBIT absoluteBits)
{
	return AddMaster("", object, key, 0, optionalBits, absoluteBits);
}


HSETTING CSettingGroupEx::AddSetting(IN std::string title, IN PSETTINGOBJECT object, IN PSETTINGKEY key,
									 IN DBIT optionalBits, IN DBIT absoluteBits)
{

	return AddMaster(title, object, key, 0, optionalBits, absoluteBits);
}


HSETTING CSettingGroupEx::AddMaster(IN PSETTINGOBJECT object, IN PSETTINGKEY key, IN DBIT dependeeBits,
									IN DBIT optionalBits, IN DBIT absoluteBits)
{
	return AddMaster("", object, key, dependeeBits, optionalBits, absoluteBits);
}


HSETTING CSettingGroupEx::AddMaster(IN std::string title, IN PSETTINGOBJECT object, IN PSETTINGKEY key,
									IN DBIT dependeeBits, IN DBIT optionalBits, IN DBIT absoluteBits)
{
	if (object == NULL)
	{
		return NULL;
	}

	PSETTINGINFOEX info = new CSettingInfoEx(key, object, title, dependeeBits, optionalBits, absoluteBits);
	if (info == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}

	// Assert that there're no stray bits.
	ASSERT(!(dependeeBits & ~m_dependencyGestalt->GetValidDependeeBits()));
	ASSERT(!(optionalBits & ~m_dependencyGestalt->GetValidDependeeBits()));
	ASSERT(!(absoluteBits & ~m_dependencyGestalt->GetValidDependeeBits()));

	m_haveDependeeBits |= dependeeBits;

	RegisterSetting(info);
	return reinterpret_cast<HSETTING>(info);
}


//////////////////////////////////////////////////////////////////////////
// Saving functions.
//////////////////////////////////////////////////////////////////////////

void CSettingGroupEx::SaveSettings()
{
	std::string cacheString;
	DBIT cacheBits = 0;

	EnterObjectLock();
	// Run through the settings list and save each one
	SETTINGINFOLIST::iterator it = m_settingList.begin();
	SETTINGINFOLIST::iterator ti = m_settingList.end();
	for ( ; it != ti; it++)
	{
		PSETTINGINFOEX info = dynamic_cast<PSETTINGINFOEX>(*it);
		info->object->Save(m_repository,
			GetSection(info->loadedDependantBits, TRUE, &cacheString, &cacheBits));
	}
	LeaveObjectLock();

	// Run through all subgroups and have them save all settings too.
	SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
	SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
	for ( ; sgit != sgti; sgit++)
	{
		(*sgit)->SaveSettings();
	}
}


void CSettingGroupEx::SaveSetting(IN HSETTING setting)
{
	PSETTINGINFOEX info = reinterpret_cast<PSETTINGINFOEX>(setting);

	std::string cacheString;
	DBIT cacheBits = 0;

	EnterObjectLock();
	info->object->Save(m_repository,
		GetSection(info->loadedDependantBits, TRUE, &cacheString, &cacheBits));
	LeaveObjectLock();
}


//////////////////////////////////////////////////////////////////////////
// State changing functions.
//////////////////////////////////////////////////////////////////////////

void CSettingGroupEx::Suspend()
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_SUSPEND|OP_TYPE_STATE;

	ProcessOperation(&opinfo);
}


BOOL CSettingGroupEx::IsSuspended()
{
	if (m_suspended > 0)
	{
		return TRUE;
	}
	if (m_parentGroup != NULL)
	{
		return m_parentGroup->IsSuspended();
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// Value changing/notifying functions
//////////////////////////////////////////////////////////////////////////

void CSettingGroupEx::Activate(IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_ACTIVATE|OP_TYPE_CONTAINED;
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroupEx::JostleBit(IN DBIT dependeeBit, IN RCSETTINGVALUE dependeeValue,
								IN BYTE options)
{
	OPERATIONINFO opinfo(this);

	opinfo.op		= OP_JOSTLE_BIT|OP_TYPE_CONTAINED;
	opinfo.info		= reinterpret_cast<PSETTINGINFOEX>(dependeeBit);
	opinfo.ptr		= reinterpret_cast<PVOID>(new CSettingValue(dependeeValue));
	opinfo.options	= options;

	ProcessOperation(&opinfo);
}


void CSettingGroupEx::SetOptionalDependantBits(IN HSETTING setting,
											   IN DBIT dependantOptionalBits, IN BYTE options)
{
	PSETTINGINFOEX info = reinterpret_cast<PSETTINGINFOEX>(setting);

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
		// This call can be deferred.
		CSettingGroup_::LoadSetting(setting, options);
	}
}


void CSettingGroupEx::EnableOptionalDependencies(IN DBIT dependeeBit, IN BOOL set, IN BYTE options)
{
	DBIT mask = m_dependencyGestalt->GetDependantMask();
	if (set)
	{
		mask |= dependeeBit;
	}
	else
	{
		mask &= ~dependeeBit;
	}

	SetEnabledOptionalDependencies(mask, options);
}


void CSettingGroupEx::SetEnabledOptionalDependencies(IN DBIT mask, IN BYTE options)
{
	BOOL maskChanges = m_dependencyGestalt->SetDependantMask(mask);
	if (maskChanges)
	{
		PSETTINGGROUPEX root = GetRootParent();
		// Calling load takes care of synchronizing all changes
		// that occur as a result of the dependant mask change.
		root->CSettingGroup_::LoadSettings(options);
	}
}


//////////////////////////////////////////////////////////////////////////
// Single value getting functions
//////////////////////////////////////////////////////////////////////////

DBIT CSettingGroupEx::GetDependeeBits(IN HSETTING setting)
{
	PSETTINGINFOEX info = reinterpret_cast<PSETTINGINFOEX>(setting);
	return info->dependeeBit;
}

DBIT CSettingGroupEx::GetOptionalDependantBits(IN HSETTING setting)
{
	PSETTINGINFOEX info = reinterpret_cast<PSETTINGINFOEX>(setting);
	return info->dependantOptionalBits;
}


DBIT CSettingGroupEx::GetAbsoluteDependantBits(IN HSETTING setting)
{
	PSETTINGINFOEX info = reinterpret_cast<PSETTINGINFOEX>(setting);
	return info->dependantAbsoluteBits;
}


DBIT CSettingGroupEx::GetEnabledOptionalDependencies()
{
	return m_dependencyGestalt->GetDependantMask();
}


//////////////////////////////////////////////////////////////////////////
// Setting config container creator
//////////////////////////////////////////////////////////////////////////

PSETTINGCONFIG CSettingGroupEx::CreateAssociationConfig(std::string title, BOOL addSettings)
{
	BYTE dependeeCount = m_dependencyGestalt->GetDependeeCount();

	CSettingConfigAssociation* config =
		new CSettingConfigAssociation(title, this, dependeeCount);

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
		std::string title = GetSettingTitle(reinterpret_cast<HSETTING>(*it));
		if (title != "")
		{
			config->AddConfig(new CSettingConfigDependant(this, reinterpret_cast<HSETTING>(*it)));
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


//////////////////////////////////////////////////////////////////////////
// Dependencies saving and loading
//////////////////////////////////////////////////////////////////////////

void CSettingGroupEx::SaveOptionalDependencies()
{
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
		// Set whether dependantOptionalBits have changed since load.
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


void CSettingGroupEx::LoadOptionalDependencies(IN BYTE options)
{
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

	PSETTINGGROUPEX root = GetRootParent();
	// This synchronizes all settings for the loaded dependencies.
	root->CSettingGroup_::LoadSettings(options);
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
			// Set whether dependantOptionalBits have changed since load.
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


//////////////////////////////////////////////////////////////////////////
// Operation processing functions.
//////////////////////////////////////////////////////////////////////////

BOOL CSettingGroupEx::ProcessValueChange(IN PSETTINGINFO info, IN PCHANGEVALUES values, IN BYTE options)
{
	BOOL changed = CSettingGroup_::ProcessValueChange(info, values, options);

	if (changed || (options & SGOF_FULLSYNC))
	{
		PSETTINGINFOEX infoex = dynamic_cast<PSETTINGINFOEX>(info);

		// If a master setting changes, check dependant settings for change.
		if (infoex->dependeeBit != 0 && !IsSuspended())
		{
			// Set the master's dependee bit as changed.
			if (m_dependencyGestalt->RegisterDependeeChange(&m_dependedValues,
				infoex->dependeeBit, values->newValue))
			{
				PSETTINGGROUPEX root = GetRootParent();
				// Check and process all settings for changes.
				root->JostleAllSettings(infoex->dependeeBit,
					infoex->dependeeBit|GetDependantBits(infoex), options);
			}
		}
		return TRUE;
	}
	return FALSE;
}


void CSettingGroupEx::PerformStateOperation(IN POPERATIONINFO opinfo)
{
	switch (opinfo->op & OP_MASK)
	{
	case OP_SUSPEND:
		m_suspended++;
		break;
	default:
		CSettingGroup_::PerformStateOperation(opinfo);
		break;
	}
}


void CSettingGroupEx::PerformContainedOperation(IN POPERATIONINFO opinfo)
{
	switch (opinfo->op & OP_MASK)
	{
	case OP_ACTIVATE:
		Activate(opinfo);
		break;
	case OP_JOSTLE_BIT:
		JostleBit(opinfo);
		break;
	default:
		CSettingGroup_::PerformContainedOperation(opinfo);
		break;
	}
}


void CSettingGroupEx::LoadSettings(IN POPERATIONINFO opinfo)
{
	if (!IsSuspended())
	{
		JostleAllSettings(0, 0, opinfo->options);
	}
}


void CSettingGroupEx::LoadSetting(IN POPERATIONINFO opinfo)
{
	JOSTLESTRUCT jostleStruct(0, 0, opinfo->options);

	if (!IsSuspended())
	{
		JostleSetting(dynamic_cast<PSETTINGINFOEX>(opinfo->info), &jostleStruct);
	}
}


void CSettingGroupEx::Activate(IN POPERATIONINFO opinfo)
{
	if (m_suspended != 0)
	{
		m_suspended--;
	}
	if (!IsSuspended())
	{
		JostleAllSettings(0, 0, opinfo->options|SGOF_FULLSYNC);
	}
}


void CSettingGroupEx::JostleBit(IN POPERATIONINFO opinfo)
{
	DBIT dependeeBit = reinterpret_cast<DBIT>(opinfo->info);
	PSETTINGVALUE dependeeValue = reinterpret_cast<PSETTINGVALUE>(opinfo->ptr);

	if (!IsSuspended())
	{
		if (m_dependencyGestalt->RegisterDependeeChange(&m_dependedValues,
			dependeeBit, *reinterpret_cast<PSETTINGVALUE>(opinfo->ptr)))
		{
			PSETTINGGROUPEX root = GetRootParent();
			// If the dependee value vector changes, process settings.
			root->JostleAllSettings(dependeeBit, dependeeBit, opinfo->options);
		}
	}

	delete dependeeValue;
}


//////////////////////////////////////////////////////////////////////////
// Changed notify list functions
//////////////////////////////////////////////////////////////////////////

CSettingGroup_::PCHANGEDNOTIFYSTACK CSettingGroupEx::GetChangedNotifyStack()
{
	if (m_parentGroup != NULL)
	{
		return m_parentGroup->GetChangedNotifyStack();
	}
	return &m_changedNotifyStack;
}


//////////////////////////////////////////////////////////////////////////
// Active notification and deferring functions
//////////////////////////////////////////////////////////////////////////

CSettingGroup_::PNOTIFICATIONSTACK CSettingGroupEx::GetNotificationStack()
{
	if (m_parentGroup != NULL)
	{
		return m_parentGroup->GetNotificationStack();
	}
	return &m_notificationStack;
}


//////////////////////////////////////////////////////////////////////////
// Jostling functions.
//////////////////////////////////////////////////////////////////////////

void CSettingGroupEx::JostleAllSettings(IN DBIT changedBits, IN DBIT checkedBits, IN BYTE options)
{
	if (options & SGOF_FULLSYNC)
	{
		// Compare every bit for changes between our recorded dependee values.
		DBIT bits = m_dependencyGestalt->CompareVectorChanges(&m_dependedValues);
		changedBits |= bits;
		checkedBits |= bits;
	}

	JOSTLESTRUCT jostleStruct(changedBits, checkedBits, options);

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
		// Don't update the subgroup if sgit is suspended.
		if (!(*sgit)->IsSuspended())
		{
			(*sgit)->JostleAllSettings(jostleStruct.changedBits, jostleStruct.checkedBits, options);
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

	// If there are unchecked dependant bits, check those first
	if (dependantBits & ~jostleStruct->checkedBits)
	{
		JostleMasters(dependantBits & ~jostleStruct->checkedBits, jostleStruct);
	}

	// Gather reasons for loading the setting:
	// - if forced reload is set.
	// - if sections are changing.
	// - if the setting is not loaded.

	// Check if forced reload is set.
	BOOL isReload = (jostleStruct->options & SGOF_RELOAD) != 0;

	// Check if sections are changing.
	BOOL isSectionChange = FALSE;
	if ((dependantBits != info->loadedDependantBits) ||
		(dependantBits & jostleStruct->changedBits))
	{
		isSectionChange = TRUE;
	}

	EnterObjectLock();

	// Check if the setting is not loaded.
	BOOL isInitialLoad = !info->object->IsSet();

	CHANGEVALUES values;

	if (isReload || isSectionChange || isInitialLoad)
	{
		// Save the setting to the old section if it is set and sections are changing.
		if (!isInitialLoad && isSectionChange)
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

		// Load the setting value from the new section
		info->object->Load(m_repository, loadSection, GetValueChangeProc, &values);

		// Update the loaded section record regardless of whether this value is used.
		info->loadedDependantBits = dependantBits;
	}
	else if ((jostleStruct->options & SGOF_FULLSYNC) ||
		((jostleStruct->options & SGOF_REVISE) && !IsSilent(jostleStruct->options)))
	{
		// Get the old and new values for revise.
		values.Set(info->object->GetValue(), info->object->GetValue());
	}

	LeaveObjectLock();

	if (values.isSet)
	{
		BOOL changed = CSettingGroup_::ProcessValueChange(info, &values, jostleStruct->options);

		// If the setting changes or FULLSYNC is set and the setting is a master.
		if ((changed || (jostleStruct->options & SGOF_FULLSYNC)) && (info->dependeeBit != 0))
		{
			// Register the master setting's value change.
			if (m_dependencyGestalt->RegisterDependeeChange(&m_dependedValues,
				info->dependeeBit, values.newValue))
			{
				jostleStruct->changedBits |= info->dependeeBit;
			}
		}
	}
}


void CSettingGroupEx::JostleMasters(IN DBIT dependeeBits, IN PJOSTLESTRUCT jostleStruct)
{
	if (m_parentGroup != NULL)
	{
		// Jostle master needs to be started at the base group.
		m_parentGroup->JostleMasters(dependeeBits, jostleStruct);
		return;
	}

	// Find masters and get bits for masters that could not be found.
	dependeeBits = _JostleMasters(dependeeBits, jostleStruct);

	// Set any left over bits for whom masters could not be found as
	// checked so they aren't searched for again in this jostle round.
	jostleStruct->checkedBits |= dependeeBits;
}


DBIT CSettingGroupEx::_JostleMasters(IN DBIT dependeeBits, IN PJOSTLESTRUCT jostleStruct)
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

	// If there are any bits left.
	if (dependeeBits != 0)
	{
		// Look for the remaining masters in subgroups.
		SUBGROUPEXLIST::iterator sgit = m_subgroupList.begin();
		SUBGROUPEXLIST::iterator sgti = m_subgroupList.end();
		for ( ; sgit != sgti; sgit++)
		{
			// Don't look for masters in suspended groups.
			if (!(*sgit)->IsSuspended())
			{
				// Find masters and get bits for masters that could not be found.
				dependeeBits = (*sgit)->_JostleMasters(dependeeBits, jostleStruct);
				// Stop if there are no masters left to check.
				if (dependeeBits == 0)
				{
					break;
				}
			}
		}
	}

	// Return the bits that could not be found.
	return dependeeBits;
}


//////////////////////////////////////////////////////////////////////////
// Jostling helper functions
//////////////////////////////////////////////////////////////////////////

PSETTINGGROUPEX CSettingGroupEx::GetRootParent()
{
	if (m_parentGroup == NULL)
	{
		return this;
	}
	return m_parentGroup->GetRootParent();
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
	m_dependantMask(0)
{
}


CSettingGroupEx::CDependencyGestalt::~CDependencyGestalt()
{
	m_dependeeList.clear();
	m_dependeeValueVector.clear();
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
		if (mask & (1 << i))
		{
			if (i < dependeeValueVector->size())
			{
				if (!(*dependeeValueVector)[i].IsEqual(m_dependeeValueVector[i]))
				{
					changes |= 1 << i;
				}
			}
			else if (m_dependeeValueVector.at(i).IsSet())
			{
				changes |= 1 << i;
			}
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

