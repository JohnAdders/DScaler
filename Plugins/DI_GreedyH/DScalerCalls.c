/////////////////////////////////////////////////////////////////////////////
// $Id: DScalerCalls.c,v 1.1 2001-10-02 17:46:38 trbarry Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
//      trbarry@trbarry.com
// Requires Avisynth source code to compile for Avisynth
// Avisynth Copyright 2000 Ben Rudiak-Gould.
//      http://www.math.berkeley.edu/~benrg/avisynth.html
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
// Change Log
//
// Date          Developer             Changes
//
// 01 Oct 2001   Tom Barry		       Create Greedy/HM as Avisynth Filter
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 0.1  2001/10/01 06:26:38  trbarry
// Add GreedyHMA (Avisynth filter wrapper) to DScaler files
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DI_GreedyHM.h"

void SetDScalerFrameInfo(BOOL IsOdd, int InPitch, int OutPitch, 
     const BYTE* pSource, BYTE* pOverlay, int FrHeight, int LLength);

static short* pScanLines[FSMAXROWS] = {0};

extern void InitDScaler(int AutoPulldown, int MedianFilter, int VerticalFilter, int EdgeEnhance)
{
#include "DI_GreedyHM2.h"

    __int64 i;
    if (AutoPulldown)
    {
        GreedyUsePulldown = TRUE;
    }

    if (MedianFilter)
    {
        GreedyUseMedianFilter = TRUE;
        if (MedianFilter > 1 && MedianFilter < 256)
        {
            GreedyMedianFilterAmt = MedianFilter;
        }
    }
    
    if (VerticalFilter)
    {
        GreedyUseVertFilter = TRUE;
    }
    
    if (EdgeEnhance)
    {
        GreedyUseEdgeEnh = TRUE;
        if (EdgeEnhance > 1 && EdgeEnhance < 101)
        {
            GreedyEdgeEnhAmt = EdgeEnhance;
        }
    }
    
    // Set up our two parms that are actually evaluated for each pixel
	i=GreedyMaxComb;
	MaxComb = i << 56 | i << 48 | i << 40 | i << 32 | i << 24 | i << 16 | i << 8 | i;    

	i = GreedyMotionThreshold;		// scale to range of 0-257
	MotionThreshold = i << 48 | i << 32 | i << 16 | i | UVMask;    

	i = GreedyMotionSense ;		// scale to range of 0-257
	MotionSense = i << 48 | i << 32 | i << 16 | i;    
	
	i = GreedyGoodPullDownLvl;					// scale to range of 0-257
	EdgeThreshold = i << 48 | i << 32 | i << 16 | i | UVMask;
		
	i=GreedyBadPullDownLvl * 128 / 100;
	EdgeSense =  i << 48 | i << 32  | i << 16  | i;    
	
	i=GreedyMedianFilterAmt;
	MedianFilterAmt =  i << 48 | i << 32 | i << 16 | i;    

	i=GreedyEdgeEnhAmt* 257/100;
	EdgeEnhAmt =  i << 48 | i << 32 | i << 16 | i;    

    pLines = &pScanLines[0];                // table of ptrs to scan lines filled in later
    }    

BOOL CallGreedyHM(BOOL IsOdd, int InPitch, int OutPitch,
     const BYTE* pSource,  BYTE* pOverlay, int FrHeight, int LLength)
{
    SetDScalerFrameInfo(IsOdd, InPitch, OutPitch,
     pSource,  pOverlay, FrHeight, LLength);
    return DI_GreedyHM();
}

// We fill in some GreedyHM external values. These should be in a parm structure, but aren't
void SetDScalerFrameInfo(BOOL IsOdd, int InPitch, int OutPitch,
     const BYTE* pSource,  BYTE* pOverlay, int FrHeight, int LLength)
{
    short* pLineW = (short *) pSource; 
    int i = 0;

    InfoIsOdd = IsOdd;
	OverlayPitch = OutPitch;
	lpCurOverlay = pOverlay;
    FieldHeight = FrHeight / 2;
    FrameHeight = FrHeight;
	LineLength = LLength;
    for ( i = 0; i < FieldHeight ; i++ ) 
    {
        pScanLines[i] = pLineW;
        pLineW += InPitch;                      // note - bumps 2 input lines
    }     
}
