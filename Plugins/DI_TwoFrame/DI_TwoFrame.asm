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

///////////////////////////////////////////////////////////////////////////////
// Deinterlace the latest field, attempting to weave wherever it won't cause
// visible artifacts.
//
// The data from the most recently captured field is always copied to the overlay
// verbatim.  For the data from the previous field, the following algorithm is
// applied to each pixel.
//
// We use the following notation for the top, middle, and bottom pixels
// of concern:
//
// Field 1 | Field 2 | Field 3 | Field 4 |
//         |   T0    |         |   T1    | scanline we copied in last iteration
//   M0    |         |    M1   |         | intermediate scanline from alternate field
//         |   B0    |         |   B1    | scanline we just copied
//
// We will weave M1 into the image if any of the following is true:
//   - M1 is similar to either B1 or T1.  This indicates that no weave
//     artifacts would be visible.  The SpatialTolerance setting controls
//     how far apart the luminances can be before pixels are considered
//     non-similar.
//   - T1 and B1 and M1 are old.  In that case any weave artifact that
//     appears isn't due to fast motion, since it was there in the previous
//     frame too.  By "old" I mean similar to their counterparts in the
//     previous frame; TemporalTolerance controls the maximum squared
//     luminance difference above which a pixel is considered "new".
//
// Pixels are processed 4 at a time using MMX instructions.
//
// SQUARING NOTE:
// We square luminance differences to amplify the effects of large
// differences and to avoid dealing with negative differences.  Unfortunately,
// we can't compare the square of difference directly against a threshold,
// thanks to the lack of an MMX unsigned compare instruction.  The
// problem is that if we had two pixels with luminance 0 and 255,
// the difference squared would be 65025, which is a negative
// 16-bit signed value and would thus compare less than a threshold.
// We get around this by dividing all the luminance values by two before
// squaring them; this results in an effective maximum luminance
// difference of 127, whose square (16129) is safely comparable.


#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif


