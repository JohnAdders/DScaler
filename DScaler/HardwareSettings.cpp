/////////////////////////////////////////////////////////////////////////////
// $Id: HardwareSettings.cpp,v 1.11 2003-10-27 10:39:51 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Laurent Garnier.  All rights reserved.
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
// Revision 1.10  2003/03/15 13:15:51  laurentg
// DScaler usage choice
//
// Revision 1.9  2003/03/08 20:50:59  laurentg
// Updated combobox to select the usage of DScaler
//
// Revision 1.8  2003/01/27 22:04:10  laurentg
// First step to merge setup hardware and hardware info dialog boxes
// CPU flag information moved in the general hardware dialog box
// Hardware info dialog box available for CX2388x
//
// Revision 1.7  2003/01/18 10:24:45  laurentg
// Suppression of the video card field from the general hardware setup dialog box
//
// Revision 1.6  2003/01/15 15:54:22  adcockj
// Fixed some keyboard focus issues
//
// Revision 1.5  2002/08/12 19:54:27  laurentg
// Selection of video card to adjust DScaler settings
//
// Revision 1.4  2002/08/11 22:59:52  laurentg
// Call to WriteSettingsToIni with bOptimizeFileAccess set to TRUE
//
// Revision 1.3  2002/08/11 16:14:36  laurentg
// New setting to choose between keep CPU for other applications or use full CPU for best results
//
// Revision 1.2  2002/08/11 13:52:03  laurentg
// Show automatically the general hardware setup dialog box the first time DScaler is started
//
// Revision 1.1  2002/08/11 12:14:02  laurentg
// Cut BT Card setup and general hardware setup in two different windows
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file HardwareSettings.cpp Hardware settings functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"
#include "DebugLog.h"
#include "Providers.h"
#include "FieldTiming.h"
#include "Cpu.h"

static void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff, int Usage, int VideoCard)
{
    // now do defaults based on the processor speed selected
    if(ProcessorSpeed == 0 && TradeOff == 0)
    {
        // User has selected below 300 MHz and low judder
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 0 && TradeOff == 1)
    {
        // User has selected below 300 MHz and best picture
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 1 && TradeOff == 0)
    {
        // User has selected 300 MHz - 500 MHz and low judder
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 1 && TradeOff == 1)
    {
        // User has selected 300 MHz - 500 MHz and best picture
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 2)
    {
        // User has selected 500 MHz - 1 GHz
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_GREEDYH);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_GREEDYH);
    }
    else
    {
        // user has fast processor use best defaults
        Setting_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), TRUE);
        Setting_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
        Setting_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), INDEX_VIDEO_TOMSMOCOMP);
        Setting_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), INDEX_VIDEO_TOMSMOCOMP);
    }

    if (Usage == 1)
    {
        Setting_ChangeDefault(DScaler_GetSetting(THREADPRIORITY), 0);
        Setting_ChangeDefault(DScaler_GetSetting(WINDOWPRIORITY), 0);
        Setting_ChangeDefault(Timing_GetSetting(SLEEPINTERVAL), 0);
        Setting_ChangeDefault(Timing_GetSetting(ALWAYSSLEEP), 0);
    }
    else if (Usage == 2)
    {
        Setting_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
        Setting_ChangeDefault(DScaler_GetSetting(THREADPRIORITY), 1);
        Setting_ChangeDefault(DScaler_GetSetting(WINDOWPRIORITY), 0);
        Setting_ChangeDefault(Timing_GetSetting(SLEEPINTERVAL), 1);
        Setting_ChangeDefault(Timing_GetSetting(ALWAYSSLEEP), 1);
    }
    else if (Usage == 0)
    {
        Setting_ChangeDefault(DScaler_GetSetting(THREADPRIORITY), 1);
        Setting_ChangeDefault(DScaler_GetSetting(WINDOWPRIORITY), 0);
        Setting_ChangeDefault(Timing_GetSetting(SLEEPINTERVAL), 1);
        Setting_ChangeDefault(Timing_GetSetting(ALWAYSSLEEP), 0);
    }

    Providers_ChangeSettingsBasedOnHW(Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), Setting_GetValue(DScaler_GetSetting(TRADEOFF)));
}

BOOL APIENTRY HardwareSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    long EnableCancelButton;

    switch (message)
    {
    case WM_INITDIALOG:
        EnableCancelButton = lParam;
        Button_Enable(GetDlgItem(hDlg, IDCANCEL), EnableCancelButton);
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Below 300 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"300 MHz - 500 MHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"500 MHz - 1 GHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Above 1 GHz");
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_SETCURSEL, Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), 0);
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Show all frames - Lowest judder");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Best picture quality");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_SETCURSEL, Setting_GetValue(DScaler_GetSetting(TRADEOFF)), 0);
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_ADDSTRING, 0, (LONG)"DScaler & other softwares - priority given to DScaler");
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_ADDSTRING, 0, (LONG)"DScaler alone - best possible results");
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_ADDSTRING, 0, (LONG)"DScaler & other softwares - priority given to other softwares");
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_SETCURSEL, Setting_GetValue(DScaler_GetSetting(FULLCPU)), 0);
        LPCSTR pCPUTypeString;
        if (CpuFeatureFlags & FEATURE_SSE2)
        {
            pCPUTypeString = "SSE2";
        }
        else if (CpuFeatureFlags & FEATURE_SSE)
        {
            pCPUTypeString = "SSE";
        }
        else if (CpuFeatureFlags & FEATURE_MMXEXT)
        {
            pCPUTypeString = "MMXEXT";
        }
        else if (CpuFeatureFlags & FEATURE_3DNOWEXT)
        {
            pCPUTypeString = "3DNOWEXT";
        }
        else if (CpuFeatureFlags & FEATURE_3DNOW)
        {
            pCPUTypeString = "3DNOW";
        }
        else
        {
            pCPUTypeString = "MMX";
        }
        SetDlgItemText(hDlg, IDC_CPU_TYPE, pCPUTypeString);
        return TRUE;
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            Setting_SetValue(DScaler_GetSetting(PROCESSORSPEED), ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED)));
            Setting_SetValue(DScaler_GetSetting(TRADEOFF), ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_TRADEOFF)));
            Setting_SetValue(DScaler_GetSetting(FULLCPU), ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FULLCPU)));
            ChangeSettingsBasedOnHW(Setting_GetValue(DScaler_GetSetting(PROCESSORSPEED)), Setting_GetValue(DScaler_GetSetting(TRADEOFF)), Setting_GetValue(DScaler_GetSetting(FULLCPU)), Setting_GetValue(DScaler_GetSetting(VIDEOCARD)));
            WriteSettingsToIni(TRUE);
            EndDialog(hDlg, TRUE);
            break;
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return (FALSE);
}
