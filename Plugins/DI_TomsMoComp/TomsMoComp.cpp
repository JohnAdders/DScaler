//#define IS_SSE2
// Copyright (c) 2002 Tom Barry.  All rights reserved.
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
//  
//  Also, this program is "Philanthropy-Ware".  That is, if you like it and 
//  feel the need to reward or inspire the author then please feel free (but
//  not obligated) to consider joining or donating to the Electronic Frontier
//  Foundation. This will help keep cyber space free of barbed wire and bullsh*t.  
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 15 Apr 2002   Tom Barry		       Create TomsMoComp Deinterlacer V 0.0.1.0
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
//
/////////////////////////////////////////////////////////////////////////////
/*

From the Readme_TomsMoComp.txt file:

        TomsMoComp - Motion Compensating Deinterlace


        TomsMoComp.dll is an Avisynth filter that uses motion compensation and adaptive
		processing to deinterlace video source.

        Just unzip the contents into your Avisynth directory, or somewhere. As the script shows,
        I made a subdirectory under Avisynth just to keep it separate.
		avs, one of the scripts I was testing:

        LoadPlugin("d:\AVISynth\TomsMoComp\Debug\TomsMoComp.dll")
        clip = AVISource("c:\vcr\bikes.avi")
        return clip.TomsMoComp(...)

        It specifies the file spec (change yours) and asks for TopFirst and SearchEffort=10 
        to be turned on. I've so far tested it only 
        with Avisynth/VirtualDub. 


        TomsMoComp Parm list:

          return clip.TomsMoComp(TopFirst, SearchEffort, Vertical_Filter)

        All the values are integer, 0=no, 1=yes:
        
          TopFirst - assume the top field, lines 0,2,4,... should be displayed first. 
	        The default is the supposedly more common BottomFirst (not for me).

		SearchEffort - determines how much effort (cpu time) will be used to find 
		    moved pixels. Currently numbers from 0-45 with 0 being practially
			just a smarter bob.

        Known issues and limitation:
        1) 	Assumes YUV (YUY2) Frame Based input. Use an AVIsynth function to convert first if
	        needed.        


        Temporary file locations:

        For now, see www.trbarry.com/TomsMoComp.zip

		THIS PROGRAM IS STILL NEW AND EXPERIMENTAL!

  Please send comments to Tom Barry (trbarry@trbarry.com)


*/
#include "avisynth.h"

#include "TomsMoComp.h"

extern  "C" BOOL AvisynthTomsMoComp_SSE(int SearchEffort, int _src_pitch, int _dst_pitch,
		int _rowsize, const BYTE* _pWeaveSrc, const BYTE* _pWeaveSrcP,
		BYTE* _pWeaveDest, BOOL _TopFirst, const BYTE* _pCopySrc,
		const BYTE* _pCopySrcP, int _FldHeight);

extern  "C" BOOL AvisynthTomsMoComp_SSE_VF(int SearchEffort, int _src_pitch, int _dst_pitch,
		int _rowsize, const BYTE* _pWeaveSrc, const BYTE* _pWeaveSrcP,
		BYTE* _pWeaveDest, BOOL _TopFirst, const BYTE* _pCopySrc,
		const BYTE* _pCopySrcP, int _FldHeight);

extern  "C" BOOL AvisynthTomsMoComp_3DNOW(int SearchEffort, int _src_pitch, int _dst_pitch,
		int _rowsize, const BYTE* _pWeaveSrc, const BYTE* _pWeaveSrcP,
		BYTE* _pWeaveDest, BOOL _TopFirst, const BYTE* _pCopySrc,
		const BYTE* _pCopySrcP, int _FldHeight);

extern  "C" BOOL AvisynthTomsMoComp_3DNOW_VF(int SearchEffort, int _src_pitch, int _dst_pitch,
		int _rowsize, const BYTE* _pWeaveSrc, const BYTE* _pWeaveSrcP,
		BYTE* _pWeaveDest, BOOL _TopFirst, const BYTE* _pCopySrc,
		const BYTE* _pCopySrcP, int _FldHeight);

