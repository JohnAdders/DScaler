/////////////////////////////////////////////////////////////////////////////
// DI_GreedyHMPulldown.c
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
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 01 Jul 2001   Tom Barry		       New Greedy (High Motion)Deinterlace method
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Deinterlace.h"
#include "DI_GreedyHM.h"
BOOL PullDown_V(BOOL SelectL2);
BOOL PullDown_InBetween();
BOOL FieldStoreMerge(BYTE * dest, __int64 * src, int clen);
BOOL FieldStoreCopy_V(BYTE * dest, __int64 * src, __int64 * src2, int clen);
BOOL FieldStoreMerge_V(BYTE * dest, __int64 * src, __int64 * src2, int clen);

int HistPtr = 0;					// where we are in Queue
static GR_PULLDOWN_INFO Hist[20] = {0};	// a short history of pulldown stats

int UpdatePulldown(int Comb, int Kontrast, int Motion)
{
	int Prev = (HistPtr+20-1) % 20;	// prev entry ptr
	int Last = (Prev + 20+1-PDAVGLEN) % 20;	// Last elem still in average at prev
	int FlagMask = 0x000ffffe;      // trunc to 20 bits, turn off low          

    // note most values are updated delay 1, except input Comb
	if (Comb < Hist[HistPtr].Comb)
	{
		Hist[HistPtr].CombChoice = Comb;
		Hist[HistPtr].Flags = ((Hist[Prev].Flags << 1) & FlagMask) | 1;
		if (Hist[HistPtr].Comb > 0 && Kontrast > 0)
		{
    		Hist[HistPtr].AvgChoice =  100 - 100 * Hist[HistPtr].CombChoice / Hist[HistPtr].Comb;
        }
    }
	else
	{
		Hist[HistPtr].CombChoice = Hist[HistPtr].Comb;
		Hist[HistPtr].Flags = (Hist[Prev].Flags << 1) & FlagMask;
		if (Comb > 0 && Kontrast > 0)
		{
//		    Hist[HistPtr].AvgChoice = 100 * (100 - 100 * Hist[HistPtr].CombChoice / Comb);
		    Hist[HistPtr].AvgChoice = 100 - 100 * Hist[HistPtr].CombChoice / Comb;
		}
	}
	Hist[HistPtr].Kontrast = Kontrast;	// Kontrast calc'd in arrears
	Hist[HistPtr].Motion = Motion;		// Motion Calc'd in arrears
	Hist[HistPtr].Avg = Hist[Prev].Avg + Hist[HistPtr].AvgChoice - Hist[Last].AvgChoice;	

	HistPtr = (++HistPtr) % 20;			// bump for next time
	Hist[HistPtr].Comb = Comb;			// only fill in Comb for curr val, rest is garbage
	Hist[HistPtr].Kontrast = 0;			// only fill in Comb for curr val, rest is garbage
	Hist[HistPtr].Motion= 0;			// only fill in Comb for curr val, rest is garbage
    if (!InfoIsOdd)
    {   
        Hist[HistPtr].Flags2 = PD_ODD;
    }
    else
    {
        Hist[HistPtr].Flags2 = 0;
    }
    return 0;
}

int CheckPD()	
{		
	int hPtr = (HistPtr + 20 - FsDelay) % 20;
	if ( Hist[hPtr].Flags2 == 0)
	{
		return FALSE;
	}
	return FALSE;
}

