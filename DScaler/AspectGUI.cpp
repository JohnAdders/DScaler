/////////////////////////////////////////////////////////////////////////////
// AspectGUI.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Michael Samblanet  All rights reserved.
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
//
// Aspect ratio control was started by Michael Samblanet <mike@cardobe.com>
// Moved into separate module by Mark D Rejhon.  
// AspectRatio.c was separated into 3 files by Michael Samblanet 13 March 2001
//
// The purpose of this module is all the calculations and handling necessary
// to map the source image onto the destination display, even if they are
// different aspect ratios.
//
// Portions copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 13 Mar 2001   Michael Samblanet     File created from code in AspectRatio.c
//									   Split out to improve redability
//
// 08 Jun 2001   Eric Schmidt          Added bounce amplitude to ini
//
//////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Other.h"
#include "AspectRatio.h"
#include "Resource.h"
#include "DebugLog.h"
#include "Status.h"
#include "BT848.h"
#include "DScaler.h"
#include "SettingsDlg.h"

#define AR_STRETCH       0
#define AR_NONANAMORPHIC 1
#define AR_ANAMORPHIC    2

// From DScaler.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
//extern HMENU hMenu;
//extern BOOL  bIsFullScreen;
//extern void ShowText(HWND hWnd, LPCTSTR szText);

BOOL Bounce_OnChange(long NewValue); // Forward declaration to reuse this code...
BOOL Orbit_OnChange(long NewValue); // Forward declaration to reuse this code...

