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
 * @file CSimpleSetting.cpp Settings classes implementation
 */

#include "stdafx.h"

#include "SettingGroup.h"
#include "Setting.h"

using namespace std;

CSettingGroup::CSettingGroup(const tstring& Name, DWORD Flags, BOOL IsActiveByDefault) :
    m_Name(Name),
    m_Flags(Flags)
{
    m_IsActive = new CYesNoSetting(Name, IsActiveByDefault, _T("SettingsPerChannel"), Name);
    m_IsActive->ReadFromIni();
}


CSettingGroup::~CSettingGroup()
{
    m_IsActive->WriteToIni(TRUE);
}

const tstring& CSettingGroup::GetName()
{
    return m_Name;
}

BOOL CSettingGroup::IsGroupActive()
{
    return m_IsActive->GetValue();
}

SmartPtr<CSimpleSetting> CSettingGroup::GetIsActiveSetting()
{
    return m_IsActive;
}
