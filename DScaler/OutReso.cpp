/////////////////////////////////////////////////////////////////////////////
// $Id: OutReso.cpp,v 1.11 2003-04-15 13:07:10 adcockj Exp $
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
// Revision 1.10  2003/04/12 15:23:23  laurentg
// Interface with PowerStrip when changing resolution (code from Olivier Borca)
//
// Revision 1.9  2003/02/14 22:06:14  laurentg
// New resolutions added : 854x480, 1024x576, 1280x720
//
// Revision 1.8  2003/02/10 21:35:37  laurentg
// Menu updated (patch from Kristian Trenskow)
//
// Revision 1.7  2003/02/08 13:16:47  laurentg
// Change resolution in full screen mode slightly updated
//
// Revision 1.6  2003/02/07 12:46:17  laurentg
// Change resolution correctly handled when DScaler is minimized and restored
//
// Revision 1.5  2003/02/07 11:28:25  laurentg
// Keep more ids for the output reso menus (100)
// New resolutions added (720x480 and 720x576)
//
// Revision 1.4  2003/02/06 14:08:03  laurentg
// Only display supported resolutions
// Do the switch only if the target display resolution is different from the current
//
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
#include "Setting.h"
#include "Other.h"
#include "DebugLog.h"
#include "Providers.h"


typedef struct
{
	BOOL	bSwitchScreen;
	int		intResWidth;
	int		intResHeight;
	int		intResDepth;
	int		intResFreq;
	BOOL	bSupported;
	// Menu related items
	int		intMenuPixelPos;
	int		intMenuDepthPos;
} sResolution;


static sResolution resSettings[] = {
	//  Do Switch	Width	Height	Depth	Frequency	Supported
	{	FALSE,		0,		0,		0,		0,			TRUE,		0,	0	},
	{	TRUE,		640,	480,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		640,	480,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		640,	480,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		640,	480,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		640,	480,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		640,	480,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		640,	480,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		640,	480,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		640,	480,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		640,	480,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		720,	480,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		720,	480,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		720,	480,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		720,	480,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		720,	480,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		720,	480,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		720,	480,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		720,	480,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		720,	480,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		720,	480,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		854,	480,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		854,	480,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		854,	480,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		854,	480,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		854,	480,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		854,	480,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		854,	480,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		854,	480,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		854,	480,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		854,	480,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		720,	576,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		720,	576,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		720,	576,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		720,	576,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		720,	576,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		720,	576,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		720,	576,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		720,	576,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		720,	576,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		720,	576,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		768,	576,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		768,	576,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		768,	576,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		768,	576,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		768,	576,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		768,	576,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		768,	576,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		768,	576,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		768,	576,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		768,	576,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		1024,	576,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		1024,	576,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		1024,	576,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		1024,	576,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		1024,	576,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		1024,	576,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		1024,	576,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		1024,	576,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		1024,	576,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		1024,	576,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		800,	600,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		800,	600,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		800,	600,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		800,	600,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		800,	600,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		800,	600,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		800,	600,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		800,	600,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		800,	600,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		800,	600,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		1280,	720,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		1280,	720,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		1280,	720,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		1280,	720,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		1280,	720,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		1280,	720,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		1280,	720,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		1280,	720,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		1280,	720,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		1280,	720,	32,		120,		FALSE,		0,	0	},
	{	TRUE,		1024,	768,	16,		60,			FALSE,		0,	0	},
	{	TRUE,		1024,	768,	16,		72,			FALSE,		0,	0	},
	{	TRUE,		1024,	768,	16,		75,			FALSE,		0,	0	},
	{	TRUE,		1024,	768,	16,		100,		FALSE,		0,	0	},
	{	TRUE,		1024,	768,	16,		120,		FALSE,		0,	0	},
	{	TRUE,		1024,	768,	32,		60,			FALSE,		0,	0	},
	{	TRUE,		1024,	768,	32,		72,			FALSE,		0,	0	},
	{	TRUE,		1024,	768,	32,		75,			FALSE,		0,	0	},
	{	TRUE,		1024,	768,	32,		100,		FALSE,		0,	0	},
	{	TRUE,		1024,	768,	32,		120,		FALSE,		0,	0	},
};


int OutputReso = 0;
LPSTR PStrip576i = NULL;
LPSTR PStrip480i = NULL;

