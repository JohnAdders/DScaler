/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file setting.h setting Header file
 */

#ifndef __SETTING_H___
#define __SETTING_H___

#include "SettingGroup.h"
#include "SettingHolder.h"


/** Base class for settings that can be represented as a long
*/
class CSimpleSetting
{
public:
    CSimpleSetting(const tstring& DisplayName, const tstring& Section, const tstring& Entry, CSettingGroup* pGroup = NULL);
    virtual ~CSimpleSetting();

    virtual SETTING_TYPE GetType() = 0;
    virtual void SetupControl(HWND hWnd) = 0;
    virtual void SetControlValue(HWND hWnd) = 0;
    virtual void SetFromControl(HWND hWnd) = 0;
    virtual LPARAM GetValueAsMessage() = 0;
    virtual void SetValueFromMessage(LPARAM LParam) = 0;
    void ChangeValue(eCHANGEVALUE TypeOfChange);
    virtual tstring GetDisplayValue() = 0;
    virtual void ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue = FALSE) = 0;

    void OSDShow();
    const tstring& GetDisplayName();

    void SetSection(const tstring& NewValue);
    const tstring& GetSection();
    void SetEntry(const tstring& NewValue);
    const tstring& GetEntry();
    BOOL ReadFromIni();
    void WriteToIni(BOOL bOptimizeFileAccess);

    void SetGroup(CSettingGroup* pGroup);
    CSettingGroup* GetGroup();

    BOOL ReadFromIniSubSection(const tstring& SubSection);
    void WriteToIniSubSection(const tstring& SubSection, BOOL bOptimizeFileAccess = TRUE);

    void DisableOnChange();
    void EnableOnChange();

protected:
    BOOL IsOnChangeEnabled();

private:
    virtual tstring GetValueAsString() = 0;
    virtual void SetValueFromString(const tstring& NewValue) = 0;
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange) = 0;
    /// Internal storage for display name
    tstring    m_DisplayName;
    /// Internal storage for default ini section
    tstring    m_Section;
    /// Internal storage for ini entry
    tstring    m_Entry;
    /// Internal storage for ini section of the last read/written value
    tstring    m_LastSavedValueIniSection;
    /// Internal storage for value of the last read/written value
    tstring    m_LastSavedValue;
    /// Internal read/write flags
    long           m_ReadWriteFlags;

    /// Setting group
    CSettingGroup* m_pGroup;
    /// OnChange calls enabled/disabled
    BOOL           m_EnableOnChange;
};

/** Wrapper for C SETTING object
*/
class CSettingWrapper : public CSimpleSetting
{
public:
    CSettingWrapper(SETTING* pSetting, CSettingGroup* pGroup = NULL);

    ~CSettingWrapper();
    SETTING_TYPE GetType();
    BOOL GetValue();
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
    virtual LPARAM GetValueAsMessage();
    virtual void SetValueFromMessage(LPARAM LParam);
    virtual tstring GetDisplayValue();
    virtual void ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue = FALSE);
    void DoOnChangeEvent();
private:
    virtual tstring GetValueAsString();
    virtual void SetValueFromString(const tstring& NewValue);
    void SetValue(LONG_PTR NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    SETTING* m_Setting;
};


/** Simple setting with a BOOL value
*/
class CYesNoSetting : public CSimpleSetting
{
public:
    CYesNoSetting(const tstring& DisplayName, BOOL Default, const tstring& Section, const tstring& Entry,
                   CSettingGroup* pGroup = NULL);
    ~CYesNoSetting();
    SETTING_TYPE GetType() {return YESNO;};
    BOOL GetValue();
    void SetValue(BOOL newValue, BOOL SuppressOnChange = FALSE);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
    virtual LPARAM GetValueAsMessage();
    virtual void SetValueFromMessage(LPARAM LParam);
    virtual tstring GetDisplayValue();
    virtual void ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue = FALSE);
    void DoOnChangeEvent();
private:
    virtual tstring GetValueAsString();
    virtual void SetValueFromString(const tstring& NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    virtual void OnChange(BOOL NewValue, BOOL OldValue) {};
    BOOL m_Default;
    BOOL m_Value;
};

/** Simple setting with a long value represenmted by a slider
*/
class CSliderSetting : public CSimpleSetting
{
public:
    CSliderSetting(const tstring& DisplayName, long Default, long Min, long Max, const tstring& Section, const tstring& Entry,
                   CSettingGroup* pGroup = NULL);
    ~CSliderSetting();
    SETTING_TYPE GetType() {return SLIDER;};
    void SetMin(long Min);
    void SetMax(long Max);
    long GetMin();
    long GetMax();
    long GetValue();
    void SetValue(long NewValue, BOOL SupressOnChange = FALSE);
    void SetStepValue(long Step);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
    virtual LPARAM GetValueAsMessage();
    virtual void SetValueFromMessage(LPARAM LParam);
    virtual tstring GetDisplayValue();
    virtual void ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue = FALSE);
    void DoOnChangeEvent();
