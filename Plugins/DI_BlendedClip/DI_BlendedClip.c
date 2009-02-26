/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 Tom Barry.  All rights reserved.
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

#include "windows.h"
#include "DS_Deinterlace.h"
#include "resource.h"
#include "..\help\helpids.h"

HINSTANCE hInst = NULL;

///////////////////////////////////////////////////////////////////////////////////
// Blended Clipping Deinterlace - Tom Barry 11/09/2000
///////////////////////////////////////////////////////////////////////////////////
// As written, Blended Clipping Deinterlace method is really designed as a testing
// laboratory.  It uses a huge confusing control panel and it has way too many
// controls to be user friendly as a final product.  It does, however, allow
// us to easily experiment with what might work best.  It may be able to provide an 
// example of some of the things that might be possible if we find a few sets of 
// parms that work well for various situations.  It can create both some decent results
// and also some very bad results depending upon both the input video and the
// parameter settings.

// Note that while I've attempted to optimize the assembler code, this method still
// eats a fair amount of CPU time.  In order to run it on my 350 or 450 mhz machines
// I must have all the DScaler performance options turned on.

// How it works:

// Deinterlace algorithms face a choice between using the most recently received
// information which is accurate in time and using older field information which
// may be more accurate in space.  Adaptive algorithms make this choice on the fly
// for each field and/or pixel. Blended algorithms try to smoothly blend two or more  
// choices together, avoiding rapidly blinking pixels and some other artifacts. The
// Blended Clipping algorithm attempts to do all of the above.
//
// Assume we are processing an odd field.  Like most other simpler deinterlacing 
// methods, the Blended Clipping method will just use the current field pInfo unchanged
// for the odd lines.  Odd lines are just copied to the video buffer for now though
// this too might be improved in the future.
//
// For the even lines we will create a blend of the Weave pixels (the previous even lines)
// and a Clipped Weave, which is itself a blend.
//
// The Clipped Weave is just the weave pixel unless it's luma (and optionally chroma) is
// outside the high/low range of its vertically adjacent pixels.  If so, the luma (chroma)
// values are clipped to the nearest adjacent value.  This is intended to be a 
// replacement for the various Interpolated BOB implementatons that usually represent
// the current time estimated value for the even lines.  To see the pure value of a
// Clipped Weave, without any other blending, just max out the "Minimum Clip" slider
// on the contol panel.  It still creates some BOB-like artifacts but they are less
// visible and don't seem to have the rapid up/down jitter of BOB implementations.

// Exactly what percentage of the Weave vs. Clipped Weave we use depends upon a number
// number of factors, each controlled by a slider in the Blended Clipping control panel.
// There are also INI parms for these values.  (currentlly 9 sliders, 2 check boxes)

// Most of the parameters should be thought of as unitless fuzzy preference values
// in the range of 0..100. The INI file parm names will be in the [deinterace] section
// will be the same as the global definitions.

// The slider names and descriptions follow.  The INI file/global names follow 
// each description:

// "Minimum Clip" slider: This can be uses to increase the amount of Clip vs Weave
// in the event that none of the other more specific values seem to work and you
// still see too many Weave artifacts ("venetion blinds").  It's best to try everything
// else first.

long        BlcMinimumClip = -15;               // currently -100 .. 100

// "Pixel Motion Sensitivity" slider:  This determines how sensitive we are to motion.
// Motion is calculated as the maximum absolute change in luma from the previous field
// in either of the two vertically adjacent pixels.  I was going to use the change in 
// luma of the Weave pixel but that is not current enough to prevent the "flash attacks"
// of venetiaon blinds that can occur with sudden scene changes.  This value is
// calculated separately for each pixel.

long    BlcPixelMotionSense = 17;

// "Recent Motion Sensitivity" slider:  This increases the tendency to use Clip based
// upon an n-period Exponential Moving Average of the recent motion.  Recent motion
// is in turn an arithmetic average of how much each pixel's luma has changed from the
// previous field.  These values are self obtained and maintained.  While this method
// does not attempt to do 3:2 pulldown I believe the motion values could be of assistance
// in the routines that do.  

long        BlcRecentMotionSense = 0;       // current -100 .. 100)     

// "Motion Average Period" slider:  This sets the period of the moving average for Recent
// Motion Sensitivity.  

// For those of you not recently programming for the stock market:

//      An Exponential Moving Average is a way to keep and update an average without 
//      keeping around a history of all the recent events.  In the stock market,
//      an n Period Exponential Moving Average of variable X is usually defined as:

//          X_new_avg = ( X_old_avg * (n-1) + 2 * X) / (n+1)

long    BlcMotionAvgPeriod = 20;        // currently 1..200

// "Pixel Comb Sensitivity" slider:  This determines how sensitive we are to the current
// comb factor of each pixel.  I used a simplified comb factor C = abs(2*W - H - L)/2,
// which is just the distance of the Weave pixel's luma from its interpolated value.
// This value is calculated separately for each pixel.  This value along with the Pixel
// Motion Sense seem to be the two main things to play with to get good results.  Generally,
// increase one of these if you get Weave artifacts and decrease one if you get BOB artifacts.
  
