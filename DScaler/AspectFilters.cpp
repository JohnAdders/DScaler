/////////////////////////////////////////////////////////////////////////////
// $Id: AspectFilters.cpp,v 1.27 2002-11-03 18:38:32 adcockj Exp $
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
// 03 Jul 2001   John Adcock           Put Filter code in a separate file
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.26  2002/10/31 14:03:33  adcockj
// Added Analogue blanking option to aspect code
//
// Revision 1.25  2002/10/11 13:40:52  kooiman
// Changed bounding screen rectangle to virtual screen. Maybe the overlay doesn't get clipped on multi-monitor setups.
//
// Revision 1.24  2002/09/18 11:38:05  kooiman
// Preparations for skinned dscaler look.
//
// Revision 1.23  2002/08/05 21:01:55  laurentg
// Square pixels mode updated
//
// Revision 1.22  2002/08/04 08:43:45  kooiman
// Moved the CResizeWindowAspectFilter before the ScreenSanity filter in the filter chain
//
// Revision 1.21  2002/03/20 11:30:20  robmuller
// Corrected window behaviour with a wrapped menu bar.
//
// Revision 1.20  2002/02/26 00:16:16  laurentg
// "Auto resize window" option not taken into account when in "Square Pixels" mode
//
// Revision 1.19  2002/02/25 22:42:23  laurentg
// Correction of a bug in method CUnCropAspectFilter::adjustAspect
//
// Revision 1.18  2002/02/23 19:07:06  laurentg
// New AR mode for stills having square pixels
//
// Revision 1.17  2002/02/23 12:00:13  laurentg
// Do nothing in WorkoutOverlaySize when source width or height is null
//
// Revision 1.16  2002/02/19 16:03:36  tobbej
// removed CurrentX and CurrentY
// added new member in CSource, NotifySizeChange
//
// Revision 1.15  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.14  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.13  2001/11/02 16:30:06  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.12  2001/09/08 15:17:58  adcockj
// Got Linear Correction working properly with squeeze
//
// Revision 1.11.2.1  2001/08/20 16:14:19  adcockj
// Massive tidy up of code to new structure
//
// Revision 1.11  2001/08/02 18:08:17  adcockj
// Made all logging code use new levels
//
// Revision 1.10  2001/07/29 10:06:42  adcockj
// Took out debug log for aspect ratio
//
// Revision 1.9  2001/07/18 18:45:32  adcockj
// Corrected file comment
//
// Revision 1.8  2001/07/18 18:38:12  adcockj
// Fixed crashing with off screen overlay
//
// Revision 1.7  2001/07/13 16:14:55  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.6  2001/07/12 16:16:39  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "AspectFilters.h"
#include "AspectRatio.h"
#include "Other.h"
#include "DScaler.h"
#include "Providers.h"

// From DScaler.cpp .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern BOOL bIsFullScreen;
extern BOOL bShowMenu;


//uncomment line below to get debug log for aspect ratio
//#define __ASPECTFILTER_DEBUG__

// Used to calculate positions for a given period and timing.
// Values go from Amplitude/2+Offset to Amplitude+Offset, down to Offset, and then back to Amplitude/2+Offset
// This results in a bounce between Offset and Amplitude+Offset, starting at the midpoint.

CPeriodBouncer::CPeriodBouncer(time_t period, double amplitude, double offset)
{ 
    time(&m_StartTime); 
    m_Period = period; 
    m_Amplitude = amplitude; 
    m_Offset = offset; 
}

