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
    eEventType EventList[] = {
        EVENT_INIT,
        EVENT_DESTROY,
        EVENT_SOURCE_PRECHANGE,
        EVENT_SOURCE_CHANGE,
        EVENT_VIDEOINPUT_PRECHANGE,
        EVENT_VIDEOINPUT_CHANGE,
        EVENT_AUDIOINPUT_PRECHANGE,
        EVENT_AUDIOINPUT_CHANGE,
        EVENT_VIDEOFORMAT_PRECHANGE,
        EVENT_VIDEOFORMAT_CHANGE,
        EVENT_CHANNEL_PRECHANGE,
        EVENT_CHANNEL_CHANGE,
        EVENT_ENDOFLIST
    };
    EventCollector->Register(this, EventList);
}

CSettingsMaster::~CSettingsMaster()
{
	EventCollector->Unregister(this);
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
void CSettingsMaster::ParseAllSettings(CEventObject* pObject, int What, eOnChangeType OnChangeType)
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
			if ((pSource != NULL) && ((CEventObject*)pSource != pObject))
			{
				continue;
			}
		}
        
        m_Holders[i].pHolder->SetLocation(&vSubLocations);

		int Num = m_Holders[i].pHolder->GetNumSettings();
        CSimpleSetting* pSetting;
        string sSection;
        eSettingFlags SettingFlags;
        
		BOOL bAction = FALSE;

        for (int n = 0; n < Num; n++)
        {
            pSetting = (CSimpleSetting*)m_Holders[i].pHolder->GetSetting(n);
            if (pSetting != NULL)
            {
                bAction = FALSE;
				SettingFlags = pSetting->GetFlags();
                      
                if ((SettingFlags&SETTINGFLAG_PER_SOURCE) && (SettingFlags&SETTINGFLAG_ALLOW_PER_SOURCE) && (m_SourceName.length()>0))
                {
                    bAction = TRUE;
                }
                else if ((SettingFlags&SETTINGFLAG_PER_VIDEOINPUT) && (SettingFlags&SETTINGFLAG_ALLOW_PER_VIDEOINPUT) && (m_VideoInputName.length()>0))
                {
                    bAction = TRUE;
                }
                else if ((SettingFlags&SETTINGFLAG_PER_AUDIOINPUT) && (SettingFlags&SETTINGFLAG_ALLOW_PER_AUDIOINPUT) && (m_AudioInputName.length()>0))
                {
                    bAction = TRUE;
                }
                else if ((SettingFlags&SETTINGFLAG_PER_VIDEOFORMAT) && (SettingFlags&SETTINGFLAG_ALLOW_PER_VIDEOFORMAT) && (m_VideoFormatName.length()>0))
                {
                    bAction = TRUE;
                }
                else if ((SettingFlags&SETTINGFLAG_PER_CHANNEL) && (SettingFlags&SETTINGFLAG_ALLOW_PER_CHANNEL) && (m_ChannelName.length()>0))
                {
                    bAction = TRUE;
                }

                if (bAction)
                {                    					
                    if (What == 0)
                    {
                        m_Holders[i].pHolder->ReadSettingFromIni(pSetting, OnChangeType);
                    }
                    else if (What == 1)
                    {
                        m_Holders[i].pHolder->WriteSettingToIni(pSetting, TRUE, OnChangeType);
                    }                    
                }
            }
        }
    }
}


void CSettingsMaster::LoadSettings(CEventObject* pObject, eOnChangeType OnChangeType)
{
    ParseAllSettings(pObject, 0, OnChangeType);
}


void CSettingsMaster::SaveSettings(CEventObject* pObject, eOnChangeType OnChangeType)
{
    ParseAllSettings(pObject, 1, OnChangeType);
}

