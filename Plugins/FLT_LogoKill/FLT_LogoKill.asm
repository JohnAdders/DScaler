/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_LogoKill.asm,v 1.1 2002-10-09 22:16:35 robmuller Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Rob Muller. All rights reserved.
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
// This code is based on the 
// LogoAway VirtualDub filter by Krzysztof Wojdon (c) 2000
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// These macro's are borrowed from TomsMoComp:
/////////////////////////////////////////////////////////////////////////////

#undef V_PAVGB
#if defined(IS_SSE)
#define V_PAVGB(mmr1,mmr2,mmrw,smask) {pavgb mmr1,mmr2 }
#elif defined(IS_3DNOW)
#define V_PAVGB(mmr1,mmr2,mmrw,smask) {pavgusb mmr1,mmr2 }
#else
#define V_PAVGB(mmr1,mmr2,mmrw,smask) __asm \
	{ \
	__asm movq mmrw,mmr2 \
	__asm pand mmrw, smask \
	__asm psrlw mmrw,1 \
	__asm pand mmr1,smask \
	__asm psrlw mmr1,1 \
	__asm paddusb mmr1,mmrw \
	}
#endif

#undef V_PMAXUB
#if defined(IS_SSE)
#define V_PMAXUB(mmr1,mmr2) {pmaxub mmr1,mmr2 }
#else
#define V_PMAXUB(mmr1,mmr2)     __asm \
	{ \
    __asm psubusb mmr1,mmr2 \
    __asm paddusb mmr1,mmr2 \
    }
#endif

#undef V_PMINUB
#if defined(IS_SSE)
#define V_PMINUB(mmr1,mmr2,mmrw) {pminub mmr1,mmr2}
#else
#define V_PMINUB(mmr1,mmr2,mmrw) __asm \
	{ \
    __asm pcmpeqb mmrw, mmrw     \
    __asm psubusb mmrw, mmr2     \
    __asm paddusb mmr1, mmrw     \
    __asm psubusb mmr1, mmrw     \
	}
#endif

/////////////////////////////////////////////////////////////////////////////
// void LimitToMaximum(LONG Maximum, BYTE* lpOverlay, long Pitch);
//
// This mode limits all luma values to a specified maximum.
//
// mm0, mm1 and mm2 will be destroyed
/////////////////////////////////////////////////////////////////////////////
#undef LimitToMaximum

#if defined(IS_SSE)
void LimitToMaximum_SSE(LONG Maximum, BYTE* lpOverlay, long Pitch)
#define LimitToMaximum LimitToMaximum_SSE
#elif defined(IS_3DNOW)
void LimitToMaximum_3DNOW(LONG Maximum, BYTE* lpOverlay, long Pitch)
#define LimitToMaximum LimitToMaximum_3DNOW
#else
void LimitToMaximum_MMX(LONG Maximum, BYTE* lpOverlay, long Pitch)
#define LimitToMaximum LimitToMaximum_MMX
#endif
{
    __int64 MaxValue = 0;

    Maximum &= 0xff;

    MaxValue = 0xff00ff00 + Maximum + (Maximum << 16);
    MaxValue += MaxValue << 32;
    _asm
    {
            movq mm1, [MaxValue]
            
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // for each row
            mov eax, Height
LOOP_MAX_OUTER:
        // for each 4 pixel chunk
            mov edx, edi
            mov ecx, Width
LOOP_MAX_INNER:
            movq mm0, qword ptr[edx]
            V_PMINUB( mm0, mm1, mm2)
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_MAX_INNER
            add edi, ebx
            dec eax
            jnz LOOP_MAX_OUTER
    }
    
}

/////////////////////////////////////////////////////////////////////////////
// void FilterLogoKiller_???(TDeinterlaceInfo* pInfo);
/////////////////////////////////////////////////////////////////////////////

