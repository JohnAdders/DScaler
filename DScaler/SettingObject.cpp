/////////////////////////////////////////////////////////////////////////////
// $Id: SettingObject.cpp,v 1.3 2005-03-17 03:55:19 atnak Exp $
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
// Revision 1.2  2004/08/13 08:52:02  atnak
// Added a definable title to CSettingObject.
//
// Revision 1.1  2004/08/06 17:12:10  atnak
// Setting repository initial upload.
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingObject.h"
#include "SettingRepository.h"
#include "SettingLimiter.h"
#include "SettingValue.h"
#include <string>


//////////////////////////////////////////////////////////////////////////
// CSettingObject
//////////////////////////////////////////////////////////////////////////

std::string CSettingObject::GetTitle() const
{
	return "";
}


BOOL CSettingObject::UseDefault(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	return FALSE;
}


BOOL CSettingObject::SetDefault(IN RCSETTINGVALUE, IN PSETTINGOBJECT_CHECKPROC, IN PVOID)
{
	return FALSE;
}


CSettingValue CSettingObject::GetDefault() const
{
	return CSettingValue();
}


BOOL CSettingObject::CheckLimiter(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	return FALSE;
}


BOOL CSettingObject::SetLimiter(IN PSETTINGLIMITER, IN PSETTINGOBJECT_CHECKPROC, IN PVOID)
{
	return FALSE;
}


PSETTINGLIMITER CSettingObject::GetLimiter() const
{
	return NULL;
}


BOOL CSettingObject::Notify(INT, RCSETTINGVALUE, RCSETTINGVALUE)
{
	return TRUE;
}


BOOL CSettingObject::Load(IN PSETTINGREPOSITORY repository, IN LPCSTR section,
						  IN LPCSTR key, IN BYTE type, OUT RSETTINGVALUE value)
{
	CHAR buffer[kMaxLoadValueLength+1];

	// Load the setting from the repository as a string buffer
	if (repository->LoadSettingString(section, key, buffer, kMaxLoadValueLength+1))
	{
		value.FromString(buffer, type);
		return TRUE;
	}
	return FALSE;
}


void CSettingObject::Save(IN PSETTINGREPOSITORY repository, IN LPCSTR section,
						  IN LPCSTR key, IN RCSETTINGVALUE value)
{
	// Write the setting to the repository
	repository->SaveSettingString(section, key, value.ToString().c_str());
}


