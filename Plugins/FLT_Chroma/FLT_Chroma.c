/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_Chroma.c,v 1.4 2003-06-26 11:42:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2003/01/03 17:26:27  adcockj
// First cut of the chroma filter - sssslllloooowwwww
//
// Revision 1.2  2003/01/02 16:22:57  adcockj
// Variable rename
//
// Revision 1.1  2003/01/02 13:15:01  adcockj
// Added new plug-ins ready for developement by copying TomsMoComp and Gamma
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// This is the long awaited chroma filter
// it does 2 things:
// 1) It can shift chroma in sub pixel increments to compensate for errors
//    Anywhere in the video chain
// 2) Perform a simple low pass vertical to smooth out the chroma image
//    to compensate for errors in 4:2:0 -> 4:2:2 convertion
/////////////////////////////////////////////////////////////////////////////
// in trying to work out what is going on below you will need to understand 
// how the pixels are stored in an MMX register
// each MMX register contains 4 pixels worth of luma in the low bytes of each word
// we number these L1 L2 L3 L4 going from left to right
// It also contains U and V samples in the hight bytes of each word, these are
// U1 V1 U3 V3
// When you look at an MMX register in the debugger you see
// V2 L3 U2 L2 V0 L1 U0 L0
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "math.h"
#include "..\help\helpids.h"

long VertShift = 0;
long HorzShift = 0;
BOOL UseLowPassFilter = FALSE;

FILTER_METHOD ChromaMethod;

//////////////////////////////////////////////////////////////////////////////
// This code applies a very simple low pass filter
// to the chroma it is a (1/4, 1/2, 1/4) filter and is applied
// in vertical strips to avoid any feedback problems
// For an explaination as top why see
// http://www.hometheaterhifi.com/volume_8_2/dvd-benchmark-special-report-chroma-bug-4-2001.html
//////////////////////////////////////////////////////////////////////////////
void DoLowPassOnly(TDeinterlaceInfo* pInfo)
{
    BYTE* Pixels = NULL;
    int i;
    int Cycles;
    int Pitch;
    const __int64 qwLumaMask    = 0x00ff00ff00ff00ff;

    if (pInfo->Overlay == NULL)
    {
        return;
    }

    Cycles = pInfo->FrameHeight - 2;
    Pitch = pInfo->OverlayPitch;
    Pixels = pInfo->Overlay;

    for (i = 0; i < pInfo->FrameWidth / 4; ++i)
    {
        _asm
        {
            mov ecx, Cycles
            mov edx, dword ptr[Pixels]
            mov ebx, dword ptr[Pitch]
            movq mm2, [edx]
            add  edx, ebx
            movq mm3, [edx]
            movq mm0, qwLumaMask
LOOP_LABEL_LPO:
            add  edx, ebx
            // mm1 = value above "current" line
            // mm2 = value of "current" line
            // mm3 = value below "current" line
            movq mm1, mm2
            movq mm2, mm3
            movq mm3, [edx]
            // get just chroma value of top line
            psrlw mm1, 8  
            // save the luma values of the current line
            movq mm5, mm2
            pand mm5, mm0
            // get just chroma value of current line * 2
            movq mm6, mm2
            psrlw mm6, 8
            psllw mm6, 1
            // get just chroma value of bottom line
            movq  mm7, mm3
            psrlw mm7, 8
            // add up all the chromas and divide by 4
            paddw mm6, mm7
            paddw mm6, mm1
            psrlw mm6, 2
            psllw mm6, 8
            // combine the old luma with the new chroma
            // and save
            por mm5, mm6
            sub  edx, ebx
            movq [edx], mm5
            add  edx, ebx
            loop LOOP_LABEL_LPO
        }
        Pixels += 8;
    }
    _asm
    {
        emms
    }
}

