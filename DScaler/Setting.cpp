/////////////////////////////////////////////////////////////////////////////
// $Id: Setting.cpp,v 1.14 2002-09-26 16:34:19 kooiman Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.13  2002/09/26 06:09:48  kooiman
// Extended settings, preliminary.
//
// Revision 1.12  2002/09/02 19:06:10  kooiman
// It is now possible to modify CSimpleSetting style settings from the TreeSettingDialog
//
// Revision 1.11  2002/08/21 20:26:31  kooiman
// Added option to ChangeDefault to only change the current value if you want it.
//
// Revision 1.10  2002/08/08 12:13:32  kooiman
// Bit more flexibility.
//
// Revision 1.9  2002/08/06 18:30:52  kooiman
// Bit more flexibility.
//
// Revision 1.8  2002/06/13 11:43:56  robmuller
// Settings at default value that did not exist in the ini file were not written to the ini file.
//
// Revision 1.7  2002/02/08 19:27:18  adcockj
// Fixed problems with video settings dialog
//
// Revision 1.6  2002/01/24 00:00:13  robmuller
// Added bOptimizeFileAccess flag to WriteToIni from the settings classes.
//
// Revision 1.5  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.4  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.3  2001/11/12 08:01:58  adcockj
// Fixed Settings Bug
//
// Revision 1.2  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.1.2.7  2001/08/22 10:40:58  adcockj
// Added basic tuner support
// Fixed recusive bug
//
// Revision 1.1.2.6  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.1.2.5  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.1.2.4  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.1.2.3  2001/08/19 14:43:47  adcockj
// Fixed memory leaks
//
// Revision 1.1.2.2  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.1.2.1  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Setting.h"
#include "Settings.h"
#include "DebugLog.h"
#include "OSD.h"
#include "DScaler.h"
#include "Slider.h"
//#include "Source.h"
#include "Providers.h"
#include "ProgramList.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSettingsHolder::CSettingsHolder(long SetMessage) :
    m_SetMessage(SetMessage)
{
}

CSettingsHolder::~CSettingsHolder()
{
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        (*it)->WriteToIni(TRUE);
        delete *it;
    }
}

void CSettingsHolder::ReadFromIni(int bInit)
{
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        //(*it)->ReadFromIni(TRUE, TRUE, bInit?ONCHANGE_INIT:ONCHANGE_SET);
        (*it)->ReadFromIni(TRUE, ONCHANGE_NONE);
    }
}

void CSettingsHolder::WriteToIni(BOOL bOptimizeFileAccess)
{
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        if ((*it)->GetGroup()!=NULL)
        {
            (*it)->WriteToIni(bOptimizeFileAccess);
        }
        else
        {
            (*it)->WriteToIni(bOptimizeFileAccess);
        }        
    }
}

long CSettingsHolder::GetNumSettings()
{
    return m_Settings.size();
}

ISetting* CSettingsHolder::GetSetting(long SettingIndex)
{
    if(SettingIndex > 0 && SettingIndex < m_Settings.size())
    {
        return m_Settings[SettingIndex];
    }
    else
    {
        return NULL;
    }
}

LONG CSettingsHolder::HandleSettingsMessage(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bHandled)
{
    LONG RetVal = 0;
    if (m_SetMessage == 0) { return 0; }
    if(wParam > 0 && wParam < m_Settings.size())
    {
        if(message == m_SetMessage)
        {
            RetVal = m_Settings[wParam]->GetValue();
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 100)
        {
            m_Settings[wParam]->SetValue(lParam, ONCHANGE_SET);
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 200)
        {
            m_Settings[wParam]->ChangeValue((eCHANGEVALUE)lParam, ONCHANGE_SET);
            *bHandled = TRUE;
        }
    }
    return RetVal;
}

void CSettingsHolder::LoadSettingStructures(SETTING *pSetting, int StartNum, int Num, CSettingGroup *pGroup)
{
    int i;
    for (i = 0; i < StartNum; i++)
    {
        if (m_Settings.size()<=i) 
        {
            m_Settings.push_back(NULL);
        }
    }
    for (i = StartNum; i < m_Settings.size(); i++)
    {
        m_Settings.pop_back();        
    }
    for (i = 0 ; i < Num; i++)
    {
        switch (pSetting->Type)
        {
        case ONOFF:
        case YESNO:
            m_Settings.push_back(new CYesNoSetting(pSetting, pGroup));
        case ITEMFROMLIST:
            m_Settings.push_back(new CListSetting(pSetting, pGroup));
        case SLIDER:
            m_Settings.push_back(new CSliderSetting(pSetting, pGroup));    
        }
    }
}

