/////////////////////////////////////////////////////////////////////////////
// $Id: VideoSettings.cpp,v 1.6 2001-07-16 18:07:50 adcockj Exp $
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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 29 Jan 2001   John Adcock           Original Release
//
// 31 Mar 2001   Laurent Garnier       Last used format saved per video input
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "bt848.h"
#include "AspectRatio.h"
#include "Audio.h"
#include "VideoSettings.h"

BOOL bSavePerInput = FALSE;
BOOL bSavePerFormat = FALSE;
BOOL bSaveTVFormatPerInput = FALSE;

char szSection[50];
char szSection2[50];

BOOL APIENTRY VideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static long THue;
    static long TBrightness;
    static long TContrast;
    static long TSaturationU;
    static long TSaturationV;
    static long TOverscan;
    static long LastSaturation;

    switch (message)
    {
    case WM_INITDIALOG:

        TBrightness = Setting_GetValue(BT848_GetSetting(BRIGHTNESS));
        TContrast = Setting_GetValue(BT848_GetSetting(CONTRAST));
        THue = Setting_GetValue(BT848_GetSetting(HUE));
        TSaturationU = Setting_GetValue(BT848_GetSetting(SATURATIONU));
        TSaturationV = Setting_GetValue(BT848_GetSetting(SATURATIONV));
        TOverscan = Setting_GetValue(Aspect_GetSetting(OVERSCAN));

        SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
        SetDlgItemInt(hDlg, IDC_D2, TContrast, FALSE);
        SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
        LastSaturation = (TSaturationU + TSaturationV) / 2;
        SetDlgItemInt(hDlg, IDC_D4, LastSaturation, FALSE);
        SetDlgItemInt(hDlg, IDC_D5, TSaturationU, FALSE);
        SetDlgItemInt(hDlg, IDC_D6, TSaturationV, FALSE);
        SetDlgItemInt(hDlg, IDC_D7, TOverscan, FALSE);

        Setting_SetupSlider(BT848_GetSetting(BRIGHTNESS), GetDlgItem(hDlg, IDC_SLIDER1));
        Setting_SetupSlider(BT848_GetSetting(CONTRAST), GetDlgItem(hDlg, IDC_SLIDER2));
        Setting_SetupSlider(BT848_GetSetting(HUE), GetDlgItem(hDlg, IDC_SLIDER3));
        Setting_SetupSlider(BT848_GetSetting(SATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
        Setting_SetupSlider(BT848_GetSetting(SATURATIONU), GetDlgItem(hDlg, IDC_SLIDER5));
        Setting_SetupSlider(BT848_GetSetting(SATURATIONV), GetDlgItem(hDlg, IDC_SLIDER6));
        Setting_SetupSlider(Aspect_GetSetting(OVERSCAN), GetDlgItem(hDlg, IDC_SLIDER7));

        Button_SetCheck(GetDlgItem(hDlg, IDC_BYFORMAT), bSavePerFormat?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hDlg, IDC_BYINPUT), bSavePerInput?BST_CHECKED:BST_UNCHECKED);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            VideoSettings_Save();
            bSavePerFormat = Button_GetCheck(GetDlgItem(hDlg, IDC_BYFORMAT)) == BST_CHECKED;
            bSavePerInput = Button_GetCheck(GetDlgItem(hDlg, IDC_BYINPUT)) == BST_CHECKED;
            VideoSettings_SetupDefaults();
            VideoSettings_Save();
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            Setting_SetValue(BT848_GetSetting(BRIGHTNESS), TBrightness);
            Setting_SetValue(BT848_GetSetting(CONTRAST), TContrast);
            Setting_SetValue(BT848_GetSetting(HUE),THue);
            Setting_SetValue(BT848_GetSetting(SATURATIONU), TSaturationU);
            Setting_SetValue(BT848_GetSetting(SATURATIONV), TSaturationV);
            Setting_SetValue(Aspect_GetSetting(OVERSCAN), TOverscan);
            EndDialog(hDlg, TRUE);
            break;

        case IDDEFAULT:
            Setting_SetDefault(BT848_GetSetting(BRIGHTNESS));
            Setting_SetDefault(BT848_GetSetting(CONTRAST));
            Setting_SetDefault(BT848_GetSetting(HUE));
            Setting_SetDefault(BT848_GetSetting(SATURATIONU));
            Setting_SetDefault(BT848_GetSetting(SATURATIONV));
            Setting_SetDefault(Aspect_GetSetting(OVERSCAN));
            
            Setting_SetupSlider(BT848_GetSetting(SATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
            
            Setting_SetControlValue(BT848_GetSetting(BRIGHTNESS), GetDlgItem(hDlg, IDC_SLIDER1));
            Setting_SetControlValue(BT848_GetSetting(CONTRAST), GetDlgItem(hDlg, IDC_SLIDER2));
            Setting_SetControlValue(BT848_GetSetting(HUE), GetDlgItem(hDlg, IDC_SLIDER3));
            Setting_SetControlValue(BT848_GetSetting(SATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
            Setting_SetControlValue(BT848_GetSetting(SATURATIONU), GetDlgItem(hDlg, IDC_SLIDER5));
            Setting_SetControlValue(BT848_GetSetting(SATURATIONV), GetDlgItem(hDlg, IDC_SLIDER6));
            Setting_SetControlValue(Aspect_GetSetting(OVERSCAN), GetDlgItem(hDlg, IDC_SLIDER7));

            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(BT848_GetSetting(BRIGHTNESS)), TRUE);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(BT848_GetSetting(CONTRAST)), FALSE);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(BT848_GetSetting(HUE)), TRUE);
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(BT848_GetSetting(SATURATION)), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(BT848_GetSetting(SATURATIONU)), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(BT848_GetSetting(SATURATIONV)), FALSE);
            SetDlgItemInt(hDlg, IDC_D7, Setting_GetValue(Aspect_GetSetting(OVERSCAN)), FALSE);
            break;
        case IDC_ALLFORMATS:
            VideoSettings_SaveCurrentToAllFormats();
            break;
        case IDC_ALLINPUTS:
            VideoSettings_SaveCurrentToAllInputs();
            break;

        default:
            break;
        }
        break;
    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER1))
        {
            Setting_SetFromControl(BT848_GetSetting(BRIGHTNESS), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(BT848_GetSetting(BRIGHTNESS)), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            Setting_SetFromControl(BT848_GetSetting(CONTRAST), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(BT848_GetSetting(CONTRAST)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            Setting_SetFromControl(BT848_GetSetting(HUE), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(BT848_GetSetting(HUE)), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            Setting_SetFromControl(BT848_GetSetting(SATURATION), (HWND)lParam);
            Setting_SetControlValue(BT848_GetSetting(SATURATIONU), GetDlgItem(hDlg, IDC_SLIDER5));
            Setting_SetControlValue(BT848_GetSetting(SATURATIONV), GetDlgItem(hDlg, IDC_SLIDER6));
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(BT848_GetSetting(SATURATION)), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(BT848_GetSetting(SATURATIONU)), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(BT848_GetSetting(SATURATIONV)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            Setting_SetFromControl(BT848_GetSetting(SATURATIONU), (HWND)lParam);
            Setting_SetupSlider(BT848_GetSetting(SATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(BT848_GetSetting(SATURATION)), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(BT848_GetSetting(SATURATIONU)), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(BT848_GetSetting(SATURATIONV)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            Setting_SetFromControl(BT848_GetSetting(SATURATIONV), (HWND)lParam);
            Setting_SetupSlider(BT848_GetSetting(SATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(BT848_GetSetting(SATURATION)), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(BT848_GetSetting(SATURATIONU)), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(BT848_GetSetting(SATURATIONV)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER7))
        {
            Setting_SetFromControl(Aspect_GetSetting(OVERSCAN), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D7, Setting_GetValue(Aspect_GetSetting(OVERSCAN)), FALSE);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}

void VideoSettings_SaveCurrentToAllInputs()
{
}

void VideoSettings_SaveCurrentToAllFormats()
{
}

void VideoSettings_SetupDefaults()
{
    switch(Setting_GetValue(BT848_GetSetting(TVFORMAT)))
    {
    case FORMAT_NTSC:
    case FORMAT_NTSC_J:
    case FORMAT_NTSC50:
        BT848_GetSetting(BRIGHTNESS)->Default = DEFAULT_BRIGHTNESS_NTSC;
        BT848_GetSetting(CONTRAST)->Default = DEFAULT_CONTRAST_NTSC;
        BT848_GetSetting(HUE)->Default = DEFAULT_HUE_NTSC;
        BT848_GetSetting(SATURATION)->Default = (DEFAULT_SAT_U_NTSC + DEFAULT_SAT_V_NTSC) / 2;
        BT848_GetSetting(SATURATIONU)->Default = DEFAULT_SAT_U_NTSC;
        BT848_GetSetting(SATURATIONV)->Default = DEFAULT_SAT_V_NTSC;
        Aspect_GetSetting(OVERSCAN)->Default = DEFAULT_OVERSCAN_NTSC;
        break;
    case FORMAT_PAL_BDGHI:
    case FORMAT_PAL_N:
    case FORMAT_PAL60:
    case FORMAT_SECAM:
    case FORMAT_PAL_M:
        BT848_GetSetting(BRIGHTNESS)->Default = DEFAULT_BRIGHTNESS_PAL;
        BT848_GetSetting(CONTRAST)->Default = DEFAULT_CONTRAST_PAL;
        BT848_GetSetting(HUE)->Default = DEFAULT_HUE_PAL;
        BT848_GetSetting(SATURATION)->Default = (DEFAULT_SAT_U_PAL + DEFAULT_SAT_V_PAL) / 2;
        BT848_GetSetting(SATURATIONU)->Default = DEFAULT_SAT_U_PAL;
        BT848_GetSetting(SATURATIONV)->Default = DEFAULT_SAT_V_PAL;
        Aspect_GetSetting(OVERSCAN)->Default = DEFAULT_OVERSCAN_PAL;
        break;
    }

    // set up defaults fro position parameters
    if(bSavePerFormat)
    {
        BT848_GetSetting(BDELAY)->Default = BT848_GetTVFormat()->bDelayB;
        *BT848_GetSetting(BDELAY)->pValue = BT848_GetTVFormat()->bDelayB;
    }
    else
    {
        BT848_GetSetting(BDELAY)->Default = 0;
    }

    BT848_GetSetting(BTFULLLUMARANGE)->Default = 0;
    BT848_GetSetting(CURRENTX)->Default = 720;
    
    switch(Setting_GetValue(BT848_GetSetting(VIDEOSOURCE)))
    {
    case SOURCE_CCIR656_1:
    case SOURCE_CCIR656_2:
    case SOURCE_CCIR656_3:
    case SOURCE_CCIR656_4:
        BT848_GetSetting(BRIGHTNESS)->Default = 0;
        BT848_GetSetting(CONTRAST)->Default = 0x80;
        BT848_GetSetting(SATURATION)->Default = 0x80;
        BT848_GetSetting(SATURATIONU)->Default = 0x80;
        BT848_GetSetting(SATURATIONV)->Default = 0x80;
        BT848_GetSetting(BTFULLLUMARANGE)->Default = 1;
        break;
    default:
        break;
    }
}


void SetupSectionNames()
{
    int Input = -1;
    int Format = -1;
    
    if(bSavePerInput)
    {
        Input = Setting_GetValue(BT848_GetSetting(VIDEOSOURCE));
    }

    if(bSavePerFormat)
    {
        Format = Setting_GetValue(BT848_GetSetting(TVFORMAT));
    }

    if(Input == -1 && Format == -1)
    {
        Setting_SetSection(BT848_GetSetting(BRIGHTNESS), "Hardware");
        Setting_SetSection(BT848_GetSetting(CONTRAST), "Hardware");
        Setting_SetSection(BT848_GetSetting(HUE), "Hardware");
        Setting_SetSection(BT848_GetSetting(SATURATIONU), "Hardware");
        Setting_SetSection(BT848_GetSetting(SATURATIONV), "Hardware");
        Setting_SetSection(BT848_GetSetting(BDELAY), "Hardware");
        Setting_SetSection(BT848_GetSetting(CURRENTX), "MainWindow");
        Setting_SetSection(BT848_GetSetting(BTFULLLUMARANGE), "Hardware");
        Setting_SetSection(Aspect_GetSetting(OVERSCAN), "Hardware");
    }
    else
    {
        sprintf(szSection, "VideoSettings_%d_%d", Input, Format);
        Setting_SetSection(BT848_GetSetting(BRIGHTNESS), szSection);
        Setting_SetSection(BT848_GetSetting(CONTRAST), szSection);
        Setting_SetSection(BT848_GetSetting(HUE), szSection);
        Setting_SetSection(BT848_GetSetting(SATURATIONU), szSection);
        Setting_SetSection(BT848_GetSetting(SATURATIONV), szSection);
        Setting_SetSection(BT848_GetSetting(BDELAY), szSection);
        Setting_SetSection(BT848_GetSetting(CURRENTX), szSection);
        Setting_SetSection(BT848_GetSetting(BTFULLLUMARANGE), szSection);
        Setting_SetSection(Aspect_GetSetting(OVERSCAN), szSection);
    }
}

void VideoSettings_Load()
{
    SetupSectionNames();
    VideoSettings_SetupDefaults();
    Setting_ReadFromIni(BT848_GetSetting(BRIGHTNESS));
    Setting_ReadFromIni(BT848_GetSetting(CONTRAST));
    Setting_ReadFromIni(BT848_GetSetting(HUE));
    Setting_ReadFromIni(BT848_GetSetting(SATURATIONU));
    Setting_ReadFromIni(BT848_GetSetting(SATURATIONV));
    Setting_ReadFromIni(BT848_GetSetting(BDELAY));
    Setting_ReadFromIni(BT848_GetSetting(CURRENTX));
    Setting_ReadFromIni(BT848_GetSetting(BTFULLLUMARANGE));
    Setting_ReadFromIni(Aspect_GetSetting(OVERSCAN));
    *BT848_GetSetting(SATURATION)->pValue = 
        (Setting_GetValue(BT848_GetSetting(SATURATIONU)) + 
        Setting_GetValue(BT848_GetSetting(SATURATIONV))) / 2;
}

void VideoSettings_Save()
{
    SetupSectionNames();
    Setting_WriteToIni(BT848_GetSetting(BRIGHTNESS), FALSE);
    Setting_WriteToIni(BT848_GetSetting(CONTRAST), FALSE);
    Setting_WriteToIni(BT848_GetSetting(HUE), FALSE);
    Setting_WriteToIni(BT848_GetSetting(SATURATIONU), FALSE);
    Setting_WriteToIni(BT848_GetSetting(SATURATIONV), FALSE);
    Setting_WriteToIni(BT848_GetSetting(BDELAY), FALSE);
    Setting_WriteToIni(BT848_GetSetting(CURRENTX), FALSE);
    Setting_WriteToIni(BT848_GetSetting(BTFULLLUMARANGE), FALSE);
    Setting_WriteToIni(Aspect_GetSetting(OVERSCAN), FALSE);
}

void SetupSectionTVFormat()
{
    int Input = -1;
    
    if(bSaveTVFormatPerInput)
    {
        Input = Setting_GetValue(BT848_GetSetting(VIDEOSOURCE));
    }

    if(Input == -1)
    {
        Setting_SetSection(BT848_GetSetting(TVFORMAT), "Hardware");
    }
    else
    {
        sprintf(szSection2, "TVFormatSettings_%d", Input);
        Setting_SetSection(BT848_GetSetting(TVFORMAT), szSection2);
    }
}

void VideoSettings_LoadTVFormat()
{
    SetupSectionTVFormat();
    Setting_ReadFromIni(BT848_GetSetting(TVFORMAT));
}

void VideoSettings_SaveTVFormat()
{
    SetupSectionTVFormat();
    Setting_WriteToIni(BT848_GetSetting(TVFORMAT), FALSE);
}


BOOL SavePerInput_OnChange(long NewValue)
{
    bSavePerInput = NewValue;
    SetupSectionNames();
    return FALSE;
}

BOOL SavePerFormat_OnChange(long NewValue)
{
    bSavePerFormat = NewValue;
    SetupSectionNames();
    return FALSE;
}

BOOL SaveTVFormatPerInput_OnChange(long NewValue)
{
    bSaveTVFormatPerInput = NewValue;
    SetupSectionTVFormat();
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING VideoSettingsSettings[VIDEOSETTINGS_SETTING_LASTONE] =
{
    {
        "Save Settings Per Input", YESNO, 0, (long*)&bSavePerInput,
         FALSE, 0, 1, 1, 1,
         NULL,
        "VideoSettings", "SavePerInput", SavePerInput_OnChange,
    },
    {
        "Save Settings Per Format", YESNO, 0, (long*)&bSavePerFormat,
         FALSE, 0, 1, 1, 1,
         NULL,
        "VideoSettings", "SavePerFormat", SavePerFormat_OnChange,
    },
    {
        "Save TV Format per input", YESNO, 0, (long*)&bSaveTVFormatPerInput,
         FALSE, 0, 1, 1, 1,
         NULL,
        "VideoSettings", "SaveTVFormatPerInput", SaveTVFormatPerInput_OnChange,
    },
};


SETTING* VideoSettings_GetSetting(VIDEOSETTINGS_SETTING Setting)
{
    if(Setting > -1 && Setting < VIDEOSETTINGS_SETTING_LASTONE)
    {
        return &(VideoSettingsSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void VideoSettings_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < VIDEOSETTINGS_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(VideoSettingsSettings[i]));
    }
    VideoSettings_LoadTVFormat();
    VideoSettings_Load();
}

void VideoSettings_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < VIDEOSETTINGS_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(VideoSettingsSettings[i]), bOptimizeFileAccess);
    }
    VideoSettings_SaveTVFormat();
    VideoSettings_Save();
}

