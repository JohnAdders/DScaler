/////////////////////////////////////////////////////////////////////////////
// $Id: SettingGroup.h,v 1.11 2005-03-17 03:55:19 atnak Exp $
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
// Revision 1.10  2005/03/05 12:15:20  atnak
// Syncing files.
//
// Revision 1.9  2004/09/11 13:00:45  adcockj
// fix compile problems with vs6
//
// Revision 1.8  2004/09/08 07:19:01  atnak
// Major changes in the way operations are handled.  For better handling of
// operations called from inside callbacks.  Plus other changes.
//
// Revision 1.7  2004/08/21 11:36:51  atnak
// Removed unnecessary function define.
//
// Revision 1.6  2004/08/20 09:16:19  atnak
// Fixed the anti-deadlock no-lock sanctuary for the notication callbacks.
//
// Revision 1.5  2004/08/20 07:30:32  atnak
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

#ifndef __SETTINGGROUP_H__
#define __SETTINGGROUP_H__

#define SETTINGGROUP_THREAD_SAFE

#include "SettingRepository.h"
#include "SettingValue.h"
#include <vector>
#include <list>

enum eSettingGroupNotifyMessage
{
	// Notifies when the setting's value is changing.  The values
	// of newValue and oldValue hold their respective values.  CHANGING
	// is sent before the setting's value has actually changed, while
	// CHANGED is sent after the change has been finalized. Returning FALSE
	// to CHANGING prevents newValue from being used.
	NOTIFY_VALUE_CHANGING,
	NOTIFY_VALUE_RECHANGING,
	NOTIFY_VALUE_CHANGED,

	// Similar to CHANGING/CHANGED but notifies only the first time a
	// setting's value is loaded (or set).  The value of oldValue is
	// undefined.  Returning FALSE to SETTING prevents newValue from
	// being used.
	NOTIFY_VALUE_SETTING,
	NOTIFY_VALUE_RESETTING,
	NOTIFY_VALUE_SET,

	// This is sent after all other notifications (including those of
	// deferred operations) of a single non-deferred operation are sent.
	// For example, in a LoadSettings() call, all CHANGING notification
	// of every setting is sent, followed by all CHANGED notification.
	// Finally NOTIFY_AFTER_CHANGES is sent.
	NOTIFY_AFTER_CHANGES,

	NOTIFY_MARKER_QUEUE,
	NOTIFY_MARKER_QUEUED,
};

enum eSettingGroupNotifyReply
{
	NOTIFY_REPLY_CONTINUE		= TRUE,
	// Valid response only to CHANGING and SETTING.  Prevents the change
	// from occurring.
	NOTIFY_REPLY_DONT_CHANGE	= FALSE,
	// Valid response only in the global notification callback.  Prevents
	// the notification from being reported to the key.
	NOTIFY_REPLY_DONT_NOTIFY	= 2,
};

// Flags for the option parameter of functions.
enum eSettingGroupOption
{
	// Notify values even when they do not change.
	SGOF_REVISE					= 1 << 0,
	// By default, loading functions will skip settings
	// that are already loaded.  This forces the value
	// to be reread from the file.
	SGOF_RELOAD					= 1 << 1,
	// No notifications will not be generated as a
	// result of the operation.  This flag contradicts
	// SGOF_REVISE.
	SGOF_SILENT					= 1 << 2,

	// Flags used by SettingGroupEx.

	// This flag forces a full dependent consistency check
	// on all settings.  This flag is used internally and
	// setting it is not required by calling applications.
	SGOF_FULLSYNC				= 1 << 3,
};

//
// Notifications:
//
// Notifications are sent when a setting's value is changing.  Out of the
// six notifications, CHANGING, RECHANGING, CHANGED, SETTING, RESETTING,
// SET, the later three have the same roles as the three before but are
// used the very first time the setting's value is set (or loaded).  From
// here on, notifications will be referred to as CHANGING, RECHANGING and
// CHANGED even where SETTING, RESETTING and SET may apply.
//
// When a setting's value is to change, the CHANGING notification is sent.
// If the return value of the CHANGING notification callback is TRUE, the
// new value is applied and a CHANGED notification is generated.  If it is
// FALSE, the new value is discarded.
//
// All notifications are sent, in order, to three notification callbacks.
// First, it is sent to any global notification callback for the group, to
// which the changing setting belongs.  Next, it is sent to the setting
// object's notification callback.  (This callback is internal and only
// has an effect if the object supports it.)  Finally, it is sent to the
// notification callback of the setting key associated with the setting.
// Returning FALSE for CHANGING or RECHANGING in any of the three
// callbacks will prevent the change.
//
// The global notification callback can be set for a group using
// SetNotifyProc().
//
// The order of three notification is broken if any of the callbacks
// return FALSE to prevent the change.  In this case, the proceeding
// callbacks are not called.  This applies even when the notification is
// CHANGED, even though returning FALSE will have no other effect.  In the
// global notification callback only, returning NOTIFY_REPLY_DONT_NOTIFY
// will acknowledge the change but will stop the callback chain.
//
// The RECHANGING notification is generated when a setting that was
// notified with CHANGING needs to change again before CHANGED is called.
// This will only happen as a result of outside intervention.  (i.e.
// Performing another operation from inside a notification callback that
// causes an already notified setting to change again.)  Returning TRUE
// for RECHANGING acknowledges value.  Returning FALSE will discard the
// new value, but a CHANGED notification will be generated nonetheless for
// original CHANGING notification.
//
// All value changes are applied straight after the acknowledging return
// of CHANGING, or RECHANGING.  (Although not before all three callbacks
// in the notification order has been called.)  Thus, querying a
// settings's value during the time between CHANGING (or RECHANGING) and
// CHANGED will always see the new value.
//

