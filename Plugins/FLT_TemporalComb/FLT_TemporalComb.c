/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_TemporalComb.c,v 1.5 2001-11-26 15:27:19 adcockj Exp $
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
// Revision 1.4  2001/11/21 15:21:41  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.3  2001/08/30 10:04:59  adcockj
// Added a "trade speed for accuracy" mode which improves detection and
//  correction of dot crawl by removing a feedback problem -- at the
//  cost of 2MB of memory, which also slows things down a bit
// Changed szIniSection for the parameters from "NoiseFilter" to "TCombFilter"
// Made some small changes to the comments
// (Added on behalf of Lindsey Dubb)
//
// Revision 1.2  2001/08/23 06:48:57  adcockj
// Fixed control header for TemporalComb filter
//
// Revision 1.1  2001/08/23 06:38:44  adcockj
// Added First version of Lindsey Dubb's filter
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"

/////////////////////////////////////////////////////////////////////////////
/*
Twinkle, twinkle little star
How I wonder what you are...
                           -- Ann and Jane Taylor

Temporal comb filter.  This noise filter takes care of shimmering -- defined
as a repeated single frame color change.  The most obvious form of this is dot
crawl, where high contrast dots move slowly up the screen. If you're using an
internal receiver or a composite connector, it's hard to miss.

How it works:
Shimmering is detected when a pixel is more similar to the pixel two frames ago
than it is to the pixel just one frame ago.

Where shimmering is detected, the pixel is averaged with the same location in
the previous frame, which tends to remove the effect.  (That's due to the
chroma carrier frequency being out of phase between frames.)

In order to distinguish between shimmering and motion, a couple tricks
are used:
- Motion is detected if the pixels at (t = 0 frames) and (t = -2 frames) are
  sufficiently different.  In that case, shimmering is rejected.
- A time weighted average of shimmering at each pixel is kept.  This is useful
  both as a confirmation of real shimmering (versus noise), and as confirmation
  that there isn't motion.  The shimmering threshold value is restricted to a
  range such that motion artifacts are kept to a minimum.

Definition of "in phase": If two pixels would normally have the same chroma
carrier frequency phase, they are referred to as "in phase." Pixels in the
same location an even number of frames apart should be in phase, and pixels
an odd number of frames apart should be out of phase.  (Well, at least that's
how NTSC seems to work.  I need to hear from people using PAL or SECAM.)

Ways this filter could be improved:
- Make use of the spatial characteristics to identify dot crawl patterns.  This
  could be done with a fast Fourier transform, or with yet more heuristics.
- Detect and make use of the causes of shimmering (maybe with an edge detector).
- Use spatial averaging where temporal averaging isn't appropriate.
- Improve inference of shimmering at a pixel by using shimmering information from
  nearby pixels.  Actually, the current algorithm already does this to a limited
  extent.  Since gpShimmerMap is only the size of a field, not a frame, two pixels
  are used to determine each value in the shimmer map.  The C version in the comments
  at the bottom of this file has a more elaborate version of this.
- Use a one field delay to confirm shimmering.  This would definitely help, but I'm
  not sure the cost (delayed picture and early sound) would be worth it.
- Give up on the ad hoc gpShimmerMap stuff, and run a hidden Markov model,  (It
  could probably be done in real time, but there are higher priorities for
  processing time.  So this would have to wait for faster processors.)
- Make better use of phase information.  The program doesn't track the past phase
  of shimmering -- it just remembers how much shimmering there was in general.
- Average between fields instead of frames when it's beneficial.
*/
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