//----------------------------------------------------------------------------
// MENU INITIALIZATION:
// Initializing aspect ratio control related checkboxes in menu
void AspectRatio_SetMenu(HMENU hMenu)
{
	CheckMenuItem(hMenu, IDM_ASPECT_FULLSCREEN, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ASPECT_LETTERBOX,  MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ASPECT_ANAMORPHIC, MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_SASPECT_133, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_144, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_155, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_166, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_178, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_185, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_200, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_235, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_166A, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_178A, MF_UNCHECKED); 
	CheckMenuItem(hMenu, IDM_SASPECT_185A, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_200A, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_235A, MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_AUTO_TOGGLE, MF_UNCHECKED);

	if (aspectSettings.AutoDetectAspect)
	{
		CheckMenuItem(hMenu, IDM_SASPECT_AUTO_TOGGLE, MF_CHECKED);
	}
	else if (aspectSettings.aspect_mode == 1)
	{
		switch (aspectSettings.source_aspect)
		{
		case 1333:
			CheckMenuItem(hMenu, IDM_SASPECT_133, MF_CHECKED);
			CheckMenuItem(hMenu, IDM_ASPECT_FULLSCREEN, MF_CHECKED);
			break;
		case 1444:
			CheckMenuItem(hMenu, IDM_SASPECT_144, MF_CHECKED);
			break;
		case 1555:
			CheckMenuItem(hMenu, IDM_SASPECT_155, MF_CHECKED);
			break;
		case 1667:
			CheckMenuItem(hMenu, IDM_SASPECT_166, MF_CHECKED);
			break;
		case 1778:
			CheckMenuItem(hMenu, IDM_SASPECT_178, MF_CHECKED);
			CheckMenuItem(hMenu, IDM_ASPECT_LETTERBOX, MF_CHECKED);
			break;
		case 1850:
			CheckMenuItem(hMenu, IDM_SASPECT_185, MF_CHECKED);
			break;
		case 2000:
			CheckMenuItem(hMenu, IDM_SASPECT_200, MF_CHECKED);
			break;
		case 2350:
			CheckMenuItem(hMenu, IDM_SASPECT_235, MF_CHECKED);
			break;
		}
	}
	else if (aspectSettings.aspect_mode == 2)
	{
		switch (aspectSettings.source_aspect)
		{
		case 1667:
			CheckMenuItem(hMenu, IDM_SASPECT_166A, MF_CHECKED);
			break;
		case 1778:
			CheckMenuItem(hMenu, IDM_SASPECT_178A, MF_CHECKED); 
			CheckMenuItem(hMenu, IDM_ASPECT_ANAMORPHIC, MF_CHECKED); 
            break;
		case 1850:
			CheckMenuItem(hMenu, IDM_SASPECT_185A, MF_CHECKED);
			break;
		case 2000:
			CheckMenuItem(hMenu, IDM_SASPECT_200A, MF_CHECKED);
			break;
		case 2350:
			CheckMenuItem(hMenu, IDM_SASPECT_235A, MF_CHECKED);
			break;
		}
	}
	CheckMenuItem(hMenu, IDM_SASPECT_0,   (aspectSettings.source_aspect == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SASPECT_CUSTOM, (aspectSettings.source_aspect && aspectSettings.source_aspect == aspectSettings.custom_source_aspect)?MF_CHECKED:MF_UNCHECKED);

	// Advanced Aspect Ratio -> Display Aspect Ratio
	CheckMenuItem(hMenu, IDM_TASPECT_0,   (aspectSettings.target_aspect == 0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TASPECT_133, (aspectSettings.target_aspect == 1333)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TASPECT_166, (aspectSettings.target_aspect == 1667)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TASPECT_178, (aspectSettings.target_aspect == 1778)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TASPECT_185, (aspectSettings.target_aspect == 1850)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TASPECT_200, (aspectSettings.target_aspect == 2000)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TASPECT_235, (aspectSettings.target_aspect == 2350)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_TASPECT_CUSTOM, (aspectSettings.target_aspect && aspectSettings.target_aspect == aspectSettings.custom_target_aspect)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_WINPOS_VERT_CENTRE, (aspectSettings.VerticalPos == VERT_POS_CENTRE)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WINPOS_VERT_TOP, (aspectSettings.VerticalPos == VERT_POS_TOP)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WINPOS_VERT_BOTTOM, (aspectSettings.VerticalPos == VERT_POS_BOTTOM)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_WINPOS_HORZ_CENTRE, (aspectSettings.HorizontalPos == HORZ_POS_CENTRE)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WINPOS_HORZ_LEFT, (aspectSettings.HorizontalPos == HORZ_POS_LEFT)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WINPOS_HORZ_RIGHT, (aspectSettings.HorizontalPos == HORZ_POS_RIGHT)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, IDM_SASPECT_CLIP, (aspectSettings.aspectImageClipped)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WINPOS_BOUNCE, (aspectSettings.bounceEnabled)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WINPOS_ORBIT, (aspectSettings.orbitEnabled)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ASPECT_DEFER_OVERLAY, (aspectSettings.deferedSetOverlay)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WINPOS_AUTOSIZE, (aspectSettings.autoResizeWindow)?MF_CHECKED:MF_UNCHECKED);

	// Zoom
	CheckMenuItem(hMenu, IDM_ZOOM_10, (aspectSettings.xZoomFactor == 100 && aspectSettings.yZoomFactor == 100)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ZOOM_15, (aspectSettings.xZoomFactor == 150 && aspectSettings.yZoomFactor == 150)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ZOOM_20, (aspectSettings.xZoomFactor == 200 && aspectSettings.yZoomFactor == 200)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ZOOM_25, (aspectSettings.xZoomFactor == 250 && aspectSettings.yZoomFactor == 250)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ZOOM_30, (aspectSettings.xZoomFactor == 300 && aspectSettings.yZoomFactor == 300)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ZOOM_35, (aspectSettings.xZoomFactor == 350 && aspectSettings.yZoomFactor == 350)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_ZOOM_40, (aspectSettings.xZoomFactor == 400 && aspectSettings.yZoomFactor == 400)?MF_CHECKED:MF_UNCHECKED);

}

//----------------------------------------------------------------------------
// MENU PROCESSING:
// Processing aspect ratio control related menu selection
// during aspect ratio control
BOOL ProcessAspectRatioSelection(HWND hWnd, WORD wMenuID)
{
	char Text[32];

	switch (wMenuID) {
	//------------------------------------------------------------------
	// Easily Accessible Aspect Ratios
	case IDM_ASPECT_FULLSCREEN:
        if (aspectSettings.AutoDetectAspect)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            // This applies to both letterbox and 4:3
		    SwitchToRatio(AR_NONANAMORPHIC, -1);
            ShowText(hWnd, "Nonanamorphic Signal");
            aspectSettings.DetectAspectNow = TRUE;
        }
        else
        {
		    SwitchToRatio(AR_NONANAMORPHIC, 1333);
		    ShowText(hWnd, "4:3 Fullscreen Signal");
        }
		break;
	case IDM_ASPECT_LETTERBOX:
        if (aspectSettings.AutoDetectAspect)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            // This applies to both letterbox and 4:3
		    SwitchToRatio(AR_NONANAMORPHIC, -1);
            ShowText(hWnd, "Nonanamorphic Signal");
            aspectSettings.DetectAspectNow = TRUE;
        }
        else
        {
		    SwitchToRatio(AR_NONANAMORPHIC, 1778);
            ShowText(hWnd, "1.78:1 Letterbox Signal");
        }
		break;
	case IDM_ASPECT_ANAMORPHIC:
        if (aspectSettings.AutoDetectAspect)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
    		SwitchToRatio(AR_ANAMORPHIC, -1);
            ShowText(hWnd, "Anamorphic Signal");
            aspectSettings.DetectAspectNow = TRUE;
        }
        else
        {
    		SwitchToRatio(AR_ANAMORPHIC, 1778);
		    ShowText(hWnd, "1.78:1 Anamorphic Signal");
        }
		break;

	//------------------------------------------------------------------
	// Image position settings
	case IDM_WINPOS_VERT_CENTRE:
	case IDM_WINPOS_VERT_TOP:
	case IDM_WINPOS_VERT_BOTTOM:
		aspectSettings.VerticalPos = (VERT_POS)(wMenuID - IDM_WINPOS_VERT_BOTTOM); 
		WorkoutOverlaySize();
		break;

	case IDM_WINPOS_HORZ_CENTRE:
	case IDM_WINPOS_HORZ_LEFT:
	case IDM_WINPOS_HORZ_RIGHT:
		aspectSettings.HorizontalPos = (HORZ_POS)(wMenuID - IDM_WINPOS_HORZ_RIGHT);
		WorkoutOverlaySize();
		break;

	case IDM_SASPECT_CLIP:
		aspectSettings.aspectImageClipped = !aspectSettings.aspectImageClipped;
		ShowText(hWnd, aspectSettings.aspectImageClipped ? "Image Clipping ON" : "Image Clipping OFF");
		break;

	case IDM_WINPOS_BOUNCE:
		Bounce_OnChange(!aspectSettings.bounceEnabled);
		ShowText(hWnd, aspectSettings.bounceEnabled ? "Image Bouncing ON" : "Image Bouncing OFF");
		break;

	case IDM_WINPOS_ORBIT:
		Orbit_OnChange(!aspectSettings.orbitEnabled);
		ShowText(hWnd, aspectSettings.orbitEnabled ? "Orbit ON" : "Orbit OFF");
		break;

	case IDM_WINPOS_AUTOSIZE:
		aspectSettings.autoResizeWindow = !aspectSettings.autoResizeWindow;
		ShowText(hWnd, aspectSettings.autoResizeWindow ? "Auto-Size Window ON" : "Auto-Size Window OFF");
		break;

	case IDM_ASPECT_DEFER_OVERLAY:
		aspectSettings.deferedSetOverlay = !aspectSettings.deferedSetOverlay;
		break;


	//-----------------------------------------------------------------
	// Autodetect aspect ratio toggles
	case IDM_SASPECT_AUTO_ON:
		aspectSettings.AutoDetectAspect = TRUE;
		ShowText(hWnd, "Auto Aspect Detect ON");
        break;
	case IDM_SASPECT_AUTO_OFF:
		aspectSettings.AutoDetectAspect = FALSE;
		ShowText(hWnd, "Auto Aspect Detect OFF");
        break;
	case IDM_SASPECT_AUTO_TOGGLE:
		aspectSettings.AutoDetectAspect = !aspectSettings.AutoDetectAspect;
		if (aspectSettings.AutoDetectAspect)
		{
			ShowText(hWnd, "Auto Aspect Detect ON");
		}
		else
		{
			ShowText(hWnd, "Auto Aspect Detect OFF");
		}
		break;


	//------------------------------------------------------------------
	// Output Display Aspect Ratios
	case IDM_TASPECT_0:
		aspectSettings.target_aspect = 0;
		ShowText(hWnd, "Aspect Ratio From Current Resolution");
		break;
	case IDM_TASPECT_133:
		aspectSettings.target_aspect = 1333;
		ShowText(hWnd, "1.33:1 Screen");
		break;
	case IDM_TASPECT_166:
		aspectSettings.target_aspect = 1667;
		ShowText(hWnd, "1.66:1 Screen");
		break;
	case IDM_TASPECT_178:
		aspectSettings.target_aspect = 1778;
		ShowText(hWnd, "1.78:1 Screen");
		break;
	case IDM_TASPECT_185:
		aspectSettings.target_aspect = 1850;
		ShowText(hWnd, "1.85:1 Screen");
		break;
	case IDM_TASPECT_200:
		aspectSettings.target_aspect = 2000;
		ShowText(hWnd, "2.00:1 Screen");
		break;
	case IDM_TASPECT_235:
		aspectSettings.target_aspect = 2350;
		ShowText(hWnd, "2.35:1 Screen");
		break;
	case IDM_TASPECT_CUSTOM:
		aspectSettings.target_aspect = aspectSettings.custom_target_aspect;
		ShowText(hWnd, "Custom Aspect Ratio Screen");
		break;

	// Manually-triggered one-time automatic detect of aspect ratio
	case IDM_SASPECT_COMPUTE:
		aspectSettings.DetectAspectNow = TRUE;
		break;

	//------------------------------------------------------------------
	// Zoom
	case IDM_ZOOM_MINUS:
		aspectSettings.xZoomFactor -= 50;
		if (aspectSettings.xZoomFactor < 10)
			aspectSettings.xZoomFactor = 10;
		aspectSettings.yZoomFactor -= 50;
		if (aspectSettings.yZoomFactor < 10)
			aspectSettings.yZoomFactor = 10;
		if ((aspectSettings.xZoomFactor == 100) && (aspectSettings.yZoomFactor == 100))
		{
			ShowText(hWnd,"Zoom Off");
		}
		else
		{
			sprintf(Text,"Zoom %.1fx", (double)aspectSettings.xZoomFactor / 100.0);
			ShowText(hWnd, Text);
		}
		break;

	case IDM_ZOOM_PLUS:
		aspectSettings.xZoomFactor += 50;
		if (aspectSettings.xZoomFactor > 1000)
			aspectSettings.xZoomFactor = 1000;
		aspectSettings.yZoomFactor += 50;
		if (aspectSettings.yZoomFactor > 1000)
			aspectSettings.yZoomFactor = 1000;
		sprintf(Text,"Zoom %.1fx", (double)aspectSettings.xZoomFactor / 100.0);
		ShowText(hWnd, Text);
		break;

	case IDM_ZOOM_10:
		aspectSettings.xZoomFactor = aspectSettings.yZoomFactor = 100;
		ShowText(hWnd,"Zoom Off");
		break;

	case IDM_ZOOM_15:
		aspectSettings.xZoomFactor = aspectSettings.yZoomFactor = 150;
		ShowText(hWnd,"Zoom 1.5x");
		break;

	case IDM_ZOOM_20:
		aspectSettings.xZoomFactor = aspectSettings.yZoomFactor = 200;
		ShowText(hWnd,"Zoom 2x");
		break;

	case IDM_ZOOM_25:
		aspectSettings.xZoomFactor = aspectSettings.yZoomFactor = 250;
		ShowText(hWnd,"Zoom 2.5x");
		break;

	case IDM_ZOOM_30:
		aspectSettings.xZoomFactor = aspectSettings.yZoomFactor = 300;
		ShowText(hWnd,"Zoom 3x");
		break;

	case IDM_ZOOM_35:
		aspectSettings.xZoomFactor = aspectSettings.yZoomFactor = 350;
		ShowText(hWnd,"Zoom 3.5x");
		break;

	case IDM_ZOOM_40:
		aspectSettings.xZoomFactor = aspectSettings.yZoomFactor = 400;
		ShowText(hWnd,"Zoom 4x");
		break;

	case IDM_VT_PAGE_MINUS:
		aspectSettings.xZoomCenter -= 5;
		if (aspectSettings.xZoomCenter < -100)
        {
            aspectSettings.xZoomCenter = -100;
        }
		sprintf(Text,"X Center %.2fx", (double)aspectSettings.xZoomCenter / 100.0);
		ShowText(hWnd, Text);
		break;

	case IDM_VT_PAGE_PLUS:
		aspectSettings.xZoomCenter += 5;
		if (aspectSettings.xZoomCenter > 200)
        {
            aspectSettings.xZoomCenter = 200;
        }
		sprintf(Text,"X Center %.2fx", (double)aspectSettings.xZoomCenter / 100.0);
		ShowText(hWnd, Text);
		break;

	case IDM_VT_PAGE_DOWN:
		aspectSettings.yZoomCenter += 5;
		if (aspectSettings.yZoomCenter > 200)
		{
			aspectSettings.yZoomCenter = 200;
		}
		sprintf(Text,"Y Center %.2fx", (double)aspectSettings.yZoomCenter / 100.0);
		ShowText(hWnd, Text);
		break;

	case IDM_VT_PAGE_UP:
		aspectSettings.yZoomCenter -= 5;
		if (aspectSettings.yZoomCenter < -100)
        {
            aspectSettings.yZoomCenter = 0100;
        }
		sprintf(Text,"Y Center %.2fx", (double)aspectSettings.yZoomCenter / 100.0);
		ShowText(hWnd, Text);
		break;

	//------------------------------------------------------------------
	default:
		// At this point, we want to reset the automatic aspect 
		// because the end user selected an "Advanced Source Aspect Ratio"
		// In this case, turn off automatic aspect ratio detect.
		// Then restart the 'switch' statement.

		aspectSettings.AutoDetectAspect = FALSE;

		//--------------------------------------------------------------
		// Advanced Source Aspect Ratios
		switch (wMenuID) {
		case IDM_SASPECT_0:
			SwitchToRatio(AR_STRETCH, 0);
			ShowText(hWnd, "Stretch Video");
			break;
		case IDM_SASPECT_133:
			SwitchToRatio(AR_NONANAMORPHIC, 1333);
			ShowText(hWnd, "4:3 Fullscreen Signal");
			break;
		case IDM_SASPECT_144:
			SwitchToRatio(AR_NONANAMORPHIC, 1444);
			ShowText(hWnd, "1.44:1 Letterbox Signal");
			break;
		case IDM_SASPECT_155:
			SwitchToRatio(AR_NONANAMORPHIC, 1555);
			ShowText(hWnd, "1.55:1 Letterbox Signal");
			break;
		case IDM_SASPECT_166:
			SwitchToRatio(AR_NONANAMORPHIC, 1667);
			ShowText(hWnd, "1.66:1 Letterbox Signal");
			break;
		case IDM_SASPECT_178:
			SwitchToRatio(AR_NONANAMORPHIC, 1778);
			ShowText(hWnd, "1.78:1 Letterbox Signal");
			break;
		case IDM_SASPECT_185:
			SwitchToRatio(AR_NONANAMORPHIC, 1850);
			ShowText(hWnd, "1.85:1 Letterbox Signal");
			break;
		case IDM_SASPECT_200:
			SwitchToRatio(AR_NONANAMORPHIC, 2000);
			ShowText(hWnd, "2.00:1 Letterbox Signal");
			break;
		case IDM_SASPECT_235:
			SwitchToRatio(AR_NONANAMORPHIC, 2350);
			ShowText(hWnd, "2.35:1 Letterbox Signal");
			break;
		case IDM_SASPECT_166A:
			SwitchToRatio(AR_ANAMORPHIC, 1667);
			ShowText(hWnd, "1.66:1 Anamorphic Signal");
			break;
		case IDM_SASPECT_178A:
			SwitchToRatio(AR_ANAMORPHIC, 1778);
			ShowText(hWnd, "1.78:1 Anamorphic Signal");
			break;
		case IDM_SASPECT_185A:
			SwitchToRatio(AR_ANAMORPHIC, 1850);
			ShowText(hWnd, "1.85:1 Anamorphic Signal");
			break;
		case IDM_SASPECT_200A:
			SwitchToRatio(AR_ANAMORPHIC, 2000);
			ShowText(hWnd, "2.00:1 Anamorphic Signal");
			break;
		case IDM_SASPECT_235A:
			SwitchToRatio(AR_ANAMORPHIC, 2350);
			ShowText(hWnd, "2.35:1 Anamorphic Signal");
			break;
		case IDM_SASPECT_CUSTOM:
			SwitchToRatio(AR_ANAMORPHIC, aspectSettings.custom_source_aspect);
			ShowText(hWnd, "Custom Aspect Ratio Signal");
			break;
		default:
			// It's not an aspect ratio related menu selection
			return FALSE;
		}
	}

    WorkoutOverlaySize();
	if (GetMenu(hWnd) != NULL)
		AspectRatio_SetMenu(GetMenu(hWnd));

	// Yes, we processed the menu selection.
	return TRUE;
}

