#ifndef _ISETTING_H_
#define _ISETTING_H_

#include "DS_ApiCommon.h"
#include "DS_Control.h"


//See ISetting.cpp for a detailed description of the functions

/**
    Specify/get reason for an OnChange call
*/
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


/**
    Flags for setting

    The SETTINGFLAG_GLOBAL and SETTINGFLAG_PER_xx flags indicate
    that the setting is read/written from/to the .ini file for
    each different source/input/format/channel.

    The SETTINGFLAG_ALLOW_xx flags indicate which of those flags
    descriped above make sense. To limit the user.

    The SETTINGFLAG_ONCHANGE_xx flags indicate if the OnChange function
    should be called if the value was changed during a 'xx' change.
    The SETTINGFLAG_ONCHANGE_VALUECHANGED flag makes sure the OnChange
    function gets called if the new value if different from the old.

*/
enum eSettingFlags
{
    SETTINGFLAG_GLOBAL    =      0x0001,    
    SETTINGFLAG_PER_SOURCE =     0x0002,   //Read/Save setting on source change and call OnChange function
    SETTINGFLAG_PER_VIDEOINPUT = 0x0004,   //Read/Save setting on video input change and call OnChange function
    SETTINGFLAG_PER_AUDIOINPUT = 0x0008,
    SETTINGFLAG_PER_VIDEOFORMAT= 0x0010,
    SETTINGFLAG_PER_CHANNEL =    0x0020,

    SETTINGFLAG_PER_MASK =       0x003F,

    
    SETTINGFLAG_FLAGSTOINI_MASK= 0x003F,   //Which flags to read/write from/to the ini file

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


/** Extended static OnChange callback function
*/
typedef BOOL (__cdecl SETTINGEX_ONCHANGE)(void* pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING* pSetting);

/** Extension to SETTING
*/
typedef struct
{
    /// Total block size (sizeof(SETTINGEX))
    long  cbSize;        
    /// Default setting flags
    long  DefaultSettingFlags;
    /// Group list terminated with NULL. Or NULL for no group list
    char** pszGroupList;
    /// Extra information for correct representation the GUI
    long  GUIinfo;
    /// Pointer send as the first parameter of the pfnExOnChange function.
    void*  pExOnChangeThis;
    /// OnChange function. If NULL, pfnOnChange is tried
    SETTINGEX_ONCHANGE* pfnExOnChange;  
    /// Actual SettingFlags used in the program.
    long  SettingFlags;
    /// Internal. Last saved settings as a cache to reduce .ini file access
    long  LastSavedSettingFlags;
    /// Internal. Last read/write section in the .ini file
    char* szLastSavedValueIniSection;
} SETTINGEXPLUS;


/** Extended SETTING structure
    Includes additional information as Flags and Groups.
*/
typedef struct
{
    /// Display name of settings
    char* szDisplayName;
    /// Setting type (ONOFF, SLIDER, etc)
    SETTING_TYPE Type;
    /// Last saved value in .ini file
    long LastSavedValue;
    /// Pointer to value
    long* pValue;
    /// Default value
    long Default;
    /// Minimum value
    long MinValue;
    /// Maximum value
    long MaxValue;
    /// Step size
    long StepValue;
    /// Divider for OSD display
    long OSDDivider;
    /// List of items
    const char** pszList;
    /// Default .ini section
    char* szIniSection;
    /// Default ini entry
    char* szIniEntry;
    /// OnChange function.
    SETTING_ONCHANGE* pfnOnChange;
    /// Total block size (sizeof(SETTINGEX))
    long  cbSize;        
    /// Default setting flags
    long  DefaultSettingFlags;
    /// Group list terminated with NULL. Or NULL for no group list
    char** pszGroupList;
    /// Extra information for correct representation the GUI
    long  GUIinfo;
    /// Pointer send as the first parameter of the pfnExOnChange function.
    void*  pExOnChangeThis;
    /// OnChange function. If NULL, pfnOnChange is tried
    SETTINGEX_ONCHANGE* pfnExOnChange;  
    /// Actual SettingFlags used in the program.
    long  SettingFlags;
    /// Internal. Last saved settings as a cache to reduce .ini file access
    long  LastSavedSettingFlags;
    /// Internal. Last read/write section in the .ini file
    char* szLastSavedValueIniSection;
} SETTINGEX;

/** Setting group info
*/
typedef struct
{
    /// Size of Setting group. sizeof(SETTINGGROUP)
    long cbSize;
    /// Group name
    char* szGroup;
    /// Group name to show
    char* szDisplayName;
    /// Extra info/description
    char* szInfo;
} SETTINGGROUP;


class CSettingGroup;

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
    virtual void SetDefaultFlags(eSettingFlags SettingFlags, BOOL bSetFlagsToDefault) = 0;
    virtual void SetFlag(eSettingFlags SettingFlag, BOOL bEnable) = 0;
    virtual eSettingFlags GetFlags() = 0;
    virtual eSettingFlags GetDefaultFlags() = 0;
    virtual BOOL ReadFlagsFromIniSection(LPCSTR szSection, BOOL bSetDefaultOnFailure = TRUE) = 0;
    virtual void WriteFlagsToIniSection(LPCSTR szSection, BOOL bOptimizeFileAccess) = 0;
    virtual eSettingFlags GetLastSavedFlagsValue() = 0; 
    virtual void FlagsOnChange(eSettingFlags OldFlags, eSettingFlags Flags) = 0;
};

