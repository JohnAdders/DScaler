/////////////////////////////////////////////////////////////////////////////
// $Id: SettingKey.h,v 1.2 2004-08-20 07:25:17 atnak Exp $
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

#ifndef __SETTINGKEY_H__
#define __SETTINGKEY_H__

#include "SettingRepository.h"
#include "SettingGroup.h"
#include "SettingLimiter.h"
#include "SettingValue.h"
#include <string>

typedef class CSettingKey *PSETTINGKEY;


//////////////////////////////////////////////////////////////////////////
// CSettingKey
//////////////////////////////////////////////////////////////////////////
class CSettingKey
{
public:
	CSettingKey();
	virtual ~CSettingKey();

	static PSETTINGOBJECT NewSetting(LPCSTR entry, BYTE type);

	virtual void LoadSetting();
	virtual void SaveSetting();

	virtual void SetValueSV(RCSETTINGVALUE value);
	virtual CSettingValue GetValueSV();

	virtual void UseDefault();
	virtual void SetDefaultSV(RCSETTINGVALUE value);
	virtual CSettingValue GetDefaultSV();

	virtual void CheckLimiter();
	virtual void SetLimiter(PSETTINGLIMITER limiter);
	virtual PSETTINGLIMITER GetLimiter();

	virtual BOOL Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue);

	void			SetController(PSETTINGGROUP controller, HSETTING identifier);
	PSETTINGGROUP	GetController();
	HSETTING		GetIdentifier();

protected:
	HSETTING		m_identifier;
	PSETTINGGROUP	m_controller;
};


//////////////////////////////////////////////////////////////////////////
// CSettingKeyLong
//////////////////////////////////////////////////////////////////////////
class CSettingKeyLong : public CSettingKey
{
public:
	CSettingKeyLong();
	virtual ~CSettingKeyLong();

	static PSETTINGOBJECT NewSetting(LPCSTR entry, INT initial);
	static PSETTINGOBJECT NewSetting(LPCSTR entry, INT initial, INT minimum, INT maximum);

	virtual void SetValue(INT value);
	virtual INT GetValue();

	virtual void SetDefault(INT value);
	virtual INT GetDefault();

	virtual void SetRange(INT minimum, INT maximum);

	virtual BOOL Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue);
	virtual BOOL Notify(INT message, INT newValue, INT oldValue);
};


//////////////////////////////////////////////////////////////////////////
// CSettingKeySlider
//////////////////////////////////////////////////////////////////////////
class CSettingKeySlider : public CSettingKeyLong
{
public:
	CSettingKeySlider();
	virtual ~CSettingKeySlider();

	virtual void Setup(INT initial, INT minimum, INT maximum, INT step = 1);

	virtual PSETTINGOBJECT CreateSetting(LPCSTR entry);
	virtual PSETTINGCONFIG CreateConfig();

	virtual void StepUp();
	virtual void StepDown();

	virtual void SetRange(INT minimum, INT maximum);

	virtual void SetMin(INT minimum);
	virtual void SetMax(INT maximum);
	virtual INT GetMin();
	virtual INT GetMax();

	virtual void SetStep(INT step);

protected:
	INT	m_initial;
	INT m_minimum;
	INT m_maximum;
	INT m_step;
};


//////////////////////////////////////////////////////////////////////////
// Macros for simplying declaration of settings with callbacks.
//////////////////////////////////////////////////////////////////////////

// There are two version of the macro:
// CSETTINGKEY_C_CALLBACK_x(ClassName, SettingName) variableName
// CSETTINGKEY_P_CALLBACK_x(SettingName) variableName
//
// The C_CALLBACK version can only be used inside a class definition.
// The ClassName field must be the name of the class.  Using this macro
// will cause the Setup() function of the setting key to be overridden
// with a version with an additional parameter inserted straight after
// the title parameter for accepting the class object pointer.  The
// advantage of the C_CALLBACK version is that the 'this' pointer in
// the callback will be a valid instance of the class.
//
// The C_CALLBACK macro will define a new class for the setting, named
// CSettingNameSetting, where SettingName is the text string passed to
// the macro.  The prototype for the callback will be defined as
// ClassName::SettingNameOnNotify(...).  The parameters to the callback
// will differ depending on the setting key.  The body of the callback,
// ClassName::SettingNameOnNotify(...), will need be defined.
//
// The P_CALLBACK version can be used outside a class definition.
// Unlike C_CALLBACK, this macro only takes the SettingName parameter.
// It is not recommended that this version be used inside a class since
// it does not provide a class instance context to the callback.
//
// The P_CALLBACK macro will define a new class for the setting, named
// CSettingNameSetting.  (Same as C_CALLBACK.)  The prototype for the
// callback will be defined as SettingNameOnNotify(...).  The body of the
// callback will need to be defined.  The parameters to the callback will
// differ depending on the setting key.
//
// If required, the variable can also be declared as a pointer:
//   CSETTINGKEY_x_CALLBACK_x(SettingName) *variableName;
//   variableName = new CSettingNameSetting();
//   delete variableName;
//

//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(...) -- internal use
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(__KeyClass, __Name) \
BOOL __Name ## OnNotify(INT message, INT newValue, INT oldValue); \
class C ## __Name ## Setting : public __KeyClass \
{ \
public: \
	BOOL Notify(INT message, INT newValue, INT oldValue) \
	{ return __Name ## OnNotify(message, newValue, oldValue); } \
}


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_C_CALLBACK_LONG(ClassName, SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_C_CALLBACK_LONG(__TClass, __Name) \
public: \
BOOL __Name ## OnNotify(INT message, INT newValue, INT oldValue); \
protected: \
class C ## __Name ## Setting : public CSettingKeyLong \
{ \
public: \
	void Setup(__TClass* ptr) \
	{ m_c = ptr; } \
	BOOL Notify(INT message, INT newValue, INT oldValue) \
	{ return m_c->__Name ## OnNotify(message, newValue, oldValue); } \
private: \
	__TClass* m_c; \
}


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_P_CALLBACK_LONG(SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_P_CALLBACK_LONG(__Name) \
	CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(CSettingKeyLong, __Name)


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_C_CALLBACK_SLIDER(ClassName, SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_C_CALLBACK_SLIDER(__TClass, __Name) \
public: \
BOOL __Name ## OnNotify(INT message, INT newValue, INT oldValue); \
protected: \
class C ## __Name ## Setting : public CSettingKeySlider \
{ \
public: \
	void Setup(__TClass* ptr, INT initial, INT minimum, INT maximum, INT step = 1) \
	{ m_c = ptr; CSettingKeySlider::Setup(initial, minimum, maximum, step); } \
	BOOL Notify(INT message, INT newValue, INT oldValue) \
	{ return m_c->__Name ## OnNotify(message, newValue, oldValue); } \
private: \
	__TClass* m_c; \
}


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_P_CALLBACK_SLIDER(SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_P_CALLBACK_SLIDER(__Name) \
	CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(CSettingKeySlider, __Name)


#endif

