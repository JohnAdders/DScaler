/////////////////////////////////////////////////////////////////////////////
// $Id$
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

#include <limits.h>
#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

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
  that there isn't motion.  The threshold on the weighted average is restricted
  to a range such that motion artifacts are kept to a minimum.

Definition of "in phase": If two pixels would normally have the same chroma
carrier frequency phase, they are referred to as "in phase." Pixels in the
same location an even number of frames apart should be in phase, and pixels
an odd number of frames apart should be out of phase.  Note that half
resolution progressive images (which are used by older game consoles)
have a different phase pattern, so they would need an entirely different
filter.

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
  processing time.  Then again, this would have to wait for faster processors.)
  Really this is a silly idea -- the current algorithm already works pretty well,
  and is already computationally expensive, so this would be a waste.  But there
  might be some easy improvement if the algorithm took some account for the
  noisiness of the data -- i.e., the shimmering evidence could be taken as a more
  complicated function of the differences between fields 0, -2, and -4.
- Make better use of phase information.  The program doesn't track the past phase
  of shimmering -- it just remembers how much shimmering there was in general.
- Average between fields instead of frames when it's beneficial.
*/
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// A few constants
/////////////////////////////////////////////////////////////////////////////

// This might need to be tuned for other processors.
// Then again, it doesn't seem to matter much what I set it to on my Athlon
// so long as it's between 64 and about 300

#define PREFETCH_STRIDE                         128

// This number is not arbitrary: It's 2^14, which is the highest power of 2 divisor
// possible without triggering a sign based error.

#define FIXED_POINT_DIVISOR                     16384

// The threshold at which motion artifacts and fade artifacts are equally problematic

#define ACCUMULATION_MINIMUM_METHOD_THRESHOLD   62

// A menu makes the NTSC/PAL choice clearer than a checkbox would be

typedef enum
{
    MODE_NTSC,
    MODE_PAL,
    MODE_LASTONE,
} combMODE;

// Uncomment this and turn on "High Quality" to turn the pixels which
// would have been averaged pink.  (Without the High Quality setting, feedback will
// make the result impossible to usefully interpret.)

//#define TCOMB_DEBUG


/////////////////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////////////////

void __cdecl    StartTemporalComb( void );
void __cdecl    ExitTemporalComb( void );
void            CleanupTemporalComb( void );

BYTE*           DumbAlignedMalloc(int siz);
BYTE*           DumbAlignedFree(BYTE* x);

void            UpdateRequiredHistory(void);
BOOL            ChangeFieldBuffering(long NewValue);
BOOL            ChangeVideoMode(long NewValue);

__declspec(dllexport) FILTER_METHOD*    GetFilterPluginInfo( long CpuFeatureFlags );
long            DispatchTemporalComb( TDeinterlaceInfo* pInfo );

void            RescaleParameters_MMXEXT( LONG* pDecayNumerator, LONG* pAveragingThreshold, DWORD Accumulation );
void            RescaleParameters_SSE( LONG* pDecayNumerator, LONG* pAveragingThreshold, DWORD Accumulation );
void            RescaleParameters_3DNOW( LONG* pDecayNumerator, LONG* pAveragingThreshold, DWORD Accumulation );
void            RescaleParameters_MMX( LONG* pDecayNumerator, LONG* pAveragingThreshold, DWORD Accumulation );

long            FilterTemporalComb_MMXEXT_PREFETCH( TDeinterlaceInfo* pInfo );
long            FilterTemporalComb_SSE_PREFETCH( TDeinterlaceInfo* pInfo );
long            FilterTemporalComb_3DNOW_PREFETCH( TDeinterlaceInfo* pInfo );
long            FilterTemporalComb_MMXEXT( TDeinterlaceInfo* pInfo );
long            FilterTemporalComb_SSE( TDeinterlaceInfo* pInfo );
long            FilterTemporalComb_3DNOW( TDeinterlaceInfo* pInfo );
long            FilterTemporalComb_MMX( TDeinterlaceInfo* pInfo );

LONG            UpdateBuffers( TDeinterlaceInfo* pInfo );


