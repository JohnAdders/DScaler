////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file Setting.cpp Settings functions
 */

#include "stdafx.h"
#include <Wininet.h.>
#include "Setting.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Settings.h"
#include "DebugLog.h"
#include "OSD.h"
#include "DScaler.h"
#include "Slider.h"

using namespace std;

/** Internal flags
*/

tstring GetWindowClass(HWND hWnd)
{
    vector<TCHAR> ClassName(256);
    GetClassName(hWnd,&ClassName[0],255);
    return &ClassName[0];
}

BOOL IsWindowClass(HWND hWnd, const tstring& ClassName)
{
    return AreEqualInsensitive(GetWindowClass(hWnd), ClassName);
}

void SliderControlSetup(HWND hWnd, long StepValue)
{
    Slider_SetRangeMin(hWnd, 0);
    Slider_SetPageSize(hWnd, StepValue);
    Slider_SetLineSize(hWnd, StepValue);
}

void SliderControlSetValue(HWND hWnd, long Value, long MinValue, long MaxValue, long Default)
{
    Slider_ClearTicks(hWnd, TRUE);
    Slider_SetRangeMax(hWnd, MaxValue - MinValue);
    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetTic(hWnd, MaxValue - Default);
    }
    else
    {
        Slider_SetTic(hWnd, Default - MinValue);
    }

    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetPos(hWnd, MaxValue - Value);
    }
    else
    {
        Slider_SetPos(hWnd, Value - MinValue);
    }
}

long SliderControlGetValue(HWND hWnd, long MinValue, long MaxValue, long StepValue)
{
    long nValue = Slider_GetPos(hWnd);
    if(GetWindowLong(hWnd, GWL_STYLE) & TBS_VERT)
    {
        nValue = MaxValue - nValue;
    }
    else
    {
        nValue = nValue + MinValue;
    }
    // make sure the value is only a multiple of the step size.
    if(StepValue != 1)
    {
        nValue = nValue - (nValue % StepValue);
    }
    return nValue;
}

void ComboBoxControlSetup(HWND hWnd, long NumItems, const char** szList)
{
    ComboBox_ResetContent(hWnd);

    if(szList != NULL)
    {
        for(int i(0); i < NumItems; ++i)
        {
            //is there any text for this item?
            if ( (szList[i] != NULL) && (*szList[i] != '\0') )
            {
                int Pos = SendMessageA(hWnd, CB_ADDSTRING, 0, (LPARAM)szList[i]);

                //store Value in itemdata
                ComboBox_SetItemData(hWnd, Pos, i);
            }
        }
    }
}

void ComboBoxSetValue(HWND hWnd, long Value)
{
    long NumItems ComboBox_GetCount(hWnd);
    for(int i(0); i < NumItems; ++i)
    {
        if(ComboBox_GetItemData(hWnd, i) == Value)
        {
            ComboBox_SetCurSel(hWnd, i);
            return;
        }
    }
}

long ComboBoxGetValue(HWND hWnd)
{
    return ComboBox_GetItemData(hWnd, ComboBox_GetCurSel(hWnd));
}


/** Constructor

    Specify setting parameters:
    Display name, default value, minimum value, maximum value,
    default ini section, ini entry, step value,
    (optional:)
    Setting group, setting flags,
    GUI info, static callback function, pointer for callback function

*/
CSimpleSetting::CSimpleSetting(const tstring& DisplayName, const tstring& Section, const tstring& Entry, CSettingGroup* pGroup) :
    m_DisplayName(DisplayName),
    m_Section(Section),
    m_Entry(Entry),
    m_ReadWriteFlags(0),
    m_pGroup(pGroup),
    m_EnableOnChange(FALSE)
{
}

CSimpleSetting::~CSimpleSetting()
{
}


BOOL CSimpleSetting::IsOnChangeEnabled()
{
    return m_EnableOnChange;
}

void CSimpleSetting::EnableOnChange()
{
    m_EnableOnChange = TRUE;
}

void CSimpleSetting::DisableOnChange()
{
    m_EnableOnChange = FALSE;
}

void CSimpleSetting::SetSection(const tstring& NewValue)
{
    m_Section = NewValue;
}

const tstring& CSimpleSetting::GetDisplayName()
{
    return m_DisplayName;
}

const tstring& CSimpleSetting::GetSection()
{
    return m_Section;
}