long    BlcPixelCombSense = 27;

// "Recent Comb Senseitivity" slider:  Operates like the Recent Motion slider but operates
// on the average Comb Factor.

long    BlcRecentCombSense = 0;

// "Comb Average Period" slider: Sets the period of the Comb exponential moving average.
// See the comments on "Motion Average Period".

long    BlcCombAvgPeriod = 20;          // currently 1.200

// "Skip High Comb Frames" slider:  I added this one in the hopes that it could help to
// skip a frame in the event of a sudden flash attack on a rapid scene change or maybe
// help to handle some very poorly mastered anime using ?:? pulldown.  I have not had
// a chance to experiment with it yet.  It will give very ugly results if you set it 
// too high.

long    BlcHighCombSkip = 10;           // larger values skip more

// "Skip Low Motion Frames" slider:  This also is just experimental an probably of low
// value.  The idea here is that any frame with sufficiently low change from the previous
// one is maybe a still frame with some video noise, and could be skipped.  Not for
// normal use.  NOTE - This slider (but not parm) will soon be replaced by the
// Vertical Smoothing slider.

long    BlcLowMotionSkip = 0;           // larger values skip more

// "Vertical Smoothing" slider: Sets a smoothing constant to smooth between the even
// and odd lines.  Not yet implemented, but the INI parm is there.

long    BlcVerticalSmoothing = 0;

// "Use Interpolated BOB instead of Clip" check box.  For those who don't like the
// Clipped Weave, this will change it to an Interpolated Bob.  All other blending and
// processing will still function. (but it probably won't look as good).

BOOL    BlcUseInterpBob = FALSE;

// "Blend Chroma Value" check box:  Usually the chroma value for the Clipped Weave is
// just taken from the pixel above it.  Checking this box causes the chroma values to 
// also use the clip algoritm.

// Checking this box seems to get richer color detail and a more theater like picture
// but it sometimes seems to create some softness or shimmering on my stock ticker or
// rapidly moving objects with lots of detail like a hockey game.

BOOL    BlcBlendChroma = TRUE;          // default should maybe be TRUE?

// Finally there is an INI parm, but not a contol to determine whether to even display
// the Blended Clipping controls when that method is selected. If set to false then
// Blended Clipping parms are determined only from the INI file.

BOOL    BlcShowControls = TRUE;

// Other global values, not user parms:

long    BlcAverageMotions[5][2] = {0};  // reserved
long    BlcTotalAverageMotion = 0;
long    BlcAverageCombs[5][2] = {0};    // reserved
long    BlcTotalAverageComb = 0;
BOOL    BlcWantsToFlip;

HWND ghDlg = NULL;

