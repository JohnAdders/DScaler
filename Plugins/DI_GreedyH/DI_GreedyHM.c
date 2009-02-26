/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//    This file is subject to the terms of the GNU General Public License as
//    published by the Free Software Foundation.  A copy of this license is
//    included with this software distribution in the file COPYING.  If you
//    do not have a copy, you may obtain a copy by writing to the Free
//    Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//    This software is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DI_GreedyHM.h"

// Note - actual default values below may be set in DI_GreedyHSETTINGS
long GreedyMaxComb = 5;                    // max comb we allow past clip
long GreedyMotionThreshold = 25;        // ignore changes < this
long GreedyMotionSense = 30;            // how rapidly to bob when > Threshold
long GreedyGoodPullDownLvl = 83;        // Best Comb avg / Comb Avg must be < thes
long GreedyBadPullDownLvl = 88;            // No Pulldown if field comb / Best avg comb > this
long GreedyHSharpnessAmt = 50;            // % H. sharpness to add or filter                
long GreedyVSharpnessAmt = 23;            // % V. sharpness to add or filter                
long GreedyMedianFilterAmt = 3;            // Don't filter if > this
long GreedyLowMotionPdLvl = 9;            // Do PullDown on if motion < this

BOOL GreedyUsePulldown = FALSE;            
BOOL GreedyUseInBetween = FALSE;
BOOL GreedyUseMedianFilter = FALSE;
BOOL GreedyUseVSharpness = FALSE;
BOOL GreedyUseHSharpness = FALSE;
BOOL GreedySSEBox = TRUE;           
UINT GreedyFeatureFlags = 0;            // Save feature flags on setup

BOOL GreedyWantsToFlip;
BOOL UpdateFieldStore();
BOOL DI_GrUpdtFS_NM_NE_P();                // Update Fieldstore, no Median Filter, No Edge Enh, Pulldown
BOOL DI_GrUpdtFS_M_NE_P();                // Update Fieldstore, Median Filter, No Edge Enh, Pulldown
BOOL DI_GrUpdtFS_NM_E_P();                // Update Fieldstore, no Median Filter, Edge Enh, Pulldown
BOOL DI_GrUpdtFS_NM_E_P_Soft();         // Update Fieldstore, no Median Filter, Edge Enh, Pulldown
BOOL DI_GrUpdtFS_M_E_P();                // Update Fieldstore, Median Filter, Edge Enh, Pulldown
BOOL DI_GrUpdtFS_M_E_P_Soft();            // Update Fieldstore, Median Filter, Edge Enh, Pulldown
BOOL DI_GrUpdtFS_NM_NE_NP();            // Update Fieldstore, no Median Filter, No Edge Enh, No Pulldown
BOOL DI_GrUpdtFS_M_NE_NP();                // Update Fieldstore, Median Filter, No Edge Enh, No Pulldown
BOOL DI_GrUpdtFS_NM_E_NP();                // Update Fieldstore, no Median Filter, Edge Enh, No Pulldown
BOOL DI_GrUpdtFS_NM_E_NP_Soft();        // Update Fieldstore, no Median Filter, Edge Enh, No Pulldown
BOOL DI_GrUpdtFS_M_E_NP();                // Update Fieldstore, Median Filter, Edge Enh, No Pulldown
BOOL DI_GrUpdtFS_M_E_NP_Soft();            // Update Fieldstore, Median Filter, Edge Enh, No Pulldown

//    Input video data is first copied to the FieldStore array, possibly doing
//  edge enhancement and median filtering. Field store is layed out to improve 
//  register usage and cache performace during further deinterlace processing.

// Hopefully we will gain enough using it to make up for the cost of filling it
// in column order. Note array transposed (1000 cols, 240 rows, 1000 cols)

__int64 FieldStore[4*FSMAXCOLS*FSMAXROWS/4] = {0};
                         // 4 fields*1000 cols*240 lines / 4 pixels per qword
extern __int64* lpFieldStore = NULL;

// A bunch of shared variables used by all the very similar routines