BOOL CanDoPulldown()					// check if we should do pulldown, doit
{
	int line;				// number of lines
	int L1;						// FieldStore elem holding top known pixels-unneeded
	int L3;						// FieldStore elem holding bottom known pxl-unneeded
	int L2;						// FieldStore elem holding newest weave pixels
	int L2P;					// FieldStore elem holding prev weave pixels
	const int PdMask1 = 0xf;    // 0b01111
	const int PdMask2 = PdMask1 << 5 | PdMask1;     // 0b01111,01111
	const int PdMask32 = PdMask1 << 20 | PdMask2 << 10 | PdMask2;    // 0b0111101111011110111101111
    const int Pd20Bits = 0x000fffff;     // 20 1 bits 
    const int PdMerge1 = 5;		// 0b00101
	const int PdMerge2 = PdMerge1 << 5 | PdMerge1;     // 0b00101,00101
    const int Pd32Pattern = PdMerge1 << 20 | PdMerge2 << 10 | PdMerge2; // 0b00101.. 5 times
    const int Pd22Pattern = 0x00055555; // 2:2 pattern = 0b01010101..
    const int Pd22PatternB = 0x000aaaaa; // 2:2 pattern = 0b10101010..
    
	const int PdMask1b = 0x1b;    // 0b11011
	const int PdMask2b = PdMask1b << 5 | PdMask1b;     // 0b11011, 0b11011
	const int PdMerge1b = 9;		// 0b01001
	const int PdMerge2b = PdMerge1b << 5 | PdMerge1b;     // 0b01x01,01x01
	BYTE* WeaveDest;			// dest for weave pixel
	BYTE* CopyDest;				// other dest, copy or vertical filter
	int CopySrc;                // where to copy from
	int hPtr = (HistPtr + 20 - FsDelay) % 20;     // curr HistPtr adj for delay
	int hPtrb = (HistPtr + 20 - 1) % 20;          // curr HistPtr delay 1
    int FlagsW = Hist[hPtrb].Flags;    

    if (! GreedyUsePulldown || Hist[hPtr].AvgChoice == 0 || Hist[hPtr].Avg == 0)
	{
        Hist[hPtr].Flags2 |= PD_VIDEO;	// flag did video, not pulldown
		return FALSE;
	}

	// If Greedy Matching doesn't help enough or this field comb raises avg too much
	// then no good.
    if (Hist[hPtr].Motion < GreedyLowMotionPdLvl*10)
//    if (100 * Hist[hPtr].Motion < GreedyLowMotionPdLvl * Hist[hPtr].Kontrast )
    {
         Hist[hPtr].Flags2 |= PD_LOW_MOTION;	// flag did pulldown due to low motion
    }

    else if ( ( ((FlagsW ^ Pd32Pattern) & PdMask32 & Pd20Bits) == 0)
            | ( ((FlagsW ^ (Pd32Pattern >> 1) ) & (PdMask32 >> 1) & Pd20Bits) == 0)
            | ( ((FlagsW ^ (Pd32Pattern >> 2) ) & (PdMask32 >> 2) & Pd20Bits) == 0)
            | ( ((FlagsW ^ (Pd32Pattern >> 3) ) & (PdMask32 >> 3) & Pd20Bits) == 0)
            | ( ((FlagsW ^ (Pd32Pattern >> 4) ) & (PdMask32 >> 4) & Pd20Bits) == 0) )
    {
        Hist[hPtr].Flags2 |= PD_32_PULLDOWN;	// flag found 20 bit 3:2 pulldown pattern
    }

/* >>> temp remove until it works better
    else if ( ( ((FlagsW ^ Pd22Pattern) & Pd20Bits) == 0 )
            | ( ((FlagsW ^ Pd22PatternB) & Pd20Bits) == 0 ) )
    {
        Hist[hPtr].Flags2 |= PD_22_PULLDOWN;	// flag found 20 bit 2:2 pulldown pattern
    }
*/

    else if (1000 * Hist[hPtr].Avg  < GreedyGoodPullDownLvl * PDAVGLEN * Hist[hPtr].Comb)
	{
        Hist[hPtr].Flags2 |= PD_VIDEO;	// flag did video, not pulldown
	    return FALSE;
	}

//	if (Hist[hPtr].CombChoice * PDAVGLEN * 100 > Hist[hPtr].AvgChoice * GreedyBadPullDownLvl )
	if (Hist[hPtr].CombChoice * 100 > Hist[hPtr].Kontrast *  GreedyBadPullDownLvl )
	{
		Hist[hPtr].Flags2 |= PD_BAD | PD_VIDEO;	// flag bad pulldown, did video
		return FALSE;
	}

	// We can do some sort of pulldown
	Hist[hPtr].Flags2 |= PD_PULLDOWN;

	if (GreedyUseInBetween) 
    {
        if (FsDelay == 2)   // for FsDelay == 2 do inbetween for (delay 1) flags 01x0101x01 pattern only
        {
            if ( (Hist[hPtrb].Flags & PdMask2b) == PdMerge2b )
            {
	            Hist[hPtr].Flags2 |= PD_MERGED;
	            return PullDown_InBetween();
            }
        }
        
        else    // for FsDelay == 1 do inbetween for flags x0101x0101 pattern only
        {
            if ( (Hist[hPtr].Flags & PdMask2) == PdMerge2 )  
            {
	            Hist[hPtr].Flags2 |= PD_MERGED;
	            return PullDown_InBetween();
            }
        }
    }

	// Do pulldown, but separate procedure if we need Vertical Filter
	if (GreedyUseVertFilter)
	{
		return PullDown_V(Hist[hPtr].Flags & 1);
	}

	// OK, do simple pulldown
	
	// set up pointers, offsets
	SetFsPtrs(&L1, &L2, &L2P, &L3, &CopySrc, &CopyDest, &WeaveDest);
	// chk forward/backward Greedy Choice Flag for this field
	if (!(Hist[hPtr].Flags & 1))
	{
		L2 = L2P;
	}

	for (line = 0; line < (FieldHeight); ++line)
	{
		FieldStoreCopy(CopyDest, &FieldStore[CopySrc], LineLength);
		CopyDest += 2 * OverlayPitch;
		CopySrc += FSMAXCOLS;

		FieldStoreCopy(WeaveDest, &FieldStore[L2], LineLength);
		WeaveDest += 2 * OverlayPitch;
		L2 += FSMAXCOLS;
	}
	return TRUE;
}

