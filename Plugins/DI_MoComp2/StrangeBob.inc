/////////////////////////////////////////////////////////////////////////////
// $Id: StrangeBob.inc,v 1.1 2003-01-02 13:15:01 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Tom Barry & John Adcock.  All rights reserved.
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
//
//  (From Tom Barry)
//  Also, this program is "Philanthropy-Ware".  That is, if you like it and 
//  feel the need to reward or inspire the author then please feel free (but
//  not obligated) to consider joining or donating to the Electronic Frontier
//  Foundation. This will help keep cyber space free of barbed wire and bullsh*t.  
//  See www.eff.org for details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

__asm
{
	
	// First, get and save our possible Bob values
	// Assume our pixels are layed out as follows with x the calc'd bob value
	// and the other pixels are from the current field
	//  
	//		  j a b c k		current field
	//            x			calculated line
	//        m d e f n		current field
	//
	// we calc the bob value luma value as:
    // if |j - n| < Thres && |a - m| > Thres 
    //  avg(j,n)
    // end if
    // if |k - m| < Thres && |c - n| > Thres 
    //  avg(k,m)
    // end if
    // if |c - d| < Thres && |b - f| > Thres 
    //  avg(c,d)
    // end if
    // if |a - f| < Thres && |b - d| > Thres 
    //  avg(a,f)
    // end if
    // if |b - e| < Thres
    //  avg(b,e)
    // end if
    // pickup any thing not yet set with avg(b,e)

	// j, n
    pxor mm5, mm5
    pxor mm6, mm6
    pxor mm7, mm7

	movq    mm0, qword ptr[ebx-2]		// value a from top left		
	movq    mm1, qword ptr[ebx+ecx-4]	// value m from bottom right			

	movq	mm3, mm0
	psubusb	mm3, mm1
	psubusb mm1, mm0
	por		mm3, mm1					// abs(a,m)

	psubusb mm3, DiffThres		// nonzero where abs(a,m) > Thres else 0
	pxor	mm4, mm4
	pcmpeqb mm3, mm4			// now ff where abs(a,m) < Thres, else 00	
	pcmpeqb	mm4, mm3			// here ff where abs(a,m) > Thres, else 00


	movq    mm0, qword ptr[ebx-4]		// value j
	movq    mm1, qword ptr[ebx+ecx+4]	// value n
	movq	mm2, mm0					
	pavgb	mm2, mm1					// avg(j,n)
    movq	mm3, mm0
	psubusb	mm0, mm1
	psubusb mm1, mm3
	por		mm0, mm1					// abs(j,n)

    movq    mm1, mm0
	psubusb mm1, DiffThres		// nonzero where abs(j,n) > Thres else 0
	pxor	mm3, mm3
	pcmpeqb mm1, mm3			// now ff where abs(j,n) < Thres, else 00	

    pand    mm1, mm4

    pand    mm2, mm1
    pand    mm0, mm1

    movq    mm3, mm1
    pxor    mm3, mm5
    pand    mm6, mm3
    pand    mm7, mm3
    pand    mm5, mm3

    por     mm5, mm1
    por     mm6, mm2
    por     mm7, mm0
    
    
    // k & m
	movq    mm0, qword ptr[ebx+2]		// value c from top left		
	movq    mm1, qword ptr[ebx+ecx+4]	// value n from bottom right			

	movq	mm3, mm0
	psubusb	mm3, mm1
	psubusb mm1, mm0
	por		mm3, mm1					// abs(c,n)

	psubusb mm3, DiffThres		// nonzero where abs(c,n) > Thres else 0
	pxor	mm4, mm4
	pcmpeqb mm3, mm4			// now ff where abs(c,n) < Thres, else 00	
	pcmpeqb	mm4, mm3			// here ff where abs(c,n) > Thres, else 00


	movq    mm0, qword ptr[ebx+4]		// value k
	movq    mm1, qword ptr[ebx+ecx-4]	// value m
	movq	mm2, mm0					
	V_PAVGB (mm2, mm1, mm3, ShiftMask)	// avg(k,m)
    movq	mm3, mm0
	psubusb	mm0, mm1
	psubusb mm1, mm3
	por		mm0, mm1					// abs(k,m)

    movq    mm1, mm0
	psubusb mm1, DiffThres		// nonzero where abs(k,m) > Thres else 0
	pxor	mm3, mm3
	pcmpeqb mm1, mm3			// now ff where abs(k,m) < Thres, else 00	

    pand    mm1, mm4

    pand    mm2, mm1
    pand    mm0, mm1

    movq    mm3, mm1
    pxor    mm3, mm5
    pand    mm6, mm3
    pand    mm7, mm3
    pand    mm5, mm3

    por     mm5, mm1
    por     mm6, mm2
    por     mm7, mm0


    // c & d
	movq    mm0, qword ptr[ebx]		// value b from top left		
	movq    mm1, qword ptr[ebx+ecx+2]	// value f from bottom right			

	movq	mm3, mm0
	psubusb	mm3, mm1
	psubusb mm1, mm0
	por		mm3, mm1					// abs(b,f)

	psubusb mm3, DiffThres		// nonzero where abs(b,f) > Thres else 0
	pxor	mm4, mm4
	pcmpeqb mm3, mm4			// now ff where abs(b,f) < Thres, else 00	
	pcmpeqb	mm4, mm3			// here ff where abs(b,f) > Thres, else 00

	movq    mm0, qword ptr[ebx+2]		// value c
	movq    mm1, qword ptr[ebx+ecx-2]	// value d
	movq	mm2, mm0					
	V_PAVGB (mm2, mm1, mm3, ShiftMask)	// avg(c,d)
    movq	mm3, mm0
	psubusb	mm0, mm1
	psubusb mm1, mm3
	por		mm0, mm1					// abs(c,d)

    movq    mm1, mm0
	psubusb mm1, DiffThres		// nonzero where abs(c,d) > Thres else 0
	pxor	mm3, mm3
	pcmpeqb mm1, mm3			// now ff where abs(c,d) < Thres, else 00	

    pand    mm1, mm4

    pand    mm2, mm1
    pand    mm0, mm1

    movq    mm3, mm1
    pxor    mm3, mm5
    pand    mm6, mm3
    pand    mm7, mm3
    pand    mm5, mm3

    por     mm5, mm1
    por     mm6, mm2
    por     mm7, mm0

    // a & f
	movq    mm0, qword ptr[ebx]		// value b from top left		
	movq    mm1, qword ptr[ebx+ecx-2]	// value d from bottom right			

	movq	mm3, mm0
	psubusb	mm3, mm1
	psubusb mm1, mm0
	por		mm3, mm1					// abs(b,d)

	psubusb mm3, DiffThres		// nonzero where abs(b,d) > Thres else 0
	pxor	mm4, mm4
	pcmpeqb mm3, mm4			// now ff where abs(b,d) < Thres, else 00	
	pcmpeqb	mm4, mm3			// here ff where abs(b,d) > Thres, else 00

	movq    mm0, qword ptr[ebx-2]		// value a
	movq    mm1, qword ptr[ebx+ecx+2]	// value f
	movq	mm2, mm0					
	V_PAVGB (mm2, mm1, mm3, ShiftMask)	// avg(a,f)
    movq	mm3, mm0
	psubusb	mm0, mm1
	psubusb mm1, mm3
	por		mm0, mm1					// abs(a,f)

    movq    mm1, mm0
	psubusb mm1, DiffThres		// nonzero where abs(a,f) > Thres else 0
	pxor	mm3, mm3
	pcmpeqb mm1, mm3			// now ff where abs(a,f) < Thres, else 00	

    pand    mm1, mm4

    pand    mm2, mm1
    pand    mm0, mm1

    movq    mm3, mm1
    pxor    mm3, mm5
    pand    mm6, mm3
    pand    mm7, mm3
    pand    mm5, mm3

    por     mm5, mm1
    por     mm6, mm2
    por     mm7, mm0

 	pand	mm5, YMask			// mask out chroma from here
 	pand	mm6, YMask			// mask out chroma from here
 	pand	mm7, YMask			// mask out chroma from here

	// b,e
	movq    mm0, qword ptr[ebx]		// value b from top 		
	movq    mm1, qword ptr[ebx+ecx]	// value e from bottom 
	movq	mm2, mm0					
	V_PAVGB (mm2, mm1, mm3, ShiftMask)	// avg(b,e)
    movq	mm3, mm0
	psubusb	mm0, mm1
	psubusb mm1, mm3
	por		mm0, mm1					// abs(b,e)

    movq    mm1, mm0
	psubusb mm1, DiffThres		// nonzero where abs(b,e) > Thres else 0
	pxor	mm3, mm3
	pcmpeqb mm1, mm3			// now ff where abs(b,e) < Thres, else 00	

    pand    mm2, mm1
    pand    mm0, mm1

    movq    mm3, mm1
    pxor    mm3, mm5
    pand    mm6, mm3
    pand    mm7, mm3
    pand    mm5, mm3

    por     mm5, mm1
    por     mm6, mm2
    por     mm7, mm0

	// bob in any leftovers
	movq    mm0, qword ptr[ebx]		// value b from top 		
	movq    mm1, qword ptr[ebx+ecx]	// value e from bottom 

// We will also calc here the max/min values to later limit comb
// so the max excursion will not exceed the Max_Comb constant

#ifdef SKIP_SEARCH		
	movq	mm2, mm0
//		pminub	mm2, mm1
	V_PMINUB (mm2, mm1, mm4)

//		pmaxub	mm6, mm2			// clip our current results so far to be above this
	V_PMAXUB (mm6, mm2)
	movq	mm2, mm0
	V_PMAXUB (mm2, mm1)
//		pminub	mm6, mm2			// clip our current results so far to be below this
	V_PMINUB (mm6, mm2, mm4)

#else
	movq	mm2, mm0
	movq	mm4, qword ptr[eax]
	psubusb mm2, mm4
	psubusb mm4, mm0
	por		mm4, mm2			// abs diff
	
	movq	mm2, mm1
	movq	mm3, qword ptr[eax+ecx]
	psubusb mm2, mm3
	psubusb mm3, mm1
	por		mm3, mm2			// abs diff
//		pmaxub  mm3, mm4			// top or bottom pixel moved most
	V_PMAXUB (mm3, mm4)			// top or bottom pixel moved most
	psubusb mm3, DiffThres		// moved more than allowed? or goes to 0?
	pxor	mm4, mm4
	pcmpeqb mm3, mm4			// now ff where low motion, else high motion
	
	movq	mm2, mm0
//		pminub	mm2, mm1
	V_PMINUB (mm2, mm1, mm4)

//		pmaxub	mm6, mm2			// clip our current results so far to be above this
	V_PMAXUB (mm6, mm2)

	psubusb mm2, mm3			// maybe decrease it to 0000.. if no surround motion
	movq	Min_Vals, mm2

	movq	mm2, mm0
	V_PMAXUB (mm2, mm1)
//		pminub	mm6, mm2			// clip our current results so far to be below this
	V_PMINUB (mm6, mm2, mm4)
	paddusb mm2, mm3			// maybe increase it to ffffff if no surround motion
	movq	Max_Vals, mm2
#endif
		
	movq	mm2, mm0						
//		pavgb	mm2, mm1					// avg(b,e)
	V_PAVGB (mm2, mm1, mm3, ShiftMask)	// avg(b,e)
			
	movq	mm3, mm0
	psubusb	mm3, mm1
	psubusb mm1, mm0
	por		mm3, mm1			// abs(b,e)
	movq	mm1, mm3			// keep copy of diffs

	pxor	mm4, mm4			
	psubusb mm3, mm7			// nonzero where new weights bigger, else 0
	pcmpeqb mm3, mm4			// now ff where new better, else 00	
    pcmpeqb mm0, mm0
    pandn   mm5, mm0
    por     mm3, mm5
	pcmpeqb	mm4, mm3			// here ff where old better, else 00

	pand	mm1, mm3
	pand	mm2, mm3

	pand    mm6, mm4
	pand    mm7, mm4

	por		mm6, mm2			// our x2 value
	por		mm7, mm1			// our x2 diffs
	movq	mm4, mm7			// save as bob uncertainty indicator
}

