/////////////////////////////////////////////////////////////////////////////
// $Id: SettingConfig.cpp,v 1.2 2004-08-08 17:03:38 atnak Exp $
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
#include "SettingConfig.h"
#include "SettingGroup.h."
#include "SettingKey.h"
#include <string>
#include <vector>
#include <sstream>


//////////////////////////////////////////////////////////////////////////
// CSettingConfig
//////////////////////////////////////////////////////////////////////////

CSettingConfig::CSettingConfig(std::string title) :
	m_title(title),
	m_purgable(TRUE)
{
}


CSettingConfig::~CSettingConfig()
{
	m_title.clear();
}


std::string CSettingConfig::GetTitle()
{
	return m_title;
}


void CSettingConfig::Begin()
{
}


void CSettingConfig::End()
{
}


void CSettingConfig::SetPurgable(BOOL purgable)
{
	m_purgable = purgable;
}


BOOL CSettingConfig::IsPurgable()
{
	return m_purgable;
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigSetting
//////////////////////////////////////////////////////////////////////////

CSettingConfigSetting::CSettingConfigSetting(PSETTINGKEY key, BOOL activeChange) :
	CSettingConfig(key->GetTitle()),
	m_settingGroup(key->GetController()),
	m_settingIdentifier(key->GetIdentifier()),
	m_activeChange(activeChange)
{
	ASSERT(m_title != "");
	ASSERT(m_settingGroup != NULL);
}


CSettingConfigSetting::CSettingConfigSetting(std::string title, PSETTINGGROUP group,
							   HSETTING setting, BOOL activeChange) :
	CSettingConfig(title),
	m_settingGroup(group),
	m_settingIdentifier(setting),
	m_activeChange(activeChange)
{
	ASSERT(m_title != "");
	ASSERT(m_settingGroup != NULL);
}


CSettingConfigSetting::~CSettingConfigSetting()
{
}


CSettingValue CSettingConfigSetting::SetValue(RCSETTINGVALUE value)
{
	if (m_activeChange)
	{
		if (!m_pendingValue.IsSet())
		{
			// Save the old value.
			m_pendingValue = m_settingGroup->GetValue(m_settingIdentifier);
		}
		// Set the new value.
		m_settingGroup->SetValue(m_settingIdentifier, value);
		return m_settingGroup->GetValue(m_settingIdentifier);
	}
	else
	{
		// Save the new value.
		m_pendingValue = value;

		PSETTINGLIMITER limiter = m_settingGroup->GetLimiter(m_settingIdentifier);
		if (limiter != NULL)
		{
			// Check the new value.
			limiter->ApplyLimit(m_pendingValue);
		}
		return m_pendingValue;
	}
}


CSettingValue CSettingConfigSetting::GetValue()
{
	if (m_activeChange || !m_pendingValue.IsSet())
	{
		return m_settingGroup->GetValue(m_settingIdentifier);
	}
	return m_pendingValue;
}


CSettingValue CSettingConfigSetting::UseDefault()
{
	return SetValue(m_settingGroup->GetDefault(m_settingIdentifier));
}


CSettingValue CSettingConfigSetting::GetDefault()
{
	return m_settingGroup->GetDefault(m_settingIdentifier);
}


void CSettingConfigSetting::ApplyValue()
{
	if (!m_activeChange && m_pendingValue.IsSet())
	{
		m_settingGroup->SetValue(m_settingIdentifier, m_pendingValue);
	}
	m_pendingValue.Reset();
}


void CSettingConfigSetting::ResetValue()
{
	if (m_activeChange && m_pendingValue.IsSet())
	{
		m_settingGroup->SetValue(m_settingIdentifier, m_pendingValue);
	}
	m_pendingValue.Reset();
}


void CSettingConfigSetting::Begin()
{
	CSettingConfig::Begin();
	m_pendingValue.Reset();
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigCheckbox
//////////////////////////////////////////////////////////////////////////

CSettingConfigCheckbox::CSettingConfigCheckbox(PSETTINGKEY key, BOOL activeChange) :
	CSettingConfigSetting(key, activeChange)
{
}


CSettingConfigCheckbox::CSettingConfigCheckbox(std::string title, PSETTINGGROUP group,
											   HSETTING setting, BOOL activeChange) :
	CSettingConfigSetting(title, group, setting, activeChange)
{
}


CSettingConfigCheckbox::~CSettingConfigCheckbox()
{
}


BOOL CSettingConfigCheckbox::GetCheckboxState()
{
	CSettingValue value = CSettingConfigSetting::GetValue();

	switch (value.GetType())
	{
	case SETTING_VALUE_INT:
		return value.GetInt() != 0;
	case SETTING_VALUE_UINT:
		return value.GetUInt() != 0;
	case SETTING_VALUE_STRING:
		return value.GetString().compare("On") == 0;
	}
	ASSERT(FALSE);
	return FALSE;
}


BOOL CSettingConfigCheckbox::SetCheckboxState(BOOL checked)
{
	CSettingValue value = CSettingConfigSetting::GetValue();

	switch (value.GetType())
	{
	case SETTING_VALUE_INT:
		value.SetInt(checked ? 1 : 0);
		break;
	case SETTING_VALUE_UINT:
		value.SetUInt(checked ? 1 : 0);
		break;
	case SETTING_VALUE_STRING:
		value.SetString(checked ? "On" : "Off");
		break;
	}
	CSettingConfigSetting::SetValue(value);
	return GetCheckboxState();
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigEditBox
//////////////////////////////////////////////////////////////////////////

CSettingConfigEditBox::CSettingConfigEditBox(PSETTINGKEY key,
											 ULONG maxLength, BOOL activeChange) :
	CSettingConfigSetting(key, activeChange),
	m_maxLength(maxLength)
{

}


CSettingConfigEditBox::CSettingConfigEditBox(std::string title, PSETTINGGROUP group,
											 HSETTING setting, ULONG maxLength,
											 BOOL activeChange) :
	CSettingConfigSetting(title, group, setting, activeChange),
	m_maxLength(maxLength)
{
}


CSettingConfigEditBox::~CSettingConfigEditBox()
{
}


ULONG CSettingConfigEditBox::GetMaxLength()
{
	return m_maxLength;
}


std::string CSettingConfigEditBox::GetEditBoxText()
{
	return CSettingConfigSetting::GetValue().ToString();
}


std::string CSettingConfigEditBox::SetEditBoxText(std::string text)
{
	CSettingValue value = CSettingConfigSetting::GetValue();

	value.FromString(text, value.GetType());

	CSettingConfigSetting::SetValue(value);
	return GetEditBoxText();
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigListBox
//////////////////////////////////////////////////////////////////////////

CSettingConfigListBox::CSettingConfigListBox(PSETTINGKEY key, ULONG count,
											 BOOL sorted, BOOL activeChange) :
	CSettingConfigSetting(key, activeChange),
	m_sorted(sorted)
{
	m_elements.reserve(count);
}


CSettingConfigListBox::CSettingConfigListBox(std::string title, PSETTINGGROUP group,
											 HSETTING setting, ULONG count,
											 BOOL sorted, BOOL activeChange) :
	CSettingConfigSetting(title, group, setting, activeChange),
	m_sorted(sorted)
{
	m_elements.reserve(count);
}


CSettingConfigListBox::CSettingConfigListBox(PSETTINGKEY key, LPCSTR elements[], ULONG count) :
	CSettingConfigSetting(key)
{
	m_elements.reserve(count);
	for (ULONG i = 0; i < count; i++)
	{
		m_elements.push_back(elements[i]);
	}
}


CSettingConfigListBox::CSettingConfigListBox(std::string title, PSETTINGGROUP group,
											 HSETTING setting, LPCSTR elements[], ULONG count) :
	CSettingConfigSetting(title, group, setting)
{
	m_elements.reserve(count);
	for (ULONG i = 0; i < count; i++)
	{
		m_elements.push_back(elements[i]);
	}
}


CSettingConfigListBox::~CSettingConfigListBox()
{
	m_elements.clear();
}


void CSettingConfigListBox::AddElement(std::string element)
{
	m_elements.push_back(element);
}


LPCSTR CSettingConfigListBox::GetElement(ULONG index)
{
	return m_elements.at(index).c_str();
}


ULONG CSettingConfigListBox::GetCount()
{
	return (ULONG)m_elements.size();
}


BOOL CSettingConfigListBox::IsSorted()
{
	return m_sorted;
}


ULONG CSettingConfigListBox::GetListBoxSelected()
{
	CSettingValue value = CSettingConfigSetting::GetValue();

	ULONG selected = 0;

	switch (value.GetType())
	{
	case SETTING_VALUE_INT:
		selected = (ULONG)value.GetInt();
	case SETTING_VALUE_UINT:
		selected = value.GetUInt();
	default:
		ASSERT(FALSE);
	}

	if (selected >= GetCount())
	{
		selected = 0;
	}
	return selected;
}


ULONG CSettingConfigListBox::SetListBoxSelected(ULONG selected)
{
	CSettingValue value = CSettingConfigSetting::GetValue();

	switch (value.GetType())
	{
	case SETTING_VALUE_INT:
		value.SetInt(selected);
		break;
	case SETTING_VALUE_UINT:
		value.SetUInt(selected);
		break;
	case SETTING_VALUE_STRING:
		value.SetString(GetElement(selected));
		break;
	}
	CSettingConfigSetting::SetValue(value);
	return GetListBoxSelected();
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigSlider
//////////////////////////////////////////////////////////////////////////

CSettingConfigSlider::CSettingConfigSlider(PSETTINGKEY key, INT minimum,
										   INT maximum, INT step, BOOL activeChange) :
	CSettingConfigSetting(key, activeChange),
	m_minimum(minimum),
	m_maximum(maximum),
	m_step(step)
{
}


CSettingConfigSlider::CSettingConfigSlider(std::string title, PSETTINGGROUP group,
										   HSETTING setting, INT minimum,
										   INT maximum, INT step, BOOL activeChange) :
	CSettingConfigSetting(title, group, setting, activeChange),
	m_minimum(minimum),
	m_maximum(maximum),
	m_step(step)
{
}


CSettingConfigSlider::~CSettingConfigSlider()
{
}


INT CSettingConfigSlider::GetMin()
{
	return m_minimum;
}


INT CSettingConfigSlider::GetMax()
{
	return m_maximum;
}


INT CSettingConfigSlider::GetStep()
{
	return m_step;
}


INT CSettingConfigSlider::GetSliderValue()
{
	CSettingValue value = CSettingConfigSetting::GetValue();

	switch (value.GetType())
	{
	case SETTING_VALUE_INT:
		return value.GetInt();
	case SETTING_VALUE_UINT:
		return value.GetUInt();
	case SETTING_VALUE_STRING:
		return strtoul(value.GetString().c_str(), NULL, 10);
	}
	ASSERT(FALSE);
	return FALSE;
}


INT CSettingConfigSlider::SetSliderValue(INT slide)
{
	CSettingValue value = CSettingConfigSetting::GetValue();

	switch (value.GetType())
	{
	case SETTING_VALUE_INT:
		value.SetInt(slide);
		break;
	case SETTING_VALUE_UINT:
		value.SetUInt(slide);
		break;
	case SETTING_VALUE_STRING:
		{
			std::ostringstream oss;
			oss << slide;
			value.SetString(oss.str());
		}
		break;
	}
	CSettingConfigSetting::SetValue(value);
	return GetSliderValue();
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigDependant
//////////////////////////////////////////////////////////////////////////

CSettingConfigDependant::CSettingConfigDependant(PSETTINGKEY key) :
	CSettingConfig(key->GetTitle()),
	m_settingGroupEx(dynamic_cast<PSETTINGGROUPEX>(key->GetController())),
	m_settingIdentifier(key->GetIdentifier())
{
	ASSERT(m_title != "");
	ASSERT(m_settingGroupEx != NULL);
}


CSettingConfigDependant::CSettingConfigDependant(std::string title,
												 PSETTINGGROUPEX group,
												 HSETTING setting) :
	CSettingConfig(title),
	m_settingGroupEx(group),
	m_settingIdentifier(setting)
{
	ASSERT(m_title != "");
	ASSERT(m_settingGroupEx != NULL);
}


CSettingConfigDependant::~CSettingConfigDependant()
{
}


BOOL CSettingConfigDependant::IsDependee()
{
	return m_dependeeBits != 0;
}


BOOL CSettingConfigDependant::IsDependee(BYTE dependencyIndex)
{
	return (m_dependeeBits & (dependencyIndex << 1)) != 0;
}


BOOL CSettingConfigDependant::IsDependant(BYTE dependencyIndex)
{
	return (m_dependantBits & (dependencyIndex << 1)) != 0;
}


BOOL CSettingConfigDependant::IsDependantLocked(BYTE dependencyIndex)
{
	return (m_dependantLockedBits & (dependencyIndex << 1)) != 0;
}


BOOL CSettingConfigDependant::SetDependant(BYTE dependencyIndex, BOOL set)
{
	if (set)
	{
		m_dependantBits |= dependencyIndex << 1;
		return TRUE;
	}

	m_dependantBits &= ~(dependencyIndex << 1);
	return FALSE;
}


void CSettingConfigDependant::ApplyValue()
{
	if (m_dependantBits != m_originalDependantBits)
	{
		m_settingGroupEx->SetOptionalDependantBits(m_settingIdentifier, m_dependantBits);
	}
}


void CSettingConfigDependant::ResetValue()
{
	m_dependantBits = m_originalDependantBits;
}


void CSettingConfigDependant::Begin()
{
	CSettingConfig::Begin();

	// Initialize the internal cache
	m_dependeeBits = m_settingGroupEx->GetDependeeBits(m_settingIdentifier);
	m_dependantBits = m_settingGroupEx->GetOptionalDependantBits(m_settingIdentifier);
	m_dependantLockedBits = m_settingGroupEx->GetAbsoluteDependantBits(m_settingIdentifier);
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigContainer
//////////////////////////////////////////////////////////////////////////

CSettingConfigContainer::CSettingConfigContainer(std::string title, ULONG reserveCount) :
	CSettingConfig(title)
{
	m_configVector.reserve(reserveCount);
}


CSettingConfigContainer::~CSettingConfigContainer()
{
	CONFIGVECTOR::iterator it = m_configVector.begin();
	CONFIGVECTOR::iterator ti = m_configVector.end();
	for ( ; it != ti; it++)
	{
		if ((*it)->IsPurgable())
		{
			delete *it;
		}
	}
	m_configVector.clear();
}


ULONG CSettingConfigContainer::AddConfig(CSettingConfig* config, BOOL purgable)
{
	ULONG index = (ULONG)m_configVector.size();
	config->SetPurgable(purgable);
	m_configVector.push_back(config);
	return index;
}


ULONG CSettingConfigContainer::GetConfigCount()
{
	return (ULONG)m_configVector.size();
}


CSettingConfig* CSettingConfigContainer::GetConfig(ULONG index)
{
	ASSERT(index < m_configVector.size());
	return m_configVector.at(index);
}


void CSettingConfigContainer::ApplyValue()
{
	CONFIGVECTOR::iterator it = m_configVector.begin();
	CONFIGVECTOR::iterator ti = m_configVector.end();
	for ( ; it != ti; it++)
	{
		(*it)->ApplyValue();
	}
}


void CSettingConfigContainer::ResetValue()
{
	CONFIGVECTOR::iterator it = m_configVector.begin();
	CONFIGVECTOR::iterator ti = m_configVector.end();
	for ( ; it != ti; it++)
	{
		(*it)->ResetValue();
	}
}


void CSettingConfigContainer::Begin()
{
	CONFIGVECTOR::iterator it = m_configVector.begin();
	CONFIGVECTOR::iterator ti = m_configVector.end();
	for ( ; it != ti; it++)
	{
		(*it)->Begin();
	}
}


void CSettingConfigContainer::End()
{
	CONFIGVECTOR::iterator it = m_configVector.begin();
	CONFIGVECTOR::iterator ti = m_configVector.end();
	for ( ; it != ti; it++)
	{
		(*it)->End();
	}
}


//////////////////////////////////////////////////////////////////////////
// CSettingConfigAssociation
//////////////////////////////////////////////////////////////////////////

CSettingConfigAssociation::CSettingConfigAssociation(std::string title,
													 PSETTINGGROUPEX group,
													 BYTE reserveDependencyCount,
													 ULONG reserveConfigCount) :
	CSettingConfigContainer(title, reserveConfigCount),
	m_associationGroup(group)
{
	ASSERT(reserveDependencyCount < 255);
	m_associationVector.reserve(reserveDependencyCount);
}


CSettingConfigAssociation::~CSettingConfigAssociation()
{
	m_associationVector.clear();
}


ULONG CSettingConfigAssociation::AddConfig(CSettingConfig* config, BOOL purgable)
{
	// This container can only accept dependant config objects
	ASSERT(config->GetType() == SETTING_CONFIG_DEPENDANT);

	ULONG index = CSettingConfigContainer::AddConfig(config, purgable);

	// If this setting is a dependee, save its pointer for quick reference.
	CSettingConfigDependant* cfg = dynamic_cast<CSettingConfigDependant*>(config);
	if (cfg->IsDependee())
	{
		BYTE dependencyCount = GetDependencyCount();
		for (BYTE i = 0; i < dependencyCount; i++)
		{
			if (cfg->IsDependee(i))
			{
				m_associationVector.at(i).dependee = cfg;
			}
		}
	}
	return index;
}


void CSettingConfigAssociation::AddDependency(std::string title)
{
	ASSERT(m_associationVector.size() < 255);
	m_associationVector.push_back(CAssociation(title));
}


BYTE CSettingConfigAssociation::GetDependencyCount()
{
	return (BYTE)m_associationVector.size();
}


std::string CSettingConfigAssociation::GetDependencyTitle(BYTE dependencyIndex)
{
	return m_associationVector.at(dependencyIndex).title;
}


ULONG CSettingConfigAssociation::GetBlockedDependant(ULONG index)
{
	CSettingConfigDependant* cfg = (CSettingConfigDependant*)GetConfig(index);

	if (!cfg->IsDependee())
	{
		return 0;
	}

	// Look for the bits that need to be blocked to prevent cyclic dependencies.
	DBIT dependeeBits = cfg->m_dependeeBits;
	BYTE dependencyCount = GetDependencyCount();
	for (BYTE i = 0; i < dependencyCount; i++)
	{
		if (m_associationVector.at(i).dependee->m_dependantBits & dependeeBits)
		{
			DBIT changedBits = dependeeBits | m_associationVector.at(i).dependee->m_dependeeBits;
			if (changedBits != dependeeBits)
			{
				dependeeBits = changedBits;
				i = (BYTE)-1;
			}
		}
	}
	return dependeeBits;
}


void CSettingConfigAssociation::ApplyValue()
{
	BOOL suspended = m_associationGroup->IsSuspended();
	if (!suspended)
	{
		// Suspend before applying the values so that the same
		// setting won't get checked multiple times.
		m_associationGroup->Suspend();
	}

	// Apply the values on all the contained config objects.
	CSettingConfigContainer::ApplyValue();

	if (!suspended)
	{
		// Reactivate if it wasn't suspended.
		m_associationGroup->Activate();
	}

	// Save the changed optional dependencies.
	m_associationGroup->SaveOptionalDependencies();
}

