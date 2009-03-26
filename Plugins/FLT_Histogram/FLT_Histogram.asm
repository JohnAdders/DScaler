/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Lindsey Dubb.  All rights reserved.
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

// tempMM is changed

#undef AVERAGE
#if defined(IS_SSE)
#define AVERAGE(destMM, sourceMM, tempMM, noLowBitsMask) __asm \
    { \
    __asm pand destMM, noLowBitsMask \
    __asm pavgb destMM, sourceMM \
    }
#elif defined(IS_3DNOW)
#define AVERAGE(destMM, sourceMM, tempMM, noLowBitsMask) __asm \
    { \
    __asm pand destMM, noLowBitsMask \
    __asm pavgusb destMM, sourceMM \
    }
#else
#define AVERAGE(destMM, sourceMM, tempMM, noLowBitsMask) __asm \
    { \
    __asm movq tempMM, noLowBitsMask \
    __asm pandn tempMM, sourceMM \
    __asm paddusb tempMM, sourceMM \
    __asm pand tempMM, noLowBitsMask \
    __asm psrlw tempMM, 1 \
    __asm pand destMM, noLowBitsMask \
    __asm psrlw destMM, 1 \
    __asm paddusb destMM, tempMM \
    }
#endif // processor specific averaging routine


#undef MAINLOOP_LABEL
#if defined( IS_SSE )
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined( IS_3DNOW )
#define MAINLOOP_LABEL DoNext8Bytes_3DNOW
#else // IS_MMX
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif


#if defined( IS_SSE )
LONG GatherHistogram_SSE( TDeinterlaceInfo* pInfo )
#elif defined( IS_3DNOW )
LONG GatherHistogram_3DNOW( TDeinterlaceInfo* pInfo )
#else   // IS_MMX
LONG GatherHistogram_MMX( TDeinterlaceInfo* pInfo )
#endif
{
    const __int64   qwNoLowBits = 0xFEFEFEFEFEFEFEFE;
    const DWORD     YByte = 0x000000FF;
    const DWORD     HighYByte = 0x00FF0000;
    const DWORD     UByte = 0x0000FF00;
    const DWORD     VByte = 0xFF000000;
    DWORD*          pLocalYHistogram = NULL;
    DWORD*          pLocalUHistogram = NULL;
    DWORD*          pLocalVHistogram = NULL;
    BYTE*           Pixels = NULL;
    BYTE*           UpPixels = NULL;
    DWORD           Index = 0;
    DWORD           OldSI = 0;
    const DWORD     Cycles = pInfo->LineLength / 8;


    // If we're not using a comb filter, we just average the line with itself.  This seems comically
    // inefficient, but appears to be just as fast.

    UpPixels = pInfo->PictureHistory[0]->pData;

    if( gCombMode == COMB_MODE_NTSC )
    {
        Pixels = UpPixels + pInfo->InputPitch;
    }
    else if( gCombMode == COMB_MODE_PAL )
    {
        Pixels = UpPixels + 2*(pInfo->InputPitch);
    }
    else
    {
        Pixels = UpPixels;
    }

    pLocalYHistogram = gpYHistogram;
    pLocalUHistogram = gpUHistogram;
    pLocalVHistogram = gpVHistogram;

    for (Index = 2; Index < (DWORD) pInfo->FieldHeight; ++Index)
    {
        _asm
        {
            mov     OldSI, esi
            mov     ecx, Cycles
            mov     edi, dword ptr[Pixels]
            mov     esi, dword ptr[UpPixels]

            MAINLOOP_LABEL:

                movq    mm0, qword ptr[edi]                 // mm0 = four pixels
                movq    mm1, qword ptr[esi]                 // mm1 = four pixels from previous line

                AVERAGE(mm0, mm1, mm2, qwNoLowBits)         // * mm0 = vertical average of pixels

                movd    edx, mm0                            // * edx = low two pixels

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalYHistogram]    // ebx points to Y histogram
                and     eax, YByte                          // eax = Y value of low pixel
                shl     eax, 2                              // eax = sizeof(DWORD)*(Y value)
                add     ebx, eax                            // ebx = pointer to corresponding Y histogram value
                inc     dword ptr[ebx]                      // increment Y histogram

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalYHistogram]    // ebx points to Y histogram
                and     eax, HighYByte                      // eax = Y value of high pixel
                shr     eax, 14                             // eax = sizeof(DWORD)*(Y value)
                add     ebx, eax                            // ebx = pointer to corresponding Y histogram value
                inc     dword ptr[ebx]                      // increment Y histogram

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalVHistogram]    // ebx points to V histogram
                and     eax, VByte                          // eax = V value of two pixels
                shr     eax, 22                             // eax = sizeof(DWORD)*(V value)
                add     ebx, eax                            // ebx = pointer to corresponding V histogram value
                inc     dword ptr[ebx]                      // increment V histogram

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalUHistogram]    // ebx points to U histogram
                and     eax, UByte                          // eax = U value of two pixels
                shr     eax, 6                              // eax = sizeof(DWORD)*(U value)
                add     ebx, eax                            // ebx = pointer to corresponding U histogram value
                inc     dword ptr[ebx]                      // increment U histogram

                psrlq   mm0, 32                             // mm0 = high two pixels in low dword
#if defined( IS_SSE )
                prefetchnta[edi + PREFETCH_STRIDE]
#elif defined( IS_3DNOW )
                prefetch[edi + PREFETCH_STRIDE]
#endif
                movd    edx, mm0                            // * edx = high two pixels

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalYHistogram]    // ebx points to Y histogram
                and     eax, YByte                          // eax = Y value of low pixel
                shl     eax, 2                              // eax = sizeof(DWORD)*(Y value)
                add     ebx, eax                            // ebx = pointer to corresponding Y histogram value
                inc     dword ptr[ebx]                      // increment Y histogram

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalYHistogram]    // ebx points to Y histogram
                and     eax, HighYByte                      // eax = Y value of high pixel
                shr     eax, 14                             // eax = sizeof(DWORD)*(Y value)
                add     ebx, eax                            // ebx = pointer to corresponding Y histogram value
                inc     dword ptr[ebx]                      // increment Y histogram

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalVHistogram]    // ebx points to V histogram
                and     eax, VByte                          // eax = V value of two pixels
                shr     eax, 22                             // eax = sizeof(DWORD)*(V value)
                add     ebx, eax                            // ebx = pointer to corresponding V histogram value
                inc     dword ptr[ebx]                      // increment V histogram

                mov     eax, edx                            // eax = pixels
                mov     ebx, dword ptr[pLocalUHistogram]    // ebx points to U histogram
                and     eax, UByte                          // eax = U value of two pixels
                shr     eax, 6                              // eax = sizeof(DWORD)*(U value)
                add     ebx, eax                            // ebx = pointer to corresponding U histogram value
                inc     dword ptr[ebx]                      // increment U histogram

                add     edi, 8
                add     esi, 8

            dec     ecx
            jnz     MAINLOOP_LABEL

            mov     esi, OldSI
       }
        Pixels += pInfo->InputPitch;
        UpPixels += pInfo->InputPitch;
    }
    _asm
    {
#if defined( IS_3DNOW )
        femms
#else
        emms
#endif
    }

    return 1000;
}