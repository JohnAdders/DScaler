/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_GradualNoise.asm,v 1.8 2002-08-07 00:42:38 lindsey Exp $
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
// Revision 1.7  2002/03/11 01:45:41  lindsey
// Corrected for use with progressive source
//
// Revision 1.6  2002/02/04 01:06:03  lindsey
// Added 3DNow optimized version
//
// Revision 1.5  2002/02/02 00:57:54  lindsey
// Removed an SSE instruction from the MMX version (Thanks to Rob for catching it!)
//
// Revision 1.4  2002/02/01 23:16:29  lindsey
// Added code for MMX computers
// Removed MMXEXT version (since it didn't do anything)
//
// Revision 1.3  2001/12/31 00:02:59  lindsey
// Fixed crashing bug when pixel width not evenly divisible by 8
// Added prefetching for a substantial speed up
//
// Revision 1.2  2001/12/28 02:51:44  lindsey
// Improved assembly formatting
// Improved assembly multiplication
// Corrected the copyright
//
// Revision 1.1.1.1  2001/12/23 01:43:52  lindsey
// Added Gradual Noise Filter
//
/////////////////////////////////////////////////////////////////////////////

// Processor specific macros:

// Processor specific sum of absolute differences of unsigned bytes

#undef SUM_ABS_DIFF_BYTES
#if defined( IS_SSE )
#define SUM_ABS_DIFF_BYTES(DestMM, SourceMM, TempMM)    psadbw DestMM, SourceMM
#else
#define SUM_ABS_DIFF_BYTES(DestMM, SourceMM, TempMM)    __asm \
    { \
    __asm movq      TempMM, SourceMM \
    __asm psubusb   TempMM, DestMM \
    __asm psubusb   DestMM, SourceMM \
    __asm por       DestMM, TempMM \
    __asm movq      TempMM, DestMM \
    __asm psrlw     DestMM, 8 \
    __asm paddusb   TempMM, DestMM \
    __asm movq      DestMM, TempMM \
    __asm psrld     TempMM, 16 \
    __asm paddusb   DestMM, TempMM \
    __asm movq      TempMM, DestMM \
    __asm psrlq     DestMM, 32 \
    __asm paddusb   DestMM, TempMM \
    __asm pand      DestMM, qwLowByte \
    }
#endif

// Processor specific bytewise unsigned maximum

#undef MAX_BYTES
#if defined( IS_SSE )
#define MAX_BYTES(DestMM, SourceMM)    pmaxub DestMM, SourceMM
#else
#define MAX_BYTES(DestMM, SourceMM)    __asm \
    { \
    __asm psubusb   DestMM, SourceMM \
    __asm paddusb   DestMM, SourceMM \
    }
#endif


// This is the implementation of the noise filter described in FLT_GradualNoise.c

#if defined ( USE_PREFETCH )
    #if defined( IS_SSE )
    #define MAINLOOP_LABEL DoNext8Bytes_SSE_PREFETCH
    #else // IS_3DNOW
    #define MAINLOOP_LABEL DoNext8Bytes_3DNOW_PREFETCH
    #endif
#else // no prefetching
    #if defined( IS_SSE )
    #define MAINLOOP_LABEL DoNext8Bytes_SSE
    #elif defined( IS_3DNOW )
    #define MAINLOOP_LABEL DoNext8Bytes_3DNOW
    #else // IS_MMX
    #define MAINLOOP_LABEL DoNext8Bytes_MMX
    #endif
#endif // Mainloop names

#if defined ( USE_PREFETCH )
    #if defined( IS_SSE )
    long FilterGradualNoise_SSE_PREFETCH( TDeinterlaceInfo *pInfo )
    #else // IS_3DNOW
    long FilterGradualNoise_3DNOW_PREFETCH( TDeinterlaceInfo *pInfo )
    #endif
#else // no prefetching
    #if defined( IS_SSE )
    long FilterGradualNoise_SSE( TDeinterlaceInfo *pInfo )
    #elif defined( IS_3DNOW )
    long FilterGradualNoise_3DNOW( TDeinterlaceInfo *pInfo )
    #else // IS_MMX
    long FilterGradualNoise_MMX( TDeinterlaceInfo *pInfo )
    #endif
