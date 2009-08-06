/////////////////////////////////////////////////////////////////////////////
// $Id$
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

/**
 * @file AspectFilter.cpp Aspect Ratio Filter Classes
 */

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "AspectFilters.h"
#include "AspectRatio.h"
#include "IOutput.h"
#include "DScaler.h"
#include "Providers.h"

using namespace std;

// From DScaler.cpp .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern BOOL bIsFullScreen;
extern BOOL bShowMenu;

//uncomment line below to get debug log for aspect ratio
//#define __ASPECTFILTER_DEBUG__

// Used to calculate positions for a given period and timing.
// Values go from Amplitude/2+Offset to Amplitude+Offset, down to Offset, and then back to Amplitude/2+Offset
// This results in a bounce between Offset and Amplitude+Offset, starting at the midpoint.

CPeriodBouncer::CPeriodBouncer(time_t period, double amplitude, double offset) :
    m_Period(period),
    m_Amplitude(amplitude),
    m_Offset(offset)
{
    time(&m_StartTime);
}

    CPeriodBouncer::CPeriodBouncer(time_t startTime, time_t period, double amplitude, double offset) :
    m_StartTime(startTime),
    m_Period(period),
    m_Amplitude(amplitude),
    m_Offset(offset)
{
}

double CPeriodBouncer::position()
{
    double phase = fmod((((double)((time(NULL)-m_StartTime)%m_Period))/m_Period+0.25),1);
    // We go from 0% to 100% over m_Period - but start 25% of the way into the phase.
    double val = phase*m_Amplitude*2.0;
    if (val > m_Amplitude)
    {
        val = fabs(2*m_Amplitude-val); // Adjust - 50% into the phase, turn around and go down
    }
    return val + m_Offset;
}

// Class containing all the rectangles a filter might want
// Dest rectanges refer to the image rectangle on the screen
// Src rectangles refer to the rectangle of the source image being used
//
// Prev rectangles refer to the values from before this current filter run
// Original rectangles refer to the initial values of _T("Current") passed into the filter chain
// Current rectangles are the Value being used and adjusted.
//
// ONLY CURRENT VALUES SHOULD BE ADJUSTED BY FILTERS!

void CAspectRectangles::DebugDump()
{
    m_CurrentOverlaySrcRect.DebugDump(_T("SRC "));
    m_CurrentOverlayDestRect.DebugDump(_T("DEST"));
}

CAspectFilter::CAspectFilter()
{
}

CAspectFilter::~CAspectFilter()
{
}

void CAspectFilter::DebugDump()
{
    ; // empty if not implemented
}
void CAspectFilter::SetChild(SmartPtr<CAspectFilter> Child)
{
    m_Child = Child;
}


COverscanAspectFilter::COverscanAspectFilter(int TopOverscanSize, int BottomOverscanSize, int LeftOverscanSize, int RightOverscanSize) :
    m_TopOverscan(TopOverscanSize),
    m_BottomOverscan(BottomOverscanSize),
    m_LeftOverscan(LeftOverscanSize),
    m_RightOverscan(RightOverscanSize)
{
}

void COverscanAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    ar.m_CurrentOverlaySrcRect.shrink(m_LeftOverscan, m_RightOverscan, m_TopOverscan, m_BottomOverscan);
}

const TCHAR* COverscanAspectFilter::getFilterName()
{
    return _T("COverscanAspectFilter");
}

void COverscanAspectFilter::DebugDump()
{
    LOG(2,_T("Overscans (Top,Bottom,Left,Right) = (%d,%d,%d,%d)"),m_TopOverscan, m_BottomOverscan, m_LeftOverscan, m_RightOverscan);
}