void CSimpleSetting::SetEntry(const tstring& NewValue)
{
    m_Entry = NewValue;
}

const tstring& CSimpleSetting::GetEntry()
{
    return m_Entry;
}


/** Read value from sub section in .ini file
    @param szSubSection Set to NULL to read from the default location
    @param bSetDefaultOnFailure If the setting was not in the .ini file, set the setting's value to the default value
    @param pSettingFlags Override setting flags of current setting if not NULL
    @return TRUE if value was in .ini file
*/
BOOL CSimpleSetting::ReadFromIniSubSection(const tstring& SubSection)
{
    BOOL IsSettingInIniFile = TRUE;

    if(!SubSection.empty())
    {
        tstring sEntry;

        if(SubSection != m_Section)
        {
            sEntry = m_Section;
            sEntry += _T("_");
            sEntry += m_Entry;
        }
        else
        {
            sEntry = m_Entry;
        }

        tstring szDefaultString;
        vector<TCHAR> szBuffer(256);

        int Len = GetPrivateProfileString(SubSection.c_str(), sEntry.c_str(), _T(""), &szBuffer[0], 255, GetIniFileForSettings());
        LOG(2, _T(" ReadFromIniSubSection %s %s Result %s"), SubSection.c_str(), sEntry.c_str(), &szBuffer[0]);

        if (Len <= 0)
        {
            IsSettingInIniFile = FALSE;
            ChangeValueInternal(RESET_SILENT);
            m_LastSavedValueIniSection = _T("");
            m_LastSavedValue = _T("");
        }
        else
        {
            IsSettingInIniFile = TRUE;
            SetValueFromString(&szBuffer[0]);
            m_LastSavedValue = GetValueAsString();
            m_LastSavedValueIniSection = SubSection;
        }
    }
    else
    {
        m_LastSavedValueIniSection = _T("");
        m_LastSavedValue = _T("");
        IsSettingInIniFile = FALSE;
    }
    return IsSettingInIniFile;
}

/** Read value from default location in .ini file
*/
BOOL CSimpleSetting::ReadFromIni()
{
    return ReadFromIniSubSection(m_Section.c_str());
}

/** Write value to szSubsection in .ini file
    Override value and setting flags if Value and/or pSettingFlags is not NULL.
*/
void CSimpleSetting::WriteToIniSubSection(const tstring& SubSection, BOOL bOptimizeFileAccess)
{
    if(!SubSection.empty())
    {
        tstring sEntry;

        if(SubSection != m_Section)
        {
            sEntry = m_Section;
            sEntry += _T("_");
            sEntry += m_Entry;
        }
        else
        {
            sEntry = m_Entry;
        }

        tstring CurrentValue(GetValueAsString());

        if(!bOptimizeFileAccess || m_LastSavedValue != CurrentValue || m_LastSavedValueIniSection != SubSection)
        {
            WritePrivateProfileString(SubSection.c_str(), sEntry.c_str(), CurrentValue.c_str(), GetIniFileForSettings());
            LOG(2, _T(" WriteToIniSubSection %s %s Value %s Was %s"), SubSection.c_str(), sEntry.c_str(), CurrentValue.c_str(), m_LastSavedValue.c_str());
        }
        else
        {
            LOG(2, _T(" WriteToIniSubSection Not Written %s %s Value %s"), SubSection.c_str(), sEntry.c_str(), CurrentValue.c_str());
        }

        m_LastSavedValue = CurrentValue;
        m_LastSavedValueIniSection = SubSection;
    }
}

void CSimpleSetting::WriteToIni(BOOL bOptimizeFileAccess)
{
    if(!m_Section.empty())
    {
        tstring CurrentValue(GetValueAsString());

        // here we want all settings in the ini file
        // so we only optimize if the value and section
        // were the same as what was loaded
        if(!bOptimizeFileAccess || CurrentValue != m_LastSavedValue || m_LastSavedValueIniSection != m_Section)
        {
            WritePrivateProfileString(m_Section.c_str(), m_Entry.c_str(), CurrentValue.c_str(), GetIniFileForSettings());
            LOG(2, _T(" WriteToIni %s %s Value %s Optimize %d Was %s"), m_Section.c_str(), m_Entry.c_str(), CurrentValue.c_str(), bOptimizeFileAccess, m_LastSavedValue.c_str());
        }
        else
        {
            LOG(2, _T(" WriteToIni Not Written %s %s Value %s"), m_Section.c_str(), m_Entry.c_str(), CurrentValue.c_str());
        }

        m_LastSavedValue = CurrentValue;
        m_LastSavedValueIniSection = m_Section;
    }
}