void PStripTiming_ReadSettingsFromIni()
{    
    PStripTiming_CleanUp();
	PStrip576i = new char[PSTRIP_TIMING_STRING_SIZE];
	PStrip480i = new char[PSTRIP_TIMING_STRING_SIZE];
    GetPrivateProfileString("PStripOutResolution", "576i", NULL, PStrip576i, PSTRIP_TIMING_STRING_SIZE, GetIniFileForSettings());
    GetPrivateProfileString("PStripOutResolution", "480i", NULL, PStrip480i, PSTRIP_TIMING_STRING_SIZE, GetIniFileForSettings());
}

void PStripTiming_CleanUp()
{
    if(PStrip576i != NULL)
    {
        delete PStrip576i; 
        PStrip576i = NULL;
    }
    if(PStrip480i != NULL)
    {
        delete PStrip480i; 
        PStrip480i = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Start of Menus related code
/////////////////////////////////////////////////////////////////////////////

void OutReso_UpdateMenu(HMENU hMenu)
{
    HMENU	hMenuReso;
	HMENU   hMenuPixel;
	HMENU   hMenuDepth;

	int		lastWidth = 1;
	int		lastHeight = 1;
	int		lastDepth = 1;
	char	szTmp[20] = "\0";

    int		i, j, n, pixel = 2, depth = 0;

    hMenuReso = GetOutResoSubmenu();
    if (hMenuReso == NULL)
    {
        return;
    }

	// Add "Do nothing" and seperator

	AppendMenu(hMenuReso, MF_STRING, IDM_OUTPUTRESO, "Don't change");
	AppendMenu(hMenuReso, MF_SEPARATOR, 0, NULL);

	// Add in menus only supported display settings

    n = sizeof (resSettings) / sizeof (resSettings[0]);
    for (i=1,j=1; i < n ; i++)
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
				resSettings[i].bSupported = FALSE;
			}
			else
			{
				resSettings[i].bSupported = TRUE;
			}
		}

		if (resSettings[i].bSupported)
		{
			UINT Flags(MF_STRING);

			if (lastWidth != resSettings[i].intResWidth  ||  lastHeight != resSettings[i].intResHeight)
			{
				sprintf(szTmp, "%dx%d", resSettings[i].intResWidth, resSettings[i].intResHeight);
				hMenuPixel = CreateMenu();
				InsertMenu(hMenuReso, -1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) hMenuPixel, szTmp);
				pixel++;
				depth = 0;
				lastDepth = 0;
			}

			if (lastDepth != resSettings[i].intResDepth)
			{
				sprintf(szTmp, "%d bit", resSettings[i].intResDepth);
				hMenuDepth = CreateMenu();
				InsertMenu(hMenuPixel, -1, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT) hMenuDepth, szTmp);
				depth++;
			}

			sprintf(szTmp, "%d Hz", resSettings[i].intResFreq);
			AppendMenu(hMenuDepth, Flags, IDM_OUTPUTRESO + j, szTmp);
			j++;

			lastWidth = resSettings[i].intResWidth;
			lastHeight = resSettings[i].intResHeight;
			lastDepth = resSettings[i].intResDepth;
			
			resSettings[i].intMenuPixelPos = pixel - 1;
			resSettings[i].intMenuDepthPos = depth - 1;

			// Stop as soon as MAX_NUMBER_RESO is reached
			if (j >= MAX_NUMBER_RESO)
			{
				break;
			}
		}
    }

	// Update the value for the RESOFULLSCREEN setting to "none"
	// if the current value is greater than the number of items in menu
	SETTING* pSetting = DScaler_GetSetting(RESOFULLSCREEN);
	if (pSetting)
	{
		if (Setting_GetValue(pSetting) > (j-1))
		{
			Setting_SetValue(pSetting, 0);
		}
	}
}


