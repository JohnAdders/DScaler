/////////////////////////////////////////////////////////////////////////////
// $Id: DI_GreedyHF.c,v 1.5 2001-07-30 18:18:59 trbarry Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 01 Feb 2001   Tom Barry		       New Greedy (High Motion)Deinterlace method
//
// 29 Jul 2001   Tom Barry		       Move CPU dependent code to DI_GreedyHF.asm
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2001/07/30 17:56:26  trbarry
// Add Greedy High Motion MMX, K6-II, K6-III, and Celeron support.
// Tweak defaults.
//
// Revision 1.3  2001/07/28 18:47:24  trbarry
// Fix Sharpness with Median Filter
// Increase Sharpness default to make obvious
// Adjust deinterlace defaults for less jitter
//
// Revision 1.2  2001/07/25 12:04:31  adcockj
// Moved Control stuff into DS_Control.h
// Added $Id and $Log to comment blocks as per standards
//
/////////////////////////////////////////////////////////////////////////////

// This is the first version of the Greedy High Motion Deinterlace method I wrote (and kept). 
// It doesn't have many of the fancier options but I left it in because it's faster. It runs with 
// a field delay of 1 in a single pass with no call needed to UpdateFieldStore. It will be called
// if no special options are needed. The logic is somewhat different than the other rtns.  TRB 7/2001
//
// This version has been modified to be compatible with other DScaler functions such as Auto Pulldown.
// It will now automatically be call if none of the UI check boxes are check and also if we are not
// running on an SSE capable machine (Athlon, Duron, P-III, fast Celeron).


#include "windows.h"
#include "DS_Deinterlace.h"
#include "DI_GreedyHM.h"

#undef SSE_TYPE
#define IS_SSE
#define SSE_TYPE SSE
#define FUNCT_NAME DI_GreedyHF_SSEn
//>>>>>>>>>>>>>>>>
/////////////////////////////////////////////////////////////////////////////
// $Id: DI_GreedyHF.c,v 1.5 2001-07-30 18:18:59 trbarry Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 01 Feb 2001   Tom Barry		       New Greedy (High Motion)Deinterlace method
// 29 Jul 2001   Tom Barry             Add 3DNOW, MMX support, create .asm mem 
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2001/07/28 18:47:24  trbarry
// Fix Sharpness with Median Filter
// Increase Sharpness default to make obvious
// Adjust deinterlace defaults for less jitter
//
// Revision 1.2  2001/07/25 12:04:31  adcockj
// Moved Control stuff into DS_Control.h
// Added $Id and $Log to comment blocks as per standards
//
/////////////////////////////////////////////////////////////////////////////