void CSettingsHolder::LoadSettingStructuresEx(SETTINGEX *pSetting, int StartNum, int Num, CSettingGroupList *pGroupList)
{
    int i;
    for (i = 0; i < StartNum; i++)
    {
        if (m_Settings.size()<=i) 
        {
            m_Settings.push_back(NULL);
        }
    }
    for (i = StartNum; i < m_Settings.size(); i++)
    {
        m_Settings.pop_back();        
    }
    for (i = 0 ; i < Num; i++)
    {
        switch (pSetting->Type)
        {
        case ONOFF:
        case YESNO:
            m_Settings.push_back(new CYesNoSetting(pSetting, pGroupList));
        case ITEMFROMLIST:
            m_Settings.push_back(new CListSetting(pSetting, pGroupList));
        case SLIDER:
            m_Settings.push_back(new CSliderSetting(pSetting, pGroupList));    
        }
    }
}

CSettingsHolderStandAlone::CSettingsHolderStandAlone() : CSettingsHolder(0)
{
}

CSettingsHolderStandAlone::~CSettingsHolderStandAlone()
{
}

CSimpleSetting::CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, long StepValue, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC *pfnOnChangeFunc, void *pThis)
{
    m_pSetting = new SETTING;
    m_bFreeSettingOnExit = TRUE;
        
    m_DisplayName = DisplayName;
    m_pSetting->szDisplayName = (char*)m_DisplayName.c_str();

    m_StoreValue = Default;
    m_pSetting->pValue = &m_StoreValue;
    
    m_pSetting->LastSavedValue = Default;
    m_pSetting->Default = Default;
    m_pSetting->MinValue = Min;
    m_pSetting->MaxValue = Max;
    m_Section = Section;
    m_pSetting->szIniSection = (char*)m_Section.c_str();
    m_Entry = Entry;
    m_pSetting->szIniEntry = (char*)m_Entry.c_str();
    m_pSetting->StepValue = StepValue;
    m_pSetting->OSDDivider = 1;
    m_pSetting->pszList = NULL;
    m_pSetting->Type = SLIDER;
    
    m_pSetting->pfnOnChange = NULL; ///todo

    m_pfnOnChangeStatic = pfnOnChangeFunc;
    m_pfnOnChangeStatic_pThis = NULL;
    
    m_SettingFlags = SettingFlags;
    m_pGroup = pGroup;
}

CSimpleSetting::CSimpleSetting(SETTING *pSetting, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC *pfnOnChangeFunc, void *pThis)
{    
    m_pSetting = pSetting;
    m_bFreeSettingOnExit = FALSE;
    
    m_pGroup = pGroup;
    m_SettingFlags = SettingFlags;

    if (pfnOnChangeFunc != NULL)
    {
        m_pfnOnChangeStatic = pfnOnChangeFunc;
    }
    else
    {
        m_pfnOnChangeStatic = StaticOnChangeWrapForSettingStructure;
    }
    m_pfnOnChangeStatic_pThis = pThis;    
}

CSimpleSetting::CSimpleSetting(SETTINGEX *pSetting,CSettingGroup *pGroup)
{    
    m_pSetting = (SETTING*)pSetting;
    m_bFreeSettingOnExit = FALSE;
    
    m_pGroup = pGroup;
    m_SettingFlags = (eSettingFlags)pSetting->SettingFlags;

    m_pfnOnChangeStatic = (ONCHANGE_STATICFUNC*)pSetting->pfnOnChange;    
    m_pfnOnChangeStatic_pThis = NULL; //pThis;
}

CSimpleSetting::CSimpleSetting(SETTINGEX *pSetting, CSettingGroupList *pList)
{
    m_pSetting = (SETTING*)pSetting;
    m_bFreeSettingOnExit = FALSE;
    
    m_pGroup = NULL;
    if (pList!=NULL) 
    {
        m_pGroup = pList->Get(pSetting->pszGroupList);
    }
    m_SettingFlags = (eSettingFlags)pSetting->SettingFlags;

    m_pfnOnChangeStatic = (ONCHANGE_STATICFUNC*)pSetting->pfnOnChange;    
    m_pfnOnChangeStatic_pThis = NULL; //pThis;
}

CSimpleSetting::~CSimpleSetting()
{
    if (m_bFreeSettingOnExit)
    {
        delete m_pSetting;
        m_pSetting = NULL;
    }
}


