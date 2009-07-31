;////////////////////////////////////////////////////////////////////////////
; $Id$
;////////////////////////////////////////////////////////////////////////////
; Copyright (c) 2000 John Adcock. All rights reserved.
;////////////////////////////////////////////////////////////////////////////
;
;   This file is subject to the terms of the GNU General Public License as
;   published by the Free Software Foundation.  A copy of this license is
;   included with this software distribution in the file COPYING.  If you
;   do not have a copy, you may obtain a copy by writing to the Free
;   Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
;
;   This software is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details
;////////////////////////////////////////////////////////////////////////////
;
; Refinements made by Mark Rejhon and Steve Grimm
;
;////////////////////////////////////////////////////////////////////////////

    .386p
    .mmx
    .xmm
    .model  flat

    .code
;////////////////////////////////////////////////////////////////////
; void memcpyMMX(void* Dest, void* Src, size_t nBytes);
;
; Uses MMX instructions to move memory around
; does as much as we can in 64 byte chunks (128-byte on SSE machines)
; using MMX instructions
; then copies any extra bytes
; assumes there will be at least 64 bytes to copy
; This code was originally from Borg's bTV plugin SDK 
;////////////////////////////////////////////////////////////////////

public _memcpyMMX

    Dest    equ [esp+12]
    Src     equ [esp+16]
    nBytes  equ [esp+20]

_memcpyMMX:
    push    edi
    push    esi

    mov     esi, dword ptr[Src]
    mov     edi, dword ptr[Dest]
    mov     ecx, nBytes
    shr     ecx, 6                      ; nBytes / 64
align 4
Memcpy_Loop:
    movq    mm0, qword ptr[esi]
    movq    mm1, qword ptr[esi+8*1]
    movq    mm2, qword ptr[esi+8*2]
    movq    mm3, qword ptr[esi+8*3]
    movq    mm4, qword ptr[esi+8*4]
    movq    mm5, qword ptr[esi+8*5]
    movq    mm6, qword ptr[esi+8*6]
    movq    mm7, qword ptr[esi+8*7]
    movq    qword ptr[edi], mm0
    movq    qword ptr[edi+8*1], mm1
    movq    qword ptr[edi+8*2], mm2
    movq    qword ptr[edi+8*3], mm3
    movq    qword ptr[edi+8*4], mm4
    movq    qword ptr[edi+8*5], mm5
    movq    qword ptr[edi+8*6], mm6
    movq    qword ptr[edi+8*7], mm7
    add     esi, 64
    add     edi, 64
    loop Memcpy_Loop

    mov     ecx, nBytes
    and     ecx, 63
    cmp     ecx, 0
    je Memcpy_End
    rep movsb

Memcpy_End:
    emms
    pop esi
    pop edi
    ret 


;////////////////////////////////////////////////////////////////////
; void memcpySSE(void* Dest, void* Src, size_t nBytes);
;
; On SSE machines we use the 
; bypass write caching to copy a bit faster.  The destination has to be
; 16-byte aligned.  
;////////////////////////////////////////////////////////////////////
public _memcpySSE

    Dest    equ [esp+12]
    Src equ [esp+16]
    nBytes  equ [esp+20]

_memcpySSE:
    push    edi
    push    esi

    mov     esi, dword ptr[Src]
    mov     edi, dword ptr[Dest]
    mov     ecx, nBytes
    shr     ecx, 7
align 4
MemcpySSE_Loop:
    ; movaps should be slightly more efficient
    ; as the data is 16 bit aligned
    movaps  xmm0, [esi]
    movaps  xmm1, [esi+16*1]
    movaps  xmm2, [esi+16*2]
    movaps  xmm3, [esi+16*3]
    movaps  xmm4, [esi+16*4]
    movaps  xmm5, [esi+16*5]
    movaps  xmm6, [esi+16*6]
    movaps  xmm7, [esi+16*7]
    movntps [edi], xmm0
    movntps [edi+16*1], xmm1
    movntps [edi+16*2], xmm2
    movntps [edi+16*3], xmm3
    movntps [edi+16*4], xmm4
    movntps [edi+16*5], xmm5
    movntps [edi+16*6], xmm6
    movntps [edi+16*7], xmm7
    add     esi, 128
    add     edi, 128
    dec ecx
    jnz MemcpySSE_Loop

    mov     ecx, nBytes
    and     ecx, 127
    cmp     ecx, 0
    je MemcpySSE_End
    rep movsb

MemcpySSE_End:
    pop esi
    pop edi
    ret 

;////////////////////////////////////////////////////////////////////
; void memcpyAMD(void* Dest, void* Src, size_t nBytes);
;
; Comes from AMD Site
;////////////////////////////////////////////////////////////////////

public _memcpyAMD

    Dest    equ [esp+12]
    Src equ [esp+16]
    nBytes  equ [esp+20]

_memcpyAMD:
    push    edi
    push    esi

    mov     esi, dword ptr[Src]
    mov     edi, dword ptr[Dest]
    mov     ecx, nBytes
    shr     ecx, 6                      ; nBytes / 64
align 4
MemcpyAMD_Loop:
    movq mm0,[esi+0]
    movq mm1,[esi+8]
    movq [edi+0],mm0
    movq [edi+8],mm1
    movq mm2,[esi+16]
    movq mm3,[esi+24]
    movq [edi+16],mm2
    movq [edi+24],mm3
    movq mm0,[esi+32]
    movq mm1,[esi+40]
    movq [edi+32],mm0
    movq [edi+40],mm1
    movq mm2,[esi+48]
    movq mm3,[esi+56]
    movq [edi+48],mm2
    movq [edi+56],mm3
    add     esi, 64
    add     edi, 64
    loop MemcpyAMD_Loop

    mov ecx, nBytes
    shr ecx,2               ; dword count
    and ecx,1111b
    neg ecx
    add ecx, MemcpyAMD_End
    jmp ecx

align 4
    movsd
    movsd                   ; perform last 1-15 dword copies
    movsd
    movsd
    movsd
    movsd
    movsd
    movsd
    movsd
    movsd                   ; perform last 1-7 dword copies
    movsd
    movsd
    movsd
    movsd
    movsd
    movsd

MemcpyAMD_End:
    pop esi
    pop edi
    ret 

;////////////////////////////////////////////////////////////////////
; void memcpySimple(void* Dest, void* Src, size_t nBytes);
;
; On SSE machines we use the 
; bypass write caching to copy a bit faster.  The destination has to be
; 16-byte aligned.  
;////////////////////////////////////////////////////////////////////
public _memcpySimple

    Dest    equ [esp+12]
    Src equ [esp+16]
    nBytes  equ [esp+20]

_memcpySimple:
    push    edi
    push    esi

    mov     esi, dword ptr[Src]
    mov     edi, dword ptr[Dest]
    mov     ecx, nBytes
    rep movsb

    pop esi
    pop edi
    ret 
    
    end

