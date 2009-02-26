/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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
 * @file SettingsMaster.h SettingsMaster Header file
 */
 
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
    void MakeSubSection(string& SubSection, CSettingGroup* pGroup);

public:
    CSettingsMaster();
    ~CSettingsMaster();
    
    void IniFile(LPCSTR szIniFile) { m_sIniFile = szIniFile; }
    void Register(CSettingsHolder* pHolder);
    void Unregister(CSettingsHolder* pHolder);        

    void SetSource(CSource* pSource);
    void SetChannelName(long NewValue);
    void SetVideoInput(long NewValue);
    void SetAudioInput(long NewValue);
    void SetVideoFormat(long NewValue);

    void SaveSettings();
    void LoadSettings();

    CSettingGroup* GetGroup(LPCSTR szDisplayName, DWORD Flags, BOOL IsActiveByDefault);

    CTreeSettingsGeneric* GetTreeSettingsPage();

    void LoadOneSetting(CSimpleSetting* pSetting);
    void WriteOneSetting(CSimpleSetting* pSetting);

private:    
    vector<CSettingGroup*> m_SettingsGroups;
};

extern CSettingsMaster* SettingsMaster;


#endif
