/////////////////////////////////////////////////////////////////////////////
// $Id: DI_VideoBob.asm,v 1.2 2001-07-13 16:13:33 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
// Based on code from Virtual Dub Plug-in by Gunnar Thalin
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

#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif

///////////////////////////////////////////////////////////////////////////////
// DeinterlaceFieldBob
//
// Deinterlaces a field with a tendency to bob rather than weave.  Best for
// high-motion scenes like sports.
//
// The algorithm for this was taken from the 
// Deinterlace - area based Vitual Dub Plug-in by
// Gunnar Thalin
///////////////////////////////////////////////////////////////////////////////
#if defined(IS_SSE)
BOOL DeinterlaceFieldBob_SSE(DEINTERLACE_INFO *info)
#elif defined(IS_3DNOW)
BOOL DeinterlaceFieldBob_3DNOW(DEINTERLACE_INFO *info)
#else
BOOL DeinterlaceFieldBob_MMX(DEINTERLACE_INFO *info)
#endif
{
    int Line;
    short* YVal1;
    short* YVal2;
    short* YVal3;
    BYTE* Dest;
    short **pOddLines = info->OddLines[0];
    short **pEvenLines = info->EvenLines[0];
    DWORD LineLength = info->LineLength;
    
    __int64 qwEdgeDetect;
    __int64 qwThreshold;
#ifdef IS_MMX
    const __int64 Mask = 0xfefefefefefefefe;
#endif
    const __int64 YMask    = 0x00ff00ff00ff00ff;

    qwEdgeDetect = EdgeDetect;
    qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
    qwThreshold = JaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);


    // copy first even line no matter what, and the first odd line if we're
    // processing an odd field.
    info->pMemcpy(info->Overlay, pEvenLines[0], LineLength);
    if (info->IsOdd)
        info->pMemcpy(info->Overlay + info->OverlayPitch, pOddLines[0], LineLength);

    for (Line = 0; Line < info->FieldHeight - 1; ++Line)
    {
        if (info->IsOdd)
        {
            YVal1 = pOddLines[Line];
            YVal2 = pEvenLines[Line + 1];
            YVal3 = pOddLines[Line + 1];
            Dest = info->Overlay + (Line * 2 + 2) * info->OverlayPitch;
        }
        else
        {
            YVal1 = pEvenLines[Line];
            YVal2 = pOddLines[Line];
            YVal3 = pEvenLines[Line + 1];
            Dest = info->Overlay + (Line * 2 + 1) * info->OverlayPitch;
        }

        // For ease of reading, the comments below assume that we're operating on an odd
        // field (i.e., that bIsOdd is true).  The exact same processing is done when we
        // operate on an even field, but the roles of the odd and even fields are reversed.
        // It's just too cumbersome to explain the algorithm in terms of "the next odd
        // line if we're doing an odd field, or the next even line if we're doing an
        // even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
        // half the time this function is called, those words' meanings will invert.

        // Copy the odd line to the overlay verbatim.
        info->pMemcpy(Dest + info->OverlayPitch, YVal3, LineLength);

        _asm
        {
            mov ecx, LineLength
            mov eax, dword ptr [YVal1]
            mov ebx, dword ptr [YVal2]
            mov edx, dword ptr [YVal3]
            mov edi, dword ptr [Dest]
            shr ecx, 3       // there are LineLength / 8 qwords

align 8
MAINLOOP_LABEL:         
            movq mm0, qword ptr[eax] 
            movq mm1, qword ptr[ebx] 
            movq mm2, qword ptr[edx]

            // get intensities in mm3 - 4
            movq mm3, mm0
            movq mm4, mm1
            movq mm5, mm2

            pand mm3, YMask
            pand mm4, YMask
            pand mm5, YMask

            // get average in mm0
#if defined(IS_SSE)
            pavgb mm0, mm2
#elif defined(IS_3DNOW)
            pavgusb mm0, mm2
#else
            pand  mm0, Mask
            pand  mm2, Mask
            psrlw mm0, 01
            psrlw mm2, 01
            paddw mm0, mm2
#endif

            // work out (O1 - E) * (O2 - E) / 2 - EdgeDetect * (O1 - O2) ^ 2 >> 12
            // result will be in mm6

            psrlw mm3, 01
            psrlw mm4, 01
            psrlw mm5, 01

            movq mm6, mm3
            psubw mm6, mm4  //mm6 = O1 - E

            movq mm7, mm5
            psubw mm7, mm4  //mm7 = O2 - E

            pmullw mm6, mm7     // mm0 = (O1 - E) * (O2 - E)

            movq mm7, mm3
            psubw mm7, mm5      // mm7 = (O1 - O2)
            pmullw mm7, mm7     // mm7 = (O1 - O2) ^ 2
            psrlw mm7, 12       // mm7 = (O1 - O2) ^ 2 >> 12
            pmullw mm7, qwEdgeDetect        // mm7  = EdgeDetect * (O1 - O2) ^ 2 >> 12

            psubw mm6, mm7      // mm6 is what we want

            pcmpgtw mm6, qwThreshold

            movq mm7, mm6

            pand mm0, mm6

            pandn mm7, mm1

            por mm7, mm0

#ifdef IS_SSE
            movntq qword ptr[edi], mm7
#else
            movq qword ptr[edi], mm7
#endif

            add eax, 8
            add ebx, 8
            add edx, 8
            add edi, 8
            dec ecx
            jne near MAINLOOP_LABEL
        }
    }

    // Copy last odd line if we're processing an even field.
    if (! info->IsOdd)
    {
        info->pMemcpy(info->Overlay + (info->FrameHeight - 1) * info->OverlayPitch,
                  pOddLines[info->FieldHeight - 1],
                  LineLength);
    }

    // clear out the MMX registers ready for doing floating point
    // again
    _asm
    {
        emms
    }

    return TRUE;
}

#undef MAINLOOP_LABEL

