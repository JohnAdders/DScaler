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
 * @file Dialogs.cpp About Dialog code
 */

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
#include "BuildNum.h"
#include "..\API\DScalerVersion.h"

INT_PTR CALLBACK AboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {
    case WM_INITDIALOG:
        {
            SetWindowText(GetDlgItem(hDlg, IDC_VERSION), GetProductNameAndVersionFull().c_str());

            SetClassLongPtr(GetDlgItem(hDlg, IDC_LINK), GCLP_HCURSOR, (LONG_PTR) hCursorHand);
            return TRUE;
        }
        break;
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            break;
        case IDC_LINK:
            ShellExecute(hDlg, _T("open"), _T("http://www.dscaler.org/"), NULL, NULL, SW_SHOWNORMAL);
            break;
        default:
            break;
        }
        break;
    }

    return (FALSE);
}


tstring GetProductNameAndVersion()
{
    return MBCSToTString("DScaler Version " VERSTRING);
}

tstring GetProductNameAndVersionFull()
{
    std::string Result("DScaler Version ");

    Result += VERSTRING;
    Result += " Compiled ";
    Result += __DATE__;
    Result += " ";
    Result += __TIME__;

    Result += " Build (";
    Result += GetSVNBuildString();
    Result += ")";
    return MBCSToTString(Result);
}
