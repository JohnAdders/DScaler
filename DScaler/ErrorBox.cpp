/////////////////////////////////////////////////////////////////////////////
// $Id: ErrorBox.cpp,v 1.10 2003-10-27 10:39:51 adcockj Exp $
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
// Change Log
//
// Date          Developer             Changes
//
// 11 Aug 2000   John Adcock           Better support for error messages
//
// 12 Dec 2000   Mark Rejhon           Changed to use onscreen display
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.9  2003/01/25 12:44:38  atnak
// Updates for new OSD code
//
// Revision 1.8  2002/09/04 17:58:09  robmuller
// Second attempt:
// Changed the debug log level to zero to make sure that the error message is written to the log file.
//
// Revision 1.7  2002/09/04 17:56:57  robmuller
// RealErrorBox() now writes the error message to the log file.
// Changed the debug log level to zero to make sure that the error message is written to the log file.
//
// Revision 1.6  2001/12/30 16:50:32  adcockj
// Prevent recusion in ErrorBox
//
// Revision 1.5  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.4  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.3  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

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

extern HWND hWnd;

void _ErrorBoxOSD(HWND hwndParent, LPCSTR szFile, int Line, LPCSTR szMessage)
{
}


void _ErrorBox(HWND hwndParent, LPCSTR szFile, int Line, LPCSTR szMessage)
{
    char szDispMessage[1024];
	// variable used to prevent recusive error problems
	static BOOL AlreadyInErrorBox = FALSE;

    // put the message into the log for debugging
    LOG(0, "ErrorBox File:%s line: %d Message: %s", szFile, Line, szMessage);

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
            _snprintf(szDispMessage, sizeof(szDispMessage), "ERROR: %s\nFile %s Line %d", szMessage, szFile, Line);
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
            _snprintf(szDispMessage, sizeof(szDispMessage), "%s\nThe error occured in %s at line %d", szMessage, szFile, Line);
            RealErrorBox(szDispMessage);
        }
		AlreadyInErrorBox = FALSE;
    }
    else
    {
        _snprintf(szDispMessage, sizeof(szDispMessage), "%s\nThe error occured in %s at line %d", szMessage, szFile, Line);
        RealErrorBox(szDispMessage);
    }
#else
    _snprintf(szDispMessage, sizeof(szDispMessage), "%s\nThe error occured in %s at line %d", szMessage, szFile, Line);
    if(hwndParent == NULL)
    {
        MessageBox(hWnd, szDispMessage, "DScaler Error", MB_ICONSTOP | MB_OK);
    }
    else
    {
        MessageBox(hwndParent, szDispMessage, "DScaler Error", MB_ICONSTOP | MB_OK);
    }
#endif
}

void _RealErrorBox(LPCSTR szFile, int Line, LPCSTR szMessage)
{
    LOG(0, "ErrorBox File:%s line: %d Message: %s", szFile, Line, szMessage);
    HideSplashScreen();
    MessageBox(hWnd, szMessage, "DScaler Error", MB_ICONSTOP | MB_OK);
}

