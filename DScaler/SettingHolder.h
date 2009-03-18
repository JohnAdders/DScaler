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

/** Base class for any class that needs acesss to a list of ISetting settings.
    The function CreateSettings should be overriden and then called in the 
    constructor of the derived class.
*/
class CSettingsHolder
{
public:
    CSettingsHolder(long SetMessage);
    ~CSettingsHolder();
    long GetNumSettings();
    ISetting* GetSetting(long SettingIndex);
    virtual void CreateSettings(LPCSTR IniSection) = 0;
    void ReadFromIni();    
    void WriteToIni(BOOL bOptimizeFileAccess);

    void AddSetting(ISetting* pSetting);

    void AddSetting(SETTING* pSetting, CSettingGroup* pGroup = NULL);

    void DisableOnChange();
    void EnableOnChange();

    void ClearSettingList(BOOL bDeleteSettings = TRUE, BOOL bWriteSettings = FALSE);

    int LoadSettingStructures(SETTING* pSetting, int StartNum, int Num, CSettingGroup* pGroup = NULL);

    LONG HandleSettingsMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bHandled);
    
    CSettingGroup *GetSettingsGroup(LPCSTR szDisplayName, DWORD Flags = 0, BOOL IsActiveByDefault = FALSE);

protected:
    std::vector<ISetting*>  m_Settings;
    long                    m_SetMessage;
    BOOL                    m_pRegistered;
    std::string             m_SettingFlagsSection;

    BOOL   RegisterMe();
};

/** Standalone settings holder
*/
class CSettingsHolderStandAlone : public CSettingsHolder
{
public:
    CSettingsHolderStandAlone();
    ~CSettingsHolderStandAlone();
    void CreateSettings(LPCSTR IniSection){;};

    ISetting* operator[](int i) { if ((i>=0) && ((size_t)i<m_Settings.size())) { return m_Settings[i]; } return NULL; }
};
#endif

