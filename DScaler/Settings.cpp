/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
 * @file Settings.cpp Settings Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Settings.h"
#include "Audio.h"
#include "VBI.h"
#include "OutThreads.h"
#include "deinterlace.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "MixerDev.h"
#include "DScaler.h"
#include "ProgramList.h"
#include "OverlayOutput.h"
#include "D3D9Output.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "FD_Common.h"
#include "FD_Prog.h"
#include "Slider.h"
#include "Splash.h"
#include "OSD.h"
#include "Filter.h"
#include "FieldTiming.h"
#include "VBI_VideoText.h"
#include "Providers.h"
#include "Calibration.h"
#include "StillSource.h"
#include "SettingsPerChannel.h"
#include "TimeShift.h"
#include "EPG.h"

typedef SETTING* (__cdecl GENERICGETSETTING)(long SettingIndex);
typedef void (__cdecl GENERICREADSETTINGS)();
typedef void (__cdecl GENERICWRITESETTINGS)(BOOL);
typedef void (__cdecl GENERICFREESETTINGS)();

typedef struct
{
    UINT GetValueMessage;    
    GENERICGETSETTING* pfnGetSetting;        
    GENERICREADSETTINGS* pfnReadSettings;    
    GENERICWRITESETTINGS* pfnWriteSettings;
    GENERICFREESETTINGS* pfnFreeSettings;
} TFileWithSettings;

TFileWithSettings Settings[] =
{
    {
        WM_DEBUG_GETVALUE,
        (GENERICGETSETTING*)Debug_GetSetting,
        Debug_ReadSettingsFromIni,
        Debug_WriteSettingsToIni,
        Debug_FreeSettings,
    },
    {
        WM_ASPECT_GETVALUE,
        (GENERICGETSETTING*)Aspect_GetSetting,
        Aspect_ReadSettingsFromIni,
        Aspect_WriteSettingsToIni,
        NULL,
    },
    {
        WM_DSCALER_GETVALUE,
        (GENERICGETSETTING*)DScaler_GetSetting,
        DScaler_ReadSettingsFromIni,
        DScaler_WriteSettingsToIni,
        DScaler_FreeSettings,
    },
    {
        WM_OUTTHREADS_GETVALUE,
        (GENERICGETSETTING*)OutThreads_GetSetting,
        OutThreads_ReadSettingsFromIni,
        OutThreads_WriteSettingsToIni,
        NULL,
    },
    {
        WM_OTHER_GETVALUE,
        (GENERICGETSETTING*)Overlay_GetSetting,
        Overlay_ReadSettingsFromIni,
        Overlay_WriteSettingsToIni,
        NULL,
    },
    {
        WM_FD50_GETVALUE,
        (GENERICGETSETTING*)FD50_GetSetting,
        FD50_ReadSettingsFromIni,
        FD50_WriteSettingsToIni,
        NULL,
    },
    {
        WM_FD60_GETVALUE,
        (GENERICGETSETTING*)FD60_GetSetting,
        FD60_ReadSettingsFromIni,
        FD60_WriteSettingsToIni,
        NULL,
    },
    {
        WM_FD_COMMON_GETVALUE,
        (GENERICGETSETTING*)FD_Common_GetSetting,
        FD_Common_ReadSettingsFromIni,
        FD_Common_WriteSettingsToIni,
        NULL,
    },
    {
        WM_OSD_GETVALUE,
        (GENERICGETSETTING*)OSD_GetSetting,
        OSD_ReadSettingsFromIni,
        OSD_WriteSettingsToIni,
        OSD_FreeSettings,
    },
    {
        WM_VBI_GETVALUE,
        (GENERICGETSETTING*)VBI_GetSetting,
        VBI_ReadSettingsFromIni,
        VBI_WriteSettingsToIni,
        NULL,
    },
    {
        WM_TIMING_GETVALUE,
        (GENERICGETSETTING*)Timing_GetSetting,
        Timing_ReadSettingsFromIni,
        Timing_WriteSettingsToIni,
        NULL,
    },
    {
        WM_MIXERDEV_GETVALUE,
        (GENERICGETSETTING*)MixerDev_GetSetting,
        MixerDev_ReadSettingsFromIni,
        MixerDev_WriteSettingsToIni,
        MixerDev_FreeSettings,
    },
    {
        WM_CHANNELS_GETVALUE,
        (GENERICGETSETTING*)Channels_GetSetting,
        Channels_ReadSettingsFromIni,
        Channels_WriteSettingsToIni,
        NULL,
    },
    {
        WM_AUDIO_GETVALUE,
        (GENERICGETSETTING*)Audio_GetSetting,
        Audio_ReadSettingsFromIni,
        Audio_WriteSettingsToIni,
        NULL,
    },
    {
        WM_VT_GETVALUE,
        (GENERICGETSETTING*)VT_GetSetting,
        VT_ReadSettingsFromIni,
        VT_WriteSettingsToIni,
        NULL,
    },
    {
        WM_CALIBR_GETVALUE,
        (GENERICGETSETTING*)Calibr_GetSetting,
        Calibr_ReadSettingsFromIni,
        Calibr_WriteSettingsToIni,
        NULL,
    },
    {
        WM_STILL_GETVALUE,
        (GENERICGETSETTING*)Still_GetSetting,
        Still_ReadSettingsFromIni,
        Still_WriteSettingsToIni,
        Still_FreeSettings,
    },
    {
        WM_ANTIPLOP_GETVALUE,
        (GENERICGETSETTING*)AntiPlop_GetSetting,
        AntiPlop_ReadSettingsFromIni,
        AntiPlop_WriteSettingsToIni,
        NULL,
    },
    {
        WM_SETTINGSPERCHANNEL_GETVALUE,
        (GENERICGETSETTING*)SettingsPerChannel_GetSetting,
        SettingsPerChannel_ReadSettingsFromIni,
        SettingsPerChannel_WriteSettingsToIni,
        NULL,
    },
    {
        WM_FDPROG_GETVALUE,
        (GENERICGETSETTING*)FDProg_GetSetting,
        FDProg_ReadSettingsFromIni,
        FDProg_WriteSettingsToIni,
        NULL,
    },
    {
        WM_EPG_GETVALUE,
        (GENERICGETSETTING*)EPG_GetSetting,
        EPG_ReadSettingsFromIni,
        EPG_WriteSettingsToIni,
        EPG_FreeSettings,
    },
    {
        WM_D3D9_GETVALUE,
        (GENERICGETSETTING*)D3D9_GetSetting,
        D3D9_ReadSettingsFromIni,
        D3D9_WriteSettingsToIni,
        NULL,
    },
};