int FsPtrP3 = 1;
int FsPtrP2 = 2;
int FsPtrP = 3;
int FsPtr = 0;
int FsDelay = 1;        // display delayed by n fields (1,2,3)

// Parm data captured from DSCALER info on call
BYTE *pLines = 0;                    // current input lines, either even or odd
/*>>>
short **pLines = 0;                    // current input lines, either even or odd
short **pOddLines = 0;
short **pEvenLines = 0;
short **pPrevLines;
>>>>> */
int    FieldHeight = 0;
int    FrameHeight = 0;
int LineLength = 0;
int OverlayPitch = 0;    
int InpPitch = 0;    
BOOL InfoIsOdd = 0;
BYTE *lpCurOverlay = 0;

// typedef void (MEMCPY_FUNC)(void* pOutput, void* pInput, size_t nSize);
MEMCPY_FUNC* pMemcpy = 0;

__int64 MaxComb=0;
__int64 EdgeThreshold=0;
__int64 EdgeSense=0;
__int64 MedianFilterAmt=0;
__int64 HSharpnessAmt=0;
__int64 MotionThreshold=0;
__int64 MotionSense=0;

__int64 SaveVals[3];                            // qword-1, qword, qword+1


// Greedy High Motion Deinterlace, internal routine
BOOL DI_GreedyHM()
{
#include "DI_GreedyHM2.h"

    if (!UpdateFieldStore())
    {
        return FALSE;
    }

    if (CanDoPulldown())
    {
        return TRUE;
    }
    if (GreedyUseHSharpness && GreedyHSharpnessAmt > 0)
    {
        return DI_GreedyHM_V();
    }
    else
    {
        return DI_GreedyHM_NV();
    }
    
    return TRUE;
}

BOOL UpdateFieldStore()
{
    if (GreedyUsePulldown)
    {
        if (GreedyUseMedianFilter && MedianFilterAmt > 0)
        {
            FsDelay = 2;
            if (GreedyUseHSharpness && GreedyHSharpnessAmt)
            {
                if (GreedyHSharpnessAmt > 0)
                {
                    return DI_GrUpdtFS_M_E_P();
                }
                else
                {
                    return DI_GrUpdtFS_M_E_P_Soft();
                }
            }
            else
            {
                return DI_GrUpdtFS_M_NE_P();
            }
        }
        else 
        {
            FsDelay = 1;
            if (GreedyUseHSharpness && GreedyHSharpnessAmt)
            {
                if (GreedyHSharpnessAmt > 0)
                {
                    return DI_GrUpdtFS_NM_E_P();
                }
                else
                {
                    return DI_GrUpdtFS_NM_E_P_Soft();
                }
            }

            else
            {
                return DI_GrUpdtFS_NM_NE_P();
            }
        }
    }
    else
    {
        if (GreedyUseMedianFilter && MedianFilterAmt > 0)
        {
            FsDelay = 2;
            if (GreedyUseHSharpness && GreedyHSharpnessAmt > 0)
            {
                if (GreedyHSharpnessAmt > 0)
                {
                    return DI_GrUpdtFS_M_E_NP();
                }
                else
                {
                    return DI_GrUpdtFS_M_E_NP_Soft();
                }
            }
            else
            {
                return DI_GrUpdtFS_M_NE_NP();
            }
        }
        else 
        {
            FsDelay = 1;
            if (GreedyUseHSharpness && GreedyHSharpnessAmt > 0)
            {
                if (GreedyHSharpnessAmt > 0)
                {
                    return DI_GrUpdtFS_NM_E_NP();
                }
                else
                {
                    return DI_GrUpdtFS_NM_E_NP_Soft();
                }
            }
            else
            {
                return DI_GrUpdtFS_NM_NE_NP();
            }
        }
    }
}

__int64 QHA;        // used for sharpness calcs
__int64 QHB;        // used for sharpness calcs
__int64 QHC;        // used for sharpness calcs

#undef REALLY_USE_SOFTNESS