void CSimpleSetting::OSDShow()
{
    tstring OSDText(MakeString() << m_DisplayName << _T(" ") << GetDisplayValue());

    OSD_ShowText(OSDText, 0);

}

void CSimpleSetting::ChangeValue(eCHANGEVALUE NewValue)
{
    switch(NewValue)
    {
    case DISPLAY:
        OSDShow();
        break;
    case ADJUSTUP:
    case ADJUSTDOWN:
    case INCREMENT:
    case DECREMENT:
    case RESET:
    case TOGGLEBOOL:
        ChangeValueInternal((eCHANGEVALUE)(NewValue + (ADJUSTUP_SILENT - ADJUSTUP)));
        OSDShow();
        break;
    case ADJUSTUP_SILENT:
    case ADJUSTDOWN_SILENT:
    case INCREMENT_SILENT:
    case DECREMENT_SILENT:
    case RESET_SILENT:
    case TOGGLEBOOL_SILENT:
        ChangeValueInternal(NewValue);
        break;
    default:
        break;
    }
}

void CSimpleSetting::SetGroup(CSettingGroup* pGroup)
{
    m_pGroup = pGroup;
}

CSettingGroup* CSimpleSetting::GetGroup()
{
    return m_pGroup;
}

/** List setting.
    Specify list with 'pszList',
    for the rest of the parameters:
    @see CSimpleSetting
*/
CListSetting::CListSetting(const tstring& DisplayName, long Default, long Max, const tstring& Section, const tstring& Entry, const char** pszList, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Section, Entry, pGroup),
    m_Default(Default),
    m_Max(Max),
    m_Value(Default),
    m_List(pszList)
{
}

CListSetting::~CListSetting()
{
}

const char** CListSetting::GetList()
{
    return m_List;
}

long CListSetting::GetValue()
{
    return m_Value;
}

void CListSetting::SetValue(long NewValue, BOOL SupressOnChange)
{
    long OldValue(m_Value);
    if(m_Value >=0 && m_Value <= m_Max)
    {
        m_Value = NewValue;
    }
    else
    {
        m_Value = m_Default;
    }
    if(!SupressOnChange && IsOnChangeEnabled())
    {
        OnChange(m_Value, OldValue);
    }
}

long CListSetting::GetNumItems()
{
    return m_Max + 1;
}

void CListSetting::ChangeValueInternal(eCHANGEVALUE TypeOfChange)
{
    switch(TypeOfChange)
    {
    case ADJUSTUP_SILENT:
    case INCREMENT_SILENT:
        if(m_Value < m_Max)
        {
            SetValue(m_Value + 1);
        }
        break;
    case ADJUSTDOWN_SILENT:
    case DECREMENT_SILENT:
        if(m_Value > 0)
        {
            SetValue(m_Value - 1);
        }
        break;
    case RESET_SILENT:
        SetValue(m_Default);
        break;
    case TOGGLEBOOL_SILENT:
    default:
        break;
    }
}

tstring CListSetting::GetValueAsString()
{
    return ToString(m_Value);
}

tstring CListSetting::GetDisplayValue()
{
    if(m_List != NULL)
    {
        return MBCSToTString(m_List[m_Value]);
    }
    else
    {
        return ToString(m_Value);
    }
}

void CListSetting::SetValueFromString(const tstring& NewValue)
{
    SetValue(FromString<long>(NewValue));
}

LPARAM CListSetting::GetValueAsMessage()
{
    return m_Value;
}

void CListSetting::SetValueFromMessage(LPARAM LParam)
{
    SetValue(LParam);
}


void CListSetting::SetupControl(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("COMBOBOX")))
    {
        ComboBoxControlSetup(hWnd, m_Max + 1, m_List);
    }
}

void CListSetting::SetControlValue(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("COMBOBOX")))
    {
        ComboBoxSetValue(hWnd, m_Value);
    }
}

void CListSetting::SetFromControl(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("COMBOBOX")))
    {
        SetValue(ComboBoxGetValue(hWnd));
    }
}

 /** Change default value
*/
void CListSetting::ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue)
{
    m_Default = NewDefaultAsMessageType;
    if (!bDontSetValue)
    {
        SetValue(NewDefaultAsMessageType);
    }
}

