////////////////////////////////////////////////////////////////////////////
// $Id: Setting.cpp,v 1.29 2003-06-14 13:27:48 laurentg Exp $
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
// Revision 1.28  2003/04/28 12:42:20  laurentg
// Management of character string settings updated
//
// Revision 1.27  2003/04/26 23:19:15  laurentg
// Character string settings
//
// Revision 1.26  2003/01/24 01:55:17  atnak
// OSD + Teletext conflict fix, offscreen buffering for OSD and Teletext,
// got rid of the pink overlay colorkey for Teletext.
//
// Revision 1.25  2003/01/23 15:03:08  adcockj
// Fix for slow channel changing
//
// Revision 1.24  2003/01/19 20:07:14  adcockj
// Setting optimization fixes
//
// Revision 1.23  2003/01/16 13:30:49  adcockj
// Fixes for various settings problems reported by Laurent 15/Jan/2003
//
// Revision 1.22  2003/01/13 19:22:44  adcockj
// Setttings bug fixes
//
// Revision 1.21  2003/01/12 16:19:34  adcockj
// Added SettingsGroup activity setting
// Corrected event sequence and channel change behaviour
//
// Revision 1.20  2003/01/10 17:52:08  adcockj
// Removed SettingFlags
//
// Revision 1.19  2003/01/10 17:38:21  adcockj
// Interrim Check in of Settings rewrite
//  - Removed SETTINGSEX structures and flags
//  - Removed Seperate settings per channel code
//  - Removed Settings flags
//  - Cut away some unused features
//
// Revision 1.18  2002/10/15 15:06:01  kooiman
// Split setting.cpp in parts + cleanup.
//
// Revision 1.17  2002/10/02 10:55:17  kooiman
// Fix event object casting and fixed groups.
//
// Revision 1.16  2002/09/29 13:55:38  adcockj
// Standards
//
// Revision 1.15  2002/09/28 13:34:07  kooiman
// Added sender object to events and added setting flag to treesettingsgeneric.
//
// Revision 1.14  2002/09/26 16:34:19  kooiman
// Lots of toolbar fixes &added EVENT_VOLUME support.
//
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
#include "Setting.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Settings.h"
#include "DebugLog.h"
#include "OSD.h"
#include "DScaler.h"
#include "Slider.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/** Internal flags
*/

#define RWFLAG_READFROMINI 1
#define RWFLAG_ISIN_INI  2
#define RWFLAG_FLAGIN_INI 4

/** Constructor

    Specify setting parameters:
    Display name, default value, minimum value, maximum value,
    default ini section, ini entry, step value,
    (optional:)
    Setting group, setting flags,
    GUI info, static callback function, pointer for callback function

*/
CSimpleSetting::CSimpleSetting(LPCSTR DisplayName, long Default, long Min, long Max, 
                               LPCSTR Section, LPCSTR Entry, long StepValue, 
                               CSettingGroup* pGroup)
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
    
    m_pSetting->pfnOnChange = NULL;

    m_pGroup = pGroup;
    m_EnableOnChange = TRUE;
    m_ReadWriteFlags = 0;

    m_SectionLastSavedValue = Default;
}


/** Constructor

    Specify setting parameters:
    Pointer to SETTING structure, 
    (optional:)
    Setting group,
    Setting flags,
    GUI info, 
    static callback function, pointer for callback function

*/
CSimpleSetting::CSimpleSetting(SETTING* pSetting, CSettingGroup* pGroup)
{    
    m_pSetting = pSetting;
    m_bFreeSettingOnExit = FALSE;
    
    m_pGroup = pGroup;
    
    m_EnableOnChange = TRUE;
    m_ReadWriteFlags = 0;

    m_SectionLastSavedValue = pSetting->Default;
}

CSimpleSetting::~CSimpleSetting()
{
    if (m_bFreeSettingOnExit)
    {
        delete m_pSetting;
        m_pSetting = NULL;
    }
}


/**
    Check if the setting should do an OnChange call
*/
BOOL CSimpleSetting::DoOnChange(long NewValue, long OldValue)
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

long CSimpleSetting::GetValue()
{
    return *m_pSetting->pValue;    
}

