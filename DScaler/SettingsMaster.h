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
class CSettingsMaster
{
protected:
    typedef struct 
    {
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

protected:        
    void ParseAllSettings(bool IsLoad);
public:
    CSettingsMaster();
    ~CSettingsMaster();
    
    void IniFile(LPCSTR szIniFile) { m_sIniFile = szIniFile; }
    void Register(CSettingsHolder* pHolder);
    void Unregister(CSettingsHolder* pHolder);        

    void GetSubLocations(CSource *Source, int VideoInput, int AudioInput, eVideoFormat VideoFormat, int Channel, vector<string> *vSubLocations);

    CSettingGroupList* Groups();

    CTreeSettingsGeneric* GroupTreeSettings(CSettingGroup* pGroup);
    
    void SetSource(CSource* pSource);
    void SetChannelName(long NewValue);
    void SetVideoInput(long NewValue);
    void SetAudioInput(long NewValue);
    void SetVideoFormat(long NewValue);

    void SaveSettings();
    void LoadSettings();

    
public:    
};

extern CSettingsMaster* SettingsMaster;

#endif
