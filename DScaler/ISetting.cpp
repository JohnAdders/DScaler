#include "stdafx.h"

#include "ISetting.h"

CSettingGroup::CSettingGroup(LPCSTR szName, DWORD Flags, BOOL IsActiveByDefault) :
    m_Name(szName),
    m_Flags(Flags),
    m_IsActive(IsActiveByDefault)
{
}


CSettingGroup::~CSettingGroup()
{
}

LPCSTR CSettingGroup::GetName() 
{
    return m_Name.c_str();
}

/** 
    Adds group to pSetting    
*/
void CSettingGroup::AddSetting(ISetting *pSetting)
{
    pSetting->SetGroup(this);    
}
