/////////////////////////////////////////////////////////////////////////////
// AspectDetect.cpp
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
// Aspect ratio control was started by Michael Samblanet <mike@cardobe.com>
// Moved into separate module by Mark D Rejhon.  
// AspectRatio.c was separated into 3 files by Michael Samblanet 13 March 2001
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
// 13 Mar 2001   Michael Samblanet     File created from code in AspectRatio.c
//									   Split out to improve redability
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
#include "VBI_WSSdecode.h"

// From dtv.c .... We really need to reduce reliance on globals by going C++!
// Perhaps in the meantime, it could be passed as a parameter to WorkoutOverlay()
extern HMENU hMenu;
extern BOOL  bIsFullScreen;
extern void ShowText(HWND hWnd, LPCTSTR szText);

//---------
// Internal variables used by auto aspect ratio detect code.

// Number of seconds of aspect ratio history to keep.
#define RATIO_HISTORY_SECONDS	600

// Number of aspect ratio changes to remember.
#define RATIO_HISTORY_CHANGES	50

// Minimum aspect ratio encountered each second for the last several minutes.
// The 0th element of this is the current second.
static int min_ratio_found[RATIO_HISTORY_SECONDS];

// Timestamp of the most recent computation of min_ratio_found.
static int min_ratio_tick_count = 0;

// Aspect ratios we've used recently.
static int ratio_used[RATIO_HISTORY_CHANGES];

// When we switched to each of the ratios in ratio_used[].
static int ratio_time[RATIO_HISTORY_CHANGES];



//----------------------------------------------------------------------------
// Switch to a new aspect ratio and record it in the ratio history list.
// Use nRatio = -1 to only switch between anamorphic/nonanamorphic
void SwitchToRatio(int nMode, int nRatio)
{
	int now = GetTickCount();

    // Update anamorphic/nonanamorphic status
	aspectSettings.aspect_mode = nMode;

    // Update aspect ratio only if a nonnegative one is specified
    if (nRatio >= 0)
    {
    	LOG(" Switching to ratio %d", nRatio);

	    // If the most recent ratio switch just happened, don't remember it since it
	    // was probably a transient ratio due to improperly locking onto a dark scene.
	    if (now - ratio_time[0] > aspectSettings.ShortRatioIgnoreMs)
	    {
		    memmove(&ratio_used[1], &ratio_used[0], sizeof(ratio_used[0]) * (RATIO_HISTORY_CHANGES - 1));
		    memmove(&ratio_time[1], &ratio_time[0], sizeof(ratio_time[0]) * (RATIO_HISTORY_CHANGES - 1));
	    }

	    ratio_used[0] = nRatio;
	    ratio_time[0] = GetTickCount();
	    aspectSettings.source_aspect = nRatio;
    }
	WorkoutOverlaySize();
}

//----------------------------------------------------------------------------
// Scan the top of a letterboxed image to find out where the picture starts.
//
// The basic idea is that we find a bounding rectangle for the image (which
// is assumed to be centered in the overlay buffer, an assumption the aspect
// ratio code makes in general) by searching from the top down to find the first
// scanline that isn't all black.  "All black" means that there aren't many
// pixels with luminance above a certain threshold.
//
// To support letterboxed movies shown on TV channels that put little channel
// logos in the corner, we allow the user to configure a maximum number of
// non-black pixels which will be ignored for purposes of the computation.
// By default this is 15% of the horizontal capture resolution, which will
// produce good results if the aspect ratio analysis is done on a bright scene.
//
// This function can almost certainly be made more efficient using MMX
// instructions.
int FindTopOfImage(short** EvenField, short **OddField)
{
	int y, x;
	int maxX = CurrentX - aspectSettings.InitialOverscan * 2;
	int maxY = CurrentY / 2 - aspectSettings.InitialOverscan;
	BYTE *pixel;
	int ignoreCount = 0;
	int ignoreCountPercent = aspectSettings.IgnoreNonBlackPixels;
	int pixelCount;
	const int BytesBetweenLuminanceValues = 2;	// just for clarity's sake
	const int SkipPixels = aspectSettings.SkipPixels;	// check fewer pixels to reduce CPU hit

	if (ignoreCountPercent == -1)
	{
		// The user didn't specify an ignore count.  Default to ~16%
		// of the horizontal size of the image.
		ignoreCountPercent = 16;
	}
	if (ignoreCountPercent > 0)
	{
		ignoreCount = CurrentX * ignoreCountPercent / SkipPixels / 100;
		if (ignoreCount == 0)
			ignoreCount = 1;
	}

	for (y = aspectSettings.InitialOverscan; y < maxY; y++)
	{
		if (y & 1)
			pixel = (BYTE *)OddField[y / 2];
		else
			pixel = (BYTE *)EvenField[y / 2];

		pixel += aspectSettings.InitialOverscan * BytesBetweenLuminanceValues;

		pixelCount = -ignoreCount;
		for (x = aspectSettings.InitialOverscan; x < maxX; x += SkipPixels)
		{
			if (((*pixel) & 0x7f) > aspectSettings.LuminanceThreshold)
				pixelCount++;
			pixel += BytesBetweenLuminanceValues * SkipPixels;
		}

		if (pixelCount > 0)
			break;
	}

	return y;
}

