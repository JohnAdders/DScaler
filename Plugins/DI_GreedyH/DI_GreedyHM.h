// DI_GreedyHM.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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
// 01 Jul 2001   Tom Barry		       Added GreedyH Deinterlace method
//
/////////////////////////////////////////////////////////////////////////////
//
// This member contains the meat of the Greedy (High Motion) deinterlace method
// It is written to not be particularly dependend upon either DScaler or Windows.
// It would be nice to keep it that way if possible as I'd like to also use it to
// port to other environments including maybe Linux, DirectShow filters, batch utilites,
// and maybe VirtualDub or TMPGEnc plug-ins.
//
// I'll add a bigger block of comments here from material I'll post on the list. Basically this
// was made from ideas used in the Blended Clip & Greedy (Low Motion) plug-in's.
//
// Then Edge Enhancement, Median Filtering, Vertical Filtering, Diagonal Jaggie Reduction (DJR ;-) ), 
// n:n pulldown matching, and In-Between Frames were built on that.
//
// !!!  THIS REQUIRES A FAST SSE BOX (Celeron, Athlon, P-III, or P4. !!!
// It will just execute a copy of the old Greedy (Low Motion) if that is not present.
//
//////////////////////////////////////////////////////////////////////////////

extern long GreedyMaxComb;				// max comb we allow past clip
extern long GreedyMotionThreshold;		// ignore changes < this
extern long GreedyMotionSense;			// how rapidly to bob when > threshold
extern long GreedyGoodPullDownLvl;		// Best comb avg / comb avg must be < this for PD
extern long GreedyBadPullDownLvl;		// don't pulldown this field if comb / best avg comb > this 
extern long GreedyEdgeEnhAmt;			// % sharpness to add				
extern long GreedyMedianFilterAmt;		// Don't filter if > this
extern long GreedyLowMotionPdLvl;		// Do pulldown for low motion frames < this

extern BOOL GreedyUsePulldown;			
extern BOOL GreedyUseInBetween;
extern BOOL GreedyUseMedianFilter;
extern BOOL GreedyUseVertFilter;
extern BOOL GreedyUseEdgeEnh;
extern BOOL GreedyUseLowMotionOnly;    // may force for non-SSE
extern BOOL GreedySSEBox;           

typedef struct 
{
    int Comb;					// combs
    int CombChoice;				// val chosen by Greedy Choice
    int Kontrast;				// sum of all abs vertical diff in a field
    int Motion;					// sum of all abs vertical diff in a field
    int Avg;					// avg of last 10 combs (actually just a total)
    int AvgChoice;				// avgs of last 10 chosen combs (actually just a total)
    int Flags;					// a circlular history of last 20 Greedy choice flags
    int Flags2;					// various status flags, mostly for debugging
} GR_PULLDOWN_INFO;

#define PD_VIDEO  1			    // did video deinterlace for this frame
#define PD_PULLDOWN  1 << 1     // did pulldown
#define PD_BAD  1 << 2		    // bad pulldown situation
#define PD_LOW_MOTION  1 << 3   // did pulldown due to low motion
#define PD_MERGED  1 << 4       // made an in between frame
#define PD_32_PULLDOWN  1 << 5  // is 3:2 pulldown
#define PD_22_PULLDOWN 1 << 6   // is 2:2 pulldown
#define PD_ODD        1 << 7    // is Odd Field

// A bunch of shared variables used by all the very similar routines
#define FSFIELDS 4		// number of fields to buffer
#define FSMAXROWS 241	// allow space for max 240 rows/field, plus a spare
#define FSMAXCOLS 1000	// allow space for max 1000 screen cols
#define FSCOLCT FSMAXCOLS * FSFIELDS / 4 // number qwords in row = cols * 4 fields / 4 pixels
#define FSCOLSIZE 32	// bytes to skip for info for next col (of 4 fields and 4 pixels)
#define FSROWSIZE FSMAXCOLS*FSCOLSIZE/4  // bytes to skip to get to info for 2nd row
#define FSSIZE FSFIELDS * FSMAXCOLS * FSMAXROWS / 4   // number qwords in FieldStore array

// Parm data captured from DSCALER info on call
extern short **pLines;					// current input lines, either even or odd
extern short **pOddLines;
extern short **pEvenLines;
extern int	FieldHeight;
extern int LineLength;
extern int OverlayPitch;	
extern BOOL InfoIsOdd;
extern BYTE *lpCurOverlay;

extern __int64 MaxComb;
extern __int64 EdgeThreshold;
extern __int64 EdgeSense;
extern __int64 MedianFilterAmt;
extern __int64 EdgeEnhAmt;
extern __int64 MotionThreshold;
extern __int64 MotionSense;
extern int FsPtr;			// current subscript in FieldStore
extern int FsPtrP;			// prev subscript int FieldStore
extern int FsPtrP2;			// 2 back
extern int FsPtrP3;			// 3 back
extern int FsDelay;				// display is delayed by n fields (1,2,3)
extern __int64 FieldStore[4*FSMAXCOLS*FSMAXROWS/4];
extern __int64* lpFieldStore;

// typedef void (MEMCPY_FUNC)(void* pOutput, void* pInput, size_t nSize);
extern MEMCPY_FUNC* pMemcpy;
extern BOOL DI_GreedyHM();
BOOL FieldStoreCopy(BYTE * dest, __int64 * src, int clen);

// return FS subscripts depending on FsDelay - Note args by reference
BOOL SetFsPtrs(int* L1, int* L2, int* L2P, int* L3, int* CopySrc, BYTE** CopyDest, BYTE** WeaveDest);
BOOL DI_GreedyHF();	    								// fast single pass deint with no options
BOOL DI_GreedyHM_NV();									// full deint with no Vertical Filter
BOOL DI_GreedyHM_V();									// full deint with Vertical Filter

#define PDAVGLEN 10									// len of pulldown average, < len of queue
int UpdatePulldown(int Comb, int Kontrast, int Motion);						
BOOL CanDoPulldown();									// check if we should do pulldown, doit
BOOL GetHistData(GR_PULLDOWN_INFO * OHist, int ct);