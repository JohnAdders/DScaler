/////////////////////////////////////////////////////////////////////////////
// $Id: FD_Common.cpp,v 1.13 2001-07-16 18:07:50 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
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
//
// Refinements made by Mark Rejhon and Steve Grimm
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//
// 02 Jan 2001   John Adcock           Fixed bug at end of GetCombFactor assember
//
// 07 Jan 2001   John Adcock           Fixed PAL detection bug
//                                     Changed GetCombFactor to work on a primary
//                                     and secondary set of fields.
//
// 08 Jan 2001   John Adcock           Global Variable Tidy up
//                                     Got rid of global.h structs.h defines.h
//
// 09 Jan 2001   John Adcock           Split out into new file
//                                     Changed functions to use DEINTERLACE_INFO
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.12  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.11  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "OutThreads.h"
#include "FD_Common.h"
#include "FD_CommonFunctions.h"
#include "DebugLog.h"
#include "Other.h"
#include "SettingsDlg.h"


// Settings
// Default values which can be overwritten by the INI file
long BitShift = 12;
long CombEdgeDetect = 625;
long CombJaggieThreshold = 73;
long DiffThreshold = 224;
BOOL UseChromaInDetect = FALSE;

void CalcCombFactor(DEINTERLACE_INFO* pInfo);
void CalcDiffFactor(DEINTERLACE_INFO* pInfo);
void DoBothCombAndDiff(DEINTERLACE_INFO* pInfo);
void CalcCombFactorChroma(DEINTERLACE_INFO* pInfo);
void CalcDiffFactorChroma(DEINTERLACE_INFO* pInfo);
void DoBothCombAndDiffChroma(DEINTERLACE_INFO* pInfo);

// want to be able to access these from the assembler routines they should
// be together in memory so don't make them const even though they are
extern "C"
{
    __int64 qwYMask = 0x00ff00ff00ff00ff;
    __int64 qwOnes = 0x0001000100010001;
    __int64 qwThreshold;
    __int64 qwBitShift;
}

void PerformFilmDetectCalculations(DEINTERLACE_INFO* pInfo, BOOL NeedComb, BOOL NeedDiff)
{
    if(NeedComb && NeedDiff)
    {
        if(UseChromaInDetect)
        {
            DoBothCombAndDiffChroma(pInfo);
        }
        else
        {
            DoBothCombAndDiff(pInfo);
        }
    }
    else if(NeedComb)
    {
        if(UseChromaInDetect)
        {
            CalcCombFactorChroma(pInfo);        
        }
        else
        {
            CalcCombFactor(pInfo);
        }
    }
    else if(NeedDiff)
    {
        if(UseChromaInDetect)
        {
            CalcDiffFactorChroma(pInfo);
        }
        else
        {
            CalcDiffFactor(pInfo);
        }
    }
}

long CalculateTotalCombFactor(DWORD* Combs, DEINTERLACE_INFO* pInfo)
{
    long CombFactor = 0;
    for (int Line = 17; Line < pInfo->FieldHeight - 17; ++Line)
    {
        if(Combs[Line - 1] > 0 && Combs[Line + 1] > 0)
        {
            if(Combs[Line - 1] < Combs[Line])
            {
                CombFactor += Combs[Line - 1];
            }
            else if(Combs[Line + 1] < Combs[Line])
            {
                CombFactor += Combs[Line + 1];
            }
            else
            {
                CombFactor += Combs[Line];
            }
        }
    }
    return CombFactor;
}

///////////////////////////////////////////////////////////////////////////////
// CalcCombFactor
//
// This routine basically calculates how close the pixels in pSecondaryLines
// are the interpelated pixels between pPrimaryLines
// this idea was taken from the VirtualDub CVideoTelecineRemover class
// at the moment it is the correct algoritm outlined in the comments
// not the one used in that program
// I only do this on the Y component as I assume that any noticable combing
// will be visible in the black and white image
// the relative sizes of the returns from this function will be used to 
// determine the best ordering of the fields
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift Value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
void CalcCombFactor(DEINTERLACE_INFO* pInfo)
{
    int Line;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];

    // If one of the fields is missing, treat them as very different.
    if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL)
    {
        pInfo->CombFactor = 0x7fffffff;
        return;
    }

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        Combs[Line] = CalcCombFactorLine(pInfo->EvenLines[0][Line] + 16,
                                        pInfo->OddLines[0][Line] + 16, 
                                        pInfo->EvenLines[0][Line + 1] + 16,
                                        pInfo->LineLength - 32);
    }

    // Clear out MMX registers before we need to do floating point again
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    LOG(" Frame %d %c CF = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->CombFactor);
    return;
}

