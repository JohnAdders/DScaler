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

    // Update aspect ratio only if a positive one is specified
    if (nRatio > 0)
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
// Scan the top or bottom of a letterboxed image to find out where the picture
// starts.
//
// The basic idea is that we find a bounding rectangle for the image (which
// is assumed to be centered in the overlay buffer, an assumption the aspect
// ratio code makes in general) by searching from the top down to find the first
// scanline that isn't all black.  "All black" means that there aren't many
// pixels with luminance above a certain threshold.
//
// To support letterboxed movies shown on TV channels that put little channel
// logos in the corner, we allow the user to configure a percentage of the
// image that will be ignored on either side.  We only look at pixels in the
// middle of the image, so channel logos will be ignored.  The default is 15%.
// This is in addition to any pixels we ignore due to the overscan setting.
//
// For speed's sake, the code is a little sloppy at the moment; it always
// looks at a multiple of 4 pixels.  It never looks at *more* pixels than are
// requested, though.
static inline int GetNonBlackCount(short *Line, int StartX, int EndX)
{
	int qwordCount = (EndX - StartX) / 4;
	int threshold;
	int counts;
	__int64 luminances;
	__int64 chromaMins;
	__int64 chromaMaxes;
	const __int64 YMask = 0x00FF00FF00FF00FF;
	const __int64 OneMask = 0x0001000100010001;
	int chromaMin, chromaMax;

	if (qwordCount <= 0)
		return 0;

	// Black pixels are U=128 and V=128.  If U or V is significantly
	// different from 128, it's a different color, e.g. green if
	// they're both 0.  We compare each U and V value to the chroma
	// min and max computed here: if ((chroma - chromaMin) > chromaMax)...
	chromaMax = aspectSettings.ChromaRange;
	if (chromaMax > 255)
		chromaMax = 255;
	chromaMin = 128 - (chromaMax / 2);

	threshold = aspectSettings.LuminanceThreshold;
	if (threshold > 255)
		threshold = 255;

	luminances = threshold;
	luminances |= (luminances << 48) | (luminances << 32) | (luminances << 16);
	chromaMins = chromaMin;
	chromaMins |= (chromaMins << 48) | (chromaMins << 32) | (chromaMins << 16);
	chromaMaxes = chromaMax;
	chromaMaxes |= (chromaMaxes << 48) | (chromaMaxes << 32) | (chromaMaxes << 16);

	// Start on a 16-byte boundary even if it means ignoring some extra
	// pixels?  Try not doing it for now, but it could make memory access
	// more efficient.

	Line += StartX;

	_asm {
		mov		ecx, qwordCount
		mov		eax, dword ptr[Line]
		movq	mm1, YMask
		movq	mm2, luminances
		movq	mm3, OneMask
		pxor	mm4, mm4			// mm4 = pixel counts, one count in each word
		movq	mm6, chromaMins
		movq	mm7, chromaMaxes

BlackLoop:
		movq	mm0, qword ptr[eax]	// mm0 = next 4 pixels
		movq	mm5, mm0
		psrlw	mm5, 8				// mm5 = chroma values in lower 8 bits of each word
		pand	mm0, mm1			// mm0 = luminance values in lower 8 bits of each word
		pcmpgtw	mm0, mm2			// mm0 = 0xFFFF where luminance > threshold
		psubw	mm5, mm6			// mm5 = chroma - chromaMin
		pand	mm5, mm1			// mm5 = lower 8 bits of (chroma - chromaMin); <0 becomes positive
		pcmpgtw	mm5, mm7			// mm5 = 0xFFFF where chroma < chromaMin or chroma > chromaMax
		por		mm0, mm5			// mm0 = 0xFFFF where U, V, or Y is past threshold
		pand	mm0, mm3			// Translate that into 1 for exceed, 0 for not, in each word
		paddw	mm4, mm0			// And add up the ones, maintaining four separate counts
		add		eax, 8				// Next qword
		loop	BlackLoop

		// Now add up all four words in mm4 to get the total overall count.
		pextrw	ebx, mm4, 0
		pextrw	eax, mm4, 1
		add		ebx, eax
		pextrw	eax, mm4, 2
		add		ebx, eax
		pextrw	eax, mm4, 3
		add		ebx, eax
		mov		counts, ebx

		emms
	}

	/*
	// Log the offending pixels
	if (counts > 0)	{
		int x;
		LOG("count %d min %d max %d lumthresh %d", counts, chromaMin, chromaMax, threshold);
		for (x = 0; x < qwordCount * 4; x++) {
			if ((Line[x] & 0xff) > threshold || ((((Line[x] & 0xff00) >> 8) - chromaMin) & 0xff) > chromaMax)
			{
				LOG("pixel %d lum %d chrom %d", x, Line[x] & 0xff, (Line[x] & 0xff00) >> 8);
			}
		}
	}
	*/

	return counts;
}

// direction is -1 to scan up from bottom of image, 1 to scan down from top.
int FindEdgeOfImage(short** EvenField, short **OddField, int direction)
{
	int y;
	int maxY = CurrentY / 2 - aspectSettings.InitialOverscan;
	int maxX;
	short *line;
	int skipCount = 0;
	int skipCountPercent = aspectSettings.SkipPercent;
	int pixelCount;

	skipCount = (CurrentX * aspectSettings.SkipPercent / 100) +
				aspectSettings.InitialOverscan;
	maxX = CurrentX - skipCount * 2;

	// Decide whether we're scanning from the top or bottom
	if (direction < 0)
	{
		y = CurrentY - aspectSettings.InitialOverscan * 2;	// from bottom
	}
	else
	{
		y = aspectSettings.InitialOverscan;	// from top
	}

	for (; y != CurrentY / 2; y += direction)
	{
		if (y & 1)
			line = OddField[y / 2];
		else
			line = EvenField[y / 2];

		pixelCount = GetNonBlackCount(line, skipCount, CurrentX - skipCount * 2);
		if (pixelCount > 0)
		{
			LOG("FindEdgeOfImage line %d count %d", y, pixelCount);
		}

		if (pixelCount > aspectSettings.IgnoreNonBlackPixels)
			break;

		if (y < 0 || y > CurrentY)
		{
			LOG("Sanity check failed; scanned past edge of screen");
			y = (direction > 0) ? aspectSettings.InitialOverscan : CurrentY - aspectSettings.InitialOverscan;
			break;
		}
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
	topBorder = FindEdgeOfImage(EvenField, OddField, 1) - aspectSettings.InitialOverscan;

	// Now find the size of the border at the bottom of the image.
	bottomBorder = CurrentY - FindEdgeOfImage(EvenField, OddField, -1) - aspectSettings.InitialOverscan;

	// The border size is the smaller of the two.
	border = (topBorder < bottomBorder) ? topBorder : bottomBorder;

	// Now the material aspect ratio is simply
	//
	//	effective width / (total image height - number of black lines at top and bottom)
	//
	// We compute effective width from height using the source-frame aspect ratio, since
	// this will change depending on whether or not the image is anamorphic.
	ratio = (int)((imageHeight * 1000) * GetActualSourceFrameAspect() / (imageHeight - border * 2));
	LOG(" top %d bot %d bord %d rat %d", topBorder, bottomBorder, border, ratio);

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

