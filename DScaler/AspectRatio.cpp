/////////////////////////////////////////////////////////////////////////////
// $Id: AspectRatio.cpp,v 1.48 2005-03-23 14:20:36 adcockj Exp $
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
//                                     So that m_Overscan does not effect Ratio
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.47  2003/10/27 10:39:50  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.46  2003/08/09 15:53:39  laurentg
// Bad refresh of the toolbar when in full screen mode corrected
//
// Revision 1.45  2003/04/30 17:30:51  laurentg
// Init of custom aspect ratio fixed
//
// Revision 1.44  2003/03/29 13:37:51  laurentg
// Allow the display of DScaler to monitors other than the primary
//
// Revision 1.43  2003/03/22 15:41:58  laurentg
// Half height deinterlace modes correctly handled in previow mode
// Center of the image in its frame with black borders
//
// Revision 1.42  2003/03/16 18:31:24  laurentg
// New multiple frames feature
//
// Revision 1.41  2003/01/07 23:27:01  laurentg
// New overscan settings
//
// Revision 1.40  2003/01/04 13:36:42  laurentg
// Two modes for AR autodetection
//
// Revision 1.39  2003/01/03 00:54:19  laurentg
// New mode for AR autodetection using only WSS
//
// Revision 1.38  2002/10/31 14:03:33  adcockj
// Added Analogue blanking option to aspect code
//
// Revision 1.37  2002/09/18 11:38:05  kooiman
// Preparations for skinned dscaler look.
//
// Revision 1.36  2002/08/12 21:29:58  laurentg
// Disable AR autodetection when switching to square pixel mode but restore it when exiting square pixel mode
//
// Revision 1.35  2002/08/05 21:01:55  laurentg
// Square pixels mode updated
//
// Revision 1.34  2002/06/24 21:49:28  laurentg
// New option to use or not WSS data when doing AR detection
//
// Revision 1.33  2002/05/30 13:06:41  robmuller
// Removed variable bIgnoreMouse.
//
// Revision 1.32  2002/04/28 16:43:37  laurentg
// New setting for aspect ratio detect
//
// Revision 1.31  2002/04/24 19:09:14  tobbej
// added missing \n
//
// Revision 1.30  2002/04/13 18:56:22  laurentg
// Checks added to manage case where the current source is not yet defined
//
// Revision 1.29  2002/02/25 23:03:51  tobbej
// added a check for pSource==NULL in WorkoutOverlaySize
// added logging
//
// Revision 1.28  2002/02/23 19:07:06  laurentg
// New AR mode for stills having square pixels
//
// Revision 1.27  2002/02/23 12:00:13  laurentg
// Do nothing in WorkoutOverlaySize when source width or height is null
//
// Revision 1.26  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.25  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.24  2001/11/26 13:02:27  adcockj
// Bug Fixes and standards changes
//
// Revision 1.23  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.22  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.21  2001/11/02 16:30:06  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.20  2001/10/18 16:20:40  adcockj
// Made Color of blanking adjustable
//
// Revision 1.19.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.19  2001/08/09 12:21:40  adcockj
// Structure name changes
//
// Revision 1.18  2001/08/06 22:32:13  laurentg
// Little improvments for AR autodetection
//
// Revision 1.17  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.16  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

