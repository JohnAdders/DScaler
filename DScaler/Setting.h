/////////////////////////////////////////////////////////////////////////////
// $Id: Setting.h,v 1.11 2002-09-26 06:09:48 kooiman Exp $
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

#include "Events.h"
#include "Settings.h"
#include "TVFormats.h"

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
    SETTINGFLAG_NO_WRITETOINI = 0x0001,
    SETTINGFLAG_NO_READFROMINI = 0x0002,    

    SETTINGFLAG_ONCHANGE_VALUECHANGED = 0x0004, //Only call onchange if the value changed
    
    SETTINGFLAG_GLOBAL    =      0x0008,    
    SETTINGFLAG_PER_SOURCE =     0x0010,   //Read/Save setting on source change and call OnChange function
    SETTINGFLAG_PER_VIDEOINPUT = 0x0020,   //Read/Save setting on video input change and call OnChange function
    SETTINGFLAG_PER_AUDIOINPUT = 0x0040,
    SETTINGFLAG_PER_VIDEOFORMAT= 0x0080,
    SETTINGFLAG_PER_CHANNEL =    0x0100,

    SETTINGFLAG_ALLOW_GLOBAL    =      0x00001000,    
    SETTINGFLAG_ALLOW_PER_SOURCE =     0x00002000,   //Read/Save setting on source change and call OnChange function
    SETTINGFLAG_ALLOW_PER_VIDEOINPUT = 0x00004000,   //Read/Save setting on video input change and call OnChange function
    SETTINGFLAG_ALLOW_PER_AUDIOINPUT = 0x00008000,
    SETTINGFLAG_ALLOW_PER_VIDEOFORMAT= 0x00010000,
    SETTINGFLAG_ALLOW_PER_CHANNEL =    0x00020000,

    SETTINGFLAG_ALLOW_ALL =            0x000FF000,    

    SETTINGFLAG_ONCHANGE_INIT =       0x00100000, //Allow onchange call on initialization
    SETTINGFLAG_ONCHANGE_SET =        0x00200000, //usualy set        
    SETTINGFLAG_ONCHANGE_READ =       0x00400000,
    SETTINGFLAG_ONCHANGE_SOURCE =     0x00800000,
    SETTINGFLAG_ONCHANGE_VIDEOINPUT=  0x01000000,
    SETTINGFLAG_ONCHANGE_AUDIOINPUT = 0x02000000,
    SETTINGFLAG_ONCHANGE_VIDEOFORMAT= 0x04000000,
    SETTINGFLAG_ONCHANGE_CHANNEL =    0x08000000,
    
    SETTINGFLAG_ONCHANGE_ALL =        0x0FF00000,
};

typedef BOOL (__cdecl SETTINGEX_ONCHANGE)(void *pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING *pSetting);

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
    SETTINGEX_ONCHANGE* pfnOnChange;    
    char **pszGroupList;    
    long  SettingFlags;    
} SETTINGEX;

typedef struct
{
    char* szGroup;
    char* szDisplayName;
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
    virtual long GetValue() = 0;
    virtual long GetMin() = 0;
    virtual long GetMax() = 0;
    virtual long GetDefault() = 0;
    virtual void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE, eOnChangeType OnChangeType = ONCHANGE_NONE) = 0;
    virtual void SetValue(long NewValue, eOnChangeType OnChangeType = ONCHANGE_SET) = 0;
    virtual void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType) {;};
    virtual void OSDShow() = 0;
    virtual const char** GetList() {return NULL;};
    virtual void SetupControl(HWND hWnd) = 0;
    virtual void SetControlValue(HWND hWnd) = 0;
    virtual void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET) = 0;
    virtual void SetGroup(CSettingGroup* pGroup) = 0; 
    virtual CSettingGroup* GetGroup() = 0;
    virtual void SetFlags(eSettingFlags SettingsFlag) = 0;
    virtual eSettingFlags GetFlags()= 0;
};

// To interface with onchange functions for cases when 
//   no derived class is possible
typedef BOOL (__cdecl ONCHANGE_STATICFUNC)(void *pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING *pSetting);

