////////////////////////////////////////////////////////////////////////////
// $Id: Setting.cpp,v 1.18 2002-10-15 15:06:01 kooiman Exp $
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
                               CSettingGroup* pGroup, eSettingFlags SettingFlags, 
                               LONG GUIinfo, SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis)
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

    m_pSettingExPlus = &m_StoreExPlus;

	m_pSettingExPlus->SettingFlags = SettingFlags;
	m_pSettingExPlus->cbSize = sizeof(SETTINGEX);
	m_pSettingExPlus->DefaultSettingFlags = SettingFlags;	
	m_pSettingExPlus->GUIinfo = GUIinfo;
	m_pSettingExPlus->pszGroupList = NULL;
	
	m_pSettingExPlus->SettingFlags = SettingFlags;
	m_pSettingExPlus->LastSavedSettingFlags = SettingFlags;
	m_pSettingExPlus->szLastSavedValueIniSection = NULL;

    m_pSettingExPlus->pfnExOnChange = pfnExOnChange;    
    m_pSettingExPlus->pExOnChangeThis = pExOnChangeThis;
	
    m_pGroup = pGroup;
    m_EnableOnChange = TRUE;
    m_ReadWriteFlags = 0;
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
CSimpleSetting::CSimpleSetting(SETTING* pSetting, CSettingGroup* pGroup, 
                               eSettingFlags SettingFlags, long GUIinfo, 
                               SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis)
{    
    m_pSetting = pSetting;
    m_bFreeSettingOnExit = FALSE;
    
    m_pGroup = pGroup;
    
	m_pSettingExPlus = &m_StoreExPlus;

	m_pSettingExPlus->cbSize = sizeof(SETTINGEX);
	m_pSettingExPlus->DefaultSettingFlags = SettingFlags;	
	m_pSettingExPlus->GUIinfo = GUIinfo;
	m_pSettingExPlus->pszGroupList = NULL;
	
	m_pSettingExPlus->SettingFlags = SettingFlags;
	m_pSettingExPlus->LastSavedSettingFlags = SettingFlags;
	m_pSettingExPlus->szLastSavedValueIniSection = NULL;
	
    m_pSettingExPlus->pfnExOnChange = pfnExOnChange;
    m_pSettingExPlus->pExOnChangeThis = pExOnChangeThis;

    m_EnableOnChange = TRUE;
    m_ReadWriteFlags = 0;
}


/** Constructor

    Specify setting parameters:
    Pointer to SETTINGEX structure, 
    (optional:)
    Pointer to the right setting group,
*/
CSimpleSetting::CSimpleSetting(SETTINGEX* pSetting,CSettingGroup* pGroup)
{    
    m_pSetting = (SETTING*)pSetting;
    m_bFreeSettingOnExit = FALSE;
    
    m_pGroup = pGroup;
    m_pSettingExPlus = (SETTINGEXPLUS*)&pSetting->cbSize;

    m_pSettingExPlus->SettingFlags = m_pSettingExPlus->DefaultSettingFlags;
	m_pSettingExPlus->LastSavedSettingFlags = m_pSettingExPlus->DefaultSettingFlags;
	m_pSettingExPlus->szLastSavedValueIniSection = NULL;    
    m_EnableOnChange = TRUE;
    m_ReadWriteFlags = 0;
}


