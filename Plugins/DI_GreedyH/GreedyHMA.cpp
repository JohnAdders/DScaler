/////////////////////////////////////////////////////////////////////////////
// $Id$
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
/*

From the Readme_GreedyHMA.txt file:

        GreedyHMA - Greedy (High Motion for Avisynth)


        GreedyHMA.dll is an Avisynth filter that executes DScaler's Greedy/HM algorithm code to
        perform pulldown matching, filtering, and video deinterlace.

        Just unzip the contents into your Avisynth directory, or somewhere. As the script shows,
        I made a subdirectory under Avisynth just to keep it separate.

        Following is Bikes101000.avs, one of the scripts I was testing:

        LoadPlugin("d:\AVISynth\GreedyHMA\Debug\GreedyHMA.dll")
        clip = AVISource("c:\vcr\bikes.avi")
        return clip.GreedyHMA(1,0,4,0,0,0,0,0)

        It specifies the file spec (change yours) and asks for TopFirst and AutoPulldown to be
        turned on, with decimation (frame dropping) to 24 fps.  I've so far tested it only 
        with Avisynth/VirtualDub. 

        I'm not sure, but there may be a bug in my code that requires the TopFirst option turned
        on. Maybe it's a quirk of my Matrox G400 MJPEG capture but I do get slightly better results
        with it on. I notice it's possible to run 2 copies of Vdub and do a frame by frame compare.
        There doesn't seem to be to much difference but a still compare of two frames (look at
        diagonals) will usually show one to be better. You can just look at the VirtualDub inputs
        for this. It's not necessary to actually create any output files yet.

        GreedyHMA Parm list:

          return clip.GreedyHMA(TopFirst, SwapFields, AutoPullDown,
                                 MedianFilter, VerticalFilter, EdgeEnhance
                                 GoodPullDownLvl, BadPullDownLvl)

        All the values are integer, 0=no, 1=yes:
        
          TopFirst - assume the top field, lines 0,2,4,... should be displayed first. 
	        The default is the supposedly more common BottomFirst (not for me).

          SwapFields - for busted capture drivers that put lines 1,3,5.. over
	        lines 0,2,4...

          AutoPullDown - Sets what types of Pulldown processing is desired.
            Valid parm values are:

            0 - No pulldown, just do pure deinterlace (Force Video). Doesn't drop
                any frames so FPS is left at 30 FPS, or wherever. Doesn't decimate
                (pentimate?). Use if you have (and want) 30 fps video source or
                maybe 25 FPS PAL video.

            1 - Auto Pulldown. Automatically decide which frames should be IVTC'd 
                or deinterlaced, depending upon the settings of the Good and Bad
                Pulldown Lvls (see below). Don't decimate. This is best if you
                have mixed film and video and wan't 30 fps output. Also better
                for PAL sources.

            2 - Pulldown only (Force Film). Assume film source, always do IVTC and
                never deinterlace. Don't decimate. Use for PAL film source.

            Values 3-5 match those above but with frame dropping (Decimation, pentimation?)
            Five frames at a time are looked at and the one most likely to be a duplicate
            is dropped, thus lowering the FPS from, say, 30 to 24.

            3 - Deinterlace (Force Video) but with frame dropping. Use for NTSC video
                if you still want to decimate. With video source there really are no
                proper fields to drop, but this will drop the ones looking most like
                dupes.

            4 - Auto Pulldown with frame dropping. The best (most automatic) setting.
                Works in most cases, at least if I get all the bugs out.

            5 - Pulldown only (Force Film) with frame dropping. This will give the best
                results if you have 100% properly mastered NTSC film source with no
                video sections and not too many edits. It can adjust for most scene 
                changes and changes in pulldown cadence, but not mixed up fields.

            I haven't tried to see what's best with all this yet. Pulldown stuff's still
            not perfect. But unless you're certain, leave this on 4 just in case. It
            should still handle high motion video sports ok.



   ***** NOTE!  THE MEDIAN FILTER IS DISABLE FOR THIS RELEASE. *****
            
          MedianFilter (temporal) - Helps get rid of noise if needed but causes a 
	        2 field video delay. (Otherwise there's a 1 field delay) Specifying 1
            turns it on using the current default of 3 (may change) but you can also 
            specify 2-255 to override this. But values over about 3-5 can cause motion
            artifacts in fast motion video.

            See note above. I broke the Median Filter adding frame dropping since it runs
            at a delay of 2 instead of everything else. It seemed less important so
            I just disable it for now. The parm is currently ignored.
    ***** NOTE *****


          VerticalFilter - Not only gets rid of noise but also some deinterlacing
	        artifacts since it's done last, after deinterlace. Useful if you are going
	        to down scale. Use only values 0,1 for parm.

          EdgeEnhance (horizontal) - Add a little sharpness to full res video. This 
  	        one can also use parm values of 2-100. The default is currently 50 if
	        1 is specified.  I may also support negative values in the future, making it a
	        horizontal filter to help set up for downscaling.

          Good Pulldown Lvl - This is a number from 1-255 that must be exceeded before
            GreedyHMA will do pulldown for a frame. This parm is honored only when
            Auto Pulldown is also turned on. The default is subject to change but is
            currently set at 90. But you will get the default value just by specifying
            0, which is recommended for most cases. It is really just a few frame average
            of the savings in comb/weave factors by properly matching frames.

          Bad Pulldown Lvl - This is a number from 1-255 that must NOT be exceeded before
            GreedyHMA will do pulldown for a frame. This parm is honored only when
            Auto Pulldown is also turned on. The default is subject to change but is
            currently set at 85. But you will get the default value just by specifying
            0, which is recommended for most cases. This is really just a measure of
            how bad a frame's comb/weave factors would be increased if we did pure 
            field matching.

          
        For now, all 8 parms must be present, there is no error checking, and no more doc except
        about Greedy/HM at www.dscaler.org


        Known issues and limitation:

        1) 	THIS IS FOR SSE MACHINES ONLY!

        This is written like the advanced options of DScaler Greedy/HM, partly in assembler for
        SSE machines only.  I've so far got zero error checking in it, so it will just CRASH if
        you don't have at least a faster Celeron, P-III, Athlon, or higher. I'm not sure if this
        will be corrected in the future, but I might if time permits.

        2) 	Currently no error checking or messages. I haven't figured out how to issue messages
	        in Avisynth yet, and there is a bunch of stuff I should check that I'm not.
	        
        3) 	Assumes YUV (YUY2) Frame Based input. Use an AVIsynth function to convert first if
	        needed.
	        
        4)	Currently limited to a frame resolution of 999x576 pixels. FIXED ...
            (Now adjusted to a limit of 1928x1081 when building for Avisynth, to support HDTV)

        5)	If you have pure well behaved film source, with no edits or pulldown quirks, and no 
	        video source, then you are still maybe better off in both performance and quality using
	        the other functions of VirtualDub for Inverse Telecine, and not bothering with GreedyHMA.
	        GreedyHMA is designed for high motion sports or mixed up video and film
	        sequences. It may also do well on anime.
	        
        6)	There is a strange bug that I don't understand at all. If you add a new or changed
	        GreedyHMA.dll to your machine, the first time you run VirtualDub/Avisynth/GreedyHMA
	        then VirtualDub may stop responding. If you then use the Task Manager to cancel 
	        VirtualDub it will run after that.  A work-around seems to be to first open any
	        other AVI file in VirtualDub the first time you bring it up after installing GreedyHMA,
            before opening any Avisynth scripts.  Later notes: it doesn't depend upon GreedyHMA at
            all, it happens without it on my machine, but I still don't understand it.
	        
        7)  Currently works properly only if fed a screen width in pixels that is a multiple of 16.
            Otherwise it may produce garbage at the right of the screen that you may have to crop.

        8)  As stated above, the Median Filter option is temporarily ignored.


        Please send comments to Tom Barry (trbarry@trbarry.com)

        Temporary file locations:

        For now, see www.trbarry.com/GreedyHMA.zip

        The source is, or will soon be, in the DScaler open source project. www.dscaler.org 

    
      *********************************************************************************************
        Finally, the following is a slightly abridged copy of the Greedy (High Motion) summary from
        the DScaler Help page:

        GREEDY (HIGH MOTION) DEINTERLACE METHOD

        WHAT IT DOES: (Basic) 

        The plugin works at two different levels. By default, at the Basic Level, it is a normal
        DScaler deinterlace method, with the following features:

        - A merge of Blended Clip, Greedy (Low Motion), and some new stuff

        - Greedy implied pulldown, like the Greedy (Low Motion) method

        - Parm blending like the Blended Clip method, for less sizzling.

        - Lookahead motion detection

        - Diagonal Jaggie Reduction (DJR ;-) ) to smooth out the stair step effect in moving
        diagonal lines. This is accomplish by horizontally smoothing only the rapidly moving 
        pixels, leaving the stationary ones alone. If a diagonal edge is not moving then it
        won't be jagged anyway under Greedy (High Motion). I've previously somewhere here
        posted a couple snaps of some hard to handle Video Essentials scenes off my test
        machine and a $135 non-progressive DVD player. See the Flag and the Lampposts.

        - Slider adjustments for Max Comb, Motion Threshold, and Motion Sensitivity. However the
        defaults should be okay for most cases.



        WHAT IT DOES: (Advanced)

        There is also an Advanced Level (experimental) which is invoked if you check any of the
        boxes in the Method UI panel. You won't be able to do this unless you are running a
        machine supporting the Intel SSE instruction set (only integer SSE instructions are
        needed). The Advanced options should run on Athlons, Intel P-III, P4, and the faster
        Celerons.

        When using the Advanced Features Greedy (HM) maintains its own internal history of the last
        4 video input fields. This means it must be able to examine every field and you can't use
        it at the same time as DScaler's regular Auto_Pulldown option. 

        The new check box enabled features are:

        - Built in 3:2, 2:2, an n:m Pulldown. This does pulldown matching, not removal, so all
        frames are displayed. I think this eventually will work better with video overlays and
        live TV viewing that is extensively edited, but it still needs improvement.

        - In-Between Frames. This gives judder reduction for those of us with fixed 60 HZ
        RTPV's. In a 3:2 pulldown situation it will select an appropriate pair of fields to
        average once out of every 5 frames. This can effectively display every new input frame
        2 1/2 times. It will cause a small amount of motion blur every fifth frame but I
        personally leave it turned on all the time now for a smoother film picture sequence.
        It will also work with 2:2 pulldown (I think) but there one half of the frames will be
        averaged and the motion blur may not be acceptable.

        - Adjustable Edge Enhancement. This is due to an idea posted by this forum's MR. Wigggles.
        It enhances only a small amount, on the horizontal axis. When turned on you can use a
        slider to adjust it.

        - Adjustable Median Filter. Also due to Mr. Wigggles, this can help filter noise. However
        if set over about 3-5 it will also cause motion artifacts on fast moving scenes. Also has
        a slider.

        - Vertical Filter. This is a post-deinterlace software vertical filter that can remove
        both noise and some leftover deinterlace artifacts. It gives results similar to the
        hardware version available on the Advanced Video Flags screen, but not so extreme and
        without the jitter.
*/