/** Slider setting.
    For the parameters:
    @see CSimpleSetting
*/
CSliderSetting::CSliderSetting(const tstring& DisplayName, long Default, long Min, long Max, const tstring& Section, const tstring& Entry, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Section, Entry, pGroup),
    m_Default(Default),
    m_Min(Min),
    m_Max(Max),
    m_Value(Default),
    m_StepValue(1)
{
}

CSliderSetting::~CSliderSetting()
{
}

long CSliderSetting::GetValue()
{
    return m_Value;
}

void CSliderSetting::SetValue(long NewValue, BOOL SuppressOnChange)
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
    if (!SuppressOnChange && IsOnChangeEnabled())
    {
        OnChange(NewValue, OldValue);
    }
}

void CSliderSetting::SetMin(long Min)
{
    m_Min = Min;
}

void CSliderSetting::SetMax(long Max)
{
    m_Max = Max;
}

long CSliderSetting::GetMin()
{
    return m_Min;
}

long CSliderSetting::GetMax()
{
    return m_Max;
}

void CSliderSetting::SetStepValue(long Step)
{
    m_StepValue = Step;
}

tstring CSliderSetting::GetValueAsString()
{
    return ToString(m_Value);
}

tstring CSliderSetting::GetDisplayValue()
{
    return ToString(m_Value);
}

void CSliderSetting::SetValueFromString(const tstring& NewValue)
{
    SetValue(FromString<long>(NewValue));
}

LPARAM CSliderSetting::GetValueAsMessage()
{
    return m_Value;
}

void CSliderSetting::SetValueFromMessage(LPARAM LParam)
{
    SetValue(LParam);
}

 /** Change default value
*/
void CSliderSetting::ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue)
{
    m_Default = NewDefaultAsMessageType;
    if (!bDontSetValue)
    {
        SetValue(NewDefaultAsMessageType);
    }
}

void CSliderSetting::ChangeValueInternal(eCHANGEVALUE TypeOfChange)
{
    switch(TypeOfChange)
    {
    case ADJUSTUP_SILENT:
        if(m_Value < m_Max)
        {
            SetValue(m_Value + GetCurrentAdjustmentStepCount(this) * m_StepValue);
        }
        break;
    case INCREMENT_SILENT:
        if(m_Value < m_Max)
        {
            SetValue(m_Value + m_StepValue);
        }
        break;
    case ADJUSTDOWN_SILENT:
        if(m_Value > m_Min)
        {
            SetValue(m_Value - GetCurrentAdjustmentStepCount(this) * m_StepValue);
        }
        break;
    case DECREMENT_SILENT:
        if(m_Value > m_Min)
        {
            SetValue(m_Value - m_StepValue);
        }
        break;
    case RESET_SILENT:
        SetValue(m_Default);
        break;
    case TOGGLEBOOL_SILENT:
    default:
        break;
    }
}


void CSliderSetting::SetupControl(HWND hWnd)
{
    if(IsWindowClass(hWnd, TRACKBAR_CLASS))
    {
        SliderControlSetup(hWnd, m_StepValue);
        SetControlValue(hWnd);
    }
}

void CSliderSetting::SetControlValue(HWND hWnd)
{
    if(IsWindowClass(hWnd, TRACKBAR_CLASS))
    {
        SliderControlSetValue(hWnd, m_Value, m_Min, m_Max, m_Default);
    }
    if(IsWindowClass(hWnd, _T("EDIT")))
    {
        Edit_SetText(hWnd, ToString(m_Value).c_str());
    }
}

void CSliderSetting::SetFromControl(HWND hWnd)
{
    if(IsWindowClass(hWnd, TRACKBAR_CLASS))
    {
        SetValue(SliderControlGetValue(hWnd, m_Min, m_Max, m_StepValue));
    }
    if(IsWindowClass(hWnd, _T("EDIT")))
    {
        vector<TCHAR> Buffer(256);
        Edit_GetText(hWnd, &Buffer[0], 255);
        SetValueFromString(&Buffer[0]);
    }
}


/** Yes/No setting.
    Has only 0 or 1 as possible values.

    For the rest of the parameters:
    @see CSimpleSetting
*/
CYesNoSetting::CYesNoSetting(const tstring& DisplayName, BOOL Default, const tstring& Section, const tstring& Entry, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Section, Entry, pGroup),
    m_Default(Default),
    m_Value(Default)
{
}

