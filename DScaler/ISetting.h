#ifndef _ISETTING_H_
#define _ISETTING_H_

#include "DS_ApiCommon.h"
#include "DS_Control.h"


//See ISetting.cpp for a detailed description of the functions

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

class CSimpleSetting;
class CYesNoSetting;

enum eGroupSettingsFlags
{
    SETTING_BY_CHANNEL = 1,
    SETTING_BY_FORMAT = 2,
    SETTING_BY_INPUT = 4,
    SETTING_BY_AUDIOINPUT = 8,
};


/**
    Setting group class.
    Contains info like name & description

    Optionally is part of a group tree list.
    Optionnaly belongs to an SettingObject.
*/
class CSettingGroup
{
public:

    CSettingGroup(LPCSTR szName, DWORD Flags = 0, BOOL IsActiveByDefault = FALSE);
    ~CSettingGroup();

    LPCSTR GetName();

    BOOL IsSetByChannel() { return ((m_Flags & SETTING_BY_CHANNEL) == SETTING_BY_CHANNEL);};
    BOOL IsSetByFormat() { return ((m_Flags & SETTING_BY_FORMAT) == SETTING_BY_FORMAT);};
    BOOL IsSetByInput() { return ((m_Flags & SETTING_BY_INPUT) == SETTING_BY_INPUT);};
    BOOL IsSetByAudioInput() { return ((m_Flags & SETTING_BY_AUDIOINPUT) == SETTING_BY_AUDIOINPUT);};

    BOOL IsGroupActive();

    void AddSetting(ISetting *pSetting);
    CSimpleSetting* GetIsActiveSetting();

protected:
    /// Name of the group
    std::string m_Name;
    DWORD m_Flags;

    CYesNoSetting* m_IsActive;
};

#endif
