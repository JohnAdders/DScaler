/////////////////////////////////////////////////////////////////////////////
// $Id: AspectFilters.h,v 1.11 2001-07-13 16:14:55 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Michael Samblanet.  All rights reserved.
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
// Change Log
//
// Date          Developer             Changes
//
// 23 Apr 2001   Michael Samblanet     File created
//
// 08 Jun 2001   Eric Schmidt          Added bounce amplitude to ini
//
// 03 Jul 2001   John Adcock           Reoganized code and changed behavious of pan
//                                     and scan filter.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ASPECTFILTERS_H___
#define __ASPECTFILTERS_H___

#include "AspectRect.h"

// This module is #included by AspectRatio.cpp for now.  It defines various filters
// which are applyed in order to adjust the aspect ratio of the video image.
// 
// The filters are stored in a linked list and applied in order.  A filter can have
// child filters to allow it to take advantage of values from before and after 
// the child filters were applied.  This is currently used by the uncrop filter which
// needs the original destination rectangle.
// 
// Used to calculate positions for a given period and timing.
// Values go from Amplitude/2+Offset to Amplitude+Offset, down to Offset, and then back to Amplitude/2+Offset
// This results in a bounce between Offset and Amplitude+Offset, starting at the midpoint.

class CPeriodBouncer
{
public:
    CPeriodBouncer(time_t period, double amplitude, double offset);
    CPeriodBouncer(time_t startTime, time_t period, double amplitude, double offset);
    double position();
protected: 
    time_t m_StartTime;
    time_t m_Period;
    double m_Amplitude;
    double m_Offset;
};

class CAspectRectangles
{
    // Class containing all the rectangles a filter might want 
    // Dest rectanges refer to the image rectangle on the screen
    // Src rectangles refer to the rectangle of the source image being used
    // 
    // Prev rectangles refer to the values from before this current filter run
    // Original rectangles refer to the initial values of "Current" passed into the filter chain
    // Current rectangles are the Value being used and adjusted.
    // 
    // ONLY CURRENT VALUES SHOULD BE ADJUSTED BY FILTERS!

public:
    void DebugDump(FILE* f);

    RECT m_PrevDestRect;
    RECT m_PrevSrcRect;

    CAspectRect m_OriginalOverlayDestRect;
    CAspectRect m_OriginalOverlaySrcRect;

    CAspectRect m_CurrentOverlayDestRect;
    CAspectRect m_CurrentOverlaySrcRect;

};

// Abstract Base class for the aspect filter
class CAspectFilter
{
public:
    CAspectFilter();
    virtual ~CAspectFilter();
    virtual LPCSTR getFilterName() = 0;

    // Called to actually perform the adjustment for 1 filter
    // If it returns TRUE, this is a request from the filter to re-run the aspect calculation
    //    The TRUE Value is currently only used by the filter which adjusts the window rectangle
    //    as this adjustment affects all calculations.  Current implementation only allows 
    //    1 level of re-calculate requests.
    virtual BOOL adjustAspect(CAspectRectangles &ar) = 0; 

    virtual void DebugDump(FILE* f);
    void SetChild(CAspectFilter* Child);
protected:
    CAspectFilter* m_Child;
};

// This filter adjust for the m_Overscan area of the image.  It checks the orbit settings to ensure
// the m_Overscan is large enough.
class COverscanAspectFilter : public CAspectFilter
{
public:
    COverscanAspectFilter(int overscanSize);
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
    virtual void DebugDump(FILE* f);

protected:
    int m_Overscan;
};

// This filter orbits the source image using independent X and Y timers.
// It is assumed that bouncing is enabled if this filter is in the chain.
// The bounce comes out of the m_Overscan (the m_Overscan filter performs a sanity check to ensure the m_Overscan is of appropriate size.
class COrbitAspectFilter : public CAspectFilter
{
public:
    COrbitAspectFilter(time_t OrbitPeriodX, time_t OrbitPeriodY, long OrbitSize);
    ~COrbitAspectFilter();
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
    virtual void DebugDump(FILE* f);

protected:
    CPeriodBouncer* m_pXOrbitBouncer;
    CPeriodBouncer* m_pYOrbitBouncer;
};


// Applys child filters than adjusts the position of the destination rectangle - this class uses bounce filters
class CBounceDestinationAspectFilter : public CAspectFilter
{
public:
    CBounceDestinationAspectFilter(time_t period);
    ~CBounceDestinationAspectFilter();
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
    virtual void DebugDump(FILE* f);

protected:
    CPeriodBouncer* m_pBouncer;
};

// Applys child filters than adjusts the position of the destination rectangle - this class fixed floating point positions
// from -1 to 1 (0 = centered, -1 = left/top +1 = right/bottom
class CPositionDestinationAspectFilter : public CAspectFilter
{
public:
    CPositionDestinationAspectFilter(double x, double y);
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
    virtual void DebugDump(FILE* f);

protected:
    double m_XPos;
    double m_YPos;
};

// Crops the source and destination rectangles to the requested aspect ratio.  
class CCropAspectFilter : public CAspectFilter 
{
public:
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
};

// Applies the child filters and uncrops the source image to use all the area available in
// the original destination rectangle.
class CUnCropAspectFilter : public CAspectFilter
{
public:
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
};

// Zooms in on the source image
// x/m_YZoom is the amount to zoom - 1 = full size, 2 = double size, 4 = quad size 
//    both zoom factors should normally be equal - any other values will wreck the 
//    aspect ratio of the image (that would be a shame after spending all this code to keep it correct <grin>
// x/m_YPos is the position to zoom in on - 0 = left/top of frame, .5 = middle, 1 = right/bottom of frame
// Normally this filter will be applied just before the CScreenSanityAspectFilter
class CPanAndZoomAspectFilter : public CAspectFilter
{
public:
    CPanAndZoomAspectFilter(long _xPos, long _yPos, long _xZoom, long _yZoom);
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
    virtual void DebugDump(FILE* f);

protected:
    double m_XPos;
    double m_YPos;
    double m_XZoom;
    double m_YZoom;
};

// Performs important sanity checks on the destination rectangle
// Should occur at the end of the aspect processing chain (but before the ResizeWindow filter)
class CScreenSanityAspectFilter : public CAspectFilter
{
public:
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
};

// Attemtps to resize the client window to match the aspect ratio
class CResizeWindowAspectFilter : public CAspectFilter
{
public:
    virtual BOOL adjustAspect(CAspectRectangles &ar);
    virtual LPCSTR getFilterName();
};

class CFilterChain
{
public:
    CFilterChain();
    ~CFilterChain();
    BOOL ApplyFilters(CAspectRectangles &ar, BOOL allowReadjust);
    void BuildFilterChain();
protected:
    vector<CAspectFilter*> m_FilterChain;
};

#endif