CAnalogueBlankingFilter::CAnalogueBlankingFilter(int SourceWidth, int SourceHeight)
{
    if(SourceHeight == 480)
    {
        m_TopShift = 1;
        m_BottomShift = 1;
        m_LeftShift = SourceWidth * 2 / 720;
        m_RightShift = SourceWidth * 4 / 720;
    }
    else if(SourceHeight == 576)
    {
        m_TopShift = 0;
        m_BottomShift = 2;
        m_LeftShift = SourceWidth * 8 / 720;
        m_RightShift = SourceWidth * 10 / 720;
    }
    else
    {
        // do nothing to image if we're not an expected height
        // since we are probably in a still or some other source
        // were analogue blanking doesn't apply
        m_TopShift = 0;
        m_BottomShift = 0;
        m_LeftShift = 0;
        m_RightShift = 0;
    }
}

void CAnalogueBlankingFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    ar.m_CurrentOverlaySrcRect.shrink(m_LeftShift, m_RightShift, m_TopShift, m_BottomShift);
}

const TCHAR* CAnalogueBlankingFilter::getFilterName()
{
    return _T("CAnalogueBlankingFilter");
}

void CAnalogueBlankingFilter::DebugDump()
{
    LOG(2,_T("AnalogueBlanking %d %d %d %d"), m_TopShift, m_BottomShift, m_LeftShift, m_RightShift);
}


// This filter orbits the source image using independent X and Y timers.
// It is assumed that bouncing is enabled if this filter is in the chain.
// The bounce comes out of the m_Overscan (the m_Overscan filter performs a sanity check to ensure the m_Overscan is of appropriate size.
COrbitAspectFilter::COrbitAspectFilter(time_t OrbitPeriodX, time_t OrbitPeriodY, long OrbitSize) :
    m_pXOrbitBouncer(new CPeriodBouncer(AspectSettings.BounceStartTime,OrbitPeriodX,OrbitSize,-OrbitSize/2.0)),
    m_pYOrbitBouncer(new CPeriodBouncer(AspectSettings.BounceStartTime,OrbitPeriodY,OrbitSize,-OrbitSize/2.0))
{
    if (AspectSettings.BounceStartTime == 0)
    {
        time(&AspectSettings.BounceStartTime);
    }
}

void COrbitAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    ar.m_CurrentOverlaySrcRect.shift((int)m_pXOrbitBouncer->position(),
                                (int)m_pYOrbitBouncer->position());
}

const TCHAR* COrbitAspectFilter::getFilterName()
{
    return _T("COrbitAspectFilter");
}

void COrbitAspectFilter::DebugDump()
{
    LOG(2,_T("m_pXOrbitBouncer = %lf, m_pYOrbitBouncer = %lf"),m_pXOrbitBouncer->position(), m_pYOrbitBouncer->position());
}

CBounceDestinationAspectFilter::CBounceDestinationAspectFilter(time_t period) :
    m_pBouncer(new CPeriodBouncer(
        AspectSettings.BounceStartTime, period,
        // BounceAmplitude ranges from 0 to 100, we want 0 to 2.
        2.0 * (double)AspectSettings.BounceAmplitude / 100.0,
        // BounceAmplitude ranges from 0 to 100, we want 0 to -1.
        -1.0 * (double)AspectSettings.BounceAmplitude / 100.0))
{
    if (AspectSettings.BounceStartTime == 0)
    {
        time(&AspectSettings.BounceStartTime);
    }
}

void CBounceDestinationAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    CAspectRect oldDest = ar.m_CurrentOverlayDestRect;

    m_Child->adjustAspect(ar, RequestRerun);

    double pos = m_pBouncer->position();
    ar.m_CurrentOverlayDestRect.shift((int) (((oldDest.width()-ar.m_CurrentOverlayDestRect.width())*pos)/2),
                                 (int) (((oldDest.height()-ar.m_CurrentOverlayDestRect.height())*pos)/2));
}

const TCHAR* CBounceDestinationAspectFilter::getFilterName()
{
    return _T("CBounceDestinationAspectFilter");
}

void CBounceDestinationAspectFilter::DebugDump()
{
    LOG(2,_T("position = %lf"),m_pBouncer->position());
}

// Applys child filters than adjusts the position of the destination rectangle - this class fixed floating point positions
// from -1 to 1 (0 = centered, -1 = left/top +1 = right/bottom
CPositionDestinationAspectFilter::CPositionDestinationAspectFilter(double x, double y) :
    m_XPos(x),
    m_YPos(y)
{
}

void CPositionDestinationAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    #ifdef __ASPECTFILTER_DEBUG__
        LOG(2,_T("PRE FILTER VALUES: %s"),this->getFilterName());
        ar.DebugDump();
    #endif

    CAspectRect oldDest = ar.m_CurrentOverlayDestRect;

    m_Child->adjustAspect(ar, RequestRerun);

    ar.m_CurrentOverlayDestRect.shift((int) (((oldDest.width()-ar.m_CurrentOverlayDestRect.width())*m_XPos)/2),
                                 (int) (((oldDest.height()-ar.m_CurrentOverlayDestRect.height())*m_YPos)/2));

    #ifdef __ASPECTFILTER_DEBUG__
        LOG(2,_T("POST FILTER VALUES: %s"),this->getFilterName());
        ar.DebugDump();
    #endif
}

const TCHAR* CPositionDestinationAspectFilter::getFilterName()
{
    return _T("CPositionDestinationAspectFilter");
}

void CPositionDestinationAspectFilter::DebugDump()
{
    LOG(2,_T("m_XPos = %lf, m_YPos = %lf"),m_XPos,m_YPos);
}

void CCropAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    #ifdef __ASPECTFILTER_DEBUG__
        LOG(2,_T("PRE FILTER VALUES: %s"),this->getFilterName());
        ar.DebugDump();
    #endif

    double MaterialAspect = AspectSettings.SourceAspect ? (AspectSettings.SourceAspect/1000.0) : ar.m_CurrentOverlayDestRect.targetAspect();

    if(AspectSettings.SourceAspectAdjust != 1000)
    {
        MaterialAspect *= (double)AspectSettings.SourceAspectAdjust / 1000.0;
    }

    // Crop the source rectangle down to the desired aspect...
    if (AspectSettings.SquarePixels)
    {
        ar.m_CurrentOverlaySrcRect.adjustTargetAspectByGrowth(MaterialAspect);
    }
    else
    {
        ar.m_CurrentOverlaySrcRect.adjustTargetAspectByShrink(MaterialAspect);
    }

    // Crop the destination rectangle
    // Bouncers are used to position the target rectangle within the cropped region...
    ar.m_CurrentOverlayDestRect.adjustTargetAspectByShrink(MaterialAspect);

    #ifdef __ASPECTFILTER_DEBUG__
        LOG(2,_T("POST FILTER VALUES: %s"),this->getFilterName());
        ar.DebugDump();
    #endif
}

const TCHAR* CCropAspectFilter::getFilterName()
{
    return _T("CCropAspectFilter");
}

