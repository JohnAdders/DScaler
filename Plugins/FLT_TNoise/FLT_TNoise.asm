/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Steven Grimm.  All rights reserved.
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

//
// This is the implementation of the noise filter described in Noise.c.
// It is broken out into this separate file because most of the logic is
// the same for the different processor variants, and doing it this way
// means we only have to maintain one copy of the code.
//

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
    #define MAINLOOP_LABEL DoNext8Bytes_SSE_PREFETCH
    #else // IS_3DNOW
    #define MAINLOOP_LABEL DoNext8Bytes_3DNow_PREFETCH
    #endif
#else // No prefetching
    #if defined(IS_SSE)
    #define MAINLOOP_LABEL DoNext8Bytes_SSE
    #elif defined(IS_3DNOW)
    #define MAINLOOP_LABEL DoNext8Bytes_3DNow
    #else
    #define MAINLOOP_LABEL DoNext8Bytes_MMX
    #endif
#endif // Main loop name

#if defined( USE_PREFETCH )
    #if defined(IS_SSE)
    long FilterTemporalNoise_SSE_PREFETCH(TDeinterlaceInfo* pInfo)
    #else // IS_3DNOW
    long FilterTemporalNoise_3DNOW_PREFETCH(TDeinterlaceInfo* pInfo)
    #endif
#else // No prefetching
    #if defined(IS_SSE)
    long FilterTemporalNoise_SSE(TDeinterlaceInfo* pInfo)
    #elif defined(IS_3DNOW)
    long FilterTemporalNoise_3DNOW(TDeinterlaceInfo* pInfo)
    #else
    long FilterTemporalNoise_MMX(TDeinterlaceInfo* pInfo)
    #endif
#endif
{
    BYTE* NewLine;
    BYTE* OldLine;
    int y;
    int Cycles;
    __int64 qwNoiseThreshold;
    const __int64 qwNoLowBitsMask   = 0xFEFEFEFEFEFEFEFE;
    const __int64 qwShiftMask       = 0xfefffefffefffeff;
    const __int64 qwLowBitsSet      = 0x0101010101010101;
    const __int64 qwChromaMask      = 0xFF00FF00FF00FF00;
    const __int64 qwLumiMask        = 0x00FF00FF00FF00FF;

    if(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK)
    {
        // if we have an interlaced source then we just want to
        // compare the last two pictures with the same polarity
        if(pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[2] == NULL)
        {
            return 1000;
        }

        NewLine = pInfo->PictureHistory[0]->pData;
        OldLine = pInfo->PictureHistory[2]->pData;
    }
    else
    {
        // if we have a progresive source then we just want to
        // compare the last two pictures
        if(pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[1] == NULL)
        {
            return 1000;
        }

        NewLine = pInfo->PictureHistory[0]->pData;
        OldLine = pInfo->PictureHistory[1]->pData;
    }

    // Need to have the current and next-to-previous fields to do the filtering.
    if (NewLine == NULL || OldLine == NULL)
    {
        return 1000;
    }

    qwNoiseThreshold = TemporalLuminanceThreshold | (TemporalChromaThreshold << 8);
    qwNoiseThreshold |= (qwNoiseThreshold << 48) | (qwNoiseThreshold << 32) | (qwNoiseThreshold << 16);
    Cycles = pInfo->LineLength / 8;

    for (y = 0; y < pInfo->FieldHeight; y++)
    {
        _asm
        {
            mov     ecx, Cycles
            mov     eax, NewLine
            mov     ebx, OldLine
            movq    mm5, qwNoiseThreshold       // mm5 = NoiseThreshold
            movq    mm6, qwShiftMask
            movq    mm7, qwNoLowBitsMask
align 8
MAINLOOP_LABEL:
            movq    mm0, qword ptr[eax]         // mm0 = NewPixel
            movq    mm1, qword ptr[ebx]         // mm1 = OldPixel
            movq    mm2, mm0                    // mm2 = NewPixel

#if defined( USE_PREFETCH )
    #if defined(IS_SSE)
            prefetchnta[eax + 64]
    #elif defined(IS_3DNOW)
            prefetchw[eax + 48]
    #endif
#endif  // prefetching
            // Now determine the weighted averages of the old and new pixel values.
            AVERAGE(mm2, mm1, mm4, mm6, mm7)    // mm6 = qwShiftMask, mm7 = qwNoLowBitsMask
            AVERAGE(mm2, mm1, mm4, mm6, mm7)    // mm6 = qwShiftMask, mm7 = qwNoLowBitsMask

#if defined( USE_PREFETCH )
    #if defined(IS_SSE)
            prefetchnta[ebx + 64]
    #endif
#endif
            // Figure out which pixels are sufficiently different from their predecessors
            // to be considered new.  There is, unfortunately, no absolute-difference
            // MMX instruction, so we OR together two unsigned saturated differences
            // (one of which will always be zero).
            movq    mm3, mm0                    // mm3 = NewPixel
            psubusb mm3, mm1                    // mm3 = max(NewPixel - OldPixel, 0)
            movq    mm4, mm1                    // mm4 = OldPixel
            psubusb mm4, mm0                    // mm4 = max(OldPixel - NewPixel, 0)
            por     mm3, mm4                    // mm3 = abs(NewPixel - OldPixel)

            // Filter out pixels whose differences are less than the threshold.
            psubusb mm3, mm5                    // mm3 = max(0, abs(NewPixel - OldPixel) - threshold)

            // Turn the filtered list into a mask.
            // Note that since pcmpgtX is a signed comparison we divide mm3 by two
            // to be sure that the MSB is not set.
            pxor    mm4, mm4                    // mm4 = 0
            paddb   mm3, qwLowBitsSet           // round upwards
            pand    mm3, mm7                    // remove low bits (mm7 = qwNoLowBitsMask)
            psrlw   mm3, 1                      // and divide by two
            pcmpgtb mm3, mm4                    // mm3 = 0xFF where abs(NewPixel - OldPixel) > threshold

            // If abs(NewPixel - OldPixel) > threshold for either the lumi or chroma value make sure
            // that both the lumi and chroma values are replaced with the new value
            movq    mm4, mm3                    // mm4 = 0xFF where abs(NewPixel - OldPixel) > threshold
            psrlw   mm3, 8                      // copy the chroma into luma
            por     mm3, mm4                    // OR with original
            psllw   mm4, 8                      // copy the lumi into chroma
            por     mm3, mm4                    // mm3 = 0xFFFF where abs(NewPixel - OldPixel) > threshold

            movq    mm4, mm3                    // mm4 = 0xFFFF where abs(NewPixel - OldPixel) > threshold
            pandn   mm4, mm2                    // mm4 = weighted avg. where abs(NewPixel - OldPixel) <= threshold
            pand    mm3, mm0                    // mm3 = NewPixel where abs(NewPixel - OldPixel) > threshold
            por     mm3, mm4                    // mm3 = result pixels

            movq    qword ptr[eax], mm3

            add     eax, 8
            add     ebx, 8
            dec     ecx
            jnz     MAINLOOP_LABEL
        }

        NewLine += pInfo->InputPitch;
        OldLine += pInfo->InputPitch;
    }

    // clear out the MMX registers ready for doing floating point
    // again
    _asm
    {
#if defined(IS_3DNOW)
        femms
#else
        emms
#endif
    }

    return 1000;
}

#undef MAINLOOP_LABEL