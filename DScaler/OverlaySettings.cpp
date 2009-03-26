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
#include "OverlayOutput.h"
#include "OverlaySettings.h"
#include "Slider.h"
#include "SettingsMaster.h"


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
            TUseOverlayControls = Setting_GetValue(WM_OTHER_GETVALUE, USEOVERLAYCONTROLS);
            TBrightness = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS);
            TContrast = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYCONTRAST);
            THue = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYHUE);
            TSaturation = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSATURATION);
            TGamma = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYGAMMA);
            TSharpness = Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSHARPNESS);

            CheckDlgButton(hDlg, IDC_OVERLAYSETTINGS_ENABLE, TUseOverlayControls ? BST_CHECKED : BST_UNCHECKED);
            SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
            SetDlgItemInt(hDlg, IDC_D2, TContrast, FALSE);
            SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
            SetDlgItemInt(hDlg, IDC_D4, TSaturation, FALSE);
            SetDlgItemInt(hDlg, IDC_D5, TGamma, FALSE);
            SetDlgItemInt(hDlg, IDC_D6, TSharpness, FALSE);

            Setting_SetupSlider(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS, GetDlgItem(hDlg, IDC_SLIDER1));
            Setting_SetupSlider(WM_OTHER_GETVALUE, OVERLAYCONTRAST, GetDlgItem(hDlg, IDC_SLIDER2));
            Setting_SetupSlider(WM_OTHER_GETVALUE, OVERLAYHUE, GetDlgItem(hDlg, IDC_SLIDER3));
            Setting_SetupSlider(WM_OTHER_GETVALUE, OVERLAYSATURATION, GetDlgItem(hDlg, IDC_SLIDER4));
            Setting_SetupSlider(WM_OTHER_GETVALUE, OVERLAYGAMMA, GetDlgItem(hDlg, IDC_SLIDER5));
            Setting_SetupSlider(WM_OTHER_GETVALUE, OVERLAYSHARPNESS, GetDlgItem(hDlg, IDC_SLIDER6));

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
            SettingsMaster->SaveAllSettings(TRUE);
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            Setting_SetValue(WM_OTHER_GETVALUE, USEOVERLAYCONTROLS, TUseOverlayControls);
            Setting_SetValue(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS, TBrightness);
            Setting_SetValue(WM_OTHER_GETVALUE, OVERLAYCONTRAST, TContrast);
            Setting_SetValue(WM_OTHER_GETVALUE, OVERLAYHUE,THue);
            Setting_SetValue(WM_OTHER_GETVALUE, OVERLAYSATURATION, TSaturation);
            Setting_SetValue(WM_OTHER_GETVALUE, OVERLAYGAMMA, TGamma);
            Setting_SetValue(WM_OTHER_GETVALUE, OVERLAYSHARPNESS, TSharpness);
            EndDialog(hDlg, TRUE);
            break;

        case IDC_DEFAULT:
            Setting_SetDefault(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS);
            Setting_SetDefault(WM_OTHER_GETVALUE, OVERLAYCONTRAST);
            Setting_SetDefault(WM_OTHER_GETVALUE, OVERLAYHUE);
            Setting_SetDefault(WM_OTHER_GETVALUE, OVERLAYSATURATION);
            Setting_SetDefault(WM_OTHER_GETVALUE, OVERLAYGAMMA);
            Setting_SetDefault(WM_OTHER_GETVALUE, OVERLAYSHARPNESS);
            
            Setting_SetControlValue(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS, GetDlgItem(hDlg, IDC_SLIDER1));
            Setting_SetControlValue(WM_OTHER_GETVALUE, OVERLAYCONTRAST, GetDlgItem(hDlg, IDC_SLIDER2));
            Setting_SetControlValue(WM_OTHER_GETVALUE, OVERLAYHUE, GetDlgItem(hDlg, IDC_SLIDER3));
            Setting_SetControlValue(WM_OTHER_GETVALUE, OVERLAYSATURATION, GetDlgItem(hDlg, IDC_SLIDER4));
            Setting_SetControlValue(WM_OTHER_GETVALUE, OVERLAYGAMMA, GetDlgItem(hDlg, IDC_SLIDER5));
            Setting_SetControlValue(WM_OTHER_GETVALUE, OVERLAYSHARPNESS, GetDlgItem(hDlg, IDC_SLIDER6));

            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS), TRUE);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYCONTRAST), FALSE);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYHUE), TRUE);
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSATURATION), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYGAMMA), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSHARPNESS), FALSE);
            break;

        case IDC_OVERLAYSETTINGS_ENABLE:
            {
                Setting_SetValue(WM_OTHER_GETVALUE, USEOVERLAYCONTROLS, Button_GetCheck(GetDlgItem(hDlg, IDC_OVERLAYSETTINGS_ENABLE)) == BST_CHECKED);
                
                BOOL bEnable = Setting_GetValue(WM_OTHER_GETVALUE, USEOVERLAYCONTROLS);
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
            Setting_SetFromControl(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS, (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYBRIGHTNESS), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            Setting_SetFromControl(WM_OTHER_GETVALUE, OVERLAYCONTRAST, (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYCONTRAST), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            Setting_SetFromControl(WM_OTHER_GETVALUE, OVERLAYHUE, (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYHUE), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            Setting_SetFromControl(WM_OTHER_GETVALUE, OVERLAYSATURATION, (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D4, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSATURATION), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            Setting_SetFromControl(WM_OTHER_GETVALUE, OVERLAYGAMMA, (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYGAMMA), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            Setting_SetFromControl(WM_OTHER_GETVALUE, OVERLAYSHARPNESS, (HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D6, Setting_GetValue(WM_OTHER_GETVALUE, OVERLAYSHARPNESS), FALSE);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}
