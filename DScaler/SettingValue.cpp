/////////////////////////////////////////////////////////////////////////////
// $Id: SettingValue.cpp,v 1.2 2005-03-17 03:55:19 atnak Exp $
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
// Revision 1.1  2004/08/06 17:12:10  atnak
// Setting repository initial upload.
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SettingValue.h"
#include <string>
#include <sstream>


//////////////////////////////////////////////////////////////////////////
// Constructors and destructor
//////////////////////////////////////////////////////////////////////////

CSettingValue::CSettingValue() :
	m_type(SETTING_VALUE_NULL),
	m_value(NULL)
{
}


CSettingValue::CSettingValue(IN BYTE type) :
	m_type(SETTING_VALUE_NULL),
	m_value(NULL)
{
	this->SetType(type);
}


CSettingValue::CSettingValue(IN const CSettingValue& value) :
	m_type(SETTING_VALUE_NULL),
	m_value(NULL)
{
	this->Copy(value);
}


CSettingValue::~CSettingValue()
{
	this->Reset();
}


//////////////////////////////////////////////////////////////////////////
// Get and set type
//////////////////////////////////////////////////////////////////////////

void CSettingValue::SetType(IN BYTE type)
{
	if (type == m_type)
		return;

	Reset();
	m_type = type;
}


BYTE CSettingValue::GetType() const
{
	return m_type;
}


//////////////////////////////////////////////////////////////////////////
// Type specific getters and setters
//////////////////////////////////////////////////////////////////////////

BOOL CSettingValue::SetString(IN std::string value)
{
	Reset();
	m_value = new std::string(value);
	m_type = SETTING_VALUE_STRING;
	return m_value == NULL ? FALSE : TRUE;
}


std::string CSettingValue::GetString() const
{
	if (m_type == SETTING_VALUE_STRING)
	{
		return m_value != NULL ? *(std::string*)m_value : "";
	}
	return "";
}


#pragma warning(disable: 4311 4312)
BOOL CSettingValue::SetUInt(IN UINT value)
{
	Reset();
	m_value = (PVOID)value;
	m_type = SETTING_VALUE_UINT;
	return TRUE;
}


UINT CSettingValue::GetUInt() const
{
	if (m_type == SETTING_VALUE_UINT)
		return (UINT)m_value;
	return 0;
}


BOOL CSettingValue::SetInt(IN INT value)
{
	Reset();
	m_value = (PVOID)value;
	m_type = SETTING_VALUE_INT;
	return TRUE;
}


INT CSettingValue::GetInt() const
{
	if (m_type == SETTING_VALUE_INT)
		return (INT)m_value;
#pragma warning(default: 4311 4312)
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Other functions
//////////////////////////////////////////////////////////////////////////

void CSettingValue::Reset()
{
	switch (m_type)
	{
	case SETTING_VALUE_STRING:
		if (m_value != NULL)
		{
			delete (std::string*)m_value;
			m_value = NULL;
		}
		break;
	default:
		m_value = NULL;
		break;
	}
	m_type = SETTING_VALUE_NULL;
}


BOOL CSettingValue::IsSet() const
{
	return m_type != SETTING_VALUE_NULL;
}


void CSettingValue::FromString(std::string from, BYTE type)
{
	switch (type)
	{
	case SETTING_VALUE_NULL:
		break;
	case SETTING_VALUE_STRING:
		SetString(from);
		break;
	case SETTING_VALUE_INT:
		SetInt(strtoul(from.c_str(), NULL, 0));
		break;
	case SETTING_VALUE_UINT:
		SetUInt(strtoul(from.c_str(), NULL, 0));
		break;
	}
}


std::string CSettingValue::ToString() const
{
	std::ostringstream oss;

	switch (m_type)
	{
	case SETTING_VALUE_NULL:
		return "NULL";
	case SETTING_VALUE_STRING:
		return GetString();
	case SETTING_VALUE_INT:
		oss << GetInt();
		return oss.str();
	case SETTING_VALUE_UINT:
		oss << GetUInt();
		return oss.str();
	}
	return "";
}


void CSettingValue::Copy(IN const CSettingValue& src)
{
	if (this != &src)
	{
		switch (src.m_type)
		{
		case SETTING_VALUE_STRING:
			if (src.m_value == NULL)
			{
				Reset();
				this->m_value = NULL;
				this->m_type = SETTING_VALUE_STRING;
			}
			else
			{
				SetString(src.GetString());
			}
			break;
		// For all non-buffered types
		default:
			Reset();
			this->m_value = src.m_value;
			this->m_type = src.m_type;
		}
	}
}


void CSettingValue::LazyCopy(IN const CSettingValue& src)
{
	if (this->m_value == src.m_value)
	{
		return;
	}

	LONG* refCount = (LONG*)src.m_value;

	InterlockedIncrement(refCount);

	Reset();
	this->m_value = src.m_value;
	this->m_type = src.m_type;
}


BOOL CSettingValue::RefDelete()
{
	LONG* refCount = (LONG*)m_value;

	if (InterlockedDecrement(refCount) == 0)
	{
		delete [] m_value;
		m_value = NULL;
		return TRUE;
	}
	m_value = NULL;
	return FALSE;
}


PVOID CSettingValue::CreateBuffer(IN size_t size)
{
#ifdef SETTING_VALUE_USE_LAZYCOPY
	LONG volatile * buffer = (LONG*)new CHAR[sizeof(LONG) + size];
	if (buffer != NULL)
	{
		*buffer = 1;
	}
	m_value = (PVOID)buffer;
	return GetBuffer();
#else
	return m_value = (PVOID)new CHAR[size];
#endif
}


PVOID CSettingValue::GetBuffer() const
{
#ifdef SETTING_VALUE_USE_LAZYCOPY
	if (m_value == NULL)
		return NULL;
	return (PVOID)(((LONG*)m_value)+1);
#else
	return m_value;
#endif
}


BOOL CSettingValue::IsEqual(IN const CSettingValue& value) const
{
	if (value.m_type != this->m_type)
		return FALSE;

	switch (m_type)
	{
	case SETTING_VALUE_STRING:
		return GetString().compare(value.GetString()) == 0;
	case SETTING_VALUE_UINT:
	case SETTING_VALUE_INT:
		return this->m_value == value.m_value;
	case SETTING_VALUE_NULL:
		return TRUE;
	}
	return FALSE;
}


CSettingValue& CSettingValue::operator= (IN const CSettingValue& src)
{
	this->Copy(src);
	return *this;
}