/**
 * @file AspectRatio.cpp Aspect Ratio Functions
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Other.h"
#include "AspectRatio.h"
#include "DebugLog.h"
#include "DScaler.h"
#include "AspectFilters.h"
#include "Deinterlace.h"
#include "Providers.h"
#include "MultiFrames.h"

#define AR_STRETCH       0
#define AR_NONANAMORPHIC 1
#define AR_ANAMORPHIC    2


TAspectSettings AspectSettings =
{
    1333,0,1,1335,1335,40,16,TRUE,3,17,0,60,3,300,15,20,
    2000,VERT_POS_CENTRE,HORZ_POS_CENTRE,
    {0,0,0,0},
    {0,0,0,0},
    {0,0,0,0},
    TRUE,FALSE,4,4,4,4,TRUE,FALSE,
    0,60*30,1000,100,FALSE,8,60,60,1000,FALSE,FALSE,
    100,100,50,50,
    FALSE,
    1000,
    0,
    FALSE,
    FALSE,
    1333,
	1,
    FALSE,
};

BOOL Bounce_OnChange(long NewValue); // Forward declaration to reuse this code...
BOOL Orbit_OnChange(long NewValue); // Forward declaration to reuse this code...

//----------------------------------------------------------------------------
// Calculate the actual aspect ratio of the source frame, independent of
// grab or display resolutions.
// JA 2001/07/10 added support for filters changing overlay position
double GetActualSourceFrameAspect()
{
    double SourceAdjust = (double)AspectSettings.SourceAspectAdjust / 1000.0;
    switch (AspectSettings.AspectMode)
    {
    case 1:
        // Letterboxed or full-frame
        return SourceAdjust * 1.333; //4.0/3.0;
    case 2:
        // Anamorphic
        return SourceAdjust * 1.778; //16.0/9.0;
    default:
        // User-specified
        return (SourceAdjust * (double)AspectSettings.SourceAspect)/1000.0;
    }
}

//----------------------------------------------------------------------------
// Calculate size and position coordinates for video overlay
// Takes into account of aspect ratio control.
void WorkoutOverlaySize(BOOL ForceRedraw, BOOL allowResize)
{
    // be very careful here as we don't want to cause a deadlock
    // we need to interact with the mian window in here but we need to
    // do so in a way that halts processing and also copes with the window thread being busy
    // while we want to do this
    if(!IsMainWindowThread())
    {
        ResetMainWindowEvent();
        PostMessageToMainWindow(UWM_OVERLAY_UPDATE, ForceRedraw, allowResize);
        WaitForMainWindowEvent();
    }
    else
    {
        CFilterChain FilterChain;
        int SourceHeight;
        int SourceWidth;

        static BOOL InFunction = FALSE;
        if(InFunction == TRUE) return;

	    CheckChangeMonitor(GetMainWnd());
	    RECT ScreenRect;
	    GetMonitorRect(GetMainWnd(), &ScreenRect);

        CAspectRectangles ar;
        CSource* pSource = Providers_GetCurrentSource();

	    if (pMultiFrames && pMultiFrames->IsActive())
	    {
		    SourceHeight = pMultiFrames->GetHeight();
		    SourceWidth = pMultiFrames->GetWidth();
	    }
	    else if (pSource)
	    {
		    SourceHeight = pSource->GetHeight();
		    SourceWidth = pSource->GetWidth();
	    }
	    else
	    {
		    SourceHeight = 720;
		    SourceWidth = 480;
	    }
        // If source width or source height is null, we do nothing
        if (SourceWidth == 0 || SourceHeight == 0)
        {
            LOG(2,"Zero height or width in WorkoutOverlaySize!");
            LOGD("Zero height or width in WorkoutOverlaySize!\n");
            return;
        }

        InFunction = TRUE;
        UpdateWindowState();

        // Setup the rectangles...
        // Previous ones...
        ar.m_PrevDestRect = AspectSettings.DestinationRect;
        ar.m_PrevSrcRect = AspectSettings.SourceRect;
        // Source frame
        ar.m_OriginalOverlaySrcRect.left = 0;
        ar.m_OriginalOverlaySrcRect.right = SourceWidth;
        ar.m_OriginalOverlaySrcRect.top = 0;
        ar.m_OriginalOverlaySrcRect.bottom = SourceHeight;
        // Set the aspect adjustment factor...
        if (AspectSettings.SquarePixels)
        {
            ar.m_OriginalOverlaySrcRect.setAspectAdjust((double)SourceWidth/(double)SourceHeight,
                                                    (double)SourceWidth/(double)SourceHeight);
        }
        else
        {
            ar.m_OriginalOverlaySrcRect.setAspectAdjust((double)SourceWidth/(double)SourceHeight,
                                                    GetActualSourceFrameAspect());
        }

        // Destination rectangle
        //ar.m_OriginalOverlayDestRect.setToClient(hWnd,TRUE);
        GetDisplayAreaRect(GetMainWnd(), &ar.m_OriginalOverlayDestRect, TRUE);
        ClientToScreen(GetMainWnd(), (POINT*) &ar.m_OriginalOverlayDestRect.left);
        ClientToScreen(GetMainWnd(), (POINT*) &ar.m_OriginalOverlayDestRect.right);

        // Set the aspect adjustment factor if the screen aspect is specified...
        if (AspectSettings.TargetAspect)
        {
            ar.m_OriginalOverlayDestRect.setAspectAdjust((double) (ScreenRect.right  - ScreenRect.left) / (double) (ScreenRect.bottom - ScreenRect.top),
                                                    AspectSettings.TargetAspect/1000.0);
        }

        // Set current values to original for starters...
        ar.m_CurrentOverlaySrcRect = ar.m_OriginalOverlaySrcRect;
        ar.m_CurrentOverlayDestRect = ar.m_OriginalOverlayDestRect;

        // Build filter chain and apply
        /// \todo Filter chain should be saved and only rebuilt if options are changed
        FilterChain.BuildFilterChain(SourceWidth, SourceHeight);
        if (FilterChain.ApplyFilters(ar, allowResize))
        {
            InFunction = FALSE;
            WorkoutOverlaySize(FALSE, FALSE); // Prevent further recursion - only allow 1 level of request for readjusting the overlay
            return;
        }

        // If we're in half-height Mode, squish the source rectangle accordingly.  This
        // allows the overlay hardware to do our bobbing for us.
	    if ( (!pMultiFrames || !pMultiFrames->IsActive())
        && (InHalfHeightMode() == TRUE) )
        {
            ar.m_CurrentOverlaySrcRect.top /= 2;
            ar.m_CurrentOverlaySrcRect.bottom /= 2;
        }

        // Save the settings....
        AspectSettings.SourceRect = ar.m_CurrentOverlaySrcRect;
        AspectSettings.DestinationRectWindow = ar.m_CurrentOverlayDestRect;
        AspectSettings.DestinationRect = ar.m_CurrentOverlayDestRect;
        ScreenToClient(GetMainWnd(), ((PPOINT)&AspectSettings.DestinationRect.left));
        ScreenToClient(GetMainWnd(), ((PPOINT)&AspectSettings.DestinationRect.right));

	    // cut the on-screen overlay surface out of the monitor screen area
	    if (AspectSettings.DestinationRectWindow.right > ScreenRect.right)
	    {
		    AspectSettings.DestinationRectWindow.right = ScreenRect.right;
	    }

	    if (AspectSettings.DestinationRectWindow.left < ScreenRect.left)
	    {
		    AspectSettings.DestinationRectWindow.left = ScreenRect.left;
	    }

	    if (AspectSettings.DestinationRectWindow.top < ScreenRect.top)
	    {
		    AspectSettings.DestinationRectWindow.top = ScreenRect.top;
	    }

	    if (AspectSettings.DestinationRectWindow.bottom > ScreenRect.bottom)
	    {
		    AspectSettings.DestinationRectWindow.bottom = ScreenRect.bottom;
	    }
	    AspectSettings.DestinationRectWindow.left   -= ScreenRect.left;
	    AspectSettings.DestinationRectWindow.right  -= ScreenRect.left;
	    AspectSettings.DestinationRectWindow.top    -= ScreenRect.top;
	    AspectSettings.DestinationRectWindow.bottom -= ScreenRect.top;

        // Invert the rectangle if necessary...
        //
        // Removed for now - seems to cause application to crash - looks
        // like video drivers can't deal with this...
        /*
        if (( AspectSettings.invertX && AspectSettings.SourceRect.right > AspectSettings.SourceRect.left)
            ||
            (!AspectSettings.invertX && AspectSettings.SourceRect.right < AspectSettings.SourceRect.left))
        {
            int t = AspectSettings.SourceRect.right;
            AspectSettings.SourceRect.right = AspectSettings.SourceRect.left;
            AspectSettings.SourceRect.left = t;
        }
        if (( AspectSettings.invertY && AspectSettings.SourceRect.bottom > AspectSettings.SourceRect.top)
            ||
            (!AspectSettings.invertY && AspectSettings.SourceRect.bottom < AspectSettings.SourceRect.top)) {
            int t = AspectSettings.SourceRect.top;
            AspectSettings.SourceRect.top = AspectSettings.SourceRect.bottom;
            AspectSettings.SourceRect.bottom = t;
        }
        */

        if(ForceRedraw)
        {
            // if we have changed source recatangle then we seem to need
            // to do a hide first
            // at least on nVidia cards
            if (memcmp(&ar.m_PrevSrcRect,&AspectSettings.SourceRect,sizeof(ar.m_PrevSrcRect)))
            {
                Overlay_Update(NULL, NULL, DDOVER_HIDE);
            }

            Overlay_Update(&AspectSettings.SourceRect, &AspectSettings.DestinationRectWindow, DDOVER_SHOW);

            InvalidateRect(GetMainWnd(), NULL, FALSE);
        }
        else
        {
            // Set the overlay
            if (!AspectSettings.DeferedSetOverlay) // MRS 2-22-01 - Defered overlay set
            {
                // if we have changed source recatangle then we seem to need
                // to do a hide first
                // at least on nVidia cards
                if (memcmp(&ar.m_PrevSrcRect,&AspectSettings.SourceRect,sizeof(ar.m_PrevSrcRect)))
                {
                    Overlay_Update(NULL, NULL, DDOVER_HIDE);
                }

                Overlay_Update(&AspectSettings.SourceRect, &AspectSettings.DestinationRectWindow, DDOVER_SHOW);

                // repaint if needed
                if (memcmp(&ar.m_PrevDestRect,&AspectSettings.DestinationRect,sizeof(ar.m_PrevDestRect)))
                {
                    // MRS 2-22-01 Invalidate just the union of the old region and the new region - no need to invalidate all of the window.
                    RECT invalidate;
                    UnionRect(&invalidate,&ar.m_PrevDestRect,&AspectSettings.DestinationRect);
                    InvalidateRect(GetMainWnd(), &invalidate, FALSE);
                }
            }
            else
            {
                // repaint now and set off overlay setting for later
                if (memcmp(&ar.m_PrevDestRect,&AspectSettings.DestinationRect,sizeof(ar.m_PrevDestRect)))
                {
                    // MRS 2-22-01 Invalidate just the union of the old region and the new region - no need to invalidate all of the window.
                    RECT invalidate;
                    UnionRect(&invalidate,&ar.m_PrevDestRect,&AspectSettings.DestinationRect);
                    InvalidateRect(GetMainWnd(), &invalidate, FALSE);
                    AspectSettings.OverlayNeedsSetting = TRUE;
                }
                else
                {
                    // if we have changed source recatangle then we seem to need
                    // to do a hide first
                    // at least on nVidia cards
                    if (memcmp(&ar.m_PrevSrcRect,&AspectSettings.SourceRect,sizeof(ar.m_PrevSrcRect)))
                    {
                        Overlay_Update(NULL, NULL, DDOVER_HIDE);
                    }
                    // If not invalidating, we need to update the overlay now...
                    Overlay_Update(&AspectSettings.SourceRect, &AspectSettings.DestinationRectWindow, DDOVER_SHOW);
                    AspectSettings.OverlayNeedsSetting = FALSE;
                }
            }
        }

        InFunction = FALSE;
    }
}