CPeriodBouncer::CPeriodBouncer(time_t startTime, time_t period, double amplitude, double offset)
{ 
    m_StartTime = startTime; 
    m_Period = period; 
    m_Amplitude = amplitude; 
    m_Offset = offset;
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
// Original rectangles refer to the initial values of "Current" passed into the filter chain
// Current rectangles are the Value being used and adjusted.
// 
// ONLY CURRENT VALUES SHOULD BE ADJUSTED BY FILTERS!

void CAspectRectangles::DebugDump()
{
    m_CurrentOverlaySrcRect.DebugDump("SRC ");
    m_CurrentOverlayDestRect.DebugDump("DEST");
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
//    The TRUE Value is currently only used by the filter which adjusts the window rectangle
//    as this adjustment affects all calculations.  Current implementation only allows 
//    1 level of re-calculate requests.

void CAspectFilter::DebugDump()
{
    ; // empty if not implemented
}
void CAspectFilter::SetChild(CAspectFilter* Child)
{
    m_Child = Child;
}


COverscanAspectFilter::COverscanAspectFilter(int overscanSize)
{
    m_Overscan = overscanSize;
}

BOOL COverscanAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    ar.m_CurrentOverlaySrcRect.shrink(m_Overscan);
    return FALSE;
}

LPCSTR COverscanAspectFilter::getFilterName()
{
    return "COverscanAspectFilter";
}
void COverscanAspectFilter::DebugDump()
{ 
    LOG(2,"Overscan = %i",m_Overscan);
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
        m_TopShift = 1;
        m_BottomShift = 1;
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

BOOL CAnalogueBlankingFilter::adjustAspect(CAspectRectangles &ar)
{
    ar.m_CurrentOverlaySrcRect.shrink(m_LeftShift, m_RightShift, m_TopShift, m_BottomShift);
    return FALSE;
}

LPCSTR CAnalogueBlankingFilter::getFilterName()
{
    return "CAnalogueBlankingFilter";
}

void CAnalogueBlankingFilter::DebugDump()
{ 
    LOG(2,"AnalogueBlanking %d %d %d %d", m_TopShift, m_BottomShift, m_LeftShift, m_RightShift);
}


// This filter orbits the source image using independent X and Y timers.
// It is assumed that bouncing is enabled if this filter is in the chain.
// The bounce comes out of the m_Overscan (the m_Overscan filter performs a sanity check to ensure the m_Overscan is of appropriate size.
COrbitAspectFilter::COrbitAspectFilter(time_t OrbitPeriodX, time_t OrbitPeriodY, long OrbitSize)
{
    if (AspectSettings.BounceStartTime == 0)
    {
        time(&AspectSettings.BounceStartTime);
    }
    m_pXOrbitBouncer = new CPeriodBouncer(AspectSettings.BounceStartTime,OrbitPeriodX,OrbitSize,-OrbitSize/2.0);
    m_pYOrbitBouncer = new CPeriodBouncer(AspectSettings.BounceStartTime,OrbitPeriodY,OrbitSize,-OrbitSize/2.0);
}

COrbitAspectFilter::~COrbitAspectFilter()
{ 
    delete m_pXOrbitBouncer; 
    delete m_pYOrbitBouncer; 
}

BOOL COrbitAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    ar.m_CurrentOverlaySrcRect.shift((int)m_pXOrbitBouncer->position(),
                                (int)m_pYOrbitBouncer->position());
    return FALSE;
}

LPCSTR COrbitAspectFilter::getFilterName()
{
    return "COrbitAspectFilter";
}

void COrbitAspectFilter::DebugDump()
{
    LOG(2,"m_pXOrbitBouncer = %lf, m_pYOrbitBouncer = %lf",m_pXOrbitBouncer->position(), m_pYOrbitBouncer->position());
}

CBounceDestinationAspectFilter::CBounceDestinationAspectFilter(time_t period)
{
    if (AspectSettings.BounceStartTime == 0)
    {
        time(&AspectSettings.BounceStartTime);
    }
    m_pBouncer = new CPeriodBouncer(
        AspectSettings.BounceStartTime, period,
        // BounceAmplitude ranges from 0 to 100, we want 0 to 2.
        2.0 * (double)AspectSettings.BounceAmplitude / 100.0,
        // BounceAmplitude ranges from 0 to 100, we want 0 to -1.
        -1.0 * (double)AspectSettings.BounceAmplitude / 100.0);
}

CBounceDestinationAspectFilter::~CBounceDestinationAspectFilter()
{
    delete m_pBouncer;
}

BOOL CBounceDestinationAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    CAspectRect oldDest = ar.m_CurrentOverlayDestRect;

    m_Child->adjustAspect(ar);

    double pos = m_pBouncer->position();
    ar.m_CurrentOverlayDestRect.shift((int) (((oldDest.width()-ar.m_CurrentOverlayDestRect.width())*pos)/2),
                                 (int) (((oldDest.height()-ar.m_CurrentOverlayDestRect.height())*pos)/2));
    return FALSE;
}