BOOL CSimpleSetting::DoOnChange(long NewValue, long OldValue, eOnChangeType OnChangeType)
{
    long Flags = (long)m_SettingFlags;
    
    switch(OnChangeType)
    {
    case ONCHANGE_NONE: return FALSE;
    case ONCHANGE_INIT: return ((Flags & SETTINGFLAG_ONCHANGE_INIT)!=0);
    case ONCHANGE_SET:  return ( ((Flags & SETTINGFLAG_ONCHANGE_SET)!=0) && 
                                 ( ((Flags&SETTINGFLAG_ONCHANGE_VALUECHANGED)==0) ||
                                   ( ((Flags&SETTINGFLAG_ONCHANGE_VALUECHANGED)!=0) && (NewValue!=OldValue)) ));
    case ONCHANGE_SET_FORCE: return TRUE;
    case ONCHANGE_SOURCECHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_INIT)!=0);
    case ONCHANGE_VIDEOINPUTCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_VIDEOINPUT)!=0);
    case ONCHANGE_AUDIOINPUTCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_AUDIOINPUT)!=0);
    case ONCHANGE_VIDEOFORMATCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_VIDEOFORMAT)!=0);
    case ONCHANGE_CHANNELCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_CHANNEL)!=0);
    }
    return FALSE;
}
long CSimpleSetting::GetValue()
{
    return *m_pSetting->pValue;    
}

void CSimpleSetting::SetValue(long NewValue, eOnChangeType OnChangeType)
{
    long OldValue = *m_pSetting->pValue;
    if(NewValue < m_pSetting->MinValue)
    {
        NewValue = m_pSetting->MinValue;
    }
    if(NewValue > m_pSetting->MaxValue)
    {
        NewValue = m_pSetting->MaxValue;
    }    
    *m_pSetting->pValue = NewValue;
    if (DoOnChange(NewValue, OldValue, OnChangeType))
    {
        OnChange(NewValue, OldValue, OnChangeType);
    }
}

void CSimpleSetting::SetDefault(eOnChangeType OnChangeType)
{
    long OldValue = *m_pSetting->pValue;
    *m_pSetting->pValue = m_pSetting->Default;
    if (DoOnChange(*m_pSetting->pValue, OldValue, OnChangeType))
    {
        OnChange(*m_pSetting->pValue, OldValue, OnChangeType);
    }
}

long CSimpleSetting::GetDefault()
{
    return m_pSetting->Default;
}


void CSimpleSetting::SetSection(LPCSTR NewValue)
{
    m_Section = NewValue;
    m_pSetting->szIniSection = (char*)m_Section.c_str();
}

LPCSTR CSimpleSetting::GetDisplayName() 
{ 
    return m_pSetting->szDisplayName;
}

LPCSTR CSimpleSetting::GetSection() 
{ 
    return m_pSetting->szIniSection;
}

void CSimpleSetting::SetEntry(LPCSTR NewValue) 
{     
    m_Entry = NewValue; 
    m_pSetting->szIniEntry = (char*)m_Entry.c_str();
}

LPCSTR CSimpleSetting::GetEntry() 
{ 
    return m_pSetting->szIniEntry;
}


void CSimpleSetting::SetFlags(eSettingFlags SettingsFlag)
{
   if (m_SettingFlags != SettingsFlag)
   {
       
       m_SettingFlags = SettingsFlag; 
   }
}

eSettingFlags CSimpleSetting::GetFlags()
{
    return m_SettingFlags;
}


BOOL CSimpleSetting::ReadFromIni(BOOL bSetDefaultOnFailure, eOnChangeType OnChangeType)
{
    long nValue;
    BOOL IsSettingInIniFile = TRUE;

    if(m_pSetting->szIniSection != NULL)
    {        
        m_SectionFromLastValue = m_pSetting->szIniSection;
        nValue = GetPrivateProfileInt(m_pSetting->szIniSection, m_pSetting->szIniEntry, m_pSetting->MinValue-100, GetIniFileForSettings());
        if(nValue == (m_pSetting->MinValue-100))
        {
            nValue = m_pSetting->Default;
            IsSettingInIniFile = FALSE;
        }
        if(nValue < m_pSetting->MinValue)
        {
            LOG(1, "%s %s Was out of range - %d is too low", m_Section.c_str(), m_Entry.c_str(), nValue);
            nValue = m_pSetting->MinValue;
        }
        if(nValue > m_pSetting->MaxValue)
        {
            LOG(1, "%s %s Was out of range - %d is too high", m_Section.c_str(), m_Entry.c_str(), nValue);
            nValue = m_pSetting->MaxValue;
        }
        if (IsSettingInIniFile || bSetDefaultOnFailure)
        {
            int OldValue = *m_pSetting->pValue;
            *m_pSetting->pValue = nValue;
            if (DoOnChange(*m_pSetting->pValue, OldValue, OnChangeType))
            {
                OnChange(*m_pSetting->pValue, OldValue, OnChangeType);
            }
        }
        if(IsSettingInIniFile)
        {
            m_pSetting->LastSavedValue = nValue;
        }
        else
        {
            m_pSetting->LastSavedValue = m_pSetting->MinValue - 100;
        }        
    }
    else
    {
        return FALSE;
    }
    return IsSettingInIniFile;
}

