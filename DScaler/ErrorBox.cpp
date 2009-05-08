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

/**
 * @file ErrorBox.cpp ErrorBox Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "DebugLog.h"
#include "splash.h"

#ifndef _DEBUG
#include "OSD.h"
#include "DScaler.h"
#endif

using namespace std;

extern HWND hWnd;

void _ErrorBoxOSD(HWND hwndParent, LPCTSTR szFile, int Line, const tstring& Message)
{
}

#ifdef _UNICODE
void _ErrorBox(HWND hwndParent, LPCTSTR szFile, int Line, const std::string& Message)
{
    _ErrorBox(hwndParent, szFile, Line, MBCSToTString(Message));
}
#endif



void _ErrorBox(HWND hwndParent, LPCTSTR szFile, int Line, const tstring& Message)
{
    TCHAR szDispMessage[1024];
    // variable used to prevent recusive error problems
    static BOOL AlreadyInErrorBox = FALSE;

    // put the message into the log for debugging
    LOG(0, _T("ErrorBox File:%s line: %d Message: %s"), szFile, Line, Message.c_str());

#ifndef _DEBUG
    if (hWnd != NULL && AlreadyInErrorBox == FALSE)
    {
        AlreadyInErrorBox = TRUE;
        HDC hDC = GetDC(hWnd);
        if (hDC != NULL)
        {
            // Show OSD text immediately and pause for 2 seconds.  OSD will
            // continue to show after the 2 seconds if there are no other
            // OSDs pending.
            _sntprintf(szDispMessage, sizeof(szDispMessage), _T("ERROR: %s\nFile %s Line %d"), Message.c_str(), szFile, Line);
            OSD_ShowTextPersistent(szDispMessage, 4);

            RECT OSDDisplayRect;
            GetDisplayAreaRect(hWnd, &OSDDisplayRect);

            // This code is special case.  Normally, OSD_ProcessDisplayUpdate()
            // is called only from one location in DScaler.cpp so that all
            // screen painting is strictly controlled. (and double buffered)
            OSD_ProcessDisplayUpdate(hDC, &OSDDisplayRect);

            ReleaseDC(hWnd, hDC);
            Sleep(2000);
        }
        else
        {
            _sntprintf(szDispMessage, sizeof(szDispMessage), _T("%s\nThe error occured in %s at line %d"), Message.c_str(), szFile, Line);
            RealErrorBox(szDispMessage);
        }
        AlreadyInErrorBox = FALSE;
    }
    else
    {
        _sntprintf(szDispMessage, sizeof(szDispMessage), _T("%s\nThe error occured in %s at line %d"), Message.c_str(), szFile, Line);
        RealErrorBox(szDispMessage);
    }
#else
    _sntprintf(szDispMessage, sizeof(szDispMessage), _T("%s\nThe error occured in %s at line %d"), Message.c_str(), szFile, Line);
    if(hwndParent == NULL)
    {
        MessageBox(hWnd, szDispMessage, _T("DScaler Error"), MB_ICONSTOP | MB_OK);
    }
    else
    {
        MessageBox(hwndParent,szDispMessage, _T("DScaler Error"), MB_ICONSTOP | MB_OK);
    }
#endif
}

void _RealErrorBox(LPCTSTR szFile, int Line, const tstring& Message)
{
    LOG(0, _T("ErrorBox File:%s line: %d Message: %s"), szFile, Line, Message.c_str());
    HideSplashScreen();
    MessageBox(hWnd, Message.c_str(), _T("DScaler Error"), MB_ICONSTOP | MB_OK);
}