void __cdecl ExitTemporalComb(void);
void CleanupTemporalComb(void);
__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags);
void RescaleParameters_MMXEXT(LONG* pDecayNumerator, LONG* pAveragingThreshold);
void RescaleParameters_SSE(LONG* pDecayNumerator, LONG* pAveragingThreshold);
void RescaleParameters_3DNOW(LONG* pDecayNumerator, LONG* pAveragingThreshold);
void RescaleParameters_MMX(LONG* pDecayNumerator, LONG* pAveragingThreshold);
long FilterTemporalComb_MMXEXT(TDeinterlaceInfo *info);
long FilterTemporalComb_SSE(TDeinterlaceInfo *info);
long FilterTemporalComb_3DNOW(TDeinterlaceInfo *info);
long FilterTemporalComb_MMX(TDeinterlaceInfo *info);
LONG UpdateBuffers(TDeinterlaceInfo *info, WORD*** pppTheseFields, WORD*** pppThoseFields);


/////////////////////////////////////////////////////////////////////////////
// Begin plugin globals
/////////////////////////////////////////////////////////////////////////////

// Array of time decayed average of shimmering at each pixel (well, sort of at each pixel)

static WORD*   gpShimmerMap = NULL;

// _Relative_ amount of shimmering (from gpShimmerMap) to trigger the shimmer compensation
// (See the RescaleParameter...() comments for an explanation of "relative")
// Set this too low, and noise will get misinterpreted as shimmering.  If you run a noise
// filter first, this problem is greatly reduced.

static LONG    gShimmerThreshold = 70;

// Percent weight given to old versus new shimmering
// This is the really important parameter -- Set it too low and you'll see lots of artifacts.
// Set it too high (> 95) and it'll take too long to identify dot crawl.

static LONG    gDecayCoefficient = 85;

// Thresholds used to detect motion. If two "in phase" pixels differ by more than this,
// motion is assumed and shimmer compensation is disabled.  They can be set surprisingly
// high in comparison with the values used in the temporal noise filter.  That's because
// this filter uses a better motion detector, and also only looks at shimmering pixels.
// Still, don't set them too high or you will see artifacts.  And don't set them too low
// or you will miss some of the worst dot crawl.  Note that you'll have to increase their
// values if you have Odd/Even Luma Peaking enabled, and can decrease them if Trade Speed
// for Accuracy in enabled.
// You can afford to set these a bit lower if you run a noise filter, first.  That cuts
// down on spurious identification of shimmering.

static LONG    gInPhaseLuminanceThreshold = 35;
static LONG    gInPhaseChromaThreshold = 35;


// Store the most recent four fields (and the current field) to prevent feedback effects
// gppFieldBuffer[0] = current field
// gppFieldBuffer[1], gppFieldBuffer[3] are previous two odd fields
// gppFieldBuffer[2], [4] are previous two even fields

#define NUM_BUFFERS     5

static LONG    gDoFieldBuffering;
static WORD*   gppFieldBuffer[NUM_BUFFERS] = {NULL, NULL, NULL, NULL, NULL};


static FILTER_METHOD TemporalCombMethod;


static SETTING FLT_TemporalCombSettings[FLT_TCOMB_SETTING_LASTONE] =
{
    {
        "Maximum in phase luminance difference", SLIDER, 0, &gInPhaseLuminanceThreshold,
        35, 0, 255, 1, 1,
        NULL,
        "TCombFilter", "InPhaseLuminanceThreshold", NULL,
    },
    {
        "Maximum in phase chroma difference", SLIDER, 0, &gInPhaseChromaThreshold,
        35, 0, 255, 1, 1,
        NULL,
        "TCombFilter", "InPhaseChromaThreshold", NULL,
    },
    {
        "Recall of past shimmering (percent)", SLIDER, 0, &gDecayCoefficient,
        85, 0, 99, 1, 1,
        NULL,
        "TCombFilter", "ShimmerRecall", NULL,
    },
    {
        "Shimmering to activate (percent)", SLIDER, 0, &gShimmerThreshold,
        70, 0, 100, 1, 1,
        NULL,
        "TCombFilter", "ShimmerThreshold", NULL,
    },
    {
        "Use temporal comb filter", ONOFF, 0, &TemporalCombMethod.bActive,
        FALSE, 0, 1, 1, 1,
        NULL,
        "TCombFilter", "UseCombFilter", NULL,
    },
    {
        "Trade speed for accuracy", ONOFF, 0, &gDoFieldBuffering,
        FALSE, 0, 1, 1, 1,
        NULL,
        "TCombFilter", "SpeedForAccuracy", NULL,
    },
};