LPCSTR CSimpleSetting::GetLastValueSection()
{
    return m_SectionFromLastValue.c_str();
}

void CSimpleSetting::WriteToIni(BOOL bOptimizeFileAccess)
{
    if(m_pSetting->szIniSection != NULL)
    {
        if(!bOptimizeFileAccess || ((*m_pSetting->pValue) != m_pSetting->LastSavedValue))
        {
	        WritePrivateProfileInt(m_pSetting->szIniSection, m_pSetting->szIniEntry, *m_pSetting->pValue, GetIniFileForSettings());
            m_pSetting->LastSavedValue = *m_pSetting->pValue;
        }
    }
}
 
void CSimpleSetting::ChangeDefault(long NewDefault, BOOL bDontSetValue, eOnChangeType OnChangeType)
{
    m_pSetting->Default = NewDefault;
    if (!bDontSetValue)    
    {
        SetValue(NewDefault, OnChangeType);
    }
}

CSimpleSetting::operator long()
{
    return *m_pSetting->pValue;    
}
 
void CSimpleSetting::OSDShow()
{
    char szBuffer[1024] = "Unexpected Display Error";

    // call to virtual get display string function
    GetDisplayText(szBuffer);

    OSD_ShowText(hWnd, szBuffer, 0);
    
}

void CSimpleSetting::Up(eOnChangeType OnChangeType)
{
    if ((*m_pSetting->pValue) < m_pSetting->MaxValue)
    {
        int nStep = GetCurrentAdjustmentStepCount(this) * m_pSetting->StepValue;
        SetValue(*m_pSetting->pValue + nStep, OnChangeType);
    }
}

void CSimpleSetting::Down(eOnChangeType OnChangeType)
{
    if ((*m_pSetting->pValue) > m_pSetting->MinValue)
    {
        int nStep = GetCurrentAdjustmentStepCount(this) * m_pSetting->StepValue;
        SetValue(*m_pSetting->pValue - nStep, OnChangeType);
    }
    
}
 
void CSimpleSetting::ChangeValue(eCHANGEVALUE NewValue, eOnChangeType OnChangeType)
{
    switch(NewValue)
    {
    case DISPLAY:
        OSDShow();
        break;
    case ADJUSTUP:
        Up(OnChangeType);
        OSDShow();
        break;
    case ADJUSTDOWN:
        Down(OnChangeType);
        OSDShow();
        break;
    case INCREMENT:
        SetValue((*m_pSetting->pValue) + m_pSetting->StepValue, OnChangeType);
        OSDShow();
        break;
    case DECREMENT:
        SetValue((*m_pSetting->pValue) - m_pSetting->StepValue, OnChangeType);
        OSDShow();
        break;
    case RESET:
        SetDefault(OnChangeType);
        OSDShow();
        break;
    case TOGGLEBOOL:
        if(GetType() == YESNO || GetType() == ONOFF)
        {
            SetValue(!(*m_pSetting->pValue),OnChangeType);
            OSDShow();
        }
        break;
    case ADJUSTUP_SILENT:
        Up(OnChangeType);
        break;
    case ADJUSTDOWN_SILENT:
        Down(OnChangeType);
        break;
    case INCREMENT_SILENT:
        SetValue((*m_pSetting->pValue) + m_pSetting->StepValue, OnChangeType);
        break;
    case DECREMENT_SILENT:
        SetValue((*m_pSetting->pValue) - m_pSetting->StepValue, OnChangeType);
        break;
    case RESET_SILENT:
        SetDefault(OnChangeType);
        break;
    case TOGGLEBOOL_SILENT:
        if(GetType() == YESNO || GetType() == ONOFF)
        {
            SetValue(!(*m_pSetting->pValue), OnChangeType);
        }
        break;
    default:
        break;
    }
    
}

void CSimpleSetting::SetStepValue(long Step)
{
    m_pSetting->StepValue = Step;
}

void CSimpleSetting::SetMin(long Min)
{
    m_pSetting->MinValue = Min;
}
 
void CSimpleSetting::SetMax(long Max)
{
    m_pSetting->MaxValue = Max;
}

long CSimpleSetting::GetMin()
{
    return m_pSetting->MinValue;
}

long CSimpleSetting::GetMax()
{
    return m_pSetting->MaxValue;
}