//
// Notification queue:
//
// For an operation like LoadSettings(), where many setting values can
// change, the CHANGING notifications for all settings that changes will be
// sent first, then all their respective CHANGED notifications are sent.
// As stated in the paragraph above, new values are applied to the setting
// straight after the acknowledging return of a CHANGING notification.
// Hence, this is before the CHANGING notifications of any other settings.
//
// The CHANGED notifications for acknowledged CHANGING notifications are
// queued, and are later notified in the same order as which CHANGING
// notifications were generated.
//

//
// Operation deferring:
//
// There are a few operations that will be deferred, if called from
// inside a notification callback.  This means that the actual operation
// is performed, not when the function is called, but after the existing
// notification returns.  Moreover, any notifications generated from the
// deferred operation will be queued on the same queue that is used by
// the operation that owns the notification callback in which the
// operation was deferred.
//
// By default, deferring is enabled for CHANGING and RECHANGING.  It is
// reset to this default every time a notification is generated.  Calling
// EnableDeferring(FALSE) will turn off deferring for the remainder of
// that single notification callback.
//
// Although it is possible to enable deferring inside any notification
// callback, it is probably pointless to do so in those other than the
// CHANGING and RECHANGING notifications.  Note that if operations are
// deferred inside a NOTIFY_AFTER_CHANGES notification, a
// NOTIFY_AFTER_CHANGES will not be generated again after those deferred
// operations are performed.
//
//
// These non-notifying functions are deferrable:
// Silence(), Suspend()
//
// These are notifying functions (all which are deferrable):
// LoadSettings(), LoadSetting(), SetValue(), UseDefault(),
// SetDefault(), CheckLimiter(), SetLimiter(), InsertMarker(),
//
// Additional notifying functions in CSettingGroupEx (all which are deferrable):
// Activate(), LoadOptionalDependencies(), EnableOptionalDependencies(),
// SetEnabledOptionalDependencies(), SetOptionalDependantBits(), JostleBit()
//

//
// Insert marker:
//
// InsertMarker() is a function whose sole purpose is for generating two
// notifications.  When InsertMarker() is called, a NOTIFY_MARKER_QUEUE is
// generated.  Provided FALSE is not returned for the NOTIFY_MARKER_QUEUE,
// a NOTIFY_MARKER_QUEUED notification is generated.  These two notifications
// work in the same way as the CHANGING/CHANGED combination.  Just like a
// CHANGED notification, the NOTIFY_MARKER_QUEUED notification is queued at
// the end of the existing notification queue (although InsertMarker() alone
// will not create much of a queue since it touches no settings).  When used
// with operation deferring, InsertMarker() can be used to perform tasks at
// specific points after other operations.
//

//
// Threading safety:
//
// These functions can be accessed from multiple threads:
// SaveSetting(), GetValue(), GetDefault(), GetLimiter()
//
// The rest should only be accessed from one thread.
//

// Typedef class pointers to simple names
typedef class CSettingGroup_ *PSETTINGGROUP;
typedef class CSettingGroupEx *PSETTINGGROUPEX;

typedef class _HSETTING { } *HSETTING;

// Every notification generated by CSettingGroup_ is first sent to
// a global callback before being distributed to an individual key.
typedef BOOL (*PSETTINGGROUP_NOTIFYPROC)(INT message,
										 RCSETTINGVALUE newValue,
										 RCSETTINGVALUE oldValue,
										 HSETTING setting,
										 PVOID context);


//////////////////////////////////////////////////////////////////////////
// CSettingGroup_
//////////////////////////////////////////////////////////////////////////
class CSettingGroup_
{
public:
	CSettingGroup_(IN std::string section, IN PSETTINGREPOSITORY repository);
	virtual ~CSettingGroup_();

	// Adds a setting to the group and associates it to the key given.
	virtual HSETTING AddSetting(IN PSETTINGOBJECT object, IN PSETTINGKEY key = NULL);
	// Adds a setting with a title to the group and associates it to the key given.
	virtual HSETTING AddSetting(IN std::string title, IN PSETTINGOBJECT object, IN PSETTINGKEY key = NULL);