//----------------------------------------------------------------------------
// Repaints the overlay colorkey, optionally with black borders around it
// during aspect ratio control
extern LPDIRECTDRAW lpDD; // Temporary expierement MRS 2-22-01

void PaintColorkey(HWND hWnd, BOOL bEnable, HDC hDC, RECT* PaintRect)
{
	// MRS 9-9-00
	HBRUSH black = CreateSolidBrush(RGB(0,0,0));
	HBRUSH overlay;
	RECT r;
	RECT r2, winRect;

	if (bEnable && OverlayActive())
	{
		overlay = CreateSolidBrush(GetNearestColor(hDC, Overlay_GetColor()));
	}
	else
	{
		overlay = CreateSolidBrush(RGB(0,0,0));
	}

	// MRS 2-22-01 - Reworked to fixup the rectangle rather than not draw piecemeal to help out defered overlay setting
	if (aspectSettings.destinationRectangle.right < aspectSettings.destinationRectangle.left) {
		int t = aspectSettings.destinationRectangle.right;
		aspectSettings.destinationRectangle.right = aspectSettings.destinationRectangle.left;
		aspectSettings.destinationRectangle.left = t;
	}
	if (aspectSettings.destinationRectangle.bottom < aspectSettings.destinationRectangle.top) {
		int t = aspectSettings.destinationRectangle.top;
		aspectSettings.destinationRectangle.top = aspectSettings.destinationRectangle.bottom;
		aspectSettings.destinationRectangle.bottom = t;
	}
	
	// Draw black in the 4 borders
	GetClientRect(hWnd,&winRect);
	
	//TJ 010508 make sure we dont paint over the statusbar
	//if we do, it will cause flashing
	if(IsStatusBarVisible())
		winRect.bottom-=StatusBar_Height();

	// Top
	r2.left = 0;
	r2.top = 0;
	r2.right = winRect.right;
	r2.bottom = aspectSettings.destinationRectangle.top;
	IntersectRect(&r, &r2, PaintRect);
	FillRect(hDC, &r, black);

	// Bottom
	r2.left = 0;
	r2.top = aspectSettings.destinationRectangle.bottom;
	r2.right = winRect.right;
	r2.bottom = winRect.bottom;
	IntersectRect(&r, &r2, PaintRect);
	FillRect(hDC, &r, black);

	// Left
	r2.left = 0;
	r2.top = 0;
	r2.right = aspectSettings.destinationRectangle.left;
	r2.bottom = winRect.bottom;
	IntersectRect(&r, &r2, PaintRect);
	FillRect(hDC, &r, black);

	// Right
	r2.left = aspectSettings.destinationRectangle.right;
	r2.top = 0;
	r2.right = winRect.right;
	r2.bottom = winRect.bottom;
	IntersectRect(&r, &r2, PaintRect);
	FillRect(hDC, &r, black);

	if (aspectSettings.overlayNeedsSetting)
    { 
        // MRS 2-22-01
		// Intended to prevent purple flashing by setting overlay
		// after drawing black but before drawing purple.
		Overlay_Update(&aspectSettings.sourceRectangle, &aspectSettings.destinationRectangleWindow, DDOVER_SHOW);
		aspectSettings.overlayNeedsSetting = FALSE;
		// Wait till current frame is done before drawing purple...
		// Overlay changes do not seem to take place (at least on a GeForce)
		// until the VBI...so need to wait to avoid purple flashing

        // JA 29/6/2001 made wait for blank optional and swicthed off
        // the flashing has been much reduced by using dark grey as
        // overlay colour.  Also this may cause the pausing effect
        // on Teletext and CC
		if (lpDD != NULL && aspectSettings.bWaitForVerticalBlank == TRUE)
        {
            lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
        }
	}

	// Draw overlay color in the middle.
	IntersectRect(&r, &aspectSettings.destinationRectangle, PaintRect);
	FillRect(hDC, &r, overlay);

	DeleteObject(black);
	DeleteObject(overlay);
}

