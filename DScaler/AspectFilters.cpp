/////////////////////////////////////////////////////////////////////////////
// AspectFilters.cpp
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Michael Samblanet.  All rights reserved.
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
// 03 Jul 2001   John Adcock           Put Filter code in a separate file
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AspectFilters.h"
#include "AspectRatio.h"
#include "Other.h"
#include "DScaler.h"
#include "Status.h"

// From dtv.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern BOOL bIsFullScreen;
extern BOOL Show_Menu;


#define __ASPECTFILTER_DEBUG__ "CAspectFilterDebug.log"
#ifdef __ASPECTFILTER_DEBUG__
	static FILE *debugLog = NULL;
#endif

/* Used to calculate positions for a given period and timing.
	Values go from Amplitude/2+Offset to Amplitude+Offset, down to Offset, and then back to Amplitude/2+Offset
	This results in a bounce between Offset and Amplitude+Offset, starting at the midpoint.
*/

CPeriodBouncer::CPeriodBouncer(time_t period, double amplitude, double offset)
{ 
	time(&m_startTime); 
	m_period = period; 
	m_amplitude = amplitude; 
	m_offset = offset; 
}

CPeriodBouncer::CPeriodBouncer(time_t startTime, time_t period, double amplitude, double offset)
{ 
	m_startTime = startTime; 
	m_period = period; 
	m_amplitude = amplitude; 
	m_offset = offset;
}

double CPeriodBouncer::position()
{
	double phase = fmod((((double)((time(NULL)-m_startTime)%m_period))/m_period+0.25),1);
		// We go from 0% to 100% over m_period - but start 25% of the way into the phase.
	double val = phase*m_amplitude*2.0;
	if (val > m_amplitude)
	{
		val = fabs(2*m_amplitude-val); // Adjust - 50% into the phase, turn around and go down
	}
	return val + m_offset;
}

/* Class containing all the rectangles a filter might want 
Dest rectanges refer to the image rectangle on the screen
Src rectangles refer to the rectangle of the source image being used

Prev rectangles refer to the values from before this current filter run
Original rectangles refer to the initial values of "Current" passed into the filter chain
Current rectangles are the value being used and adjusted.

ONLY CURRENT VALUES SHOULD BE ADJUSTED BY FILTERS!
*/

void CAspectRectangles::DebugDump(FILE *f)
{
	fprintf(f,"SRC : "); rCurrentOverlaySrc.DebugDump(f);
	fprintf(f,"DEST: "); rCurrentOverlayDest.DebugDump(f);
}

CAspectFilter::CAspectFilter()
{ 
    m_Child = NULL;
}

CAspectFilter::~CAspectFilter()
{ 
    if(m_Child != NULL)
    {
        delete m_Child;
    }
}

// Called to actually perform the adjustment for 1 filter
// If it returns TRUE, this is a request from the filter to re-run the aspect calculation
//    The TRUE value is currently only used by the filter which adjusts the window rectangle
//    as this adjustment affects all calculations.  Current implementation only allows 
//    1 level of re-calculate requests.

void CAspectFilter::DebugDump(FILE *f)
{
	; // empty if not implemented
}
void CAspectFilter::SetChild(CAspectFilter* Child)
{
    m_Child = Child;
}


COverscanAspectFilter::COverscanAspectFilter(int overscanSize)
{
	overscan = overscanSize;
}

BOOL COverscanAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	ar.rCurrentOverlaySrc.shrink(overscan);
	return FALSE;
}

LPCSTR COverscanAspectFilter::getFilterName()
{
	return "COverscanAspectFilter";
}
void COverscanAspectFilter::DebugDump(FILE *f)
{ 
	fprintf(f,"Overscan = %i\n",overscan);
}

