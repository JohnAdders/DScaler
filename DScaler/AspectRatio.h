/////////////////////////////////////////////////////////////////////////////
// AspectRatio.h
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
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 12 Sep 2000   Mark Rejhon           Centralized aspect ratio code
//                                     into separate module
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 13 Mar 2001   Michael Samblanet     Split AspectRatio.c into 3 files
//									   Moved settings into a structure, declared some functions
// 08 Jun 2001   Eric Schmidt          added bounce amplitude to ini
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _ASPECT_H_
#define _ASPECT_H_ 1

#include "settings.h"

// Get Hold of the AspectRatio.c file settings
SETTING* Aspect_GetSetting(ASPECT_SETTING Setting);
void Aspect_ReadSettingsFromIni();
void Aspect_WriteSettingsToIni();
void Aspect_FinalSetup();
void Aspect_ShowUI();

#define DEFAULT_OVERSCAN_NTSC 12
#define DEFAULT_OVERSCAN_PAL 4

#define ABS(x) ((x) < 0 ? -(x) : (x))

// The HORIZ_POS and VERT_POS are ordered in reverse
// because it works better during increment/decrement operations
typedef enum
{
	VERT_POS_BOTTOM = 0,
	VERT_POS_CENTRE,
	VERT_POS_TOP,
} VERT_POS;

typedef enum
{
	HORZ_POS_RIGHT = 0,
	HORZ_POS_CENTRE,
	HORZ_POS_LEFT,
} HORZ_POS;

BOOL    ProcessAspectRatioSelection(HWND hWnd, WORD wMenuID);
void    AspectRatio_SetMenu(HMENU hMenu);
void    WorkoutOverlaySize(BOOL allowResize = TRUE);
void    PaintColorkey(HWND hWnd, BOOL bEnable, HDC hDC, RECT* PaintRect);
int		FindAspectRatio(short** EvenField, short** OddField);
void	AdjustAspectRatio(short** EvenField, short** OddField);
void	SetHalfHeight(BOOL IsHalfHeight);
void	GetSourceRect(RECT *rect);
void    GetDestRect(RECT *rect);
void	SwitchToRatio(int nMode, int nRatio);
double  GetActualSourceFrameAspect();

// TODO: Cleanup this struct, possibly split into 2 or 3 structures
// based on function of the various settings.
typedef struct 
{
	// Added variable in dTV.c to track which aspect mode we are currently in
	// Use aspect * 1000 (1.66 = 1660, 2.35 = 2350, etc)
	// Use negative values for source_aspect to imply anamorphic sources
	// Note: target_aspect is the aspect ratio of the screen.
	int source_aspect;
	int target_aspect;

	// Mode 0 = do nothing, 1 = Letterboxed, 2 = 16:9 anamorphic
	int aspect_mode;

	int custom_source_aspect;
	int custom_target_aspect;

	// Luminance cutoff for a black pixel for letterbox detection.  0-127.
	long LuminanceThreshold;

	// Range of chroma values (centered around 128) we'll consider black for
	// letterbox detection.  0-255.
	long ChromaRange;

	// Ignore this many non-black pixels when detecting letterbox.  Setting
	// this too high may cause problems with dark scenes.
	long IgnoreNonBlackPixels;

	// Skip over this percentage of pixels on each side (left and right) when
	// detecting letterbox.  If zero, no pixels are skipped.
	long SkipPercent;

	// Nonzero to continuously scan for aspect ratio changes.
	BOOL AutoDetectAspect;

	// For aspect autodetect, require the same aspect ratio for this number of
	// frames before zooming in.
	long ZoomInFrameCount;

	// For aspect autodetect, zoom in quickly if we've used this ratio in the
	// last N seconds.
	long AspectHistoryTime;

	// For aspect autodetect, only zoom in if we haven't detected a smaller
	// ratio in some amount of time.
	long AspectConsistencyTime;

	// For aspect autodetect, consider two ratios to be equal if they're within
	// this amount of each other.  This is not in pixels, but in aspect*1000
	// units.
	long AspectEqualFudgeFactor;

	// Don't remember aspect ratios that lasted less than this many milliseconds.
	long ShortRatioIgnoreMs;

	// Where does the window sit on the screen
	// defaults to bang in the middle
	VERT_POS VerticalPos;
	HORZ_POS HorizontalPos;

	RECT destinationRectangle;
	RECT sourceRectangle;
	RECT destinationRectangleWindow; // MRS 2-22-01 
	BOOL deferedSetOverlay; // MRS 2-22-01 - Wait until middle of WM_PAINT to set overlay (between drawing of black bars and drawing of overlay color)
	BOOL overlayNeedsSetting; // MRS 2-22-01

	int InitialOverscan;

	// If TRUE, black bars are drawn.  If FALSE, we leave any leftover image on the screen
	// For advanced use - typically used FALSE with top-aligned for viewing captions
	BOOL aspectImageClipped;

	// If TRUE, the WorkoutOverlaySize function will gradually bounce the image
	// across any black space, starting at the middle, working to the right/bottom
	// then bouncing to the right/top.
	// NOTE: SOMETHING ELSE must take responsibility for calling WorkoutOverlaySize
	// to ensure the bouncing takes place.
	BOOL bounceEnabled;
	// Time to consider bouncing as started - once set it is not reset
	// until dTV is restarted.

	time_t bounceStartTime;
	// Number of minutes for a complete cycle of bounce to occur (default is half hour)
	time_t bouncePeriod;
	long timerBounceMS; // # of miliseconds between aspect updates
	long bounceAmplitude; // percentage of window over which the overlay bounces

	// Orbit - shifts the source image around on a regular basis
	// Shares the bounceStartTime for calculations
	// PeriodX and PeriodY should be different for ideal results
	BOOL orbitEnabled;
	int orbitSize; // # of pixels of variation (both X and Y axis)
	time_t orbitPeriodX; // Time to move across the entire orbit area on X axis (seconds)
	time_t orbitPeriodY; // Time to move across the entire orbit area on Y axis (seconds)
	long timerOrbitMS; // # of miliseconds between aspect updates for orbiting (miliseconds)
	BOOL autoResizeWindow; // If TRUE, resize non-fullscreen window to fit image exactly

	// True if we want to use whatever ratio is present on the next frame.
	BOOL DetectAspectNow;

	// Zoom
	long xZoomFactor;
	long yZoomFactor;
	long xZoomCenter;
	long yZoomCenter;

    // WaitForVerticalBlank
    BOOL bWaitForVerticalBlank;
} TAspectSettingsStruct;

extern TAspectSettingsStruct aspectSettings;

#endif
