/////////////////////////////////////////////////////////////////////////////
// $Id: SettingKey.cpp,v 1.2 2004-08-14 13:45:23 adcockj Exp $
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
#include "SettingKey.h"
#include "SettingGroup.h"
#include "SettingValue.h"
#include "SettingObject.h"
#include "SettingConfig.h"
#include <string>


//////////////////////////////////////////////////////////////////////////
// CSettingKey
//////////////////////////////////////////////////////////////////////////

CSettingKey::CSettingKey() :
	m_controller(NULL)
{
}


CSettingKey::~CSettingKey()
{
}


PSETTINGOBJECT CSettingKey::NewSetting(LPCSTR entry, BYTE type)
{
	PSETTINGOBJECT object = (PSETTINGOBJECT)new CSettingObjectContained(entry, type);
	if (object == NULL)
	{
		OUT_OF_MEMORY_ERROR;
	}
	return object;
}


std::string CSettingKey::GetTitle()
{
	return "";
}


void CSettingKey::LoadSetting()
{
	ASSERT(m_controller != NULL);
	m_controller->LoadSetting(m_identifier);
}


void CSettingKey::SaveSetting()
{
	ASSERT(m_controller != NULL);
	m_controller->SaveSetting(m_identifier);
}


void CSettingKey::SetValueSV(RCSETTINGVALUE value)
{
	ASSERT(m_controller != NULL);
	m_controller->SetValue(m_identifier, value);
}


CSettingValue CSettingKey::GetValueSV()
{
	ASSERT(m_controller != NULL);
	return m_controller->GetValue(m_identifier);
}


void CSettingKey::UseDefault()
{
	ASSERT(m_controller != NULL);
	m_controller->UseDefault(m_identifier);
}


void CSettingKey::SetDefaultSV(RCSETTINGVALUE value)
{
	ASSERT(m_controller != NULL);
	m_controller->SetDefault(m_identifier, value);
}


CSettingValue CSettingKey::GetDefaultSV()
{
	ASSERT(m_controller != NULL);
	return m_controller->GetDefault(m_identifier);
}


void CSettingKey::CheckLimiter()
{
	ASSERT(m_controller != NULL);
	m_controller->CheckLimiter(m_identifier);
}


void CSettingKey::SetLimiter(PSETTINGLIMITER limiter)
{
	ASSERT(m_controller != NULL);
	m_controller->SetLimiter(m_identifier, limiter);
}


PSETTINGLIMITER CSettingKey::GetLimiter()
{
	ASSERT(m_controller != NULL);
	return m_controller->GetLimiter(m_identifier);
}


BOOL CSettingKey::Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue)
{
	return TRUE;
}


void CSettingKey::SetController(PSETTINGGROUP controller, HSETTING setting)
{
	m_controller = controller;
	m_identifier = setting;
}


PSETTINGGROUP CSettingKey::GetController()
{
	return m_controller;
}


HSETTING CSettingKey::GetIdentifier()
{
	ASSERT(m_controller != NULL);
	return m_identifier;
}


//////////////////////////////////////////////////////////////////////////
// CSettingKeyTitled
//////////////////////////////////////////////////////////////////////////

CSettingKeyTitled::CSettingKeyTitled()
{
}


CSettingKeyTitled::~CSettingKeyTitled()
{
}


void CSettingKeyTitled::Setup(std::string title)
{
	m_title = title;
}


std::string CSettingKeyTitled::GetTitle()
{
	return m_title;
}


//////////////////////////////////////////////////////////////////////////
// CSettingKeyLong
//////////////////////////////////////////////////////////////////////////
CSettingKeyLong::CSettingKeyLong()
{
}


CSettingKeyLong::~CSettingKeyLong()
{
}


PSETTINGOBJECT CSettingKeyLong::NewSetting(LPCSTR entry, INT initial)
{
	PSETTINGOBJECT object = CSettingKey::NewSetting(entry, SETTING_VALUE_INT);

	CSettingValue defaultValue;
	defaultValue.SetInt(initial);
	object->SetDefault(defaultValue, NULL, NULL);
	return object;
}


