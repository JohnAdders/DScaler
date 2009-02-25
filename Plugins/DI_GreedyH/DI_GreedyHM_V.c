/////////////////////////////////////////////////////////////////////////////
// $Id$
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
// 01 Jul 2001   Tom Barry		       Break out Greedy (High Motion) Deinterlace, w/Vert Filter
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2001/08/17 19:30:55  trbarry
// Fix OBO error in GreedyH Vertical Filter
//
// Revision 1.3  2001/08/17 17:08:42  trbarry
// GreedyH performance enhancement:
//
// Unroll loop to support Write Combining in Vertical Filter
// (curiously this now peforms better than without V. Filter)
//
// Revision 1.2  2001/07/25 12:04:31  adcockj
// Moved Control stuff into DS_Control.h
// Added $Id and $Log to comment blocks as per standards
//
/////////////////////////////////////////////////////////////////////////////

// This version handles Greedy High Motion with Vertical Filtering

#include "windows.h"
//>>>>>>>>>#include "DS_Deinterlace.h"
#include "DI_GreedyHM.h"

// debugging options
#undef USE_JAGGIE_REDUCTION
#undef USE_GREEDY_CHOICE
#undef USE_CLIP
#undef USE_BOB_BLEND

#define USE_JAGGIE_REDUCTION
#define USE_GREEDY_CHOICE
#define USE_CLIP
#define USE_BOB_BLEND


#define FUNCT_NAME DI_GreedyHM_V

BOOL FUNCT_NAME()
{
#include "DI_GreedyHM2.h"
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
	__int64 LastAvg=0;					//interp value from left qword
	__int64 SaveQword1=0;				// Temp Save pixels
	__int64 SaveQword2=0;				// Temp Save pixels
	__int64 SaveQword3=0;				// Temp Save pixels



	// set up pointers, offsets
	SetFsPtrs(&L1, &L2, &L2P, &L3, &CopySrc, &CopyDest, &WeaveDest);
	L2 = __min(L2, L2P);				// Subscript to 1st of 2 possible weave pixels, our base addr
	L1 = (L1 - L2) * 8;					// now is signed offset from L2  
	L3 = (L3 - L2) * 8;					// now is signed offset from L2  
	pFieldStore = & FieldStore[0];		// starting ptr into FieldStore[L2]
	pFieldStoreEnd = & FieldStore[FieldHeight * FSCOLCT];		// ending ptr into FieldStore[L2]
	pL2 = & FieldStore[L2];				// starting ptr into FieldStore[L2]
	LoopCtrW = LineLength / 32;		    // do 8 bytes at a time, adjusted

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
		mov		ecx, OverlayPitch			// overlay pitch
		mov		word ptr [LastAvg+6], 0     // init left avg lazy way
		
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
		}

// 1st 4 qwords
#define FSOFFS 0 * FSCOLSIZE				// following include needs an offset
#include "DI_GreedyDeLoop.asm"
		_asm
		{
		pavgb   mm1, mm4
		movntq	qword ptr[edx], mm1         // avg clipped best with above line
		pavgb   mm3, mm4
		movq	SaveQword1, mm3	            // avg clipped best with below line, save for later
        }

// 2nd 4 qwords
#undef  FSOFFS
#define FSOFFS 1 * FSCOLSIZE				// following include needs an offset
#include "DI_GreedyDeLoop.asm"
		_asm
		{
		pavgb   mm1, mm4
		movntq	qword ptr[edx+8], mm1       // avg clipped best with above line
		pavgb   mm3, mm4
		movq	SaveQword2, mm3	            // avg clipped best with below line, save for later
        }
// 3rd 4 qwords
#undef  FSOFFS
#define FSOFFS 2 * FSCOLSIZE				// following include needs an offset
#include "DI_GreedyDeLoop.asm"
		_asm
		{
		pavgb   mm1, mm4
		movntq	qword ptr[edx+16], mm1      // avg clipped best with above line
		pavgb   mm3, mm4
		movq	SaveQword3, mm3	            // avg clipped best with below line, save for later
        }

// 4'th 4 qwords
#undef  FSOFFS
#define FSOFFS 3 * FSCOLSIZE				// following include needs an offset
#include "DI_GreedyDeLoop.asm"
		_asm
		{
        movq    mm5, SaveQword1             // get saved pixels
        movq    mm6, SaveQword2             // get saved pixels
        movq    mm7, SaveQword3             // get saved pixels
        pavgb   mm1, mm4
		movntq	qword ptr[edx+24], mm1      // avg clipped best with above line
		pavgb   mm3, mm4
		movntq	qword ptr[edi], mm5	        // store saved pixels
		movntq	qword ptr[edi+8], mm6	    // store saved pixels
		movntq	qword ptr[edi+16], mm7	    // store saved pixels
		movntq	qword ptr[edi+24], mm3	        // avg clipped best with below line
        }

        _asm
        {
		// bump ptrs and loop for next 4 qword
		lea		edx,[edx+32]				// bump CopyDest
		lea		edi,[edi+32]				// bump WeaveDest
		lea		esi,[esi+4*FSCOLSIZE]			
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
		
