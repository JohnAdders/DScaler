/////////////////////////////////////////////////////////////////////////////
// $Id: DI_Greedy2Frame.asm,v 1.8 2001-11-23 17:18:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock, Tom Barry, Steve Grimm  All rights reserved.
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
// Revision 1.7  2001/11/22 22:27:00  adcockj
// Bug Fixes
//
// Revision 1.6  2001/11/21 15:21:40  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.5  2001/07/31 06:48:33  adcockj
// Fixed index bug spotted by Peter Gubanov
//
// Revision 1.4  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

// This is the implementation of the Greedy 2-frame deinterlace algorithm described in
// DI_Greedy2Frame.c.  It's in a separate file so we can compile variants for different
// CPU types; most of the code is the same in the different variants.

#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif

///////////////////////////////////////////////////////////////////////////////
// Field 1 | Field 2 | Field 3 | Field 4 |
//   T0    |         |    T1   |         | 
//         |   M0    |         |    M1   | 
//   B0    |         |    B1   |         | 
//


// debugging feature
// output the value of mm4 at this point which is pink where we will weave
// and green were we are going to bob
// uncomment next line to see this
//#define CHECK_BOBWEAVE

#if defined(IS_SSE)
BOOL DeinterlaceGreedy2Frame_SSE(TDeinterlaceInfo* pInfo)
#elif defined(IS_3DNOW)
BOOL DeinterlaceGreedy2Frame_3DNOW(TDeinterlaceInfo* pInfo)
#else
BOOL DeinterlaceGreedy2Frame_MMX(TDeinterlaceInfo* pInfo)
#endif
{
    int Line;
    BYTE* M1;
    BYTE* M0;
    BYTE* T0;
    BYTE* T1;
    BYTE* B1;
    BYTE* B0;
	BYTE* B0UseInAsm;
    DWORD OldSI;
    DWORD OldSP;
    BYTE* Dest = pInfo->Overlay;
    BYTE* Dest2;
    DWORD Pitch = pInfo->InputPitch;
    DWORD LineLength = pInfo->LineLength;

    const __int64 YMask    = 0x00ff00ff00ff00ff;

    __int64 qwGreedyTwoFrameThreshold;
    const __int64 Mask = 0x7f7f7f7f7f7f7f7f;
    const __int64 DwordOne = 0x0000000100000001;    
    const __int64 DwordTwo = 0x0000000200000002;    

    qwGreedyTwoFrameThreshold = GreedyTwoFrameThreshold;
    qwGreedyTwoFrameThreshold += (GreedyTwoFrameThreshold2 << 8);
    qwGreedyTwoFrameThreshold += (qwGreedyTwoFrameThreshold << 48) +
                                (qwGreedyTwoFrameThreshold << 32) + 
                                (qwGreedyTwoFrameThreshold << 16);


    if(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
    {
        M1 = pInfo->PictureHistory[0]->pData;
        T1 = pInfo->PictureHistory[1]->pData;
        B1 = T1 + Pitch;
        M0 = pInfo->PictureHistory[2]->pData;
        T0 = pInfo->PictureHistory[3]->pData;
        B0 = T0 + Pitch;
    }
    else
    {
        M1 = pInfo->PictureHistory[0]->pData + Pitch;
        T1 = pInfo->PictureHistory[1]->pData;
        B1 = T1 + Pitch;
        M0 = pInfo->PictureHistory[2]->pData + Pitch;
        T0 = pInfo->PictureHistory[3]->pData;
        B0 = T0 + Pitch;

        pInfo->pMemcpy(Dest, pInfo->PictureHistory[0]->pData, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;
    }

    for (Line = 0; Line < pInfo->FieldHeight - 1; ++Line)
    {
        // Always use the most recent data verbatim.  By definition it's correct (it'd
        // be shown on an interlaced display) and our job is to fill in the spaces
        // between the new lines.
        pInfo->pMemcpy(Dest, T1, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;
        Dest2 = Dest;

		B0UseInAsm = B0;
        _asm
        {
            // We'll be using a couple registers that have meaning in the C code, so
            // save them.
            mov OldSI, esi
            mov OldSP, esp

            // Figure out what to do with the scanline above the one we just copied.
            // See above for a description of the algorithm.

            mov ecx, LineLength
            mov eax, dword ptr [T1]     
            mov ebx, dword ptr [M1]     
            mov edx, dword ptr [B1]     
            mov esi, dword ptr [M0]     
            mov esp, dword ptr [T0]
            shr ecx, 3                      // there are LineLength / 8 qwords
            movq    mm6, Mask

align 8
MAINLOOP_LABEL:

            mov edi, dword ptr [B0UseInAsm]
            movq    mm1, qword ptr[eax]     // T1
            movq    mm0, qword ptr[ebx]     // M1
            movq    mm3, qword ptr[edx]     // B1
            movq    mm2, qword ptr[esi]     // M0

            // Average T1 and B1 so we can do interpolated bobbing if we bob onto T1.
            movq mm7, mm3                   // mm7 = B1

#if defined(IS_SSE)
            pavgb mm7, mm1
#elif defined(IS_3DNOW)
            pavgusb mm7, mm1
#else
            movq mm5, mm1                   // mm5 = T1
            psrlw mm7, 1                    // mm7 = B1 / 2
            pand mm7, mm6                   // mask off lower bits
            psrlw mm5, 1                    // mm5 = T1 / 2
            pand mm5, mm6                   // mask off lower bits
            paddw mm7, mm5                  // mm7 = (T1 + B1) / 2
#endif

            // calculate |M1-M0| put result in mm4 need to keep mm0 intact
            // if we have a good processor then make mm0 the average of M1 and M0
            // which should make weave look better when there is small amounts of
            // movement
#if defined(IS_SSE)
            movq    mm4, mm0
            movq    mm5, mm2
            psubusb mm4, mm2
            psubusb mm5, mm0
            por     mm4, mm5
            psrlw   mm4, 1
            pavgb mm0, mm2
            pand    mm4, mm6
#elif defined(IS_3DNOW)
            movq    mm4, mm0
            movq    mm5, mm2
            psubusb mm4, mm2
            psubusb mm5, mm0
            por     mm4, mm5
            psrlw   mm4, 1
            pavgusb mm0, mm2
            pand    mm4, mm6
#else
            movq    mm4, mm0
            psubusb mm4, mm2
            psubusb mm2, mm0
            por     mm4, mm2
            psrlw   mm4, 1
            pand    mm4, mm6
#endif


            // if |M1-M0| > Threshold we want dword worth of twos
            pcmpgtb mm4, qwGreedyTwoFrameThreshold
            pand    mm4, Mask               // get rid of any sign bit
            pcmpgtd mm4, DwordOne           // do we want to bob
            pandn   mm4, DwordTwo

            movq    mm2, qword ptr[esp]     // mm2 = T0

            // calculate |T1-T0| put result in mm5
            movq    mm5, mm2
            psubusb mm5, mm1
            psubusb mm1, mm2
            por     mm5, mm1
            psrlw   mm5, 1
            pand    mm5, mm6

            // if |T1-T0| > Threshold we want dword worth of ones
            pcmpgtb mm5, qwGreedyTwoFrameThreshold
            pand    mm5, mm6                // get rid of any sign bit
            pcmpgtd mm5, DwordOne           
            pandn   mm5, DwordOne
            paddd mm4, mm5

            movq    mm2, qword ptr[edi]     // B0

            // calculate |B1-B0| put result in mm5
            movq    mm5, mm2
            psubusb mm5, mm3
            psubusb mm3, mm2
            por     mm5, mm3
            psrlw   mm5, 1
            pand    mm5, mm6

            // if |B1-B0| > Threshold we want dword worth of ones
            pcmpgtb mm5, qwGreedyTwoFrameThreshold
            pand    mm5, mm6                // get rid of any sign bit
            pcmpgtd mm5, DwordOne
            pandn   mm5, DwordOne
            paddd mm4, mm5

            // Get the dest pointer.
            add edi, 8
            mov dword ptr[B0UseInAsm], edi
            mov edi, dword ptr[Dest2]

            pcmpgtd mm4, DwordTwo

// debugging feature
// output the value of mm4 at this point which is pink where we will weave
// and green were we are going to bob
#ifdef CHECK_BOBWEAVE
#ifdef IS_SSE
            movntq qword ptr[edi], mm4
#else
            movq qword ptr[edi], mm4
#endif
#else

            movq mm5, mm4
             // mm4 now is 1 where we want to weave and 0 where we want to bob
            pand    mm4, mm0                
            pandn   mm5, mm7                
            por     mm4, mm5                
#ifdef IS_SSE
            movntq qword ptr[edi], mm4
#else
            movq qword ptr[edi], mm4
#endif
#endif
            // Advance to the next set of pixels.
            add edi, 8
            add eax, 8
            add ebx, 8
            add edx, 8
            mov dword ptr[Dest2], edi
            add esi, 8
            add esp, 8
            dec ecx
            jne near MAINLOOP_LABEL

            mov esi, OldSI
            mov esp, OldSP
        }
        Dest += pInfo->OverlayPitch;

        M1 += Pitch;
        T1 += Pitch;
        B1 += Pitch;
        M0 += Pitch;
        T0 += Pitch;
        B0 += Pitch;
    }

    if(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
    {
        pInfo->pMemcpy(Dest, T1, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;
        pInfo->pMemcpy(Dest, M1, pInfo->LineLength);
    }
    else
    {
        pInfo->pMemcpy(Dest, T1, pInfo->LineLength); 
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