/////////////////////////////////////////////////////////////////////////////
// $Id: DI_GreedyHF.c,v 1.3 2001-07-28 18:47:24 trbarry Exp $
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
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2001/07/25 12:04:31  adcockj
// Moved Control stuff into DS_Control.h
// Added $Id and $Log to comment blocks as per standards
//
/////////////////////////////////////////////////////////////////////////////

// This is the first version of the Greedy High Motion Deinterlace method I wrote (and kept). 
// It doesn't have many of the fancier options but I left it in because it's faster. It runs with 
// a field delay of 1 in a single pass with no call needed to UpdateFieldStore. It will be called
// if no special options are needed. The logic is somewhat different than the other rtns.  TRB 7/2001

#include "windows.h"
#include "DS_Deinterlace.h"
#include "DI_GreedyHM.h"

BOOL DI_GreedyHF()
{

#include "DI_GreedyHM2.h"
	int Line;
	int	LoopCtr;
	__int64* L1;					// ptr to Line1, of 3
	short* L2N;						// ptr to Line2, in buffer
	__int64* L2;					// ptr to Line2, in FieldStore
	__int64* L2P;					// ptr to prev Line2, the weave line, in FieldStore
	short* L3;						// ptr to Line3 - in input buffer
	BYTE* Dest;
	__int64 LastAvg=0;			//interp value from left qword

// Next 4 lines would usually be done by UpdateFieldStore but we don't call that here
	FsPtrP3 = FsPtrP2;			// now is subscript of oldest field 
	FsPtrP2 = FsPtrP;			// now is subscript of prev field same parity
	FsPtrP = FsPtr;				// now is subscript of prev field
	FsPtr = (++FsPtr) % 4;      // bump to nex

	pMemcpy(lpCurOverlay, pEvenLines[0], LineLength);	// Always copy first line
	if (!InfoIsOdd)
	{
		pMemcpy(lpCurOverlay + OverlayPitch, pOddLines[0], LineLength);  // copy 2nd line if even
	}

	for (Line = 0; Line < (FieldHeight - 1); ++Line)
	{
		LoopCtr = LineLength / 8;				// there are LineLength / 8 qwords per line

// Assume here we are not doing median filtering so we are delaying only one field
// When we are not doing Median filter we have to delay the display by 1 fields so at Time=5
// we are displaying a screen for Time=4, For ODD fields we display an even field and have
// the following, representing part of 1 column on the screen when Line=0, 
// Time = 5, and W = the weave pixel we want to calc to display for Time=4:
// (FsPtr's are subscripts, not pointers)

//  Row  Fields (at Time=1..5)  Just got odd frame, display even frame 4
//  ---  --------------------
//        1  2  3  4  5
//   0       x     L1       Even Rows are directly copied, FsPtrP will point to L1
//   1      L2P    W  L2	We create the W pixel somehow, PsPtr will point to L2
//   2       x     L3       Even Rows directly copied for Odd fields
//							Note L3 not avail on last line, L1 used twice there

		if (InfoIsOdd) // delay 1 so if Odd field then we make Even frame 
		{
			L1 = &FieldStore[FSCOLCT*Line + FsPtrP];	// begin curr top even line	
			L2 = &FieldStore[FSCOLCT*Line + FsPtr];		// begin of odd line 2 ago, will repl w/new
			L2P = &FieldStore[FSCOLCT*Line + FsPtrP2];	// begin of prev odd line
			L2N = pLines[Line];							// data from the new buffer, odd line
			Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;	// DL1, odd weave pixel dest
			L3 = pEvenLines[Line + 1];     // use data from the newest buffer for now
		}

//  Row  Fields (at Time=1..5)  Just got even frame 4, display odd Frame 3
//  ---  --------------------
//        1  2  3  4  5
//   0						Copied
//   1			L1			All odd rows are directly copied
//   2      L2P W  L2		We create the W pixel somehow, FsPtr will point to L2
//   3          L3			Odd Rows directly copied, FsPtrP will point to L3
		else			
		{
			L1 = &FieldStore[FSCOLCT*Line + FsPtrP];	// beg of top curr odd line
			L2 = &FieldStore[FSCOLCT * (Line+1) + FsPtr]; // begin of even line 2 ago,will repl w/new
			L2P = &FieldStore[FSCOLCT * (Line+1) + FsPtrP2]; // begin of prev even line
			L2N = pLines[Line+1];				// data from the new buffer, even line
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;	// DL2
			L3 = pOddLines[Line + 1];		// use data from the newest buffer for now
		}
		pMemcpy(Dest + OverlayPitch, L3, LineLength);
//		pMemcpy(Dest , L2N, LineLength);		//>> for debug only
// For ease of reading, the comments below assume that we're operating on an odd
// field (i.e., that InfoIsOdd is true).  Assume the obvious for even lines..

		_asm
		{
			mov word ptr [LastAvg+6], 0       // init easy way
			mov eax, dword ptr [L1]			// The assumed good top pixels	
			mov ebx, dword ptr [L2]		    // The where to save new weave pixel
			mov esi, dword ptr [L2N]	    // The new possible weave pixel
			mov edx, dword ptr [L2P]		// The prev possible weave pixel
			mov edi, dword ptr [Dest]       // DL1 if Odd or DL2 if Even 
			align 8
DoNext8Bytes:			

			movq	mm0, qword ptr[esi]	    // L2N & L2 - the newest pixel value 
			movq	mm1, qword ptr[eax]		// L1
			movq	mm2, qword ptr[edx]		// L2P 
			movq	mm3, qword ptr[eax+FSROWSIZE] // L3, next odd row
			movq    qword ptr[ebx], mm0		// save our new value for next time
			movq	mm6, mm1				// L1 - get simple single pixel interp
			pavgb   mm6, mm3
			pand    mm6, YMask              // keep only luma

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
			
			movq	mm5, qword ptr[eax+FSCOLSIZE] // next horiz qword from L1
			pavgb   mm5, qword ptr[eax+FSCOLSIZE+FSROWSIZE] // next horiz qword from L3
			psllq	mm5, 48					// left just 1 pixel
			movq	mm7, mm6                // another copy of simple bob pixel
			psrlq   mm7, 16					// right just 3 pixels
			por		mm5, mm7				// combine
			pavgb	mm4, mm5				// avg of forward and prev by 1 pixel
			pand    mm4, YMask              // keep only luma
			pavgb	mm6, mm4				// avg of center and surround interp vals
// >> temp try more % of edges, softening of bob pixels only
            pavgb	mm4, mm6				// 1/4 center, 3/4 adjacent
    		pavgb	mm6, mm4				// 3/8 center, 5/8 adjacent

// get abs value of possible L2 comb
			movq    mm4, mm6				// work copy of interp val
			movq	mm7, mm2				// L2
			psubusb mm7, mm4				// L2 - avg
			movq	mm5, mm4				// avg
			psubusb mm5, mm2				// avg - L2
			por		mm5, mm7				// abs(avg-L2)

// get abs value of possible LP2 comb
			movq	mm7, mm0				// LP2
			psubusb mm7, mm4				// LP2 - avg
			psubusb	mm4, mm0				// avg - LP2
			por		mm4, mm7				// abs(avg-LP2)

// use L2 or LP2 depending upon which makes smaller comb
			psubusb mm4, mm5				// see if it goes to zero
			psubusb mm5, mm5				// 0
			pcmpeqb mm4, mm5				// if (mm4=0) then FF else 0
			pcmpeqb mm5, mm4				// opposite of mm4

// if Comb(LP2) <= Comb(L2) then mm4=ff, mm5=0 else mm4=0, mm5 = 55
			pand	mm5, mm2				// use L2 if mm5 == ff, else 0
			pand	mm4, mm0				// use LP2 if mm4 = ff, else 0
			por		mm4, mm5				// may the best win
			pand    mm4, YMask              // keep only luma

// Inventory: at this point we have the following values:
// mm0 = LP2 (or L2)
// mm1 = L1
// mm2 = L2 (or LP2)
// mm3 = L3
// mm4 = the best of L2,LP2 weave pixel, base upon comb (luma only)
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
			pminsw  mm0, mm7				// max = 256
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
			pmaxub	mm2, mm3                // now = Max(L1,L3)
			movq	mm0, mm1				// copy L1
			pminub	mm0, mm3				// now = Min(L1,L3)
// allow the value to be above the high or below the low by amt of MaxComb
			psubusb mm0, MaxCombW			// lower min by diff
			paddusb	mm2, MaxCombW			// increase max by diff
			pmaxub	mm4, mm0				// now = Max(best,Min(L1,L3)
			pminub	mm2, mm4 				// now = Min( Max(best, Min(L1,L3), L2 )=L2 clipped
			pand	mm2, YMask				// keep only luma from calc'd value
// chroma comes from avg(L1,L3) 
			movq	mm6, mm1				// L1
	        pavgb   mm6, mm3				// avg L3
			pand    mm6, UVMask             // keep only chroma
			por		mm2, mm6				// and combine

			movntq qword ptr[edi], mm2        // move in our clipped best

// bump ptrs and loop
			lea		eax,[eax+FSCOLSIZE]	 			
			lea		ebx,[ebx+FSCOLSIZE]
			lea		edx,[edx+FSCOLSIZE]
			lea		esi,[esi+8]			
			lea		edi,[edi+8]			
			dec		LoopCtr
			jnz		DoNext8Bytes

			emms
		}

/*
	// Copy last odd line if we're processing an Odd field.
	if (InfoIsOdd)
	{
		pMemcpy(lpCurOverlay + (FrameHeight - 1) * OverlayPitch,
				  pOddLines[FieldHeight - 1],
				  LineLength);
	}
*/

	}
	return TRUE;

}
