/////////////////////////////////////////////////////////////////////////////
// CAspectRect.hpp
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
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
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 23 Apr 2001   Michael Samblanet     File created
//
// 08 Jun 2001   Eric Schmidt          Added bounce amplitude to ini
//
/////////////////////////////////////////////////////////////////////////////

/*
This module is #included by AspectRatio.cpp for now.  It defines various filters
which are applyed in order to adjust the aspect ratio of the video image.

The filters are stored in a linked list and applied in order.  A filter can have
child filters to allow it to take advantage of values from before and after 
the child filters were applied.  This is currently used by the uncrop filter which
needs the original destination rectangle.
*/


/* This define will write to a file as named in the define containing debug information
   on the aspect filter execution - if no path is specified, working directory is used. 
   This file can get big pretty quickly - only use for debugging
*/
#define __ASPECTFILTER_DEBUG__ "CAspectFilterDebug.log"
#ifdef __ASPECTFILTER_DEBUG__
	static FILE *debugLog = NULL;
#endif

/* Used to calculate positions for a given period and timing.
	Values go from Amplitude/2+Offset to Amplitude+Offset, down to Offset, and then back to Amplitude/2+Offset
	This results in a bounce between Offset and Amplitude+Offset, starting at the midpoint.
*/
class CPeriodBouncer
{
public:
	CPeriodBouncer(time_t period, double amplitude, double offset)
	{ 
		time(&m_startTime); 
		m_period = period; 
		m_amplitude = amplitude; 
		m_offset = offset; 
	}
	CPeriodBouncer(time_t startTime, time_t period, double amplitude, double offset)
	{ 
		m_startTime = startTime; 
		m_period = period; 
		m_amplitude = amplitude; 
		m_offset = offset;
	}

	double position()
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
	
protected: 
	time_t m_startTime;
	time_t m_period;
	double m_amplitude;
	double m_offset;
};

class CAspectRectangles
{
/* Class containing all the rectangles a filter might want 
   Dest rectanges refer to the image rectangle on the screen
   Src rectangles refer to the rectangle of the source image being used

   Prev rectangles refer to the values from before this current filter run
   Original rectangles refer to the initial values of "Current" passed into the filter chain
   Current rectangles are the value being used and adjusted.

   ONLY CURRENT VALUES SHOULD BE ADJUSTED BY FILTERS!
*/
public:
	RECT rPrevDest;
	RECT rPrevSrc;

	CAspectRect rOriginalOverlayDest;
	CAspectRect rOriginalOverlaySrc;

	CAspectRect rCurrentOverlayDest;
	CAspectRect rCurrentOverlaySrc;

	void DebugDump(FILE *f)
	{
		fprintf(f,"SRC : "); rCurrentOverlaySrc.DebugDump(f);
		fprintf(f,"DEST: "); rCurrentOverlayDest.DebugDump(f);
	}
};

/* Abstract Base class for the aspect filter */
class CAspectFilter
{
public:
	CAspectFilter()
	{ 
		firstChild = next = NULL; 
		bEnabled = TRUE; 
	}
	virtual ~CAspectFilter()
	{ 
		delete firstChild; 
		delete next; 
	}
	virtual LPCSTR getFilterName() = 0;

	// Called to actually perform the adjustment for 1 filter
	// If it returns TRUE, this is a request from the filter to re-run the aspect calculation
	//    The TRUE value is currently only used by the filter which adjusts the window rectangle
	//    as this adjustment affects all calculations.  Current implementation only allows 
	//    1 level of re-calculate requests.
	virtual BOOL adjustAspect(CAspectRectangles &ar) = 0; 

	// Applies all filters in a chain.  See above for return value.
	// If allowReadjust is FALSE, the filter will ignore any re-calculate requests from filters
	// to avoid infinite recursion.
	virtual BOOL applyFilters(CAspectRectangles &ar, BOOL allowReadjust)
	{
		#ifdef __ASPECTFILTER_DEBUG__
			if (debugLog == NULL) 
			{
				debugLog = fopen(__ASPECTFILTER_DEBUG__,"wt");
				setvbuf(debugLog,NULL,_IONBF,0);
			}
		#endif

		CAspectFilter *f = this;
		while (f)
		{
			if (bEnabled)
			{
				#ifdef __ASPECTFILTER_DEBUG__
					fprintf(debugLog,"PRE FILTER VALUES: %s\n",f->getFilterName());
					f->DebugDump(debugLog);
					ar.DebugDump(debugLog);
				#endif
				BOOL readjust = f->adjustAspect(ar);
				#ifdef __ASPECTFILTER_DEBUG__
					fprintf(debugLog,"POST FILTER VALUES: %s\n",f->getFilterName());
					ar.DebugDump(debugLog);
					if (readjust) 
						if (allowReadjust) fprintf(debugLog,"READJUST REQUESTED\n");
						else fprintf(debugLog,"READJUST REQUESTED BUT NOT ALLOWED\n");
				#endif
				if (readjust && allowReadjust) return TRUE;
			}
			else if (firstChild)
			{
				#ifdef __ASPECTFILTER_DEBUG__
					fprintf(debugLog,"DISABLED FILTER: %s\n",f->getFilterName());
					f->DebugDump(debugLog);
					ar.DebugDump(debugLog);
				#endif
				// Filter disabled but it has children...disable it...
				if (firstChild->adjustAspect(ar) && allowReadjust) return TRUE;
			}
			f = f->next;
		}
		return FALSE;
	}