//----------------------------------------------------------------------------
int FindBottomOfImage(short** EvenField, short** OddField)
{
	int y, x;
	int maxX = CurrentX - aspectSettings.InitialOverscan * 2;
	int maxY = CurrentY - aspectSettings.InitialOverscan * 2;
	BYTE *pixel;
	int ignoreCount = 0;
	int ignoreCountPercent = aspectSettings.IgnoreNonBlackPixels;
	int pixelCount;
	const int BytesBetweenLuminanceValues = 2;	// just for clarity's sake
	const int SkipPixels = aspectSettings.SkipPixels;	// check fewer pixels to reduce CPU hit

	if (ignoreCountPercent == -1)
	{
		// The user didn't specify an ignore count.  Default to ~16%
		// of the horizontal size of the image.
		ignoreCountPercent = 16;
	}
	if (ignoreCountPercent > 0)
	{
		ignoreCount = CurrentX * ignoreCountPercent / SkipPixels / 100;
		if (ignoreCount == 0)
			ignoreCount = 1;
	}

	for (y = maxY - 1; y >= maxY / 2; y--)
	{
		if (y & 1)
			pixel = (BYTE *)OddField[y / 2];
		else
			pixel = (BYTE *)EvenField[y / 2];

		pixel += aspectSettings.InitialOverscan * BytesBetweenLuminanceValues;

		pixelCount = -ignoreCount;
		for (x = aspectSettings.InitialOverscan; x < maxX; x += SkipPixels)
		{
			if (((*pixel) & 0x7f) > aspectSettings.LuminanceThreshold)
				pixelCount++;
			pixel += BytesBetweenLuminanceValues * SkipPixels;
		}

		if (pixelCount > 0)
			break;
	}

	return y;
}


//----------------------------------------------------------------------------
// Adjust the source aspect ratio to fit whatever is currently onscreen.
int FindAspectRatio(short** EvenField, short** OddField)
{
	int ratio;
	int topBorder, bottomBorder, border;
	int imageHeight = CurrentY - aspectSettings.InitialOverscan * 2;

	// If the aspect mode is set to "use source", revert to assuming that the
	// source frame is 4:3.  We have to assume *some* source-frame aspect ratio
	// here or there aren't enough inputs to derive the material aspect ratio.
	if (aspectSettings.aspect_mode == 0)
		aspectSettings.aspect_mode = 1;

	// Find the top of the image relative to the overscan area.  Overscan has to
	// be discarded from the computations since it can't really be regarded as
	// part of the picture.
	topBorder = FindTopOfImage(EvenField, OddField) - aspectSettings.InitialOverscan;

	// Now find the size of the border at the bottom of the image.
	bottomBorder = CurrentY - FindBottomOfImage(EvenField, OddField) - aspectSettings.InitialOverscan * 2;

	// The border size is the smaller of the two.
	border = (topBorder < bottomBorder) ? topBorder : bottomBorder;

	// Now the material aspect ratio is simply
	//
	//	effective width / (total image height - number of black lines at top and bottom)
	//
	// We compute effective width from height using the source-frame aspect ratio, since
	// this will change depending on whether or not the image is anamorphic.
	ratio = (int)((imageHeight * 1000) * GetActualSourceFrameAspect() / (imageHeight - border * 2));
	//LOG(" top %d bot %d bord %d rat %d", topBorder, bottomBorder, border, ratio);

	return ratio;
}

