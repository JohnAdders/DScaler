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

#include "ISetting.h"
#include "SettingHolder.h"


/** Base class for settings that can be represented as a long    
*/
class CSimpleSetting : public ISetting
{
public:
    CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, long StepValue, 
                   CSettingGroup* pGroup = NULL);
    CSimpleSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);
    virtual ~CSimpleSetting();
        
    long GetDefault();
    void SetDefault();
    void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE);

    void SetStepValue(long Step);
    void SetMin(long Min);
    void SetMax(long Max);    
    long GetMin();
    long GetMax();
    
    long GetValue();
    operator long();
        
    void SetValue(long NewValue, BOOL bSupressOnChange = FALSE);
    void Up();
    void Down();
    void ChangeValue(eCHANGEVALUE NewValue);    
    
    void OSDShow();
    LPCSTR GetDisplayName();

    void SetSection(LPCSTR NewValue);
    LPCSTR GetSection();
    void SetEntry(LPCSTR NewValue);
    LPCSTR GetEntry();
    BOOL ReadFromIni();
    void WriteToIni(BOOL bOptimizeFileAccess);
    
    void SetGroup(CSettingGroup* pGroup); 
    CSettingGroup* GetGroup();
        
    BOOL ReadFromIniSubSection(LPCSTR szSubSection);
    void WriteToIniSubSection(LPCSTR szSubSection, BOOL bOptimizeFileAccess = TRUE);

    SETTING* GetSETTING() { return m_pSetting; }
    
    virtual void OnChange(long NewValue, long OldValue);   
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
    std::string    m_SectionLastSavedValueIniSection;
    /// Internal storage for value of the last read/written value
    long           m_SectionLastSavedValue;
    /// Internal storage for ini section of the last read/written value
    std::string    m_sLastSavedValueIniSection;
    /// Internal storage for the actual value of the setting
    long           m_StoreValue;
    
    /// Set to TRUE to free m_pSetting at destruction
    BOOL           m_bFreeSettingOnExit;
    /// Internal read/write flags
    long           m_ReadWriteFlags;

    /// Actual setting info.
    SETTING*       m_pSetting;

    /// Setting group
    CSettingGroup* m_pGroup;
    /// OnChange calls enabled/disabled
    BOOL           m_EnableOnChange;
    
    ///Check flags and decide if onchange should be called
    BOOL DoOnChange(long NewValue, long OldValue);
};

/** Simple setting with a BOOL value
*/
class CYesNoSetting : public CSimpleSetting
{
public:
    CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry, 
                   CSettingGroup* pGroup = NULL);
    CYesNoSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);

    ~CYesNoSetting();
    SETTING_TYPE GetType() {return YESNO;};
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);

    void FillSettingStructure(SETTING* pSetting);
};

/** Simple setting with a long value represenmted by a slider
*/
class CSliderSetting : public CSimpleSetting
{
public:    
    CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, 
                   CSettingGroup* pGroup = NULL);
    CSliderSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);
    
    ~CSliderSetting();
    SETTING_TYPE GetType() {return SLIDER;};
    void SetOSDDivider(long OSDDivider);
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);

    void FillSettingStructure(SETTING* pSetting);
private:
    //long m_OSDDivider;
};

/** Simple setting with a selection from a list
*/
class CListSetting : public CSimpleSetting
{
public:
    CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList, 
                   CSettingGroup* pGroup = NULL);
    CListSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);

    ~CListSetting();
    SETTING_TYPE GetType() {return ITEMFROMLIST;};
    const char** GetList() {return m_pSetting->pszList;}
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);

    void FillSettingStructure(SETTING* pSetting);
private:
//    const char** m_List;
};


/** Simple setting with a character string value
*/
class CStringSetting : public CSimpleSetting
{
public:
    CStringSetting(LPCSTR DisplayName, long Default, LPCSTR Section, LPCSTR Entry, CSettingGroup* pGroup = NULL);
    CStringSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);

    ~CStringSetting();
    SETTING_TYPE GetType() {return CHARSTRING;};
    void GetDisplayText(LPSTR szBuffer);
    void SetDefault();
    void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE);
    void SetValue(long NewValue, BOOL bSupressOnChange = FALSE);
    void Up();
    void Down();
    void ChangeValue(eCHANGEVALUE NewValue);    

    BOOL ReadFromIni();
    void WriteToIni(BOOL bOptimizeFileAccess);
    BOOL ReadFromIniSubSection(LPCSTR szSubSection);
    void WriteToIniSubSection(LPCSTR szSubSection, BOOL bOptimizeFileAccess = TRUE);

    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd);

    void FillSettingStructure(SETTING* pSetting);
private:
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, BOOL Default, LPCSTR Section, CSettingGroup* pGroup = NULL) : \
             CYesNoSetting(DisplayName, Default, Section, #Name, pGroup), m_Parent((Class*)Parent) {;} \
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, CSettingGroup* pGroup = NULL) : \
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Max, LPCSTR Section, const char** pszList, CSettingGroup* pGroup = NULL) : \
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, char* Default, LPCSTR Section, CSettingGroup* pGroup = NULL) : \
             CStringSetting(DisplayName, (long)Default, Section, #Name, pGroup), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);


#endif

