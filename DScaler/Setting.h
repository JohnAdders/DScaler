/////////////////////////////////////////////////////////////////////////////
// $Id: Setting.h,v 1.13 2002-10-02 10:55:17 kooiman Exp $
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

#include <vector>

#include "DS_ApiCommon.h"
#include "DS_Control.h"
#include "Events.h"
//#include "Settings.h"
//#include "TVFormats.h"

//Specify/get type of OnChangeEven
enum eOnChangeType
{
    ONCHANGE_NONE = 0,      //Don't call onchange
    ONCHANGE_INIT,          //first time    
    ONCHANGE_SET,           //user/program action (used most of the time)
    ONCHANGE_SET_FORCE,     //user/program action (used most of the time), ignore valuechanged flag
    ONCHANGE_SOURCECHANGE,  //new value because of source change
    ONCHANGE_VIDEOINPUTCHANGE,
    ONCHANGE_AUDIOINPUTCHANGE,
    ONCHANGE_VIDEOFORMATCHANGE,
    ONCHANGE_CHANNELCHANGE
};

enum eSettingFlags
{
    SETTINGFLAG_GLOBAL    =      0x0001,    
    SETTINGFLAG_PER_SOURCE =     0x0002,   //Read/Save setting on source change and call OnChange function
    SETTINGFLAG_PER_VIDEOINPUT = 0x0004,   //Read/Save setting on video input change and call OnChange function
    SETTINGFLAG_PER_AUDIOINPUT = 0x0008,
    SETTINGFLAG_PER_VIDEOFORMAT= 0x0010,
    SETTINGFLAG_PER_CHANNEL =    0x0020,

    SETTINGFLAG_PER_MASK =       0x003F,
    SETTINGFLAG_FLAGSTOINI_MASK= 0x003F,

    SETTINGFLAG_READFROMINI =    0x0100,
    SETTINGFLAG_ISIN_INI =       0x0200,
    
    SETTINGFLAG_FLAGIN_INI =     0x0400,

    SETTINGFLAG_HEXVALUE =       0x0800,   //For settings GUI, mask in GUIinfo
    SETTINGFLAG_BITMASK  =       0x1000,   //For settings GUI, mask in GUIinfo, bitnames in pszList     

    SETTINGFLAG_ALLOW_GLOBAL    =       0x00010000,    
    SETTINGFLAG_ALLOW_PER_SOURCE =      0x00020000,   //Read/Save setting on source change and call OnChange function
    SETTINGFLAG_ALLOW_PER_VIDEOINPUT =  0x00040000,   //Read/Save setting on video input change and call OnChange function
    SETTINGFLAG_ALLOW_PER_AUDIOINPUT =  0x00080000,
    SETTINGFLAG_ALLOW_PER_VIDEOFORMAT=  0x00100000,
    SETTINGFLAG_ALLOW_PER_CHANNEL =     0x00200000,

    SETTINGFLAG_ALLOW_ALL =             0x003F0000,    
    SETTINGFLAG_ALLOW_MASK=             0x003F0000,    

    
    SETTINGFLAG_ONCHANGE_VALUECHANGED = 0x00800000, //Only call onchange if the value changed    

    SETTINGFLAG_ONCHANGE_INIT =         0x01000000, //Allow onchange call on initialization
    SETTINGFLAG_ONCHANGE_SET =          0x02000000, //usualy set        
    SETTINGFLAG_ONCHANGE_READ =         0x04000000,
    SETTINGFLAG_ONCHANGE_SOURCE =       0x08000000,
    SETTINGFLAG_ONCHANGE_VIDEOINPUT=    0x10000000,
    SETTINGFLAG_ONCHANGE_AUDIOINPUT =   0x20000000,
    SETTINGFLAG_ONCHANGE_VIDEOFORMAT=   0x40000000,
    SETTINGFLAG_ONCHANGE_CHANNEL =      0x80000000,
    
    SETTINGFLAG_ONCHANGE_ALL =          0xFE000000, //excluded: init

};

