#include "stdafx.h"

#include "SettingHolder.h"
#include "SettingsMaster.h"

CSettingsHolder::CSettingsHolder(long SetMessage) :
    m_SetMessage(SetMessage)
{    
    m_SettingFlagsSection = "SettingFlags";
        
    m_pRegistered = FALSE;

    RegisterMe();        
}

CSettingsHolder::~CSettingsHolder()
{
    if ((SettingsMaster != NULL) && m_pRegistered)
    {
        SettingsMaster->Unregister(this);
        m_pRegistered = FALSE;
    }
    // Delete settings and write settings
    ClearSettingList(TRUE, TRUE);
}


void CSettingsHolder::ClearSettingList(BOOL bDeleteSettings, BOOL bWriteSettings)
{
    for (int i = 0; i < m_Settings.size(); i++)
    {
        if (m_Settings[i] != NULL)
        {
            if (bWriteSettings)
            {
                m_Settings[i]->WriteToIni(TRUE);
            }
            if (bDeleteSettings)
            {
                delete m_Settings[i];
            }
            m_Settings[i] = NULL;
        }
    }    
    m_Settings.clear();
}

/**
     Registers the SettingsHolder to the SettingsMaster.
     The SettingsMaster receives source/input/format & channel
     change events and read&writes the proper settings from/to
     the SettingsHolders.
*/
BOOL CSettingsHolder::RegisterMe()
{
    if (!m_pRegistered && (SettingsMaster != NULL))
    {
        m_pRegistered = TRUE;
        SettingsMaster->Register(this);        
    }
    return m_pRegistered;
}

/**
    Add a settings to the list.
    Makes sure there are no duplicates.
*/
void CSettingsHolder::AddSetting(ISetting* pSetting)
{    
    for (int i = 0; i < m_Settings.size(); i++)
    {
        if (m_Settings[i] == pSetting)
        {
            //Already in list
            return; 
        }
    }
    //Add to list
    m_Settings.push_back(pSetting);
    //
    RegisterMe();
}

void CSettingsHolder::AddSetting(SETTING* pSetting, CSettingGroup* pGroup)
{
    ISetting* pISetting = NULL;

    switch (pSetting->Type)
    {
    case ONOFF:
    case YESNO:
        pISetting = new CYesNoSetting(pSetting, pGroup);
        break;
    case ITEMFROMLIST:
        pISetting = new CListSetting(pSetting, pGroup);
        break;
    case SLIDER:
        pISetting = new CSliderSetting(pSetting, pGroup);
        break;
    }

    if (pISetting != NULL)
    {
        AddSetting(pISetting);
    }
}

/** 
    Removes a setting from the list.
*/
void CSettingsHolder::RemoveSetting(ISetting* pSetting)
{
    vector<ISetting*> NewList;
    for (int i = 0; i < m_Settings.size(); i++)
    {
        if ((m_Settings[i] == NULL) || (m_Settings[i] == pSetting))
        {
            //Don't add
        }
        else
        {
            NewList.push_back(m_Settings[i]);
        }
    }
    m_Settings = NewList;
}

/**
    Sets the sublocations for the SETTINGFLAG_PER_xxx flags.
    Used by SettingsMaster to set the right location
    based on the current state of the source, inputs, format
    and channel.
*/
void CSettingsHolder::SetLocation(vector<string>* pvSubLocations)
{
    m_NewSubLocations = TRUE;
    
    if (pvSubLocations == NULL)
    {
        m_SubLocations.clear();
    }
    else
    {
        m_SubLocations = *pvSubLocations;
    }
}

/**
    Concatenates the sublocations of the settingflags 
    that are enabled in pSetting.
*/
string CSettingsHolder::GetLocation(ISetting *pSetting)
{
    if (pSetting == NULL)
    {
        return "";
    }
    if (m_NewSubLocations)
    {
        m_NewSubLocations = FALSE;
        m_Location = "";

        for (int i = 0; i < m_SubLocations.size(); i++)
        {
            m_Location += m_SubLocations[i]+"_";
        }
        int Len = m_Location.length();        
        if ((Len>0) && (m_Location[Len-1] == '_')) 
        { 
            m_Location = m_Location.substr(0,Len-1); 
        }
    }
    return m_Location;
}

/**
    Reads a setting value & flag from the .ini file.
    The location in the .ini file depends on the sublocation and enabled flags.
*/    
void CSettingsHolder::ReadSettingFromIni(ISetting *pSetting)
{
    string sLocation = GetLocation(pSetting);
        
    // Read from ini file.
    if (sLocation.length() == 0)
    {
        pSetting->ReadFromIni(TRUE);
    }
    else
    {
        pSetting->ReadFromIniSubSection(sLocation.c_str(), TRUE);
    }
}