#include "avisynth.h"
#include "DI_GreedyHM.h"
extern "C" void InitDScaler(int AutoPulldown, int MedianFilter, int VerticalFilter, 
                            int EdgeEnhance, int GoodPullDownLvl, int BadPullDownLvl);

extern  "C" void SetDScalerFrameInfo(BOOL IsOdd, int InPitch, int OutPitch, 
     const BYTE* pSource, BYTE* pOverlay, int FrHeight, int LLength);
extern  "C" BOOL CallGreedyHM(BOOL IsOdd, int InPitch, int OutPitch, 
     const BYTE* pSource, BYTE* pOverlay, int FrHeight, int LLength, BOOL WriteFrame);
extern "C" BOOL UpdateFieldStore();


class GreedyHMA : public GenericVideoFilter {
    BOOL TopFirst;
    BOOL SwapFields;
    int  Max_Frames;
    int  PulldownType;
    int  PrevFrame;
    int  PrevInFrame;
    int  dropFrame;

public:
    GreedyHMA(PClip _child, BOOL _TopFirst, BOOL _SwapFields, int _PulldownType) : GenericVideoFilter(_child)
    {
        TopFirst = _TopFirst;
        SwapFields = _SwapFields;
        PulldownType = _PulldownType;
        Max_Frames = vi.num_frames;
        PrevFrame = -2;                 // last frame processed, if any
        PrevInFrame = -2;               // last frame requested, if any
        dropFrame = 1;                  // next frame to drop

        if (PulldownType > 2)           // if we want to decimate to 24 fps, drop one of each 5
        {
            vi.num_frames = vi.num_frames * 4 / 5;
            vi.SetFPS(vi.fps_numerator * 4, vi.fps_denominator * 5);
        }
    }