/**
    Set value
*/
void CSimpleSetting::SetValue(long NewValue, BOOL bSupressOnChange)
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
    if (!bSupressOnChange && DoOnChange(NewValue, OldValue))
    {
        OnChange(NewValue, OldValue);
    }
}

/**
    Set default value.
*/
void CSimpleSetting::SetDefault()
{
    long OldValue = *m_pSetting->pValue;
    *m_pSetting->pValue = m_pSetting->Default;
    if (DoOnChange(*m_pSetting->pValue, OldValue))
    {
        OnChange(*m_pSetting->pValue, OldValue);
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


/** Read value from sub section in .ini file
    @param szSubSection Set to NULL to read from the default location
    @param bSetDefaultOnFailure If the setting was not in the .ini file, set the setting's value to the default value
    @param pSettingFlags Override setting flags of current setting if not NULL
    @return TRUE if value was in .ini file            
*/
BOOL CSimpleSetting::ReadFromIniSubSection(LPCSTR szSubSection)
{
    long nValue;
    long nSavedValue;
    BOOL IsSettingInIniFile = TRUE;

    if(m_pSetting->szIniSection != NULL)
    {        
        string sEntry;
		char* szIniEntry;

        sEntry = m_pSetting->szIniSection;
		sEntry += "_";
		sEntry+= m_pSetting->szIniEntry;
		szIniEntry = (char*)sEntry.c_str();
		
		char szDefaultString[] = {0};
		char szBuffer[256];
		
		int Len = GetPrivateProfileString(szSubSection, szIniEntry, szDefaultString, szBuffer, 255, GetIniFileForSettings());
        LOG(2, " ReadFromIniSubSection %s %s Result %s", szSubSection, szIniEntry, szBuffer);

		if (Len <= 0)
		{
			IsSettingInIniFile = FALSE;
			nValue = m_pSetting->Default;
			nSavedValue = nValue;
		}
		else
		{
			IsSettingInIniFile = TRUE;

			char* szValue = szBuffer;
			nValue = atoi(szValue);
			nSavedValue = nValue;
		       
			// If the value is out of range, set it to its default value
			if ( (nValue < m_pSetting->MinValue)
			  || (nValue > m_pSetting->MaxValue) )
			{
				if(nValue < m_pSetting->MinValue)
				{
					LOG(1, "toto %s %s Was out of range - %d is too low", szSubSection, szIniEntry, nValue);
				}
				else
				{
					LOG(1, "%s %s Was out of range - %d is too high", szSubSection, szIniEntry, nValue);
				}
				nValue = m_pSetting->Default;
			}
		}

		int OldValue = *m_pSetting->pValue;
		*m_pSetting->pValue = nValue;
	
        // only call OnChange when there actually is a change
        // this will help keep channel changes slick
		if(*m_pSetting->pValue != OldValue && DoOnChange(*m_pSetting->pValue, OldValue))
	    {
		    OnChange(*m_pSetting->pValue, OldValue);
		}
		
	    m_SectionLastSavedValue = nSavedValue;
		m_SectionLastSavedValueIniSection = szSubSection;

    }
    else
    {
        m_SectionLastSavedValueIniSection = "";
        IsSettingInIniFile = FALSE;
    }
    return IsSettingInIniFile;
}

/** Read value from default location in .ini file
*/
BOOL CSimpleSetting::ReadFromIni()
{
    long nValue;
    long nSavedValue;
    BOOL IsSettingInIniFile = TRUE;

    if(m_pSetting->szIniSection != NULL)
    {        
		char szDefaultString[] = {0};
		char szBuffer[256];
		
		int Len = GetPrivateProfileString(m_pSetting->szIniSection, m_pSetting->szIniEntry, szDefaultString, szBuffer, 255, GetIniFileForSettings());
        LOG(2, " ReadFromIni %s %s Result %s", m_pSetting->szIniSection, m_pSetting->szIniEntry, szBuffer);

		if (Len <= 0)
		{
			IsSettingInIniFile = FALSE;
			nValue = m_pSetting->Default;
		}
		else
		{
			IsSettingInIniFile = TRUE;

			char* szValue = szBuffer;
			nValue = atoi(szValue);
			nSavedValue = nValue;
		       
			// If the value is out of range, set it to its default value
			if ( (nValue < m_pSetting->MinValue)
			  || (nValue > m_pSetting->MaxValue) )
			{
				if(nValue < m_pSetting->MinValue)
				{
					LOG(1, "titi %s %s Was out of range - %d is too low", m_pSetting->szIniSection, m_pSetting->szIniEntry, nValue);
				}
				else
				{
					LOG(1, "%s %s Was out of range - %d is too high", m_pSetting->szIniSection, m_pSetting->szIniEntry, nValue);
				}
				nValue = m_pSetting->Default;
			}
		}
        if (IsSettingInIniFile)
        {            
			int OldValue = *m_pSetting->pValue;
			*m_pSetting->pValue = nValue;
		
			if (DoOnChange(*m_pSetting->pValue, OldValue))
	        {
		        OnChange(*m_pSetting->pValue, OldValue);
			}
			m_pSetting->LastSavedValue = nSavedValue;
			m_sLastSavedValueIniSection = m_pSetting->szIniSection;
        }        
        else
        {
            m_sLastSavedValueIniSection = "";
        }
    }
    else
    {
        m_sLastSavedValueIniSection = "";
        IsSettingInIniFile =  FALSE;
    }
    return IsSettingInIniFile;
}

/** Write value to szSubsection in .ini file
    Override value and setting flags if Value and/or pSettingFlags is not NULL.
*/
void CSimpleSetting::WriteToIniSubSection(LPCSTR szSubSection, BOOL bOptimizeFileAccess)
{
    if(m_pSetting->szIniSection != NULL)
    {
        string sEntry;
		char* szIniEntry;

        sEntry = m_pSetting->szIniSection;
		sEntry += "_";
		sEntry+= m_pSetting->szIniEntry;
		szIniEntry = (char*)sEntry.c_str();

		long Val = *m_pSetting->pValue; 

        BOOL bWriteValue = FALSE;

        if(bOptimizeFileAccess)
        {
            if(m_SectionLastSavedValue != Val || m_SectionLastSavedValueIniSection != szSubSection)
            {
                bWriteValue = TRUE;
            }
            else
            {
                bWriteValue = FALSE;
            }
        }
        else
        {
            bWriteValue = TRUE;
        }

        // hopefully stops noise 
        if(bWriteValue)
        {
		    WritePrivateProfileInt(szSubSection, szIniEntry, Val, GetIniFileForSettings());
            LOG(2, " WriteToIniSubSection %s %s Value %d", szSubSection, szIniEntry, Val);
        }
        else
        {
            LOG(2, " WriteToIniSubSection Not Written %s %s Value %d Was ", szSubSection, szIniEntry, Val, m_SectionLastSavedValue);
        }


        m_SectionLastSavedValue = Val;
		m_SectionLastSavedValueIniSection = szSubSection;
    }
}

void CSimpleSetting::WriteToIni(BOOL bOptimizeFileAccess)
{
    if(m_pSetting->szIniSection != NULL)
    {
		long Val = *m_pSetting->pValue; 

        // here we want all settings in the ini file
        // so we only optimize if the value and section 
        // were the same as what was loaded
        if(!bOptimizeFileAccess || Val != m_pSetting->LastSavedValue || m_sLastSavedValueIniSection != m_pSetting->szIniSection)
        {
		    WritePrivateProfileInt(m_pSetting->szIniSection, m_pSetting->szIniEntry, Val, GetIniFileForSettings());
            LOG(2, " WriteToIni %s %s Value %d", m_pSetting->szIniSection, m_pSetting->szIniEntry, Val);
        }
        else
        {
            LOG(2, " WriteToIni Not Written %s %s Value %d Was ", m_pSetting->szIniSection, m_pSetting->szIniEntry, Val, m_pSetting->LastSavedValue);
        }

        m_pSetting->LastSavedValue = Val;
		
		m_sLastSavedValueIniSection = m_pSetting->szIniSection;
    }
}
 

 /** Change default value
*/
void CSimpleSetting::ChangeDefault(long NewDefault, BOOL bDontSetValue)
{
    m_pSetting->Default = NewDefault;
    if (!bDontSetValue)    
    {
        SetValue(NewDefault);
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

    OSD_ShowText(szBuffer, 0);
    
}

void CSimpleSetting::Up()
{
    if ((*m_pSetting->pValue) < m_pSetting->MaxValue)
    {
        int nStep = GetCurrentAdjustmentStepCount(this) * m_pSetting->StepValue;
        SetValue(*m_pSetting->pValue + nStep);
    }
}

void CSimpleSetting::Down()
{
    if ((*m_pSetting->pValue) > m_pSetting->MinValue)
    {
        int nStep = GetCurrentAdjustmentStepCount(this) * m_pSetting->StepValue;
        SetValue(*m_pSetting->pValue - nStep);
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
        SetValue((*m_pSetting->pValue) + m_pSetting->StepValue);
        OSDShow();
        break;
    case DECREMENT:
        SetValue((*m_pSetting->pValue) - m_pSetting->StepValue);
        OSDShow();
        break;
    case RESET:
        SetDefault();
        OSDShow();
        break;
    case TOGGLEBOOL:
        if(GetType() == YESNO || GetType() == ONOFF)
        {
            SetValue(!(*m_pSetting->pValue));
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
        SetValue((*m_pSetting->pValue) + m_pSetting->StepValue);
        break;
    case DECREMENT_SILENT:
        SetValue((*m_pSetting->pValue) - m_pSetting->StepValue);
        break;
    case RESET_SILENT:
        SetDefault();
        break;
    case TOGGLEBOOL_SILENT:
        if(GetType() == YESNO || GetType() == ONOFF)
        {
            SetValue(!(*m_pSetting->pValue));
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

/** Default OnChange function
    Child class can override

    This function calls the pfnExOnChange function
    or the pfnOnChange function of the setting
    if one of them is not NULL
*/
void CSimpleSetting::OnChange(long NewValue, long OldValue)
{
    if (m_pSetting->pfnOnChange!=NULL)
    {
        m_pSetting->pfnOnChange(NewValue);
    }
}




/** List setting.
    Specify list with 'pszList',
    for the rest of the parameters: 
    @see CSimpleSetting
*/
CListSetting::CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Default, 0, Max, Section, Entry, 1, pGroup)    
{
    m_pSetting->Type = ITEMFROMLIST;
    m_pSetting->pszList = pszList;
}

CListSetting::CListSetting(SETTING* pSetting, CSettingGroup* pGroup) : 
	CSimpleSetting(pSetting, pGroup)
{   
}

CListSetting::~CListSetting()
{
}

void CListSetting::GetDisplayText(LPSTR szBuffer)
{
    char* szName = m_pSetting->szDisplayName;
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

void CListSetting::SetFromControl(HWND hWnd)
{
    SetValue(ComboBox_GetCurSel(hWnd));
}



/** Slider setting.
    For the parameters: 
    @see CSimpleSetting
*/
CSliderSetting::CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Default, Min, Max, Section, Entry, 1, pGroup)
{
    m_pSetting->OSDDivider = 1;
    m_pSetting->Type = SLIDER;
}

CSliderSetting::CSliderSetting(SETTING* pSetting, CSettingGroup* pGroup) : 
	CSimpleSetting(pSetting, pGroup)
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
    char* szName = m_pSetting->szDisplayName;
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

void CSliderSetting::SetFromControl(HWND hWnd)
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
    // make sure the value is only a multiple of the step size.
    if(m_pSetting->StepValue != 1)
    {
        nValue = nValue - (nValue % m_pSetting->StepValue);
    }
    SetValue(nValue);
}


/** Yes/No setting.
    Has only 0 or 1 as possible values.    
    
    For the rest of the parameters: 
    @see CSimpleSetting
*/
CYesNoSetting::CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Default, 0, 1, Section, Entry, 1, pGroup)
{
    m_pSetting->Type = YESNO;
}

CYesNoSetting::CYesNoSetting(SETTING* pSetting, CSettingGroup* pGroup) : 
	CSimpleSetting(pSetting, pGroup)
{  
}

CYesNoSetting::~CYesNoSetting()
{
}

void CYesNoSetting::GetDisplayText(LPSTR szBuffer)
{
    char* szName = m_pSetting->szDisplayName;
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

void CYesNoSetting::SetFromControl(HWND hWnd)
{
    SetValue(Button_GetCheck(hWnd) == BST_CHECKED);
}


/** Character string setting.
    For the rest of the parameters: 
    @see CSimpleSetting
*/
CStringSetting::CStringSetting(LPCSTR DisplayName, long Default, LPCSTR Section, LPCSTR Entry, CSettingGroup* pGroup) :
    CSimpleSetting(DisplayName, Default, 0, 0, Section, Entry, 1, pGroup)
{
    m_pSetting->Type = CHARSTRING;
	if (*m_pSetting->pValue != NULL)
	{
		char* str = new char[strlen((char *)(*m_pSetting->pValue)) + 1];
		strcpy(str, (char *)(*m_pSetting->pValue));
		*m_pSetting->pValue = (long)str;
	}
}

CStringSetting::CStringSetting(SETTING* pSetting, CSettingGroup* pGroup) : 
	CSimpleSetting(pSetting, pGroup)
{
}

CStringSetting::~CStringSetting()
{
    if (m_bFreeSettingOnExit)
    {
		if (*m_pSetting->pValue != NULL)
		{
			delete (char *)(*m_pSetting->pValue);
		}
        delete m_pSetting;
        m_pSetting = NULL;
    }
}

void CStringSetting::GetDisplayText(LPSTR szBuffer)
{
    char* szName = m_pSetting->szDisplayName;
    if (szName == NULL)
    {
        szName = m_pSetting->szIniEntry;
    }
    sprintf(szBuffer, "%s %s", szName, *m_pSetting->pValue ? (char*)(*m_pSetting->pValue) : "");
}

void CStringSetting::SetValue(long NewValue, BOOL bSupressOnChange)
{
    long OldValue = *m_pSetting->pValue;
	char* str = new char[strlen((char *)NewValue) + 1];
	strcpy(str, (char *)NewValue);
	*m_pSetting->pValue = (long)str;
    if (!bSupressOnChange && DoOnChange(NewValue, OldValue))
    {
        OnChange(NewValue, OldValue);
    }
	if (OldValue != NULL)
	{
		delete (char *)OldValue;
	}
}

void CStringSetting::SetDefault()
{
	SetValue(m_pSetting->Default);
}

/** Read value from sub section in .ini file
    @param szSubSection Set to NULL to read from the default location
    @param bSetDefaultOnFailure If the setting was not in the .ini file, set the setting's value to the default value
    @param pSettingFlags Override setting flags of current setting if not NULL
    @return TRUE if value was in .ini file            
*/
BOOL CStringSetting::ReadFromIniSubSection(LPCSTR szSubSection)
{
    BOOL IsSettingInIniFile = TRUE;

    if(m_pSetting->szIniSection != NULL)
    {        
        string sEntry;
		char* szIniEntry;

        sEntry = m_pSetting->szIniSection;
		sEntry += "_";
		sEntry+= m_pSetting->szIniEntry;
		szIniEntry = (char*)sEntry.c_str();
		
		char szDefaultString[] = {0};
		char szBuffer[256];
		char* szValue;
		
		int Len = GetPrivateProfileString(szSubSection, szIniEntry, szDefaultString, szBuffer, 255, GetIniFileForSettings());
        LOG(2, " ReadFromIniSubSection %s %s Result %s", szSubSection, szIniEntry, szBuffer);

		if (Len <= 0)
		{
			IsSettingInIniFile = FALSE;
			szValue = (char *)(m_pSetting->Default);
		}
		else
		{
			IsSettingInIniFile = TRUE;
			szValue = (char *)szBuffer;		       
		}

		int OldValue = *m_pSetting->pValue;
		char* str = new char[strlen(szValue) + 1];
		strcpy(str, szValue);
		*m_pSetting->pValue = (long)str;

        // only call OnChange when there actually is a change
        // this will help keep channel changes slick
		if(strcmp((char*)*m_pSetting->pValue, (char*)OldValue) && DoOnChange(*m_pSetting->pValue, OldValue))
	    {
		    OnChange(*m_pSetting->pValue, OldValue);
		}

		if (OldValue != NULL)
		{
			delete (char *)OldValue;
		}	

		m_SectionLastSavedValueIniSection = szSubSection;
    }
    else
    {
        m_SectionLastSavedValueIniSection = "";
        IsSettingInIniFile = FALSE;
    }
    return IsSettingInIniFile;
}

/** Read value from default location in .ini file
*/
BOOL CStringSetting::ReadFromIni()
{
    BOOL IsSettingInIniFile = TRUE;

    if(m_pSetting->szIniSection != NULL)
    {        
		char szDefaultString[] = {0};
		char szBuffer[256];
		char* szValue;
		
		int Len = GetPrivateProfileString(m_pSetting->szIniSection, m_pSetting->szIniEntry, szDefaultString, szBuffer, 255, GetIniFileForSettings());
        LOG(2, " ReadFromIni %s %s Result %s", m_pSetting->szIniSection, m_pSetting->szIniEntry, szBuffer);

		if (Len <= 0)
		{
			IsSettingInIniFile = FALSE;
			szValue = (char *)(m_pSetting->Default);
		}
		else
		{
			IsSettingInIniFile = TRUE;
			szValue = (char *)szBuffer;
		}
        if (IsSettingInIniFile)
        {            
			int OldValue = *m_pSetting->pValue;
			char* str = new char[strlen(szValue) + 1];
			strcpy(str, szValue);
			*m_pSetting->pValue = (long)str;

			if (DoOnChange(*m_pSetting->pValue, OldValue))
	        {
		        OnChange(*m_pSetting->pValue, OldValue);
			}

			if (OldValue != NULL)
			{
				delete (char *)OldValue;
			}

			m_sLastSavedValueIniSection = m_pSetting->szIniSection;
        }        
        else
        {
            m_sLastSavedValueIniSection = "";
        }
    }
    else
    {
        m_sLastSavedValueIniSection = "";
        IsSettingInIniFile =  FALSE;
    }
    return IsSettingInIniFile;
}

/** Write value to szSubsection in .ini file
    Override value and setting flags if Value and/or pSettingFlags is not NULL.
*/
void CStringSetting::WriteToIniSubSection(LPCSTR szSubSection, BOOL bOptimizeFileAccess)
{
    if(m_pSetting->szIniSection != NULL)
    {
        string sEntry;
		char* szIniEntry;

        sEntry = m_pSetting->szIniSection;
		sEntry += "_";
		sEntry+= m_pSetting->szIniEntry;
		szIniEntry = (char*)sEntry.c_str();

		WritePrivateProfileString(szSubSection, szIniEntry, (char*)(*m_pSetting->pValue), GetIniFileForSettings());
        LOG(2, " WriteToIniSubSection %s %s Value %s", szSubSection, szIniEntry, (char*)(*m_pSetting->pValue));

		m_SectionLastSavedValueIniSection = szSubSection;
    }
}

void CStringSetting::WriteToIni(BOOL bOptimizeFileAccess)
{
    if(m_pSetting->szIniSection != NULL)
    {
		WritePrivateProfileString(m_pSetting->szIniSection, m_pSetting->szIniEntry, (char*)(*m_pSetting->pValue), GetIniFileForSettings());
        LOG(2, " WriteToIni %s %s Value %s", m_pSetting->szIniSection, m_pSetting->szIniEntry, (char*)(*m_pSetting->pValue));

		m_sLastSavedValueIniSection = m_pSetting->szIniSection;
    }
}

void CStringSetting::ChangeDefault(long NewDefault, BOOL bDontSetValue)
{
}

void CStringSetting::Up()
{
}

void CStringSetting::Down()
{
}
 
void CStringSetting::ChangeValue(eCHANGEVALUE NewValue)
{
    switch(NewValue)
    {
    case DISPLAY:
        OSDShow();
        break;
    case RESET:
        SetDefault();
        OSDShow();
        break;
    case RESET_SILENT:
        SetDefault();
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
}

void CStringSetting::SetFromControl(HWND hWnd)
{
}