// Applies the child filters and uncrops the source image to use all the area available in
// the original destination rectangle.
void CUnCropAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    // Save source and dest going in - needed for un-cropping the window...
    CAspectRect rOriginalDest(ar.m_CurrentOverlayDestRect);
    CAspectRect rOriginalSrc(ar.m_CurrentOverlaySrcRect);

    // Apply sub-filters
    m_Child->adjustAspect(ar, RequestRerun);
    CAspectRect lastSrc = ar.m_CurrentOverlaySrcRect;

    // Figure out where we have space left and add it back in (up to the amount of image we have)
    double vScale = (double)ar.m_CurrentOverlayDestRect.height() / (double)ar.m_CurrentOverlaySrcRect.height();
    double hScale = (double)ar.m_CurrentOverlayDestRect.width() / (double)ar.m_CurrentOverlaySrcRect.width();

    // Scale the source image to use the entire display area
    ar.m_CurrentOverlaySrcRect.left -= (int)floor((ar.m_CurrentOverlayDestRect.left - rOriginalDest.left)/hScale);
    ar.m_CurrentOverlaySrcRect.right += (int)floor((rOriginalDest.right - ar.m_CurrentOverlayDestRect.right)/hScale);
    ar.m_CurrentOverlaySrcRect.top -= (int)floor((ar.m_CurrentOverlayDestRect.top - rOriginalDest.top)/vScale);
    ar.m_CurrentOverlaySrcRect.bottom += (int)floor((rOriginalDest.bottom - ar.m_CurrentOverlayDestRect.bottom)/vScale);

    // Can't use crop function in rectangle - rounding differences in scaling factors
    // due to calculating factors at different times cause issues
    // Need to do all cropping and scaling w/ same scale factors...

    // Clip the source image to actually available image
    if (ar.m_CurrentOverlaySrcRect.left < rOriginalSrc.left) ar.m_CurrentOverlaySrcRect.left = rOriginalSrc.left;
    if (ar.m_CurrentOverlaySrcRect.right > rOriginalSrc.right) ar.m_CurrentOverlaySrcRect.right = rOriginalSrc.right;
    if (ar.m_CurrentOverlaySrcRect.top < rOriginalSrc.top) ar.m_CurrentOverlaySrcRect.top = rOriginalSrc.top;
    if (ar.m_CurrentOverlaySrcRect.bottom > rOriginalSrc.bottom) ar.m_CurrentOverlaySrcRect.bottom = rOriginalSrc.bottom;

    // Now scale the destination to the source remaining
    ar.m_CurrentOverlayDestRect.left += (int)floor((ar.m_CurrentOverlaySrcRect.left-lastSrc.left)*hScale);
    ar.m_CurrentOverlayDestRect.right -= (int)floor((lastSrc.right-ar.m_CurrentOverlaySrcRect.right)*hScale);
    ar.m_CurrentOverlayDestRect.top += (int)floor((ar.m_CurrentOverlaySrcRect.top-lastSrc.top)*vScale);
    ar.m_CurrentOverlayDestRect.bottom -= (int)floor((lastSrc.bottom-ar.m_CurrentOverlaySrcRect.bottom)*vScale);
}

const TCHAR* CUnCropAspectFilter::getFilterName()
{
    return _T("CUnCropAspectFilter");
}

// Zooms in on the source image
// x/m_YZoom is the amount to zoom - 1 = full size, 2 = double size, 4 = quad size
//    both zoom factors should normally be equal - any other values will wreck the
//    aspect ratio of the image (that would be a shame after spending all this code to keep it correct <grin>
// x/m_YPos is the position to zoom in on - 0 = left/top of frame, .5 = middle, 1 = right/bottom of frame
// Normally this filter will be applied just before the CScreenSanityAspectFilter
CPanAndZoomAspectFilter::CPanAndZoomAspectFilter(long _xPos, long _yPos, long _xZoom, long _yZoom)
{
    m_XPos = (double)_xPos / 100.0;
    m_YPos = (double)_yPos / 100.0;
    m_XZoom = (double)_xZoom / 100.0;
    m_YZoom = (double)_yZoom / 100.0;
}

void CPanAndZoomAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    int dx;
    int dy;
    CAspectRect rOriginalSrc(ar.m_CurrentOverlaySrcRect);

    if(m_XZoom >= 1.0)
    {
        dx = (int)floor(ar.m_CurrentOverlaySrcRect.width() * (1.0 - 1.0/m_XZoom));
        ar.m_CurrentOverlaySrcRect.shrink(0,dx,0,0);

        // do we have to crop the input if we do we have to
        // change the output too
        if(m_XPos > 1.0)
        {
            ar.m_CurrentOverlaySrcRect.shift(dx,0);
            dx = (int)floor(ar.m_CurrentOverlaySrcRect.width() * (m_XPos - 1.0));
            ar.m_CurrentOverlaySrcRect.shrink(0,dx,0,0);
            dx = (int)floor(ar.m_CurrentOverlayDestRect.width() * (m_XPos - 1.0));
            ar.m_CurrentOverlayDestRect.shrink(dx, 0, 0, 0);
        }
        else if(m_XPos < 0.0)
        {
            dx = (int)floor(ar.m_CurrentOverlaySrcRect.width() * -m_XPos);
            ar.m_CurrentOverlaySrcRect.shrink(dx,0,0,0);
            dx = (int)floor(ar.m_CurrentOverlayDestRect.width() * -m_XPos);
            ar.m_CurrentOverlayDestRect.shrink(0, dx, 0, 0);
        }
        else
        {
            ar.m_CurrentOverlaySrcRect.shift((int)floor(dx*m_XPos), 0);
        }
    }
    else
    {
        dx = (int)floor(ar.m_CurrentOverlayDestRect.width() * (1.0 - m_XZoom));
        ar.m_CurrentOverlayDestRect.shrink(0,dx,0,0);

        // do we have to crop the input if we do we have to
        // change the output too
        if(m_XPos > 1.0)
        {
            ar.m_CurrentOverlayDestRect.shift(dx,0);
            dx = (int)floor(ar.m_CurrentOverlaySrcRect.width() * (m_XPos - 1.0));
            ar.m_CurrentOverlaySrcRect.shrink(0,dx,0,0);
            dx = (int)floor(ar.m_CurrentOverlayDestRect.width() * (m_XPos - 1.0));
            ar.m_CurrentOverlayDestRect.shrink(dx, 0, 0, 0);
        }
        else if(m_XPos < 0.0)
        {
            dx = (int)floor(ar.m_CurrentOverlaySrcRect.width() * -m_XPos);
            ar.m_CurrentOverlaySrcRect.shrink(dx,0,0,0);
            dx = (int)floor(ar.m_CurrentOverlayDestRect.width() * -m_XPos);
            ar.m_CurrentOverlayDestRect.shrink(0, dx, 0, 0);
        }
        else
        {
            ar.m_CurrentOverlayDestRect.shift((int)floor(dx*m_XPos), 0);
        }
    }


    if(m_YZoom >= 1.0)
    {
        dy = (int)floor(ar.m_CurrentOverlaySrcRect.height() * (1.0 - 1.0/m_YZoom));
        ar.m_CurrentOverlaySrcRect.shrink(0,0,0,dy);

        // do we have to crop the input if we do we have to
        // change the output too
        if(m_YPos > 1.0)
        {
            ar.m_CurrentOverlaySrcRect.shift(0, dy);
            dy = (int)floor(ar.m_CurrentOverlaySrcRect.height() * (m_YPos - 1.0));
            ar.m_CurrentOverlaySrcRect.shrink(0,0,0,dy);
            dy = (int)floor(ar.m_CurrentOverlayDestRect.height() * (m_YPos - 1.0));
            ar.m_CurrentOverlayDestRect.shrink(0, 0, dy, 0);
        }
        else if(m_YPos < 0.0)
        {
            dy = (int)floor(ar.m_CurrentOverlaySrcRect.height() * -m_YPos);
            ar.m_CurrentOverlaySrcRect.shrink(0,0,dy,0);
            dy =(int)floor(ar.m_CurrentOverlayDestRect.height() * -m_YPos);
            ar.m_CurrentOverlayDestRect.shrink(0, 0, 0, dy);
        }
        else
        {
            ar.m_CurrentOverlaySrcRect.shift(0, (int)floor(dy*m_YPos));
        }
    }
    else
    {
        dy = (int)floor(ar.m_CurrentOverlayDestRect.height() * (1.0 - m_YZoom));
        ar.m_CurrentOverlayDestRect.shrink(0,0,0,dy);

        // do we have to crop the input if we do we have to
        // change the output too
        if(m_YPos > 1.0)
        {
            ar.m_CurrentOverlayDestRect.shift(0, dy);
            dy = (int)floor(ar.m_CurrentOverlaySrcRect.height() * (m_YPos - 1.0));
            ar.m_CurrentOverlaySrcRect.shrink(0,0,0,dy);
            dy = (int)floor(ar.m_CurrentOverlayDestRect.height() * (m_YPos - 1.0));
            ar.m_CurrentOverlayDestRect.shrink(0, 0, dy, 0);
        }
        else if(m_YPos < 0.0)
        {
            dy = (int)floor(ar.m_CurrentOverlaySrcRect.height() * -m_YPos);
            ar.m_CurrentOverlaySrcRect.shrink(0,0,dy,0);
            dy =(int)floor(ar.m_CurrentOverlayDestRect.height() * -m_YPos);
            ar.m_CurrentOverlayDestRect.shrink(0, 0, 0, dy);
        }
        else
        {
            ar.m_CurrentOverlayDestRect.shift(0, (int)floor(dy*m_YPos));
        }
    }

    // Clip the source image to actually available image
    if (ar.m_CurrentOverlaySrcRect.left < rOriginalSrc.left) ar.m_CurrentOverlaySrcRect.left = rOriginalSrc.left;
    if (ar.m_CurrentOverlaySrcRect.right > rOriginalSrc.right) ar.m_CurrentOverlaySrcRect.right = rOriginalSrc.right;
    if (ar.m_CurrentOverlaySrcRect.top < rOriginalSrc.top) ar.m_CurrentOverlaySrcRect.top = rOriginalSrc.top;
    if (ar.m_CurrentOverlaySrcRect.bottom > rOriginalSrc.bottom) ar.m_CurrentOverlaySrcRect.bottom = rOriginalSrc.bottom;
}