	// Sets the callback function that will be called when a notification
	// is generated by CSettingGroup_.  The context pointer value is not used
	// by CSettingGroup_ directly but will be sent to all calls of the callback.
	virtual void SetNotifyProc(IN PSETTINGGROUP_NOTIFYPROC, IN PVOID context);
	// This function is only valid when called from inside a notification
	// callback.  When deferring is enabled, operations of particular functions
	// called from inside the notification callback well be deferred until after
	// the callback has returned.
	virtual void EnableDeferring(IN BOOL enable = TRUE);

	// Load all settings owned by this group
	virtual void LoadSettings(IN BYTE options = 0);
	// Save all settings owned by this group
	virtual void SaveSettings();

	// Generates a NOTIFY_MARKER_QUEUE followed by a NOTIFY_MARKER_QUEUED.
	virtual void InsertMarker(IN INT markerID);

	// Stops all notifications from being sent.
	virtual void Silence(IN BOOL silence);
	virtual inline BOOL IsSilent() const;

	// Gets the PSETTINGKEY that is associated with the setting.
	virtual PSETTINGKEY GetSettingKey(IN HSETTING setting);
	// Gets the setting's title.
	virtual std::string GetSettingTitle(IN HSETTING setting);

	// Returns TRUE if CHANGING has been sent and CHANGED is pending.
	virtual BOOL IsPendingChanged(IN HSETTING setting);
	// Removes any CHANGED notification that is pending after a CHANGING
	// notification has been sent.  Using this creates an exception to the
	// rule that a CHANGED notification will be sent for every CHANGING.
	virtual BOOL SilencePendingChanged(IN HSETTING setting);

	// Incoming link functions for CSettingKey.  Although these functions can
	// be called directly with the value of HSETTING returned by AddSetting(),
	// using a CSettingKey will simply the task.
	virtual void LoadSetting(IN HSETTING setting, IN BYTE options = 0);
	virtual void SaveSetting(IN HSETTING setting);

	virtual void SetValue(IN HSETTING setting, IN RCSETTINGVALUE value, IN BYTE options = 0);
	virtual CSettingValue GetValue(IN HSETTING setting);

	virtual void UseDefault(IN HSETTING setting, IN BYTE options = 0);
	virtual void SetDefault(IN HSETTING setting, IN RCSETTINGVALUE value, IN BYTE options = 0);
	virtual CSettingValue GetDefault(IN HSETTING setting);

	virtual void CheckLimiter(IN HSETTING setting, IN BYTE options = 0);
	virtual void SetLimiter(IN HSETTING setting, IN PSETTINGLIMITER limiter, IN BYTE options = 0);
	virtual PSETTINGLIMITER GetLimiter(IN HSETTING setting);

protected:
	// Flags used by CSettingInfo->flags.
	enum
	{
		// Set when CHANGED notification should be sent for the setting
		FLAG_CHANGING			= 1 << 0,
		// Sets if the change notifications are the initial version
		FLAG_INITIAL			= 1 << 1,
		// Keep track of how many flags there're.
		FLAG_GROUP_LAST			= 2,
	};

	// Break up of the op value
	enum
	{
		OP_TYPE_OFFSET			= 6,
		OP_MASK					= 0x3f,
		OP_TYPE_MASK			= 0xc0,
	};

	// Deferred operations
	enum
	{
		OP_SILENCE				= 0,
		OP_LOAD_SETTINGS,
		OP_INSERT_MARKER,
		OP_LOAD_SETTING,
		OP_SET_VALUE,
		OP_USE_DEFAULT,
		OP_SET_DEFAULT,
		OP_CHECK_LIMITER,
		OP_SET_LIMITER,
		OP_GROUP_LAST
	};

	// Deferred operation type
	enum
	{
		OP_TYPE_STATE			= 0 << OP_TYPE_OFFSET,
		OP_TYPE_CONTAINED		= 1 << OP_TYPE_OFFSET,
		OP_TYPE_OBJECT			= 2 << OP_TYPE_OFFSET,
	};

	// Flags used by NOTIFICATIONSTATE flags.
	enum
	{
		FLAG_DEFER				= 1 << 0,
	};


	// This class defines the buffer that is used to store information
	// about a single setting in the class's persistent storage.
	// it should really be protected but vs6 doesn't like that
public:
	typedef class CSettingInfo
	{
	public:
		CSettingInfo(PSETTINGKEY key, PSETTINGOBJECT object, std::string title = "");
		virtual ~CSettingInfo();
	public:
		PSETTINGKEY			key;
		PSETTINGOBJECT		object;
		std::string			title;
		BYTE				flags;
	} *PSETTINGINFO;

protected:
	// Type used to store list of all setting information.
	typedef std::list<PSETTINGINFO>	SETTINGINFOLIST;

