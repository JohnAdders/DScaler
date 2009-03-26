/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Rob Muller.  All rights reserved.
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

#if defined(IS_SSE)
#define LOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define LOOP_LABEL DoNext8Bytes_3DNow
#else
#define LOOP_LABEL DoNext8Bytes_MMX
#endif

// note that the pixels are stored as         Y0 C0a Y1 C0b Y2 C1a Y3 C1b (4 pixels)
// a swapped block of pixels looks like this: Y3 C1a Y2 C1b Y1 C0a Y0 C0b
#define SWAPPIXELS(pixelMM, temp1MM, temp2MM, CMask, MaskLumi02, MaskLumi13) __asm \
    { \
    __asm movq    temp1MM, pixelMM          /* make a copy */ \
    __asm psllq   pixelMM, 32               /* shift pixel 2 and 3 two pixels to the left */ \
    __asm psrlq   temp1MM, 32               /* shift pixel 0 and 1 two pixels to the right */ \
    __asm por     pixelMM, temp1MM          /* combine the pixels again */ \
    __asm movq    temp2MM, pixelMM          /* make a copy */ \
    __asm movq    temp1MM, pixelMM          /* make another copy */ \
    __asm pand    temp2MM, CMask            /* the chroma part of the pixels is ready now */ \
    __asm psllq   pixelMM, 16               /* shift 1 pixel to the left */ \
    __asm psrlq   temp1MM, 16               /* shift 1 pixel to the right */ \
    __asm pand    pixelMM, MaskLumi02       /* remove one pixel and all chroma*/ \
    __asm pand    temp1MM, MaskLumi13       /* remove one pixel and all chroma*/ \
    __asm por     pixelMM, temp2MM          /* combine with the chroma */ \
    __asm por     pixelMM, temp1MM          /* combine the pixels */ \
    }

#if defined(IS_SSE)
long FilterMirror_SSE(TDeinterlaceInfo* pInfo)
#elif defined(IS_3DNOW)
long FilterMirror_3DNOW(TDeinterlaceInfo* pInfo)
#else
long FilterMirror_MMX(TDeinterlaceInfo* pInfo)
#endif
{
    int y = 0;
    int Cycles = 0;
    const __int64   qwMaskLumi02    = 0x00FF000000FF0000;
    const __int64   qwMaskLumi13    = 0x000000FF000000FF;
    const __int64   qwCMask         = 0xFF00FF00FF00FF00;
    BYTE* Pixels = NULL;

    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[0]->pData == NULL)
    {
        return 1000;
    }

    Pixels = pInfo->PictureHistory[0]->pData;
    Cycles = pInfo->LineLength / 8 / 2;         // half line in 8 byte chunks

    for (y = 0; y < pInfo->FieldHeight; y++)
    {
        _asm
        {
            movq    mm5, qwCMask
            movq    mm6, qwMaskLumi02
            movq    mm7, qwMaskLumi13

            mov     eax, Pixels                 // eax point to the first pixel of the line
            mov     edx, Cycles
            shl     edx, 4
            sub     edx, 8
            add     edx, eax                    // edx now points to the last pixel of the line
            mov     ecx, Cycles
LOOP_LABEL:
            movq    mm0, [eax]                  // get a pixelblock from the first half of the line
            movq    mm3, [edx]                  // get the corresponding block from the second half

                                                // swap the pixels from the first block
            SWAPPIXELS(mm0, mm1, mm2, mm5, mm6, mm7)

            movq    [edx], mm0                  // and swap the block of pixels

                                                // swap the pixels from the second block
            SWAPPIXELS(mm3, mm1, mm2, mm5, mm6, mm7)

            movq    [eax], mm3                  // and swap the block of pixels

            add     eax, 8                      // next pixel block
            sub     edx, 8                      // previous pixel block
            dec     ecx
            jne     LOOP_LABEL
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
