/////////////////////////////////////////////////////////////////////////////
// FD_Common.c
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock. All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
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

#include "stdafx.h"
#include "OutThreads.h"
#include "FD_Common.h"
#include "DebugLog.h"


// Settings
// Default values which can be overwritten by the INI file
long BitShift = 13;
long CombEdgeDetect = 625;
long CombJaggieThreshold = 73;
long DiffThreshold = 224;


///////////////////////////////////////////////////////////////////////////////
// GetCombFactor
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
// the BitShift value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
long GetCombFactor(DEINTERLACE_INFO *pInfo)
{
	int Line;
	WORD LineFactor;
	long CombFactor = 0;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	long ActiveX = pInfo->SourceRect.right - pInfo->SourceRect.left;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 qwOnes = 0x0001000100010001;

	__int64 qwEdgeDetect;
	__int64 qwThreshold;

	// If we've already computed the comb factor, just return it.
	if (pInfo->CombFactor > -1)
		return pInfo->CombFactor;

	// If one of the fields is missing, treat them as very different.
	if (pInfo->OddLines[0] == NULL || pInfo->EvenLines[0] == NULL)
	{
		pInfo->CombFactor = 0x7fffffff;
		return pInfo->CombFactor;
	}

	qwEdgeDetect = CombEdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = CombJaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	for (Line = pInfo->SourceRect.top / 2; Line < pInfo->SourceRect.bottom / 2 - 1; ++Line)
	{
		if(pInfo->IsOdd)
		{
			YVal1 = pInfo->OddLines[0][Line] + (pInfo->SourceRect.left & ~1);
			YVal2 = pInfo->EvenLines[0][Line + 1] + (pInfo->SourceRect.left & ~1);
			YVal3 = pInfo->OddLines[0][Line + 1] + (pInfo->SourceRect.left & ~1);
		}
		else
		{
			YVal1 = pInfo->EvenLines[0][Line] + (pInfo->SourceRect.left & ~1);
			YVal2 = pInfo->OddLines[0][Line] + (pInfo->SourceRect.left & ~1);
			YVal3 = pInfo->EvenLines[0][Line + 1] + (pInfo->SourceRect.left & ~1);
		}

		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			mov edx,dword ptr [YVal3]
			shr ecx, 2       // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			pxor mm0, mm0    // mm0 = 0
align 8
Next8Bytes:
			movq mm3, qword ptr[eax] 
			movq mm4, qword ptr[ebx] 
			movq mm5, qword ptr[edx]

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// work out (O1 - E) * (O2 - E) - EdgeDetect * (O1 - O2) ^ 2 >> 12
			// result will be in mm6

			psrlw mm3, 01
			psrlw mm4, 01
			psrlw mm5, 01

			movq mm6, mm3
			psubw mm6, mm4		//mm6 = O1 - E

			movq mm7, mm5
			psubw mm7, mm4		//mm7 = O2 - E

			pmullw mm6, mm7		// mm0 = (O1 - E) * (O2 - E)

			movq mm7, mm3
			psubw mm7, mm5		// mm7 = (O1 - O2)
			pmullw mm7, mm7		// mm7 = (O1 - O2) ^ 2
			psrlw mm7, 12		// mm7 = (O1 - O2) ^ 2 >> 12
			pmullw mm7, qwEdgeDetect		// mm1  = EdgeDetect * (O1 - O2) ^ 2 >> 12

			psubw mm6, mm7      // mm6 is what we want

			pcmpgtw mm6, qwThreshold

			pand mm6, qwOnes

			paddw mm0, mm6

			add eax, 8
			add ebx, 8
			add edx, 8

			dec ecx
			jne near Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
            mov ax, cx
            shr ecx, 16
            add ax, cx
			mov LineFactor, ax
		}
		CombFactor += LineFactor;
	}

    // Clear out MMX registers before we need to do floating point again
    _asm
    {
 		emms
    }

	pInfo->CombFactor = CombFactor;
	LOG(" Frame %d %c CF = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->CombFactor);
	return CombFactor;
}