// FUNCT_NAME must be defined before include
BOOL FUNCT_NAME()
{
	#include "DI_GreedyHM2.h"
    int Line;
	int	LoopCtr;
	short* L1;					// ptr to Line1, of 3
	short* L2;					// ptr to Line2, the weave line
	short* L3;					// ptr to Line3
	short* L2P;					// ptr to prev Line2
	BYTE* Dest;
	__int64 QW256B;
	__int64 LastAvg=0;			//interp value from left qword
	__int64 i;

    i = 0xffffffff - 256;
    QW256B =  i << 48 |  i << 32 | i << 16 | i;  // save a couple instr on PMINSW instruct.

	if (pOddLines == NULL || pEvenLines == NULL || pPrevLines == NULL)
		return FALSE;

	// copy first even line no matter what, and the first odd line if we're
	// processing an EVEN field. (note diff from other deint rtns.)
	pMemcpy(lpCurOverlay, pEvenLines[0], LineLength);	// DL0
	if (!InfoIsOdd)
		pMemcpy(lpCurOverlay + OverlayPitch, pOddLines[0], LineLength);  // DL1
	for (Line = 0; Line < (FieldHeight - 1); ++Line)
	{
		LoopCtr = LineLength / 8;				// there are LineLength / 8 qwords per line

		if (InfoIsOdd)
		{
			L1 = pEvenLines[Line];		
			L2 = pOddLines[Line];	
			L3 = pEvenLines[Line + 1];	
			L2P = pPrevLines[Line];			// prev Odd lines
			Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;	// DL1
		}
		else
		{
			L1 = pOddLines[Line] ;		
			L2 = pEvenLines[Line + 1];		
			L3 = pOddLines[Line + 1];   
			L2P = pPrevLines[Line + 1];			// prev even lines
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;	// DL2
		}
		pMemcpy(Dest + OverlayPitch, L3, LineLength);

// For ease of reading, the comments below assume that we're operating on an odd
// field (i.e., that InfoIsOdd is true).  Assume the obvious for even lines..

		_asm
		{
			mov word ptr [LastAvg+6], 0       // init easy way
			mov eax, dword ptr [L1]		
            lea ebx, [eax+8]                // next qword needed by DJR
			mov ecx, dword ptr [L3]		
            sub ecx, eax                    // carry L3 addr as an offset
            mov edx, dword ptr [L2P]		
            mov esi, dword ptr [L2]		
			mov edi, dword ptr [Dest]       // DL1 if Odd or DL2 if Even 	

align 8
DoNext8Bytes:			

			movq	mm0, qword ptr[esi]	    // L2 - the newest weave pixel value 
			movq	mm1, qword ptr[eax]		// L1 - the top pixel
			movq	mm2, qword ptr[edx]		// L2P - the prev weave pixel 
			movq	mm3, qword ptr[eax+ecx] // L3, next odd row
			movq	mm6, mm1				// L1 - get simple single pixel interp
//			pavgb   mm6, mm3                // use macro below
            V_PAVGB (mm6, mm3, mm4, ShiftMask) 
//>>			pand    mm6, YMask              // keep only luma

// DJR - Diagonal Jaggie Reduction
// In the event that we are going to use an average (Bob) pixel we do not want a jagged
// stair step effect.  To combat this we avg in the 2 horizontally adjacen pixels into the
// interpolated Bob mix. This will do horizontal smoothing for only the Bob'd pixels.

			movq    mm4, LastAvg			// the bob value from prev qword in row
			movq	LastAvg, mm6			// save for next pass
			psrlq   mm4, 48					// right justify 1 pixel
			movq	mm7, mm6				// copy of simple bob pixel
			psllq   mm7, 16                 // left justify 3 pixels
			por     mm4, mm7				// and combine
			
			movq	mm5, qword ptr[ebx] // next horiz qword from L1
//			pavgb   mm5, qword ptr[eax+FSCOLSIZE+FSROWSIZE] // next horiz qword from L3, use macro below
//>>>            V_PAVGB (mm5, qword ptr[eax+FSCOLSIZE+FSROWSIZE], mm7, ShiftMask) 
			psllq	mm5, 48					// left just 1 pixel
			movq	mm7, mm6                // another copy of simple bob pixel
			psrlq   mm7, 16					// right just 3 pixels
			por		mm5, mm7				// combine
//			pavgb	mm4, mm5				// avg of forward and prev by 1 pixel, use macro
            V_PAVGB (mm4, mm5, mm5, ShiftMask)   // mm5 gets modified if MMX
			pand    mm4, YMask              // keep only luma
//			pavgb	mm6, mm4				// avg of center and surround interp vals, use macro
            V_PAVGB (mm6, mm4, mm7, ShiftMask)  

// Don't do any more averaging than needed for mmx. It hurts performance and causes rounding errors.
#ifndef IS_MMX
//          pavgb	mm4, mm6				// 1/4 center, 3/4 adjacent
            V_PAVGB (mm4, mm6, mm7, ShiftMask)  
//    		pavgb	mm6, mm4				// 3/8 center, 5/8 adjacent
            V_PAVGB (mm6, mm4, mm7, ShiftMask)  
#endif

// get abs value of possible L2 comb
			movq    mm4, mm6				// work copy of interp val
			movq	mm7, mm2				// L2
			psubusb mm7, mm4				// L2 - avg
			movq	mm5, mm4				// avg
			psubusb mm5, mm2				// avg - L2
			por		mm5, mm7				// abs(avg-L2)

// get abs value of possible L2P comb
			movq	mm7, mm0				// L2P
			psubusb mm7, mm4				// L2P - avg
			psubusb	mm4, mm0				// avg - L2P
			por		mm4, mm7				// abs(avg-L2P)

// use L2 or L2P depending upon which makes smaller comb
			psubusb mm4, mm5				// see if it goes to zero
			psubusb mm5, mm5				// 0
			pcmpeqb mm4, mm5				// if (mm4=0) then FF else 0
			pcmpeqb mm5, mm4				// opposite of mm4

// if Comb(L2P) <= Comb(L2) then mm4=ff, mm5=0 else mm4=0, mm5 = 55
			pand	mm5, mm2				// use L2 if mm5 == ff, else 0
			pand	mm4, mm0				// use L2P if mm4 = ff, else 0
			por		mm4, mm5				// may the best win
			pand    mm4, YMask              // keep only luma

// Inventory: at this point we have the following values:
// mm0 = L2P (or L2)
// mm1 = L1
// mm2 = L2 (or L2P)
// mm3 = L3
// mm4 = the best of L2,L2P weave pixel, base upon comb (luma only)
// mm6 = the avg interpolated value, if we need to use it

// Let's measure movement, as how much the weave pixel has changed
			movq	mm7, mm2
			psubusb mm2, mm0
			psubusb mm0, mm7
			por		mm0, mm2				// abs value of change, used later

// Blend weave pixel with bob pixel, depending on motion val in mm0			
			psubusb mm0, MotionThresholdW   // test Threshold, clear chroma
			pmullw  mm0, MotionSenseW  // mul by user factor, keep low 16 bits
			movq    mm7, QW256
#ifdef IS_SSE
			pminsw  mm0, mm7				// max = 256  
#else
            paddusw mm0, QW256B              // add, may sat at fff..
            psubusw mm0, QW256B              // now = Min(L1,256)
#endif
			psubusw mm7, mm0				// so the 2 sum to 256, weighted avg
			pmullw  mm4, mm7				// use more weave for less motion
			pmullw  mm6, mm0				// use more bob for large motion
			paddusw mm4, mm6				// combine
			psrlw   mm4, 8					// div by 256 to get weighted avg	

// Now lets clip our chosen value to be not outside of the range
// of the high/low range L1-L3 by more than MaxComb.
// This allows some comb but limits the damages and also allows more
// detail than a boring oversmoothed clip.
			movq	mm2, mm1				// copy L1
//			pmaxub	mm2, mm3                // use macro
			V_PMAXUB (mm2, mm3)             // now = Max(L1,L3)
			movq	mm0, mm1				// copy L1
//			pminub	mm0, mm3				// now = Min(L1,L3), use macro
            V_PMINUB (mm0, mm3, mm6)
// allow the value to be above the high or below the low by amt of MaxComb
			psubusb mm0, MaxCombW			// lower min by diff
			paddusb	mm2, MaxCombW			// increase max by diff
//			pmaxub	mm4, mm0				// now = Max(best,Min(L1,L3) use macro
            V_PMAXUB (mm4, mm0)
//			pminub	mm2, mm4 				// now = Min( Max(best, Min(L1,L3), L2 )=L2 clipped
            V_PMINUB (mm2, mm4, mm6)
			pand	mm2, YMask				// keep only luma from calc'd value
// chroma comes from avg(L1,L3) 
			movq	mm6, mm1				// L1
#ifdef IS_SSE                               // skip for mmx for performance
            pavgb   mm6, mm3				// avg L3
#endif
			pand    mm6, UVMask             // keep only chroma
			por		mm2, mm6				// and combine

            V_MOVNTQ (qword ptr[edi], mm2)  // move in our clipped best, use macro

// bump ptrs and loop
			lea		eax,[eax+8]				
			lea		ebx,[ebx+8]				
			lea		edx,[edx+8]
			lea		edi,[edi+8]			
			lea		esi,[esi+8]
			dec		LoopCtr
			jnz		DoNext8Bytes
		}
	}

	// Copy last odd line if we're processing an Odd field.
	if (InfoIsOdd)
	{
		pMemcpy(lpCurOverlay + (FrameHeight - 1) * OverlayPitch,
				  pOddLines[FieldHeight - 1],
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



//>>>>>>>>>>>>>>>>
#undef SSE_TYPE
#undef IS_SSE
#undef FUNCT_NAME
#undef SSE_TYPE

#define IS_SSE
#define SSE_TYPE SSE
#define FUNCT_NAME DI_GreedyHF_SSE
#include "DI_GreedyHF.asm"
#undef SSE_TYPE
#undef IS_SSE
#undef FUNCT_NAME

#define IS_3DNOW
#define FUNCT_NAME DI_GreedyHF_3DNOW
#define SSE_TYPE 3DNOW
#include "DI_GreedyHF.asm"
#undef SSE_TYPE
#undef IS_3DNOW
#undef FUNCT_NAME

#define IS_3DNOW
#define SSE_TYPE MMX
#define FUNCT_NAME DI_GreedyHF_MMX
#include "DI_GreedyHF.asm"
#undef SSE_TYPE
#undef IS_3DNOW
#undef FUNCT_NAME