// A version of UpdateFieldStore with Median Filter and Edge Enhancement and Pulldown
#define USE_PULLDOWN
#define USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_M_E_P
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with Median Filter but no Edge Enhancement and Pulldown
#define USE_PULLDOWN
#define USE_MEDIAN_FILTER
#undef USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_M_NE_P
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with no Median Filter or Edge Enhancement and Pulldown
#define USE_PULLDOWN
#undef USE_MEDIAN_FILTER
#undef USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_NM_NE_P
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with no Median Filter but Edge Enhancement and Pulldown
#define USE_PULLDOWN
#undef USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_NM_E_P
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with Median Filter and Edge Enhancement and no Pulldown
#undef USE_PULLDOWN
#define USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_M_E_NP
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with Median Filter but no Edge Enhancement and no Pulldown
#undef USE_PULLDOWN
#define USE_MEDIAN_FILTER
#undef USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_M_NE_NP
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with no Median Filter or Edge Enhancement and no Pulldown
#undef USE_PULLDOWN
#undef USE_MEDIAN_FILTER
#undef USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_NM_NE_NP
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with no Median Filter but Edge Enhancement and no Pulldown
#undef USE_PULLDOWN
#undef USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_NM_E_NP
#include "DI_GrUpdtFS.asm"

// Add 4 more flavors where Sharpness (Edge Enhancement) is negative meaning Softness
#define REALLY_USE_SOFTNESS

// A version of UpdateFieldStore with Median Filter and SOFT Edge Enhancement and Pulldown
#define USE_PULLDOWN
#define USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_M_E_P_Soft
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with no Median Filter but SOFT Edge Enhancement and Pulldown
#define USE_PULLDOWN
#undef USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_NM_E_P_Soft
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with Median Filter and SOFT Edge Enhancement and no Pulldown
#undef USE_PULLDOWN
#define USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_M_E_NP_Soft
#include "DI_GrUpdtFS.asm"

// A version of UpdateFieldStore with no Median Filter but SOFT Edge Enhancement and no Pulldown
#undef USE_PULLDOWN
#undef USE_MEDIAN_FILTER
#define USE_SHARPNESS        
#undef FUNC_NAME        
#define FUNC_NAME DI_GrUpdtFS_NM_E_NP_Soft
#include "DI_GrUpdtFS.asm"


// copy 1 line from Fieldstore to overlay buffer, mult of 32 bytes
BOOL FieldStoreCopy(BYTE * dest, __int64 * src, int clen)
{
    int ct = clen / 32;
    _asm
    {
        mov        esi, src
        mov        edi, dest                    // new output line dest
        mov        ecx, ct

cloop:    
        movq    mm0, qword ptr[esi]
        movq    mm1, qword ptr[esi+FSCOLSIZE]
        movq    mm2, qword ptr[esi+FSCOLSIZE*2]
        movq    mm3, qword ptr[esi+FSCOLSIZE*3]
        movntq    qword ptr[edi], mm0
        movntq    qword ptr[edi+8], mm1
        movntq    qword ptr[edi+16], mm2
        movntq    qword ptr[edi+24], mm3
        lea        esi, [esi+FSCOLSIZE*4]
        lea        edi, [edi+32]
        loop    cloop                        // go do next 4 qwords
        sfence
        emms
    }
    return TRUE;
}