	void EnableFilter(BOOL enabled = FALSE, BOOL childrenAlso = FALSE)
	{ 
		bEnabled = enabled; 
		if (childrenAlso)
		{
			CAspectFilter *f = this;
			while (f)
			{
				f->EnableFilter(enabled,childrenAlso);
				f = f->next;
			}
		}
	}
	
	BOOL getEnabled()
	{
		return bEnabled;
	}

	// TODO: these should be protected members...
	CAspectFilter *firstChild; // First child filter
	CAspectFilter *next; // Next filter in linked list of filters

	virtual void DebugDump(FILE *f)
	{
		; // empty if not implemented
	}

protected:
	
	BOOL bEnabled; // If TRUE, the filter is enabled - otherwise it is ignored.
				   // If a filter is disabled, child filters get run by ApplyFilters
};

// This filter adjust for the overscan area of the image.  It checks the orbit settings to ensure
// the overscan is large enough.
class COverscanAspectFilter : public CAspectFilter
{
public:
	COverscanAspectFilter(int overscanSize)
	{
		overscan = overscanSize;
	}

	virtual BOOL adjustAspect(CAspectRectangles &ar)
	{
		ar.rCurrentOverlaySrc.shrink(overscan);
		return FALSE;
	}
	virtual LPCSTR getFilterName()
	{
		return "COverscanAspectFilter";
	}
	virtual void DebugDump(FILE *f)
	{ 
		fprintf(f,"Overscan = %i\n",overscan);
	}

protected:
	int overscan;
};

// This filter orbits the source image using independent X and Y timers.
// It is assumed that bouncing is enabled if this filter is in the chain.
// The bounce comes out of the overscan (the overscan filter performs a sanity check to ensure the overscan is of appropriate size.
class COrbitAspectFilter : public CAspectFilter
{
public:
	COrbitAspectFilter(time_t orbitPeriodX, time_t orbitPeriodY, long orbitSize)
	{
		if (aspectSettings.bounceStartTime == 0)
		{
			time(&aspectSettings.bounceStartTime);
		}
		xOrbit = new CPeriodBouncer(aspectSettings.bounceStartTime,orbitPeriodX,orbitSize,-orbitSize/2.0);
		yOrbit = new CPeriodBouncer(aspectSettings.bounceStartTime,orbitPeriodY,orbitSize,-orbitSize/2.0);
	}
	~COrbitAspectFilter()
	{ 
		delete xOrbit; 
		delete yOrbit; 
	}

	virtual BOOL adjustAspect(CAspectRectangles &ar)
	{
		ar.rCurrentOverlaySrc.shift((int)xOrbit->position(),
									(int)yOrbit->position());
		return FALSE;
	}
	virtual LPCSTR getFilterName()
	{
		return "COrbitAspectFilter";
	}
	virtual void DebugDump(FILE *f)
	{
		fprintf(f,"xOrbit = %lf, yOrbit = %lf\n",xOrbit->position(), yOrbit->position());
	}

protected:
	CPeriodBouncer *xOrbit;
	CPeriodBouncer *yOrbit;
};


// Applys child filters than adjusts the position of the destination rectangle - this class uses bounce filters
class CBounceDestinationAspectFilter : public CAspectFilter
{
public:
	CBounceDestinationAspectFilter(time_t period)
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
	~CBounceDestinationAspectFilter()
	{
		delete bouncer;
	}

	virtual BOOL adjustAspect(CAspectRectangles &ar)
	{
		CAspectRect oldDest = ar.rCurrentOverlayDest;

		firstChild->applyFilters(ar,FALSE);

		double pos = bouncer->position();
		ar.rCurrentOverlayDest.shift((int) (((oldDest.width()-ar.rCurrentOverlayDest.width())*pos)/2),
									 (int) (((oldDest.height()-ar.rCurrentOverlayDest.height())*pos)/2));
		return FALSE;
	}
	virtual LPCSTR getFilterName()
	{
		return "CBounceDestinationAspectFilter"; 
	}
	virtual void DebugDump(FILE *f)
	{
		fprintf(f,"position = %lf\n",bouncer->position());
	}

protected:
	CPeriodBouncer *bouncer;
};

// Applys child filters than adjusts the position of the destination rectangle - this class fixed floating point positions
// from -1 to 1 (0 = centered, -1 = left/top +1 = right/bottom
class CPositionDestinationAspectFilter : public CAspectFilter
{
public:
	CPositionDestinationAspectFilter(double x, double y)
	{
		xPos = x; 
		yPos = y;
	}
	