// This filter orbits the source image using independent X and Y timers.
// It is assumed that bouncing is enabled if this filter is in the chain.
// The bounce comes out of the overscan (the overscan filter performs a sanity check to ensure the overscan is of appropriate size.
COrbitAspectFilter::COrbitAspectFilter(time_t orbitPeriodX, time_t orbitPeriodY, long orbitSize)
{
	if (aspectSettings.bounceStartTime == 0)
	{
		time(&aspectSettings.bounceStartTime);
	}
	xOrbit = new CPeriodBouncer(aspectSettings.bounceStartTime,orbitPeriodX,orbitSize,-orbitSize/2.0);
	yOrbit = new CPeriodBouncer(aspectSettings.bounceStartTime,orbitPeriodY,orbitSize,-orbitSize/2.0);
}

COrbitAspectFilter::~COrbitAspectFilter()
{ 
	delete xOrbit; 
	delete yOrbit; 
}

BOOL COrbitAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	ar.rCurrentOverlaySrc.shift((int)xOrbit->position(),
								(int)yOrbit->position());
	return FALSE;
}

LPCSTR COrbitAspectFilter::getFilterName()
{
	return "COrbitAspectFilter";
}

void COrbitAspectFilter::DebugDump(FILE *f)
{
	fprintf(f,"xOrbit = %lf, yOrbit = %lf\n",xOrbit->position(), yOrbit->position());
}

CBounceDestinationAspectFilter::CBounceDestinationAspectFilter(time_t period)
{
	if (aspectSettings.bounceStartTime == 0)
	{
		time(&aspectSettings.bounceStartTime);
	}
	bouncer = new CPeriodBouncer(
        aspectSettings.bounceStartTime, period,
        // bounceAmplitude ranges from 0 to 100, we want 0 to 2.
        2.0 * (double)aspectSettings.bounceAmplitude / 100.0,
        // bounceAmplitude ranges from 0 to 100, we want 0 to -1.
        -1.0 * (double)aspectSettings.bounceAmplitude / 100.0);
}

CBounceDestinationAspectFilter::~CBounceDestinationAspectFilter()
{
	delete bouncer;
}

BOOL CBounceDestinationAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	CAspectRect oldDest = ar.rCurrentOverlayDest;

	m_Child->adjustAspect(ar);

	double pos = bouncer->position();
	ar.rCurrentOverlayDest.shift((int) (((oldDest.width()-ar.rCurrentOverlayDest.width())*pos)/2),
								 (int) (((oldDest.height()-ar.rCurrentOverlayDest.height())*pos)/2));
	return FALSE;
}

LPCSTR CBounceDestinationAspectFilter::getFilterName()
{
	return "CBounceDestinationAspectFilter"; 
}

void CBounceDestinationAspectFilter::DebugDump(FILE *f)
{
	fprintf(f,"position = %lf\n",bouncer->position());
}

// Applys child filters than adjusts the position of the destination rectangle - this class fixed floating point positions
// from -1 to 1 (0 = centered, -1 = left/top +1 = right/bottom
CPositionDestinationAspectFilter::CPositionDestinationAspectFilter(double x, double y)
{
	xPos = x; 
	yPos = y;
}

BOOL CPositionDestinationAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	CAspectRect oldDest = ar.rCurrentOverlayDest;

	m_Child->adjustAspect(ar);

	ar.rCurrentOverlayDest.shift((int) (((oldDest.width()-ar.rCurrentOverlayDest.width())*xPos)/2),
								 (int) (((oldDest.height()-ar.rCurrentOverlayDest.height())*yPos)/2));
	return FALSE;
}

LPCSTR CPositionDestinationAspectFilter::getFilterName() 
{ 
	return "CPositionDestinationAspectFilter"; 
}

void CPositionDestinationAspectFilter::DebugDump(FILE *f)
{
	fprintf(f,"xPos = %lf, yPos = %lf\n",xPos,yPos); 
}

BOOL CCropAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	double MaterialAspect = aspectSettings.source_aspect ? (aspectSettings.source_aspect/1000.0) : ar.rCurrentOverlayDest.targetAspect();

	// Crop the source rectangle down to the desired aspect...
	ar.rCurrentOverlaySrc.adjustTargetAspectByShrink(MaterialAspect);

	// Crop the destination rectangle
	// Bouncers are used to position the target rectangle within the cropped region...
	ar.rCurrentOverlayDest.adjustTargetAspectByShrink(MaterialAspect);
	return FALSE;
}

