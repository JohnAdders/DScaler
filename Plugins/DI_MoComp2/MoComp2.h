/////////////////////////////////////////////////////////////////////////////
// $Id: MoComp2.h,v 1.1 2003-01-02 13:15:01 adcockj Exp $
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

#include <malloc.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#define BOOL unsigned int
#define BYTE unsigned char

BOOL DeinterlaceMoComp2_SSE(TDeinterlaceInfo* pInfo);
BOOL DeinterlaceMoComp2_MMX(TDeinterlaceInfo* pInfo);
BOOL DeinterlaceMoComp2_3DNOW(TDeinterlaceInfo* pInfo);
#define MyMemCopy pMyMemcpy
extern long SearchEffort;

static int  Fieldcopy(void *dest, const void *src, size_t count, 
			int rows, int dst_pitch, int src_pitch);
static int  Search_Effort_0();
static int  Search_Effort_1();
static int  Search_Effort_3();
static int  Search_Effort_5();
static int  Search_Effort_9();
static int  Search_Effort_11();
static int  Search_Effort_13();
static int  Search_Effort_15();
static int  Search_Effort_19();
static int  Search_Effort_21();
static int  Search_Effort_Max();

__declspec(align(128))
static __int64 BobDiffW[2]; 

__declspec(align(16))
static __int64 BobVal[2]; 

__declspec(align(16))
static __int64 Min_Vals[2]; 

__declspec(align(16))
static __int64 Max_Vals[2]; 

__declspec(align(16))
static const __int64 Max_Mov[2] =  {0x0404040404040404,0x0404040404040404}; 
//static const __int64 Max_Mov[2] =  {0x0f0f0f0f0f0f0f0f,0x0f0f0f0f0f0f0f0f}; 
//static const __int64 Max_Mov[2] =  {0x0808080808080808,0x0808080808080808}; 

__declspec(align(16))
static const __int64 DiffThres[2] =  {0x0f0f0f0f0f0f0f0f,0x0f0f0f0f0f0f0f0f}; 

__declspec(align(16))
static const __int64 YMask[2] = {0x00ff00ff00ff00ff,0x00ff00ff00ff00ff}; // keeps only luma

__declspec(align(16))
static const __int64 UVMask[2] =   {0xff00ff00ff00ff00,0xff00ff00ff00ff00}; // keeps only chroma

__declspec(align(16))
static const __int64 TENS[2]     = {0x0a0a0a0a0a0a0a0a,0x0a0a0a0a0a0a0a0a}; 

__declspec(align(16))
static const __int64 FOURS[2] =    {0x0404040404040404,0x0404040404040404}; 

__declspec(align(16))
static const __int64 ONES[2] =     {0x0101010101010101,0x0101010101010101}; 

__declspec(align(16))
static const __int64 Max_Comb[2] = {0x0202020202020202,0x0202020202020202}; 

__declspec(align(16))
static const __int64 WHITE[2] =    {0x7fff0fff7fff0fff,0x7fff0fff7fff0fff};

static const __int64 ShiftMask = 0xfefffefffefffeff;	// to avoid shifting chroma to luma

static __int64 swork = 0;

/*
// A bunch of things that may need 16 byte alignment

__declspec(align(16))
static const __int64 FIFTEENS[2] = {0x0F0F0F0F0F0F0F0F,0x0F0F0F0F0F0F0F0F}; 

__declspec(align(16))
static const __int64 TWENTIES[2] = {0x1414141414141414,0x1414141414141414}; 

__declspec(align(16))
static const __int64 SIXES[2] =    {0x0606060606060606,0x0606060606060606}; 

__declspec(align(16))
static const __int64 FIVES[2] =    {0x0505050505050505,0x0505050505050505}; 

__declspec(align(16))
static const __int64 THREES[2] =   {0x0303030303030303,0x0303030303030303}; 

__declspec(align(16))
static const __int64 TWOS[2] =     {0x0202020202020202,0x0202020202020202}; 

*/


