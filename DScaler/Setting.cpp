/////////////////////////////////////////////////////////////////////////////
// $Id: Setting.cpp,v 1.5 2001-11-29 14:04:07 adcockj Exp $
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
        (*it)->WriteToIni();
        delete *it;
    }
}

void CSettingsHolder::ReadFromIni()
{
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        (*it)->ReadFromIni();
    }
}

void CSettingsHolder::WriteToIni()
{
    for(vector<ISetting*>::iterator it = m_Settings.begin();
        it != m_Settings.end();
        ++it)
    {
        (*it)->WriteToIni();
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
    if(wParam > 0 && wParam < m_Settings.size())
    {
        if(message == m_SetMessage)
        {
            RetVal = m_Settings[wParam]->GetValue();
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 100)
        {
            m_Settings[wParam]->SetValue(lParam);
            *bHandled = TRUE;
        }
        else if(message == m_SetMessage + 200)
        {
            m_Settings[wParam]->ChangeValue((eCHANGEVALUE)lParam);
            *bHandled = TRUE;
        }
    }
    return RetVal;
}

CSimpleSetting::CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, long StepValue)
{
    m_DisplayName = DisplayName;
    m_Value = Default;
    m_Default = Default;
    m_Min = Min;
    m_Max = Max;
    m_Section = Section;
    m_Entry = Entry;
    m_StepValue = StepValue;
}

CSimpleSetting::~CSimpleSetting()
{
}

long CSimpleSetting::GetValue()
{
    return m_Value;    
}

void CSimpleSetting::SetValue(long NewValue)
{
    long OldValue = m_Value;
    if(NewValue < m_Min)
    {
        NewValue = m_Min;
    }
    if(NewValue > m_Max)
    {
        NewValue = m_Max;
    }
    m_Value = NewValue;
    OnChange(NewValue, OldValue);
}

void CSimpleSetting::SetDefault()
{
    long OldValue = m_Value;
    m_Value = m_Default;
    OnChange(m_Default, OldValue);
}

long CSimpleSetting::GetDefault()
{
    return m_Default;
}


void CSimpleSetting::SetSection(LPCSTR NewValue)
{
    m_Section = NewValue;
}

void CSimpleSetting::ReadFromIni()
{
    long nValue;

    if(!m_Section.empty())
    {
        nValue = GetPrivateProfileInt(m_Section.c_str(), m_Entry.c_str(), m_Min - 100, GetIniFileForSettings());
        if(nValue == m_Min - 100)
        {
            nValue = m_Default;
        }
        if(nValue < m_Min)
        {
            LOG(1, "%s %s Was out of range - %d is too low", m_Section.c_str(), m_Entry.c_str(), nValue);
            nValue = m_Min;
        }
        if(nValue > m_Max)
        {
            LOG(1, "%s %s Was out of range - %d is too high", m_Section.c_str(), m_Entry.c_str(), nValue);
            nValue = m_Max;
        }
        m_Value = nValue;
    }
}

void CSimpleSetting::WriteToIni()
{
    if(!m_Section.empty())
    {
	    WritePrivateProfileInt(m_Section.c_str(), m_Entry.c_str(), m_Value, GetIniFileForSettings());
    }
}
 
void CSimpleSetting::ChangeDefault(long NewDefault)
{
    m_Default = NewDefault;
    m_Value = NewDefault;
}

CSimpleSetting::operator long()
{
    return m_Value;    
}
 
void CSimpleSetting::OSDShow()
{
    char szBuffer[1024] = "Unexpected Display Error";

    // call to virtual get display string function
    GetDisplayText(szBuffer);

    OSD_ShowText(hWnd, szBuffer, 0);
    
}

void CSimpleSetting::Up()
{
    if (m_Value < m_Max)
    {
        int nStep = GetCurrentAdjustmentStepCount(this) * m_StepValue;
        SetValue(m_Value + nStep);
    }
}

void CSimpleSetting::Down()
{
    if (m_Value > m_Min)
    {
        int nStep = GetCurrentAdjustmentStepCount(this) * m_StepValue;
        SetValue(m_Value - nStep);
    }
    
}
 
