/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_LogoKill.asm,v 1.3 2002-10-14 21:55:36 robmuller Exp $
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
// Revision 1.2  2002/10/14 20:43:42  robmuller
// Changed into input filter. New mode added. Miscellaneous improvements.
//
// Revision 1.1  2002/10/09 22:16:35  robmuller
// Implemented 3dnow and MMX versions.
//
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
void LimitToMaximum_SSE(LONG Maximum, BYTE* lpLogoRect, long Pitch, long Height, long Width)
#define LimitToMaximum LimitToMaximum_SSE
#elif defined(IS_3DNOW)
void LimitToMaximum_3DNOW(LONG Maximum, BYTE* lpLogoRect, long Pitch, long Height, long Width)
#define LimitToMaximum LimitToMaximum_3DNOW
#else
void LimitToMaximum_MMX(LONG Maximum, BYTE* lpLogoRect, long Pitch, long Height, long Width)
#define LimitToMaximum LimitToMaximum_MMX
#endif
{
    // TODO: implement code to handle 4 byte alignment
    __int64 MaxValue = 0;

    Maximum &= 0xff;

    MaxValue = 0xff00ff00 + Maximum + (Maximum << 16);
    MaxValue += MaxValue << 32;
    _asm
    {
            movq mm1, [MaxValue]
            
            // set edi to top left
            mov edi, lpLogoRect
            mov ebx, Pitch
            // for each row
            mov eax, Height
LOOP_MAX_OUTER:
        // for each 4 pixel chunk
            mov edx, edi
            mov ecx, Width
            shr ecx, 2
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
    BYTE* lpLogoRect = NULL; 
    long Pitch = pInfo->InputPitch;

    long Top    = Top_UI;
    long Height = Height_UI;
    long Left   = Left_UI;
    long Width  = Width_UI;

    // 8 byte aligned variables for use by MMX routines
    BYTE* lpLogoRect8 = NULL; 
    long Left8, Width8;

    // Limit the logo rectangle to the boundaries of the screen
    if(Top + Height > 1000)
    {
        Height = 1000 - Top;
    }
    if(Left + Width > 1000)
    {
        Width = 1000 - Left;
    }

    // transform to lines/pixels
    Top    = pInfo->FieldHeight*Top/1000;
    Height = pInfo->FieldHeight*Height/1000;
    Left   = pInfo->FrameWidth*Left/1000;
    Width  = pInfo->FrameWidth*Width/1000;

    // align Left and Width to 2 pixels to make calculations with chroma easier
    Left = Left & ~1;                           // align towards left
    Width += Width % 2;                         // align towards right

    if (pInfo->PictureHistory[0] == NULL || pInfo->PictureHistory[0]->pData == NULL)
    {
        return 1000;
    }

    lpLogoRect = pInfo->PictureHistory[0]->pData + Left*2 + Top*Pitch;

    Left8 = Left & ~7;                          //align the left boundary to 8 bytes; round down
    Width8 = (Width + 7) & ~7;                  //align the right boundary to 8 bytes; round up
    lpLogoRect8 = pInfo->PictureHistory[0]->pData + Left8*2 + Top*Pitch;
    
    switch(Mode)
    {
    case MODE_DYNAMIC_MAX:
        {
            BYTE MaxByte = 0;
            BYTE* pByte = lpLogoRect;
            int i = 0;

            // first find the highest luma value of the top and bottom boundary
            MaxByte = 0;
            for(i = 0; i < Width; i++)
            {
                if(*pByte > MaxByte)
                {
                    MaxByte = *pByte;
                }
                pByte++;
                pByte++;
            }
            pByte =  lpLogoRect + Pitch*Height;
            for(i = 0; i < Width; i++)
            {
                if(*pByte > MaxByte)
                {
                    MaxByte = *pByte;
                }
                pByte++;
                pByte++;
            }
            // left/right boundary
            pByte =  lpLogoRect;
            for(i = 0; i < Height; i++)
            {
                if(*pByte > MaxByte)
                {
                    MaxByte = *pByte;
                }
                pByte += Pitch;
            }
            pByte =  lpLogoRect + Width*2;
            for(i = 0; i < Height; i++)
            {
                if(*pByte > MaxByte)
                {
                    MaxByte = *pByte;
                }
                pByte += Pitch;
            }

            LimitToMaximum(MaxByte, lpLogoRect8, Pitch, Height, Width8);
        }
        break;

    case MODE_MAX:
        LimitToMaximum(Max, lpLogoRect8, Pitch, Height, Width8);
        break;

    case MODE_WEIGHTED_HV:
		{

            int i, j;
            BYTE* pByte;
            TwoPixel* pTwoPixel;
            TwoPixel* pFirstLine;
            TwoPixel* pLastLine;
            TwoPixel Hor, Vert;

            pFirstLine = (TwoPixel*)lpLogoRect;
            pLastLine = (TwoPixel*)(lpLogoRect + Height*Pitch);

            for(j = 1; j < Height - 1; j++)
            {
                pByte = lpLogoRect + j*Pitch;
                pTwoPixel = (TwoPixel*)(lpLogoRect + j*Pitch);
                for(i = 1; i < Width/2 - 1; i++)
                {
                    Hor.Lumi1    = (BYTE)((pTwoPixel[0].Lumi1*(Width-i*2) + pTwoPixel[Width/2].Lumi2*(i*2))/(Width));
                    Hor.Lumi2    = (BYTE)((pTwoPixel[0].Lumi1*(Width-i*2-1) + pTwoPixel[Width/2].Lumi2*(i*2+1))/(Width));
                    Hor.Chroma1  = (BYTE)((pTwoPixel[0].Chroma1*(Width-i*2) + pTwoPixel[Width/2].Chroma1*(i*2))/(Width));
                    Hor.Chroma2  = (BYTE)((pTwoPixel[0].Chroma2*(Width-i*2) + pTwoPixel[Width/2].Chroma2*(i*2))/(Width));

                    Vert.Lumi1   = (BYTE)((pFirstLine[i].Lumi1*(Height-j) + pLastLine[i].Lumi2*j)/(Height));
                    Vert.Lumi2   = (BYTE)((pFirstLine[i].Lumi2*(Height-j) + pLastLine[i].Lumi2*j)/(Height));
                    Vert.Chroma1 = (BYTE)((pFirstLine[i].Chroma1*(Height-j) + pLastLine[i].Chroma1*j)/(Height));
                    Vert.Chroma2 = (BYTE)((pFirstLine[i].Chroma2*(Height-j) + pLastLine[i].Chroma2*j)/(Height));

                    pTwoPixel[i].Lumi1 = (Hor.Lumi1 + Vert.Lumi1)/2;
                    pTwoPixel[i].Lumi2 = (Hor.Lumi2 + Vert.Lumi2)/2;
                    pTwoPixel[i].Chroma1 = (Hor.Chroma1 + Vert.Chroma1)/2;
                    pTwoPixel[i].Chroma2 = (Hor.Chroma2 + Vert.Chroma2)/2;
                }
            }
            break;
        }
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
			BYTE* lpCurrent_Pointer;
			int top_pitch;

			Width2=Width/2;
			top_pitch=Pitch*Top;

			for (i=0;i<Height;i++)
			{
				left1=*(lpLogoRect+i*Pitch);
				left2=*(lpLogoRect+i*Pitch+1);
				left3=*(lpLogoRect+i*Pitch+2);
				left4=*(lpLogoRect+i*Pitch+3);
				right1=*(lpLogoRect+i*Pitch+Width2*4);
				right2=*(lpLogoRect+i*Pitch+Width2*4+1);
				right3=*(lpLogoRect+i*Pitch+Width2*4+2);
				right4=*(lpLogoRect+i*Pitch+Width2*4+3);
				Mul1=abs(Height-1-i);
				Mul2=i;
				Weight1=abs(((Height-1)/2)-i);
				++Weight1;
				for(j=0;j<Width2;j++)
				{
					Weight2=abs(Width/4-1-j);
					++Weight2;
					Mul3=abs(Width2-1-j);
					Mul4=j;
					up1=*(lpLogoRect+j*4);
					up2=*(lpLogoRect+j*4+1);
					up3=*(lpLogoRect+j*4+2);
					up4=*(lpLogoRect+j*4+3);

					down1=*(lpLogoRect+Pitch*Height+j*4);
					down2=*(lpLogoRect+Pitch*Height+j*4+1);
					down3=*(lpLogoRect+Pitch*Height+j*4+2);
					down4=*(lpLogoRect+Pitch*Height+j*4+3);
					lpCurrent_Pointer=lpLogoRect+i*Pitch+j*4;

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
            mov edi, lpLogoRect8
            mov ebx, Pitch
            // loop over height
            mov eax, Height
            LOOP_SMOOTH_OUTER:
            // loop over width
            mov edx, edi
            mov ecx, Width8
            shr ecx, 2
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
			int Width_third=Width/3/4;
			int Height_third=Height/3;
			int Height_2third=Height/3*2;
        _asm
        {
            mov edi, lpLogoRect8
            mov ebx, Pitch
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
            mov edi, lpLogoRect8
            mov ebx, Pitch
            // loop over height
            mov eax, Height
            LOOP_SMOOTH_OUTER_ASM:
            // loop over width
            mov edx, edi
            mov ecx, Width8
            shr ecx, 2
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
        {
            int i;

            for(i = 0; i < Height; i++)
            {
                memset(lpLogoRect + i*Pitch, 0x7f, Width*2);
            }

            break;
        }
    }
    _asm
    {
        emms
    }
    return 1000;
}
