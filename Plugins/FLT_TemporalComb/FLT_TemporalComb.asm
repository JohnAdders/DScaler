/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_TemporalComb.asm,v 1.1 2001-08-23 06:38:43 adcockj Exp $
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
/////////////////////////////////////////////////////////////////////////////

// This is the implementation of the comb filter described in TemporalComb.c.
// It is broken out into this separate file because almost all the code is
// the same for the different processor variants, and doing it this way
// means we only have to maintain one copy of the code.


/////////////////////////////////////////////////////////////////////////////
// Processor specific notes:
/////////////////////////////////////////////////////////////////////////////

// - The MMXEXT (Athlon) and SSE code is identical, except for the use of
//   prefetchw (prefetch for read/write) on the Athlon.  The improvement
//   with prefetchw is pretty minor on my computer -- It may not even be
//   significant.  But it _should_ help, and there's no loss from doing so,
//   so I've kept it in.
// - Prefetching note:  I don't like prefetching -- It seems to be a command
//   whose effect needs to be tuned differently for every processor, and
//   possibly for every memory architecture.  But it gave the routine a 2X
//   speedup -- on my machine, at least -- so it'll stay.
// - The 3DNOW code uses prefetchw where appropriate, but not prefetch.
//   That's because -- based on an optimization guide -- prefetch doesn't
//   help the K6.
// - The MMX and 3DNOW code use pmulhw or pmulhrw instead of pmulhuw, which
//   results  in the loss of one bit of precision in the decay of the shimmer
//   map.  This matters some for very low decay percentages.
// - The MMX code uses a slower averaging routine which rounds down.The other
//   code uses a faster averaging routine which rounds up. It would be possible
//   to average better by rounding toward one of the operands, but it's not
//   worth the slowdown.
// - The 3DNOW code uses femms instead of emms.  Whee.


/////////////////////////////////////////////////////////////////////////////
// A few macros for the assembly code
/////////////////////////////////////////////////////////////////////////////

// Processor specific averaging:
// Set destMM to average of destMM and sourceMM
// (Code from DI_GreedyHM.h by Tom Barry)
// Note that the MMX version rounds down; the other versions round up.
// Changes destMM, tempMM

#undef AVERAGE
#if defined(IS_SSE) || defined(IS_MMXEXT)
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask) \
    pavgb destMM, sourceMM
#elif defined(IS_3DNOW)
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask) \
    pavgusb destMM, sourceMM
#else
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask) __asm \
    { \
	__asm movq tempMM, sourceMM \
	__asm pand tempMM, shiftMask \
	__asm psrlw tempMM, 1 \
	__asm pand destMM, shiftMask \
	__asm psrlw destMM, 1 \
	__asm paddusb destMM, tempMM \
    }
#endif // processor specific averaging routine


// Set destMM to a byte packed array of (bytewise) |source1MM - source2MM|
// Changes destMM, tempMM

#ifndef ABSOLUTE_DIFFERENCE_BYTES
#define ABSOLUTE_DIFFERENCE_BYTES(destMM, source1MM, source2MM, tempMM) __asm \
    { \
    __asm movq destMM, source1MM \
    __asm psubusb destMM, source2MM \
    __asm movq tempMM, source2MM \
    __asm psubusb tempMM, source1MM \
    __asm por destMM, tempMM \
    }
#endif // ifndef ABSOLUTE_DIFFERENCE_BYTES


// Low precision multiplication tweak:
// For processors without pmulhuw, use pmulhw or pmulhrw instead, but divide the
// coefficient by 2, then shift left 1 after multiplication

#undef MULTIPLY_WORDS_SHIFT_RIGHT_16
#if defined(IS_MMXEXT) || defined(IS_SSE)
#define MULTIPLY_WORDS_SHIFT_RIGHT_16(destMM, coefficient) \
    pmulhuw destMM, coefficient
#elif defined (IS_3DNOW)
#define MULTIPLY_WORDS_SHIFT_RIGHT_16(destMM, coefficient) __asm \
    { \
    __asm pmulhrw destMM, coefficient \
    __asm psllw destMM, 1 \
    }