    PVideoFrame __stdcall GetFrame(int inFrame, IScriptEnvironment* env);
    int __stdcall ProcessFrame(int _useFrame, PVideoFrame dst, unsigned char* dstp, BOOL WriteFrame, IScriptEnvironment* env);
    int __stdcall WhatDoWeSkip(int useFrame, IScriptEnvironment* env);
};
PVideoFrame __stdcall GreedyHMA::GetFrame(int inFrame, IScriptEnvironment* env) 
{
int i;
int useFrame = PulldownType > 2
                ? inFrame * 5 / 4           // cycle 0,1,2,3,5,6,7,8,10
                : inFrame;

    PVideoFrame dst = env->NewVideoFrame(vi);
    unsigned char* dstp = dst->GetWritePtr();

    if (PulldownType > 2)
    {
        if (!(useFrame % 5))                            // at start of a group of 5
        {
            dropFrame = WhatDoWeSkip(useFrame, env);     // go find what to drop in this group of 5
        }
        else if (inFrame-1 != PrevInFrame) // Virtualdub random access?
        {
            dropFrame = WhatDoWeSkip((useFrame/5)*5, env); // pretend it wasn't
        }
            
        if  (useFrame >= dropFrame)
        {
            if  (useFrame == dropFrame)
            {
                i = ProcessFrame(useFrame, dst, dstp, false, env);// table, don't write buffer
                PrevFrame = useFrame;
            }    
            useFrame++;
        }
    }

    if (useFrame-1 != PrevFrame)            // out of sequence, reinit
    {
        i = ProcessFrame(useFrame-1, dst, dstp, false, env);    // but just table don't write buffer
    }    
        
    i = ProcessFrame(useFrame, dst, dstp, true, env);       // table and then write the frame
    PrevFrame = useFrame;
    PrevInFrame = inFrame;
    return dst;
}