BOOL DeinterlaceBlendedClip(TDeinterlaceInfo* pInfo)
{
    int Line;
    int LoopCtr;
    int OddPtr;
    long X;
    BYTE* L1;                  // ptr to Line1, of 3
    BYTE* L2;                  // ptr to Line2, the weave line
    BYTE* L3;                  // ptr to Line3
    BYTE* LP1;                 // ptr to prev Line1
    BYTE* LP3;                 // ptr to prev Line3
    BYTE* Dest = pInfo->Overlay;
    DWORD Pitch = pInfo->InputPitch;
    const __int64 YMask     = 0x00ff00ff00ff00ff;   // to keep only luma
    const __int64 UVMask    = 0xff00ff00ff00ff00;   // to keep only chroma
    const __int64 ShiftMask = 0xfefffefffefffeff;   // to avoid shifting chroma to luma
    const __int64 SomeOnes  = 0x0001000100010001;   
    __int64 i;
    __int64 MinClip;
    __int64 MinClipMinus;
    __int64 PixelMotionSense;
    __int64 PixelCombSense;
    __int64 L1Mask;                 // determines blended chroma vs. chroma from line 1
    __int64 LCMask;                 // determines whether we use clip or blend chroma
    __int64 BobMask;                // determines whether we bob or blend chroma
    __int64 MotionAvgL;             // work sum for one line
    __int64 CombAvgL;               // "
    __int64 MotionAvg;              // total sum/avg
    __int64 CombAvg;                // "
    
    union 
    {
        __int64 Wqword;         
        __int32 Wlong[2];
    } W;

    // Set up and scale our user parms
    MotionAvg = 0;
    CombAvg = 0;


    if (BlcBlendChroma)
    {
        if (BlcUseInterpBob)        // use Luma, chroma from interp bob 
        {
            L1Mask = 0;
            LCMask = 0;
            BobMask = 0xfefffefffefffeff;
        }
        else                        // use Luma, chroma from Clip
        {
            L1Mask = 0;
            LCMask = 0xffffffffffffffff;
            BobMask = 0;
        }
    }
    else
    {
        if (BlcUseInterpBob)        // use Luma from bob, chroma from L1
        {
            L1Mask = UVMask;
            LCMask = 0;
            BobMask = YMask;
        }
        else                        // use Luma from Clip, chroma from L1
        {
            L1Mask = UVMask;
            LCMask = YMask;
            BobMask = 0;
        }
    }

// The effects of the recent average motion, recent average comb, and their relative
// weighting parameters can be precalculated only once per frame and rolled into the
// working value of the user constant parm, BlcMinimumClip, so we don't have to do
// calculations on these for every pixel.  Only the greater of recent motion or comb is used.

// A certain amount of fudging is needed for these values in the next few lines since it
// is desirable the effects center about realistic actual values of avg motion & comb.  
// A random sampling of the values in both avg motion and avg comb show values usually
// in the 1000-4000 range, with occasional jumps to much higher values.  Since the user
// parms are 0-100 and we want output values 0-65535 we will assume a max usual value for each
// average of 4500 and so divide by ((100*4500)/65535)) = 7 for now. (up to 1/5 for new Comb)

// Note the motion and comb average values have been scaled up by 256 in the averaging rtn, so
// the typical value of 2000 means an average change of about 8 in the 8 bit luma values.
// Both BlcMinimumClip and BlcRecentMotionSense may now have negative values but since
// we are using saturated arithmatic those are set in a separate field.
    X = __max(BlcRecentMotionSense,0);
    X = (X * BlcTotalAverageMotion / 7) 
                + (BlcRecentCombSense * BlcTotalAverageComb) / 5;
    i = __max((BlcMinimumClip * 65535 / 100), 0);
    i = __min( (X + i), 65535);             // scale to range of 0-65535
    MinClip = i << 48 | i << 32 | i << 16 | i;
    
    X = __max( (-BlcRecentMotionSense * BlcTotalAverageMotion / 10), 0)
        + __max( (-BlcMinimumClip), 0);
    i = __min(X, 255);
    MinClipMinus = i << 48 | i << 32 | i << 16 | i;

// Set up our two parms that are actually evaluated for each pixel
    i = BlcPixelMotionSense * 257/100;      // scale to range of 0-257
    PixelMotionSense = i << 48 | i << 32 | i << 16 | i;    // only 32 bits?>>>>
    
    i = BlcPixelCombSense * 257/100;        // scale to range of 0-257
    PixelCombSense = i << 48 | i << 32 | i << 16 | i;    // only 32 bits?>>>>
    
    OddPtr = (pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_ODD) ? 1 : 0;
    
// copy first even line no matter what, and the first odd line if we're
// processing an odd field.
    if(OddPtr)
    {
        L1 = pInfo->PictureHistory[0]->pData;
        L2 = pInfo->PictureHistory[1]->pData + Pitch;  
        L3 = L1 + Pitch;   
        LP1 = pInfo->PictureHistory[2]->pData;         // prev Odd lines
        LP3 = LP1 + Pitch;       

        // copy first even line
        pInfo->pMemcpy(Dest, pInfo->PictureHistory[1]->pData, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;

        // copy first odd line
        pInfo->pMemcpy(Dest, L1, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;

    }
    else
    {
        L1 = pInfo->PictureHistory[0]->pData;
        L2 = pInfo->PictureHistory[1]->pData;  
        L3 = L1 + Pitch;   
        LP1 = pInfo->PictureHistory[2]->pData;         // prev Odd lines
        LP3 = LP1 + Pitch;       

        // copy first line
        pInfo->pMemcpy(Dest, L1, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;
    }

    for (Line = 0; Line < (pInfo->FieldHeight - 1); ++Line)
    {
        LoopCtr = pInfo->LineLength / 8;             // there are LineLength / 8 qwords per line
        MotionAvgL = 0;
        CombAvgL = 0;

// For ease of reading, the comments below assume that we're operating on an odd
// field (i.e., that pInfo->IsOdd is true).  The exact same processing is done when we
// operate on an even field, but the roles of the odd and even fields are reversed.
// It's just too cumbersome to explain the algorithm in terms of "the next odd
// line if we're doing an odd field, or the next even line if we're doing an
// even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
// half the time this function is called, those words' meanings will invert.

        _asm
        {
            mov eax, dword ptr [L1]     
            mov ebx, dword ptr [L2]     
            mov edx, dword ptr [L3]     
            mov esi, dword ptr [LP1]        
            mov ecx, dword ptr [LP3]   
            mov edi, dword ptr [Dest]       // DL2 if Odd or DL1 if Even 
            movq mm7,YMask                  // useful constant within loop 
            
            align 8
DoNext8Bytes:           
            movq mm0, qword ptr[eax]        // L1
            movq mm1, qword ptr[ebx]        // L2
            movq mm2, qword ptr[edx]        // L3

// OK, now we've moved in DL3 and Dl1 (earlier).  We have to make something for DL2
// What we will do is use L2 unless it is > Max(L1,L3) or < Min(L2,L3). If so clip to the closer.
// Note we are clipping all of Y,U, and V values here, not just masking Luma

            movq mm4, mm0                   // L1
            movq mm6, mm2                   // L3
            psubusb mm4, mm2                // - L3, with saturation
            paddusb mm4, mm2                // now = Max(L1,L3)

            pcmpeqb mm3, mm3                    // all ffffffff
            psubusb mm3, mm0                // - L1 
            paddusb mm6, mm3                // add may sat at fff..
            psubusb mm6, mm3                // now = Min(L1,L3)
            
            movq mm5,mm1                    // work copy of L2,the weave value
            psubusb mm5, mm6                // L2 - Min
            paddusb mm5, mm6                // now = Max(L2,Min(L1,L3)

            pcmpeqb mm3, mm3                // all ffffffff
            psubusb mm3, mm5                // - Max(L2,Min(L1,L3) 
            paddusb mm4, mm3                // add may sat at FFF..
            psubusb mm4, mm3                // now = Min( Max(L2, Min(L1,L3), L2 )=L2 clipped

// We have created the clipped value but we may not want to use it, depending on user
// parm BlcUseInterpBoB.  Make the bob value too.
            
            movq    mm5, mm0                // L1
            pand    mm5, ShiftMask          // "
            psrlw   mm5, 1
            movq    mm6, mm2                // L3
            pand    mm6, ShiftMask          // "
            psrlw   mm6, 1
            paddb   mm5, mm6                // interpolated bob here is just the average

// Now use our preset flag fields to select which we want

            movq    mm6, mm5                // copy of Bob
            pand    mm6, BobMask            // may mask out nothing, chroma, or all
            pand    mm4, LCMask             // may mask out nothing, chroma, or all
            por     mm4, mm6                // our choice, with or without chroma

// Now is a good time to calculate the Comb Factor.  A simple version is just the
// distance between L2 and the bob (interpolated middle) value now in mm5.
//  try a different way for now, maybe change back later - TRB 11/14/00
            movq    mm6, mm1                // L2
            psubusb mm6, mm5                // L2 - bob, with sat
            psubusb mm5, mm1                // bob - L2
            por     mm5, mm6                // abs diff (bob - L2)
            pand    mm5, mm7                // keep only luma
            movq    mm6, mm5                // save a copy for pixel comb sense calc
            paddusw mm5, CombAvgL           // bump our hist average
            movq    CombAvgL, mm5           // and save again
            pmullw  mm6, PixelCombSense     // mul by user factor, keep only low 16 bits


// Instead let's let the Comb Factor just be the difference between L2 and the clipped
// value.  It will be zero for any pixel lying in the range where it does not get clipped.
// This avoids penalizing pixels that just happens to be in a high vertical contrast area.
// Doing this gives an adjustment similar to the use of the EdgeDetect value in the
// original Video Delinterlace routine.
            movq    mm6, mm1                // L2
            movq    mm5, mm4                // our clipped value, call it LC
            psubusb mm6, mm5                // L2 - LC, with sat
            psubusb mm5, mm1                // LC - L2
            por     mm5, mm6                // abs diff (LC - L2)
            pand    mm5, mm7                // keep only luma
            movq    mm6, mm5                // save a copy for pixel comb sense calc
            paddusw mm5, CombAvgL           // bump our hist average
            movq    CombAvgL, mm5           // and save again
            psubusb mm6, MinClipMinus       // possibly forgive small values
            pmullw  mm6, PixelCombSense     // mul by user factor, keep only low 16 bits
            paddusw mm6, mm6                // try making it bigger
            paddusw mm6, mm6                // again
            paddusw mm6, mm6                // again
            paddusw mm6, mm6                // again
            paddusw mm6, mm6                // again
            paddusw mm6, mm6                // again

// Let's see how much L1 or L3 have changed since the last frame.  If L1 or L3 has  
// changed a lot (we take the greater) then L2 (the weave pixel) probably has also.
// Note that just measuring the change in L2 directly would not be current enough
// in time to avoid the 'flash attack' of weaves during quick scene changes.
// Calc first for L3 change but sum only L1 change in our saved totals for the average.
// L2 is destroyed here, no longer available.

            movq    mm5, qword ptr[ecx]     // LP3, prev L3
            movq    mm3, mm2                // work copy of L3
            psubusb mm3, mm5                // L3 - LP3
            psubusb mm5, mm2                // LP3 - L3
            por     mm5, mm3                // abs(L3 - LP3)
            pand    mm5, mm7                // Ymask, keep luma

            movq    mm3, mm0                // L1, another copy
            movq    mm2, qword ptr[esi]     // LP1, the previous value of L1
            psubusb mm3, mm2                // L1 - LP1, unsigned & sat.
            psubusb mm2, mm0                // LP1 - L1, unsigned & sat.
            por     mm2, mm3                // abs(L1-LP1)
            pand    mm2, mm7                // Ymask, keep luma

            movq    mm3, MotionAvgL         // good time to update our average totals
            paddusw mm3, mm2
            movq    MotionAvgL, mm3

            psubusb mm2, mm5
            paddusb mm2, mm5                // max of abs(L1-LP1) and abs(L3-LP3)
            psubusb mm2, MinClipMinus       // but maybe ignore some small changes

            pmullw  mm2, PixelMotionSense   // mul by user factor, keep only low 16 bits        
            paddusw mm2, mm6                // combine with our pixel comb
            paddusw mm2, mm2                // let's dbl them both for greater sensitivity

// Now turn the motion & comb factors in mm2 into a 2 blending factors that sum to 256

            paddusw mm2,MinClip             // add user and history factors to bias upward
            pcmpeqw mm3,mm3                 // set all fff... = 65536
            psubusw mm3,mm2                 // get (more or less) 64k-mm2
            paddusw mm2, SomeOnes           // adjust so they total 64K?
            psrlw   mm2,8                   // clip factor
            psrlw   mm3,8                   // weave factor, both sum to +-256 

// We still have the clipped (or bob) value in mm4.  Let's call it LC below.
// mm2 and mm3 should now have factors for how much clip & weave respecively, sum=256
            movq    mm5,mm4                 // save copy of clipped val, LC
            pand    mm4,mm7                 // Ymask, keep luma from clipped val, LC
            pmullw  mm4,mm2                 // clip fact * Clip luma
            pand    mm1,mm7                 // Ymask, keep luma from weave val, L2
            pmullw  mm1,mm3                 // weave fact * weave luma
            paddusw mm4,mm1                 
            psrlw   mm4,8                   // back to 8 bit luma

// combine luma results with chroma and store 4 pixels
            pand    mm5, UVMask             // maybe keep Chroma from LC, or 0
            pand    mm0, L1Mask             // Maybe chroma comes from here
            por     mm4, mm0
            por     mm4, mm5                // combine them and we've got it
            movq qword ptr[edi], mm4        // and that is our final answer

// bump ptrs and loop
            lea     eax,[eax+8]             
            lea     ebx,[ebx+8]
            lea     ecx,[ecx+8]
            lea     edx,[edx+8]
            lea     edi,[edi+8]         
            lea     esi,[esi+8]
            dec     LoopCtr
            jnz     DoNext8Bytes

// done with one line but have to roll some totals that might otherwise overflow
            movq    mm0, CombAvgL
            pmaddwd mm0, SomeOnes
            paddd   mm0, CombAvg
            movq    CombAvg, mm0
            
            movq    mm0, MotionAvgL
            pmaddwd mm0, SomeOnes
            paddd   mm0, MotionAvg
            movq    MotionAvg, mm0
        }
        Dest += pInfo->OverlayPitch;
        pInfo->pMemcpy(Dest, L3, pInfo->LineLength);
        Dest += pInfo->OverlayPitch;

        L1 += Pitch;
        L2 += Pitch;  
        L3 += Pitch;   
        LP1 += Pitch;
        LP3 += Pitch;       
    }

    // Copy last odd line if we're processing an even field.
    if(pInfo->PictureHistory[0]->Flags & PICTURE_INTERLACED_EVEN)
    {
        pInfo->pMemcpy(Dest,
                  L2,
                  pInfo->LineLength);
        
    }

    // need to clear up MMX registers
    _asm
    {
        emms
    }

    // We will keep moving averages of the Motion and Comb factors.  For extra precision the 
    // values will be kept scaled up by 256.  See comments on rtn header about averages.

    BlcWantsToFlip = TRUE;          // assume we do 

    W.Wqword = MotionAvg;
    X = 256 * (W.Wlong[0] + W.Wlong[1]) / ( (pInfo->FieldHeight - 1) * pInfo->FrameWidth );

    // Do we want to skip the frame because motion is too small? Check before updating avg.
    if (BlcTotalAverageMotion > 0  
            && (100 * X / BlcTotalAverageMotion) < BlcLowMotionSkip)
    {
        BlcWantsToFlip = FALSE;
    }
    
    BlcTotalAverageMotion = (BlcTotalAverageMotion * (BlcMotionAvgPeriod - 1) + 2 * X)
            / (BlcMotionAvgPeriod + 1);

    W.Wqword = CombAvg;
    X = 256 * (W.Wlong[0] + W.Wlong[1]) / ( (pInfo->FieldHeight - 1) * pInfo->FrameWidth );

    if (BlcTotalAverageComb > 0       // Skip a very high comb frame?
            && (100 * X / BlcTotalAverageComb) > 10 * (100 - BlcHighCombSkip))
    {
        BlcWantsToFlip = FALSE;
    }
    
    BlcTotalAverageComb = (BlcTotalAverageComb * (BlcCombAvgPeriod - 1) + 2 * X)
            / (BlcCombAvgPeriod + 1);

    return BlcWantsToFlip;
}

void SetHorSliderInt(HWND hDlgItem, int yPos, int Value, int nMin, int nMax)
{
    int x = 5 + 160 *(Value - nMin) / (nMax - nMin);
    MoveWindow(hDlgItem, x, yPos +5, 10, 12, TRUE);
}

int GetHorSliderInt(int MouseX, int nMin, int nMax)
{
    int i;
    i = nMin + (MouseX - 5) * (nMax - nMin) / 160;
    if (i < nMin)
        i = nMin;
    else if (i > nMax)
        i = nMax;
    return i;
}

void SetBlcDisplayControls(HWND hDlg)
{
    // Set the sliders and visible numbers correctly
    SetDlgItemInt(hDlg, IDC_MIN_CLIP_V, BlcMinimumClip, TRUE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_MIN_CLIP),  35, BlcMinimumClip,  -100, 100);

    SetDlgItemInt(hDlg, IDC_PIXEL_MOV_V, BlcPixelMotionSense, FALSE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_PIXEL_MOV),  85, BlcPixelMotionSense,  0, 100);
    
    SetDlgItemInt(hDlg, IDC_AVG_MOV_V, BlcRecentMotionSense, TRUE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_AVG_MOV),  135, BlcRecentMotionSense,  -100, 100);
    
    SetDlgItemInt(hDlg, IDC_MOV_PERIOD_V, BlcMotionAvgPeriod, FALSE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_MOV_PERIOD),  185,  BlcMotionAvgPeriod,  1, 200);
    
    SetDlgItemInt(hDlg, IDC_PIXEL_COMB_V, BlcPixelCombSense, FALSE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_PIXEL_COMB),  235, BlcPixelCombSense,  0, 100);
    
    SetDlgItemInt(hDlg, IDC_AVG_COMB_V, BlcRecentCombSense, FALSE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_AVG_COMB),  285, BlcRecentCombSense,  0, 100);
    
    SetDlgItemInt(hDlg, IDC_COMB_PERIOD_V, BlcCombAvgPeriod, FALSE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_COMB_PERIOD),  335, BlcCombAvgPeriod,  1, 200);
    
    SetDlgItemInt(hDlg, IDC_COMB_SKIP_V, BlcHighCombSkip, FALSE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_COMB_SKIP),  385, BlcHighCombSkip,  0, 100);
    
    SetDlgItemInt(hDlg, IDC_MOTION_SKIP_V, BlcLowMotionSkip, FALSE);
    SetHorSliderInt(GetDlgItem(hDlg, IDC_MOTION_SKIP),  435, BlcLowMotionSkip,  0, 100);
    
    CheckDlgButton(hDlg, IDC_BLEND_CHROMA, BlcBlendChroma);
    CheckDlgButton(hDlg, IDC_USE_INTERP_BOB, BlcUseInterpBob);
    

}
// dialog code for new Blended Clipping Deinterlace control panel - TRB 11/00
BOOL APIENTRY BlendedClipProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int x, y;
    static int TMinimumClip;
    static UINT TPixelMotionSense;
    static int TRecentMotionSense;
    static UINT TMotionAvgPeriod;
    static UINT TPixelCombSense;
    static UINT TRecentCombSense;
    static UINT TCombAvgPeriod;
    static UINT THighCombSkip;
    static UINT TLowMotionSkip;
    static BOOL TUseInterpBob;
    static BOOL TBlendChroma;
    switch (message)
    {
    case WM_INITDIALOG:

        // Capture the current global values
        TMinimumClip = BlcMinimumClip;
        TPixelMotionSense = BlcPixelMotionSense;
        TRecentMotionSense = BlcRecentMotionSense;
        TMotionAvgPeriod = BlcMotionAvgPeriod;
        TPixelCombSense = BlcPixelCombSense;
        TRecentCombSense = BlcRecentCombSense;
        TCombAvgPeriod = BlcCombAvgPeriod;
        THighCombSkip = BlcHighCombSkip;
        TLowMotionSkip = BlcLowMotionSkip;
        TUseInterpBob = BlcUseInterpBob;
        TBlendChroma = BlcBlendChroma;
        SetBlcDisplayControls(hDlg);
        return TRUE;

        break;

    case WM_DESTROY:
        ghDlg = NULL;
        return TRUE;
        break;

    case WM_MOUSEMOVE:

        if (wParam == MK_LBUTTON)
            {
                y = HIWORD(lParam);
                x = LOWORD(lParam);
                
                if ((x >= 5) && (x <= 166))
                {
                    if ((y >= 35) && (y <= 55))     // Is Minimum clip slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_MIN_CLIP), x, 35+5, 10, 12, TRUE);
                        BlcMinimumClip = GetHorSliderInt(x, -100, 100);
                        SetDlgItemInt(hDlg, IDC_MIN_CLIP_V, BlcMinimumClip, TRUE);
                    }
                    if ((y >= 85) && (y <= 105))        // Is Pixel Motion slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_PIXEL_MOV), x, 85+5, 10, 12, TRUE);
                        BlcPixelMotionSense = GetHorSliderInt(x, 0, 100);
                        SetDlgItemInt(hDlg, IDC_PIXEL_MOV_V, BlcPixelMotionSense, TRUE);
                    }
                    if ((y >= 135) && (y <= 155))       // Is Avg Hist slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_AVG_MOV), x, 135+5, 10, 12, TRUE);
                        BlcRecentMotionSense = GetHorSliderInt(x, -100, 100);
                        SetDlgItemInt(hDlg, IDC_AVG_MOV_V, BlcRecentMotionSense, TRUE);
                    }
                    if ((y >= 185) && (y <= 205))       // Is Hist Mov Avg Peroid clip slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_MOV_PERIOD), x, 185+5, 10, 12, TRUE);
                        BlcMotionAvgPeriod = GetHorSliderInt(x, 1, 200);
                        SetDlgItemInt(hDlg, IDC_MOV_PERIOD_V, BlcMotionAvgPeriod, TRUE);
                    }
                    if ((y >= 235) && (y <= 255))       // Is Pixel Comb clip slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_PIXEL_COMB), x, 235+5, 10, 12, TRUE);
                        BlcPixelCombSense = GetHorSliderInt(x, 0, 100);
                        SetDlgItemInt(hDlg, IDC_PIXEL_COMB_V, BlcPixelCombSense, TRUE);
                    }
                    if ((y >= 285) && (y <= 305))       // Is Hist Comb slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_AVG_COMB), x, 285+5, 10, 12, TRUE);
                        BlcRecentCombSense = GetHorSliderInt(x, 0, 100);
                        SetDlgItemInt(hDlg, IDC_AVG_COMB_V, BlcRecentCombSense, TRUE);
                    }
                    if ((y >= 335) && (y <= 355))       // Is Hist Comb Period slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_COMB_PERIOD), x, 335+5, 10, 12, TRUE);
                        BlcCombAvgPeriod = GetHorSliderInt(x, 1, 200);
                        SetDlgItemInt(hDlg, IDC_COMB_PERIOD_V, BlcCombAvgPeriod, TRUE);
                    }
                    if ((y >= 385) && (y <= 405))       // Is Suppress High Comb slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_COMB_SKIP), x, 385+5, 10, 12, TRUE);
                        BlcHighCombSkip = GetHorSliderInt(x, 0, 100);
                        SetDlgItemInt(hDlg, IDC_COMB_SKIP_V, BlcHighCombSkip, TRUE);
                    }
                    if ((y >= 435) && (y <= 455))       // Is Suppress Low motion slider?
                    {
                        MoveWindow(GetDlgItem(hDlg, IDC_MOTION_SKIP), x, 435+5, 10, 12, TRUE);
                        BlcLowMotionSkip = GetHorSliderInt(x, 0, 100);
                        SetDlgItemInt(hDlg, IDC_MOTION_SKIP_V, BlcLowMotionSkip, TRUE);
                    }
                }
            }
            return TRUE;

    case WM_COMMAND:

        switch LOWORD(wParam)
        {
        case IDOK:                          // Is Done, use our new parms
            DestroyWindow(hDlg);
            return TRUE;
            break;

        case IDCANCEL:                      
            BlcMinimumClip = TMinimumClip;
            BlcPixelMotionSense = TPixelMotionSense;
            BlcRecentMotionSense = TRecentMotionSense;
            BlcMotionAvgPeriod = TMotionAvgPeriod;
            BlcPixelCombSense = TPixelCombSense;
            BlcRecentCombSense = TRecentCombSense;
            BlcCombAvgPeriod = TCombAvgPeriod;
            BlcHighCombSkip = THighCombSkip;
            BlcLowMotionSkip = TLowMotionSkip;
            BlcUseInterpBob = TUseInterpBob;
            BlcBlendChroma = TBlendChroma;
            DestroyWindow(hDlg);
            return TRUE;
            break;

        case IDC_BLEND_CHROMA:              // Blend chroma from mult pixels
            BlcBlendChroma = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_BLEND_CHROMA));
            return TRUE;
            break;  

        case IDC_USE_INTERP_BOB:                // Blend chroma from mult pixels
            BlcUseInterpBob = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_USE_INTERP_BOB));
            return TRUE;
            break;  


        case IDC_DEFAULT:
            BlcMinimumClip= -15;
            BlcPixelMotionSense = 17;
            BlcRecentMotionSense = 0;
            BlcMotionAvgPeriod = 20;        // currently 1..200
            BlcPixelCombSense = 27;
            BlcRecentCombSense = 0;
            BlcCombAvgPeriod = 20;          // currently 1.200
            BlcHighCombSkip = 10;           // larger values skip more
            BlcLowMotionSkip = 0;           // larger values skip more
            BlcVerticalSmoothing = 0;       // not currently used
            BlcUseInterpBob = FALSE;
            BlcBlendChroma = TRUE;          // default should maybe be TRUE?
            SetBlcDisplayControls(hDlg);
            return TRUE;
            break;
        
        default:
            break;

        }
        break;

    default:
        break;
    }
    return (FALSE);
}

