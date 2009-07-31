/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Torbjörn Jansson.  All rights reserved.
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

/**
 * @file ColorConverter.cpp implementation of the CColorConverter class.
 */

#include "stdafx.h"
#include "ColorConverter.h"
#include "cpu.h"
#include <dvdmedia.h>

BYTE *P3_RGBtoYUV(short *dest, BYTE *src, DWORD w);
BYTE *C_RGBtoYUV(short *dest, BYTE *src, DWORD w);

CColorConverter::CColorConverter()
:m_pfnConv(NULL),m_width(0),m_height(0),m_bitcount(0),m_bNeedVertMirror(false)
{

}

CColorConverter::~CColorConverter()
{

}

bool CColorConverter::CanCovert(const AM_MEDIA_TYPE *mt)
{
    if(mt->majortype==MEDIATYPE_Video)
    {
        if(mt->subtype==MEDIASUBTYPE_RGB24)
        {
            return true;
        }
    }
    return false;
}

bool CColorConverter::SetFormat(const AM_MEDIA_TYPE *mt)
{
    if(mt==NULL)
    {
        m_pfnConv=NULL;
        m_width=0;
        m_height=0;
        m_bitcount=0;
        return true;
    }

    if(mt->majortype==MEDIATYPE_Video)
    {
        if(mt->formattype==FORMAT_VideoInfo && mt->cbFormat>0)
        {
            VIDEOINFOHEADER *vh=(VIDEOINFOHEADER *)mt->pbFormat;
            m_width=vh->bmiHeader.biWidth;
            
            m_height=abs(vh->bmiHeader.biHeight);
            m_bitcount=vh->bmiHeader.biBitCount;
        }
        else if(mt->formattype==FORMAT_VideoInfo2 && mt->cbFormat>0)
        {
            VIDEOINFOHEADER2 *vh2=(VIDEOINFOHEADER2 *)mt->pbFormat;
            m_width=vh2->bmiHeader.biWidth;
            
            m_height=abs(vh2->bmiHeader.biHeight);
            m_bitcount=vh2->bmiHeader.biBitCount;
        }
        else
        {
            return false;
        }

        if(mt->subtype==MEDIASUBTYPE_RGB24)
        {
            if(CpuFeatureFlags&(FEATURE_SSE | FEATURE_MMXEXT))
            {
                m_pfnConv=P3_RGBtoYUV;
            }
            else
            {
                m_pfnConv=C_RGBtoYUV;
            }
            //m_bNeedVertMirror= m_height<0 ? false : true;
            m_bNeedVertMirror=true;
            return true;
        }
    }
    return false;
}

bool CColorConverter::Convert(BYTE *dst,BYTE *src,COVERSION_FORMAT cnv,bool &bVertMirror)
{
    if(m_pfnConv==NULL)
    {
        return false;
    }
    
    long SrcLineSize=m_width*m_bitcount/8;
    long DstLineSize=m_width*2;
    bool bVMirror=bVertMirror ? !m_bNeedVertMirror : m_bNeedVertMirror;

    switch(cnv)
    {
    case CNV_ALL:
        {
            for(long i=0;i<m_height;i++)
            {
                long offset=bVMirror ? (m_height-1-i)*SrcLineSize : i*SrcLineSize;
                ATLASSERT(offset>=0);
                m_pfnConv((short*)(dst+i*DstLineSize),src+offset,m_width);
            }
            break;
        }
    case CNV_EVEN:
        {
            for(long i=0;i<m_height/2;i++)
            {
                //if doing vert mirror, use the odd line instead
                long offset=bVMirror ? (m_height-1-(2*i+1))*SrcLineSize : (2*i)*SrcLineSize;
                ATLASSERT(offset>=0);
                m_pfnConv((short*)(dst+i*DstLineSize),src+offset,m_width);
            }
            break;
        }
    case CNV_ODD:
        {
            for(long i=0;i<m_height/2;i++)
            {
                long offset=bVMirror ? (m_height-1-(2*i))*SrcLineSize : (2*i+1)*SrcLineSize;
                ATLASSERT(offset>=0);
                m_pfnConv((short*)(dst+i*DstLineSize),src+offset,m_width);
            }
            break;
        }
    default:
        return false;
    }
    bVertMirror=bVMirror;
    return true;
}

/*

  Y = R *  .299 + G *  .587 + B *  .114;
  U = R * -.168 + G * -.332 + B *  .500 + 128.;
  V = R *  .500 + G * -.419 + B * -.081 + 128.;

  These are the ones used by an example on Intel's dev site...
  If we go with these, we should recalculate the inverse for the other mmx func.

  Y = 0.299 R + 0.587 G + 0.114 B
  U =-0.146 R - 0.288 G + 0.434 B            
  V = 0.617 R - 0.517 G - 0.100 G

  Y = [(9798 R + 19235G + 3736 B) / 32768]
  U = [(-4784 R - 9437 G + 4221 B) / 32768] + 128    
  V = [(20218R - 16941G - 3277 B) / 32768] + 128

*/