static FILTER_METHOD TemporalCombMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Temporal Comb Filter",                 // Displayed name                          
    NULL,                                   // Use same name in menu
    FALSE,                                  // Not initially active
    TRUE,                                   // Do call on input  ! need to find out what this does !
    FilterTemporalComb_MMX, 
    0,                                      
    FALSE,                                  // Does not run if we've run out of time
    NULL,                                   // No initialization procedure
    ExitTemporalComb,                       // Deallocation routine
    NULL,                                   // Module handle; Filled in by DScaler
    FLT_TCOMB_SETTING_LASTONE,              // Number of settings
    FLT_TemporalCombSettings,
    WM_FLT_TCOMB_GETVALUE - WM_USER,        // Settings offset
    TRUE,
    3,
};


/////////////////////////////////////////////////////////////////////////////
// Main code (included from FLT_TemporalComb.asm)
/////////////////////////////////////////////////////////////////////////////

#define IS_MMXEXT
#include "FLT_TemporalComb.asm"
#undef IS_MMXEXT

#define IS_SSE
#include "FLT_TemporalComb.asm"
#undef IS_SSE

#define IS_3DNOW
#include "FLT_TemporalComb.asm"
#undef IS_3DNOW

#define IS_MMX
#include "FLT_TemporalComb.asm"
#undef IS_MMX


////////////////////////////////////////////////////////////////////////////
// Start of utility code
/////////////////////////////////////////////////////////////////////////////

void __cdecl ExitTemporalComb(void)
{
    CleanupTemporalComb();
}


void CleanupTemporalComb(void)
{
    DWORD   Index = 0;
    if (gpShimmerMap != NULL)
    {
        free(gpShimmerMap);
        gpShimmerMap = NULL;
    }
    for ( ; Index < NUM_BUFFERS; ++Index)
    {
        if (gppFieldBuffer[Index] != NULL)
        {
            free(gppFieldBuffer[Index]);
            gppFieldBuffer[Index] = NULL;
        }
    }
}


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    // MMXEXT is (currently) the Athlon -- Coded separately since prefetchw is used

    if (CpuFeatureFlags & FEATURE_MMXEXT)
    {
        TemporalCombMethod.pfnAlgorithm = FilterTemporalComb_MMXEXT;
    }
    if (CpuFeatureFlags & FEATURE_SSE)
    {
        TemporalCombMethod.pfnAlgorithm = FilterTemporalComb_SSE;
    }
    else if (CpuFeatureFlags & FEATURE_3DNOW)
    {
        TemporalCombMethod.pfnAlgorithm = FilterTemporalComb_3DNOW;
    }
    else
    {
        TemporalCombMethod.pfnAlgorithm = FilterTemporalComb_MMX;
    }
    return &TemporalCombMethod;
}


// Need malloc() and free(), so this is commented out

/*
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
*/


// Update the (paired circular) field buffer array
// Returns 1000 if the filter needs to abort (due either to memory allocation problems
// or a partially full buffer), 0 otherwise