/**
    Register a settingholder to the list.
    The HolderID is for uniquely identifying a settingsholder from
    anywhere within the program so one can easily request any setting
    from anywhere. Setting values & Holder IDs are in DS_Control.h
*/
void CSettingsMaster::Register(SETTINGHOLDERID HolderID, CSettingsHolder* pHolder)
{
    if (pHolder == NULL)
    {
        return;
    }
       
    Unregister(pHolder);

    TSettingsHolderInfo shi;
    BOOL bIsSource = FALSE;

    shi.HolderID = HolderID;
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

void CSettingsMaster::Unregister(SETTINGHOLDERID HolderID)
{
    vector<TSettingsHolderInfo> NewList;
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if (m_Holders[i].HolderID == HolderID)
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

/**
    Event callback function.    
*/
void CSettingsMaster::OnEvent(CEventObject* pObject, eEventType Event, long OldValue, long NewValue, eEventType* ComingUp)
{
    switch (Event)
    {
    case EVENT_SOURCE_PRECHANGE:    
        if (m_SourceName.length()>0) 
        { 
            SaveSettings(pObject, ONCHANGE_SOURCECHANGE);
        }
        break;        
    
    case EVENT_SOURCE_CHANGE:    
        if (NewValue!=0)
        {
            m_SourceName = ((CSource*)NewValue)->IDString();
            LoadSettings(pObject, ONCHANGE_SOURCECHANGE);
        }
        else
        {
            m_SourceName = "";
        }
        break;
     
    case EVENT_CHANNEL_PRECHANGE:   
        if (m_ChannelName.length()>0) 
        { 
            SaveSettings(pObject, ONCHANGE_CHANNELCHANGE); 
        }
        break;
        
    case EVENT_CHANNEL_CHANGE:
        if (NewValue>=0)
        {        
            char szBuffer[33];
            m_ChannelName = string("Channel") + itoa(NewValue, szBuffer, 10);
            LoadSettings(pObject, ONCHANGE_CHANNELCHANGE);  
        }
        else
        {
            m_ChannelName = "";
        }
        break;
    
    case EVENT_VIDEOINPUT_PRECHANGE:
        if (m_VideoInputName.length()>0) 
        { 
            SaveSettings(pObject, ONCHANGE_VIDEOINPUTCHANGE); 
        }
        break;        

    case EVENT_VIDEOINPUT_CHANGE:
        if (NewValue>=0)
        {
            char szBuffer[33];
            m_VideoInputName = string("VideoInput") +  itoa(NewValue, szBuffer, 10);
            LoadSettings(pObject, ONCHANGE_VIDEOINPUTCHANGE);
        }
        else
        {
            m_VideoInputName = "";
        }
        break;    

    case EVENT_AUDIOINPUT_PRECHANGE:
        if (m_AudioInputName.length()>0) 
        { 
            SaveSettings(pObject, ONCHANGE_AUDIOINPUTCHANGE); 
        }
        break;
        
    case EVENT_AUDIOINPUT_CHANGE:
        if (NewValue>=0)
        {
            char szBuffer[33];
            m_AudioInputName = string("AudioInput") +  itoa(NewValue, szBuffer, 10);
            LoadSettings(pObject, ONCHANGE_AUDIOINPUTCHANGE);
        }
        else
        {
            m_AudioInputName = "";
        }
        break;
        
    case EVENT_VIDEOFORMAT_PRECHANGE:
        if (m_VideoFormatName.length()>0) 
        { 
            SaveSettings(pObject, ONCHANGE_VIDEOFORMATCHANGE);
        }
        break;
        
    case EVENT_VIDEOFORMAT_CHANGE:
        if (NewValue>=0)
        {
            char szBuffer[33];
            m_VideoFormatName = string("VideoFormat") +  itoa(NewValue, szBuffer, 10);            
            LoadSettings(pObject, ONCHANGE_VIDEOFORMATCHANGE);
        }
        else
        {
            m_VideoFormatName = "";
        }
        break;
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


/**
    Get setting 'n' from setting holder 'HolderID'.
*/
ISetting* CSettingsMaster::GetSetting(SETTINGHOLDERID HolderID, int n)
{
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if ( (m_Holders[i].pHolder != NULL) && (m_Holders[i].pHolder->GetID() == HolderID) )
		{        
            return m_Holders[i].pHolder->GetSetting(n);
        }
    }
    return NULL;
}
