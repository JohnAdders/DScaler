/////////////////////////////////////////////////////////////////////////////
// $Id: SearchLoopBottom.inc,v 1.2 2003-02-11 15:40:54 adcockj Exp $
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

__asm
{
    // these should be the values held as we go in
    // mm0 = 0
    // mm1 = weave pixels
    // mm3 = "movement" in the centre
    // mm6 = Bob pixels          
    // Save1 = Real Weave 

    movq	mm2, qword ptr[ebx]
    movq	mm4, qword ptr[eax]

    PABS(mm2, mm4, mm5)

    movq	mm4, qword ptr[ebx + ecx]
    movq	mm7, qword ptr[eax + ecx]

    PABS(mm4, mm7, mm5)
    
    // mm0 = 0
    // mm1 = weave pixels
    // mm2 = "movement" in the top
    // mm3 = "movement" in the centre
    // mm4 = "movement" in the top
    // mm6 = Bob pixels          

    psubusb mm2, MOVE           // non-zero where mm2 > TENS i.e. Movement
    movq    mm7, mm2
    pand    mm7, UVMask
    pand    mm2, YMask
    pcmpeqw mm2, mm0            // FFFF where the luma has no movement
    pcmpeqd mm7, mm0            // FFFFFFFF where the Chroma has no movement
    pand    mm2, mm7            // FF where there is no luma or chroma movement
    pcmpeqb mm2, mm0            // all ff where movement by byte

    psubusb mm3, MOVE           // non-zero where mm3 > TENS i.e. Movement
    movq    mm7, mm3
    pand    mm7, UVMask
    pand    mm3, YMask
    pcmpeqw mm3, mm0            // FFFF where the luma has no movement 
    pcmpeqd mm7, mm0            // FFFFFFFF where the Chroma has no movement
    pand    mm3, mm7            // FF where there is no luma or chroma movement
    pcmpeqb mm3, mm0            // all ff where movement by byte

    psubusb mm4, MOVE           // non-zero where mm4 > TENS i.e. Movement
    movq    mm7, mm4
    pand    mm7, UVMask
    pand    mm4, YMask
    pcmpeqw mm4, mm0            // FFFF where the luma has no movement
    pcmpeqd mm7, mm0            // FFFFFFFF where the Chroma has no movement
    pand    mm4, mm7            // FF where there is no luma or chroma movement
    pcmpeqb mm4, mm0            // all ff where movement by byte

    pand  mm2, mm4              // top and bottom moving
    por  mm3, mm2               // where we should bob


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
}
