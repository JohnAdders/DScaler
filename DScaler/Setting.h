/////////////////////////////////////////////////////////////////////////////
// $Id: Setting.h,v 1.14 2002-10-15 15:06:01 kooiman Exp $
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

#include "ISetting.h"
#include "SettingHolder.h"


/** Base class for settings that can be represented as a long    
*/
class CSimpleSetting : public ISetting
{
public:
    CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, long StepValue, 
                   CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0,
                   SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CSimpleSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CSimpleSetting(SETTINGEX* pSetting, CSettingGroup* pGroup);
    CSimpleSetting(SETTINGEX* pSetting, CSettingGroupList* pList);
    virtual ~CSimpleSetting();
        
    long GetDefault();
    void SetDefault(eOnChangeType OnChangeType = ONCHANGE_SET);
    void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE, eOnChangeType OnChangeType = ONCHANGE_NONE);

    void SetStepValue(long Step);
    void SetMin(long Min);
    void SetMax(long Max);    
    long GetMin();
    long GetMax();
    
    long GetValue();
    operator long();
        
    void SetValue(long NewValue, eOnChangeType OnChangeType = ONCHANGE_SET);
    void Up(eOnChangeType OnChangeType = ONCHANGE_SET);
    void Down(eOnChangeType OnChangeType = ONCHANGE_SET);
    void ChangeValue(eCHANGEVALUE NewValue, eOnChangeType OnChangeType = ONCHANGE_SET);    
    
    void OSDShow();
    LPCSTR GetDisplayName();

    void SetFlags(eSettingFlags SettingsFlag);
    void SetFlag(eSettingFlags SettingFlag, BOOL bEnable);
    void SetDefaultFlags(eSettingFlags SettingFlags, BOOL bSetFlagsToDefault);
    eSettingFlags GetFlags();
    eSettingFlags GetDefaultFlags();
    eSettingFlags GetLastSavedFlagsValue();
    BOOL ReadFlagsFromIniSection(LPCSTR szSection, BOOL bSetDefaultOnFailure = TRUE);
    void WriteFlagsToIniSection(LPCSTR szSection, BOOL bOptimizeFileAccess);
    virtual void FlagsOnChange(eSettingFlags OldFlags, eSettingFlags Flags);

    void SetSection(LPCSTR NewValue);
    LPCSTR GetSection();
    void SetEntry(LPCSTR NewValue);
    LPCSTR GetEntry();
    LPCSTR GetLastSavedValueIniSection();
    BOOL ReadFromIni(BOOL bSetDefaultOnFailure = TRUE, eOnChangeType OnChangeType = ONCHANGE_NONE);
    void WriteToIni(BOOL bOptimizeFileAccess);
    
    void SetGroup(CSettingGroup* pGroup); 
    CSettingGroup* GetGroup();
        
    BOOL ReadFromIniSubSection(LPCSTR szSubSection, long* Value = NULL, BOOL bSetDefaultOnFailure = TRUE, eOnChangeType OnChangeType = ONCHANGE_NONE, eSettingFlags* pSettingFlags = NULL);
    void WriteToIniSubSection(LPCSTR szSubSection, BOOL bOptimizeFileAccess, long* Value = NULL, eSettingFlags* pSettingFlags = NULL);

    SETTING* GetSETTING() { return m_pSetting; }
    SETTINGEXPLUS* GetSETTINGEXPLUS() { return m_pSettingExPlus; }
    
    virtual void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType);   
    void DisableOnChange();
    void EnableOnChange();

    virtual void GetDisplayText(LPSTR szBuffer) = 0;    
protected:    
    /// Internal storage for display name
    std::string    m_DisplayName;
    /// Internal storage for default ini section
    std::string    m_Section;
    /// Internal storage for ini entry
    std::string    m_Entry;
    /// Internal storage for ini section of the last read/written value
    //std::string    m_SectionFromLastValue;
    /// Internal storage for ini section of the last read/written value
    std::string    m_sLastSavedValueIniSection;
    /// Internal storage for the actual value of the setting
    long           m_StoreValue;
    /// Internal structure with extended setting options
    SETTINGEXPLUS  m_StoreExPlus;
    
    /// Set to TRUE to free m_pSetting at destruction
    BOOL           m_bFreeSettingOnExit;
    /// Internal read/write flags
    long           m_ReadWriteFlags;

    /// Actual setting info.
    SETTING*       m_pSetting;
    /// Pointer to extended setting info.
    SETTINGEXPLUS* m_pSettingExPlus;

    /// Setting group
    CSettingGroup* m_pGroup;
    /// OnChange calls enabled/disabled
    BOOL           m_EnableOnChange;
    
    ///Check flags and decide if onchange should be called
    BOOL DoOnChange(long NewValue, long OldValue, eOnChangeType OnChangeType);

    ///Support for C-style SETTING
    //BOOL OnChangeWrapForSettingStructure(long NewValue, long OldValue, eOnChangeType OnChangeType);

    ///Static wrap for OnChange callback function
    //static BOOL StaticOnChangeWrapForSettingStructure(void* pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING* pSetting);
};

/** Simple setting with a BOOL value
*/
class CYesNoSetting : public CSimpleSetting
{
public:
    CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry, 
                   CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0,
                   SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CYesNoSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CYesNoSetting(SETTINGEX* pSetting, CSettingGroup* pGroup);
    CYesNoSetting(SETTINGEX* pSetting, CSettingGroupList* pList);

    ~CYesNoSetting();
    SETTING_TYPE GetType() {return YESNO;};
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET);

    void FillSettingStructure(SETTING* pSetting);
    void FillSettingStructureEx(SETTINGEX* pSetting);    
};

/** Simple setting with a long value represenmted by a slider
*/
class CSliderSetting : public CSimpleSetting
{
public:    
    CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, 
                   CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0,
                   SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CSliderSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CSliderSetting(SETTINGEX* pSetting, CSettingGroup* pGroup);
    CSliderSetting(SETTINGEX* pSetting, CSettingGroupList* pList);
    
    ~CSliderSetting();
    SETTING_TYPE GetType() {return SLIDER;};
    void SetOSDDivider(long OSDDivider);
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET);

    void FillSettingStructure(SETTING* pSetting);
    void FillSettingStructureEx(SETTINGEX* pSetting);    
private:
    //long m_OSDDivider;
};

/** Simple setting with a selection from a list
*/
class CListSetting : public CSimpleSetting
{
public:
    CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList, 
                   CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0,
                   SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CListSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, SETTINGEX_ONCHANGE* pfnExOnChange = NULL, void* pExOnChangeThis = NULL);
    CListSetting(SETTINGEX* pSetting, CSettingGroup* pGroup);
    CListSetting(SETTINGEX* pSetting, CSettingGroupList* pList);

    ~CListSetting();
    SETTING_TYPE GetType() {return ITEMFROMLIST;};
    const char** GetList() {return m_pSetting->pszList;}
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET);

    void FillSettingStructure(SETTING* pSetting);
    void FillSettingStructureEx(SETTINGEX* pSetting);    
private:
//    const char** m_List;
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, BOOL Default, LPCSTR Section, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0) : \
             CYesNoSetting(DisplayName, Default, Section, #Name, pGroup, SettingFlags, GUIinfo), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0) : \
             CSliderSetting(DisplayName, Default, Min, Max, Section, #Name, pGroup, SettingFlags, GUIinfo), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Max, LPCSTR Section, const char** pszList, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0) : \
             CListSetting(DisplayName, Default, Max, Section, #Name, pszList, pGroup, SettingFlags, GUIinfo), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);



#endif

