/////////////////////////////////////////////////////////////////////////////
// $Id: VideoSettings.cpp,v 1.14 2002-02-09 02:44:55 laurentg Exp $
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
// Revision 1.13  2001/11/29 14:04:07  adcockj
// Added Javadoc comments
//
// Revision 1.12  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.11  2001/11/14 11:28:03  adcockj
// Bug fixes
//
// Revision 1.10  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.9  2001/11/02 16:30:08  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.8  2001/09/03 13:46:06  adcockj
// Added PAL-NC thanks to Eduardo José Tagle
//
// Revision 1.7  2001/08/21 13:23:08  adcockj
// Increment build
//
// Revision 1.6.2.4  2001/08/21 16:42:16  adcockj
// Per format/input settings and ini file fixes
//
// Revision 1.6.2.3  2001/08/21 09:43:01  adcockj
// Brought branch up to date with latest code fixes
//
// Revision 1.6.2.2  2001/08/18 17:09:30  adcockj
// Got to compile, still lots to do...
//
// Revision 1.6.2.1  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.6  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.5  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "AspectRatio.h"
#include "Audio.h"
#include "VideoSettings.h"
#include "Providers.h"
#include "Setting.h"
#include "Slider.h"

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
    static long TSaturation;
    static long TSaturationU;
    static long TSaturationV;
    static long TOverscan;
    static long LastSaturation;
    static ISetting* Hue = NULL;
    static ISetting* Brightness = NULL;
    static ISetting* Contrast = NULL;
    static ISetting* SaturationU = NULL;
    static ISetting* SaturationV = NULL;
    static ISetting* Saturation = NULL;
    static ISetting* Overscan = NULL;

    switch (message)
    {
    case WM_INITDIALOG:
        Hue = Providers_GetCurrentSource()->GetHue();    
        Brightness = Providers_GetCurrentSource()->GetBrightness();    
        Contrast = Providers_GetCurrentSource()->GetContrast();    
        Saturation = Providers_GetCurrentSource()->GetSaturation();    
        SaturationU = Providers_GetCurrentSource()->GetSaturationU();    
        SaturationV = Providers_GetCurrentSource()->GetSaturationV();    
        Overscan = Providers_GetCurrentSource()->GetOverscan();    

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

        if(Overscan != NULL)
        {
            TOverscan = Overscan->GetValue();
            SetDlgItemInt(hDlg, IDC_D7, TOverscan, TRUE);
            Overscan->SetupControl(GetDlgItem(hDlg, IDC_SLIDER7));
        }
        else
        {
            Edit_Enable(GetDlgItem(hDlg, IDC_D7), FALSE);
            Slider_Enable(GetDlgItem(hDlg, IDC_SLIDER7), FALSE);
        }

        Button_SetCheck(GetDlgItem(hDlg, IDC_BYFORMAT), bSavePerFormat?BST_CHECKED:BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hDlg, IDC_BYINPUT), bSavePerInput?BST_CHECKED:BST_UNCHECKED);

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            bSavePerFormat = Button_GetCheck(GetDlgItem(hDlg, IDC_BYFORMAT)) == BST_CHECKED;
            bSavePerInput = Button_GetCheck(GetDlgItem(hDlg, IDC_BYINPUT)) == BST_CHECKED;
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
            if(Overscan != NULL)
            {
                Overscan->SetValue(TOverscan);
            }
            EndDialog(hDlg, TRUE);
            break;

        case IDC_DEFAULT:
            if(Brightness != NULL)
            {
                Brightness->SetDefault();
                Brightness->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER1));
                SetDlgItemInt(hDlg, IDC_D1, Brightness->GetValue(), TRUE);
            }
            if(Contrast != NULL)
            {
                Contrast->SetDefault();
                Contrast->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER2));
                SetDlgItemInt(hDlg, IDC_D2, Contrast->GetValue(), FALSE);
            }
            if(Hue != NULL)
            {
                Hue->SetDefault();
                Hue->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER3));
                SetDlgItemInt(hDlg, IDC_D3, Hue->GetValue(), TRUE);
            }
            if(Saturation != NULL)
            {
                Saturation->SetDefault();
                Saturation->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER4));
                SetDlgItemInt(hDlg, IDC_D4, Saturation->GetValue(), FALSE);
            }
            if(SaturationU != NULL)
            {
                SaturationU->SetDefault();
                SaturationU->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));
                SetDlgItemInt(hDlg, IDC_D5, SaturationU->GetValue(), FALSE);
            }
            if(SaturationV != NULL)
            {
                SaturationV->SetDefault();
                SaturationV->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER6));
                SetDlgItemInt(hDlg, IDC_D6, SaturationV->GetValue(), FALSE);
            }
            if(Overscan != NULL)
            {
                Overscan->SetDefault();
                Overscan->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER7));
                SetDlgItemInt(hDlg, IDC_D7, Overscan->GetValue(), FALSE);
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
            SaturationU->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER5));
            SaturationV->SetControlValue(GetDlgItem(hDlg, IDC_SLIDER6));
            SetDlgItemInt(hDlg, IDC_D4, Saturation->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D5, SaturationU->GetValue(), FALSE);
            SetDlgItemInt(hDlg, IDC_D6, SaturationV->GetValue(), FALSE);
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
        else if((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER7))
        {
            Overscan->SetFromControl((HWND)lParam);
            SetDlgItemInt(hDlg, IDC_D7, Overscan->GetValue(), FALSE);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}


