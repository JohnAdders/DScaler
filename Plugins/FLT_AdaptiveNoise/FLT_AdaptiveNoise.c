/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Lindsey Dubb.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////

//#define ADAPTIVE_NOISE_DEBUG

#include <limits.h>
#include <math.h>

#include "windows.h"
#include "DS_Filter.h"

/////////////////////////////////////////////////////////////////////////////
/*

And thus the native hue of resolution
Is sicklied o'er with the pale cast of thought,
And enterprises of great pitch and moment
With this regard their currents turn awry,
And lose the name of action.
                             -- Hamlet

Adaptive noise reduction filter

This plug-in detects the global noise level based on the histogram of noise throughout
the image.  The estimated noise is combined user supplied noise/motion tradeoff values 
to decide how to best combine the old and new pixel values to choose a displayed pixel
color.

This is as much a motion detector as it is a noise detector.  Feel free to pull ideas
from here to improve motion detection in other parts of the program.


A note to developers:

Hidden in this filter is a very useful motion detection indicator.  Just #define
ADAPTIVE_NOISE_DEBUG and turn on "pink dots" and you'll get a histogram of the
N statistic (see below) along with some other less useful stats. (These stats are
explained just above the AnalyzeHistogram() routine.)

In addition, you can have the filter periodically flash all pixels whose N value
is in a certain range.  To do so, set the Testing Threshold and Second Testing Threshold
sliders to the bottom and top of the range of N you want to view, then turn on
Use Testing Switch.

Together these provide a nice way to see whether an alteration to a motion detection
statistic has helped differentiate particular regions.  Its use is limited to N-like
statistics, though.


The algorithm:

At each four pixel horizontal block, the sum of absolute byte differences is calculated 
between the block and the same block in the previous frame.  The block size and the absolute
difference statistic were chosen entirely for speed reasons.  It would probably make more
sense to use a squarer block and a sum of squared differences statistic, instead.

The "cumulative absolute difference statistic" will be called "N". It is calculated as:

A = Max(N for the ~corresponding block from the previous _field_, MaxNoise)
B = Max(N for the block to the left in the current field, MaxNoise)
C = Max(N for the block above in the current field, MaxNoise)

D = Decay factor (fixed to 0.78 in the current build)
E = Horizontal decay factor (fixed to (0.78)^2 in the current build)

F = Absolute byte difference between current block and block in previous frame

Then,
N = Max(A*D, B*E, C*D) + F

In other words, it's a weird IIRish filter -- The oddity comes from the use of
the Max function.  The reasoning behind this is that strong evidence for motion
from any direction should have a high weight, overruling the lack of evidence from
any other direction.  This was done in a desperate attempt to cut down on blurring
of faces, grass fields, and other low contrast moving textures.

The horizontal decay is faster because the blocks are approximately
2 horizontal x 1 vertical when you take the interlace into account.  On top of that,
noise tends to be horizontally correlated.  Vertical and temporal decay are the same
for computational reasons: It saves a multiplication.

Optimally, you would want to derive the spatial decay factor from the level of
detail (specifically, the frequency with which the image switches between mooving
and stationary regions) and the amount of change in the regions of motion.

MaxNoise exists to prevent a single dramatic change from causing too great a
perturbation of N.  Beyond a certain point, motion is certain, so the statistic
shouldn't increase further.  Yes, this could be handled more effectively.

Note that the filter is entirely unidirectional.  There's surely some room for
improvement, there.

The new N value is then used to determine what kind of weighting to use between
the old and new pixel values in that block.  This is similar to the method used
in the "Gradual Noise" filter, but N is used in place of the simple sum of absolute
differences, and there is a new "noise baseline" term.  More exactly,

G = Noise baseline
H = Noise multiplier

M = motion evidence = 1             if (N - G)/H > 1
                      (N - G)/H     if 0 < (N - G)/H < 1
                      0**           if (N - G)/H < 0

Result pixel = (bytewise) oldPixel * (1 - M) + newPixel * M
** As with the Gradual Noise filter, the amount of change is always set to a minimum
of 1.  So if oldPixel != newPixel, then even if M would be 0, the result pixel is
oldPixel moved 1 bit toward the newPixel value.


The Noise baseline G and Noise multiplier H are determined through analysis of a
histogram of the N statistic from the entire previous field.  The histogram is collected
from all pixels whose values are not too near 0 or 255.  The noise at low and
(to a lesser extent) high valued pixels is reduced due to an edge effect, resulting
in a different and lower distribution, which would foil the peak finder.

When there is no motion, the histogram is distributed as something like a chi^2.
An arbitrary low (pixel width / 16) quantile Q of the histogram is collected, along with the
lowest "significant" peak P.  These measures were chosen for robustness to motion
and interference: The higher part of the distribution changes a great deal in response
to motion or interference.

These estimates along with the past estimate of Q and P are combined according to the
reliability of the current and past estimates to produce the new estimates Q' and P'.
See below in the comments for the AnalyzeHistogram() procedure for detail.  But it's
weird, arbitrary stuff, so I wouldn't look if I were you.

From the curve determined by Q' and P', combined with the user specified
"Noise Reduction" and "Stability", the new Noise baseline G and Noise
multiplier H are determined.  Specifically:

R = P' - Q' = "Curve width"  (which is used as a general measure of the curve's breadth)

G = Q' + R/4 - R*(100 - Stability)/100

So G is in the range
(Q' - R*3/4, Q' + R/4)
with the default
G = Q' - R/4

And the noise multiplier is determined as

H = Q' + R/4 + R*NoiseReduction/50

So H is in the range
(Q' + R/4, Q' + 17R/4)
with the default
H = Q' + 5R/4 = P' + R/4


Things tried and discarded:

Ornstein-Uhlenbeck ("red") noise reduction
If you look closely at the noise in a normal TV signal, you'll notice that it
is horizontally highly correlated.  So it makes sense to model the noise as a random
walk with bias toward return to 0 -- an Ornstein-Uhlenbeck process.  I have come up with
some ways to successfully cut way back on horizontally correlated noise (generally, I've
used the previous horizontal noise estimate as a predictor of the next noise estimate),
but so far they've all caused artifacts.  There's probably a good way to do this.

Temporal gradients
In most gradual motion, the color changes directionally -- or so you'd think.  I tried to
use the direction of change at a pixel as an additional contribution to the motion evidence,
but it was completely useless.  Using indicator pink pixels, it was clear that directionality
of change was completely worthless as an indicator -- It showed up reliably only when the 
motion was so obvious that the mechanisms already in place could identify it without help.

Things which need to be done:
- Lots of cleanup.  This is still a mess, and there are a bunch of unnecessarily arbitrary
  constants hanging around.

Potential improvements:
- Global motion detection.  Full screen pans are this filter's worst enemy, since they
  often cause a low peak which is different from the noise peak.  Partial screen motion
  is much less of a problem.  And -- very importantly -- Global motion detection is a much
  easier problem, and very possibly doable in real time.  Maybe some kind of mipmappish
  approach would be the way to go?
- Get the Ornstein-Uhlenbeck stuff to work (see above)
- Transmit the information up and to the left, as well as down and to the right.  Maybe do
  two passes and use a FIR filter to blur the color difference measures?
- Delay one or more fields, and make use of future colors and motion.
- Use picture information to determine the decay coefficients used to calculate N
- Optimization: My sole optimization has been the addition of a few prefetch instructions.
  They sped the filter 2x (from 600 MHz to 300 MHz on my SDR Athlon), but there's probably
  a bit more to speed to be had.

*/
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// This might need to be tuned for other processors.
// Then again, it doesn't seem to matter much what I set it to on my Athlon
// so long as it's between 64 and about 300