const TCHAR* CPanAndZoomAspectFilter::getFilterName()
{
    return _T("CPanAndZoomAspectFilter");
}
void CPanAndZoomAspectFilter::DebugDump()
{
    LOG(2,_T("m_XPos = %lf, m_YPos = %lf, m_XZoom = %lf, m_YZoom = %lf"),m_XPos,m_YPos,m_XZoom,m_YZoom);
}

CScreenSanityAspectFilter::CScreenSanityAspectFilter(int SrcWidth, int SrcHeight)
{
    m_SrcWidth = SrcWidth;
    m_SrcHeight = SrcHeight;
}

// Performs important sanity checks on the destination rectangle
// Should occur at the end of the aspect processing chain (but before the ResizeWindow filter)
void CScreenSanityAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    // crop the Destination rect so that the overlay destination region is
    // always on the screen we will also update the source area to reflect this
    // so that we see the appropriate portion on the screen
    // (this should make us compatable with YXY)
    RECT screenRect;
    if (GetSystemMetrics(SM_CXVIRTUALSCREEN) == 0 || GetSystemMetrics(SM_CYVIRTUALSCREEN) == 0)
    {
        screenRect.top = 0;
        screenRect.bottom = GetSystemMetrics(SM_CYSCREEN);
        screenRect.left = 0;
        screenRect.right = GetSystemMetrics(SM_CXSCREEN);
    }
    else
    {
        screenRect.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
        screenRect.bottom = GetSystemMetrics(SM_YVIRTUALSCREEN)+GetSystemMetrics(SM_CYVIRTUALSCREEN);
        screenRect.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
        screenRect.right = GetSystemMetrics(SM_XVIRTUALSCREEN)+GetSystemMetrics(SM_CXVIRTUALSCREEN);
    }
    RECT sourceRect = {0, 0, m_SrcWidth, m_SrcHeight};

    ar.m_CurrentOverlayDestRect.crop(screenRect,&ar.m_CurrentOverlaySrcRect);

    // then make sure we are still onscreen
    ar.m_CurrentOverlayDestRect.cropToFitRect(screenRect);

    // then make sure we are still onscreen
    ar.m_CurrentOverlaySrcRect.cropToFitRect(sourceRect);

    // make sure that any alignment restrictions are taken care of
    if (GetActiveOutput()->GetSrcSizeAlign() > 1)
    {
        ar.m_CurrentOverlaySrcRect.align(GetActiveOutput()->GetSrcSizeAlign());
    }
    if (GetActiveOutput()->GetDestSizeAlign() > 1)
    {
        ar.m_CurrentOverlayDestRect.align(GetActiveOutput()->GetDestSizeAlign());
    }

    // Ensure we do not shrink too small...avoids crashes when window gets too small
    ar.m_CurrentOverlayDestRect.enforceMinSize(1);
    ar.m_CurrentOverlaySrcRect.enforceMinSize(1);
}

