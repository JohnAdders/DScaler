#ifndef _SETTINGSMASTER_H_
#define _SETTINGSMASTER_H_

#include "Setting.h"
#include "SettingHolder.h"
#include "Events.h"

class CTreeSettingsGeneric;


/** SettingsMaster
    Maintains a list of all SettingsHolders
    and reads/writes setting from/to the .ini file
    on a source/input/format/channel change event
    (if the proper setting flags are set).

    The SettingsMaster also holds one big group list
    for a structured overview of all settings.

    Used by CSettingsHolder.

    There is one global instance of this class created in DScaler.cpp
*/
class CSettingsMaster : public CEventObject
{
protected:
    typedef struct 
    {
        SETTINGHOLDERID HolderID;
        CSettingsHolder* pHolder;
        BOOL bIsSource;    
    } TSettingsHolderInfo;
    
    /// List of setting holders
    vector<TSettingsHolderInfo> m_Holders;

    /// The global group list
    CSettingGroupList* m_SettingGroupList;
    /// Name of the ini file
    string m_sIniFile;
    
    /// Names of the current source, inputs, video format and channel
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
public:
    CSettingsMaster();
    ~CSettingsMaster();
    
    void IniFile(LPCSTR szIniFile) { m_sIniFile = szIniFile; }
    void Register(SETTINGHOLDERID szHolderID, CSettingsHolder* pHolder);
    void Unregister(SETTINGHOLDERID szHolderID);
    void Unregister(CSettingsHolder* pHolder);        

    void GetSubLocations(CSource *Source, int VideoInput, int AudioInput, eVideoFormat VideoFormat, int Channel, vector<string> *vSubLocations);

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