#else \\ IS_MMX
#define MULTIPLY_WORDS_SHIFT_RIGHT_16(destMM, coefficient) __asm \
    { \
    __asm pmulhw destMM, coefficient \
    __asm psllw destMM, 1 \
    }
#endif // processor specific unsigned multiply and shift right 16


/////////////////////////////////////////////////////////////////////////////
// A few constants
/////////////////////////////////////////////////////////////////////////////

// This might need to be tuned for other processors.
// Then again, it doesn't seem to matter much what I set it to on my Athlon
// so long as it's between 64 and about 300

#define PREFETCH_STRIDE     128

// This number is not arbirtary: It's 2^14, which is the highest power of 2 divisor
// possible without triggering a sign based error.

#define FIXED_POINT_DIVISOR 16384

// Amount to add to the shimmer map when shimmering is detected. 327 isn't arbitrary:
// It's MAX_SHORT/100.  That's because with the decayCoefficient = 99%, the accumulated
// value can get up to 100*(accumulation amount).  The number is used in a signed
// comparison, and it's important that the value doesn't wrap around:

#define ACCUMULATION        327

// The threshold at which motion artifacts and fade artifacts are equally problematic

#define ACCUMULATION_MINIMUM_METHOD_THRESHOLD   62


/////////////////////////////////////////////////////////////////////////////
// Processor-specific lablelling
/////////////////////////////////////////////////////////////////////////////

#undef MAINLOOP_LABEL

#if defined(IS_MMXEXT)
#define MAINLOOP_LABEL DoNext32Bytes_MMXEXT
#elif defined(IS_SSE)
#define MAINLOOP_LABEL DoNext32Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext32Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext32Bytes_MMX
#endif // processor specific inner loop jump target


/////////////////////////////////////////////////////////////////////////////
// The actual main code.  Wow.
/////////////////////////////////////////////////////////////////////////////


// Translate user supplied parameters into values useful to the plugin

#undef RESCALING_PROCEDURE_NAME
#if defined(IS_MMXEXT)
#define RESCALING_PROCEDURE_NAME    RescaleParameters_MMXEXT
#elif defined(IS_SSE)
#define RESCALING_PROCEDURE_NAME    RescaleParameters_SSE
#elif defined(IS_3DNOW)
#define RESCALING_PROCEDURE_NAME    RescaleParameters_3DNOW
#else
#define RESCALING_PROCEDURE_NAME    RescaleParameters_MMX
#endif  // processor specific routine name