/** Constructor

    Specify setting parameters:
    Pointer to SETTINGEX structure, 
    (optional:)
    Pointer to group list where the group will be created
*/
CSimpleSetting::CSimpleSetting(SETTINGEX* pSetting, CSettingGroupList* pList)
{
    m_pSetting = (SETTING*)pSetting;
    m_bFreeSettingOnExit = FALSE;
    
    m_pGroup = NULL;
    if (pList!=NULL) 
    {
        m_pGroup = pList->GetGroup(NULL,pSetting->pszGroupList);
    }
    m_pSettingExPlus = (SETTINGEXPLUS*)&pSetting->cbSize;
    m_EnableOnChange = TRUE;

    m_pSettingExPlus->SettingFlags = m_pSettingExPlus->DefaultSettingFlags;
	m_pSettingExPlus->LastSavedSettingFlags = m_pSettingExPlus->DefaultSettingFlags;
	m_pSettingExPlus->szLastSavedValueIniSection = NULL;

    m_EnableOnChange = TRUE;
    m_ReadWriteFlags = 0;
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
BOOL CSimpleSetting::DoOnChange(long NewValue, long OldValue, eOnChangeType OnChangeType)
{
    if (!m_EnableOnChange)
    {
       return FALSE;
    }

    long Flags = m_pSettingExPlus->SettingFlags;

    if (Flags&SETTINGFLAG_ONCHANGE_VALUECHANGED)
    {
        if (NewValue == OldValue)
        {
            return FALSE;
        }
    }
    
    switch(OnChangeType)
    {
    case ONCHANGE_NONE: return FALSE;
    case ONCHANGE_INIT: return ((Flags & SETTINGFLAG_ONCHANGE_INIT)!=0);
    case ONCHANGE_SET:  return ((Flags & SETTINGFLAG_ONCHANGE_SET)!=0);                                
    case ONCHANGE_SET_FORCE: return TRUE;
    case ONCHANGE_SOURCECHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_INIT)!=0);
    case ONCHANGE_VIDEOINPUTCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_VIDEOINPUT)!=0);
    case ONCHANGE_AUDIOINPUTCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_AUDIOINPUT)!=0);
    case ONCHANGE_VIDEOFORMATCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_VIDEOFORMAT)!=0);
    case ONCHANGE_CHANNELCHANGE: return ((Flags & SETTINGFLAG_ONCHANGE_CHANNEL)!=0);
    }
    return FALSE;
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

/**
    Set default value.
    Use OnChangeType = ONCHANGE_NONE to avoid that
    the actual value will be set to the default value
*/
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

void CSimpleSetting::SetFlags(eSettingFlags SettingFlags)
{
   if (m_pSettingExPlus->SettingFlags != SettingFlags)
   {
       
       eSettingFlags Old = (eSettingFlags)m_pSettingExPlus->SettingFlags;
	   m_pSettingExPlus->SettingFlags = SettingFlags; 
	   FlagsOnChange(Old, (eSettingFlags)m_pSettingExPlus->SettingFlags);
   }
}

void CSimpleSetting::SetDefaultFlags(eSettingFlags SettingFlags, BOOL bSetFlagsToDefault)
{
    m_pSettingExPlus->DefaultSettingFlags = SettingFlags;
    if (!(m_ReadWriteFlags & RWFLAG_FLAGIN_INI))
    {
        m_pSettingExPlus->LastSavedSettingFlags = m_pSettingExPlus->DefaultSettingFlags;
    }
    m_pSettingExPlus->DefaultSettingFlags = SettingFlags;
    if (bSetFlagsToDefault)
    {
        SetFlags(SettingFlags);
    }
}

/** Set one flag to enabled or disabled
*/
void CSimpleSetting::SetFlag(eSettingFlags Flag, BOOL bEnabled)
{
   eSettingFlags SettingFlags = (eSettingFlags)((m_pSettingExPlus->SettingFlags&~Flag) | (bEnabled?Flag:0));
   SetFlags(SettingFlags);
}

eSettingFlags CSimpleSetting::GetFlags()
{
    return (eSettingFlags)m_pSettingExPlus->SettingFlags;
}

eSettingFlags CSimpleSetting::GetDefaultFlags()
{
	return (eSettingFlags)m_pSettingExPlus->DefaultSettingFlags;
}	

eSettingFlags CSimpleSetting::GetLastSavedFlagsValue()
{
	return (eSettingFlags)m_pSettingExPlus->LastSavedSettingFlags;
}