#define NUMSETTINGS (sizeof(Settings)/ sizeof(TFileWithSettings))

char szIniFile[MAX_PATH] = "DScaler.ini";

void SetIniFileForSettings(LPSTR Name)
{
    GetCurrentDirectory(MAX_PATH, szIniFile);
    if (*Name == 0)         // add parm TRB 12/00
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
    for(int i(0); i < NUMSETTINGS; ++i)
    {
        if(Settings[i].pfnReadSettings != NULL)
        {
            Settings[i].pfnReadSettings();
        }
    }
    Providers_ReadFromIni();       
}

LONG Settings_HandleSettingMsgs(HWND hWnd, UINT message, UINT wParam, LONG lParam, BOOL* bDone)
{
    LONG RetVal = 0;
    *bDone = FALSE;
    int i;

    for(i = 0; (*bDone == FALSE) && (i < NUMSETTINGS); ++i)
    {
        if(message == Settings[i].GetValueMessage)
        {
            RetVal =  Setting_GetValue(Settings[i].pfnGetSetting(wParam));
            *bDone = TRUE;
        }
        else if(message == Settings[i].GetValueMessage + 100)
        {
            Setting_SetValue(Settings[i].pfnGetSetting(wParam), lParam);
            *bDone = TRUE;
        }
        else if(message == Settings[i].GetValueMessage + 200)
        {
            Setting_ChangeValue(Settings[i].pfnGetSetting(wParam), (eCHANGEVALUE)lParam);
            *bDone = TRUE;
        }
    }
    return RetVal;
}