#define PREFETCH_STRIDE                         128

// Base rate at which the estimate of reliability is moved toward the new estimate

#define RELIABILITY_DECAY                       0.008

// Vertical scale of the histogram display: Each hash mark represents 10*HISTOGRAM_SCALE points of
// the cumulative color difference.

#define HISTOGRAM_SCALE                         2

// Total length of the histogram
// If gDecayCoefficient is increased above ~95%, this should be increased, too

#define HISTOGRAM_LENGTH    1024

// Rate of decay of the mean and mean^2 averaging for determining peak variance

#define PEAK_VARIANCE_DECAY 0.80

// Intervals at which to make comparisons when finding peaks in the histogram

#define PEAK_NEAR_COMPARISON    1
#define PEAK_MIDDLE_COMPARISON  4
#define PEAK_FAR_COMPARISON     16


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

long            FilterAdaptiveNoise( TDeinterlaceInfo* pInfo );
long            FilterAdaptiveNoise_DEBUG( TDeinterlaceInfo* pInfo );

void            AnalyzeHistogram( TDeinterlaceInfo* pInfo, DWORD MaxNoise, DOUBLE* pCumBaseline, DOUBLE* pCumDifferencePeak );

void __cdecl    ExitAdaptiveNoise( void );
void            CleanupAdaptiveNoise( void );


