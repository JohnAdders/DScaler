/////////////////////////////////////////////////////////////////////////////
// ErrorBox.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
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

#include "stdafx.h"
#include "splash.h"

#ifndef _DEBUG
#include "OSD.h"
#endif

extern HWND hWnd;

void _ErrorBox(HWND hwndParent, LPCSTR szFile, int Line, LPCSTR szMessage)
{
	char szDispMessage[1024];

#ifndef _DEBUG
    if (hWnd != NULL)
    {
		HDC hDC;
		hDC = GetDC(hWnd);
        // Show OSD text immediately and pause for 2 seconds.
        // OSD will continue to show after 2 seconds,
        // if there are no other OSD's pending afterwards. (thus the reason for 2 second delay)
        _snprintf(szDispMessage, sizeof(szDispMessage), "ERROR: %s", szMessage);
        OSD_ShowTextPersistent(hWnd, szDispMessage, 4);
        OSD_Redraw(hWnd, hDC);
		ReleaseDC(hWnd, hDC);
        Sleep(2000);
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

void RealErrorBox(LPCSTR msg)
{
	HideSplashScreen();
	MessageBox(hWnd, msg, "DScaler Error", MB_ICONSTOP | MB_OK);
}