void DoUpVertShift(TDeinterlaceInfo* pInfo, int MyVertShift)
{
    BYTE* Pixels = NULL;
    int i;
    int Cycles;
    int Pitch;
    int TopDiff;
    __int64 qwLumaMask    = 0x00ff00ff00ff00ff;
    __int64 qwTopWeight;
    __int64 qwBottomWeight;

    if (pInfo->Overlay == NULL)
    {
        return;
    }

    Cycles = pInfo->FrameWidth / 4;
    Pitch = pInfo->OverlayPitch;
    Pixels = pInfo->Overlay;
    TopDiff = MyVertShift / 8;
    qwBottomWeight = MyVertShift - TopDiff * 8;
    qwTopWeight = 8 - qwBottomWeight;
    qwTopWeight += (qwTopWeight << 48) + (qwTopWeight << 32) + (qwTopWeight << 16);
    qwBottomWeight += (qwBottomWeight << 48) + (qwBottomWeight << 32) + (qwBottomWeight << 16);

    for (i = 0; i < pInfo->FrameHeight - TopDiff - 1; ++i)
    {
        _asm
        {
            mov ecx, Cycles
            mov ebx, dword ptr[Pitch]
            mov eax, ebx
            // this takes eax and puts result in eax and edx
            // so don't use edx until afterwards
            mul TopDiff
            mov edx, dword ptr[Pixels]
            add eax, edx
            movq mm0, qwLumaMask
            movq mm6, qwTopWeight
            movq mm7, qwBottomWeight
LOOP_LABEL_UVS:
            // mm1 = value of "current" line
            // mm2 = Chroma of Top line
            // mm3 = Chroma of Bottom line
            movq mm1, [edx]
            movq mm2, [eax]
            add eax, ebx
            movq mm3, [eax]
            sub eax, ebx
            pand mm1, mm0
            psrlw mm2, 8  
            psrlw mm3, 8  
            pmullw mm2, mm6
            pmullw mm3, mm7
            paddw mm2, mm3
            psrlw mm2, 3
            psllw mm2, 8
            // combine the old luma with the new chroma
            // and save
            por mm1, mm2
            movq [edx], mm1
            add  edx, 8
            add  eax, 8
            loop LOOP_LABEL_UVS
        }
        Pixels += Pitch;
    }
    _asm
    {
        emms
    }
}

void DoDownVertShift(TDeinterlaceInfo* pInfo, int MyVertShift)
{
    BYTE* Pixels = NULL;
    int i;
    int Cycles;
    int Pitch;
    int BottomDiff;
    __int64 qwLumaMask    = 0x00ff00ff00ff00ff;
    __int64 qwTopWeight;
    __int64 qwBottomWeight;

    if (pInfo->Overlay == NULL)
    {
        return;
    }

    Cycles = pInfo->FrameWidth / 4;
    Pitch = pInfo->OverlayPitch;
    BottomDiff = MyVertShift / 8;
    qwTopWeight = MyVertShift - BottomDiff * 8;
    qwBottomWeight = 8 - qwTopWeight;
    qwTopWeight += (qwTopWeight << 48) + (qwTopWeight << 32) + (qwTopWeight << 16);
    qwBottomWeight += (qwBottomWeight << 48) + (qwBottomWeight << 32) + (qwBottomWeight << 16);

    for (i = pInfo->FrameHeight - 1; i > BottomDiff; --i)
    {
        Pixels = pInfo->Overlay + i * pInfo->OverlayPitch;
        _asm
        {
            mov ecx, Cycles
            mov ebx, dword ptr[Pitch]
            mov eax, ebx
            // this takes eax and puts result in eax and edx
            // so don't use edx until afterwards
            mul BottomDiff
            mov edx, dword ptr[Pixels]
            neg eax
            add eax, edx
            movq mm0, qwLumaMask
            movq mm6, qwTopWeight
            movq mm7, qwBottomWeight
LOOP_LABEL_DVS:
            // mm1 = value of "current" line
            // mm2 = Chroma of Top line
            // mm3 = Chroma of Bottom line
            movq mm1, [edx]
            sub eax, ebx
            movq mm2, [eax]
            add eax, ebx
            movq mm3, [eax]
            pand mm1, mm0
            psrlw mm2, 8  
            psrlw mm3, 8  
            pmullw mm2, mm6
            pmullw mm3, mm7
            paddw mm2, mm3
            psrlw mm2, 3
            psllw mm2, 8
            // combine the old luma with the new chroma
            // and save
            por mm1, mm2
            movq [edx], mm1
            add  edx, 8
            add  eax, 8
            loop LOOP_LABEL_DVS
        }
    }
    _asm
    {
        emms
    }
}