/////////////////////////////////////////////////////////////////////////////
// Begin plugin globals
/////////////////////////////////////////////////////////////////////////////

// Map of the N statistic (see the explanation of the algorithm, above)

static BYTE*    gpChangeMap = NULL;

// Histogram of the number of pixels with each value of the N statistic

static BYTE*    gpHistogram = NULL;

// Percent of N which is added below and after the current block
// This setting is currently hidden, since it doesn't have much useful play.

long            gDecayCoefficient = 78;

// Determines the placement toward the "start" of the N histogram at which we completely
// compensate for noise

long            gStability = 50;

// Determines the placement after the peak of the N histogram where we decide that there
// must be motion in a block

long            gNoiseReduction = 50;

// Turn on a histogram and statistics readout

long            gIndicator = FALSE;

// Turn on a dot when the running estimate of noise is close to the estimate from the
// current picture

long            gShowLockDot = FALSE;


#ifdef ADAPTIVE_NOISE_DEBUG
// Toggle pink pixel flashing

long            gTestingFlag = TRUE;

// Value of N above which the block will periodically be flashed pink

long            gTestingThreshold = 50;

// Value of N below which the block will periodically be flashed pink

long            gSecondTestingThreshold = 50;
#endif  // Debug globals


/////////////////////////////////////////////////////////////////////////////
// Control settings
/////////////////////////////////////////////////////////////////////////////


// The debug version has a few more switches

#ifdef ADAPTIVE_NOISE_DEBUG
typedef enum
{
    ANOISETESTOPTION = FLT_ANOISE_SETTING_LASTONE,
    ANOISETESTTHRESHOLD,
    ANOISESECONDTESTTHRESHOLD,
    FLT_ANOISE_SETTING_DEBUG_LASTONE,
} FLT_ANOISE_DEBUG_SETTING;
#endif


FILTER_METHOD AdaptiveNoiseMethod;


#ifdef ADAPTIVE_NOISE_DEBUG
SETTING FLT_AdaptiveNoiseSettings[FLT_ANOISE_SETTING_DEBUG_LASTONE] =
#else
SETTING FLT_AdaptiveNoiseSettings[FLT_ANOISE_SETTING_LASTONE] =
#endif
{
    {
        "Stability", SLIDER, 0, &gStability,
        50, 0, 100, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "AStability", NULL,
    },
    {
        "Noise Reduction", SLIDER, 0, &gNoiseReduction,
        50, 0, 200, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "AdaptiveNoiseReduction", NULL,
    },
    {
        "Lock Dot", ONOFF, 0, &gShowLockDot,
        FALSE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "LockDot", NULL,
    },
    {
#ifdef ADAPTIVE_NOISE_DEBUG
        "Activate Adaptive Noise Filter", ONOFF, 0, &AdaptiveNoiseMethod.bActive,
#else
        "Activate Adaptive Noise Filter", NOT_PRESENT, 0, &AdaptiveNoiseMethod.bActive,
#endif // Show "activate" flag when testing
        FALSE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "ActivateANoiseFilter", NULL,
    },
    {
#ifdef ADAPTIVE_NOISE_DEBUG
        // Okay, so they're not pink
        "Pink dots", ONOFF, 0, &gIndicator,
#else
        "Pink dots", NOT_PRESENT, 0, &gIndicator,
#endif // Hide histogram option (but keep it in the .ini file) for users
        FALSE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "APinkIndicator", NULL,
    },
        {
#ifdef ADAPTIVE_NOISE_DEBUG
        "Motion memory (percent)", SLIDER, 0, &gDecayCoefficient,
#else
        "Motion memory (percent)", NOT_PRESENT, 0, &gDecayCoefficient,
#endif // Show motion memory slider when testing
        78, 0, 95, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "MotionMemory", NULL,
    },
#ifdef ADAPTIVE_NOISE_DEBUG
    {
        "Use Testing Switch", ONOFF, 0, &gTestingFlag,
        TRUE, 0, 1, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "ATestingIndicator", NULL,
    },
    {
        "Testing Threshold", SLIDER, 0, &gTestingThreshold,
        50, 0, 2000, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "ATestingThreshold", NULL,
    },
    {
        "Second Testing Threshold", SLIDER, 0, &gSecondTestingThreshold,
        50, 0, 2000, 1, 1,
        NULL,
        "AdaptiveNoiseFilter", "AsecondTestingThreshold", NULL,
    },
#endif  // Add a debug switch and two sliders for testing purposes
};


