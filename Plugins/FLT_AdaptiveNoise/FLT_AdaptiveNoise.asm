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
// Revision 1.1.1.1  2001/12/31 01:25:17  lindsey
// Added FLT_AdaptiveNoise
//
//
/////////////////////////////////////////////////////////////////////////////

// This is the implementation of the noise filter described in FLT_AdaptiveNoise.c .
// It's in a separate file to make it easier for me to allow different options
// using the preprocessor and switch between them by toggling a checkbox.  The
// architecture also makes it easy to try processor specific code, if it becomes
// useful.


#if defined( IS_DEBUG_FLAG )
long FilterAdaptiveNoise_DEBUG( TDeinterlaceInfo* pInfo )
#else   // Normal version
long FilterAdaptiveNoise( TDeinterlaceInfo* pInfo )
#endif  // Name of core routine
{
    BYTE*           pSource = NULL;                                 // Pointer to current field
    BYTE*           pLocalHistogram = NULL;                         // Copy of pointer to histogram array
    LONG            Index = 0;
    const LONG      Cycles = (pInfo->LineLength/8)*8;               // Number of blocks per line * 8
    // Noise multiplier is (1/gNoiseReduction), in 0x10000 fixed point.
    // This times the measured noise gives the weight given to the new pixel color
    __int64         qwNoiseBaseline = 0;                            // Lowest amount of color difference in pure noise
    __int64         qwNoiseMultiplier = 0;                          // Amount to multiply times (color difference - baseline) to get

    BYTE*           pLast = NULL;                                   // Pointer to field[t = -2]
    LONG            LongBaseline = 0;
    const __int64   qwPositive = 0x0000000000007FFF;                // Used to limit values so a signed comparison can be used
    const __int64   qwBlackMask = 0x0000000020202020;               // Minimum color value below which a pixel is left out of the histogram
    const __int64   qwWhiteMask = 0x00000000EFEFEFEF;               // Maximum color value above which a pixel is left out of the histogram
                                                                    // the noise compensation

    // Amount of motion correlation between adjacent (up-down or temporal) blocks
    const __int64   qwMotionDecay = (0x10000*gDecayCoefficient + 50)/100;
    // Amount of motion correlation between adjacent (horizontal) blocks
    const __int64   qwHorizontalDecay= (0x10000*gDecayCoefficient*gDecayCoefficient + 5000)/10000;
    const __int64   qwChromaMask = 0xFF00FF00FF00FF00;
    const __int64   qwOnes = 0x0101010101010101;

    BYTE*           pMap= NULL;                                     // Motion map pointer
    BYTE*           pMapDown= NULL;                                 // Pointer to next line's motion map
    DWORD           BottomLine = pInfo->SourceRect.bottom/2;        // Limit processing to displayed picture; !!! Wrong for half-height !!!
    DWORD           ThisLine = pInfo->SourceRect.top/2;             // !!! Wrong for half-height modes !!!
    DWORD           OldSI;
    DWORD           OldSP;
    DWORD           DWordNoiseMultiplier = 0;
    DWORD           DWordMaxNoise = 0;

    __int64         qwMaxNoise = 0;                                 // Maximum allowed movement measure
    const __int64   qwHighDWord = 0xFFFFFFFF00000000;
    const __int64   qwHistogramMax = HISTOGRAM_LENGTH - 1;          // Avoid histogram buffer overruns
    static DWORD    sLastInputPitch = 0;
    static DWORD    sLastFieldHeight = 0;

    // Most of this could be done with fixed point, but that would make my head hurt
    static DOUBLE   sCumBaseline = 10.0;                            // The derived cumulative baseline noise color difference
    static DOUBLE   sCumDifferencePeak = 20.0;                      // The derived average noise value from the color difference histogram
    DOUBLE          CurveWidth = 0.0;                               // Estimated width of the noise distribution curve
    DOUBLE          DoubleBaseline = 0.0;                           // Derived baseline for noise; affected by "Stability"
    DOUBLE          DoubleNoiseThreshold = 0.0;                     // Derived multiplier for noise compensation; affected by "Noise Reduction"


#if defined( IS_DEBUG_FLAG )
    __int64         qwPinkThreshold = 0x7FFFFFFF7FFFFFFF;
    __int64         qwPinkLowThreshold = 0;
    __int64         qwPinkHolder = 0;
    static LONG     sTestingCounter = 0;
#endif

    // Need to have the current and next-to-previous fields to do the filtering.
    if( (pInfo->PictureHistory[0] == NULL) || (pInfo->PictureHistory[2] == NULL) )
    {
        return 1000;
    }

    if( (pInfo->InputPitch != sLastInputPitch) || ((DWORD)pInfo->FieldHeight != sLastFieldHeight) )
    {
        sLastInputPitch = pInfo->InputPitch;
        sLastFieldHeight = pInfo->FieldHeight;
        CleanupAdaptiveNoise();
    }




#if defined( IS_DEBUG_FLAG )
#define PINK_INTERVAL   120
    // Flash pink once per interval within the specified cumulative color difference range
    if( --sTestingCounter < 0 )
    {
        sTestingCounter = PINK_INTERVAL;
    }
    if( sTestingCounter == 1 )
    {
        qwPinkThreshold = gTestingThreshold;
        qwPinkLowThreshold = gSecondTestingThreshold;
    }
#endif


    // Initialize buffers

    if (gpChangeMap == NULL)
    {
        gpChangeMap = malloc(pInfo->InputPitch * pInfo->FieldHeight);
        if (gpChangeMap == NULL)
        {
            return 1000;    // !! Should notify user !!
        }
        for ( Index = 0 ; (DWORD)Index < (pInfo->InputPitch * pInfo->FieldHeight)/sizeof(DWORD); ++Index)
        {
            gpChangeMap[Index] = 0;
        }
    }

    if (gpHistogram == NULL)
    {
        gpHistogram = malloc((HISTOGRAM_LENGTH) * sizeof(DWORD));
        if (gpHistogram == NULL)
        {
            return 1000;
        }
    }
    // Histogram needs to be emptied before each field
    for( Index = 0; Index < HISTOGRAM_LENGTH * sizeof(DWORD) / sizeof(DOUBLE); ++Index)
    {
        ((DOUBLE*)gpHistogram)[Index] = 0;
    }


    // Interpret parameters

    CurveWidth = sCumDifferencePeak - sCumBaseline;

    DoubleNoiseThreshold = sCumBaseline + CurveWidth/4.0 + CurveWidth*gNoiseReduction/25.0;
    if( DoubleNoiseThreshold < 5.0 )
    {
        DoubleNoiseThreshold = 5.0;
    }
    DWordNoiseMultiplier = (DWORD)((0x10000+(DoubleNoiseThreshold/2.0))/DoubleNoiseThreshold);
    qwNoiseMultiplier = DWordNoiseMultiplier;

    DoubleBaseline = (sCumBaseline - CurveWidth) + ((DoubleNoiseThreshold - (sCumBaseline - CurveWidth))*(gStability/100.0));
    LongBaseline = (LONG) (DoubleBaseline + 0.5);
    if( LongBaseline < 0 )
    {
        LongBaseline = 0;
    }
    qwNoiseBaseline = LongBaseline;



    // Don't let the cumulative noise difference get too large -- otherwise, there will be a delay after a scene
    // change before noise reduction kicks in.  This also makes sense in terms of the noise statistic: A change of
    // 500 doesn't really give a better indicator of motion than a change of 250.
    // This formula could be improved:  It's currently a quick and dirty solution
    if( gDecayCoefficient != 0 )
    {
        DWordMaxNoise = (120*100*100)/((100 - gDecayCoefficient)*gDecayCoefficient);
        DWordMaxNoise += (DWORD) ( sCumDifferencePeak + CurveWidth + 100.0*CurveWidth/gDecayCoefficient );
        if( DWordMaxNoise > SHRT_MAX )
        {
            DWordMaxNoise = SHRT_MAX;
        }
    }
    else
    {
        DWordMaxNoise = SHRT_MAX;
    }
    qwMaxNoise = DWordMaxNoise;


    // Make sure we have leave one line on the top and bottom unprocessed.  This avoids the need to make
    // a special case for them.
    if( ThisLine < 1 )
    {
        ThisLine = 1;
    }
    if( (LONG)BottomLine > pInfo->FieldHeight - 1 )   // !!! Wrong for half height modes !!!
    {
        BottomLine = pInfo->FieldHeight - 1;
    }

    // Prepare pointers for image processing
    pSource = pInfo->PictureHistory[0]->pData + (ThisLine * pInfo->InputPitch);
    pLast = pInfo->PictureHistory[2]->pData + (ThisLine * pInfo->InputPitch);
    pMap = gpChangeMap + (ThisLine * pInfo->InputPitch);
    pMapDown = pMap + pInfo->InputPitch;
    pLocalHistogram = gpHistogram;

    // Main image processing loop
    for( ; ThisLine < BottomLine; ++ThisLine )
    {

#if defined( IS_DEBUG_FLAG )
#define MAINLOOP_LABEL DoNext8Bytes_DEBUG
#else   // Normal version
#define MAINLOOP_LABEL DoNext8Bytes
#endif  // Assembly jump target

        _asm 
        {
            mov     OldSI, esi                  // Preserve si register
            mov     OldSP, esp                  // Preserve sp register
            
            mov     ecx, Cycles

            // Load pointers into normal registers
            mov     edi, dword ptr[pSource]
            mov     ebx, dword ptr[pLast]
            mov     eax, dword ptr[pMap]
            mov     edx, dword ptr[pMapDown]

            pxor    mm7, mm7
            pxor    mm5, mm5            

align 8
MAINLOOP_LABEL:

            // Asterisks indicate register assignments which are used much later in the routine

            movq    mm0, qword ptr[edi]     // * mm0 = NewPixel
            movq    mm1, qword ptr[ebx]     // * mm1 = OldPixel
            movq    mm3, qword ptr[eax]     // mm3 = mapValue
            movq    mm6, qword ptr[edx]     // mm6 = mapDownValue


            // Get the noise adjustment multiplier
            
            // First find the cumulative color difference
            movq    mm2, mm0                // mm2 = NewPixel
            psadbw  mm2, mm1                // mm2 = Sum(|byte differences Old,New|)
            prefetchnta[edi + PREFETCH_STRIDE]
            paddusw mm2, mm3                // mm2 = Cumulative weighted sum of differences
            movq    mm4, mm2                // mm4 = Cumulative weighted sum of differences


            // Deal with correlations
            
            // Decay the movement at this pixel
            pminsw  mm4, qwMaxNoise         // limit noise value to reasonable range
            pmulhuw mm4, qwMotionDecay      // mm4 = temporally decayed sum of differences
            prefetchnta[ebx + PREFETCH_STRIDE]
            movq    qword ptr[eax], mm4     // store updated map

            // Decay the movement below and to the right
            movq    mm5, qword ptr[eax + 8] // get next map
            movq    mm7, mm2                // mm7 = Cumulative weights
            pmulhuw mm7, qwHorizontalDecay  // mm7 = weight to apply to next horizontal block
            prefetchnta[eax + PREFETCH_STRIDE]
            pmaxsw  mm5, mm7                // mm5 = max(old next weight, proposed new next weight)
            movq    qword ptr[eax + 8], mm5 // store updated next map
            pmaxsw  mm6, mm4                // mm6 = max(down map value, decayed value at this pixel)
            movq    qword ptr[edx], mm6     // store updated down map


#if defined( IS_DEBUG_FLAG )
            // Determine whether the cumulative difference is within the test parameter thresholds
            movq    mm6, mm2                // mm6 = Cumulative weighted sum of differences
            movq    mm5, qwPinkLowThreshold
            pcmpgtd mm5, mm6                // mm5 (low dword) on if difference < second test threshold
            pcmpgtd mm6, qwPinkThreshold    // mm6 (low dword) on if difference > test threshold
            pand     mm6, mm5 
            movq    qwPinkHolder, mm6
#endif  // Cumulative difference range indicator


            // Statistics

            // Edge ("black") effect compensation: Skip statistics if color value is near 0
            movq    mm6, mm0                // mm6 = NewPixel
            pxor    mm5, mm5                // mm5 = 0
            psubusb mm6, qwBlackMask        // mm6 (low dword bytes) = 0 where near to "black"
            pcmpeqb mm6, mm5                // mm6 bytewise on where near to black
            pcmpeqd mm6, mm5                // mm6 = (low dword) on iff no elements near black

            // "White" edge effect stuff (less important than the black edge): skip if value is near 255
            movq    mm7, mm0                // mm7 = NewPixel
            psubusb mm7, qwWhiteMask        // mm7 (low dword bytes) = 0 where not near to "white"
            pcmpgtb mm7, mm5                // mm7 bytewise on where near to white (mm7 < 127 due to previous step)
            pcmpeqd mm7, mm5                // mm7 = (low dword) on iff no elements near white

            // Apply the edge effect results
            pand    mm6, mm7                // mm6 on iff no elements (in low dword) near white or black
            pcmpeqb mm5, mm5                // mm5 = all on
            pxor    mm6, mm5                // mm6 off iff no elements near white or black
            movq    mm4, mm2                // mm4 = Cumulative weighted sum of differences
            por     mm4, mm6                // mm4 = cumulative weighted sum, or low word on where ~black

            // Accumulate the histogram data
            pand    mm4, qwPositive         // mm4 limited to positive values (rarely necessary)
            pminsw  mm4, qwHistogramMax     // mm4 limited to fit in histogram array
            movd    esp, mm4                // esp = (limited) cumulative weighted sum
            shl     esp, 2                  // esp = corresponding jump from pHistogram
            mov     esi, dword ptr[pLocalHistogram]
            add     esi, esp                // esi = Pointer into histogram array
            inc     dword ptr[esi]


            // Calculate the noise correction for this block:

            // Subtract out the ~miniumum noise value
            psubusw mm2, qwNoiseBaseline

            // Determine the proper muliplier to use:
            // NoiseMultiplier is always less than 0x10000/2, so we can use a signed multiply:
            pmaddwd mm2, qwNoiseMultiplier  // mm2 (low Dword) = Multiplier to move toward new pixel value
            movq    mm3, mm2                // mm3 = same
            pxor    mm6, mm6                // mm6 = 0
            pcmpgtw mm2, mm6                // mm2 = 0x.......FFFF.... if ignoring old pixel value
            psrlq   mm2, 16                 // mm2 (low word) = FFFF if ignoring old pixel value (else 0)
            por     mm3, mm2                // mm3 = (low word) Adjusted multiplier to move toward new
            pshufw  mm2, mm3, 0x00          // * mm2 = (wordwise) adjusted multiplier to move toward new

            // Apply the multiplier to the color differences
            // Determine the color difference:
            movq    mm4, mm1                // mm4 = OldPixel
            psubusb mm4, mm0                // mm4 = bytewise max(old - new, 0)
            psubusb mm0, mm1                // mm0 = bytewise max (new - old, 0)
            por     mm4, mm0                // * mm4 = bytewise |new - old|
            pcmpeqb mm0, mm6                // * mm0 = bytewise on where old >= new
            movq    mm5, mm4                // mm5 = bytewise |new - old|
            pcmpeqb mm5, mm6                // mm5 bytewise on where |new - old| = 0
            pandn   mm5, qwOnes             // mm5 bytewise 1 where |new - old| != 0 (used for "round at least 1")

            pcmpeqw mm6, mm6                // mm6 = all on
            pcmpeqw mm6, mm2                // mm6 = all on iff multiplier = 0xFFFF (~= 1 in fixed point)
            pand    mm6, mm4                // mm6 = bytewise |new - old| iff multiplier = 0xFFFF
            pmaxub  mm6, mm5                // mm6 = same, corrected to a minimum of 1 where there is change

            // Multiply the change by the determined compensation factor
            movq    mm7, mm4                // mm7 = bytewise |new - old|
            movq    mm3, qwChromaMask
            pand    mm7, mm3                // mm7 = bytewise |new - old| chroma
            pmulhuw mm7, mm2                // mm7 = amount of chroma to add/subtract from old, with remainders
            prefetchnta[edx + PREFETCH_STRIDE]
            pand    mm7, mm3                // mm7 = amount of chroma to add/subtract from old
            pandn   mm3, mm4                // mm3 = bytewise |new - old| luma
            pmulhuw mm3, mm2                // mm3 = amount of luma to add/subtract from old
            por     mm7, mm3                // mm7 = amount to add/subtract from old
            pmaxub  mm7, mm6                // mm7 = corrected to to deal with the motion and round up to 1
            
            // Apply the calculated change
            movq    mm3, mm7                // mm3 = same
            pand    mm7, mm0                // mm7 = amount to subtract
            psubusb mm1, mm7                // mm1 = Partially adjusted oldpixel
            pandn   mm0, mm3                // mm0 = amount to add
            paddusb mm1, mm0                // mm1 = Adjusted pixel value

#if defined( IS_DEBUG_FLAG )
            // Set a pink block if the cumulative difference was between the test thresholds
            movq    mm6, qwPinkHolder
            por     mm1, mm6                // mm1 low dword all on if above threshold
#endif

            movq    qword ptr[edi], mm1

            add     edi, 8
            add     eax, 8
            add     ebx, 8

            sub     ecx, 8
            jnz     MAINLOOP_LABEL

            mov     esi, OldSI                  // Restore si register
            mov     esp, OldSP                  // Restore sp register
        }

#undef MAINLOOP_LABEL

        pSource += pInfo->InputPitch;
        pLast += pInfo->InputPitch;
        pMap += pInfo->InputPitch;
        pMapDown += pInfo->InputPitch;
    }

    // Some floating point operations follow
    _asm
    {
        emms
    }

    AnalyzeHistogram( pInfo, DWordMaxNoise, &sCumBaseline, &sCumDifferencePeak );

    return 1000;
}

