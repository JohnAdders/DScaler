/////////////////////////////////////////////////////////////////////////////
// $Id: FD_Common.cpp,v 1.28 2003-04-15 13:05:35 adcockj Exp $
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
//                                     Changed functions to use TDeinterlaceInfo
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.27  2002/10/29 11:05:28  adcockj
// Renamed CT2388x to CX2388x
//
// Revision 1.26  2002/09/11 18:19:41  adcockj
// Prelimainary support for CX2388x based cards
//
// Revision 1.25  2002/07/28 08:11:27  laurentg
// "Include Chroma in Detection" removed from the menus
//
// Revision 1.24  2002/06/13 12:10:22  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.23  2001/11/23 19:33:14  adcockj
// Fixes to bob to make is less jittery
//
// Revision 1.22  2001/11/23 10:49:17  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.21  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.20  2001/11/21 15:21:39  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.19  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.18  2001/09/05 15:08:43  adcockj
// Updated Loging
//
// Revision 1.17  2001/08/09 21:42:34  adcockj
// Removed LOGD
//
// Revision 1.16  2001/08/08 18:24:13  adcockj
// Fixed film delay bug
//
// Revision 1.15  2001/08/08 08:54:32  adcockj
// Added Delay option to film modes
// Switched comb modes to use greedy (low) on bad cadence instead of doings it's own thing
//
// Revision 1.14  2001/08/02 16:43:05  adcockj
// Added Debug level to LOG function
//
// Revision 1.13  2001/07/16 18:07:50  adcockj
// Added Optimisation parameter to ini file saving
//
// Revision 1.12  2001/07/13 16:14:56  adcockj
// Changed lots of variables to match Coding standards
//
// Revision 1.11  2001/07/12 16:16:40  adcockj
// Added CVS Id and Log
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "OutThreads.h"
#include "FD_Common.h"
#include "FD_CommonFunctions.h"
#include "DebugLog.h"
#include "Other.h"


// Settings
// Default values which can be overwritten by the INI file
long BitShift = 12;
long CombEdgeDetect = 625;
long CombJaggieThreshold = 73;
long DiffThreshold = 224;
BOOL UseChromaInDetect = FALSE;
long FilmFlipDelay = 0;

void CalcCombFactor(TDeinterlaceInfo* pInfo);
void CalcDiffFactor(TDeinterlaceInfo* pInfo);
void DoBothCombAndDiff(TDeinterlaceInfo* pInfo);
void CalcCombFactorChroma(TDeinterlaceInfo* pInfo);
void CalcDiffFactorChroma(TDeinterlaceInfo* pInfo);
void DoBothCombAndDiffChroma(TDeinterlaceInfo* pInfo);

// want to be able to access these from the assembler routines they should
// be together in memory so don't make them const even though they are
extern "C"
{
    __int64 qwYMask = 0x00ff00ff00ff00ff;
    __int64 qwMask = 0x7f7f7f7f7f7f7f7f;
    __int64 qwOnes = 0x0001000100010001;
    __int64 qwThreshold;
    __int64 qwBitShift;
}

void PerformFilmDetectCalculations(TDeinterlaceInfo* pInfo, BOOL NeedComb, BOOL NeedDiff)
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

// performs a full compare over the progressive frames
// store the result of the diff in FieldDiff
void PerformProgFilmDetectCalculations(TDeinterlaceInfo* pInfo)
{
    // If we skipped a field, treat the new one as maximally different.
    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[1] == NULL ||
        pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_MASK ||
        pInfo->PictureHistory[1]->Flags & PICTURE_INTERLACED_MASK)
    {
        pInfo->FieldDiff = 0x7fffffff;
        return;
    }

    int Line;
    long DiffFactor = 0;
    DWORD Pitch = pInfo->InputPitch;
    
    qwBitShift = BitShift;

    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData + 32 * Pitch;
    BYTE* PreviousLine = pInfo->PictureHistory[1]->pData + 32 * Pitch;

    for (Line = 32; Line < pInfo->FieldHeight - 32; ++Line)
    {
        DiffFactor += CalcDiffFactorLine(CurrentLine + 32,
                                            PreviousLine + 32,
                                            pInfo->LineLength - 64);
        CurrentLine += Pitch;
        PreviousLine += Pitch;
    }

    _asm
    {
        emms
    }

    pInfo->FieldDiff = DiffFactor;
    LOG(2, "Frame %d FD = %d", pInfo->CurrentFrame, pInfo->FieldDiff);
}


