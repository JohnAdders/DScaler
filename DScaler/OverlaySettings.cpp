/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
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

#include "stdafx.h"
#include "resource.h"
#include "Other.h"
#include "OverlaySettings.h"


BOOL APIENTRY OverlaySettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static long TUse;
    static long THue;
    static long TBrightness;
    static long TContrast;
    static long TSaturation;
    static long TGamma;
    static long TSharpness;

    switch (message)
    {
    case WM_INITDIALOG:

        TUse = Setting_GetValue(Other_GetSetting(USEOVERLAYCONTROLS));
        TBrightness = Setting_GetValue(Other_GetSetting(OVERLAYBRIGHTNESS));
        TContrast = Setting_GetValue(Other_GetSetting(OVERLAYCONTRAST));
        THue = Setting_GetValue(Other_GetSetting(OVERLAYHUE));
        TSaturation = Setting_GetValue(Other_GetSetting(OVERLAYSATURATION));
        TGamma = Setting_GetValue(Other_GetSetting(OVERLAYGAMMA));
        TSharpness = Setting_GetValue(Other_GetSetting(OVERLAYSHARPNESS));

        Button_SetCheck(GetDlgItem(hDlg, IDC_CHECK1), Setting_GetValue(Other_GetSetting(USEOVERLAYCONTROLS))?MF_CHECKED:MF_UNCHECKED);

        SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
        SetDlgItemInt(hDlg, IDC_D2, TContrast, FALSE);
        SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
        SetDlgItemInt(hDlg, IDC_D4, TSaturation, FALSE);
        SetDlgItemInt(hDlg, IDC_D5, TGamma, FALSE);
        SetDlgItemInt(hDlg, IDC_D6, TSharpness, FALSE);

        Setting_SetupSlider(Other_GetSetting(OVERLAYBRIGHTNESS), GetDlgItem(hDlg, IDC_SLIDER1));
        Setting_SetupSlider(Other_GetSetting(OVERLAYCONTRAST), GetDlgItem(hDlg, IDC_SLIDER2));
        Setting_SetupSlider(Other_GetSetting(OVERLAYHUE), GetDlgItem(hDlg, IDC_SLIDER3));
        Setting_SetupSlider(Other_GetSetting(OVERLAYSATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
        Setting_SetupSlider(Other_GetSetting(OVERLAYGAMMA), GetDlgItem(hDlg, IDC_SLIDER5));
        Setting_SetupSlider(Other_GetSetting(OVERLAYSHARPNESS), GetDlgItem(hDlg, IDC_SLIDER6));
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            OverlaySettings_Save();
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            Setting_SetValue(Other_GetSetting(USEOVERLAYCONTROLS), TUse);
            Setting_SetValue(Other_GetSetting(OVERLAYBRIGHTNESS), TBrightness);
            Setting_SetValue(Other_GetSetting(OVERLAYCONTRAST), TContrast);
            Setting_SetValue(Other_GetSetting(OVERLAYHUE),THue);
            Setting_SetValue(Other_GetSetting(OVERLAYSATURATION), TSaturation);
            Setting_SetValue(Other_GetSetting(OVERLAYGAMMA), TGamma);
            Setting_SetValue(Other_GetSetting(OVERLAYSHARPNESS), TSharpness);
            EndDialog(hDlg, TRUE);
            break;

        case IDDEFAULT:
            Setting_SetDefault(Other_GetSetting(OVERLAYBRIGHTNESS));
            Setting_SetDefault(Other_GetSetting(OVERLAYCONTRAST));
            Setting_SetDefault(Other_GetSetting(OVERLAYHUE));
            Setting_SetDefault(Other_GetSetting(OVERLAYSATURATION));
            Setting_SetDefault(Other_GetSetting(OVERLAYGAMMA));
            Setting_SetDefault(Other_GetSetting(OVERLAYSHARPNESS));
            
            Setting_SetControlValue(Other_GetSetting(OVERLAYBRIGHTNESS), GetDlgItem(hDlg, IDC_SLIDER1));
            Setting_SetControlValue(Other_GetSetting(OVERLAYCONTRAST), GetDlgItem(hDlg, IDC_SLIDER2));
            Setting_SetControlValue(Other_GetSetting(OVERLAYHUE), GetDlgItem(hDlg, IDC_SLIDER3));
            Setting_SetControlValue(Other_GetSetting(OVERLAYSATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
            Setting_SetControlValue(Other_GetSetting(OVERLAYGAMMA), GetDlgItem(hDlg, IDC_SLIDER5));
            Setting_SetControlValue(Other_GetSetting(OVERLAYSHARPNESS), GetDlgItem(hDlg, IDC_SLIDER6));

            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(Other_GetSetting(OVERLAYBRIGHTNESS)), TRUE);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(Other_GetSetting(OVERLAYCONTRAST)), FALSE);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(Other_GetSetting(OVERLAYHUE)), TRUE);
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(Other_GetSetting(OVERLAYSATURATION)), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(Other_GetSetting(OVERLAYGAMMA)), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(Other_GetSetting(OVERLAYSHARPNESS)), FALSE);
            break;

        case IDC_CHECK1:
            Setting_SetValue(Other_GetSetting(USEOVERLAYCONTROLS), Button_GetCheck(GetDlgItem(hDlg, IDC_CHECK1)) == BST_CHECKED);
            break;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER1))
        {
            Setting_SetFromControl(Other_GetSetting(OVERLAYBRIGHTNESS), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(Other_GetSetting(OVERLAYBRIGHTNESS)), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            Setting_SetFromControl(Other_GetSetting(OVERLAYCONTRAST), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(Other_GetSetting(OVERLAYCONTRAST)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            Setting_SetFromControl(Other_GetSetting(OVERLAYHUE), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(Other_GetSetting(OVERLAYHUE)), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            Setting_SetFromControl(Other_GetSetting(OVERLAYSATURATION), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(Other_GetSetting(OVERLAYSATURATION)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            Setting_SetFromControl(Other_GetSetting(OVERLAYGAMMA), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(Other_GetSetting(OVERLAYGAMMA)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            Setting_SetFromControl(Other_GetSetting(OVERLAYSHARPNESS), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(Other_GetSetting(OVERLAYSHARPNESS)), FALSE);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}


void OverlaySettings_Load()
{
    Setting_ReadFromIni(Other_GetSetting(USEOVERLAYCONTROLS));
    Setting_ReadFromIni(Other_GetSetting(OVERLAYBRIGHTNESS));
    Setting_ReadFromIni(Other_GetSetting(OVERLAYCONTRAST));
    Setting_ReadFromIni(Other_GetSetting(OVERLAYHUE));
    Setting_ReadFromIni(Other_GetSetting(OVERLAYSATURATION));
    Setting_ReadFromIni(Other_GetSetting(OVERLAYGAMMA));
    Setting_ReadFromIni(Other_GetSetting(OVERLAYSHARPNESS));
}


void OverlaySettings_Save()
{
    Setting_WriteToIni(Other_GetSetting(USEOVERLAYCONTROLS), FALSE);
    Setting_WriteToIni(Other_GetSetting(OVERLAYBRIGHTNESS), FALSE);
    Setting_WriteToIni(Other_GetSetting(OVERLAYCONTRAST), FALSE);
    Setting_WriteToIni(Other_GetSetting(OVERLAYHUE), FALSE);
    Setting_WriteToIni(Other_GetSetting(OVERLAYSATURATION), FALSE);
    Setting_WriteToIni(Other_GetSetting(OVERLAYGAMMA), FALSE);
    Setting_WriteToIni(Other_GetSetting(OVERLAYSHARPNESS), FALSE);
}
