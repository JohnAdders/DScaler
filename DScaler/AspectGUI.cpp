/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file AspectGUI.cpp Aspect Ratio GUI Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "DScaler.h"
#include "VBI.h"
#include "OutThreads.h"
#include "SettingsPerChannel.h"
#include "SettingsMaster.h"
#include "Providers.h"
#include "OverlayOutput.h"


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

    CheckMenuItem(hMenu, IDM_SASPECT_SQUARE, MF_UNCHECKED);

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
    CheckMenuItem(hMenu, IDM_SASPECT_AUTO2_TOGGLE, MF_UNCHECKED);

    if(AspectSettings.AutoDetectAspect == 1)
    {
        ModifyMenu(hMenu, IDM_ASPECT_LETTERBOX, MF_STRING, IDM_ASPECT_LETTERBOX, _T("&Non Anamorphic"));
        ModifyMenu(hMenu, IDM_ASPECT_ANAMORPHIC, MF_STRING, IDM_ASPECT_ANAMORPHIC, _T("Anamo&rphic"));
        EnableMenuItem(hMenu, IDM_SASPECT_AUTO_TOGGLE, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_SASPECT_AUTO2_TOGGLE, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_ASPECT_FULLSCREEN, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_ASPECT_LETTERBOX, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_ASPECT_ANAMORPHIC, MF_ENABLED);
    }
    else if(AspectSettings.AutoDetectAspect == 2)
    {
        ModifyMenu(hMenu, IDM_ASPECT_LETTERBOX, MF_STRING, IDM_ASPECT_LETTERBOX, _T("&Non Anamorphic"));
        ModifyMenu(hMenu, IDM_ASPECT_ANAMORPHIC, MF_STRING, IDM_ASPECT_ANAMORPHIC, _T("Anamo&rphic"));
        EnableMenuItem(hMenu, IDM_SASPECT_AUTO_TOGGLE, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_SASPECT_AUTO2_TOGGLE, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_ASPECT_FULLSCREEN, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_ASPECT_LETTERBOX, MF_GRAYED);
        EnableMenuItem(hMenu, IDM_ASPECT_ANAMORPHIC, MF_GRAYED);
    }
    else
    {
        ModifyMenu(hMenu, IDM_ASPECT_LETTERBOX, MF_STRING, IDM_ASPECT_LETTERBOX, _T("16:9 &Letterboxed"));
        ModifyMenu(hMenu, IDM_ASPECT_ANAMORPHIC, MF_STRING, IDM_ASPECT_ANAMORPHIC, _T("&16:9 Anamorphic"));
        EnableMenuItem(hMenu, IDM_SASPECT_AUTO_TOGGLE, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_SASPECT_AUTO2_TOGGLE, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_ASPECT_FULLSCREEN, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_ASPECT_LETTERBOX, MF_ENABLED);
        EnableMenuItem(hMenu, IDM_ASPECT_ANAMORPHIC, MF_ENABLED);
    }

    if(AspectSettings.SquarePixels)
    {
        CheckMenuItem(hMenu, IDM_SASPECT_SQUARE, MF_CHECKED);
    }
    else if(AspectSettings.AutoDetectAspect == 1)
    {
        CheckMenuItem(hMenu, IDM_SASPECT_AUTO_TOGGLE, MF_CHECKED);
        if (AspectSettings.AspectMode == 1)
        {
            CheckMenuItem(hMenu, IDM_ASPECT_LETTERBOX, MF_CHECKED);
        }
        else if (AspectSettings.AspectMode == 2)
        {
            CheckMenuItem(hMenu, IDM_ASPECT_ANAMORPHIC, MF_CHECKED);
        }
    }
    else if(AspectSettings.AutoDetectAspect == 2)
    {
        CheckMenuItem(hMenu, IDM_SASPECT_AUTO2_TOGGLE, MF_CHECKED);
        if (AspectSettings.AspectMode == 1)
        {
            CheckMenuItem(hMenu, IDM_ASPECT_LETTERBOX, MF_CHECKED);
        }
        else if (AspectSettings.AspectMode == 2)
        {
            CheckMenuItem(hMenu, IDM_ASPECT_ANAMORPHIC, MF_CHECKED);
        }
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
    CheckMenuItemBool(hMenu, IDM_WINPOS_AUTOSIZE, (AspectSettings.AutoResizeWindow));
    CheckMenuItemBool(hMenu, IDM_ANALOGUE_BLANKING, (AspectSettings.bAnalogueBlanking));
    EnableMenuItem(hMenu, IDM_ANALOGUE_BLANKING, Providers_GetCurrentSource() && Providers_GetCurrentSource()->GetAnalogueBlanking() ? MF_ENABLED : MF_GRAYED);

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
    TCHAR Text[32];
    static BOOL WSSWasEnabled = FALSE;
    CSource* pSource;

    switch (wMenuID)
    {
    //------------------------------------------------------------------
    // Easily Accessible Aspect Ratios
    case IDM_ASPECT_FULLSCREEN:
        AspectSettings.SquarePixels = FALSE;
        if (AspectSettings.AutoDetectAspect == 1)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            // This applies to both letterbox and 4:3
            SwitchToRatio(AR_NONANAMORPHIC, AspectSettings.SourceAspect);
            ShowText(hWnd, _T("Nonanamorphic Signal"));
            AspectSettings.DetectAspectNow = TRUE;
        }
        else if (AspectSettings.AutoDetectAspect == 0)
        {
            SwitchToRatio(AR_NONANAMORPHIC, 1333);
            ShowText(hWnd, _T("4:3 Fullscreen Signal"));
        }
        else
        {
            ShowText(hWnd, _T("Command refused"));
        }
        break;
    case IDM_ASPECT_LETTERBOX:
        AspectSettings.SquarePixels = FALSE;
        if (AspectSettings.AutoDetectAspect == 1)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            // This applies to both letterbox and 4:3
            SwitchToRatio(AR_NONANAMORPHIC, AspectSettings.SourceAspect);
            ShowText(hWnd, _T("Nonanamorphic Signal"));
            AspectSettings.DetectAspectNow = TRUE;
        }
        else if (AspectSettings.AutoDetectAspect == 0)
        {
            SwitchToRatio(AR_NONANAMORPHIC, 1778);
            ShowText(hWnd, _T("1.78:1 Letterbox Signal"));
        }
        else
        {
            ShowText(hWnd, _T("Command refused"));
        }
        break;
    case IDM_ASPECT_ANAMORPHIC:
        AspectSettings.SquarePixels = FALSE;
        if (AspectSettings.AutoDetectAspect == 1)
        {
            // If autodetect enabled, don't change aspect ratio, just anamorphic status
            SwitchToRatio(AR_ANAMORPHIC, AspectSettings.SourceAspect);
            ShowText(hWnd, _T("Anamorphic Signal"));
            AspectSettings.DetectAspectNow = TRUE;
        }
        else if (AspectSettings.AutoDetectAspect == 0)
        {
            SwitchToRatio(AR_ANAMORPHIC, 1778);
            ShowText(hWnd, _T("1.78:1 Anamorphic Signal"));
        }
        else
        {
            ShowText(hWnd, _T("Command refused"));
        }
        break;

    //------------------------------------------------------------------
    // Image position settings
    case IDM_WINPOS_VERT_CENTRE:
    case IDM_WINPOS_VERT_TOP:
    case IDM_WINPOS_VERT_BOTTOM:
        AspectSettings.VerticalPos = (eVertPos)(wMenuID - IDM_WINPOS_VERT_BOTTOM);
        WorkoutOverlaySize(TRUE);
        break;

    case IDM_WINPOS_HORZ_CENTRE:
    case IDM_WINPOS_HORZ_LEFT:
    case IDM_WINPOS_HORZ_RIGHT:
        AspectSettings.HorizontalPos = (eHorzPos)(wMenuID - IDM_WINPOS_HORZ_RIGHT);
        WorkoutOverlaySize(TRUE);
        break;

    case IDM_ANALOGUE_BLANKING:
        pSource = Providers_GetCurrentSource();
        if (pSource && pSource->GetAnalogueBlanking() != NULL)
        {
            pSource->GetAnalogueBlanking()->SetValue(!pSource->GetAnalogueBlanking()->GetValue());
            AspectSettings.bAnalogueBlanking = pSource->GetAnalogueBlanking()->GetValue();
            ShowText(hWnd, AspectSettings.bAnalogueBlanking ? _T("Analogue Blanking ON") : _T("Analogue Blanking OFF"));
        }
        break;


    case IDM_SASPECT_CLIP:
        AspectSettings.AspectImageClipped = !AspectSettings.AspectImageClipped;
        ShowText(hWnd, AspectSettings.AspectImageClipped ? _T("Image Clipping ON") : _T("Image Clipping OFF"));
        break;

    case IDM_WINPOS_BOUNCE:
        Bounce_OnChange(!AspectSettings.BounceEnabled);
        ShowText(hWnd, AspectSettings.BounceEnabled ? _T("Image Bouncing ON") : _T("Image Bouncing OFF"));
        break;

    case IDM_WINPOS_ORBIT:
        Orbit_OnChange(!AspectSettings.OrbitEnabled);
        ShowText(hWnd, AspectSettings.OrbitEnabled ? _T("Orbit ON") : _T("Orbit OFF"));
        break;

    case IDM_WINPOS_AUTOSIZE:
        AspectSettings.AutoResizeWindow = !AspectSettings.AutoResizeWindow;
        ShowText(hWnd, AspectSettings.AutoResizeWindow ? _T("Auto-Size Window ON") : _T("Auto-Size Window OFF"));
        break;

    case IDM_ASPECT_DEFER_OVERLAY:
        AspectSettings.DeferedSetOverlay = !AspectSettings.DeferedSetOverlay;
        break;


    //-----------------------------------------------------------------
    // Autodetect aspect ratio toggles
    case IDM_SASPECT_AUTO_ON:
        if (AspectSettings.AutoDetectAspect == 0)
        {
            UpdateSquarePixelsMode(FALSE);
            AspectSettings.AutoDetectAspect = 1;
            ShowText(hWnd, _T("Auto Detect Black Bars ON"));
            if (AspectSettings.bUseWSS)
            {
                if (!Setting_GetValue(WM_VBI_GETVALUE, DOWSS))
                {
                    Setting_SetValue(WM_VBI_GETVALUE, DOWSS, TRUE);
                    WSSWasEnabled = TRUE;
                }
                if (!Setting_GetValue(WM_VBI_GETVALUE, CAPTURE_VBI))
                {
                    SendMessage(hWnd, WM_COMMAND, IDM_VBI, 0);
                }
            }
        }
        break;
    case IDM_SASPECT_AUTO_OFF:
        if (AspectSettings.AutoDetectAspect == 1)
        {
            AspectSettings.AutoDetectAspect = 0;
            ShowText(hWnd, _T("Auto Detect Black Bars OFF"));
            if (AspectSettings.bUseWSS && WSSWasEnabled)
            {
                if (Setting_GetValue(WM_VBI_GETVALUE, DOWSS))
                {
                    Setting_SetValue(WM_VBI_GETVALUE, DOWSS, FALSE);
                }
                WSSWasEnabled = FALSE;
            }
        }
        break;
    case IDM_SASPECT_AUTO_TOGGLE:
        if (AspectSettings.AutoDetectAspect != 0)
        {
            AspectSettings.AutoDetectAspect = 0;
            ShowText(hWnd, _T("Auto Detect Black Bars OFF"));
        }
        else
        {
            UpdateSquarePixelsMode(FALSE);
            AspectSettings.AutoDetectAspect = 1;
            ShowText(hWnd, _T("Auto Detect Black Bars ON"));
        }
        if (AspectSettings.AutoDetectAspect && AspectSettings.bUseWSS)
        {
            if (!Setting_GetValue(WM_VBI_GETVALUE, DOWSS))
            {
                Setting_SetValue(WM_VBI_GETVALUE, DOWSS, TRUE);
                WSSWasEnabled = TRUE;
            }
            if (!Setting_GetValue(WM_VBI_GETVALUE, CAPTURE_VBI))
            {
                SendMessage(hWnd, WM_COMMAND, IDM_VBI, 0);
            }
        }
        else if (AspectSettings.bUseWSS && WSSWasEnabled)
        {
            if (Setting_GetValue(WM_VBI_GETVALUE, DOWSS))
            {
                Setting_SetValue(WM_VBI_GETVALUE, DOWSS, FALSE);
            }
            WSSWasEnabled = FALSE;
        }
        break;
    case IDM_SASPECT_AUTO2_TOGGLE:
        if (AspectSettings.AutoDetectAspect != 0)
        {
            AspectSettings.AutoDetectAspect = 0;
            ShowText(hWnd, _T("Use Signal Aspect Data OFF"));
        }
        else
        {
            UpdateSquarePixelsMode(FALSE);
            AspectSettings.AutoDetectAspect = 2;
            ShowText(hWnd, _T("Use Signal Aspect Data ON"));
        }
        if (AspectSettings.AutoDetectAspect)
        {
            if (!Setting_GetValue(WM_VBI_GETVALUE, DOWSS))
            {
                Setting_SetValue(WM_VBI_GETVALUE, DOWSS, TRUE);
                WSSWasEnabled = TRUE;
            }
            if (!Setting_GetValue(WM_VBI_GETVALUE, CAPTURE_VBI))
            {
                SendMessage(hWnd, WM_COMMAND, IDM_VBI, 0);
            }
        }
        else if (WSSWasEnabled)
        {
            if (Setting_GetValue(WM_VBI_GETVALUE, DOWSS))
            {
                Setting_SetValue(WM_VBI_GETVALUE, DOWSS, FALSE);
            }
            WSSWasEnabled = FALSE;
        }
        break;
    case IDM_SASPECT_SQUARE:
        UpdateSquarePixelsMode(!AspectSettings.SquarePixels);
        if (AspectSettings.SquarePixels)
        {
            ShowText(hWnd, _T("Square Pixels ON"));
        }
        else
        {
            ShowText(hWnd, _T("Square Pixels OFF"));
        }
        break;


    //------------------------------------------------------------------
    // Output Display Aspect Ratios
    case IDM_TASPECT_0:
        AspectSettings.TargetAspect = 0;
        ShowText(hWnd, _T("Aspect Ratio From Current Resolution"));
        break;
    case IDM_TASPECT_133:
        AspectSettings.TargetAspect = 1333;
        ShowText(hWnd, _T("1.33:1 Screen"));
        break;
    case IDM_TASPECT_166:
        AspectSettings.TargetAspect = 1667;
        ShowText(hWnd, _T("1.66:1 Screen"));
        break;
    case IDM_TASPECT_178:
        AspectSettings.TargetAspect = 1778;
        ShowText(hWnd, _T("1.78:1 Screen"));
        break;
    case IDM_TASPECT_185:
        AspectSettings.TargetAspect = 1850;
        ShowText(hWnd, _T("1.85:1 Screen"));
        break;
    case IDM_TASPECT_200:
        AspectSettings.TargetAspect = 2000;
        ShowText(hWnd, _T("2.00:1 Screen"));
        break;
    case IDM_TASPECT_235:
        AspectSettings.TargetAspect = 2350;
        ShowText(hWnd, _T("2.35:1 Screen"));
        break;
    case IDM_TASPECT_CUSTOM:
        AspectSettings.TargetAspect = AspectSettings.custom_target_aspect;
        ShowText(hWnd, _T("Custom Aspect Ratio Screen"));
        break;

    // Manually-triggered one-time automatic detect of aspect ratio
    case IDM_SASPECT_COMPUTE:
        AspectSettings.DetectAspectNow = TRUE;
        UpdateSquarePixelsMode(FALSE);
        break;

    //------------------------------------------------------------------
    // Zoom
    case IDM_ZOOM_MINUS:
        AspectSettings.ZoomFactorX--;
        if (AspectSettings.ZoomFactorX < 1)
        {
            AspectSettings.ZoomFactorX = 1;
        }
        AspectSettings.ZoomFactorY--;
        if (AspectSettings.ZoomFactorY < 1)
        {
            AspectSettings.ZoomFactorY = 1;
        }
        if ((AspectSettings.ZoomFactorX == 100) && (AspectSettings.ZoomFactorY == 100))
        {
            ShowText(hWnd,_T("Zoom Off"));
        }
        else
        {
            _stprintf(Text, _T("Zoom %.2fx"), (double)AspectSettings.ZoomFactorX / 100.0);
            ShowText(hWnd, Text);
        }
        break;

    case IDM_ZOOM_PLUS:
        if(AspectSettings.ZoomFactorX >= 1)
        {
            AspectSettings.ZoomFactorX++;
        }
        else
        {
            AspectSettings.ZoomFactorX = 1;
        }
        if (AspectSettings.ZoomFactorX > 1000)
        {
            AspectSettings.ZoomFactorX = 1000;
        }

        if(AspectSettings.ZoomFactorY >= 1)
        {
            AspectSettings.ZoomFactorY++;
        }
        else
        {
            AspectSettings.ZoomFactorY = 1;
        }
        if (AspectSettings.ZoomFactorY > 1000)
        {
            AspectSettings.ZoomFactorY = 1000;
        }
        if ((AspectSettings.ZoomFactorX == 100) && (AspectSettings.ZoomFactorY == 100))
        {
            ShowText(hWnd,_T("Zoom Off"));
        }
        else
        {
            _stprintf(Text, _T("Zoom %.2fx"), (double)AspectSettings.ZoomFactorX / 100.0);
            ShowText(hWnd, Text);
        }
        break;

    case IDM_ZOOM_10:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 100;
        ShowText(hWnd,_T("Zoom Off"));
        break;

    case IDM_ZOOM_15:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 150;
        ShowText(hWnd,_T("Zoom 1.5x"));
        break;

    case IDM_ZOOM_20:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 200;
        ShowText(hWnd,_T("Zoom 2x"));
        break;

    case IDM_ZOOM_25:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 250;
        ShowText(hWnd,_T("Zoom 2.5x"));
        break;

    case IDM_ZOOM_30:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 300;
        ShowText(hWnd,_T("Zoom 3x"));
        break;

    case IDM_ZOOM_35:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 350;
        ShowText(hWnd,_T("Zoom 3.5x"));
        break;

    case IDM_ZOOM_40:
        AspectSettings.ZoomFactorX = AspectSettings.ZoomFactorY = 400;
        ShowText(hWnd,_T("Zoom 4x"));
        break;

    case IDM_VT_PAGE_MINUS:
        AspectSettings.ZoomCenterX -= 1;
        if (AspectSettings.ZoomCenterX < -100)
        {
            AspectSettings.ZoomCenterX = -100;
        }
        _stprintf(Text, _T("X Center %.2fx"), (double)AspectSettings.ZoomCenterX / 100.0);
        ShowText(hWnd, Text);
        break;

    case IDM_VT_PAGE_PLUS:
        AspectSettings.ZoomCenterX += 1;
        if (AspectSettings.ZoomCenterX > 200)
        {
            AspectSettings.ZoomCenterX = 200;
        }
        _stprintf(Text, _T("X Center %.2fx"), (double)AspectSettings.ZoomCenterX / 100.0);
        ShowText(hWnd, Text);
        break;

    case IDM_VT_PAGE_DOWN:
        AspectSettings.ZoomCenterY += 1;
        if (AspectSettings.ZoomCenterY > 200)
        {
            AspectSettings.ZoomCenterY = 200;
        }
        _stprintf(Text, _T("Y Center %.2fx"), (double)AspectSettings.ZoomCenterY / 100.0);
        ShowText(hWnd, Text);
        break;

    case IDM_VT_PAGE_UP:
        AspectSettings.ZoomCenterY -= 1;
        if (AspectSettings.ZoomCenterY < -100)
        {
            AspectSettings.ZoomCenterY = 100;
        }
        _stprintf(Text, _T("Y Center %.2fx"), (double)AspectSettings.ZoomCenterY / 100.0);
        ShowText(hWnd, Text);
        break;

    //------------------------------------------------------------------
    default:
        // At this point, we want to reset the automatic aspect
        // because the end user selected an _T("Advanced Source Aspect Ratio")
        // In this case, turn off automatic aspect ratio detect.
        // Then restart the 'switch' statement.

        //--------------------------------------------------------------
        // Advanced Source Aspect Ratios
        switch (wMenuID)
        {
        case IDM_SASPECT_0:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_STRETCH, 0);
            ShowText(hWnd, _T("Stretch Video"));
            break;
        case IDM_SASPECT_133:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 1333);
            ShowText(hWnd, _T("4:3 Fullscreen Signal"));
            break;
        case IDM_SASPECT_144:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 1444);
            ShowText(hWnd, _T("1.44:1 Letterbox Signal"));
            break;
        case IDM_SASPECT_155:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 1555);
            ShowText(hWnd, _T("1.55:1 Letterbox Signal"));
            break;
        case IDM_SASPECT_166:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 1667);
            ShowText(hWnd, _T("1.66:1 Letterbox Signal"));
            break;
        case IDM_SASPECT_178:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 1778);
            ShowText(hWnd, _T("1.78:1 Letterbox Signal"));
            break;
        case IDM_SASPECT_185:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 1850);
            ShowText(hWnd, _T("1.85:1 Letterbox Signal"));
            break;
        case IDM_SASPECT_200:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 2000);
            ShowText(hWnd, _T("2.00:1 Letterbox Signal"));
            break;
        case IDM_SASPECT_235:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_NONANAMORPHIC, 2350);
            ShowText(hWnd, _T("2.35:1 Letterbox Signal"));
            break;
        case IDM_SASPECT_166A:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_ANAMORPHIC, 1667);
            ShowText(hWnd, _T("1.66:1 Anamorphic Signal"));
            break;
        case IDM_SASPECT_178A:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_ANAMORPHIC, 1778);
            ShowText(hWnd, _T("1.78:1 Anamorphic Signal"));
            break;
        case IDM_SASPECT_185A:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_ANAMORPHIC, 1850);
            ShowText(hWnd, _T("1.85:1 Anamorphic Signal"));
            break;
        case IDM_SASPECT_200A:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_ANAMORPHIC, 2000);
            ShowText(hWnd, _T("2.00:1 Anamorphic Signal"));
            break;
        case IDM_SASPECT_235A:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_ANAMORPHIC, 2350);
            ShowText(hWnd, _T("2.35:1 Anamorphic Signal"));
            break;
        case IDM_SASPECT_CUSTOM:
            AspectSettings.AutoDetectAspect = 0;
            AspectSettings.SquarePixels = FALSE;
            SwitchToRatio(AR_ANAMORPHIC, AspectSettings.CustomSourceAspect);
            ShowText(hWnd, _T("Custom Aspect Ratio Signal"));
            break;
        default:
            // It's not an aspect ratio related menu selection
            return FALSE;
        }
    }

    WorkoutOverlaySize(TRUE);
    if (GetMenu(hWnd) != NULL)
        AspectRatio_SetMenu(GetMenu(hWnd));

    // Yes, we processed the menu selection.
    return TRUE;
}