LPCSTR CCropAspectFilter::getFilterName()
{ 
	return "CCropAspectFilter";
}

// Applies the child filters and uncrops the source image to use all the area available in
// the original destination rectangle.
BOOL CUnCropAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	// Save source and dest going in - needed for un-cropping the window...
	CAspectRect rOriginalDest(ar.rCurrentOverlayDest);
	CAspectRect rOriginalSrc(ar.rCurrentOverlaySrc);

	// Apply sub-filters
	m_Child->adjustAspect(ar);
	CAspectRect lastSrc = ar.rCurrentOverlaySrc;

	// Figure out where we have space left and add it back in (up to the amount of image we have)
	double vScale = ar.rCurrentOverlayDest.height() / ar.rCurrentOverlaySrc.height();
	double hScale = ar.rCurrentOverlayDest.width() / ar.rCurrentOverlaySrc.width();

	// Scale the source image to use the entire display area
	ar.rCurrentOverlaySrc.left -= (int)floor((ar.rCurrentOverlayDest.left - rOriginalDest.left)/hScale);
	ar.rCurrentOverlaySrc.right += (int)floor((rOriginalDest.right - ar.rCurrentOverlayDest.right)/hScale);
	ar.rCurrentOverlaySrc.top -= (int)floor((ar.rCurrentOverlayDest.top - rOriginalDest.top)/vScale);
	ar.rCurrentOverlaySrc.bottom += (int)floor((rOriginalDest.bottom - ar.rCurrentOverlayDest.bottom)/vScale);

	// Can't use crop function in rectangle - rounding differences in scaling factors
	// due to calculating factors at different times cause issues
	// Need to do all cropping and scaling w/ same scale factors...

	// Clip the source image to actually available image
	if (ar.rCurrentOverlaySrc.left < rOriginalSrc.left) ar.rCurrentOverlaySrc.left = rOriginalSrc.left;
	if (ar.rCurrentOverlaySrc.right > rOriginalSrc.right) ar.rCurrentOverlaySrc.right = rOriginalSrc.right;
	if (ar.rCurrentOverlaySrc.top < rOriginalSrc.top) ar.rCurrentOverlaySrc.top = rOriginalSrc.top;
	if (ar.rCurrentOverlaySrc.bottom > rOriginalSrc.bottom) ar.rCurrentOverlaySrc.bottom = rOriginalSrc.bottom;
	
	// Now scale the destination to the source remaining
	ar.rCurrentOverlayDest.left += (int)floor((ar.rCurrentOverlaySrc.left-lastSrc.left)*hScale);
	ar.rCurrentOverlayDest.right -= (int)floor((lastSrc.right-ar.rCurrentOverlaySrc.right)*hScale);
	ar.rCurrentOverlayDest.top += (int)floor((ar.rCurrentOverlaySrc.top-lastSrc.top)*vScale);
	ar.rCurrentOverlayDest.bottom -= (int)floor((lastSrc.bottom-ar.rCurrentOverlaySrc.bottom)*vScale);

	return FALSE;
}

LPCSTR CUnCropAspectFilter::getFilterName()
{ 
	return "CUnCropAspectFilter"; 
}

// Zooms in on the source image
// x/yZoom is the amount to zoom - 1 = full size, 2 = double size, 4 = quad size 
//    both zoom factors should normally be equal - any other values will wreck the 
//    aspect ratio of the image (that would be a shame after spending all this code to keep it correct <grin>
// x/yPos is the position to zoom in on - 0 = left/top of frame, .5 = middle, 1 = right/bottom of frame
// Normally this filter will be applied just before the CScreenSanityAspectFilter
CPanAndZoomAspectFilter::CPanAndZoomAspectFilter(long _xPos, long _yPos, long _xZoom, long _yZoom)
{
	xPos = (double)_xPos / 100.0; 
	yPos = (double)_yPos / 100.0;
	xZoom = (double)_xZoom / 100.0; 
	yZoom = (double)_yZoom / 100.0;
}