CYesNoSetting::~CYesNoSetting()
{
}

void CYesNoSetting::SetValue(BOOL NewValue, BOOL SuppressOnChange)
{
    BOOL OldValue = m_Value;
    m_Value = NewValue?TRUE:FALSE;
    if (!SuppressOnChange && IsOnChangeEnabled())
    {
        OnChange(NewValue, OldValue);
    }
}

BOOL CYesNoSetting::GetValue()
{
    return m_Value;
}

void CYesNoSetting::SetupControl(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("Button")))
    {
        Button_SetText(hWnd, GetDisplayName().c_str());
    }
}

/** Change default value
*/
void CYesNoSetting::ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue)
{
    m_Default = NewDefaultAsMessageType;
    if (!bDontSetValue)
    {
        SetValue(NewDefaultAsMessageType);
    }
}

void CYesNoSetting::SetControlValue(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("Button")))
    {
        Button_SetCheck(hWnd, m_Value?BST_CHECKED:BST_UNCHECKED);
    }
}

void CYesNoSetting::SetFromControl(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("Button")))
    {
        SetValue(Button_GetCheck(hWnd) == BST_CHECKED);
    }
}

tstring CYesNoSetting::GetDisplayValue()
{
    return m_Value?_T("YES"):_T("NO");
}

tstring CYesNoSetting::GetValueAsString()
{
    return ToString(m_Value);
}

void CYesNoSetting::SetValueFromString(const tstring& NewValue)
{
    if(AreEqualInsensitive(NewValue, _T("YES")) || AreEqualInsensitive(NewValue, _T("TRUE")))
    {
        SetValue(TRUE);
    }
    else if(AreEqualInsensitive(NewValue, _T("NO")) || AreEqualInsensitive(NewValue, _T("FALSE")))
    {
        SetValue(FALSE);
    }
    else
    {
        SetValue(FromString<long>(NewValue));
    }
}

LPARAM CYesNoSetting::GetValueAsMessage()
{
    return m_Value;
}

void CYesNoSetting::SetValueFromMessage(LPARAM LParam)
{
    SetValue(LParam);
}

void CYesNoSetting::ChangeValueInternal(eCHANGEVALUE TypeOfChange)
{
    switch(TypeOfChange)
    {
    case ADJUSTUP_SILENT:
    case INCREMENT_SILENT:
        if(m_Value == FALSE)
        {
            SetValue(TRUE);
        }
        break;
    case ADJUSTDOWN_SILENT:
    case DECREMENT_SILENT:
        if(m_Value == TRUE)
        {
            SetValue(FALSE);
        }
        break;
    case RESET_SILENT:
        SetValue(m_Default);
        break;
    case TOGGLEBOOL_SILENT:
        SetValue(!m_Value);
        break;
    default:
        break;
    }
}


/** Character tstring setting.
    For the rest of the parameters:
    @see CSimpleSetting
*/
CStringSetting::CStringSetting(const tstring& DisplayName, const tstring& Default, const tstring& Section, const tstring& Entry, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Section, Entry, pGroup),
    m_Value(Default),
    m_Default(Default)

{
}

CStringSetting::~CStringSetting()
{
}

const TCHAR* CStringSetting::GetValue()
{
    return m_Value.c_str();
}

tstring CStringSetting::GetValueAsString()
{
    return m_Value;
}

tstring CStringSetting::GetDisplayValue()
{
    return m_Value;
}

void CStringSetting::ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue)
{
    throw logic_error("Can't change tstring defaults");
}

void CStringSetting::SetValue(const TCHAR* NewValue, BOOL bSuppressOnChange)
{
    tstring OldValue = m_Value;
    if(NewValue != 0)
    {
        m_Value = NewValue;
    }
    else
    {
        m_Value = _T("");
    }
    if (!bSuppressOnChange && IsOnChangeEnabled())
    {
        OnChange(m_Value, OldValue);
    }
}

void CStringSetting::SetValueFromString(const tstring& NewValue)
{
    SetValue(NewValue.c_str());
}

LPARAM CStringSetting::GetValueAsMessage()
{
    return (LPARAM)m_Value.c_str();
}

void CStringSetting::SetValueFromMessage(LPARAM LParam)
{
    SetValue(MBCSToTString((const char*)LParam).c_str());
}