LPCSTR CBounceDestinationAspectFilter::getFilterName()
{
    return "CBounceDestinationAspectFilter"; 
}

void CBounceDestinationAspectFilter::DebugDump()
{
    LOG(2,"position = %lf",m_pBouncer->position());
}

// Applys child filters than adjusts the position of the destination rectangle - this class fixed floating point positions
// from -1 to 1 (0 = centered, -1 = left/top +1 = right/bottom
CPositionDestinationAspectFilter::CPositionDestinationAspectFilter(double x, double y)
{
    m_XPos = x; 
    m_YPos = y;
}

BOOL CPositionDestinationAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    #ifdef __ASPECTFILTER_DEBUG__
        LOG(2,"PRE FILTER VALUES: %s",this->getFilterName());
        ar.DebugDump();
    #endif

    CAspectRect oldDest = ar.m_CurrentOverlayDestRect;

    m_Child->adjustAspect(ar);

    ar.m_CurrentOverlayDestRect.shift((int) (((oldDest.width()-ar.m_CurrentOverlayDestRect.width())*m_XPos)/2),
                                 (int) (((oldDest.height()-ar.m_CurrentOverlayDestRect.height())*m_YPos)/2));

    #ifdef __ASPECTFILTER_DEBUG__
        LOG(2,"POST FILTER VALUES: %s",this->getFilterName());
        ar.DebugDump();
    #endif

    return FALSE;
}

LPCSTR CPositionDestinationAspectFilter::getFilterName() 
{ 
    return "CPositionDestinationAspectFilter"; 
}

void CPositionDestinationAspectFilter::DebugDump()
{
    LOG(2,"m_XPos = %lf, m_YPos = %lf",m_XPos,m_YPos); 
}

BOOL CCropAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    #ifdef __ASPECTFILTER_DEBUG__
        LOG(2,"PRE FILTER VALUES: %s",this->getFilterName());
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
        LOG(2,"POST FILTER VALUES: %s",this->getFilterName());
        ar.DebugDump();
    #endif

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
    CAspectRect rOriginalDest(ar.m_CurrentOverlayDestRect);
    CAspectRect rOriginalSrc(ar.m_CurrentOverlaySrcRect);

    // Apply sub-filters
    m_Child->adjustAspect(ar);
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

    return FALSE;
}

LPCSTR CUnCropAspectFilter::getFilterName()
{ 
    return "CUnCropAspectFilter"; 
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

BOOL CPanAndZoomAspectFilter::adjustAspect(CAspectRectangles &ar)
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

    return FALSE;
}

LPCSTR CPanAndZoomAspectFilter::getFilterName()
{
    return "CPanAndZoomAspectFilter";
}
void CPanAndZoomAspectFilter::DebugDump()
{ 
    LOG(2,"m_XPos = %lf, m_YPos = %lf, m_XZoom = %lf, m_YZoom = %lf",m_XPos,m_YPos,m_XZoom,m_YZoom);
}

CScreenSanityAspectFilter::CScreenSanityAspectFilter(int SrcWidth, int SrcHeight)
{
    m_SrcWidth = SrcWidth;
    m_SrcHeight = SrcHeight;
}