long CalculateTotalCombFactor(DWORD* Combs, TDeinterlaceInfo* pInfo)
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
void CalcCombFactor(TDeinterlaceInfo* pInfo)
{
    // If one of the fields is missing, treat them as very different.
    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[1] == NULL)
    {
        pInfo->CombFactor = 0x7fffffff;
        return;
    }

    int Line;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];
    DWORD Pitch = pInfo->InputPitch;
    BYTE* EvenLine;
    BYTE* OddLine;
    BOOL IsOdd((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

    if(IsOdd)
    {
        EvenLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
        OddLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
    }
    else
    {
        EvenLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
        OddLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
    }

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        Combs[Line] = CalcCombFactorLine(EvenLine + 16,
                                        OddLine + 16, 
                                        EvenLine + Pitch + 16,
                                        pInfo->LineLength - 32);
        EvenLine += Pitch;
        OddLine += Pitch;
    }

    // Clear out MMX registers before we need to do floating point again
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    LOG(2, "Frame %d %c CF = %d", pInfo->CurrentFrame, IsOdd ? 'O' : 'E', pInfo->CombFactor);
    return;
}

void CalcCombFactorChroma(TDeinterlaceInfo* pInfo)
{
    // If one of the fields is missing, treat them as very different.
    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[1] == NULL)
    {
        pInfo->CombFactor = 0x7fffffff;
        return;
    }

    int Line;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];
    DWORD Pitch = pInfo->InputPitch;
    BYTE* EvenLine;
    BYTE* OddLine;
    BOOL IsOdd((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

    if(IsOdd)
    {
        EvenLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
        OddLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
    }
    else
    {
        EvenLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
        OddLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
    }

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        Combs[Line] = CalcCombFactorLineChroma(EvenLine + 16,
                                        OddLine + 16, 
                                        EvenLine + Pitch + 16,
                                        pInfo->LineLength - 32);
        EvenLine += Pitch;
        OddLine += Pitch;
    }

    // Clear out MMX registers before we need to do floating point again
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    LOG(2, "Frame %d %c CF = %d", pInfo->CurrentFrame, IsOdd ? 'O' : 'E', pInfo->CombFactor);
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
void CalcDiffFactor(TDeinterlaceInfo* pInfo)
{
    // If we skipped a field, treat the new one as maximally different.
    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[2] == NULL)
    {
        pInfo->FieldDiff = 0x7fffffff;
        return;
    }

    int Line;
    long DiffFactor = 0;
    DWORD Pitch = pInfo->InputPitch;
    BOOL IsOdd((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);
    
    qwBitShift = BitShift;

    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
    BYTE* PreviousLine = pInfo->PictureHistory[2]->pData + 16 * Pitch;

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        DiffFactor += CalcDiffFactorLine(CurrentLine + 16,
                                            PreviousLine + 16,
                                            pInfo->LineLength - 32);
        CurrentLine += Pitch;
        PreviousLine += Pitch;
    }

    _asm
    {
        emms
    }

    pInfo->FieldDiff = DiffFactor;
    LOG(2, "Frame %d %c FD = %d", pInfo->CurrentFrame, IsOdd ? 'O' : 'E', pInfo->FieldDiff);
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
void CalcDiffFactorChroma(TDeinterlaceInfo* pInfo)
{
    // If we skipped a field, treat the new one as maximally different.
    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[2] == NULL)
    {
        pInfo->FieldDiff = 0x7fffffff;
        return;
    }

    int Line;
    long DiffFactor = 0;
    DWORD Pitch = pInfo->InputPitch;
    BOOL IsOdd((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);
    
    qwBitShift = BitShift;

    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
    BYTE* PreviousLine = pInfo->PictureHistory[2]->pData + 16 * Pitch;

    for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
    {
        DiffFactor += CalcDiffFactorLineChroma(CurrentLine + 16,
                                                PreviousLine + 16,
                                                pInfo->LineLength - 32);
        CurrentLine += Pitch;
        PreviousLine += Pitch;
    }

    _asm
    {
        emms
    }

    pInfo->FieldDiff = DiffFactor;
    LOG(2, "Frame %d %c FD = %d", pInfo->CurrentFrame, IsOdd ? 'O' : 'E', pInfo->FieldDiff);
}

void DoBothCombAndDiff(TDeinterlaceInfo* pInfo)
{
    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[1] == NULL || pInfo->PictureHistory[2] == NULL)
    {
        pInfo->CombFactor = 0x7fffffff;
        pInfo->FieldDiff = 0x7fffffff;
        return;
    }

    int Line;
    long DiffFactor = 0;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];
    DWORD Pitch = pInfo->InputPitch;
    BOOL IsOdd((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);
    qwBitShift = BitShift;
    
    if(IsOdd)
    {
        BYTE* EvenLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
        BYTE* OddLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
        BYTE* PreviousLine = pInfo->PictureHistory[2]->pData + 16 * Pitch;
        // If one of the fields is missing, treat them as very different.
        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLine(EvenLine + 16,
                                            OddLine + 16, 
                                            EvenLine + Pitch + 16,
                                            pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLine(OddLine + 16,
                                                PreviousLine + 16,
                                                pInfo->LineLength - 32);
            EvenLine += Pitch;
            OddLine += Pitch;
            PreviousLine += Pitch;
        }
    }
    else
    {
        BYTE* EvenLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
        BYTE* OddLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
        BYTE* PreviousLine = pInfo->PictureHistory[2]->pData + 16 * Pitch;

        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLine(EvenLine + 16,
                                            OddLine + 16, 
                                            EvenLine + Pitch + 16,
                                            pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLine(EvenLine + 16,
                                                PreviousLine + 16,
                                                pInfo->LineLength - 32);
            EvenLine += Pitch;
            OddLine += Pitch;
            PreviousLine += Pitch;
        }
    }
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    pInfo->FieldDiff = DiffFactor;
    LOG(2, "Frame %d %c FD = %d \t CF = %d", pInfo->CurrentFrame, IsOdd ? 'O' : 'E', pInfo->FieldDiff, pInfo->CombFactor);
}

