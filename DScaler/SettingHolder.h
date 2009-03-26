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
 * @file SettingHolder.h SettingHolder Header file
 */

#ifndef _SETTINGHOLDER_H_
#define _SETTINGHOLDER_H_

#include "Setting.h"
#include "TVFormats.h"
class CSource;

/** Base class for any class that needs acesss to a list of CSimpleSetting settings.
    The function CreateSettings should be overriden and then called in the
    constructor of the derived class.
*/
class CSettingsHolder
{
public:
    CSettingsHolder(long SetMessage = 0, UINT HelpID = 0);
    virtual ~CSettingsHolder();
    long GetNumSettings();
    void ReadFromIni();
    void WriteToIni(BOOL bOptimizeFileAccess);
    long GetMessageID() { return m_SetMessage; }

    void AddSetting(SmartPtr<CSimpleSetting> pSetting);
    void AddSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);
    void AddSettings(SETTING* pSetting, int Num, CSettingGroup* pGroup = NULL);

    SmartPtr<CSimpleSetting> GetSetting(long SettingIndex);

    void DisableOnChange();
    void EnableOnChange();

    UINT GetHelpID() { return m_HelpID;};

    LONG HandleSettingsMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bHandled);

protected:
    typedef std::vector< SmartPtr<CSimpleSetting> > SettingsArray;

    SettingsArray   m_Settings;
    long            m_SetMessage;
    std::string     m_SettingFlagsSection;
    UINT            m_HelpID;

};

#endif