	typedef class CHANGEVALUES
	{
	public:
		CHANGEVALUES() : isSet(FALSE) { };

		void Set(IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue)
		{
			this->newValue = newValue;
			this->oldValue = oldValue;
			this->isSet = TRUE;
		}

	public:
		BOOL			isSet;
		CSettingValue	newValue;
		CSettingValue	oldValue;
	} CHANGEVALUES, *PCHANGEVALUES;

	// This structure is used to store temporally data that is
	// used by the common change routines.
	typedef struct
	{
		PSETTINGGROUP	group;
		PSETTINGINFO	info;
		CSettingValue	oldValue;
	} CHANGEDNOTIFY, *PCHANGEDNOTIFY;

	// Stores information necessary about a single operation.
	typedef struct OPERATIONINFO
	{
		OPERATIONINFO(PSETTINGGROUP group) : op(0), options(0),
			info(NULL), group(group), lParam(NULL) { };
		BYTE					op;
		BYTE					options;
		PSETTINGINFO			info;
		PSETTINGGROUP			group;
		union
		{
			PSETTINGVALUE		value;
			PSETTINGLIMITER		limiter;
			PVOID				ptr;
			LPARAM				lParam;
		};
	} OPERATIONINFO, *POPERATIONINFO;

	// Type used to store list of settings that are changing.
	typedef std::list<CHANGEDNOTIFY> CHANGEDNOTIFYLIST, *PCHANGEDNOTIFYLIST;
	// Type used to store a list of deferred operations.
	typedef std::list<OPERATIONINFO> OPERATIONLIST, *POPERATIONLIST;

	typedef struct
	{
		PSETTINGINFO		identity;
		BYTE				flags;
		POPERATIONLIST		deferList;
	} NOTIFICATIONSTATE, *PNOTIFICATIONSTATE;


	// Stack for storing information for durations when changes occur.
	typedef std::list<PCHANGEDNOTIFYLIST> CHANGEDNOTIFYSTACK, *PCHANGEDNOTIFYSTACK;
	// Stack for storing information for windows of notification time;
	typedef std::list<PNOTIFICATIONSTATE> NOTIFICATIONSTACK, *PNOTIFICATIONSTACK;

	// Initialize and cleanup multithread protection.
	virtual inline void InitializeLocking();
	virtual inline void CleanupLocking();

	// Multi-thread protection lock and unlock functions.  Not the best
	// form of protection in terms of performance but it should provide
	// sufficient safety as long as no two threads call a changing
	// function at the same time.  The purpose of this lock is for
	// protecting access when one thread calls a value getting function
	// while another is changing the values of settings.
	virtual inline void EnterObjectLock();
	virtual inline void LeaveObjectLock();

	// Puts info onto the setting info list.
	virtual void RegisterSetting(IN PSETTINGINFO info);

	// Sets or unsets flag(s) in info->flags.
	virtual inline void SetInfoFlag(IN PSETTINGINFO info, IN BYTE flags, IN BOOL set);
	virtual inline BOOL GetInfoFlag(IN PSETTINGINFO info, IN BYTE flags, IN BOOL strict = FALSE);

	virtual BOOL IsSilent(IN BYTE options) const;

	// This function should notify CHANGING and change the values of all
	// settings then put the appropriate CHANGEDNOTIFY structure onto the
	// changed notify list.
	virtual void LoadSettings(IN POPERATIONINFO opinfo);
	// This function performs the work of InsertMarker().
	virtual void InsertMarker(IN POPERATIONINFO opinfo);

	// Load a single setting.
	virtual void LoadSetting(IN POPERATIONINFO opinfo);

	// Processes operations and defers if necessary.
	virtual void ProcessOperation(IN POPERATIONINFO opinfo);
	// Performs the operation described by opinfo.
	virtual void PerformOperation(IN POPERATIONINFO opinfo);
	// Functions for performing specific types of operations.
	virtual void PerformStateOperation(IN POPERATIONINFO opinfo);
	virtual void PerformContainedOperation(IN POPERATIONINFO opinfo);
	virtual void PerformObjectOperation(IN POPERATIONINFO opinfo, IN PCHANGEVALUES values);

	// Performs the necessary notifications and changes when a setting value changes.
	virtual BOOL ProcessValueChange(IN PSETTINGINFO info, IN PCHANGEVALUES values,
									IN BYTE options);

	// Get the changed notification stack and the active list.
	virtual inline PCHANGEDNOTIFYSTACK GetChangedNotifyStack();
	virtual inline PCHANGEDNOTIFYLIST GetActiveChangedNotifyList();

	// Sets the active changed notification list.
	virtual inline void SetActiveChangedNotifyList(IN PCHANGEDNOTIFYLIST changedList);
	virtual inline void RevertActiveChangedNotifyList();

