/////////////////////////////////////////////////////////////////////////////
// $Id: SettingConfig.h,v 1.1 2004-08-06 17:12:10 atnak Exp $
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

#ifndef __SETTINGCONFIG_H__
#define __SETTINGCONFIG_H__

#include "SettingRepository.h"
#include "SettingValue.h"
#include <string>
#include <vector>

// These classes are for defining how individual settings appear in
// a setting configuration interface.

enum eSettingConfigType
{
	SETTING_CONFIG_CHECKBOX,
	SETTING_CONFIG_EDITBOX,
	SETTING_CONFIG_LISTBOX,
	SETTING_CONFIG_SLIDER,
	SETTING_CONFIG_DEPENDANT,

	SETTING_CONFIG_CONTAINER,
	SETTING_CONFIG_ASSOCIATION,
};


typedef class CSettingConfig* PSETTINGCONFIG;


//////////////////////////////////////////////////////////////////////////
// CSettingConfig
//////////////////////////////////////////////////////////////////////////
class CSettingConfig
{
public:
	CSettingConfig(std::string title);
	virtual ~CSettingConfig();

	virtual inline BYTE GetType()=0;
	virtual std::string GetTitle();

	// Called when the OK or Apply button is pressed.
	virtual void ApplyValue()=0;
	// Called when the Cancel button is pressed.
	virtual void ResetValue()=0;

	// Sets if object can be purged automatically.  TRUE by default.
	virtual inline void SetPurgable(BOOL purgable);
	// Gets if object can be purged automatically.
	virtual inline BOOL IsPurgable();

protected:
	std::string		m_title;
	BOOL			m_purgable;
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigSetting
//////////////////////////////////////////////////////////////////////////
class CSettingConfigSetting : public CSettingConfig
{
public:
	CSettingConfigSetting(PSETTINGKEY key, BOOL activeChange = TRUE);
	CSettingConfigSetting(std::string title, PSETTINGGROUP group,
		HSETTING setting, BOOL activeChange = TRUE);
	virtual ~CSettingConfigSetting();

	virtual inline BYTE GetType()=0;

	virtual CSettingValue SetValue(RCSETTINGVALUE value);
	virtual CSettingValue GetValue();

	virtual CSettingValue UseDefault();
	virtual CSettingValue GetDefault();

	// Called to apply any pending values when the OK button is pressed.
	virtual void ApplyValue();
	// Called to reset the value when the Cancel button is pressed.
	virtual void ResetValue();

protected:
	PSETTINGGROUP	m_settingGroup;
	HSETTING		m_settingIdentifier;
	BOOL			m_activeChange;
	CSettingValue	m_pendingValue;
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigCheckbox
//////////////////////////////////////////////////////////////////////////
class CSettingConfigCheckbox : public CSettingConfigSetting
{
public:
	CSettingConfigCheckbox(PSETTINGKEY key, BOOL activeChange = TRUE);
	CSettingConfigCheckbox(std::string title,
		PSETTINGGROUP group, HSETTING setting, BOOL activeChange = TRUE);
	virtual ~CSettingConfigCheckbox();

	virtual inline BYTE GetType() { return SETTING_CONFIG_CHECKBOX; };

	virtual BOOL GetCheckboxState();
	virtual BOOL SetCheckboxState(BOOL checked);
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigEditBox
//////////////////////////////////////////////////////////////////////////
class CSettingConfigEditBox : public CSettingConfigSetting
{
public:
	CSettingConfigEditBox(PSETTINGKEY key, ULONG maxLength, BOOL activeChange = TRUE);
	CSettingConfigEditBox(std::string title, PSETTINGGROUP group,
		HSETTING setting, ULONG maxLength, BOOL activeChange = TRUE);
	virtual ~CSettingConfigEditBox();

	virtual inline BYTE GetType() { return SETTING_CONFIG_EDITBOX; };
	virtual ULONG GetMaxLength();

	virtual std::string GetEditBoxText();
	virtual std::string SetEditBoxText(std::string text);

protected:
	ULONG m_maxLength;
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigListBox
//////////////////////////////////////////////////////////////////////////
class CSettingConfigListBox : public CSettingConfigSetting
{
public:
	// Reserves space for count amount elemnts.  Use AddElement() to add.
	CSettingConfigListBox(PSETTINGKEY key, ULONG count,
		BOOL sorted, BOOL activeChange = TRUE);
	CSettingConfigListBox(std::string title,
		PSETTINGGROUP group, HSETTING setting,
		ULONG count, BOOL sorted, BOOL activeChange = TRUE);

	// Adds count elements pointed to by elements[] to the list.
	CSettingConfigListBox(PSETTINGKEY key, LPCSTR elements[], ULONG count);
	CSettingConfigListBox(std::string title,
		PSETTINGGROUP group, HSETTING setting, LPCSTR elements[], ULONG count);
	virtual ~CSettingConfigListBox();

	virtual inline BYTE GetType() { return SETTING_CONFIG_LISTBOX; };
	virtual void AddElement(std::string element);

	virtual LPCSTR GetElement(ULONG index);
	virtual ULONG GetCount();

	virtual inline BOOL IsSorted();