void CalcCombFactorChroma(DEINTERLACE_INFO* pInfo)
{
    int Line;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];

    // If one of the fields is missing, treat them as very different.
    if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL)
    {
        pInfo->CombFactor = 0x7fffffff;
        return;
    }

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        Combs[Line] = CalcCombFactorLineChroma(pInfo->EvenLines[0][Line] + 16,
                                        pInfo->OddLines[0][Line] + 16, 
                                        pInfo->EvenLines[0][Line + 1] + 16,
                                        pInfo->LineLength - 32);
    }

    // Clear out MMX registers before we need to do floating point again
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    LOG(" Frame %d %c CF = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->CombFactor);
    return;
}

///////////////////////////////////////////////////////////////////////////////
// CalcDiffFactor
//
// This routine basically calculates how close the pixels in pLines2
// are to the pixels in pLines1
// this is my attempt to implement Mark Rejhon's 3:2 pulldown code
// we will use this to dect the times when we get three fields in a row from
// the same frame
// the result is the total average diffrence between the Y components of each pixel
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift Value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
void CalcDiffFactor(DEINTERLACE_INFO* pInfo)
{
    int Line;
    long DiffFactor = 0;
    short** pLines1;
    short** pLines2;
    
    qwBitShift = BitShift;

    if(pInfo->IsOdd)
    {
        pLines1 = pInfo->OddLines[1];
        pLines2 = pInfo->OddLines[0];
    }
    else
    {
        pLines1 = pInfo->EvenLines[1];
        pLines2 = pInfo->EvenLines[0];
    }

    // If we skipped a field, treat the new one as maximally different.
    if (pLines1 == NULL || pLines2 == NULL)
    {
        pInfo->FieldDiff = 0x7fffffff;
        return;
    }

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        DiffFactor += CalcDiffFactorLine(pLines1[Line] + 16,
                                                pLines2[Line] + 16,
                                                pInfo->LineLength - 32);
    }

    _asm
    {
        emms
    }

    pInfo->FieldDiff = DiffFactor;
    LOG(" Frame %d %c FD = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->FieldDiff);
}

///////////////////////////////////////////////////////////////////////////////
// CalcDiffFactorChroma
//
// This routine basically calculates how close the pixels in pLines2
// are to the pixels in pLines1
// this is my attempt to implement Mark Rejhon's 3:2 pulldown code
// we will use this to dect the times when we get three fields in a row from
// the same frame
// the result is the total average diffrence between the Y components of each pixel
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift Value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
void CalcDiffFactorChroma(DEINTERLACE_INFO* pInfo)
{
    int Line;
    long DiffFactor = 0;
    short** pLines1;
    short** pLines2;
    
    qwBitShift = BitShift;

    if(pInfo->IsOdd)
    {
        pLines1 = pInfo->OddLines[1];
        pLines2 = pInfo->OddLines[0];
    }
    else
    {
        pLines1 = pInfo->EvenLines[1];
        pLines2 = pInfo->EvenLines[0];
    }

    // If we skipped a field, treat the new one as maximally different.
    if (pLines1 == NULL || pLines2 == NULL)
    {
        pInfo->FieldDiff = 0x7fffffff;
        return;
    }

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        DiffFactor += CalcDiffFactorLineChroma(pLines1[Line] + 16,
                                                    pLines2[Line] + 16,
                                                    pInfo->LineLength - 32);
    }

    _asm
    {
        emms
    }

    pInfo->FieldDiff = DiffFactor;
    LOG(" Frame %d %c FD = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->FieldDiff);
}