typedef BOOL (__cdecl SETTINGEX_ONCHANGE)(void* pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING* pSetting);

typedef struct
{
    long  cbSize;        //total block size (sizeof(SETTINGEX))
    long  DefaultSettingFlags;
    char** pszGroupList;
    long  GUIinfo;  //depends on flags
    void*  pExOnChangeThis;
    SETTINGEX_ONCHANGE* pfnExOnChange;  
    long  SettingFlags;
    long  LastSavedSettingFlags;
    char* szLastSavedValueIniSection;
} SETTINGEXPLUS;

typedef struct
{
    char* szDisplayName;
    SETTING_TYPE Type;
    long LastSavedValue;
    long* pValue;
    long Default;
    long MinValue;
    long MaxValue;
    long StepValue;
    long OSDDivider;
    const char** pszList;
    char* szIniSection;
    char* szIniEntry;
    SETTING_ONCHANGE* pfnOnChange;
    long  cbSize;        //total block size
    long  DefaultSettingFlags;
    char** pszGroupList;
    long  GUIinfo;  //depends on flags  
    SETTINGEX_ONCHANGE* pfnExOnChange;  
    void*  pExOnChangeThis;
    long  SettingFlags;
    long  LastSavedSettingFlags;
    char* szLastSavedValueIniSection;
} SETTINGEX;

typedef struct
{
    long cbSize;
    char* szGroup;
    char* szDisplayName;
    char* szPopupInfo;
} SETTINGGROUP;


class CSettingGroup;
class CSettingGroupList;
class ISetting;
class CSource;


/** Interface for control of a setting
*/
class ISetting
{
public:
    virtual ~ISetting() {;};
    virtual void SetDefault(eOnChangeType OnChangeType = ONCHANGE_SET) = 0;
    virtual SETTING_TYPE GetType() = 0;
    virtual void ChangeValue(eCHANGEVALUE NewValue, eOnChangeType OnChangeType = ONCHANGE_SET ) = 0;
    virtual BOOL ReadFromIni(BOOL bSetDefaultOnFailure = TRUE, eOnChangeType OnChangeType = ONCHANGE_NONE) = 0;
    virtual void WriteToIni(BOOL bOptimizeFileAccess) = 0;
    virtual BOOL ReadFromIniSubSection(LPCSTR szSubSection, long* Value = NULL, BOOL bSetDefaultOnFailure = TRUE, eOnChangeType OnChangeType = ONCHANGE_NONE, eSettingFlags* pSettingFlags = NULL) = 0;
    virtual void WriteToIniSubSection(LPCSTR szSubSection, BOOL bOptimizeFileAccess, long* Value = NULL, eSettingFlags* pSettingFlags = NULL) = 0;  
    virtual LPCSTR GetLastSavedValueIniSection() = 0;
    virtual long GetValue() = 0;
    virtual long GetMin() = 0;
    virtual long GetMax() = 0;
    virtual long GetDefault() = 0;
    virtual void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE, eOnChangeType OnChangeType = ONCHANGE_NONE) = 0;
    virtual void SetValue(long NewValue, eOnChangeType OnChangeType = ONCHANGE_SET) = 0;
    virtual void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType) {;};
    virtual void DisableOnChange() = 0;
    virtual void EnableOnChange() = 0;
    virtual void OSDShow() = 0;
    virtual const char** GetList() {return NULL;};
    virtual void SetupControl(HWND hWnd) = 0;
    virtual void SetControlValue(HWND hWnd) = 0;
    virtual void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET) = 0;
    virtual void SetGroup(CSettingGroup* pGroup) = 0; 
    virtual CSettingGroup* GetGroup() = 0;
    virtual void SetFlags(eSettingFlags SettingFlags) = 0;
    virtual void SetFlag(eSettingFlags SettingFlag, BOOL bEnable) = 0;
    virtual eSettingFlags GetFlags() = 0;
    virtual eSettingFlags GetDefaultFlags() = 0;
    virtual BOOL ReadFlagsFromIniSection(LPCSTR szSection, BOOL bSetDefaultOnFailure = TRUE) = 0;
    virtual void WriteFlagsToIniSection(LPCSTR szSection, BOOL bOptimizeFileAccess) = 0;
    virtual eSettingFlags GetLastSavedFlagsValue() = 0; 
    virtual void FlagsOnChange(eSettingFlags OldFlags, eSettingFlags Flags) = 0;
};