// Define a few macros for CPU dependent instructions. 
// I suspect I don't really understand how the C macro preprocessor works but
// this seems to get the job done.          // TRB 7/01

// BEFORE USING THESE YOU MUST SET:

// #define SSE_TYPE SSE            (or MMX or 3DNOW)

// some macros for pavgb instruction
//      V_PAVGB(mmr1, mmr2, mmr work register, smask) mmr2 may = mmrw if you can trash it


#define V_PAVGB_MMX(mmr1,mmr2,mmrw,smask) __asm \
	{ \
	__asm movq mmrw,mmr2 \
	__asm pand mmrw, smask \
	__asm psrlw mmrw,1 \
	__asm pand mmr1,smask \
	__asm psrlw mmr1,1 \
	__asm paddusb mmr1,mmrw \
	}

#define V_PAVGB_SSE(mmr1,mmr2,mmrw,smask) {pavgb mmr1,mmr2 }
#define V_PAVGB_3DNOW(mmr1,mmr2,mmrw,smask) {pavgusb mmr1,mmr2 }
#define V_PAVGB(mmr1,mmr2,mmrw,smask) V_PAVGB2(mmr1,mmr2,mmrw,smask,SSE_TYPE) 
#define V_PAVGB2(mmr1,mmr2,mmrw,smask,ssetyp) V_PAVGB3(mmr1,mmr2,mmrw,smask,ssetyp) 
#define V_PAVGB3(mmr1,mmr2,mmrw,smask,ssetyp) V_PAVGB_##ssetyp##(mmr1,mmr2,mmrw,smask) 

// some macros for pmaxub instruction
//      V_PMAXUB(mmr1, mmr2)    
#define V_PMAXUB_MMX(mmr1,mmr2)     __asm \
	{ \
    __asm psubusb mmr1,mmr2 \
    __asm paddusb mmr1,mmr2 \
    }

#define V_PMAXUB_SSE(mmr1,mmr2) {pmaxub mmr1,mmr2 }
#define V_PMAXUB_3DNOW(mmr1,mmr2) V_PMAXUB_MMX(mmr1,mmr2)  // use MMX version
#define V_PMAXUB(mmr1,mmr2) V_PMAXUB2(mmr1,mmr2,SSE_TYPE) 
#define V_PMAXUB2(mmr1,mmr2,ssetyp) V_PMAXUB3(mmr1,mmr2,ssetyp) 
#define V_PMAXUB3(mmr1,mmr2,ssetyp) V_PMAXUB_##ssetyp##(mmr1,mmr2) 

// some macros for pminub instruction
//      V_PMINUB(mmr1, mmr2, mmr work register)     mmr2 may NOT = mmrw
#define V_PMINUB_MMX(mmr1,mmr2,mmrw) __asm \
	{ \
    __asm pcmpeqb mmrw,mmrw     \
    __asm psubusb mmrw,mmr2     \
    __asm paddusb mmr1, mmrw     \
    __asm psubusb mmr1, mmrw     \
	}

#define V_PMINUB_SSE(mmr1,mmr2,mmrw) {pminub mmr1,mmr2}
#define V_PMINUB_3DNOW(mmr1,mmr2,mmrw) V_PMINUB_MMX(mmr1,mmr2,mmrw)  // use MMX version
#define V_PMINUB(mmr1,mmr2,mmrw) V_PMINUB2(mmr1,mmr2,mmrw,SSE_TYPE) 
#define V_PMINUB2(mmr1,mmr2,mmrw,ssetyp) V_PMINUB3(mmr1,mmr2,mmrw,ssetyp) 
#define V_PMINUB3(mmr1,mmr2,mmrw,ssetyp) V_PMINUB_##ssetyp##(mmr1,mmr2,mmrw) 