/**
    The groups can be part of a settingobject.
    Like a settingsholder.
*/    
class CSettingObject
{
    //At least one virtual function is required to store type information for dynamic casts
    virtual IamNowVirtual(){;};
};



class CSettingGroupList;
/**
    Setting group class.
    Contains info like name & description

    Optionally is part of a group tree list.
    Optionnaly belongs to an SettingObject.
*/
class CSettingGroup
{
public:
    CSettingGroup(CSettingGroupList *pGroupList, LPCSTR szGroupName, LPCSTR szLongName, LPCSTR szInfoText = NULL, CSettingObject* pObject = NULL);
    ~CSettingGroup();

    void SetLongName(LPCSTR szLongName);
    void SetInfoText(LPCSTR szInfoText);
    LPCSTR GetName();
    LPCSTR GetLongName();
    LPCSTR GetInfoText();

    CSettingObject* GetObject() { return m_pObject; }

    void AddSetting(ISetting *pSetting);
    void AddSetting(ISetting *pSetting, eSettingFlags Flags);
    
    CSettingGroup *GetGroup(LPCSTR szGroupName, LPCSTR szLongName = NULL, LPCSTR szInfoText = NULL);        
protected:
    /// Pointer to the list the group is in. NULL for no list.
    CSettingGroupList *m_pGroupList;
    /// Object the current group belongs too. Can be NULL
    CSettingObject* m_pObject;
    /// Name of the group
    std::string m_sGroupName;
    /// Display name of the group
    std::string m_sDisplayName;
    /// Info
    std::string m_sInfoText;      
};

/** 
    Setting group list
    Tree like structure with subgroups
*/
class CSettingGroupList
{
protected:
    struct TSubGroupInfo
    {
        CSettingGroup* pGroup;
        vector<struct TSubGroupInfo> vSubGroups;
    };
    /// Internal group list. Start of tree.
    TSubGroupInfo m_GroupList;

    TSubGroupInfo* FindGroupRecursive(TSubGroupInfo* GroupList, CSettingGroup* pGroup);
    TSubGroupInfo* FindGroupRecursive(TSubGroupInfo* pGroupList, CSettingObject* pObject, char** pszGroupNames);
    
    TSubGroupInfo* FindAndCreateRecursive(TSubGroupInfo *pGroupList, CSettingObject* pObject, char** pszGroupList, char** pszDisplayNameList, char** pszTooltip);
    void DeleteGroupsRecursive(TSubGroupInfo* pGroupList);
public:    
    CSettingGroupList();
    ~CSettingGroupList();

    CSettingGroup* GetGroup(CSettingObject* pObject, char** pszGroupList, char** pszDisplayNameList = NULL, char** pszTooltip = NULL);         
    CSettingGroup* GetRootGroup(CSettingObject* pObject, LPCSTR szGroupName, LPCSTR szDisplayName = NULL, LPCSTR szTooltip = NULL);
    CSettingGroup* GetSubGroup(CSettingGroup* pGroup, LPCSTR szSubGroup, LPCSTR szDisplayName = NULL, LPCSTR szTooltip = NULL);

    CSettingGroup* FindGroup(CSettingObject* pObject, char** pszGroupList);    
    CSettingGroup* Find(int* Index);
    int NumGroups(int* Index);  
    
    void Clear();
};



#endif