// Performs important sanity checks on the destination rectangle
// Should occur at the end of the aspect processing chain (but before the ResizeWindow filter)
BOOL CScreenSanityAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    // crop the Destination rect so that the overlay destination region is 
    // always on the screen we will also update the source area to reflect this
    // so that we see the appropriate portion on the screen
    // (this should make us compatable with YXY)        
    RECT screenRect = {0,0,GetSystemMetrics(SM_CXVIRTUALSCREEN),GetSystemMetrics(SM_CYVIRTUALSCREEN) };
    RECT sourceRect = {0, 0, m_SrcWidth, m_SrcHeight};

    if ((screenRect.right == 0) || (screenRect.bottom == 0))
    {
        screenRect.right = GetSystemMetrics(SM_CXSCREEN);
        screenRect.bottom = GetSystemMetrics(SM_CYSCREEN);
    }
    

    ar.m_CurrentOverlayDestRect.crop(screenRect,&ar.m_CurrentOverlaySrcRect);

    // then make sure we are still onscreen
    ar.m_CurrentOverlayDestRect.cropToFitRect(screenRect);

    // then make sure we are still onscreen
    ar.m_CurrentOverlaySrcRect.cropToFitRect(sourceRect);

    // make sure that any alignment restrictions are taken care of
    if (SrcSizeAlign > 1)
    {
        ar.m_CurrentOverlaySrcRect.align(SrcSizeAlign);
    }
    if (DestSizeAlign > 1)
    {
        ar.m_CurrentOverlayDestRect.align(DestSizeAlign);
    }

    // Ensure we do not shrink too small...avoids crashes when window gets too small
    ar.m_CurrentOverlayDestRect.enforceMinSize(1);
    ar.m_CurrentOverlaySrcRect.enforceMinSize(1);
    return FALSE;
}

LPCSTR CScreenSanityAspectFilter::getFilterName()
{ 
    return "CScreenSanityAspectFilter";
}