void CSimpleSetting::SetGroup(CSettingGroup* pGroup)
{
    m_pGroup = pGroup;
}

CSettingGroup* CSimpleSetting::GetGroup()
{
    return m_pGroup;
}

void CSimpleSetting::OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType)
{
    //No override, try static
    if ((m_pfnOnChangeStatic!=NULL) && (OnChangeType!=ONCHANGE_NONE))
    {
        m_pfnOnChangeStatic(m_pfnOnChangeStatic_pThis, NewValue, OldValue, OnChangeType, m_pSetting);
    }
}

CListSetting::CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC pfnOnChangeFunc, void *pThis) :
    CSimpleSetting(DisplayName, Default, 0, Max, Section, Entry, 1, pGroup, SettingFlags, pfnOnChangeFunc, pThis)    
{
    m_pSetting->Type = ITEMFROMLIST;
    m_pSetting->pszList = pszList;
}

CListSetting::CListSetting(SETTING *pSetting, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC *pfnOnChangeFunc, void *pThis) : CSimpleSetting(pSetting, pGroup, SettingFlags, pfnOnChangeFunc, pThis)
{   
}

CListSetting::CListSetting(SETTINGEX *pSetting, CSettingGroup *pGroup) : CSimpleSetting(pSetting, pGroup)
{ 
}

CListSetting::CListSetting(SETTINGEX *pSetting, CSettingGroupList *pGroupList) : CSimpleSetting(pSetting, pGroupList)
{ 
}

CListSetting::~CListSetting()
{
}

void CListSetting::GetDisplayText(LPSTR szBuffer)
{
    char *szName = m_pSetting->szDisplayName;
    if (szName == NULL)
    {
        szName = m_pSetting->szIniEntry;
    }

    if(m_pSetting->pszList != NULL)
    {
        
        sprintf(szBuffer, "%s %s", szName, m_pSetting->pszList[*m_pSetting->pValue]);
    }
    else
    {
        sprintf(szBuffer, "%s %d", szName, *m_pSetting->pValue);
    }
}

void CListSetting::SetupControl(HWND hWnd)
{
}

void CListSetting::SetControlValue(HWND hWnd)
{
    ComboBox_SetCurSel(hWnd, *m_pSetting->pValue);
}

void CListSetting::SetFromControl(HWND hWnd, eOnChangeType OnChangeType)
{
    SetValue(ComboBox_GetCurSel(hWnd), OnChangeType);
}


CSliderSetting::CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC pfnOnChangeFunc, void *pThis) :
    CSimpleSetting(DisplayName, Default, Min, Max, Section, Entry, 1, pGroup, SettingFlags, pfnOnChangeFunc, pThis)
{
    m_pSetting->OSDDivider = 1;
    m_pSetting->Type = SLIDER;
}

CSliderSetting::CSliderSetting(SETTING *pSetting, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC *pfnOnChangeFunc, void *pThis) : CSimpleSetting(pSetting, pGroup, SettingFlags, pfnOnChangeFunc, pThis)
{    
}

CSliderSetting::CSliderSetting(SETTINGEX *pSetting, CSettingGroup *pGroup) : CSimpleSetting(pSetting, pGroup)
{ 
}


CSliderSetting::CSliderSetting(SETTINGEX *pSetting, CSettingGroupList *pGroupList) : CSimpleSetting(pSetting, pGroupList)
{ 
}


CSliderSetting::~CSliderSetting()
{
}

void CSliderSetting::SetOSDDivider(long OSDDivider)
{
    m_pSetting->OSDDivider = OSDDivider;
}


void CSliderSetting::GetDisplayText(LPSTR szBuffer)
{
    char *szName = m_pSetting->szDisplayName;
    if (szName == NULL)
    {
        szName = m_pSetting->szIniEntry;
    }

    if(m_pSetting->OSDDivider == 1)
    {
        sprintf(szBuffer, "%s %d",szName, *m_pSetting->pValue);
    }
    else if(m_pSetting->OSDDivider == 8)
    {
        sprintf(szBuffer, "%s %.3f", szName, (float)(*m_pSetting->pValue) / (float)m_pSetting->OSDDivider);
    }
    else
    {
        sprintf(szBuffer, "%s %.*f", szName, (int)log10(m_pSetting->OSDDivider), (float)(*m_pSetting->pValue) / (float)m_pSetting->OSDDivider);
    }
}

void CSliderSetting::SetupControl(HWND hWnd)
{
    Slider_SetRangeMin(hWnd, 0);
    Slider_SetPageSize(hWnd, m_pSetting->StepValue);
    Slider_SetLineSize(hWnd, m_pSetting->StepValue);
    SetControlValue(hWnd);
}

