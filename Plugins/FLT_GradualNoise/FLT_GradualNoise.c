/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001, 2002 Lindsey Dubb.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.9  2002/06/18 19:46:08  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.8  2002/06/13 12:10:25  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.7  2002/02/23 03:22:16  lindsey
// Exposed the "Use this filter" settings
//
// Revision 1.6  2002/02/04 01:06:03  lindsey
// Added 3DNow optimized version
//
// Revision 1.5  2002/02/01 23:19:49  lindsey
// Enabled SSE version, again.  (Oops.)
//
// Revision 1.4  2002/02/01 23:16:29  lindsey
// Added code for MMX computers
// Removed MMXEXT version (since it didn't do anything)
//
// Revision 1.3  2001/12/31 00:02:59  lindsey
// Fixed crashing bug when pixel width not evenly divisible by 8
// Added prefetching for a substantial speed up
//
// Revision 1.2  2001/12/28 02:52:14  lindsey
// Corrected a settings typo
// Prevented a crash with very low Noise Reduction settings
//
// Revision 1.1.1.1  2001/12/23 01:43:53  lindsey
// Added Gradual Noise Filter
//
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

/////////////////////////////////////////////////////////////////////////////
/*
We all learn by experience, and your lesson this time is that you should
never lose sight of the alternative.
                                             -- Sherlock Holmes


Gradual noise reduction filter.  This noise filter smooths out slight
variations in pixels between adjacent frames.

This algorithm is very similar to what Andrew Dowsey came up with in his "Adaptive
Temporal Averaging" for his DirectShow filter.  The algorithms differ in 1) their
block size,  2) their motion estimation (sum of absolute differences versus mean
squared error), 3) rounding.


The algorithm:

This filter gets the sum of absolute differences between a four pixel
horizontal block in the current image and the same block in the preceding
frame.  This isn't the best local motion measure, but it's very fast due to
the psadbw SSE instruction.

This difference measure is used to determine the kind of averaging which will be
conducted.  If it's more than the "noise reduction" parameter, motion is
inferred.  In that case, we just use the new pixel values.  If it's less than the 
noise reduction, we use the ratio of (difference/noise reduction) to determine the
weighting of the old and new values.

Somewhat more formally:

  N = Sum_block(|oldByte - newByte|)
  R = Noise Reduction parameter
  M = (motion evidence) = 1     if N/R >= 1
                          N/R   otherwise
  Result pixel = (bytewise) oldPixel * (1 - M) + newPixel * M


Rounding has a very significant effect on the algorithm.  In general, for
computational reasons, values are rounded down.  An important exception 
occurs when
    M > 0 and oldPixel != newPixel
but
    oldPixel * (1 - M) + newPixel * M
rounds to oldPixel.  In that case, the Result pixel is rounded to one toward
the newPixel value.  This makes sure that very gradual variation is maintained.


The rationale:

This filter was designed to get rid of the artifacts caused by the use of
a hard threshold in the Temporal Noise Filter.  The problems were:
- Speckles:
  Noise tends to follow something like a normal distribution. When an extreme
  value comes up, the Temporal Noise Filter will conclude that it represents
  motion.  When this happens in a stationary area where averaging is occuring, a
  speckle is seen.
- Color bias, and the inability to fade all the way to black:
  These are both due to a bias in the averaging routine used in the filter.
- Loss of color depth:
  When there's just a little noise, the Temporal Noise Filter will completely
  stabilize the colors.  This loses some information, as wavering between
  nearby colors can indicate that the true value is between them.
- Posterization:
  One cause of posterization is loss of color depth described above. Another is
  the approximate averaging on machines without an MMX averaging instruction.
  The third cause is more complicated...
  In an area of low contrast motion, it is possible for part of the area to
  barely exceed the noise threshold while another part is just below it. As a
  result, the second area will be averaged while the first is left alone.  This
  causes an artificial boundary in the image.
  Because those pixels which are affected by the artifacts are not averaged,
  their difference with the proceeding pixel will have a higher variance, thus
  propagating the error through time.  (Thanks to Rob Muller for pointing this
  out.)

By using a gradual method rather than a hard threshold, those artifacts are
affected so:
- Speckles:
  This is greatly improved.  Extreme values from the noise distribution are
  partially averaged, so they stand out much less.  The blockwise motion
  estimation also helps reduce the effect of speckles.  But really extreme
  speckles (like from an old film) will look just as bad as before.
- Color bias, and the inability to fade all the way to black:
  This is solved.
- Loss of color depth:
  This is improved by the "round to one" part of the algorithm.  But there is
  still a significant loss.
- Posterization:
  This is significantly improved.  In areas of low contrast motion, there's no
  threshold value at which averaging stops, so there the artifactual borders are
  less obvious.  There are still some borders, though, due to rounding and loss of 
  color depth.

With that stuff in mind, the goal was to make the filter as fast as possible.  As
a result, it isn't any better at avoiding the most important noise reduction artifact:
blurring.


An alternate rationale:

The above explanation sounds good, but this plugin is really a cut down version of
a filter designed as a loose approximation of a more complicated model.

Model -- Assume the image is a combination of "stationary" and "moving" pixels. Assume
noise is distributed ~ N(0, n_sigma^2) around the "true" image value, and motion is
distributed ~ N(0, m_sigma^2) around the previous "true" image value.  Noise
occurs for all pixels regardless of whether they are moving. Assume the perceived
image error at a pixel Q = ("true" image - displayed image)^2 . For the purpose of this
plugin, make the questionable assumption that pixels never switch between {stationary,
moving}, ignore edge effects, and ignore correlation between neighboring pixels.

Then, as a function of the color difference, the variances, the distribution of past
estimates, and the prior probability of motion, you can find P(moving|Data) and
P(stationary|Data) and calculate the pixel value which will minimize Q.

That's not what this plugin does.  But for a reasonable range of m_sigma and n_sigma,
when applied to a single pixel, the function is a passable approximation. One caveat: I
haven't looked at the effect of the four pixel blocking.  Maybe it's some relative of
a chi^2 df=8?

You can try to estimate n_sigma^2, m_sigma^2, and use temporal and spatial correlation to
come up with a much better estimator of motion.  But that's a job for a different -- and
slower -- plugin.  (Specifically, the Adaptive Noise plugin -- but it doesn't include
an m_sigma^2 estimate, though it would be nice if it did.)


Yowza! Look at all that text!


A new artifact:

The filter gives blocking artifacts when it's used at really high Noise Reduction values.
These appear as vertical lines in areas of motion.


Other notes:

Don't use this and the temporal noise filter at the same time.  They're both using the
same data to achieve the same goal, so it just doesn't make sense.  Combined, they give
you the "comical posterization" filter.

*/
/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// This might need to be tuned for other processors.
// Then again, it doesn't seem to matter much what I set it to on my Athlon
// so long as it's between 64 and about 300