extern  "C" BOOL AvisynthTomsMoComp_MMX(int SearchEffort, int _src_pitch, int _dst_pitch,
		int _rowsize, const BYTE* _pWeaveSrc, const BYTE* _pWeaveSrcP,
		BYTE* _pWeaveDest, BOOL _TopFirst, const BYTE* _pCopySrc,
		const BYTE* _pCopySrcP, int _FldHeight);

extern  "C" BOOL AvisynthTomsMoComp_MMX_VF(int SearchEffort, int _src_pitch, int _dst_pitch,
		int _rowsize, const BYTE* _pWeaveSrc, const BYTE* _pWeaveSrcP,
		BYTE* _pWeaveDest, BOOL _TopFirst, const BYTE* _pCopySrc,
		const BYTE* _pCopySrcP, int _FldHeight);



class TomsMoComp : public GenericVideoFilter {
    BOOL TopFirst;
    BOOL Use_Vertical_Filter;
    int  Max_Frames;
    int  SearchEffort;
    int  PrevFrame;
    int  PrevInFrame;
    int  dropFrame;
	int  useFrame;

    PVideoFrame src;
    PVideoFrame dst;
    PVideoFrame prevSrc;
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
	BYTE* oldBuff;
	BYTE* newBuff;
	bool SSE2enabled;
	bool SSEMMXenabled;
	bool _3DNOWenabled;
	int __stdcall Fieldcopy(void *dest, const void *src, size_t count, 
				int rows, int dst_pitch, int src_pitch);
	int __stdcall DoBorders(void *dest, const void *src, size_t count, 
				int rows, int dst_pitch, int src_pitch);

public:
    TomsMoComp(PClip _child, BOOL _TopFirst, int _Search_Effort, int _Use_Vertical_Filter,
		IScriptEnvironment* env) : GenericVideoFilter(_child)
    {
        TopFirst = _TopFirst;
        Use_Vertical_Filter = _Use_Vertical_Filter;
        SearchEffort = _Search_Effort;
        Max_Frames = vi.num_frames;
        PrevFrame = -2;                 // last frame processed, if any
        PrevInFrame = -2;               // last frame requested, if any
        dropFrame = 1;                  // next frame to drop
		SSE2enabled = (env->GetCPUFlags() & CPUF_SSE2) !=0;
		SSEMMXenabled = (env->GetCPUFlags() & CPUF_INTEGER_SSE) !=0;
		_3DNOWenabled = (env->GetCPUFlags() & CPUF_3DNOW) !=0;

	    if (!vi.IsYUY2())
		{
			env->ThrowError("TomsMoComp: Supports YUY2 color format only");
		}
	    if (vi.width % 4)
		{
			env->ThrowError("TomsMoComp: Horizontal pixels must divide by 4");
		}

		if (SearchEffort == -1)
		{
			vi.height = 2 * vi.height;
			TopFirst = 0;
		}		
    }

    PVideoFrame __stdcall GetFrame(int inFrame, IScriptEnvironment* env);
    int __stdcall ProcessFrame(PVideoFrame dst, unsigned char* dstp, BOOL WriteFrame, IScriptEnvironment* env);

};


PVideoFrame __stdcall TomsMoComp::GetFrame(int inFrame, IScriptEnvironment* env) 
{
int i;
	useFrame = inFrame;

    src = child->GetFrame(useFrame, env);
    dst = env->NewVideoFrame(vi);
    dstp = dst->GetWritePtr();
    src_pitch = src->GetPitch();
    dst_pitch = dst->GetPitch();
    rowsize = dst->GetRowSize();
	RowPixels = rowsize >> 1;
    height = dst->GetHeight();
	FldHeight = height / 2;
       
    i = ProcessFrame(dst, dstp, true, env);       // table and then write the frame
    PrevFrame = useFrame;
    PrevInFrame = inFrame;
	
	__asm emms;

    return dst;
}

