/////////////////////////////////////////////////////////////////////////////
// $Id: AspectGUI.cpp,v 1.26 2001-10-18 16:24:07 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Michael Samblanet  All rights reserved.
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
//                                     Split out to improve redability
//
// 08 Jun 2001   Eric Schmidt          Added bounce amplitude to ini
//
//////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.25  2001/10/18 16:20:40  adcockj
// Made Color of blanking adjustable
//
// Revision 1.24  2001/09/05 21:05:29  adcockj
// Bug Fixes for new overlay code
//
// Revision 1.23  2001/09/03 13:06:49  adcockj
// Increment version
//
// Revision 1.22  2001/08/06 22:32:13  laurentg
// Little improvments for AR autodetection
//
// Revision 1.21  2001/08/03 09:52:42  adcockj
// Added range checking on settings and fixed setting with out of range errors
//
// Revision 1.20  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.19  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.18  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
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
    CheckMenuItem(hMenu, IDM_ASPECT_LETTERBOX, MF_UNCHECKED);
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

    if(AspectSettings.AutoDetectAspect)
    {
        CheckMenuItem(hMenu, IDM_SASPECT_AUTO_TOGGLE, MF_CHECKED);
    }
    else if (AspectSettings.AspectMode == 1)
    {
        switch (AspectSettings.SourceAspect)
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
    else if (AspectSettings.AspectMode == 2)
    {
        switch (AspectSettings.SourceAspect)
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
    CheckMenuItemBool(hMenu, IDM_SASPECT_0, (AspectSettings.SourceAspect == 0));
    CheckMenuItemBool(hMenu, IDM_SASPECT_CUSTOM, (AspectSettings.SourceAspect && AspectSettings.SourceAspect == AspectSettings.CustomSourceAspect));

    // Advanced Aspect Ratio -> Display Aspect Ratio
    CheckMenuItemBool(hMenu, IDM_TASPECT_0, (AspectSettings.TargetAspect == 0));
    CheckMenuItemBool(hMenu, IDM_TASPECT_133, (AspectSettings.TargetAspect == 1333));
    CheckMenuItemBool(hMenu, IDM_TASPECT_166, (AspectSettings.TargetAspect == 1667));
    CheckMenuItemBool(hMenu, IDM_TASPECT_178, (AspectSettings.TargetAspect == 1778));
    CheckMenuItemBool(hMenu, IDM_TASPECT_185, (AspectSettings.TargetAspect == 1850));
    CheckMenuItemBool(hMenu, IDM_TASPECT_200, (AspectSettings.TargetAspect == 2000));
    CheckMenuItemBool(hMenu, IDM_TASPECT_235, (AspectSettings.TargetAspect == 2350));
    CheckMenuItemBool(hMenu, IDM_TASPECT_CUSTOM, (AspectSettings.TargetAspect && AspectSettings.TargetAspect == AspectSettings.custom_target_aspect));

    CheckMenuItemBool(hMenu, IDM_WINPOS_VERT_CENTRE, (AspectSettings.VerticalPos == VERT_POS_CENTRE));
    CheckMenuItemBool(hMenu, IDM_WINPOS_VERT_TOP, (AspectSettings.VerticalPos == VERT_POS_TOP));
    CheckMenuItemBool(hMenu, IDM_WINPOS_VERT_BOTTOM, (AspectSettings.VerticalPos == VERT_POS_BOTTOM));

    CheckMenuItemBool(hMenu, IDM_WINPOS_HORZ_CENTRE, (AspectSettings.HorizontalPos == HORZ_POS_CENTRE));
    CheckMenuItemBool(hMenu, IDM_WINPOS_HORZ_LEFT, (AspectSettings.HorizontalPos == HORZ_POS_LEFT));
    CheckMenuItemBool(hMenu, IDM_WINPOS_HORZ_RIGHT, (AspectSettings.HorizontalPos == HORZ_POS_RIGHT));

    CheckMenuItemBool(hMenu, IDM_SASPECT_CLIP, (AspectSettings.AspectImageClipped));
    CheckMenuItemBool(hMenu, IDM_WINPOS_BOUNCE, (AspectSettings.BounceEnabled));
    CheckMenuItemBool(hMenu, IDM_WINPOS_ORBIT, (AspectSettings.OrbitEnabled));
    CheckMenuItemBool(hMenu, IDM_ASPECT_DEFER_OVERLAY, (AspectSettings.DeferedSetOverlay));
    CheckMenuItemBool(hMenu, IDM_WINPOS_AUTOSIZE, (AspectSettings.AutoResizeWindow));

    // Zoom
    CheckMenuItemBool(hMenu, IDM_ZOOM_10, (AspectSettings.ZoomFactorX == 100 && AspectSettings.ZoomFactorY == 100));
    CheckMenuItemBool(hMenu, IDM_ZOOM_15, (AspectSettings.ZoomFactorX == 150 && AspectSettings.ZoomFactorY == 150));
    CheckMenuItemBool(hMenu, IDM_ZOOM_20, (AspectSettings.ZoomFactorX == 200 && AspectSettings.ZoomFactorY == 200));
    CheckMenuItemBool(hMenu, IDM_ZOOM_25, (AspectSettings.ZoomFactorX == 250 && AspectSettings.ZoomFactorY == 250));
    CheckMenuItemBool(hMenu, IDM_ZOOM_30, (AspectSettings.ZoomFactorX == 300 && AspectSettings.ZoomFactorY == 300));
    CheckMenuItemBool(hMenu, IDM_ZOOM_35, (AspectSettings.ZoomFactorX == 350 && AspectSettings.ZoomFactorY == 350));
    CheckMenuItemBool(hMenu, IDM_ZOOM_40, (AspectSettings.ZoomFactorX == 400 && AspectSettings.ZoomFactorY == 400));

}

//----------------------------------------------------------------------------
// MENU PROCESSING:
// Processing aspect ratio control related menu selection
// during aspect ratio control
BOOL ProcessAspectRatioSelection(HWND hWnd, WORD wMenuID)
{
    char Text[32];

    switch (wMenuID)
    {
    //------------------------------------------------------------------
    // Easily Accessible Aspect Ratios
    case IDM_ASPECT_FULLSCREEN:
        if (AspectSettings.AutoDetectAspect)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            // This applies to both letterbox and 4:3
            SwitchToRatio(AR_NONANAMORPHIC, -1);
            ShowText(hWnd, "Nonanamorphic Signal");
            AspectSettings.DetectAspectNow = TRUE;
        }
        else
        {
            SwitchToRatio(AR_NONANAMORPHIC, 1333);
            ShowText(hWnd, "4:3 Fullscreen Signal");
        }
        break;
    case IDM_ASPECT_LETTERBOX:
        if (AspectSettings.AutoDetectAspect)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            // This applies to both letterbox and 4:3
            SwitchToRatio(AR_NONANAMORPHIC, -1);
            ShowText(hWnd, "Nonanamorphic Signal");
            AspectSettings.DetectAspectNow = TRUE;
        }
        else
        {
            SwitchToRatio(AR_NONANAMORPHIC, 1778);
            ShowText(hWnd, "1.78:1 Letterbox Signal");
        }
        break;
    case IDM_ASPECT_ANAMORPHIC:
        if (AspectSettings.AutoDetectAspect)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            SwitchToRatio(AR_ANAMORPHIC, -1);
            ShowText(hWnd, "Anamorphic Signal");
            AspectSettings.DetectAspectNow = TRUE;
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
        AspectSettings.VerticalPos = (eVertPos)(wMenuID - IDM_WINPOS_VERT_BOTTOM); 
        WorkoutOverlaySize();
        break;

    case IDM_WINPOS_HORZ_CENTRE:
    case IDM_WINPOS_HORZ_LEFT:
    case IDM_WINPOS_HORZ_RIGHT:
        AspectSettings.HorizontalPos = (eHorzPos)(wMenuID - IDM_WINPOS_HORZ_RIGHT);
        WorkoutOverlaySize();
        break;

    case IDM_SASPECT_CLIP:
        AspectSettings.AspectImageClipped = !AspectSettings.AspectImageClipped;
        ShowText(hWnd, AspectSettings.AspectImageClipped ? "Image Clipping ON" : "Image Clipping OFF");
        break;

    case IDM_WINPOS_BOUNCE:
        Bounce_OnChange(!AspectSettings.BounceEnabled);
        ShowText(hWnd, AspectSettings.BounceEnabled ? "Image Bouncing ON" : "Image Bouncing OFF");
        break;

    case IDM_WINPOS_ORBIT:
        Orbit_OnChange(!AspectSettings.OrbitEnabled);
        ShowText(hWnd, AspectSettings.OrbitEnabled ? "Orbit ON" : "Orbit OFF");
        break;

    case IDM_WINPOS_AUTOSIZE:
        AspectSettings.AutoResizeWindow = !AspectSettings.AutoResizeWindow;
        ShowText(hWnd, AspectSettings.AutoResizeWindow ? "Auto-Size Window ON" : "Auto-Size Window OFF");
        break;

    case IDM_ASPECT_DEFER_OVERLAY:
        AspectSettings.DeferedSetOverlay = !AspectSettings.DeferedSetOverlay;
        break;


    //-----------------------------------------------------------------
    // Autodetect aspect ratio toggles
    case IDM_SASPECT_AUTO_ON:
        AspectSettings.AutoDetectAspect = TRUE;
        ShowText(hWnd, "Auto Aspect Detect ON");
        break;
    case IDM_SASPECT_AUTO_OFF:
        AspectSettings.AutoDetectAspect = FALSE;
        ShowText(hWnd, "Auto Aspect Detect OFF");
        break;
    case IDM_SASPECT_AUTO_TOGGLE:
        AspectSettings.AutoDetectAspect = !AspectSettings.AutoDetectAspect;
        if (AspectSettings.AutoDetectAspect)
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
        AspectSettings.TargetAspect = 0;
        ShowText(hWnd, "Aspect Ratio From Current Resolution");
        break;
    case IDM_TASPECT_133:
        AspectSettings.TargetAspect = 1333;
        ShowText(hWnd, "1.33:1 Screen");
        break;
    case IDM_TASPECT_166:
        AspectSettings.TargetAspect = 1667;
        ShowText(hWnd, "1.66:1 Screen");
        break;
    case IDM_TASPECT_178:
        AspectSettings.TargetAspect = 1778;
        ShowText(hWnd, "1.78:1 Screen");
        break;
    case IDM_TASPECT_185:
        AspectSettings.TargetAspect = 1850;
        ShowText(hWnd, "1.85:1 Screen");
        break;
    case IDM_TASPECT_200:
        AspectSettings.TargetAspect = 2000;
        ShowText(hWnd, "2.00:1 Screen");
        break;
    case IDM_TASPECT_235:
        AspectSettings.TargetAspect = 2350;
        ShowText(hWnd, "2.35:1 Screen");
        break;
    case IDM_TASPECT_CUSTOM:
        AspectSettings.TargetAspect = AspectSettings.custom_target_aspect;
        ShowText(hWnd, "Custom Aspect Ratio Screen");
        break;

    // Manually-triggered one-time automatic detect of aspect ratio
    case IDM_SASPECT_COMPUTE:
        AspectSettings.DetectAspectNow = TRUE;
        break;

    //------------------------------------------------------------------
    // Zoom
    case IDM_ZOOM_MINUS:
        AspectSettings.ZoomFactorX -= 50;
        if (AspectSettings.ZoomFactorX < 50)
        {
            AspectSettings.ZoomFactorX = 25;
        }
        AspectSettings.ZoomFactorY -= 50;
        if (AspectSettings.ZoomFactorY < 50)
        {
            AspectSettings.ZoomFactorY = 50;
        }
        if ((AspectSettings.ZoomFactorX == 100) && (AspectSettings.ZoomFactorY == 100))
        {
            ShowText(hWnd,"Zoom Off");
        }
        else
        {
            sprintf(Text,"Zoom %.1fx", (double)AspectSettings.ZoomFactorX / 100.0);
            ShowText(hWnd, Text);
        }
        break;

    case IDM_ZOOM_PLUS:
        if(AspectSettings.ZoomFactorX >= 50)
        {
            AspectSettings.ZoomFactorX += 50;
        }
        else
        {
            AspectSettings.ZoomFactorX = 50;
        }
        if (AspectSettings.ZoomFactorX > 1000)
        {
            AspectSettings.ZoomFactorX = 1000;
        }
        AspectSettings.ZoomFactorY += 50;
        if (AspectSettings.ZoomFactorY > 1000)
        {
            AspectSettings.ZoomFactorY = 1000;
        }
        sprintf(Text,"Zoom %.1fx", (double)AspectSettings.ZoomFactorX / 100.0);
        ShowText(hWnd, Text);
        break;

    case IDM_ZOOM_10:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 100;
        ShowText(hWnd,"Zoom Off");
        break;

    case IDM_ZOOM_15:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 150;
        ShowText(hWnd,"Zoom 1.5x");
        break;

    case IDM_ZOOM_20:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 200;
        ShowText(hWnd,"Zoom 2x");
        break;

    case IDM_ZOOM_25:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 250;
        ShowText(hWnd,"Zoom 2.5x");
        break;

    case IDM_ZOOM_30:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 300;
        ShowText(hWnd,"Zoom 3x");
        break;

    case IDM_ZOOM_35:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 350;
        ShowText(hWnd,"Zoom 3.5x");
        break;

    case IDM_ZOOM_40:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 400;
        ShowText(hWnd,"Zoom 4x");
        break;

    case IDM_VT_PAGE_MINUS:
        AspectSettings.ZoomCenterX -= 5;
        if (AspectSettings.ZoomCenterX < -100)
        {
            AspectSettings.ZoomCenterX = -100;
        }
        sprintf(Text,"X Center %.2fx", (double)AspectSettings.ZoomCenterX / 100.0);
        ShowText(hWnd, Text);
        break;

    case IDM_VT_PAGE_PLUS:
        AspectSettings.ZoomCenterX += 5;
        if (AspectSettings.ZoomCenterX > 200)
        {
            AspectSettings.ZoomCenterX = 200;
        }
        sprintf(Text,"X Center %.2fx", (double)AspectSettings.ZoomCenterX / 100.0);
        ShowText(hWnd, Text);
        break;

    case IDM_VT_PAGE_DOWN:
        AspectSettings.ZoomCenterY += 5;
        if (AspectSettings.ZoomCenterY > 200)
        {
            AspectSettings.ZoomCenterY = 200;
        }
        sprintf(Text,"Y Center %.2fx", (double)AspectSettings.ZoomCenterY / 100.0);
        ShowText(hWnd, Text);
        break;

    case IDM_VT_PAGE_UP:
        AspectSettings.ZoomCenterY -= 5;
        if (AspectSettings.ZoomCenterY < -100)
        {
            AspectSettings.ZoomCenterY = 100;
        }
        sprintf(Text,"Y Center %.2fx", (double)AspectSettings.ZoomCenterY / 100.0);
        ShowText(hWnd, Text);
        break;

    //------------------------------------------------------------------
    default:
        // At this point, we want to reset the automatic aspect 
        // because the end user selected an "Advanced Source Aspect Ratio"
        // In this case, turn off automatic aspect ratio detect.
        // Then restart the 'switch' statement.

        AspectSettings.AutoDetectAspect = FALSE;

        //--------------------------------------------------------------
        // Advanced Source Aspect Ratios
        switch (wMenuID) 
        {
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
            SwitchToRatio(AR_ANAMORPHIC, AspectSettings.CustomSourceAspect);
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
    HBRUSH Background = CreateSolidBrush(RGB(AspectSettings.MaskGreyShade,
                                        AspectSettings.MaskGreyShade,
                                        AspectSettings.MaskGreyShade));
    HBRUSH Overlay;
    RECT r;
    RECT r2, winRect;

    if (bEnable && OverlayActive())
    {
        Overlay = CreateSolidBrush(GetNearestColor(hDC, Overlay_GetColor()));
    }
    else
    {
        Overlay = CreateSolidBrush(RGB(0,0,0));
    }

    // MRS 2-22-01 - Reworked to fixup the rectangle rather than not draw piecemeal to help out defered overlay setting
    if (AspectSettings.DestinationRect.right < AspectSettings.DestinationRect.left) {
        int t = AspectSettings.DestinationRect.right;
        AspectSettings.DestinationRect.right = AspectSettings.DestinationRect.left;
        AspectSettings.DestinationRect.left = t;
    }
    if (AspectSettings.DestinationRect.bottom < AspectSettings.DestinationRect.top) {
        int t = AspectSettings.DestinationRect.top;
        AspectSettings.DestinationRect.top = AspectSettings.DestinationRect.bottom;
        AspectSettings.DestinationRect.bottom = t;
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
    r2.bottom = AspectSettings.DestinationRect.top;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    // Bottom
    r2.left = 0;
    r2.top = AspectSettings.DestinationRect.bottom;
    r2.right = winRect.right;
    r2.bottom = winRect.bottom;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    // Left
    r2.left = 0;
    r2.top = 0;
    r2.right = AspectSettings.DestinationRect.left;
    r2.bottom = winRect.bottom;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    // Right
    r2.left = AspectSettings.DestinationRect.right;
    r2.top = 0;
    r2.right = winRect.right;
    r2.bottom = winRect.bottom;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    if (AspectSettings.OverlayNeedsSetting)
    { 
        // MRS 2-22-01
        // Intended to prevent purple flashing by setting overlay
        // after drawing black but before drawing purple.
        Overlay_Update(&AspectSettings.SourceRect, &AspectSettings.DestinationRectWindow, DDOVER_SHOW);
        AspectSettings.OverlayNeedsSetting = FALSE;
        // Wait till current frame is done before drawing purple...
        // Overlay changes do not seem to take place (at least on a GeForce)
        // until the VBI...so need to wait to avoid purple flashing

        // JA 29/6/2001 made wait for blank optional and swicthed off
        // the flashing has been much reduced by using dark grey as
        // overlay colour.  Also this may cause the pausing effect
        // on Teletext and CC
        if (lpDD != NULL && AspectSettings.bWaitForVerticalBlank == TRUE)
        {
            lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
        }
    }

    // Draw overlay color in the middle.
    IntersectRect(&r, &AspectSettings.DestinationRect, PaintRect);
    FillRect(hDC, &r, Overlay);

    DeleteObject(Background);
    DeleteObject(Overlay);
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
    AspectSettings.InitialOverscan = Overscan;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL AspectMode_OnChange(long NewValue)
{
    AspectSettings.AspectMode = NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL TargetAspect_OnChange(long NewValue)
{
    AspectSettings.TargetAspect = NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL CustomTargetAspect_OnChange(long NewValue)
{
    AspectSettings.TargetAspect = NewValue;
    AspectSettings.custom_target_aspect = NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL SourceAspect_OnChange(long NewValue)
{
    AspectSettings.AutoDetectAspect = FALSE;
    AspectSettings.SourceAspect = NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL CustomSourceAspect_OnChange(long NewValue)
{
    AspectSettings.AutoDetectAspect = FALSE;
    AspectSettings.CustomSourceAspect = NewValue;
    AspectSettings.SourceAspect = NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL HorizPos_OnChange(long NewValue)
{
    AspectSettings.HorizontalPos = (eHorzPos)NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL VertPos_OnChange(long NewValue)
{
    AspectSettings.VerticalPos = (eVertPos)NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL Bounce_OnChange(long NewValue)
{
    AspectSettings.BounceEnabled = NewValue != 0;
    if (!AspectSettings.BounceEnabled)
    {
        KillTimer(hWnd, TIMER_BOUNCE);
    }
    else
    {
        SetTimer(hWnd, TIMER_BOUNCE, AspectSettings.TimerBounceMS, NULL);
    }
    return FALSE;
}

BOOL Orbit_OnChange(long NewValue) 
{
    AspectSettings.OrbitEnabled = NewValue != 0;
    if (!AspectSettings.OrbitEnabled) 
    {
        KillTimer(hWnd, TIMER_ORBIT);
    }
    else
    {
        SetTimer(hWnd, TIMER_ORBIT, AspectSettings.TimerOrbitMS, NULL);
    }
    return FALSE;
}

BOOL Clipping_OnChange(long NewValue)
{
    AspectSettings.AspectImageClipped = (BOOL)NewValue;
    WorkoutOverlaySize();
    return FALSE;
}

BOOL XZoom_Factor_OnChange(long NewValue)
{
    AspectSettings.ZoomFactorX = NewValue;  
    WorkoutOverlaySize();
    return FALSE;
}

BOOL YZoom_Factor_OnChange(long NewValue)
{
    AspectSettings.ZoomFactorY = NewValue;  
    WorkoutOverlaySize();
    return FALSE;
}

BOOL XZoom_Center_OnChange(long NewValue)
{
    AspectSettings.ZoomCenterX = NewValue;  
    WorkoutOverlaySize();
    return FALSE;
}

BOOL YZoom_Center_OnChange(long NewValue)
{
    AspectSettings.ZoomCenterY = NewValue;  
    WorkoutOverlaySize();
    return FALSE;
}

BOOL ChromaRange_OnChange(long NewValue)
{
    AspectSettings.ChromaRange = NewValue;  
    WorkoutOverlaySize();
    return FALSE;
}

BOOL MaskGreyShade_OnChange(long NewValue)
{
    AspectSettings.MaskGreyShade = NewValue;  
    WorkoutOverlaySize();
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING AspectGUISettings[ASPECT_SETTING_LASTONE] =
{
    {
        "Overscan", SLIDER, 0, (long*)&AspectSettings.InitialOverscan,
        DEFAULT_OVERSCAN_NTSC, 0, 150, 1, 1,
        NULL,
        "Hardware", "InitialOverscan", Aspect_Overscan_OnChange,
    },
    {
        "Source Aspect", SLIDER, 0, (long*)&AspectSettings.SourceAspect,
        1333, 1000, 3000, 1, 1000,
        NULL,
        "ASPECT", "SourceAspect", SourceAspect_OnChange,
    },
    {
        "Custom Source Aspect", SLIDER, 0, (long*)&AspectSettings.CustomSourceAspect,
        1335, 1000, 3000, 5, 1000,
        NULL,
        "ASPECT", "CustomSourceAspect", CustomSourceAspect_OnChange,
    },
    {
        "Screen Aspect", SLIDER, 0, (long*)&AspectSettings.TargetAspect,
        1333, 0, 3000, 1, 1000,
        NULL,
        "ASPECT", "TargetAspect", TargetAspect_OnChange,
    },
    {
        "Custom Screen Aspect", SLIDER, 0, (long*)&AspectSettings.custom_target_aspect,
        1335, 1000, 3000, 5, 1000,
        NULL,
        "ASPECT", "CustomTargetAspect", CustomTargetAspect_OnChange,
    },
    {
        "Aspect Mode", SLIDER, 0, (long*)&AspectSettings.AspectMode,
        1, 0, 2, 1, 1,
        NULL,
        "ASPECT", "Mode", AspectMode_OnChange,
    },
    {
        "Auto Detect Aspect Sensitivity", SLIDER, 0, (long*)&AspectSettings.LuminanceThreshold,
        40, 0, 255, 1, 1,
        NULL,
        "ASPECT_DETECT", "LuminanceThreshold", NULL,
    },
    {
        "Ignore Non-Black Pixels", SLIDER, 0, (long*)&AspectSettings.IgnoreNonBlackPixels,
        3, 0, 750, 1, 1,
        NULL,
        "ASPECT_DETECT", "IgnoreNonBlackPixels", NULL,
    },
    {
        "Auto Detect Aspect", ONOFF, 0, (long*)&AspectSettings.AutoDetectAspect,
        FALSE, 0, 1, 1, 1,
        NULL,
        "ASPECT_DETECT", "AutoDetectAspect", NULL,
    },
    {
        "Zoom In Frame Count", SLIDER, 0, (long*)&AspectSettings.ZoomInFrameCount,
        60, 0, 1000, 10, 1,
        NULL,
        "ASPECT_DETECT", "ZoomInFrameCount", NULL,
    },
    {
        "Aspect History Time", SLIDER, 0, (long*)&AspectSettings.AspectHistoryTime,
        300, 0, 3000, 10, 1,
        NULL,
        "ASPECT_DETECT", "AspectHistoryTime", NULL,
    },
    {
        "Aspect Consistency Time", SLIDER, 0, (long*)&AspectSettings.AspectConsistencyTime,
        15, 0, 300, 5, 1,
        NULL,
        "ASPECT_DETECT", "AspectConsistencyTime", NULL,
    },
    {
        "Vert Image Pos", ITEMFROMLIST, 0, (long*)&AspectSettings.VerticalPos,
        VERT_POS_CENTRE, 0, 2, 1, 1,
        VertPosString,
        "ASPECT", "VerticalPos", VertPos_OnChange,
    },
    {
        "Horiz Image Pos", ITEMFROMLIST, 0, (long*)&AspectSettings.HorizontalPos,
        HORZ_POS_CENTRE, 0, 2, 1, 1,
        HorzPosString,
        "ASPECT", "HorizontalPos", HorizPos_OnChange,
    },
    {
        "Clipping", ONOFF, 0, (long*)&AspectSettings.AspectImageClipped,
        TRUE, 0, 1, 1, 1,
        NULL,
        "ASPECT", "Clipping", Clipping_OnChange,
    },
    {
        "Bounce", ONOFF, 0, (long*)&AspectSettings.BounceEnabled,
        FALSE, 0, 1, 1, 1,
        NULL,
        "ASPECT", "Bounce", Bounce_OnChange,
    },
    {
        "Bounce Period (Secs)", SLIDER, 0, (long*)&AspectSettings.BouncePeriod,
        60*30, 60, 60*600, 1, 1,
        NULL,
        "ASPECT", "BouncePeriod", NULL,
    },
    {
        "Defer Setting Overlay", ONOFF, 0, (long*)&AspectSettings.DeferedSetOverlay,
        TRUE, 0, 1, 1, 1,
        NULL,
        "ASPECT", "DeferedOverlay", NULL,
    },
    {
        "Bounce Timer Period (ms)", SLIDER, 0, (long*)&AspectSettings.TimerBounceMS,
        1000, 100, 5000, 1, 1,
        NULL,
        "ASPECT", "BounceTimerPeriod", NULL,
    },
    {
        "Bounce Amplitude (%)", SLIDER, 0, (long*)&AspectSettings.BounceAmplitude,
        100, 0, 100, 1, 1,
        NULL,
        "ASPECT", "BounceAmplitude", NULL,
    },
    {
        "Orbit", ONOFF, 0, (long*)&AspectSettings.OrbitEnabled,
        FALSE, 0, 1, 1, 1,
        NULL,
        "ASPECT", "Orbit", Orbit_OnChange,
    },
    {
        "Orbit Period X", SLIDER, 0, (long*)&AspectSettings.OrbitPeriodX,
        60*45, 60, 60*600, 1, 1,
        NULL,
        "ASPECT", "OrbitPeriodX", NULL,
    },
    {
        "Orbit Period Y", SLIDER, 0, (long*)&AspectSettings.OrbitPeriodY,
        60*60, 60, 60*600, 1, 1,
        NULL,
        "ASPECT", "OrbitPeriodY", NULL,
    },
    {
        "Orbit Size", SLIDER, 0, (long*)&AspectSettings.OrbitSize,
        8, 0, 200, 1, 1,
        NULL,
        "ASPECT", "OrbitSize", NULL,
    },
    {
        "Orbit Timer Period (ms)", SLIDER, 0, (long*)&AspectSettings.TimerOrbitMS,
        60*1000, 1000, 600 * 1000, 1, 1,
        NULL,
        "ASPECT", "OrbitTimerPeriod", NULL,
    },
    {
        "Auto-Size Window", ONOFF, 0, (long*)&AspectSettings.AutoResizeWindow,
        FALSE, 0, 1, 1, 1,
        NULL,
        "ASPECT", "AutoSizeWindow", NULL,
    },
    {
        "Skip Percentage", SLIDER, 0, (long*)&AspectSettings.SkipPercent,
        17, 0, 49, 1, 1,
        NULL,
        "ASPECT_DETECT", "SkipPercent", NULL,
    },
    {
        "X Zoom Factor", SLIDER, 0, &AspectSettings.ZoomFactorX,
        100, 1, 1000, 10, 100,
        NULL,
        "ASPECT", "XZoomFactor", XZoom_Factor_OnChange,
    },
    {
        "Y Zoom Factor", SLIDER, 0, &AspectSettings.ZoomFactorY,
        100, 1, 1000, 10, 100,
        NULL,
        "ASPECT", "YZoomFactor", YZoom_Factor_OnChange,
    },
    {
        "X Zoom Center", SLIDER, 0, &AspectSettings.ZoomCenterX,
        50, -100, 200, 5, 100,
        NULL,
        "ASPECT", "XZoomCenter", XZoom_Center_OnChange,
    },
    {
        "Y Zoom Center", SLIDER, 0, &AspectSettings.ZoomCenterY,
        50, -100, 200, 5, 100,
        NULL,
        "ASPECT", "YZoomCenter", YZoom_Center_OnChange,
    },
    {
        "Chroma Range", SLIDER, 0, &AspectSettings.ChromaRange,
        16, 0, 255, 1, 1,
        NULL,
        "ASPECT_DETECT", "ChromaRange", ChromaRange_OnChange,
    },
    {
        "Wait for Vertical Blank While Drawing", ONOFF, 0, (long*)&AspectSettings.bWaitForVerticalBlank,
        FALSE, 0, 1, 1, 1,
        NULL,
        "ASPECT", "WaitForVerticalBlank", NULL,
    },
    {
        "Zoom Out Frame Count", SLIDER, 0, (long*)&AspectSettings.ZoomOutFrameCount,
        3, 1, 1000, 1, 1,
        NULL,
        "ASPECT_DETECT", "ZoomOutFrameCount", NULL,
    },
    {
        "Allow greater ratio than screen ratio", ONOFF, 0, (long*)&AspectSettings.bAllowGreaterThanScreen,
        TRUE, 0, 1, 1, 1,
        NULL,
        "ASPECT_DETECT", "AllowGreaterThanScreen", NULL,
    },
    {
        "Mask Grey Shade", SLIDER, 0, (long*)&AspectSettings.MaskGreyShade,
        0, 0, 255, 1, 1,
        NULL,
        "ASPECT_DETECT", "MaskGreyShade", MaskGreyShade_OnChange,
    },
};

SETTING* Aspect_GetSetting(ASPECT_SETTING Setting)
{
    if(Setting > -1 && Setting < ASPECT_SETTING_LASTONE)
    {
        return &(AspectGUISettings[Setting]);
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
        Setting_ReadFromIni(&(AspectGUISettings[i]));
    }
}

void Aspect_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < ASPECT_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(AspectGUISettings[i]), bOptimizeFileAccess);
    }
}

void Aspect_FinalSetup()
{
    Bounce_OnChange(AspectSettings.BounceEnabled);
    Orbit_OnChange(AspectSettings.OrbitEnabled);
}

void Aspect_ShowUI()
{
    CSettingsDlg::ShowSettingsDlg("Aspect Ratio Settings",AspectGUISettings, ASPECT_SETTING_LASTONE);
}