int __stdcall GreedyHMA::ProcessFrame(int _useFrame, PVideoFrame dst, unsigned char* dstp, 
                                      BOOL WriteFrame, IScriptEnvironment* env) 
{
    int useFrame = _useFrame < 0    // we always want a prev frame so maybe
                    ? 0             // use frame 0 twice at beginning, better
                    : _useFrame;

    if (useFrame > Max_Frames-1)
    {
        return 0;
    }
    PVideoFrame src = child->GetFrame(useFrame, env);
    const unsigned char* srcp = src->GetReadPtr();
    BOOL retc = 0;
    const int src_pitch = src->GetPitch();
    const int dst_pitch = dst->GetPitch();
    const int row_size = dst->GetRowSize();
    const int height = dst->GetHeight();

    if (TopFirst)
    {
        if (SwapFields)
        {                               
            // First do Top lines (0,2,4..) but take from Bottom (1,3,5..)
            SetDScalerFrameInfo(false, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size);  
            UpdateFieldStore();
            // Then do Bottom (1,3,5) lines, but take from Top (0,2,4..)
            retc = CallGreedyHM(true, src_pitch, dst_pitch,
                srcp, dstp, height, row_size, WriteFrame);
        }
        else
        {
            // First do Top lines (0,2,4..) take from Top
            SetDScalerFrameInfo(false, src_pitch, dst_pitch,
                srcp, dstp, height, row_size);  
            UpdateFieldStore();
            // Then do Bottom lines (1,3,5..), take from bottom
            retc = CallGreedyHM(true, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size, WriteFrame);
        }
    }
    else                    // not Top First
        if (SwapFields)
        {
            // First do Bottom lines (1,3,5..), but take from Top (0,2,4..)
            SetDScalerFrameInfo(true, src_pitch, dst_pitch,
                srcp, dstp, height, row_size);  
            UpdateFieldStore();
            // Then do Top lines (0,2,4..), take from Bottom (1,3,5..)
            retc = CallGreedyHM(false, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size, WriteFrame);
        }
        else
        {                   // neither Top First or Swap fields
            // First do Bottom lines (1,3,5..), take from bottom
            SetDScalerFrameInfo(true, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size);  
            UpdateFieldStore();
            // Then do Even (top) lines, assuming they come second (unless TopFirst)
            retc = CallGreedyHM(false, src_pitch, dst_pitch,
                srcp, dstp, height, row_size, WriteFrame);
        }


    return 0;
}

