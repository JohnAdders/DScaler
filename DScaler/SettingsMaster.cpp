#include "stdafx.h"

#include "Source.h"
#include "Providers.h"
#include "TreeSettingsGeneric.h"
#include "SettingsMaster.h"

CSettingsMaster::CSettingsMaster() 
{
}

CSettingsMaster::~CSettingsMaster()
{
    for (int i = 0; i < m_SettingsGroups.size(); i++)
    {
        delete m_SettingsGroups[i];
    }
    m_SettingsGroups.clear();
}

/**
    Runs through all settings to see if they want to read/write their values
    we only save or load those setting that are being held by channel/format etc
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
        
        //m_Holders[i].pHolder->SetLocation(&vSubLocations);

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
    }
    else
    {
        m_VideoFormatName = "";
    }
}

CSettingGroup* CSettingsMaster::GetGroup(CSettingObject *pObject, LPCSTR szName, DWORD Flags, BOOL IsActiveByDefault)
{
    for (int i = 0; i < m_SettingsGroups.size(); i++)
    {
        if(strcmp(m_SettingsGroups[i]->GetName(), szName) == 0)
        {
            return m_SettingsGroups[i];
        }
    }
    CSettingGroup* pNewGroup = new CSettingGroup(szName, Flags, IsActiveByDefault);
    m_SettingsGroups.push_back(pNewGroup);
    return pNewGroup;
}