void RESCALING_PROCEDURE_NAME(LONG* pDecayNumerator, LONG* pAveragingThreshold)
{
    DWORD           MinThreshold = 0;
    DWORD           MaxThreshold = 0;
    DWORD           ThisShimmer = 0;
    DWORD           LastShimmer = 0;
    DWORD           LocalDecayCoefficient = 0;
#if defined(IS_3DNOW)
    DWORD           RoundingCompensation = 1 + FIXED_POINT_DIVISOR/2;
#else
    DWORD           RoundingCompensation = 0;
#endif

    LocalDecayCoefficient = gDecayCoefficient;

    // We're using fixed point math

    *pDecayNumerator = (FIXED_POINT_DIVISOR*LocalDecayCoefficient + 50)/100;

    // Here things get a little weird.
    // I'll first define a few variables:
    //   D = decay coefficient = gDecayCoefficient/100
    //   A = accumulation constant
    //   S = accumulated shimmer necessary to trigger a response
    // gShimmerThreshold needs to be reparameterized for three reasons:
    // - If it's too high, it'll be impossible to reach the threshold, which
    //   makes it less than useful.
    // - It it's too low, there are some very predictable nasty artifacts.  Specifically:
    //    - If S < D * (maximum accumulation ~= A / (1 - D)), then it'll be possible for so
    //      much shimmering evidence to accumulate that a single field without shimmering
    //      won't be enough to prevent the shimmer compensation from triggering.  That will
    //      result in a blurred fade out (but just in the areas which had been shimmering),
    //      which looks bad.
    //    - If S < D*A + A, a single frame of motion can be taken as enough evidence to
    //      trigger shimmering.  (Remember that each word in the shimmer map corresponds with
    //      a pixel in both the even and odd fields.) This looks really bad, since this filter
    //      will happily average together pixels of wildly differing colors.  In reality this
    //      bound should be higher than that, since noise and long term motion can add to the
    //      shimmering value.
    // - The useful values of the shimmer threshold are closely tied to the value of the decay
    //   coefficient.  So if either is changed, the shimmer threshold needs to be reevaluated.
    // As it happens, slow fadeout is most important when the decay coefficient is high, while
    // misinterpretation of fast motion is most important when the decay coefficient is low.
    // The boundary occurs at 62 -- Well, really at 61.8034... (For the numerologists: that's
    // the golden ratio * 100)

    // Determining the maximum possible accumulated shimmer:
    // This is a plain geometric series, but with a little rounding error to make things
    // difficult.

    do
    {
        LastShimmer = ThisShimmer;
        ThisShimmer = (((*pDecayNumerator)*LastShimmer + RoundingCompensation) / FIXED_POINT_DIVISOR);
        ThisShimmer += ACCUMULATION;
#if defined(IS_3DNOW) || defined(IS_MMX)
        ThisShimmer &= 0xFFFFFFFE;
#endif // Lost ones bit
    } while (LastShimmer < ThisShimmer);
    MaxThreshold = LastShimmer;

    // The equations for finding the minimum threshold are a bit unwieldy so that rounding
    // errors are dealt with correctly.

    if (LocalDecayCoefficient < ACCUMULATION_MINIMUM_METHOD_THRESHOLD)
    {
        // Below (A + D*A) and we'll see lots of motion artifacts:

        MinThreshold = ((*pDecayNumerator)*ACCUMULATION + RoundingCompensation)/FIXED_POINT_DIVISOR;
#if defined(IS_3DNOW) || defined(IS_MMX)
        MinThreshold &= 0xFFFFFFFE;
#endif // Lost ones bit
        MinThreshold += ACCUMULATION;
    }
    else
    {
        // Below (D * (maximum accumulation ~= A / (1 - D))) and we'll see fade out artifacts:

        MinThreshold = ((*pDecayNumerator)*MaxThreshold + RoundingCompensation)/FIXED_POINT_DIVISOR;
#if defined(IS_3DNOW) || defined(IS_MMX)
        MinThreshold &= 0xFFFFFFFE;
#endif // Lost ones bit
    }

    // This is probably unnecessary, but just in case rounding causes something strange...

    if (MinThreshold > MaxThreshold)
    {
        MinThreshold = MaxThreshold;
    }

    // Scale the gShimmerThreshold percent onto the range of values determined above:
    // (One is subtracted from pAveragingThreshold because a ">" rather than ">=" is used for comparison)

    *pAveragingThreshold = MinThreshold + ((gShimmerThreshold)*(MaxThreshold - MinThreshold) + 50)/100 - 1;
}



// The image processing routine