FILTER_METHOD AdaptiveNoiseMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Adaptive Noise Filter",                // Displayed name
    "Noise Reduction (Adaptive)",           // For consistency with the Temporal Noise filter
    FALSE,                                  // Not initially active
    TRUE,                                   // Call on input so pulldown/deinterlacing can benefit
    FilterAdaptiveNoise,                    // Algorithm to use (really decided by GetFilterPluginInfo)
    0,                                      // Menu: assign automatically
    FALSE,                                  // Does not run if we've run out of time
    NULL,                                   // No initialization procedure
    ExitAdaptiveNoise,                      // Deallocation routine
    NULL,                                   // Menu handle (for DScaler)
#ifdef ADAPTIVE_NOISE_DEBUG
    FLT_ANOISE_SETTING_DEBUG_LASTONE,
#else
    FLT_ANOISE_SETTING_LASTONE,             // Number of settings
#endif  // More settings when in debug mode
    FLT_AdaptiveNoiseSettings,
    WM_FLT_ANOISE_GETVALUE - WM_USER,       // Settings offset
    TRUE,                                   // Can handle interlaced material
    2,                                      // Requires field before last
};


/////////////////////////////////////////////////////////////////////////////
// Main code (included from FLT_AdaptiveNoise.asm)
/////////////////////////////////////////////////////////////////////////////

#ifdef ADAPTIVE_NOISE_DEBUG
long DispatchAdaptiveNoise( TDeinterlaceInfo *pInfo )
{
    long    FilterResult = 1000;

    if( gTestingFlag == TRUE )
    {
        FilterResult = FilterAdaptiveNoise_DEBUG( pInfo );
    }
    else
    {
        FilterResult = FilterAdaptiveNoise( pInfo );
    }
    return FilterResult;
}
#endif // Function dispatcher for the debug version


// Include the main code from a different file to allow for easy comparisons of
// different algorithms -- or different versions for different processors, if that
// becomes a good idea.

#include "FLT_AdaptiveNoise.asm"

#ifdef ADAPTIVE_NOISE_DEBUG
#define IS_DEBUG_FLAG
#include "FLT_AdaptiveNoise.asm"
#undef IS_DEBUG_FLAG
#endif  // Debug switch version of code


////////////////////////////////////////////////////////////////////////////
// Start of utility code
/////////////////////////////////////////////////////////////////////////////


// Memory cleanup routines

void __cdecl ExitAdaptiveNoise( void )
{
    CleanupAdaptiveNoise();
    return;
}


void CleanupAdaptiveNoise( void )
{
    if( gpChangeMap != NULL )
    {
        free( gpChangeMap );
        gpChangeMap = NULL;
    }
    if( gpHistogram != NULL )
    {
        free( gpHistogram );
        gpHistogram = NULL;
    }
    return;
}


// DScaler interaction routines

__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags )
{
    // We need psadbw and make use of the maximization instructions, so at least
    // MMXEXT is required.
    if( CpuFeatureFlags & (FEATURE_SSE | FEATURE_MMXEXT) )
    {
#ifdef ADAPTIVE_NOISE_DEBUG
        AdaptiveNoiseMethod.pfnAlgorithm = DispatchAdaptiveNoise;
#else
        AdaptiveNoiseMethod.pfnAlgorithm = FilterAdaptiveNoise;
#endif // Debug switch version of code
    }
    else
    {
        AdaptiveNoiseMethod.pfnAlgorithm = NULL;
    }
    return &AdaptiveNoiseMethod;
}