void DoLeftHorzShift(TDeinterlaceInfo* pInfo, int MyHorzShift)
{
    BYTE* Pixels = NULL;
    int i;
    int Cycles;
    int Pitch;
    __int64 qwLumaMask    = 0x00ff00ff00ff00ff;
    __int64 qwHalfMask1   = 0x00000000ffffffff;
    __int64 qwHalfMask2   = 0xffffffff00000000;
    __int64 qwWeight1     = 0x0000000000000000;
    __int64 qwWeight2     = 0x0000000000000000;
    __int64 qwWeight3     = 0x0000000000000000;

    if (pInfo->Overlay == NULL)
    {
        return;
    }

    Cycles = pInfo->FrameWidth / 4 - 1;
    Pitch = pInfo->OverlayPitch;
    Pixels = pInfo->Overlay;
    if(MyHorzShift < 16)
    {
        qwWeight2 =  MyHorzShift;
        qwWeight1 =  16 - qwWeight2;
        qwWeight1 += (qwWeight1 << 48) + (qwWeight1 << 32) + (qwWeight1 << 16);
        qwWeight2 += (qwWeight2 << 48) + (qwWeight2 << 32) + (qwWeight2 << 16);

        for (i = 0; i < pInfo->FrameHeight; ++i)
        {
            _asm
            {
                mov ecx, Cycles
                mov edx, dword ptr[Pixels]
                movq mm0, qwLumaMask
                movq mm7, qwHalfMask1
                movq mm6, [edx]
LOOP_LABEL_LHS1:
                // mm1 = Store of "current" luma set 
                // mm2 = 00 00 V1 U1
                // mm3 = V3 U3 00 00
                // mm4 = 00 00 V5 U5
                // mm6 = Store of "next" pixel set 
                add edx, 8
                movq mm1, mm6
                movq mm2, mm6
                pand mm1, mm0
                psrlw mm2, 8
                movq mm3, mm2
                pand mm2, mm7
                pand mm3, qwHalfMask2
                movq mm6, [edx]
                movq mm4, mm6
                psrlw mm4, 8

                pmullw mm2, qwWeight1

                // OK now we have mm5 spare
                movq mm5, mm3
                pmullw mm3, qwWeight1
                paddw mm2, mm3
                psrlq mm5, 32
                pmullw mm5, qwWeight2
                paddw mm2, mm5

                psllq mm4, 32
                pmullw mm4, qwWeight2
                paddw mm2, mm4
            
                psrlw mm2, 4
                psllw mm2, 8

                por mm1, mm2
                sub edx, 8
                movq [edx], mm1
                add edx, 8
                loop LOOP_LABEL_LHS1
            }
            Pixels += Pitch;
        }
    }
    else
    {
        qwWeight3 =  MyHorzShift - 16;
        qwWeight2 =  16 - qwWeight3;
        qwWeight3 += (qwWeight3 << 48) + (qwWeight3 << 32) + (qwWeight3 << 16);
        qwWeight2 += (qwWeight2 << 48) + (qwWeight2 << 32) + (qwWeight2 << 16);

        for (i = 0; i < pInfo->FrameHeight; ++i)
        {
            _asm
            {
                mov ecx, Cycles
                mov edx, dword ptr[Pixels]
                movq mm0, qwLumaMask
                movq mm7, qwHalfMask1
                movq mm6, [edx]
    LOOP_LABEL_LPO:
                // mm1 = Store of "current" luma set 
                // mm2 = 00 00 V1 U1
                // mm3 = V3 U3 00 00
                // mm4 = 00 00 V5 U5
                // mm5 = V7 U7 00 00
                // mm6 = Store of "next" pixel set 
                add edx, 8
                movq mm1, mm6
                movq mm2, mm6
                pand mm1, mm0
                psrlw mm2, 8
                movq mm3, mm2
                pand mm2, mm7
                pand mm3, qwHalfMask2
                movq mm6, [edx]
                movq mm4, mm6
                psrlw mm4, 8
                movq mm2, mm4
                pand mm4, mm7
                pand mm2, qwHalfMask2

                pmullw mm2, qwWeight3

                psrlq mm3, 32
                pmullw mm3, qwWeight2
                paddw mm2, mm3

                movq mm5, mm4
                pmullw mm4, qwWeight3
                paddw mm2, mm4
                psllq mm5, 32
                pmullw mm5, qwWeight2
                paddw mm2, mm5
            
                psrlw mm2, 4
                psllw mm2, 8

                por mm1, mm2
                sub edx, 8
                movq [edx], mm1
                add edx, 8
                loop LOOP_LABEL_LPO
            }
            Pixels += Pitch;
        }
    }

    _asm
    {
        emms
    }
}

