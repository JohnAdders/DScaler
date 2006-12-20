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

/**
 * @file OverlaySettings.cpp Overlay Settings functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "IOutput.h"
#include "OverlaySettings.h"
#include "Slider.h"


BOOL APIENTRY OverlaySettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static long THue;
    static long TBrightness;
    static long TContrast;
    static long TSaturation;
    static long TGamma;
    static long TSharpness;
    static BOOL TUseOverlayControls;
    
    switch (message)
    {
    case WM_INITDIALOG:
        {
            TUseOverlayControls = Setting_GetValue(Overlay_GetSetting(USEOVERLAYCONTROLS));
            TBrightness = Setting_GetValue(Overlay_GetSetting(OVERLAYBRIGHTNESS));
            TContrast = Setting_GetValue(Overlay_GetSetting(OVERLAYCONTRAST));
            THue = Setting_GetValue(Overlay_GetSetting(OVERLAYHUE));
            TSaturation = Setting_GetValue(Overlay_GetSetting(OVERLAYSATURATION));
            TGamma = Setting_GetValue(Overlay_GetSetting(OVERLAYGAMMA));
            TSharpness = Setting_GetValue(Overlay_GetSetting(OVERLAYSHARPNESS));

            CheckDlgButton(hDlg, IDC_OVERLAYSETTINGS_ENABLE, TUseOverlayControls ? BST_CHECKED : BST_UNCHECKED);
            SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
            SetDlgItemInt(hDlg, IDC_D2, TContrast, FALSE);
            SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
            SetDlgItemInt(hDlg, IDC_D4, TSaturation, FALSE);
            SetDlgItemInt(hDlg, IDC_D5, TGamma, FALSE);
            SetDlgItemInt(hDlg, IDC_D6, TSharpness, FALSE);

            Setting_SetupSlider(Overlay_GetSetting(OVERLAYBRIGHTNESS), GetDlgItem(hDlg, IDC_SLIDER1));
            Setting_SetupSlider(Overlay_GetSetting(OVERLAYCONTRAST), GetDlgItem(hDlg, IDC_SLIDER2));
            Setting_SetupSlider(Overlay_GetSetting(OVERLAYHUE), GetDlgItem(hDlg, IDC_SLIDER3));
            Setting_SetupSlider(Overlay_GetSetting(OVERLAYSATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
            Setting_SetupSlider(Overlay_GetSetting(OVERLAYGAMMA), GetDlgItem(hDlg, IDC_SLIDER5));
            Setting_SetupSlider(Overlay_GetSetting(OVERLAYSHARPNESS), GetDlgItem(hDlg, IDC_SLIDER6));

            BOOL bEnable = TUseOverlayControls;
            EnableWindow(GetDlgItem(hDlg, IDC_SLIDER1), bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_SLIDER2), bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_SLIDER3), bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_SLIDER4), bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_SLIDER5), bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_SLIDER6), bEnable);
            EnableWindow(GetDlgItem(hDlg, IDC_DEFAULT), bEnable);
        }
        return TRUE;
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            OverlaySettings_Save();
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            Setting_SetValue(Overlay_GetSetting(USEOVERLAYCONTROLS), TUseOverlayControls);
            Setting_SetValue(Overlay_GetSetting(OVERLAYBRIGHTNESS), TBrightness);
            Setting_SetValue(Overlay_GetSetting(OVERLAYCONTRAST), TContrast);
            Setting_SetValue(Overlay_GetSetting(OVERLAYHUE),THue);
            Setting_SetValue(Overlay_GetSetting(OVERLAYSATURATION), TSaturation);
            Setting_SetValue(Overlay_GetSetting(OVERLAYGAMMA), TGamma);
            Setting_SetValue(Overlay_GetSetting(OVERLAYSHARPNESS), TSharpness);
            EndDialog(hDlg, TRUE);
            break;

        case IDC_DEFAULT:
            Setting_SetDefault(Overlay_GetSetting(OVERLAYBRIGHTNESS));
            Setting_SetDefault(Overlay_GetSetting(OVERLAYCONTRAST));
            Setting_SetDefault(Overlay_GetSetting(OVERLAYHUE));
            Setting_SetDefault(Overlay_GetSetting(OVERLAYSATURATION));
            Setting_SetDefault(Overlay_GetSetting(OVERLAYGAMMA));
            Setting_SetDefault(Overlay_GetSetting(OVERLAYSHARPNESS));
            
            Setting_SetControlValue(Overlay_GetSetting(OVERLAYBRIGHTNESS), GetDlgItem(hDlg, IDC_SLIDER1));
            Setting_SetControlValue(Overlay_GetSetting(OVERLAYCONTRAST), GetDlgItem(hDlg, IDC_SLIDER2));
            Setting_SetControlValue(Overlay_GetSetting(OVERLAYHUE), GetDlgItem(hDlg, IDC_SLIDER3));
            Setting_SetControlValue(Overlay_GetSetting(OVERLAYSATURATION), GetDlgItem(hDlg, IDC_SLIDER4));
            Setting_SetControlValue(Overlay_GetSetting(OVERLAYGAMMA), GetDlgItem(hDlg, IDC_SLIDER5));
            Setting_SetControlValue(Overlay_GetSetting(OVERLAYSHARPNESS), GetDlgItem(hDlg, IDC_SLIDER6));

            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(Overlay_GetSetting(OVERLAYBRIGHTNESS)), TRUE);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(Overlay_GetSetting(OVERLAYCONTRAST)), FALSE);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(Overlay_GetSetting(OVERLAYHUE)), TRUE);
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(Overlay_GetSetting(OVERLAYSATURATION)), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(Overlay_GetSetting(OVERLAYGAMMA)), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(Overlay_GetSetting(OVERLAYSHARPNESS)), FALSE);
            break;

        case IDC_OVERLAYSETTINGS_ENABLE:
            {
                Setting_SetValue(Overlay_GetSetting(USEOVERLAYCONTROLS), Button_GetCheck(GetDlgItem(hDlg, IDC_OVERLAYSETTINGS_ENABLE)) == BST_CHECKED);
                
                BOOL bEnable = Setting_GetValue(Overlay_GetSetting(USEOVERLAYCONTROLS));
                EnableWindow(GetDlgItem(hDlg, IDC_SLIDER1), bEnable);
                EnableWindow(GetDlgItem(hDlg, IDC_SLIDER2), bEnable);
                EnableWindow(GetDlgItem(hDlg, IDC_SLIDER3), bEnable);
                EnableWindow(GetDlgItem(hDlg, IDC_SLIDER4), bEnable);
                EnableWindow(GetDlgItem(hDlg, IDC_SLIDER5), bEnable);
                EnableWindow(GetDlgItem(hDlg, IDC_SLIDER6), bEnable);
                EnableWindow(GetDlgItem(hDlg, IDC_DEFAULT), bEnable);
                }
            break;

        default:
            break;
        }
        break;

    case WM_VSCROLL:
    case WM_HSCROLL:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER1))
        {
            Setting_SetFromControl(Overlay_GetSetting(OVERLAYBRIGHTNESS), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(Overlay_GetSetting(OVERLAYBRIGHTNESS)), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            Setting_SetFromControl(Overlay_GetSetting(OVERLAYCONTRAST), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(Overlay_GetSetting(OVERLAYCONTRAST)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            Setting_SetFromControl(Overlay_GetSetting(OVERLAYHUE), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(Overlay_GetSetting(OVERLAYHUE)), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            Setting_SetFromControl(Overlay_GetSetting(OVERLAYSATURATION), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(Overlay_GetSetting(OVERLAYSATURATION)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            Setting_SetFromControl(Overlay_GetSetting(OVERLAYGAMMA), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(Overlay_GetSetting(OVERLAYGAMMA)), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            Setting_SetFromControl(Overlay_GetSetting(OVERLAYSHARPNESS), (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(Overlay_GetSetting(OVERLAYSHARPNESS)), FALSE);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}


void OverlaySettings_Save()
{
    Setting_WriteToIni(Overlay_GetSetting(USEOVERLAYCONTROLS), FALSE);
    Setting_WriteToIni(Overlay_GetSetting(OVERLAYBRIGHTNESS), FALSE);
    Setting_WriteToIni(Overlay_GetSetting(OVERLAYCONTRAST), FALSE);
    Setting_WriteToIni(Overlay_GetSetting(OVERLAYHUE), FALSE);
    Setting_WriteToIni(Overlay_GetSetting(OVERLAYSATURATION), FALSE);
    Setting_WriteToIni(Overlay_GetSetting(OVERLAYGAMMA), FALSE);
    Setting_WriteToIni(Overlay_GetSetting(OVERLAYSHARPNESS), FALSE);
}