void CStringSetting::ChangeValueInternal(eCHANGEVALUE TypeOfChange)
{
    switch(TypeOfChange)
    {
    case RESET_SILENT:
        SetValue(m_Default.c_str());
        break;
    default:
        break;
    }
}

void CStringSetting::SetupControl(HWND hWnd)
{
}

void CStringSetting::SetControlValue(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("EDIT")))
    {
        Edit_SetText(hWnd, ToString(m_Value).c_str());
    }
}

void CStringSetting::SetFromControl(HWND hWnd)
{
    if(IsWindowClass(hWnd, _T("EDIT")))
    {
        vector<TCHAR> Buffer(256);
        Edit_GetText(hWnd, &Buffer[0], 255);
        SetValue(&Buffer[0]);
    }
}

CEmptySetting::CEmptySetting() :
    CYesNoSetting(_T(""), FALSE, _T(""), _T(""))
{
}

CSettingWrapper::CSettingWrapper(SETTING* pSetting, CSettingGroup* pGroup) :
    CSimpleSetting(MBCSToTString(pSetting->szDisplayName), MBCSToTString(pSetting->szIniSection), MBCSToTString(pSetting->szIniEntry), pGroup),
    m_Setting(pSetting)
{
}

CSettingWrapper::~CSettingWrapper()
{
}

SETTING_TYPE CSettingWrapper::GetType()
{
    return m_Setting->Type;
}

void CSettingWrapper::SetupControl(HWND hWnd)
{
    switch(m_Setting->Type)
    {
    case ITEMFROMLIST:
        if(IsWindowClass(hWnd, _T("COMBOBOX")))
        {
            ComboBoxControlSetup(hWnd, m_Setting->MaxValue + 1, m_Setting->pszList);
        }
        break;
    case YESNO:
    case ONOFF:
        if(IsWindowClass(hWnd, _T("Button")))
        {
            Button_SetText(hWnd, GetDisplayName().c_str());
        }
        break;
    case SLIDER:
        if(IsWindowClass(hWnd, TRACKBAR_CLASS))
        {
            SliderControlSetup(hWnd, m_Setting->StepValue);
            SetControlValue(hWnd);
        }
        break;
    default:
        break;
    }
}

void CSettingWrapper::SetControlValue(HWND hWnd)
{
    switch(m_Setting->Type)
    {
    case ITEMFROMLIST:
        if(IsWindowClass(hWnd, _T("COMBOBOX")))
        {
            ComboBoxSetValue(hWnd, *m_Setting->pValue);
        }
        break;
    case YESNO:
    case ONOFF:
        if(IsWindowClass(hWnd, _T("BUTTON")))
        {
            Button_SetCheck(hWnd, *(m_Setting->pValue)?BST_CHECKED:BST_UNCHECKED);
        }
        break;
    case SLIDER:
        if(IsWindowClass(hWnd, TRACKBAR_CLASS))
        {
            SliderControlSetValue(hWnd, *m_Setting->pValue, m_Setting->MinValue, m_Setting->MaxValue, m_Setting->Default);
        }
        if(IsWindowClass(hWnd, _T("EDIT")))
        {
            Edit_SetText(hWnd, ToString(*m_Setting->pValue).c_str());
        }
        break;
    case CHARSTRING:
        if(IsWindowClass(hWnd, _T("EDIT")))
        {
            SetWindowTextA(hWnd, (LPCSTR)(*m_Setting->pValue));
        }
    case WCHARSTRING:
        if(IsWindowClass(hWnd, _T("EDIT")))
        {
            SetWindowTextW(hWnd, (LPCWSTR)(*m_Setting->pValue));
        }
    default:
        break;
    }
}