#endif // Main procedure names
{
    BYTE*           pSource = NULL;
    const LONG      Cycles = (pInfo->LineLength/8)*8;
    // Noise multiplier is (1/gNoiseReduction), in 0x10000 fixed point.
    // This times the measured noise gives the weight given to the new pixel color
    const __int64   qwNoiseMultiplier = (0x10000+(gNoiseReduction/2))/(gNoiseReduction);
    const __int64   qwChromaMask = 0xFF00FF00FF00FF00;
    const __int64   qwOnes = 0x0101010101010101;
    const __int64   qwWordOnes = 0x0001000100010001;
    const __int64   qwLowByte = 0x00000000000000FF;
    const __int64   qwLowWord = 0x000000000000FFFF;

    BYTE*           pLast = NULL;
    DWORD           ThisLine = 0;
    const DWORD     BottomLine = pInfo->FieldHeight;
    DWORD           LastIndex = 0;                  // Index to previous frame


    if( (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK) == 0 )
    {
        LastIndex = 1;
    }
    else    // Interlaced
    {
        LastIndex = 2;
    }


    // Need to have the current and next-to-previous fields to do the filtering.
    if( (pInfo->PictureHistory[0] == NULL) || (pInfo->PictureHistory[LastIndex] == NULL) )
    {
        return 1000;
    }

    // Warning: Dense code ahead

    pSource = pInfo->PictureHistory[0]->pData + (ThisLine * pInfo->InputPitch);
    pLast = pInfo->PictureHistory[LastIndex]->pData + (ThisLine * pInfo->InputPitch);

    for( ; ThisLine < BottomLine; ++ThisLine )
    {

        _asm 
        {
            mov     ecx, Cycles

            // Load pointers into normal registers
            mov     edi, dword ptr[pSource]
            mov     ebx, dword ptr[pLast]

align 8
MAINLOOP_LABEL:

            // Asterisks indicate register assignments which are used much later in the routine
            movq    mm0, qword ptr[edi]     // * mm0 = NewPixel
            movq    mm1, qword ptr[ebx]     // * mm1 = OldPixel

            // Get the noise adjustment multiplier

            movq    mm2, mm0                // mm2 = NewPixel
            SUM_ABS_DIFF_BYTES(mm2, mm1, mm3) // mm2 = Sum(|byte differences Old,New|)

            // The NoiseMultiplier is always less than 0x10000/2, so we can use a signed multiply:
            pmaddwd mm2, qwNoiseMultiplier    // mm2 (low Dword) = Multiplier to move toward new pixel value

#if defined ( USE_PREFETCH )
    #if defined( IS_SSE )
            prefetchnta[edi + PREFETCH_STRIDE]
    #elif defined( IS_3DNOW )
            prefetch[edi + PREFETCH_STRIDE]
    #endif
#endif // prefetch
            movq    mm3, mm2                // mm3 = same
            pxor    mm6, mm6                // mm6 = 0
            pcmpgtw mm2, mm6                // mm2 = 0x.......FFFF.... if ignoring old pixel value
            psrlq   mm2, 16                 // mm2 (low word) = FFFF if ignoring old pixel value (else 0)
            por     mm3, mm2                // mm3 = (low word) Adjusted multiplier to move toward new
#if defined( IS_SSE )
            pshufw  mm2, mm3, 0x00          // * mm2 = (wordwise) adjusted multiplier to move toward new
#else   // IS_MMX or IS_3DNOW
            pand    mm3, qwLowWord          // mm3 = same limited to low word 
            movq    mm2, mm3                // mm2 = same
            psllq   mm3, 16                 // mm3 = moved to second word
            por     mm2, mm3                // mm2 = copied to first and second words
            movq    mm3, mm2                // mm3 = same
            psllq   mm3, 32                 // mm3 = moved to third and fourth words
            por     mm2, mm3                // mm2 = low word copied to all four words
#endif

            movq    mm4, mm1                // mm4 = OldPixel
            psubusb mm4, mm0                // mm4 = bytewise max(old - new, 0)
            psubusb mm0, mm1                // mm0 = bytewise max (new - old, 0)
            por     mm4, mm0                // * mm4 = bytewise |new - old|
            pcmpeqb mm0, mm6                // * mm0 = bytewise on where old >= new
            movq    mm5, mm4                // mm5 = bytewise |new - old|
            pcmpeqb mm5, mm6                // mm5 bytewise on where |new - old| = 0
            pandn   mm5, qwOnes             // mm5 bytewise 1 where |new - old| != 0 (used for "round at least 1")

            pcmpeqw mm6, mm6                // mm6 = all on
            pcmpeqw mm6, mm2                // mm6 = all on iff multiplier = 0xFFFF (~= 1)
            pand    mm6, mm4                // mm6 = bytewise |new - old| iff multiplier = 0xFFFF
            MAX_BYTES(mm6, mm5)             // mm6 = same, corrected to a minimum of 1 where there is change

            // Multiply the change by the determined compensation factor
            movq    mm7, mm4                // mm7 = bytewise |new - old|
            movq    mm3, qwChromaMask
            pand    mm7, mm3                // mm7 = bytewise |new - old| chroma
#if defined( IS_SSE )
            pmulhuw mm7, mm2                // mm7 = amount of chroma to add/subtract from old, with remainders
            pand    mm7, mm3                // mm7 = amount of chroma to add/subtract from old
#else // IS_MMX or IS_3DNOW
            psrlq   mm7, 8                  // mm7 = bytewise |new - old| chroma in low byte of words
            movq    mm5, mm2                // mm5 = multiplier to move toward new
            psraw   mm5, 15                 // mm5 = words filled with high bit of multiplier words
            pand    mm5, mm7                // mm5 = wordwise chroma to add/subtract iff high bit of mulitplier is on
            pmulhw  mm7, mm2                // mm7 = signed product of chroma and multiplier
            // A twos compliment trick:
            paddw   mm7, mm5                // mm7 = unsigned product of chroma and multiplier
            psllq   mm7, 8                  // mm7 = amount of chroma to add/subtract from old, without remainders
#endif

#if defined ( USE_PREFETCH )
    #if defined( IS_SSE )
            prefetchnta[ebx + PREFETCH_STRIDE]
    #endif
#endif // prefetch

            pandn   mm3, mm4                // mm3 = bytewise |new - old| luma
#if defined( IS_SSE )
            pmulhuw mm3, mm2                // mm3 = amount of luma to add/subtract from old
#else // IS_MMX or IS_3DNOW
            movq    mm5, mm2                // mm5 = multiplier to move toward new
            psraw   mm5, 15                 // mm5 = words filled with high bit of multiplier words
            pand    mm5, mm3                // mm5 = wordwise chroma to add/subtract iff high bit of mulitplier is on
            pmulhw  mm3, mm2                // mm3 = signed product of chroma and multiplier
            paddw   mm3, mm5                // mm3 = unsigned product of chroma and multiplier
#endif
            por     mm7, mm3                // mm7 = amount to add/subtract from old
            MAX_BYTES(mm7, mm6)             // mm7 = corrected to to deal with the motion and round up to 1
            
            // Apply the calculated change
            movq    mm3, mm7                // mm3 = same
            pand    mm7, mm0                // mm7 = amount to subtract
            psubusb mm1, mm7                // mm1 = Partially adjusted oldpixel
            pandn   mm0, mm3                // mm0 = amount to add
            paddusb mm1, mm0                // mm1 = Adjusted pixel value


            movq qword ptr[edi], mm1

            add     edi, 8
            add     ebx, 8
            sub     ecx, 8
            jnz     MAINLOOP_LABEL
        }

        pSource += pInfo->InputPitch;
        pLast += pInfo->InputPitch;
    }

    // clear out the MMX registers ready for doing floating point
    // again
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

#undef MAINLOOP_LABEL