// Note names are most-signif first, but the contents are least-signif first.
static short const128_0_128_0[4] = { 0, 128, 0, 128 };
static short UgUbYgYb[4] = { 3736, 19235,  16384, -10879 };
static short Ur0Yr0[4] =   {    0,  9798,     0,   -5505 };
static short VgVbYgYb[4] = { 3736, 19235, -2654,  -13730 };
static short Vr0Yr0[4] =   {    0,  9798,     0,   16384 };

BYTE *P3_RGBtoYUV(short *dest, BYTE *src, DWORD w)
{
    _asm
    {
        mov esi, src
        mov edi, dest
        mov eax, w
        shr eax, 2

        pxor mm0, mm0              ; mm0 = 0, constant
        movq mm7, const128_0_128_0 ; constant

    next4:
        // Process 4 pixels.  First YUYV is a, second is b.
        // Source must be 12-byte aligned.
        movd mm1, [esi+0]      ; mm1 = X X X X B1a R0a G0a B0a
        movd mm2, [esi+4]      ; mm2 = X X X X G0b B0b R1a G1a
        movd mm3, [esi+8]      ; mm3 = X X X X R1b G1b B1b R0b
        add esi, 12

        punpcklbw mm1, mm0     ; mm1 = B1a R0a G0a B0a
        punpcklbw mm2, mm0     ; mm2 = G0b B0b R1a G1a
        punpcklbw mm3, mm0     ; mm3 = R1b G1b B1b R0b


        pshufw mm4, mm1, 0x44  ; mm4 = G0a B0a G0a B0a
        pmaddwd mm4, UgUbYgYb  ; mm4 =   Ugb     Ygb

        pshufw mm5, mm1, 0x88  ; mm5 = R0a XXX R0a XXX
        pmaddwd mm5, Ur0Yr0    ; mm5 =   Ur      Yr

        paddd mm4, mm5         ; mm4 =   Ua      Y0a
        psrad mm4, 15          ; mm4 >>= 15


        pshufw mm5, mm1, 0x0f  ; mm5 = XXX XXX B1a B1a
        pshufw mm6, mm2, 0x00  ; mm6 = XXX XXX G1a G1a

        punpcklwd mm5, mm6     ; mm5 = G1a B1a G1a B1a
        pmaddwd mm5, VgVbYgYb  ; mm5 =   Vgb     Ygb

        pshufw mm6, mm2, 0x44  ; mm6 = R1a XXX R1a XXX
        pmaddwd mm6, Vr0Yr0    ; mm6 =   Vr      Yr

        paddd mm5, mm6         ; mm5 =   Va      Y1a
        psrad mm5, 15          ; mm5 >>= 15


        pshufw mm2, mm2, 0xee  ; mm2 = G0b B0b G0b B0b
        pmaddwd mm2, UgUbYgYb  ; mm2 =   Ugb     Ygb

        pshufw mm1, mm3, 0x00  ; mm1 = R0b XXX R0b XXX
        pmaddwd mm1, Ur0Yr0    ; mm1 =   Ur      Yr

        paddd mm2, mm1         ; mm2 =   Ub      Y0b
        psrad mm2, 15          ; mm2 >>= 15


        pshufw mm6, mm3, 0x99  ; mm6 = G1b B1b G1b B1b
        pmaddwd mm6, VgVbYgYb  ; mm6 =   Vgb     Ygb

        pshufw mm3, mm3, 0xcc  ; mm3 = R1b XXX R1b XXX
        pmaddwd mm3, Vr0Yr0    ; mm3 =   Vr      Yr

        paddd mm6, mm3         ; mm6 =   Vb      Y1b
        psrad mm6, 15          ; mm6 >>= 15


        packssdw mm4, mm5      ; mm4 = Va Y1a Ua Y0a
        packssdw mm2, mm6      ; mm2 = Vb Y1b Ub Y0b

        paddsw mm4, mm7        ; mm4 = Va+128 Y1a Ua+128 Y0a
        paddsw mm2, mm7        ; mm2 = Vb+128 Y1b Ub+128 Y0b

        packuswb mm4, mm2   ; mm4 = Vb+128 Y1b Ub+128 Y0b Va+128 Y1a Ua+128 Y0a


        // Destination must be 8-byte aligned.
        movq [edi], mm4
        add edi, 8


        dec eax
        jnz next4

        mov src, esi
        emms
    }

    return src;
}

// The c-equivalent if P3-or-better is not present in cpu flags.
BYTE *C_RGBtoYUV(short *dest, BYTE *src, DWORD w)
{
    LPBYTE dst = (LPBYTE)dest;
    DWORD w2 = w >> 1;
    while (w2--)
    {
        int B = *src++;
        int G = *src++;
        int R = *src++;

        int Y = (R * 9798 + G * 19235 + B * 3736) >> 15;
        int U = ((R * -5505 + G * -10879 + B * 16384) >> 15) + 128;

    #define clip(x) (((x) & 0xffffff00) ? ((x) & 0x80000000) ? 0 : 255 : (x))

        *dst++ = clip(Y);
        *dst++ = clip(U);

        B = *src++;
        G = *src++;
        R = *src++;

        Y = (R * 9798 + G * 19235 + B * 3736) >> 15;
        int V = ((R * 16384 + G * -13730 + B * -2654) >> 15) + 128;

        *dst++ = clip(Y);
        *dst++ = clip(V);
    }

    return src;
}