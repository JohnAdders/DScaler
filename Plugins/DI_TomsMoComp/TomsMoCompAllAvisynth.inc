/////////////////////////////////////////////////////////////////////////////
// $Id$
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
//
/////////////////////////////////////////////////////////////////////////////


//#include "windows.h"
//#include "DS_Deinterlace.h"
#include "TomsMoComp.h"
/*
int  Max_Frames;
int  PrevFrame;
int  PrevInFrame;
int  dropFrame;
*/
BOOL IsOdd;
#define TopFirst IsOdd
unsigned char* dstp;

const BYTE* pWeaveSrc;
const BYTE* pWeaveSrcP;
BYTE* pWeaveDest;
const BYTE* pCopySrc;
const BYTE* pCopySrcP;
BYTE* pCopyDest;

int src_pitch;
int dst_pitch;
int rowsize;
int RowPixels;
int height;
int FldHeight;
int BadAlign;

BOOL FUNCT_NAME(int SearchEffort, int _src_pitch, int _dst_pitch,
		int _rowsize, const BYTE* _pWeaveSrc, const BYTE* _pWeaveSrcP,
		BYTE* _pWeaveDest, BOOL _TopFirst, const BYTE* _pCopySrc,
		const BYTE* _pCopySrcP, int _FldHeight)
{
	// fill in a few parm values where other rtns and asm can access them
	src_pitch = _src_pitch;
	dst_pitch = _dst_pitch;
	rowsize = _rowsize;
	pWeaveSrc = _pWeaveSrc;
	pWeaveSrcP = _pWeaveSrcP;
	pWeaveDest = _pWeaveDest;
	TopFirst = _TopFirst;
	pCopySrc = _pCopySrc;
	pCopySrcP = _pCopySrcP;
	FldHeight = _FldHeight;

	// Go fill in the hard part, being variously lazy depending upon
	// SearchEffort

	if (SearchEffort == -1)
	{
		Do_Dbl_Resize();
	}

	else if (SearchEffort == 0)
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

	__asm emms;

    return 0;
}

int Do_Dbl_Resize()		// smart double the height with WierdBob
{
#define DBL_RESIZE
#define SKIP_SEARCH
#include "SearchLoopTop.inc"	
#include "SearchLoopBottom.inc"
#undef SKIP_SEARCH
#undef DBL_RESIZE
}

#include "TomsMoCompAll2.inc"