void DoRightHorzShift(TDeinterlaceInfo* pInfo, int MyHorzShift)
{
    BYTE* Pixels = NULL;
    int i;
    int Cycles;
    int Pitch;
    __int64 qwLumaMask    = 0x00ff00ff00ff00ff;
    __int64 qwHalfMask1   = 0x00000000ffffffff;
    __int64 qwHalfMask2   = 0xffffffff00000000;
    __int64 qwWeight1     = 0x0000000000000000;
    __int64 qwWeight2     = 0x0000000000000000;
    __int64 qwWeight3     = 0x0000000000000000;

    if (pInfo->Overlay == NULL)
    {
        return;
    }

    Cycles = pInfo->FrameWidth / 4 - 1;
    Pitch = pInfo->OverlayPitch;
    Pixels = pInfo->Overlay + 2 + pInfo->FrameWidth * 2 - 8;
    if(MyHorzShift < 16)
    {
        qwWeight2 =  MyHorzShift;
        qwWeight1 =  16 - qwWeight2;
        qwWeight1 += (qwWeight1 << 48) + (qwWeight1 << 32) + (qwWeight1 << 16);
        qwWeight2 += (qwWeight2 << 48) + (qwWeight2 << 32) + (qwWeight2 << 16);
        _asm
        {
            mov ecx, Cycles
            mov edx, dword ptr[Pixels]
            movq mm0, qwLumaMask
            movq mm7, qwHalfMask1
            movq mm6, [edx]
LOOP_LABEL_RHS1:
            sub edx, 8
            movq mm1, mm6
            movq mm2, mm6
            pand mm1, mm0
            psrlw mm2, 8
            movq mm3, mm2
            pand mm2, mm7
            pand mm3, qwHalfMask2
            movq mm6, [edx]
            movq mm5, mm6
            psrlw mm5, 8
            pand mm5, qwHalfMask2
            pmullw mm3, qwWeight1

            // OK now we have mm4 so use it
            movq mm4, mm2
            pmullw mm2, qwWeight1
            paddw mm3, mm2
            psllq mm4, 32
            pmullw mm4, qwWeight2
            paddw mm3, mm4

            psrlq mm5, 32
            pmullw mm5, qwWeight2
            paddw mm3, mm5
        
            psrlw mm3, 4
            psllw mm3, 8

            por mm1, mm3
            add edx, 8
            movq [edx], mm1
            sub edx, 8
            loop LOOP_LABEL_RHS1
        }
        Pixels += Pitch;
    }
    else
    {
        qwWeight3 =  MyHorzShift - 16;
        qwWeight2 =  16 - qwWeight3;
        qwWeight3 += (qwWeight3 << 48) + (qwWeight3 << 32) + (qwWeight3 << 16);
        qwWeight2 += (qwWeight2 << 48) + (qwWeight2 << 32) + (qwWeight2 << 16);
        _asm
        {
            mov ecx, Cycles
            mov edx, dword ptr[Pixels]
            movq mm0, qwLumaMask
            movq mm7, qwHalfMask1
            movq mm6, [edx]

LOOP_LABEL_RHS2:
            sub edx, 8
            movq mm1, mm6
            movq mm2, mm6
            pand mm1, mm0
            psrlw mm2, 8
            pand mm2, mm7
            movq mm6, [edx]
            movq mm3, mm6
            psrlw mm3, 8
            movq mm5, mm3
            pand mm3, mm7
            pand mm5, qwHalfMask2

            pmullw mm3, qwWeight3

            psllq mm2, 32
            pmullw mm2, qwWeight2
            paddw mm3, mm2

            movq mm4, mm5
            pmullw mm5, qwWeight3
            paddw mm3, mm5
            psrlq mm4, 32
            pmullw mm4, qwWeight2
            paddw mm3, mm4
        
            psrlw mm3, 4
            psllw mm3, 8

            por mm1, mm3
            add edx, 8
            movq [edx], mm1
            sub edx, 8
            loop LOOP_LABEL_RHS2
        }
        Pixels += Pitch;
    }

    for (i = 0; i < pInfo->FrameHeight; ++i)
    {
        _asm
        {
            mov ecx, Cycles
            mov edx, dword ptr[Pixels]
            movq mm0, qwLumaMask
            movq mm7, qwHalfMask1
            movq mm6, [edx]
LOOP_LABEL_LPO:
            // mm1 = Store of "current" luma set 
            // mm2 = 00 00 V1 U1
            // mm3 = V3 U3 00 00
            // mm4 = 00 00 V5 U5
            // mm5 = V7 U7 00 00
            // mm6 = Store of "next" pixel set 
            sub edx, 8
            movq mm1, mm6
            movq mm2, mm6
            pand mm1, mm0
            psrlw mm2, 8
            movq mm3, mm2
            pand mm2, mm7
            pand mm3, qwHalfMask2
            movq mm6, [edx]
            movq mm4, mm6
            psrlw mm4, 8
            movq mm5, mm4
            pand mm4, mm7
            pand mm5, qwHalfMask2
            pmullw mm3, qwWeight1
            pmullw mm4, qwWeight3
            paddw mm3, mm4
            // OK now we have mm4 back we use it
            movq mm4, mm2
            pmullw mm2, qwWeight1
            paddw mm3, mm2
            psllq mm4, 32
            pmullw mm4, qwWeight2
            paddw mm3, mm4

            movq mm4, mm5
            pmullw mm5, qwWeight3
            paddw mm3, mm5
            psrlq mm4, 32
            pmullw mm4, qwWeight2
            paddw mm3, mm4
            
            psrlw mm3, 4
            psllw mm3, 8

            por mm1, mm3
            add edx, 8
            movq [edx], mm1
            sub edx, 8
            loop LOOP_LABEL_LPO
        }
        Pixels += Pitch;
    }
    _asm
    {
        emms
    }
}