/** Read value from sub section in .ini file
    @param szSubSection Set to NULL to read from the default location
    @param Value If not NULL, this value is set instead of the Setting's value
    @param bSetDefaultOnFailure If the setting was not in the .ini file, set the setting's value to the default value
    @param OnChangeType Call type for OnChange function. (ONCHANGE_NONE for no call)
    @param pSettingFlags Override setting flags of current setting if not NULL
    @return TRUE if value was in .ini file            
*/
BOOL CSimpleSetting::ReadFromIniSubSection(LPCSTR szSubSection, long* Value, BOOL bSetDefaultOnFailure, eOnChangeType OnChangeType, eSettingFlags* pSettingFlags)
{
    long nValue;
    BOOL IsSettingInIniFile = TRUE;

    if(m_pSetting->szIniSection != NULL)
    {        
        string sEntry;
		char* szIniEntry;
		if (szSubSection == NULL)
		{	
			szSubSection = m_pSetting->szIniSection;
			szIniEntry = m_pSetting->szIniEntry;
		}
		else
		{
			sEntry = m_pSetting->szIniSection;
			sEntry += "_";
			sEntry+= m_pSetting->szIniEntry;
			szIniEntry = (char*)sEntry.c_str();
		}
		if (pSettingFlags == NULL) { pSettingFlags = (eSettingFlags*)&m_pSettingExPlus->SettingFlags; }
		
        //nValue = GetPrivateProfileInt(szSubSectionn, szIniEntry, m_pSetting->MinValue-100, GetIniFileForSettings());
		char szDefaultString[] = {0};
		char szBuffer[256];
		
		int Len = GetPrivateProfileString(szSubSection, szIniEntry, szDefaultString, szBuffer, 255, GetIniFileForSettings());
		if (Len <= 0)
		{
			IsSettingInIniFile = FALSE;
			nValue = m_pSetting->Default;
		}
		else
		{
			IsSettingInIniFile = TRUE;

			char* szValue = szBuffer;
			if (*pSettingFlags & (SETTINGFLAG_HEXVALUE|SETTINGFLAG_BITMASK))
			{
				if ((Len>=2) && (szBuffer[0]=='0') && (szBuffer[1]=='x'))
				{
					sscanf(szBuffer,"0x%x",&nValue);
				}
				else
				{
					nValue = atoi(szValue);
				}
			}
			else
			{
				nValue = atoi(szValue);
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
		}
        if (IsSettingInIniFile || bSetDefaultOnFailure)
        {            
			if (Value != NULL)
			{
				*Value = nValue;
			}
			else
			{
				int OldValue = *m_pSetting->pValue;
				*m_pSetting->pValue = nValue;
			
				if (DoOnChange(*m_pSetting->pValue, OldValue, OnChangeType))
	            {
		            OnChange(*m_pSetting->pValue, OldValue, OnChangeType);
			    }
				m_pSetting->LastSavedValue = nValue;
				m_sLastSavedValueIniSection = szSubSection;
				m_pSettingExPlus->szLastSavedValueIniSection = (char*)m_sLastSavedValueIniSection.c_str();
			}
        }        
    }
    else
    {
        return FALSE;
    }
    return IsSettingInIniFile;
}

/** Read value from default location in .ini file
*/
BOOL CSimpleSetting::ReadFromIni(BOOL bSetDefaultOnFailure, eOnChangeType OnChangeType)
{
	return ReadFromIniSubSection(NULL,NULL, bSetDefaultOnFailure, OnChangeType, NULL);
}

LPCSTR CSimpleSetting::GetLastSavedValueIniSection()
{
    return m_pSettingExPlus->szLastSavedValueIniSection;
}

/** Write value to szSubsection in .ini file
    Override value and setting flags if Value and/or pSettingFlags is not NULL.
*/
void CSimpleSetting::WriteToIniSubSection(LPCSTR szSubSection, BOOL bOptimizeFileAccess, long* Value, eSettingFlags* pSettingFlags)
{
    if(m_pSetting->szIniSection != NULL)
    {
        string sEntry;
		char* szIniEntry;
		if (szSubSection == NULL)
		{	
			szSubSection = m_pSetting->szIniSection;
			szIniEntry = m_pSetting->szIniEntry;
		}
		else
		{
			sEntry = m_pSetting->szIniSection;
			sEntry += "_";
			sEntry+= m_pSetting->szIniEntry;
			szIniEntry = (char*)sEntry.c_str();
		}

		long Val;
		if (Value != NULL) { Val = *Value; } else { Val = *m_pSetting->pValue; }
		if (pSettingFlags == NULL) { pSettingFlags = (eSettingFlags*)&m_pSettingExPlus->SettingFlags; }

		if(!bOptimizeFileAccess || (Val != m_pSetting->LastSavedValue) || ((m_pSettingExPlus->szLastSavedValueIniSection!=NULL) && (strcmp(m_pSettingExPlus->szLastSavedValueIniSection, szSubSection))))
        {	        
			if (*pSettingFlags & (SETTINGFLAG_HEXVALUE|SETTINGFLAG_BITMASK))
			{
				char szBuffer[12];
				sprintf(szBuffer,"0x%08x",Val);
				WritePrivateProfileString(szSubSection, szIniEntry, szBuffer, GetIniFileForSettings());
			}
			else
			{
				WritePrivateProfileInt(szSubSection, szIniEntry, Val, GetIniFileForSettings());
			}
            m_pSetting->LastSavedValue = Val;
			
			m_sLastSavedValueIniSection = szSubSection;
			m_pSettingExPlus->szLastSavedValueIniSection = (char*)m_sLastSavedValueIniSection.c_str();
        }
    }
}

void CSimpleSetting::WriteToIni(BOOL bOptimizeFileAccess)
{
    WriteToIniSubSection(NULL, bOptimizeFileAccess, NULL);	
}
 

 /** Change default value
*/
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

/** Default OnChange function
    Child class can override

    This function calls the pfnExOnChange function
    or the pfnOnChange function of the setting
    if one of them is not NULL
*/
void CSimpleSetting::OnChange(long NewValue, long OldValue, eOnChangeType OnChangeType)
{
    //No override, try static
    if ((m_pSettingExPlus->pfnExOnChange!=NULL) && (OnChangeType!=ONCHANGE_NONE))
    {
        m_pSettingExPlus->pfnExOnChange(m_pSettingExPlus->pExOnChangeThis, NewValue, OldValue, OnChangeType, m_pSetting);
    }
    else if ((m_pSetting->pfnOnChange!=NULL) && (OnChangeType!=ONCHANGE_NONE))
    {
        m_pSetting->pfnOnChange(NewValue);
    }
}

void CSimpleSetting::FlagsOnChange(eSettingFlags OldFlags, eSettingFlags Flags)
{
	///\todo
}

/** Read setting flags from szSection
*/
BOOL CSimpleSetting::ReadFlagsFromIniSection(LPCSTR szSection, BOOL bSetDefaultOnFailure)
{
	eSettingFlags FlagsSetting = SETTINGFLAG_BITMASK;
	long Flags  = 0;	
	    
    BOOL IsSettingInIniFile = TRUE;
    string sEntry;
	char* szIniEntry;
	char szDefaultString[] = {0};
	char szBuffer[256];

    sEntry = m_pSetting->szIniSection;
	sEntry += "_";
	sEntry+= m_pSetting->szIniEntry;
	szIniEntry = (char*)sEntry.c_str();
	
	int Len = GetPrivateProfileString(szSection, szIniEntry, szDefaultString, szBuffer, 255, GetIniFileForSettings());
	if (Len <= 0)
	{
		IsSettingInIniFile = FALSE;
        Flags = m_pSettingExPlus->DefaultSettingFlags;
	}
	else
	{
		IsSettingInIniFile = TRUE;

		char* szValue = szBuffer;
		if ((Len>=2) && (szBuffer[0]=='0') && (szBuffer[1]=='x'))
		{
			sscanf(szBuffer,"0x%x",&Flags);
		}
		else
		{
			Flags = atoi(szValue);
		}
		    
	    // Make sure only the right flags are read
        long Mask = SETTINGFLAG_FLAGSTOINI_MASK;
        Mask = (Mask & ~SETTINGFLAG_PER_MASK) | ((m_pSettingExPlus->SettingFlags & SETTINGFLAG_ALLOW_MASK)>>16);        
        Flags = (m_pSettingExPlus->SettingFlags&~Mask) | (Flags&Mask);
		m_ReadWriteFlags |= RWFLAG_FLAGIN_INI;
		
		eSettingFlags OldFlags = (eSettingFlags)m_pSettingExPlus->SettingFlags;
		m_pSettingExPlus->SettingFlags = (long)Flags;
		FlagsOnChange(OldFlags, (eSettingFlags)Flags);
		m_pSettingExPlus->LastSavedSettingFlags = Flags;
	}
	return IsSettingInIniFile;
}

/** Read setting flags to szSection
*/
void CSimpleSetting::WriteFlagsToIniSection(LPCSTR szSection, BOOL bOptimizeFileAccess)
{
	// Make sure only the right flags are written
    eSettingFlags FlagsSetting = SETTINGFLAG_BITMASK;
	long Flags = m_pSettingExPlus->SettingFlags;
    long Mask = SETTINGFLAG_FLAGSTOINI_MASK;
    Mask = (Mask & ~SETTINGFLAG_PER_MASK) | ((Flags & SETTINGFLAG_ALLOW_MASK)>>16);        
    Flags = (m_pSettingExPlus->SettingFlags&~Mask) | (Flags&Mask);
	
	if (!bOptimizeFileAccess || (m_pSettingExPlus->SettingFlags != m_pSettingExPlus->LastSavedSettingFlags))
	{
		WriteToIniSubSection(szSection, FALSE, &Flags, &FlagsSetting);
	}
	
	m_ReadWriteFlags |= RWFLAG_FLAGIN_INI;
	m_pSettingExPlus->LastSavedSettingFlags = m_pSettingExPlus->SettingFlags;	
}





/** List setting.
    Specify list with 'pszList',
    for the rest of the parameters: 
    @see CSimpleSetting
*/
CListSetting::CListSetting(LPCSTR DisplayName, long Default, long Max, LPCSTR Section, LPCSTR Entry, const char** pszList, CSettingGroup* pGroup, eSettingFlags SettingFlags, long GUIinfo, SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis) :
    CSimpleSetting(DisplayName, Default, 0, Max, Section, Entry, 1, pGroup, SettingFlags, GUIinfo, pfnExOnChange, pExOnChangeThis)    
{
    m_pSetting->Type = ITEMFROMLIST;
    m_pSetting->pszList = pszList;
}

CListSetting::CListSetting(SETTING* pSetting, CSettingGroup* pGroup, eSettingFlags SettingFlags, long GUIinfo, SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis) : 
	CSimpleSetting(pSetting, pGroup, SettingFlags, GUIinfo, pfnExOnChange, pExOnChangeThis)
{   
}

CListSetting::CListSetting(SETTINGEX* pSetting, CSettingGroup* pGroup) : CSimpleSetting(pSetting, pGroup)
{ 
}

CListSetting::CListSetting(SETTINGEX* pSetting, CSettingGroupList* pGroupList) : CSimpleSetting(pSetting, pGroupList)
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

void CListSetting::SetFromControl(HWND hWnd, eOnChangeType OnChangeType)
{
    SetValue(ComboBox_GetCurSel(hWnd), OnChangeType);
}



/** Slider setting.
    For the parameters: 
    @see CSimpleSetting
*/
CSliderSetting::CSliderSetting(LPCSTR DisplayName, long Default, long Min, long Max, LPCSTR Section, LPCSTR Entry, CSettingGroup* pGroup, eSettingFlags SettingFlags, long GUIinfo, SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis) :
    CSimpleSetting(DisplayName, Default, Min, Max, Section, Entry, 1, pGroup, SettingFlags, GUIinfo, pfnExOnChange, pExOnChangeThis)
{
    m_pSetting->OSDDivider = 1;
    m_pSetting->Type = SLIDER;
}

CSliderSetting::CSliderSetting(SETTING* pSetting, CSettingGroup* pGroup, eSettingFlags SettingFlags, long GUIinfo, SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis) : 
	CSimpleSetting(pSetting, pGroup, SettingFlags, GUIinfo, pfnExOnChange, pExOnChangeThis)
{    
}

CSliderSetting::CSliderSetting(SETTINGEX* pSetting, CSettingGroup* pGroup) : CSimpleSetting(pSetting, pGroup)
{ 
}


CSliderSetting::CSliderSetting(SETTINGEX* pSetting, CSettingGroupList* pGroupList) : CSimpleSetting(pSetting, pGroupList)
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


/** Yes/No setting.
    Has only 0 or 1 as possible values.    
    
    For the rest of the parameters: 
    @see CSimpleSetting
*/
CYesNoSetting::CYesNoSetting(LPCSTR DisplayName, BOOL Default, LPCSTR Section, LPCSTR Entry, CSettingGroup* pGroup, eSettingFlags SettingFlags, long GUIinfo, SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis) :
    CSimpleSetting(DisplayName, Default, 0, 1, Section, Entry, 1, pGroup, SettingFlags, GUIinfo, pfnExOnChange, pExOnChangeThis)
{
    m_pSetting->Type = YESNO;
}

CYesNoSetting::CYesNoSetting(SETTING* pSetting, CSettingGroup* pGroup, eSettingFlags SettingFlags, long GUIinfo, SETTINGEX_ONCHANGE* pfnExOnChange, void* pExOnChangeThis) : 
	CSimpleSetting(pSetting, pGroup, SettingFlags, GUIinfo, pfnExOnChange, pExOnChangeThis)
{  
}

CYesNoSetting::CYesNoSetting(SETTINGEX* pSetting, CSettingGroup* pGroup) : CSimpleSetting(pSetting, pGroup)
{    
}

CYesNoSetting::CYesNoSetting(SETTINGEX* pSetting, CSettingGroupList* pGroupList) : CSimpleSetting(pSetting, pGroupList)
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

void CYesNoSetting::SetFromControl(HWND hWnd, eOnChangeType OnChangeType)
{
    SetValue(Button_GetCheck(hWnd) == BST_CHECKED, OnChangeType);
}