//----------------------------------------------------------------------------
// Returns the current source rectangle.
void GetSourceRect(RECT* rect)
{
    memcpy(rect, &AspectSettings.SourceRect, sizeof(RECT));
}

//----------------------------------------------------------------------------
// Returns the current source rectangle.
void GetDestRect(RECT* rect)
{
    memcpy(rect, &AspectSettings.DestinationRect, sizeof(RECT));
}

int UpdateSquarePixelsMode(BOOL set)
{
    static int SavedSourceAspect = -1;
    static int SavedAspectMode = -1;
    static int SavedAutodetectAspect = 0;
    int result = 0;

    if (set)
    {
        if (!AspectSettings.SquarePixels)
        {
            SavedSourceAspect = AspectSettings.SourceAspect;
            SavedAspectMode = AspectSettings.AspectMode;
            SavedAutodetectAspect = AspectSettings.AutoDetectAspect;
            result = 1;
        }
        AspectSettings.AutoDetectAspect = 0;
		if (pMultiFrames && pMultiFrames->IsActive())
		{
            int width = pMultiFrames->GetWidth();
            int height = pMultiFrames->GetHeight();
            if (height != 0)
            {
                AspectSettings.SourceAspect = width * 1000 / height;
                AspectSettings.AspectMode = AR_NONANAMORPHIC;
                result = 1;
            }
		}
        else if (Providers_GetCurrentSource() != NULL)
        {
            int width = Providers_GetCurrentSource()->GetWidth();
            int height = Providers_GetCurrentSource()->GetHeight();
            if (height != 0)
            {
                AspectSettings.SourceAspect = width * 1000 / height;
                AspectSettings.AspectMode = AR_NONANAMORPHIC;
                result = 1;
            }
        }
    }
    else
    {
        if (AspectSettings.SquarePixels)
        {
            AspectSettings.SourceAspect = SavedSourceAspect;
            AspectSettings.AspectMode = SavedAspectMode;
            AspectSettings.AutoDetectAspect = SavedAutodetectAspect;
            result = 1;
        }
    }
    AspectSettings.SquarePixels = set;
    return result;
}