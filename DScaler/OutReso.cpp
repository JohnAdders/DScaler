/////////////////////////////////////////////////////////////////////////////
// $Id: OutReso.cpp,v 1.3 2003-02-06 12:22:57 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Laurent Garnier  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
//
// This module uses code from Kristian Trenskow provided as DScaler patch
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2003/02/06 09:59:40  laurentg
// Change resolution in full screen
//
// Revision 1.1  2003/02/06 00:58:53  laurentg
// Change output resolution (first step)
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DScaler.h"
#include "OutReso.h"
#include "DebugLog.h"


typedef struct
{
	BOOL	bSwitchScreen;
	char	szMenuLabel[32];
	int		intResWidth;
	int		intResHeight;
	int		intResDepth;
	int		intResFreq;
} sResolution;


static sResolution resSettings[] = {
	{	FALSE,	"None",						0,		0,		0,	0	},
	{	TRUE,	"640x480 16bit 60 Hz",		640,	480,	16,	60	},
	{	TRUE,	"640x480 16bit 72 Hz",		640,	480,	16,	72	},
	{	TRUE,	"640x480 16bit 75 Hz",		640,	480,	16,	75	},
	{	TRUE,	"640x480 16bit 100 Hz",		640,	480,	16,	100	},
	{	TRUE,	"640x480 16bit 120 Hz",		640,	480,	16,	120	},
	{	TRUE,	"640x480 32bit 60 Hz",		640,	480,	32,	60	},
	{	TRUE,	"640x480 32bit 72 Hz",		640,	480,	32,	72	},
	{	TRUE,	"640x480 32bit 75 Hz",		640,	480,	32,	75	},
	{	TRUE,	"640x480 32bit 100 Hz",		640,	480,	32,	100	},
	{	TRUE,	"640x480 32bit 120 Hz",		640,	480,	32,	120	},
	{	TRUE,	"800x600 16bit 60 Hz",		800,	600,	16,	60	},
	{	TRUE,	"800x600 16bit 72 Hz",		800,	600,	16,	72	},
	{	TRUE,	"800x600 16bit 75 Hz",		800,	600,	16,	75	},
	{	TRUE,	"800x600 16bit 100 Hz",		800,	600,	16,	100	},
	{	TRUE,	"800x600 16bit 120 Hz",		800,	600,	16,	120	},
	{	TRUE,	"800x600 32bit 60 Hz",		800,	600,	32,	60	},
	{	TRUE,	"800x600 32bit 72 Hz",		800,	600,	32,	72	},
	{	TRUE,	"800x600 32bit 75 Hz",		800,	600,	32,	75	},
	{	TRUE,	"800x600 32bit 100 Hz",		800,	600,	32,	100	},
	{	TRUE,	"800x600 32bit 120 Hz",		800,	600,	32,	120	},
	{	TRUE,	"1024x768 16bit 60 Hz",		1024,	768,	16,	60	},
	{	TRUE,	"1024x768 16bit 72 Hz",		1024,	768,	16,	72	},
	{	TRUE,	"1024x768 16bit 75 Hz",		1024,	768,	16,	75	},
	{	TRUE,	"1024x768 16bit 100 Hz",	1024,	768,	16,	100	},
	{	TRUE,	"1024x768 16bit 120 Hz",	1024,	768,	16,	120	},
	{	TRUE,	"1024x768 32bit 60 Hz",		1024,	768,	32,	60	},
	{	TRUE,	"1024x768 32bit 72 Hz",		1024,	768,	32,	72	},
	{	TRUE,	"1024x768 32bit 75 Hz",		1024,	768,	32,	75	},
	{	TRUE,	"1024x768 32bit 100 Hz",	1024,	768,	32,	100	},
	{	TRUE,	"1024x768 32bit 120 Hz",	1024,	768,	32,	120	},
};


int OutputReso = 0;


/////////////////////////////////////////////////////////////////////////////
// Start of Menus related code
/////////////////////////////////////////////////////////////////////////////

void OutReso_UpdateMenu(HMENU hMenu)
{
    HMENU	hMenuReso;
    int		i;
    int		n;

    hMenuReso = GetOutResoSubmenu();
    if (hMenuReso == NULL)
    {
        return;
    }

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (i=0; i < n ; i++)
    {
        UINT Flags(MF_STRING);
        AppendMenu(hMenuReso, Flags, IDM_OUTPUTRESO + i, resSettings[i].szMenuLabel);
    }
}


void OutReso_SetMenu(HMENU hMenu)
{
    HMENU   hMenuReso;
    int     i;
    int     n;

    hMenuReso = GetOutResoSubmenu();
    if (hMenuReso == NULL)
    {
        return;
    }

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (i=0; i < n ; i++)
    {
        CheckMenuItemBool(hMenuReso, IDM_OUTPUTRESO + i, i == OutputReso);
    }
}


BOOL ProcessOutResoSelection(HWND hWnd, WORD wMenuID)
{
    if (wMenuID >= IDM_OUTPUTRESO && wMenuID < (IDM_OUTPUTRESO + 32))
    {
		OutputReso = wMenuID - IDM_OUTPUTRESO;
        return TRUE;
    }
    return FALSE;
}

void OutReso_Change(HWND hWnd, BOOL bUseRegistrySettings)
{
    DEVMODE dm;

	if (resSettings[OutputReso].bSwitchScreen)
	{
        dm.dmSize = sizeof(DEVMODE);
        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		if (bUseRegistrySettings)
		{
	        EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm);
		}
		else
		{
			dm.dmPelsWidth = resSettings[OutputReso].intResWidth;
			dm.dmPelsHeight = resSettings[OutputReso].intResHeight;
			dm.dmBitsPerPel = resSettings[OutputReso].intResDepth;
			dm.dmDisplayFrequency = resSettings[OutputReso].intResFreq;
		}
		Overlay_Stop(hWnd);
//        ShowWindow(hWnd, SW_HIDE);
        ChangeDisplaySettings(&dm, 0);
//        ShowWindow(hWnd, SW_SHOW);
		Overlay_Start(hWnd);
	}
}