void DoBothCombAndDiff(DEINTERLACE_INFO* pInfo)
{
    int Line;
    long DiffFactor = 0;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);
    qwBitShift = BitShift;
    
    if(pInfo->IsOdd)
    {
        // If one of the fields is missing, treat them as very different.
        if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL || pInfo->OddLines[1] == NULL)
        {
            pInfo->CombFactor = 0x7fffffff;
            pInfo->FieldDiff = 0x7fffffff;
            return;
        }
        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLine(pInfo->EvenLines[0][Line] + 16,
                                            pInfo->OddLines[0][Line] + 16, 
                                            pInfo->EvenLines[0][Line + 1] + 16,
                                            pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLine(pInfo->OddLines[0][Line] + 16,
                                                        pInfo->OddLines[1][Line] + 16,
                                                        pInfo->LineLength - 32);
        }
    }
    else
    {
        // If one of the fields is missing, treat them as very different.
        if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL || pInfo->EvenLines[1] == NULL)
        {
            pInfo->CombFactor = 0x7fffffff;
            pInfo->FieldDiff = 0x7fffffff;
            return;
        }
        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLine(pInfo->EvenLines[0][Line] + 16,
                                            pInfo->OddLines[0][Line] + 16, 
                                            pInfo->EvenLines[0][Line + 1] + 16,
                                            pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLine(pInfo->EvenLines[0][Line] + 16,
                                                        pInfo->EvenLines[1][Line] + 16,
                                                        pInfo->LineLength - 32);
        }
    }
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    pInfo->FieldDiff = DiffFactor;
    LOG(" Frame %d %c FD = %d \t CF = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->FieldDiff, pInfo->CombFactor);
}

void DoBothCombAndDiffChroma(DEINTERLACE_INFO* pInfo)
{
    int Line;
    DWORD DiffFactor = 0;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);
    qwBitShift = BitShift;
    
    if(pInfo->IsOdd)
    {
        // If one of the fields is missing, treat them as very different.
        if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL || pInfo->OddLines[1] == NULL)
        {
            pInfo->CombFactor = 0x7fffffff;
            pInfo->FieldDiff = 0x7fffffff;
            return;
        }
        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLineChroma(pInfo->EvenLines[0][Line] + 16,
                                            pInfo->OddLines[0][Line] + 16, 
                                            pInfo->EvenLines[0][Line + 1] + 16,
                                            pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLineChroma(pInfo->OddLines[0][Line] + 16,
                                                        pInfo->OddLines[1][Line] + 16,
                                                        pInfo->LineLength - 32);
        }
    }
    else
    {
        // If one of the fields is missing, treat them as very different.
        if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL || pInfo->EvenLines[1] == NULL)
        {
            pInfo->CombFactor = 0x7fffffff;
            pInfo->FieldDiff = 0x7fffffff;
            return;
        }
        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLineChroma(pInfo->EvenLines[0][Line] + 16,
                                            pInfo->OddLines[0][Line] + 16, 
                                            pInfo->EvenLines[0][Line + 1] + 16,
                                            pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLineChroma(pInfo->EvenLines[0][Line] + 16,
                                                        pInfo->EvenLines[1][Line] + 16,
                                                        pInfo->LineLength - 32);
        }
    }
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    pInfo->FieldDiff = DiffFactor;
    LOG(" Frame %d %c FD = %d \t CF = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->FieldDiff, pInfo->CombFactor);
}

