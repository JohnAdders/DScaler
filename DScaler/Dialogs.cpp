/////////////////////////////////////////////////////////////////////////////
// Dialogs.cpp
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
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 08 Jan 2001   John Adcock           Added Version info to about box
//
// 09 Jan 2001   Tom Barry             Added Chip type to hardware dialog
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "Dialogs.h"
#include "BT848.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"
#include "Audio.h"
#include "Tuner.h"
#include "VBI.h"
#include "CPU.h"
#include "Slider.h"
#include "AspectRatio.h"

BOOL APIENTRY AboutProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    DWORD   dwVerInfoSize;      // Size of version information block
    LPSTR   lpVersion;          // String pointer to 'version' text
    DWORD   dwVerHnd=0;         // An 'ignored' parameter, always '0'
    UINT    uVersionLen;        // Current length of full version string
    WORD    wRootLen;           // length of the 'root' portion of string
    char    szFullPath[MAX_PATH];   // full path of module
    char    szResult[256];      // Temporary result string
    char    szGetName[256];     // String to use for extracting version info

    switch (message)
    {
    case WM_INITDIALOG:
            // Now lets dive in and pull out the version information:
            GetModuleFileName (hInst, szFullPath, sizeof(szFullPath));
            dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
            if (dwVerInfoSize)
            {
                LPSTR lpstrVffInfo;
                HGLOBAL hMem;
                hMem = (LPSTR)GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
                lpstrVffInfo = (LPSTR)GlobalLock(hMem);
                GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
                // The below 'hex' value looks a little confusing, but
                // essentially what it is, is the hexidecimal representation
                // of a couple different values that represent the language
                // and character set that we are wanting string values for.
                // 040904E4 is a very common one, because it means:
                //   US English, Windows MultiLingual characterset
                // Or to pull it all apart:
                // 04------        = SUBLANG_ENGLISH_USA
                // --09----        = LANG_ENGLISH
                // ----04BO        = Codepage
                lstrcpy(szGetName, "\\StringFileInfo\\040904B0\\");	 
                wRootLen = lstrlen(szGetName); // Save this position
                
                // Set the title of the dialog:
                lstrcat (szGetName, "ProductName");
                if(VerQueryValue((LPVOID)lpstrVffInfo,
                    (LPSTR)szGetName,
                    (void**)&lpVersion,
                    (UINT *)&uVersionLen))
                {
                    lstrcpy(szResult, "About ");
                    lstrcat(szResult, lpVersion);
                    SetWindowText (hDlg, szResult);

                    lstrcpy(szResult, lpVersion);
                    lstrcat(szResult, " Version ");

                    szGetName[wRootLen] = (char)0;
                    lstrcat (szGetName, "ProductVersion");

                    if(VerQueryValue((LPVOID)lpstrVffInfo,
                        (LPSTR)szGetName,
                        (void**)&lpVersion,
                        (UINT *)&uVersionLen))
                    {
                        lstrcat(szResult, lpVersion);
                        lstrcat(szResult, " Compiled ");
                        lstrcat(szResult, __DATE__);
                        lstrcat(szResult, " ");
                        lstrcat(szResult, __TIME__);

                        SetWindowText (GetDlgItem(hDlg, IDC_VERSION), szResult);
                    }
                }
            } // if (dwVerInfoSize)
            
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_LINK:
            ShellExecute(hDlg, "open", "http://www.avsforum.com/", NULL, NULL, SW_SHOWNORMAL);
            break;
        default:
            break;
        }
        break;
    }

    return (FALSE);
}

BOOL APIENTRY VPSInfoProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
        ShowVPSInfo = hDlg;
        SetTimer(hDlg, 100, 1000, NULL);
        break;
    case WM_TIMER:
        SetDlgItemInt(hDlg, IDT_VBI_FPS, VBI_FPS, FALSE);
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            ShowVPSInfo = NULL;
            KillTimer(hDlg, 100);
            EndDialog(hDlg, TRUE);
        }
        break;
    }

    return (FALSE);
}

BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_TEXT1, BT848_ChipType());
        SetDlgItemText(hDlg, IDC_TEXT6, BT848_VendorID());
        SetDlgItemText(hDlg, IDC_TEXT7, BT848_DeviceID());
        SetDlgItemText(hDlg, IDC_TEXT13, Tuner_Status());
        SetDlgItemText(hDlg, IDC_TEXT14, Audio_MSP_Status());
        SetDlgItemText(hDlg, IDC_TEXT16, Audio_MSP_VersionString());
        SetDlgItemText(hDlg, IDC_AUTODECTECTID, TVCard_AutoDetectID());

        SetDlgItemText(hDlg, IDC_TEXT18, "YUV2");
        
        // TB 20010109 added Chip type
        if (CpuFeatureFlags & FEATURE_SSE2)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE2");
        }
        else if (CpuFeatureFlags & FEATURE_SSE)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "SSE");
        }
        else if (CpuFeatureFlags & FEATURE_MMXEXT)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMXEXT");
        }
        else if (CpuFeatureFlags & FEATURE_3DNOWEXT)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOWEXT");
        }
        else if (CpuFeatureFlags & FEATURE_3DNOW)
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "3DNOW");
        }
        else
        {
            SetDlgItemText(hDlg, IDC_CPU_TYPE, "MMX");
        }


        break;

    case WM_COMMAND:

        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            EndDialog(hDlg, TRUE);
        }

        break;
    }

    return (FALSE);
}


LPCSTR GetProductNameAndVersion()
{
    DWORD   dwVerInfoSize;      // Size of version information block
    LPSTR   lpVersion;          // String pointer to 'version' text
    DWORD   dwVerHnd=0;         // An 'ignored' parameter, always '0'
    UINT    uVersionLen;        // Current length of full version string
    WORD    wRootLen;           // length of the 'root' portion of string
    char    szFullPath[MAX_PATH];   // full path of module
    static char szResult[256] = DSCALER_APPNAME;    // Temporary result string
    char szGetName[256];

    // Now lets dive in and pull out the version information:
    GetModuleFileName (hInst, szFullPath, sizeof(szFullPath));
    dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
    if (dwVerInfoSize)
    {
        LPSTR lpstrVffInfo;
        HGLOBAL hMem;
        hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
        lpstrVffInfo = (LPSTR)GlobalLock(hMem);
        GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
        // The below 'hex' value looks a little confusing, but
        // essentially what it is, is the hexidecimal representation
        // of a couple different values that represent the language
        // and character set that we are wanting string values for.
        // 040904E4 is a very common one, because it means:
        //   US English, Windows MultiLingual characterset
        // Or to pull it all apart:
        // 04------        = SUBLANG_ENGLISH_USA
        // --09----        = LANG_ENGLISH
        // ----04BO        = Codepage
        lstrcpy(szGetName, "\\StringFileInfo\\040904B0\\");	 
        wRootLen = lstrlen(szGetName); // Save this position

        // Set the title of the dialog:
        lstrcat (szGetName, "ProductName");
        if(VerQueryValue((LPVOID)lpstrVffInfo,
            (LPSTR)szGetName,
            (void**)&lpVersion,
            (UINT *)&uVersionLen))
        {
            lstrcpy(szResult, lpVersion);

            szGetName[wRootLen] = (char)0;
            lstrcat (szGetName, "ProductVersion");

            if(VerQueryValue((LPVOID)lpstrVffInfo,
                (LPSTR)szGetName,
                (void**)&lpVersion,
                (UINT *)&uVersionLen))
            {
                lstrcat(szResult, " Version ");
                lstrcat(szResult, lpVersion);
            }
        }
    }
    return szResult;
}
