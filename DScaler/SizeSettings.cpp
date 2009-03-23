/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier.  All rights reserved.
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
 * @file SizeSettings.cpp Size Settings Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SizeSettings.h"
#include "Providers.h"
#include "Setting.h"
#include "Slider.h"

BOOL APIENTRY SizeSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static BOOL TAnalogueBlanking;
    static long TTopOverscan;
    static long TBottomOverscan;
    static long TLeftOverscan;
    static long TRightOverscan;
    static long THDelay;
    static long TVDelay;
    static CYesNoSetting* AnalogueBlanking = NULL;
    static CSliderSetting* TopOverscan = NULL;
    static CSliderSetting* BottomOverscan = NULL;
    static CSliderSetting* LeftOverscan = NULL;
    static CSliderSetting* RightOverscan = NULL;
    static CSliderSetting* HDelay = NULL;
    static CSliderSetting* VDelay = NULL;

    switch (message)
    {
    case WM_INITDIALOG:
        AnalogueBlanking = Providers_GetCurrentSource()->GetAnalogueBlanking();
        TopOverscan = Providers_GetCurrentSource()->GetTopOverscan();
        BottomOverscan = Providers_GetCurrentSource()->GetBottomOverscan();
        LeftOverscan = Providers_GetCurrentSource()->GetLeftOverscan();
        RightOverscan = Providers_GetCurrentSource()->GetRightOverscan();
        HDelay = Providers_GetCurrentSource()->GetHDelay();
        VDelay = Providers_GetCurrentSource()->GetVDelay();

        if(AnalogueBlanking != NULL)
        {
            TAnalogueBlanking = AnalogueBlanking->GetValue();
            AnalogueBlanking->SetControlValue(GetDlgItem(hDlg, IDC_CHECK1));
        }
        else
        {
            CheckDlgButton(hDlg, IDC_CHECK1, BST_UNCHECKED);
            Edit_Enable(GetDlgItem(hDlg, IDC_CHECK1), FALSE);
        }

        if(TopOverscan != NULL)
        {
            TTopOverscan = TopOverscan->GetValue();
            SetDlgItemInt(hDlg, IDC_D1, TTopOverscan, TRUE);
            TopOverscan->SetupControl(GetDlgItem(hDlg, IDC_SLIDER1));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D1), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER1), FALSE);
        }

        if(BottomOverscan != NULL)
        {
            TBottomOverscan = BottomOverscan->GetValue();
            SetDlgItemInt(hDlg, IDC_D2, TBottomOverscan, TRUE);
            BottomOverscan->SetupControl(GetDlgItem(hDlg, IDC_SLIDER2));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D2), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER2), FALSE);
        }

        if(LeftOverscan != NULL)
        {
            TLeftOverscan = LeftOverscan->GetValue();
            SetDlgItemInt(hDlg, IDC_D3, TLeftOverscan, TRUE);
            LeftOverscan->SetupControl(GetDlgItem(hDlg, IDC_SLIDER3));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D3), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER3), FALSE);
        }

        if(RightOverscan != NULL)
        {
            TRightOverscan = RightOverscan->GetValue();
            SetDlgItemInt(hDlg, IDC_D4, TRightOverscan, TRUE);
            RightOverscan->SetupControl(GetDlgItem(hDlg, IDC_SLIDER4));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D4), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER4), FALSE);
        }

        if(HDelay != NULL)
        {
            THDelay = HDelay->GetValue();
            SetDlgItemInt(hDlg, IDC_D5, THDelay, TRUE);
            HDelay->SetupControl(GetDlgItem(hDlg, IDC_SLIDER5));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D5), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER5), FALSE);
        }

        if(VDelay != NULL)
        {
            TVDelay = VDelay->GetValue();
            SetDlgItemInt(hDlg, IDC_D6, TVDelay, TRUE);
            VDelay->SetupControl(GetDlgItem(hDlg, IDC_SLIDER6));
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
            if(AnalogueBlanking != NULL)
            {
                AnalogueBlanking->SetValue(TAnalogueBlanking);
            }
            if(TopOverscan != NULL)
            {
                TopOverscan->SetValue(TTopOverscan);
            }
            if(BottomOverscan != NULL)
            {
                BottomOverscan->SetValue(TBottomOverscan);
            }
            if(LeftOverscan != NULL)
            {
                LeftOverscan->SetValue(TLeftOverscan);
            }
            if(RightOverscan != NULL)
            {
                RightOverscan->SetValue(TRightOverscan);
            }
            if(HDelay != NULL)
            {
                HDelay->SetValue(THDelay);
            }
            if(VDelay != NULL)
            {
                VDelay->SetValue(TVDelay);
            }
            EndDialog(hDlg, TRUE);
            break;

        case IDC_DEFAULT:
            if(AnalogueBlanking != NULL)
            {
                AnalogueBlanking->ChangeValue(RESET_SILENT);
                AnalogueBlanking->SetControlValue(GetDlgItem(hDlg, IDC_CHECK1));
            }
            if(TopOverscan != NULL)
            {
                TopOverscan->ChangeValue(RESET_SILENT);
                TopOverscan->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER1));
                SetDlgItemInt(hDlg, IDC_D1, TopOverscan->GetValue(), FALSE);
            }
            if(BottomOverscan != NULL)
            {
                BottomOverscan->ChangeValue(RESET_SILENT);
                BottomOverscan->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER2));
                SetDlgItemInt(hDlg, IDC_D2, BottomOverscan->GetValue(), FALSE);
            }
            if(LeftOverscan != NULL)
            {
                LeftOverscan->ChangeValue(RESET_SILENT);
                LeftOverscan->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER3));
                SetDlgItemInt(hDlg, IDC_D3, LeftOverscan->GetValue(), FALSE);
            }
            if(RightOverscan != NULL)
            {
                RightOverscan->ChangeValue(RESET_SILENT);
                RightOverscan->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER4));
                SetDlgItemInt(hDlg, IDC_D4, RightOverscan->GetValue(), FALSE);
            }
            if(HDelay != NULL)
            {
                HDelay->ChangeValue(RESET_SILENT);
                HDelay->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));
                SetDlgItemInt(hDlg, IDC_D5, HDelay->GetValue(), TRUE);
            }
            if(VDelay != NULL)
            {
                VDelay->ChangeValue(RESET_SILENT);
                VDelay->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER6));
                SetDlgItemInt(hDlg, IDC_D6, VDelay->GetValue(), TRUE);
            }
            break;

        case IDC_CHECK1:
            if(AnalogueBlanking != NULL)
            {
                AnalogueBlanking->SetFromControl(GetDlgItem(hDlg, IDC_CHECK1));
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
            TopOverscan->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D1, TopOverscan->GetValue(), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER2))
        {
            BottomOverscan->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D2, BottomOverscan->GetValue(), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER3))
        {
            LeftOverscan->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D3, LeftOverscan->GetValue(), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER4))
        {
            RightOverscan->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D4, RightOverscan->GetValue(), FALSE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER5))
        {
            HDelay->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D5, HDelay->GetValue(), TRUE);
        }
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER6))
        {
            VDelay->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D6, VDelay->GetValue(), TRUE);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}