void CSliderSetting::SetControlValue(HWND hWnd)
{
    Slider_ClearTicks(hWnd, TRUE);
    Slider_SetRangeMax(hWnd, m_pSetting->MaxValue - m_pSetting->MinValue);
    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetTic(hWnd, m_pSetting->MaxValue - m_pSetting->Default);
    }
    else
    {
        Slider_SetTic(hWnd, m_pSetting->Default - m_pSetting->MinValue);
    }

    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetPos(hWnd, m_pSetting->MaxValue - (*m_pSetting->pValue));
    }
    else
    {
        Slider_SetPos(hWnd, (*m_pSetting->pValue) - m_pSetting->MinValue);
    }
}

void CSliderSetting::SetFromControl(HWND hWnd, eOnChangeType OnChangeType)
{
    long nValue = Slider_GetPos(hWnd);
    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        nValue = m_pSetting->MaxValue - nValue;
    }
    else
    {
        nValue = nValue + m_pSetting->MinValue;
    }
    SetValue(nValue, OnChangeType);
}



CYesNoSetting::CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC pfnOnChangeFunc, void *pThis) :
    CSimpleSetting(DisplayName, Default, 0, 1, Section, Entry, 1, pGroup, SettingFlags, pfnOnChangeFunc, pThis)
{
    m_pSetting->Type = YESNO;
}

CYesNoSetting::CYesNoSetting(SETTING *pSetting, CSettingGroup *pGroup, eSettingFlags SettingFlags, ONCHANGE_STATICFUNC *pfnOnChangeFunc, void *pThis) : CSimpleSetting(pSetting, pGroup, SettingFlags, pfnOnChangeFunc, pThis)
{  
}

CYesNoSetting::CYesNoSetting(SETTINGEX *pSetting, CSettingGroup *pGroup) : CSimpleSetting(pSetting, pGroup)
{    
}

CYesNoSetting::CYesNoSetting(SETTINGEX *pSetting, CSettingGroupList *pGroupList) : CSimpleSetting(pSetting, pGroupList)
{    
}


CYesNoSetting::~CYesNoSetting()
{
}

void CYesNoSetting::GetDisplayText(LPSTR szBuffer)
{
    char *szName = m_pSetting->szDisplayName;
    if (szName == NULL)
    {
        szName = m_pSetting->szIniEntry;
    }
    sprintf(szBuffer, "%s %s", szName, (*m_pSetting->pValue)?"YES":"NO");
}

void CYesNoSetting::SetupControl(HWND hWnd)
{
}

void CYesNoSetting::SetControlValue(HWND hWnd)
{
    Button_SetCheck(hWnd, (*m_pSetting->pValue));
}

void CYesNoSetting::SetFromControl(HWND hWnd, eOnChangeType OnChangeType)
{
    SetValue(Button_GetCheck(hWnd) == BST_CHECKED, OnChangeType);
}

BOOL CSimpleSetting::OnChangeWrapForSettingStructure(long NewValue, long OldValue, eOnChangeType OnChangeType)
{
   if (m_pSetting->pfnOnChange!=NULL)
   {        
       return m_pSetting->pfnOnChange(NewValue);
   }
   return FALSE;
}

BOOL CSimpleSetting::StaticOnChangeWrapForSettingStructure(void *pThis, long NewValue, long OldValue, eOnChangeType OnChangeType, SETTING *pSetting)
{
   if (pThis != NULL)
   {
       return ((CSimpleSetting*)pThis)->OnChangeWrapForSettingStructure(NewValue, OldValue, OnChangeType);
   }
   return FALSE;
}

CSettingGroup::CSettingGroup(LPCSTR szGroupName, LPCSTR szLongName)
{
    m_sGroupName = szGroupName;
    m_sDisplayName = szLongName;
}

CSettingGroup::~CSettingGroup()
{
}

void CSettingGroup::SetLongName(LPCSTR szLongName)
{
    m_sDisplayName = szLongName;
}
LPCSTR CSettingGroup::GetName() 
{
    return m_sGroupName.c_str();
}
LPCSTR CSettingGroup::GetLongName()
{
    return m_sDisplayName.c_str();
}


CSettingGroupList::CSettingGroupList()
{    
}

CSettingGroupList::~CSettingGroupList()
{    
}

