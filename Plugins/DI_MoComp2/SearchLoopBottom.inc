/////////////////////////////////////////////////////////////////////////////
// $Id: SearchLoopBottom.inc,v 1.1 2003-01-02 13:15:01 adcockj Exp $
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

// JA 9/Dec/2002
// failed experiment
// but leave in placeholder for me to play about
#ifdef DONT_USE_STRANGE_BOB
        // Use the best weave if diffs less than 10 as that
        // means the image is still or moving cleanly
        // if there is motion we will clip which will catch anything
        psubusb mm7, FOURS          // sets bits to zero if weave diff < 4
        pxor    mm0, mm0
        pcmpeqb mm7, mm0            // all ff where weave better, else 00
        pcmpeqb mm0, mm7            // all ff where bob better, else 00
        pand    mm0, mm6            // use bob for these pixel values
        pand    mm7, mm5            // use weave for these
        por     mm0, mm7            // combine both
#else
        // Use the better of bob or weave
//      pminub  mm4, TENS           // the most we care about
        V_PMINUB (mm4, TENS, mm0)   // the most we care about

        psubusb mm7, mm4            // foregive that much from weave est?
        psubusb mm7, FOURS          // bias it a bit toward weave
        pxor    mm0, mm0
        pcmpeqb mm7, mm0            // all ff where weave better, else 00
        pcmpeqb mm0, mm7            // all ff where bob better, else 00
        pand    mm0, mm6            // use bob for these pixel values
        pand    mm7, mm5            // use weave for these
        por     mm0, mm7            // combine both
#endif


//      pminub  mm0, Max_Vals       // but clip to catch the stray error
        V_PMINUB (mm0, Max_Vals, mm1) // but clip to catch the stray error
//      pmaxub  mm0, Min_Vals
        V_PMAXUB (mm0, Min_Vals)



        mov     eax, pDest


//      movntq  qword ptr[eax+edx], mm0
        V_MOVNTQ (qword ptr[eax+edx], mm0)

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
    
    return 0;