void OutReso_SetMenu(HMENU hMenu)
{
    HMENU   hMenuReso;
    int     i, j, n, selected;

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
			CheckMenuItem(hMenuReso, resSettings[i].intMenuPixelPos, MF_UNCHECKED | MF_BYPOSITION);
			HMENU hMenuPixel = GetSubMenu(hMenuReso, resSettings[i].intMenuPixelPos);
			CheckMenuItem(hMenuPixel, resSettings[i].intMenuDepthPos, MF_UNCHECKED | MF_BYPOSITION);
	        CheckMenuItem(hMenuReso, IDM_OUTPUTRESO + j, MF_UNCHECKED);
			if (j==OutputReso)
			{
				selected = i;
			}

			j++;
			// Stop as soon as MAX_NUMBER_RESO is reached
			if (j >= MAX_NUMBER_RESO)
			{
				break;
			}
		}
    }

	CheckMenuItem(hMenuReso, IDM_OUTPUTRESO + OutputReso, MF_CHECKED);
	if (OutputReso>0)
	{
		CheckMenuItem(hMenuReso, resSettings[selected].intMenuPixelPos, MF_CHECKED | MF_BYPOSITION);
		HMENU hMenuPixel = GetSubMenu(hMenuReso, resSettings[selected].intMenuPixelPos);
		CheckMenuItem(hMenuPixel, resSettings[selected].intMenuDepthPos, MF_CHECKED | MF_BYPOSITION);
	}
}


BOOL ProcessOutResoSelection(HWND hWnd, WORD wMenuID)
{
    if (wMenuID >= IDM_OUTPUTRESO && wMenuID < (IDM_OUTPUTRESO + MAX_NUMBER_RESO))
    {
		OutputReso = wMenuID - IDM_OUTPUTRESO;
        return TRUE;
    }
    return FALSE;
}

void OutReso_Change(HWND hWnd, HWND hPSWnd, BOOL bUseRegistrySettings, BOOL bCaptureRunning, LPSTR lTimingString, BOOL bApplyPStripTimingString)
{
	// If PowerStrip has been found, use it
	if(hPSWnd)
	{
		ATOM aPStripTimingATOM;
		
		if((lTimingString != NULL) && (bApplyPStripTimingString))
		{
			aPStripTimingATOM = GlobalAddAtom(lTimingString);		

			// Apply the PowerStrip timing string
			// If the PostMessage is successfull, the Atom is automatically deleted
			if(!PostMessage(hPSWnd, UM_SETPSTRIPTIMING, 0, aPStripTimingATOM))
			{
				GlobalDeleteAtom(aPStripTimingATOM);
			}
		}
		else if (Providers_GetCurrentSource())
		{
			// Get the video format
			eVideoFormat videoFormat = Providers_GetCurrentSource()->GetFormat();

			// 576i_50Hz and 576i_60Hz
			if((videoFormat == VIDEOFORMAT_PAL_B) || (videoFormat == VIDEOFORMAT_PAL_D) || (videoFormat == VIDEOFORMAT_PAL_G) || (videoFormat == VIDEOFORMAT_PAL_H)
				|| (videoFormat == VIDEOFORMAT_PAL_I) || (videoFormat == VIDEOFORMAT_PAL_M) || (videoFormat == VIDEOFORMAT_PAL_N)
				|| (videoFormat == VIDEOFORMAT_PAL_60) || (videoFormat == VIDEOFORMAT_PAL_N_COMBO))
			{				
				aPStripTimingATOM = GlobalAddAtom(PStrip576i);
			}
			// 480i_50Hz and 480i_60Hz
			else if((videoFormat == VIDEOFORMAT_NTSC_M) || (videoFormat == VIDEOFORMAT_NTSC_M_Japan) || (videoFormat == VIDEOFORMAT_NTSC_50))
			{				
				aPStripTimingATOM = GlobalAddAtom(PStrip480i);
			}	
			
			// Apply the PowerStrip timing string
			// If the PostMessage is successfull, the Atom is automatically deleted
			if(!PostMessage(hPSWnd, UM_SETPSTRIPTIMING, 0, aPStripTimingATOM))
			{
				GlobalDeleteAtom(aPStripTimingATOM);
			}
		}
	}
	else
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
				BOOL bOverlay = OverlayActive();

				// Stop the overlay (and the capture)
				if (bOverlay)
				{
					if (bCaptureRunning)
					{
						Overlay_Stop(hWnd);
					}
					else
					{
						Overlay_Destroy();
					}
				}

	//	        ShowWindow(hWnd, SW_HIDE);
				ChangeDisplaySettings(&dm, 0);
	//	        ShowWindow(hWnd, SW_SHOW);

				// Restart the overlay (and the capture)
				if (bOverlay)
				{
					if (bCaptureRunning)
					{
						Overlay_Start(hWnd);
					}
					else
					{
						Overlay_Create();
					}
				}
			}
		}
	}
}