CSettingGroup* CSettingGroupList::Get(char **pszList)
{
    char *szGroupName = NULL;
    int i;

    if (pszList != NULL) 
    {
        i = 0;
        while (pszList[i]!=NULL) 
        { 
            szGroupName = pszList[i]; 
            i++; 
        }
    }
    if (szGroupName == NULL) { return NULL; }
    for (i = 0; i < m_GroupList.size(); i++)
    {
        if (!strcmp(m_GroupList[i]->GetName(),szGroupName)) { break; }
    }
    if (i>=m_GroupList.size())
    {
        CSettingGroup *pGroup = new CSettingGroup(szGroupName, NULL);
        m_GroupList.push_back(pGroup);
    }
    return m_GroupList[i];
}

CSettingsMaster::CSettingsMaster(CEventCollector *pEventCollector) : 
m_SettingGroupList(NULL)
{
    eEventType EventList[] = {
        EVENT_INIT,
        EVENT_DESTROY,
        EVENT_SOURCE_PRECHANGE,
        EVENT_SOURCE_CHANGE,
        EVENT_VIDEOINPUT_PRECHANGE,
        EVENT_VIDEOINPUT_CHANGE,
        EVENT_AUDIOINPUT_PRECHANGE,
        EVENT_AUDIOINPUT_CHANGE,
        EVENT_VIDEOFORMAT_PRECHANGE,
        EVENT_VIDEOFORMAT_CHANGE,
        EVENT_CHANNEL_PRECHANGE,
        EVENT_CHANNEL_CHANGE,
        EVENT_ENDOFLIST
    };
    pEventCollector->Register(this, EventList);
}

CSettingsMaster::~CSettingsMaster()
{
}

CSettingGroupList* CSettingsMaster::Groups()
{
    if (m_SettingGroupList == NULL)
    {
        m_SettingGroupList = new CSettingGroupList;
    }    
    return m_SettingGroupList;
}


void CSettingsMaster::ReadFromIni(BOOL bInit)
{
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if (m_Holders[i].pHolder != NULL)
        {
            m_Holders[i].pHolder->ReadFromIni(bInit);
        }
    }
}

void CSettingsMaster::WriteToIni(BOOL bOptimizeFileAccess)
{
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if (m_Holders[i].pHolder != NULL)
        {
            m_Holders[i].pHolder->WriteToIni(bOptimizeFileAccess);
        }
    }
}


void CSettingsMaster::ModifyOneSetting(ISetting *pSetting, int What, eOnChangeType OnChangeType)
{
    if (What==1)
    {                
        /*if (strcmp(pSetting->GetLastValueSection(),pSetting->GetEntry())
        {            
            pSetting->WriteToIni(TRUE);
        }
        else*/
        {
            pSetting->WriteToIni(FALSE);
        }
    }
    else if (What==0)
    {        
        pSetting->ReadFromIni(TRUE, OnChangeType);
    }
}

void CSettingsMaster::ParseAllSettings(int What, eOnChangeType OnChangeType)
{
    for (int i = 0; i < m_Holders.size(); i++)
    {
        int Num = m_Holders[i].pHolder->GetNumSettings();
        CSimpleSetting *pSetting;
        eSettingFlags SettingFlags;
        string sSection;

        for (int n = 0; n < Num; n++)
        {
            pSetting = (CSimpleSetting*)m_Holders[i].pHolder->GetSetting(n);
            if (pSetting != NULL)
            {
                SettingFlags = pSetting->GetFlags();
                      
                if ((SettingFlags&SETTINGFLAG_PER_SOURCE) && (m_SourceName.length()>0))
                {
                    sSection+=m_SourceName+"_";
                }
                else if ((SettingFlags&SETTINGFLAG_PER_VIDEOINPUT) && (m_VideoInputName.length()>0))
                {
                    sSection+=m_VideoInputName+"_";
                }
                else if ((SettingFlags&SETTINGFLAG_PER_AUDIOINPUT) && (m_AudioInputName.length()>0))
                {
                    sSection+=m_AudioInputName+"_";
                }
                else if ((SettingFlags&SETTINGFLAG_PER_VIDEOFORMAT) && (m_VideoFormatName.length()>0))
                {
                    sSection+=m_VideoFormatName+"_";
                }
                else if ((SettingFlags&SETTINGFLAG_PER_CHANNEL) && (m_ChannelName.length()>0))
                {
                    sSection+=m_ChannelName+"_";
                }
                if (sSection.length() > 0)
                {
                    string sOldEntry = pSetting->GetEntry();
                    string sOldSection = pSetting->GetSection();
                    
                    string sNewSection = sSection;
                    string sNewEntry   = sOldSection+"_"+sOldEntry;

                    pSetting->SetEntry(sNewEntry.c_str());
                    pSetting->SetSection(sNewSection.c_str());

                    ModifyOneSetting(pSetting, What, OnChangeType);

                    pSetting->SetEntry(sOldEntry.c_str());
                    pSetting->SetSection(sOldSection.c_str());
                } else
                {
                    if (SettingFlags&SETTINGFLAG_GLOBAL)
                    {
                        ModifyOneSetting(pSetting, What, OnChangeType);
                    }
                }
            }
        }
    }
}