/** Base class for settings that can be represented as a long
*/
class CSimpleSetting : public ISetting
{
public:
    CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, long StepValue, 
                   CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), 
                   ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CSimpleSetting(SETTING *pSetting, CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CSimpleSetting(SETTINGEX *pSetting, CSettingGroup *pGroup);
    CSimpleSetting(SETTINGEX *pSetting, CSettingGroupList *pList);
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
    eSettingFlags GetFlags();
    void SetSection(LPCSTR NewValue);
    LPCSTR GetSection();
    void SetEntry(LPCSTR NewValue);
    LPCSTR GetEntry();
    LPCSTR GetLastValueSection();
    BOOL ReadFromIni(BOOL bSetDefaultOnFailure = TRUE, eOnChangeType OnChangeType = ONCHANGE_NONE);
    void WriteToIni(BOOL bOptimizeFileAccess);
    
    void SetGroup(CSettingGroup* pGroup); 
    CSettingGroup* GetGroup();

    SETTING *GetSETTING() { return m_pSetting; }
    
    virtual void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType);   

    virtual void GetDisplayText(LPSTR szBuffer) = 0;    
protected:    
    std::string  m_DisplayName;
    std::string  m_Section;
    std::string  m_Entry;
    std::string  m_SectionFromLastValue;
    long m_StoreValue;
    BOOL m_bFreeSettingOnExit;

    SETTING *m_pSetting;
    CSettingGroup *m_pGroup;
    eSettingFlags m_SettingFlags;
    eSettingFlags m_DefaultFlags;

    //Check flags and decide if onchange should be called
    BOOL DoOnChange(long NewValue, long OldValue, eOnChangeType OnChangeType);

    //Support for non-derived class onchange function
    ONCHANGE_STATICFUNC *m_pfnOnChangeStatic;
    void *m_pfnOnChangeStatic_pThis;

    //Support for C-style SETTING
    SETTING_ONCHANGE    *m_pfnOnChangeSettingStructure;
    long                *m_pValueSettingStructure;
    BOOL OnChangeWrapForSettingStructure(long NewValue, long OldValue, eOnChangeType OnChangeType);

    static BOOL StaticOnChangeWrapForSettingStructure(void *pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING *pSetting);
};

/** Simple setting with a BOOL value
*/
class CYesNoSetting : public CSimpleSetting
{
public:
    CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry, 
                   CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), 
                   ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CYesNoSetting(SETTING *pSetting, CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CYesNoSetting(SETTINGEX *pSetting, CSettingGroup *pGroup);
    CYesNoSetting(SETTINGEX *pSetting, CSettingGroupList *pList);

    ~CYesNoSetting();
    SETTING_TYPE GetType() {return YESNO;};
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET);

    void FillSettingStructure(SETTING *pSetting);
    void FillSettingStructureEx(SETTINGEX *pSetting);    
};

/** Simple setting with a long value represenmted by a slider
*/
class CSliderSetting : public CSimpleSetting
{
public:    
    CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, 
                   CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), 
                   ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CSliderSetting(SETTING *pSetting, CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CSliderSetting(SETTINGEX *pSetting, CSettingGroup *pGroup);
    CSliderSetting(SETTINGEX *pSetting, CSettingGroupList *pList);
    
    ~CSliderSetting();
    SETTING_TYPE GetType() {return SLIDER;};
    void SetOSDDivider(long OSDDivider);
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET);

    void FillSettingStructure(SETTING *pSetting);
    void FillSettingStructureEx(SETTINGEX *pSetting);    
private:
    //long m_OSDDivider;
};

/** Simple setting with a selection from a list
*/
class CListSetting : public CSimpleSetting
{
public:
    CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList, 
                   CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), 
                   ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CListSetting(SETTING *pSetting, CSettingGroup *pGroup = NULL, eSettingFlags SettingFlags = (eSettingFlags)(SETTINGFLAG_ONCHANGE_VALUECHANGED|SETTINGFLAG_GLOBAL|SETTINGFLAG_ALLOW_ALL|SETTINGFLAG_ONCHANGE_ALL), ONCHANGE_STATICFUNC *pfnOnChangeFunc = NULL, void *pThis = NULL);
    CListSetting(SETTINGEX *pSetting, CSettingGroup *pGroup);
    CListSetting(SETTINGEX *pSetting, CSettingGroupList *pList);

    ~CListSetting();
    SETTING_TYPE GetType() {return ITEMFROMLIST;};
    const char** GetList() {return m_pSetting->pszList;}
    void GetDisplayText(LPSTR szBuffer);
    void SetupControl(HWND hWnd);
    void SetControlValue(HWND hWnd);
    void SetFromControl(HWND hWnd, eOnChangeType OnChangeType = ONCHANGE_SET);

    void FillSettingStructure(SETTING *pSetting);
    void FillSettingStructureEx(SETTINGEX *pSetting);    
