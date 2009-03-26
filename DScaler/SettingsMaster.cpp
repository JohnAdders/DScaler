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
#include "Settings.h"
#include "Audio.h"
#include "VBI.h"
#include "OutThreads.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "MixerDev.h"
#include "DScaler.h"
#include "ProgramList.h"
#include "OverlayOutput.h"
#include "D3D9Output.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "FD_Prog.h"
#include "Slider.h"
#include "Splash.h"
#include "OSD.h"
#include "Filter.h"
#include "FieldTiming.h"
#include "VBI_VideoText.h"
#include "Providers.h"
#include "Calibration.h"
#include "StillSource.h"
#include "SettingsPerChannel.h"
#include "TimeShift.h"
#include "EPG.h"

using namespace std;

CSettingsMaster::CSettingsMaster(LPCSTR szIniFile) :
    m_sIniFile(szIniFile)
{
}

CSettingsMaster::~CSettingsMaster()
{
}

void CSettingsMaster::Initialize()
{
    AddSettings(WM_DEBUG_GETVALUE, (GENERICGETSETTING)Debug_GetSetting);
    Register(Aspect_GetSettingsHolder());
    AddSettings(WM_DSCALER_GETVALUE, (GENERICGETSETTING)DScaler_GetSetting);
    AddSettings(WM_OUTTHREADS_GETVALUE, (GENERICGETSETTING)OutThreads_GetSetting);
    AddSettings(WM_OTHER_GETVALUE, (GENERICGETSETTING)Overlay_GetSetting);
    AddSettings(WM_FD50_GETVALUE, (GENERICGETSETTING)FD50_GetSetting);
    AddSettings(WM_FD60_GETVALUE, (GENERICGETSETTING)FD60_GetSetting);
    AddSettings(WM_FD_COMMON_GETVALUE, (GENERICGETSETTING)FD_Common_GetSetting);
    AddSettings(WM_OSD_GETVALUE, (GENERICGETSETTING)OSD_GetSetting);
    AddSettings(WM_VBI_GETVALUE, (GENERICGETSETTING)VBI_GetSetting);
    AddSettings(WM_TIMING_GETVALUE, (GENERICGETSETTING)Timing_GetSetting);
    AddSettings(WM_CHANNELS_GETVALUE, (GENERICGETSETTING)Channels_GetSetting);
    AddSettings(WM_AUDIO_GETVALUE, (GENERICGETSETTING)Audio_GetSetting);
    AddSettings(WM_VT_GETVALUE, (GENERICGETSETTING)VT_GetSetting);
    AddSettings(WM_CALIBR_GETVALUE, (GENERICGETSETTING)Calibr_GetSetting);
    AddSettings(WM_STILL_GETVALUE, (GENERICGETSETTING)Still_GetSetting);
    AddSettings(WM_ANTIPLOP_GETVALUE, (GENERICGETSETTING)AntiPlop_GetSetting);
    AddSettings(WM_SETTINGSPERCHANNEL_GETVALUE, (GENERICGETSETTING)SettingsPerChannel_GetSetting);
    AddSettings(WM_FDPROG_GETVALUE, (GENERICGETSETTING)FDProg_GetSetting);
    AddSettings(WM_EPG_GETVALUE, (GENERICGETSETTING)EPG_GetSetting);
    //AddSettings(WM_D3D9_GETVALUE, (GENERICGETSETTING)D3D9_GetSetting);

    for(Holders::iterator it = m_Holders.begin(); it != m_Holders.end(); ++it)
    {
        (*it)->DisableOnChange();
        (*it)->ReadFromIni();
        (*it)->EnableOnChange();
    }

    Providers_ReadFromIni();
}

void CSettingsMaster::AddSettings(long MessageIdRoot, GENERICGETSETTING GetSettingFunction)
{
    SmartPtr<CSettingsHolder> Holder = new CSettingsHolder(MessageIdRoot);
    int i(0);

    SETTING* NewSetting = GetSettingFunction(i);
    while(NewSetting != NULL)
    {
        Holder->AddSetting(new CSettingWrapper(NewSetting));
        ++i;
        NewSetting = GetSettingFunction(i);
    }

    m_Holders.push_back(Holder);
}
void CSettingsMaster::LoadOneGroupedSetting(CSimpleSetting* pSetting)
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
                pSetting->ReadFromIniSubSection(SubSection);
            }
        }
    }
}

void CSettingsMaster::WriteOneGroupedSetting(CSimpleSetting* pSetting)
{
    CSettingGroup* pGroup = pSetting->GetGroup();
    if(pGroup != NULL)
    {
        if(pGroup->IsGroupActive())
        {
            string SubSection;
            MakeSubSection(SubSection, pGroup);
            if(SubSection.length() > 0)
            {
                pSetting->WriteToIniSubSection(SubSection.c_str());
            }
        }
    }
}


void CSettingsMaster::ParseSettingHolder(CSettingsHolder* Holder, BOOL IsLoad)
{
    int Num = Holder->GetNumSettings();

    BOOL bAction = FALSE;

    for (int n = 0; n < Num; n++)
    {
        if (IsLoad)
        {
            LoadOneGroupedSetting(Holder->GetSetting(n));
        }
        else
        {
            WriteOneGroupedSetting(Holder->GetSetting(n));
        }
    }
}