BOOL CSettingObject::SetValue(IN OUT RSETTINGVALUE store, IN RCSETTINGVALUE value,
							  IN PCSETTINGOBJECT object,
							  IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	// Call the callback and halt the change if it returns FALSE
	if (checkProc != NULL && !(checkProc)(value, store, object, context))
	{
		return FALSE;
	}

	// Set the new value
	store = value;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CSettingObjectContained
//////////////////////////////////////////////////////////////////////////

CSettingObjectContained::CSettingObjectContained(IN LPCSTR key, IN BYTE type) :
	CSettingObject(),
	m_limiter(NULL),
	m_defaultIsNewer(FALSE)
{
	// Create a new buffer for the key
	if ((m_key = new CHAR[strlen(key)+1]) == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	strcpy((CHAR*)m_key, key);

	// Save the object's type in m_default
	m_default.SetType(type);
}


CSettingObjectContained::~CSettingObjectContained()
{
	if (m_key != NULL)
	{
		delete [] m_key;
	}
	if (m_limiter != NULL)
	{
		delete m_limiter;
	}
}


LPCSTR CSettingObjectContained::GetKey() const
{
	return m_key;
}


BOOL CSettingObjectContained::Load(IN PSETTINGREPOSITORY repository, IN LPCSTR section,
								   IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	CSettingValue newValue;

	TRACE("+++ Loading Setting: %s\n", m_key);
	// Load the setting's value from the section in the repository into newValue
	if (CSettingObject::Load(repository, section, m_key, m_default.GetType(), newValue))
	{
		m_savedValue.Copy(newValue);

		// Make sure the value is within the limiter bounds
		if (m_limiter != NULL && m_limiter->OutOfLimit(newValue))
		{
			// Reset the value to default if the loaded value is not valid
			newValue = m_default;
		}
	}
	else
	{
		m_savedValue.Reset();

		// Since the setting was not found in the repository, use the default if a
		// value is not already set or if the default was set after than the value.
		if (!IsSet() || m_defaultIsNewer)
		{
			newValue = m_default;
		}
		else
		{
			// Keep the existing value.
			return FALSE;
		}
	}

	// Set the new value
	return _SetValue(newValue, checkProc, context);
}


void CSettingObjectContained::Save(IN PSETTINGREPOSITORY repository, IN LPCSTR section)
{
	TRACE("+++ Saving Setting: %s\n", m_key);
	// Save the value only if it has changed from what is in the repository
	if (m_value.IsSet() && !m_value.IsEqual(m_savedValue))
	{
		CSettingObject::Save(repository, section, m_key, m_value);
		m_savedValue = m_value;
	}
}


BOOL CSettingObjectContained::IsSet() const
{
	return m_value.IsSet();
}


BOOL CSettingObjectContained::SetValue(IN RCSETTINGVALUE value,
									   IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	ASSERT(value.GetType() == m_default.GetType());

	std::string str = value.ToString();
	TRACE("+++ Value Set: %s (%s)\n", m_key, str.c_str());
	CSettingValue newValue(value);

	if (m_limiter != NULL)
	{
		m_limiter->ApplyLimit(newValue);
	}

	// Set the new value
	return _SetValue(newValue, checkProc, context);
}


BOOL CSettingObjectContained::_SetValue(IN RCSETTINGVALUE value,
										IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	// Set the new value depending on the callback
	if (CSettingObject::SetValue(m_value, value, this, checkProc, context))
	{
		// The default is no longer newer than the value
		m_defaultIsNewer = FALSE;
		return TRUE;
	}
	return FALSE;
}


CSettingValue CSettingObjectContained::GetValue() const
{
	TRACE("+++ Value Get: %s\n", m_key);
	return m_value;
}


BOOL CSettingObjectContained::UseDefault(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	TRACE("+++ Value UseDefault: %s\n", m_key);
	return _SetValue(m_default, checkProc, context);
}


BOOL CSettingObjectContained::SetDefault(IN RCSETTINGVALUE value, IN PSETTINGOBJECT_CHECKPROC, IN PVOID)
{
	ASSERT(value.GetType() == m_default.GetType());

	TRACE("+++ Misc SetDefault: %s\n", m_key);
	if (!value.IsEqual(m_default))
	{
		// Save the new default
		m_default.Copy(value);
		// The default is newer than the setting's value
		m_defaultIsNewer = TRUE;
	}
	return FALSE;
}


CSettingValue CSettingObjectContained::GetDefault() const
{
	TRACE("+++ Query GetDefault: %s\n", m_key);
	return m_default;
}


BOOL CSettingObjectContained::CheckLimiter(IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	TRACE("+++ Value CheckLimiter: %s\n", m_key);
	if (IsSet())
	{
		if (m_limiter->OutOfLimit(m_value))
		{
			CSettingValue newValue(m_value);
			if (m_limiter->ApplyLimit(newValue))
			{
				return _SetValue(newValue, checkProc, context);
			}
		}
	}
	return FALSE;
}


BOOL CSettingObjectContained::SetLimiter(IN PSETTINGLIMITER limiter,
										 IN PSETTINGOBJECT_CHECKPROC checkProc, IN PVOID context)
{
	TRACE("+++ Misc SetLimiter: %s\n", m_key);
	if (limiter != m_limiter)
	{
		if (m_limiter != NULL)
		{
			delete m_limiter;
		}
		m_limiter = limiter;
	}
	return CheckLimiter(checkProc, context);
}


PSETTINGLIMITER CSettingObjectContained::GetLimiter() const
{
	TRACE("+++ Query GetLimiter: %s\n", m_key);
	return m_limiter;
}


BOOL CSettingObjectContained::Notify(INT msg, RCSETTINGVALUE, RCSETTINGVALUE)
{
	if (msg == NOTIFY_VALUE_CHANGING || msg == NOTIFY_VALUE_SETTING)
	{
		TRACE("+++ Notify Pre: %s\n", m_key);
	}
	else if (msg == NOTIFY_VALUE_CHANGED || msg == NOTIFY_VALUE_SET)
	{
		TRACE("+++ Notify Post: %s\n", m_key);
	}
	else if (msg == NOTIFY_VALUE_RESETTING || NOTIFY_VALUE_RECHANGING)
	{
		TRACE("+++ Notify Repre: %s\n", m_key);
	}
	return TRUE;
}