// To interface with onchange functions for cases when 
//   no derived class is possible
typedef BOOL (__cdecl ONCHANGE_STATICFUNC)(void* pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING* pSetting);

/** Base class for settings that can be represented as a long
*/
class CSimpleSetting : public ISetting
{
public:
    CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, long StepValue, 
                   CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0,
                   ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
    CSimpleSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
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
    //Local string & value storage
    std::string  m_DisplayName;
    std::string  m_Section;
    std::string  m_Entry;
    std::string  m_SectionFromLastValue;
    std::string  m_sLastSavedValueIniSection;
    long m_StoreValue;
    SETTINGEXPLUS m_StoreExPlus;
    //Free SETTING on exit
    BOOL m_bFreeSettingOnExit;

    SETTING* m_pSetting;
    SETTINGEXPLUS* m_pSettingExPlus;

    CSettingGroup* m_pGroup;
    BOOL    m_EnableOnChange;
    
    //Check flags and decide if onchange should be called
    BOOL DoOnChange(long NewValue, long OldValue, eOnChangeType OnChangeType);

    //Support for C-style SETTING
    BOOL OnChangeWrapForSettingStructure(long NewValue, long OldValue, eOnChangeType OnChangeType);

    static BOOL StaticOnChangeWrapForSettingStructure(void* pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING* pSetting);
};

/** Simple setting with a BOOL value
*/
class CYesNoSetting : public CSimpleSetting
{
public:
    CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry, 
                   CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0,
                   ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
    CYesNoSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
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
                   ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
    CSliderSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
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
                   ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
    CListSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ONCHANGE_ALL), long GUIinfo = 0, ONCHANGE_STATICFUNC* pfnOnChangeFunc = NULL, void* pThis = NULL);
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

class CSettingsHolder;


//Holder for group info (only name + description for now)
class CSettingGroup
{
public:
    CSettingGroup(LPCSTR szGroupName, LPCSTR szLongName, LPCSTR szInfoText = NULL, int Info = 0, CSettingsHolder* pObject = NULL);
    ~CSettingGroup();

    void SetLongName(LPCSTR szLongName);
    LPCSTR GetName();
    LPCSTR GetLongName();
    LPCSTR GetInfoText();

    CSettingsHolder* GetObject() { return m_pObject; }
    int   ObjectOnly() { return m_OnlyCurrentObject; }

protected:
    CSettingsHolder* m_pObject;
    int  m_OnlyCurrentObject;
    std::string m_sGroupName;
    std::string m_sDisplayName;
    std::string m_sInfoText;
};

class CSettingGroupList
{
protected:
    struct TSubGroupInfo
    {
        CSettingGroup* pGroup;
        vector<struct TSubGroupInfo> vSubGroups;
    };

    TSubGroupInfo m_GroupList;

    TSubGroupInfo* FindGroupRecursive(TSubGroupInfo* GroupList, CSettingGroup* pGroup);
    void DeleteGroupsRecursive(TSubGroupInfo* pGroupList);
public:    
    CSettingGroupList();
    ~CSettingGroupList();

    CSettingGroup* Get(CSettingsHolder* pObject, char** pszGroupList, char** pszDisplayNameList = NULL, char** pszTooltip = NULL);         
    CSettingGroup* GetGroup(CSettingsHolder* pObject, LPCSTR szGroupName, LPCSTR szDisplayName = NULL, LPCSTR szTooltip = NULL);
    CSettingGroup* GetSubGroup(CSettingGroup* pGroup, LPCSTR szSubGroup, LPCSTR szDisplayName = NULL, LPCSTR szTooltip = NULL);