/////////////////////////////////////////////////////////////////////////////
// Begin plugin globals
/////////////////////////////////////////////////////////////////////////////


// Look back 4 or 8 fields?

combMODE                gMode = MODE_NTSC;


static long             gUsePrefetching = TRUE;

// Array of time decayed average of shimmering at each pixel (well, sort of at each pixel)

static BYTE*            gpShimmerMap = NULL;

// _Relative_ amount of shimmering (from gpShimmerMap) to trigger the shimmer compensation
// (See the RescaleParameter...() comments for an explanation of "relative")
// Set this too low, and noise will get misinterpreted as shimmering.  If you run a noise
// filter first, this problem is greatly reduced.

static LONG             gShimmerThreshold = 70;

// Percent weight given to old versus new shimmering
// This is the really important parameter -- Set it too low and you'll see lots of artifacts.
// Set it too high (> 95) and it'll take too long to identify dot crawl.

static LONG             gDecayCoefficient = 85;

// Threshold used to detect motion. If two "in phase" pixels differ by more than this,
// motion is assumed and shimmer compensation is disabled.  It can be set surprisingly
// high in comparison with the values used in the temporal noise filter.  That's because
// this filter uses a better motion detector, and also only looks at shimmering pixels.
// Still, don't set it too high or you will see artifacts.  And don't set it too low
// or you will miss some of the worst dot crawl.  Note that you'll have to increase its
// value if you have Odd/Even Luma Peaking enabled, and can decrease it if High Quality
// is enabled.
// You can afford to set this a bit lower if you run a noise filter, first.  That cuts
// down on spurious identification of shimmering.

static LONG             gInPhaseColorThreshold = 35;


// For "High Quality," store the most recent four fields (and the current
// field) to prevent feedback effects

#define MAX_BUFFERS     9

static LONG             gDoFieldBuffering = FALSE;
static BYTE*            gppFieldBuffer[MAX_BUFFERS] = { NULL };



static FILTER_METHOD    TemporalCombMethod;


LPCSTR ModeList[] =
{
    "NTSC",
    "PAL"
};

static SETTING          FLT_TemporalCombSettings[FLT_TCOMB_SETTING_LASTONE] =
{
    {   // Keep old name for .ini file for backward compatibility
        "Maximum Color Variation", SLIDER, 0, (LONG_PTR*)&gInPhaseColorThreshold,
        35, 0, 255, 1, 1,
        NULL,
        "TCombFilter", "InPhaseChromaThreshold", NULL,
    },
    {
        "Fast Memory Access", ONOFF, 0, (LONG_PTR*)&gUsePrefetching,
        TRUE, 0, 1, 1, 1,
        NULL,
        "TCombFilter", "UsePrefetching", NULL,
    },
    {
        "Recall of Past Shimmering (Percent)", SLIDER, 0, (LONG_PTR*)&gDecayCoefficient,
        85, 0, 99, 1, 1,
        NULL,
        "TCombFilter", "ShimmerRecall", NULL,
    },
    {
        "Shimmering to Activate (Percent)", SLIDER, 0, (LONG_PTR*)&gShimmerThreshold,
        70, 0, 100, 1, 1,
        NULL,
        "TCombFilter", "ShimmerThreshold", NULL,
    },
    {
        "Temporal Comb Filter", ONOFF, 0, (LONG_PTR*)&TemporalCombMethod.bActive,
        FALSE, 0, 1, 1, 1,
        NULL,
        "TCombFilter", "UseCombFilter", NULL,
    },
    {
        "High Quality", ONOFF, 0, (LONG_PTR*)&gDoFieldBuffering,
        FALSE, 0, 1, 1, 1,
        NULL,
        "TCombFilter", "SpeedForAccuracy", ChangeFieldBuffering,
    },
    {
        "Video Mode", ITEMFROMLIST, 0, (LONG*)&gMode,
        MODE_NTSC, MODE_NTSC, MODE_LASTONE -1, 1, 1,
        ModeList,
        "TCombFilter", "DisplayMode", ChangeVideoMode,
    },
};