private:
    virtual tstring GetValueAsString();
    virtual void SetValueFromString(const tstring& NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    virtual void OnChange(long NewValue, long OldValue) {};
    long m_Default;
    long m_Min;
    long m_Max;
    long m_Value;
    long m_StepValue;
};

/** Simple setting with a selection from a list
*/
class CListSetting : public CSimpleSetting
{
public:
    CListSetting(const tstring& DisplayName, long Default, long Max, const tstring& Section, const tstring& Entry, const char** pszList,
                   CSettingGroup* pGroup = NULL);
    ~CListSetting();
    SETTING_TYPE GetType() {return ITEMFROMLIST;};
    const char** GetList();
    long GetValue();
    void SetValue(long NewValue, BOOL SupressOnChange = FALSE);
    long GetNumItems();
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
    virtual LPARAM GetValueAsMessage();
    virtual void SetValueFromMessage(LPARAM LParam);
    virtual void ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue = FALSE);
    virtual tstring GetDisplayValue();
    void DoOnChangeEvent();
private:
    virtual tstring GetValueAsString();
    virtual void SetValueFromString(const tstring& NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    virtual void OnChange(long NewValue, long OldValue) {};
    long m_Default;
    long m_Max;
    long m_Value;
    const char** m_List;
};


/** Simple setting with a character tstring value
*/
class CStringSetting : public CSimpleSetting
{
public:
    CStringSetting(const tstring& DisplayName, const tstring& Default, const tstring& Section, const tstring& Entry, CSettingGroup* pGroup = NULL);
    ~CStringSetting();
    SETTING_TYPE GetType() {return TCHARSTRING;};
    void GetDisplayText(LPTSTR szBuffer);
    const TCHAR* GetValue();
    void SetValue(const TCHAR* NewValue, BOOL SupressOnChange = FALSE);
    void ChangeValue(eCHANGEVALUE NewValue);

    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
    virtual LPARAM GetValueAsMessage();
    virtual void SetValueFromMessage(LPARAM LParam);
    virtual tstring GetDisplayValue();
    virtual void ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue = FALSE);
    void DoOnChangeEvent();
private:
    virtual tstring GetValueAsString();
    virtual void SetValueFromString(const tstring& NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    virtual void OnChange(const tstring& NewValue, const tstring& OldValue) {};
    tstring m_Value;
    tstring m_Default;

};

class CEmptySetting : public CYesNoSetting
{
public:
    CEmptySetting();
};


/// This class should be used to store the value of all tstring settings within the
/// main DScaler app, using this class means that the allocated tstring gets deleted properly on exit
class SettingStringValue
{
public:
    SettingStringValue();
    ~SettingStringValue();
    operator LPCTSTR();
    BOOL IsValid();
    LONG_PTR* GetPointer();
    void clear();
    void SetValue(const tstring& NewValue);
private:
    TCHAR* m_Value;
};


/**
    Macro's to simplify definition of classes derived from one of the above
    setting class types.
    Defines an OnChange function.
*/

#define DEFINE_YESNO_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CYesNoSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, const tstring& DisplayName, BOOL Default, const tstring& Section, CSettingGroup* pGroup = NULL) : \
             CYesNoSetting(DisplayName, Default, Section, _T(#Name), pGroup), m_Parent((Class*)Parent) {;} \
        virtual void OnChange(BOOL NewValue, BOOL OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    SmartPtr<C ## Name ## Setting> m_## Name;\
    public: \
    void Name ## OnChange(BOOL NewValue, BOOL OldValue);

#define DEFINE_SLIDER_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CSliderSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, const tstring& DisplayName, long Default, long Min, long Max, const tstring& Section, CSettingGroup* pGroup = NULL) : \
             CSliderSetting(DisplayName, Default, Min, Max, Section, _T(#Name), pGroup), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    SmartPtr<C ## Name ## Setting> m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);

#define DEFINE_LIST_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CListSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, const tstring& DisplayName, long Default, long Max, const tstring& Section, const char** pszList, CSettingGroup* pGroup = NULL) : \
             CListSetting(DisplayName, Default, Max, Section, _T(#Name), pszList, pGroup), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    SmartPtr<C ## Name ## Setting> m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);

#define DEFINE_STRING_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CStringSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, const tstring& DisplayName, TCHAR* Default, const tstring& Section, CSettingGroup* pGroup = NULL) : \
             CStringSetting(DisplayName, Default, Section, _T(#Name), pGroup), m_Parent((Class*)Parent) {;} \
        void OnChange(const tstring& NewValue, const tstring& OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    SmartPtr<C ## Name ## Setting> m_## Name;\
    public: \
    void Name ## OnChange(const tstring& NewValue, const tstring& OldValue);


#endif

