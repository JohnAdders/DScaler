/////////////////////////////////////////////////////////////////////////////
// AspectRatio.cpp
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
//									   Added prototype image bouncing code
//									     (currently has issues with purple flashing)
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
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Other.h"
#include "AspectRatio.h"
#include "Resource.h"
#include "DebugLog.h"
#include "Status.h"
#include "BT848.h"
#include "DScaler.h"
#include "AspectRect.h"

// From dtv.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern HMENU hMenu;
extern BOOL  bIsFullScreen;
extern void ShowText(HWND hWnd, LPCTSTR szText);
extern BOOL Show_Menu;

int HalfHeight = FALSE;

#include "AspectFilters.h"

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
    FALSE
};

BOOL Bounce_OnChange(long NewValue); // Forward declaration to reuse this code...
BOOL Orbit_OnChange(long NewValue); // Forward declaration to reuse this code...

//----------------------------------------------------------------------------
// Calculate the actual aspect ratio of the source frame, independent of
// grab or display resolutions.
double GetActualSourceFrameAspect()
{
	switch (aspectSettings.aspect_mode) 
	{
	case 1:
		// Letterboxed or full-frame
		return 1.333; //4.0/3.0;
	case 2:
		// Anamorphic
		return 1.778; //16.0/9.0;
	default:
		// User-specified
		return aspectSettings.source_aspect/1000.0;
	}
}

//----------------------------------------------------------------------------
// Enter or leave half-height mode.
// True if we're in half-height mode (even or odd scanlines only).
void SetHalfHeight(int IsHalfHeight)
{
	if (IsHalfHeight != HalfHeight)
	{
		HalfHeight = IsHalfHeight;
		WorkoutOverlaySize();
	}
}

CAspectFilter* BuildFilterChain()
{
	CAspectFilter *head, *cur;
	if (aspectSettings.orbitEnabled)
	{ 
		int overscan = aspectSettings.InitialOverscan;
		if (aspectSettings.orbitEnabled && overscan*2 < aspectSettings.orbitSize)
		{
			overscan = (aspectSettings.orbitSize+1)/2;
		}
		head = cur = new COverscanAspectFilter(aspectSettings.InitialOverscan);
		cur->next = new COrbitAspectFilter(aspectSettings.orbitPeriodX, aspectSettings.orbitPeriodY, aspectSettings.orbitSize); 
		cur = cur->next;
	}
	else 
	{
		head = cur = new COverscanAspectFilter(aspectSettings.InitialOverscan);
	}
	if (aspectSettings.aspect_mode)
	{ 
		CAspectFilter *position;
		
		if (aspectSettings.bounceEnabled)
		{
			position = new CBounceDestinationAspectFilter(aspectSettings.bouncePeriod);
		}
		else
		{
			double xPos, yPos;
			switch (aspectSettings.HorizontalPos)
			{
			case HORZ_POS_LEFT:
				xPos = -1; 
				break;
			case HORZ_POS_RIGHT:
				xPos = 1; 
				break;
			default: 
				xPos = 0; 
				break;
			}
			switch (aspectSettings.VerticalPos)
			{
			case VERT_POS_TOP: 
				yPos = -1;
				break;
			case VERT_POS_BOTTOM: 
				yPos = 1; 
				break;
			default:
				yPos = 0; 
				break;
			}
			position = new CPositionDestinationAspectFilter(xPos,yPos);
		}
		
		cur->next = position;
		cur->next->firstChild = new CCropAspectFilter();

		if (!aspectSettings.aspectImageClipped)
		{
			CAspectFilter *uncrop = new CUnCropAspectFilter(); 
			uncrop->firstChild = cur->next;
			cur->next = uncrop;
		}
		cur = cur->next;
	}

	// This like is where image zooming would be implemented...
	// Sample code zooms in 2x on the center of the image
	// See comments in AspectFilters.hpp for PanAndZoomAspectFilter details.
	if (aspectSettings.xZoomFactor != 100 && aspectSettings.yZoomFactor != 100)
	{
		cur->next = new CPanAndZoomAspectFilter(aspectSettings.xZoomCenter,
											   aspectSettings.yZoomCenter,
											   aspectSettings.xZoomFactor,
											   aspectSettings.yZoomFactor);
		cur = cur->next;
	}

	cur->next = new CScreenSanityAspectFilter();
	cur = cur->next;
	if (aspectSettings.autoResizeWindow)
	{
		cur->next = new CResizeWindowAspectFilter();
		cur = cur->next;
	}
	
	return head;
}

//----------------------------------------------------------------------------
// Calculate size and position coordinates for video overlay
// Takes into account of aspect ratio control.
void WorkoutOverlaySize(BOOL allowResize)
{
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
	CAspectFilter *head = BuildFilterChain();
	if (head->applyFilters(ar, allowResize))
	{
		delete head;
		InFunction = FALSE;
		WorkoutOverlaySize(FALSE); // Prevent further recursion - only allow 1 level of request for readjusting the overlay
		return;
	} 
	else 
	{
		delete head;
	}

	// If we're in half-height mode, squish the source rectangle accordingly.  This
	// allows the overlay hardware to do our bobbing for us.
	if (HalfHeight)
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
		Overlay_Update(&aspectSettings.sourceRectangle, &aspectSettings.destinationRectangleWindow, DDOVER_SHOW);
	}
	else
	{
		aspectSettings.overlayNeedsSetting = TRUE;
	}

	// Save the Overlay Destination and force a repaint 
	// MRS 2-23-01 Only invalidate if we changed something
	if (memcmp(&ar.rPrevDest,&aspectSettings.destinationRectangle,sizeof(ar.rPrevDest)))
	{ 
		// MRS 2-22-01 Invalidate just the union of the old region and the new region - no need to invalidate all of the window.
		RECT invalidate;
		UnionRect(&invalidate,&ar.rPrevDest,&aspectSettings.destinationRectangle);
		InvalidateRect(hWnd,&invalidate,FALSE);
	}
	else if (aspectSettings.overlayNeedsSetting)
	{
		// If not invalidating, we need to update the overlay now...
		Overlay_Update(&aspectSettings.sourceRectangle, &aspectSettings.destinationRectangleWindow, DDOVER_SHOW);
		aspectSettings.overlayNeedsSetting = FALSE;
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