//----------------------------------------------------------------------------
// Aspect ratio API

const char* HorzPosString[3] =
{
	"RIGHT", "CENTER", "LEFT"
};

const char* VertPosString[3] =
{
	"BOTTOM", "CENTER", "TOP"
};

BOOL Aspect_Overscan_OnChange(long Overscan)
{
	aspectSettings.InitialOverscan = Overscan;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL AspectMode_OnChange(long NewValue)
{
	aspectSettings.aspect_mode = NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL TargetAspect_OnChange(long NewValue)
{
	aspectSettings.target_aspect = NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL CustomTargetAspect_OnChange(long NewValue)
{
	aspectSettings.target_aspect = NewValue;
	aspectSettings.custom_target_aspect = NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL SourceAspect_OnChange(long NewValue)
{
	aspectSettings.AutoDetectAspect = FALSE;
	aspectSettings.source_aspect = NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL CustomSourceAspect_OnChange(long NewValue)
{
	aspectSettings.AutoDetectAspect = FALSE;
	aspectSettings.custom_source_aspect = NewValue;
	aspectSettings.source_aspect = NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL HorizPos_OnChange(long NewValue)
{
	aspectSettings.HorizontalPos = (HORZ_POS)NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL VertPos_OnChange(long NewValue)
{
	aspectSettings.VerticalPos = (VERT_POS)NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL Bounce_OnChange(long NewValue)
{
	aspectSettings.bounceEnabled = NewValue != 0;
	if (!aspectSettings.bounceEnabled)
    {
		KillTimer(hWnd, TIMER_BOUNCE);
	}
    else
    {
	    SetTimer(hWnd, TIMER_BOUNCE, aspectSettings.timerBounceMS, NULL);
	}
	return FALSE;
}

BOOL Orbit_OnChange(long NewValue) 
{
	aspectSettings.orbitEnabled = NewValue != 0;
	if (!aspectSettings.orbitEnabled) 
    {
        KillTimer(hWnd, TIMER_ORBIT);
	}
    else
    {
	    SetTimer(hWnd, TIMER_ORBIT, aspectSettings.timerOrbitMS, NULL);
	}
	return FALSE;
}

BOOL Clipping_OnChange(long NewValue)
{
	aspectSettings.aspectImageClipped = (BOOL)NewValue;
	WorkoutOverlaySize();
	return FALSE;
}

BOOL XZoom_Factor_OnChange(long NewValue)
{
	aspectSettings.xZoomFactor = NewValue;	
    WorkoutOverlaySize();
	return FALSE;
}

BOOL YZoom_Factor_OnChange(long NewValue)
{
	aspectSettings.yZoomFactor = NewValue;	
    WorkoutOverlaySize();
	return FALSE;
}

BOOL XZoom_Center_OnChange(long NewValue)
{
	aspectSettings.xZoomCenter = NewValue;	
    WorkoutOverlaySize();
	return FALSE;
}

BOOL YZoom_Center_OnChange(long NewValue)
{
	aspectSettings.yZoomCenter = NewValue;	
    WorkoutOverlaySize();
	return FALSE;
}

BOOL ChromaRange_OnChange(long NewValue)
{
	aspectSettings.ChromaRange = NewValue;	
    WorkoutOverlaySize();
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING AspectSettings[ASPECT_SETTING_LASTONE] =
{
	{
		"Overscan", SLIDER, 0, (long*)&aspectSettings.InitialOverscan,
		DEFAULT_OVERSCAN_NTSC, 0, 150, 1, 1,
		NULL,
		"Hardware", "InitialOverscan", Aspect_Overscan_OnChange,
	},
	{
		"Source Aspect", SLIDER, 0, (long*)&aspectSettings.source_aspect,
		1333, 1000, 3000, 1, 1000,
		NULL,
		"ASPECT", "SourceAspect", SourceAspect_OnChange,
	},
	{
		"Custom Source Aspect", SLIDER, 0, (long*)&aspectSettings.custom_source_aspect,
		1335, 1000, 3000, 5, 1000,
		NULL,
		"ASPECT", "CustomSourceAspect", CustomSourceAspect_OnChange,
	},
	{
		"Screen Aspect", SLIDER, 0, (long*)&aspectSettings.target_aspect,
		1333, 0, 3000, 1, 1000,
		NULL,
		"ASPECT", "TargetAspect", TargetAspect_OnChange,
	},
	{
		"Custom Screen Aspect", SLIDER, 0, (long*)&aspectSettings.custom_target_aspect,
		1335, 1000, 3000, 5, 1000,
		NULL,
		"ASPECT", "CustomTargetAspect", CustomTargetAspect_OnChange,
	},
	{
		"Aspect Mode", SLIDER, 0, (long*)&aspectSettings.aspect_mode,
		1, 0, 2, 1, 1,
		NULL,
		"ASPECT", "Mode", AspectMode_OnChange,
	},
	{
		"Auto Detect Aspect Sensitivity", SLIDER, 0, (long*)&aspectSettings.LuminanceThreshold,
		40, 0, 255, 1, 1,
		NULL,
		"ASPECT", "LuminanceThreshold", NULL,
	},
	{
		"Ignore Non-Black Pixels", SLIDER, 0, (long*)&aspectSettings.IgnoreNonBlackPixels,
		3, 0, 750, 1, 1,
		NULL,
		"ASPECT", "IgnoreNonBlackPixels", NULL,
	},
	{
		"Auto Detect Aspect", ONOFF, 0, (long*)&aspectSettings.AutoDetectAspect,
		FALSE, 0, 1, 1, 1,
		NULL,
		"ASPECT", "AutoDetectAspect", NULL,
	},
	{
		"Zoom In Frame Count", SLIDER, 0, (long*)&aspectSettings.ZoomInFrameCount,
		60, 0, 1000, 10, 1,
		NULL,
		"ASPECT", "ZoomInFrameCount", NULL,
	},
	{
		"Aspect History Time", SLIDER, 0, (long*)&aspectSettings.AspectHistoryTime,
		300, 0, 3000, 10, 1,
		NULL,
		"ASPECT", "AspectHistoryTime", NULL,
	},
	{
		"AspectConsistencyTime", SLIDER, 0, (long*)&aspectSettings.AspectConsistencyTime,
		15, 0, 300, 5, 1,
		NULL,
		"ASPECT", "AspectConsistencyTime", NULL,
	},
	{
		"Vert Image Pos", ITEMFROMLIST, 0, (long*)&aspectSettings.VerticalPos,
		VERT_POS_CENTRE, 0, 2, 1, 1,
		VertPosString,
		"ASPECT", "VerticalPos", VertPos_OnChange,
	},
	{
		"Horiz Image Pos", ITEMFROMLIST, 0, (long*)&aspectSettings.HorizontalPos,
		HORZ_POS_CENTRE, 0, 2, 1, 1,
		HorzPosString,
		"ASPECT", "HorizontalPos", HorizPos_OnChange,
	},
	{
		"Clipping", ONOFF, 0, (long*)&aspectSettings.aspectImageClipped,
		TRUE, 0, 1, 1, 1,
		NULL,
		"ASPECT", "Clipping", Clipping_OnChange,
	},
	{
		"Bounce", ONOFF, 0, (long*)&aspectSettings.bounceEnabled,
		FALSE, 0, 1, 1, 1,
		NULL,
		"ASPECT", "Bounce", Bounce_OnChange,
	},
	{
		"Bounce Period", SLIDER, 0, (long*)&aspectSettings.bouncePeriod,
		60*30, 0, 2, 1, 1,
		NULL,
		"ASPECT", "BouncePeriod", NULL,
	},
	{
		"Defer Setting Overlay", ONOFF, 0, (long*)&aspectSettings.deferedSetOverlay,
		TRUE, 0, 1, 1, 1,
		NULL,
		"ASPECT", "deferedOverlay", NULL,
	},
	{
		"Bounce Timer Period (ms)", SLIDER, 0, (long*)&aspectSettings.timerBounceMS,
		1000, 0, 2, 1, 1,
		NULL,
		"ASPECT", "BounceTimerPeriod", NULL,
	},
	{
		"Bounce Amplitude (%)", SLIDER, 0, (long*)&aspectSettings.bounceAmplitude,
		100, 0, 100, 1, 1,
		NULL,
		"ASPECT", "BounceAmplitude", NULL,
	},
	{
		"Orbit", ONOFF, 0, (long*)&aspectSettings.orbitEnabled,
		FALSE, 0, 1, 1, 1,
		NULL,
		"ASPECT", "Orbit", Orbit_OnChange,
	},
	{
		"Orbit Period X", SLIDER, 0, (long*)&aspectSettings.orbitPeriodX,
		60*45, 0, 2, 1, 1,
		NULL,
		"ASPECT", "OrbitPeriodX", NULL,
	},
	{
		"Orbit Period Y", SLIDER, 0, (long*)&aspectSettings.orbitPeriodY,
		60*60, 0, 2, 1, 1,
		NULL,
		"ASPECT", "OrbitPeriodY", NULL,
	},
	{
		"Orbit Size", SLIDER, 0, (long*)&aspectSettings.orbitSize,
		8, 0, 2, 1, 1,
		NULL,
		"ASPECT", "OrbitSize", NULL,
	},
	{
		"Orbit Timer Period (ms)", SLIDER, 0, (long*)&aspectSettings.timerOrbitMS,
		60000, 0, 2, 1, 1,
		NULL,
		"ASPECT", "OrbitTimerPeriod", NULL,
	},
	{
		"Auto-Size Window", ONOFF, 0, (long*)&aspectSettings.autoResizeWindow,
		FALSE, 0, 1, 1, 1,
		NULL,
		"ASPECT", "AutoSizeWindow", NULL,
	},
	{
		"Skip Percentage", SLIDER, 0, (long*)&aspectSettings.SkipPercent,
		17, 0, 49, 1, 1,
		NULL,
		"ASPECT", "SkipPercent", NULL,
	},
	{
		"X Zoom Factor", SLIDER, 0, &aspectSettings.xZoomFactor,
		100, 1, 1000, 10, 100,
		NULL,
		"ASPECT", "XZoomFactor", XZoom_Factor_OnChange,
	},
	{
		"Y Zoom Factor", SLIDER, 0, &aspectSettings.yZoomFactor,
		100, 1, 1000, 10, 100,
		NULL,
		"ASPECT", "YZoomFactor", YZoom_Factor_OnChange,
	},
	{
		"X Zoom Center", SLIDER, 0, &aspectSettings.xZoomCenter,
		50, -100, 200, 5, 100,
		NULL,
		"ASPECT", "XZoomCenter", XZoom_Center_OnChange,
	},
	{
		"Y Zoom Center", SLIDER, 0, &aspectSettings.yZoomCenter,
		50, -100, 200, 5, 100,
		NULL,
		"ASPECT", "YZoomCenter", YZoom_Center_OnChange,
	},
	{
		"Chroma Range", SLIDER, 0, &aspectSettings.ChromaRange,
		16, 0, 255, 1, 1,
		NULL,
		"ASPECT", "ChromaRange", ChromaRange_OnChange,
	},
	{
		"Wait for Vertical Blank While Drawing", ONOFF, 0, (long*)&aspectSettings.bWaitForVerticalBlank,
		FALSE, 0, 1, 1, 1,
		NULL,
		"ASPECT", "WaitForVerticalBlank", NULL,
	},
};

SETTING* Aspect_GetSetting(ASPECT_SETTING Setting)
{
	if(Setting > -1 && Setting < ASPECT_SETTING_LASTONE)
	{
		return &(AspectSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void Aspect_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < ASPECT_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(AspectSettings[i]));
	}
}

void Aspect_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < ASPECT_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(AspectSettings[i]));
	}
}

void Aspect_FinalSetup()
{
	Bounce_OnChange(aspectSettings.bounceEnabled);
	Orbit_OnChange(aspectSettings.orbitEnabled);
}

void Aspect_ShowUI()
{
	CSettingsDlg::ShowSettingsDlg("Aspect Ratio Settings",AspectSettings, ASPECT_SETTING_LASTONE);
}