void __cdecl BlendedClipShowUI(HWND hwndMain)
{
    if(ghDlg == NULL)
    {
        ghDlg = CreateDialog(hInst, "BLENDED_CLIP", hwndMain, BlendedClipProc);
    }
}

void __cdecl BlendedClipExit(void)
{
    if(ghDlg != NULL)
    {
        DestroyWindow(ghDlg);
    }
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING DI_BlendedClipSettings[DI_BLENDEDCLIP_SETTING_LASTONE] =
{
    {
        "BlcMinimumClip", SLIDER, 0, &BlcMinimumClip,
        -15, -100, 100, 1, 1,
        NULL,
        "Deinterlace", "BlcMinimumClip", NULL,
    },
    {
        "BlcPixelMotionSense", SLIDER, 0, &BlcPixelMotionSense,
        17, 0, 100, 1, 1,
        NULL,
        "Deinterlace", "BlcPixelMotionSense", NULL,
    },
    {
        "BlcMotionAvgPeriod", SLIDER, 0, &BlcMotionAvgPeriod,
        20, -100, 100, 1, 1,
        NULL,
        "Deinterlace", "BlcMotionAvgPeriod", NULL,
    },
    {
        "BlcRecentMotionSense", SLIDER, 0, &BlcRecentMotionSense,
        0, -100, 100, 1, 1,
        NULL,
        "Deinterlace", "BlcRecentMotionSense", NULL,
    },
    {
        "BlcPixelCombSense", SLIDER, 0, &BlcPixelCombSense,
        27, 0, 200, 1, 1,
        NULL,
        "Deinterlace", "BlcPixelCombSense", NULL,
    },
    {
        "BlcRecentCombSense", SLIDER, 0, &BlcRecentCombSense,
        0, 0, 200, 1, 1,
        NULL,
        "Deinterlace", "BlcRecentCombSense", NULL,
    },
    {
        "BlcCombAvgPeriod", SLIDER, 0, &BlcCombAvgPeriod,
        20, 0, 200, 1, 1,
        NULL,
        "Deinterlace", "BlcCombAvgPeriod", NULL,
    },
    {
        "BlcHighCombSkip", SLIDER, 0, &BlcHighCombSkip,
        10, 0, 200, 1, 1,
        NULL,
        "Deinterlace", "BlcHighCombSkip", NULL,
    },
    {
        "BlcLowMotionSkip", SLIDER, 0, &BlcLowMotionSkip,
        0, 0, 200, 1, 1,
        NULL,
        "Deinterlace", "BlcLowMotionSkip", NULL,
    },
    {
        "BlcVerticalSmoothing", SLIDER, 0, &BlcVerticalSmoothing,
        0, 0, 200, 1, 1,
        NULL,
        "Deinterlace", "BlcVerticalSmoothing", NULL,
    },
    {
        "BlcUseInterpBob", ONOFF, 0, &BlcUseInterpBob,
        FALSE, 0, 1, 1, 1,
        NULL,
        "Deinterlace", "BlcUseInterpBob", NULL,
    },
    {
        "BlcBlendChroma", ONOFF, 0, &BlcBlendChroma,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Deinterlace", "BlcBlendChroma", NULL,
    },
    {
        "BlcShowControls", ONOFF, 0, &BlcShowControls,
        TRUE, 0, 1, 1, 1,
        NULL,
        "Deinterlace", "BlcShowControls", NULL,
    },
};

DEINTERLACE_METHOD BlendedClipMethod =
{
    sizeof(DEINTERLACE_METHOD),
    DEINTERLACE_CURRENT_VERSION,
    "Blended Clip", 
    NULL,
    FALSE, 
    FALSE, 
    DeinterlaceBlendedClip, 
    50, 
    60,
    DI_BLENDEDCLIP_SETTING_LASTONE,
    DI_BlendedClipSettings,
    INDEX_BLENDED_CLIP,
    NULL,
    NULL,
    BlendedClipShowUI,
    BlendedClipExit,
    3,
    0,
    0,
    WM_DI_BLENDEDCLIP_GETVALUE - WM_APP,
    NULL,
    0,
    FALSE,
    FALSE,
    IDH_BLENDEDCLIP,
};


__declspec(dllexport) DEINTERLACE_METHOD* GetDeinterlacePluginInfo(long CpuFeatureFlags)
{
    return &BlendedClipMethod;
}

BOOL WINAPI DllMain(HANDLE hInstance, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    hInst = hInstance;
    return TRUE;
}