#if defined(IS_SSE)
BOOL DeinterlaceFieldTwoFrame_SSE(TDeinterlaceInfo* pInfo)
#elif defined(IS_3DNOW)
BOOL DeinterlaceFieldTwoFrame_3DNOW(TDeinterlaceInfo* pInfo)
#else
BOOL DeinterlaceFieldTwoFrame_MMX(TDeinterlaceInfo* pInfo)
#endif
{
    int Line;
    BYTE* YVal0;
    BYTE* YVal1;
    BYTE* YVal2;
    BYTE* OVal0;
    BYTE* OVal1;
    BYTE* OVal2;
    DWORD OldSI;
    DWORD OldSP;
    BYTE* Dest = pInfo->Overlay;
    DWORD Pitch = pInfo->InputPitch;
    DWORD LineLength = pInfo->LineLength;

    const __int64 YMask    = 0x00ff00ff00ff00ff;

    __int64 qwSpatialTolerance;
    __int64 qwTemporalTolerance;
    __int64 qwAllOnes = 0xffffffffffffffff;
    __int64 qwBobbedPixels;
    const __int64 Mask = 0x7f7f7f7f7f7f7f7f;

    qwSpatialTolerance = TwoFrameSpatialTolerance / 4;      // divide by 4 because of squaring behavior, see below
    qwSpatialTolerance += (qwSpatialTolerance << 48) + (qwSpatialTolerance << 32) + (qwSpatialTolerance << 16);
    qwTemporalTolerance = TwoFrameTemporalTolerance / 4;
    qwTemporalTolerance += (qwTemporalTolerance << 48) + (qwTemporalTolerance << 32) + (qwTemporalTolerance << 16);

    // copy first even line no matter what, and the first odd line if we're
    // processing an odd field.

    if(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
    {
        YVal0 = pInfo->PictureHistory[0]->pData;
        YVal1 = pInfo->PictureHistory[1]->pData + Pitch;
        YVal2 = YVal0 + Pitch;
        OVal0 = pInfo->PictureHistory[2]->pData;
        OVal1 = pInfo->PictureHistory[3]->pData + Pitch;
        OVal2 = OVal0 + Pitch;

        pInfo->pMemcpy(Dest, pInfo->PictureHistory[1]->pData, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;

        pInfo->pMemcpy(Dest, YVal0, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;
    }
    else
    {
        YVal0 = pInfo->PictureHistory[0]->pData;
        YVal1 = pInfo->PictureHistory[1]->pData;
        YVal2 = YVal0 + Pitch;
        OVal0 = pInfo->PictureHistory[2]->pData;
        OVal1 = pInfo->PictureHistory[3]->pData;
        OVal2 = OVal0 + Pitch;

        pInfo->pMemcpy(Dest, YVal0, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;
    }

    for (Line = 0; Line < pInfo->FieldHeight - 1; ++Line)
    {
        BYTE* Dest2 = Dest;
        BYTE* OVal2UseInAsm = OVal2;
        _asm
        {
            // We'll be using a couple registers that have meaning in the C code, so
            // save them.
            mov OldSI, esi
            mov OldSP, esp

            // Figure out what to do with the scanline above the one we just copied.
            // See above for a description of the algorithm.

            mov ecx, LineLength
            mov eax, dword ptr [YVal0]      // eax = T1
            mov ebx, dword ptr [YVal1]      // ebx = M1
            mov esp, dword ptr [YVal2]      // esp = B1
            mov edx, dword ptr [OVal0]      // edx = T0
            mov esi, dword ptr [OVal1]      // esi = M0
            shr ecx, 3                      // there are LineLength / 8 qwords

align 8
MAINLOOP_LABEL:

            mov edi, dword ptr [OVal2UseInAsm]      // edi = B0
            movq mm0, qword ptr[eax]        // mm0 = T1
            movq mm1, qword ptr[esp]        // mm1 = B1
            movq mm2, qword ptr[ebx]        // mm2 = M1

            // Average T1 and B1 so we can do interpolated bobbing if we bob onto T1.
            movq mm7, mm1                   // mm7 = B1

#if defined(IS_SSE)
            pavgb mm7, mm0
#elif defined(IS_3DNOW)
            pavgusb mm7, mm0
#else
            movq mm5, mm0                   // mm5 = T1
            psrlw mm7, 1                    // mm7 = B1 / 2
            pand mm7, Mask                  // mask off lower bits
            psrlw mm5, 1                    // mm5 = T1 / 2
            pand mm5, Mask                  // mask off lower bits
            paddw mm7, mm5                  // mm7 = (T1 + B1) / 2
#endif
            movq qwBobbedPixels, mm7

            // Now that we've averaged them, we no longer care about the chroma
            // values of T1 and B1 (all our comparisons are luminance-only).
            pand mm0, YMask                 // mm0 = luminance(T1)
            pand mm1, YMask                 // mm1 = luminance(B1)

            // Find out whether M1 is new.  "New" means the square of the
            // luminance difference between M1 and M0 is less than the temporal
            // tolerance.
            //
            movq mm7, mm2                   // mm7 = M1
            movq mm4, qword ptr[esi]        // mm4 = M0
            pand mm7, YMask                 // mm7 = luminance(M1)
            movq mm6, mm7                   // mm6 = luminance(M1)     used below
            pand mm4, YMask                 // mm4 = luminance(M0)
            psubsw mm7, mm4                 // mm7 = M1 - M0
            psraw mm7, 1                    // mm7 = M1 - M0 (see SQUARING NOTE above)
            pmullw mm7, mm7                 // mm7 = (M1 - M0) ^ 2
            pcmpgtw mm7, qwTemporalTolerance // mm7 = 0xffff where (M1 - M0) ^ 2 > threshold, 0x0000 otherwise

            // Find out how different T1 and M1 are.
            movq mm3, mm0                   // mm3 = T1
            psubsw mm3, mm6                 // mm3 = T1 - M1
            psraw mm3, 1                    // mm3 = T1 - M1 (see SQUARING NOTE above)
            pmullw mm3, mm3                 // mm3 = (T1 - M1) ^ 2
            pcmpgtw mm3, qwSpatialTolerance // mm3 = 0xffff where (T1 - M1) ^ 2 > threshold, 0x0000 otherwise

            // Find out how different B1 and M1 are.
            movq mm4, mm1                   // mm4 = B1
            psubsw mm4, mm6                 // mm4 = B1 - M1
            psraw mm4, 1                    // mm4 = B1 - M1 (see SQUARING NOTE above)
            pmullw mm4, mm4                 // mm4 = (B1 - M1) ^ 2
            pcmpgtw mm4, qwSpatialTolerance // mm4 = 0xffff where (B1 - M1) ^ 2 > threshold, 0x0000 otherwise

            // We care about cases where M1 is different from both T1 and B1.
            pand mm3, mm4                   // mm3 = 0xffff where M1 is different from T1 and B1, 0x0000 otherwise

            // Find out whether T1 is new.
            movq mm4, mm0                   // mm4 = T1
            movq mm5, qword ptr[edx]        // mm5 = T0
            pand mm5, YMask                 // mm5 = luminance(T0)
            psubsw mm4, mm5                 // mm4 = T1 - T0
            psraw mm4, 1                    // mm4 = T1 - T0 (see SQUARING NOTE above)
            pmullw mm4, mm4                 // mm4 = (T1 - T0) ^ 2 / 4
            pcmpgtw mm4, qwTemporalTolerance // mm4 = 0xffff where (T1 - T0) ^ 2 > threshold, 0x0000 otherwise

            // Find out whether B1 is new.
            movq mm5, mm1                   // mm5 = B1
            movq mm6, qword ptr[edi]        // mm6 = B0
            pand mm6, YMask                 // mm6 = luminance(B0)
            psubsw mm5, mm6                 // mm5 = B1 - B0
            psraw mm5, 1                    // mm5 = B1 - B0 (see SQUARING NOTE above)
            pmullw mm5, mm5                 // mm5 = (B1 - B0) ^ 2
            pcmpgtw mm5, qwTemporalTolerance // mm5 = 0xffff where (B1 - B0) ^ 2 > threshold, 0x0000 otherwise

            // We care about cases where M1 is old and either T1 or B1 is old.
            por mm4, mm5                    // mm4 = 0xffff where T1 or B1 is new
            por mm4, mm7                    // mm4 = 0xffff where T1 or B1 or M1 is new
            movq mm6, qwAllOnes             // mm6 = 0xffffffffffffffff
            pxor mm4, mm6                   // mm4 = 0xffff where T1 and B1 and M1 are old

            // Pick up the interpolated (T1+B1)/2 pixels.
            movq mm1, qwBobbedPixels        // mm1 = (T1 + B1) / 2

            // At this point:
            //  mm1 = (T1+B1)/2
            //  mm2 = M1
            //  mm3 = mask, 0xffff where M1 is different from both T1 and B1
            //  mm4 = mask, 0xffff where T1 and B1 and M1 are old
            //  mm6 = 0xffffffffffffffff
            //
            // Now figure out where we're going to weave and where we're going to bob.
            // We'll weave if all pixels are old or M1 isn't different from both its
            // neighbors.
            pxor mm3, mm6                   // mm3 = 0xffff where M1 is the same as either T1 or B1
            por mm3, mm4                    // mm3 = 0xffff where M1 and T1 and B1 are old or M1 = T1 or B1
            pand mm2, mm3                   // mm2 = woven data where T1 or B1 isn't new or they're different
            pandn mm3, mm1                  // mm3 = bobbed data where T1 or B1 is new and they're similar
            por mm3, mm2                    // mm3 = finished pixels

            // Shuffle some registers around since there aren't enough of them
            // to hold all our pointers at once.
            add edi, 8
            mov dword ptr[OVal2UseInAsm], edi
            mov edi, dword ptr[Dest2]

           // Put the pixels in place.
#ifdef IS_SSE
            movntq qword ptr[edi], mm3
#else
            movq qword ptr[edi], mm3
#endif

            // Advance to the next set of pixels.
            add eax, 8
            add ebx, 8
            add edx, 8
            add esi, 8
            add esp, 8
            add edi, 8
            mov dword ptr[Dest2], edi
            dec ecx
            jne near MAINLOOP_LABEL

            mov esi, OldSI
            mov esp, OldSP
        }

        Dest += pInfo->OverlayPitch;

        // Always use the most recent data verbatim.  By definition it's correct (it'd
        // be shown on an interlaced display) and our job is to fill in the spaces
        // between the new lines.
        pInfo->pMemcpy(Dest, YVal2, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;

        YVal0 += Pitch;
        YVal1 += Pitch;
        YVal2 += Pitch;
        OVal0 += Pitch;
        OVal1 += Pitch;
        OVal2 += Pitch;

    }

    // Copy last odd line if we're processing an even field.
    if(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN)
    {
        pInfo->pMemcpy(Dest, YVal1, pInfo->LineLength);
    }

    // clear out the MMX registers ready for doing floating point
    // again
    DO_EMMS;

    return TRUE;
}

#undef MAINLOOP_LABEL