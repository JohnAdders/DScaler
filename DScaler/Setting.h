/////////////////////////////////////////////////////////////////////////////
// $Id: Setting.h,v 1.8 2002-08-17 18:05:30 tobbej Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

#ifndef __SETTING_H___
#define __SETTING_H___

#include "Settings.h"

class ISetting;

/** Base class for any calss that needs acesss to a list of ISetting settings.
    The function CreateSettings should be overriden and then called in the 
    constructor of the derived class.
*/
class CSettingsHolder
{
public:
    CSettingsHolder(long SetMessage);
    ~CSettingsHolder();
    long GetNumSettings();
    ISetting* GetSetting(long SettingIndex);
    LONG HandleSettingsMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bHandled);
    virtual void CreateSettings(LPCSTR IniSection) = 0;
    void ReadFromIni();
    void WriteToIni(BOOL bOptimizeFileAccess);
protected:
    vector<ISetting*> m_Settings;
    long m_SetMessage;
};

/** Interface for control of a setting
*/
class ISetting
{
public:
    virtual ~ISetting() {;};
    virtual void SetDefault(BOOL bNoOnChange = FALSE) = 0;
    virtual SETTING_TYPE GetType() = 0;
    virtual void ChangeValue(eCHANGEVALUE NewValue) = 0;
    virtual BOOL ReadFromIni(BOOL bDontSetDefault = FALSE) = 0;
    virtual void WriteToIni(BOOL bOptimizeFileAccess) = 0;
    virtual long GetValue() = 0;
    virtual long GetMin() = 0;
    virtual long GetMax() = 0;
    virtual long GetDefault() = 0;
    virtual void SetValue(long NewValue, BOOL bNoOnChange = FALSE) = 0;
    virtual void OnChange(long NewValue, long OldValue) {;};
    virtual void OSDShow() = 0;
    virtual const char** GetList() {return NULL;};
    virtual void SetupControl(HWND hWnd) = 0;
    virtual void SetControlValue(HWND hWnd) = 0;
    virtual void SetFromControl(HWND hWnd) = 0;
};

/** Base class for settings that can be represented as a long
*/
class CSimpleSetting : public ISetting
{
public:
    CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, long StepValue);
    virtual ~CSimpleSetting();
    long GetValue();
    void SetValue(long NewValue, BOOL bNoOnChange = FALSE);
    void SetDefault(BOOL bNoOnChange = FALSE);
    operator long();
    void OSDShow();
    void Up();
    void Down();
    void ChangeValue(eCHANGEVALUE NewValue);
    void SetSection(LPCSTR NewValue);
    LPCSTR GetSection() { return m_Section.c_str(); }
    void SetEntry(LPCSTR NewValue) { m_Entry = NewValue; }
    LPCSTR GetEntry() { return m_Entry.c_str(); }
    BOOL ReadFromIni(BOOL bDontSetDefault = FALSE);
    void WriteToIni(BOOL bOptimizeFileAccess);
    void SetStepValue(long Step);
    void SetMin(long Min);
    void SetMax(long Max);
    void ChangeDefault(long NewDefault);
    long GetMin();
    long GetMax();
    long GetDefault();
    virtual void GetDisplayText(LPSTR szBuffer) = 0;
protected:
    std::string  m_DisplayName;
    long m_Value;
    long m_LastSavedValue;
    long m_Default;
    long m_Max;
    long m_Min;
    std::string  m_Section;
    std::string  m_Entry;
    long m_StepValue;
    long m_SettingID;
};

/** Simple setting with a BOOL value
*/
class CYesNoSetting : public CSimpleSetting
{
public:
    CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry);
    ~CYesNoSetting();
    SETTING_TYPE GetType() {return YESNO;};
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
};

/** Simple setting with a long value represenmted by a slider
*/
class CSliderSetting : public CSimpleSetting
{
public:
    CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry);
    ~CSliderSetting();
    SETTING_TYPE GetType() {return SLIDER;};
    void SetOSDDivider(long OSDDivider);
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
private:
    long m_OSDDivider;
};

/** Simple setting with a selection from a list
*/
class CListSetting : public CSimpleSetting
{
public:
    CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList);
    ~CListSetting();
    SETTING_TYPE GetType() {return ITEMFROMLIST;};
    const char** GetList() {return m_List;}
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
private:
    const char** m_List;
};

#define DEFINE_YESNO_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CYesNoSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, BOOL Default, LPCSTR Section) : CYesNoSetting(DisplayName, Default, Section, #Name), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);

#define DEFINE_SLIDER_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CSliderSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section) : \
             CSliderSetting(DisplayName, Default, Min, Max, Section, #Name), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);

#define DEFINE_LIST_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CListSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Max, LPCSTR Section, const char** pszList) : \
             CListSetting(DisplayName, Default, Max, Section, #Name, pszList), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);

#endif