#if defined(IS_MMXEXT)
long FilterTemporalComb_MMXEXT(DEINTERLACE_INFO *info)
#elif defined(IS_SSE)
long FilterTemporalComb_SSE(DEINTERLACE_INFO *info)
#elif defined(IS_3DNOW)
long FilterTemporalComb_3DNOW(DEINTERLACE_INFO *info)
#else
long FilterTemporalComb_MMX(DEINTERLACE_INFO *info)
#endif  // processor specific routine name
{
    DWORD           ThisLine = 0;
    WORD***         pppTheseFrames = NULL;
    WORD*           pSource = NULL;
    WORD*           pLastLast = NULL;
    const __int64   qwAccumulate = 0x0147014701470147;  // = quadword of 327 (= ACCUMULATION) decimal
    __int64         qwDecayCoefficient = 0;

    const LONG      Cycles = -(((int) info->LineLength/32) * 32);
    static LONG     sAveragingThreshold = 0;
    WORD*           pLast = NULL;
    WORD*           pMap = NULL;
    __int64         qwMotionThreshold = 0;
    __int64         qwAveragingThreshold = 0;

    __int64         qwShiftMask = 0xFEFFFEFFFEFFFEFF;
    DWORD           BottomLine = 0;
    static DWORD    sDecayNumerator = 0;
    static DWORD    sLastOverlayPitch = 0;
    static int      sLastFieldHeight = 0;
    static DWORD    sLastShimmerPercent = 0;
    static DWORD    sLastDecayPercent = 0;

    if ((info->OverlayPitch != sLastOverlayPitch) || (info->FieldHeight != sLastFieldHeight))
    {
        sLastOverlayPitch = info->OverlayPitch;
        sLastFieldHeight = info->FieldHeight;
        if (gpShimmerMap != NULL)
        {
            free(gpShimmerMap);
            gpShimmerMap = NULL;
        }
    }
    if (gpShimmerMap == NULL)
    {
        DWORD   index = 0;
        gpShimmerMap = malloc(info->OverlayPitch * info->FieldHeight * sizeof(short));
        if (gpShimmerMap == NULL)
        {
            return 1000;    // !! Should notify user !!
        }
        for ( ; index < (info->OverlayPitch * info->FieldHeight); ++index)
        {
            gpShimmerMap[index] = 0;
        }
    }

    if (info->IsOdd == TRUE)
    {
        pppTheseFrames = info->OddLines;
    }
    else
    {
        pppTheseFrames = info->EvenLines;
    }

    if ((pppTheseFrames[0] == NULL) || (pppTheseFrames[1] == NULL) || (pppTheseFrames[2] == NULL))
    {
        return 1000;
    }

    // Have either of the scaled parameters changed?
    // If so, we need to reparameterize again.  Reparameterization is a good idea with this filter
    // because otherwise, most parameter combinations result in useless behavior -- either too many
    // artifacts, or too little averaging.

    if (( (DWORD)gShimmerThreshold != sLastShimmerPercent) || ( (DWORD)gDecayCoefficient != sLastDecayPercent))
    {
        sLastShimmerPercent = gShimmerThreshold;
        sLastDecayPercent = gDecayCoefficient;
        RESCALING_PROCEDURE_NAME(&sDecayNumerator, &sAveragingThreshold);
    }

    // A small math trick to cut down on shifts:
    // The decay numerator has already been scaled to fixed point (divisor = 2^14).
    // In the assembly code, PMULHUW is used to multiply and then implicitly shift
    // right by 16.  So to perform the multiplication, we first need to multiply
    // the decay coefficient by 4:
    // BUT: For machines without PMULHUW, this needs to be done with PMULHW and an
    // extra shift, so only multiply here by 2:

#if defined (IS_MMXEXT) || defined(IS_SSE)
    qwDecayCoefficient = sDecayNumerator << 2;
#else
    qwDecayCoefficient = sDecayNumerator << 1;
#endif
    qwDecayCoefficient |= (qwDecayCoefficient << 48) | (qwDecayCoefficient << 32) | (qwDecayCoefficient << 16);

    qwAveragingThreshold = sAveragingThreshold;
    qwAveragingThreshold |= (qwAveragingThreshold << 48) | (qwAveragingThreshold << 32) | (qwAveragingThreshold << 16);

    qwMotionThreshold = gInPhaseLuminanceThreshold | (gInPhaseChromaThreshold << 8);
    qwMotionThreshold |= (qwMotionThreshold << 48) | (qwMotionThreshold << 32) | (qwMotionThreshold << 16);

    // Limit processing to the displayed part of the picture
    BottomLine = info->SourceRect.bottom/2;

    for (ThisLine = info->SourceRect.top; ThisLine < BottomLine; ++ThisLine)
    {
        // OverlayPitch/2 is the number of shorts per line of gpShimmerMap

        pMap = gpShimmerMap + (ThisLine * info->OverlayPitch/2);
        pSource = pppTheseFrames[0][ThisLine];
        pLast = pppTheseFrames[1][ThisLine];
        pLastLast = pppTheseFrames[2][ThisLine];

        __asm
        {
            // Cycles is negative for optimization reasons.  So the subtractions below actually add the
            // real number of cycles.  The indexing is then determined by adding the negative counter
            // (i.e., subtracting its absolute value).

            // What a confusing explantion.  I should probably have done this the sensible way, instead.

            mov ecx, Cycles

            // load pointers into normal registers

            mov eax, dword ptr[pSource]
            sub eax, ecx
            mov edi, dword ptr[pLast]
            sub edi, ecx
            mov ebx, dword ptr[pLastLast]
            sub ebx, ecx
            mov edx, dword ptr[pMap]
            sub edx, ecx

            // 32 bytes are handled per loop in order to make the preload instructions
            // a little more efficient.

align 8
MAINLOOP_LABEL:
            // Loop 1

            movq mm0, qword ptr[eax + ecx]                  // mm0 = sourcePixel
            movq mm1, qword ptr[edi + ecx]                  // mm1 = lastPixel

            // Get inPhase, outPhase deltas

            ABSOLUTE_DIFFERENCE_BYTES(mm3, mm0, mm1, mm7)   // mm3 = outPhase delta
        
            movq mm2, qword ptr[ebx + ecx]                  // mm2 = lastLastPixel

#if defined(IS_3DNOW) || defined(IS_MMXEXT)
            prefetchw [eax + ecx + PREFETCH_STRIDE]
#elif defined(IS_SSE)
            prefetchnta [eax + ecx + PREFETCH_STRIDE]
#endif

            ABSOLUTE_DIFFERENCE_BYTES(mm4, mm0, mm2, mm7)   // mm4 = inPhase delta
        
            // Make mask of "significant" shimmering
            // (where inPhase < outPhase, and low detected motion)

            pcmpgtb mm3, mm4                                // mm3 bytewise on where outPhase > inPhase
                                                            // i.e., on where shimmering
            psubusb mm4, qwMotionThreshold                  // mm4 bytewise off where no motion
            pxor mm7, mm7                                   // mm7 = 0
            pcmpeqw mm4, mm7                                // mm4 wordwise on where no motion
            pcmpeqw mm3, mm7                                // mm3 wordwise off where shimmering
            pandn mm3, mm4                                  // mm3 wordwise on where shimmering & no motion

            // Update shimmerMap where significant shimmering was detected

            movq mm4, qword ptr[edx + ecx]                  // mm4 = shimmer map at this pixel
            movq mm2, qwAccumulate                          // mm2 = amount to add if shimmering is detected
            pand mm2, mm3                                   // mm2 = amount to add
            paddusw mm4, mm2                                // mm4 = updated shimmer value

            // Check shimmerMap at pixel: if it's above the threshold,
            // average at this location to reduce dot crawl

            movq mm6, mm4                                   // mm6 = shimmer map
            pcmpgtw mm4, qwAveragingThreshold               // mm4 wordwise on where we need to average
            pand mm1, mm4                                   // mm1 = lastPixel where we need to average
            pandn mm4, mm0                                  // mm4 = srcPixel where we don't need to avg
            por mm4, mm1                                    // mm4 = appropriate mix of src and last
            AVERAGE(mm4, mm0, mm2, qwShiftMask)             // mm4 = adjusted pixel value
            movq qword ptr[eax + ecx], mm4                  // store the new value

            // Decay shimmerMap (mm6 is the prior value)

            MULTIPLY_WORDS_SHIFT_RIGHT_16(mm6, qwDecayCoefficient)
            movq qword ptr[edx + ecx], mm6


            // Loop 2

            movq mm0, qword ptr[eax + ecx + 8]              // mm0 = sourcePixel
            movq mm1, qword ptr[edi + ecx + 8]              // mm1 = lastPixel

            // Get inPhase, outPhase deltas

            ABSOLUTE_DIFFERENCE_BYTES(mm3, mm0, mm1, mm7)   // mm3 = outPhase delta
        
            movq mm2, qword ptr[ebx + ecx + 8]              // mm2 = lastLastPixel

#if defined(IS_SSE) || defined(IS_MMXEXT)
            prefetchnta [edi + ecx + PREFETCH_STRIDE]
#endif

            ABSOLUTE_DIFFERENCE_BYTES(mm4, mm0, mm2, mm7)   // mm4 = inPhase delta
        
            // Make mask of "significant" shimmering
            // (where inPhase < outPhase, and low detected motion)

            pcmpgtb mm3, mm4                                // mm3 bytewise on where outPhase > inPhase
                                                            // i.e., on where shimmering
            psubusb mm4, qwMotionThreshold                  // mm4 bytewise off where no motion
            pxor mm7, mm7                                   // mm7 = 0
            pcmpeqw mm4, mm7                                // mm4 wordwise on where no motion
            pcmpeqw mm3, mm7                                // mm3 wordwise off where shimmering
            pandn mm3, mm4                                  // mm3 wordwise on where shimmering & no motion

            // Update shimmerMap where significant shimmering was detected

            movq mm4, qword ptr[edx + ecx + 8]              // mm4 = shimmer map at this pixel
            movq mm2, qwAccumulate                          // mm2 = amount to add if shimmering is detected
            pand mm2, mm3                                   // mm2 = amount to add
            paddusw mm4, mm2                                // mm4 = updated shimmer value

            // Check shimmerMap at pixel: if it's above the threshold,
            // average at this location to reduce dot crawl

            movq mm6, mm4                                   // mm6 = shimmer map
            pcmpgtw mm4, qwAveragingThreshold               // mm4 wordwise on where we need to average
            pand mm1, mm4                                   // mm1 = lastPixel where we need to average
            pandn mm4, mm0                                  // mm4 = srcPixel where we don't need to avg
            por mm4, mm1                                    // mm4 = appropriate mix of src and last
            AVERAGE(mm4, mm0, mm2, qwShiftMask)             // mm4 = adjusted pixel value
            movq qword ptr[eax + ecx + 8], mm4              // store the new value

            // Decay shimmerMap (mm6 is the prior value)

            MULTIPLY_WORDS_SHIFT_RIGHT_16(mm6, qwDecayCoefficient)
            movq qword ptr[edx + ecx + 8], mm6


            // Loop 3

            movq mm0, qword ptr[eax + ecx + 16]             // mm0 = sourcePixel
            movq mm1, qword ptr[edi + ecx + 16]             // mm1 = lastPixel

            // Get inPhase, outPhase deltas

            ABSOLUTE_DIFFERENCE_BYTES(mm3, mm0, mm1, mm7)   // mm3 = outPhase delta
        
            movq mm2, qword ptr[ebx + ecx + 16]             // mm2 = lastLastPixel

#if defined(IS_SSE) || defined(IS_MMXEXT)
            prefetchnta [ebx + ecx + PREFETCH_STRIDE]
#endif

            ABSOLUTE_DIFFERENCE_BYTES(mm4, mm0, mm2, mm7)   // mm4 = inPhase delta
        
            // Make mask of "significant" shimmering
            // (where inPhase < outPhase, and low detected motion)

            pcmpgtb mm3, mm4                                // mm3 bytewise on where outPhase > inPhase
                                                            // i.e., on where shimmering
            psubusb mm4, qwMotionThreshold                  // mm4 bytewise off where no motion
            pxor mm7, mm7                                   // mm7 = 0
            pcmpeqw mm4, mm7                                // mm4 wordwise on where no motion
            pcmpeqw mm3, mm7                                // mm3 wordwise off where shimmering
            pandn mm3, mm4                                  // mm3 wordwise on where shimmering & no motion

            // Update shimmerMap where significant shimmering was detected

            movq mm4, qword ptr[edx + ecx + 16]             // mm4 = shimmer map at this pixel
            movq mm2, qwAccumulate                          // mm2 = amount to add if shimmering is detected
            pand mm2, mm3                                   // mm2 = amount to add
            paddusw mm4, mm2                                // mm4 = updated shimmer value

            // Check shimmerMap at pixel: if it's above the threshold,
            // average at this location to reduce dot crawl

            movq mm6, mm4                                   // mm6 = shimmer map
            pcmpgtw mm4, qwAveragingThreshold               // mm4 wordwise on where we need to average
            pand mm1, mm4                                   // mm1 = lastPixel where we need to average
            pandn mm4, mm0                                  // mm4 = srcPixel where we don't need to avg
            por mm4, mm1                                    // mm4 = appropriate mix of src and last
            AVERAGE(mm4, mm0, mm2, qwShiftMask)             // mm4 = adjusted pixel value
            movq qword ptr[eax + ecx + 16], mm4             // store the new value

            // Decay shimmerMap (mm6 is the prior value)

            MULTIPLY_WORDS_SHIFT_RIGHT_16(mm6, qwDecayCoefficient)
            movq qword ptr[edx + ecx + 16], mm6


            // Loop 4

            movq mm0, qword ptr[eax + ecx + 24]             // mm0 = sourcePixel
            movq mm1, qword ptr[edi + ecx + 24]             // mm1 = lastPixel

            // Get inPhase, outPhase deltas

            ABSOLUTE_DIFFERENCE_BYTES(mm3, mm0, mm1, mm7)   // mm3 = outPhase delta
        
            movq mm2, qword ptr[ebx + ecx + 24]             // mm2 = lastLastPixel

#if defined(IS_3DNOW) || defined(IS_MMXEXT)
            prefetchw [edx + ecx + PREFETCH_STRIDE]
#elif defined(IS_SSE)
            prefetchnta [edx + ecx + PREFETCH_STRIDE]
#endif

            ABSOLUTE_DIFFERENCE_BYTES(mm4, mm0, mm2, mm7)   // mm4 = inPhase delta
        
            // Make mask of "significant" shimmering
            // (where inPhase < outPhase, and low detected motion)

            pcmpgtb mm3, mm4                                // mm3 bytewise on where outPhase > inPhase
                                                            // i.e., on where shimmering
            psubusb mm4, qwMotionThreshold                  // mm4 bytewise off where no motion
            pxor mm7, mm7                                   // mm7 = 0
            pcmpeqw mm4, mm7                                // mm4 wordwise on where no motion
            pcmpeqw mm3, mm7                                // mm3 wordwise off where shimmering
            pandn mm3, mm4                                  // mm3 wordwise on where shimmering & no motion

            // Update shimmerMap where significant shimmering was detected

            movq mm4, qword ptr[edx + ecx + 24]             // mm4 = shimmer map at this pixel
            movq mm2, qwAccumulate                          // mm2 = amount to add if shimmering is detected
            pand mm2, mm3                                   // mm2 = amount to add
            paddusw mm4, mm2                                // mm4 = updated shimmer value

            // Check shimmerMap at pixel: if it's above the threshold,
            // average at this location to reduce dot crawl

            movq mm6, mm4                                   // mm6 = shimmer map
            pcmpgtw mm4, qwAveragingThreshold               // mm4 wordwise on where we need to average
            pand mm1, mm4                                   // mm1 = lastPixel where we need to average
            pandn mm4, mm0                                  // mm4 = srcPixel where we don't need to avg
            por mm4, mm1                                    // mm4 = appropriate mix of src and last
            AVERAGE(mm4, mm0, mm2, qwShiftMask)             // mm4 = adjusted pixel value
            movq qword ptr[eax + ecx + 24], mm4             // store the new value

            // Decay shimmerMap (mm6 is the prior value)

            MULTIPLY_WORDS_SHIFT_RIGHT_16(mm6, qwDecayCoefficient)
            movq qword ptr[edx + ecx + 24], mm6


            // Loopy stuff

            add ecx, 32
            jnz MAINLOOP_LABEL
        }                        
    }


    // need to clear up MMX registers

    _asm
    {
#if defined (IS_3DNOW)
        femms
#else
        emms
#endif
    }

    return 1000;
}