// this function removes any comments, leading spaces and tabs, trailing spaces, tabs,
// carriage returns and new line characters
LPTSTR CleanUpLine(LPTSTR lpString)
{
    int i = 0;
    char *string = lpString;
    
    if(string == NULL || string[0] == '\0')
    {
        return lpString;
    }

    // remove leading spaces and tabs
    while(string[0] == ' ' || string[0] == '\t')
    {
        string++;
    }

    strcpy(lpString, string);

    // remove any comments
    string = strchr(lpString, ';');
    if(string != NULL)
    {
        *string = '\0';
    }

    // remove trailing spaces, tabs, newline and carriage return
    i = strlen(lpString) - 1;
    while(i >= 0 && (lpString[i] == ' ' || lpString[i] == '\t' || lpString[i] == '\r') ||
          lpString[i] == '\n')
    {
        lpString[i--] = '\0';
    }
    return lpString;
}

// this function returns TRUE if lpString contains a valid section name ('[blahblah]')
BOOL IsSectionName(LPCTSTR lpString)
{
    char localcopy[2560];

    if(lpString == NULL)
    {
        return FALSE;
    }

    strncpy(localcopy, lpString, sizeof(localcopy));

    CleanUpLine(localcopy);

    if(strlen(localcopy) < 3)
    {
        return FALSE;
    }
    return (localcopy[0] == '[' && localcopy[strlen(localcopy) - 1] == ']');
}

#define TEMPFILENAME "ThisFileCanBeRemoved.tmp"

// This function adds an empty line before each new section in the ini file to greatly
// enhance readability.
void BeautifyIniFile(LPCTSTR lpIniFileName)
{
    FILE* IniFile = NULL;
    FILE* TempFile = NULL;
    char szTempFile[MAX_PATH] = "";
    char buf[2560] = "";
    char lastline[2560] = "";
    BOOL IOError = FALSE;

    buf[2550] = '\0';

    GetCurrentDirectory(MAX_PATH, szTempFile);
    strcat(szTempFile, "\\");
    strcat(szTempFile, TEMPFILENAME);

    TempFile = fopen(szTempFile, "w");
    if(TempFile == NULL)
    {
        LOG(1, "BeautifyIniFile: Error opening temp file for writing.");
        return;        
    }
    else
    {
        IniFile = fopen(lpIniFileName, "r");
        if(IniFile == NULL)
        {
            fclose(TempFile);
            remove(szTempFile);
            LOG(1, "BeautifyIniFile: Error opening ini file for reading.");
            return;
        }
        else
        {
            while(!feof(IniFile))
            {
                if(fgets(buf, 2550, IniFile) == NULL)
                {
                    if(feof(IniFile))
                    {
                        break;
                    }
                    else
                    {
                        IOError = TRUE;
                        LOG(1, "BeautifyIniFile: Error reading ini file.");
                    }
                }
                if(IsSectionName(buf) && lastline[0] != '\0')
                {
                    if(fputs("\n", TempFile) < 0)
                    {
                        IOError = TRUE;
                        LOG(1, "BeautifyIniFile: Error writing temp file (#1).");
                    }
                }
                if(fputs(buf, TempFile) < 0)
                {
                    IOError = TRUE;
                    LOG(1, "BeautifyIniFile: Error writing temp file (#2).");
                }
                if(IOError)
                {
                    fclose(IniFile);
                    fclose(TempFile);
                    remove(szTempFile);
                    return;
                }
                strncpy(lastline, buf, sizeof(lastline));
                CleanUpLine(lastline);
            }
        }
        fclose(IniFile);
        fclose(TempFile);
    }
    remove(lpIniFileName);

    // RM Sept 28 2006: The following while loop and Sleep statement should not be needed.
    // It was added because Antivir Personal Edition Classic was causing problems when
    // the win32 file heuristic of the Antivir Guard was set to the medium level.
    // Antivir causes a delay in removing the ini file which causes the rename to fail.
    //
    int i = 0;
    while(rename(szTempFile, lpIniFileName) != 0)
    {
        Sleep(1);
        if(i++ > 100)
        {
            LOG(1, "BeautifyIniFile: rename failed.");
            break;
        }
    }
    if(i != 0)
    {
        LOG(1, "BeautifyIniFile: Sleep needed %d time(s) before rename.", i);
    }
}

void WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    for(int i(0); i < NUMSETTINGS; ++i)
    {
        if(Settings[i].pfnWriteSettings != NULL)
        {
            Settings[i].pfnWriteSettings(bOptimizeFileAccess);
        }
    }

    Deinterlace_WriteSettingsToIni(bOptimizeFileAccess);
    Filter_WriteSettingsToIni(bOptimizeFileAccess);
    Providers_WriteToIni(bOptimizeFileAccess);

    // These two lines flushes current INI file to disk (in case of abrupt poweroff shortly afterwards)
    WritePrivateProfileString(NULL, NULL, NULL, szIniFile);

    BeautifyIniFile(szIniFile);
}

void FreeSettings()
{
    for(int i(0); i < NUMSETTINGS; ++i)
    {
        if (Settings[i].pfnFreeSettings != NULL)
        {
            Settings[i].pfnFreeSettings();
        }
    }
    Deinterlace_FreeSettings();
}

void WritePrivateProfileInt(LPCTSTR lpAppName,  LPCTSTR lpKeyName,  int nValue, LPCTSTR lpFileName)
{
    char szValue[128];
    sprintf(szValue, "%d", nValue);
    WritePrivateProfileString(lpAppName,  lpKeyName,  szValue, lpFileName);
    LOG(2, " WritePrivateProfileInt %s %s Value %s", lpAppName, lpKeyName, szValue);
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
    case CHARSTRING:
        return *pSetting->pValue;
        break;
    default:
        return 0;
    }
}

BOOL Setting_SetValue(SETTING* pSetting, long Value, int iForceOnChange)
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
    case CHARSTRING:
        NewValue = Value;
        break;
    default:
        return FALSE;
        break;
    }
    
    // If no action is needed, bail out early. This prevents the long delays when
    // pSetting->pfnOnChange() takes a while.
    if (pSetting->Type == CHARSTRING && *pSetting->pValue != NULL)
    {
        if (!strcmp((char *)(*pSetting->pValue), (char *)NewValue) && (iForceOnChange!=1))
        {
            return FALSE;
        }
    }
    else
    {
        if ((*pSetting->pValue == NewValue) && (iForceOnChange!=1))
        {
            return FALSE;
        }
    }

    if ((pSetting->pfnOnChange != NULL) && (iForceOnChange>=0))
    {
        return pSetting->pfnOnChange(NewValue); 
    }
    else
    {
        if (pSetting->Type == CHARSTRING)
        {
            if (*pSetting->pValue != NULL)
            {
                delete [] (char *)(*pSetting->pValue);
            }
            char* str = new char[strlen((char *)NewValue) + 1];
            strcpy(str, (char *)NewValue);
            *pSetting->pValue = (long)str;
        }
        else
        {
            *pSetting->pValue = NewValue;
        }
        return FALSE;
    }
}


void Setting_SetDefault(SETTING* pSetting)
{
    Setting_SetValue(pSetting, pSetting->Default);
}

void Setting_SetupSlider(SETTING* pSetting, HWND hSlider)
{
    Slider_ClearTicks(hSlider, TRUE);
    Slider_SetRangeMax(hSlider, pSetting->MaxValue - pSetting->MinValue);
    Slider_SetRangeMin(hSlider, 0);
    Slider_SetPageSize(hSlider, pSetting->StepValue);
    Slider_SetLineSize(hSlider, pSetting->StepValue);
    if(GetWindowLong(hSlider, GWL_STYLE) & TBS_VERT)
    {
        Slider_SetTic(hSlider, pSetting->MaxValue - pSetting->Default);
    }
    else
    {
        Slider_SetTic(hSlider, pSetting->Default - pSetting->MinValue);
    }
    Setting_SetControlValue(pSetting, hSlider);
}

void Setting_SetControlValue(SETTING* pSetting, HWND hControl)
{
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
        if(GetWindowLong(hControl, GWL_STYLE) & TBS_VERT)
        {
            Slider_SetPos(hControl, pSetting->MaxValue - *pSetting->pValue);
        }
        else
        {
            Slider_SetPos(hControl, *pSetting->pValue - pSetting->MinValue);
        }
        break;
    default:
        break;
    }
}