//----------------------------------------------------------------------------
// Automatic Aspect Ratio Detection
// Continuously adjust the source aspect ratio.  This is called once per frame.
void AdjustAspectRatio(short** EvenField, short** OddField)
{
	static int lastNewRatio = 0;
	static int newRatioFrameCount = 0;
	int newRatio, newMode;
	int tick_count = GetTickCount();
	int tickCutoff = tick_count - (aspectSettings.AspectHistoryTime * 1000);
	int i;
	int haveSeenThisRatio, haveSeenSmallerRatio;
	int WssSourceRatio;
	int maxRatio;

	if(EvenField == NULL || OddField == NULL)
	{
		return;
	}

	// ADDED by Mark Rejhon: Eliminates the "tiny slit" problem in starry 
	// scenes such as those in Star Wars or start of Toy Story 2,
	// at least during full screen mode.
	// FIXME: Would be nice to access 'AdjustedWindowAspect' in WorkoutOverlaySize()
	// so that I can also do this for windowed mode too.
	if (aspectSettings.DetectAspectNow || aspectSettings.AutoDetectAspect)
	{
		newRatio = FindAspectRatio(EvenField, OddField);

		// Get aspect ratio from WSS data
		// (WssSourceRatio = -1 if ratio is not defined in WSS data)
		if (! WSS_GetRecommendedAR(&newMode, &WssSourceRatio))
		{
			newMode = aspectSettings.aspect_mode;
			WssSourceRatio = -1;
		}

		// If source is anamorphic
		if (newMode == 2)
		{
			// Convert ratio to a 16/9 ratio
			newRatio *= 1333 / 1000;
		}

		// The ratio must be at least the ratio defined in WSS data
		if (newRatio < WssSourceRatio)
		{
			newRatio = WssSourceRatio;
		}

		if (bIsFullScreen && aspectSettings.target_aspect)
		{
			if (aspectSettings.target_aspect > WssSourceRatio)
				maxRatio = aspectSettings.target_aspect;
			else
				maxRatio = WssSourceRatio;
			if (newRatio > maxRatio)
			{
				newRatio = maxRatio;
			}
		}
	}

	// If the user told us to detect the current ratio, do it.
	if (aspectSettings.DetectAspectNow)
	{

		SwitchToRatio(newMode, newRatio);
		newRatioFrameCount = 0;
		aspectSettings.DetectAspectNow = FALSE;
		return;
	}
	else if (aspectSettings.AutoDetectAspect)
	{
		// If we've just crossed a 1-second boundary, scroll the aspect ratio
		// histories.  If not, update the max ratio found in the current second.
		if (tick_count / 1000 != min_ratio_tick_count / 1000)
		{
			min_ratio_tick_count = tick_count;
			memmove(&min_ratio_found[1], &min_ratio_found[0], sizeof(min_ratio_found[0]) * (RATIO_HISTORY_SECONDS - 1));
			min_ratio_found[0] = newRatio;
		}
		else if (newRatio < min_ratio_found[0])
		{
			min_ratio_found[0] = newRatio;
		}

		// If the mode (anamorphic/non anamorphic) changes, switch to the new
		// mode and ratio immediately
		if (newMode != aspectSettings.aspect_mode)
		{
			SwitchToRatio(newMode, newRatio);
			newRatioFrameCount = 0;
		}
		// If the new ratio is less than the old one -- that is, if we've just
		// become less letterboxed -- switch to the new ratio immediately to
		// avoid cutting the image off.
		else if (newRatio < aspectSettings.source_aspect)
		{
			SwitchToRatio(newMode, newRatio);
			newRatioFrameCount = 0;
		}
		else if (ABS(newRatio - aspectSettings.source_aspect) > aspectSettings.AspectEqualFudgeFactor && newRatio == lastNewRatio)
		{
			// Require the same aspect ratio for some number of frames, or no
			// narrower aspect ratio found for the last AspectConsistencyTime seconds,
			// before zooming in.
			haveSeenSmallerRatio = 0;
			for (i = 1; i < aspectSettings.AspectConsistencyTime; i++)
				if (newRatio > min_ratio_found[i])
				{
					haveSeenSmallerRatio = 1;
					break;
				}

			if (! haveSeenSmallerRatio || ++newRatioFrameCount >= aspectSettings.ZoomInFrameCount)
			{
				// If we're looking at aspect ratio histories, the new ratio must be
				// equal (or very close) to one we've _used_ in the recent past, or
				// there must have been no smaller ratio _found_ in the recent past.
				// That is, don't zoom in more than we've zoomed in recently unless
				// we'd previously zoomed to the same ratio.  This helps prevent
				// temporary zooms into letterboxed material during dark scenes, while
				// allowing the code to quickly switch in and out of zoomed mode when
				// full-frame commercials come on.
				haveSeenThisRatio = 0;
				if (aspectSettings.AspectHistoryTime > 0)
				{
					for (i = 1; i < RATIO_HISTORY_CHANGES && ratio_time[i] > tickCutoff; i++)
						if (ABS(newRatio - ratio_used[i]) <= aspectSettings.AspectEqualFudgeFactor)
						{
							haveSeenThisRatio = 1;
							break;
						}
				}

				if (aspectSettings.AspectConsistencyTime <= 0 ||
					(haveSeenThisRatio && newRatioFrameCount >= aspectSettings.ZoomInFrameCount) ||
					! haveSeenSmallerRatio)
				{
					SwitchToRatio(newMode, newRatio);
				}
			}
		}
		else
		{
			// If this is a wider ratio than the previous one, require it to stick
			// around for the full frame count.
			if (lastNewRatio < newRatio)
				newRatioFrameCount = 0;

			lastNewRatio = newRatio;
		}
	}
}

