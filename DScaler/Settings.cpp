/////////////////////////////////////////////////////////////////////////////
// Settings.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           changed to windows Ini file functions
//
//  3 Nov 2000   Michael Eskin         Added override of initial BDELAY setting
//               Conexant Systems      by adding non-zero InitialBDelay in .ini
//                                     File. Changed NTSC defaults to 0x5C
//
// 21 Dec 2000   John Adcock           Added function to setup ini file name
//
// 26 Dec 2000   Eric Schmidt          Fixed remember-last-channel-on-startup.
//
// 05 Jan 2001   John Adcock           Added GetRefreshRate function
//                                     Added DoAccurateFlips parameter
//                                     Added gPALFilmFallbackMode setting
//
// 07 Jan 2001   John Adcock           Added gNTSCFilmFallbackMode setting
//                                     Changed saving of gPulldownMode
//                                     so that we don't ever restart in film mode
//                                     when we're doing autodetect  
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 08 Jan 2001   John Adcock           Made all ini file reads use initial values
//                                     of variables rather than hardcoded values here
//
// 26 Dec 2001   Eric Schmidt          Added Custom Channel Order.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "Settings.h"
#include "Tuner.h"
#include "Audio.h"
#include "BT848.h"
#include "VBI.h"
#include "OutThreads.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "MixerDev.h"
#include "DScaler.h"
#include "ProgramList.h"
#include "other.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "Slider.h"
#include "Splash.h"
#include "OSD.h"
#include "TVCards.h"
#include "VideoSettings.h"
#include "Filter.h"
#include "FieldTiming.h"

char szIniFile[MAX_PATH] = "DScaler.ini";

void SetIniFileForSettings(LPSTR Name)
{
	GetCurrentDirectory(MAX_PATH, szIniFile);
	if (*Name == 0)			// add parm TRB 12/00
	{
		strcat(szIniFile, "\\DScaler.ini");
	}
	else
	{
		strcat(szIniFile, "\\");
		strcat(szIniFile, Name);
	}
}

LPCSTR GetIniFileForSettings()
{
	return szIniFile;
}

void LoadSettingsFromIni()
{
	char szKey[128];
	int i;

	// TVCard setting must be called first as this modifys some defaults for the
	// other settings, but we need to be able to override the defaults.
	TVCard_ReadSettingsFromIni();

	BT848_ReadSettingsFromIni();

	// Video Settings Also modifies defaults and ini sections in other
	// files so must be called early
	VideoSettings_ReadSettingsFromIni();

	// Read in rest of settings from each source files read method
	Aspect_ReadSettingsFromIni();
	BT848_ReadSettingsFromIni();
	DScaler_ReadSettingsFromIni();
	OutThreads_ReadSettingsFromIni();
	Other_ReadSettingsFromIni();
	FD50_ReadSettingsFromIni();
	FD60_ReadSettingsFromIni();
	FD_Common_ReadSettingsFromIni();
	OSD_ReadSettingsFromIni();
	VBI_ReadSettingsFromIni();
	MixerDev_ReadSettingsFromIni();
	Timing_ReadSettingsFromIni();

	GetPrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, DebugLogFilename, MAX_PATH, szIniFile);
	DebugLogEnabled = GetPrivateProfileInt("Files", "DebugLogEnabled", DebugLogEnabled, szIniFile);

	Capture_VBI = (GetPrivateProfileInt("Show", "CaptureVBI", Capture_VBI, szIniFile) != 0);  
	CurrentProgramm = GetPrivateProfileInt("Show", "LastProgram", CurrentProgramm, szIniFile);
	bCustomChannelOrder = GetPrivateProfileInt("Show", "CustomChannelOrder", bCustomChannelOrder, szIniFile);

	AudioSource = (AUDIOMUXTYPE)GetPrivateProfileInt("Sound", "AudioSource", AudioSource, szIniFile);
		
	bSaveSettings = (GetPrivateProfileInt("Show", "SaveSettings", bSaveSettings, szIniFile) != 0);
	CountryCode = GetPrivateProfileInt("Show", "CountryCode", CountryCode, szIniFile);


	MSPMode = GetPrivateProfileInt("MSP", "MSPMode", MSPMode, szIniFile);	
	MSPMajorMode = GetPrivateProfileInt("MSP", "MSPMajorMode", MSPMajorMode, szIniFile);	
	MSPMinorMode = GetPrivateProfileInt("MSP", "MSPMinorMode", MSPMinorMode, szIniFile);	
	MSPStereo = GetPrivateProfileInt("MSP", "MSPStereo", MSPStereo, szIniFile);	
	AutoStereoSelect = (GetPrivateProfileInt("MSP", "MSPAutoStereo", AutoStereoSelect, szIniFile) != 0);	
	InitialVolume = GetPrivateProfileInt("MSP", "Volume", InitialVolume, szIniFile);	
	InitialSpatial = GetPrivateProfileInt("MSP", "Spatial", InitialSpatial, szIniFile);	
	InitialLoudness = GetPrivateProfileInt("MSP", "Loudness", InitialLoudness, szIniFile);	
	InitialBass = GetPrivateProfileInt("MSP", "Bass", InitialBass, szIniFile);	
	InitialTreble = GetPrivateProfileInt("MSP", "Treble", InitialTreble, szIniFile);	
	InitialBalance = GetPrivateProfileInt("MSP", "Balance", InitialBalance, szIniFile);	
	InitialSuperBass = (GetPrivateProfileInt("MSP", "SuperBass", InitialSuperBass, szIniFile) != 0);	

	for(i = 0; i < 5; i++)
	{
		sprintf(szKey, "Equalizer%d", i + 1);
		InitialEqualizer[i] = GetPrivateProfileInt("MSP", szKey, 0, szIniFile);
	}
}

