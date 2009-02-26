/////////////////////////////////////////////////////////////////////////////
// $Id$
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

#ifndef __SETTINGKEY_H__
#define __SETTINGKEY_H__

#include "SettingRepository.h"
#include "SettingGroup.h"
#include "SettingLimiter.h"
#include "SettingValue.h"
#include <string>

#include "ISetting.h"

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

    virtual void LoadSetting(BYTE options = 0);
    virtual void SaveSetting();

    virtual void SetValueSV(RCSETTINGVALUE value, BYTE options = 0);
    virtual CSettingValue GetValueSV();

    virtual void UseDefault(BYTE options = 0);
    virtual void SetDefaultSV(RCSETTINGVALUE value, BYTE options = 0);
    virtual CSettingValue GetDefaultSV();

    virtual void CheckLimiter(BYTE options = 0);
    virtual void SetLimiter(PSETTINGLIMITER limiter, BYTE options = 0);
    virtual PSETTINGLIMITER GetLimiter();

    virtual BOOL Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue);

    void            SetController(PSETTINGGROUP controller, HSETTING identifier);
    PSETTINGGROUP    GetController() const;
    HSETTING        GetIdentifier() const;

    inline operator PSETTINGKEY() { return this; };
    inline operator HSETTING() const { return GetIdentifier(); };

protected:
    HSETTING        m_identifier;
    PSETTINGGROUP    m_controller;
};


//////////////////////////////////////////////////////////////////////////
// CSettingKeyLong
//////////////////////////////////////////////////////////////////////////
class CSettingKeyLong : public CSettingKey
{
public:
    CSettingKeyLong();
    virtual ~CSettingKeyLong();

    static PSETTINGOBJECT NewSetting(LPCSTR entry, long initial);
    static PSETTINGOBJECT NewSetting(LPCSTR entry, long initial, long minimum, long maximum);

    virtual void SetValue(long value, BYTE options = 0);
    virtual long GetValue();

    virtual void SetDefault(long value, BYTE options = 0);
    virtual long GetDefault();

    virtual void SetRange(long minimum, long maximum, BYTE options = 0);

    virtual BOOL Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue);
    virtual BOOL Notify(INT message, long newValue, long oldValue);
};


//////////////////////////////////////////////////////////////////////////
// CSettingKeyString
//////////////////////////////////////////////////////////////////////////
class CSettingKeyString : public CSettingKey
{
public:
    CSettingKeyString();
    virtual ~CSettingKeyString();

    static PSETTINGOBJECT NewSetting(LPCSTR entry, std::string initial);
    static PSETTINGOBJECT NewSetting(LPCSTR entry, std::string initial, ULONG maxLength);

    virtual void SetValue(std::string value, BYTE options = 0);
    virtual std::string GetValue();

    virtual void SetDefault(std::string value, BYTE options = 0);
    virtual std::string GetDefault();

    virtual void SetMaxLength(ULONG maxLength, BYTE options = 0);

    virtual BOOL Notify(INT message, RCSETTINGVALUE newValue, RCSETTINGVALUE oldValue);
    virtual BOOL Notify(INT message, std::string newValue, std::string oldValue);
};


//////////////////////////////////////////////////////////////////////////
// CSettingKeySlider
//////////////////////////////////////////////////////////////////////////
class CSettingKeySlider : public CSettingKeyLong
{
public:
    CSettingKeySlider();
    virtual ~CSettingKeySlider();

    virtual void Setup(long initial, long minimum, long maximum, long step = 1);

    virtual PSETTINGOBJECT CreateSetting(LPCSTR entry);
    virtual PSETTINGCONFIG CreateSliderConfig();

    virtual void StepUp(BYTE options = 0);
    virtual void StepDown(BYTE options = 0);

    virtual void SetRange(long minimum, long maximum, BYTE options = 0);

    virtual void SetMin(long minimum, BYTE options = 0);
    virtual void SetMax(long maximum, BYTE options = 0);
    virtual long GetMin();
    virtual long GetMax();

    virtual void SetStep(long step);

protected:
    long m_initial;
    long m_minimum;
    long m_maximum;
    long m_step;
};


//////////////////////////////////////////////////////////////////////////
// CSettingKeyDSSimple
//////////////////////////////////////////////////////////////////////////
class CSettingKeyDSSimple : public CSettingKeySlider, public ISetting
{
public:
    CSettingKeyDSSimple();
    virtual ~CSettingKeyDSSimple();

    virtual void SetDefault() { CSettingKeySlider::UseDefault(); }
    virtual SETTING_TYPE GetType() { return SLIDER; };

    virtual long GetValue() { return CSettingKeySlider::GetValue(); }
    virtual void ChangeValue(eCHANGEVALUE method);

    virtual long GetMin() { return CSettingKeySlider::GetMin(); }
    virtual long GetMax() { return CSettingKeySlider::GetMax(); }
    virtual long GetDefault() { return CSettingKeySlider::GetDefault(); }

    virtual void ChangeDefault(long newDefault, BOOL = FALSE);
    virtual void SetDefault(long newValue, BOOL = FALSE);
    virtual void SetValue(long newValue, BOOL = FALSE);

    virtual void OSDShow();

    virtual void SetupControl(HWND hWnd);
    virtual void SetControlValue(HWND hWnd);
    virtual void SetFromControl(HWND hWnd);

