/////////////////////////////////////////////////////////////////////////////
// $Id: DI_Greedy.asm,v 1.2 2001-07-13 16:13:33 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Tom Barry  All rights reserved.
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
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif


// This is a simple lightweight DeInterlace method that uses little CPU time
// but gives very good results for low or intermedite motion.
// It defers frames by one field, but that does not seem to produce noticeable
// lip sync problems.
//
// The method used is to take either the older or newer weave pixel depending
// upon which give the smaller comb factor, and then clip to avoid large damage
// when wrong.
//
// I'd intended this to be part of a larger more elaborate method added to 
// Blended Clip but this give too good results for the CPU to ignore here.

#if defined(IS_SSE)
BOOL DeinterlaceGreedy_SSE(DEINTERLACE_INFO *info)
#elif defined(IS_3DNOW)
BOOL DeinterlaceGreedy_3DNOW(DEINTERLACE_INFO *info)
#else
BOOL DeinterlaceGreedy_MMX(DEINTERLACE_INFO *info)
#endif
{
    int Line;
    int LoopCtr;
    short* L1;                  // ptr to Line1, of 3
    short* L2;                  // ptr to Line2, the weave line
    short* L3;                  // ptr to Line3
    short* LP2;                 // ptr to prev Line2
    BYTE* Dest;
    BYTE *lpCurOverlay = info->Overlay;
    short **pOddLines = info->OddLines[0];
    short **pEvenLines = info->EvenLines[0];
    short **pPrevLines = info->IsOdd ? info->OddLines[1] : info->EvenLines[1];
#ifdef IS_MMX
    const __int64 ShiftMask = 0xfefffefffefffeff;   // to avoid shifting chroma to luma
#endif
    __int64 MaxComb;
    __int64 i;

    i = GreedyMaxComb;          // How badly do we let it weave? 0-255
    MaxComb = i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i;    
    

    if (pOddLines == NULL || pEvenLines == NULL || pPrevLines == NULL)
        return FALSE;

    // copy first even line no matter what, and the first odd line if we're
    // processing an EVEN field. (note diff from other deint rtns.)
    info->pMemcpy(lpCurOverlay, pEvenLines[0], info->LineLength);   // DL0
    if (!info->IsOdd)
        info->pMemcpy(lpCurOverlay + info->OverlayPitch, pOddLines[0], info->LineLength);  // DL1
    for (Line = 0; Line < (info->FieldHeight - 1); ++Line)
    {
        LoopCtr = info->LineLength / 8;             // there are LineLength / 8 qwords per line

        if (info->IsOdd)
        {
            L1 = pEvenLines[Line];      
            L2 = pOddLines[Line];   
            L3 = pEvenLines[Line + 1];  
            LP2 = pPrevLines[Line];         // prev Odd lines
            Dest = lpCurOverlay + (Line * 2 + 1) * info->OverlayPitch;  // DL1
        }
        else
        {
            L1 = pOddLines[Line] ;      
            L2 = pEvenLines[Line + 1];      
            L3 = pOddLines[Line + 1];   
            LP2 = pPrevLines[Line + 1];         // prev even lines
            Dest = lpCurOverlay + (Line * 2 + 2) * info->OverlayPitch;  // DL2
        }
        info->pMemcpy(Dest + info->OverlayPitch, L3, info->LineLength);

// For ease of reading, the comments below assume that we're operating on an odd
// field (i.e., that info->IsOdd is true).  Assume the obvious for even lines..

        _asm
        {
            mov eax, dword ptr [L1]     
            mov ebx, dword ptr [L2]     
            mov edx, dword ptr [L3]     
            mov esi, dword ptr [LP2]        
            mov edi, dword ptr [Dest]       // DL1 if Odd or DL2 if Even 
            
align 8
MAINLOOP_LABEL:         
            movq    mm1, qword ptr[eax]     // L1
            movq    mm2, qword ptr[ebx]     // L2
            movq    mm3, qword ptr[edx]     // L3
            movq    mm0, qword ptr[esi]     // LP2

            // average L1 and L3 leave result in mm4
            movq    mm4, mm1                // L1
#if defined(IS_SSE)
            pavgb mm4, mm3
#elif defined(IS_3DNOW)
            pavgusb mm4, mm3
#else
            pand    mm4, ShiftMask          // "
            psrlw   mm4, 1
            movq    mm5, mm3                // L3
            pand    mm5, ShiftMask          // "
            psrlw   mm5, 1
            paddb   mm4, mm5                // the average, for computing comb
#endif

// get abs value of possible L2 comb
            movq    mm7, mm2                // L2
            psubusb mm7, mm4                // L2 - avg
            movq    mm5, mm4                // avg
            psubusb mm5, mm2                // avg - L2
            por     mm5, mm7                // abs(avg-L2)
            movq    mm6, mm4                // copy of avg for later

// get abs value of possible LP2 comb
            movq    mm7, mm0                // LP2
            psubusb mm7, mm4                // LP2 - avg
            psubusb mm4, mm0                // avg - LP2
            por     mm4, mm7                // abs(avg-LP2)

// use L2 or LP2 depending upon which makes smaller comb
            psubusb mm4, mm5                // see if it goes to zero
            psubusb mm5, mm5                // 0
            pcmpeqb mm4, mm5                // if (mm4=0) then FF else 0
            pcmpeqb mm5, mm4                // opposite of mm4

// if Comb(LP2) <= Comb(L2) then mm4=ff, mm5=0 else mm4=0, mm5 = 55
            pand    mm5, mm2                // use L2 if mm5 == ff, else 0
            pand    mm4, mm0                // use LP2 if mm4 = ff, else 0
            por     mm4, mm5                // may the best win

// Now lets clip our chosen value to be not outside of the range
// of the high/low range L1-L3 by more than abs(L1-L3)
// This allows some comb but limits the damages and also allows more
// detail than a boring oversmoothed clip.

            movq    mm2, mm1                // copy L1
            psubusb mm2, mm3                // - L3, with saturation
            paddusb mm2, mm3                // now = Max(L1,L3)

            pcmpeqb mm7, mm7                // all ffffffff
            psubusb mm7, mm1                // - L1 
            paddusb mm3, mm7                // add, may sat at fff..
            psubusb mm3, mm7                // now = Min(L1,L3)

// allow the value to be above the high or below the low by amt of MaxComb
            paddusb mm2, MaxComb            // increase max by diff
            psubusb mm3, MaxComb            // lower min by diff

            psubusb mm4, mm3                // best - Min
            paddusb mm4, mm3                // now = Max(best,Min(L1,L3)

            pcmpeqb mm7, mm7                // all ffffffff
            psubusb mm7, mm4                // - Max(best,Min(best,L3) 
            paddusb mm2, mm7                // add may sat at FFF..
            psubusb mm2, mm7                // now = Min( Max(best, Min(L1,L3), L2 )=L2 clipped

#ifdef IS_SSE
            movntq qword ptr[edi], mm2      // move in our clipped best
#else
            movq qword ptr[edi], mm2        // move in our clipped best
#endif

// bump ptrs and loop
            lea     eax,[eax+8]             
            lea     ebx,[ebx+8]
            lea     edx,[edx+8]
            lea     edi,[edi+8]         
            lea     esi,[esi+8]
            dec     LoopCtr
            jnz     MAINLOOP_LABEL
        }
    }

    // Copy last odd line if we're processing an Odd field.
    if (info->IsOdd)
    {
        info->pMemcpy(lpCurOverlay + (info->FrameHeight - 1) * info->OverlayPitch,
                  pOddLines[info->FieldHeight - 1],
                  info->LineLength);
    }

    // clear out the MMX registers ready for doing floating point
    // again
    _asm
    {
        emms
    }
    return TRUE;
}

#undef MAINLOOP_LABEL