	// Enqueues, updates and processes the changed notification list.
	virtual void EnqueueChangedNotification(IN PCHANGEDNOTIFY notification);
	virtual void SendChangedNotifications(IN PCHANGEDNOTIFYLIST changedList);
	virtual void UpdateChangedNotification(IN PSETTINGINFO info, IN BOOL removeOnly);

	// Gets the notification state stack and the active notification.
	virtual inline PNOTIFICATIONSTACK GetNotificationStack();
	virtual inline PNOTIFICATIONSTATE GetActiveNotification();

	// Enqueues, stack and processes the deferred operations list.
	virtual void EnqueueDeferredOperation(IN POPERATIONINFO opinfo);
	virtual void StackDeferredOperations(IN POPERATIONLIST oplist);
	virtual void ProcessDeferredOperations();

	// This callback is be called by CSettingObject when a setting is about
	// to change its value.  This routine just grabs the new and old values and
	// blocks the change.
	static BOOL GetValueChangeProc(RCSETTINGVALUE newValue,
								   RCSETTINGVALUE oldValue,
								   PCSETTINGOBJECT object,
								   PVOID context);

	// This method takes care of sending a notification first to the global
	// notification callback then to the respective setting key.
	virtual inline BOOL Notify(IN PSETTINGINFO info, IN INT message,
		IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue);

	// Shortcut for sending CHANGING notifications
	virtual inline BOOL NotifyChanging(IN PSETTINGINFO info,
		IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue);
	// Shortcut for sending RECHANGING notifications
	virtual inline BOOL NotifyRechanging(IN PSETTINGINFO info,
		IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue);
	// Shortcut for sending CHANGED notifications
	virtual inline BOOL NotifyChanged(IN PSETTINGINFO info,
		IN RCSETTINGVALUE newValue, IN RCSETTINGVALUE oldValue);
	// Shortcut for sending NOTIFY_MARKER_QUEUE notifications
	virtual inline BOOL NotifyMarkerQueue(IN RCSETTINGVALUE markerValue);
	// Shortcut for sending NOTIFY_MARKER_QUEUED notifications
	virtual inline BOOL NotifyMarkerQueued(IN RCSETTINGVALUE markerValue);
	// Shortcut for sending NOTIFY_AFTER_CHANGES notifications.
	virtual inline BOOL NotifyAfterChanges();

protected:
	CSettingRepository*			m_repository;
	SETTINGINFOLIST				m_settingList;
	// Repository section string for this group
	std::string					m_section;

	CHANGEDNOTIFYSTACK			m_changedNotifyStack;
	NOTIFICATIONSTACK			m_notificationStack;

	POPERATIONLIST				m_deferredOperationList;

	// Notifications are silenced if this is greater than zero.
	CHAR						m_silent;
	// Critical section used for protecting group access.
	CRITICAL_SECTION			m_objectLockCriticalSection;

	// Global notify callback and its context value
	PSETTINGGROUP_NOTIFYPROC	m_notifyProc;
	PVOID						m_callbackContext;
};


//////////////////////////////////////////////////////////////////////////
// CSettingGroupEx - Extension for associated settings
//////////////////////////////////////////////////////////////////////////

//
// Associated settings allow for a subset of settings in the group to
// differ in value depending on the value of a dependee setting to which
// it is associated to.  The different values of the single setting is
// stored in different sections of the repository so that it can be
// recalled as needed, as the dependee setting's value changes.  A single
// setting can be associated to depend on the values of multiple dependee
// settings.  (In which case a change in any of the dependee settings
// will trigger an update.)
//
// To create an association, first create an "dependency bit" (association
// bit) using CreateDependency().  The created bit is then used to refer
// to a single dependee-to-dependants association from there on.  The
// dependency bit is a single binary bit in an integer variable.  It can
// be unioned or masked with other dependency bits with standard bitwise
// operators.  The maximum number of dependency bits that can be created
// depends on the size of the DBIT type and is exactly the number of bits
// in the size.  (The equation sizeof(DBIT) * 8 will give the exact limit.)
//
// Once a dependency bit is created, it can be used in an association.  A
// setting added to the group or subgroup with AddMaster() can be set as
// the dependee of the association by passing the dependency bit as the
// dependeeBit value of the function.  Settings added to the group or
// subgroup with AddSetting() or AddMaster() can be set as dependents of an
// association by passing the dependency bit as the dependentOptionalBits
// or dependentAbsoluteBits value of the function.
//
// A setting can be a dependee to at most one association but can be
// dependent to multiple associations.  Although no checks are performed,
// cyclic dependencies are not allowed. (i.e. A setting cannot be dependent
// to itself whether directly or indirectly.)  The result is undefined if
// any of these criteria are not met.
//
// The AddMaster() and AddSetting() parameters dependantOptionalBits and
// dependantAbsoluteBits differ in that dependency bits set in
// dependantOptionalBits will only act in an association if a mask given
// to SetDependantMask() includes the same bit.  Dependency bits passed
// to dependantAbsoluteBits will always form an association.  By default,
// the mask is zero (no association) before SetDependantMask() is called.
//
// Groups suspended with Suspend() will not process linked operations until
// a later time when it is reactivated with Activate().  This means that
// although it will perform SetValue() and change the value of the single
// setting, any settings that depend on the changed setting are not updated
// if either the dependee or dependent setting is in a suspended group (or
// a subgroup of a suspended group).  Upon calling Activate(), all
// outstanding updates for the group's settings and of any non-suspended
// subgroups are performed on the spot.
//
// LoadSettings() and LoadSetting() will have no effect if called on a
// suspended group.
//

