/////////////////////////////////////////////////////////////////////////////
// $Id: TomsMoCompAll.inc,v 1.3 2002-12-10 16:32:21 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Tom Barry  All rights reserved.
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
//  Also, this program is "Philanthropy-Ware".  That is, if you like it and 
//  feel the need to reward or inspire the author then please feel free (but
//  not obligated) to consider joining or donating to the Electronic Frontier
//  Foundation. This will help keep cyber space free of barbed wire and bullsh*t.  
//  See www.eff.org for details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/11/26 21:32:14  adcockj
// Made new strange bob method optional
//
// Revision 1.1  2002/07/07 20:07:24  trbarry
// First cut at TomsMoComp, motion compensated deinterlace
//
//
/////////////////////////////////////////////////////////////////////////////


#include "windows.h"
#include "DS_Deinterlace.h"
#include "TomsMoComp.h"

int  Max_Frames;
int  PrevFrame;
int  PrevInFrame;
int  dropFrame;
BOOL IsOdd;
#define TopFirst IsOdd
unsigned char* dstp;

const BYTE* pWeaveSrc;
const BYTE* pWeaveSrcP;
BYTE* pWeaveDest;
const BYTE* pCopySrc;
const BYTE* pCopySrcP;
BYTE* pCopyDest;
MEMCPY_FUNC* pMyMemcpy;

int src_pitch;
int dst_pitch;
int rowsize;
int RowPixels;
int height;
int FldHeight;
int BadAlign;

BOOL FUNCT_NAME(TDeinterlaceInfo* pInfo)
{
	pMyMemcpy = pInfo->pMemcpy;

    dstp = pInfo->Overlay;
    src_pitch = pInfo->InputPitch;
    dst_pitch = pInfo->OverlayPitch;
    rowsize = pInfo->LineLength;
	RowPixels = rowsize >> 1;
	FldHeight = pInfo->FieldHeight;
       
    pCopySrc = pInfo->PictureHistory[1]->pData;
    pCopySrcP = pInfo->PictureHistory[3]->pData;
    pWeaveSrc = pInfo->PictureHistory[0]->pData;  
    pWeaveSrcP = pInfo->PictureHistory[2]->pData;
	IsOdd =	pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD;

#ifdef IS_SSE2
// SSE2 support temporarily deleted

#endif

    if(IsOdd)
    {
		// if we have an odd field we copy an even field and weave an odd field
		pCopyDest = pInfo->Overlay;
		pWeaveDest = pInfo->Overlay + dst_pitch;
	}
    else
    {
		// if we have an ever field we copy an odd field and weave an even field
		pCopyDest = pInfo->Overlay + dst_pitch;
		pWeaveDest = pInfo->Overlay;

	}
	// copy 1st and last weave lines 
	Fieldcopy(pWeaveDest, pCopySrc, rowsize,		
				1, dst_pitch*2, src_pitch);
	Fieldcopy(pWeaveDest+(FldHeight-1)*dst_pitch*2,
		pCopySrc+(FldHeight-1)*src_pitch, rowsize, 
				1, dst_pitch*2, src_pitch);

#ifdef USE_VERTICAL_FILTER

// Vertical Filter currently not implemented for DScaler !!

	// copy 1st and last lines the copy field
	Fieldcopy(pCopyDest, pCopySrc, rowsize, 
				1, dst_pitch*2, src_pitch);
	Fieldcopy(pCopyDest+(FldHeight-1)*dst_pitch*2,
		pCopySrc+(FldHeight-1)*src_pitch, rowsize, 
				1, dst_pitch*2, src_pitch);
#else

	// copy all of the copy field
	Fieldcopy(pCopyDest, pCopySrc, rowsize, 
				FldHeight, dst_pitch*2, src_pitch);
#endif	
	// then go fill in the hard part, being variously lazy depending upon
	// SearchEffort

    if(UseStrangeBob == FALSE)
    {
		if (SearchEffort == 0)
		{
			Search_Effort_0();
		}

		else if (SearchEffort <= 1)
		{
			Search_Effort_1();
		}

	/*	else if (SearchEffort <= 2)
		{
			Search_Effort_2();
		}
	*/
		else if (SearchEffort <= 3)
		{
			Search_Effort_3();
		}

		else if (SearchEffort <= 5)
		{
			Search_Effort_5();
		}

		else if (SearchEffort <= 9)
		{
			Search_Effort_9();
		}

		else if (SearchEffort <= 11)
		{
			Search_Effort_11();
		}

		else if (SearchEffort <= 13)
		{
			Search_Effort_13();
		}

		else if (SearchEffort <= 15)
		{
			Search_Effort_15();
		}
		else if (SearchEffort <= 19)
		{
			Search_Effort_19();
		}
		else if (SearchEffort <= 21)
		{
			Search_Effort_21();
		}

		else 
		{
			Search_Effort_Max();
		}
	}
	else
	{
		if (SearchEffort == 0)
		{
			Search_Effort_0_SB();
		}

		else if (SearchEffort <= 1)
		{
			Search_Effort_1_SB();
		}

	/*	else if (SearchEffort <= 2)
		{
			Search_Effort_2_SB();
		}
	*/
		else if (SearchEffort <= 3)
		{
			Search_Effort_3_SB();
		}

		else if (SearchEffort <= 5)
		{
			Search_Effort_5_SB();
		}

		else if (SearchEffort <= 9)
		{
			Search_Effort_9_SB();
		}

		else if (SearchEffort <= 11)
		{
			Search_Effort_11_SB();
		}

		else if (SearchEffort <= 13)
		{
			Search_Effort_13_SB();
		}

		else if (SearchEffort <= 15)
		{
			Search_Effort_15_SB();
		}
		else if (SearchEffort <= 19)
		{
			Search_Effort_19_SB();
		}
		else if (SearchEffort <= 21)
		{
			Search_Effort_21_SB();
		}

		else 
		{
			Search_Effort_Max_SB();
		}
	}
	__asm emms;

    return TRUE;
}

int Fieldcopy(void *dest, const void *src, size_t count, 
				int rows, int dst_pitch, int src_pitch)
{
BYTE* pDest = (BYTE*) dest;
BYTE* pSrc = (BYTE*) src;
int i;
	
	for (i=0; i < rows; i++)
	{
		MyMemCopy(pDest, pSrc, count);
		pSrc += src_pitch;
		pDest += dst_pitch;
	}
	return 0;
}



#include "TomsMoCompAll2.inc"

#define USE_STRANGE_BOB

#include "TomsMoCompAll2.inc"