///////////////////////////////////////////////////////////////////////////////
// CompareFields
//
// This routine basically calculates how close the pixels in pLines2
// are to the pixels in pLines1
// this is my attempt to implement Mark Rejhon's 3:2 pulldown code
// we will use this to dect the times when we get three fields in a row from
// the same frame
// the result is the total average diffrence between the Y components of each pixel
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
long CompareFields(DEINTERLACE_INFO *pInfo)
{
	int Line;
	DWORD LineFactor;
	long DiffFactor = 0;
	short* YVal1;
	short* YVal2;
	long ActiveX = pInfo->SourceRect.right - pInfo->SourceRect.left;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	__int64 wBitShift    = BitShift;
	short** pLines1;
	short** pLines2;

	// If we've already computed the field difference, just return it.
	if (pInfo->FieldDiff > -1)
		return pInfo->FieldDiff;

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
		return 0x7fffffff;

	for (Line = pInfo->SourceRect.top / 2; Line < pInfo->SourceRect.bottom / 2; ++Line)
	{
		YVal1 = pLines1[Line] + (pInfo->SourceRect.left & ~1);
		YVal2 = pLines2[Line] + (pInfo->SourceRect.left & ~1);
		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			shr ecx, 2		 // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			movq mm7, wBitShift
			pxor mm0, mm0    // mm0 = 0  this is running total
align 8
Next8Bytes:
			movq mm4, qword ptr[eax] 
			movq mm5, qword ptr[ebx] 
			pand mm5, mm1    // get only Y compoment
			pand mm4, mm1    // get only Y compoment

			psubw mm4, mm5   // mm4 = Y1 - Y2
			pmaddwd mm4, mm4 // mm4 = (Y1 - Y2) ^ 2
			psrld mm4, mm7   // divide mm4 by 2 ^ Bitshift
			paddd mm0, mm4   // keep total in mm0

			add eax, 8
			add ebx, 8
			
			dec ecx
			jne near Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
			mov LineFactor, ecx
			emms
		}
		DiffFactor += (long)sqrt(LineFactor);
	}

	pInfo->FieldDiff = DiffFactor;
	LOG(" Frame %d %c FD = %d", pInfo->CurrentFrame, pInfo->IsOdd ? 'O' : 'E', pInfo->FieldDiff);
	return DiffFactor;
}