void DoBothCombAndDiffExperimental(DEINTERLACE_INFO* pInfo)
{
    int Line;
    int LoopCtr;
    short* L1;     // ptr to Line1, of 3
    short* L2;     // ptr to Line2, the weave line
    short* L3;     // ptr to Line3
    short* LP2;     // ptr to prev Line2
    short** pOddLines = pInfo->OddLines[0];
    short** pEvenLines = pInfo->EvenLines[0];
    short** pPrevLines = pInfo->IsOdd ? pInfo->OddLines[1] : pInfo->EvenLines[1];
    const __int64 qwShiftMask = 0xfefffefffefffeff; // to avoid shifting chroma to luma
    const __int64 qwOnes = 0x0001000100010001;
    __int64 qwThresholdWeave;
    __int64 qwThresholdDiff;
    __int64 i;
    unsigned long DiffFactor = 0;
    unsigned long WeaveFactor = 0;

    i = 20; // what is different
    qwThresholdWeave = i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i;
    i = 10; // what is different
    qwThresholdDiff = i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i;


    if (pOddLines == NULL || pEvenLines == NULL || pPrevLines == NULL)
        return;

    for (Line = 16; Line < (pInfo->FieldHeight - 16); ++Line)
    {
        LoopCtr = pInfo->LineLength / 8;    // there are LineLength / 8 qwords per line

        if (pInfo->IsOdd)
        {
            L1 = pEvenLines[Line];
            L2 = pOddLines[Line];
            L3 = pEvenLines[Line + 1];
            LP2 = pPrevLines[Line];   // prev Odd lines
        }
        else
        {
            L1 = pOddLines[Line] ;
            L2 = pEvenLines[Line + 1];
            L3 = pOddLines[Line + 1];
            LP2 = pPrevLines[Line + 1];   // prev even lines
        }

        _asm
        {
            mov eax, dword ptr [L1]
            mov ebx, dword ptr [L2]
            mov edx, dword ptr [L3]
            mov esi, dword ptr [LP2]
            mov ecx, LoopCtr
            pxor mm7,mm7
            pxor mm6,mm6
            pxor mm5,mm5

            align 8
            MAINLOOP_LABEL:
            movq mm0, qword ptr[eax]  // L1
            movq mm1, qword ptr[edx]  // L3

            // average L1 and L3 leave result in mm0
            pand mm0, qwShiftMask   // L1
            psrlw mm0, 1
            pand mm1, qwShiftMask   // L3
            psrlw   mm1, 1
            paddb   mm0, mm1    // the average, for computing comb

            movq mm1, qword ptr[ebx]  // L2
            // get abs Value of possible L2 comb answer in mm0
            // mm2 will have L2 in it
            movq mm2, mm1    // L2
            psubusb mm1, mm0    // L2 - avg
            psubusb mm0, mm2    // avg - L2
            por  mm0, mm1    // abs(avg-L2)

            pand mm0, qwShiftMask
            psrlw mm0, 1           // abs(avg-L2)/2
            pcmpgtb mm0, qwThresholdWeave
            pand mm0, qwOnes       // 1 if abs(avg-L2)/2 > Threshold
            paddusb mm7, mm0                // Count of all times the comb threshold
                                    // has been exceeded

            movq mm1, qword ptr[esi]     // LP2
            movq mm0, mm1    // L2
            psubusb mm1, mm2    // L2 - LP2
            psubusb mm2, mm0    // LP2 - L2
            por  mm1, mm2    // abs(LP2-L2)

            pand mm1, qwShiftMask
            psrlw mm1, 1           // abs(avg-L2)/2
            pcmpgtb mm1, qwThresholdDiff
            pand mm1, qwOnes       // 1 if abs(avg-L2)/2 > Threshold
            paddusb mm6, mm1                // Count of all times the diff threshold

            // bump ptrs and loop
            lea  eax,[eax+8]
            lea  ebx,[ebx+8]
            lea  edx,[edx+8]
            lea  esi,[esi+8]
            loop  MAINLOOP_LABEL

            // OK so now we have mm6 and mm7 as out totals
            xor edx, edx
            xor ebx, ebx
            movd eax, mm6
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            psrlq mm6,32
            movd eax, mm6
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            add DiffFactor, ebx

            xor edx, edx
            xor ebx, ebx
            movd eax, mm7
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            psrlq mm7,32
            movd eax, mm7
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            shr eax, 8
            mov dl, al
            add ebx, edx
            add WeaveFactor, ebx
        }
    }

    pInfo->CombFactor = WeaveFactor;
    pInfo->FieldDiff = DiffFactor;

    LOG(" Frame %d %c FD = %d CF = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->FieldDiff, pInfo->CombFactor);

    // clear out the MMX registers ready for doing floating point
    // again
    _asm
    {
        emms
    }
    return;
}


///////////////////////////////////////////////////////////////////////////////
// Simple Weave.  Copies alternating scanlines from the most recent fields.
BOOL Weave(DEINTERLACE_INFO* pInfo)
{
    int i;
    BYTE* lpOverlay = pInfo->Overlay;

    if (pInfo->EvenLines[0] == NULL || pInfo->OddLines[0] == NULL)
        return FALSE;

    for (i = 0; i < pInfo->FieldHeight; i++)
    {
        pInfo->pMemcpy(lpOverlay, pInfo->EvenLines[0][i], pInfo->LineLength);
        lpOverlay += pInfo->OverlayPitch;

        pInfo->pMemcpy(lpOverlay, pInfo->OddLines[0][i], pInfo->LineLength);
        lpOverlay += pInfo->OverlayPitch;
    }
    _asm
    {
        emms
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Simple Bob.  Copies the most recent field to the overlay, with each scanline
// copied twice.
/////////////////////////////////////////////////////////////////////////////
BOOL Bob(DEINTERLACE_INFO* pInfo)
{
    int i;
    BYTE* lpOverlay = pInfo->Overlay;
    short** lines;
 
    // If field is odd we will offset it down 1 line to avoid jitter  TRB 1/21/01
    if (pInfo->IsOdd)
    {
        lines = pInfo->OddLines[0];
        // No recent data?  We can't do anything.
        if (lines == NULL)
            return FALSE;

        if (pInfo->CpuFeatureFlags & FEATURE_SSE)
        {
            memcpySSE(lpOverlay, lines[0], pInfo->LineLength);   // extra copy of first line
            lpOverlay += pInfo->OverlayPitch;                    // and offset out output ptr
            for (i = 0; i < pInfo->FieldHeight - 1; i++)
            {
                memcpyBOBSSE(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    lines[i], pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
            }
            memcpySSE(lpOverlay, lines[i], pInfo->LineLength);   // only 1 copy of last line
        }
        else
        {
            memcpyMMX(lpOverlay, lines[0], pInfo->LineLength);   // extra copy of first line
            lpOverlay += pInfo->OverlayPitch;                    // and offset out output ptr
            for (i = 0; i < pInfo->FieldHeight - 1; i++)
            {
                memcpyBOBMMX(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    lines[i], pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
            }
            memcpyMMX(lpOverlay, lines[i], pInfo->LineLength);   // only 1 copy of last line
        }
    }   
    else
    {
        lines = pInfo->EvenLines[0];
        if (lines == NULL)
                return FALSE;
        if (pInfo->CpuFeatureFlags & FEATURE_SSE)
        {
            for (i = 0; i < pInfo->FieldHeight; i++)
            {
                memcpyBOBSSE(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    lines[i], pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
            }
        }
        else
        {
            for (i = 0; i < pInfo->FieldHeight; i++)
            {
                memcpyBOBMMX(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    lines[i], pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
            }
        }
    }
    // need to clear up MMX registers
    _asm
    {
        emms
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FD_CommonSettings[FD_COMMON_SETTING_LASTONE] =
{
    {
        "Bit Shift", SLIDER, 0, &BitShift,
        12, 0, 15, 1, 1,
        NULL,
        "Pulldown", "BitShift", NULL,

    },
    {
        "Comb Edge Detect", SLIDER, 0, &CombEdgeDetect,
        625, 0, 10000, 5, 1,
        NULL,
        "Pulldown", "EdgeDetect", NULL,

    },
    {
        "Comb Jaggie Threshold", SLIDER, 0, &CombJaggieThreshold,
        73, 0, 5000, 5, 1,
        NULL,
        "Pulldown", "JaggieThreshold", NULL,

    },
    {
        "DiffThreshold", SLIDER, 0, &DiffThreshold,
        224, 0, 5000, 5, 1,
        NULL,
        "Pulldown", "DiffThreshold", NULL,

    },
    {
        "Chroma Detect", ONOFF, 0, (long*)&UseChromaInDetect,
        0, 0, 1, 1, 1,
        NULL,
        "Pulldown", "UseChroma", NULL,

    },
};

SETTING* FD_Common_GetSetting(FD_COMMON_SETTING Setting)
{
    if(Setting > -1 && Setting < FD_COMMON_SETTING_LASTONE)
    {
        return &(FD_CommonSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void FD_Common_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < FD_COMMON_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(FD_CommonSettings[i]));
    }
}

void FD_Common_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < FD_COMMON_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(FD_CommonSettings[i]), bOptimizeFileAccess);
    }
}

void FD_Common_SetMenu(HMENU hMenu)
{
    CheckMenuItemBool(hMenu, IDM_USECHROMA, UseChromaInDetect);
}

void FD_Common_ShowUI()
{
    CSettingsDlg::ShowSettingsDlg("Pulldown Shared Settings",FD_CommonSettings, FD_COMMON_SETTING_LASTONE);
}
