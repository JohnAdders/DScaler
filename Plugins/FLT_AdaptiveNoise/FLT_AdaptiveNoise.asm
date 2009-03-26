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

// This is the implementation of the noise filter described in FLT_AdaptiveNoise.c .
// It's in a separate file to make it easier for me to allow different options
// using the preprocessor and switch between them by toggling a checkbox.  The
// architecture also makes it easy to try processor specific code, if it becomes
// useful.

#if defined( USE_SPATIAL )
    #if defined( IS_DEBUG_FLAG )
    long FilterAdaptiveNoise_DEBUG_SPATIAL( TDeinterlaceInfo* pInfo )
    #elif defined( USE_PREFETCH )
    long FilterAdaptiveNoise_PREFETCH_SPATIAL( TDeinterlaceInfo* pInfo )
    #else   // Normal version
    long FilterAdaptiveNoise_SPATIAL( TDeinterlaceInfo* pInfo )
    #endif
#else
    #if defined( IS_DEBUG_FLAG )
    long FilterAdaptiveNoise_DEBUG( TDeinterlaceInfo* pInfo )
    #elif defined( USE_PREFETCH )
    long FilterAdaptiveNoise_PREFETCH( TDeinterlaceInfo* pInfo )
    #else   // Normal version
    long FilterAdaptiveNoise( TDeinterlaceInfo* pInfo )
    #endif