void DoLeftHorzShiftUpVertShift(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoLeftHorzShift(pInfo, MyHorzShift);
    DoUpVertShift(pInfo, MyVertShift);
}

void DoLeftHorzShiftDownVertShift(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoLeftHorzShift(pInfo, MyHorzShift);
    DoDownVertShift(pInfo, MyVertShift);
}

void DoRightHorzShiftUpVertShift(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoRightHorzShift(pInfo, MyHorzShift);
    DoUpVertShift(pInfo, MyVertShift);
}

void DoRightHorzShiftDownVertShift(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoRightHorzShift(pInfo, MyHorzShift);
    DoDownVertShift(pInfo, MyVertShift);
}

void DoRightHorzShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyHorzShift)
{
    DoRightHorzShift(pInfo, MyHorzShift);
    DoLowPassOnly(pInfo);
}

void DoUpVertShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyVertShift)
{
    DoUpVertShift(pInfo, MyVertShift);
    DoLowPassOnly(pInfo);
}

void DoDownVertShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyVertShift)
{
    DoDownVertShift(pInfo, MyVertShift);
    DoLowPassOnly(pInfo);
}

void DoLeftHorzShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyHorzShift)
{
    DoLeftHorzShift(pInfo, MyHorzShift);
    DoLowPassOnly(pInfo);
}


void DoLeftHorzShiftUpVertShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoLeftHorzShiftUpVertShift(pInfo, MyHorzShift, MyVertShift);
    DoLowPassOnly(pInfo);
}

void DoLeftHorzShiftDownVertShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoLeftHorzShiftDownVertShift(pInfo, MyHorzShift, MyVertShift);
    DoLowPassOnly(pInfo);
}

void DoRightHorzShiftUpVertShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoRightHorzShiftUpVertShift(pInfo, MyHorzShift, MyVertShift);
    DoLowPassOnly(pInfo);
}

void DoRightHorzShiftDownVertShiftWithLowPass(TDeinterlaceInfo* pInfo, int MyHorzShift, int MyVertShift)
{
    DoRightHorzShiftDownVertShift(pInfo, MyHorzShift, MyVertShift);
    DoLowPassOnly(pInfo);
}