BOOL CPanAndZoomAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    int dx;
    int dy;
	CAspectRect rOriginalSrc(ar.rCurrentOverlaySrc);

    if(xZoom > 1.0)
    {
    	dx = (int)floor(ar.rCurrentOverlaySrc.width() * (1.0 - 1.0/xZoom));
    	ar.rCurrentOverlaySrc.shrink(0,dx,0,0);
    }
    else
    {
    	dx = (int)floor(ar.rCurrentOverlayDest.width() * (1.0 - xZoom));
    	ar.rCurrentOverlayDest.shrink(0,dx,0,0);
    }

    // do we have to crop the input if we do we have to
    // change the output too
    if(xPos > 1.0)
    {
    	ar.rCurrentOverlaySrc.shift(dx,0);
        dx = (int)floor(ar.rCurrentOverlaySrc.width() * (xPos - 1.0));
        ar.rCurrentOverlaySrc.shrink(0,dx,0,0);
        dx = (int)floor(ar.rCurrentOverlayDest.width() * (xPos - 1.0));
        ar.rCurrentOverlayDest.shrink(dx, 0, 0, 0);
    }
    else if(xPos < 0.0)
    {
        dx = (int)floor(ar.rCurrentOverlaySrc.width() * -xPos);
        ar.rCurrentOverlaySrc.shrink(dx,0,0,0);
        dx = (int)floor(ar.rCurrentOverlayDest.width() * -xPos);
        ar.rCurrentOverlayDest.shrink(0, dx, 0, 0);
    }
    else
    {
    	ar.rCurrentOverlaySrc.shift((int)floor(dx*xPos), 0);
    }

    if(yZoom > 1.0)
    {
    	dy = (int)floor(ar.rCurrentOverlaySrc.height() * (1.0 - 1.0/yZoom));
    	ar.rCurrentOverlaySrc.shrink(0,0,0,dy);
    }
    else
    {
    	dy = (int)floor(ar.rCurrentOverlayDest.height() * (1.0 - yZoom));
    	ar.rCurrentOverlayDest.shrink(0,0,0,dy);
    }

    // do we have to crop the input if we do we have to
    // change the output too
    if(yPos > 1.0)
    {
    	ar.rCurrentOverlaySrc.shift(0, dy);
        dy = (int)floor(ar.rCurrentOverlaySrc.height() * (yPos - 1.0));
        ar.rCurrentOverlaySrc.shrink(0,0,0,dy);
        dy = (int)floor(ar.rCurrentOverlayDest.height() * (yPos - 1.0));
        ar.rCurrentOverlayDest.shrink(0, 0, dy, 0);
    }
    else if(yPos < 0.0)
    {
        dy = (int)floor(ar.rCurrentOverlaySrc.height() * -yPos);
        ar.rCurrentOverlaySrc.shrink(0,0,dy,0);
        dy =(int)floor(ar.rCurrentOverlayDest.height() * -yPos);
        ar.rCurrentOverlayDest.shrink(0, 0, 0, dy);
    }
    else
    {
    	ar.rCurrentOverlaySrc.shift(0, (int)floor(dy*yPos));
    }

	// Clip the source image to actually available image
	if (ar.rCurrentOverlaySrc.left < rOriginalSrc.left) ar.rCurrentOverlaySrc.left = rOriginalSrc.left;
	if (ar.rCurrentOverlaySrc.right > rOriginalSrc.right) ar.rCurrentOverlaySrc.right = rOriginalSrc.right;
	if (ar.rCurrentOverlaySrc.top < rOriginalSrc.top) ar.rCurrentOverlaySrc.top = rOriginalSrc.top;
	if (ar.rCurrentOverlaySrc.bottom > rOriginalSrc.bottom) ar.rCurrentOverlaySrc.bottom = rOriginalSrc.bottom;

	return FALSE;
}

LPCSTR CPanAndZoomAspectFilter::getFilterName()
{
	return "CPanAndZoomAspectFilter";
}
void CPanAndZoomAspectFilter::DebugDump(FILE *f)
{ 
	fprintf(f,"xPos = %lf, yPos = %lf, xZoom = %lf, yZoom = %lf\n",xPos,yPos,xZoom,yZoom);
}

