/////////////////////////////////////////////////////////////////////////////
// $Id: Dialogs.cpp,v 1.16 2002-03-04 20:49:36 adcockj Exp $
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
// 08 Jan 2001   John Adcock           Added Version Info to about box
//
// 09 Jan 2001   Tom Barry             Added Chip Type to hardware dialog
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.15  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.14  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.13  2001/11/02 16:33:07  adcockj
// Removed conflict tags
//
// Revision 1.12  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.11  2001/09/02 12:13:21  adcockj
// Changed dscaler webiste
// Tidied up resource spelling
//
// Revision 1.10.2.3  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.10.2.2  2001/08/17 16:35:14  adcockj
// Another interim check-in still doesn't compile. Getting closer ...
//
// Revision 1.10.2.1  2001/08/14 16:41:36  adcockj
// Renamed driver
// Got to compile with new class based card
//
// Revision 1.10  2001/07/29 10:16:51  adcockj
// Added build number to about box
//
// Revision 1.9  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.8  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Dialogs.h"
#include "DScaler.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"
#include "Audio.h"
#include "VBI.h"
#include "CPU.h"
#include "Slider.h"
#include "AspectRatio.h"
#include "Crash.h"

BOOL APIENTRY AboutProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    DWORD   dwVerInfoSize;      // Size of version information block
    LPSTR   lpVersion;          // String pointer to 'version' text
    DWORD   dwVerHnd=0;         // An 'ignored' parameter, always '0'
    UINT    uVersionLen;        // Current length of full version string
    WORD    wRootLen;           // length of the 'root' portion of string
    char    szFullPath[MAX_PATH];   // full path of module
    char    szResult[256];      // Temporary result string
    char    szGetName[256];     // String to use for extracting version Info

    switch (message)
    {
    case WM_INITDIALOG:
            // Now lets dive in and pull out the version information:
            GetModuleFileName (hDScalerInst, szFullPath, sizeof(szFullPath));
            dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
            if (dwVerInfoSize)
            {
                LPSTR lpstrVffInfo;
                HGLOBAL hMem;
                hMem = (LPSTR)GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
                lpstrVffInfo = (LPSTR)GlobalLock(hMem);
                GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
                // The below 'hex' Value looks a little confusing, but
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
                    (UINT*)&uVersionLen))
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
                        (UINT*)&uVersionLen))
                    {
                        lstrcat(szResult, lpVersion);
                        lstrcat(szResult, " Compiled ");
                        lstrcat(szResult, __DATE__);
                        lstrcat(szResult, " ");
                        lstrcat(szResult, __TIME__);

                        lstrcat(szResult, " Build (");
                        sprintf(szGetName,"%d", gBuildNum);
                        lstrcat(szResult, szGetName);
                        lstrcat(szResult, ")");

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
            ShellExecute(hDlg, "open", "http://sourceforge.net/forum/?group_id=7420", NULL, NULL, SW_SHOWNORMAL);
            break;
        case IDC_LINK2:
            ShellExecute(hDlg, "open", "http://www.dscaler.org/", NULL, NULL, SW_SHOWNORMAL);
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
        SetDlgItemInt(hDlg, IDC_VBI_FPS, VBIFPS, FALSE);
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
    GetModuleFileName (hDScalerInst, szFullPath, sizeof(szFullPath));
    dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
    if (dwVerInfoSize)
    {
        LPSTR lpstrVffInfo;
        HGLOBAL hMem;
        hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
        lpstrVffInfo = (LPSTR)GlobalLock(hMem);
        GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
        // The below 'hex' Value looks a little confusing, but
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
            (UINT*)&uVersionLen))
        {
            lstrcpy(szResult, lpVersion);

            szGetName[wRootLen] = (char)0;
            lstrcat (szGetName, "ProductVersion");

            if(VerQueryValue((LPVOID)lpstrVffInfo,
                (LPSTR)szGetName,
                (void**)&lpVersion,
                (UINT*)&uVersionLen))
            {
                lstrcat(szResult, " Version ");
                lstrcat(szResult, lpVersion);
            }
        }
    }
    return szResult;
}