// Return sorted last N Hist Records - note current entry never complete yet
BOOL PullDown_InBetween()					
{
	int	line;
	int EvenL = __min(FsPtrP, FsPtrP3);	    // where to get even lines
	int OddL = __min(FsPtr, FsPtrP2);		//  " odd lines
	BYTE* Dest = lpCurOverlay;
	if (InfoIsOdd)
	{
	    EvenL = __min(FsPtrP, FsPtrP3);	// first copy ptr
	    OddL = __min(FsPtr, FsPtrP2);		// first weave ptr
	}
	else
	{
	    EvenL = __min(FsPtr, FsPtrP2);	// first copy ptr
	    OddL = __min(FsPtrP, FsPtrP3);		// first weave ptr
	}
    if (GreedyUseVertFilter)
    {
        for (line = 0; line < (FieldHeight - 1); ++line)
	    {

		    FieldStoreMerge_V(Dest, &FieldStore[EvenL], &FieldStore[OddL], LineLength);
		    Dest += OverlayPitch;
		    EvenL += FSMAXCOLS;

		    FieldStoreMerge_V(Dest, &FieldStore[OddL], &FieldStore[EvenL], LineLength);
		    Dest += OverlayPitch;
		    OddL += FSMAXCOLS;
	    }
        // one more time but dup last line
        FieldStoreMerge_V(Dest, &FieldStore[EvenL], &FieldStore[OddL], LineLength);
		Dest += OverlayPitch;
        FieldStoreMerge_V(Dest, &FieldStore[EvenL], &FieldStore[OddL], LineLength);

    }
    else    // not vertical filter
    {
       	for (line = 0; line < FieldHeight; ++line)
	    {

		    FieldStoreMerge(Dest, &FieldStore[EvenL], LineLength);
		    Dest += OverlayPitch;
		    EvenL += FSMAXCOLS;

		    FieldStoreMerge(Dest, &FieldStore[OddL], LineLength);
		    Dest += OverlayPitch;
		    OddL += FSMAXCOLS;
	    }
    }
return TRUE;
}

// copy 1 line from Fieldstore to overlay buffer, mult of 32 bytes, merging with succeeding line.
BOOL FieldStoreCopy_V(BYTE * dest, __int64 * src, __int64 * src2, int clen)
{
	int ct = clen / 32;
	_asm
	{
		mov		esi, src                    // a source line
		mov		ebx, src2                   // the other source line to merge
        sub     ebx, esi                    // carry as offset
		mov		edi, dest					// new output line dest
		mov		ecx, ct

cloop:	
		movq	mm0, qword ptr[esi]
		movq	mm1, qword ptr[esi+FSCOLSIZE]
		movq	mm2, qword ptr[esi+FSCOLSIZE*2]
		movq	mm3, qword ptr[esi+FSCOLSIZE*3]

		pavgb	mm0, qword ptr[esi+ebx + 16]		// next frame 2 qwords removed
		pavgb	mm1, qword ptr[esi+ebx + FSCOLSIZE + 16]
		pavgb	mm2, qword ptr[esi+ebx + FSCOLSIZE*2 + 16]
		pavgb	mm3, qword ptr[esi+ebx + FSCOLSIZE*3 + 16]
		
		movntq	qword ptr[edi], mm0
		movntq	qword ptr[edi+8], mm1
		movntq	qword ptr[edi+16], mm2
		movntq	qword ptr[edi+24], mm3
		lea		esi, [esi+FSCOLSIZE*4]
		lea		edi, [edi+32]
		loop	cloop						// go do next 4 qwords
		sfence
		emms
	}
	return TRUE;
}


