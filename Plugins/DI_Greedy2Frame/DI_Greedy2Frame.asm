/////////////////////////////////////////////////////////////////////////////
// DI_Greedy2Frame.asm
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock, Tom Barry, Steve Grimm  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////

// This is the implementation of the Greedy 2-frame deinterlace algorithm described in
// DI_Greedy2Frame.c.  It's in a separate file so we can compile variants for different
// CPU types; most of the code is the same in the different variants.

#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif

///////////////////////////////////////////////////////////////////////////////
// Field 1 | Field 2 | Field 3 | Field 4 |
//   T0    |         |    T1   |         | 
//         |   M0    |         |    M1   | 
//   B0    |         |    B1   |         | 
//


// debugging feature
// output the value of mm4 at this point which is pink where we will weave
// and green were we are going to bob
// uncomment next line to see this
//#define CHECK_BOBWEAVE

#if defined(IS_SSE)
BOOL DeinterlaceGreedy2Frame_SSE(DEINTERLACE_INFO *info)
#elif defined(IS_3DNOW)
BOOL DeinterlaceGreedy2Frame_3DNOW(DEINTERLACE_INFO *info)
#else
BOOL DeinterlaceGreedy2Frame_MMX(DEINTERLACE_INFO *info)
#endif
{
	int Line;
	short* M1;
	short* M0;
	short* T0;
	short* T1;
	short* B1;
	short* B0;
	DWORD OldSI;
	DWORD OldSP;
	BYTE* Dest;
	BYTE* Dest2;
	DWORD LineLength = info->LineLength;

	const __int64 YMask    = 0x00ff00ff00ff00ff;

	__int64 qwGreedyTwoFrameThreshold;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;
	const __int64 DwordOne = 0x0000000100000001;	
	const __int64 DwordTwo = 0x0000000200000002;	

	if (info->OddLines[0] == NULL || info->OddLines[1] == NULL ||
		info->EvenLines[0] == NULL || info->EvenLines[1] == NULL)
	{
		return FALSE;
	}

	qwGreedyTwoFrameThreshold = GreedyTwoFrameThreshold;
	qwGreedyTwoFrameThreshold += (GreedyTwoFrameThreshold2 << 8);
	qwGreedyTwoFrameThreshold += (qwGreedyTwoFrameThreshold << 48) +
								(qwGreedyTwoFrameThreshold << 32) + 
								(qwGreedyTwoFrameThreshold << 16);

	Dest = info->Overlay;
	// copy first even line if we're doing an even line
	if(!info->IsOdd)
	{
		info->pMemcpy(Dest, info->EvenLines[0][0], info->LineLength);
		Dest += info->OverlayPitch;
	}

	for (Line = 0; Line < info->FieldHeight - 1; ++Line)
	{
		if (info->IsOdd)
		{
			M1 = info->OddLines[0][Line];
			T1 = info->EvenLines[0][Line];
			B1 = info->EvenLines[0][Line + 1];
			M0 = info->OddLines[1][Line];
			T0 = info->EvenLines[1][Line];
			B0 = info->EvenLines[1][Line + 1];
		}
		else
		{
			M1 = info->EvenLines[0][Line + 1];
			T1 = info->OddLines[0][Line];
			B1 = info->OddLines[0][Line + 1];
			M0 = info->EvenLines[1][Line + 1];
			T0 = info->OddLines[1][Line];
			B0 = info->OddLines[1][Line + 1];
		}

		// Always use the most recent data verbatim.  By definition it's correct (it'd
		// be shown on an interlaced display) and our job is to fill in the spaces
		// between the new lines.
		info->pMemcpy(Dest, T1, info->LineLength);
		Dest += info->OverlayPitch;
		Dest2 = Dest;

	    _asm
	    {
		    // We'll be using a couple registers that have meaning in the C code, so
		    // save them.
		    mov OldSI, esi
		    mov OldSP, esp

		    // Figure out what to do with the scanline above the one we just copied.
		    // See above for a description of the algorithm.

		    mov ecx, LineLength
		    mov eax, dword ptr [T1]		
		    mov ebx, dword ptr [M1]		
		    mov edx, dword ptr [B1]		
		    mov esi, dword ptr [M0]		
		    mov esp, dword ptr [T0]
		    shr ecx, 3						// there are LineLength / 8 qwords
		    movq    mm6, Mask

align 8
MAINLOOP_LABEL:

		    mov edi, dword ptr [B0]
		    movq	mm1, qword ptr[eax]		// T1
		    movq	mm0, qword ptr[ebx]		// M1
		    movq	mm3, qword ptr[edx]		// B1
		    movq	mm2, qword ptr[esi]     // M0

		    // Average T1 and B1 so we can do interpolated bobbing if we bob onto T1.
		    movq mm7, mm3					// mm7 = B1

#if defined(IS_SSE)
		    pavgb mm7, mm1
#elif defined(IS_3DNOW)
		    pavgusb mm7, mm1
#else
		    movq mm5, mm1					// mm5 = T1
		    psrlw mm7, 1					// mm7 = B1 / 2
		    pand mm7, mm6					// mask off lower bits
		    psrlw mm5, 1					// mm5 = T1 / 2
		    pand mm5, mm6					// mask off lower bits
		    paddw mm7, mm5					// mm7 = (T1 + B1) / 2
#endif

            // calculate |M1-M0| put result in mm4 need to keep mm0 intact
			// if we have a good processor then make mm0 the average of M1 and M0
			// which should make weave look better when there is small amounts of
			// movement
#if defined(IS_SSE)
		    movq	mm4, mm0
		    movq	mm5, mm2
		    psubusb mm4, mm2
		    psubusb mm5, mm0
		    por		mm4, mm5
		    psrlw	mm4, 1
		    pavgb mm0, mm2
		    pand	mm4, mm6
#elif defined(IS_3DNOW)
		    movq	mm4, mm0
		    movq	mm5, mm2
		    psubusb mm4, mm2
		    psubusb mm5, mm0
		    por		mm4, mm5
		    psrlw	mm4, 1
		    pavgusb mm0, mm2
		    pand	mm4, mm6
#else
		    movq	mm4, mm0
		    psubusb mm4, mm2
		    psubusb mm2, mm0
		    por		mm4, mm2
		    psrlw	mm4, 1
		    pand	mm4, mm6
#endif


            // if |M1-M0| > Threshold we want dword worth of twos
		    pcmpgtb mm4, qwGreedyTwoFrameThreshold
		    pand	mm4, Mask				// get rid of any sign bit
		    pcmpgtd mm4, DwordOne			// do we want to bob
		    pandn   mm4, DwordTwo

		    movq	mm2, qword ptr[esp]		// mm2 = T0

            // calculate |T1-T0| put result in mm5
		    movq	mm5, mm2
		    psubusb mm5, mm1
		    psubusb mm1, mm2
		    por		mm5, mm1
		    psrlw	mm5, 1
		    pand	mm5, mm6

            // if |T1-T0| > Threshold we want dword worth of ones
		    pcmpgtb mm5, qwGreedyTwoFrameThreshold
		    pand	mm5, mm6				// get rid of any sign bit
		    pcmpgtd mm5, DwordOne			
		    pandn   mm5, DwordOne
		    paddd mm4, mm5

		    movq	mm2, qword ptr[edi]     // B0

            // calculate |B1-B0| put result in mm5
		    movq	mm5, mm2
		    psubusb mm5, mm3
		    psubusb mm3, mm2
		    por		mm5, mm3
		    psrlw	mm5, 1
		    pand	mm5, mm6

            // if |B1-B0| > Threshold we want dword worth of ones
		    pcmpgtb mm5, qwGreedyTwoFrameThreshold
		    pand	mm5, mm6				// get rid of any sign bit
		    pcmpgtd mm5, DwordOne
		    pandn   mm5, DwordOne
		    paddd mm4, mm5

		    // Get the dest pointer.
		    add edi, 8
		    mov dword ptr[B0], edi
		    mov edi, dword ptr[Dest2]

		    pcmpgtd mm4, DwordTwo

// debugging feature
// output the value of mm4 at this point which is pink where we will weave
// and green were we are going to bob
#ifdef CHECK_BOBWEAVE
#ifdef IS_SSE
		    movntq qword ptr[edi], mm4
#else
		    movq qword ptr[edi], mm4
#endif
#else

		    movq mm5, mm4
             // mm4 now is 1 where we want to weave and 0 where we want to bob
		    pand	mm4, mm0				
		    pandn	mm5, mm7				
		    por		mm4, mm5				
#ifdef IS_SSE
		    movntq qword ptr[edi], mm4
#else
		    movq qword ptr[edi], mm4
#endif
#endif
		    // Advance to the next set of pixels.
		    add edi, 8
		    add eax, 8
		    add ebx, 8
		    add edx, 8
		    mov dword ptr[Dest2], edi
		    add esi, 8
		    add esp, 8
		    dec ecx
		    jne near MAINLOOP_LABEL

		    mov esi, OldSI
		    mov esp, OldSP
	    }
		Dest += info->OverlayPitch;
	}

	// Copy last odd line if we're processing an even field.
	if(info->IsOdd)
	{
		info->pMemcpy(Dest,
				  info->EvenLines[info->FieldHeight - 1],
				  info->LineLength);
		Dest += info->OverlayPitch;
		info->pMemcpy(Dest,
				  info->OddLines[info->FieldHeight - 1],
				  info->LineLength);
	}
	else
	{
		info->pMemcpy(Dest,
				  info->OddLines[info->FieldHeight - 1],
				  info->LineLength);
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