// Type for holding dependency bits.
typedef BYTE DBIT;


class CSettingGroupEx : public CSettingGroup_
{
public:
	CSettingGroupEx(IN std::string section, IN PSETTINGREPOSITORY repository,
		PSETTINGGROUPEX parent = NULL);
	virtual ~CSettingGroupEx();

	// Creates a new dependency bit.  The returned value can be passed
	// as the dependantOptionalBits and dependantAbsoluteBits values of
	// AddSetting() and AddMaster() in this group and subgroups.  It can
	// also be combined with other values returned by this same function
	// by bitwise-ORing the values together.
	virtual DBIT CreateDependency(IN std::string title, IN std::string section);

	// Adds a setting to the group
	virtual HSETTING AddSetting(IN PSETTINGOBJECT object, IN PSETTINGKEY key = NULL,
								IN DBIT dependantOptionalBits = 0,
								IN DBIT dependantAbsoluteBits = 0);

	// Adds a master setting to the group
	virtual HSETTING AddMaster(IN PSETTINGOBJECT object, IN PSETTINGKEY key,
							   IN DBIT dependeeBits,
							   IN DBIT dependantOptionalBits = 0,
							   IN DBIT dependantAbsoluteBits = 0);

	// Adds a setting with a title to the group
	virtual HSETTING AddSetting(IN std::string title,
								IN PSETTINGOBJECT object, IN PSETTINGKEY key = NULL,
								IN DBIT dependantOptionalBits = 0,
								IN DBIT dependantAbsoluteBits = 0);

	// Adds a master with a title setting to the group
	virtual HSETTING AddMaster(IN std::string title,
							   IN PSETTINGOBJECT object, IN PSETTINGKEY key,
							   IN DBIT dependeeBits,
							   IN DBIT dependantOptionalBits = 0,
							   IN DBIT dependantAbsoluteBits = 0);

	// Save all settings owned by this group.
	virtual void SaveSettings();
	// Save a single setting owned by this group.
	virtual void SaveSetting(IN HSETTING setting);

	// Loads the optional dependants mask (the value that is set with
	// SetDependantMask()) and loads previously saved dependantOptionalBits
	// values for every setting.
	virtual void LoadOptionalDependencies(IN BYTE options = 0);
	// Saves the optional dependants mask and saves any dependantOptionalBits
	// values that that have changed since the adding or loading of the setting.
	virtual void SaveOptionalDependencies();

	// Creates a subgroup.  Subgroups share the same dependency bits
	// and pretty much all the other attributes as the parent.  Their
	// only apparent use is when only a subset of settings need to be
	// suspended.  The pointer returned by this function should not
	// be deleted directly.  Deleting the parent deletes all its
	// subgroups.
	virtual PSETTINGGROUPEX CreateSubgroup();

	// Enables or disables dependeeBits that are set in the dependantOptionalBits
	// value of every setting.  This function cannot be called from a subgroup.
	// If suspended is TRUE, any related updates are not performed.
	virtual inline void EnableOptionalDependencies(IN DBIT dependeeBit, IN BOOL set,
												   IN BYTE options = 0);

	// Sets the bit list of all optional dependee bits that are enabled.
	virtual inline void SetEnabledOptionalDependencies(IN DBIT mask,
													   IN BYTE options = 0);

	// Gets the bit list of all optional dependee bits that are enabled.
	virtual inline DBIT GetEnabledOptionalDependencies();

	// Stops settings in the group and its sub-groups from processing
	// dependencies.  Groups are not suspended by default at creation.
	virtual inline void Suspend();
	// Gets whether or not the group is suspended
	virtual inline BOOL IsSuspended();
	// Re-enables dependencies processing and synchronizes any changes
	// that happened while suspended.
	virtual void Activate(IN BYTE options = 0);

	// Forces the group to check and change all its settings as if the setting
	// associated to dependeeBit changed its value to dependeeValue.  Any
	// setting associated to dependeeBit (if any) is not directly changed in
	// the calling of this.  If suspended is TRUE, related updates are not
	// performed.
	virtual void JostleBit(IN DBIT dependeeBit, IN RCSETTINGVALUE dependeeValue,
						   IN BYTE options = 0);

