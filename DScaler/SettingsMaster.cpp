#include "stdafx.h"

#include "Source.h"
#include "Providers.h"
#include "TreeSettingsGeneric.h"
#include "SettingsMaster.h"

/** Setup Settingsmaster so
    it can receive change events
*/
CSettingsMaster::CSettingsMaster() :
m_SettingGroupList(NULL)
{
}

CSettingsMaster::~CSettingsMaster()
{
	if (m_SettingGroupList != NULL)
	{
		delete m_SettingGroupList;
		m_SettingGroupList = NULL;
	}	
}

/// Get global group list
CSettingGroupList* CSettingsMaster::Groups()
{
    if (m_SettingGroupList == NULL)
    {
        m_SettingGroupList = new CSettingGroupList;
    }    
    return m_SettingGroupList;
}

/**
    Runs through all settings to see if they want to read/write their value
    on a change event
*/
void CSettingsMaster::ParseAllSettings(bool IsLoad)
{
    vector<string> vSubLocations;

    vSubLocations.push_back("");
    vSubLocations.push_back(m_SourceName);
    vSubLocations.push_back(m_VideoInputName);
    vSubLocations.push_back(m_AudioInputName);
    vSubLocations.push_back(m_VideoFormatName);
    vSubLocations.push_back(m_ChannelName);
    
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if ((m_Holders[i].bIsSource) && (m_Holders[i].pHolder!=NULL))  //only save/load setting of its own 
		{
            CSource* pSource;
            try 
            {
                pSource = dynamic_cast<CSource*>(m_Holders[i].pHolder);            
            } 
            catch (...)
            {
                pSource = NULL;
            }
            // Skip the holder if the source isn't the current one
			if ((pSource != NULL) && (pSource != Providers_GetCurrentSource()))
			{
				continue;
			}
		}
        
        m_Holders[i].pHolder->SetLocation(&vSubLocations);

		int Num = m_Holders[i].pHolder->GetNumSettings();
        CSimpleSetting* pSetting;
        string sSection;
        
		BOOL bAction = FALSE;

        for (int n = 0; n < Num; n++)
        {
            pSetting = (CSimpleSetting*)m_Holders[i].pHolder->GetSetting(n);
            if (pSetting != NULL)
            {
                if (IsLoad)
                {
                    m_Holders[i].pHolder->ReadSettingFromIni(pSetting);
                }
                else
                {
                    m_Holders[i].pHolder->WriteSettingToIni(pSetting, TRUE);
                }                    
            }
        }
    }
}


void CSettingsMaster::LoadSettings()
{
    ParseAllSettings(true);
}


void CSettingsMaster::SaveSettings()
{
    ParseAllSettings(false);
}

/**
    Register a settingholder to the list.
    The HolderID is for uniquely identifying a settingsholder from
    anywhere within the program so one can easily request any setting
    from anywhere. Setting values & Holder IDs are in DS_Control.h
*/
void CSettingsMaster::Register(CSettingsHolder* pHolder)
{
    if (pHolder == NULL)
    {
        return;
    }
       
    Unregister(pHolder);

    TSettingsHolderInfo shi;
    BOOL bIsSource = FALSE;

    shi.pHolder = pHolder;

    //Find out if the settingsholder is part of a source
    try
    {
        CSource* pSource;
        pSource = dynamic_cast<CSource*>(pHolder);
        if (pSource != NULL)
        {
            //Successfull cast. So holder is a source
            bIsSource = TRUE;
        }
    }
    catch (...)
    {
        bIsSource = FALSE;
    }
	shi.bIsSource = bIsSource;

    //Add to holder list
    m_Holders.push_back(shi);  
}

void CSettingsMaster::Unregister(CSettingsHolder* pHolder)
{
    vector<TSettingsHolderInfo> NewList;
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if (m_Holders[i].pHolder == pHolder)
        {
            //
        } 
        else 
        {
            NewList.push_back(m_Holders[i]);
        }
    }
    m_Holders = NewList;
}

void CSettingsMaster::SetSource(CSource* pSource)
{
    if (pSource != NULL)
    {
        m_SourceName = pSource->IDString();
        LoadSettings();
    }
    else
    {
        m_SourceName = "";
    }
}

void CSettingsMaster::SetChannelName(long NewValue)
{
    if (NewValue>=0)
    {        
        char szBuffer[33];
        m_ChannelName = string("Channel") + itoa(NewValue, szBuffer, 10);
        LoadSettings();
    }
    else
    {
        m_ChannelName = "";
    }
}

void CSettingsMaster::SetVideoInput(long NewValue)
{
    if (NewValue>=0)
    {
        char szBuffer[33];
        m_VideoInputName = string("VideoInput") +  itoa(NewValue, szBuffer, 10);
        LoadSettings();
    }
    else
    {
        m_VideoInputName = "";
    }
}

void CSettingsMaster::SetAudioInput(long NewValue)
{
    if (NewValue>=0)
    {
        char szBuffer[33];
        m_AudioInputName = string("AudioInput") +  itoa(NewValue, szBuffer, 10);
    }
    else
    {
        m_AudioInputName = "";
    }
}

void CSettingsMaster::SetVideoFormat(long NewValue)
{
    if (NewValue>=0)
    {
        char szBuffer[33];
        m_VideoFormatName = string("VideoFormat") +  itoa(NewValue, szBuffer, 10);            
        LoadSettings();
    }
    else
    {
        m_VideoFormatName = "";
    }
}

/**
    Construct a list of settings in group 'pGroup'
*/
CTreeSettingsGeneric* CSettingsMaster::GroupTreeSettings(CSettingGroup* pGroup)
{
	vector<CSimpleSetting*> SettingList;

	for (int i = 0; i < m_Holders.size(); i++)
    {
        if ((m_Holders[i].bIsSource) && (m_Holders[i].pHolder!=NULL))  //only save/load setting of its own 
		{
            CSource* pSource;
            try 
            {
                pSource = dynamic_cast<CSource*>(m_Holders[i].pHolder);            
            } 
            catch (...)
            {
                pSource = NULL;
            }
			if ((pSource != NULL) && (pSource != Providers_GetCurrentSource()))
			{
				continue;
			}
		}
		
		int Num = m_Holders[i].pHolder->GetNumSettings();
        CSimpleSetting* pSetting;
        
        for (int n = 0; n < Num; n++)
        {
            pSetting = (CSimpleSetting*)m_Holders[i].pHolder->GetSetting(n);
            if ((pSetting != NULL) && (pSetting->GetGroup() == pGroup))
            {
				SettingList.push_back(pSetting);
			}
		}
	}
	if (SettingList.size() == 0)
	{
		return NULL;
	}
	else
	{
		char* szName = (char*)pGroup->GetLongName();
		if ((szName == NULL) || (szName[0]==0)) 
		{ 
			szName = (char*)pGroup->GetName(); 
		}	
        return new CTreeSettingsGeneric(szName,SettingList);
	}
}