LONG UpdateBuffers(TDeinterlaceInfo *info, WORD*** pppTheseFields, WORD*** pppThoseFields)
{
    DWORD           Index = 0;
    static LONG     HistoryWait = NUM_BUFFERS + 1;
    WORD*           pTempBuffer = NULL;

    --HistoryWait;

    // We need the last four fields to be available. We've already checked the
    // 2nd and 4th, so...

    if ((pppThoseFields[0] == NULL) || (pppThoseFields[2] == NULL))
    {
        return 1000;
    }
    for ( ; Index < NUM_BUFFERS; ++Index)
    {
        if (gppFieldBuffer[Index] == NULL)
        {
            HistoryWait = Index;
            gppFieldBuffer[Index] = malloc(info->OverlayPitch * sizeof(short) * info->FieldHeight);
            if (gppFieldBuffer[Index] == NULL)
            {
                return 1000;    // !! Should notify user !!
            }
        }

    }

    // Slide the field history along

    if (info->IsOdd)
    {
        pTempBuffer = gppFieldBuffer[4];
        gppFieldBuffer[4] = gppFieldBuffer[2];
        gppFieldBuffer[2] = gppFieldBuffer[0];
        gppFieldBuffer[0] = pTempBuffer;
    }
    else
    {
        pTempBuffer = gppFieldBuffer[3];
        gppFieldBuffer[3] = gppFieldBuffer[1];
        gppFieldBuffer[1] = gppFieldBuffer[0];
        gppFieldBuffer[0] = pTempBuffer;
    }

    // Copy current field to gppFieldBuffer[0] so we can compare against it without feedback

    for (Index = 0 ; Index < (DWORD)info->FieldHeight; ++Index)
    {
        info->pMemcpy(gppFieldBuffer[0] + Index*(info->OverlayPitch/sizeof(short)),
            pppTheseFields[0][Index], info->LineLength);
    }
    if (HistoryWait > 0)
    {
        return 1000;            // Need to have filled the buffers with useful data
    }
    HistoryWait = 0;
    return 0;
}


