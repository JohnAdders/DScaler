/////////////////////////////////////////////////////////////////////////////
// $Id: GreedyHMA.cpp,v 1.1 2001-10-02 17:46:38 trbarry Exp $
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
        return clip.GreedyHMA(1,0,1,0,0,0)

        It specifies the file spec (change yours) and asks for TopFirst and AutoPulldown to be
        turned on.  I've so far tested it only on VirtualDub. 

        I'm not sure, but there may be a bug in my code that requires the TopFirst option turned
        on. Maybe it's a quirk of my Matrox G400 MJPEG capture but I do get slightly better results
        with it on. I notice it's possible to run 2 copies of Vdub and do a frame by frame compare.
        There doesn't seem to be to much difference but a still compare of two frames (look at
        diagonals) will usually show one to be better. You can just look at the VirtualDub inputs
        for this. It's not necessary to actually create any output files yet.

        The 2nd script is similar to the first except with Median & Vertical filters on. It also
        has a SelectEvery() filter to drop one out of every 5 frames, to get 24 fps.

        GreedyHMA Parm list:

          return clip.GreedyHMA(TopFirst, SwapFields, AutoPullDown,
                                 MedianFilter, VerticalFilter, EdgeEnhance)

        All the values are integer, 0=no, 1=yes:

          TopFirst - assume the top field, lines 0,2,4,... should be displayed first. 
	        The default is the supposedly more common BottomFirst (not for me).

          SwapFields - for busted capture drivers that put lines 1,3,5.. over
	        lines 0,2,4...

          AutoPullDown - Does Auto Pulldown matching if needed to get rid of the weave 
	        artifacts but doesn't drop any frames. Use SelectEvery, Virualdub, or 
	        Decimate functions instead if desired.

            I haven't tried to see what's best with that yet. Pulldown stuff's still
            not perfect. But unless you're certain, leave this on just in case. It
            should still handle high motion video sports ok. Since I already have
	        the needed information while processing I may add a Decimate function
	        here in the future. But currently it's just pulldown matching, outputting
	        30 fps and not deleting any frames.

            A value of AutoPullDown = 2 may be supported in the future as soon as I figure
            out how to tell Avisynth how to drop frames.  This will then be full Adaptive
            Pulldown, decimating from 30 to 24 fps, dropping one of each 5 frames. But
            it's not yet implemented.

          MedianFilter (temporal) - Helps get rid of noise if needed but causes a 
	        2 field video delay. (Otherwise there's a 1 field delay) Specifying 1
            turns it on using the current default of 3 (may change) but you can also 
            specify 2-255 to override this. But values over about 3-5 can cause motion
            artifacts in fast motion video.

          VerticalFilter - Not only gets rid of noise but also some deinterlacing
	        artifacts since it's done last, after deinterlace. Useful if you are going
	        to down scale. Use only values 0,1 for parm.

          EdgeEnhance (horizontal) - Add a little sharpness to full res video. This 
  	        one can also use parm values of 2-100. The default is currently 50 if
	        1 is specified.  I may also support negative values in the future, making it a
	        horizontal filter to help set up for downscaling.

        For now, all 6 parms must be present, there is no error checking, and no more doc except
        about Greedy/HM at www.dscaler.org


        Known issues and limitation:

        1) 	THIS IS FOR SSE MACHINES ONLY!

        This is written like the advanced options of DScaler Greedy/HM, partly in assembler for
        SSE machines only.  I've so far got zero error checking in it, so it will just CRASH if
        you don't have at least a faster Celeron, P-III, Athlon, or higher. I'm not sure if this
        will be corrected in the future, but I might if time permits.

        2) 	Currently no error checking or messages. I haven't figured out how to issue messages
	        in Avisynth yet, and there is a bunch of stuff I should check that I'm not.
	        
        3) 	Assumes YUB (YUY2) Frame Based input. Use an AVIsynth function to convert first if
	        needed.
	        
        4)	Currently limited to a frame resolution of 999x576 pixels. FIXED ...
            (Now adjusted to a limit of 1928x1081 when building for Avisynth, to support HDTV)

        5)	If you have pure well behaved film source, with no edits or pulldown quirks, and no 
	        video source, then you are probably better off in both performance and quality using
	        the other functions of VirtualDub for Inverse Telecine, and not bothering with GreedyHMA.
	        GreedyHMA is designed for high motion sports or mixed up video and film
	        sequences. It may also do well on anime.
	        
        6)	There is a strange bug that I don't understand at all. If you add a new or changed
	        GreedyHMA.dll to your machine, the first time you run VirtualDub/Avisynth/GreedyHMA
	        then VirtualDub may stop responding. If you then use the Task Manager to cancel 
	        VirtualDub it will run after that.  A work-around seems to be to first open any
	        other AVI file in VirtualDub the first time you bring it up after installing GreedyHMA,
            before opening any Avisynth scripts.
	        

        Please send comments to Tom Barry (trbarry@trbarry.com)

        Temporary file locations:

        For now, see www.trbarry.com/GreedyHMA.zip

        The source is, or will soon be, temporarily at www.trbarry.com/GreedyHMAsrc.zip but it 
        may be eventually included in the DScaler project if the other team members think it'
        a good idea.


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
extern "C" void InitDScaler(int AutoPulldown, int MedianFilter, int VerticalFilter, int EdgeEnhance);

