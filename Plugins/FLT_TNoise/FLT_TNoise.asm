/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_TNoise.asm,v 1.5 2001-11-22 22:27:00 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Steven Grimm.  All rights reserved.
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
// Revision 1.4  2001/11/21 15:21:41  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.3  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

//
// This is the implementation of the noise filter described in Noise.c.
// It is broken out into this separate file because most of the logic is
// the same for the different processor variants, and doing it this way
// means we only have to maintain one copy of the code.
//

#if defined(IS_SSE)
#define MAINLOOP_LABEL DoNext8Bytes_SSE
#elif defined(IS_3DNOW)
#define MAINLOOP_LABEL DoNext8Bytes_3DNow
#else
#define MAINLOOP_LABEL DoNext8Bytes_MMX
#endif

#if defined(IS_SSE)
long FilterTemporalNoise_SSE(TDeinterlaceInfo* pInfo)
#elif defined(IS_3DNOW)
long FilterTemporalNoise_3DNOW(TDeinterlaceInfo* pInfo)
#else
long FilterTemporalNoise_MMX(TDeinterlaceInfo* pInfo)
#endif
{
    BYTE* NewLine; 
	BYTE* OldLine;
    int y;
    int Cycles;
    __int64 qwNoiseThreshold;
#ifdef IS_MMX
    const __int64 qwAvgMask = 0xFEFEFEFEFEFEFEFE;
#endif
	
	
	if(pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[2] == NULL)
	{
		return 1000;
	}

	NewLine = pInfo->PictureHistory[0]->pData;
	OldLine = pInfo->PictureHistory[2]->pData;

    // Need to have the current and next-to-previous fields to do the filtering.
    if (NewLine == NULL || OldLine == NULL)
    {
        return 1000;
    }

    qwNoiseThreshold = TemporalLuminanceThreshold | (TemporalChromaThreshold << 8);
    qwNoiseThreshold |= (qwNoiseThreshold << 48) | (qwNoiseThreshold << 32) | (qwNoiseThreshold << 16);
    Cycles = pInfo->LineLength / 8;

    for (y = 0; y < pInfo->FieldHeight; y++)
    {
        _asm 
        {
            mov ecx, Cycles
            mov eax, NewLine
            mov ebx, OldLine
            movq mm5, qwNoiseThreshold      // mm5 = NoiseThreshold

MAINLOOP_LABEL:

            movq mm0, qword ptr[eax]        // mm0 = NewPixel
            movq mm1, qword ptr[ebx]        // mm1 = OldPixel
            movq mm2, mm0                   // mm2 = NewPixel

            // Now determine the weighted averages of the old and new pixel values.
#if defined(IS_SSE)
            pavgb mm2, mm1                  // mm2 = avg(NewPixel, OldPixel)
            pavgb mm2, mm1                  // mm2 = avg(NewPixel, OldPixel, OldPixel, OldPixel)
#elif defined(IS_3DNOW)
            pavgusb mm2, mm1                // mm2 = avg(NewPixel, OldPixel)
            pavgusb mm2, mm1                // mm2 = avg(NewPixel, OldPixel, OldPixel, OldPixel)
#else
            movq mm3, mm1                   // mm3 = OldPixel
            movq mm4, qwAvgMask             // mm4 = mask to remove lower bits of bytes
            pand mm3, mm4                   // mm3 = OldPixel with LSBs removed
            pand mm2, mm4                   // mm4 = OldPixel with LSBs removed
            psrlw mm3, 1                    // mm3 = OldPixel / 2
            psrlw mm2, 1                    // mm2 = NewPixel / 2
            paddusb mm2, mm3                // mm2 = avg(NewPixel, OldPixel)
            pand mm2, mm4                   // mm2 = avg(NewPixel, OldPixel) with LSBs removed
            psrlw mm2, 1                    // mm2 = avg(NewPixel, OldPixel) / 2
            paddusb mm2, mm3                // mm2 = avg(NewPixel, OldPixel, OldPixel, OldPixel)
#endif

            // Figure out which pixels are sufficiently different from their predecessors
            // to be considered new.  There is, unfortunately, no absolute-difference
            // MMX instruction, so we OR together two unsigned saturated differences
            // (one of which will always be zero).
            movq mm3, mm0                   // mm3 = NewPixel
            psubusb mm3, mm1                // mm3 = max(NewPixel - OldPixel, 0)
            movq mm4, mm1                   // mm4 = OldPixel
            psubusb mm4, mm0                // mm4 = max(OldPixel - NewPixel, 0)
            por mm3, mm4                    // mm3 = abs(NewPixel - OldPixel)
            
            // Filter out pixels whose differences are less than the threshold.
            psubusb mm3, mm5                // mm3 = max(0, abs(NewPixel - OldPixel) - threshold)

            // Turn the filtered list into a mask.  While we've been operating on bytes
            // up to now since we want to treat luminance and chroma differences as
            // equal for purposes of comparing against the threshold, now we have to
            // treat pixels as a whole.
            pxor mm4, mm4                   // mm4 = 0
            pcmpgtw mm3, mm4                // mm3 = 0xFFFF where abs(NewPixel - OldPixel) > threshold
            movq mm6, mm3                   // mm6 = 0xFFFF where abs(NewPixel - OldPixel) > threshold
            pandn mm6, mm2                  // mm6 = weighted avg. where abs(NewPixel - OldPixel) <= threshold
            pand mm3, mm0                   // mm3 = NewPixel where abs(NewPixel - OldPixel) > threshold
            por mm3, mm6                    // mm3 = result pixels

            movq qword ptr[eax], mm3

            add eax, 8
            add ebx, 8
            loop MAINLOOP_LABEL
        }

        NewLine += pInfo->InputPitch;
        OldLine += pInfo->InputPitch;
    }

    // clear out the MMX registers ready for doing floating point
    // again
    _asm
    {
        emms
    }

    return 1000;
}

#undef MAINLOOP_LABEL