void CSettingWrapper::SetFromControl(HWND hWnd)
{
    switch(m_Setting->Type)
    {
    case ITEMFROMLIST:
        if(IsWindowClass(hWnd, _T("COMBOBOX")))
        {
            SetValue(ComboBoxGetValue(hWnd));
        }
        break;
    case YESNO:
    case ONOFF:
        if(IsWindowClass(hWnd, _T("BUTTON")))
        {
            SetValue(Button_GetCheck(hWnd) == BST_CHECKED);
        }
        break;
    case SLIDER:
        if(IsWindowClass(hWnd, TRACKBAR_CLASS))
        {
            SetValue(SliderControlGetValue(hWnd, m_Setting->MinValue, m_Setting->MaxValue, m_Setting->StepValue));
        }
        if(IsWindowClass(hWnd, _T("EDIT")))
        {
            vector<TCHAR> Buffer(256);
            Edit_GetText(hWnd, &Buffer[0], 255);
            SetValue(_ttol(&Buffer[0]));
        }
        break;
    case CHARSTRING:
        if(IsWindowClass(hWnd, _T("EDIT")))
        {
            vector<char> Buffer(256);
            GetWindowTextA(hWnd, &Buffer[0], 255);
            SetValue((long)&Buffer[0]);
        }
    case WCHARSTRING:
        if(IsWindowClass(hWnd, _T("EDIT")))
        {
            vector<wchar_t> Buffer(256);
            GetWindowTextW(hWnd, &Buffer[0], 255);
            SetValue((long)&Buffer[0]);
        }
    default:
        break;
    }
}

tstring CSettingWrapper::GetValueAsString()
{
    if(m_Setting->Type == CHARSTRING)
    {
        return *m_Setting->pValue ? MBCSToTString((const char*)(*m_Setting->pValue)) : _T("");
    }
    else if(m_Setting->Type == WCHARSTRING)
    {
        tstring ValueAsString(*m_Setting->pValue ? UnicodeToTString((const wchar_t*)(*m_Setting->pValue)) : _T(""));
        return EncodeASCIISafeString(ValueAsString);
    }
    else
    {
        return ToString(*(m_Setting->pValue));
    }
}
void CSettingWrapper::ChangeDefault(long NewDefaultAsMessageType, BOOL bDontSetValue)
{
    if(m_Setting->Type != CHARSTRING && m_Setting->Type != WCHARSTRING)
    {
        m_Setting->Default = NewDefaultAsMessageType;
        if (!bDontSetValue)
        {
            SetValue(NewDefaultAsMessageType);
        }
    }
    else
    {
        throw logic_error("Can't change tstring defaults");
    }
}

tstring FormatFloat(float Number, int Decimals)
{
    tostringstream oss;
    oss << fixed << setprecision(Decimals) << Number;
    return oss.str();
}

tstring CSettingWrapper::GetDisplayValue()
{
    switch(m_Setting->Type)
    {
    case ITEMFROMLIST:
        return MBCSToTString(m_Setting->pszList[*(m_Setting->pValue)]);
        break;
    case YESNO:
        return *(m_Setting->pValue)?_T("YES"):_T("NO");
        break;
    case ONOFF:
        return *(m_Setting->pValue)?_T("ON"):_T("OFF");
        break;
    case SLIDER:
        if(m_Setting->OSDDivider == 1)
        {
            return ToString(*(m_Setting->pValue));
        }
        else if(m_Setting->OSDDivider == 8)
        {
            return FormatFloat((float)*(m_Setting->pValue) / (float)m_Setting->OSDDivider, 3);
        }
        else
        {
            return FormatFloat((float)*(m_Setting->pValue) / (float)m_Setting->OSDDivider, (int)log10((double)m_Setting->OSDDivider));
        }
        break;
    case CHARSTRING:
        *m_Setting->pValue ? MBCSToTString((const char*)(*m_Setting->pValue)) : _T("");
        break;
    case WCHARSTRING:
        *m_Setting->pValue ? UnicodeToTString((const wchar_t*)(*m_Setting->pValue)) : _T("");
        break;
    default:
        break;
    }
    return _T("");
}

void CSettingWrapper::SetValueFromString(const tstring& NewValue)
{
    if(m_Setting->Type == CHARSTRING)
    {
        string ShortValue(TStringToMBCS(NewValue));
        SetValue(reinterpret_cast<LONG_PTR>(ShortValue.c_str()));
    }
    else if(m_Setting->Type == WCHARSTRING)
    {
        wstring LongValue(TStringToUnicode(DecodeASCIISafeString(NewValue)));
        SetValue(reinterpret_cast<LONG_PTR>(LongValue.c_str()));
    }
    else
    {
        SetValue(FromString<long>(NewValue));
    }
}