void CSettingsMaster::LoadSettings(eOnChangeType OnChangeType)
{
    ParseAllSettings(0, OnChangeType);
}


void CSettingsMaster::SaveSettings(eOnChangeType OnChangeType)
{
    ParseAllSettings(1, OnChangeType);
}

void CSettingsMaster::Register(SETTINGHOLDERID HolderID, CSettingsHolder *pHolder)
{
    Unregister(pHolder);
    TSettingsHolderInfo shi;    
    shi.HolderID = HolderID;
    shi.pHolder = pHolder;
    m_Holders.push_back(shi);
}

void CSettingsMaster::Unregister(SETTINGHOLDERID HolderID)
{
    vector<TSettingsHolderInfo> NewList;
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if (m_Holders[i].HolderID == HolderID)
        {
            //
        } 
        else 
        {
            NewList.push_back(m_Holders[i]);
        }
    }
    m_Holders = NewList;
}
void CSettingsMaster::Unregister(CSettingsHolder *pHolder)
{
    vector<TSettingsHolderInfo> NewList;
    for (int i = 0; i < m_Holders.size(); i++)
    {
        if (m_Holders[i].pHolder == pHolder)
        {
            //
        } 
        else 
        {
            NewList.push_back(m_Holders[i]);
        }
    }
    m_Holders = NewList;
}

void CSettingsMaster::OnEvent(eEventType Event, long OldValue, long NewValue, eEventType *ComingUp)
{
    switch (Event)
    {
    case EVENT_SOURCE_PRECHANGE:    
        if (m_SourceName.length()>0) 
        { 
            SaveSettings(ONCHANGE_SOURCECHANGE);
        }
        break;
    
    case EVENT_SOURCE_CHANGE:    
        if (NewValue!=0)
        {
            m_SourceName = ((CSource*)NewValue)->IDString();
            LoadSettings(ONCHANGE_SOURCECHANGE);
        }
        else
        {
            m_SourceName = "";
        }
        break;
     
    case EVENT_CHANNEL_PRECHANGE:   
        if (m_ChannelName.length()>0) 
        { 
            SaveSettings(ONCHANGE_CHANNELCHANGE); 
        }
        break;
    
    case EVENT_CHANNEL_CHANGE:
        if (NewValue>=0)
        {        
            char szBuffer[33];
            m_ChannelName = string("Channel") + itoa(NewValue, szBuffer, 10);
            LoadSettings(ONCHANGE_CHANNELCHANGE);
        }
        else
        {
            m_ChannelName = "";
        }
        break;
    
    case EVENT_VIDEOINPUT_PRECHANGE:
        if (m_VideoInputName.length()>0) 
        { 
            SaveSettings(ONCHANGE_VIDEOINPUTCHANGE); 
        }
        break;

    case EVENT_VIDEOINPUT_CHANGE:
        if (NewValue>=0)
        {
            char szBuffer[33];
            m_VideoInputName = string("VideoInput") +  itoa(NewValue, szBuffer, 10);
            LoadSettings(ONCHANGE_VIDEOINPUTCHANGE);
        }
        else
        {
            m_VideoInputName = "";
        }
        break;    

    case EVENT_AUDIOINPUT_PRECHANGE:
        if (m_AudioInputName.length()>0) 
        { 
            SaveSettings(ONCHANGE_AUDIOINPUTCHANGE); 
        }
        break;

    case EVENT_AUDIOINPUT_CHANGE:
        if (NewValue>=0)
        {
            char szBuffer[33];
            m_AudioInputName = string("AudioInput") +  itoa(NewValue, szBuffer, 10);
            LoadSettings(ONCHANGE_AUDIOINPUTCHANGE);
        }
        else
        {
            m_AudioInputName = "";
        }
        break;
    case EVENT_VIDEOFORMAT_PRECHANGE:
        if (m_VideoFormatName.length()>0) 
        { 
            SaveSettings(ONCHANGE_VIDEOFORMATCHANGE); 
        }
        break;

    case EVENT_VIDEOFORMAT_CHANGE:
        if (NewValue>=0)
        {
            char szBuffer[33];
            m_VideoFormatName = string("VideoFormat") +  itoa(NewValue, szBuffer, 10);            
            LoadSettings(ONCHANGE_VIDEOFORMATCHANGE);
        }
        else
        {
            m_VideoFormatName = "";
        }
        break;
    }    
}