static FILTER_METHOD    TemporalCombMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Temporal Comb Filter",                 // Displayed name
    "Temporal Comb",                        // Name in menu
    FALSE,                                  // Not initially active
    TRUE,                                   // Call on input so pulldown can benefit from it
    DispatchTemporalComb,                   // Algorithm to use (really decided by GetFilterPluginInfo)
    0,                                      // Menu: assign automatically
    FALSE,                                  // Does not run if we're out of time
    StartTemporalComb,                      // Initialization procedure
    ExitTemporalComb,                       // Deallocation routine
    NULL,                                   // Module handle; Filled in by DScaler
    FLT_TCOMB_SETTING_LASTONE,              // Number of settings
    FLT_TemporalCombSettings,
    WM_FLT_TCOMB_GETVALUE - WM_APP,         // Settings offset
    TRUE,                                   // Can handle interlaced material (but progressive?)
    9,                                      // Required past fields (9 are only necessary with PAL low quality mode)
    IDH_TEMPORAL_COMB,
};



// Change the HistoryRequired field to match what is really necessary
// This is important since DScaler will otherwise skip the filter even when it could
// really run just fine.

BOOL ChangeFieldBuffering(long NewValue)
{
    gDoFieldBuffering = NewValue;
    UpdateRequiredHistory();
    return FALSE;
}


BOOL ChangeVideoMode(long NewValue)
{
    gMode = NewValue;
    UpdateRequiredHistory();
    return FALSE;
}


void UpdateRequiredHistory(void)
{
    if(gDoFieldBuffering == TRUE)
    {
        TemporalCombMethod.HistoryRequired = 1;
    }
    else
    {
        if(gMode == MODE_PAL)
        {
            TemporalCombMethod.HistoryRequired = 9;
        }
        else // gMode == MODE_NTSC
        {
            TemporalCombMethod.HistoryRequired = 5;
        }
    }
    return;
}



long DispatchTemporalComb( TDeinterlaceInfo* pInfo )
{

    if( gUsePrefetching == TRUE )
    {
       // MMXEXT with 3DNOW is the Athlon -- Coded separately since prefetchw is used
        if( (pInfo->CpuFeatureFlags & FEATURE_MMXEXT) && (pInfo->CpuFeatureFlags & FEATURE_3DNOW) )
        {
            FilterTemporalComb_MMXEXT_PREFETCH( pInfo );
        }
        else if( pInfo->CpuFeatureFlags & FEATURE_SSE )
        {
            FilterTemporalComb_SSE_PREFETCH( pInfo );
        }
        else if( pInfo->CpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterTemporalComb_3DNOW_PREFETCH( pInfo );
        }
        else
        {
            FilterTemporalComb_MMX( pInfo );
        }
    }
    else
    {
        if( (pInfo->CpuFeatureFlags & FEATURE_MMXEXT) && (pInfo->CpuFeatureFlags & FEATURE_3DNOW) )
        {
            FilterTemporalComb_MMXEXT( pInfo );
        }
        else if( pInfo->CpuFeatureFlags & FEATURE_SSE )
        {
            FilterTemporalComb_SSE( pInfo );
        }
        else if( pInfo->CpuFeatureFlags & FEATURE_3DNOW )
        {
            FilterTemporalComb_3DNOW( pInfo );
        }
        else
        {
            FilterTemporalComb_MMX( pInfo );
        }
    }
    return 1000;
}


/////////////////////////////////////////////////////////////////////////////
// Main code (included from FLT_TemporalComb.asm)
/////////////////////////////////////////////////////////////////////////////

#define USE_PREFETCH

    #define IS_MMXEXT
    #include "FLT_TemporalComb.asm"
    #undef  IS_MMXEXT

    #define IS_SSE
    #include "FLT_TemporalComb.asm"
    #undef  IS_SSE

    #define IS_3DNOW
    #include "FLT_TemporalComb.asm"
    #undef  IS_3DNOW

#undef USE_PREFETCH

    #define IS_MMXEXT
    #include "FLT_TemporalComb.asm"
    #undef  IS_MMXEXT

    #define IS_SSE
    #include "FLT_TemporalComb.asm"
    #undef  IS_SSE

    #define IS_3DNOW
    #include "FLT_TemporalComb.asm"
    #undef  IS_3DNOW

    #define IS_MMX
    #include "FLT_TemporalComb.asm"
    #undef  IS_MMX


