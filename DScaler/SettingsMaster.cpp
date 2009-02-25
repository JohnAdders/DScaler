/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.12  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.11  2003/05/31 18:45:21  laurentg
// Possibility to save video format per video input corrected
//
// Revision 1.10  2003/05/30 10:06:24  adcockj
// Fix for format names in ini file
//
// Revision 1.9  2003/04/23 08:30:57  adcockj
// Prevent section names from having square brackets
//
// Revision 1.8  2003/01/21 10:41:40  adcockj
// Added header
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file SettingsMaster.cpp CSettingsMaster Implementation
 */

#include "stdafx.h"

#include "Source.h"
#include "Providers.h"
#include "TreeSettingsGeneric.h"
#include "SettingsMaster.h"
#include "SettingsPerChannel.h"
#include "TVFormats.h"
#include "ProgramList.h"
#include "DebugLog.h"

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

void CSettingsMaster::LoadOneSetting(CSimpleSetting* pSetting)
{
    if (pSetting != NULL)
    {
        string SubSection("");

        CSettingGroup* pGroup = pSetting->GetGroup();
        if(pGroup != NULL)
        {
            if(pGroup->IsGroupActive())
            {
                MakeSubSection(SubSection, pGroup);
                if(SubSection.length() > 0)
                {
					pSetting->ReadFromIniSubSection(SubSection.c_str());
                }
            }
        }
    }
}

void CSettingsMaster::WriteOneSetting(CSimpleSetting* pSetting)
{
    if (pSetting != NULL)
    {
        string SubSection("");

        CSettingGroup* pGroup = pSetting->GetGroup();
        if(pGroup != NULL)
        {
            if(pGroup->IsGroupActive())
            {
                MakeSubSection(SubSection, pGroup);
                if(SubSection.length() > 0)
                {
                    pSetting->WriteToIniSubSection(SubSection.c_str());
                }
            }
        }
    }
}

/**
    Runs through all settings to see if they want to read/write their values
    we only save or load those setting that are being held by channel/format etc
*/
void CSettingsMaster::ParseAllSettings(bool IsLoad)
{
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
        
		int Num = m_Holders[i].pHolder->GetNumSettings();
        
		BOOL bAction = FALSE;
        
        for (int n = 0; n < Num; n++)
        {
			if (IsLoad)
			{
				LoadOneSetting((CSimpleSetting*)m_Holders[i].pHolder->GetSetting(n));
			}
			else
			{
				WriteOneSetting((CSimpleSetting*)m_Holders[i].pHolder->GetSetting(n));
			}
        }
    }
}

void CSettingsMaster::MakeSubSection(string& SubSection, CSettingGroup* pGroup)
{
    static CSettingGroup* LastGroup = NULL;
    static DWORD LastFlags = -1;
    if(LastGroup == pGroup)
    {
        return;
    }

    SubSection = "";

    // if we are doing have an audio input check to see if
    // we have any settings by that
    if(m_AudioInputName.length() > 0)
    {
        if(pGroup->IsSetByAudioInput() && (SettingsPerChannel_IsPerInput() || SettingsPerChannel_IsPerChannel() || SettingsPerChannel_IsPerFormat()))
        {
            SubSection = m_SourceName;
            SubSection += "_";
            SubSection += m_AudioInputName;
            return;
        }
    }

    // if we are doing a channel
    // then we only want the channel name
    // if we are doing by channels
    if(m_ChannelName.length() > 0)
    {
        if(pGroup->IsSetByChannel() && SettingsPerChannel_IsPerChannel())
        {
            SubSection = m_SourceName;
            SubSection += "_";
            for(int i(0); i < m_ChannelName.length(); ++i)
            {
                if(m_ChannelName[i] != '[' && m_ChannelName[i] != ']')
                {
                    SubSection += m_ChannelName[i];
                }
            }
            return;
        }
        if(pGroup->IsSetByInput() && SettingsPerChannel_IsPerInput())
        {
            SubSection = m_SourceName;
            SubSection += "_";
            SubSection += "Tuner";
        }
        if(pGroup->IsSetByFormat() && SettingsPerChannel_IsPerFormat())
        {
            if(SubSection.length() == 0)
            {
                SubSection = m_SourceName;
            }
            SubSection += "_";
            SubSection += m_VideoFormatName;
        }
        return;
    }

    if(m_VideoInputName.length() > 0)
    {
        if(pGroup->IsSetByInput() && SettingsPerChannel_IsPerInput())
        {
            SubSection = m_SourceName;
            SubSection += "_";
            SubSection += m_VideoInputName;
        }
    }

    if(m_VideoFormatName.length() > 0)
    {
        if(pGroup->IsSetByFormat() && SettingsPerChannel_IsPerFormat())
        {
            if(SubSection.length() == 0)
            {
                SubSection = m_SourceName;
            }
            SubSection += "_";
            SubSection += m_VideoFormatName;
        }
    }
}