BOOL Setting_SetFromControl(SETTING* pSetting, HWND hControl)
{
    long nValue;

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
        if(GetWindowLong(hControl, GWL_STYLE) & TBS_VERT)
        {
            nValue = pSetting->MaxValue - nValue;
        }
        else
        {
            nValue = nValue + pSetting->MinValue;
        }
        break;
    
    default:
        return FALSE;
        break;
    }
    return Setting_SetValue(pSetting, nValue);
}

BOOL Setting_ReadFromIni(SETTING* pSetting, BOOL bDontSetDefault)
{
    long nValue;
    long nSavedValue;
    BOOL IsSettingInIniFile = TRUE;

    if(pSetting->szIniSection != NULL)
    {
        if (pSetting->Type == CHARSTRING)
        {
            char szDefaultString[] = {0};
            char szBuffer[256];
            char* szValue;
            nValue = GetPrivateProfileString(pSetting->szIniSection, pSetting->szIniEntry, szDefaultString, szBuffer, 255, szIniFile);
            if (nValue <= 0)
            {
                IsSettingInIniFile = FALSE;
                szValue = (char *)(pSetting->Default);            
            }
            else
            {
                IsSettingInIniFile = TRUE;
                szValue = (char *)szBuffer;
            }
            if (IsSettingInIniFile || !bDontSetDefault)
            {
                if (*pSetting->pValue != NULL)
                {
                    delete [] (char *)(*pSetting->pValue);
                }
                char* str = new char[strlen(szValue) + 1];
                strcpy(str, szValue);
                *pSetting->pValue = (long)str;
            }
            LOG(2, " Setting_ReadFromIni %s %s Value %s", pSetting->szIniSection, pSetting->szIniEntry, *pSetting->pValue);
        }
        else
        {
            nValue = GetPrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, pSetting->MinValue - 100, szIniFile);
            LOG(2, " Setting_ReadFromIni %s %s Value %d", pSetting->szIniSection, pSetting->szIniEntry, nValue);
            nSavedValue = nValue;
            if(nValue == pSetting->MinValue - 100)
            {
                nValue = pSetting->Default;            
                IsSettingInIniFile = FALSE;
            }
            // If the value is out of range, set it to its default value
            if ( (nValue < pSetting->MinValue)
              || (nValue > pSetting->MaxValue) )
            {
                if(nValue < pSetting->MinValue)
                {
                    LOG(1, "%s %s Was out of range - %d is too low", pSetting->szIniSection, pSetting->szIniEntry, nValue);
                }
                else
                {
                    LOG(1, "%s %s Was out of range - %d is too high", pSetting->szIniSection, pSetting->szIniEntry, nValue);
                }
                nValue = pSetting->Default;
            }
            if (IsSettingInIniFile || !bDontSetDefault)
            {
                *pSetting->pValue = nValue;
            }
            if(IsSettingInIniFile)
            {
                pSetting->LastSavedValue = nSavedValue;
            }
            else
            {
                pSetting->LastSavedValue = pSetting->MinValue - 100;
            }
        }
    }
    else
    {
        IsSettingInIniFile =  FALSE;
    }
    return IsSettingInIniFile;
}

void Setting_WriteToIni(SETTING* pSetting, BOOL bOptimizeFileAccess)
{
    if(pSetting->szIniSection != NULL)
    {
        if( !bOptimizeFileAccess || (pSetting->Type == CHARSTRING) || (pSetting->LastSavedValue != *pSetting->pValue) )
        {
            if (pSetting->Type == CHARSTRING)
            {
                WritePrivateProfileString(pSetting->szIniSection, pSetting->szIniEntry, (char *)(*pSetting->pValue), szIniFile);
                LOG(2, " Setting_WriteToIni %s %s Value %s", pSetting->szIniSection, pSetting->szIniEntry, (char*)(*pSetting->pValue));
            }
            else
            {
                WritePrivateProfileInt(pSetting->szIniSection, pSetting->szIniEntry, *pSetting->pValue, szIniFile);
                pSetting->LastSavedValue = *pSetting->pValue;
                LOG(2, " Setting_WriteToIni %s %s Value %d", pSetting->szIniSection, pSetting->szIniEntry, *pSetting->pValue);
            }
        }
    }
}

