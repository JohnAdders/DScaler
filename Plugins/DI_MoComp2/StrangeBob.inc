/////////////////////////////////////////////////////////////////////////////
// $Id: StrangeBob.inc,v 1.2 2003-02-11 15:41:05 adcockj Exp $
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
// Revision 1.1  2003/01/02 13:15:01  adcockj
// Added new plug-ins ready for developement by copying TomsMoComp and Gamma
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// This file contains an implementation of the alogorithm described in:
// T. Chen, H.R. Wu; & Z.H. Yu, 
// "Efficient deinterlacing algorithm using edge-based line average interpolation", 
// Optical Engineering, Volume 39, Issue 8, (2000)
/////////////////////////////////////////////////////////////////////////////

__asm
{
	
	// First, get and save our possible Bob values
	// Assume our pixels are layed out as follows with x the calc'd bob value
	// and the other pixels are from the current field
	//  
	//		   a b c 		current field
	//           x			calculated line
	//         d e f 		current field
	//

	movq    mm1, qword ptr[ebx-2]
	movq    mm2, qword ptr[ebx]
	movq    mm3, qword ptr[ebx + 2]

	movq    mm4, qword ptr[ebx+ecx - 2]
	movq    mm5, qword ptr[ebx+ecx]
	movq    mm6, qword ptr[ebx+ecx + 2]

    movq mm7, mm1
    V_PAVGB(mm7, mm6, mm0, ShiftMask)
    movq Save1, mm7
    
    movq mm7, mm3
    V_PAVGB(mm7, mm4, mm0, ShiftMask)
    movq Save2, mm7

    movq mm7, mm1
    PABS(mm1, mm6, mm0)
    PABS(mm6, mm2, mm0)
    PABS(mm7, mm5, mm0)
    V_PAVGB(mm6, mm7, mm0, ShiftMask)

    movq mm7, mm3
    PABS(mm3, mm4, mm0)
    PABS(mm4, mm2, mm0)
    PABS(mm7, mm5, mm0)
    V_PAVGB(mm4, mm7, mm0, ShiftMask)

    movq mm7, mm2
    PABS(mm2, mm5, mm0)
    V_PAVGB(mm5, mm7, mm0, ShiftMask)

    // at this point the  following registers are set
    // mm1 = |a - f|
    // mm2 = |b - e|
    // mm3 = |c - d|
    // mm4 = avg(|b - d|, |c - e|)
    // mm5 = avg(b, e)
    // mm6 = avg(|a - e|, |b - f|)
    // Save1 = avg(a, f)
    // Save2 = avg(c, d)
    
	pxor	mm7, mm7			
	pxor	mm0, mm0			
	psubusb mm4, mm6			// nonzero where mm4 > mm6 
	pcmpeqb mm4, mm0			// now ff where mm4 <= mm6
    movq    mm6, mm4
    pcmpeqb mm6, mm0            // now ff where mm6 < mm4


    // make the corresponding one unsuitable
    paddusb mm1, mm4
    paddusb mm3, mm6

    movq mm4, mm1
    movq mm6, mm3
    psubusb mm1, mm3            // nonzero  where (|a - f| > |c - d|)
	pcmpeqb mm1, mm0			// now ff where (|a - f| < |c - d|)
    movq mm7, mm1
    COMBINE(mm4, mm6, mm1)
    movq mm6, Save1
    movq mm3, Save2
    COMBINE(mm6, mm3, mm7)     

    // so now we should have mm4 as current best diff and mm6 as best average
    // of the diagonals
    psubusb mm4, mm2            // nonzero  where (Best > |b - e|)
	pcmpeqb mm4, mm0			// now ff where (Best <= |b - e|)
    COMBINE(mm6, mm5, mm4)     
    pand mm6, YMask
    pand mm5, UVMask
    por mm6, mm5
}