// Need malloc() and free(), so this is commented out:
/*
BOOL WINAPI _DllMainCRTStartup( HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved )
{
    return TRUE;
}
*/


/*

Automated curve psychoanalysis:

This is currently pretty sloppy code.  Yes, I hope to fix it eventually.

Often, material (especially sports) will have a few brief parts with good information about the noise, but
otherwise have too much noise to tell much.  This routine is designed to lock onto the good noise estimate,
but to eventually give up and go with a poor estimate if that's all which is available.  The use of poor
estimates is necessary when viewing material with heavy interference.

The baseline of the noise is easy to determine -- It's just defined as a low quantile of the histogram.

The peak is a little tougher -- I use a median smoother followed by a simple hill climber.  The peak
finder is meant to find the first moderately large peak.  This purposefully avoids higher valued peaks
in favor of the one with the lowest color difference, since the lowest peak is most likely to be
the one which represents noise.  There are two drawbacks to this:
- If the screen is showing material with two different noise levels, only the lower will be detected
- If interference is affecting only part of the screen, only the cleanest part will be detected

The reliability measure of an estimate depends on four factors:
- The height of the histogram's peak
- The (time weighted) coefficient of variation of the peak estimate
- The difference between the new peak estimate and the old estimate
- The reliability of the previous estimate

This is handled in a really haphazard way, with lots of arbitrary powers and constants  -- I'll just refer you
to the code, since it's likely to change before I next update this comment.  Good luck, since the commenting in
the code is poor.

There is also some code in here to show a bunch of data about the histogram.  This can be a very useful
tool for people working on motion detection.  The histogram shows:

- The histogram is displayed on the left of the screen.  Each hash mark represents 40 points of the
  motion statistic (decayed sum of absolute color differences).
- There are five dots which appear within the histogram display.  From left to right:
    - The current peak estimate
    - The current baseline estimate
    - The cumulative baseline estimate
    - The cumulative peak estimate
    - The quantity (maximum allowed noise value * decay). Expect a peak just above this if there is lots of motion.
      This dot often drops below the bottom of the screen.
- There are two dots which appear above the histogram.  From top to bottom:
    - The reliability
    - (1.0 - Coefficient of variation of the peak estimate)^16  (Yes, really!)

*/

