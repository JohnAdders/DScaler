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
    CSimpleSetting(const std::string& DisplayName, const std::string& Section, const std::string& Entry, CSettingGroup* pGroup = NULL);
    virtual ~CSimpleSetting();

    virtual SETTING_TYPE GetType() = 0;
    virtual void SetupControl(HWND hWnd) = 0;
    virtual void SetControlValue(HWND hWnd) = 0;
    virtual void SetFromControl(HWND hWnd) = 0;
    virtual LPARAM GetValueAsMessage() = 0;
    virtual void SetValueFromMessage(LPARAM LParam) = 0;
    void ChangeValue(eCHANGEVALUE TypeOfChange);
    virtual std::string GetDisplayValue() = 0;
    
    void OSDShow();
    const std::string& GetDisplayName();

    void SetSection(const std::string& NewValue);
    const std::string& GetSection();
    void SetEntry(const std::string& NewValue);
    const std::string& GetEntry();
    BOOL ReadFromIni();
    void WriteToIni(BOOL bOptimizeFileAccess);
    
    void SetGroup(CSettingGroup* pGroup); 
    CSettingGroup* GetGroup();
        
    BOOL ReadFromIniSubSection(const std::string& SubSection);
    void WriteToIniSubSection(const std::string& SubSection, BOOL bOptimizeFileAccess = TRUE);

    void DisableOnChange();
    void EnableOnChange();

protected:
    BOOL IsOnChangeEnabled();

private:    
    virtual std::string GetValueAsString() = 0;
    virtual void SetValueFromString(const std::string& NewValue) = 0;
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange) = 0;
    /// Internal storage for display name
    std::string    m_DisplayName;
    /// Internal storage for default ini section
    std::string    m_Section;
    /// Internal storage for ini entry
    std::string    m_Entry;
    /// Internal storage for ini section of the last read/written value
    std::string    m_LastSavedValueIniSection;
    /// Internal storage for value of the last read/written value
    std::string    m_LastSavedValue;
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
    virtual std::string GetDisplayValue();
private:    
    virtual std::string GetValueAsString();
    virtual void SetValueFromString(const std::string& NewValue);
    void SetValue(long NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    SETTING* m_Setting;
};


/** Simple setting with a BOOL value
*/
class CYesNoSetting : public CSimpleSetting
{
public:
    CYesNoSetting(const std::string& DisplayName, BOOL Default, const std::string& Section, const std::string& Entry, 
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
    void ChangeDefault(BOOL NewDefault, BOOL bDontSetValue = FALSE);
    virtual std::string GetDisplayValue();
private:    
    virtual std::string GetValueAsString();
    virtual void SetValueFromString(const std::string& NewValue);
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
    CSliderSetting(const std::string& DisplayName, long Default, long Min, long Max, const std::string& Section, const std::string& Entry, 
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
    void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE);
    virtual LPARAM GetValueAsMessage();
    virtual void SetValueFromMessage(LPARAM LParam);
    virtual std::string GetDisplayValue();
private:    
    virtual std::string GetValueAsString();
    virtual void SetValueFromString(const std::string& NewValue);
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
    CListSetting(const std::string& DisplayName, long Default, long Max, const std::string& Section, const std::string& Entry, const char** pszList, 
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
    void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE);
    virtual std::string GetDisplayValue();
private:    
    virtual std::string GetValueAsString();
    virtual void SetValueFromString(const std::string& NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    virtual void OnChange(long NewValue, long OldValue) {};
    long m_Default;
    long m_Max;
    long m_Value;
    const char** m_List;
};


/** Simple setting with a character string value
*/
class CStringSetting : public CSimpleSetting
{
public:
    CStringSetting(const std::string& DisplayName, const std::string& Default, const std::string& Section, const std::string& Entry, CSettingGroup* pGroup = NULL);
    ~CStringSetting();
    SETTING_TYPE GetType() {return CHARSTRING;};
    void GetDisplayText(LPSTR szBuffer);
    const char* GetValue();
    void SetValue(const char* NewValue, BOOL SupressOnChange = FALSE);
    void ChangeValue(eCHANGEVALUE NewValue);    

    BOOL ReadFromIni();
    void WriteToIni(BOOL bOptimizeFileAccess);
    BOOL ReadFromIniSubSection(const std::string& szSubSection);
    void WriteToIniSubSection(const std::string& szSubSection, BOOL bOptimizeFileAccess = TRUE);

    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);
    virtual LPARAM GetValueAsMessage();
    virtual void SetValueFromMessage(LPARAM LParam);
    virtual std::string GetDisplayValue();
private:    
    virtual std::string GetValueAsString();
    virtual void SetValueFromString(const std::string& NewValue);
    virtual void ChangeValueInternal(eCHANGEVALUE TypeOfChange);
    virtual void OnChange(const std::string& NewValue, const std::string& OldValue) {};
    std::string m_Value;
    std::string m_Default;

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
        C ## Name ## Setting(const Class* Parent, const std::string& DisplayName, BOOL Default, const std::string& Section, CSettingGroup* pGroup = NULL) : \
             CYesNoSetting(DisplayName, Default, Section, #Name, pGroup), m_Parent((Class*)Parent) {;} \
        virtual void OnChange(BOOL NewValue, BOOL OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(BOOL NewValue, BOOL OldValue);

#define DEFINE_SLIDER_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CSliderSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, const std::string& DisplayName, long Default, long Min, long Max, const std::string& Section, CSettingGroup* pGroup = NULL) : \
             CSliderSetting(DisplayName, Default, Min, Max, Section, #Name, pGroup), m_Parent((Class*)Parent) {;} \
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
        C ## Name ## Setting(const Class* Parent, const std::string& DisplayName, long Default, long Max, const std::string& Section, const char** pszList, CSettingGroup* pGroup = NULL) : \
             CListSetting(DisplayName, Default, Max, Section, #Name, pszList, pGroup), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);

#define DEFINE_STRING_CALLBACK_SETTING(Class, Name) \
    protected: \
    class C ## Name ## Setting : public CStringSetting \
    { \
    public: \
        C ## Name ## Setting(const Class* Parent, const std::string& DisplayName, char* Default, const std::string& Section, CSettingGroup* pGroup = NULL) : \
             CStringSetting(DisplayName, (long)Default, Section, #Name, pGroup), m_Parent((Class*)Parent) {;} \
        void OnChange(const std::string& NewValue, const std::string& OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(const std::string& NewValue, const std::string& OldValue);


#endif

