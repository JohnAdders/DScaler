    /////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Revision 1.3  2003/05/26 15:54:36  adcockj
// Improved Performance of motion detection
// Fixed bug with pBobP
//
// Revision 1.2  2003/02/11 15:40:54  adcockj
// New implementation of StrangeBob based off paper
// Tidy up sources
//
// Revision 1.1  2003/01/02 13:15:01  adcockj
// Added new plug-ins ready for developement by copying TomsMoComp and Gamma
//
/////////////////////////////////////////////////////////////////////////////

__asm
{
    // these should be the values held as we go in
    // mm0 = 0
    // mm1 = weave pixels
    // mm3 = "movement" in the centre
    // mm6 = Bob pixels          

    // operate only on luma as we will always bob the chroma
    pand    mm3, YMask

    movq	mm2, qword ptr[ebx]
    movq	mm4, qword ptr[eax]

    PABS(mm2, mm4, mm5)
    pand    mm2, YMask

    movq	mm4, qword ptr[ebx + ecx]
    movq	mm7, qword ptr[eax + ecx]

    PABS(mm4, mm7, mm5)
    pand    mm4, YMask
    
    // mm0 = 0
    // mm1 = weave pixels
    // mm2 = "movement" in the top
    // mm3 = "movement" in the centre
    // mm4 = "movement" in the bottom
    // mm6 = Bob pixels          

    psubusb mm2, MOVE           // non-zero where mm2 > MOVE i.e. Movement
    pcmpeqd mm2, mm0            // FFFF where the luma has no movement in two pixels
    pcmpeqd mm2, mm0            // all ff where movement in either of the two pixels

    psubusb mm3, MOVE           // non-zero where mm3 > TENS i.e. Movement
    pcmpeqd mm3, mm0            // FFFF where the luma has no movement in two pixels
    pcmpeqd mm3, mm0            // all ff where movement in either of the two pixels

    psubusb mm4, MOVE           // non-zero where mm4 > TENS i.e. Movement
    pcmpeqd mm4, mm0            // FFFF where the luma has no movement in two pixels
    pcmpeqd mm4, mm0            // all ff where movement in either of the two pixels

    pand  mm2, mm4              // top and bottom moving
    por  mm3, mm2               // where we should bob

    por mm3, UVMask

    COMBINE(mm6, mm1, mm3)

    mov     eax, pDest

    V_MOVNTQ (qword ptr[eax+edx], mm6)

    lea     edx, [edx+8]        // bump offset pointer
    cmp     edx, Last8          // done with line?
    jb      LoopQ               // y
    
    }   

    // adjust for next line
    pSrc  += src_pitch2;
    pSrcP += src_pitch2;
    pDest += dst_pitch2;
    pBob += src_pitch2;
    pBobP += src_pitch2;
}
