/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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

// This might need to be tuned for different processors.
// Prefetching seems to get an extra ~30% on my 133MHz bus SDR Athlon

#define PREFETCH_STRIDE                         128


// Processor specific averaging:
// Set destMM to average of destMM and sourceMM
// Note that this is a somewhat unconventional averaging function: It rounds toward
// the first operand if it is (even and larger) or (odd and smaller).  This is faster
// and just as effective here as "round toward even."
// Explanation of the MMX version: 1 is added to the source pixel if it is odd (and != 255)
// Then half the (adjusted) source pixel (rounding down -- which is effectively the same as
// rounding the unadjusted pixel up unless source == 255) is added to half the destination
// pixel (also rounding down). This gives the same result as the much faster and less 
// complicated versions for other processors
//.Yes, shiftMask and noLowBitsMask could be the same, but this is a little easier to
// follow.

// tempMM is changed 

#undef AVERAGE
#if defined(IS_SSE)
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask, noLowBitsMask) __asm \
    { \
    __asm pand destMM, noLowBitsMask \
    __asm pavgb destMM, sourceMM \
    }
#elif defined(IS_3DNOW)
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask, noLowBitsMask) __asm \
    { \
    __asm pand destMM, noLowBitsMask \
    __asm pavgusb destMM, sourceMM \
    }
#else
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask, noLowBitsMask) __asm \
    { \
    __asm movq tempMM, noLowBitsMask \
    __asm pandn tempMM, sourceMM \
    __asm paddusb tempMM, sourceMM \
    __asm pand tempMM, shiftMask \
    __asm psrlw tempMM, 1 \
    __asm pand destMM, shiftMask \
    __asm psrlw destMM, 1 \
    __asm paddusb destMM, tempMM \
    }
#endif // processor specific averaging routine


#if defined( USE_PREFETCH )
    #if defined(IS_SSE)
    #define LOOP_LABEL DoNext8Bytes_SSE_PREFETCH
    #else // IS_3DNOW
    #define LOOP_LABEL DoNext8Bytes_3DNow_PREFETCH
    #endif
#else // no prefetching
    #if defined(IS_SSE)
    #define LOOP_LABEL DoNext8Bytes_SSE
    #elif defined(IS_3DNOW)
    #define LOOP_LABEL DoNext8Bytes_3DNow
    #else
    #define LOOP_LABEL DoNext8Bytes_MMX
    #endif
#endif // loop label name

// Hidden in the preprocessor stuff below is the actual routine

#if defined( USE_PREFETCH )
    #if defined(IS_SSE)
    long FilterSharpness_SSE_PREFETCH(TDeinterlaceInfo* pInfo)
    #else // IS_3DNOW
    long FilterSharpness_3DNOW_PREFETCH(TDeinterlaceInfo* pInfo)
    #endif
#else // no prefetching
    #if defined(IS_SSE)
    long FilterSharpness_SSE(TDeinterlaceInfo* pInfo)
    #elif defined(IS_3DNOW)
    long FilterSharpness_3DNOW(TDeinterlaceInfo* pInfo)
    #else
    long FilterSharpness_MMX(TDeinterlaceInfo* pInfo)
    #endif
#endif // main procedure name
{
    BYTE* Pixels = pInfo->PictureHistory[0]->pData;
    int y;
    __int64         qwYMask = 0x00ff00ff00ff00ff;
    __int64         qwSharpness;
    const __int64   qwShiftMask = 0xfefffefffefffeff;
    const __int64   qwNoLowBitsMask = 0xfefefefefefefefe;
    const __int64   qwRounding = 0x0080008000800080;
    int Cycles;

    // Need to have the current field to do the filtering.
    // and if we have nothing to do just return
    if (Pixels == NULL || 
        Sharpness == 0)
    {
        return 1000;
    }

    qwSharpness = Sharpness;
    qwSharpness |= qwSharpness << 48 | qwSharpness << 32 | qwSharpness << 16;
    Cycles = pInfo->LineLength / 8 - 2;

    for (y = 0; y < pInfo->FieldHeight; y++)
    {
        _asm
        {
            mov     eax, Pixels
            movq    mm1, [eax]
            add     eax, 8
            movq    mm2, [eax]
            mov     ecx, Cycles
            movq    mm5, qwYMask
            movq    mm4, qwSharpness
LOOP_LABEL:
            movq    mm0, mm1
            movq    mm1, mm2
            movq    mm2, [eax + 8]

            // do edge enhancement. 
            movq    mm7, mm1                // work copy of curr pixel val
            psrlq   mm0, 48                 // right justify 1 pixel from qword to left
            psllq   mm7, 16                 // left justify 3 pixels
            por     mm0, mm7                // and combine

            movq    mm6, mm2                // copy of right qword pixel val
            movq    mm7, mm1                // another copy of L2N current
            psllq   mm6, 48                 // left just 1 pixel from qword to right
            psrlq   mm7, 16                 // right just 3 pixels
            por     mm6, mm7                // combine
            // avg of forward and prev by 1 pixel
            AVERAGE(mm0, mm6, mm7, qwShiftMask, qwNoLowBitsMask)

            // we handle the possible plus and minus sharpness adjustments separately
            movq    mm7, mm1                // another copy of L2N
            psubusb mm7, mm0                // curr - surround
            psubusb mm0, mm1                // surround - curr
            pand    mm7, mm5                // YMask
            pand    mm0, mm5                // YMask
            pmullw  mm7, mm4                // mult by sharpness factor
            pmullw  mm0, mm4                // mult by sharpness factor
            paddusw mm7, qwRounding         // correct for rounding
            paddusw mm0, qwRounding         // correct for rounding

#if defined( USE_PREFETCH )
    #if defined(IS_SSE)
            prefetchnta[eax + PREFETCH_STRIDE]
    #elif defined(IS_3DNOW)
            prefetchw[eax + PREFETCH_STRIDE]
    #endif
#endif // prefetching
            psrlw   mm7, 8                  // now have diff*EdgeEnhAmt/256 ratio
            psrlw   mm0, 8                  // now have diff*EdgeEnhAmt/256 ratio

            paddusb mm7, mm1                // edge enhancement up
            psubusb mm7, mm0                // edge enhancement down, mm7 now our sharpened value

            movq    [eax], mm7
            add     eax, 8
            dec     ecx
            jne     near LOOP_LABEL
        }
        Pixels += pInfo->InputPitch;
    }
    _asm 
    {
        emms
    }
    return 1000;
}

#undef LOOP_LABEL