//----------------------------------------------------------------------------
// Repaints the overlay colorkey, optionally with black borders around it
// during aspect ratio control


void PaintColorkey(HWND hWnd, BOOL bEnable, HDC hDC, RECT* PaintRect, BOOL bNoMiddlePainting)
{
    // MRS 9-9-00
    HBRUSH Background = CreateSolidBrush(RGB(AspectSettings.MaskGreyShade,
                                        AspectSettings.MaskGreyShade,
                                        AspectSettings.MaskGreyShade));
    HBRUSH Overlay;
    RECT r;
    RECT r2, winRect;

    if (bEnable && GetActiveOutput()->OverlayActive())
    {
        Overlay = CreateSolidBrush(GetActiveOutput()->Overlay_GetCorrectedColor(hDC));
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
    //GetClientRect(hWnd,&winRect);
    GetDisplayAreaRect(hWnd,&winRect, TRUE);

    // Top
    r2.left = winRect.left;
    r2.top = winRect.top;
    r2.right = winRect.right;
    r2.bottom = AspectSettings.DestinationRect.top;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    // Bottom
    r2.left = winRect.left;
    r2.top = AspectSettings.DestinationRect.bottom;
    r2.right = winRect.right;
    r2.bottom = winRect.bottom;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    // Left
    r2.left = winRect.left;
    r2.top = winRect.top;
    r2.right = AspectSettings.DestinationRect.left;
    r2.bottom = winRect.bottom;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    // Right
    r2.left = AspectSettings.DestinationRect.right;
    r2.top = winRect.top;
    r2.right = winRect.right;
    r2.bottom = winRect.bottom;
    IntersectRect(&r, &r2, PaintRect);
    FillRect(hDC, &r, Background);

    if (AspectSettings.OverlayNeedsSetting)
    {
        // MRS 2-22-01
        // Intended to prevent purple flashing by setting overlay
        // after drawing black but before drawing purple.
        GetActiveOutput()->Overlay_Update(&AspectSettings.SourceRect, &AspectSettings.DestinationRectWindow, DDOVER_SHOW);
        AspectSettings.OverlayNeedsSetting = FALSE;
        // Wait till current frame is done before drawing purple...
        // Overlay changes do not seem to take place (at least on a GeForce)
        // until the VBI...so need to wait to avoid purple flashing

        // JA 29/6/2001 made wait for blank optional and swicthed off
        // the flashing has been much reduced by using dark grey as
        // overlay colour.  Also this may cause the pausing effect
        // on Teletext and CC
        if (AspectSettings.bWaitForVerticalBlank == TRUE)
        {
            GetActiveOutput()->WaitForVerticalBlank();
        }
    }

    // Draw overlay color in the middle.
    if(!bNoMiddlePainting)
    {
        IntersectRect(&r, &AspectSettings.DestinationRect, PaintRect);
        FillRect(hDC, &r, Overlay);
    }

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

BOOL AspectMode_OnChange(long NewValue)
{
    AspectSettings.AspectMode = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL TargetAspect_OnChange(long NewValue)
{
    AspectSettings.TargetAspect = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL CustomTargetAspect_OnChange(long NewValue)
{
    AspectSettings.TargetAspect = NewValue;
    AspectSettings.custom_target_aspect = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL SourceAspect_OnChange(long NewValue)
{
    AspectSettings.AutoDetectAspect = 0;
    AspectSettings.SourceAspect = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL CustomSourceAspect_OnChange(long NewValue)
{
    AspectSettings.AutoDetectAspect = 0;
    AspectSettings.CustomSourceAspect = NewValue;
    AspectSettings.SourceAspect = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL HorizPos_OnChange(long NewValue)
{
    AspectSettings.HorizontalPos = (eHorzPos)NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL VertPos_OnChange(long NewValue)
{
    AspectSettings.VerticalPos = (eVertPos)NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL Bounce_OnChange(long NewValue)
{
    AspectSettings.BounceEnabled = NewValue != 0;
    if (!AspectSettings.BounceEnabled)
    {
        KillTimer(GetMainWnd(), TIMER_BOUNCE);
    }
    else
    {
        SetTimer(GetMainWnd(), TIMER_BOUNCE, AspectSettings.TimerBounceMS, NULL);
    }
    return FALSE;
}

BOOL Orbit_OnChange(long NewValue)
{
    AspectSettings.OrbitEnabled = NewValue != 0;
    if (!AspectSettings.OrbitEnabled)
    {
        KillTimer(GetMainWnd(), TIMER_ORBIT);
    }
    else
    {
        SetTimer(GetMainWnd(), TIMER_ORBIT, AspectSettings.TimerOrbitMS, NULL);
    }
    return FALSE;
}

BOOL Clipping_OnChange(long NewValue)
{
    AspectSettings.AspectImageClipped = (BOOL)NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL XZoom_Factor_OnChange(long NewValue)
{
    AspectSettings.ZoomFactorX = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL YZoom_Factor_OnChange(long NewValue)
{
    AspectSettings.ZoomFactorY = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL XZoom_Center_OnChange(long NewValue)
{
    AspectSettings.ZoomCenterX = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL YZoom_Center_OnChange(long NewValue)
{
    AspectSettings.ZoomCenterY = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL ChromaRange_OnChange(long NewValue)
{
    AspectSettings.ChromaRange = NewValue;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

BOOL MaskGreyShade_OnChange(long NewValue)
{
    AspectSettings.MaskGreyShade = NewValue;
    InvalidateRect(GetMainWnd(), NULL, FALSE);
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING AspectGUISettings[ASPECT_SETTING_LASTONE] =
{
    {
        "Source Aspect", SLIDER, 0, (LONG_PTR*)&AspectSettings.SourceAspect,
        1333, 1000, 3000, 1, 1000,
        NULL,
        "Aspect", "SourceAspect", SourceAspect_OnChange,
    },
    {
        "Custom Source Aspect", SLIDER, 0, (LONG_PTR*)&AspectSettings.CustomSourceAspect,
        1335, 1000, 3000, 5, 1000,
        NULL,
        "Aspect", "CustomSourceAspect", CustomSourceAspect_OnChange,
    },
    {
        "Screen Aspect", SLIDER, 0, (LONG_PTR*)&AspectSettings.TargetAspect,
        0, 0, 3000, 1, 1000,
        NULL,
        "Aspect", "TargetAspect", TargetAspect_OnChange,
    },
    {
        "Custom Screen Aspect", SLIDER, 0, (LONG_PTR*)&AspectSettings.custom_target_aspect,
        1335, 1000, 3000, 5, 1000,
        NULL,
        "Aspect", "CustomTargetAspect", CustomTargetAspect_OnChange,
    },
    {
        "Aspect Mode", SLIDER, 0, (LONG_PTR*)&AspectSettings.AspectMode,
        1, 0, 2, 1, 1,
        NULL,
        "Aspect", "Mode", AspectMode_OnChange,
    },
    {
        "Auto Detect Aspect Sensitivity", SLIDER, 0, (LONG_PTR*)&AspectSettings.LuminanceThreshold,
        40, 0, 255, 1, 1,
        NULL,
        "Aspect_Detect", "LuminanceThreshold", NULL,
    },
    {
        "Ignore Non-Black Pixels", SLIDER, 0, (LONG_PTR*)&AspectSettings.IgnoreNonBlackPixels,
        3, 0, 750, 1, 1,
        NULL,
        "Aspect_Detect", "IgnoreNonBlackPixels", NULL,
    },
    {
        "Auto Detect Aspect", SLIDER, 0, (LONG_PTR*)&AspectSettings.AutoDetectAspect,
        0, 0, 2, 1, 1,
        NULL,
        "Aspect_Detect", "AutoDetectAspect", NULL,
    },
    {
        "Zoom In Frame Count", SLIDER, 0, (LONG_PTR*)&AspectSettings.ZoomInFrameCount,
        60, 0, 1000, 10, 1,
        NULL,
        "Aspect_Detect", "ZoomInFrameCount", NULL,
    },
    {
        "Aspect History Time", SLIDER, 0, (LONG_PTR*)&AspectSettings.AspectHistoryTime,
        300, 0, 3000, 10, 1,
        NULL,
        "Aspect_Detect", "AspectHistoryTime", NULL,
    },
    {
        "Aspect Consistency Time", SLIDER, 0, (LONG_PTR*)&AspectSettings.AspectConsistencyTime,
        15, 0, 300, 5, 1,
        NULL,
        "Aspect_Detect", "AspectConsistencyTime", NULL,
    },
    {
        "Vert Image Pos", ITEMFROMLIST, 0, (LONG_PTR*)&AspectSettings.VerticalPos,
        VERT_POS_CENTRE, 0, 2, 1, 1,
        VertPosString,
        "Aspect", "VerticalPos", VertPos_OnChange,
    },
    {
        "Horiz Image Pos", ITEMFROMLIST, 0, (LONG_PTR*)&AspectSettings.HorizontalPos,
        HORZ_POS_CENTRE, 0, 2, 1, 1,
        HorzPosString,
        "Aspect", "HorizontalPos", HorizPos_OnChange,
    },
    {
        "Clipping", ONOFF, 0, (LONG_PTR*)&AspectSettings.AspectImageClipped,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Aspect", "Clipping", Clipping_OnChange,
    },
    {
        "Bounce", ONOFF, 0, (LONG_PTR*)&AspectSettings.BounceEnabled,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Aspect", "Bounce", Bounce_OnChange,
    },
    {
        "Bounce Period (Secs)", SLIDER, 0, (LONG_PTR*)&AspectSettings.BouncePeriod,
        60*30, 60, 60*600, 1, 1,
        NULL,
        "Aspect", "BouncePeriod", NULL,
    },
    {
        "Defer Setting Overlay", ONOFF, 0, (LONG_PTR*)&AspectSettings.DeferedSetOverlay,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Aspect", "DeferedOverlay", NULL,
    },
    {
        "Bounce Timer Period (ms)", SLIDER, 0, (LONG_PTR*)&AspectSettings.TimerBounceMS,
        1000, 100, 5000, 1, 1,
        NULL,
        "Aspect", "BounceTimerPeriod", NULL,
    },
    {
        "Bounce Amplitude (%)", SLIDER, 0, (LONG_PTR*)&AspectSettings.BounceAmplitude,
        100, 0, 100, 1, 1,
        NULL,
        "Aspect", "BounceAmplitude", NULL,
    },
    {
        "Orbit", ONOFF, 0, (LONG_PTR*)&AspectSettings.OrbitEnabled,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Aspect", "Orbit", Orbit_OnChange,
    },
    {
        "Orbit Period X", SLIDER, 0, (LONG_PTR*)&AspectSettings.OrbitPeriodX,
        60*45, 60, 60*600, 1, 1,
        NULL,
        "Aspect", "OrbitPeriodX", NULL,
    },
    {
        "Orbit Period Y", SLIDER, 0, (LONG_PTR*)&AspectSettings.OrbitPeriodY,
        60*60, 60, 60*600, 1, 1,
        NULL,
        "Aspect", "OrbitPeriodY", NULL,
    },
    {
        "Orbit Size", SLIDER, 0, (LONG_PTR*)&AspectSettings.OrbitSize,
        8, 0, 200, 1, 1,
        NULL,
        "Aspect", "OrbitSize", NULL,
    },
    {
        "Orbit Timer Period (ms)", SLIDER, 0, (LONG_PTR*)&AspectSettings.TimerOrbitMS,
        60*1000, 1000, 600 * 1000, 1, 1,
        NULL,
        "Aspect", "OrbitTimerPeriod", NULL,
    },
    {
        "Auto-Size Window", ONOFF, 0, (LONG_PTR*)&AspectSettings.AutoResizeWindow,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Aspect", "AutoSizeWindow", NULL,
    },
    {
        "Skip Percentage", SLIDER, 0, (LONG_PTR*)&AspectSettings.SkipPercent,
        17, 0, 49, 1, 1,
        NULL,
        "Aspect_Detect", "SkipPercent", NULL,
    },
    {
        "X Zoom Factor", SLIDER, 0, (LONG_PTR*)&AspectSettings.ZoomFactorX,
        100, 1, 1000, 1, 100,
        NULL,
        "Aspect", "XZoomFactor", XZoom_Factor_OnChange,
    },
    {
        "Y Zoom Factor", SLIDER, 0, (LONG_PTR*)&AspectSettings.ZoomFactorY,
        100, 1, 1000, 1, 100,
        NULL,
        "Aspect", "YZoomFactor", YZoom_Factor_OnChange,
    },
    {
        "X Zoom Center", SLIDER, 0, (LONG_PTR*)&AspectSettings.ZoomCenterX,
        50, -100, 200, 1, 100,
        NULL,
        "Aspect", "XZoomCenter", XZoom_Center_OnChange,
    },
    {
        "Y Zoom Center", SLIDER, 0, (LONG_PTR*)&AspectSettings.ZoomCenterY,
        50, -100, 200, 1, 100,
        NULL,
        "Aspect", "YZoomCenter", YZoom_Center_OnChange,
    },
    {
        "Chroma Range", SLIDER, 0, (LONG_PTR*)&AspectSettings.ChromaRange,
        16, 0, 255, 1, 1,
        NULL,
        "Aspect_Detect", "ChromaRange", ChromaRange_OnChange,
    },
    {
        "Wait for Vertical Blank While Drawing", ONOFF, 0, (LONG_PTR*)&AspectSettings.bWaitForVerticalBlank,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Aspect", "WaitForVerticalBlank", NULL,
    },
    {
        "Zoom Out Frame Count", SLIDER, 0, (LONG_PTR*)&AspectSettings.ZoomOutFrameCount,
        3, 1, 1000, 1, 1,
        NULL,
        "Aspect_Detect", "ZoomOutFrameCount", NULL,
    },
    {
        "Allow greater ratio than screen ratio", ONOFF, 0, (LONG_PTR*)&AspectSettings.bAllowGreaterThanScreen,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Aspect_Detect", "AllowGreaterThanScreen", NULL,
    },
    {
        "Mask Grey Shade", SLIDER, 0, (LONG_PTR*)&AspectSettings.MaskGreyShade,
        0, 0, 255, 1, 1,
        NULL,
        "Aspect_Detect", "MaskGreyShade", MaskGreyShade_OnChange,
    },
    {
        "Use WSS data", ONOFF, 0, (LONG_PTR*)&AspectSettings.bUseWSS,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Aspect_Detect", "UseWSS", NULL,
    },
    {
        "Default Source Aspect", SLIDER, 0, (LONG_PTR*)&AspectSettings.DefaultSourceAspect,
        1333, 1333, 3000, 1, 1000,
        NULL,
        "Aspect_Detect", "DefaultSourceAspect", NULL,
    },
    {
        "Default Aspect Mode", SLIDER, 0, (LONG_PTR*)&AspectSettings.DefaultAspectMode,
        1, 1, 2, 1, 1,
        NULL,
        "Aspect_Detect", "DefaultAspectMode", NULL,
    },
};


SmartPtr<CSettingsHolder> Aspect_GetSettingsHolder()
{
    SmartPtr<CSettingsHolder> AspectSettingsHolder(new CSettingsHolder(WM_ASPECT_GETVALUE));
    CSettingGroup *pRatioGroup = SettingsMaster->GetGroup(_T("Aspect - Aspect Ratio"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);
    CSettingGroup *pBounceGroup = SettingsMaster->GetGroup(_T("View - Bounce"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);
    CSettingGroup *pAutoSizeGroup = SettingsMaster->GetGroup(_T("View - AutoSize"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);
    CSettingGroup *pOrbitGroup = SettingsMaster->GetGroup(_T("View - Orbit"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);
    CSettingGroup *pImagePositionGroup = SettingsMaster->GetGroup(_T("View - Image position"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);
    CSettingGroup *pZoomGroup = SettingsMaster->GetGroup(_T("View - Zoom"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);
    CSettingGroup *pMiscGroup = SettingsMaster->GetGroup(_T("Aspect - Misc"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    CSettingGroup *pAspectDetectGroup = SettingsMaster->GetGroup(_T("Aspect Detect - AR Detect On Off"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);
    CSettingGroup *pAspectDetectSettingsGroup = SettingsMaster->GetGroup(_T("Aspect Detect - AR Detect Settings"), SETTING_BY_CHANNEL | SETTING_BY_FORMAT | SETTING_BY_INPUT, FALSE);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[SOURCE_ASPECT], pRatioGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[CUSTOM_SOURCE_ASPECT], pRatioGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[TARGET_ASPECT], pRatioGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[CUSTOM_TARGET_ASPECT], pRatioGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ASPECT_MODE], pRatioGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[VERTICALPOS], pImagePositionGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[HORIZONTALPOS], pImagePositionGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[BOUNCE], pBounceGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[BOUNCEPERIOD], pBounceGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[BOUNCETIMERPERIOD], pBounceGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[BOUNCEAMPLITUDE], pBounceGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[AUTOSIZEWINDOW], pAutoSizeGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[ORBIT], pOrbitGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ORBITPERIODX], pOrbitGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ORBITPERIODY], pOrbitGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ORBITSIZE], pOrbitGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ORBITTIMERPERIOD], pOrbitGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[XZOOMFACTOR], pZoomGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[YZOOMFACTOR], pZoomGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[XZOOMCENTER], pZoomGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[YZOOMCENTER], pZoomGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[CLIPPING], pMiscGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[DEFERSETOVERLAY], pMiscGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[WAITFORVERTBLANKINDRAW], pMiscGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[AUTODETECTASPECT], pAspectDetectGroup);

    AspectSettingsHolder->AddSetting(&AspectGUISettings[LUMINANCETHRESHOLD], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[IGNORENONBLACKPIXELS], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ZOOMINFRAMECOUNT], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ASPECTHISTORYTIME], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ASPECTCONSISTENCYTIME], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[SKIPPERCENT], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[CHROMARANGE], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ZOOMOUTFRAMECOUNT], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[ALLOWGREATERTHANSCREEN], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[MASKGREYSHADE], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[USEWSS], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[DEFAULTSOURCEASPECT], pAspectDetectSettingsGroup);
    AspectSettingsHolder->AddSetting(&AspectGUISettings[DEFAULTASPECTMODE], pAspectDetectSettingsGroup);

#ifdef _DEBUG
    if (ASPECT_SETTING_LASTONE != AspectSettingsHolder->GetNumSettings())
    {
        LOGD(_T("Number of settings in Aspect source is not equal to the number of settings in DS_Control.h\n"));
        LOGD(_T("DS_Control.h or AspectGui.cpp are probably not in sync with each other.\n"));
    }
#endif
    return AspectSettingsHolder;
}

void Aspect_FinalSetup()
{
    Bounce_OnChange(AspectSettings.BounceEnabled);
    Orbit_OnChange(AspectSettings.OrbitEnabled);
}
