#ifndef _SETTINGHOLDER_H_
#define _SETTINGHOLDER_H_

#include "Setting.h"
#include "TVFormats.h"
class CSource;

/** Base class for any class that needs acesss to a list of ISetting settings.
    The function CreateSettings should be overriden and then called in the 
    constructor of the derived class.
*/
class CSettingsHolder : public CSettingObject
{
public:
    CSettingsHolder(long SetMessage, SETTINGHOLDERID SettingHolderID = (SETTINGHOLDERID)0);
    ~CSettingsHolder();
    long GetNumSettings();
    ISetting* GetSetting(long SettingIndex);
    virtual void CreateSettings(LPCSTR IniSection) = 0;
    void ReadFromIni(int bInit = 1);    
    void WriteToIni(BOOL bOptimizeFileAccess);

    void ReadSettingFromIni(ISetting *pSetting, eOnChangeType OnChangeType = ONCHANGE_NONE);
    void WriteSettingToIni(ISetting *pSetting, BOOL bOptimizeFileAccess, eOnChangeType OnChangeType = ONCHANGE_NONE);    

    void ReadFromIni(CSource *pSource, ISetting *pSettingVideoInput, ISetting *pSettingAudioInput, ISetting *pVideoFormat, ISetting *pChannel,
                     int &VideoInput, int &AudioInput, eVideoFormat &VideoFormat, int &Channel, eOnChangeType OnChangeType = ONCHANGE_NONE);

    void AddSetting(ISetting* pSetting);
    void RemoveSetting(ISetting* pSetting);

    void AddSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);
    void AddSetting(SETTINGEX* pSetting, CSettingGroupList* pGroupList = NULL);

    void DisableOnChange();
    void EnableOnChange();

    void ClearSettingList(BOOL bDeleteSettings = TRUE, BOOL bWriteSettings = FALSE);

    int LoadSettingStructures(SETTING* pSetting, int StartNum, int Num, CSettingGroup* pGroup = NULL);
    int LoadSettingStructuresEx(SETTINGEX* pSetting, int StartNum, int Num, CSettingGroupList* pGroupList = NULL);    

    LONG HandleSettingsMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bHandled);
    
    CSettingGroup *GetSettingsGroup(CSettingObject *pObject, LPCSTR szGroupName, LPCSTR szDisplayName = NULL, LPCSTR szTooltip = NULL);
    CSettingGroup *GetSettingsGroup(LPCSTR szGroupName, LPCSTR szDisplayName = NULL, LPCSTR szTooltip = NULL);
    CSettingGroup *FindGroup(char **pszGroupList);
    SETTINGHOLDERID GetID() { return m_SettingHolderID; }
    void SetLocation(vector<string>* pvSubLocations);
    string GetLocation(ISetting *pSetting);
    
    static ISetting* GetSetting(SETTINGHOLDERID HolderID, long SettingIndex);
protected:
    vector<ISetting*>   m_Settings;    
    long                m_SetMessage;
    SETTINGHOLDERID     m_SettingHolderID;
    BOOL                m_pRegistered;
    string              m_SettingFlagsSection;
    vector<string>      m_SubLocations;    
    string              m_Location;
    BOOL                m_NewSubLocations;
    long                m_LastLocationFlag;

    BOOL   RegisterMe();
};

/** Standalone settings holder
*/
class CSettingsHolderStandAlone : public CSettingsHolder
{
public:
    CSettingsHolderStandAlone(SETTINGHOLDERID HolderID);
    ~CSettingsHolderStandAlone();
    void CreateSettings(LPCSTR IniSection){;};

    ISetting* operator[](int i) { if ((i>=0) && (i<m_Settings.size())) { return m_Settings[i]; } return NULL; }
};
#endif