private:
//    const char** m_List;
};


//Holder for group info (only name + description for now)
class CSettingGroup
{
public:
    CSettingGroup(LPCSTR szGroupName, LPCSTR szLongName);
    ~CSettingGroup();

    void SetLongName(LPCSTR szLongName);
    LPCSTR GetName();
    LPCSTR GetLongName();

protected:
    //SETTINGGROUP *m_pSettingGroup;
    std::string m_sGroupName;
    std::string m_sDisplayName;
};

class CSettingGroupList
{
protected:
    vector<CSettingGroup*> m_GroupList;
public:    
    CSettingGroupList();
    ~CSettingGroupList();

    CSettingGroup *Get(char **pszGroupList);
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

    void Add(ISetting *pSetting);
    void Remove(ISetting *pSetting);

    void LoadSettingStructures(SETTING *pSetting, int StartNum, int Num, CSettingGroup *pGroup = NULL);
    void LoadSettingStructuresEx(SETTINGEX *pSetting, int StartNum, int Num, CSettingGroupList *pGroupList = NULL);

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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, BOOL Default, LPCSTR Section, CSettingGroup *pGroup = NULL) : CYesNoSetting(DisplayName, Default, Section, #Name, pGroup), m_Parent((Class*)Parent) {;} \
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, CSettingGroup *pGroup = NULL) : \
             CSliderSetting(DisplayName, Default, Min, Max, Section, #Name, pGroup), m_Parent((Class*)Parent) {;} \
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
        C ## Name ## Setting(const Class* Parent, LPCSTR DisplayName, long Default, long Max, LPCSTR Section, const char** pszList, CSettingGroup *pGroup = NULL) : \
             CListSetting(DisplayName, Default, Max, Section, #Name, pszList, pGroup), m_Parent((Class*)Parent) {;} \
        void OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType) {m_Parent->Name ## OnChange(NewValue, OldValue);} \
    private: \
        Class* m_Parent; \
    }; \
    C ## Name ## Setting* m_## Name;\
    public: \
    void Name ## OnChange(long NewValue, long OldValue);



class CSettingsMaster : public CEventObject
{
protected:
    typedef struct 
    {
        SETTINGHOLDERID HolderID;
        CSettingsHolder *pHolder;
    } TSettingsHolderInfo;
    
    vector<TSettingsHolderInfo> m_Holders;

    CSettingGroupList *m_SettingGroupList;
    string m_sIniFile;
    
    string m_SourceName;
    string m_VideoInputName;
    string m_AudioInputName;
    string m_VideoFormatName;
    string m_ChannelName;

    virtual void OnEvent(eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
protected:        
    void SaveSettings(eOnChangeType OnChangeType);
    void LoadSettings(eOnChangeType OnChangeType);
    void ParseAllSettings(int What, eOnChangeType OnChangeType);
    void ModifyOneSetting(ISetting *pSetting, int What, eOnChangeType OnChangeType);
public:
    CSettingsMaster(CEventCollector *pEventCollector);
    ~CSettingsMaster();
    
    void IniFile(LPCSTR szIniFile) { m_sIniFile = szIniFile; }
    void Register(SETTINGHOLDERID szHolderID, CSettingsHolder *pHolder);
    void Unregister(SETTINGHOLDERID szHolderID);
    void Unregister(CSettingsHolder *pHolder);
    void SetupEventCallbacks();
    void ReadFromIni(BOOL bInit = FALSE);
    void WriteToIni(BOOL bOptimizeFileAccess);

    CSettingGroupList* Groups();
    
    int GetNumSettings(SETTINGHOLDERID szHolderID);
    ISetting *GetSetting(SETTINGHOLDERID HolderID, int i);
    
public:    
    void EventCallback(eEventType EventType, long OldValue, long NewValue);
    //Static wrap
    static void EventCallback(void *pThis, eEventType EventType, long OldValue, long NewValue);
};

extern CSettingsMaster *SettingsMaster;


#endif