PSETTINGOBJECT CSettingKeyLong::NewSetting(LPCSTR entry, INT initial, INT minimum, INT maximum)
{
	PSETTINGOBJECT object = CSettingKeyLong::NewSetting(entry, initial);

	PSETTINGLIMITER limiter = (PSETTINGLIMITER)new CSettingLimiterClampInt(minimum, maximum);
	object->SetLimiter(limiter, NULL, NULL);
	return object;
}


void CSettingKeyLong::SetValue(INT value)
{
	CSettingValue newValue;
	newValue.SetInt(value);
	SetValueSV(newValue);
}


INT CSettingKeyLong::GetValue()
{
	CSettingValue value = GetValueSV();
	ASSERT(value.GetType() == SETTING_VALUE_INT);
	return value.GetInt();
}


void CSettingKeyLong::SetDefault(INT value)
{
	CSettingValue defaultValue;
	defaultValue.SetInt(value);
	SetDefaultSV(defaultValue);
}


INT CSettingKeyLong::GetDefault()
{
	CSettingValue value = GetDefaultSV();
	ASSERT(value.GetType() == SETTING_VALUE_INT);
	return value.GetInt();
}


void CSettingKeyLong::SetRange(INT minimum, INT maximum)
{
	PSETTINGLIMITER limiter = (PSETTINGLIMITER)new CSettingLimiterClampInt(minimum, maximum);
	CSettingKey::SetLimiter(limiter);
}


BOOL CSettingKeyLong::Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue)
{
	// Assert new value but don't assert old because old can be not set.
	ASSERT(newValue.GetType() == SETTING_VALUE_INT);

	INT newInt = newValue.GetInt();
	INT oldInt = oldValue.GetInt();

	return Notify(message, newInt, oldInt);
}


BOOL CSettingKeyLong::Notify(INT message, INT newValue, INT oldValue)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// CSettingKeySlider
//////////////////////////////////////////////////////////////////////////

CSettingKeySlider::CSettingKeySlider() :
	m_initial(0),
	m_minimum(0),
	m_maximum(0),
	m_step(0)
{
}


CSettingKeySlider::~CSettingKeySlider()
{
}


void CSettingKeySlider::Setup(std::string title, INT initial, INT minimum, INT maximum, INT step)
{
	CSettingKeyTitled::Setup(title);
	m_initial = initial;
	m_minimum = minimum;
	m_maximum = maximum;
	m_step = step;
}


PSETTINGOBJECT CSettingKeySlider::CreateSetting(LPCSTR entry)
{
	return CSettingKeyLong::NewSetting(entry, m_initial, m_minimum, m_maximum);
}


PSETTINGCONFIG CSettingKeySlider::CreateConfig()
{
	return (PSETTINGCONFIG)new CSettingConfigSlider(m_title, m_controller, m_identifier,
		m_minimum, m_maximum, m_step);
}


void CSettingKeySlider::StepUp()
{
	SetValue(GetValue() + m_step);
}


void CSettingKeySlider::StepDown()
{
	SetValue(GetValue() - m_step);
}


void CSettingKeySlider::SetRange(INT minimum, INT maximum)
{
	m_minimum = minimum;
	m_maximum = maximum;
	CSettingKeyLong::SetRange(m_minimum, m_maximum);
}


void CSettingKeySlider::SetMin(INT minimum)
{
	m_minimum = minimum;
	CSettingKeyLong::SetRange(m_minimum, m_maximum);
}


void CSettingKeySlider::SetMax(INT maximum)
{
	m_maximum = maximum;
	CSettingKeyLong::SetRange(m_minimum, m_maximum);
}


INT CSettingKeySlider::GetMin()
{
	return m_minimum;
}


INT CSettingKeySlider::GetMax()
{
	return m_maximum;
}


void CSettingKeySlider::SetStep(INT step)
{
	m_step = step;
}