#define PREFETCH_STRIDE                         128

/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags );
BOOL WINAPI     _DllMainCRTStartup( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved );
long            DispatchGradualNoise( TDeinterlaceInfo *pInfo );

static long         FilterGradualNoise_SSE_PREFETCH( TDeinterlaceInfo *pInfo );
static long         FilterGradualNoise_3DNOW_PREFETCH( TDeinterlaceInfo *pInfo );
static long         FilterGradualNoise_SSE( TDeinterlaceInfo *pInfo );
static long         FilterGradualNoise_3DNOW( TDeinterlaceInfo *pInfo );
static long         FilterGradualNoise_MMX( TDeinterlaceInfo *pInfo );


/////////////////////////////////////////////////////////////////////////////
// Begin plugin globals
/////////////////////////////////////////////////////////////////////////////

long        gUsePrefetching = TRUE;

// Stored information about the machine, used when choosing which code version to run

static long gCpuFeatureFlags = 0;

// The maximum sum of absolute differences in a four pixel
// block.  Above this, the old pixel values will be ignored.

long        gNoiseReduction = 40;


FILTER_METHOD GradualNoiseMethod;

SETTING FLT_GradualNoiseSettings[FLT_GNOISE_SETTING_LASTONE] =
{
    {
        "Noise Reduction", SLIDER, 0, &gNoiseReduction,
        40, 5, 100, 1, 1,
        NULL,
        "GradualNoiseFilter", "NoiseReduction", NULL,
    },
    {
        "Fast Memory Access", ONOFF, 0, &gUsePrefetching,
        TRUE, 0, 1, 1, 1,
        NULL,
        "GradualNoiseFilter", "UsePrefetching", NULL,
    },
    {
        "Gradual Noise Filter", ONOFF, 0, &GradualNoiseMethod.bActive,
        FALSE, 0, 1, 1, 1,
        NULL,
        "GradualNoiseFilter", "UseGradualNoiseFilter", NULL,
    },
};

