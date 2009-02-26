/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 Tom Barry & John Adcock.  All rights reserved.
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
//
//  (From Tom Barry)
//  Also, this program is "Philanthropy-Ware".  That is, if you like it and 
//  feel the need to reward or inspire the author then please feel free (but
//  not obligated) to consider joining or donating to the Electronic Frontier
//  Foundation. This will help keep cyber space free of barbed wire and bullsh*t.  
//  See www.eff.org for details
/////////////////////////////////////////////////////////////////////////////

BYTE*    pDest;
const BYTE*    pSrcP;
const BYTE*    pSrc;
const BYTE*    pBob;
const BYTE*    pBobP;

int        src_pitch2 = src_pitch;            // even & odd lines are not interleaved in DScaler

int        dst_pitch2 = 2 * dst_pitch;
int     y;

int     Last8 = (rowsize-8);            // ofs to last 8 bytes in row

int        SaveMotion = 0;
int        dst_pitchw = dst_pitch; // local stor so asm can ref
    pSrc  = pWeaveSrc;            // points 1 weave line above
    pSrcP = pWeaveSrcP;            // " 
    pDest = pWeaveDest + dst_pitch2;

    if (TopFirst)
    {
        pBob = pCopySrc + src_pitch2;      // remember one weave line just copied previously
        pBobP = pCopySrcP + src_pitch2;
    }
    else
    {
        pBob =  pCopySrc;
        pBobP =  pCopySrcP;
    }

    for (y=1; y < FldHeight-1; y++)    
    {

        // pretend it's indented -->>
        __asm
        {
        // Loop general reg usage
        //
        // eax - pBobP, then pDest 
        // ebx - pBob
        // ecx - src_pitch2
        // edx - current offset
        // edi - prev weave pixels, 1 line up
        // esi - next weave pixels, 1 line up




        // simple bob first 8 bytes
        mov        ebx, pBob
        mov        ecx, src_pitch2

        movq    mm0, qword ptr[ebx]
//        pavgb    mm0, qword ptr[ebx+ecx]
        V_PAVGB (mm0, qword ptr[ebx+ecx], mm2, ShiftMask)
        mov        edi, pDest
        V_MOVNTQ    (qword ptr[edi], mm0)

        // simple bob last 8 bytes
        mov        edx, Last8
        lea        esi, [ebx+edx]
        movq    mm0, qword ptr[esi]
//        pavgb    mm0, qword ptr[esi+ecx]
        V_PAVGB    (mm0, qword ptr[esi+ecx], mm2, ShiftMask)
        V_MOVNTQ    (qword ptr[edi+edx], mm0)

        // now loop and get the middle qwords
        mov        esi, pSrc
        mov        edi, pSrcP
        mov        edx, 8                // curr offset into all lines

LoopQ:    
        mov        eax, pBobP
        add        edi, 8        
        add        esi, 8        
        add        ebx, 8
        add        eax, edx
        }

#include "StrangeBob.inc"