void CSettingWrapper::SetValue(LONG_PTR NewValue)
{
    LONG_PTR OldValue(*m_Setting->pValue);

    if(m_Setting->Type == CHARSTRING)
    {
        if(*m_Setting->pValue)
        {
            delete [] (char* )(*m_Setting->pValue);
        }
        const char* NewString = (char*)NewValue;
        size_t Len(strlen(NewString));
        if(Len > 0)
        {
            *m_Setting->pValue = (long)new char[Len + 1];
            strcpy((char*)(*m_Setting->pValue), NewString);
        }
        else
        {
            *m_Setting->pValue = 0;
        }
    }
    else if(m_Setting->Type == WCHARSTRING)
    {
        if(*m_Setting->pValue)
        {
            delete [] (wchar_t* )(*m_Setting->pValue);
        }
        const wchar_t* NewString = (wchar_t*)NewValue;
        size_t Len(wcslen(NewString));
        if(Len > 0)
        {
            *m_Setting->pValue = (long)new wchar_t[Len + 1];
            wcscpy((wchar_t*)(*m_Setting->pValue), NewString);
        }
        else
        {
            *m_Setting->pValue = 0;
        }
    }
    else
    {
        if(NewValue < m_Setting->MinValue)
        {
            *m_Setting->pValue = m_Setting->MinValue;
        }
        else if(NewValue > m_Setting->MaxValue)
        {
            *m_Setting->pValue = m_Setting->MaxValue;
        }
        else
        {
            *m_Setting->pValue = NewValue;
        }
    }
    if(IsOnChangeEnabled() && m_Setting->pfnOnChange != NULL)
    {
        m_Setting->pfnOnChange(NewValue);
    }
}


LPARAM CSettingWrapper::GetValueAsMessage()
{
    return *m_Setting->pValue;
}

void CSettingWrapper::SetValueFromMessage(LPARAM LParam)
{
    SetValue(LParam);
}

void CSettingWrapper::ChangeValueInternal(eCHANGEVALUE TypeOfChange)
{
    if(m_Setting == NULL)
    {
        return;
    }
    switch(TypeOfChange)
    {
    case ADJUSTUP_SILENT:
        if (m_Setting->Type != CHARSTRING && m_Setting->Type != WCHARSTRING)
        {
            if (*m_Setting->pValue < m_Setting->MaxValue)
            {
                SetValue(*m_Setting->pValue + GetCurrentAdjustmentStepCount(m_Setting) * m_Setting->StepValue);
            }
        }
        break;
    case ADJUSTDOWN_SILENT:
        if (m_Setting->Type != CHARSTRING && m_Setting->Type != WCHARSTRING)
        {
            if (*m_Setting->pValue > m_Setting->MinValue)
            {
                SetValue(*m_Setting->pValue - GetCurrentAdjustmentStepCount(m_Setting) * m_Setting->StepValue);
            }
        }
        break;
    case INCREMENT_SILENT:
        if (m_Setting->Type != CHARSTRING && m_Setting->Type != WCHARSTRING)
        {
            SetValue(*m_Setting->pValue + m_Setting->StepValue);
        }
        break;
    case DECREMENT_SILENT:
        if (m_Setting->Type != CHARSTRING && m_Setting->Type != WCHARSTRING)
        {
            SetValue(*m_Setting->pValue - m_Setting->StepValue);
        }
        break;
    case RESET_SILENT:
        SetValue(m_Setting->Default);
        break;
    case TOGGLEBOOL_SILENT:
        if(m_Setting->Type == YESNO || m_Setting->Type == ONOFF)
        {
            SetValue(!*m_Setting->pValue);
        }
        break;
    default:
        break;
    }
}

SettingStringValue::SettingStringValue() :
    m_Value(0)
{
}

SettingStringValue::~SettingStringValue()
{
    clear();
}

SettingStringValue::operator LPCTSTR()
{
    if(m_Value)
    {
        return m_Value;
    }
    else
    {
        return _T("");
    }
}

BOOL SettingStringValue::IsValid()
{
    return m_Value != 0 && m_Value[0] != 0;
}


LONG_PTR* SettingStringValue::GetPointer()
{
    return (LONG_PTR*)&m_Value;
}

void SettingStringValue::SetValue(const tstring& NewValue)
{
    clear();
    if(!NewValue.empty())
    {
        m_Value  = new TCHAR[NewValue.length() + 1];
        _tcscpy(m_Value, NewValue.c_str());
    }
}


void SettingStringValue::clear()
{
    if(m_Value)
    {
        delete [] m_Value;
        m_Value = 0;
    }
}