// Performs important sanity checks on the destination rectangle
// Should occur at the end of the aspect processing chain (but before the ResizeWindow filter)
BOOL CScreenSanityAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	// crop the Destination rect so that the overlay destination region is 
	// always on the screen we will also update the source area to reflect this
	// so that we see the appropriate portion on the screen
	// (this should make us compatable with YXY)
	RECT screenRect = {0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
	ar.rCurrentOverlayDest.crop(screenRect,&ar.rCurrentOverlaySrc);

	// make sure that any alignment restrictions are taken care of
	if (SrcSizeAlign > 1)
	{
		ar.rCurrentOverlaySrc.align(SrcSizeAlign);
	}
	if (DestSizeAlign > 1)
	{
		ar.rCurrentOverlayDest.align(DestSizeAlign);
	}

	// Ensure we do not shrink too small...avoids crashes when window gets too small
	ar.rCurrentOverlayDest.enforceMinSize(1);
	ar.rCurrentOverlaySrc.enforceMinSize(1);
	return FALSE;
}

LPCSTR CScreenSanityAspectFilter::getFilterName()
{ 
	return "CScreenSanityAspectFilter";
}

// Attemtps to resize the client window to match the aspect ratio
BOOL CResizeWindowAspectFilter::adjustAspect(CAspectRectangles &ar)
{
	if (!bIsFullScreen) 
	{
		// See if we need to resize the window
		CAspectRect currentClientRect;
		CAspectRect newRect = ar.rCurrentOverlayDest;
			
		currentClientRect.setToClient(hWnd,TRUE);
		if (IsStatusBarVisible())
		{
			currentClientRect.bottom -= StatusBar_Height();
		}

		#ifdef __ASPECTFILTER_DEBUG__
			fprintf(debugLog, "Current Client Rect:"); currentClientRect.DebugDump(debugLog);
			fprintf(debugLog, "Target Client Rect :"); newRect.DebugDump(debugLog);
		#endif

		// Do we match????
		if (!currentClientRect.tolerantEquals(newRect,8))
		{
			// Nope!  Scale the existing window using "smart" logic to grow or shrink the window as needed
			RECT screenRect = {0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };

			currentClientRect.adjustSourceAspectSmart(newRect.sourceAspect(),screenRect);
			
			#ifdef __ASPECTFILTER_DEBUG__
				fprintf(debugLog, "New Client Rect    :"); currentClientRect.DebugDump(debugLog);
			#endif
						
			// Add the status bar back in...
			if (IsStatusBarVisible())
			{
				currentClientRect.bottom += StatusBar_Height();		
			}
			
			// Convert client rect to window rect...
			currentClientRect.enforceMinSize(8);
			AdjustWindowRectEx(&currentClientRect,GetWindowLong(hWnd,GWL_STYLE),Show_Menu,0);
			
			#ifdef __ASPECTFILTER_DEBUG__
				fprintf(debugLog, "New Window Pos     :"); 
				currentClientRect.DebugDump(debugLog);
			#endif

			// Set the window...
			SetWindowPos(hWnd,NULL,currentClientRect.left,currentClientRect.top,currentClientRect.width(),currentClientRect.height(),
						 SWP_NOZORDER);
			
			#ifdef __ASPECTFILTER_DEBUG__
                currentClientRect.setToClient(hWnd,TRUE);
				if (IsStatusBarVisible())
				{
					currentClientRect.bottom -= StatusBar_Height();
				}
				fprintf(debugLog, "Actual New Client  :"); currentClientRect.DebugDump(debugLog);
			#endif

			// Recalculate the overlay
			return TRUE;
		}
	} 
	return FALSE;
}

LPCSTR CResizeWindowAspectFilter::getFilterName()
{
	return "CResizeWindowAspectFilter";
}

CFilterChain::CFilterChain()
{
}

