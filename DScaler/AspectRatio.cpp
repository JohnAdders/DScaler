/////////////////////////////////////////////////////////////////////////////
// AspectRatio.cpp
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
// Aspect ratio contrl was started by Michael Samblanet <mike@cardobe.com>
// Moved into separate module by Mark D Rejhon.  
//
// The purpose of this module is all the calculations and handling necessary
// to map the source image onto the destination display, even if they are
// different aspect ratios.
//
// Portions copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
// Portions copyright (C) 2000 John Adcock
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2000   Michael Samblanet     Aspect ratio code contributed 
//                                     to dTV project
//
// 12 Sep 2000   Mark Rejhon           Centralized aspect ratio code
//                                     into separate module
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 11 Jan 2001   John Adcock           Added Code for window position
//                                     Changed SourceFrameAspect to use
//                                     CurrentX/Y rather than SourceRect
//                                     So that overscan does not effect Ratio
//                                     Fixed bug in FindBottomOfImage when
//                                     Overscan is 0
//
// 21 Feb 2001   Michael Samblanet     Added 1.44 and 1.55 ratio support
//                                     Added code to handle clipped menu item
//                                     Added prototype image bouncing code
//                                       (currently has issues with purple flashing)
//
// 22 Feb 2001   Michael Samblanet     Added defered setting of overlay region to
//                                     avoid purple flashing
//                                     Made bounce timer a ini setting and changed to 1sec default
//
// 23 Feb 2001   Michael Samblanet     Added experemental orbiting code
// 24 Feb 2001   Michael Samblanet     Minor bug fixes to invalidate code
// 10 Mar 2001   Michael Samblanet     Added first draft auto-resize window code
// 11 Mar 2001   Michael Samblanet     Converted to C++ file, initial pass at reworking the aspect calculation code
// 23 Mar 2001   Michael Samblanet     Reworked code to use filter chain, major testing and debugging
// 03 May 2001   Michael Samblanet     Moved half-height logic to correct location in aspect code
//                                     Expieremental inversion code (invert source rect if options are enabled)
// 07 May 2001   John Adcock           Reformmated code
// 08 Jun 2001   Eric Schmidt          Added bounce amplitude to ini
// 03 Jul 2001   John Adcock           Cleaned up Filter chain code
//                                     Changed setings and behaviour for pan and scan
//                                     to allow a variety of settings
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Other.h"
#include "AspectRatio.h"
#include "Resource.h"
#include "DebugLog.h"
#include "Status.h"
#include "BT848.h"
#include "DScaler.h"
#include "AspectFilters.h"
#include "Deinterlace.h"

#define AR_STRETCH       0
#define AR_NONANAMORPHIC 1
#define AR_ANAMORPHIC    2


TAspectSettingsStruct aspectSettings = 
{
    1333,0,1,0,0,40,16,1,17,FALSE,60,300,15,20,
    2000,VERT_POS_CENTRE,HORZ_POS_CENTRE,
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    TRUE,FALSE,4,TRUE,FALSE,
    0,60*30,1000,100,FALSE,8,60,60,1000,FALSE,FALSE,
    100,100,50,50,
    FALSE,
    1000
};

BOOL Bounce_OnChange(long NewValue); // Forward declaration to reuse this code...
BOOL Orbit_OnChange(long NewValue); // Forward declaration to reuse this code...

//----------------------------------------------------------------------------
// Calculate the actual aspect ratio of the source frame, independent of
// grab or display resolutions.
// JA 2001/07/10 added support for filters changing overlay position
double GetActualSourceFrameAspect()
{
    double SourceAdjust = (double)aspectSettings.source_aspect_adjust / 1000.0;
    switch (aspectSettings.aspect_mode) 
    {
    case 1:
        // Letterboxed or full-frame
        return SourceAdjust * 1.333; //4.0/3.0;
    case 2:
        // Anamorphic
        return SourceAdjust * 1.778; //16.0/9.0;
    default:
        // User-specified
        return (SourceAdjust * (double)aspectSettings.source_aspect)/1000.0;
    }
}