int __stdcall TomsMoComp::ProcessFrame(PVideoFrame dst, unsigned char* dstp, 
                                      BOOL WriteFrame, IScriptEnvironment* env) 
{
    const unsigned char* srcp = src->GetReadPtr();
	int SearchEffortW = 0;

    if (useFrame > Max_Frames-1 || useFrame < 0)
    {
        return 0;
    }
	
	if (useFrame)
	{
		SearchEffortW = SearchEffort;
		prevSrc = child->GetFrame(useFrame-1, env);
		pWeaveSrcP = TopFirst
			? prevSrc->GetReadPtr() + src_pitch
			: prevSrc->GetReadPtr();
		pCopySrcP = TopFirst
			? prevSrc->GetReadPtr() + src_pitch
			: prevSrc->GetReadPtr();
	}

    BOOL retc = 0;
    if (SearchEffort == -1)				// note - DBL_Resize carried ast TopFirst == 0 hrtrtr
    {
		SearchEffortW = SearchEffort;
		pWeaveSrc = srcp;
		pCopySrc = srcp;
		pWeaveDest = dstp;
		pCopyDest = dstp;
		Fieldcopy(pWeaveDest+dst_pitch, pCopySrc, rowsize,				// copy top ODD line
					1, dst_pitch, src_pitch);
		Fieldcopy(pWeaveDest + (height-1)*dst_pitch,			// copy bottom ODD line	
			pCopySrc+(FldHeight-1)*src_pitch, rowsize, 
					1, dst_pitch, src_pitch);
		if (Use_Vertical_Filter)
		{
			Fieldcopy(pWeaveDest, pCopySrc, rowsize,				// copy top EVEN line
						1, dst_pitch, src_pitch);
			Fieldcopy(pWeaveDest + (height-2)*dst_pitch,			// copy bottom EVEN line	
				pCopySrc+(FldHeight-1)*src_pitch, rowsize, 
						1, dst_pitch, src_pitch);
		}
		else
		{
			Fieldcopy(pCopyDest, pCopySrc, rowsize,			// copy all EVEN lines (base 0)
					FldHeight, dst_pitch*2, src_pitch);		// I don't remember why???
		}
	}
    else if (TopFirst)
    {
		pWeaveSrc = srcp+src_pitch;
		pCopySrc = srcp;
		pWeaveDest = dstp+dst_pitch;
		pCopyDest = dstp;
		Fieldcopy(pWeaveDest, pCopySrc, rowsize,				// bob this later?
					1, dst_pitch*2, src_pitch*2);
		Fieldcopy(pWeaveDest+(FldHeight-1)*dst_pitch*2,
			pCopySrc+(FldHeight-1)*src_pitch*2, rowsize, 
					1, dst_pitch*2, src_pitch*2);
		if (Use_Vertical_Filter)
		{
			Fieldcopy(pCopyDest, pCopySrc, rowsize, 
						1, dst_pitch*2, src_pitch*2);
			Fieldcopy(pCopyDest+(FldHeight-1)*dst_pitch*2,		// copy last bob line
						pCopySrc+(FldHeight-1)*src_pitch*2, rowsize, 
						1, dst_pitch*2, src_pitch*2);
		}
		else
		{
			Fieldcopy(pCopyDest, pCopySrc, rowsize, 
						FldHeight, dst_pitch*2, src_pitch*2);
		}
	}
    else             // not Top First
	{
		pWeaveSrc = srcp;
		pCopySrc = srcp + src_pitch;
		pWeaveDest = dstp;
		pCopyDest = dstp + dst_pitch;
		Fieldcopy(pWeaveDest, pCopySrc, rowsize, 
					1, dst_pitch*2, src_pitch*2);
		Fieldcopy(pWeaveDest+(FldHeight-1)*dst_pitch*2,			// bob this later
			pCopySrc+(FldHeight-1)*src_pitch*2, rowsize, 
					1, dst_pitch*2, src_pitch*2);
		if (Use_Vertical_Filter)
		{	
			Fieldcopy(pCopyDest, pCopySrc, rowsize, 
						1, dst_pitch*2, src_pitch*2);			// copy first bob line
			Fieldcopy(pCopyDest+(FldHeight-1)*dst_pitch*2,
				pCopySrc+(FldHeight-1)*src_pitch*2, rowsize,	// copy last bob line
						1, dst_pitch*2, src_pitch*2);
			pWeaveDest += 2*dst_pitch;		// bug, adj for poor planning
		}
		else
		{
			Fieldcopy(pCopyDest, pCopySrc, rowsize, 
						FldHeight, dst_pitch*2, src_pitch*2);
		}
	}
	if (Use_Vertical_Filter)
	{
		if (SSEMMXenabled)
		{
			AvisynthTomsMoComp_SSE_VF(SearchEffortW, src_pitch, dst_pitch,
				rowsize, pWeaveSrc, pWeaveSrcP,
				pWeaveDest, TopFirst, pCopySrc,
				pCopySrcP, FldHeight);
		}
		else if (_3DNOWenabled)
		{
			AvisynthTomsMoComp_3DNOW_VF(SearchEffortW, src_pitch, dst_pitch,
				rowsize, pWeaveSrc, pWeaveSrcP,
				pWeaveDest, TopFirst, pCopySrc,
				pCopySrcP, FldHeight);
		}
		else
		{
			AvisynthTomsMoComp_MMX_VF(SearchEffortW, src_pitch, dst_pitch,
				rowsize, pWeaveSrc, pWeaveSrcP,
				pWeaveDest, TopFirst, pCopySrc,
				pCopySrcP, FldHeight);
		}
	}
	else
	{
		if (SSEMMXenabled)
		{
			AvisynthTomsMoComp_SSE(SearchEffortW, src_pitch, dst_pitch,
				rowsize, pWeaveSrc, pWeaveSrcP,
				pWeaveDest, TopFirst, pCopySrc,
				pCopySrcP, FldHeight);
		}
		else if (_3DNOWenabled)
		{
			AvisynthTomsMoComp_3DNOW(SearchEffortW, src_pitch, dst_pitch,
				rowsize, pWeaveSrc, pWeaveSrcP,
				pWeaveDest, TopFirst, pCopySrc,
				pCopySrcP, FldHeight);
		}
		else
		{
			AvisynthTomsMoComp_MMX(SearchEffortW, src_pitch, dst_pitch,
				rowsize, pWeaveSrc, pWeaveSrcP,
				pWeaveDest, TopFirst, pCopySrc,
				pCopySrcP, FldHeight);
		}
	}
	return 0;
}