	virtual ULONG GetListBoxSelected();
	virtual ULONG SetListBoxSelected(ULONG selected);

protected:
	std::vector<std::string>	m_elements;
	BOOL						m_sorted;
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigSlider
//////////////////////////////////////////////////////////////////////////
class CSettingConfigSlider : public CSettingConfigSetting
{
public:
	CSettingConfigSlider(PSETTINGKEY key, INT minimum, INT maximum,
		INT step = 1, BOOL activeChange = TRUE);
	CSettingConfigSlider(std::string title, PSETTINGGROUP group, HSETTING setting,
		INT minimum, INT maximum, INT step = 1, BOOL activeChange = TRUE);
	virtual ~CSettingConfigSlider();

	virtual inline BYTE GetType() { return SETTING_CONFIG_SLIDER; };
	virtual INT GetMin();
	virtual INT GetMax();
	virtual INT GetStep();

	virtual INT GetSliderValue();
	virtual INT SetSliderValue(INT slide);

protected:
	INT m_minimum;
	INT m_maximum;
	INT m_step;
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigDependant
//////////////////////////////////////////////////////////////////////////
class CSettingConfigDependant : public CSettingConfig
{
public:
	CSettingConfigDependant(PSETTINGKEY key);
	CSettingConfigDependant(std::string title, PSETTINGGROUPEX group, HSETTING setting);
	virtual ~CSettingConfigDependant();

	virtual inline BYTE GetType() { return SETTING_CONFIG_DEPENDANT; };

	virtual inline BOOL IsDependee();
	virtual inline BOOL IsDependee(BYTE dependencyIndex);

	virtual inline BOOL IsDependant(BYTE dependencyIndex);
	virtual inline BOOL IsDependantLocked(BYTE dependencyIndex);
	virtual inline BOOL SetDependant(BYTE dependencyIndex, BOOL set);

	// Called when the OK or Apply button is pressed.
	virtual void ApplyValue();
	// Called when the Cancel button is pressed.
	virtual void ResetValue();

private:
	friend class CSettingConfigAssociation;
	PSETTINGGROUPEX	m_settingGroupEx;
	HSETTING		m_settingIdentifier;
	DBIT			m_dependeeBits;
	DBIT			m_dependantBits;
	DBIT			m_dependantLockedBits;
	DBIT			m_originalDependantBits;
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigContainer
//////////////////////////////////////////////////////////////////////////
class CSettingConfigContainer : public CSettingConfig
{
public:
	CSettingConfigContainer(std::string title, ULONG reserveCount = 0);
	virtual ~CSettingConfigContainer();

	virtual inline BYTE GetType() { return SETTING_CONFIG_CONTAINER; };

	// Adds a CSettingConfig as a sub-config of the container class.  The
	// value of purgable defines whether the passed object can be deleted
	// by CSettingConfigContainer when the destructor is called.
	virtual ULONG AddConfig(CSettingConfig* config, BOOL purgable = TRUE);

	virtual ULONG GetConfigCount();
	virtual CSettingConfig* GetConfig(ULONG index);

	// Called when the OK or Apply button is pressed.
	virtual void ApplyValue();
	// Called when the Cancel button is pressed.
	virtual void ResetValue();

protected:
	typedef std::vector<CSettingConfig*> CONFIGVECTOR;
	CONFIGVECTOR m_configVector;
};


//////////////////////////////////////////////////////////////////////////
// CSettingConfigAssociation
//////////////////////////////////////////////////////////////////////////
class CSettingConfigAssociation : public CSettingConfigContainer
{
public:
	CSettingConfigAssociation(std::string title, PSETTINGGROUPEX group,
		BYTE reserveDependencyCount, ULONG reserveConfigCount = 0);
	virtual ~CSettingConfigAssociation();

	virtual inline BYTE GetType() { return SETTING_CONFIG_ASSOCIATION; };

	// Override.  Adds a CSettingConfig as a sub-config of the container
	// class.  The value of purgable defines whether the passed object can
	// be deleted by CSettingConfigContainer when the destructor is called.
	virtual ULONG AddConfig(CSettingConfig* config, BOOL purgable = TRUE);

	// Adds a dependency in the association.
	virtual void AddDependency(std::string title);

	// Gets the number of dependencies in the associations.
	virtual inline BYTE GetDependencyCount();
	// Gets the title of a single dependency.
	virtual std::string GetDependencyTitle(BYTE dependencyIndex);

	// For the dependency setting at index given, GetBlockedDependant()
	// returns the bits of dependencyIndexes that cannot be set as
	// dependants because of cyclic dependencies.
	virtual ULONG GetBlockedDependant(ULONG index);

	// Called when the OK or Apply button is pressed.
	virtual void ApplyValue();

protected:
	class CAssociation
	{
	public:
		CAssociation(std::string title) : title(title), dependee(NULL) { };
		std::string					title;
		CSettingConfigDependant*	dependee;
	};
	typedef std::vector<CAssociation> ASSOCIATIONVECTOR;

	PSETTINGGROUPEX		m_associationGroup;
	ASSOCIATIONVECTOR	m_associationVector;
};


#endif