// copy 1 line from Fieldstore to overlay buffer, mult of 32 bytes, merging with succeeding line and 
// succeeding frame.
BOOL FieldStoreMerge_V(BYTE * dest, __int64 * src, __int64 * src2, int clen)
{
	int ct = clen / 32;
	_asm
	{
		mov		esi, src                    // a source line
		mov		ebx, src2                   // the other source line to merge
        sub     ebx, esi                    // carry as offset
		mov		edi, dest					// new output line dest
		mov		ecx, ct

cloop:	
		movq	mm0, qword ptr[esi]             // 4 qwords from current row and field
		movq	mm1, qword ptr[esi + FSCOLSIZE]
		movq	mm2, qword ptr[esi + FSCOLSIZE * 2]
		movq	mm3, qword ptr[esi + FSCOLSIZE * 3]

		movq	mm4, qword ptr[esi + ebx]		// 4 more of same from following line
		movq	mm5, qword ptr[esi + ebx + FSCOLSIZE]
		movq	mm6, qword ptr[esi + ebx + FSCOLSIZE * 2]
		movq	mm7, qword ptr[esi + ebx + FSCOLSIZE * 3]

        pavgb	mm0, qword ptr[esi + 16]		// current row, next frame 2 qwords removed
		pavgb	mm1, qword ptr[esi + FSCOLSIZE + 16]
		pavgb	mm2, qword ptr[esi + FSCOLSIZE * 2 + 16]
		pavgb	mm3, qword ptr[esi + FSCOLSIZE * 3 + 16]
		
        pavgb	mm4, qword ptr[esi + ebx + 16]		// next row, next frame 2 qwords removed
		pavgb	mm5, qword ptr[esi + ebx + FSCOLSIZE + 16]
		pavgb	mm6, qword ptr[esi + ebx + FSCOLSIZE * 2 + 16]
		pavgb	mm7, qword ptr[esi + ebx + FSCOLSIZE * 3 + 16]

        pavgb	mm0, mm4
		pavgb	mm1, mm5
		pavgb	mm2, mm6
		pavgb	mm3, mm7
		
		movntq	qword ptr[edi], mm0
		movntq	qword ptr[edi + 8], mm1
		movntq	qword ptr[edi + 16], mm2
		movntq	qword ptr[edi + 24], mm3

		lea		esi, [esi + FSCOLSIZE * 4]
		lea		edi, [edi + 32]
		loop	cloop						// go do next 4 qwords
		sfence
		emms
	}
	return TRUE;
}


// copy 1 line from Fieldstore to overlay buffer, mult of 32 bytes, merging 2 frames
// Only the lower of the 2 src addr is passed, other assumed
BOOL FieldStoreMerge(BYTE * dest, __int64 * src, int clen)
{
	int ct = clen / 32;
	_asm
	{
		mov		esi, src
		mov		edi, dest					// new output line dest
		mov		ecx, ct

cloop:	
		movq	mm0, qword ptr[esi]
		movq	mm1, qword ptr[esi+FSCOLSIZE]
		movq	mm2, qword ptr[esi+FSCOLSIZE*2]
		movq	mm3, qword ptr[esi+FSCOLSIZE*3]

		pavgb	mm0, qword ptr[esi + 16]		// next frame 2 qwords removed
		pavgb	mm1, qword ptr[esi+FSCOLSIZE + 16]
		pavgb	mm2, qword ptr[esi+FSCOLSIZE*2 + 16]
		pavgb	mm3, qword ptr[esi+FSCOLSIZE*3 + 16]
		
		movntq	qword ptr[edi], mm0
		movntq	qword ptr[edi+8], mm1
		movntq	qword ptr[edi+16], mm2
		movntq	qword ptr[edi+24], mm3
		lea		esi, [esi+FSCOLSIZE*4]
		lea		edi, [edi+32]
		loop	cloop						// go do next 4 qwords
		sfence
		emms
	}
	return TRUE;
}

// Return sorted last N Hist Records - note current entry never complete yet
BOOL GetHistData(GR_PULLDOWN_INFO * OHist, int ct)					
{
	int i;
	int j= (HistPtr+20-ct) % 20;
	for (i = 0; i < ct; ++i)
	{
		*OHist = Hist[j];
		OHist++;
		j = (++j) % 20;
	}
	return TRUE;
}


