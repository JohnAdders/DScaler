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