LONG Settings_HandleSettingMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
	LONG RetVal = 0;
	*bDone = TRUE;

	switch(message)
	{
		case WM_ASPECT_GETVALUE:
			RetVal =  Setting_GetValue(Aspect_GetSetting((ASPECT_SETTING)wParam));
			break;
		case WM_BT848_GETVALUE:		
			RetVal =  Setting_GetValue(BT848_GetSetting((BT848_SETTING)wParam));
			break;
		case WM_DSCALER_GETVALUE:		
			RetVal =  Setting_GetValue(DScaler_GetSetting((DSCALER_SETTING)wParam));
			break;
		case WM_OUTHREADS_GETVALUE:
			RetVal =  Setting_GetValue(OutThreads_GetSetting((OUTTHREADS_SETTING)wParam));
			break;
		case WM_OTHER_GETVALUE:		
			RetVal =  Setting_GetValue(Other_GetSetting((OTHER_SETTING)wParam));
			break;
		case WM_FD50_GETVALUE:		
			RetVal =  Setting_GetValue(FD50_GetSetting((FD50_SETTING)wParam));
			break;
		case WM_FD60_GETVALUE:		
			RetVal =  Setting_GetValue(FD60_GetSetting((FD60_SETTING)wParam));
			break;
		case WM_FD_COMMON_GETVALUE:
			RetVal =  Setting_GetValue(FD_Common_GetSetting((FD_COMMON_SETTING)wParam));
			break;
		case WM_TVCARD_GETVALUE:		
			RetVal = Setting_GetValue(TVCard_GetSetting((TVCARD_SETTING)wParam));
			break;
		case WM_VIDEOSETTINGS_GETVALUE:		
			RetVal = Setting_GetValue(VideoSettings_GetSetting((VIDEOSETTINGS_SETTING)wParam));
			break;
		case WM_OSD_GETVALUE:		
			RetVal = Setting_GetValue(OSD_GetSetting((OSD_SETTING)wParam));
			break;
		case WM_VBI_GETVALUE:		
			RetVal = Setting_GetValue(VBI_GetSetting((VBI_SETTING)wParam));
			break;
		case WM_MIXERDEV_GETVALUE:		
			RetVal = Setting_GetValue(MixerDev_GetSetting((MIXERDEV_SETTING)wParam));
			break;
		case WM_TIMING_GETVALUE:		
			RetVal = Setting_GetValue(Timing_GetSetting((TIMING_SETTING)wParam));
			break;

		case WM_ASPECT_SETVALUE:
			Setting_SetValue(Aspect_GetSetting((ASPECT_SETTING)wParam), lParam);
			break;
		case WM_BT848_SETVALUE:		
			Setting_SetValue(BT848_GetSetting((BT848_SETTING)wParam), lParam);
			break;
		case WM_DSCALER_SETVALUE:		
			Setting_SetValue(DScaler_GetSetting((DSCALER_SETTING)wParam), lParam);
			break;
		case WM_OUTHREADS_SETVALUE:
			Setting_SetValue(OutThreads_GetSetting((OUTTHREADS_SETTING)wParam), lParam);
			break;
		case WM_OTHER_SETVALUE:		
			Setting_SetValue(Other_GetSetting((OTHER_SETTING)wParam), lParam);
			break;
		case WM_FD50_SETVALUE:		
			Setting_SetValue(FD50_GetSetting((FD50_SETTING)wParam), lParam);
			break;
		case WM_FD60_SETVALUE:		
			Setting_SetValue(FD60_GetSetting((FD60_SETTING)wParam), lParam);
			break;
		case WM_FD_COMMON_SETVALUE:
			Setting_SetValue(FD_Common_GetSetting((FD_COMMON_SETTING)wParam), lParam);
			break;
		case WM_TVCARD_SETVALUE:		
			Setting_SetValue(TVCard_GetSetting((TVCARD_SETTING)wParam), lParam);
			break;
		case WM_VIDEOSETTINGS_SETVALUE:		
			Setting_SetValue(VideoSettings_GetSetting((VIDEOSETTINGS_SETTING)wParam), lParam);
			break;
		case WM_OSD_SETVALUE:		
			Setting_SetValue(OSD_GetSetting((OSD_SETTING)wParam), lParam);
			break;
		case WM_VBI_SETVALUE:		
			Setting_SetValue(VBI_GetSetting((VBI_SETTING)wParam), lParam);
			break;
		case WM_MIXERDEV_SETVALUE:		
			Setting_SetValue(MixerDev_GetSetting((MIXERDEV_SETTING)wParam), lParam);
			break;
		case WM_TIMING_SETVALUE:		
			Setting_SetValue(Timing_GetSetting((TIMING_SETTING)wParam), lParam);
			break;

		case WM_ASPECT_CHANGEVALUE:
			Setting_ChangeValue(Aspect_GetSetting((ASPECT_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_BT848_CHANGEVALUE:		
			Setting_ChangeValue(BT848_GetSetting((BT848_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_DSCALER_CHANGEVALUE:		
			Setting_ChangeValue(DScaler_GetSetting((DSCALER_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_OUTHREADS_CHANGEVALUE:
			Setting_ChangeValue(OutThreads_GetSetting((OUTTHREADS_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_OTHER_CHANGEVALUE:		
			Setting_ChangeValue(Other_GetSetting((OTHER_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_FD50_CHANGEVALUE:		
			Setting_ChangeValue(FD50_GetSetting((FD50_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_FD60_CHANGEVALUE:		
			Setting_ChangeValue(FD60_GetSetting((FD60_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_FD_COMMON_CHANGEVALUE:
			Setting_ChangeValue(FD_Common_GetSetting((FD_COMMON_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_TVCARD_CHANGEVALUE:		
			Setting_ChangeValue(TVCard_GetSetting((TVCARD_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_VIDEOSETTINGS_CHANGEVALUE:		
			Setting_ChangeValue(VideoSettings_GetSetting((VIDEOSETTINGS_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_OSD_CHANGEVALUE:		
			Setting_ChangeValue(OSD_GetSetting((OSD_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_VBI_CHANGEVALUE:		
			Setting_ChangeValue(VBI_GetSetting((VBI_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_MIXERDEV_CHANGEVALUE:		
			Setting_ChangeValue(MixerDev_GetSetting((MIXERDEV_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		case WM_TIMING_CHANGEVALUE:		
			Setting_ChangeValue(Timing_GetSetting((TIMING_SETTING)wParam), (eCHANGEVALUE)lParam);
			break;
		
		default:
			*bDone = FALSE;
			break;
	}
    return RetVal;
}


void WriteSettingsToIni()
{
	char szKey[128];
	int i;

	Aspect_WriteSettingsToIni();
	BT848_WriteSettingsToIni();
	DScaler_WriteSettingsToIni();
	OutThreads_WriteSettingsToIni();
	Other_WriteSettingsToIni();
	FD50_WriteSettingsToIni();
	FD60_WriteSettingsToIni();
	FD_Common_WriteSettingsToIni();
	Deinterlace_WriteSettingsToIni();
	TVCard_WriteSettingsToIni();
	VideoSettings_WriteSettingsToIni();
	OSD_WriteSettingsToIni();
	Filter_WriteSettingsToIni();
	VBI_WriteSettingsToIni();
	MixerDev_WriteSettingsToIni();
	Timing_WriteSettingsToIni();

	WritePrivateProfileString("Files", "DebugLogFilename", DebugLogFilename, szIniFile);
	WritePrivateProfileInt("Files", "DebugLogEnabled", DebugLogEnabled, szIniFile);

	WritePrivateProfileInt("Show", "CaptureVBI", Capture_VBI, szIniFile);
	WritePrivateProfileInt("Show", "LastProgram", CurrentProgramm, szIniFile);
	WritePrivateProfileInt("Show", "CustomChannelOrder", bCustomChannelOrder, szIniFile);

	WritePrivateProfileInt("Sound", "AudioSource", AudioSource, szIniFile);

	WritePrivateProfileInt("Show", "SaveSettings", bSaveSettings, szIniFile);
	WritePrivateProfileInt("Show", "CountryCode", CountryCode, szIniFile);

	WritePrivateProfileInt("MSP", "MSPMode", MSPMode, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPMajorMode", MSPMajorMode, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPMinorMode", MSPMinorMode, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPStereo", MSPStereo, szIniFile);	
	WritePrivateProfileInt("MSP", "MSPAutoStereo", AutoStereoSelect, szIniFile);	
	WritePrivateProfileInt("MSP", "Volume", InitialVolume, szIniFile);	
	WritePrivateProfileInt("MSP", "Spatial", InitialSpatial, szIniFile);	
	WritePrivateProfileInt("MSP", "Loudness", InitialLoudness, szIniFile);	
	WritePrivateProfileInt("MSP", "Bass", InitialBass, szIniFile);	
	WritePrivateProfileInt("MSP", "Treble", InitialTreble, szIniFile);	
	WritePrivateProfileInt("MSP", "Balance", InitialBalance, szIniFile);	
	WritePrivateProfileInt("MSP", "SuperBass", InitialSuperBass, szIniFile);	

	for(i = 0; i < 5; i++)
	{
		sprintf(szKey, "Equalizer%d", i + 1);
		WritePrivateProfileInt("MSP", szKey, InitialEqualizer[i], szIniFile);
	}
   
	// These two lines flushes current INI file to disk (in case of abrupt poweroff shortly afterwards)
    WritePrivateProfileString(NULL, NULL, NULL, szIniFile);
    
    // Disk cache flushing:
    // The below doesn't seem to be needed and only seem to 
    // cause the video to freeze for a full second.
    //_flushall(); 
}

void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName)
{
	char szValue[128];
	sprintf(szValue, "%d", nValue);
	WritePrivateProfileString(lpAppName,  lpKeyName,  szValue, lpFileName);
}

// Start of new UI code
// Not to be used yet

void SetControlVisibility(HWND hDlg, int ControlID, BOOL IsVisible)
{
	if(!IsVisible)
	{
		ShowWindow(GetDlgItem(hDlg, ControlID), SW_HIDE);
	}
}

long Setting_GetValue(SETTING* pSetting)
{
	if(pSetting == NULL)
	{
		return -1;
	}
	switch(pSetting->Type)
	{
	case YESNO:
	case ONOFF:
		return (BOOL) *pSetting->pValue;
		break;
	case ITEMFROMLIST:
	case SLIDER:
	case NUMBER:
		return *pSetting->pValue;
		break;
	default:
		return 0;
	}
}

BOOL Setting_SetValue(SETTING* pSetting, long Value)
{
	long NewValue;
	if(pSetting == NULL)
	{
		return FALSE;
	}

	switch(pSetting->Type)
	{
	case YESNO:
	case ONOFF:
		 NewValue = (Value != 0);
		break;
	case ITEMFROMLIST:
	case SLIDER:
	case NUMBER:
		if(Value > pSetting->MaxValue)
		{
			NewValue = pSetting->MaxValue;
		}
		else if(Value < pSetting->MinValue)
		{
			NewValue = pSetting->MinValue;
		}
		else
		{
			NewValue = Value;
		}
		break;
	default:
		return FALSE;
		break;
	}
	
	if(pSetting->pfnOnChange != NULL)
	{
		return pSetting->pfnOnChange(NewValue); 
	}
	else
	{
		*pSetting->pValue = NewValue;
		return FALSE;
	}
}

void Setting_SetDefault(SETTING* pSetting)
{
	Setting_SetValue(pSetting, pSetting->Default);
}

void Setting_SetupSlider(SETTING* pSetting, HWND hSlider)
{
	Slider_SetRangeMax(hSlider, pSetting->MaxValue);
	Slider_SetRangeMin(hSlider, pSetting->MinValue);
	Slider_SetPageSize(hSlider, pSetting->StepValue);
	Slider_SetLineSize(hSlider, 1);
	Slider_SetTic(hSlider, pSetting->Default);
	Setting_SetControlValue(pSetting, hSlider);
}

void Setting_SetControlValue(SETTING* pSetting, HWND hControl)
{
	char szBuffer[15];

	switch(pSetting->Type)
	{
	case YESNO:
	case ONOFF:
		Button_SetCheck(hControl, *pSetting->pValue);
		break;

	case ITEMFROMLIST:
		ComboBox_SetCurSel(hControl, *pSetting->pValue);
		break;

	case SLIDER:
		Slider_SetPos(hControl, *pSetting->pValue);
		break;
	
	case NUMBER:
		Edit_SetText(hControl, _itoa(*pSetting->pValue, szBuffer, 10));
		break;
	default:
		break;
	}
}

BOOL Setting_SetFromControl(SETTING* pSetting, HWND hControl)
{
	long nValue;
	char szBuffer[15];

	switch(pSetting->Type)
	{
	case YESNO:
	case ONOFF:
		nValue = (Button_GetCheck(hControl) == BST_CHECKED);
		break;

	case ITEMFROMLIST:
		nValue = ComboBox_GetCurSel(hControl);
		break;

	case SLIDER:
		nValue = Slider_GetPos(hControl);
		break;
	
	case NUMBER:
		Edit_GetText(hControl, szBuffer, 15);
		nValue = atoi(szBuffer);
		break;
	default:
		break;
	}
	return Setting_SetValue(pSetting, nValue);
}

HWND Setting_CreateControl(SETTING* pSetting, HWND hDlg, int* VertPos)
{
	HWND hNewControl = NULL;
	
	switch(pSetting->Type)
	{
	case YESNO:
	case ONOFF:
		break;
	case ITEMFROMLIST:
		break;
	case SLIDER:
		break;
	case NUMBER:
		break;
	default:
		break;
	}
	*VertPos += 20;
	return hNewControl;
}

void Setting_SetupControl(SETTING* pSetting, HWND hControl)
{
	int i;

	switch(pSetting->Type)
	{
	case ITEMFROMLIST:
		i = 0;
		while(pSetting->pszList[i] != NULL)
		{
			ComboBox_InsertString(hControl, -1, pSetting->pszList[i]);
		}
		break;

	case SLIDER:
		Setting_SetupSlider(pSetting, hControl);
		break;
	default:
		break;
	}
	Setting_SetControlValue(pSetting, hControl);
}

void Setting_ReadFromIni(SETTING* pSetting)
{
	long nValue;

	if(pSetting->szIniSection != NULL)
	{
		nValue = GetPrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
		if(nValue < pSetting->MinValue)
		{
			nValue = pSetting->Default;
		}
		*pSetting->pValue = nValue;
		pSetting->OriginalValue = *pSetting->pValue;
	}
}

void Setting_WriteToIni(SETTING* pSetting)
{
	if(pSetting->szIniSection != NULL)
	{
//		if(pSetting->OriginalValue != *pSetting->pValue)
		{
//			if(pSetting->Default != *pSetting->pValue)
			{
				WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, *pSetting->pValue, szIniFile);
			}
//			else
//			{
//				WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
//			}
		}
	}
}

void Setting_OSDShow(SETTING* pSetting, HWND hWnd)
{
	char szBuffer[1024] = "Unexpected Display Error";

	switch(pSetting->Type)
	{
	case ITEMFROMLIST:
		sprintf(szBuffer, "%s %s", pSetting->szDisplayName, pSetting->pszList[*(pSetting->pValue)]);
		break;
	case YESNO:
		sprintf(szBuffer, "%s %s", pSetting->szDisplayName, *(pSetting->pValue)?"YES":"NO");
		break;
	case ONOFF:
		sprintf(szBuffer, "%s %s", pSetting->szDisplayName, *(pSetting->pValue)?"ON":"OFF");
		break;
	case SLIDER:
	case NUMBER:
		if(pSetting->OSDDivider == 1)
		{
			sprintf(szBuffer, "%s %d", pSetting->szDisplayName, *(pSetting->pValue));
		}
		else if(pSetting->OSDDivider == 8)
		{
			sprintf(szBuffer, "%s %.3f", pSetting->szDisplayName, (float)*(pSetting->pValue) / (float)pSetting->OSDDivider);
		}
		else
		{
			sprintf(szBuffer, "%s %.*f", pSetting->szDisplayName, (int)log10(pSetting->OSDDivider), (float)*(pSetting->pValue) / (float)pSetting->OSDDivider);
		}
		break;
	default:
		break;
	}
	OSD_ShowText(hWnd, szBuffer, 0);
}
//---------------------------------------------------------------------------
// This function allows for accelerated slider adjustments
// For example, adjusting Contrast or Brightness faster the longer 
// you hold down the adjustment key.
int GetCurrentAdjustmentStepCount(SETTING* pSetting)
{
    static DWORD        dwLastTick = 0;
    static DWORD        dwFirstTick = 0;
    static DWORD        dwTaps = 0;
    DWORD               dwTick;
    DWORD               dwElapsedSinceLastCall;
    DWORD               dwElapsedSinceFirstTick;
    int                 nStepCount;
	static SETTING* 	LastSetting = NULL;

	if(LastSetting != pSetting)
	{
		dwLastTick = 0;
		dwFirstTick = 0;
		dwTaps = 0;
		LastSetting = pSetting;
	}

    dwTick = GetTickCount();
    dwElapsedSinceLastCall = dwTick - dwLastTick;
    dwElapsedSinceFirstTick = dwTick - dwFirstTick;

    if ((dwTaps < ADJ_MINIMUM_REPEAT_BEFORE_ACCEL) &&
        (dwElapsedSinceLastCall < ADJ_BUTTON_REPRESS_REPEAT_DELAY))
    {
        // Ensure that the button or keypress is repeated or tapped
        // a minimum number of times before acceleration begins
        dwFirstTick = dwTick;
        nStepCount = 1;
    }
    if (dwElapsedSinceLastCall < ADJ_KEYB_TYPEMATIC_REPEAT_DELAY)
    {
        // This occurs if the end-user is holding down a keyboard key.
        // The longer the time has elapsed since the keyboard key has
        // been held down, the bigger the adjustment steps become, up to a maximum.
        nStepCount = 1 + (dwElapsedSinceFirstTick / ADJ_KEYB_TYPEMATIC_ACCEL_STEP);
        if (nStepCount > ADJ_KEYB_TYPEMATIC_MAX_STEP)
        {
            nStepCount = ADJ_KEYB_TYPEMATIC_MAX_STEP;
        }
    }
    else if (dwElapsedSinceLastCall < ADJ_BUTTON_REPRESS_REPEAT_DELAY)
    {
        // This occurs if the end-user is tapping a button repeatedly
        // such as on a handheld remote control, when a universal remote
        // is programmed with a keypress.  Most remotes cannot repeat 
        // a keypress automatically, so the end user must tap the key.
        // The longer the time has elapsed since the first button press,
        // the bigger the adjustment steps become, up to a maximum.
        nStepCount = 1 + (dwElapsedSinceFirstTick / ADJ_BUTTON_REPRESS_ACCEL_STEP);
        if (nStepCount > ADJ_BUTTON_REPRESS_MAX_STEP)
        {
            nStepCount = ADJ_BUTTON_REPRESS_MAX_STEP;
        }
    }
    else
    {
        // The keypress or button press is no longer consecutive, 
        // so reset adjustment step.
        dwFirstTick = dwTick;
        nStepCount = 1;
        dwTaps = 0;
    }
    dwTaps++;
    dwLastTick = dwTick;
    return nStepCount;
}

void Setting_SetSection(SETTING* pSetting, LPSTR NewValue)
{
	if(pSetting != NULL)
	{
		pSetting->szIniSection = NewValue;
	}
}

void Setting_Up(SETTING* pSetting)
{
    int nStep = 0;

    if (*pSetting->pValue < pSetting->MaxValue)
    {
        nStep = GetCurrentAdjustmentStepCount(pSetting) * pSetting->StepValue;
		Setting_SetValue(pSetting, *pSetting->pValue + nStep);
    }
}

void Setting_Down(SETTING* pSetting)
{
    int nStep = 0;

    if (*pSetting->pValue > pSetting->MinValue)
    {
        nStep = GetCurrentAdjustmentStepCount(pSetting) * pSetting->StepValue;
		Setting_SetValue(pSetting, *pSetting->pValue - nStep);
    }
}

void Setting_ChangeValue(SETTING* pSetting, eCHANGEVALUE NewValue)
{
	if(pSetting == NULL)
	{
		return;
	}
	switch(NewValue)
	{
	case DISPLAY:
		Setting_OSDShow(pSetting, hWnd);
		break;
	case ADJUSTUP:
		Setting_Up(pSetting);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case ADJUSTDOWN:
		Setting_Down(pSetting);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case INCREMENT:
		Setting_SetValue(pSetting, Setting_GetValue(pSetting) + pSetting->StepValue);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case DECREMENT:
		Setting_SetValue(pSetting, Setting_GetValue(pSetting) - pSetting->StepValue);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case RESET:
		Setting_SetDefault(pSetting);
		Setting_OSDShow(pSetting, hWnd);
		break;
	case TOGGLEBOOL:
		if(pSetting->Type == YESNO || pSetting->Type == ONOFF)
		{
			Setting_SetValue(pSetting, !Setting_GetValue(pSetting));
			Setting_OSDShow(pSetting, hWnd);
		}
		break;
	case ADJUSTUP_SILENT:
		Setting_Up(pSetting);
		break;
	case ADJUSTDOWN_SILENT:
		Setting_Down(pSetting);
		break;
	case INCREMENT_SILENT:
		Setting_SetValue(pSetting, Setting_GetValue(pSetting) + pSetting->StepValue);
		break;
	case DECREMENT_SILENT:
		Setting_SetValue(pSetting, Setting_GetValue(pSetting) - pSetting->StepValue);
		break;
	case RESET_SILENT:
		Setting_SetDefault(pSetting);
		break;
	case TOGGLEBOOL_SILENT:
		if(pSetting->Type == YESNO || pSetting->Type == ONOFF)
		{
			Setting_SetValue(pSetting, !Setting_GetValue(pSetting));
		}
		break;
	default:
		break;
	}
}

BOOL APIENTRY UISubMenuProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	static UI_SUBMENU* pSubMenu;
	int i;
	HWND Controls[8];
	long PrevValues[8];
	int VertPos = 0;

	switch (message)
	{
	case WM_INITDIALOG:
		pSubMenu = (UI_SUBMENU*)lParam;
		SetWindowText(hDlg, pSubMenu->szDisplayName);
		for(i = 0;i < 8;i++)
		{
			Controls[i] = Setting_CreateControl(pSubMenu->Elements[i], hDlg, &VertPos);
			Setting_SetupControl(pSubMenu->Elements[i], Controls[i]);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC1 + i), pSubMenu->Elements[i]->szDisplayName);
			PrevValues[i] = Setting_GetValue(pSubMenu->Elements[i]);
		}
		break;

	case WM_COMMAND:
		switch LOWORD(wParam)
		{
		case IDOK:
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			// revert to old value
			for(i = 0;i < 8 && pSubMenu->Elements[i] != NULL;i++)
			{
				Setting_SetValue(pSubMenu->Elements[i], PrevValues[i]);
			}
			EndDialog(hDlg, FALSE);
			break;

		case IDDEFAULTS:
			// revert to default values
			for(i = 0;i < 8 && pSubMenu->Elements[i] != NULL;i++)
			{
				*pSubMenu->Elements[i]->pValue = pSubMenu->Elements[i]->Default;
				Setting_SetControlValue(pSubMenu->Elements[i], Controls[i]);
			}
			break;

		case IDC_CHECK1:
		case IDC_CHECK2:
		case IDC_CHECK3:
		case IDC_CHECK4:
		case IDC_CHECK5:
		case IDC_CHECK6:
		case IDC_CHECK7:
		case IDC_CHECK8:
			i = LOWORD(wParam) -  IDC_CHECK1;
			Setting_SetFromControl(pSubMenu->Elements[i], Controls[i]);
			break;
		case WM_VSCROLL:
		case WM_HSCROLL:
			for(i = 0; i < 8; i++)
			{
				if((HWND)lParam == Controls[i])
				{
					Setting_SetFromControl(pSubMenu->Elements[i], Controls[i]);
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return FALSE;
}

void DisplayUISubMenuAsDialog(UI_SUBMENU* pSubMenu)
{
	DialogBoxParam(hInst, "UI_SUB_MENU", hWnd, UISubMenuProc, (LPARAM)pSubMenu);
}