// Attemtps to resize the client window to match the aspect ratio
BOOL CResizeWindowAspectFilter::adjustAspect(CAspectRectangles &ar)
{
    LONG OrigClientTop = 0;

    if (!bIsFullScreen) 
    {
        // See if we need to resize the window
        CAspectRect currentClientRect;
        CAspectRect newRect = ar.m_CurrentOverlayDestRect;
            
        /*currentClientRect.setToClient(hWnd,TRUE);
        if (IsStatusBarVisible())
        {
            currentClientRect.bottom -= StatusBar_Height();
        }*/
        GetDisplayAreaRect(hWnd,&currentClientRect);
        ClientToScreen(hWnd, (POINT*) &currentClientRect.left);
        ClientToScreen(hWnd, (POINT*) &currentClientRect.right);

        #ifdef __ASPECTFILTER_DEBUG__
            currentClientRect.DebugDump("Current Client Rect");
            newRect.DebugDump("Target Client Rect");
        #endif

        // Do we match????
        if (!currentClientRect.tolerantEquals(newRect,8))
        {
            // Nope!  Scale the existing window using "smart" logic to grow or shrink the window as needed
            RECT screenRect = {0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };

            currentClientRect.adjustSourceAspectSmart(newRect.sourceAspect(),screenRect);
            
            #ifdef __ASPECTFILTER_DEBUG__
                currentClientRect.DebugDump("New Client Rect");
            #endif
                        
            // Add the status bar back in...
            /*if (IsStatusBarVisible())
            {
                currentClientRect.bottom += StatusBar_Height();     
            }*/
                        
            currentClientRect.enforceMinSize(8);
            
            // Convert client rect to window rect...
            AddDisplayAreaRect(hWnd,&currentClientRect);

            OrigClientTop = currentClientRect.top;
            AdjustWindowRectEx(&currentClientRect,
                               GetWindowLong(hWnd,GWL_STYLE),
                               FALSE, /* we deal with the menu later */
                               GetWindowLong(hWnd, GWL_EXSTYLE));
            // Adjust for the menu bar
            // Workaround since AdjustWindowRectEx does not work correct with a wrapped menu bar
            if(bShowMenu)
            {
                RECT TempRect = currentClientRect;
                SendMessage(hWnd, WM_NCCALCSIZE, FALSE, (LPARAM)(LPRECT)&TempRect);
                currentClientRect.top -= TempRect.top - OrigClientTop;
            }
            
            #ifdef __ASPECTFILTER_DEBUG__
                currentClientRect.DebugDump("New Window Pos");
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
                currentClientRect.DebugDump("Actual New Client  ");
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

void CFilterChain::BuildFilterChain(int SrcWidth, int SrcHeight)
{
    if (AspectSettings.bAnalogueBlanking)
    { 
        m_FilterChain.push_back(new CAnalogueBlankingFilter(SrcWidth, SrcHeight)); 
    }

    if (AspectSettings.OrbitEnabled)
    { 
        int m_Overscan = AspectSettings.InitialOverscan;
        if (AspectSettings.OrbitEnabled && m_Overscan*2 < AspectSettings.OrbitSize)
        {
            m_Overscan = (AspectSettings.OrbitSize+1)/2;
        }
        m_FilterChain.push_back(new COverscanAspectFilter(AspectSettings.InitialOverscan));
        m_FilterChain.push_back(new COrbitAspectFilter(AspectSettings.OrbitPeriodX, AspectSettings.OrbitPeriodY, AspectSettings.OrbitSize)); 
    }
    else 
    {
        m_FilterChain.push_back(new COverscanAspectFilter(AspectSettings.InitialOverscan));
    }
    if (AspectSettings.AspectMode)
    { 
        CAspectFilter* PosFilter;
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
        
        PosFilter->SetChild(new CCropAspectFilter());

        if (!AspectSettings.AspectImageClipped || AspectSettings.SquarePixels)
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
    if (AspectSettings.ZoomFactorX != 100 || AspectSettings.ZoomFactorY != 100 ||
        AspectSettings.ZoomCenterX != 50 || AspectSettings.ZoomCenterY != 50)
    {
        m_FilterChain.push_back(new CPanAndZoomAspectFilter(AspectSettings.ZoomCenterX,
                                               AspectSettings.ZoomCenterY,
                                               AspectSettings.ZoomFactorX,
                                               AspectSettings.ZoomFactorY));
    }
    
    if (AspectSettings.AutoResizeWindow)
    {
        m_FilterChain.push_back(new CResizeWindowAspectFilter());
    }

    m_FilterChain.push_back(new CScreenSanityAspectFilter(SrcWidth, SrcHeight));
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

// Applies all filters in a chain.  See above for return Value.
// If allowReadjust is FALSE, the filter will ignore any re-calculate requests from filters
// to avoid infinite recursion.
BOOL CFilterChain::ApplyFilters(CAspectRectangles &ar, BOOL allowReadjust)
{
    for(vector<CAspectFilter*>::iterator it = m_FilterChain.begin();
        it != m_FilterChain.end();
        ++it)
    {
        #ifdef __ASPECTFILTER_DEBUG__
            LOG(2,"PRE FILTER VALUES: %s",(*it)->getFilterName());
            (*it)->DebugDump();
            ar.DebugDump();
        #endif
        BOOL readjust = (*it)->adjustAspect(ar);
        #ifdef __ASPECTFILTER_DEBUG__
            LOG(2,"POST FILTER VALUES: %s",(*it)->getFilterName());
            ar.DebugDump();
            if (readjust)
            {
                if (allowReadjust)
                {
                    LOG(2,"READJUST REQUESTED\n");
                }
                else
                {
                    LOG(2,"READJUST REQUESTED BUT NOT ALLOWED\n");
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