/*
The C version

Yes, the assembly version is much different.  The C version uses some ideas which got
removed from the assembly version for speed reasons.  It's also a bit easier to read.

{
    unsigned short***   theseFrames;
    unsigned int        thisLine;
    static int          sIndicatorCounter = 0;

    ++sIndicatorCounter;

    if (gShimmerMap == NULL) {
        unsigned long    index;
        gShimmerMap = malloc(info->OverlayPitch * info->FrameHeight * sizeof(short));
        if (!gShimmerMap) {
            return 1000;
        }
        for (index = 0; index < (info->OverlayPitch * info->FrameHeight * sizeof(short))/sizeof(long); ++index) {
            gShimmerMap[index] = 0;
        }
    }

    // mapPtr starts by pointing to the second line, since we need to be able
    // to access the previous line, too.
    if (info->IsOdd) {
        theseFrames = info->OddLines;
    }
    else {
        theseFrames = info->EvenLines;
    }

    if (!theseFrames[0] || !theseFrames[1] || !theseFrames[2]) {
        return 1000;
    }

    // We detect shimmer in the current line, but check shimmer values in the previous line
    // This is done to allow information from lower in the picture to inform pixels further
    // up.
    for (thisLine = 2; thisLine < info->FieldHeight - 1; ++thisLine) {
        unsigned short*     mapPtr;
        unsigned short*     srcPtr;
        unsigned short*     srcUpPtr;
        unsigned short*     lastPtr;
        unsigned short*     lastUpPtr;
        unsigned short*     lastLastPtr;
        unsigned int        hIndex;

        // OverlayPitch/2 is the number of shorts per line
        mapPtr = gShimmerMap
                    + (thisLine * info->OverlayPitch)
                    + ((info->IsOdd) ? info->OverlayPitch/2 : 0)
                    + 1;  // We're starting on the second pixel


        srcPtr = theseFrames[0][thisLine];
        srcUpPtr = theseFrames[0][thisLine - 1];
        lastPtr = theseFrames[1][thisLine];
        lastUpPtr = theseFrames[1][thisLine - 1];
        lastLastPtr = theseFrames[2][thisLine];

        for (hIndex = 1; hIndex < info->FrameWidth - 1; ++hIndex) {
            unsigned long       thisY, thisUV;
            unsigned long       lastY, lastUV;
            unsigned long       lastLastY, lastLastUV;
            int                 inPhaseDelta, outPhaseDelta;
            
            thisY = (*srcPtr) & 0x00FF;
            lastY = (*lastPtr) & 0x00FF;
            lastLastY = (*lastLastPtr) & 0x00FF;
            inPhaseDelta = thisY - lastLastY;
            inPhaseDelta = (inPhaseDelta > 0) ? inPhaseDelta : -inPhaseDelta;
            outPhaseDelta = thisY - lastY;
            outPhaseDelta = (outPhaseDelta > 0) ? outPhaseDelta : -outPhaseDelta;
            // Update changemap where significant shimmering was detected
            if ((outPhaseDelta > inPhaseDelta) && (inPhaseDelta < gTemporalLuminanceThreshold)) {
                *(mapPtr - info->OverlayPitch) += gAccumVertical;
                *(mapPtr - info->OverlayPitch/2) += gAccumVertical;
                *(mapPtr - 1) += gAccumHorizontal;
                *(mapPtr) += gAccumulateAt;
                *(mapPtr + 1) += gAccumHorizontal;
                *(mapPtr + info->OverlayPitch/2) += gAccumVertical;
                *(mapPtr + info->OverlayPitch) += gAccumVertical;
            }
            else {
                thisUV = (*srcPtr) & 0xFF00;
                lastUV = (*lastPtr) & 0xFF00;
                lastLastUV = (*lastLastPtr) & 0xFF00;
                inPhaseDelta = thisUV - lastLastUV;
                inPhaseDelta = (inPhaseDelta > 0) ? inPhaseDelta : -inPhaseDelta;
                inPhaseDelta >>= 8;
                outPhaseDelta = thisUV - lastUV;
                outPhaseDelta = (outPhaseDelta > 0) ? outPhaseDelta : -outPhaseDelta;
                outPhaseDelta >>= 8;
                // Update changemap where significant shimmering was detected
                if ((outPhaseDelta > inPhaseDelta) && (inPhaseDelta < gTemporalChromaThreshold)) {
                    *(mapPtr - info->OverlayPitch) += gAccumVertical;
                    *(mapPtr - info->OverlayPitch/2) += gAccumVertical;
                    *(mapPtr - 1) += gAccumHorizontal;
                    *(mapPtr) += gAccumulateAt;
                    *(mapPtr + 1) += gAccumHorizontal;
                    *(mapPtr + info->OverlayPitch/2) += gAccumVertical;
                    *(mapPtr + info->OverlayPitch) += gAccumVertical;
                }
            }
            if (*(mapPtr - info->OverlayPitch) > gAveragingThreshold) {
                // Update changemap near any pixel which passes the threshold required for averaging
                if (gFunctionTest) {
                    *(mapPtr - info->OverlayPitch - info->OverlayPitch) += gTriggeredAccum;
                    *(mapPtr - info->OverlayPitch - info->OverlayPitch/2) += gTriggeredAccum;
                    *(mapPtr - info->OverlayPitch - 1) += gTriggeredAccum;
                    *(mapPtr - info->OverlayPitch) += gTriggeredAccum;
                    *(mapPtr - info->OverlayPitch + 1) += gTriggeredAccum;
                    *(mapPtr - info->OverlayPitch/2) += gTriggeredAccum;
                    *(mapPtr) += gTriggeredAccum;
                }
                if ((gIndicator) && !(sIndicatorCounter % 16)) {
                    *srcUpPtr = 0xFFFF;
                }
                else {
                    *srcUpPtr = ((*srcUpPtr & 0xFEFE) >> 1) + ((*lastUpPtr & 0xFEFE) >> 1);
                }
            }
            
            *(mapPtr - info->OverlayPitch) = ((*(mapPtr - info->OverlayPitch)) * gDecayNumerator) >> 9;
                        

            ++srcPtr;
            ++srcUpPtr;
            ++lastPtr;
            ++lastUpPtr;
            ++lastLastPtr;
            ++mapPtr;
        }
    }



    // need to clear up MMX registers
    _asm
    {
        emms
    }

    return 1000;
}

*/