void CSimpleSetting::ChangeValue(eCHANGEVALUE NewValue)
{
    switch(NewValue)
    {
    case DISPLAY:
        OSDShow();
        break;
    case ADJUSTUP:
        Up();
        OSDShow();
        break;
    case ADJUSTDOWN:
        Down();
        OSDShow();
        break;
    case INCREMENT:
        SetValue(m_Value + m_StepValue);
        OSDShow();
        break;
    case DECREMENT:
        SetValue(m_Value - m_StepValue);
        OSDShow();
        break;
    case RESET:
        SetDefault();
        OSDShow();
        break;
    case TOGGLEBOOL:
        if(GetType() == YESNO || GetType() == ONOFF)
        {
            SetValue(!m_Value);
            OSDShow();
        }
        break;
    case ADJUSTUP_SILENT:
        Up();
        break;
    case ADJUSTDOWN_SILENT:
        Down();
        break;
    case INCREMENT_SILENT:
        SetValue(m_Value + m_StepValue);
        break;
    case DECREMENT_SILENT:
        SetValue(m_Value - m_StepValue);
        break;
    case RESET_SILENT:
        SetDefault();
        break;
    case TOGGLEBOOL_SILENT:
        if(GetType() == YESNO || GetType() == ONOFF)
        {
            SetValue(!m_Value);
        }
        break;
    default:
        break;
    }
    
}

void CSimpleSetting::SetStepValue(long Step)
{
    m_StepValue = Step;
}

void CSimpleSetting::SetMin(long Min)
{
    m_Min = Min;
}
 
void CSimpleSetting::SetMax(long Max)
{
    m_Max = Max;
}

long CSimpleSetting::GetMin()
{
    return m_Min;
}

long CSimpleSetting::GetMax()
{
    return m_Max;
}

CListSetting::CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList) :
    CSimpleSetting(DisplayName, Default, 0, Max, Section, Entry, 1),
    m_List(pszList)
{
}

CListSetting::~CListSetting()
{
}

void CListSetting::GetDisplayText(LPSTR szBuffer)
{
    if(m_List != NULL)
    {
        sprintf(szBuffer, "%s %s", m_DisplayName.c_str(), m_List[m_Value]);
    }
    else
    {
        sprintf(szBuffer, "%s %d", m_DisplayName.c_str(), m_Value);
    }
}

void CListSetting::SetupControl(HWND hWnd)
{
}

void CListSetting::SetControlValue(HWND hWnd)
{
    ComboBox_SetCurSel(hWnd, m_Value);
}

void CListSetting::SetFromControl(HWND hWnd)
{
    SetValue(ComboBox_GetCurSel(hWnd));
}


CSliderSetting::CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry) :
    CSimpleSetting(DisplayName, Default, Min, Max, Section, Entry, 1),
    m_OSDDivider(1)
{
}


CSliderSetting::~CSliderSetting()
{
}

void CSliderSetting::SetOSDDivider(long OSDDivider)
{
    m_OSDDivider = OSDDivider;
}


void CSliderSetting::GetDisplayText(LPSTR szBuffer)
{
    if(m_OSDDivider == 1)
    {
        sprintf(szBuffer, "%s %d", m_DisplayName.c_str(), m_Value);
    }
    else if(m_OSDDivider == 8)
    {
        sprintf(szBuffer, "%s %.3f", m_DisplayName.c_str(), (float)m_Value / (float)m_OSDDivider);
    }
    else
    {
        sprintf(szBuffer, "%s %.*f", m_DisplayName.c_str(), (int)log10(m_OSDDivider), (float)m_Value / (float)m_OSDDivider);
    }
}

void CSliderSetting::SetupControl(HWND hWnd)
{
    Slider_ClearTicks(hWnd, TRUE);
    Slider_SetRangeMax(hWnd, m_Max - m_Min);
    Slider_SetRangeMin(hWnd, 0);
    Slider_SetPageSize(hWnd, m_StepValue);
    Slider_SetLineSize(hWnd, m_StepValue);
    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetTic(hWnd, m_Max - m_Default);
    }
    else
    {
        Slider_SetTic(hWnd, m_Default - m_Min);
    }
    SetControlValue(hWnd);
}

void CSliderSetting::SetControlValue(HWND hWnd)
{
    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetPos(hWnd, m_Max - m_Value);
    }
    else
    {
        Slider_SetPos(hWnd, m_Value - m_Min);
    }
}

void CSliderSetting::SetFromControl(HWND hWnd)
{
    long nValue = Slider_GetPos(hWnd);
    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        nValue = m_Max - nValue;
    }
    else
    {
        nValue = nValue + m_Min;
    }
    SetValue(nValue);
}



CYesNoSetting::CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry) :
    CSimpleSetting(DisplayName, Default, 0, 1, Section, Entry, 1)
{
}

CYesNoSetting::~CYesNoSetting()
{
}

void CYesNoSetting::GetDisplayText(LPSTR szBuffer)
{
    sprintf(szBuffer, "%s %s", m_DisplayName.c_str(), m_Value?"YES":"NO");
}

void CYesNoSetting::SetupControl(HWND hWnd)
{
}

void CYesNoSetting::SetControlValue(HWND hWnd)
{
    Button_SetCheck(hWnd, m_Value);
}

void CYesNoSetting::SetFromControl(HWND hWnd)
{
    SetValue(Button_GetCheck(hWnd) == BST_CHECKED);
}