extern  "C" void SetDScalerFrameInfo(BOOL IsOdd, int InPitch, int OutPitch, 
     const BYTE* pSource, BYTE* pOverlay, int FrHeight, int LLength);
extern  "C" BOOL CallGreedyHM(BOOL IsOdd, int InPitch, int OutPitch, 
     const BYTE* pSource, BYTE* pOverlay, int FrHeight, int LLength);
extern "C" BOOL UpdateFieldStore();
//extern "C" BOOL DI_GreedyHM();

//void *memcpyw( void *dest, const void *src, size_t count );


class GreedyHMA : public GenericVideoFilter {
    BOOL TopFirst;
    BOOL SwapFields;
public:
//    GreedyHMA(int _EvenFirst, PClip _child ) : GenericVideoFilter(_child)
    GreedyHMA(PClip _child, BOOL _TopFirst, BOOL _SwapFields) : GenericVideoFilter(_child)
    {
        TopFirst = _TopFirst;
        SwapFields = _SwapFields;
    }
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
};


PVideoFrame __stdcall GreedyHMA::GetFrame(int n, IScriptEnvironment* env) 
{

    PVideoFrame src = child->GetFrame(n, env);
    PVideoFrame dst = env->NewVideoFrame(vi);
        const unsigned char* srcp = src->GetReadPtr();

    unsigned char* dstp = dst->GetWritePtr();
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
                srcp, dstp, height, row_size);
        }
        else
        {
            // First do Top lines (0,2,4..) take from Top
            SetDScalerFrameInfo(false, src_pitch, dst_pitch,
                srcp, dstp, height, row_size);  
            UpdateFieldStore();
            // Then do Bottom lines (1,3,5..), take from bottom
            retc = CallGreedyHM(true, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size);
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
            retc = CallGreedyHM(true, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size);
        }
        else
        {                   // neither Top First or Swap fields
            // First do Bottom lines (1,3,5..), take from bottom
            SetDScalerFrameInfo(true, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size);  
            UpdateFieldStore();
            // Then do Even (top) lines, assuming they come second (unless TopFirst)
            retc = CallGreedyHM(false, src_pitch, dst_pitch,
                srcp + src_pitch, dstp, height, row_size);
        }


    return dst;
}


AVSValue __cdecl Create_GreedyHMA(AVSValue args, void* user_data,
IScriptEnvironment* env) 
{
    int i;
    i = args[3].AsInt();
    i = args[4].AsInt();
    i = args[5].AsInt();
    i = args[6].AsInt();

    InitDScaler(args[3].AsInt(), args[4].AsInt(), args[5].AsInt(), args[6].AsInt()); 
    return new GreedyHMA(args[0].AsClip(), args[1].AsInt(), args[2].AsInt());
//    return new GreedyHMA(args[0].AsClip());
}


extern "C" __declspec(dllexport) const char* __stdcall
AvisynthPluginInit(IScriptEnvironment* env) {
    env->AddFunction("GreedyHMA", "ciiiiii", Create_GreedyHMA, 0);
    return "`GreedyHMA' DScaler DeInterlace";
}