/**
    Writes a setting value & flag to the .ini file.
    The location in the .ini file depends on the sublocation and enabled flags.
*/    
void CSettingsHolder::WriteSettingToIni(ISetting *pSetting, BOOL bOptimizeFileAccess)
{
    string sLocation = GetLocation(pSetting);
        
    if (sLocation.length() == 0)
    {
        pSetting->WriteToIni(bOptimizeFileAccess);
    }
    else
    {
         pSetting->WriteToIniSubSection(sLocation.c_str());
    }
}

/**
    Reads alls settings of the list from the .ini file.
*/    
void CSettingsHolder::ReadFromIni()
{    
    RegisterMe();
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        ReadSettingFromIni((*it));
    }
}

/**
    Writes alls settings of the list to the .ini file.    
*/    
void CSettingsHolder::WriteToIni(BOOL bOptimizeFileAccess)
{    
    RegisterMe();
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        WriteSettingToIni((*it), bOptimizeFileAccess);
    }
}

/**
    Number of settings in the list.    
*/    
long CSettingsHolder::GetNumSettings()
{
    return m_Settings.size();
}


/**
    Get setting at index SettingsIndex.    
*/    
ISetting* CSettingsHolder::GetSetting(long SettingIndex)
{
    RegisterMe();
    if(SettingIndex >= 0 && SettingIndex < m_Settings.size())
    {
        return m_Settings[SettingIndex];
    }
    else
    {
        return NULL;
    }
}

/**
    Handles WM_xx_SETVALUE/WM_xx_GETVALUE/WM_xx_CHANGEVALUE
    window messages.
    Only if the message offset specified at initialization is not 0.
*/    
LONG CSettingsHolder::HandleSettingsMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bHandled)
{
    LONG RetVal = 0;
    if (m_SetMessage == 0) { return 0; }
    if(wParam >= 0 && wParam < m_Settings.size())
    {
        if(message == m_SetMessage)
        {
            RetVal = m_Settings[wParam]->GetValue();
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 100)
        {
            m_Settings[wParam]->SetValue(lParam);
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 200)
        {
            m_Settings[wParam]->ChangeValue((eCHANGEVALUE)lParam);
            *bHandled = TRUE;
        }
    }
    return RetVal;
}

/**
    Adds an array of SETTING structures to the SettingHolder.
    Optionally specify a default SettingGroup    
*/
int CSettingsHolder::LoadSettingStructures(SETTING* pSetting, int StartNum, int Num, CSettingGroup* pGroup)
{
    int Count(0);

    m_Settings.clear();

    for (int i(StartNum) ; i < Num; i++)
    {
        if(pSetting[i].szDisplayName != NULL)
        {
            AddSetting(&pSetting[i], pGroup);
            ++Count;
        }
    }    
    RegisterMe();
    return Count;
}

/**
    Enable all OnChange calls
*/
void CSettingsHolder::EnableOnChange()
{
    for (int i = 0; i < m_Settings.size(); i++)
    {
        if (m_Settings[i] != NULL)
        {
            m_Settings[i]->EnableOnChange();
        }
    }
}

/**
    Disable all OnChange calls if a value of a setting is changed.   
*/
void CSettingsHolder::DisableOnChange()
{
    for (int i = 0; i < m_Settings.size(); i++)
    {
        if (m_Settings[i] != NULL)
        {
            m_Settings[i]->DisableOnChange();
        }
    }
}

/**
    Get/Create a root group from the settingsmaster.
    The SettingObject the group belongs to is the SettingHolder itself.
*/
CSettingGroup* CSettingsHolder::GetSettingsGroup(LPCSTR szName, DWORD Flags, BOOL IsActiveByDefault)
{
    return GetSettingsGroup(this, szName, Flags, IsActiveByDefault);
}

/**
    Get/Create a root group from the settingsmaster.  
*/
CSettingGroup* CSettingsHolder::GetSettingsGroup(CSettingObject *pObject, LPCSTR szName, DWORD Flags, BOOL IsActiveByDefault)
{
    RegisterMe();
    return SettingsMaster->GetGroup(pObject, szName, Flags, IsActiveByDefault);
}

/** Read settings from the right ini section.
    It takes care of dependencies.
    (Like videoformat dependent of video input)

    ///\todo: finish
*/
void CSettingsHolder::ReadFromIni(CSource *pSource, ISetting *pSettingVideoInput, ISetting *pSettingAudioInput, ISetting *pVideoFormat, ISetting *pChannel,
                     int &VideoInput, int &AudioInput, eVideoFormat &VideoFormat, int &Channel)
{    
    RegisterMe();
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        ReadSettingFromIni((*it));    
    }
}


/**
    A standalone SettingsHolder

    
*/
CSettingsHolderStandAlone::CSettingsHolderStandAlone() : CSettingsHolder(0)
{
}

CSettingsHolderStandAlone::~CSettingsHolderStandAlone()
{
}