void Setting_OSDShow(SETTING* pSetting, HWND hWnd)
{
    char szBuffer[1024] = "Unexpected Display Error";

    if(pSetting->szDisplayName != NULL)
    {
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
                sprintf(szBuffer, "%s %.*f", pSetting->szDisplayName, (int)log10((double)pSetting->OSDDivider), (float)*(pSetting->pValue) / (float)pSetting->OSDDivider);
            }
            break;
        case CHARSTRING:
            sprintf(szBuffer, "%s %s", pSetting->szDisplayName, *pSetting->pValue ? (char*)(*pSetting->pValue) : "");
            break;
        default:
            break;
        }
        OSD_ShowText(szBuffer, 0);
    }
    else
    {
        OSD_ShowText("Not Supported", 0);
    }
}
//---------------------------------------------------------------------------
// This function allows for accelerated slider adjustments
// For example, adjusting Contrast or Brightness faster the longer 
// you hold down the adjustment key.
int GetCurrentAdjustmentStepCount(void* pSetting)
{
    static DWORD        dwLastTick = 0;
    static DWORD        dwFirstTick = 0;
    static DWORD        dwTaps = 0;
    DWORD               dwTick;
    DWORD               dwElapsedSinceLastCall;
    DWORD               dwElapsedSinceFirstTick;
    int                 nStepCount;
    static void*     LastSetting = NULL;

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

    if (pSetting->Type == CHARSTRING)
        return;

    if (*pSetting->pValue < pSetting->MaxValue)
    {
        nStep = GetCurrentAdjustmentStepCount(pSetting) * pSetting->StepValue;
        Setting_SetValue(pSetting, *pSetting->pValue + nStep);
    }
}

void Setting_Down(SETTING* pSetting)
{
    int nStep = 0;

    if (pSetting->Type == CHARSTRING)
        return;

    if (*pSetting->pValue > pSetting->MinValue)
    {
        nStep = GetCurrentAdjustmentStepCount(pSetting) * pSetting->StepValue;
        Setting_SetValue(pSetting, *pSetting->pValue - nStep);
    }
}

void Setting_ChangeDefault(SETTING* pSetting, long Default, BOOL bDontTouchValue)
{
    if (pSetting->Type == CHARSTRING)
        return;

    pSetting->Default = Default;
    if (!bDontTouchValue)
    {
        *pSetting->pValue = Default;
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
        Setting_OSDShow(pSetting, GetMainWnd());
        break;
    case ADJUSTUP:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_Up(pSetting);
            Setting_OSDShow(pSetting, GetMainWnd());
        }
        break;
    case ADJUSTDOWN:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_Down(pSetting);
            Setting_OSDShow(pSetting, GetMainWnd());
        }
        break;
    case INCREMENT:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_SetValue(pSetting, Setting_GetValue(pSetting) + pSetting->StepValue);
            Setting_OSDShow(pSetting, GetMainWnd());
        }
        break;
    case DECREMENT:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_SetValue(pSetting, Setting_GetValue(pSetting) - pSetting->StepValue);
            Setting_OSDShow(pSetting, GetMainWnd());
        }
        break;
    case RESET:
        Setting_SetDefault(pSetting);
        Setting_OSDShow(pSetting, GetMainWnd());
        break;
    case TOGGLEBOOL:
        if(pSetting->Type == YESNO || pSetting->Type == ONOFF)
        {
            Setting_SetValue(pSetting, !Setting_GetValue(pSetting));
            Setting_OSDShow(pSetting, GetMainWnd());
        }
        break;
    case ADJUSTUP_SILENT:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_Up(pSetting);
        }
        break;
    case ADJUSTDOWN_SILENT:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_Down(pSetting);
        }
        break;
    case INCREMENT_SILENT:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_SetValue(pSetting, Setting_GetValue(pSetting) + pSetting->StepValue);
        }
        break;
    case DECREMENT_SILENT:
        if (pSetting->Type != CHARSTRING)
        {
            Setting_SetValue(pSetting, Setting_GetValue(pSetting) - pSetting->StepValue);
        }
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

void Setting_Free(SETTING* pSetting)
{
    if ( (pSetting != NULL)
      && (pSetting->Type == CHARSTRING)
      && (*pSetting->pValue != NULL) )
    {
        delete [] (char *)(*pSetting->pValue);
        *pSetting->pValue = NULL;
    }
}