void DoBothCombAndDiffChroma(TDeinterlaceInfo* pInfo)
{
    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[1] == NULL || pInfo->PictureHistory[2] == NULL)
    {
        pInfo->CombFactor = 0x7fffffff;
        pInfo->FieldDiff = 0x7fffffff;
        return;
    }

    int Line;
    long DiffFactor = 0;
    DWORD Combs[DSCALER_MAX_HEIGHT / 2];
    DWORD Pitch = pInfo->InputPitch;
    BOOL IsOdd((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);

    qwThreshold = CombJaggieThreshold;
    qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);
    qwBitShift = BitShift;
    
    if(IsOdd)
    {
        BYTE* EvenLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
        BYTE* OddLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
        BYTE* PreviousLine = pInfo->PictureHistory[2]->pData + 16 * Pitch;
        // If one of the fields is missing, treat them as very different.
        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLineChroma(EvenLine + 16,
                                                    OddLine + 16, 
                                                    EvenLine + Pitch + 16,
                                                    pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLineChroma(OddLine + 16,
                                                    PreviousLine + 16,
                                                    pInfo->LineLength - 32);
            EvenLine += Pitch;
            OddLine += Pitch;
            PreviousLine += Pitch;
        }
    }
    else
    {
        BYTE* EvenLine = pInfo->PictureHistory[0]->pData + 16 * Pitch;
        BYTE* OddLine = pInfo->PictureHistory[1]->pData + 16 * Pitch;
        BYTE* PreviousLine = pInfo->PictureHistory[2]->pData + 16 * Pitch;

        for (Line = 16; Line < pInfo->FieldHeight - 16; ++Line)
        {
            Combs[Line] = CalcCombFactorLineChroma(EvenLine + 16,
                                                    OddLine + 16, 
                                                    EvenLine + Pitch + 16,
                                                    pInfo->LineLength - 32);
            DiffFactor += CalcDiffFactorLineChroma(EvenLine + 16,
                                                    PreviousLine + 16,
                                                    pInfo->LineLength - 32);
            EvenLine += Pitch;
            OddLine += Pitch;
            PreviousLine += Pitch;
        }
    }
    _asm
    {
        emms
    }

    pInfo->CombFactor = CalculateTotalCombFactor(Combs, pInfo);
    pInfo->FieldDiff = DiffFactor;
    LOG(2, "Frame %d %c FD = %d \t CF = %d", pInfo->CurrentFrame, IsOdd ? 'O' : 'E', pInfo->FieldDiff, pInfo->CombFactor);
}

