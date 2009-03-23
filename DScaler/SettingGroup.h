/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 JOhn Adcock.  All rights reserved.
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
 * @file ISetting.h interface for the ISetting class.
 */

#ifndef _ISETTING_H_
#define _ISETTING_H_

#include "DS_ApiCommon.h"
#include "DS_Control.h"


//See ISetting.cpp for a detailed description of the functions

class CSettingGroup;

/** Interface for control of a setting
*/

class CSimpleSetting;
class CYesNoSetting;

enum eGroupSettingsFlags
{
    SETTING_BY_CHANNEL = 1,
    SETTING_BY_FORMAT = 2,
    SETTING_BY_INPUT = 4,
    SETTING_BY_AUDIOINPUT = 8,
};


/**
    Setting group class.
    Contains info like name & description

    Optionally is part of a group tree list.
    Optionnaly belongs to an SettingObject.
*/
class CSettingGroup
{
public:

    CSettingGroup(LPCSTR szName, DWORD Flags = 0, BOOL IsActiveByDefault = FALSE);
    ~CSettingGroup();

    LPCSTR GetName();

    BOOL IsSetByChannel() { return ((m_Flags & SETTING_BY_CHANNEL) == SETTING_BY_CHANNEL);};
    BOOL IsSetByFormat() { return ((m_Flags & SETTING_BY_FORMAT) == SETTING_BY_FORMAT);};
    BOOL IsSetByInput() { return ((m_Flags & SETTING_BY_INPUT) == SETTING_BY_INPUT);};
    BOOL IsSetByAudioInput() { return ((m_Flags & SETTING_BY_AUDIOINPUT) == SETTING_BY_AUDIOINPUT);};

    BOOL IsGroupActive();

    void AddSetting(CSimpleSetting *pSetting);
    CSimpleSetting* GetIsActiveSetting();

protected:
    /// Name of the group
    std::string m_Name;
    DWORD m_Flags;

    SmartPtr<CYesNoSetting> m_IsActive;
};

#endif