// Pulldown with vertical filter
BOOL PullDown_V(BOOL SelectL2)
{
	int line;				// number of lines
	int	LoopCtr;				// number of qwords in line - 1
	int	LoopCtrW;				// number of qwords in line - 1

	int L1;						// offset to FieldStore elem holding top known pixels
	int L3;						// offset to FieldStore elem holding bottom known pxl
	int L2;						// offset to FieldStore elem holding newest weave pixels
	int L2P;					// offset to FieldStore elem holding prev weave pixels
	__int64* pFieldStore;		// ptr into FieldStore qwords
	__int64* pFieldStoreEnd;	// ptr to Last FieldStore qword
	__int64* pL2;				// ptr into FieldStore[L2] 
	BYTE* WeaveDest;					// dest for weave pixel
	BYTE* CopyDest;				// other dest, copy or vertical filter
	int CopySrc;

	int DestIncr = 2 * OverlayPitch;  // we go throug overlay buffer 2 lines per

	// set up pointers, offsets
	SetFsPtrs(&L1, &L2, &L2P, &L3, &CopySrc, &CopyDest, &WeaveDest);
	if (!SelectL2)
	{
		L2 = L2P;
	}
	L1 = (L1 - L2) * 8;					// now is signed offset from L2  
	L3 = (L3 - L2) * 8;					// now is signed offset from L2  
	pFieldStore = & FieldStore[0];		// starting ptr into FieldStore[L2]
	pFieldStoreEnd = & FieldStore[FieldHeight * FSCOLCT];		// ending ptr into FieldStore[L2]
	pL2 = & FieldStore[L2];				// starting ptr into FieldStore[L2]
	LoopCtrW = LineLength / 8 - 1;		// do 8 bytes at a time, adjusted

	for (line = 0; line < (FieldHeight); ++line)
	{
		LoopCtr = LoopCtrW;				// actually qword counter
		if (WeaveDest == lpCurOverlay)    // on first line may just copy first and last
		{
			FieldStoreCopy(lpCurOverlay, &FieldStore[CopySrc], LineLength);
			WeaveDest += DestIncr;		// bump for next, CopyDest already OK
			pL2 = & FieldStore[L2 + FSCOLCT];
		}
		else
		{			

		_asm		// should indent here but I can't read it
		{
		mov		edi, WeaveDest				// get ptr to line ptrs	
		mov		esi, dword ptr [pL2]		// addr of our 1st qword in FieldStore
		mov		eax, L1						// offset to top 	
		mov		ebx, L3						// offset to top comb 	
		lea     edx, [esi+eax]				// where L1 would point
		cmp		edx, pFieldStore			// before begin of fieldstore?
		jnb		L1OK						// n, ok
		mov		eax, ebx					// else use this for top pixel vals
L1OK:
		lea     edx, [esi+ebx]				// where L2 would point
		cmp		edx, pFieldStoreEnd			// after end of fieldstore?
		jb		L3OK						// n, ok
		mov		ebx, eax					// else use this bottom pixel vals

L3OK:		
		mov		edx, CopyDest

		align 8
QwordLoop:

#define FSOFFS 0 * FSCOLSIZE				// following include needs an offset

// The Value FSOFFS must be defined before including this header.
//
// On exit mm2 will contain the value of the calculated weave pixel, not yet stored.
// It is also expected that mm1 and mm3 will still contain the vertically adjacent pixels
// which may be needed for the vertical filter.
		movq	mm1, qword ptr[esi+eax+FSOFFS]	// L1
		movq	mm2, qword ptr[esi+FSOFFS]		// L2 
		movq	mm3, qword ptr[esi+ebx+FSOFFS] // L3

		pavgb   mm1, mm2
		movntq	qword ptr[edx], mm1		// top & middle pixels avg
		pavgb   mm3, mm2
		movntq	qword ptr[edi], mm3		// middle & bottom

		// bump ptrs and loop for next qword
		lea		edx,[edx+8]				// bump CopyDest
		lea		edi,[edi+8]				// bump WeaveDest
		lea		esi,[esi+FSCOLSIZE]			
		dec		LoopCtr
		jg		QwordLoop			

// Ok, done with one line
		mov		esi, pL2				// addr of our 1st qword in FieldStore
		lea     esi, [esi+FSROWSIZE]    // bump to next row
		mov		pL2, esi				// addr of our 1st qword in FieldStore for line
		mov     edi, WeaveDest			// ptr to curr overlay buff line start
		add     edi, DestIncr			// but we want to skip 1
		mov		WeaveDest, edi			// update for next loop
		mov     edx, CopyDest			// ptr to curr overlay buff line start
		add     edx, DestIncr			// but we want to skip 1
		mov		CopyDest, edx			// update for next loop
		sfence
		emms
		}
		}		// should undent here but I can't read it
	}

  return TRUE;
}	
		