///////////////////////////////////////////////////////////////////////////////
// Simple Weave.  Copies alternating scanlines from the most recent fields.
BOOL Weave(TDeinterlaceInfo* pInfo)
{
    int i;
    BYTE *lpOverlay = pInfo->Overlay;
    BYTE* CurrentOddLine;
    BYTE* CurrentEvenLine;
    DWORD Pitch = pInfo->InputPitch;

    if (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
    {
        CurrentOddLine = pInfo->PictureHistory[0]->pData;
        CurrentEvenLine = pInfo->PictureHistory[1]->pData;
    }
    else
    {
        CurrentOddLine = pInfo->PictureHistory[1]->pData;
        CurrentEvenLine = pInfo->PictureHistory[0]->pData;
    }
    
    for (i = 0; i < pInfo->FieldHeight; i++)
    {
        pInfo->pMemcpy(lpOverlay, CurrentEvenLine, pInfo->LineLength);
        lpOverlay += pInfo->OverlayPitch;
        CurrentEvenLine += Pitch;

        pInfo->pMemcpy(lpOverlay, CurrentOddLine, pInfo->LineLength);
        lpOverlay += pInfo->OverlayPitch;
        CurrentOddLine += Pitch;
    }
    _asm
    {
        emms
    }
    return TRUE;
}

BOOL WeaveDelay(TDeinterlaceInfo* pInfo, int Delay)
{
    int i;
    BYTE *lpOverlay = pInfo->Overlay;
    BYTE* CurrentOddLine;
    BYTE* CurrentEvenLine;
    DWORD Pitch = pInfo->InputPitch;

    if (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
    {
        CurrentOddLine = pInfo->PictureHistory[Delay]->pData;
        CurrentEvenLine = pInfo->PictureHistory[Delay + 1]->pData;
    }
    else
    {
        CurrentOddLine = pInfo->PictureHistory[Delay + 1]->pData;
        CurrentEvenLine = pInfo->PictureHistory[Delay]->pData;
    }

    for (i = 0; i < pInfo->FieldHeight; i++)
    {
        pInfo->pMemcpy(lpOverlay, CurrentEvenLine, pInfo->LineLength);
        lpOverlay += pInfo->OverlayPitch;
        CurrentEvenLine += Pitch;

        pInfo->pMemcpy(lpOverlay, CurrentOddLine, pInfo->LineLength);
        lpOverlay += pInfo->OverlayPitch;
        CurrentOddLine += Pitch;
    }
    _asm
    {
        emms
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// SimpleFilmMode.  Copies alternating scanlines from the input after
// accounting for an adjustable delay.
///////////////////////////////////////////////////////////////////////////////
BOOL SimpleFilmMode(TDeinterlaceInfo* pInfo, PFNFLIP* pfnFlip)
{
    BYTE* lpOverlay = pInfo->Overlay;
    int TestField;
    int TestOdd;
    BOOL IsOdd((pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) > 0);


    if(IsOdd == TRUE)
    {
        TestField = (pInfo->CurrentFrame + 5 - FilmFlipDelay / 2) % 5;
        TestOdd = ((FilmFlipDelay % 2) == 0);
    }
    else
    {
        TestField = (pInfo->CurrentFrame + 5 - (FilmFlipDelay + 1) / 2) % 5;
        TestOdd = ((FilmFlipDelay % 2) != 0);
    }

    if(pfnFlip(TestField, TestOdd) == TRUE)
    {
        return WeaveDelay(pInfo, FilmFlipDelay);
    }
    else
    {
        return FALSE;
    }
}


/////////////////////////////////////////////////////////////////////////////
// Simple Bob.  Copies the most recent field to the overlay, with each scanline
// copied twice.
/////////////////////////////////////////////////////////////////////////////
BOOL Bob(TDeinterlaceInfo* pInfo)
{
    int i;
    BYTE* lpOverlay = pInfo->Overlay;
    BYTE* CurrentLine = pInfo->PictureHistory[0]->pData;
    DWORD Pitch = pInfo->InputPitch;

    // No recent data?  We can't do anything.
    if (CurrentLine == NULL)
    {
        return FALSE;
    }
    
    // If field is odd we will offset it down 1 line to avoid jitter  TRB 1/21/01
    if (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD)
    {
        if (pInfo->CpuFeatureFlags & FEATURE_SSE)
        {
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // extra copy of first line
            lpOverlay += pInfo->OverlayPitch;                            // and offset out output ptr
            for (i = 0; i < pInfo->FieldHeight - 1; i++)
            {
                memcpyBOBSSE(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
            }
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // only 1 copy of last line
        }
        else
        {
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // extra copy of first line
            lpOverlay += pInfo->OverlayPitch;                    // and offset out output ptr
            for (i = 0; i < pInfo->FieldHeight - 1; i++)
            {
                memcpyBOBMMX(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
            }
            pInfo->pMemcpy(lpOverlay, CurrentLine, pInfo->LineLength);   // only 1 copy of last line
        }
    }   
    else
    {
        if (pInfo->CpuFeatureFlags & FEATURE_SSE)
        {
            for (i = 0; i < pInfo->FieldHeight; i++)
            {
                memcpyBOBSSE(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
            }
        }
        else
        {
            for (i = 0; i < pInfo->FieldHeight; i++)
            {
                memcpyBOBMMX(lpOverlay, lpOverlay + pInfo->OverlayPitch,
                    CurrentLine, pInfo->LineLength);
                lpOverlay += 2 * pInfo->OverlayPitch;
                CurrentLine += Pitch;
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
    {
        "Film Flip Delay", SLIDER, 0, &FilmFlipDelay,
        0, 0, 3, 1, 1,
        NULL,
        "Pulldown", "FilmFlipDelay", NULL,

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
}

CTreeSettingsGeneric* FD_Common_GetTreeSettingsPage()
{
    return new CTreeSettingsGeneric("Pulldown Shared Settings",FD_CommonSettings, FD_COMMON_SETTING_LASTONE);
}