void DoBothCombAndDiff(DEINTERLACE_INFO *info)
{
	int Line;
	int LoopCtr;
	short* L1;     // ptr to Line1, of 3
	short* L2;     // ptr to Line2, the weave line
	short* L3;     // ptr to Line3
	short* LP2;     // ptr to prev Line2
	short **pOddLines = info->OddLines[0];
	short **pEvenLines = info->EvenLines[0];
	short **pPrevLines = info->IsOdd ? info->OddLines[1] : info->EvenLines[1];
	const __int64 qwShiftMask = 0xfefffefffefffeff; // to avoid shifting chroma	to luma
	const __int64 qwOnes = 0x0001000100010001;
	__int64 qwThresholdWeave;
	__int64 qwThresholdDiff;
	__int64 i;
	unsigned long DiffFactor = 0;
	unsigned long WeaveFactor = 0;

	i = 20; // what is different
	qwThresholdWeave = i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i;
	i = 12; // what is different
	qwThresholdDiff = i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i;


	if (pOddLines == NULL || pEvenLines == NULL || pPrevLines == NULL)
		return;

	for (Line = 16; Line < (info->FieldHeight - 16); ++Line)
	{
		LoopCtr = info->LineLength / 8;    // there are LineLength / 8 qwords per line

		if (info->IsOdd)
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
			// get abs value of possible L2 comb answer in mm0
			// mm2 will have L2 in it
			movq mm2, mm1    // L2
			psubusb mm1, mm0    // L2 - avg
			psubusb mm0, mm2    // avg - L2
			por  mm0, mm1    // abs(avg-L2)

			pand mm0, qwShiftMask
			psrlw mm0, 1           // abs(avg-L2)/2
			pcmpgtb mm0, qwThresholdWeave
			pand mm0, qwOnes       // 1 if abs(avg-L2)/2 > Threshold
			paddusb mm7, mm0                // count of all times the comb threshold
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
			paddusb mm6, mm1                // count of all times the diff threshold

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

	info->CombFactor = WeaveFactor;
	info->FieldDiff = DiffFactor;

	LOG(" Frame %d %c FD = %d CF = %d", info->CurrentFrame, info->IsOdd ? 'O' : 'E', info->FieldDiff, info->CombFactor);

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
BOOL Weave(DEINTERLACE_INFO *info)
{
	int i;
	BYTE *lpOverlay = info->Overlay;

	if (info->EvenLines[0] == NULL || info->OddLines[0] == NULL)
		return FALSE;

	for (i = 0; i < info->FieldHeight; i++)
	{
		info->pMemcpy(lpOverlay, info->EvenLines[0][i], info->LineLength);
		lpOverlay += info->OverlayPitch;

		info->pMemcpy(lpOverlay, info->OddLines[0][i], info->LineLength);
		lpOverlay += info->OverlayPitch;
	}
	_asm
	{
		emms
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// Copies memory to two locations using MMX registers for speed.
void memcpyBOBMMX(void *Dest1, void *Dest2, void *Src, size_t nBytes)
{
	__asm
	{
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest1]
		mov     ebx, dword ptr[Dest2]
		mov		ecx, nBytes
		shr     ecx, 6                      // nBytes / 64
align 8
CopyLoop:
		movq	mm0, qword ptr[esi]
		movq	mm1, qword ptr[esi+8*1]
		movq	mm2, qword ptr[esi+8*2]
		movq	mm3, qword ptr[esi+8*3]
		movq	mm4, qword ptr[esi+8*4]
		movq	mm5, qword ptr[esi+8*5]
		movq	mm6, qword ptr[esi+8*6]
		movq	mm7, qword ptr[esi+8*7]
		movq	qword ptr[edi], mm0
		movq	qword ptr[edi+8*1], mm1
		movq	qword ptr[edi+8*2], mm2
		movq	qword ptr[edi+8*3], mm3
		movq	qword ptr[edi+8*4], mm4
		movq	qword ptr[edi+8*5], mm5
		movq	qword ptr[edi+8*6], mm6
		movq	qword ptr[edi+8*7], mm7
		movq	qword ptr[ebx], mm0
		movq	qword ptr[ebx+8*1], mm1
		movq	qword ptr[ebx+8*2], mm2
		movq	qword ptr[ebx+8*3], mm3
		movq	qword ptr[ebx+8*4], mm4
		movq	qword ptr[ebx+8*5], mm5
		movq	qword ptr[ebx+8*6], mm6
		movq	qword ptr[ebx+8*7], mm7
		add		esi, 64
		add		edi, 64
		add		ebx, 64
		dec ecx
		jne near CopyLoop

		mov		ecx, nBytes
		and     ecx, 63
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		mov byte ptr[ebx], dl
		inc esi
		inc edi
		inc ebx
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
	}
}

/////////////////////////////////////////////////////////////////////////////
// Copies memory to two locations using MMX registers for speed.
void memcpyBOBSSE(void *Dest1, void *Dest2, void *Src, size_t nBytes)
{
	__asm
	{
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest1]
		mov     ebx, dword ptr[Dest2]
		mov		ecx, nBytes
		shr     ecx, 7                      // nBytes / 128
align 8
CopyLoop:
		movaps	xmm0, xmmword ptr[esi]
		movaps	xmm1, xmmword ptr[esi+16*1]
		movaps	xmm2, xmmword ptr[esi+16*2]
		movaps	xmm3, xmmword ptr[esi+16*3]
		movaps	xmm4, xmmword ptr[esi+16*4]
		movaps	xmm5, xmmword ptr[esi+16*5]
		movaps	xmm6, xmmword ptr[esi+16*6]
		movaps	xmm7, xmmword ptr[esi+16*7]
		movntps	xmmword ptr[edi], xmm0
		movntps	xmmword ptr[edi+16*1], xmm1
		movntps	xmmword ptr[edi+16*2], xmm2
		movntps	xmmword ptr[edi+16*3], xmm3
		movntps	xmmword ptr[edi+16*4], xmm4
		movntps	xmmword ptr[edi+16*5], xmm5
		movntps	xmmword ptr[edi+16*6], xmm6
		movntps	xmmword ptr[edi+16*7], xmm7
		movntps	xmmword ptr[ebx], xmm0
		movntps	xmmword ptr[ebx+16*1], xmm1
		movntps	xmmword ptr[ebx+16*2], xmm2
		movntps	xmmword ptr[ebx+16*3], xmm3
		movntps	xmmword ptr[ebx+16*4], xmm4
		movntps	xmmword ptr[ebx+16*5], xmm5
		movntps	xmmword ptr[ebx+16*6], xmm6
		movntps	xmmword ptr[ebx+16*7], xmm7
		add		esi, 128
		add		edi, 128
		add		ebx, 128
		dec ecx
		jne near CopyLoop

		mov		ecx, nBytes
		and     ecx, 127
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		mov byte ptr[ebx], dl
		inc esi
		inc edi
		inc ebx
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
	}
}

/////////////////////////////////////////////////////////////////////////////
// Simple Bob.  Copies the most recent field to the overlay, with each scanline
// copied twice.
/////////////////////////////////////////////////////////////////////////////
BOOL Bob(DEINTERLACE_INFO *info)
{
	int i;
	BYTE *lpOverlay = info->Overlay;
	short **lines;
 
	// If field is odd we will offset it down 1 line to avoid jitter  TRB 1/21/01
	if (info->IsOdd)
	{
		lines = info->OddLines[0];
		// No recent data?  We can't do anything.
		if (lines == NULL)
			return FALSE;

		if (info->CpuFeatureFlags & FEATURE_SSE)
		{
			memcpySSE(lpOverlay, lines[0], info->LineLength);	// extra copy of first line
			lpOverlay += info->OverlayPitch;					// and offset out output ptr
			for (i = 0; i < info->FieldHeight - 1; i++)
			{
				memcpyBOBSSE(lpOverlay, lpOverlay + info->OverlayPitch,
					lines[i], info->LineLength);
				lpOverlay += 2 * info->OverlayPitch;
			}
			memcpySSE(lpOverlay, lines[i], info->LineLength);	// only 1 copy of last line
		}
		else
		{
			memcpyMMX(lpOverlay, lines[0], info->LineLength);	// extra copy of first line
			lpOverlay += info->OverlayPitch;					// and offset out output ptr
			for (i = 0; i < info->FieldHeight - 1; i++)
			{
				memcpyBOBMMX(lpOverlay, lpOverlay + info->OverlayPitch,
					lines[i], info->LineLength);
				lpOverlay += 2 * info->OverlayPitch;
			}
			memcpyMMX(lpOverlay, lines[i], info->LineLength);	// only 1 copy of last line
		}
	}	
	else
	{
		lines = info->EvenLines[0];
		if (lines == NULL)
				return FALSE;
		if (info->CpuFeatureFlags & FEATURE_SSE)
		{
			for (i = 0; i < info->FieldHeight; i++)
			{
				memcpyBOBSSE(lpOverlay, lpOverlay + info->OverlayPitch,
					lines[i], info->LineLength);
				lpOverlay += 2 * info->OverlayPitch;
			}
		}
		else
		{
			for (i = 0; i < info->FieldHeight; i++)
			{
				memcpyBOBMMX(lpOverlay, lpOverlay + info->OverlayPitch,
					lines[i], info->LineLength);
				lpOverlay += 2 * info->OverlayPitch;
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
		13, 0, 15, 1, 1,
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

void FD_Common_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < FD_COMMON_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(FD_CommonSettings[i]));
	}
}