	// Gets the dependee bits of a setting.
	virtual DBIT GetDependeeBits(IN HSETTING setting);

	// Gets optional and absolute dependant bits of a setting.
	virtual DBIT GetOptionalDependantBits(IN HSETTING setting);
	virtual DBIT GetAbsoluteDependantBits(IN HSETTING setting);

	// Sets the optional dependant bits of a setting.  This function, like
	// AddSetting() and AddMaster() does not check for cyclic dependencies.
	virtual void SetOptionalDependantBits(IN HSETTING setting,
										  IN DBIT dependantOptionalBits,
										  IN BYTE options = 0);

	// Creates a CSettingConfigAssociation object that is filled with all the
	// settings in the group.  The returned object should be deleted with
	// 'delete' when it is no longer needed.  Since some values used by the
	// configuration class is cached within the object, any changes that
	// happen to associations in the group will not be visible to any
	// CSettingConfigAssociation objects that were created before the change.
	virtual PSETTINGCONFIG CreateAssociationConfig(std::string title,
												   BOOL addsettings = FALSE);

protected:
	// Flags used by CSettingInfoEx->flags.
	enum
	{
		// Set when dependantOptionalBits of a setting is changed
		// since last load.
		FLAG_OPTDEPCHANGED		= 1 << (FLAG_GROUP_LAST),
		FLAG_GROUPEX_LAST		= FLAG_GROUP_LAST + 1,
	};

	// Deferred operations
	enum
	{
		OP_SUSPEND				= OP_GROUP_LAST,
		OP_ACTIVATE,
		OP_JOSTLE_BIT,
		OP_GROUPEX_LAST

	};

	// This type is an extension of CSettingInfo to hold the necessary
	// information of settings with dependencies.
    class CSettingInfoEx : public CSettingGroup_::CSettingInfo
	{
	public:
		CSettingInfoEx(IN PSETTINGKEY key, IN PSETTINGOBJECT object,
			std::string title = "", IN DBIT dependeeBit = 0,
			IN DBIT dependantOptionalBits = 0, IN DBIT dependantAbsoluteBits = 0);
		virtual ~CSettingInfoEx();
	public:
		DBIT dependeeBit;
		DBIT dependantOptionalBits;
		DBIT dependantAbsoluteBits;
		DBIT loadedDependantBits;
	};
    typedef CSettingInfoEx* PSETTINGINFOEX;

	// This vector type is used to hold the dependee values for
	// those used by the group.
	typedef std::vector<CSettingValue> DEPENDVALUEVECTOR;
	typedef DEPENDVALUEVECTOR *PDEPENDVALUEVECTOR;
	typedef const DEPENDVALUEVECTOR *PCDEPENDVALUEVECTOR;

	//////////////////////////////////////////////////////////////////////////
	// CSettingGroupEx::CDependencyGestalt
	//////////////////////////////////////////////////////////////////////////
	typedef class CDependencyGestalt
	{
	public:
		CDependencyGestalt();
		virtual ~CDependencyGestalt();

		// Create a new dependee bit
		DBIT CreateDependee(IN std::string title, IN std::string section);

		// Gets the number of dependee bits that have been created.
		inline BYTE GetDependeeCount();
		// Gets the title of a specific dependee bit.
		inline std::string GetDependeeTitle(BYTE index);
		// Gets the mask of all valid bits.
		inline DBIT GetValidDependeeBits();

		// Creates a string that describes the section for the dependantBits given.
		std::string CreateSection(IN DBIT dependantBits, IN LPCSTR baseSection,
								  IN LPCSTR sectionDelimiter,
								  IN PCDEPENDVALUEVECTOR dependeeValueVector = NULL);

		// Puts the dependee change into the pending changes list.  Returns
		// TRUE if the value actually causes a change.
		BOOL RegisterDependeeChange(IN PCDEPENDVALUEVECTOR dependeeValueVector,
									IN DBIT dependeeBit, IN RCSETTINGVALUE value);

		// Gets all the bits that have changed between currentValueVector and the
		// internal pending changes vector.  Only the bits set in the mask are compared.
		DBIT CompareVectorChanges(IN PCDEPENDVALUEVECTOR dependeeValueVector,
								  IN DBIT mask = ~(DBIT)0);

		// Forget all changes registered so far.
		void ClearVectorChanges();
		// Copies all pending changes to currentValueVector.  Only the bits set
		// in the mask are copied.
		void ApplyVectorChanges(IN OUT PDEPENDVALUEVECTOR dependeeValueVector,
								IN DBIT mask = ~(DBIT)0);

		// Sets and gets the dependant mask value.  This value is just stored by
		// this class.  It isn't used as part of any other process in the class.
		inline BOOL SetDependantMask(IN DBIT dependantMask);
		inline DBIT GetDependantMask();

	protected:
		inline BYTE DependeeBitToIndex(IN DBIT dependeeBit);

	private:
		class CDependeeInfo
		{
		public:
			CDependeeInfo(std::string title, std::string section) :
				title(title), section(section) { };
			virtual ~CDependeeInfo() { };
			std::string title;
			std::string section;
		};

		std::vector<CDependeeInfo>	m_dependeeList;
		DBIT						m_validDependeeBits;
		DEPENDVALUEVECTOR			m_dependeeValueVector;
		DBIT						m_dependantMask;
	} *PDEPENDENCYGESTALT;

protected:
	// This type is used to hold a list of all the subgroups.
	typedef std::list<CSettingGroupEx*> SUBGROUPEXLIST;

