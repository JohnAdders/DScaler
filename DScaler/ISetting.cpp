//
// $Id: ISetting.cpp,v 1.6 2003-10-27 10:39:52 adcockj Exp $
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
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

/**
 * @file ISetting.cpp Settings classes implementation
 */

#include "stdafx.h"

#include "ISetting.h"
#include "Setting.h"

CSettingGroup::CSettingGroup(LPCSTR szName, DWORD Flags, BOOL IsActiveByDefault) :
    m_Name(szName),
    m_Flags(Flags)
{
    m_IsActive = new CYesNoSetting(szName, IsActiveByDefault, "SettingsPerChannel", szName);
    m_IsActive->ReadFromIni();
}


CSettingGroup::~CSettingGroup()
{
    m_IsActive->WriteToIni(FALSE);
    delete m_IsActive;
}

LPCSTR CSettingGroup::GetName() 
{
    return m_Name.c_str();
}

BOOL CSettingGroup::IsGroupActive() 
{
    return m_IsActive->GetValue();
}

CSimpleSetting* CSettingGroup::GetIsActiveSetting()
{
    return m_IsActive;
}


/** 
    Adds group to pSetting    
*/
void CSettingGroup::AddSetting(ISetting *pSetting)
{
    pSetting->SetGroup(this);    
}