    void Clear();

    CSettingGroup* Get(int* Index);
    int NumGroups(int* Index);  
};

/** Base class for any class that needs acesss to a list of ISetting settings.
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
    virtual void CreateSettings(LPCSTR IniSection) = 0;
    void ReadFromIni(int bInit = 1);
    void WriteToIni(BOOL bOptimizeFileAccess);

    void Add(ISetting* pSetting);
    void Remove(ISetting* pSetting);

    void DisableOnChange();
    void EnableOnChange();

    void LoadSettingStructures(SETTING* pSetting, int StartNum, int Num, CSettingGroup* pGroup = NULL);
    void LoadSettingStructuresEx(SETTINGEX* pSetting, int StartNum, int Num, CSettingGroupList* pGroupList = NULL);

    LONG HandleSettingsMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bHandled);
    
    virtual void OnReadFromIni(SETTINGHOLDERID HolderID, BOOL bBefore) {;};
    virtual void OnWriteToIni(SETTINGHOLDERID HolderID, BOOL bBefore) {;};
    
protected:
    vector<ISetting*> m_Settings;    
    long   m_SetMessage;

    void SettingsProcessChange(eOnChangeType OnChangeType);
};

/** Standalone settings holder
*/
class CSettingsHolderStandAlone : public CSettingsHolder
{
public:
    CSettingsHolderStandAlone();
    ~CSettingsHolderStandAlone();
    void CreateSettings(LPCSTR IniSection){;};

    ISetting* operator[](int i) { if ((i>=0) && (i<m_Settings.size())) { return m_Settings[i]; } return NULL; }
};



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


class CTreeSettingsGeneric;

class CSettingsMaster : public CEventObject
{
protected:
    typedef struct 
    {
        SETTINGHOLDERID HolderID;
        CSettingsHolder* pHolder;
        BOOL bIsSource;    
    } TSettingsHolderInfo;
    
    vector<TSettingsHolderInfo> m_Holders;

    CSettingGroupList* m_SettingGroupList;
    string m_sIniFile;
    
    string m_SourceName;
    string m_VideoInputName;
    string m_AudioInputName;
    string m_VideoFormatName;
    string m_ChannelName;

    virtual void OnEvent(CEventObject* pObject, eEventType Event, long OldValue, long NewValue, eEventType* ComingUp);
protected:        
    void SaveSettings(CEventObject* pObject, eOnChangeType OnChangeType);
    void LoadSettings(CEventObject* pObject, eOnChangeType OnChangeType);
    void ParseAllSettings(CEventObject* pObject, int What, eOnChangeType OnChangeType);
    void ModifyOneSetting(string sSubSection, ISetting* pSetting, int What, eOnChangeType OnChangeType);
    
    void ReadFlagsFromIni(CSettingsHolder* pHolder);
    void WriteFlagsToIni(CSettingsHolder* pHolder, BOOL bOptimizeFileAccess);
public:
    CSettingsMaster();
    ~CSettingsMaster();
    
    void IniFile(LPCSTR szIniFile) { m_sIniFile = szIniFile; }
    void Register(SETTINGHOLDERID szHolderID, CSettingsHolder* pHolder, BOOL bIsSource = false);
    void Unregister(SETTINGHOLDERID szHolderID);
    void Unregister(CSettingsHolder* pHolder);
    void ReadFlagsFromIni();
    void WriteFlagsToIni(BOOL bOptimizeFileAccess);

    CSettingGroupList* Groups();

    CTreeSettingsGeneric* GroupTreeSettings(CSettingGroup* pGroup);
    
    int GetNumSettings(SETTINGHOLDERID szHolderID);
    ISetting* GetSetting(SETTINGHOLDERID HolderID, int i);
    
public:    
    void EventCallback(eEventType EventType, long OldValue, long NewValue);
    //Static wrap
    static void EventCallback(void* pThis, eEventType EventType, long OldValue, long NewValue);
};

extern CSettingsMaster* SettingsMaster;


#endif