void CFilterChain::BuildFilterChain()
{
	if (aspectSettings.orbitEnabled)
	{ 
		int overscan = aspectSettings.InitialOverscan;
		if (aspectSettings.orbitEnabled && overscan*2 < aspectSettings.orbitSize)
		{
			overscan = (aspectSettings.orbitSize+1)/2;
		}
		m_FilterChain.push_back(new COverscanAspectFilter(aspectSettings.InitialOverscan));
		m_FilterChain.push_back(new COrbitAspectFilter(aspectSettings.orbitPeriodX, aspectSettings.orbitPeriodY, aspectSettings.orbitSize)); 
	}
	else 
	{
		m_FilterChain.push_back(new COverscanAspectFilter(aspectSettings.InitialOverscan));
	}
	if (aspectSettings.aspect_mode)
	{ 
        CAspectFilter* PosFilter;
		if (aspectSettings.bounceEnabled)
		{
			PosFilter = new CBounceDestinationAspectFilter(aspectSettings.bouncePeriod);
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
			PosFilter = new CPositionDestinationAspectFilter(xPos,yPos);
		}
		
		PosFilter->SetChild(new CCropAspectFilter());

		if (!aspectSettings.aspectImageClipped)
		{
            CAspectFilter* UnCropFilter = new CUnCropAspectFilter();
            UnCropFilter->SetChild(PosFilter);
			m_FilterChain.push_back(UnCropFilter); 
		}
        else
        {
			m_FilterChain.push_back(PosFilter); 
        }
	}

	// This like is where image zooming would be implemented...
	// Sample code zooms in 2x on the center of the image
	// See comments in AspectFilters.hpp for PanAndZoomAspectFilter details.
	if (aspectSettings.xZoomFactor != 100 || aspectSettings.yZoomFactor != 100 ||
		aspectSettings.xZoomCenter != 50 || aspectSettings.yZoomCenter != 50)
	{
		m_FilterChain.push_back(new CPanAndZoomAspectFilter(aspectSettings.xZoomCenter,
											   aspectSettings.yZoomCenter,
											   aspectSettings.xZoomFactor,
											   aspectSettings.yZoomFactor));
	}

	m_FilterChain.push_back(new CScreenSanityAspectFilter());
	if (aspectSettings.autoResizeWindow)
	{
		m_FilterChain.push_back(new CResizeWindowAspectFilter());
	}
}

CFilterChain::~CFilterChain()
{
    for( vector<CAspectFilter*>::iterator it = m_FilterChain.begin();
        it != m_FilterChain.end();
        ++it)
    {
        delete (*it);
    }
    m_FilterChain.empty();
}

// Applies all filters in a chain.  See above for return value.
// If allowReadjust is FALSE, the filter will ignore any re-calculate requests from filters
// to avoid infinite recursion.
BOOL CFilterChain::ApplyFilters(CAspectRectangles &ar, BOOL allowReadjust)
{
	#ifdef __ASPECTFILTER_DEBUG__
		if (debugLog == NULL) 
		{
			debugLog = fopen(__ASPECTFILTER_DEBUG__,"wt");
			setvbuf(debugLog,NULL,_IONBF,0);
		}
	#endif

    for(vector<CAspectFilter*>::iterator it = m_FilterChain.begin();
        it != m_FilterChain.end();
        ++it)
    {
		#ifdef __ASPECTFILTER_DEBUG__
			fprintf(debugLog,"PRE FILTER VALUES: %s\n",(*it)->getFilterName());
			(*it)->DebugDump(debugLog);
			ar.DebugDump(debugLog);
		#endif
		BOOL readjust = (*it)->adjustAspect(ar);
		#ifdef __ASPECTFILTER_DEBUG__
			fprintf(debugLog,"POST FILTER VALUES: %s\n",(*it)->getFilterName());
			ar.DebugDump(debugLog);
			if (readjust)
            {
				if (allowReadjust)
                {
                    fprintf(debugLog,"READJUST REQUESTED\n");
                }
				else
                {
                    fprintf(debugLog,"READJUST REQUESTED BUT NOT ALLOWED\n");
                }
            }
		#endif
		if (readjust && allowReadjust)
        {
            return TRUE;
        }
	}
	return FALSE;
}
