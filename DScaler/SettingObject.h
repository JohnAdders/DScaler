/////////////////////////////////////////////////////////////////////////////
// $Id: SettingObject.h,v 1.1 2004-08-06 17:12:10 atnak Exp $
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

#ifndef __SETTINGOBJECT_H__
#define __SETTINGOBJECT_H__

#include "SettingRepository.h"
#include "SettingLimiter.h"
#include "SettingValue.h"

// Typedef class pointers to simple names
typedef class CSettingObject *PSETTINGOBJECT;
typedef const class CSettingObject *PCSETTINGOBJECT;

// Callback function that can be given to Load() and SetValue()
// and will be called before the setting change is made.  The
// callback can return FALSE to prevent the setting change.
typedef BOOL (*PSETTINGOBJECT_CHECKPROC)(RCSETTINGVALUE newValue,
										 RCSETTINGVALUE oldValue,
										 PCSETTINGOBJECT object,
										 PVOID context);


//////////////////////////////////////////////////////////////////////////
// CSettingObject
//////////////////////////////////////////////////////////////////////////
class CSettingObject
{
public:
	CSettingObject() { };
	virtual ~CSettingObject() { };

	// Gets the key string that is used in the repository
	virtual inline LPCSTR GetKey() const =0;

	// Load the setting's value from the section in the repository provided
	virtual BOOL Load(IN PSETTINGREPOSITORY repository, IN LPCSTR section,
					  IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)=0;
	// Save the setting's value to the section in the repository provided
	virtual void Save(IN PSETTINGREPOSITORY repository, IN LPCSTR section)=0;

	// Gets whether the value is set
	virtual inline BOOL IsSet() const =0;

	// Set the setting's value to the specified value
	virtual BOOL SetValue(IN RCSETTINGVALUE value,
						  IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)=0;
	// Gets the current setting's value
	virtual CSettingValue GetValue() const =0;

	// Sets the setting's value to the value of default
	virtual BOOL UseDefault(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);

	// Sets the setting's default value
	virtual BOOL SetDefault(IN RCSETTINGVALUE value,
							IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);

	// Gets the default value.
	virtual CSettingValue GetDefault() const;

	// Check that the setting's limit is still within bounds of the limiter
	// or change the value if necessary.
	virtual BOOL CheckLimiter(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);

	// Sets the setting's value limiter.  The caller relinquishes ownership
	// of the limiter object.  CSettingObject will delete limiter when it is
	// no longer needed.  limiter must point to an object allocated with new.
	virtual BOOL SetLimiter(IN PSETTINGLIMITER limiter,
							IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);

	// Gets the setting's value limiter.  The limiter is stored in memory
	// owned by CSettingObject.  It should not be deleted.
	virtual PSETTINGLIMITER GetLimiter() const;

	// The object can have its own notify function that is called by the group.
	virtual BOOL Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue);

protected:
	// Load the value of type type from the section in the repository
	static BOOL Load(IN PSETTINGREPOSITORY repository, IN LPCSTR section,
					 IN LPCSTR key, IN BYTE type, OUT RSETTINGVALUE value);

	// Save the value to the section in the repository
	static void Save(IN PSETTINGREPOSITORY repository, IN LPCSTR section,
                     IN LPCSTR key, IN RCSETTINGVALUE value);

	// Puts value into store depending on the callback return value of checkProc
	static BOOL SetValue(IN OUT RSETTINGVALUE store, IN RCSETTINGVALUE value,
						 IN PCSETTINGOBJECT object,
						 IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);


private:
	enum eSettingObjectConstants
	{
		// Maximum amount of characters to allow for when reading
		// in settings from the file.
		kMaxLoadValueLength		= 512,
	};
};


//////////////////////////////////////////////////////////////////////////
// CSettingObjectContaiend extends/implements CSettingObject
//////////////////////////////////////////////////////////////////////////

//
// Load regime and how default is used:
//
// IF setting in repository
//   -> load
// ELSE
//   -> IF IsSet()
//        -> IF default newer than existing value
//             -> default
//           ELSE
//             -> keep existing value
//      ELSE
//        -> default

class CSettingObjectContained : private CSettingObject
{
public:
	// Key is a string representing the setting's identifier.
	// Type is one of the CSettingValue types that will be used
	// by this setting object.
	CSettingObjectContained(IN LPCSTR key, IN BYTE type);
	virtual ~CSettingObjectContained();

	// Gets the key string that is used in the repository
	inline LPCSTR GetKey() const;

	// Load the value from the section in the repository provided
	BOOL Load(IN PSETTINGREPOSITORY repository, IN LPCSTR section,
			  IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);
	// Save the value to the section in the repository provided
	void Save(IN PSETTINGREPOSITORY repository, IN LPCSTR section);

	// Gets whether the value is set
	inline BOOL IsSet() const;

	// Set the value to the specified value
	BOOL SetValue(IN RCSETTINGVALUE value, IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);
	// Gets the current value
	inline CSettingValue GetValue() const;

	// Sets the setting's value to that value in default
	BOOL UseDefault(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);

	// Set or get the default value that will be used when no other value
	// is available.
	BOOL SetDefault(IN RCSETTINGVALUE value, IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);
	// Gets the default value.
	CSettingValue GetDefault() const;

	// Check that the setting's limit is still within bounds of the limiter
	// or change the value if necessary.
	virtual BOOL CheckLimiter(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);

	// Caller relinquishes ownership of limiter object.
	// CSettingObject will delete limiter on destruction.
	BOOL SetLimiter(IN PSETTINGLIMITER limiter, IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);
	// Gets the setting's value limiter.  The limiter is stored in memory
	// owned by CSettingObject.  It should not be deleted.
	virtual PSETTINGLIMITER GetLimiter() const;

protected:
	// Set the value without limiter check
	BOOL _SetValue(IN RCSETTINGVALUE value, IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context);

private:
	// Setting's identifier
	LPCSTR				m_key;
	// Setting's value
	CSettingValue		m_value;
	// Setting's default
	CSettingValue		m_default;
	// Setting's value restrictions
	PSETTINGLIMITER		m_limiter;
	// Setting value that is in the repository
	CSettingValue		m_savedValue;
	// TRUE if default value is newer than setting's value
	BOOL				m_defaultIsNewer;
};

#endif