// return FS subscripts depending on delay
BOOL SetFsPtrs(int* L1, int* L2, int* L2P, int* L3, int* CopySrc, BYTE** CopyDest, BYTE** WeaveDest)        
{
    if (FsDelay == 2)
    {
        if (InfoIsOdd)
        {
            // Assume here we are doing median filtering so we are delaying by 2 fields.
            // When we are doing Median filter we have to delay the display by 2 fields so at Time=5
            // we are displaying a screen for Time=3, For ODD fields we display an odd field and have
            // the following, representing part of 1 column on the screen when Line=0, 
            // Time = 5, and W = the weave pixel we want to calc:

            //  Row  Fields (at Time=1..5)  Just got odd field 5, display odd frame 3
            //  ---  --------------------
            //        1  2  3  4  5
            //  -1            L1      x        Not really any -1 row but we pretend at first            
            //   0      L2P W  L2        We create the W pixel somehow, FsPtrP will point to L2
            //   1          L3      x        Odd Rows directly copied, FsPtrP2 will point to L3

            *L3 = FsPtrP2;                // Bottom curr pixel offset is prev odd pixel
            *L1 = *L3 - FSMAXCOLS;        // top curr pixel offset, tricked on 1st line
            *L2 = FsPtrP;                // the newest weave pixel
            *L2P = FsPtrP3;                // the Oldest weave pixel
            *CopySrc = FsPtrP2;            // Always copy from prev pixels
            *WeaveDest = lpCurOverlay;   // where the weave pixel goes
            *CopyDest = lpCurOverlay+OverlayPitch;    // Dest for copy or vert filter pixel pixel
        }

        else
        {
            //  Row  Fields (at Time=1..5)  Just got even frame 4, display even frame 2
            //  ---  --------------------
            //        1  2  3  4  5
            //   0       L1       x        Even Rows are directly copied, FsPtrP2 will point to L1
            //   1    L2P     W  L2            We create the W pixel somehow, FsPtrP will point to L2
            //   2       L3    x        Even Rows directly copied for Even fields

            *L1 = FsPtrP2;                // top curr pixel subscript
            *L3 = *L1 + FSMAXCOLS;            // bottom curr pixel subscript, tricked on last line
            *L2 = FsPtrP;                // the newest weave pixel
            *L2P = FsPtrP3;                // the Oldest weave pixel
            *CopySrc = FsPtrP2;            // Always copy from prev pixels
            *WeaveDest = lpCurOverlay + OverlayPitch;  // Dest for weave pixel
            *CopyDest = lpCurOverlay;    // Dest for copy or vert filter pixel pixel
        }
    }
    else                                            // Assume FsDelay = 1;
    {
        if (InfoIsOdd)
        {
            // Assume here we are not doing median filtering so we are delaying only one field
            // When we are not doing Median filter we have to delay the display by 1 fields so at Time=5
            // we are displaying a screen for Time=4, For ODD fields we display an even field and have
            // the following, representing part of 1 column on the screen when Line=0, 
            // Time = 5, and W = the weave pixel we want to calc:

            //  Row  Fields (at Time=1..5)  Just got odd frame 5, display even frame 4
            //  ---  --------------------
            //        1  2  3  4  5
            //   0       x     L1       Even Rows are directly copied, FsPtrP will point to L1
            //   1      L2P    W  L2    We create the W pixel somehow, PsPtr will point to L2
            //   2       x     L3       Even Rows directly copied for Odd fields
            //                            Note L3 not avail on last line, L1 used twice there
            *L1 = FsPtrP;                // top curr pixel subscript
            *L3 = *L1+FSMAXCOLS;        // bottom curr pixel subscript, tricked on last line
            *L2 = FsPtr;                 // the newest weave pixel
            *L2P = FsPtrP2;                // the Oldest weave pixel
            *CopySrc = FsPtrP;            // Always copy from prev pixels
            *WeaveDest = lpCurOverlay + OverlayPitch;  // Dest for weave pixel
            *CopyDest = lpCurOverlay;            // Dest for copy or vert filter pixel pixel
        }

        else
        {
            //  Row  Fields (at Time=1..5)  Just got even frame 4, display odd frame 3
            //  ---  --------------------
            //        1  2  3  4  5
            //  -1            L1            Not really any -1 row but we pretend at first            
            //   0      L2P W  L2        We create the W pixel somehow, PsPtr will point to L2
            //   1          L3            Odd Rows directly copied, FsPtrP will point to L3

            *L3 = FsPtrP;                // Bottom curr pixel offset is prev odd pixel
            *L1 = *L3 - FSMAXCOLS;        // top curr pixel offset, tricked on 1st line
            *L2 = FsPtr;                 // the newest weave pixel
            *L2P = FsPtrP2;                // the Oldest weave pixel
            *CopySrc = FsPtrP;            // Always copy from prev pixels
            *WeaveDest = lpCurOverlay;
            *CopyDest = lpCurOverlay+OverlayPitch;    // Dest for copy or vert filter pixel pixel
        }
    }
    return TRUE;
}