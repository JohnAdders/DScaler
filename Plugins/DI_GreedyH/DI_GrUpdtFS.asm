/////////////////////////////////////////////////////////////////////////////
// DI_GrUpdtFS.asm
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry  All rights reserved.
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



// The following 2 values may be defined before using this include.
// FUNC_NAME must be defined.
// #define USE_SHARPNESS		
// #define USE_MEDIAN_FILTER
// #define FUNC_NAME DI_GrUpdtFS_NM_NE
BOOL FUNC_NAME()
{
#include "DI_GreedyHM2.h"
	__int64* pFieldStore;		// ptr into FieldStore qwords
	short **pLinesW = pLines;	// current input lines, local storage is faster
	int LineCtr = FieldHeight;	// number of lines to do
    int FirstLine = LineCtr - 2;  // don't use top 3 lines in totals, re-clear totals here
    int LastLine = 4;           // don't use last 3 lines in totals, save totals here
	int	LoopCtr;				// number of qwords in line - 1
	int	LoopCtrW;				// number of qwords in line - 1
	int FsPrev;					// FieldStore elem holding pixels from prev field line
	int FsPrev2;				// Offset to prev pixel (this line) to be median filtered
	int FsNewOld;				// FieldStore elem holding oldest then newest
	int Motion = 0;				// our scaled motion total	
	int CombSum = 0;			// our scaled comb total
	int ContrSum = 0;			// our scaled contrast total
	int CombScale;				// multiplier to keep comb as 100 * avg/pixel	

	if (pLines == NULL)
		return FALSE;

// perc and adjust our global FieldStore subscripts
	FsPtrP3 = FsPtrP2;			// now is subscript of oldest field 
	FsPtrP2 = FsPtrP;			// now is subscript of prev field same parity
	FsPtrP = FsPtr;				// now is subscript of prev field
	FsPtr = (++FsPtr) % 4;      // bump to nex

	FsNewOld = FsPtr * 8;		// Offset to Oldest odd pixel, will be replaced
	FsPrev = FsPtrP * 8;		// FieldStore elem holding pixels from prev field line
	FsPrev2 = FsPtrP2 * 8;		// Offset to prev pixel (this line) to be median filtered

	LineCtr = FieldHeight;		// number lines to do

	CombScale = (FieldHeight - 6) * LineLength / 100;  // Divide totals by this later
	pFieldStore = & FieldStore[0];		// starting ptr into FieldStore
	
	LoopCtr = LineLength / 8 - 1;		// do 8 bytes at a time, adjusted
	LoopCtrW = LoopCtr;
	
	_asm
	{
		mov		esi, pLinesW				// get ptr to line ptrs
		mov		edi, dword ptr [pFieldStore]// addr of our 1st qword in FieldStore
		mov		ecx, dword ptr [FsNewOld]   // where to find oldest,save new pixel
		mov		edx, dword ptr [FsPrev2]	// The prev possible weave pixel
		pxor	mm3, mm3					// clear comb & Kontrast totals
		xor		ebx, ebx					// clear motion totol

		align 8
LineLoop:
		mov		eax, dword ptr [FsPrev]		// offset to pixels from prev line 	
		mov		esi, dword ptr[esi]			// actual get line addr

#ifdef USE_SHARPNESS
// If we are using edge enhancement then the asm loop will expect to be entered
// with mm0,mm1,mm2 holding the left, middle, and right pixel qwords
// On the last pass we will enter the loop at QwordLoop2 to avoid pixels off the end of the line.

		movq	mm1, qword ptr[esi]		// curr qword
		movq	mm0, mm1				// also pretend is left pixels

		align 8
QwordLoop:
		movq	mm2, qword ptr[esi+8]	// pixels to the right, for edge enh.
QwordLoop2:
// do edge enhancement. 
		movq	mm7, mm1				// work copy of curr pixel val
		psrlq   mm0, 48					// right justify 1 pixel from qword to left
		psllq   mm7, 16                 // left justify 3 pixels
		por     mm0, mm7				// and combine
		
		movq	mm6, mm2				// copy of right qword pixel val
		psllq	mm6, 48					// left just 1 pixel from qword to right
		movq	mm7, mm1                // another copy of L2N current
		psrlq   mm7, 16					// right just 3 pixels
		por		mm6, mm7				// combine
		pavgb	mm0, mm6				// avg of forward and prev by 1 pixel

// we handle the possible plus and minus sharpness adjustments separately
		movq    mm7, mm1				// another copy of L2N
		psubusb mm7, mm0				// curr - surround
		pand	mm7, YMask
		pmullw  mm7, EdgeEnhAmt          // mult by sharpness factor
		psrlw   mm7, 8					// now have diff*EdgeEnhAmt/256 ratio			

		psubusb mm0, mm1                // surround - curr
		pand	mm0, YMask
		pmullw  mm0, EdgeEnhAmt          // mult by sharpness factor
		psrlw   mm0, 8					// now have diff*EdgeEnhAmt/256 ratio			

		paddusb mm7, mm1				// edge enhancement up
		psubusb mm7, mm0                // edge enhancement down, mm7 now our sharpened value
		movq	mm4, qword ptr[edi+eax]	// prefetch FsPrev, need later
		movq    mm0, mm1				// migrate for next pass through loop
		movq    mm1, mm2				// migrate for next pass through loop
  
#else

// If we are not using edge enhancement we just need the current value in mm1
QwordLoop:
		movq    mm1, qword ptr[esi]		// no sharpness, just get curr value
		movq	mm4, qword ptr[edi+eax]	// prefetch FsPrev, need later
		movq	mm7, mm1				// work copy of curr pixel val
#endif										// end of sharpness code

		movq    mm2, qword ptr[edi+ecx]  // FsNewOld, fetch before store new pixel
		movq    qword ptr[edi+ecx], mm7 // save our sharp new value for next time
		
// Now is a good time to calc comb, contrast, and motion
//>>> need to optimize this again >>>
        pand    mm4, YMask
		movq	mm5, mm4				// work copy of FsPrev
        movq    mm6, mm7
        pand    mm6, YMask
		psadbw  mm4, mm6				// sum of abs differences is comb
        
        movq    mm6, YMask
		pand    mm6, qword ptr[edi+eax+FSROWSIZE]
		psadbw  mm5, mm6               	// sum of abs differences is contrast
		punpckldq mm4, mm5				// move mm5 to high dword of mm4
		paddd   mm3, mm4				// and accum result(s)

		movq	mm5, qword ptr[edi+edx] // pixels from previous field, same row & col
        pand    mm5, YMask
		movq	mm6, mm7
        pand    mm6, YMask
		psadbw  mm6, mm5				// sum of abs differences is motion
		movd	mm4, ebx				// our motion total
		paddd   mm4, mm6				// accum 
		movd	ebx, mm4				// update our motion total

#ifdef USE_MEDIAN_FILTER

// apply median filter to prev pixels to (from FsPrev2) qword and save
// in:	mm7 = new pixels		
//		mm5 = prev pixels
//		mm2 = old pixels

		movq	mm5, qword ptr[edi+edx] // pixels from previous field, same row & col
		movq	mm6, mm7				// work copy of new pixels
		pminub	mm6, mm2				// Lowest of new and old
		pmaxub	mm7, mm2				// Highest of new and old
		pminub	mm7, mm5				// no higher than highest
		pmaxub	mm7, mm6                // no lower than lowest

// decide if we want to use the filtered value, depending upon how much effect it has
		movq    mm6, mm7
		psubusb mm6, mm5				// how different is the filtered val
		movq    mm4, mm5
		psubusb mm4, mm7				// how different is the filtered val
		por     mm6, mm4				// the abs diff caused by filter

		psubusb mm6, MedianFilterAmt    // bigger than max filter?
		pxor    mm4, mm4
		pcmpeqb mm6, mm4				// will be FFF.. if we should filter small change
		pand    mm7, mm6				// so use filtered val
		pcmpeqb mm6, mm4				// will be FFF.. if we shouldn't filter
		pand	mm5, mm6				// so use unfiltered val
		por		mm7, mm5				// combine
		movq	qword ptr[edi+edx], mm7	// save maybe filtered val for later

#endif									// end of median filter code

// bump ptrs and loop for next qword in row
		lea		edi,[edi+FSCOLSIZE]
		lea		esi,[esi+8]			
		dec		LoopCtr

#ifdef USE_SHARPNESS
		jg		QwordLoop				// if we are not at the end of the row
		movq    mm2, mm1				// if on last qword use same qword again
		jz		QwordLoop2				// fall thru only if neg
#else
		jnl		QwordLoop			
#endif

// Ok, done with one line

        mov     eax, LineCtr
        cmp     eax, FirstLine          // ignore some lines, clear totals here?
        jnz     NotFirst                // no
        pxor    mm3, mm3                // clear Comb, Kontras
        xor     ebx, ebx                // clear motion

NotFirst:
        cmp     eax, LastLine           // ignore some lines, save totals early?
        jnz     NotLast                 // no
		mov		Motion, ebx				// Save our Motion total now 
		movd    CombSum, mm3			// Save our comb total
		psrlq	mm3, 32					// shift our Kontrast total
		movd    ContrSum, mm3			// save that too

NotLast:
		movq    qword ptr[edi], mm1     // jaggie reduction needs one to right later
		mov		eax, LoopCtrW
		mov     LoopCtr, eax            // reset ctr

		mov		edi, pFieldStore		// addr of our 1st qword in FieldStore
		lea     edi, [edi+FSROWSIZE]    // bump to next row
		mov		pFieldStore, edi		// addr of our 1st qword in FieldStore for line

		mov     esi, pLinesW			// ptr to curr ptr
		lea     esi, [esi+4]			// but we want the next one
		mov		pLinesW, esi			// update for next loop

		dec		LineCtr
		jnz		LineLoop				// if not to last line yet

		emms
	}

	UpdatePulldown(CombSum / CombScale, ContrSum / CombScale,
		Motion / CombScale);  // go update our pulldown status for new field
	return TRUE;
}	