void AnalyzeHistogram( TDeinterlaceInfo* pInfo, DWORD MaxNoise, DOUBLE* pCumBaseline, DOUBLE* pCumDifferencePeak )
{
    LONG            Index = 0;
    static DOUBLE   sPeakMean = 20.0;           // Accumulated average of noise value peaks (for variance calculation)
    static DOUBLE   sPeakSquaredMean = 400.0;   // Accumulated average of squared noise value peaks (for variance)
    static DOUBLE   sReliability = 0.001;       // Reliability of current estimates; Set low to allow early data to count heavily

    // Medianish smoothing of the histogram (doesn't really need to be done for the whole histogram)

    for( Index = 1; Index < HISTOGRAM_LENGTH - 1; ++Index )
    {
        DWORD           highest;
        DWORD           lowest;
        DWORD           thisTry;
        DWORD*          pDWordHistogram = (DWORD*)gpHistogram;

        highest = pDWordHistogram[Index - 1];
        lowest = highest;
        thisTry = pDWordHistogram[Index];
        if( thisTry > highest )
        {
            highest = thisTry;
        }
        else 
        {
            lowest = thisTry;
        }
        thisTry = pDWordHistogram[Index+1];
        if( thisTry > highest )
        {
            pDWordHistogram[Index] = highest;
        }
        else if( thisTry < lowest )
        {
            pDWordHistogram[Index] = lowest;
        }
        else
        {
            pDWordHistogram[Index] = thisTry;
        }
    }


    // Draw histogram

    if( gIndicator == TRUE )
    {
        for( Index = 0; Index < pInfo->FieldHeight - 20; ++Index )
        {
            if ( ((((DWORD*)gpHistogram)[Index*HISTOGRAM_SCALE]) < 1280) )
            {
                *(pInfo->PictureHistory[0]->pData + ((Index+20) * pInfo->InputPitch) + (((DWORD*)gpHistogram)[Index*HISTOGRAM_SCALE]/2)*2) = 0xFF;
            }
            else
            {
                *(pInfo->PictureHistory[0]->pData + ((Index+20) * pInfo->InputPitch) + 1280) = 0xFF;
            }
            if (Index % 10 == 0)
            {
                *(pInfo->PictureHistory[0]->pData + ((Index+20) * pInfo->InputPitch) + 100) = 0xFF;
            }
        }
    }

    // Find baseline and first local maximum in histogram, as well as their reliability

    {
        DWORD*      pDWordHistogram = (DWORD*)gpHistogram;
        DOUBLE      SignalStrength = 0.0;
        DOUBLE      CoeffVar = 0.01;
        DOUBLE      ProbSameSignal = 0.0;   // Not really a probability
        DWORD       Accumulator = 0;
        DWORD       Baseline = 0;
        DWORD       DeltaPeak = 0;

        for( Index = 0; Index < HISTOGRAM_LENGTH - PEAK_FAR_COMPARISON; ++Index )
        {

            Accumulator += pDWordHistogram[Index];
            if( Accumulator >  pInfo->LineLength/16)
            {
                if( (gIndicator == TRUE) && (Index/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
                {   // Show this field's baseline
                    *(pInfo->PictureHistory[0]->pData + ((Index/HISTOGRAM_SCALE+20) * pInfo->InputPitch) + 300) = 0xFF;
                }
                Baseline = Index;
                break;
            }
        }
        // Don't look for the peak past MaxNoise*Decay, since it will be artifactual. (Since noise is limited to an exact
        // value, there will be a peak around MaxNoise*Decay).  !! Improve this !! 
        for( ; (Index < HISTOGRAM_LENGTH) && ((Index < (LONG)(MaxNoise*(gDecayCoefficient+1))/100) || (Index < 100)); ++Index )
        { // Simple peak finder -- It's designed to find the first moderately big peak
            if( (pDWordHistogram[Index] >= 5) &&
                (pDWordHistogram[Index+PEAK_NEAR_COMPARISON] < pDWordHistogram[Index]) &&
                (pDWordHistogram[Index+PEAK_MIDDLE_COMPARISON] <= pDWordHistogram[Index+PEAK_NEAR_COMPARISON]) &&
                (pDWordHistogram[Index+PEAK_FAR_COMPARISON] <= pDWordHistogram[Index+PEAK_MIDDLE_COMPARISON])
            ) {
                if( (gIndicator == TRUE) && (Index/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
                {   // Show this field's peak
                    *(pInfo->PictureHistory[0]->pData + ((Index/HISTOGRAM_SCALE+20) * pInfo->InputPitch) + 200) = 0xFF;
                }
                DeltaPeak = Index;
                sPeakMean = sPeakMean * PEAK_VARIANCE_DECAY +
                    DeltaPeak * (1.0 - PEAK_VARIANCE_DECAY);
                sPeakSquaredMean = sPeakSquaredMean * PEAK_VARIANCE_DECAY +
                    (DeltaPeak * DeltaPeak) * (1.0 - PEAK_VARIANCE_DECAY);
                if( sPeakMean < .999 )
                {   // Always slightly consider the alternative
                    CoeffVar = .999;
                }
                else
                {
                    CoeffVar = sPeakSquaredMean - sPeakMean*sPeakMean;  // the variance
                    CoeffVar /= sPeakSquaredMean; // really CV^2
                }
                CoeffVar = 1.0 - sqrt(CoeffVar);
                CoeffVar *= CoeffVar;
                CoeffVar *= CoeffVar;
                CoeffVar *= CoeffVar;
                CoeffVar *= CoeffVar;
                {   // Show Coefficient of variation statistic
                    DWORD  Jindex = 0;
                    Jindex = (DWORD)(CoeffVar*pInfo->FrameWidth);
                    Jindex *= 2;
                    if( gIndicator == TRUE )
                    {
                        *(pInfo->PictureHistory[0]->pData + (15 * pInfo->InputPitch) + Jindex) = 0xFF;
                    }
                }
                SignalStrength = pDWordHistogram[DeltaPeak];
                // Normalize the maximum strength of signal to the amount of data collected
                // The divisor of pInfo->FrameWidth is more arbitrary than it looks: Any constant times
                // the FrameWidth would be reasonable.
                SignalStrength /= pInfo->FrameWidth;        
                SignalStrength *= CoeffVar;
                if (SignalStrength > 1.0)
                {
                    SignalStrength = 1.0;
                }
                SignalStrength *= SignalStrength;
                SignalStrength *= SignalStrength;
                SignalStrength *= RELIABILITY_DECAY;
                break;
            }
        }
        // Update the baseline and peak estimates, weighting with new estimate reliability relative to old reliability
        *pCumBaseline = (*pCumBaseline)*(sReliability/(SignalStrength+sReliability)) +
            Baseline*SignalStrength/(sReliability+SignalStrength);
        *pCumDifferencePeak = (*pCumDifferencePeak)*(sReliability/(SignalStrength+sReliability)) +
            DeltaPeak*SignalStrength/(sReliability+SignalStrength);
        if( (gIndicator == TRUE) && ((*pCumBaseline)/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
        {   // Show baseline estimate
            *(pInfo->PictureHistory[0]->pData + (((LONG)((*pCumBaseline)/HISTOGRAM_SCALE) + 20) * pInfo->InputPitch) + 400) = 0xFF;
        }
        if( (gIndicator == TRUE) && ((*pCumDifferencePeak)/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
        {   // Show peak estimate
            *(pInfo->PictureHistory[0]->pData + (((LONG)((*pCumDifferencePeak)/HISTOGRAM_SCALE) + 20) * pInfo->InputPitch) + 500) = 0xFF;
        }
        // If we get a weak signal at the location of our current estimate, don't drop the reliability
        ProbSameSignal = (*pCumDifferencePeak) - DeltaPeak;
        ProbSameSignal = ProbSameSignal*ProbSameSignal/(*pCumDifferencePeak);
        ProbSameSignal = 1.0 - ProbSameSignal;
        if( ProbSameSignal < 0.0 )
        {
            ProbSameSignal = 0.0;
        }
        // Bias toward the more reliable signals, since we can just ignore the lousy ones.
        // Paradoxically, this means poor signals reduce the reliability less than mediocre signals do.  This is
        // desirable behavior because the worse the signal, the more we want to hold onto the older estimate.
        if( sReliability > SignalStrength/RELIABILITY_DECAY )
        {
            sReliability = sReliability*ProbSameSignal +
                (1.0 - ProbSameSignal)*sReliability*(1.0-SignalStrength) +
                (1.0 - ProbSameSignal)*(SignalStrength/RELIABILITY_DECAY)*SignalStrength;
        }
        else
        {
            sReliability = sReliability*(1-SignalStrength) + (SignalStrength/RELIABILITY_DECAY)*SignalStrength;
        }
/*      This alternative causes reliability to fall too fast if it's already low
        sReliability = sReliability*((sReliability)/(SignalStrength+sReliability)) +
            (SignalStrength/0.008)*(SignalStrength)/(sReliability+SignalStrength); //0.00006;
*/
        if( gIndicator == TRUE )
        {   // Show reliability estimate
            DWORD  Jindex = (DWORD) (sReliability * pInfo->FrameWidth);
            *(pInfo->PictureHistory[0]->pData + (10 * pInfo->InputPitch) + Jindex*sizeof(WORD)) = 0xFF;
        }
        if( gShowLockDot == TRUE )
        {
            if( ProbSameSignal > 0.001 )
            {
                *((DWORD*)( pInfo->PictureHistory[0]->pData +
                    (16 * pInfo->InputPitch) + 64)) = 0x4FFF;
            }
        }
    }

    if( ((LONG)(MaxNoise*(gDecayCoefficient+1))/400 < pInfo->FieldHeight - 20) && (gIndicator == TRUE) )
    {   // Show MaxNoise point -- This is often off the bottom of the screen
        *(pInfo->PictureHistory[0]->pData + ((((MaxNoise*(gDecayCoefficient+1))/400) + 20) * pInfo->InputPitch) + 600) = 0xFF;
    }
/*
    if( (gIndicator == TRUE) && (pInfo->PictureHistory[0]->IsFirstInSeries) )
    {
        *((DWORD*)(pInfo->PictureHistory[0]->pData + ((Index+10) * pInfo->InputPitch) + 100)) = 0xFFFF;
    }
*/
}