////////////////////////////////////////////////////////////////////////////
// Start of utility code
/////////////////////////////////////////////////////////////////////////////


void __cdecl StartTemporalComb( void )
{
    UpdateRequiredHistory();
    return;
}


void __cdecl ExitTemporalComb( void )
{
    CleanupTemporalComb();
    return;
}


void CleanupTemporalComb( void )
{
    DWORD   Index = 0;
    if( gpShimmerMap != NULL )
    {
        DumbAlignedFree( gpShimmerMap );
        gpShimmerMap = NULL;
    }
    else
    {
        ;       // do nothing
    }
    for( ; Index < MAX_BUFFERS; ++Index)
    {
        if( gppFieldBuffer[Index] != NULL )
        {
            DumbAlignedFree( gppFieldBuffer[Index] );
            gppFieldBuffer[Index] = NULL;
        }
        else
        {
            ;   // do nothing
        }
    }
    return;
}


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo( long CpuFeatureFlags )
{
    return &TemporalCombMethod;
}


// Aligned memory allocation wrapper
// Copied from Tom Barry's aligned malloc/free in StillSource.cpp

BYTE* DumbAlignedMalloc(int siz)
{
    BYTE* x = (BYTE*)CoTaskMemAlloc(siz+16);
    BYTE** y = (BYTE**) (x+16);
    y = (BYTE**) (((unsigned int) y & 0xfffffff0) - 4);
    *y = x;
    return (BYTE*) y+4;
}


BYTE* DumbAlignedFree(BYTE* x)
{
    BYTE* y =  *(BYTE**)(x-4);
    CoTaskMemFree(y);
    return 0;
}


BOOL WINAPI NoCRTDllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}


// Update (roll) the field buffer array
// Returns 1000 if the filter needs to abort (due either to memory allocation problems
// or a partially full buffer), 0 otherwise
// Initialization here could be improved, but I won't bother since this routine is
// likely to become obsolete.

LONG UpdateBuffers( TDeinterlaceInfo* pInfo )
{
    // NTSC needs to save 4 past fields, PAL needs to save 8
    const LONG        kUsedBuffers[2] = {5, 9};
    int             Index = 0;
    static LONG     sHistoryWait = 0;
    BYTE*           pTempBuffer = NULL;

    --sHistoryWait;

    for ( ; Index < kUsedBuffers[gMode]; ++Index)
    {
        if( gppFieldBuffer[Index] == NULL )
        {
            sHistoryWait = Index;    // ~ number of fields until buffer is filled with data
            gppFieldBuffer[Index] = DumbAlignedMalloc( pInfo->InputPitch * pInfo->FieldHeight );
            if( gppFieldBuffer[Index] == NULL )
            {
                return 1000;        // !! Should notify user !!
            }
        }
        else
        {
            ;                       // do nothing
        }

    }


    // Roll the field history along

    pTempBuffer = gppFieldBuffer[kUsedBuffers[gMode] - 1];
    for(Index = kUsedBuffers[gMode] - 1; Index != 0; --Index)
    {
        gppFieldBuffer[Index] = gppFieldBuffer[Index - 1];
    }
    gppFieldBuffer[0] = pTempBuffer;

    // Verify that we have data for this field

    if(pInfo->PictureHistory[0] == NULL)
    {
        return 1000;
    }

    // Copy current field to gppFieldBuffer[0] so we can compare against it without feedback
    for(Index = 0; Index < pInfo->FieldHeight; ++Index)
    {
        pInfo->pMemcpy(
            gppFieldBuffer[0] + (Index*pInfo->InputPitch),
            pInfo->PictureHistory[0]->pData + (Index*pInfo->InputPitch),
            pInfo->LineLength
        );
    }
    DO_EMMS;

    if (sHistoryWait > 100)
    {
        return 1000;            // Don't use buffers until they've been initialized with useful data
    }
    else
    {
        sHistoryWait = 0;
        return 0;
    }
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
    DO_EMMS;

    return 1000;
}

*/