// This rtn figures out which frame is most likely to be a duplicate. That is,
// which one most resembles its preceding frame. Only the dominant field is used
// since at this point we don't know if or how the alternate field will be deinterlaced.
// This assumes that by the time a frame is displayed the dominant field will be
// delayed 1, not the current field. I havn't yet figured out the ramifications of
// median filter, which actually forces a 2 field delay (so I'm turning it off for now).
// This looks ahead at 5 fields at a time, and 1 back.
int __stdcall GreedyHMA::WhatDoWeSkip(int useFrame, IScriptEnvironment* env)
{
int i;
int j;
int skiprec = useFrame+4;       // default skip last of bad block
int skipval = 0x7fffffff;
int skipvalw = 0;

PVideoFrame src1;
const unsigned char* srcp1;
int src_pitch1;

PVideoFrame src2;
const unsigned char* srcp2;
int src_pitch2;

int row_size;
int height;
    
    for (i=useFrame; i <= useFrame+4; i++)
    {
        skipvalw = 0;
        if (i > 0)
        {
            if (i < Max_Frames)
            {
                src1 = child->GetFrame(i-1, env);
                srcp1 = src1->GetReadPtr();
                src_pitch1 = src1->GetPitch();
                row_size = src1->GetRowSize() / 32;     // will process in 32 byte chunks
                height = src1->GetHeight();

                src2 = child->GetFrame(i, env);
                srcp2 = src2->GetReadPtr();
                src_pitch2 = src2->GetPitch();

            // remember to adjust for Median filter here (somehow?)
                if ((TopFirst && SwapFields) || (!TopFirst && !SwapFields))
                {
                    srcp1 += src_pitch1;    // point at 2nd line
                    srcp2 += src_pitch2;    // "
                }
            
                src_pitch1 += src_pitch1;               // will do every other line
                src_pitch2 += src_pitch2;               // will do every other line
            
                for (j=1; j <= height/2; j++)
                {
		            _asm		
		            {
                    mov     esi, srcp1          // ptr to prev frame data
                    mov     edi, srcp2          // ptr to curr frame data
                    mov     edx, row_size       // count of 32 byte chunks
                    movd    mm7, skipvalw       // will accum total differences

		            align 8
            CompLoop:   
                    movq    mm0, qword ptr[esi]
                    movq    mm1, qword ptr[esi+8]
                    movq    mm2, qword ptr[esi+16]
                    movq    mm3, qword ptr[esi+64]
    	            psadbw  mm0, qword ptr[edi]		// sum of abs differences 
    	            psadbw  mm1, qword ptr[edi+8]	
    	            psadbw  mm2, qword ptr[edi+16]	
    	            psadbw  mm3, qword ptr[edi+24]	
                    paddd   mm7, mm0                // accum totals
                    paddd   mm7, mm1
                    paddd   mm7, mm2
                    paddd   mm7, mm3

                    lea     esi, [esi+32]           // bump ptrs & loop
                    lea     edi, [edi+32]
                    dec    edx 
                    jnz     CompLoop
                    movd    skipvalw, mm7
                    emms
                    }                               // done with one line

                    srcp1 += src_pitch1;            // will do every other line
                    srcp2 += src_pitch2;            // will do every other line
                }
            }
            if (skipvalw < skipval)
            {
                skipval = skipvalw;
                skiprec = i;
            }
        }
    }
    return skiprec;
}




AVSValue __cdecl Create_GreedyHMA(AVSValue args, void* user_data,
IScriptEnvironment* env) 
{
    int i;
    i = args[3].AsInt();
    i = args[4].AsInt();
    i = args[5].AsInt();
    i = args[6].AsInt();

    InitDScaler(args[3].AsInt(), args[4].AsInt(), args[5].AsInt(), 
        args[6].AsInt(), args[7].AsInt(), args[8].AsInt()); 
    return new GreedyHMA(args[0].AsClip(), args[1].AsInt(), args[2].AsInt(), args[3].AsInt());
}


extern "C" __declspec(dllexport) const char* __stdcall
AvisynthPluginInit(IScriptEnvironment* env) {
    env->AddFunction("GreedyHMA", "ciiiiiiii", Create_GreedyHMA, 0);
    return "`GreedyHMA' DScaler DeInterlace";
}