#endif  // Name of core routine
{
    BYTE*           pUp = NULL;                                 // Pointer to previous scanline
    BYTE*           pLocalHistogram = NULL;                     // Copy of pointer to histogram array
    const LONG      Cycles = (pInfo->LineLength/8) - 1;         // Number of blocks per line
    LONG            Index = 0;
    __int64         qwLastPixel = 0;                            // Holds pixel to ease horizontal blending
    const __int64   qwHighTail = 0x0000000000013333;            // Amount (120%) of difference curve where newPixel is incremented toward oldPixel

    BYTE*           pLast = NULL;                               // Pointer to field[t = -2], or [t = -1] for progressive material
    LONG            LongBaseline = 0;
    const __int64   qwPositive = 0x0000000000007FFF;            // Used to limit values so a signed comparison can be used
    const __int64   qwBlackMask = 0x0000000020202020;           // Minimum color value below which a pixel is left out of the histogram
    const __int64   qwWhiteMask = 0x00000000EFEFEFEF;           // Maximum color value above which a pixel is left out of the histogram

    // Amount of motion correlation between adjacent (horizontal) blocks
    __int64         qwHorizontalDecay= 0;
    const __int64   qwChromaMask = 0xFF00FF00FF00FF00;
    const __int64   qwOnes = 0x0101010101010101;
    __int64         qwMaxNoise = 0;                             // Maximum allowed movement measure

    BYTE*           pMap= NULL;                                 // Motion map pointer
    const DWORD     LocalInputPitch = pInfo->InputPitch;
    DWORD           BottomLine = pInfo->FieldHeight;    // Limit processing to displayed picture;
    DWORD           ThisLine = pInfo->DestRect.top/2;
    DWORD           OldSI = 0;
    DWORD           OldSP = 0;
    DWORD           DWordNoiseMultiplier = 0;
    DWORD           DWordMaxNoise = 0;

    // Most of this could be done with fixed point, but that would make my head hurt
    static DOUBLE   sCumBaseline = INITIAL_BASELINE ;           // The derived cumulative baseline noise color difference
    static DOUBLE   sCumDifferencePeak = INITIAL_PEAK_MEAN;     // The derived average noise value from the color difference histogram
    DOUBLE          CurveWidth = 0.0;                           // Estimated width of the noise distribution curve
    DOUBLE          DoubleBaseline = 0.0;                       // Derived baseline for noise; affected by "Stability"

    DOUBLE          DoubleNoiseThreshold = 0.0;                 // Derived multiplier for noise compensation; affected by "Noise Reduction"
    DOUBLE          DoubleVerticalDecay = 0.0;
    DOUBLE          DoubleHorizontalDecay = 0.0;
    DOUBLE          HighNoiseThreshold = 0.0;

    const __int64   qwHighDWord = 0xFFFFFFFF00000000;
    const __int64   qwHistogramMax = HISTOGRAM_LENGTH - 1;      // Avoid histogram buffer overruns
    static long     sLastInputPitch = 0;
    static DWORD    sLastFieldHeight = 0;
    DWORD           DWordDifferencePeak = 0;
    DWORD           LastIndex = 0;                              // Index to previous frame

    __int64         qwNextPixel = 0;
    const __int64   qwNoLowBits = 0xFEFEFEFEFEFEFEFE;
    const __int64   qwFourFifthsMult = 0xCCCCCCCCCCCCCCCC;
    const __int64   qwLowWordOn = 0x000000000000FFFF;

    __int64         qwSpatialBaseline = 0;
    __int64         qwSpatialBaselineMove = 0;

    __int64         qwSpatialMult = 0;
    __int64         qwSpatialMultMove = 0;
    __int64         qwNoiseBaseline = 0;                        // Lowest amount of color difference in pure noise
    __int64         qwNoiseBaselineMove = 0;
    __int64         qwNoiseMultiplier = 0;                      // Amount to multiply times (color difference - baseline) to get
    __int64         qwNoiseMultiplierMove = 0;

    __int64         qwVerticalDecay = 0;
    DOUBLE          VarianceReduction = 0.0;

    DWORD           DWordHighMultiplier = 0;
    long            EffectiveStability = 0;
    static BOOLEAN  sDoShowDot = FALSE;
    char            tempString[16];

#if defined( IS_DEBUG_FLAG )
    __int64         qwThresholdTest = 0x0000000000000001;
    __int64         qwThresholdAnd = 0x0;
    __int64         qwPinkThreshold = 0x7FFFFFFF7FFFFFFF;
    __int64         qwPinkLowThreshold = 0;
    __int64         qwPinkHolder = 0;
    static LONG     sTestingCounter;
#endif

    if( (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK) == 0 )
    {
        LastIndex = 1;
    }
    else    // Interlaced
    {
        LastIndex = 2;
    }

    // Need to have the current field and same field from previous frame to do the filtering.
    if( (pInfo->PictureHistory[0] == NULL) || (pInfo->PictureHistory[LastIndex] == NULL) )
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
#define PINK_INTERVAL   30
    // Flash pink once per interval within the specified cumulative color difference range
    --sTestingCounter;
    if( sTestingCounter < 0 )
    {
        sTestingCounter = PINK_INTERVAL;
    }
    qwThresholdTest = 0;
    qwThresholdAnd = 0x0000000000000000;
    if( sTestingCounter == PINK_INTERVAL )
    {
        qwPinkThreshold = gTestingThreshold;
        qwPinkLowThreshold = gSecondTestingThreshold;
        qwThresholdTest = 0x0000000000007FFF;
        qwThresholdAnd = 0xFFFFFFFFFFFFFFFF;
    }
#endif


    // Initialize buffers

    if (gpChangeMap == NULL)
    {
        gpChangeMap = DumbAlignedMalloc(pInfo->InputPitch * pInfo->FieldHeight);
        if (gpChangeMap == NULL)
        {
            return 1000;    // !! Should notify user !!
        }
        for ( Index = 0 ; (DWORD)Index < (pInfo->InputPitch * pInfo->FieldHeight)/sizeof(DOUBLE); ++Index)
        {
            ((DOUBLE*)gpChangeMap)[Index] = 0.0;
        }
    }

    if (gpHistogram == NULL)
    {
        gpHistogram = DumbAlignedMalloc((HISTOGRAM_LENGTH) * sizeof(DWORD));
        if (gpHistogram == NULL)
        {
            return 1000;
        }
    }
    // Histogram needs to be emptied before each field
    for( Index = 0; Index < HISTOGRAM_LENGTH * sizeof(DWORD) / sizeof(DOUBLE); ++Index)
    {
        ((DOUBLE*)gpHistogram)[Index] = 0.0;
    }


    // Show the indicator dot.  Note that it is better to do this before processing rather than after,
    // so that the dot isn't detected as motion.  (But it's still seen as motion when it turns on or
    // off.)
    // !! Assumes pixel width >= 32 !!

    if( (gShowLockDot == TRUE) )
    {
        if( sDoShowDot == TRUE )
        {
            *((DWORD*)( pInfo->PictureHistory[0]->pData + (16 * pInfo->InputPitch) + 64)) = 0x4FFF;
        }
        // Set the motion map at the lock dot to 0
        *((DWORD*)( gpChangeMap + (LOCK_DOT_V * pInfo->InputPitch) + LOCK_DOT_H*2)) = 0;
    }



    // Interpret parameters
    // Read FLT_AdaptiveNoise.txt for a description of what's going on, here

    DoubleVerticalDecay = (sqrt(1.0 + gDecayCoeff/50.0) - 1.0)/2.0;
    DoubleHorizontalDecay = DoubleVerticalDecay*DoubleVerticalDecay;

    qwHorizontalDecay = 0xFFFF000000000000 + (DWORD) (0x10000*DoubleHorizontalDecay);
    qwVerticalDecay = 0xFFFF000000000000 + (DWORD) (0x10000*DoubleVerticalDecay);


    CurveWidth = sCumDifferencePeak - sCumBaseline;
    CurveWidth = CurveWidth * sqrt(sCumDifferencePeak/80.0);
    if( sCumDifferencePeak - CurveWidth > sCumBaseline )
    {
        // CurveWidth = sCumDifferencePeak - sCumBaseline;
    }
    else if( sCumDifferencePeak - 2*CurveWidth < 1.0 )
    {
        CurveWidth = (sCumDifferencePeak - 1.0)/2.0;
    }

    if (CurveWidth < 5.0)
    {
        CurveWidth = 5.0;
    }

    if( (gIndicator == TRUE) && ((sCumDifferencePeak-CurveWidth)/HISTOGRAM_SCALE < pInfo->FieldHeight - 20) )
    {   // Show baseline estimate
        *(pInfo->PictureHistory[0]->pData + (((LONG)((sCumDifferencePeak-CurveWidth)/HISTOGRAM_SCALE) + 20) * pInfo->InputPitch) + 424) = 0xFF;
    }

    // Variance in averaged areas is reduced by ~1.3 (determined empirically).  Note that complete reduction would be sqrt(2)
    VarianceReduction = 1.3;

    if( gShowReadout == TRUE )
    {
        sprintf(tempString, "%6.3f", (sCumDifferencePeak*(100.0-gDecayCoeff)*VarianceReduction)/(4.0*100.0));
        DrawString(pInfo->PictureHistory[0]->pData, pInfo->InputPitch, 1, 1, tempString);
    }




    // Figure out the thresholds and multipliers for noise reduction.  The "...Move" terms represent the
    // increase or decrease of the threshold due to motion

    // This (and many of the other settings) are something of a fudge when the baseline is < 0.  It would be
    // better to allow for negative baseline values.

    DoubleBaseline = sCumDifferencePeak - 2*CurveWidth + (CurveWidth*gNoiseReduction/12.5)*(gStability/100.0);
    LongBaseline = (LONG) (DoubleBaseline + 0.5);
    if( LongBaseline < 0 )
    {
        LongBaseline = 0;
        EffectiveStability = (long)(((2*CurveWidth - sCumDifferencePeak)*100.0*12.5 / (CurveWidth*gNoiseReduction)) + 0.5);
    }
    else
    {
        EffectiveStability = gStability;
    }
    qwNoiseBaseline = LongBaseline;

    DoubleBaseline = (sCumDifferencePeak - 2*CurveWidth + (CurveWidth*gNoiseReduction/12.5)*(EffectiveStability/100.0))*(VarianceReduction-1.0);
    LongBaseline = (LONG) (DoubleBaseline + 0.5);
    if( LongBaseline < 0 )
    {
        LongBaseline = 0;
    }
    qwNoiseBaselineMove = LongBaseline;


    DoubleNoiseThreshold = (CurveWidth*gNoiseReduction/12.5)*((100-EffectiveStability)/100.0);
    if( DoubleNoiseThreshold < 3.0 )
    {   // Make sure DWordNoiseMultiplier is less than 0x10000/2, so we can use a signed multiply
        DoubleNoiseThreshold = 3.0;
    }
    DWordNoiseMultiplier = (DWORD)((0x10000+(DoubleNoiseThreshold/2.0))/DoubleNoiseThreshold);
    qwNoiseMultiplier = DWordNoiseMultiplier;

    HighNoiseThreshold = (CurveWidth*gNoiseReduction*VarianceReduction/12.5)*((100-EffectiveStability)/100.0);
    if( HighNoiseThreshold < 4.0 )
    {   // Make sure DWordNoiseMultiplier is less than 0x10000/2, so we can use a signed multiply
        HighNoiseThreshold = 4.0;
    }
    DWordHighMultiplier = (DWORD)((0x10000+(HighNoiseThreshold/2.0))/HighNoiseThreshold);

    qwNoiseMultiplierMove = (DWordNoiseMultiplier - DWordHighMultiplier);


#if defined( USE_SPATIAL )
    DoubleBaseline = (sCumDifferencePeak - 2*CurveWidth + (CurveWidth*gNoiseReduction*gSmoothing/1250.0)*(EffectiveStability/100.0))*0.433013;
    LongBaseline = (LONG) (DoubleBaseline + 0.5);
    if( LongBaseline < 0 )
    {
        LongBaseline = 0;
    }
    qwSpatialBaseline = (LongBaseline*(100-gDecayCoeff))/100;
    DoubleBaseline = (sCumDifferencePeak - 2*CurveWidth + (CurveWidth*gNoiseReduction*gSmoothing/1250.0)*(EffectiveStability/100.0))*0.791727;
    LongBaseline = (LONG) (DoubleBaseline + 0.5);
    if( LongBaseline < 0 )
    {
        LongBaseline = 0;
    }
    qwSpatialBaselineMove = (LongBaseline*(100-gDecayCoeff))/100;


    DoubleNoiseThreshold = (CurveWidth*gNoiseReduction*gSmoothing*0.433013/1250.0)*((100-EffectiveStability)/100.0);
    DoubleNoiseThreshold = DoubleNoiseThreshold*(100-gDecayCoeff)/100;
    if( DoubleNoiseThreshold < 3.0 )
    {   // Make sure DWordNoiseMultiplier is less than 0x10000/2, so we can use a signed multiply
        DoubleNoiseThreshold = 3.0;
    }
    DWordNoiseMultiplier = (DWORD)((0x10000+(DoubleNoiseThreshold/2.0))/DoubleNoiseThreshold);
    qwSpatialMult = DWordNoiseMultiplier;

    HighNoiseThreshold = (CurveWidth*gNoiseReduction*gSmoothing*1.22474/1250.0)*((100-EffectiveStability)/100.0);
    HighNoiseThreshold = HighNoiseThreshold*(100-gDecayCoeff)/100;
    if( HighNoiseThreshold < 4.0 )
    {   // Make sure DWordNoiseMultiplier is less than 0x10000/2, so we can use a signed multiply
        HighNoiseThreshold = 4.0;
    }
    DWordHighMultiplier = (DWORD)((0x10000+(HighNoiseThreshold/2.0))/HighNoiseThreshold);

    qwSpatialMultMove = (DWordNoiseMultiplier - DWordHighMultiplier);

    if( gDoSmoothing == FALSE )
    {
        qwSpatialBaseline = 0x0000;
        qwSpatialBaselineMove = 0x0000;
        qwSpatialMult = 0xFFFF;
        qwSpatialMultMove = 0x0000;
    }
#endif // spatial parameter determination


    // Don't let the cumulative noise difference get too large -- otherwise, there will be a delay after a scene
    // change before noise reduction kicks in.  This also makes sense in terms of the noise statistic: A change of
    // 500 doesn't really give a better indicator of motion than a change of 250.

    // This formula makes sure that -- after one field/block's distance, the motion evidence will be significantly
    // greater than necessary to provide evidence for motion.  This doesn't sound like a really good idea -- Intuitively,
    // you'd think that decay to allow a certain amount of color averaging would be better.  But this setup is necessary
    // to prevent the color difference curve from getting an artifactual peak.  Well, actually, this method does not
    // prevent the artifactual peak -- rather, it places it far enough from the real peak to allow it to be identified
    // and ignored.

    if( gDecayCoeff != 0 )
    {
        DWordMaxNoise = (DWORD) ( (DoubleNoiseThreshold + sCumBaseline +
            NOISE_MAX_CURVE_WIDTHS*CurveWidth)*(100/gDecayCoeff) +
            NOISE_MAX_EXTRA );
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


    // Make sure we leave one line on the top and bottom unprocessed.  This avoids the need to make
    // a special case during the image processing.

    if( ThisLine < 1 )
    {
        ThisLine = 1;
    }
    if( (LONG)BottomLine > pInfo->FieldHeight - 2 )
    {
        BottomLine = pInfo->FieldHeight - 2;
    }

    // At the (first?) and last lines, set the map value to the low peak value.
    // this helps avoid artifacts at the top and bottom of the screen
    DWordDifferencePeak = (DWORD) (sCumDifferencePeak + 0.5);
    for( Index = 0; (DWORD)Index < pInfo->LineLength/sizeof(DWORD); ++Index )
    {
        *(DWORD*)(gpChangeMap + (BottomLine * pInfo->InputPitch) + Index*sizeof(DWORD)) = DWordDifferencePeak;
    }
/*  Doesn't seem to be necessary
    for( Index = 0; (DWORD)Index < pInfo->LineLength/sizeof(DWORD); ++Index )
    {
        *(DWORD*)(gpChangeMap + (ThisLine * pInfo->InputPitch) + Index*sizeof(DWORD)) = DWordDifferencePeak;
    }
*/

    // Prepare pointers for image processing

    pLast = pInfo->PictureHistory[LastIndex]->pData + (ThisLine * pInfo->InputPitch);
    pMap = gpChangeMap + (ThisLine * pInfo->InputPitch);
    pLocalHistogram = gpHistogram;
    pUp = pInfo->PictureHistory[0]->pData + ((ThisLine-1) * pInfo->InputPitch);

    // Main image processing loop

    for( ; ThisLine < BottomLine; ++ThisLine )
    {

#undef MAINLOOP_LABEL

#if defined( USE_SPATIAL )
    #if defined( IS_DEBUG_FLAG )
    #define MAINLOOP_LABEL DoNext8Bytes_DEBUG_SPATIAL
    #elif defined( USE_PREFETCH )
    #define MAINLOOP_LABEL DoNext8Bytes_PREFETCH_SPATIAL
    #else   // Normal version
    #define MAINLOOP_LABEL DoNext8Bytes_SPATIAL
    #endif  // Assembly jump target
#else
    #if defined( IS_DEBUG_FLAG )
    #define MAINLOOP_LABEL DoNext8Bytes_DEBUG
    #elif defined( USE_PREFETCH )
    #define MAINLOOP_LABEL DoNext8Bytes_PREFETCH
    #else   // Normal version
    #define MAINLOOP_LABEL DoNext8Bytes
    #endif  // Assembly jump target
#endif
        _asm
        {
            mov     ecx, Cycles

            mov     OldSI, esi                      // Preserve si register
            mov     OldSP, esp                      // Preserve sp register

            // Load pointers into normal registers
            mov     eax, dword ptr[pMap]
            mov     edx, LocalInputPitch            // edx = bytes between lines in the map

            movq    mm6, qword ptr[eax]             // mm6 = upMapValue

            mov     edi, dword ptr[pUp]
            mov     ebx, dword ptr[pLast]

            movq    mm5, qword ptr[edi + edx]
            movq    qwNextPixel, mm5

            pxor    mm7, mm7
            movq    qwLastPixel, mm7

align 8
MAINLOOP_LABEL:

            // Asterisks indicate register assignments which are used much later in the routine

            movq    mm3, qword ptr[eax + edx]       // mm3 = mapValue

            movq    mm2, qword ptr[edi + edx + 8]   // mm2 = NextPixel
            movq    mm0, qwNextPixel                // * mm0 = NewPixel
            movq    qwNextPixel, mm2

            movq    mm1, qword ptr[ebx]             // * mm1 = OldPixel


            // Get the noise/motion statistic "N"

            // Find the cumulative color difference

            movq    mm2, mm0                        // mm2 = NewPixel
            psadbw  mm2, mm1                        // mm2 = Sum(|byte differences Old,New|)

#if defined( USE_PREFETCH )
            prefetchnta[ebx + PREFETCH_STRIDE]
#endif
            paddusw mm2, mm3                        // mm2 = Cumulative weighted sum of differences ("N")
            pminsw  mm2, qwMaxNoise                 // * mm2 = N limited to reasonable range
            pand    mm2, qwLowWordOn

            // Deal with correlations

            // Decay the movement below and to the right
            movq    mm3, qword ptr[eax + 2*edx]     // mm3 = downMapValue
            movq    mm5, qword ptr[eax + edx + 8]   // get next map
            movq    mm7, mm2                        // mm7 = Cumulative differences
            pmulhuw mm7, qwHorizontalDecay          // mm7 = weight to apply to next horizontal block
#if defined( USE_PREFETCH )
            prefetchnta[eax + 2*edx + PREFETCH_STRIDE]
#endif
            paddusw mm5, mm7                        // mm5 = sum(right map value, decayed value at this pixel)
            movq    qword ptr[eax + edx + 8], mm5   // store updated rightMap

            movq    mm5, qword ptr[eax + edx - 8]   // get left map
            paddusw mm5, mm7                        // mm5 = sum(old next weight, proposed new next weight)
            movq    qword ptr[eax + edx - 8], mm5   // store updated left map

            // mm6 (= up map value) is collected at the bottom of the loop to avoid some cache problems
            movq    mm4, mm2                        // mm4 =  Cumulative differences
            pmulhuw mm4, qwVerticalDecay
            paddusw mm6, mm4                        // mm6 = sum(up map value, decayed value at this pixel)
            movq    qword ptr[eax], mm6
            paddusw mm3, mm4                        // mm3 = sum(down map value, decayed value at this pixel)
            movq    qword ptr[eax + 2*edx], mm3


#if defined( IS_DEBUG_FLAG )
/*
            // Determine whether the cumulative difference is within the test parameter thresholds
            movq    mm6, mm2                        // mm6 = Cumulative weighted sum of differences
            movq    mm5, qwPinkLowThreshold
            pcmpgtd mm5, mm6                        // mm5 (low dword) on if difference < second test threshold
            pcmpgtd mm6, qwPinkThreshold            // mm6 (low dword) on if difference > test threshold
            pand    mm6, mm5
            movq    qwPinkHolder, mm6
*/
#endif  // Cumulative difference range indicator


            // Statistics

            // Edge ("black") effect compensation: Skip statistics if color value is near 0
            // "White" edge effect stuff (less important than the black edge): skip if value is near 255
            movq    mm6, mm0                        // mm6 = NewPixel
            pxor    mm5, mm5                        // mm5 = 0
            psubusb mm6, qwBlackMask                // mm6 (low dword bytes) = 0 where near to "black"
            movq    mm7, mm0                        // mm7 = NewPixel
            pcmpeqb mm6, mm5                        // mm6 bytewise on where near to black
            psubusb mm7, qwWhiteMask                // mm7 (low dword bytes) = 0 where not near to "white"
            pcmpeqd mm6, mm5                        // mm6 = (low dword) on iff no elements near black
            pcmpgtb mm7, mm5                        // mm7 bytewise on where near to white (mm7 < 127 due to previous step)
            pcmpeqd mm7, mm5                        // mm7 = (low dword) on iff no elements near white

            // Apply the edge effect results
            pand    mm6, mm7                        // mm6 on iff no elements (in low dword) near white or black
            pcmpeqb mm5, mm5                        // mm5 = all on
            pxor    mm6, mm5                        // mm6 off iff no elements near white or black
            movq    mm4, mm2                        // mm4 = Cumulative weighted sum of differences
            por     mm4, mm6                        // mm4 = cumulative weighted sum, or low word on where ~black

            // Accumulate the histogram data
            pand    mm4, qwPositive                 // mm4 limited to positive values (rarely necessary)
            pminsw  mm4, qwHistogramMax             // mm4 limited to fit in histogram array
            movd    esp, mm4                        // esp = (limited) cumulative weighted sum

#if defined( USE_PREFETCH )
            prefetchnta[edi + edx + PREFETCH_STRIDE]
#endif
            shl     esp, 2                          // esp = corresponding jump from pHistogram
            mov     esi, dword ptr[pLocalHistogram]
            add     esi, esp                        // esi = Pointer into histogram array
            inc     dword ptr[esi]



            // Subtract out the ~miniumum noise value
            movq    mm7, qword ptr[eax + edx]       // mm7 = mapValue
            psrlq   mm7, 48                         // mm7 = old multiplier in low word

/*
            movq    mm3, mm7
            movq    mm3, qwThresholdTest
            psubusw mm3, mm7
            pxor    mm4, mm4
            pcmpeqw mm4, mm3
            pshufw  mm3, mm4, 0x00
            pand    mm3, qwThresholdAnd
            por     mm0, mm3
*/


            movq    mm6, qwNoiseBaselineMove
            pmulhuw mm6, mm7                        // mm6 = motion increase to baseline
            psubusw mm2, qwNoiseBaseline
            psubusw mm2, mm6                        // mm2 = difference - baseline

            movq    mm6, qwNoiseMultiplierMove      // mm6 = base amount to decrease multipler for movement
            movq    mm4, qwNoiseMultiplier          // mm4 = muliplier before motion adjustment
            pmulhuw mm6, mm7                        // mm6 = amount to decrease multiplier due to movement
            psubusw mm4, mm6                        // mm4 = adjusted noise multiplier

            pmaddwd mm2, mm4                        // mm2 (low Dword) = Multiplier to move toward new pixel value
            movq    mm3, mm2                        // mm3 (low Dword) = Multiplier to move toward new pixel value



            // Determine the proper muliplier to use:
            // NoiseMultiplier is always less than 0x10000/2, so we can use a signed multiply:
#if defined( USE_PREFETCH )
            prefetchnta[eax + edx + PREFETCH_STRIDE]
#endif
            pxor    mm6, mm6                        // mm6 = 0
            pcmpgtw mm2, mm6                        // mm2 = 0x.......FFFF.... if ignoring old pixel value
            psrlq   mm2, 16                         // mm2 (low word) = FFFF if ignoring old pixel value (else 0)***********

            movq    mm4, qwHighTail                 // mm4 = 0x0....014000
            pcmpgtd mm4, mm3                        // mm4 = low dword on if multiplier is less than the "tail" threshold (< 0x14000 )
            psrlq   mm4, 31                         // mm4 = 1 if multiplier is less than the "tail" threshold
            pand    mm4, mm2                        // mm4 = 1 if multiplier is in the "tail" ( 0x14000 > multiplier > 0x0FFFF )

            por     mm3, mm2                        // mm3 = (low word) Adjusted multiplier to move toward new
            psubusw mm3, mm4                        // mm3 = (low word) Adjusted multiplier to move toward new, with tail correction

            pshufw  mm2, mm3, 0x00                  // * mm2 = (wordwise) adjusted multiplier to move toward new

            pmulhuw mm3, mm3                        // mm3 = motion^2
//            pmulhuw mm3, mm2                        // mm3 = motion^3
            psllq   mm3, 48                         // mm3 = motion^2 in high word
            movq    qword ptr[eax + edx], mm3       // store map value

/***************************************/

            // Apply the multiplier to the color differences
            // Determine the color difference:
            movq    mm4, mm1                        // mm4 = OldPixel
            psubusb mm4, mm0                        // mm4 = bytewise max(old - new, 0)
            psubusb mm0, mm1                        // mm0 = bytewise max (new - old, 0)
            por     mm4, mm0                        // * mm4 = bytewise |new - old|
            pcmpeqb mm0, mm6                        // * mm0 = bytewise on where old >= new
            movq    mm5, mm4                        // mm5 = bytewise |new - old|
            pcmpeqb mm5, mm6                        // mm5 bytewise on where |new - old| = 0

            pcmpeqw mm6, mm6                        // mm6 = all on
            pcmpeqw mm6, mm2                        // mm6 = all on iff multiplier = 0xFFFF (~= 1 in fixed point)
            pandn   mm5, qwOnes                     // mm5 bytewise 1 where |new - old| != 0 (used for "round at least 1")
            pand    mm6, mm4                        // mm6 = bytewise |new - old| iff multiplier = 0xFFFF
            pmaxub  mm6, mm5                        // mm6 = same, corrected to a minimum of 1 where there is change

            // Multiply the change by the determined compensation factor
            movq    mm3, qwChromaMask
            movq    mm7, mm4                        // mm7 = bytewise |new - old|
            pand    mm7, mm3                        // mm7 = bytewise |new - old| chroma
            pmulhuw mm7, mm2                        // mm7 = amount of chroma to add/subtract from old, with remainders
#if defined( USE_PREFETCH )
            prefetchnta[eax + PREFETCH_STRIDE]
#endif
            pand    mm7, mm3                        // mm7 = amount of chroma to add/subtract from old
            pandn   mm3, mm4                        // mm3 = bytewise |new - old| luma
            pmulhuw mm3, mm2                        // mm3 = amount of luma to add/subtract from old
            por     mm7, mm3                        // mm7 = amount to add/subtract from old
            pmaxub  mm7, mm6                        // mm7 = corrected to to deal with the motion and round up to 1

            // Apply the calculated change
            movq    mm3, mm7                        // mm3 = same
            pand    mm7, mm0                        // mm7 = amount to subtract
            psubusb mm1, mm7                        // mm1 = Partially adjusted oldpixel
            pandn   mm0, mm3                        // mm0 = amount to add
            paddusb mm1, mm0                        // mm1 = Adjusted pixel value


#if defined( USE_SPATIAL )
            // Spatial smoother
            // mm2 still = multiplier to move toward new
            // Stored for use below: 1 (sort of)
            // mm1 = adjusted newPixel
            // mm2 = multipler to move toward new (the bigger, the more spatial filtering to use)

            pmulhuw mm2, mm2

            movq    mm0, qword ptr[edi]             // mm0 = UpPixel
            movq    mm4, qword ptr[edi + 2*edx]     // mm4 = DownPixel
            pand    mm0, qwNoLowBits
            pavgb   mm0, mm4                        // mm0 = Average(Up, Down)
            pand    mm0, qwNoLowBits
            pshufw  mm4, mm1, 0x4E                  // mm4 = dword swapped from thisPixel

            movq    mm3, qwLastPixel
            psrlq   mm3, 32
            movq    mm5, qwNextPixel
            psllq   mm5, 32
            por     mm5, mm3                        // mm5 = left, right dwords
            pand    mm5, qwNoLowBits
            pavgb   mm5, mm4

            pavgb   mm0, mm5                        // mm0 = average of nearby values = "nearPixel"


            movq    mm3, mm0                        // mm3 = same
            movq    mm5, mm1                        // mm5 = ThisPixel

            psadbw  mm3, mm1                        // mm3 = Sum(|byte differences This,Near|)

            psubusw mm3, qwSpatialBaseline          // mm3 = difference - minimum (no motion) baseline
            movq    mm4, qwSpatialBaselineMove
            pmulhuw mm4, mm2                        // mm4 = increase in baseline due to motion
            psubusw mm3, mm4                        // mm3 = Max(spatial difference - baseline, 0)


            movq    mm4, qwSpatialMultMove
            pmulhuw mm4, mm2                        // mm4 = multiplier reduction based on motion
            movq    mm6, qwSpatialMult
            psubusw mm6, mm4                        // mm5 = motion adjusted noise multiplier

            pmaddwd mm3, mm6                        // mm3 = Multiplier to move toward thisPixel values
            movq    mm6, qwLowWordOn                // mm6 = 0xFFFF
            psubd   mm6, mm3                        // mm6 (low Dword) = Multiplier to move toward nearPixel values (may be negative)
            movq    mm3, mm6                        // mm3 = same
            pxor    mm7, mm7                        // mm7 = 0
            pcmpgtd mm3, mm7                        // mm3 = 0xFFFFFFFF iff multiplier > 0
            pand    mm3, mm6                        // mm3 = (low word) multiplier to move toward nearPixel values (0 if no movement)
            pmulhuw mm3, mm2                        // mm3 = (low word) multiplier to move toward nearPixel, adjusted for temporal amount

            pshufw  mm3, mm3, 0x00                  // * mm3 = (wordwise) multiplier to move toward nearPixel

            pmulhuw mm3, qwFourFifthsMult           // * mm3 = (wordwise) multiplier to move toward nearPixel

            psubusb mm5, mm0                        // mm5 = bytewise max(this - near, 0)
            psubusb mm0, mm1                        // mm0 = bytewise max(near - this, 0)
            por     mm5, mm0                        // mm5 = bytewise |this - near|
            pcmpeqb mm0, mm7                        // mm0 bytewse on where (this >= near)

            movq    mm4, qwChromaMask
            movq    mm7, mm5                        // mm7 = bytewise |this - near|
            pand    mm7, mm4                        // mm7 = bytewise |this - near| chroma
            pmulhuw mm7, mm3                        // mm7 = amount of chroma to add/subtract from thisPixel, with remainders
            pand    mm7, mm4                        // mm7 = amount of chroma to add/subtract from thisPixel

            pandn   mm4, mm5                        // mm4 = bytewise |this - near| luma
            pmulhuw mm4, mm3                        // mm4 = amount of luma to add/subtract from thisPixel
            por     mm7, mm4                        // mm7 = amount to add/subtract from thisPixel

            movq    mm3, mm7                        // mm3 = same
            pand    mm7, mm0                        // mm7 = amount to subtract
            psubusb mm1, mm7                        // mm1 = Partially adjusted oldpixel
            pandn   mm0, mm3                        // mm0 = amount to add
            paddusb mm1, mm0                        // mm1 = Adjusted pixel value

#endif // USE_SPATIAL



#if defined( IS_DEBUG_FLAG )
            // Set a pink block if the cumulative difference was between the test thresholds
            movq    mm6, qwPinkHolder
            por     mm1, mm6                        // mm1 low dword all on if above threshold
#endif
            movq    mm6, qword ptr[eax + 8]         // * mm6 = upMapValue (done in a weird place to avoid a bank conflict)

            movq    qwLastPixel, mm1
            movq    qword ptr[edi + edx], mm1       // store the pixel's adjusted value

            add     edi, 8
            add     eax, 8
            add     ebx, 8


            dec     ecx
            jnz     MAINLOOP_LABEL

            mov     esi, OldSI                      // Restore si register
            mov     esp, OldSP                      // Restore sp register
        }

        pUp += pInfo->InputPitch;
        pLast += pInfo->InputPitch;
        pMap += pInfo->InputPitch;

    }

    // Some floating point operations follow
    _asm
    {
        emms
    }


    AnalyzeHistogram( pInfo, DWordMaxNoise, &sCumBaseline, &sCumDifferencePeak, &sDoShowDot );

    return 1000;
}