void CSettingsMaster::LoadSettings()
{
//LOG(1, "LoadSettings m_SourceName %s", m_SourceName.c_str());
//LOG(1, "LoadSettings m_VideoInputName %s", m_VideoInputName.c_str());
//LOG(1, "LoadSettings m_AudioInputName %s", m_AudioInputName.c_str());
//LOG(1, "LoadSettings m_VideoFormatName %s", m_VideoFormatName.c_str());
//LOG(1, "LoadSettings m_ChannelName %s", m_ChannelName.c_str());
    ParseAllSettings(true);
}


void CSettingsMaster::SaveSettings()
{
//LOG(1, "SaveSettings m_SourceName %s", m_SourceName.c_str());
//LOG(1, "SaveSettings m_VideoInputName %s", m_VideoInputName.c_str());
//LOG(1, "SaveSettings m_AudioInputName %s", m_AudioInputName.c_str());
//LOG(1, "SaveSettings m_VideoFormatName %s", m_VideoFormatName.c_str());
//LOG(1, "SaveSettings m_ChannelName %s", m_ChannelName.c_str());
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
//LOG(1, "m_SourceName %s", m_SourceName.c_str());
}

void CSettingsMaster::SetChannelName(long NewValue)
{
    if (NewValue >= 0)
    {        
        m_ChannelName = Channel_GetName();
    }
    else
    {
        m_ChannelName = "";
    }
//LOG(1, "m_ChannelName %s", m_ChannelName.c_str());
}

void CSettingsMaster::SetVideoInput(long NewValue)
{
    if (NewValue>=0)
    {
        char szBuffer[33];
        m_VideoInputName = string("VideoInput") +  _itoa(NewValue, szBuffer, 10);
    }
    else
    {
        m_VideoInputName = "";
    }
//LOG(1, "m_VideoInputName %s", m_VideoInputName.c_str());
}

void CSettingsMaster::SetAudioInput(long NewValue)
{
    if (NewValue>=0)
    {
        char szBuffer[33];
        m_AudioInputName = string("AudioInput") +  _itoa(NewValue, szBuffer, 10);
    }
    else
    {
        m_AudioInputName = "";
    }
//LOG(1, "m_AudioInputName %s", m_AudioInputName.c_str());
}

void CSettingsMaster::SetVideoFormat(long NewValue)
{
    if (NewValue >= 0 && NewValue < VIDEOFORMAT_LASTONE)
    {
        m_VideoFormatName = VideoFormatSaveNames[NewValue];
    }
    else
    {
        m_VideoFormatName = "";
    }
//LOG(1, "m_VideoFormatName %d (%d) ===> %s", NewValue, VIDEOFORMAT_LASTONE, m_VideoFormatName.c_str());
}

CSettingGroup* CSettingsMaster::GetGroup(LPCSTR szName, DWORD Flags, BOOL IsActiveByDefault)
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

CTreeSettingsGeneric* CSettingsMaster::GetTreeSettingsPage()
{
    vector <CSimpleSetting*>vSettingsList;

    for (int i = 0; i < m_SettingsGroups.size(); i++)
    {
        vSettingsList.push_back(m_SettingsGroups[i]->GetIsActiveSetting());
    }
    return new CTreeSettingsGeneric("Activate Setting's Saving",vSettingsList);
}
