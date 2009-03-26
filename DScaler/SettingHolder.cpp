//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 John Adcock.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file SettingHolder.cpp CSettingHolder Implementation
 */

#include "stdafx.h"

#include "SettingHolder.h"
#include "SettingsMaster.h"

using namespace std;

CSettingsHolder::CSettingsHolder(long SetMessage, UINT HelpID) :
    m_SetMessage(SetMessage),
    m_HelpID(HelpID)
{
    m_SettingFlagsSection = "SettingFlags";
}

CSettingsHolder::~CSettingsHolder()
{
}

/**
    Add a settings to the list.
    Makes sure there are no duplicates.
*/
void CSettingsHolder::AddSetting(SmartPtr<CSimpleSetting> pSetting)
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
}

void CSettingsHolder::AddSetting(SETTING* pSetting, CSettingGroup* pGroup)
{
    if(pSetting != NULL)
    {
        AddSetting(new CSettingWrapper(pSetting));
    }
}

/**
    Reads alls settings of the list from the .ini file.
*/
void CSettingsHolder::ReadFromIni()
{
    for(SettingsArray::iterator it = m_Settings.begin(); it != m_Settings.end(); ++it)
    {
        (*it)->ReadFromIni();
    }
}

/**
    Writes all settings of the list to the .ini file.
*/
void CSettingsHolder::WriteToIni(BOOL bOptimizeFileAccess)
{
    for(SettingsArray::iterator it = m_Settings.begin(); it != m_Settings.end(); ++it)
    {
        (*it)->WriteToIni(bOptimizeFileAccess);
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
SmartPtr<CSimpleSetting> CSettingsHolder::GetSetting(long SettingIndex)
{
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

    if (m_SetMessage == 0)
    {
        return 0;
    }

    if(wParam >= 0 && wParam < m_Settings.size())
    {
        SmartPtr<CSimpleSetting> pSetting = m_Settings[wParam];
        if(message == m_SetMessage)
        {
            RetVal = pSetting->GetValueAsMessage();
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 100)
        {
            pSetting->SetValueFromMessage(lParam);
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 200)
        {
            pSetting->ChangeValue((eCHANGEVALUE)lParam);
            *bHandled = TRUE;
        }
    }
    return RetVal;
}

/**
    Adds an array of SETTING structures to the SettingHolder.
    Optionally specify a default SettingGroup
*/
void CSettingsHolder::AddSettings(SETTING* pSetting, int Num, CSettingGroup* pGroup)
{
    for (int i(0) ; i < Num; i++)
    {
        if(pSetting[i].szDisplayName != NULL)
        {
            AddSetting(&pSetting[i], pGroup);
        }
    }
}

/**
    Enable all OnChange calls
*/
void CSettingsHolder::EnableOnChange()
{
    for (int i = 0; i < m_Settings.size(); i++)
    {
        m_Settings[i]->EnableOnChange();
    }
}

/**
    Disable all OnChange calls if a value of a setting is changed.
*/
void CSettingsHolder::DisableOnChange()
{
    for (int i = 0; i < m_Settings.size(); i++)
    {
        m_Settings[i]->DisableOnChange();
    }
}