// some macros for movntq instruction
//      V_MOVNTQ(mmr1, mmr2) 
#define V_MOVNTQ_MMX(mmr1,mmr2) {movq mmr1,mmr2}
#define V_MOVNTQ_3DNOW(mmr1,mmr2) {movq mmr1,mmr2 }
#define V_MOVNTQ_SSE(mmr1,mmr2) {movntq mmr1,mmr2 }
#define V_MOVNTQ(mmr1,mmr2) V_MOVNTQ2(mmr1,mmr2,SSE_TYPE) 
#define V_MOVNTQ2(mmr1,mmr2,ssetyp) V_MOVNTQ3(mmr1,mmr2,ssetyp) 
#define V_MOVNTQ3(mmr1,mmr2,ssetyp) V_MOVNTQ_##ssetyp##(mmr1,mmr2)

// end of macros

// macro load a field from this object
#define thisLoad(reg, intfield) \
		__asm {	\
		__asm mov	reg, this \
		__asm mov   reg, dword ptr [reg].intfield \
		}


#define MERGE4PIXavg(PADDR1, PADDR2) \
__asm { \
		__asm movq	mm0, qword ptr[(PADDR1)] /* our 4 pixels */ \
		__asm movq mm1, qword ptr[(PADDR2)] /* our pixel2 value */ \
		__asm movq mm2, mm0			/* another copy of our pixel1 value */ \
		__asm movq mm3, mm1			/* another copy of our pixel1 value */ \
		__asm psubusb mm2, mm1 \
		__asm psubusb mm3, mm0 \
		__asm por mm2,mm3 \
		__asm V_PAVGB (mm0, mm1, mm3, ShiftMask) /* avg of 2 pixels */ \
		__asm movq mm3, mm2			/* another copy of our our weights */ \
		__asm pxor	 mm1, mm1 \
		__asm psubusb mm3, mm7			/* nonzero where old weights lower, else 0 */ \
		__asm pcmpeqb mm3, mm1			/* now ff where new better, else 00	*/ \
		__asm pcmpeqb mm1, mm3		/* here ff where old better, else 00 */ \
		__asm pand	mm0, mm3			/* keep only better new pixels */ \
		__asm pand  mm2, mm3			/* and weights */ \
		__asm pand	mm5, mm1			/* keep only better old pixels */ \
		__asm pand  mm7, mm1 \
		__asm por	mm5, mm0			/* and merge new & old vals */ \
		__asm por	mm7, mm2 \
}


#define MERGE4PIXavgH(PADDR1A, PADDR1B, PADDR2A, PADDR2B) \
__asm { \
		__asm movq mm0, mmword ptr[(PADDR1A)] /* our 4 pixels */ \
		__asm movq mm1, mmword ptr[(PADDR2A)] /* our pixel2 value */ \
		__asm movq mm2, mmword ptr[(PADDR1B)] /* our 4 pixels */ \
		__asm movq mm3, mmword ptr[(PADDR2B)] /* our pixel2 value */ \
		__asm V_PAVGB(mm0, mm2, mm2, ShiftMask) \
		__asm V_PAVGB(mm1, mm3, mm3, ShiftMask) \
		__asm movq mm2, mm0			/* another copy of our pixel1 value */ \
		__asm movq mm3, mm1			/* another copy of our pixel1 value */ \
		__asm psubusb mm2, mm1 \
		__asm psubusb mm3, mm0 \
		__asm por mm2,mm3 \
		__asm V_PAVGB(mm0, mm1, mm3, ShiftMask)	/* avg of 2 pixels */ \
		__asm movq mm3, mm2			/* another copy of our our weights */ \
		__asm pxor	 mm1, mm1 \
		__asm psubusb mm3, mm7			/* nonzero where old weights lower, else 0 */ \
		__asm pcmpeqb mm3, mm1			/* now ff where new better, else 00	*/ \
		__asm pcmpeqb mm1, mm3		/* here ff where old better, else 00 */ \
		__asm pand	mm0, mm3			/* keep only better new pixels */ \
		__asm pand  mm2, mm3			/* and weights */ \
		__asm pand	mm5, mm1			/* keep only better old pixels */ \
		__asm pand  mm7, mm1 \
		__asm por	mm5, mm0			/* and merge new & old vals */ \
		__asm por	mm7, mm2 \
}


#define RESET_CHROMA	__asm	{por	mm7, UVMask}	