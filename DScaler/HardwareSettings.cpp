/////////////////////////////////////////////////////////////////////////////
// $Id$
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
#include "SettingsMaster.h"

static void ChangeSettingsBasedOnHW(int ProcessorSpeed, int TradeOff, int Usage, int VideoCard)
{
    // now do defaults based on the processor speed selected
    if(ProcessorSpeed == 0 && TradeOff == 0)
    {
        // User has selected below 300 MHz and low judder
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, WAITFORFLIP, TRUE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, AUTODETECT, FALSE);
        Setting_ChangeDefault(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(WM_FD50_GETVALUE, PALFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 0 && TradeOff == 1)
    {
        // User has selected below 300 MHz and best picture
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, WAITFORFLIP, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, AUTODETECT, FALSE);
        Setting_ChangeDefault(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(WM_FD50_GETVALUE, PALFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 1 && TradeOff == 0)
    {
        // User has selected 300 MHz - 500 MHz and low judder
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, WAITFORFLIP, TRUE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, AUTODETECT, TRUE);
        Setting_ChangeDefault(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(WM_FD50_GETVALUE, PALFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 1 && TradeOff == 1)
    {
        // User has selected 300 MHz - 500 MHz and best picture
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, WAITFORFLIP, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, AUTODETECT, TRUE);
        Setting_ChangeDefault(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
        Setting_ChangeDefault(WM_FD50_GETVALUE, PALFILMFALLBACKMODE, INDEX_VIDEO_GREEDY);
    }
    else if(ProcessorSpeed == 2)
    {
        // User has selected 500 MHz - 1 GHz
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, WAITFORFLIP, TRUE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, AUTODETECT, TRUE);
        Setting_ChangeDefault(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE, INDEX_VIDEO_GREEDYH);
        Setting_ChangeDefault(WM_FD50_GETVALUE, PALFILMFALLBACKMODE, INDEX_VIDEO_GREEDYH);
    }
    else
    {
        // user has fast processor use best defaults
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, WAITFORFLIP, TRUE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS, FALSE);
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, AUTODETECT, TRUE);
        Setting_ChangeDefault(WM_FD60_GETVALUE, NTSCFILMFALLBACKMODE, INDEX_VIDEO_MOCOMP2);
        Setting_ChangeDefault(WM_FD50_GETVALUE, PALFILMFALLBACKMODE, INDEX_VIDEO_MOCOMP2);
    }

    if (Usage == 1)
    {
        Setting_ChangeDefault(WM_DSCALER_GETVALUE, THREADPRIORITY, 0);
        Setting_ChangeDefault(WM_DSCALER_GETVALUE, WINDOWPRIORITY, 0);
        Setting_ChangeDefault(WM_TIMING_GETVALUE, SLEEPINTERVAL, 0);
        Setting_ChangeDefault(WM_TIMING_GETVALUE, ALWAYSSLEEP, 0);
    }
    else if (Usage == 2)
    {
        Setting_ChangeDefault(WM_OUTTHREADS_GETVALUE, DOACCURATEFLIPS, FALSE);
        Setting_ChangeDefault(WM_DSCALER_GETVALUE, THREADPRIORITY, 1);
        Setting_ChangeDefault(WM_DSCALER_GETVALUE, WINDOWPRIORITY, 0);
        Setting_ChangeDefault(WM_TIMING_GETVALUE, SLEEPINTERVAL, 1);
        Setting_ChangeDefault(WM_TIMING_GETVALUE, ALWAYSSLEEP, 1);
    }
    else if (Usage == 0)
    {
        Setting_ChangeDefault(WM_DSCALER_GETVALUE, THREADPRIORITY, 1);
        Setting_ChangeDefault(WM_DSCALER_GETVALUE, WINDOWPRIORITY, 0);
        Setting_ChangeDefault(WM_TIMING_GETVALUE, SLEEPINTERVAL, 1);
        Setting_ChangeDefault(WM_TIMING_GETVALUE, ALWAYSSLEEP, 0);
    }

    Providers_ChangeSettingsBasedOnHW(Setting_GetValue(WM_DSCALER_GETVALUE, PROCESSORSPEED), Setting_GetValue(WM_DSCALER_GETVALUE, TRADEOFF));
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
        SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_SETCURSEL, Setting_GetValue(WM_DSCALER_GETVALUE, PROCESSORSPEED), 0);
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Show all frames - Lowest judder");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Best picture quality");
        SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_SETCURSEL, Setting_GetValue(WM_DSCALER_GETVALUE, TRADEOFF), 0);
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_ADDSTRING, 0, (LONG)"DScaler & other softwares - priority given to DScaler");
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_ADDSTRING, 0, (LONG)"DScaler alone - best possible results");
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_ADDSTRING, 0, (LONG)"DScaler & other softwares - priority given to other softwares");
        SendMessage(GetDlgItem(hDlg, IDC_FULLCPU), CB_SETCURSEL, Setting_GetValue(WM_DSCALER_GETVALUE, FULLCPU), 0);
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
            Setting_SetValue(WM_DSCALER_GETVALUE, PROCESSORSPEED, ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED)));
            Setting_SetValue(WM_DSCALER_GETVALUE, TRADEOFF, ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_TRADEOFF)));
            Setting_SetValue(WM_DSCALER_GETVALUE, FULLCPU, ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_FULLCPU)));
            ChangeSettingsBasedOnHW(Setting_GetValue(WM_DSCALER_GETVALUE, PROCESSORSPEED), Setting_GetValue(WM_DSCALER_GETVALUE, TRADEOFF), Setting_GetValue(WM_DSCALER_GETVALUE, FULLCPU), Setting_GetValue(WM_DSCALER_GETVALUE, VIDEOCARD));
            SettingsMaster->SaveAllSettings(TRUE);
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