    // Unsupported
    virtual BOOL ReadFromIni() { return FALSE; }
    virtual void WriteToIni(BOOL) { }

    virtual BOOL ReadFromIniSubSection(LPCSTR) { return FALSE; }
    virtual void WriteToIniSubSection(LPCSTR, BOOL = TRUE) { }

    virtual void DisableOnChange() { }
    virtual void EnableOnChange() { }

    virtual void SetGroup(CSettingGroup*) { }; 
    virtual CSettingGroup* GetGroup() { return NULL; }
};


//////////////////////////////////////////////////////////////////////////
// Macros for simplifying declaration of settings with callbacks.
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
// CSETTINGKEY_C_CALLBACK_NOTIFY_LONG(...) -- internal use
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_C_CALLBACK_NOTIFY_LONG(__KeyClass, __TClass, __Name) \
public: \
    BOOL __Name ## OnNotify(INT message, long newValue, long oldValue); \
protected: \
class C ## __Name ## Setting : public __KeyClass \
{ \
public: \
    C ## __Name ## Setting() : __KeyClass(), m_c(NULL) { } \
    void SetParent(__TClass* ptr) \
    { m_c = ptr; } \
    BOOL Notify(INT message, long newValue, long oldValue) \
    { return m_c == NULL || m_c->__Name ## OnNotify(message, newValue, oldValue); } \
private: \
    __TClass* m_c; \
}


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(...) -- internal use
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(__KeyClass, __Name) \
BOOL __Name ## OnNotify(INT message, long newValue, long oldValue); \
class C ## __Name ## Setting : public __KeyClass \
{ \
public: \
    BOOL Notify(INT message, long newValue, long oldValue) \
    { return __Name ## OnNotify(message, newValue, oldValue); } \
}


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_C_CALLBACK_INIT(InstanceVariable)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_C_CALLBACK_INIT(__Instance) \
    (__Instance).SetParent(this)


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_C_CALLBACK_LONG(ClassName, SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_C_CALLBACK_LONG(__TClass, __Name) \
    CSETTINGKEY_C_CALLBACK_NOTIFY_LONG(CSettingKeyLong, __TClass, __Name)


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_P_CALLBACK_LONG(SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_P_CALLBACK_LONG(__Name) \
    CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(CSettingKeyLong, __Name)


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_C_CALLBACK_SLIDER(ClassName, SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_C_CALLBACK_SLIDER(__TClass, __Name) \
    CSETTINGKEY_C_CALLBACK_NOTIFY_LONG(CSettingKeySlider, __TClass, __Name)


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_P_CALLBACK_SLIDER(SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_P_CALLBACK_SLIDER(__Name) \
    CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(CSettingKeySlider, __Name)


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_C_CALLBACK_DSSIMPLE(ClassName, SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_C_CALLBACK_DSSIMPLE(__TClass, __Name) \
    CSETTINGKEY_C_CALLBACK_NOTIFY_LONG(CSettingKeyDSSimple, __TClass, __Name)


//////////////////////////////////////////////////////////////////////////
// CSETTINGKEY_P_CALLBACK_DSSIMPLE(SettingName)
//////////////////////////////////////////////////////////////////////////
#define CSETTINGKEY_P_CALLBACK_DSSIMPLE(__Name) \
    CSETTINGKEY_P_CALLBACK_NOTIFY_LONG(CSettingKeyDSSimple, __Name)


//////////////////////////////////////////////////////////////////////////
// SRUtil_Ex_Add_Long, SRUtil_Ex_Add_Long
//////////////////////////////////////////////////////////////////////////
void SRUtil_Set_Add(PSETTINGGROUP group);
void SRUtil_Ex_Set_Add(PSETTINGGROUPEX groupEx, DBIT dependentBits = 0, DBIT absoluteBits = 0);
HSETTING SRUtil_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue);
HSETTING SRUtil_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue, LONG minValue, LONG maxValue);
HSETTING SRUtil_Add_Long(CSettingKeySlider& key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue);
HSETTING SRUtil_Add_Long(CSettingKeySlider& key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue, LONG minValue, LONG maxValue, LONG stepValue = 1);
HSETTING SRUtil_Add_String(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LPCSTR defaultValue);
HSETTING SRUtil_Add_String(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LPCSTR defaultValue, ULONG maxLength);
HSETTING SRUtil_Ex_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue);
HSETTING SRUtil_Ex_Add_Long(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue,  LONG minValue, LONG maxValue);
HSETTING SRUtil_Ex_Add_Long(CSettingKeySlider& key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue);
HSETTING SRUtil_Ex_Add_Long(CSettingKeySlider& key, LPCSTR displayName, LPCSTR entryName, LONG defaultValue,  LONG minValue, LONG maxValue, LONG stepValue = 1);
HSETTING SRUtil_Ex_Add_String(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LPCSTR defaultValue);
HSETTING SRUtil_Ex_Add_String(PSETTINGKEY key, LPCSTR displayName, LPCSTR entryName, LPCSTR defaultValue, ULONG maxLength);
HSETTING SRUtil_Ex_AddMaster_Long(PSETTINGKEY key, DBIT masterBits, LPCSTR displayName, LPCSTR entryName, LONG defaultValue,  LONG minValue, LONG maxValue);


#endif