FILTER_METHOD GradualNoiseMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Gradual Noise Filter",                 // Displayed name
    "Noise Reduction (Gradual)",            // For consistency with the Temporal Noise filter
    FALSE,                                  // Not initially active
    TRUE,                                   // Call on input so pulldown/deinterlacing can benefit
    DispatchGradualNoise,                   // Algorithm to use (really decided by GetFilterPluginInfo)
    0,                                      // Menu: assign automatically
    FALSE,                                  // Does not run if we've run out of time
    NULL,                                   // No initialization procedure
    NULL,                                   // No deallocation
    NULL,                                   // Menu handle (for DScaler)
    FLT_GNOISE_SETTING_LASTONE,             // Number of settings            
    FLT_GradualNoiseSettings,
    WM_FLT_GNOISE_GETVALUE - WM_APP,        // Settings offset
    TRUE,                                   // Can handle interlaced material
    2,                                      // Requires field before last
    IDH_GRADUAL_NOISE
};


/////////////////////////////////////////////////////////////////////////////
// Main code (included from FLT_GNoise.asm)
/////////////////////////////////////////////////////////////////////////////

#define USE_PREFETCH

#define IS_SSE
#include "FLT_GradualNoise.asm"
#undef IS_SSE

#define IS_3DNOW
#include "FLT_GradualNoise.asm"
#undef IS_3DNOW

#undef USE_PREFETCH

#define IS_SSE
#include "FLT_GradualNoise.asm"
#undef IS_SSE

#define IS_3DNOW
#include "FLT_GradualNoise.asm"
#undef IS_3DNOW

#define IS_MMX
#include "FLT_GradualNoise.asm"
#undef IS_MMX


long DispatchGradualNoise( TDeinterlaceInfo *pInfo )
{

    if( gUsePrefetching == TRUE )
    {
        if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
        {
            FilterGradualNoise_SSE_PREFETCH( pInfo );
        }
        else if( gCpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterGradualNoise_3DNOW_PREFETCH( pInfo );
        }
        else
        {
            FilterGradualNoise_MMX( pInfo );
        }
    }
    else
    {
        if( (gCpuFeatureFlags & FEATURE_SSE) || (gCpuFeatureFlags & FEATURE_MMXEXT) )
        {
            FilterGradualNoise_SSE( pInfo );
        }
        else if( gCpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterGradualNoise_3DNOW( pInfo );
        }
        else
        {
            FilterGradualNoise_MMX( pInfo );
        }
    }
    return 1000;
}


////////////////////////////////////////////////////////////////////////////
// Start of utility code
/////////////////////////////////////////////////////////////////////////////

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags )
{
    GradualNoiseMethod.pfnAlgorithm = DispatchGradualNoise;
    gCpuFeatureFlags = CpuFeatureFlags;

    return &GradualNoiseMethod;
}

BOOL WINAPI _DllMainCRTStartup( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved )
{
    return TRUE;
}