	// Structure for holding temporally values required during
	// the jostling process.
	typedef struct JOSTLESTRUCT
	{
		JOSTLESTRUCT(DBIT changedBits, DBIT checkedBits, BYTE options)
			: saveSectionCacheBits(0), loadSectionCacheBits(0),
			changedBits(0), checkedBits(0), options(0) { }

	public:
		DBIT			changedBits;
		DBIT			checkedBits;
		BYTE			options;
		DBIT			saveSectionCacheBits;
		std::string		saveSectionCacheString;
		DBIT			loadSectionCacheBits;
		std::string		loadSectionCacheString;
	} JOSTLESTRUCT, *PJOSTLESTRUCT;


	// The string that is used to delimit sub-section when saving and
	// loading from the repository.  Changing will lose previously
	// saved sections
	static const LPCSTR SECTIONDELIM;
	// This string is attached to the section string for the section
	// used to store optional dependant bits for every setting.
	static const LPCSTR DEPENDANTSECTIONPOSTFIX;
	static const LPCSTR DEPENDANTMASKSAVEKEY;

	// Internal functions for saving and loading optional dependant bits.
	virtual void _SaveOptionalDependencies(IN LPCSTR, IN LPSTR, IN DWORD);
	virtual void _LoadOptionalDependencies(IN LPCSTR, IN DBIT, IN LPSTR, IN DWORD);

	virtual PSETTINGGROUPEX GetRootParent();

	virtual void LoadSettings(IN POPERATIONINFO opinfo);
	virtual void LoadSetting(IN POPERATIONINFO opinfo);
	virtual void Activate(IN POPERATIONINFO opinfo);
	virtual void JostleBit(IN POPERATIONINFO opinfo);

	// Overrides for slight changes in operations for group ex.
	virtual void PerformStateOperation(IN POPERATIONINFO opinfo);
	virtual void PerformContainedOperation(IN POPERATIONINFO opinfo);

	// Performs all necessary calls for a single setting value change.
	virtual BOOL ProcessValueChange(IN PSETTINGINFO info, IN PCHANGEVALUES values,
		IN BYTE options);

	// Checks and processes all setting in the group and subgroups for changes.
	virtual void JostleAllSettings(IN DBIT changedBits,
								   IN DBIT checkedBits, IN BYTE options);
	// Checks and processes the setting for any change necessary because of another
	// setting change.
	virtual void JostleSetting(IN PSETTINGINFOEX info, IN PJOSTLESTRUCT jostleStruct);
	// Finds masters for dependees set by dependeeBits and calls JostleSetting() on those
	virtual void JostleMasters(IN DBIT dependeeBits, IN PJOSTLESTRUCT jostleStruct);
	// Finds masters calls JostleSetting() on those.  Without base group checking.
	DBIT _JostleMasters(IN DBIT dependeeBits, IN PJOSTLESTRUCT jostleStruct);

	// Override to get the same changed notify stack for subgroups.
	virtual inline PCHANGEDNOTIFYSTACK GetChangedNotifyStack();;
	// Override to get the same notification stack for subgroups.
	virtual inline PNOTIFICATIONSTACK GetNotificationStack();

	// Gets the bits that a setting depends on.
	virtual inline DBIT GetDependantBits(IN PSETTINGINFOEX info);
	// Gets the string describing the section to which a setting belongs.
	virtual LPCSTR GetSection(IN DBIT dependantBits, IN BOOL useOldValues,
		IN OUT std::string* cacheString, IN OUT DBIT* cacheBits);

	// Internal functions for adding all settings to the a config container.
	void _CreateAssociationConfig(PSETTINGCONFIG association);

	// Database of information about a specific dependency
	PDEPENDENCYGESTALT	m_dependencyGestalt;

	// A vector that this group's settings rely on for dependency.
	DEPENDVALUEVECTOR	m_dependedValues;
	// List of all subgroups directly owned by this group.
	SUBGROUPEXLIST		m_subgroupList;
	// Records the group's parent group
	PSETTINGGROUPEX		m_parentGroup;

	// Records whether or not this group is suspended.
	BOOL				m_suspended;

	// Records all the bits for which this group owns a master for.
	DBIT				m_haveDependeeBits;
};


#endif