int TomsMoComp::Fieldcopy(void *dest, const void *src, size_t count, 
				int rows, int dst_pitch, int src_pitch)
{
BYTE* pDest = (BYTE*) dest;
BYTE* pSrc = (BYTE*) src;
	
	for (int i=0; i < rows; i++)
	{
		memcpy(pDest, pSrc, count);
		pSrc += src_pitch;
		pDest += dst_pitch;
	}
	return 0;
}
int TomsMoComp::DoBorders(void *dest, const void *src, size_t count, 
				int rows, int dst_pitch, int src_pitch)
{
	_asm
	{
		mov	esi, src
		mov	edi, dest
		mov	ecx, rows
		mov	ebx, count
		sub		ebx, 8						// offset to last 8 bytes

bloop:
		movq	mm0, qword ptr[esi]			// left border
		movq	mm1, qword ptr[esi+8]		// "
		movq	mm2, qword ptr[esi+ebx]		// right border
		movq	mm3, qword ptr[esi+ebx+8]	// "	
		
		movq	qword ptr[edi], mm0			
		movq	qword ptr[edi+8], mm1			
		movq	qword ptr[edi+ebx], mm2	
		movq	qword ptr[edi+ebx+8], mm3	
		add     esi, src_pitch
		add		edi, dst_pitch
		loop	bloop
		emms
	}
	return 0;
}


AVSValue __cdecl Create_TomsMoComp(AVSValue args, void* user_data,
IScriptEnvironment* env) 
{
    return new TomsMoComp(args[0].AsClip(), args[1].AsInt(), args[2].AsInt(),
		args[3].AsInt(), env);
}


extern "C" __declspec(dllexport) const char* __stdcall
AvisynthPluginInit(IScriptEnvironment* env) {
    env->AddFunction("TomsMoComp", "ciii", Create_TomsMoComp, 0);
    return "`TomsMoComp' DeInterlace";
}