	virtual BOOL adjustAspect(CAspectRectangles &ar)
	{
		CAspectRect oldDest = ar.rCurrentOverlayDest;

		firstChild->applyFilters(ar,FALSE);

		ar.rCurrentOverlayDest.shift((int) (((oldDest.width()-ar.rCurrentOverlayDest.width())*xPos)/2),
									 (int) (((oldDest.height()-ar.rCurrentOverlayDest.height())*yPos)/2));
		return FALSE;
	}
	virtual LPCSTR getFilterName() 
	{ 
		return "CPositionDestinationAspectFilter"; 
	}
	virtual void DebugDump(FILE *f)
	{
		fprintf(f,"xPos = %lf, yPos = %lf\n",xPos,yPos); 
	}

protected:
	double xPos;
	double yPos;
};

// Crops the source and destination rectangles to the requested aspect ratio.  
class CCropAspectFilter : public CAspectFilter 
{
public:
	virtual BOOL adjustAspect(CAspectRectangles &ar)
	{
		double MaterialAspect = aspectSettings.source_aspect ? (aspectSettings.source_aspect/1000.0) : ar.rCurrentOverlayDest.targetAspect();

		// Crop the source rectangle down to the desired aspect...
		ar.rCurrentOverlaySrc.adjustTargetAspectByShrink(MaterialAspect);

		// Crop the destination rectangle
		// Bouncers are used to position the target rectangle within the cropped region...
		ar.rCurrentOverlayDest.adjustTargetAspectByShrink(MaterialAspect);
		return FALSE;
	}
	virtual LPCSTR getFilterName()
	{ 
		return "CCropAspectFilter";
	}
};

// Applies the child filters and uncrops the source image to use all the area available in
// the original destination rectangle.
class CUnCropAspectFilter : public CAspectFilter
{
public:
	virtual BOOL adjustAspect(CAspectRectangles &ar)
	{
		// Save source and dest going in - needed for un-cropping the window...
		CAspectRect rOriginalDest(ar.rCurrentOverlayDest);
		CAspectRect rOriginalSrc(ar.rCurrentOverlaySrc);

		// Apply sub-filters
		firstChild->applyFilters(ar,FALSE);
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
	virtual LPCSTR getFilterName()
	{ 
		return "CUnCropAspectFilter"; 
	}
};

// Zooms in on the source image
// x/yZoom is the amount to zoom - 1 = full size, 2 = double size, 4 = quad size 
//    both zoom factors should normally be equal - any other values will wreck the 
//    aspect ratio of the image (that would be a shame after spending all this code to keep it correct <grin>
// x/yPos is the position to zoom in on - 0 = left/top of frame, .5 = middle, 1 = right/bottom of frame
// Normally this filter will be applied just before the CScreenSanityAspectFilter
class CPanAndZoomAspectFilter : public CAspectFilter
{
public:
	CPanAndZoomAspectFilter(long _xPos, long _yPos, long _xZoom, long _yZoom)
	{
		xPos = _xPos / 100; 
		yPos = _yPos / 100;
		xZoom = _xZoom / 100; 
		yZoom = _yZoom / 100;
	}
	
	virtual BOOL adjustAspect(CAspectRectangles &ar)
	{
		int dx = (int)floor(ar.rCurrentOverlaySrc.width() * (1.0 - 1.0/xZoom));
		int dy = (int)floor(ar.rCurrentOverlaySrc.height() * (1.0 - 1.0/yZoom));

		ar.rCurrentOverlaySrc.shrink(0,dx,0,dy);
		ar.rCurrentOverlaySrc.shift((int)floor(dx*xPos),
									(int)floor(dy*yPos));

		return FALSE;
	}

	virtual LPCSTR getFilterName()
	{
		return "CPanAndZoomAspectFilter";
	}
	virtual void DebugDump(FILE *f)
	{ 
		fprintf(f,"xPos = %lf, yPos = %lf, xZoom = %lf, yZoom = %lf\n",xPos,yPos,xZoom,yZoom);
	}

protected:
	double xPos;
	double yPos;
	double xZoom;
	double yZoom;
};

// Performs important sanity checks on the destination rectangle
// Should occur at the end of the aspect processing chain (but before the ResizeWindow filter)
class CScreenSanityAspectFilter : public CAspectFilter
{
public:
	virtual BOOL adjustAspect(CAspectRectangles &ar)
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
	virtual LPCSTR getFilterName()
	{ 
		return "CScreenSanityAspectFilter";
	}
};

// Attemtps to resize the client window to match the aspect ratio
class CResizeWindowAspectFilter : public CAspectFilter
{
public:
	virtual BOOL adjustAspect(CAspectRectangles &ar)
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

	virtual LPCSTR getFilterName()
	{
		return "CResizeWindowAspectFilter";
	}
};