/**
    Runs through all settings to see if they want to read/write their values
    we only save or load those setting that are being held by channel/format etc
*/
void CSettingsMaster::ParseAllSettings(BOOL IsLoad)
{
    for (int i = 0; i < m_Holders.size(); i++)
    {
        ParseSettingHolder(m_Holders[i], IsLoad);
    }
    ParseSettingHolder(Providers_GetCurrentSource(), IsLoad);
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

void CSettingsMaster::LoadGroupedSettings()
{
//LOG(1, "LoadGroupedSettings m_SourceName %s", m_SourceName.c_str());
//LOG(1, "LoadGroupedSettings m_VideoInputName %s", m_VideoInputName.c_str());
//LOG(1, "LoadGroupedSettings m_AudioInputName %s", m_AudioInputName.c_str());
//LOG(1, "LoadGroupedSettings m_VideoFormatName %s", m_VideoFormatName.c_str());
//LOG(1, "LoadGroupedSettings m_ChannelName %s", m_ChannelName.c_str());
    ParseAllSettings(TRUE);
}


void CSettingsMaster::SaveGroupedSettings()
{
//LOG(1, "SaveGroupedSettings m_SourceName %s", m_SourceName.c_str());
//LOG(1, "SaveGroupedSettings m_VideoInputName %s", m_VideoInputName.c_str());
//LOG(1, "SaveGroupedSettings m_AudioInputName %s", m_AudioInputName.c_str());
//LOG(1, "SaveGroupedSettings m_VideoFormatName %s", m_VideoFormatName.c_str());
//LOG(1, "SaveGroupedSettings m_ChannelName %s", m_ChannelName.c_str());
    ParseAllSettings(FALSE);
}

void CSettingsMaster::Register(SmartPtr<CSettingsHolder> pHolder)
{
    //Add to holder list
    m_Holders.push_back(pHolder);
}

void CSettingsMaster::Unregister(SmartPtr<CSettingsHolder> pHolder)
{
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if (m_Holders[i] == pHolder)
        {
            m_Holders.erase(m_Holders.begin() + i);
            return;
        }
    }
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
    if (NewValue >= 0)
    {
        m_ChannelName = Channel_GetName();
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
        m_VideoInputName = MakeString() << "VideoInput" << NewValue;
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
        m_AudioInputName = MakeString() << "AudioInput" << NewValue;
    }
    else
    {
        m_AudioInputName = "";
    }
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
        if(m_SettingsGroups[i]->GetName() == szName)
        {
            return m_SettingsGroups[i];
        }
    }
    SmartPtr<CSettingGroup> pNewGroup = new CSettingGroup(szName, Flags, IsActiveByDefault);
    m_SettingsGroups.push_back(pNewGroup);
    return pNewGroup;
}

SmartPtr<CTreeSettingsGeneric> CSettingsMaster::GetTreeSettingsPage()
{
    SmartPtr<CSettingsHolder> Holder(new CSettingsHolder);

    for (int i = 0; i < m_SettingsGroups.size(); i++)
    {
        Holder->AddSetting(m_SettingsGroups[i]->GetIsActiveSetting());
    }
    return new CTreeSettingsGeneric("Activate Setting's Saving", Holder);
}


void CSettingsMaster::SaveAllSettings(BOOL bOptimizeFileAccess)
{
    for(Holders::iterator it = m_Holders.begin(); it != m_Holders.end(); ++it)
    {
        (*it)->WriteToIni(bOptimizeFileAccess);
    }

    Providers_WriteToIni(bOptimizeFileAccess);

    // These two lines flushes current INI file to disk (in case of abrupt poweroff shortly afterwards)
    WritePrivateProfileString(NULL, NULL, NULL, m_sIniFile.c_str());

    //BeautifyIniFile(m_sIniFile.c_str());
}

SmartPtr<CSettingsHolder> CSettingsMaster::FindMsgHolder(long Message)
{
    for(Holders::iterator it = m_Holders.begin(); it != m_Holders.end(); ++it)
    {
        if(Message == (*it)->GetMessageID())
        {
            return *it;
        }
    }
    return 0L;
}

LONG CSettingsMaster::HandleSettingMsgs(HWND hWnd, UINT Message, UINT wParam, LONG lParam, BOOL* bDone)
{
    *bDone = FALSE;

    SmartPtr<CSettingsHolder> Holder(FindMsgHolder(Message));
    if(Holder)
    {
        SmartPtr<CSimpleSetting> Setting = Holder->GetSetting(wParam);
        if(Setting)
        {
            *bDone = TRUE;
            return Setting->GetValueAsMessage();
        }
        else
        {
            return 0;
        }
    }

    Holder = FindMsgHolder(Message - 100);
    if(Holder)
    {
        SmartPtr<CSimpleSetting> Setting = Holder->GetSetting(wParam);
        if(Setting)
        {
            *bDone = TRUE;
            Setting->SetValueFromMessage(lParam);
        }
        return 0;
    }

    Holder = FindMsgHolder(Message - 200);
    if(Holder)
    {
        SmartPtr<CSimpleSetting> Setting = Holder->GetSetting(wParam);
        if(Setting)
        {
            *bDone = TRUE;
            Setting->ChangeValue((eCHANGEVALUE)lParam);
        }
        return 0;
    }
    else
    {
        return 0;
    }
}