const TCHAR* CScreenSanityAspectFilter::getFilterName()
{
    return _T("CScreenSanityAspectFilter");
}

// Attemtps to resize the client window to match the aspect ratio
void CResizeWindowAspectFilter::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    LONG OrigClientTop = 0;

    // See if we need to resize the window
    CAspectRect currentClientRect;
    CAspectRect newRect = ar.m_CurrentOverlayDestRect;

    /*currentClientRect.setToClient(hWnd,TRUE);
    if (IsStatusBarVisible())
    {
        currentClientRect.bottom -= StatusBar_Height();
    }*/
    GetDisplayAreaRect(GetMainWnd(), &currentClientRect, TRUE);
    ClientToScreen(GetMainWnd(), (POINT*) &currentClientRect.left);
    ClientToScreen(GetMainWnd(), (POINT*) &currentClientRect.right);

    #ifdef __ASPECTFILTER_DEBUG__
        currentClientRect.DebugDump(_T("Current Client Rect"));
        newRect.DebugDump(_T("Target Client Rect"));
    #endif

    // Do we match????
    if (!currentClientRect.tolerantEquals(newRect,8))
    {
        // Nope!  Scale the existing window using _T("smart") logic to grow or shrink the window as needed
        RECT screenRect = {0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
        //
        // Laurent's comment
        // Maybe the two following lines should replace the previous one.
        //
        //RECT screenRect;
        //GetMonitorRect(hWnd, &screenRect);

        currentClientRect.adjustSourceAspectSmart(newRect.sourceAspect(),screenRect);

        #ifdef __ASPECTFILTER_DEBUG__
            currentClientRect.DebugDump(_T("New Client Rect"));
        #endif

        // Add the status bar back in...
        /*if (IsStatusBarVisible())
        {
            currentClientRect.bottom += StatusBar_Height();
        }*/

        currentClientRect.enforceMinSize(8);

        // Convert client rect to window rect...
        AddDisplayAreaRect(GetMainWnd(), &currentClientRect);

        OrigClientTop = currentClientRect.top;
        AdjustWindowRectEx(&currentClientRect,
                           GetWindowLong(GetMainWnd(), GWL_STYLE),
                           FALSE, /* we deal with the menu later */
                           GetWindowLong(GetMainWnd(), GWL_EXSTYLE));
        // Adjust for the menu bar
        // Workaround since AdjustWindowRectEx does not work correctly with a wrapped menu bar
        if(bShowMenu)
        {
            RECT TempRect = currentClientRect;
            SendMessage(GetMainWnd(), WM_NCCALCSIZE, FALSE, (LPARAM)(LPRECT)&TempRect);
            currentClientRect.top -= TempRect.top - OrigClientTop;
        }

        #ifdef __ASPECTFILTER_DEBUG__
            currentClientRect.DebugDump(_T("New Window Pos"));
        #endif

        // Set the window...
        SetWindowPos(GetMainWnd(), NULL,currentClientRect.left,currentClientRect.top,currentClientRect.width(),currentClientRect.height(),
                     SWP_NOZORDER);

        #ifdef __ASPECTFILTER_DEBUG__
            currentClientRect.setToClient(hWnd,TRUE);
            if (IsStatusBarVisible())
            {
                currentClientRect.bottom -= StatusBar_Height();
            }
            currentClientRect.DebugDump(_T("Actual New Client  "));
        #endif

        // Recalculate the overlay
        RequestRerun = TRUE;
    }
}

const TCHAR* CResizeWindowAspectFilter::getFilterName()
{
    return _T("CResizeWindowAspectFilter");
}

