/////////////////////////////////////////////////////////////////////////////
// $Id: DI_OldGame.asm,v 1.2 2001-08-30 10:03:51 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Lindsey Dubb.  All rights reserved.
// based on OddOnly and Temporal Noise DScaler Plugins
// (c) John Adcock & Steve Grimm
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
// Revision 1.1  2001/07/30 08:25:22  adcockj
// Added Lindsey Dubb's method
//
/////////////////////////////////////////////////////////////////////////////

// Processor specific averaging:
// Set destMM to average of destMM and sourceMM
// (Code from DI_GreedyHM.h by Tom Barry, with modification)
// Note that the MMX version rounds down; the other versions round toward the
// first operand.  And yes, shiftMask and noLowBitsMask could be the same, but
// this is a little easier to follow.
// sourceMM and tempMM are changed 

#undef AVERAGE
#if defined(IS_SSE) || defined(IS_MMXEXT)
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask, noLowBitsMask) __asm \
    { \
    __asm pand sourceMM, noLowBitsMask \
    __asm pavgb destMM, sourceMM \
    }
#elif defined(IS_3DNOW)
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask, noLowBitsMask) __asm \
    { \
    __asm pand sourceMM, noLowBitsMask \
    __asm pavgusb destMM, sourceMM \
    }
#else
#define AVERAGE(destMM, sourceMM, tempMM, shiftMask, noLowBitsMask) __asm \
    { \
	__asm pand sourceMM, shiftMask \
	__asm psrlw sourceMM, 1 \
	__asm pand destMM, shiftMask \
	__asm psrlw destMM, 1 \
	__asm paddusb destMM, sourceMM \
    }
#endif // processor specific averaging routine

#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif

// Hidden in the preprocessor stuff below is the actual routine

#if defined(IS_SSE)
long OldGameFilter_SSE(DEINTERLACE_INFO *info)
#elif defined(IS_3DNOW)
long OldGameFilter_3DNOW(DEINTERLACE_INFO *info)
#else
long OldGameFilter_MMX(DEINTERLACE_INFO *info)
#endif
{
#ifdef LD_DEBUG
    {
        char    OutputString[64];
        wsprintf(OutputString, "Motion %u", info->CombFactor);
        if (gPfnSetStatus != NULL)
        {
            gPfnSetStatus(OutputString);
        }
    }
#endif
    // If the field is significantly different than the previous one,
    // show the new frame unaltered.
    // This is just a tiny change on the evenOnly/oddOnly filters

    if ( (info->bMissedFrame) ||
        ((gDisableMotionChecking == FALSE) && (info->CombFactor > gMaxComb)) )
    {
        WORD**     ppThisField = NULL;
        DWORD      LineTarget = 0;

        if (info->IsOdd) 
        {
            ppThisField = info->OddLines[0];
        }
        else 
        {
            ppThisField = info->EvenLines[0];
        }
        if (ppThisField == NULL)
        {
            return TRUE;
        }
        for (LineTarget = 0; LineTarget < (DWORD)info->FieldHeight; LineTarget++)
        {
            // copy latest field's rows to overlay, resulting in a half-height image.
            info->pMemcpy(info->Overlay + LineTarget * info->OverlayPitch,
                        ppThisField[LineTarget],
                        info->LineLength);
        }
    }
    // If the field is very similar to the last one, average them.
    // This code is a cut down version of Steven Grimm's temporal noise filter.
    // It does a really nice job on video via a composite connector 
    else
    {
        WORD**          ppNewLines = NULL;
        WORD**          ppOldLines = NULL;
        const DWORD     Cycles = ((DWORD)info->LineLength) / 8;
        const __int64   qwShiftMask = 0xFEFFFEFFFEFFFEFF;
        const __int64   qwNoLowBitsMask = 0xFEFEFEFEFEFEFEFE;
        WORD*           pDestination = (unsigned short *) (info->Overlay);
        DWORD           LineTarget = 0;

        if (info->IsOdd)
        {
            ppNewLines = info->OddLines[0];
            ppOldLines = info->EvenLines[0];
        }
        else
        {
            ppNewLines = info->EvenLines[0];
            ppOldLines = info->OddLines[0];
        }

        if ((ppNewLines == NULL) || (ppOldLines == NULL))
        {
            return TRUE;
        }

        for (LineTarget = 0; LineTarget < (DWORD)info->FieldHeight; ++LineTarget)
        {
            _asm 
            {
                mov esi, pDestination           // Destination is incremented at the bottom of the loop
                mov ecx, Cycles
                mov ebx, LineTarget
                shl ebx, 2
                mov edx, ppNewLines
                add edx, ebx
                mov eax, dword ptr[edx]
                mov edx, ppOldLines
                add edx, ebx
                mov ebx, dword ptr[edx]

MAINLOOP_LABEL:

                movq mm2, qword ptr[eax]        // mm2 = NewPixel
                movq mm1, qword ptr[ebx]        // mm1 = OldPixel

                // Now determine the weighted averages of the old and new pixel values.
                // Since the frames are likely to be similar for only a short time, use
                // a more even weighting than employed in the temporal nose filter
                AVERAGE(mm2, mm1, mm7, qwShiftMask, qwNoLowBitsMask)

                movq qword ptr[esi], mm2        // Output to the overlay buffer

                add eax, 8
                add ebx, 8
                add esi, 8                      // Move the output pointer
                loop MAINLOOP_LABEL
            }
            pDestination += info->OverlayPitch/2;
        }
    }
    _asm
    {
        emms
    }
    return TRUE;
}

#undef MAINLOOP_LABEL