long __cdecl FilterChroma(TDeinterlaceInfo* pInfo)
{
    // take copies of variables so that they don't change
    // while running
    long MyVertShift = VertShift;
    long MyHorzShift = HorzShift;
    BOOL MyUseLowPassFilter = UseLowPassFilter;

    if(MyHorzShift == 0)
    {
        if(MyVertShift == 0)
        {
            if(MyUseLowPassFilter == TRUE)
            {
                DoLowPassOnly(pInfo);
            }
        }
        else
        {
            if(MyUseLowPassFilter == TRUE)
            {
                if(VertShift > 0)
                {
                    DoUpVertShiftWithLowPass(pInfo, MyVertShift);
                }
                else
                {
                    DoDownVertShiftWithLowPass(pInfo, MyVertShift);
                }
            }
            else
            {
                if(VertShift > 0)
                {
                    DoUpVertShift(pInfo, MyVertShift);
                }
                else
                {
                    DoDownVertShift(pInfo, -MyVertShift);
                }
            }

        }
    }
    else if(MyHorzShift < 0)
    {
        if(MyVertShift == 0)
        {
            if(MyUseLowPassFilter == TRUE)
            {
                DoLeftHorzShiftWithLowPass(pInfo, -MyHorzShift);
            }
            else
            {
                DoLeftHorzShift(pInfo, -MyHorzShift);
            }
        }
        else
        {
            if(MyUseLowPassFilter == TRUE)
            {
                if(VertShift > 0)
                {
                    DoLeftHorzShiftUpVertShiftWithLowPass(pInfo, -MyHorzShift, MyVertShift);
                }
                else
                {
                    DoLeftHorzShiftDownVertShiftWithLowPass(pInfo, -MyHorzShift, -MyVertShift);
                }
            }
            else
            {
                if(VertShift > 0)
                {
                    DoLeftHorzShiftUpVertShift(pInfo, -MyHorzShift, MyVertShift);
                }
                else
                {
                    DoLeftHorzShiftDownVertShift(pInfo, -MyHorzShift, -MyVertShift);
                }
            }
        }
    }
    else
    {
        if(MyVertShift == 0)
        {
            if(MyUseLowPassFilter == TRUE)
            {
                DoRightHorzShiftWithLowPass(pInfo, MyHorzShift);
            }
            else
            {
                DoRightHorzShift(pInfo, MyHorzShift);
            }
        }
        else
        {
            if(MyUseLowPassFilter == TRUE)
            {
                if(VertShift > 0)
                {
                    DoRightHorzShiftUpVertShiftWithLowPass(pInfo, MyHorzShift, MyVertShift);
                }
                else
                {
                    DoRightHorzShiftDownVertShiftWithLowPass(pInfo, MyHorzShift, -MyVertShift);
                }
            }
            else
            {
                if(VertShift > 0)
                {
                    DoRightHorzShiftUpVertShift(pInfo, MyHorzShift, MyVertShift);
                }
                else
                {
                    DoRightHorzShiftDownVertShift(pInfo, MyHorzShift, -MyVertShift);
                }
            }

        }
    }

    return 1000;
}


////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////
SETTING FLT_ChromaSettings[FLT_CHROMA_SETTING_LASTONE] =
{
    {
        "Horizontal Shift", SLIDER, 0, &HorzShift,
        0, -32, 32, 1, 1,
        NULL,
        "ChromaFilter", "HorzShift", NULL,
    },
    {
        "Vertical Shift", SLIDER, 0, &VertShift,
        0, -32, 32, 1, 1,
        NULL,
        "ChromaFilter", "VertShift", NULL,
    },
    {
        "Vertical Low-Pass Filter", ONOFF, 0, &UseLowPassFilter,
        FALSE, 0, 1, 1, 1,
        NULL,
        "ChromaFilter", "UseLowPassFilter", NULL,
    },
    {
        "Chroma Filter", ONOFF, 0, &(ChromaMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "ChromaFilter", "UseChromaFilter", NULL,
    },
};

FILTER_METHOD ChromaMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Chroma Filter",
    "&Chroma",
    FALSE,
    FALSE,
    FilterChroma, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_CHROMA_SETTING_LASTONE,
    FLT_ChromaSettings,
    WM_FLT_CHROMA_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_FILTERS,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &ChromaMethod;
}


BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}
