/////////////////////////////////////////////////////////////////////////////
// $Id: OutReso.cpp,v 1.4 2003-02-06 14:08:03 laurentg Exp $
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
// Revision 1.3  2003/02/06 12:22:57  laurentg
// Take the refresh rate when changing resolution (choice between 60, 72, 75, 100 and 120 Hz)
//
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
	BOOL	bSupported;
} sResolution;


static sResolution resSettings[] = {
	{	FALSE,	"None",						0,		0,		0,	0,		TRUE	},
	{	TRUE,	"640x480 16bit 60 Hz",		640,	480,	16,	60,		TRUE	},
	{	TRUE,	"640x480 16bit 72 Hz",		640,	480,	16,	72,		TRUE	},
	{	TRUE,	"640x480 16bit 75 Hz",		640,	480,	16,	75,		TRUE	},
	{	TRUE,	"640x480 16bit 100 Hz",		640,	480,	16,	100,	TRUE	},
	{	TRUE,	"640x480 16bit 120 Hz",		640,	480,	16,	120,	TRUE	},
	{	TRUE,	"640x480 32bit 60 Hz",		640,	480,	32,	60,		TRUE	},
	{	TRUE,	"640x480 32bit 72 Hz",		640,	480,	32,	72,		TRUE	},
	{	TRUE,	"640x480 32bit 75 Hz",		640,	480,	32,	75,		TRUE	},
	{	TRUE,	"640x480 32bit 100 Hz",		640,	480,	32,	100,	TRUE	},
	{	TRUE,	"640x480 32bit 120 Hz",		640,	480,	32,	120,	TRUE	},
	{	TRUE,	"800x600 16bit 60 Hz",		800,	600,	16,	60,		TRUE	},
	{	TRUE,	"800x600 16bit 72 Hz",		800,	600,	16,	72,		TRUE	},
	{	TRUE,	"800x600 16bit 75 Hz",		800,	600,	16,	75,		TRUE	},
	{	TRUE,	"800x600 16bit 100 Hz",		800,	600,	16,	100,	TRUE	},
	{	TRUE,	"800x600 16bit 120 Hz",		800,	600,	16,	120,	TRUE	},
	{	TRUE,	"800x600 32bit 60 Hz",		800,	600,	32,	60,		TRUE	},
	{	TRUE,	"800x600 32bit 72 Hz",		800,	600,	32,	72,		TRUE	},
	{	TRUE,	"800x600 32bit 75 Hz",		800,	600,	32,	75,		TRUE	},
	{	TRUE,	"800x600 32bit 100 Hz",		800,	600,	32,	100,	TRUE	},
	{	TRUE,	"800x600 32bit 120 Hz",		800,	600,	32,	120,	TRUE	},
	{	TRUE,	"1024x768 16bit 60 Hz",		1024,	768,	16,	60,		TRUE	},
	{	TRUE,	"1024x768 16bit 72 Hz",		1024,	768,	16,	72,		TRUE	},
	{	TRUE,	"1024x768 16bit 75 Hz",		1024,	768,	16,	75,		TRUE	},
	{	TRUE,	"1024x768 16bit 100 Hz",	1024,	768,	16,	100,	TRUE	},
	{	TRUE,	"1024x768 16bit 120 Hz",	1024,	768,	16,	120,	TRUE	},
	{	TRUE,	"1024x768 32bit 60 Hz",		1024,	768,	32,	60,		TRUE	},
	{	TRUE,	"1024x768 32bit 72 Hz",		1024,	768,	32,	72,		TRUE	},
	{	TRUE,	"1024x768 32bit 75 Hz",		1024,	768,	32,	75,		TRUE	},
	{	TRUE,	"1024x768 32bit 100 Hz",	1024,	768,	32,	100,	TRUE	},
	{	TRUE,	"1024x768 32bit 120 Hz",	1024,	768,	32,	120,	TRUE	},
};


int OutputReso = 0;


/////////////////////////////////////////////////////////////////////////////
// Start of Menus related code
/////////////////////////////////////////////////////////////////////////////

void OutReso_UpdateMenu(HMENU hMenu)
{
    HMENU	hMenuReso;
    int		i, j, n;

    hMenuReso = GetOutResoSubmenu();
    if (hMenuReso == NULL)
    {
        return;
    }

	// Add in menus only supported display settings

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (i=0,j=0; i < n ; i++)
    {
		if (resSettings[i].bSwitchScreen)
		{
			DEVMODE dm;
			dm.dmSize = sizeof(DEVMODE);
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
			dm.dmPelsWidth = resSettings[i].intResWidth;
			dm.dmPelsHeight = resSettings[i].intResHeight;
			dm.dmBitsPerPel = resSettings[i].intResDepth;
			dm.dmDisplayFrequency = resSettings[i].intResFreq;
			if (ChangeDisplaySettings(&dm, CDS_TEST) != DISP_CHANGE_SUCCESSFUL)
			{
				EnableMenuItemBool(hMenuReso, IDM_OUTPUTRESO + i, FALSE);
				resSettings[i].bSupported = FALSE;
			}
			else
			{
				resSettings[i].bSupported = TRUE;
			}
		}
		else
		{
			resSettings[i].bSupported = TRUE;
		}
		if (resSettings[i].bSupported)
		{
			UINT Flags(MF_STRING);
			AppendMenu(hMenuReso, Flags, IDM_OUTPUTRESO + j, resSettings[i].szMenuLabel);
			j++;
		}
    }
}


void OutReso_SetMenu(HMENU hMenu)
{
    HMENU   hMenuReso;
    int     i, j, n;

    hMenuReso = GetOutResoSubmenu();
    if (hMenuReso == NULL)
    {
        return;
    }

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (i=0,j=0; i < n ; i++)
    {
		if (resSettings[i].bSupported)
		{
	        CheckMenuItemBool(hMenuReso, IDM_OUTPUTRESO + j, j == OutputReso);
			j++;
		}
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
    DEVMODE dm_cur;
    int     i, idx, n;

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (idx=0,i=0; idx < n ; idx++)
    {
		if (resSettings[idx].bSupported)
		{
			if (i == OutputReso)
			{
				break;
			}
			i++;
		}
    }

	if (resSettings[idx].bSwitchScreen)
	{
        dm.dmSize = sizeof(DEVMODE);
        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		if (bUseRegistrySettings)
		{
			// Get the display settings from registry
	        EnumDisplaySettings(NULL, ENUM_REGISTRY_SETTINGS, &dm);
		}
		else
		{
			// Use the display settings defined by the user
			dm.dmPelsWidth = resSettings[idx].intResWidth;
			dm.dmPelsHeight = resSettings[idx].intResHeight;
			dm.dmBitsPerPel = resSettings[idx].intResDepth;
			dm.dmDisplayFrequency = resSettings[idx].intResFreq;
		}

		// Change display settings only if different from current
        dm_cur.dmSize = sizeof(DEVMODE);
        dm_cur.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm_cur);
		if ( (dm.dmPelsWidth != dm_cur.dmPelsWidth)
		  || (dm.dmPelsHeight != dm_cur.dmPelsHeight)
		  || (dm.dmBitsPerPel != dm_cur.dmBitsPerPel)
		  || (dm.dmDisplayFrequency != dm_cur.dmDisplayFrequency))
		{
			Overlay_Stop(hWnd);
//	        ShowWindow(hWnd, SW_HIDE);
			ChangeDisplaySettings(&dm, 0);
//	        ShowWindow(hWnd, SW_SHOW);
			Overlay_Start(hWnd);
		}
	}
}