#if defined(IS_SSE)
long FilterLogoKiller_SSE(TDeinterlaceInfo* pInfo)
#elif defined(IS_3DNOW)
long FilterLogoKiller_3DNOW(TDeinterlaceInfo* pInfo)
#else
long FilterLogoKiller_MMX(TDeinterlaceInfo* pInfo)
#endif
{
    BYTE* lpOverlay = pInfo->Overlay + Left * 8;
    const __int64 qwGrey = 0x7f7f7f7f7f7f7f7f;
    long Pitch = pInfo->OverlayPitch;
    
    // check bounds
    if((Top + Height) >= pInfo->FrameHeight ||
        (Left + Width) >= pInfo->FrameWidth / 4)
    {
        return 1000;
    }

    switch(Mode)
    {
    case MODE_DYNAMIC_MAX:
        {
            BYTE MaxByte = 0;
            BYTE* pByte = NULL;
            int i = 0;

            // first find the highest luma value of the top and bottom row
            MaxByte = 0;
            pByte = lpOverlay + Top*Pitch;
            for(i = 0; i < (Width*4); i++)
            {
                if(*pByte > MaxByte)
                {
                    MaxByte = *pByte;
                }
                pByte++;
                pByte++;
            }
            pByte =  lpOverlay + Pitch*(Top + Height);
            for(i = 0; i < (Width*4); i++)
            {
                if(*pByte > MaxByte)
                {
                    MaxByte = *pByte;
                }
                pByte++;
                pByte++;
            }
            LimitToMaximum(MaxByte, lpOverlay, Pitch);
        }
        break;

    case MODE_MAX:
        LimitToMaximum(Max, lpOverlay, Pitch);
        break;

    // weighted average mode supplied by
    // Jochen Trenner
    case MODE_WEIGHTED_C:
		{
			int i,j;
			long Width2;
			long Mul1, Mul2, Mul3, Mul4;
			long Weight1, Weight2;
			long up1, up2, up3, up4, 
				down1, down2, down3, down4,
				left1, left2, left3, left4,
				right1, right2, right3, right4;
			long	ipo1, ipo2, ipo3, ipo4;
			BYTE* lpOverlay_Pointer;
			BYTE* lpCurrent_Pointer;
			int top_pitch;

			Width2=Width*2;
			top_pitch=Pitch*Top;
			lpOverlay_Pointer=lpOverlay+top_pitch;

			for (i=0;i<Height;i++)
			{
				left1=*(lpOverlay_Pointer+i*Pitch);
				left2=*(lpOverlay_Pointer+i*Pitch+1);
				left3=*(lpOverlay_Pointer+i*Pitch+2);
				left4=*(lpOverlay_Pointer+i*Pitch+3);
				right1=*(lpOverlay_Pointer+i*Pitch+Width2*4);
				right2=*(lpOverlay_Pointer+i*Pitch+Width2*4+1);
				right3=*(lpOverlay_Pointer+i*Pitch+Width2*4+2);
				right4=*(lpOverlay_Pointer+i*Pitch+Width2*4+3);
				Mul1=abs(Height-1-i);
				Mul2=i;
				Weight1=abs(((Height-1)/2)-i);
				++Weight1;
				for(j=0;j<Width2;j++)
				{
					Weight2=abs(Width-1-j);
					++Weight2;
					Mul3=abs(Width2-1-j);
					Mul4=j;
					up1=*(lpOverlay_Pointer+j*4);
					up2=*(lpOverlay_Pointer+j*4+1);
					up3=*(lpOverlay_Pointer+j*4+2);
					up4=*(lpOverlay_Pointer+j*4+3);

					down1=*(lpOverlay_Pointer+Pitch*Height+j*4);
					down2=*(lpOverlay_Pointer+Pitch*Height+j*4+1);
					down3=*(lpOverlay_Pointer+Pitch*Height+j*4+2);
					down4=*(lpOverlay_Pointer+Pitch*Height+j*4+3);
					lpCurrent_Pointer=lpOverlay_Pointer+i*Pitch+j*4;

					ipo1=(((((Mul1*up1)+(Mul2*down1))*Weight1/(Mul1+Mul2))+(((Mul3*left1)+Mul4*right1)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));
					ipo2=(((((Mul1*up2)+(Mul2*down2))*Weight1/(Mul1+Mul2))+(((Mul3*left2)+Mul4*right2)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));
					ipo3=(((((Mul1*up3)+(Mul2*down3))*Weight1/(Mul1+Mul2))+(((Mul3*left3)+Mul4*right3)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));
					ipo4=(((((Mul1*up4)+(Mul2*down4))*Weight1/(Mul1+Mul2))+(((Mul3*left4)+Mul4*right4)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));

					if (i>Height/3 && i<2*Height/3)
					{
						ipo1=(2*ipo1+up1+down1)/4;
						ipo2=(2*ipo2+up2+down2)/4;
						ipo3=(2*ipo3+up3+down3)/4;
						ipo4=(2*ipo4+up4+down4)/4;

					}



					*lpCurrent_Pointer=(BYTE)ipo1;
					*(lpCurrent_Pointer+1)=(BYTE)ipo2;
					*(lpCurrent_Pointer+2)=(BYTE)ipo3;
					*(lpCurrent_Pointer+3)=(BYTE)ipo4;
				}
			}

if(gUseSmoothing)
{
	_asm
        {
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // loop over height
            mov eax, Height
            LOOP_SMOOTH_OUTER:
            // loop over width
            mov edx, edi
            mov ecx, Width
            LOOP_SMOOTH_INNER:
			add edx, ebx //2 samples one line below
			movq mm4, qword ptr[edx-8]
			movq mm5, qword ptr[edx+8]
			V_PAVGB(mm4, mm5, mm1, ShiftMask)
			sub edx, ebx
            sub edx, ebx //2 samples one line above
			movq mm1, qword ptr[edx-8]
			movq mm2, qword ptr[edx+8]
			V_PAVGB(mm1, mm2, mm7, ShiftMask);
			V_PAVGB(mm1, mm4, mm7, ShiftMask);
			add edx, ebx
            movq mm0, qword ptr[edx]
			V_PAVGB(mm0,mm1, mm2, ShiftMask);
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_SMOOTH_INNER
            add edi, ebx
            dec eax
            jnz LOOP_SMOOTH_OUTER
        }
}
        }
        break;
    // weighted average mode supplied by
    // Jochen Trenner
    case MODE_WEIGHTED_ASM:
		{
			int Width_third=Width/3;
			int Height_third=Height/3;
			int Height_2third=Height/3*2;
        _asm
        {
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            mov eax, Height
            LOOP_JT1_OUTER:
            mov edx, edi
            movq mm0, qword ptr[edx]
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
            movq mm1, qword ptr[edx]
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			movq mm2, mm0
			V_PAVGB(mm2, mm1, mm3, ShiftMask)
            mov ecx, Width_third

            LOOP_JT1_LEFT:
			push eax
			push ecx
			mov ecx, Height
			sub ecx, eax
			imul ecx, ebx
			sub edx, ecx
			movq mm3, qword ptr[edx]
			add edx, ecx
			imul eax, ebx
			add edx, eax
			movq mm4, qword ptr[edx]
			sub edx, eax
			pop ecx
			pop eax
			cmp eax, Height_2third
			jge LA
			cmp eax, Height_third
			jge LM
			movq mm5, mm4
			jmp LEFT_AVG
            LA:			
            movq mm5, mm3
			jmp LEFT_AVG
            LM:
            V_PAVGB(mm4, mm3, mm5, ShiftMask)
			movq mm5, mm4
            LEFT_AVG:	
            V_PAVGB(mm5, mm0, mm3, ShiftMask)
            movq qword ptr[edx], mm5
            add edx, 8
            dec ecx
            jnz LOOP_JT1_LEFT

            mov ecx, Width_third
            LOOP_JT1_MIDDLE:
			push eax
			push ecx
			mov ecx, Height
			sub ecx, eax
			imul ecx, ebx
			sub edx, ecx
			movq mm3, qword ptr[edx]
			add edx, ecx
			imul eax, ebx
			add edx, eax
			movq mm4, qword ptr[edx]
			sub edx, eax
			pop ecx
			pop eax
			cmp eax, Height_2third
			jge MA
			cmp eax, Height_third
			jge MM
			movq mm5, mm4
			jmp MIDDLE_AVG
            MA:
            movq mm5, mm3
			jmp MIDDLE_AVG
            MM:
            V_PAVGB(mm4, mm3, mm5, ShiftMask)
			movq mm5, mm4
            MIDDLE_AVG:	
            V_PAVGB(mm5, mm2, mm3, ShiftMask)
			movq qword ptr[edx], mm5
            add edx, 8
            dec ecx
            jnz LOOP_JT1_MIDDLE

            mov ecx, Width_third
            
            LOOP_JT1_RIGHT:
			push eax
			push ecx
			mov ecx, Height
			sub ecx, eax
			imul ecx, ebx
			sub edx, ecx
			movq mm3, qword ptr[edx]
			add edx, ecx
			imul eax, ebx
			add edx, eax
			movq mm4, qword ptr[edx]
			sub edx, eax
			pop ecx
			pop eax
			cmp eax, Height_2third
			jge RA
			cmp eax, Height_third
			jge RM
			movq mm5, mm4
			jmp RIGHT_AVG
            RA:			
            movq mm5, mm3
			jmp RIGHT_AVG
            RM:			
            V_PAVGB(mm4, mm3, mm5, ShiftMask)
			movq mm5, mm4
            RIGHT_AVG:	
            V_PAVGB(mm5, mm1, mm3, ShiftMask)
            movq qword ptr[edx], mm5
            add edx, 8
            dec ecx
            jnz LOOP_JT1_RIGHT

            add edi, ebx
            dec eax
            jnz LOOP_JT1_OUTER
        }
if(gUseSmoothing)
{
	_asm
        {
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // loop over height
            mov eax, Height
            LOOP_SMOOTH_OUTER_ASM:
            // loop over width
            mov edx, edi
            mov ecx, Width
            LOOP_SMOOTH_INNER_ASM:
			add edx, ebx //2 samples one line below
			movq mm4, qword ptr[edx-8]
			movq mm5, qword ptr[edx+8]
			V_PAVGB(mm4, mm5, mm1, ShiftMask)
			sub edx, ebx
            sub edx, ebx //2 samples one line above
			movq mm1, qword ptr[edx-8]
			movq mm2, qword ptr[edx+8]
			V_PAVGB(mm1, mm2, mm0, ShiftMask)
			V_PAVGB(mm1, mm4, mm0, ShiftMask)
			add edx, ebx
            movq mm0, qword ptr[edx]
			V_PAVGB(mm0,mm1, mm4, ShiftMask)
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_SMOOTH_INNER_ASM
            add edi, ebx
            dec eax
            jnz LOOP_SMOOTH_OUTER_ASM
        }
}
        break;
		}
    case MODE_GREY:
    default:
        _asm
        {
            // set up mm0 as mid grey
            movq mm0, qwGrey
            
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // loop over height
            mov eax, Height
            LOOP_GREY_OUTER:
            // loop over width
            mov edx, edi
            mov ecx, Width
            LOOP_GREY_INNER:
            // set area to grey
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_GREY_INNER
            add edi, ebx
            dec eax
            jnz LOOP_GREY_OUTER
        }
        break;
    }
    _asm
    {
        emms
    }
    return 1000;
}
