#ifndef _ISETTING_H_
#define _ISETTING_H_

#include "DS_ApiCommon.h"
#include "DS_Control.h"


//See ISetting.cpp for a detailed description of the functions

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
    virtual void SetDefault() = 0;
    virtual SETTING_TYPE GetType() = 0;
    virtual void ChangeValue(eCHANGEVALUE NewValue) = 0;
    virtual BOOL ReadFromIni(BOOL bSetDefaultOnFailure = TRUE) = 0;
    virtual void WriteToIni(BOOL bOptimizeFileAccess) = 0;
    virtual BOOL ReadFromIniSubSection(LPCSTR szSubSection, BOOL bSetDefaultOnFailure = TRUE) = 0;
    virtual void WriteToIniSubSection(LPCSTR szSubSection) = 0;  
    virtual long GetValue() = 0;
    virtual long GetMin() = 0;
    virtual long GetMax() = 0;
    virtual long GetDefault() = 0;
    virtual void ChangeDefault(long NewDefault, BOOL bDontSetValue = FALSE) = 0;
    virtual void SetValue(long NewValue, BOOL bSupressOnChange = FALSE) = 0;
    virtual void OnChange(long NewValue, long OldValue) {;};
    virtual void DisableOnChange() = 0;
    virtual void EnableOnChange() = 0;
    virtual void OSDShow() = 0;
    virtual const char** GetList() {return NULL;};
    virtual void SetupControl(HWND hWnd) = 0;
    virtual void SetControlValue(HWND hWnd) = 0;
    virtual void SetFromControl(HWND hWnd) = 0;
    virtual void SetGroup(CSettingGroup* pGroup) = 0; 
    virtual CSettingGroup* GetGroup() = 0;
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