//----------------------------------------------------------------------------
// Calculate size and position coordinates for video overlay
// Takes into account of aspect ratio control.
void WorkoutOverlaySize(BOOL allowResize)
{
    CFilterChain FilterChain;
    bIgnoreMouse = TRUE;

    static BOOL InFunction = FALSE;
    if(InFunction == TRUE) return;

    InFunction = TRUE;
    UpdateWindowState();

    CAspectRectangles ar;

    // Setup the rectangles...
    // Previous ones...
    ar.rPrevDest = aspectSettings.destinationRectangle;
    ar.rPrevSrc = aspectSettings.sourceRectangle;
    // Source frame
    ar.rOriginalOverlaySrc.left = 0;
    ar.rOriginalOverlaySrc.right = CurrentX;
    ar.rOriginalOverlaySrc.top = 0;
    ar.rOriginalOverlaySrc.bottom = CurrentY;
    // Set the aspect adjustment factor...
    ar.rOriginalOverlaySrc.setAspectAdjust((double)CurrentX/(double)CurrentY,
                                            GetActualSourceFrameAspect());

    // Destination rectangle
    ar.rOriginalOverlayDest.setToClient(hWnd,TRUE);
    
    // Adjust for status bar...
    if (IsStatusBarVisible())
    {
        ar.rOriginalOverlayDest.bottom -= StatusBar_Height();
    }

    // Set the aspect adjustment factor if the screen aspect is specified...
    if (aspectSettings.target_aspect)
    {
        ar.rOriginalOverlayDest.setAspectAdjust((double) GetSystemMetrics(SM_CXSCREEN) / (double) GetSystemMetrics(SM_CYSCREEN),
                                                 aspectSettings.target_aspect/1000.0);
    }
    
    // Set current values to original for starters...
    ar.rCurrentOverlaySrc = ar.rOriginalOverlaySrc;
    ar.rCurrentOverlayDest = ar.rOriginalOverlayDest;

    // Build filter chain and apply
    // TODO: Filter chain should be saved and only rebuilt if options are changed
    FilterChain.BuildFilterChain();
    if (FilterChain.ApplyFilters(ar, allowResize))
    {
        InFunction = FALSE;
        WorkoutOverlaySize(FALSE); // Prevent further recursion - only allow 1 level of request for readjusting the overlay
        return;
    }

    // If we're in half-height mode, squish the source rectangle accordingly.  This
    // allows the overlay hardware to do our bobbing for us.
    if (InHalfHeightMode() == TRUE)
    {
        ar.rCurrentOverlaySrc.top /= 2;
        ar.rCurrentOverlaySrc.bottom /= 2;
    }

    // Save the settings....
    aspectSettings.sourceRectangle = ar.rCurrentOverlaySrc;
    aspectSettings.destinationRectangleWindow = ar.rCurrentOverlayDest; 
    aspectSettings.destinationRectangle = ar.rCurrentOverlayDest;
    ScreenToClient(hWnd,((PPOINT)&aspectSettings.destinationRectangle.left));
    ScreenToClient(hWnd,((PPOINT)&aspectSettings.destinationRectangle.right));

    // Invert the rectangle if necessary...
    /*
    // Removed for now - seems to cause application to crash - looks
    // like video drivers can't deal with this...
    if (( aspectSettings.invertX && aspectSettings.sourceRectangle.right > aspectSettings.sourceRectangle.left) 
        ||
        (!aspectSettings.invertX && aspectSettings.sourceRectangle.right < aspectSettings.sourceRectangle.left)) {
        int t = aspectSettings.sourceRectangle.right;
        aspectSettings.sourceRectangle.right = aspectSettings.sourceRectangle.left;
        aspectSettings.sourceRectangle.left = t;
    }
    if (( aspectSettings.invertY && aspectSettings.sourceRectangle.bottom > aspectSettings.sourceRectangle.top) 
        ||
        (!aspectSettings.invertY && aspectSettings.sourceRectangle.bottom < aspectSettings.sourceRectangle.top)) {
        int t = aspectSettings.sourceRectangle.top;
        aspectSettings.sourceRectangle.top = aspectSettings.sourceRectangle.bottom;
        aspectSettings.sourceRectangle.bottom = t;
    }
    */

    

    // Set the overlay
    if (!aspectSettings.deferedSetOverlay) // MRS 2-22-01 - Defered overlay set
    {
        // if we have changed source recatangle then we seem to need
        // to do a hide first
        // at least on nVidia cards
        if (memcmp(&ar.rPrevSrc,&aspectSettings.sourceRectangle,sizeof(ar.rPrevSrc)))
        {
            Overlay_Update(NULL, NULL, DDOVER_HIDE);
        }

        Overlay_Update(&aspectSettings.sourceRectangle, &aspectSettings.destinationRectangleWindow, DDOVER_SHOW);

        // repaint if needed
        if (memcmp(&ar.rPrevDest,&aspectSettings.destinationRectangle,sizeof(ar.rPrevDest)))
        { 
            // MRS 2-22-01 Invalidate just the union of the old region and the new region - no need to invalidate all of the window.
            RECT invalidate;
            UnionRect(&invalidate,&ar.rPrevDest,&aspectSettings.destinationRectangle);
            InvalidateRect(hWnd,&invalidate,FALSE);
        }
    }
    else
    {
        // repaint now and set off overlay setting for later
        if (memcmp(&ar.rPrevDest,&aspectSettings.destinationRectangle,sizeof(ar.rPrevDest)))
        { 
            // MRS 2-22-01 Invalidate just the union of the old region and the new region - no need to invalidate all of the window.
            RECT invalidate;
            UnionRect(&invalidate,&ar.rPrevDest,&aspectSettings.destinationRectangle);
            InvalidateRect(hWnd,&invalidate,FALSE);
            aspectSettings.overlayNeedsSetting = TRUE;
        }
        else
        {
            // if we have changed source recatangle then we seem to need
            // to do a hide first
            // at least on nVidia cards
            if (memcmp(&ar.rPrevSrc,&aspectSettings.sourceRectangle,sizeof(ar.rPrevSrc)))
            {
                Overlay_Update(NULL, NULL, DDOVER_HIDE);
            }
            // If not invalidating, we need to update the overlay now...
            Overlay_Update(&aspectSettings.sourceRectangle, &aspectSettings.destinationRectangleWindow, DDOVER_SHOW);
            aspectSettings.overlayNeedsSetting = FALSE;
        }
    }

    InFunction = FALSE;
}

//----------------------------------------------------------------------------
// Returns the current source rectangle.
void GetSourceRect(RECT *rect)
{
    memcpy(rect, &aspectSettings.sourceRectangle, sizeof(RECT));
}

//----------------------------------------------------------------------------
// Returns the current source rectangle.
void GetDestRect(RECT *rect)
{
    memcpy(rect, &aspectSettings.destinationRectangle, sizeof(RECT));
}