CMasterFilterChain::CMasterFilterChain(int SrcWidth, int SrcHeight)
{
    if (AspectSettings.bAnalogueBlanking)
    {
        m_FilterChain.push_back(SmartPtr<CAspectFilter>(new CAnalogueBlankingFilter(SrcWidth, SrcHeight)));
    }

    if (AspectSettings.OrbitEnabled)
    {
        // TO BE CHANGED (LG) : don't know what to put in m_Overscan
        // and why m_Overscan is not used after ?
        int m_Overscan = AspectSettings.InitialTopOverscan;
        if (AspectSettings.OrbitEnabled && m_Overscan*2 < AspectSettings.OrbitSize)
        {
            m_Overscan = (AspectSettings.OrbitSize+1)/2;
        }
        m_FilterChain.push_back(SmartPtr<CAspectFilter>(new COverscanAspectFilter(AspectSettings.InitialTopOverscan, AspectSettings.InitialBottomOverscan, AspectSettings.InitialLeftOverscan, AspectSettings.InitialRightOverscan)));
        m_FilterChain.push_back(SmartPtr<CAspectFilter>(new COrbitAspectFilter(AspectSettings.OrbitPeriodX, AspectSettings.OrbitPeriodY, AspectSettings.OrbitSize)));
    }
    else
    {
        m_FilterChain.push_back(SmartPtr<CAspectFilter>(new COverscanAspectFilter(AspectSettings.InitialTopOverscan, AspectSettings.InitialBottomOverscan, AspectSettings.InitialLeftOverscan, AspectSettings.InitialRightOverscan)));
    }
    if (AspectSettings.AspectMode)
    {
        SmartPtr<CAspectFilter> PosFilter;
        if (AspectSettings.BounceEnabled)
        {
            PosFilter = new CBounceDestinationAspectFilter(AspectSettings.BouncePeriod);
        }
        else
        {
            double m_XPos, m_YPos;
            switch (AspectSettings.HorizontalPos)
            {
            case HORZ_POS_LEFT:
                m_XPos = -1;
                break;
            case HORZ_POS_RIGHT:
                m_XPos = 1;
                break;
            default:
                m_XPos = 0;
                break;
            }
            switch (AspectSettings.VerticalPos)
            {
            case VERT_POS_TOP:
                m_YPos = -1;
                break;
            case VERT_POS_BOTTOM:
                m_YPos = 1;
                break;
            default:
                m_YPos = 0;
                break;
            }
            PosFilter = new CPositionDestinationAspectFilter(m_XPos,m_YPos);
        }

        PosFilter->SetChild(SmartPtr<CAspectFilter>(new CCropAspectFilter()));

        if (!AspectSettings.AspectImageClipped || AspectSettings.SquarePixels)
        {
            SmartPtr<CAspectFilter> UnCropFilter(new CUnCropAspectFilter());
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
    if (AspectSettings.ZoomFactorX != 100 || AspectSettings.ZoomFactorY != 100 ||
        AspectSettings.ZoomCenterX != 50 || AspectSettings.ZoomCenterY != 50)
    {
        m_FilterChain.push_back(SmartPtr<CAspectFilter>(new CPanAndZoomAspectFilter(AspectSettings.ZoomCenterX,
																				   AspectSettings.ZoomCenterY,
																				   AspectSettings.ZoomFactorX,
																				   AspectSettings.ZoomFactorY)));
    }

    // only put resize filter in if main window is not full screen
    if (AspectSettings.AutoResizeWindow && !bIsFullScreen)
    {
        m_FilterChain.push_back(SmartPtr<CAspectFilter>(new CResizeWindowAspectFilter()));
    }

    m_FilterChain.push_back(SmartPtr<CAspectFilter>(new CScreenSanityAspectFilter(SrcWidth, SrcHeight)));
}

// Applies all filters in a chain.  See above for return Value.
// If allowReadjust is FALSE, the filter will ignore any re-calculate requests from filters
// to avoid infinite recursion.
void CMasterFilterChain::adjustAspect(CAspectRectangles &ar, BOOL& RequestRerun)
{
    for(vector<SmartPtr<CAspectFilter> >::iterator it = m_FilterChain.begin();
        it != m_FilterChain.end();
        ++it)
    {
        (*it)->adjustAspect(ar, RequestRerun);
    }
}

const TCHAR* CMasterFilterChain::getFilterName()
{
    return _T("Master Filter Chain");
}

