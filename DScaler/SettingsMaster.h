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
public:
    CSettingsMaster(LPCTSTR szIniFile);
    ~CSettingsMaster();

    void Initialize();

    void Register(SmartPtr<CSettingsHolder> pHolder);
    void Unregister(SmartPtr<CSettingsHolder> pHolder);

    void SetSource(CSource* pSource);
    void SetChannelName(long NewValue);
    void SetVideoInput(long NewValue);
    void SetAudioInput(long NewValue);
    void SetVideoFormat(long NewValue);

    void SaveGroupedSettings();
    void LoadGroupedSettings();

    void SaveAllSettings(BOOL bOptimizeFileAccess);

    CSettingGroup* GetGroup(LPCTSTR szDisplayName, DWORD Flags = 0, BOOL IsActiveByDefault = FALSE);

    SmartPtr<CSettingsHolder> GetSettingsPage();

    void LoadOneGroupedSetting(CSimpleSetting* pSetting);
    void WriteOneGroupedSetting(CSimpleSetting* pSetting);

    LONG HandleSettingMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone);

    SmartPtr<CSettingsHolder> FindMsgHolder(long Message);

private:
    typedef SETTING* (__cdecl *GENERICGETSETTING)(long SettingIndex);
    void AddSettings(long MessageIdRoot, GENERICGETSETTING GetSettingFunction);

    void ParseSettingHolder(CSettingsHolder* Holder, BOOL IsLoad);
    void ParseAllSettings(BOOL IsLoad);
    void MakeSubSection(tstring& SubSection, CSettingGroup* pGroup);

    typedef std::vector< SmartPtr<CSettingsHolder> > Holders;
    /// List of setting holders
    Holders m_Holders;

    /// Name of the ini file
    tstring m_sIniFile;

    /// Names of the current source, inputs, video format and channel
    tstring m_SourceName;
    tstring m_VideoInputName;
    tstring m_AudioInputName;
    tstring m_VideoFormatName;
    tstring m_ChannelName;

    std::vector< SmartPtr<CSettingGroup> > m_SettingsGroups;
};

extern SmartPtr<CSettingsMaster> SettingsMaster;


#endif
