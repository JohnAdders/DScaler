/////////////////////////////////////////////////////////////////////////////
// $Id: DI_OldGame.asm,v 1.1 2001-07-30 08:25:22 adcockj Exp $
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
/////////////////////////////////////////////////////////////////////////////

#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif

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
        char    szInfo[64];
        wsprintf(szInfo, "Comb %u", info->CombFactor);
        if (gPfnSetStatus != NULL)
        {
            gPfnSetStatus(szInfo);
        }
    }
#endif

    // If the field is significantly different than the previous one,
    // show the new frame unaltered.
    // This is just a tiny change on the evenOnly/oddOnly filters
    if ((info->bMissedFrame) || (info->CombFactor > gMaxComb))
    {
        short**     ThisField;
        int         LineTarget;

        if (info->IsOdd) 
        {
            ThisField = info->OddLines[0];
        }
        else 
        {
            ThisField = info->EvenLines[0];
        }
        for (LineTarget = 0; LineTarget < info->FieldHeight; LineTarget++)
        {
            // copy latest field's rows to overlay, resulting in a half-height image.
            info->pMemcpy(info->Overlay + LineTarget * info->OverlayPitch,
                        ThisField[LineTarget],
                        info->LineLength);
        }
    }
    // If the field is very similar to the last one, average them.
    // This code is a cut down version of Steven Grimm's temporal noise filter.
    else
    {
        short**         NewLines;
        short**         OldLines;
        int             y;
        int             Cycles;
        unsigned short* Destination = (unsigned short *) (info->Overlay);
#ifdef IS_MMX
        const __int64   qwAvgMask = 0xFEFEFEFEFEFEFEFE;
#endif

        if (info->IsOdd)
        {
            NewLines = info->OddLines[0];
            OldLines = info->EvenLines[0];
        }
        else
        {
            NewLines = info->EvenLines[0];
            OldLines = info->OddLines[0];
        }

        Cycles = info->LineLength / 8;


        for (y = 0; y < info->FieldHeight; y++)
        {
            _asm 
            {
                mov esi, Destination            // Destination is incremented at the bottom of the loop
                mov ecx, Cycles
                mov ebx, y
                shl ebx, 2
                mov edx, NewLines
                add edx, ebx
                mov eax, dword ptr[edx]
                mov edx, OldLines
                add edx, ebx
                mov ebx, dword ptr[edx]

MAINLOOP_LABEL:

                movq mm2, qword ptr[eax]        // mm0 = NewPixel
                movq mm1, qword ptr[ebx]        // mm1 = OldPixel

                // Now determine the weighted averages of the old and new pixel values.
                // Since the frames are likely to be similar for only a short time, use
                // a more even weighting than employed in the temporal nose filter
#if defined(IS_SSE)
                pavgb mm2, mm1                  // mm2 = avg(NewPixel, OldPixel)
#elif defined(IS_3DNOW)
                pavgusb mm2, mm1                // mm2 = avg(NewPixel, OldPixel)
#else
                movq mm3, mm1                   // mm3 = OldPixel
                movq mm4, qwAvgMask             // mm4 = mask to remove lower bits of bytes
                pand mm3, mm4                   // mm3 = OldPixel with LSBs removed
                pand mm2, mm4                   // mm4 = OldPixel with LSBs removed
                psrlw mm3, 1                    // mm3 = OldPixel / 2
                psrlw mm2, 1                    // mm2 = NewPixel / 2
                paddusb mm2, mm3                // mm2 = avg(NewPixel, OldPixel)
#endif

                movq qword ptr[esi], mm2        // Output to the overlay buffer

                add eax, 8
                add ebx, 8
                add esi, 8                      // Move the output pointer
                loop MAINLOOP_LABEL
            }
            Destination += info->OverlayPitch/2;
        }
    }
    _asm
    {
        emms
    }
    return TRUE;
}

#undef MAINLOOP_LABEL
