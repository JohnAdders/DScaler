/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_GradualNoise.asm,v 1.2 2001-12-28 02:51:44 lindsey Exp $
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
// Revision 1.1.1.1  2001/12/23 01:43:52  lindsey
// Added Gradual Noise Filter
//
/////////////////////////////////////////////////////////////////////////////

// This is the implementation of the noise filter described in FLT_GradualNoise.c

#if defined( IS_SSE )
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#else // IS_MMXEXT
#define MAINLOOP_LABEL DoNext8Bytes_MMXEXT
#endif

#if defined( IS_SSE )
long FilterGradualNoise_SSE( TDeinterlaceInfo *pInfo )
#else // IS_MMXEXT
long FilterGradualNoise_MMXEXT( TDeinterlaceInfo *pInfo )
#endif
{
    BYTE*           pSource = NULL;
    DWORD           ThisLine = pInfo->SourceRect.top/2;
    const LONG      Cycles = pInfo->LineLength;
    // Noise multiplier is (1/gNoiseReduction), in 0x10000 fixed point.
    // This times the measured noise gives the weight given to the new pixel color
    const DWORD     NoiseMultiplier = (0x10000+(gNoiseReduction/2))/(gNoiseReduction);
    const __int64   qwChromaMask = 0xFF00FF00FF00FF00;
    const __int64   qwOnes = 0x0101010101010101;

    BYTE*           pLast = NULL;
    const DWORD     BottomLine = pInfo->SourceRect.bottom/2;        // Limit processing to displayed picture

    // Need to have the current and next-to-previous fields to do the filtering.
    if( (pInfo->PictureHistory[0] == NULL) || (pInfo->PictureHistory[2] == NULL) )
    {
        return 1000;
    }

    // Warning: Dense code ahead

    pSource = pInfo->PictureHistory[0]->pData + (ThisLine * pInfo->InputPitch);
    pLast = pInfo->PictureHistory[2]->pData + (ThisLine * pInfo->InputPitch);

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
            psadbw  mm2, mm1                // mm2 = Sum(|byte differences Old,New|)

            // The NoiseMultiplier is always less than 0x10000/2, so we can use a signed multiply:
            pmaddwd mm2, NoiseMultiplier    // mm2 (low Dword) = Multiplier to move toward new pixel value
            movq    mm3, mm2                // mm3 = same
            pxor    mm6, mm6                // mm6 = 0
            pcmpgtw mm2, mm6                // mm2 = 0x.......FFFF.... if ignoring old pixel value
            psrlq   mm2, 16                 // mm2 (low word) = FFFF if ignoring old pixel value (else 0)
            por     mm3, mm2                // mm3 = (low word) Adjusted multiplier to move toward new
            pshufw  mm2, mm3, 0x00          // * mm2 = (wordwise) adjusted multiplier to move toward new

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
            pmaxub  mm6, mm5                // mm6 = same, corrected to a minimum of 1 where there is change

            // Multiply the change by the determined compensation factor
            movq    mm7, mm4                // mm7 = bytewise |new - old|
            movq    mm3, qwChromaMask
            pand    mm7, mm3                // mm7 = bytewise |new - old| chroma
            pmulhuw mm7, mm2                // mm7 = amount of chroma to add/subtract from old, with remainders
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
        emms
    }

    return 1000;
}

#undef MAINLOOP_LABEL