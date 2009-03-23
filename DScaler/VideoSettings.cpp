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
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file VideoSettings.cpp VideoSettings dialog
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "VideoSettings.h"
#include "Providers.h"
#include "Setting.h"
#include "Slider.h"

BOOL APIENTRY VideoSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static long THue;
    static long TBrightness;
    static long TContrast;
    static long TSaturation;
    static long TSaturationU;
    static long TSaturationV;
    static CSliderSetting* Hue = NULL;
    static CSliderSetting* Brightness = NULL;
    static CSliderSetting* Contrast = NULL;
    static CSliderSetting* SaturationU = NULL;
    static CSliderSetting* SaturationV = NULL;
    static CSliderSetting* Saturation = NULL;

    switch (message)
    {
    case WM_INITDIALOG:
        Hue = Providers_GetCurrentSource()->GetHue();    
        Brightness = Providers_GetCurrentSource()->GetBrightness();    
        Contrast = Providers_GetCurrentSource()->GetContrast();    
        Saturation = Providers_GetCurrentSource()->GetSaturation();    
        SaturationU = Providers_GetCurrentSource()->GetSaturationU();    
        SaturationV = Providers_GetCurrentSource()->GetSaturationV();    

        if(Brightness != NULL)
        {
            TBrightness = Brightness->GetValue();
            SetDlgItemInt(hDlg, IDC_D1, TBrightness, TRUE);
            Brightness->SetupControl(GetDlgItem(hDlg, IDC_SLIDER1));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D1), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER1), FALSE);
        }

        if(Contrast != NULL)
        {
            TContrast = Contrast->GetValue();
            SetDlgItemInt(hDlg, IDC_D2, TContrast, TRUE);
            Contrast->SetupControl(GetDlgItem(hDlg, IDC_SLIDER2));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D2), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER2), FALSE);
        }

        if(Hue != NULL)
        {
            THue = Hue->GetValue();
            SetDlgItemInt(hDlg, IDC_D3, THue, TRUE);
            Hue->SetupControl(GetDlgItem(hDlg, IDC_SLIDER3));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D3), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER3), FALSE);
        }

        if(Saturation != NULL)
        {
            TSaturation = Saturation->GetValue();
            SetDlgItemInt(hDlg, IDC_D4, TSaturation, TRUE);
            Saturation->SetupControl(GetDlgItem(hDlg, IDC_SLIDER4));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D4), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER4), FALSE);
        }

        if(SaturationU != NULL)
        {
            TSaturationU = SaturationU->GetValue();
            SetDlgItemInt(hDlg, IDC_D5, TSaturationU, TRUE);
            SaturationU->SetupControl(GetDlgItem(hDlg, IDC_SLIDER5));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D5), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER5), FALSE);
        }

        if(SaturationV != NULL)
        {
            TSaturationV = SaturationV->GetValue();
            SetDlgItemInt(hDlg, IDC_D6, TSaturationV, TRUE);
            SaturationV->SetupControl(GetDlgItem(hDlg, IDC_SLIDER6));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D6), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER6), FALSE);
        }
        return TRUE;
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            if(Brightness != NULL)
            {
                Brightness->SetValue(TBrightness);
            }
            if(Contrast != NULL)
            {
                Contrast->SetValue(TContrast);
            }
            if(Hue != NULL)
            {
                Hue->SetValue(THue);
            }
            if(Saturation != NULL)
            {
                Saturation->SetValue(TSaturation);
            }
            if(SaturationU != NULL)
            {
                SaturationU->SetValue(TSaturationU);
            }
            if(SaturationV != NULL)
            {
                SaturationV->SetValue(TSaturationV);
            }
            EndDialog(hDlg, TRUE);
            break;

        case IDC_DEFAULT:
            if(Brightness != NULL)
            {
                Brightness->ChangeValue(RESET_SILENT);
                Brightness->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER1));
                SetDlgItemInt(hDlg, IDC_D1, Brightness->GetValue(), TRUE);
            }
            if(Contrast != NULL)
            {
                Contrast->ChangeValue(RESET_SILENT);
                Contrast->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER2));
                SetDlgItemInt(hDlg, IDC_D2, Contrast->GetValue(), FALSE);
            }
            if(Hue != NULL)
            {
                Hue->ChangeValue(RESET_SILENT);
                Hue->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER3));
                SetDlgItemInt(hDlg, IDC_D3, Hue->GetValue(), TRUE);
            }
            if(Saturation != NULL)
            {
                Saturation->ChangeValue(RESET_SILENT);
                Saturation->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER4));
                SetDlgItemInt(hDlg, IDC_D4, Saturation->GetValue(), FALSE);
            }
            if(SaturationU != NULL)
            {
                SaturationU->ChangeValue(RESET_SILENT);
                SaturationU->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));
                SetDlgItemInt(hDlg, IDC_D5, SaturationU->GetValue(), FALSE);
            }
            if(SaturationV != NULL)
            {
                SaturationV->ChangeValue(RESET_SILENT);
                SaturationV->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER6));
                SetDlgItemInt(hDlg, IDC_D6, SaturationV->GetValue(), FALSE);
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
            Brightness->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, Brightness->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            Contrast->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, Contrast->GetValue(), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            Hue->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, Hue->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            Saturation->SetFromControl((HWND)lParam);
            if(SaturationU!=NULL)
            {
                SaturationU->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));
            }
            if(SaturationV!=NULL)
            {
                SaturationV->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER6));
            }
            SetDlgItemInt(hDlg, IDC_D4, Saturation->GetValue(), FALSE);
            if(SaturationU!=NULL)
            {
                SetDlgItemInt(hDlg, IDC_D5, SaturationU->GetValue(), FALSE);
            }
            if(SaturationV!=NULL)
            {
                SetDlgItemInt(hDlg, IDC_D6, SaturationV->GetValue(), FALSE);
            }
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            SaturationU->SetFromControl((HWND)lParam);
            Saturation->SetupControl(GetDlgItem(hDlg, IDC_SLIDER4));
            SetDlgItemInt(hDlg, IDC_D4, Saturation->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, SaturationU->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, SaturationV->GetValue(), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            SaturationV->SetFromControl((HWND)lParam);
            Saturation->SetupControl(GetDlgItem(hDlg, IDC_SLIDER4));
            SetDlgItemInt(hDlg, IDC_D4, Saturation->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, SaturationU->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, SaturationV->GetValue(), FALSE);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}


